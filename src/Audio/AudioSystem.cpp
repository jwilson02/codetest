#include "AudioSystem.h"
#include "MusicPlayer.h"
#include "SoundEffect.h"
#include "AudioSource.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>

// For JSON parsing (you'll need to link a JSON library like nlohmann/json)
// #include <nlohmann/json.hpp>
// using json = nlohmann::json;

namespace Audio {

AudioSystem* AudioSystem::s_instance = nullptr;

AudioSystem& AudioSystem::getInstance() {
    if (!s_instance) {
        s_instance = new AudioSystem();
    }
    return *s_instance;
}

AudioSystem::~AudioSystem() {
    shutdown();
}

bool AudioSystem::initialize(const AudioConfig& config) {
    if (m_initialized) {
        std::cerr << "AudioSystem already initialized" << std::endl;
        return true;
    }

    m_config = config;

    // Initialize SDL audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL audio: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(config.frequency, config.format, config.channels, config.chunkSize) < 0) {
        std::cerr << "Failed to open audio device: " << Mix_GetError() << std::endl;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }

    // Allocate mixing channels
    Mix_AllocateChannels(config.maxChannels);

    // Initialize audio format support
    int flags = MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC;
    int initted = Mix_Init(flags);
    if ((initted & flags) != flags) {
        std::cout << "Warning: Not all audio formats initialized: " << Mix_GetError() << std::endl;
    }

    // Set initial volumes
    Mix_VolumeMusic(static_cast<int>(config.musicVolume * MIX_MAX_VOLUME));
    Mix_Volume(-1, static_cast<int>(config.sfxVolume * MIX_MAX_VOLUME));

    // Initialize category volumes
    m_categoryVolumes[AudioCategory::Music] = config.musicVolume;
    m_categoryVolumes[AudioCategory::SFX] = config.sfxVolume;
    m_categoryVolumes[AudioCategory::Ambient] = config.ambientVolume;
    m_categoryVolumes[AudioCategory::Voice] = config.voiceVolume;
    m_categoryVolumes[AudioCategory::UI] = 1.0f;
    m_categoryVolumes[AudioCategory::Footsteps] = 0.6f;
    m_categoryVolumes[AudioCategory::Combat] = 0.9f;
    m_categoryVolumes[AudioCategory::Environment] = 0.7f;

    // Create music player
    m_musicPlayer = std::make_unique<MusicPlayer>();

    // Set callbacks
    Mix_ChannelFinished(channelFinishedCallback);
    Mix_HookMusicFinished(musicFinishedCallback);

    m_initialized = true;
    std::cout << "AudioSystem initialized successfully" << std::endl;
    std::cout << "  Frequency: " << config.frequency << " Hz" << std::endl;
    std::cout << "  Channels: " << config.channels << std::endl;
    std::cout << "  Max mixing channels: " << config.maxChannels << std::endl;

    return true;
}

void AudioSystem::shutdown() {
    if (!m_initialized) {
        return;
    }

    // Stop all audio
    stopAllChannels();
    if (m_musicPlayer) {
        m_musicPlayer->stop();
    }

    // Clean up sound effects
    for (auto& pair : m_soundEffects) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    m_soundEffects.clear();

    // Clean up audio sources
    m_audioSources.clear();

    // Clean up music player
    m_musicPlayer.reset();

    // Shutdown SDL_mixer
    Mix_CloseAudio();
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    m_initialized = false;
    std::cout << "AudioSystem shut down" << std::endl;
}

void AudioSystem::update(float deltaTime) {
    if (!m_initialized) {
        return;
    }

    // Update music player
    if (m_musicPlayer) {
        m_musicPlayer->update(deltaTime);
    }

    // Update ducking
    updateDucking(deltaTime);

    // Update 3D audio sources
    for (auto& source : m_audioSources) {
        source->update(deltaTime, m_listener);
    }

    // Clean up finished audio sources
    cleanupFinishedSources();

    // Update channel volumes based on category and ducking
    for (auto& pair : m_activeChannels) {
        int channel = pair.first;
        ChannelInfo& info = pair.second;

        if (Mix_Playing(channel)) {
            float volume = info.baseVolume * getCategoryVolume(info.category);

            // Apply ducking if active
            auto duckIt = m_ducking.find(info.category);
            if (duckIt != m_ducking.end() && duckIt->second.enabled && m_isDucking) {
                volume *= (1.0f - duckIt->second.currentAmount);
            }

            // Apply master volume
            volume *= m_config.masterVolume;

            // Apply mute
            if (m_muted) {
                volume = 0.0f;
            }

            Mix_Volume(channel, static_cast<int>(volume * MIX_MAX_VOLUME));
        }
    }
}

void AudioSystem::setConfig(const AudioConfig& config) {
    m_config = config;

    // Apply new volumes
    setMasterVolume(config.masterVolume);
    setMusicVolume(config.musicVolume);
    setSFXVolume(config.sfxVolume);
    setAmbientVolume(config.ambientVolume);
    setVoiceVolume(config.voiceVolume);
}

void AudioSystem::setMasterVolume(float volume) {
    m_config.masterVolume = std::clamp(volume, 0.0f, 1.0f);
}

void AudioSystem::setMusicVolume(float volume) {
    m_config.musicVolume = std::clamp(volume, 0.0f, 1.0f);
    m_categoryVolumes[AudioCategory::Music] = m_config.musicVolume;

    if (m_musicPlayer) {
        m_musicPlayer->setVolume(m_config.musicVolume * m_config.masterVolume);
    }
}

void AudioSystem::setSFXVolume(float volume) {
    m_config.sfxVolume = std::clamp(volume, 0.0f, 1.0f);
    m_categoryVolumes[AudioCategory::SFX] = m_config.sfxVolume;
}

void AudioSystem::setAmbientVolume(float volume) {
    m_config.ambientVolume = std::clamp(volume, 0.0f, 1.0f);
    m_categoryVolumes[AudioCategory::Ambient] = m_config.ambientVolume;
}

void AudioSystem::setVoiceVolume(float volume) {
    m_config.voiceVolume = std::clamp(volume, 0.0f, 1.0f);
    m_categoryVolumes[AudioCategory::Voice] = m_config.voiceVolume;
}

void AudioSystem::setCategoryVolume(AudioCategory category, float volume) {
    m_categoryVolumes[category] = std::clamp(volume, 0.0f, 1.0f);
}

float AudioSystem::getCategoryVolume(AudioCategory category) const {
    auto it = m_categoryVolumes.find(category);
    if (it != m_categoryVolumes.end()) {
        return it->second;
    }
    return 1.0f;
}

void AudioSystem::setMuted(bool muted) {
    if (m_muted == muted) {
        return;
    }

    m_muted = muted;

    if (muted) {
        m_preMuteVolume = m_config.masterVolume;
        Mix_VolumeMusic(0);
        Mix_Volume(-1, 0);
    } else {
        m_config.masterVolume = m_preMuteVolume;
        Mix_VolumeMusic(static_cast<int>(m_config.musicVolume * m_config.masterVolume * MIX_MAX_VOLUME));
    }
}

void AudioSystem::toggleMute() {
    setMuted(!m_muted);
}

void AudioSystem::setListenerPosition(float x, float y, float z) {
    m_listener.x = x;
    m_listener.y = y;
    m_listener.z = z;
}

void AudioSystem::setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                         float upX, float upY, float upZ) {
    m_listener.forwardX = forwardX;
    m_listener.forwardY = forwardY;
    m_listener.forwardZ = forwardZ;
    m_listener.upX = upX;
    m_listener.upY = upY;
    m_listener.upZ = upZ;
}

void AudioSystem::setAudioState(AudioState state) {
    if (m_currentState == state) {
        return;
    }

    m_currentState = state;

    // Notify music player of state change
    if (m_musicPlayer) {
        m_musicPlayer->onStateChanged(state);
    }

    std::cout << "Audio state changed to: " << static_cast<int>(state) << std::endl;
}

bool AudioSystem::loadSoundEffect(const std::string& name, const std::string& filePath) {
    // Check if already loaded
    if (m_soundEffects.find(name) != m_soundEffects.end()) {
        std::cout << "Sound effect '" << name << "' already loaded" << std::endl;
        return true;
    }

    Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str());
    if (!chunk) {
        std::cerr << "Failed to load sound effect '" << name << "': " << Mix_GetError() << std::endl;
        return false;
    }

    m_soundEffects[name] = chunk;
    std::cout << "Loaded sound effect: " << name << " from " << filePath << std::endl;
    return true;
}

bool AudioSystem::loadMusic(const std::string& name, const std::string& filePath) {
    if (!m_musicPlayer) {
        std::cerr << "Music player not initialized" << std::endl;
        return false;
    }

    return m_musicPlayer->loadTrack(name, filePath);
}

bool AudioSystem::loadSoundMapping(const std::string& jsonPath) {
    // TODO: Implement JSON parsing for sound mappings
    // This would load a JSON file that maps sound names to file paths
    std::cout << "Loading sound mapping from: " << jsonPath << std::endl;

    // Example implementation would parse JSON and call loadSoundEffect for each entry

    return true;
}

bool AudioSystem::loadMusicPlaylist(const std::string& jsonPath) {
    if (!m_musicPlayer) {
        std::cerr << "Music player not initialized" << std::endl;
        return false;
    }

    return m_musicPlayer->loadPlaylist(jsonPath);
}

int AudioSystem::playSoundEffect(const std::string& name, float volume, int loops, AudioCategory category) {
    auto it = m_soundEffects.find(name);
    if (it == m_soundEffects.end()) {
        std::cerr << "Sound effect not found: " << name << std::endl;
        return -1;
    }

    // Set chunk volume
    Mix_VolumeChunk(it->second, static_cast<int>(volume * MIX_MAX_VOLUME));

    // Play on first available channel
    int channel = Mix_PlayChannel(-1, it->second, loops);
    if (channel == -1) {
        std::cerr << "No available channels to play sound: " << name << std::endl;
        return -1;
    }

    // Track channel info
    ChannelInfo info;
    info.category = category;
    info.soundName = name;
    info.baseVolume = volume;
    info.is3D = false;
    m_activeChannels[channel] = info;

    applyVolumeToChannel(channel, category);

    return channel;
}

int AudioSystem::playSoundEffect3D(const std::string& name, float x, float y, float z,
                                   float volume, AudioCategory category) {
    auto it = m_soundEffects.find(name);
    if (it == m_soundEffects.end()) {
        std::cerr << "Sound effect not found: " << name << std::endl;
        return -1;
    }

    // Calculate distance and attenuation
    float distance = calculateDistance(x, y, z, m_listener.x, m_listener.y, m_listener.z);
    float attenuation = calculateAttenuation(distance);

    // Apply occlusion
    float finalVolume = volume * attenuation;
    if (m_config.enableOcclusion && m_occlusionFactor > 0.0f) {
        finalVolume *= (1.0f - m_occlusionFactor);
    }

    Mix_VolumeChunk(it->second, static_cast<int>(finalVolume * MIX_MAX_VOLUME));

    // Calculate panning (left-right positioning)
    float dx = x - m_listener.x;
    float angle = std::atan2(dx, z - m_listener.z) * 180.0f / M_PI;
    int panAngle = static_cast<int>(angle);

    int channel = Mix_PlayChannel(-1, it->second, 0);
    if (channel == -1) {
        return -1;
    }

    // Set panning for stereo positioning
    Mix_SetPosition(channel, panAngle, static_cast<Uint8>(distance));

    // Track channel info
    ChannelInfo info;
    info.category = category;
    info.soundName = name;
    info.baseVolume = finalVolume;
    info.is3D = true;
    m_activeChannels[channel] = info;

    return channel;
}

void AudioSystem::stopChannel(int channel) {
    Mix_HaltChannel(channel);
    m_activeChannels.erase(channel);
}

void AudioSystem::stopAllChannels() {
    Mix_HaltChannel(-1);
    m_activeChannels.clear();
}

void AudioSystem::enableDucking(AudioCategory category, float duckAmount) {
    DuckingInfo& info = m_ducking[category];
    info.enabled = true;
    info.targetAmount = std::clamp(duckAmount, 0.0f, 1.0f);
    m_isDucking = true;
}

void AudioSystem::disableDucking(AudioCategory category) {
    auto it = m_ducking.find(category);
    if (it != m_ducking.end()) {
        it->second.enabled = false;

        // Check if any ducking is still active
        bool anyActive = false;
        for (const auto& pair : m_ducking) {
            if (pair.second.enabled) {
                anyActive = true;
                break;
            }
        }
        m_isDucking = anyActive;
    }
}

void AudioSystem::setReverbAmount(float amount) {
    m_reverbAmount = std::clamp(amount, 0.0f, 1.0f);
    // Note: SDL_mixer doesn't have built-in reverb, you'd need to implement
    // this with audio effects or use a more advanced audio library
}

void AudioSystem::setOcclusionFactor(float factor) {
    m_occlusionFactor = std::clamp(factor, 0.0f, 1.0f);
}

void AudioSystem::setDopplerFactor(float factor) {
    m_dopplerFactor = std::max(0.0f, factor);
}

bool AudioSystem::isChannelPlaying(int channel) const {
    return Mix_Playing(channel) != 0;
}

void AudioSystem::setChannelVolume(int channel, float volume) {
    Mix_Volume(channel, static_cast<int>(std::clamp(volume, 0.0f, 1.0f) * MIX_MAX_VOLUME));
}

void AudioSystem::fadeOutChannel(int channel, int ms) {
    Mix_FadeOutChannel(channel, ms);
}

void AudioSystem::setChannelFinishedCallback(std::function<void(int)> callback) {
    m_channelFinishedCallback = callback;
}

void AudioSystem::setMusicFinishedCallback(std::function<void()> callback) {
    m_musicFinishedCallback = callback;
}

int AudioSystem::getActiveChannels() const {
    return Mix_Playing(-1);
}

int AudioSystem::getAvailableChannels() const {
    return m_config.maxChannels - Mix_Playing(-1);
}

size_t AudioSystem::getLoadedSoundsCount() const {
    return m_soundEffects.size();
}

std::vector<std::string> AudioSystem::getSupportedFormats() {
    return {"WAV", "OGG", "MP3", "FLAC", "AIFF", "VOC"};
}

// Private methods

void AudioSystem::applyVolumeToChannel(int channel, AudioCategory category) {
    auto it = m_activeChannels.find(channel);
    if (it == m_activeChannels.end()) {
        return;
    }

    float volume = it->second.baseVolume * getCategoryVolume(category) * m_config.masterVolume;

    if (m_muted) {
        volume = 0.0f;
    }

    Mix_Volume(channel, static_cast<int>(volume * MIX_MAX_VOLUME));
}

float AudioSystem::calculateDistance(float x1, float y1, float z1, float x2, float y2, float z2) const {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

float AudioSystem::calculateAttenuation(float distance, float maxDistance) const {
    if (distance <= 0.0f) {
        return 1.0f;
    }

    if (distance >= maxDistance) {
        return 0.0f;
    }

    // Inverse square law with minimum distance
    float minDistance = 1.0f;
    if (distance < minDistance) {
        return 1.0f;
    }

    float attenuation = minDistance / distance;
    return std::clamp(attenuation, 0.0f, 1.0f);
}

void AudioSystem::updateDucking(float deltaTime) {
    for (auto& pair : m_ducking) {
        DuckingInfo& info = pair.second;

        if (info.enabled && m_isDucking) {
            // Duck down
            info.currentAmount += info.speed * deltaTime;
            info.currentAmount = std::min(info.currentAmount, info.targetAmount);
        } else {
            // Duck up (restore volume)
            info.currentAmount -= info.speed * deltaTime;
            info.currentAmount = std::max(info.currentAmount, 0.0f);
        }
    }
}

void AudioSystem::cleanupFinishedSources() {
    m_audioSources.erase(
        std::remove_if(m_audioSources.begin(), m_audioSources.end(),
                      [](const std::unique_ptr<AudioSource>& source) {
                          return source->isFinished();
                      }),
        m_audioSources.end()
    );
}

// Static callbacks

void AudioSystem::channelFinishedCallback(int channel) {
    AudioSystem& audio = getInstance();

    // Remove from active channels
    audio.m_activeChannels.erase(channel);

    // Call user callback if set
    if (audio.m_channelFinishedCallback) {
        audio.m_channelFinishedCallback(channel);
    }
}

void AudioSystem::musicFinishedCallback() {
    AudioSystem& audio = getInstance();

    // Call user callback if set
    if (audio.m_musicFinishedCallback) {
        audio.m_musicFinishedCallback();
    }
}

} // namespace Audio
