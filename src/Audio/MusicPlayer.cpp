#include "MusicPlayer.h"
#include "AudioSystem.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// For JSON parsing
// #include <nlohmann/json.hpp>
// using json = nlohmann::json;

namespace Audio {

MusicPlayer::MusicPlayer()
    : m_lastAudioState(AudioState::Exploration) {
}

MusicPlayer::~MusicPlayer() {
    stop();

    // Free all loaded music
    for (auto& track : m_tracks) {
        if (track.music) {
            Mix_FreeMusic(track.music);
            track.music = nullptr;
        }

        for (auto& layer : track.layers) {
            if (layer.music) {
                Mix_FreeMusic(layer.music);
                layer.music = nullptr;
            }
        }
    }

    if (m_transitionOldMusic) {
        Mix_FreeMusic(m_transitionOldMusic);
        m_transitionOldMusic = nullptr;
    }
}

void MusicPlayer::update(float deltaTime) {
    // Update transitions
    if (m_isTransitioning) {
        updateTransition(deltaTime);
    }

    // Update dynamic layers
    if (m_currentTrack && m_currentTrack->hasLayers) {
        updateLayers(deltaTime);
    }
}

bool MusicPlayer::loadTrack(const std::string& name, const std::string& filePath, AudioState associatedState) {
    // Check if already loaded
    if (findTrack(name)) {
        std::cout << "Music track '" << name << "' already loaded" << std::endl;
        return true;
    }

    Mix_Music* music = Mix_LoadMUS(filePath.c_str());
    if (!music) {
        std::cerr << "Failed to load music track '" << name << "': " << Mix_GetError() << std::endl;
        return false;
    }

    MusicTrack track;
    track.name = name;
    track.filePath = filePath;
    track.music = music;
    track.associatedState = associatedState;
    track.hasLayers = false;

    m_tracks.push_back(track);

    std::cout << "Loaded music track: " << name << " from " << filePath << std::endl;
    return true;
}

bool MusicPlayer::loadTrackWithLayers(const std::string& name, const std::vector<std::string>& layerPaths,
                                      AudioState associatedState) {
    if (layerPaths.empty()) {
        std::cerr << "No layer paths provided for track '" << name << "'" << std::endl;
        return false;
    }

    MusicTrack track;
    track.name = name;
    track.associatedState = associatedState;
    track.hasLayers = true;

    // Load each layer
    for (size_t i = 0; i < layerPaths.size(); ++i) {
        Mix_Music* music = Mix_LoadMUS(layerPaths[i].c_str());
        if (!music) {
            std::cerr << "Failed to load music layer " << i << " for '" << name << "': " << Mix_GetError() << std::endl;

            // Clean up already loaded layers
            for (auto& layer : track.layers) {
                if (layer.music) {
                    Mix_FreeMusic(layer.music);
                }
            }
            return false;
        }

        MusicLayer layer;
        layer.name = name + "_layer" + std::to_string(i);
        layer.music = music;
        layer.volume = 0.0f;
        layer.targetVolume = (i == 0) ? 1.0f : 0.0f;  // Only first layer starts enabled
        layer.isActive = (i == 0);
        layer.shouldPlay = false;

        track.layers.push_back(layer);
    }

    m_tracks.push_back(track);

    std::cout << "Loaded layered music track: " << name << " with " << layerPaths.size() << " layers" << std::endl;
    return true;
}

bool MusicPlayer::loadPlaylist(const std::string& jsonPath) {
    // TODO: Implement JSON parsing for playlists
    // This would load a JSON file containing track information
    std::cout << "Loading music playlist from: " << jsonPath << std::endl;

    // Example JSON structure:
    // {
    //   "tracks": [
    //     {
    //       "name": "exploration_theme",
    //       "file": "assets/audio/music/exploration.ogg",
    //       "state": "exploration",
    //       "layers": []
    //     },
    //     {
    //       "name": "combat_theme",
    //       "file": "assets/audio/music/combat_base.ogg",
    //       "state": "combat",
    //       "layers": [
    //         "assets/audio/music/combat_percussion.ogg",
    //         "assets/audio/music/combat_intensity.ogg"
    //       ]
    //     }
    //   ]
    // }

    return true;
}

bool MusicPlayer::play(const std::string& trackName, int loops) {
    MusicTrack* track = findTrack(trackName);
    if (!track) {
        std::cerr << "Music track not found: " << trackName << std::endl;
        return false;
    }

    // Stop current music
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }

    // For layered music, play first layer
    if (track->hasLayers && !track->layers.empty()) {
        if (Mix_PlayMusic(track->layers[0].music, loops) == -1) {
            std::cerr << "Failed to play layered music: " << Mix_GetError() << std::endl;
            return false;
        }

        track->layers[0].isActive = true;
        track->layers[0].shouldPlay = true;
        Mix_VolumeMusic(static_cast<int>(track->layers[0].volume * m_globalVolume * MIX_MAX_VOLUME));
    } else {
        // Play regular track
        if (Mix_PlayMusic(track->music, loops) == -1) {
            std::cerr << "Failed to play music: " << Mix_GetError() << std::endl;
            return false;
        }

        Mix_VolumeMusic(static_cast<int>(track->volume * m_globalVolume * MIX_MAX_VOLUME));
    }

    m_currentTrackName = trackName;
    m_currentTrack = track;

    std::cout << "Playing music: " << trackName << std::endl;
    return true;
}

bool MusicPlayer::playForState(AudioState state) {
    auto it = m_stateTrackMapping.find(state);
    if (it != m_stateTrackMapping.end()) {
        return play(it->second);
    }

    std::cerr << "No music track mapped to state: " << static_cast<int>(state) << std::endl;
    return false;
}

void MusicPlayer::stop() {
    Mix_HaltMusic();
    m_currentTrackName.clear();
    m_currentTrack = nullptr;
    m_isTransitioning = false;
}

void MusicPlayer::pause() {
    Mix_PauseMusic();
}

void MusicPlayer::resume() {
    Mix_ResumeMusic();
}

void MusicPlayer::fadeOut(float duration) {
    int ms = static_cast<int>(duration * 1000.0f);
    Mix_FadeOutMusic(ms);
    m_currentTrackName.clear();
    m_currentTrack = nullptr;
}

void MusicPlayer::fadeIn(const std::string& trackName, float duration, int loops) {
    MusicTrack* track = findTrack(trackName);
    if (!track) {
        std::cerr << "Music track not found: " << trackName << std::endl;
        return;
    }

    int ms = static_cast<int>(duration * 1000.0f);

    if (track->hasLayers && !track->layers.empty()) {
        if (Mix_FadeInMusic(track->layers[0].music, loops, ms) == -1) {
            std::cerr << "Failed to fade in music: " << Mix_GetError() << std::endl;
            return;
        }
    } else {
        if (Mix_FadeInMusic(track->music, loops, ms) == -1) {
            std::cerr << "Failed to fade in music: " << Mix_GetError() << std::endl;
            return;
        }
    }

    m_currentTrackName = trackName;
    m_currentTrack = track;
}

void MusicPlayer::transitionTo(const std::string& trackName, MusicTransition::Type type,
                               float duration, int loops) {
    MusicTrack* newTrack = findTrack(trackName);
    if (!newTrack) {
        std::cerr << "Music track not found: " << trackName << std::endl;
        return;
    }

    // If same track, do nothing
    if (m_currentTrackName == trackName) {
        return;
    }

    m_currentTransition.type = type;
    m_currentTransition.duration = duration;
    m_transitionTargetTrack = trackName;
    m_transitionTargetLoops = loops;

    switch (type) {
        case MusicTransition::Type::Immediate:
            stop();
            play(trackName, loops);
            break;

        case MusicTransition::Type::Crossfade:
            m_isTransitioning = true;
            m_transitionProgress = 0.0f;
            fadeIn(trackName, duration, loops);
            break;

        case MusicTransition::Type::FadeOut:
            fadeOut(duration);
            m_isTransitioning = true;
            m_transitionProgress = 0.0f;
            break;

        case MusicTransition::Type::Sequential:
            // Wait for current track to finish (implement with callback)
            m_isTransitioning = true;
            break;
    }

    std::cout << "Transitioning music to: " << trackName << std::endl;
}

bool MusicPlayer::enableLayer(const std::string& layerName) {
    MusicLayer* layer = findLayer(layerName);
    if (!layer) {
        return false;
    }

    layer->shouldPlay = true;
    layer->targetVolume = 1.0f;
    return true;
}

bool MusicPlayer::disableLayer(const std::string& layerName) {
    MusicLayer* layer = findLayer(layerName);
    if (!layer) {
        return false;
    }

    layer->shouldPlay = false;
    layer->targetVolume = 0.0f;
    return true;
}

bool MusicPlayer::setLayerVolume(const std::string& layerName, float volume) {
    MusicLayer* layer = findLayer(layerName);
    if (!layer) {
        return false;
    }

    layer->targetVolume = std::clamp(volume, 0.0f, 1.0f);
    return true;
}

bool MusicPlayer::fadeInLayer(const std::string& layerName, float duration) {
    MusicLayer* layer = findLayer(layerName);
    if (!layer) {
        return false;
    }

    layer->shouldPlay = true;
    layer->targetVolume = 1.0f;
    layer->fadeSpeed = 1.0f / duration;
    return true;
}

bool MusicPlayer::fadeOutLayer(const std::string& layerName, float duration) {
    MusicLayer* layer = findLayer(layerName);
    if (!layer) {
        return false;
    }

    layer->targetVolume = 0.0f;
    layer->fadeSpeed = 1.0f / duration;
    return true;
}

void MusicPlayer::onStateChanged(AudioState newState) {
    if (newState == m_lastAudioState) {
        return;
    }

    m_lastAudioState = newState;

    // Handle adaptive music based on state
    switch (newState) {
        case AudioState::Combat:
            // Enable combat layers if current track has them
            if (m_currentTrack && m_currentTrack->hasLayers) {
                enableLayer(m_currentTrack->name + "_layer1");  // Percussion
                enableLayer(m_currentTrack->name + "_layer2");  // Intensity
            } else {
                // Otherwise, transition to combat music
                auto it = m_stateTrackMapping.find(newState);
                if (it != m_stateTrackMapping.end()) {
                    transitionTo(it->second, MusicTransition::Type::Crossfade, 1.0f);
                }
            }
            break;

        case AudioState::Exploration:
            // Disable combat layers
            if (m_currentTrack && m_currentTrack->hasLayers) {
                disableLayer(m_currentTrack->name + "_layer1");
                disableLayer(m_currentTrack->name + "_layer2");
            } else {
                auto it = m_stateTrackMapping.find(newState);
                if (it != m_stateTrackMapping.end()) {
                    transitionTo(it->second, MusicTransition::Type::Crossfade, 2.0f);
                }
            }
            break;

        case AudioState::Stealth:
        case AudioState::Dialogue:
        case AudioState::Cutscene:
        case AudioState::Menu:
            // Handle other states
            auto it = m_stateTrackMapping.find(newState);
            if (it != m_stateTrackMapping.end()) {
                transitionTo(it->second, MusicTransition::Type::Crossfade, 1.5f);
            }
            break;
    }
}

void MusicPlayer::setStateTrack(AudioState state, const std::string& trackName) {
    m_stateTrackMapping[state] = trackName;
}

void MusicPlayer::setVolume(float volume) {
    m_globalVolume = std::clamp(volume, 0.0f, 1.0f);
    Mix_VolumeMusic(static_cast<int>(m_globalVolume * MIX_MAX_VOLUME));
}

bool MusicPlayer::isPlaying() const {
    return Mix_PlayingMusic() != 0;
}

bool MusicPlayer::isPaused() const {
    return Mix_PausedMusic() != 0;
}

float MusicPlayer::getPosition() const {
    // Note: SDL_mixer doesn't provide position tracking by default
    // You'd need to track this manually or use Mix_GetMusicPosition if available
    return 0.0f;
}

void MusicPlayer::setTempo(float tempo) {
    // Note: SDL_mixer doesn't support tempo changes by default
    // You'd need to use a more advanced audio library or implement it manually
    std::cout << "Tempo change requested: " << tempo << " (not implemented in SDL_mixer)" << std::endl;
}

void MusicPlayer::setPitch(float pitch) {
    // Note: SDL_mixer doesn't support pitch changes by default
    std::cout << "Pitch change requested: " << pitch << " (not implemented in SDL_mixer)" << std::endl;
}

// Private methods

void MusicPlayer::updateTransition(float deltaTime) {
    m_transitionProgress += deltaTime;

    if (m_currentTransition.type == MusicTransition::Type::FadeOut) {
        if (m_transitionProgress >= m_currentTransition.duration) {
            // Fade out complete, play new track
            play(m_transitionTargetTrack, m_transitionTargetLoops);
            m_isTransitioning = false;
            m_transitionProgress = 0.0f;
        }
    } else if (m_currentTransition.type == MusicTransition::Type::Crossfade) {
        if (m_transitionProgress >= m_currentTransition.duration) {
            m_isTransitioning = false;
            m_transitionProgress = 0.0f;
            cleanupOldMusic();
        }
    }
}

void MusicPlayer::updateLayers(float deltaTime) {
    if (!m_currentTrack) {
        return;
    }

    for (auto& layer : m_currentTrack->layers) {
        // Update layer volume towards target
        if (layer.volume < layer.targetVolume) {
            layer.volume += layer.fadeSpeed * deltaTime;
            layer.volume = std::min(layer.volume, layer.targetVolume);
        } else if (layer.volume > layer.targetVolume) {
            layer.volume -= layer.fadeSpeed * deltaTime;
            layer.volume = std::max(layer.volume, layer.targetVolume);
        }

        // Note: SDL_mixer doesn't support multiple simultaneous music tracks
        // For true layered music, you'd need to:
        // 1. Use Mix_Chunk instead of Mix_Music for layers
        // 2. Or use a more advanced audio library like FMOD or Wwise
        // 3. Or pre-mix layers and switch between different mixes

        // For now, this demonstrates the concept
        // In a real implementation, you'd play layers as sound effects
        // or use a more sophisticated audio engine
    }
}

void MusicPlayer::cleanupOldMusic() {
    if (m_transitionOldMusic) {
        // Note: Be careful with freeing music that might still be playing
        // In practice, you'd need more careful tracking
        m_transitionOldMusic = nullptr;
    }
}

MusicTrack* MusicPlayer::findTrack(const std::string& name) {
    for (auto& track : m_tracks) {
        if (track.name == name) {
            return &track;
        }
    }
    return nullptr;
}

MusicLayer* MusicPlayer::findLayer(const std::string& name) {
    if (!m_currentTrack) {
        return nullptr;
    }

    for (auto& layer : m_currentTrack->layers) {
        if (layer.name == name) {
            return &layer;
        }
    }
    return nullptr;
}

} // namespace Audio
