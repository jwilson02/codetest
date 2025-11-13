#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace Launcher {

// Forward declarations
class MainMenu;
class CharacterSelect;
class IntroSequence;
class FirstTimeSetup;

/**
 * @brief Launcher state enumeration
 */
enum class LauncherState {
    SplashScreen,
    StudioLogos,
    FirstTimeSetup,
    MainMenu,
    CharacterSelection,
    LoadGame,
    Settings,
    Credits,
    IntroSequence,
    StartingGame,
    Exiting
};

/**
 * @brief DLC/Expansion information structure
 */
struct DLCInfo {
    std::string id;
    std::string name;
    std::string version;
    std::string description;
    std::string iconPath;
    bool installed;
    bool enabled;
    uint64_t fileSize;
    std::string releaseDate;
};

/**
 * @brief Studio logo configuration
 */
struct StudioLogo {
    std::string texturePath;
    float displayDuration;
    float fadeInDuration;
    float fadeOutDuration;
    std::string soundPath;
};

/**
 * @brief Particle effect data for menu animations
 */
struct MenuParticle {
    float x, y;
    float vx, vy;
    float lifetime;
    float maxLifetime;
    float size;
    SDL_Color color;
    float alpha;
};

/**
 * @brief Game Launcher System
 *
 * Manages the entire game launch sequence including splash screens,
 * studio logos, main menu, character selection, and game initialization.
 * Supports DLC detection, first-time setup, and smooth transitions.
 */
class GameLauncher {
public:
    GameLauncher();
    ~GameLauncher();

    /**
     * @brief Initialize the launcher system
     * @param renderer SDL renderer
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @return True if initialization was successful
     */
    bool Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight);

    /**
     * @brief Update launcher logic
     * @param deltaTime Time elapsed since last frame
     */
    void Update(float deltaTime);

    /**
     * @brief Render launcher UI
     * @param renderer SDL renderer
     */
    void Render(SDL_Renderer* renderer);

    /**
     * @brief Handle input events
     * @param event SDL event
     */
    void HandleInput(const SDL_Event& event);

    /**
     * @brief Check if launcher is ready to start the game
     * @return True if game should start
     */
    bool ShouldStartGame() const { return shouldStartGame_; }

    /**
     * @brief Check if launcher should exit
     * @return True if launcher should exit
     */
    bool ShouldExit() const { return shouldExit_; }

    /**
     * @brief Get selected character class
     * @return Character class name
     */
    std::string GetSelectedCharacter() const { return selectedCharacter_; }

    /**
     * @brief Get current launcher state
     * @return Current state
     */
    LauncherState GetState() const { return currentState_; }

    /**
     * @brief Set callback for when game starts
     * @param callback Function to call when game starts
     */
    void SetOnGameStart(std::function<void(const std::string&)> callback) {
        onGameStart_ = callback;
    }

    /**
     * @brief Set callback for when launcher exits
     * @param callback Function to call on exit
     */
    void SetOnExit(std::function<void()> callback) {
        onExit_ = callback;
    }

    /**
     * @brief Check if this is the first time launching the game
     * @return True if first launch
     */
    bool IsFirstLaunch() const;

    /**
     * @brief Get list of detected DLCs
     * @return Vector of DLC information
     */
    const std::vector<DLCInfo>& GetDetectedDLCs() const { return detectedDLCs_; }

    /**
     * @brief Enable or disable a DLC
     * @param dlcId DLC identifier
     * @param enabled Enable/disable state
     */
    void SetDLCEnabled(const std::string& dlcId, bool enabled);

    /**
     * @brief Check if gamepad navigation is active
     * @return True if using gamepad
     */
    bool IsGamepadActive() const { return gamepadActive_; }

    /**
     * @brief Get transition progress (0.0 to 1.0)
     * @return Current transition progress
     */
    float GetTransitionProgress() const { return transitionProgress_; }

private:
    // State management
    void TransitionToState(LauncherState newState);
    void UpdateSplashScreen(float deltaTime);
    void UpdateStudioLogos(float deltaTime);
    void UpdateFirstTimeSetup(float deltaTime);
    void UpdateMainMenu(float deltaTime);
    void UpdateCharacterSelection(float deltaTime);
    void UpdateIntroSequence(float deltaTime);

    // Rendering
    void RenderSplashScreen(SDL_Renderer* renderer);
    void RenderStudioLogos(SDL_Renderer* renderer);
    void RenderParticles(SDL_Renderer* renderer);
    void RenderTransition(SDL_Renderer* renderer);

    // Initialization
    bool LoadConfiguration();
    bool LoadStudioLogos();
    void DetectDLCs();
    void CheckFirstLaunch();

    // Particle system
    void InitializeParticles();
    void UpdateParticles(float deltaTime);
    void SpawnParticle(float x, float y);

    // Input handling
    void HandleKeyboardInput(const SDL_Event& event);
    void HandleGamepadInput(const SDL_Event& event);
    void HandleMouseInput(const SDL_Event& event);

    // Utility
    void SkipCurrentScreen();
    void SaveLaunchPreferences();
    void LoadLaunchPreferences();

    // State
    LauncherState currentState_;
    LauncherState previousState_;
    bool shouldStartGame_;
    bool shouldExit_;
    bool isFirstLaunch_;
    bool gamepadActive_;
    std::string selectedCharacter_;

    // Screen properties
    int screenWidth_;
    int screenHeight_;
    SDL_Renderer* renderer_;

    // Timing
    float splashTimer_;
    float logoTimer_;
    float transitionProgress_;
    float transitionDuration_;
    bool isTransitioning_;

    // Studio logos
    std::vector<StudioLogo> studioLogos_;
    int currentLogoIndex_;
    float currentLogoAlpha_;

    // Components
    std::unique_ptr<MainMenu> mainMenu_;
    std::unique_ptr<CharacterSelect> characterSelect_;
    std::unique_ptr<IntroSequence> introSequence_;
    std::unique_ptr<FirstTimeSetup> firstTimeSetup_;

    // DLC management
    std::vector<DLCInfo> detectedDLCs_;

    // Particle effects
    std::vector<MenuParticle> particles_;
    float particleSpawnTimer_;
    bool particlesEnabled_;

    // Callbacks
    std::function<void(const std::string&)> onGameStart_;
    std::function<void()> onExit_;

    // Configuration
    bool skipSplash_;
    bool skipLogos_;
    bool showIntro_;
    float splashDuration_;
};

} // namespace Launcher
