#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace Audio {

enum class AudioState;  // Forward declaration

/**
 * @brief Music layer for dynamic layered music
 */
struct MusicLayer {
    std::string name;
    Mix_Music* music = nullptr;
    float volume = 1.0f;
    float targetVolume = 1.0f;
    float fadeSpeed = 2.0f;  // Speed of volume transitions
    bool isActive = false;
    bool shouldPlay = false;
};

/**
 * @brief Music track with multiple layers
 */
struct MusicTrack {
    std::string name;
    std::string filePath;
    Mix_Music* music = nullptr;
    std::vector<MusicLayer> layers;
    AudioState associatedState;
    int loopCount = -1;  // -1 = infinite
    float volume = 1.0f;
    bool hasLayers = false;
};

/**
 * @brief Music transition settings
 */
struct MusicTransition {
    enum class Type {
        Immediate,      // Stop old, play new
        Crossfade,      // Fade out old while fading in new
        FadeOut,        // Fade out old, then play new
        Sequential      // Wait for old to finish, then play new
    };

    Type type = Type::Crossfade;
    float duration = 2.0f;  // Duration in seconds
};

/**
 * @brief Music player with advanced features
 * - Dynamic music layers
 * - Adaptive music based on game state
 * - Crossfading and transitions
 * - Playlist management
 */
class MusicPlayer {
public:
    MusicPlayer();
    ~MusicPlayer();

    // Update (call once per frame)
    void update(float deltaTime);

    // Track loading
    bool loadTrack(const std::string& name, const std::string& filePath,
                   AudioState associatedState = static_cast<AudioState>(0));
    bool loadTrackWithLayers(const std::string& name, const std::vector<std::string>& layerPaths,
                            AudioState associatedState = static_cast<AudioState>(0));
    bool loadPlaylist(const std::string& jsonPath);

    // Playback control
    bool play(const std::string& trackName, int loops = -1);
    bool playForState(AudioState state);
    void stop();
    void pause();
    void resume();
    void fadeOut(float duration = 2.0f);
    void fadeIn(const std::string& trackName, float duration = 2.0f, int loops = -1);

    // Transitions
    void transitionTo(const std::string& trackName, MusicTransition::Type type = MusicTransition::Type::Crossfade,
                     float duration = 2.0f, int loops = -1);

    // Layer control (for dynamic music)
    bool enableLayer(const std::string& layerName);
    bool disableLayer(const std::string& layerName);
    bool setLayerVolume(const std::string& layerName, float volume);
    bool fadeInLayer(const std::string& layerName, float duration = 2.0f);
    bool fadeOutLayer(const std::string& layerName, float duration = 2.0f);

    // Adaptive music (responds to game state)
    void onStateChanged(AudioState newState);
    void setStateTrack(AudioState state, const std::string& trackName);

    // Volume control
    void setVolume(float volume);
    float getVolume() const { return m_globalVolume; }

    // Query
    bool isPlaying() const;
    bool isPaused() const;
    std::string getCurrentTrack() const { return m_currentTrackName; }
    float getPosition() const;  // Position in seconds

    // Tempo and pitch (if supported)
    void setTempo(float tempo);  // 1.0 = normal
    void setPitch(float pitch);  // 1.0 = normal

private:
    // Internal helpers
    void updateTransition(float deltaTime);
    void updateLayers(float deltaTime);
    void cleanupOldMusic();
    MusicTrack* findTrack(const std::string& name);
    MusicLayer* findLayer(const std::string& name);

private:
    // Tracks and layers
    std::vector<MusicTrack> m_tracks;
    std::unordered_map<AudioState, std::string> m_stateTrackMapping;

    // Current playback
    std::string m_currentTrackName;
    MusicTrack* m_currentTrack = nullptr;
    float m_globalVolume = 1.0f;

    // Transition state
    bool m_isTransitioning = false;
    MusicTransition m_currentTransition;
    float m_transitionProgress = 0.0f;
    std::string m_transitionTargetTrack;
    int m_transitionTargetLoops = -1;
    Mix_Music* m_transitionOldMusic = nullptr;

    // Audio state
    AudioState m_lastAudioState;
};

} // namespace Audio
