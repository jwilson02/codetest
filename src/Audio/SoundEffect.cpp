#include "SoundEffect.h"
#include "AudioSystem.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <cmath>

namespace Audio {

// Random number generator for variations
static std::random_device rd;
static std::mt19937 gen(rd());

// SoundEffect implementation

SoundEffect::SoundEffect(const std::string& name)
    : m_name(name)
    , m_category(AudioCategory::SFX)
    , m_priority(SoundPriority::Normal) {
}

SoundEffect::~SoundEffect() {
    // Free all variations
    for (auto& variation : m_variations) {
        if (variation.chunk) {
            Mix_FreeChunk(variation.chunk);
        }
    }
}

bool SoundEffect::load(const std::string& filePath) {
    Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str());
    if (!chunk) {
        std::cerr << "Failed to load sound effect '" << m_name << "': " << Mix_GetError() << std::endl;
        return false;
    }

    SoundVariation variation;
    variation.filePath = filePath;
    variation.chunk = chunk;
    variation.pitchVariation = 0.0f;
    variation.volumeVariation = 0.0f;

    m_variations.push_back(variation);

    std::cout << "Loaded sound effect: " << m_name << " from " << filePath << std::endl;
    return true;
}

bool SoundEffect::loadVariations(const std::vector<std::string>& filePaths) {
    if (filePaths.empty()) {
        std::cerr << "No file paths provided for sound variations" << std::endl;
        return false;
    }

    for (const auto& filePath : filePaths) {
        if (!load(filePath)) {
            return false;
        }
    }

    std::cout << "Loaded " << filePaths.size() << " variations for sound: " << m_name << std::endl;
    return true;
}

void SoundEffect::setVolume(float volume) {
    m_volume = std::clamp(volume, 0.0f, 1.0f);

    // Update all loaded chunks
    for (auto& variation : m_variations) {
        if (variation.chunk) {
            Mix_VolumeChunk(variation.chunk, static_cast<int>(m_volume * MIX_MAX_VOLUME));
        }
    }
}

int SoundEffect::play(int loops) {
    if (m_variations.empty()) {
        std::cerr << "Cannot play sound '" << m_name << "': no variations loaded" << std::endl;
        return -1;
    }

    Mix_Chunk* chunk = getRandomVariation();
    if (!chunk) {
        return -1;
    }

    float volume = getRandomVolume();
    Mix_VolumeChunk(chunk, static_cast<int>(volume * MIX_MAX_VOLUME));

    int channel = Mix_PlayChannel(-1, chunk, loops);
    if (channel == -1) {
        std::cerr << "Failed to play sound '" << m_name << "': " << Mix_GetError() << std::endl;
        return -1;
    }

    m_activeInstances.push_back(channel);
    return channel;
}

int SoundEffect::play3D(float x, float y, float z, int loops) {
    if (m_variations.empty()) {
        std::cerr << "Cannot play sound '" << m_name << "': no variations loaded" << std::endl;
        return -1;
    }

    // Get audio system for listener position
    AudioSystem& audio = AudioSystem::getInstance();
    const AudioListener& listener = audio.getListener();

    // Calculate distance
    float dx = x - listener.x;
    float dy = y - listener.y;
    float dz = z - listener.z;
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    // Calculate attenuation
    float attenuation = 1.0f;
    if (distance > m_minDistance) {
        if (distance >= m_maxDistance) {
            return -1;  // Too far, don't play
        }

        attenuation = m_minDistance / distance;
        attenuation = std::clamp(attenuation, 0.0f, 1.0f);
    }

    Mix_Chunk* chunk = getRandomVariation();
    if (!chunk) {
        return -1;
    }

    float volume = getRandomVolume() * attenuation;
    Mix_VolumeChunk(chunk, static_cast<int>(volume * MIX_MAX_VOLUME));

    // Calculate panning
    float angle = std::atan2(dx, dz) * 180.0f / M_PI;
    int panAngle = static_cast<int>(angle);

    int channel = Mix_PlayChannel(-1, chunk, loops);
    if (channel == -1) {
        return -1;
    }

    // Set 3D position
    Mix_SetPosition(channel, panAngle, static_cast<Uint8>(std::min(distance, 255.0f)));

    m_activeInstances.push_back(channel);
    return channel;
}

void SoundEffect::stop() {
    for (int channel : m_activeInstances) {
        Mix_HaltChannel(channel);
    }
    m_activeInstances.clear();
}

void SoundEffect::stopInstance(int channel) {
    Mix_HaltChannel(channel);

    auto it = std::find(m_activeInstances.begin(), m_activeInstances.end(), channel);
    if (it != m_activeInstances.end()) {
        m_activeInstances.erase(it);
    }
}

void SoundEffect::onChannelFinished(int channel) {
    auto it = std::find(m_activeInstances.begin(), m_activeInstances.end(), channel);
    if (it != m_activeInstances.end()) {
        m_activeInstances.erase(it);
    }
}

Mix_Chunk* SoundEffect::getRandomVariation() {
    if (m_variations.empty()) {
        return nullptr;
    }

    if (m_variations.size() == 1) {
        return m_variations[0].chunk;
    }

    // Round-robin with random skip to avoid repetition
    std::uniform_int_distribution<> dis(0, static_cast<int>(m_variations.size()) - 1);
    int index = dis(gen);

    // Avoid playing the same variation twice in a row
    if (index == m_lastVariationIndex && m_variations.size() > 1) {
        index = (index + 1) % m_variations.size();
    }

    m_lastVariationIndex = index;
    return m_variations[index].chunk;
}

float SoundEffect::getRandomPitch() {
    if (m_pitchVariation <= 0.0f) {
        return 1.0f;
    }

    std::uniform_real_distribution<> dis(-m_pitchVariation, m_pitchVariation);
    return 1.0f + static_cast<float>(dis(gen));
}

float SoundEffect::getRandomVolume() {
    float volume = m_volume;

    if (m_volumeVariation > 0.0f) {
        std::uniform_real_distribution<> dis(-m_volumeVariation, m_volumeVariation);
        volume += static_cast<float>(dis(gen));
        volume = std::clamp(volume, 0.0f, 1.0f);
    }

    return volume;
}

// SoundEffectManager implementation

SoundEffectManager::SoundEffectManager() {
    // Set default max instances per category
    m_categoryMaxInstances[AudioCategory::Music] = 1;
    m_categoryMaxInstances[AudioCategory::SFX] = 16;
    m_categoryMaxInstances[AudioCategory::Ambient] = 8;
    m_categoryMaxInstances[AudioCategory::Voice] = 4;
    m_categoryMaxInstances[AudioCategory::UI] = 8;
    m_categoryMaxInstances[AudioCategory::Footsteps] = 2;
    m_categoryMaxInstances[AudioCategory::Combat] = 12;
    m_categoryMaxInstances[AudioCategory::Environment] = 10;
}

SoundEffectManager::~SoundEffectManager() {
    stopAll();
}

bool SoundEffectManager::loadSound(const std::string& name, const std::string& filePath,
                                   AudioCategory category) {
    auto sound = std::make_unique<SoundEffect>(name);
    sound->setCategory(category);

    if (!sound->load(filePath)) {
        return false;
    }

    m_sounds[name] = std::move(sound);
    return true;
}

bool SoundEffectManager::loadSoundWithVariations(const std::string& name,
                                                 const std::vector<std::string>& filePaths,
                                                 AudioCategory category) {
    auto sound = std::make_unique<SoundEffect>(name);
    sound->setCategory(category);

    if (!sound->loadVariations(filePaths)) {
        return false;
    }

    m_sounds[name] = std::move(sound);
    return true;
}

bool SoundEffectManager::loadFromJSON(const std::string& jsonPath) {
    // TODO: Implement JSON parsing for sound mappings
    std::cout << "Loading sound mapping from: " << jsonPath << std::endl;

    // Example JSON structure:
    // {
    //   "sounds": [
    //     {
    //       "name": "footstep_grass",
    //       "category": "footsteps",
    //       "priority": "low",
    //       "volume": 0.6,
    //       "variations": [
    //         "assets/audio/sfx/footstep_grass_01.wav",
    //         "assets/audio/sfx/footstep_grass_02.wav",
    //         "assets/audio/sfx/footstep_grass_03.wav"
    //       ],
    //       "pitchVariation": 0.1,
    //       "volumeVariation": 0.2
    //     }
    //   ]
    // }

    return true;
}

int SoundEffectManager::playSound(const std::string& name, float volume, int loops) {
    auto it = m_sounds.find(name);
    if (it == m_sounds.end()) {
        std::cerr << "Sound not found: " << name << std::endl;
        return -1;
    }

    SoundEffect* sound = it->second.get();

    // Check instance limit
    if (sound->getActiveInstances() >= m_maxInstancesPerSound) {
        // Stop oldest instance
        sound->stop();
    }

    // Check category limit
    int categoryInstances = getActiveSoundsInCategory(sound->getCategory());
    int maxCategoryInstances = m_categoryMaxInstances[sound->getCategory()];
    if (categoryInstances >= maxCategoryInstances) {
        // Don't play if category is full and priority is too low
        if (sound->getPriority() < m_priorityThreshold) {
            return -1;
        }
    }

    float originalVolume = sound->getVolume();
    sound->setVolume(volume);

    int channel = sound->play(loops);

    sound->setVolume(originalVolume);

    if (channel != -1) {
        m_channelToSound[channel] = name;
    }

    return channel;
}

int SoundEffectManager::playSound3D(const std::string& name, float x, float y, float z,
                                   float volume, int loops) {
    auto it = m_sounds.find(name);
    if (it == m_sounds.end()) {
        std::cerr << "Sound not found: " << name << std::endl;
        return -1;
    }

    SoundEffect* sound = it->second.get();

    // Check instance limit
    if (sound->getActiveInstances() >= m_maxInstancesPerSound) {
        sound->stop();
    }

    float originalVolume = sound->getVolume();
    sound->setVolume(volume);

    int channel = sound->play3D(x, y, z, loops);

    sound->setVolume(originalVolume);

    if (channel != -1) {
        m_channelToSound[channel] = name;
    }

    return channel;
}

void SoundEffectManager::startSoundLoop(const std::string& name, float interval) {
    // Check if already looping
    for (const auto& loop : m_loopingSounds) {
        if (loop.name == name) {
            return;
        }
    }

    LoopingSound loop;
    loop.name = name;
    loop.interval = interval;
    loop.timer = 0.0f;
    loop.lastChannel = -1;

    m_loopingSounds.push_back(loop);

    // Play immediately
    playSound(name);
}

void SoundEffectManager::stopSoundLoop(const std::string& name) {
    m_loopingSounds.erase(
        std::remove_if(m_loopingSounds.begin(), m_loopingSounds.end(),
                      [&name](const LoopingSound& loop) {
                          return loop.name == name;
                      }),
        m_loopingSounds.end()
    );
}

void SoundEffectManager::stopSound(const std::string& name) {
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        it->second->stop();
    }
}

void SoundEffectManager::stopCategory(AudioCategory category) {
    for (auto& pair : m_sounds) {
        if (pair.second->getCategory() == category) {
            pair.second->stop();
        }
    }
}

void SoundEffectManager::stopAll() {
    for (auto& pair : m_sounds) {
        pair.second->stop();
    }
    m_channelToSound.clear();
    m_loopingSounds.clear();
}

void SoundEffectManager::setPriorityThreshold(SoundPriority threshold) {
    m_priorityThreshold = threshold;
}

void SoundEffectManager::setMaxInstancesPerSound(int max) {
    m_maxInstancesPerSound = std::max(1, max);
}

void SoundEffectManager::setMaxInstancesPerCategory(AudioCategory category, int max) {
    m_categoryMaxInstances[category] = std::max(1, max);
}

SoundEffect* SoundEffectManager::getSound(const std::string& name) {
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        return it->second.get();
    }
    return nullptr;
}

int SoundEffectManager::getActiveSounds() const {
    int count = 0;
    for (const auto& pair : m_sounds) {
        count += pair.second->getActiveInstances();
    }
    return count;
}

int SoundEffectManager::getActiveSoundsInCategory(AudioCategory category) const {
    int count = 0;
    for (const auto& pair : m_sounds) {
        if (pair.second->getCategory() == category) {
            count += pair.second->getActiveInstances();
        }
    }
    return count;
}

void SoundEffectManager::update(float deltaTime) {
    // Update looping sounds
    for (auto& loop : m_loopingSounds) {
        loop.timer += deltaTime;

        if (loop.timer >= loop.interval) {
            loop.timer = 0.0f;
            loop.lastChannel = playSound(loop.name);
        }
    }
}

void SoundEffectManager::onChannelFinished(int channel) {
    auto it = m_channelToSound.find(channel);
    if (it != m_channelToSound.end()) {
        const std::string& soundName = it->second;

        auto soundIt = m_sounds.find(soundName);
        if (soundIt != m_sounds.end()) {
            soundIt->second->onChannelFinished(channel);
        }

        m_channelToSound.erase(it);
    }
}

} // namespace Audio
