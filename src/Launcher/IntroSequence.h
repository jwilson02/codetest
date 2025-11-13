#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <functional>

namespace Launcher {

/**
 * @brief Cinematic scene types
 */
enum class SceneType {
    FadeIn,
    StillImage,
    TextScroll,
    DialogueScene,
    ActionSequence,
    FadeOut,
    Transition
};

/**
 * @brief Cinematic scene configuration
 */
struct CinematicScene {
    SceneType type;
    std::string backgroundPath;
    std::string musicPath;
    std::string voicePath;
    std::vector<std::string> textLines;
    float duration;
    float currentTime;
    SDL_Color textColor;
    bool skippable;
};

/**
 * @brief Subtitle configuration
 */
struct Subtitle {
    std::string text;
    float startTime;
    float endTime;
    SDL_Color color;
    bool visible;
};

/**
 * @brief Intro Cinematic Sequence
 *
 * Professional intro cinematic system featuring:
 * - Multiple scene types (images, text scrolls, dialogue)
 * - Smooth transitions and fades
 * - Subtitle support with timing
 * - Music and voice acting integration
 * - Skippable scenes
 * - Letterbox effect
 * - Chapter markers
 */
class IntroSequence {
public:
    IntroSequence();
    ~IntroSequence();

    /**
     * @brief Initialize the intro sequence
     * @param renderer SDL renderer
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @return True if initialization successful
     */
    bool Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight);

    /**
     * @brief Update the intro sequence
     * @param deltaTime Time elapsed since last frame
     */
    void Update(float deltaTime);

    /**
     * @brief Render the intro sequence
     * @param renderer SDL renderer
     */
    void Render(SDL_Renderer* renderer);

    /**
     * @brief Handle input events
     * @param event SDL event
     */
    void HandleInput(const SDL_Event& event);

    /**
     * @brief Skip the current scene or entire sequence
     */
    void Skip();

    /**
     * @brief Check if the intro sequence is complete
     * @return True if complete
     */
    bool IsComplete() const { return currentScene_ >= static_cast<int>(scenes_.size()); }

    /**
     * @brief Set callback for sequence completion
     * @param callback Function to call when complete
     */
    void SetOnComplete(std::function<void()> callback) {
        onComplete_ = callback;
    }

    /**
     * @brief Set callback for skip action
     * @param callback Function to call when skipped
     */
    void SetOnSkip(std::function<void()> callback) {
        onSkip_ = callback;
    }

    /**
     * @brief Pause the intro sequence
     */
    void Pause() { paused_ = true; }

    /**
     * @brief Resume the intro sequence
     */
    void Resume() { paused_ = false; }

    /**
     * @brief Check if paused
     * @return True if paused
     */
    bool IsPaused() const { return paused_; }

    /**
     * @brief Get current scene index
     * @return Scene index
     */
    int GetCurrentScene() const { return currentScene_; }

    /**
     * @brief Get total number of scenes
     * @return Scene count
     */
    int GetTotalScenes() const { return static_cast<int>(scenes_.size()); }

private:
    // Initialization
    void LoadIntroScenes();
    void LoadSubtitles();
    void LoadAudioAssets();

    // Rendering
    void RenderCurrentScene(SDL_Renderer* renderer);
    void RenderLetterbox(SDL_Renderer* renderer);
    void RenderSubtitles(SDL_Renderer* renderer);
    void RenderProgress(SDL_Renderer* renderer);
    void RenderSkipPrompt(SDL_Renderer* renderer);

    // Scene rendering
    void RenderFadeInScene(SDL_Renderer* renderer, const CinematicScene& scene);
    void RenderStillImageScene(SDL_Renderer* renderer, const CinematicScene& scene);
    void RenderTextScrollScene(SDL_Renderer* renderer, const CinematicScene& scene);
    void RenderDialogueScene(SDL_Renderer* renderer, const CinematicScene& scene);
    void RenderActionScene(SDL_Renderer* renderer, const CinematicScene& scene);
    void RenderFadeOutScene(SDL_Renderer* renderer, const CinematicScene& scene);

    // Updates
    void UpdateCurrentScene(float deltaTime);
    void UpdateSubtitles(float deltaTime);
    void UpdateMusic(float deltaTime);
    void TransitionToNextScene();

    // Utilities
    void PlaySceneMusic(const CinematicScene& scene);
    void StopMusic();
    void PlayVoiceLine(const std::string& voicePath);
    float GetSceneProgress() const;
    Subtitle* GetActiveSubtitle();

    // Screen properties
    int screenWidth_;
    int screenHeight_;
    SDL_Renderer* renderer_;

    // Scene management
    std::vector<CinematicScene> scenes_;
    int currentScene_;
    float totalTime_;

    // Subtitles
    std::vector<Subtitle> subtitles_;
    int currentSubtitle_;

    // State
    bool paused_;
    bool allowSkip_;
    float skipPromptAlpha_;
    float skipHoldTime_;
    float skipHoldRequired_;

    // Visual effects
    int letterboxHeight_;
    float fadeAlpha_;
    float textScrollOffset_;

    // Callbacks
    std::function<void()> onComplete_;
    std::function<void()> onSkip_;

    // Configuration
    bool showProgress_;
    bool showSkipPrompt_;
    bool letterboxEnabled_;
};

} // namespace Launcher
