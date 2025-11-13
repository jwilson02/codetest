#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace Launcher {

/**
 * @brief Menu button configuration
 */
struct MenuButton {
    std::string id;
    std::string label;
    std::string iconPath;
    SDL_Rect rect;
    bool enabled;
    bool hovered;
    float scale;
    float glowIntensity;
    std::function<void()> onClick;
};

/**
 * @brief Menu animation state
 */
struct MenuAnimation {
    float time;
    float duration;
    bool active;
    int startValue;
    int endValue;
    int currentValue;
};

/**
 * @brief Main Menu System
 *
 * Professional main menu with smooth animations, particle effects,
 * and gamepad navigation support. Features include:
 * - Animated background with particles
 * - Smooth button transitions
 * - Gamepad and keyboard navigation
 * - Settings and credits screens
 * - Save game detection and management
 */
class MainMenu {
public:
    MainMenu();
    ~MainMenu();

    /**
     * @brief Initialize the main menu
     * @param renderer SDL renderer
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @return True if initialization successful
     */
    bool Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight);

    /**
     * @brief Update menu logic
     * @param deltaTime Time elapsed since last frame
     */
    void Update(float deltaTime);

    /**
     * @brief Render the menu
     * @param renderer SDL renderer
     */
    void Render(SDL_Renderer* renderer);

    /**
     * @brief Handle input events
     * @param event SDL event
     */
    void HandleInput(const SDL_Event& event);

    /**
     * @brief Set callback for New Game button
     * @param callback Function to call with character name
     */
    void SetOnNewGame(std::function<void(const std::string&)> callback) {
        onNewGame_ = callback;
    }

    /**
     * @brief Set callback for Continue button
     * @param callback Function to call
     */
    void SetOnContinue(std::function<void()> callback) {
        onContinue_ = callback;
    }

    /**
     * @brief Set callback for Settings button
     * @param callback Function to call
     */
    void SetOnSettings(std::function<void()> callback) {
        onSettings_ = callback;
    }

    /**
     * @brief Set callback for Credits button
     * @param callback Function to call
     */
    void SetOnCredits(std::function<void()> callback) {
        onCredits_ = callback;
    }

    /**
     * @brief Set callback for Exit button
     * @param callback Function to call
     */
    void SetOnExit(std::function<void()> callback) {
        onExit_ = callback;
    }

    /**
     * @brief Check if save game exists
     * @return True if save game found
     */
    bool HasSaveGame() const { return hasSaveGame_; }

    /**
     * @brief Get currently selected button index
     * @return Button index
     */
    int GetSelectedButtonIndex() const { return selectedButtonIndex_; }

    /**
     * @brief Enable or disable menu animations
     * @param enabled Animation state
     */
    void SetAnimationsEnabled(bool enabled) { animationsEnabled_ = enabled; }

    /**
     * @brief Enable or disable particle effects
     * @param enabled Particle state
     */
    void SetParticlesEnabled(bool enabled) { particlesEnabled_ = enabled; }

private:
    // Initialization
    void CreateButtons();
    void LoadMenuAssets();
    void CheckSaveGames();

    // Rendering
    void RenderBackground(SDL_Renderer* renderer);
    void RenderTitle(SDL_Renderer* renderer);
    void RenderButtons(SDL_Renderer* renderer);
    void RenderButton(SDL_Renderer* renderer, const MenuButton& button);
    void RenderVersionInfo(SDL_Renderer* renderer);
    void RenderGamepadHints(SDL_Renderer* renderer);

    // Updates
    void UpdateAnimations(float deltaTime);
    void UpdateButtons(float deltaTime);
    void UpdateBackground(float deltaTime);

    // Input handling
    void HandleKeyboardInput(const SDL_Event& event);
    void HandleGamepadInput(const SDL_Event& event);
    void HandleMouseInput(const SDL_Event& event);

    // Navigation
    void SelectNextButton();
    void SelectPreviousButton();
    void ActivateSelectedButton();
    void SelectButtonAtPosition(int x, int y);

    // Effects
    void PlayButtonHoverEffect(int buttonIndex);
    void PlayButtonClickEffect(int buttonIndex);
    void ShakeScreen(float intensity);

    // Screen properties
    int screenWidth_;
    int screenHeight_;
    SDL_Renderer* renderer_;

    // Menu buttons
    std::vector<MenuButton> buttons_;
    int selectedButtonIndex_;
    bool gamepadNavigationActive_;

    // Animations
    bool animationsEnabled_;
    bool particlesEnabled_;
    float titlePulseTimer_;
    float backgroundScrollOffset_;
    float buttonAnimTimer_;

    MenuAnimation fadeAnimation_;
    MenuAnimation slideAnimation_;

    // State
    bool hasSaveGame_;
    bool isGamepadConnected_;
    float screenShakeIntensity_;
    float screenShakeTimer_;

    // Callbacks
    std::function<void(const std::string&)> onNewGame_;
    std::function<void()> onContinue_;
    std::function<void()> onSettings_;
    std::function<void()> onCredits_;
    std::function<void()> onExit_;

    // Visual properties
    SDL_Color titleColor_;
    SDL_Color accentColor_;
    float globalAlpha_;
};

} // namespace Launcher
