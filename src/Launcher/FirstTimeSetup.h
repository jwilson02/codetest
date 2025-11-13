#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace Launcher {

/**
 * @brief Setup wizard page types
 */
enum class SetupPage {
    Welcome,
    Language,
    Graphics,
    Audio,
    Controls,
    Accessibility,
    Privacy,
    Complete
};

/**
 * @brief Graphics quality preset
 */
enum class GraphicsQuality {
    Low,
    Medium,
    High,
    Ultra,
    Custom
};

/**
 * @brief Control scheme preset
 */
enum class ControlScheme {
    Keyboard,
    Gamepad,
    KeyboardMouse,
    Custom
};

/**
 * @brief Setup configuration storage
 */
struct SetupConfig {
    std::string language;
    GraphicsQuality graphicsQuality;
    int screenWidth;
    int screenHeight;
    bool fullscreen;
    bool vsync;

    float masterVolume;
    float musicVolume;
    float sfxVolume;
    float voiceVolume;

    ControlScheme controlScheme;
    bool invertY;
    float mouseSensitivity;

    bool subtitlesEnabled;
    bool screenReaderEnabled;
    bool colorblindMode;
    bool reduceMotion;

    bool telemetryEnabled;
    bool crashReportsEnabled;

    std::map<std::string, std::string> customSettings;
};

/**
 * @brief UI control types for wizard
 */
enum class ControlType {
    Button,
    Slider,
    Toggle,
    Dropdown,
    TextInput
};

/**
 * @brief Wizard UI control
 */
struct WizardControl {
    std::string id;
    std::string label;
    ControlType type;
    SDL_Rect rect;
    bool enabled;
    bool hovered;
    float value;
    std::vector<std::string> options;
    int selectedOption;
    std::function<void()> onChange;
};

/**
 * @brief First-Time Setup Wizard
 *
 * Professional setup wizard for first-time users featuring:
 * - Multi-page guided setup
 * - Graphics quality detection and presets
 * - Audio configuration with test sounds
 * - Control scheme selection and testing
 * - Accessibility options
 * - Privacy and telemetry settings
 * - Smooth page transitions
 * - Progress indicator
 * - Ability to skip or go back
 */
class FirstTimeSetup {
public:
    FirstTimeSetup();
    ~FirstTimeSetup();

    /**
     * @brief Initialize the setup wizard
     * @param renderer SDL renderer
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @return True if initialization successful
     */
    bool Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight);

    /**
     * @brief Update wizard logic
     * @param deltaTime Time elapsed since last frame
     */
    void Update(float deltaTime);

    /**
     * @brief Render the wizard
     * @param renderer SDL renderer
     */
    void Render(SDL_Renderer* renderer);

    /**
     * @brief Handle input events
     * @param event SDL event
     */
    void HandleInput(const SDL_Event& event);

    /**
     * @brief Check if setup is complete
     * @return True if complete
     */
    bool IsComplete() const { return currentPage_ == SetupPage::Complete; }

    /**
     * @brief Get the configured settings
     * @return Setup configuration
     */
    const SetupConfig& GetConfig() const { return config_; }

    /**
     * @brief Set callback for setup completion
     * @param callback Function to call when complete
     */
    void SetOnComplete(std::function<void()> callback) {
        onComplete_ = callback;
    }

    /**
     * @brief Skip the setup wizard (use defaults)
     */
    void SkipSetup();

    /**
     * @brief Go to specific page
     * @param page Target page
     */
    void GoToPage(SetupPage page);

private:
    // Initialization
    void DetectSystemCapabilities();
    void LoadDefaultSettings();
    void CreatePageControls(SetupPage page);

    // Rendering
    void RenderBackground(SDL_Renderer* renderer);
    void RenderHeader(SDL_Renderer* renderer);
    void RenderCurrentPage(SDL_Renderer* renderer);
    void RenderPageControls(SDL_Renderer* renderer);
    void RenderControl(SDL_Renderer* renderer, const WizardControl& control);
    void RenderProgressBar(SDL_Renderer* renderer);
    void RenderNavigationButtons(SDL_Renderer* renderer);
    void RenderPageTransition(SDL_Renderer* renderer);

    // Page rendering
    void RenderWelcomePage(SDL_Renderer* renderer);
    void RenderLanguagePage(SDL_Renderer* renderer);
    void RenderGraphicsPage(SDL_Renderer* renderer);
    void RenderAudioPage(SDL_Renderer* renderer);
    void RenderControlsPage(SDL_Renderer* renderer);
    void RenderAccessibilityPage(SDL_Renderer* renderer);
    void RenderPrivacyPage(SDL_Renderer* renderer);
    void RenderCompletePage(SDL_Renderer* renderer);

    // Control rendering
    void RenderButton(SDL_Renderer* renderer, const WizardControl& control);
    void RenderSlider(SDL_Renderer* renderer, const WizardControl& control);
    void RenderToggle(SDL_Renderer* renderer, const WizardControl& control);
    void RenderDropdown(SDL_Renderer* renderer, const WizardControl& control);

    // Updates
    void UpdateAnimations(float deltaTime);
    void UpdateControls(float deltaTime);

    // Navigation
    void NextPage();
    void PreviousPage();
    void CompleteSetup();

    // Input handling
    void HandleKeyboardInput(const SDL_Event& event);
    void HandleGamepadInput(const SDL_Event& event);
    void HandleMouseInput(const SDL_Event& event);

    // Control interaction
    void SelectNextControl();
    void SelectPreviousControl();
    void ActivateControl(WizardControl& control);
    void UpdateControlValue(WizardControl& control, float delta);

    // Settings
    void ApplyGraphicsPreset(GraphicsQuality quality);
    void ApplyControlPreset(ControlScheme scheme);
    void TestAudioSettings();
    void DetectOptimalSettings();
    void SaveConfiguration();

    // Utilities
    int GetPageIndex(SetupPage page) const;
    std::string GetPageTitle(SetupPage page) const;
    std::string GetPageDescription(SetupPage page) const;

    // Screen properties
    int screenWidth_;
    int screenHeight_;
    SDL_Renderer* renderer_;

    // State
    SetupPage currentPage_;
    SetupConfig config_;
    std::vector<WizardControl> currentControls_;
    int selectedControlIndex_;

    // System detection
    bool systemDetected_;
    int detectedScreenWidth_;
    int detectedScreenHeight_;
    GraphicsQuality recommendedQuality_;

    // Animation
    float transitionProgress_;
    float transitionDuration_;
    bool isTransitioning_;
    SetupPage transitionTarget_;
    float pageAnimTimer_;

    // Callbacks
    std::function<void()> onComplete_;

    // Visual
    SDL_Color accentColor_;
    bool gamepadActive_;
};

} // namespace Launcher
