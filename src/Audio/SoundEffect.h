#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace Audio {

enum class AudioCategory;  // Forward declaration

/**
 * @brief Sound effect priority (for channel management)
 */
enum class SoundPriority {
    Low = 0,        // Can be easily interrupted
    Normal = 1,     // Default priority
    High = 2,       // Important sounds
    Critical = 3    // Never interrupt (UI, dialogue, etc.)
};

/**
 * @brief Sound effect variations for randomization
 */
struct SoundVariation {
    std::string filePath;
    Mix_Chunk* chunk = nullptr;
    float pitchVariation = 0.0f;
    float volumeVariation = 0.0f;
};

/**
 * @brief Sound effect instance
 */
struct SoundInstance {
    int channel = -1;
    std::string soundName;
    AudioCategory category;
    SoundPriority priority;
    float volume = 1.0f;
    bool is3D = false;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    bool isLooping = false;
    float fadeOutTime = 0.0f;
};

/**
 * @brief Sound effect definition
 */
class SoundEffect {
public:
    SoundEffect(const std::string& name);
    ~SoundEffect();

    // Loading
    bool load(const std::string& filePath);
    bool loadVariations(const std::vector<std::string>& filePaths);
    bool isLoaded() const { return !m_variations.empty(); }

    // Properties
    void setCategory(AudioCategory category) { m_category = category; }
    AudioCategory getCategory() const { return m_category; }

    void setPriority(SoundPriority priority) { m_priority = priority; }
    SoundPriority getPriority() const { return m_priority; }

    void setVolume(float volume);
    float getVolume() const { return m_volume; }

    void setMaxDistance(float distance) { m_maxDistance = distance; }
    float getMaxDistance() const { return m_maxDistance; }

    void setMinDistance(float distance) { m_minDistance = distance; }
    float getMinDistance() const { return m_minDistance; }

    // Variation settings
    void setPitchVariation(float variation) { m_pitchVariation = variation; }
    void setVolumeVariation(float variation) { m_volumeVariation = variation; }

    // Playback
    int play(int loops = 0);
    int play3D(float x, float y, float z, int loops = 0);
    void stop();
    void stopInstance(int channel);

    // Query
    const std::string& getName() const { return m_name; }
    int getActiveInstances() const { return static_cast<int>(m_activeInstances.size()); }
    bool isPlaying() const { return !m_activeInstances.empty(); }

    // Internal
    void onChannelFinished(int channel);

private:
    Mix_Chunk* getRandomVariation();
    float getRandomPitch();
    float getRandomVolume();

private:
    std::string m_name;
    std::vector<SoundVariation> m_variations;
    AudioCategory m_category;
    SoundPriority m_priority = SoundPriority::Normal;

    float m_volume = 1.0f;
    float m_maxDistance = 100.0f;
    float m_minDistance = 1.0f;
    float m_pitchVariation = 0.0f;
    float m_volumeVariation = 0.0f;

    std::vector<int> m_activeInstances;
    int m_lastVariationIndex = -1;  // For round-robin variation selection
};

/**
 * @brief Sound effect manager
 * Manages sound effects by category, priority, and instances
 */
class SoundEffectManager {
public:
    SoundEffectManager();
    ~SoundEffectManager();

    // Loading
    bool loadSound(const std::string& name, const std::string& filePath,
                   AudioCategory category = static_cast<AudioCategory>(1));
    bool loadSoundWithVariations(const std::string& name, const std::vector<std::string>& filePaths,
                                AudioCategory category = static_cast<AudioCategory>(1));
    bool loadFromJSON(const std::string& jsonPath);

    // Playback
    int playSound(const std::string& name, float volume = 1.0f, int loops = 0);
    int playSound3D(const std::string& name, float x, float y, float z,
                   float volume = 1.0f, int loops = 0);

    // Group playback (for footsteps, etc.)
    void startSoundLoop(const std::string& name, float interval);
    void stopSoundLoop(const std::string& name);

    // Control
    void stopSound(const std::string& name);
    void stopCategory(AudioCategory category);
    void stopAll();

    // Priority management
    void setPriorityThreshold(SoundPriority threshold);
    SoundPriority getPriorityThreshold() const { return m_priorityThreshold; }

    // Polyphony limiting
    void setMaxInstancesPerSound(int max);
    void setMaxInstancesPerCategory(AudioCategory category, int max);

    // Query
    SoundEffect* getSound(const std::string& name);
    int getActiveSounds() const;
    int getActiveSoundsInCategory(AudioCategory category) const;

    // Update
    void update(float deltaTime);

    // Callbacks
    void onChannelFinished(int channel);

private:
    std::unordered_map<std::string, std::unique_ptr<SoundEffect>> m_sounds;
    std::unordered_map<int, std::string> m_channelToSound;
    std::unordered_map<AudioCategory, int> m_categoryMaxInstances;

    SoundPriority m_priorityThreshold = SoundPriority::Low;
    int m_maxInstancesPerSound = 4;

    // Looping sounds (for ambient effects)
    struct LoopingSound {
        std::string name;
        float interval;
        float timer;
        int lastChannel;
    };
    std::vector<LoopingSound> m_loopingSounds;
};

} // namespace Audio
