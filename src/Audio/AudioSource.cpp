#include "AudioSource.h"
#include "AudioSystem.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace Audio {

AudioSource::AudioSource()
    : m_category(AudioCategory::SFX) {
}

AudioSource::~AudioSource() {
    stop();
}

void AudioSource::setPosition(float x, float y, float z) {
    m_x = x;
    m_y = y;
    m_z = z;
}

void AudioSource::getPosition(float& x, float& y, float& z) const {
    x = m_x;
    y = m_y;
    z = m_z;
}

void AudioSource::setVelocity(float vx, float vy, float vz) {
    m_vx = vx;
    m_vy = vy;
    m_vz = vz;
}

void AudioSource::getVelocity(float& vx, float& vy, float& vz) const {
    vx = m_vx;
    vy = m_vy;
    vz = m_vz;
}

void AudioSource::setSound(const std::string& soundName) {
    m_soundName = soundName;
}

bool AudioSource::play(int loops) {
    if (m_soundName.empty()) {
        std::cerr << "Cannot play AudioSource: no sound assigned" << std::endl;
        return false;
    }

    AudioSystem& audio = AudioSystem::getInstance();
    const AudioListener& listener = audio.getListener();

    // Calculate initial distance
    float dx = m_x - listener.x;
    float dy = m_y - listener.y;
    float dz = m_z - listener.z;
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    // Check if within audible range
    if (distance > m_maxDistance) {
        m_finished = true;
        return false;
    }

    // Play the sound
    m_channel = audio.playSoundEffect3D(m_soundName, m_x, m_y, m_z, m_volume, m_category);

    if (m_channel == -1) {
        m_finished = true;
        return false;
    }

    m_playing = true;
    m_paused = false;
    m_finished = false;

    return true;
}

void AudioSource::stop() {
    if (m_channel != -1) {
        AudioSystem& audio = AudioSystem::getInstance();
        audio.stopChannel(m_channel);
        m_channel = -1;
    }

    m_playing = false;
    m_paused = false;
    m_finished = true;
}

void AudioSource::pause() {
    if (m_channel != -1 && m_playing && !m_paused) {
        Mix_Pause(m_channel);
        m_paused = true;
    }
}

void AudioSource::resume() {
    if (m_channel != -1 && m_playing && m_paused) {
        Mix_Resume(m_channel);
        m_paused = false;
    }
}

void AudioSource::setVolume(float volume) {
    m_volume = std::clamp(volume, 0.0f, 1.0f);

    if (m_channel != -1 && m_playing) {
        AudioSystem& audio = AudioSystem::getInstance();
        audio.setChannelVolume(m_channel, m_volume);
    }
}

void AudioSource::setMinDistance(float distance) {
    m_minDistance = std::max(0.0f, distance);
}

void AudioSource::setMaxDistance(float distance) {
    m_maxDistance = std::max(m_minDistance, distance);
}

void AudioSource::setAttenuationModel(AttenuationModel model) {
    m_attenuationModel = model;
}

void AudioSource::setRolloffFactor(float factor) {
    m_rolloffFactor = std::max(0.0f, factor);
}

void AudioSource::setCone(float innerAngle, float outerAngle, float outerVolume) {
    m_hasCone = true;
    m_coneInnerAngle = std::clamp(innerAngle, 0.0f, 360.0f);
    m_coneOuterAngle = std::clamp(outerAngle, m_coneInnerAngle, 360.0f);
    m_coneOuterVolume = std::clamp(outerVolume, 0.0f, 1.0f);
}

void AudioSource::setConeOrientation(float dirX, float dirY, float dirZ) {
    float length = std::sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ);
    if (length > 0.0f) {
        m_coneDirX = dirX / length;
        m_coneDirY = dirY / length;
        m_coneDirZ = dirZ / length;
    }
}

void AudioSource::setOcclusion(float amount) {
    m_occlusion = std::clamp(amount, 0.0f, 1.0f);
}

void AudioSource::setReverbSend(float amount) {
    m_reverbSend = std::clamp(amount, 0.0f, 1.0f);
}

void AudioSource::setDopplerFactor(float factor) {
    m_dopplerFactor = std::max(0.0f, factor);
}

void AudioSource::setLooping(bool loop) {
    m_looping = loop;
}

void AudioSource::setCategory(AudioCategory category) {
    m_category = category;
}

void AudioSource::setPriority(int priority) {
    m_priority = std::clamp(priority, 0, 255);
}

bool AudioSource::isPlaying() const {
    if (m_channel == -1) {
        return false;
    }

    AudioSystem& audio = AudioSystem::getInstance();
    return audio.isChannelPlaying(m_channel);
}

bool AudioSource::isPaused() const {
    return m_paused;
}

void AudioSource::update(float deltaTime, const AudioListener& listener) {
    if (!m_playing || m_channel == -1) {
        return;
    }

    // Check if channel is still playing
    AudioSystem& audio = AudioSystem::getInstance();
    if (!audio.isChannelPlaying(m_channel)) {
        if (m_looping && !m_finished) {
            // Restart if looping
            play(0);
        } else {
            m_finished = true;
            m_playing = false;
            m_channel = -1;
        }
        return;
    }

    // Apply 3D audio effects
    applyEffects(listener);
}

float AudioSource::calculateAttenuation(float distance) const {
    if (distance <= m_minDistance) {
        return 1.0f;
    }

    if (distance >= m_maxDistance) {
        return 0.0f;
    }

    float normalizedDistance = (distance - m_minDistance) / (m_maxDistance - m_minDistance);
    float attenuation = 1.0f;

    switch (m_attenuationModel) {
        case AttenuationModel::Linear:
            attenuation = 1.0f - normalizedDistance * m_rolloffFactor;
            break;

        case AttenuationModel::Inverse:
            attenuation = m_minDistance / (m_minDistance + m_rolloffFactor * (distance - m_minDistance));
            break;

        case AttenuationModel::InverseSquare:
            {
                float ratio = distance / m_minDistance;
                attenuation = 1.0f / (1.0f + m_rolloffFactor * (ratio * ratio - 1.0f));
            }
            break;

        case AttenuationModel::Exponential:
            attenuation = std::exp(-m_rolloffFactor * normalizedDistance);
            break;
    }

    return std::clamp(attenuation, 0.0f, 1.0f);
}

float AudioSource::calculateConeAttenuation(const AudioListener& listener) const {
    if (!m_hasCone || m_coneInnerAngle >= 360.0f) {
        return 1.0f;
    }

    // Calculate direction from source to listener
    float dx = listener.x - m_x;
    float dy = listener.y - m_y;
    float dz = listener.z - m_z;

    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (distance < 0.001f) {
        return 1.0f;
    }

    dx /= distance;
    dy /= distance;
    dz /= distance;

    // Calculate angle between cone direction and listener direction
    float dot = m_coneDirX * dx + m_coneDirY * dy + m_coneDirZ * dz;
    float angle = std::acos(std::clamp(dot, -1.0f, 1.0f)) * 180.0f / M_PI;

    if (angle <= m_coneInnerAngle / 2.0f) {
        return 1.0f;
    }

    if (angle >= m_coneOuterAngle / 2.0f) {
        return m_coneOuterVolume;
    }

    // Interpolate between inner and outer cone
    float t = (angle - m_coneInnerAngle / 2.0f) / (m_coneOuterAngle / 2.0f - m_coneInnerAngle / 2.0f);
    return 1.0f + t * (m_coneOuterVolume - 1.0f);
}

float AudioSource::calculateDopplerPitch(const AudioListener& listener) const {
    if (m_dopplerFactor <= 0.0f) {
        return 1.0f;
    }

    // Simplified Doppler effect calculation
    // In a real implementation, you'd need listener velocity too

    float dx = listener.x - m_x;
    float dy = listener.y - m_y;
    float dz = listener.z - m_z;

    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (distance < 0.001f) {
        return 1.0f;
    }

    // Relative velocity along the line connecting source and listener
    float relativeVelocity = (m_vx * dx + m_vy * dy + m_vz * dz) / distance;

    // Speed of sound in air (m/s)
    const float speedOfSound = 343.0f;

    float dopplerShift = 1.0f + (relativeVelocity / speedOfSound) * m_dopplerFactor;
    return std::clamp(dopplerShift, 0.5f, 2.0f);  // Reasonable limits
}

void AudioSource::applyEffects(const AudioListener& listener) {
    if (m_channel == -1) {
        return;
    }

    // Calculate distance
    float dx = m_x - listener.x;
    float dy = m_y - listener.y;
    float dz = m_z - listener.z;
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    // Calculate attenuation
    float attenuation = calculateAttenuation(distance);

    // Apply cone attenuation
    if (m_hasCone) {
        attenuation *= calculateConeAttenuation(listener);
    }

    // Apply occlusion
    if (m_occlusion > 0.0f) {
        attenuation *= (1.0f - m_occlusion);
    }

    // Calculate volume
    float finalVolume = m_volume * attenuation;

    // Update channel volume
    AudioSystem& audio = AudioSystem::getInstance();
    audio.setChannelVolume(m_channel, finalVolume);

    // Update panning
    float angle = std::atan2(dx, dz) * 180.0f / M_PI;
    int panAngle = static_cast<int>(angle);
    Mix_SetPosition(m_channel, panAngle, static_cast<Uint8>(std::min(distance, 255.0f)));

    // Note: SDL_mixer doesn't support pitch shifting by default
    // For Doppler effect, you'd need a more advanced audio library
    // or implement pitch shifting manually
}

// AudioEmitter implementation

int AudioEmitter::playSound(const std::string& soundName, float volume) {
    AudioSystem& audio = AudioSystem::getInstance();
    return audio.playSoundEffect3D(soundName, m_x, m_y, m_z, volume);
}

void AudioEmitter::playAmbient(const std::string& soundName, float volume, float radius) {
    if (!m_ambientSource) {
        m_ambientSource = std::make_unique<AudioSource>();
    }

    m_ambientSource->setPosition(m_x, m_y, m_z);
    m_ambientSource->setSound(soundName);
    m_ambientSource->setVolume(volume);
    m_ambientSource->setMaxDistance(radius);
    m_ambientSource->setLooping(true);
    m_ambientSource->setCategory(AudioCategory::Ambient);
    m_ambientSource->play(-1);  // Loop infinitely
}

void AudioEmitter::stopAmbient() {
    if (m_ambientSource) {
        m_ambientSource->stop();
        m_ambientSource.reset();
    }
}

// AudioZone implementation

void AudioZone::setBounds(float minX, float minY, float minZ,
                         float maxX, float maxY, float maxZ) {
    m_minX = minX;
    m_minY = minY;
    m_minZ = minZ;
    m_maxX = maxX;
    m_maxY = maxY;
    m_maxZ = maxZ;
}

void AudioZone::setSound(const std::string& soundName, float volume) {
    m_soundName = soundName;
    m_volume = volume;
}

void AudioZone::setFadeDistance(float distance) {
    m_fadeDistance = std::max(0.0f, distance);
}

bool AudioZone::isListenerInZone(const AudioListener& listener) const {
    return listener.x >= m_minX && listener.x <= m_maxX &&
           listener.y >= m_minY && listener.y <= m_maxY &&
           listener.z >= m_minZ && listener.z <= m_maxZ;
}

float AudioZone::getBlendFactor(const AudioListener& listener) const {
    if (isListenerInZone(listener)) {
        return 1.0f;
    }

    // Calculate distance to nearest edge
    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;

    if (listener.x < m_minX) dx = m_minX - listener.x;
    else if (listener.x > m_maxX) dx = listener.x - m_maxX;

    if (listener.y < m_minY) dy = m_minY - listener.y;
    else if (listener.y > m_maxY) dy = listener.y - m_maxY;

    if (listener.z < m_minZ) dz = m_minZ - listener.z;
    else if (listener.z > m_maxZ) dz = listener.z - m_maxZ;

    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    if (distance >= m_fadeDistance) {
        return 0.0f;
    }

    return 1.0f - (distance / m_fadeDistance);
}

void AudioZone::update(float deltaTime, const AudioListener& listener) {
    float blendFactor = getBlendFactor(listener);

    if (blendFactor > 0.0f && !m_isActive) {
        // Enter zone - start sound
        if (!m_source) {
            m_source = std::make_unique<AudioSource>();
            m_source->setSound(m_soundName);
            m_source->setLooping(true);
            m_source->setCategory(AudioCategory::Ambient);

            // Position at center of zone
            float centerX = (m_minX + m_maxX) / 2.0f;
            float centerY = (m_minY + m_maxY) / 2.0f;
            float centerZ = (m_minZ + m_maxZ) / 2.0f;
            m_source->setPosition(centerX, centerY, centerZ);
        }

        m_source->setVolume(m_volume * blendFactor);
        if (!m_source->isPlaying()) {
            m_source->play(-1);
        }
        m_isActive = true;
    } else if (blendFactor <= 0.0f && m_isActive) {
        // Exit zone - stop sound
        if (m_source) {
            m_source->stop();
            m_source.reset();
        }
        m_isActive = false;
    } else if (m_isActive && m_source) {
        // Update volume based on blend factor
        m_source->setVolume(m_volume * blendFactor);
    }
}

} // namespace Audio
