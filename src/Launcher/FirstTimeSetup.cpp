#include "FirstTimeSetup.h"
#include <cmath>
#include <algorithm>

namespace Launcher {

FirstTimeSetup::FirstTimeSetup()
    : screenWidth_(1280),
      screenHeight_(720),
      renderer_(nullptr),
      currentPage_(SetupPage::Welcome),
      selectedControlIndex_(0),
      systemDetected_(false),
      detectedScreenWidth_(1920),
      detectedScreenHeight_(1080),
      recommendedQuality_(GraphicsQuality::High),
      transitionProgress_(0.0f),
      transitionDuration_(0.4f),
      isTransitioning_(false),
      transitionTarget_(SetupPage::Welcome),
      pageAnimTimer_(0.0f),
      accentColor_({75, 144, 226, 255}),
      gamepadActive_(false) {

    // Initialize default config
    LoadDefaultSettings();
}

FirstTimeSetup::~FirstTimeSetup() {
}

bool FirstTimeSetup::Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    renderer_ = renderer;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;

    // Detect system capabilities
    DetectSystemCapabilities();

    // Create controls for welcome page
    CreatePageControls(currentPage_);

    SDL_Log("FirstTimeSetup initialized");
    return true;
}

void FirstTimeSetup::Update(float deltaTime) {
    // Update animations
    UpdateAnimations(deltaTime);

    // Update controls
    UpdateControls(deltaTime);

    // Update page transition
    if (isTransitioning_) {
        transitionProgress_ += deltaTime / transitionDuration_;
        if (transitionProgress_ >= 1.0f) {
            transitionProgress_ = 1.0f;
            isTransitioning_ = false;
            currentPage_ = transitionTarget_;
            CreatePageControls(currentPage_);
        }
    }
}

void FirstTimeSetup::Render(SDL_Renderer* renderer) {
    // Render background
    RenderBackground(renderer);

    // Render header
    RenderHeader(renderer);

    // Render current page content
    RenderCurrentPage(renderer);

    // Render page controls
    RenderPageControls(renderer);

    // Render progress bar
    RenderProgressBar(renderer);

    // Render navigation buttons
    RenderNavigationButtons(renderer);

    // Render transition effect
    if (isTransitioning_) {
        RenderPageTransition(renderer);
    }
}

void FirstTimeSetup::HandleInput(const SDL_Event& event) {
    // Detect input type
    if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERAXISMOTION) {
        gamepadActive_ = true;
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEMOTION) {
        gamepadActive_ = false;
    }

    HandleKeyboardInput(event);
    HandleGamepadInput(event);
    HandleMouseInput(event);
}

void FirstTimeSetup::SkipSetup() {
    // Use detected optimal settings
    ApplyGraphicsPreset(recommendedQuality_);
    CompleteSetup();
}

void FirstTimeSetup::GoToPage(SetupPage page) {
    if (currentPage_ == page) return;

    transitionTarget_ = page;
    isTransitioning_ = true;
    transitionProgress_ = 0.0f;
}

// Private methods

void FirstTimeSetup::DetectSystemCapabilities() {
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
        detectedScreenWidth_ = displayMode.w;
        detectedScreenHeight_ = displayMode.h;
        systemDetected_ = true;

        // Determine recommended quality based on resolution
        if (detectedScreenWidth_ >= 3840) {
            recommendedQuality_ = GraphicsQuality::Ultra;
        } else if (detectedScreenWidth_ >= 1920) {
            recommendedQuality_ = GraphicsQuality::High;
        } else if (detectedScreenWidth_ >= 1280) {
            recommendedQuality_ = GraphicsQuality::Medium;
        } else {
            recommendedQuality_ = GraphicsQuality::Low;
        }

        SDL_Log("Detected resolution: %dx%d, recommended quality: %d",
                detectedScreenWidth_, detectedScreenHeight_,
                static_cast<int>(recommendedQuality_));
    }
}

void FirstTimeSetup::LoadDefaultSettings() {
    config_.language = "English";
    config_.graphicsQuality = GraphicsQuality::High;
    config_.screenWidth = 1920;
    config_.screenHeight = 1080;
    config_.fullscreen = false;
    config_.vsync = true;

    config_.masterVolume = 0.7f;
    config_.musicVolume = 0.8f;
    config_.sfxVolume = 0.7f;
    config_.voiceVolume = 0.9f;

    config_.controlScheme = ControlScheme::KeyboardMouse;
    config_.invertY = false;
    config_.mouseSensitivity = 0.5f;

    config_.subtitlesEnabled = true;
    config_.screenReaderEnabled = false;
    config_.colorblindMode = false;
    config_.reduceMotion = false;

    config_.telemetryEnabled = true;
    config_.crashReportsEnabled = true;
}

void FirstTimeSetup::CreatePageControls(SetupPage page) {
    currentControls_.clear();
    selectedControlIndex_ = 0;

    int centerX = screenWidth_ / 2;
    int startY = 280;
    int controlWidth = 500;
    int controlHeight = 50;
    int spacing = 20;

    switch (page) {
        case SetupPage::Welcome:
            // No controls needed for welcome page
            break;

        case SetupPage::Language: {
            WizardControl langDropdown;
            langDropdown.id = "language";
            langDropdown.label = "Select Language";
            langDropdown.type = ControlType::Dropdown;
            langDropdown.rect = {centerX - controlWidth/2, startY, controlWidth, controlHeight};
            langDropdown.enabled = true;
            langDropdown.hovered = false;
            langDropdown.value = 0;
            langDropdown.options = {"English", "Spanish", "French", "German", "Japanese", "Chinese"};
            langDropdown.selectedOption = 0;
            currentControls_.push_back(langDropdown);
            break;
        }

        case SetupPage::Graphics: {
            // Quality preset dropdown
            WizardControl qualityDropdown;
            qualityDropdown.id = "graphics_quality";
            qualityDropdown.label = "Graphics Quality";
            qualityDropdown.type = ControlType::Dropdown;
            qualityDropdown.rect = {centerX - controlWidth/2, startY, controlWidth, controlHeight};
            qualityDropdown.enabled = true;
            qualityDropdown.hovered = false;
            qualityDropdown.options = {"Low", "Medium", "High", "Ultra", "Custom"};
            qualityDropdown.selectedOption = static_cast<int>(config_.graphicsQuality);
            currentControls_.push_back(qualityDropdown);

            // Resolution dropdown
            WizardControl resolutionDropdown;
            resolutionDropdown.id = "resolution";
            resolutionDropdown.label = "Resolution";
            resolutionDropdown.type = ControlType::Dropdown;
            resolutionDropdown.rect = {centerX - controlWidth/2, startY + (controlHeight + spacing), controlWidth, controlHeight};
            resolutionDropdown.enabled = true;
            resolutionDropdown.hovered = false;
            resolutionDropdown.options = {"1280x720", "1920x1080", "2560x1440", "3840x2160"};
            resolutionDropdown.selectedOption = 1;
            currentControls_.push_back(resolutionDropdown);

            // Fullscreen toggle
            WizardControl fullscreenToggle;
            fullscreenToggle.id = "fullscreen";
            fullscreenToggle.label = "Fullscreen";
            fullscreenToggle.type = ControlType::Toggle;
            fullscreenToggle.rect = {centerX - controlWidth/2, startY + 2 * (controlHeight + spacing), controlWidth, controlHeight};
            fullscreenToggle.enabled = true;
            fullscreenToggle.hovered = false;
            fullscreenToggle.value = config_.fullscreen ? 1.0f : 0.0f;
            currentControls_.push_back(fullscreenToggle);

            // VSync toggle
            WizardControl vsyncToggle;
            vsyncToggle.id = "vsync";
            vsyncToggle.label = "VSync";
            vsyncToggle.type = ControlType::Toggle;
            vsyncToggle.rect = {centerX - controlWidth/2, startY + 3 * (controlHeight + spacing), controlWidth, controlHeight};
            vsyncToggle.enabled = true;
            vsyncToggle.hovered = false;
            vsyncToggle.value = config_.vsync ? 1.0f : 0.0f;
            currentControls_.push_back(vsyncToggle);
            break;
        }

        case SetupPage::Audio: {
            // Master volume slider
            WizardControl masterVolume;
            masterVolume.id = "master_volume";
            masterVolume.label = "Master Volume";
            masterVolume.type = ControlType::Slider;
            masterVolume.rect = {centerX - controlWidth/2, startY, controlWidth, controlHeight};
            masterVolume.enabled = true;
            masterVolume.hovered = false;
            masterVolume.value = config_.masterVolume;
            currentControls_.push_back(masterVolume);

            // Music volume slider
            WizardControl musicVolume;
            musicVolume.id = "music_volume";
            musicVolume.label = "Music Volume";
            musicVolume.type = ControlType::Slider;
            musicVolume.rect = {centerX - controlWidth/2, startY + (controlHeight + spacing), controlWidth, controlHeight};
            musicVolume.enabled = true;
            musicVolume.hovered = false;
            musicVolume.value = config_.musicVolume;
            currentControls_.push_back(musicVolume);

            // SFX volume slider
            WizardControl sfxVolume;
            sfxVolume.id = "sfx_volume";
            sfxVolume.label = "Sound Effects Volume";
            sfxVolume.type = ControlType::Slider;
            sfxVolume.rect = {centerX - controlWidth/2, startY + 2 * (controlHeight + spacing), controlWidth, controlHeight};
            sfxVolume.enabled = true;
            sfxVolume.hovered = false;
            sfxVolume.value = config_.sfxVolume;
            currentControls_.push_back(sfxVolume);

            // Voice volume slider
            WizardControl voiceVolume;
            voiceVolume.id = "voice_volume";
            voiceVolume.label = "Voice Volume";
            voiceVolume.type = ControlType::Slider;
            voiceVolume.rect = {centerX - controlWidth/2, startY + 3 * (controlHeight + spacing), controlWidth, controlHeight};
            voiceVolume.enabled = true;
            voiceVolume.hovered = false;
            voiceVolume.value = config_.voiceVolume;
            currentControls_.push_back(voiceVolume);
            break;
        }

        case SetupPage::Controls: {
            // Control scheme dropdown
            WizardControl schemeDropdown;
            schemeDropdown.id = "control_scheme";
            schemeDropdown.label = "Control Scheme";
            schemeDropdown.type = ControlType::Dropdown;
            schemeDropdown.rect = {centerX - controlWidth/2, startY, controlWidth, controlHeight};
            schemeDropdown.enabled = true;
            schemeDropdown.hovered = false;
            schemeDropdown.options = {"Keyboard Only", "Gamepad", "Keyboard + Mouse", "Custom"};
            schemeDropdown.selectedOption = static_cast<int>(config_.controlScheme);
            currentControls_.push_back(schemeDropdown);

            // Mouse sensitivity slider
            WizardControl sensitivity;
            sensitivity.id = "mouse_sensitivity";
            sensitivity.label = "Mouse Sensitivity";
            sensitivity.type = ControlType::Slider;
            sensitivity.rect = {centerX - controlWidth/2, startY + (controlHeight + spacing), controlWidth, controlHeight};
            sensitivity.enabled = true;
            sensitivity.hovered = false;
            sensitivity.value = config_.mouseSensitivity;
            currentControls_.push_back(sensitivity);

            // Invert Y toggle
            WizardControl invertY;
            invertY.id = "invert_y";
            invertY.label = "Invert Y Axis";
            invertY.type = ControlType::Toggle;
            invertY.rect = {centerX - controlWidth/2, startY + 2 * (controlHeight + spacing), controlWidth, controlHeight};
            invertY.enabled = true;
            invertY.hovered = false;
            invertY.value = config_.invertY ? 1.0f : 0.0f;
            currentControls_.push_back(invertY);
            break;
        }

        case SetupPage::Accessibility: {
            // Subtitles toggle
            WizardControl subtitles;
            subtitles.id = "subtitles";
            subtitles.label = "Enable Subtitles";
            subtitles.type = ControlType::Toggle;
            subtitles.rect = {centerX - controlWidth/2, startY, controlWidth, controlHeight};
            subtitles.enabled = true;
            subtitles.hovered = false;
            subtitles.value = config_.subtitlesEnabled ? 1.0f : 0.0f;
            currentControls_.push_back(subtitles);

            // Colorblind mode toggle
            WizardControl colorblind;
            colorblind.id = "colorblind_mode";
            colorblind.label = "Colorblind Mode";
            colorblind.type = ControlType::Toggle;
            colorblind.rect = {centerX - controlWidth/2, startY + (controlHeight + spacing), controlWidth, controlHeight};
            colorblind.enabled = true;
            colorblind.hovered = false;
            colorblind.value = config_.colorblindMode ? 1.0f : 0.0f;
            currentControls_.push_back(colorblind);

            // Reduce motion toggle
            WizardControl reduceMotion;
            reduceMotion.id = "reduce_motion";
            reduceMotion.label = "Reduce Motion Effects";
            reduceMotion.type = ControlType::Toggle;
            reduceMotion.rect = {centerX - controlWidth/2, startY + 2 * (controlHeight + spacing), controlWidth, controlHeight};
            reduceMotion.enabled = true;
            reduceMotion.hovered = false;
            reduceMotion.value = config_.reduceMotion ? 1.0f : 0.0f;
            currentControls_.push_back(reduceMotion);
            break;
        }

        case SetupPage::Privacy: {
            // Telemetry toggle
            WizardControl telemetry;
            telemetry.id = "telemetry";
            telemetry.label = "Enable Anonymous Usage Data";
            telemetry.type = ControlType::Toggle;
            telemetry.rect = {centerX - controlWidth/2, startY, controlWidth, controlHeight};
            telemetry.enabled = true;
            telemetry.hovered = false;
            telemetry.value = config_.telemetryEnabled ? 1.0f : 0.0f;
            currentControls_.push_back(telemetry);

            // Crash reports toggle
            WizardControl crashReports;
            crashReports.id = "crash_reports";
            crashReports.label = "Send Crash Reports";
            crashReports.type = ControlType::Toggle;
            crashReports.rect = {centerX - controlWidth/2, startY + (controlHeight + spacing), controlWidth, controlHeight};
            crashReports.enabled = true;
            crashReports.hovered = false;
            crashReports.value = config_.crashReportsEnabled ? 1.0f : 0.0f;
            currentControls_.push_back(crashReports);
            break;
        }

        default:
            break;
    }
}

void FirstTimeSetup::RenderBackground(SDL_Renderer* renderer) {
    // Gradient background
    for (int y = 0; y < screenHeight_; y++) {
        float gradient = static_cast<float>(y) / screenHeight_;
        Uint8 r = static_cast<Uint8>(20 + gradient * 10);
        Uint8 g = static_cast<Uint8>(20 + gradient * 10);
        Uint8 b = static_cast<Uint8>(30 + gradient * 20);

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, screenWidth_, y);
    }

    // Decorative elements
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 20);

    for (int i = 0; i < 3; i++) {
        SDL_Rect decorRect = {
            i * 400 - 100,
            i * 200,
            300,
            300
        };
        SDL_RenderDrawRect(renderer, &decorRect);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::RenderHeader(SDL_Renderer* renderer) {
    // Header panel
    SDL_Rect headerRect = {0, 0, screenWidth_, 120};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 240);
    SDL_RenderFillRect(renderer, &headerRect);

    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 255);
    SDL_RenderDrawLine(renderer, 0, 120, screenWidth_, 120);

    // Title
    SDL_Rect titleRect = {
        screenWidth_ / 2 - 250,
        30,
        500,
        60
    };

    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 255);
    for (int i = 0; i < 2; i++) {
        SDL_RenderDrawRect(renderer, &titleRect);
        titleRect.x++; titleRect.y++;
        titleRect.w -= 2; titleRect.h -= 2;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::RenderCurrentPage(SDL_Renderer* renderer) {
    // Page content panel
    SDL_Rect contentRect = {
        50,
        150,
        screenWidth_ - 100,
        screenHeight_ - 280
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 220);
    SDL_RenderFillRect(renderer, &contentRect);

    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 150);
    SDL_RenderDrawRect(renderer, &contentRect);

    // Page title
    SDL_Rect pageTitleRect = {
        100,
        180,
        screenWidth_ - 200,
        40
    };

    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(renderer, &pageTitleRect);

    // Page description
    SDL_Rect descRect = {
        100,
        230,
        screenWidth_ - 200,
        30
    };

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
    SDL_RenderDrawRect(renderer, &descRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Render page-specific content
    switch (currentPage_) {
        case SetupPage::Welcome:
            RenderWelcomePage(renderer);
            break;
        case SetupPage::Complete:
            RenderCompletePage(renderer);
            break;
        default:
            break;
    }
}

void FirstTimeSetup::RenderPageControls(SDL_Renderer* renderer) {
    for (size_t i = 0; i < currentControls_.size(); ++i) {
        const auto& control = currentControls_[i];
        bool selected = (static_cast<int>(i) == selectedControlIndex_);

        if (selected && gamepadActive_) {
            // Draw selection indicator
            SDL_Rect selectRect = control.rect;
            selectRect.x -= 5;
            selectRect.y -= 5;
            selectRect.w += 10;
            selectRect.h += 10;

            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 180);
            SDL_RenderDrawRect(renderer, &selectRect);
        }

        RenderControl(renderer, control);
    }
}

void FirstTimeSetup::RenderControl(SDL_Renderer* renderer, const WizardControl& control) {
    switch (control.type) {
        case ControlType::Button:
            RenderButton(renderer, control);
            break;
        case ControlType::Slider:
            RenderSlider(renderer, control);
            break;
        case ControlType::Toggle:
            RenderToggle(renderer, control);
            break;
        case ControlType::Dropdown:
            RenderDropdown(renderer, control);
            break;
        default:
            break;
    }
}

void FirstTimeSetup::RenderButton(SDL_Renderer* renderer, const WizardControl& control) {
    SDL_Rect rect = control.rect;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (control.hovered) {
        SDL_SetRenderDrawColor(renderer, 60, 60, 80, 240);
    } else {
        SDL_SetRenderDrawColor(renderer, 40, 40, 60, 220);
    }
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b,control.hovered ? 255 : 180);
    SDL_RenderDrawRect(renderer, &rect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::RenderSlider(SDL_Renderer* renderer, const WizardControl& control) {
    // Label
    SDL_Rect labelRect = {control.rect.x, control.rect.y - 25, control.rect.w, 20};
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &labelRect);

    // Slider track
    SDL_Rect trackRect = {
        control.rect.x,
        control.rect.y + control.rect.h / 2 - 3,
        control.rect.w,
        6
    };

    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderFillRect(renderer, &trackRect);

    // Slider fill
    int fillWidth = static_cast<int>(control.rect.w * control.value);
    SDL_Rect fillRect = {trackRect.x, trackRect.y, fillWidth, trackRect.h};

    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 255);
    SDL_RenderFillRect(renderer, &fillRect);

    // Slider handle
    int handleX = trackRect.x + fillWidth;
    SDL_Rect handleRect = {handleX - 8, trackRect.y - 8, 16, trackRect.h + 16};

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &handleRect);

    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 255);
    SDL_RenderDrawRect(renderer, &handleRect);
}

void FirstTimeSetup::RenderToggle(SDL_Renderer* renderer, const WizardControl& control) {
    // Label
    SDL_Rect labelRect = {control.rect.x, control.rect.y, control.rect.w - 100, control.rect.h};
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &labelRect);

    // Toggle background
    SDL_Rect toggleBg = {
        control.rect.x + control.rect.w - 80,
        control.rect.y + control.rect.h / 2 - 15,
        60,
        30
    };

    bool isOn = control.value > 0.5f;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (isOn) {
        SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 200);
    } else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 70, 200);
    }
    SDL_RenderFillRect(renderer, &toggleBg);

    SDL_SetRenderDrawColor(renderer, 100, 100, 110, 255);
    SDL_RenderDrawRect(renderer, &toggleBg);

    // Toggle handle
    int handleX = isOn ? toggleBg.x + 35 : toggleBg.x + 5;
    SDL_Rect handleRect = {handleX, toggleBg.y + 5, 20, 20};

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &handleRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::RenderDropdown(SDL_Renderer* renderer, const WizardControl& control) {
    // Label
    SDL_Rect labelRect = {control.rect.x, control.rect.y - 25, control.rect.w, 20};
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &labelRect);

    // Dropdown box
    SDL_Rect boxRect = control.rect;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 40, 40, 60, 240);
    SDL_RenderFillRect(renderer, &boxRect);

    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b,
                          control.hovered ? 255 : 180);
    SDL_RenderDrawRect(renderer, &boxRect);

    // Selected option text (placeholder)
    if (control.selectedOption >= 0 && control.selectedOption < static_cast<int>(control.options.size())) {
        SDL_Rect textRect = {
            boxRect.x + 15,
            boxRect.y + boxRect.h / 2 - 10,
            boxRect.w - 50,
            20
        };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &textRect);
    }

    // Dropdown arrow
    SDL_Rect arrowRect = {
        boxRect.x + boxRect.w - 35,
        boxRect.y + boxRect.h / 2 - 8,
        15,
        15
    };
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &arrowRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::RenderWelcomePage(SDL_Renderer* renderer) {
    SDL_Rect welcomeBox = {
        screenWidth_ / 2 - 300,
        300,
        600,
        200
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 50, 50, 70, 200);
    SDL_RenderFillRect(renderer, &welcomeBox);

    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    for (int i = 0; i < 2; i++) {
        SDL_RenderDrawRect(renderer, &welcomeBox);
        welcomeBox.x++; welcomeBox.y++;
        welcomeBox.w -= 2; welcomeBox.h -= 2;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::RenderCompletePage(SDL_Renderer* renderer) {
    SDL_Rect completeBox = {
        screenWidth_ / 2 - 250,
        300,
        500,
        150
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 76, 175, 80, 200);
    SDL_RenderFillRect(renderer, &completeBox);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawRect(renderer, &completeBox);
        completeBox.x++; completeBox.y++;
        completeBox.w -= 2; completeBox.h -= 2;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::RenderProgressBar(SDL_Renderer* renderer) {
    int totalPages = 8;
    int currentPageNum = GetPageIndex(currentPage_);

    SDL_Rect progressBg = {
        50,
        screenHeight_ - 100,
        screenWidth_ - 100,
        10
    };

    SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
    SDL_RenderFillRect(renderer, &progressBg);

    float progress = static_cast<float>(currentPageNum) / totalPages;
    int fillWidth = static_cast<int>(progressBg.w * progress);

    SDL_Rect progressFill = {progressBg.x, progressBg.y, fillWidth, progressBg.h};
    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 255);
    SDL_RenderFillRect(renderer, &progressFill);
}

void FirstTimeSetup::RenderNavigationButtons(SDL_Renderer* renderer) {
    // Back button
    if (currentPage_ != SetupPage::Welcome) {
        SDL_Rect backButton = {50, screenHeight_ - 70, 150, 50};

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 220);
        SDL_RenderFillRect(renderer, &backButton);

        SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 255);
        SDL_RenderDrawRect(renderer, &backButton);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    // Next/Finish button
    SDL_Rect nextButton = {screenWidth_ - 200, screenHeight_ - 70, 150, 50};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (currentPage_ == SetupPage::Privacy) {
        SDL_SetRenderDrawColor(renderer, 76, 175, 80, 220);
    } else {
        SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 220);
    }
    SDL_RenderFillRect(renderer, &nextButton);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &nextButton);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::RenderPageTransition(SDL_Renderer* renderer) {
    float alpha = std::sin(transitionProgress_ * M_PI);
    Uint8 overlayAlpha = static_cast<Uint8>(alpha * 100);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, overlayAlpha);

    SDL_Rect fullScreen = {0, 0, screenWidth_, screenHeight_};
    SDL_RenderFillRect(renderer, &fullScreen);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void FirstTimeSetup::UpdateAnimations(float deltaTime) {
    pageAnimTimer_ += deltaTime * 2.0f;
}

void FirstTimeSetup::UpdateControls(float deltaTime) {
    // Smooth hover transitions for controls
    for (auto& control : currentControls_) {
        // Update hover state smoothly
    }
}

void FirstTimeSetup::NextPage() {
    SetupPage nextPage;

    switch (currentPage_) {
        case SetupPage::Welcome:
            nextPage = SetupPage::Language;
            break;
        case SetupPage::Language:
            nextPage = SetupPage::Graphics;
            break;
        case SetupPage::Graphics:
            nextPage = SetupPage::Audio;
            break;
        case SetupPage::Audio:
            nextPage = SetupPage::Controls;
            break;
        case SetupPage::Controls:
            nextPage = SetupPage::Accessibility;
            break;
        case SetupPage::Accessibility:
            nextPage = SetupPage::Privacy;
            break;
        case SetupPage::Privacy:
            CompleteSetup();
            return;
        default:
            return;
    }

    GoToPage(nextPage);
}

void FirstTimeSetup::PreviousPage() {
    SetupPage prevPage;

    switch (currentPage_) {
        case SetupPage::Language:
            prevPage = SetupPage::Welcome;
            break;
        case SetupPage::Graphics:
            prevPage = SetupPage::Language;
            break;
        case SetupPage::Audio:
            prevPage = SetupPage::Graphics;
            break;
        case SetupPage::Controls:
            prevPage = SetupPage::Audio;
            break;
        case SetupPage::Accessibility:
            prevPage = SetupPage::Controls;
            break;
        case SetupPage::Privacy:
            prevPage = SetupPage::Accessibility;
            break;
        default:
            return;
    }

    GoToPage(prevPage);
}

void FirstTimeSetup::CompleteSetup() {
    SaveConfiguration();
    currentPage_ = SetupPage::Complete;

    SDL_Log("Setup complete!");

    if (onComplete_) {
        onComplete_();
    }
}

void FirstTimeSetup::HandleKeyboardInput(const SDL_Event& event) {
    if (event.type != SDL_KEYDOWN) return;

    switch (event.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            SelectPreviousControl();
            break;

        case SDLK_DOWN:
        case SDLK_s:
            SelectNextControl();
            break;

        case SDLK_LEFT:
        case SDLK_a:
            if (selectedControlIndex_ >= 0 && selectedControlIndex_ < static_cast<int>(currentControls_.size())) {
                UpdateControlValue(currentControls_[selectedControlIndex_], -0.1f);
            }
            break;

        case SDLK_RIGHT:
        case SDLK_d:
            if (selectedControlIndex_ >= 0 && selectedControlIndex_ < static_cast<int>(currentControls_.size())) {
                UpdateControlValue(currentControls_[selectedControlIndex_], 0.1f);
            }
            break;

        case SDLK_RETURN:
        case SDLK_SPACE:
            if (selectedControlIndex_ >= 0 && selectedControlIndex_ < static_cast<int>(currentControls_.size())) {
                ActivateControl(currentControls_[selectedControlIndex_]);
            } else {
                NextPage();
            }
            break;

        case SDLK_ESCAPE:
            PreviousPage();
            break;
    }
}

void FirstTimeSetup::HandleGamepadInput(const SDL_Event& event) {
    if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        switch (event.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                SelectPreviousControl();
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                SelectNextControl();
                break;

            case SDL_CONTROLLER_BUTTON_A:
                NextPage();
                break;

            case SDL_CONTROLLER_BUTTON_B:
                PreviousPage();
                break;
        }
    }
}

void FirstTimeSetup::HandleMouseInput(const SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION) {
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;

        for (auto& control : currentControls_) {
            control.hovered = (mouseX >= control.rect.x &&
                             mouseX < control.rect.x + control.rect.w &&
                             mouseY >= control.rect.y &&
                             mouseY < control.rect.y + control.rect.h);
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;

        for (auto& control : currentControls_) {
            if (mouseX >= control.rect.x && mouseX < control.rect.x + control.rect.w &&
                mouseY >= control.rect.y && mouseY < control.rect.y + control.rect.h) {
                ActivateControl(control);
                break;
            }
        }

        // Check navigation buttons
        SDL_Rect nextButton = {screenWidth_ - 200, screenHeight_ - 70, 150, 50};
        SDL_Rect backButton = {50, screenHeight_ - 70, 150, 50};

        if (mouseX >= nextButton.x && mouseX < nextButton.x + nextButton.w &&
            mouseY >= nextButton.y && mouseY < nextButton.y + nextButton.h) {
            NextPage();
        } else if (mouseX >= backButton.x && mouseX < backButton.x + backButton.w &&
                   mouseY >= backButton.y && mouseY < backButton.y + backButton.h) {
            PreviousPage();
        }
    }
}

void FirstTimeSetup::SelectNextControl() {
    if (currentControls_.empty()) return;

    selectedControlIndex_ = (selectedControlIndex_ + 1) % static_cast<int>(currentControls_.size());
}

void FirstTimeSetup::SelectPreviousControl() {
    if (currentControls_.empty()) return;

    selectedControlIndex_--;
    if (selectedControlIndex_ < 0) {
        selectedControlIndex_ = static_cast<int>(currentControls_.size()) - 1;
    }
}

void FirstTimeSetup::ActivateControl(WizardControl& control) {
    if (control.type == ControlType::Toggle) {
        control.value = (control.value > 0.5f) ? 0.0f : 1.0f;
    } else if (control.type == ControlType::Dropdown) {
        control.selectedOption = (control.selectedOption + 1) % static_cast<int>(control.options.size());
    }

    if (control.onChange) {
        control.onChange();
    }
}

void FirstTimeSetup::UpdateControlValue(WizardControl& control, float delta) {
    if (control.type == ControlType::Slider) {
        control.value = std::clamp(control.value + delta, 0.0f, 1.0f);

        if (control.onChange) {
            control.onChange();
        }
    }
}

void FirstTimeSetup::ApplyGraphicsPreset(GraphicsQuality quality) {
    config_.graphicsQuality = quality;

    switch (quality) {
        case GraphicsQuality::Low:
            config_.screenWidth = 1280;
            config_.screenHeight = 720;
            break;
        case GraphicsQuality::Medium:
            config_.screenWidth = 1920;
            config_.screenHeight = 1080;
            break;
        case GraphicsQuality::High:
            config_.screenWidth = 1920;
            config_.screenHeight = 1080;
            break;
        case GraphicsQuality::Ultra:
            config_.screenWidth = 2560;
            config_.screenHeight = 1440;
            break;
        default:
            break;
    }
}

void FirstTimeSetup::ApplyControlPreset(ControlScheme scheme) {
    config_.controlScheme = scheme;
}

void FirstTimeSetup::TestAudioSettings() {
    SDL_Log("Testing audio settings...");
}

void FirstTimeSetup::DetectOptimalSettings() {
    SDL_Log("Detecting optimal settings...");
}

void FirstTimeSetup::SaveConfiguration() {
    SDL_Log("Saving configuration...");
    // In a real implementation, save to file
}

int FirstTimeSetup::GetPageIndex(SetupPage page) const {
    return static_cast<int>(page);
}

std::string FirstTimeSetup::GetPageTitle(SetupPage page) const {
    switch (page) {
        case SetupPage::Welcome: return "Welcome";
        case SetupPage::Language: return "Language Selection";
        case SetupPage::Graphics: return "Graphics Settings";
        case SetupPage::Audio: return "Audio Settings";
        case SetupPage::Controls: return "Controls";
        case SetupPage::Accessibility: return "Accessibility";
        case SetupPage::Privacy: return "Privacy";
        case SetupPage::Complete: return "Setup Complete";
        default: return "";
    }
}

std::string FirstTimeSetup::GetPageDescription(SetupPage page) const {
    switch (page) {
        case SetupPage::Welcome:
            return "Welcome to the game! Let's configure your settings.";
        case SetupPage::Language:
            return "Select your preferred language.";
        case SetupPage::Graphics:
            return "Configure graphics settings for optimal performance.";
        case SetupPage::Audio:
            return "Adjust audio levels to your preference.";
        case SetupPage::Controls:
            return "Set up your control scheme.";
        case SetupPage::Accessibility:
            return "Configure accessibility options.";
        case SetupPage::Privacy:
            return "Choose your privacy preferences.";
        case SetupPage::Complete:
            return "Setup complete! Ready to start your adventure.";
        default:
            return "";
    }
}

} // namespace Launcher
