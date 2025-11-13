#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <memory>

namespace Audio {

enum class AudioCategory;
struct AudioListener;

/**
 * @brief Attenuation model for distance-based volume reduction
 */
enum class AttenuationModel {
    Linear,         // Linear falloff
    Inverse,        // 1/distance
    InverseSquare,  // 1/(distance^2) - realistic
    Exponential     // Exponential decay
};

/**
 * @brief 3D audio source
 * Represents a sound emitter in 3D space with spatial audio properties
 */
class AudioSource {
public:
    AudioSource();
    ~AudioSource();

    // Position
    void setPosition(float x, float y, float z);
    void getPosition(float& x, float& y, float& z) const;

    // Velocity (for Doppler effect)
    void setVelocity(float vx, float vy, float vz);
    void getVelocity(float& vx, float& vy, float& vz) const;

    // Sound assignment
    void setSound(const std::string& soundName);
    const std::string& getSound() const { return m_soundName; }

    // Playback
    bool play(int loops = 0);
    void stop();
    void pause();
    void resume();

    // Volume and distance
    void setVolume(float volume);
    float getVolume() const { return m_volume; }

    void setMinDistance(float distance);
    float getMinDistance() const { return m_minDistance; }

    void setMaxDistance(float distance);
    float getMaxDistance() const { return m_maxDistance; }

    void setAttenuationModel(AttenuationModel model);
    AttenuationModel getAttenuationModel() const { return m_attenuationModel; }

    // Rolloff factor (affects attenuation curve)
    void setRolloffFactor(float factor);
    float getRolloffFactor() const { return m_rolloffFactor; }

    // Cone (directional sound)
    void setCone(float innerAngle, float outerAngle, float outerVolume);
    void setConeOrientation(float dirX, float dirY, float dirZ);

    // Occlusion
    void setOcclusion(float amount);
    float getOcclusion() const { return m_occlusion; }

    // Reverb send
    void setReverbSend(float amount);
    float getReverbSend() const { return m_reverbSend; }

    // Doppler
    void setDopplerFactor(float factor);
    float getDopplerFactor() const { return m_dopplerFactor; }

    // Looping
    void setLooping(bool loop);
    bool isLooping() const { return m_looping; }

    // Category
    void setCategory(AudioCategory category);
    AudioCategory getCategory() const { return m_category; }

    // Priority
    void setPriority(int priority);
    int getPriority() const { return m_priority; }

    // Query
    bool isPlaying() const;
    bool isPaused() const;
    bool isFinished() const { return m_finished; }

    // Update (called by audio system)
    void update(float deltaTime, const AudioListener& listener);

    // Channel
    int getChannel() const { return m_channel; }

private:
    float calculateAttenuation(float distance) const;
    float calculateConeAttenuation(const AudioListener& listener) const;
    float calculateDopplerPitch(const AudioListener& listener) const;
    void applyEffects(const AudioListener& listener);

private:
    // Position and movement
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_z = 0.0f;
    float m_vx = 0.0f;
    float m_vy = 0.0f;
    float m_vz = 0.0f;

    // Sound properties
    std::string m_soundName;
    float m_volume = 1.0f;
    float m_minDistance = 1.0f;
    float m_maxDistance = 100.0f;
    float m_rolloffFactor = 1.0f;
    AttenuationModel m_attenuationModel = AttenuationModel::InverseSquare;

    // Directional cone
    bool m_hasCone = false;
    float m_coneInnerAngle = 360.0f;
    float m_coneOuterAngle = 360.0f;
    float m_coneOuterVolume = 0.0f;
    float m_coneDirX = 0.0f;
    float m_coneDirY = 0.0f;
    float m_coneDirZ = 1.0f;

    // Effects
    float m_occlusion = 0.0f;
    float m_reverbSend = 0.0f;
    float m_dopplerFactor = 1.0f;

    // Playback state
    int m_channel = -1;
    bool m_looping = false;
    bool m_playing = false;
    bool m_paused = false;
    bool m_finished = false;

    // Category and priority
    AudioCategory m_category;
    int m_priority = 128;
};

/**
 * @brief Audio emitter - simple wrapper for positioning sounds
 */
class AudioEmitter {
public:
    AudioEmitter() = default;

    void setPosition(float x, float y, float z) {
        m_x = x;
        m_y = y;
        m_z = z;
    }

    void getPosition(float& x, float& y, float& z) const {
        x = m_x;
        y = m_y;
        z = m_z;
    }

    // Play a sound at this emitter's position
    int playSound(const std::string& soundName, float volume = 1.0f);

    // Play looping ambient sound
    void playAmbient(const std::string& soundName, float volume = 1.0f, float radius = 50.0f);
    void stopAmbient();

private:
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_z = 0.0f;
    std::unique_ptr<AudioSource> m_ambientSource;
};

/**
 * @brief Audio zone - area-based audio trigger
 */
class AudioZone {
public:
    AudioZone() = default;

    void setBounds(float minX, float minY, float minZ,
                  float maxX, float maxY, float maxZ);

    void setSound(const std::string& soundName, float volume = 1.0f);

    void setFadeDistance(float distance);

    bool isListenerInZone(const AudioListener& listener) const;
    float getBlendFactor(const AudioListener& listener) const;

    void update(float deltaTime, const AudioListener& listener);

private:
    float m_minX = 0.0f;
    float m_minY = 0.0f;
    float m_minZ = 0.0f;
    float m_maxX = 0.0f;
    float m_maxY = 0.0f;
    float m_maxZ = 0.0f;

    std::string m_soundName;
    float m_volume = 1.0f;
    float m_fadeDistance = 5.0f;

    std::unique_ptr<AudioSource> m_source;
    bool m_isActive = false;
};

} // namespace Audio
