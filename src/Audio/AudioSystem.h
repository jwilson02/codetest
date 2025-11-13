#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace Audio {

// Forward declarations
class MusicPlayer;
class SoundEffect;
class AudioSource;

/**
 * @brief Audio context and quality settings
 */
struct AudioConfig {
    int frequency = 44100;          // Sample rate
    Uint16 format = MIX_DEFAULT_FORMAT;  // Audio format
    int channels = 2;               // Stereo
    int chunkSize = 2048;           // Buffer size
    int maxChannels = 32;           // Max simultaneous sounds
    float masterVolume = 1.0f;      // Master volume (0.0 - 1.0)
    float musicVolume = 0.7f;       // Music volume
    float sfxVolume = 0.8f;         // Sound effects volume
    float ambientVolume = 0.5f;     // Ambient sounds volume
    float voiceVolume = 1.0f;       // Voice volume
    bool enableReverb = true;       // Reverb effects
    bool enable3DAudio = true;      // 3D positional audio
    bool enableOcclusion = true;    // Audio occlusion
};

/**
 * @brief Audio listener (usually the player/camera)
 */
struct AudioListener {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float forwardX = 0.0f;
    float forwardY = 0.0f;
    float forwardZ = 1.0f;
    float upX = 0.0f;
    float upY = 1.0f;
    float upZ = 0.0f;
};

/**
 * @brief Audio category for mixing and volume control
 */
enum class AudioCategory {
    Music,
    SFX,
    Ambient,
    Voice,
    UI,
    Footsteps,
    Combat,
    Environment
};

/**
 * @brief Audio state for adaptive music
 */
enum class AudioState {
    Exploration,
    Combat,
    Stealth,
    Dialogue,
    Cutscene,
    Menu
};

/**
 * @brief Main audio system manager
 * Handles initialization, resource management, and audio processing
 */
class AudioSystem {
public:
    static AudioSystem& getInstance();

    // Initialization and shutdown
    bool initialize(const AudioConfig& config = AudioConfig());
    void shutdown();
    bool isInitialized() const { return m_initialized; }

    // Update (call once per frame)
    void update(float deltaTime);

    // Configuration
    const AudioConfig& getConfig() const { return m_config; }
    void setConfig(const AudioConfig& config);

    // Volume control
    void setMasterVolume(float volume);
    void setMusicVolume(float volume);
    void setSFXVolume(float volume);
    void setAmbientVolume(float volume);
    void setVoiceVolume(float volume);
    void setCategoryVolume(AudioCategory category, float volume);
    float getCategoryVolume(AudioCategory category) const;

    // Mute control
    void setMuted(bool muted);
    bool isMuted() const { return m_muted; }
    void toggleMute();

    // Listener (camera/player position)
    void setListenerPosition(float x, float y, float z);
    void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                float upX, float upY, float upZ);
    const AudioListener& getListener() const { return m_listener; }

    // Audio state management
    void setAudioState(AudioState state);
    AudioState getAudioState() const { return m_currentState; }

    // Resource loading
    bool loadSoundEffect(const std::string& name, const std::string& filePath);
    bool loadMusic(const std::string& name, const std::string& filePath);
    bool loadSoundMapping(const std::string& jsonPath);
    bool loadMusicPlaylist(const std::string& jsonPath);

    // Component access
    MusicPlayer* getMusicPlayer() { return m_musicPlayer.get(); }

    // Sound effect playback (convenience methods)
    int playSoundEffect(const std::string& name, float volume = 1.0f,
                       int loops = 0, AudioCategory category = AudioCategory::SFX);
    int playSoundEffect3D(const std::string& name, float x, float y, float z,
                         float volume = 1.0f, AudioCategory category = AudioCategory::SFX);
    void stopChannel(int channel);
    void stopAllChannels();

    // Audio ducking (reduce volume when voice plays)
    void enableDucking(AudioCategory category, float duckAmount = 0.5f);
    void disableDucking(AudioCategory category);

    // Reverb and effects
    void setReverbAmount(float amount);
    float getReverbAmount() const { return m_reverbAmount; }

    // Occlusion (walls blocking sound)
    void setOcclusionFactor(float factor);
    float getOcclusionFactor() const { return m_occlusionFactor; }

    // Doppler effect
    void setDopplerFactor(float factor);
    float getDopplerFactor() const { return m_dopplerFactor; }

    // Channel management
    bool isChannelPlaying(int channel) const;
    void setChannelVolume(int channel, float volume);
    void fadeOutChannel(int channel, int ms);

    // Callbacks
    void setChannelFinishedCallback(std::function<void(int)> callback);
    void setMusicFinishedCallback(std::function<void()> callback);

    // Statistics
    int getActiveChannels() const;
    int getAvailableChannels() const;
    size_t getLoadedSoundsCount() const;

    // Audio formats supported
    static std::vector<std::string> getSupportedFormats();

private:
    AudioSystem() = default;
    ~AudioSystem();
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    // Internal helpers
    void applyVolumeToChannel(int channel, AudioCategory category);
    float calculateDistance(float x1, float y1, float z1, float x2, float y2, float z2) const;
    float calculateAttenuation(float distance, float maxDistance = 100.0f) const;
    void updateDucking(float deltaTime);
    void cleanupFinishedSources();

    // Static callbacks
    static void channelFinishedCallback(int channel);
    static void musicFinishedCallback();

private:
    bool m_initialized = false;
    AudioConfig m_config;
    AudioListener m_listener;
    AudioState m_currentState = AudioState::Exploration;

    // Volume control
    bool m_muted = false;
    float m_preMuteVolume = 1.0f;
    std::unordered_map<AudioCategory, float> m_categoryVolumes;

    // Audio effects
    float m_reverbAmount = 0.3f;
    float m_occlusionFactor = 0.0f;
    float m_dopplerFactor = 1.0f;

    // Ducking
    struct DuckingInfo {
        bool enabled = false;
        float targetAmount = 0.5f;
        float currentAmount = 0.0f;
        float speed = 4.0f;  // Duck/unduck speed
    };
    std::unordered_map<AudioCategory, DuckingInfo> m_ducking;
    bool m_isDucking = false;

    // Components
    std::unique_ptr<MusicPlayer> m_musicPlayer;
    std::unordered_map<std::string, Mix_Chunk*> m_soundEffects;
    std::vector<std::unique_ptr<AudioSource>> m_audioSources;

    // Channel tracking
    struct ChannelInfo {
        AudioCategory category = AudioCategory::SFX;
        std::string soundName;
        float baseVolume = 1.0f;
        bool is3D = false;
        std::shared_ptr<AudioSource> source;
    };
    std::unordered_map<int, ChannelInfo> m_activeChannels;

    // Callbacks
    std::function<void(int)> m_channelFinishedCallback;
    std::function<void()> m_musicFinishedCallback;

    // Singleton instance
    static AudioSystem* s_instance;
};

} // namespace Audio
