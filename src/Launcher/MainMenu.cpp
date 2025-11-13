#include "MainMenu.h"
#include <cmath>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace Launcher {

MainMenu::MainMenu()
    : screenWidth_(1280),
      screenHeight_(720),
      renderer_(nullptr),
      selectedButtonIndex_(0),
      gamepadNavigationActive_(false),
      animationsEnabled_(true),
      particlesEnabled_(true),
      titlePulseTimer_(0.0f),
      backgroundScrollOffset_(0.0f),
      buttonAnimTimer_(0.0f),
      hasSaveGame_(false),
      isGamepadConnected_(false),
      screenShakeIntensity_(0.0f),
      screenShakeTimer_(0.0f),
      titleColor_({255, 215, 0, 255}),
      accentColor_({75, 144, 226, 255}),
      globalAlpha_(1.0f) {

    fadeAnimation_ = {0.0f, 0.5f, false, 0, 255, 0};
    slideAnimation_ = {0.0f, 0.4f, false, 0, 0, 0};
}

MainMenu::~MainMenu() {
}

bool MainMenu::Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    renderer_ = renderer;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;

    // Check for save games
    CheckSaveGames();

    // Create menu buttons
    CreateButtons();

    // Load assets
    LoadMenuAssets();

    // Check for gamepad
    isGamepadConnected_ = SDL_NumJoysticks() > 0;

    // Start fade-in animation
    fadeAnimation_.active = true;
    fadeAnimation_.time = 0.0f;

    SDL_Log("MainMenu initialized");
    return true;
}

void MainMenu::Update(float deltaTime) {
    // Update animations
    if (animationsEnabled_) {
        UpdateAnimations(deltaTime);
    }

    // Update buttons
    UpdateButtons(deltaTime);

    // Update background
    UpdateBackground(deltaTime);

    // Update screen shake
    if (screenShakeIntensity_ > 0.0f) {
        screenShakeTimer_ += deltaTime;
        screenShakeIntensity_ = std::max(0.0f, screenShakeIntensity_ - deltaTime * 5.0f);
    }
}

void MainMenu::Render(SDL_Renderer* renderer) {
    // Apply screen shake
    int shakeX = 0, shakeY = 0;
    if (screenShakeIntensity_ > 0.0f) {
        shakeX = static_cast<int>(std::sin(screenShakeTimer_ * 20.0f) * screenShakeIntensity_ * 10.0f);
        shakeY = static_cast<int>(std::cos(screenShakeTimer_ * 15.0f) * screenShakeIntensity_ * 10.0f);
    }

    // Render background
    RenderBackground(renderer);

    // Render title
    RenderTitle(renderer);

    // Render buttons
    RenderButtons(renderer);

    // Render version info
    RenderVersionInfo(renderer);

    // Render gamepad hints if gamepad is active
    if (gamepadNavigationActive_ && isGamepadConnected_) {
        RenderGamepadHints(renderer);
    }

    // Apply fade effect if active
    if (fadeAnimation_.active) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        Uint8 alpha = static_cast<Uint8>(255 - fadeAnimation_.currentValue);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
        SDL_Rect fullScreen = {0, 0, screenWidth_, screenHeight_};
        SDL_RenderFillRect(renderer, &fullScreen);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void MainMenu::HandleInput(const SDL_Event& event) {
    // Detect input type
    if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERAXISMOTION) {
        gamepadNavigationActive_ = true;
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEMOTION) {
        gamepadNavigationActive_ = false;
    }

    // Delegate to specific handlers
    HandleKeyboardInput(event);
    HandleGamepadInput(event);
    HandleMouseInput(event);
}

// Private methods

void MainMenu::CreateButtons() {
    buttons_.clear();

    int buttonWidth = 350;
    int buttonHeight = 60;
    int spacing = 20;
    int startY = screenHeight_ / 2 - 50;
    int centerX = screenWidth_ / 2 - buttonWidth / 2;

    // New Game button
    MenuButton newGameBtn;
    newGameBtn.id = "new_game";
    newGameBtn.label = "New Game";
    newGameBtn.iconPath = "assets/ui/icons/new_game.png";
    newGameBtn.rect = {centerX, startY, buttonWidth, buttonHeight};
    newGameBtn.enabled = true;
    newGameBtn.hovered = false;
    newGameBtn.scale = 1.0f;
    newGameBtn.glowIntensity = 0.0f;
    newGameBtn.onClick = [this]() {
        if (onNewGame_) onNewGame_("Warrior");
    };
    buttons_.push_back(newGameBtn);

    // Continue button
    MenuButton continueBtn;
    continueBtn.id = "continue";
    continueBtn.label = "Continue";
    continueBtn.iconPath = "assets/ui/icons/continue.png";
    continueBtn.rect = {centerX, startY + (buttonHeight + spacing), buttonWidth, buttonHeight};
    continueBtn.enabled = hasSaveGame_;
    continueBtn.hovered = false;
    continueBtn.scale = 1.0f;
    continueBtn.glowIntensity = 0.0f;
    continueBtn.onClick = [this]() {
        if (onContinue_) onContinue_();
    };
    buttons_.push_back(continueBtn);

    // Load Game button
    MenuButton loadBtn;
    loadBtn.id = "load_game";
    loadBtn.label = "Load Game";
    loadBtn.iconPath = "assets/ui/icons/load.png";
    loadBtn.rect = {centerX, startY + (buttonHeight + spacing) * 2, buttonWidth, buttonHeight};
    loadBtn.enabled = hasSaveGame_;
    loadBtn.hovered = false;
    loadBtn.scale = 1.0f;
    loadBtn.glowIntensity = 0.0f;
    loadBtn.onClick = [this]() {
        if (onContinue_) onContinue_(); // Reuse continue for now
    };
    buttons_.push_back(loadBtn);

    // Settings button
    MenuButton settingsBtn;
    settingsBtn.id = "settings";
    settingsBtn.label = "Settings";
    settingsBtn.iconPath = "assets/ui/icons/settings.png";
    settingsBtn.rect = {centerX, startY + (buttonHeight + spacing) * 3, buttonWidth, buttonHeight};
    settingsBtn.enabled = true;
    settingsBtn.hovered = false;
    settingsBtn.scale = 1.0f;
    settingsBtn.glowIntensity = 0.0f;
    settingsBtn.onClick = [this]() {
        if (onSettings_) onSettings_();
    };
    buttons_.push_back(settingsBtn);

    // Credits button
    MenuButton creditsBtn;
    creditsBtn.id = "credits";
    creditsBtn.label = "Credits";
    creditsBtn.iconPath = "assets/ui/icons/credits.png";
    creditsBtn.rect = {centerX, startY + (buttonHeight + spacing) * 4, buttonWidth, buttonHeight};
    creditsBtn.enabled = true;
    creditsBtn.hovered = false;
    creditsBtn.scale = 1.0f;
    creditsBtn.glowIntensity = 0.0f;
    creditsBtn.onClick = [this]() {
        if (onCredits_) onCredits_();
    };
    buttons_.push_back(creditsBtn);

    // Exit button
    MenuButton exitBtn;
    exitBtn.id = "exit";
    exitBtn.label = "Exit Game";
    exitBtn.iconPath = "assets/ui/icons/exit.png";
    exitBtn.rect = {centerX, startY + (buttonHeight + spacing) * 5, buttonWidth, buttonHeight};
    exitBtn.enabled = true;
    exitBtn.hovered = false;
    exitBtn.scale = 1.0f;
    exitBtn.glowIntensity = 0.0f;
    exitBtn.onClick = [this]() {
        if (onExit_) onExit_();
    };
    buttons_.push_back(exitBtn);

    // Select first enabled button
    for (size_t i = 0; i < buttons_.size(); ++i) {
        if (buttons_[i].enabled) {
            selectedButtonIndex_ = static_cast<int>(i);
            break;
        }
    }
}

void MainMenu::LoadMenuAssets() {
    // In a real implementation, load textures and fonts here
    SDL_Log("Loading menu assets...");
}

void MainMenu::CheckSaveGames() {
    // Check if save games exist
    std::string savePath = "saves";

    if (fs::exists(savePath)) {
        try {
            for (const auto& entry : fs::directory_iterator(savePath)) {
                if (entry.is_regular_file()) {
                    hasSaveGame_ = true;
                    SDL_Log("Save game detected: %s", entry.path().filename().string().c_str());
                    break;
                }
            }
        } catch (const std::exception& e) {
            SDL_Log("Error checking save games: %s", e.what());
        }
    }

    if (!hasSaveGame_) {
        SDL_Log("No save games found");
    }
}

void MainMenu::RenderBackground(SDL_Renderer* renderer) {
    // Render animated gradient background
    for (int y = 0; y < screenHeight_; y++) {
        float gradient = static_cast<float>(y) / screenHeight_;
        float wave = std::sin(backgroundScrollOffset_ + gradient * 2.0f) * 0.1f;

        Uint8 r = static_cast<Uint8>(10 + wave * 50);
        Uint8 g = static_cast<Uint8>(10 + wave * 30);
        Uint8 b = static_cast<Uint8>(25 + gradient * 30 + wave * 40);

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, screenWidth_, y);
    }

    // Render geometric patterns
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 30);

    for (int i = 0; i < 5; i++) {
        float offset = backgroundScrollOffset_ * 0.5f + i * M_PI * 0.4f;
        int x = static_cast<int>(screenWidth_ / 2 + std::sin(offset) * 300);
        int y = static_cast<int>(screenHeight_ / 2 + std::cos(offset) * 200);

        SDL_Rect pattern = {x - 50, y - 50, 100, 100};
        SDL_RenderDrawRect(renderer, &pattern);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void MainMenu::RenderTitle(SDL_Renderer* renderer) {
    // Render animated game title
    float pulse = 1.0f + std::sin(titlePulseTimer_) * 0.05f;

    int titleWidth = static_cast<int>(600 * pulse);
    int titleHeight = static_cast<int>(120 * pulse);

    SDL_Rect titleRect = {
        screenWidth_ / 2 - titleWidth / 2,
        80,
        titleWidth,
        titleHeight
    };

    // Draw glow effect
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < 3; i++) {
        SDL_Rect glowRect = {
            titleRect.x - i * 4,
            titleRect.y - i * 4,
            titleRect.w + i * 8,
            titleRect.h + i * 8
        };
        Uint8 glowAlpha = static_cast<Uint8>(100 / (i + 1));
        SDL_SetRenderDrawColor(renderer, titleColor_.r, titleColor_.g, titleColor_.b, glowAlpha);
        SDL_RenderDrawRect(renderer, &glowRect);
    }

    // Draw main title box
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 200);
    SDL_RenderFillRect(renderer, &titleRect);

    SDL_SetRenderDrawColor(renderer, titleColor_.r, titleColor_.g, titleColor_.b, 255);
    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawRect(renderer, &titleRect);
        titleRect.x++; titleRect.y++;
        titleRect.w -= 2; titleRect.h -= 2;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Render subtitle
    SDL_Rect subtitleRect = {
        screenWidth_ / 2 - 200,
        220,
        400,
        30
    };
    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 200);
    SDL_RenderDrawRect(renderer, &subtitleRect);
}

void MainMenu::RenderButtons(SDL_Renderer* renderer) {
    for (size_t i = 0; i < buttons_.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == selectedButtonIndex_);

        if (isSelected && gamepadNavigationActive_) {
            buttons_[i].hovered = true;
        }

        RenderButton(renderer, buttons_[i]);
    }
}

void MainMenu::RenderButton(SDL_Renderer* renderer, const MenuButton& button) {
    SDL_Rect rect = button.rect;

    // Apply scale
    if (button.scale != 1.0f) {
        int widthDiff = static_cast<int>(rect.w * (button.scale - 1.0f));
        int heightDiff = static_cast<int>(rect.h * (button.scale - 1.0f));
        rect.x -= widthDiff / 2;
        rect.y -= heightDiff / 2;
        rect.w += widthDiff;
        rect.h += heightDiff;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Render glow if hovered
    if (button.hovered && button.enabled) {
        for (int i = 0; i < 5; i++) {
            SDL_Rect glowRect = {
                rect.x - i * 2,
                rect.y - i * 2,
                rect.w + i * 4,
                rect.h + i * 4
            };
            Uint8 alpha = static_cast<Uint8>(button.glowIntensity * 50 / (i + 1));
            SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, alpha);
            SDL_RenderDrawRect(renderer, &glowRect);
        }
    }

    // Render button background
    if (button.enabled) {
        if (button.hovered) {
            SDL_SetRenderDrawColor(renderer, 60, 60, 80, 240);
        } else {
            SDL_SetRenderDrawColor(renderer, 30, 30, 50, 220);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 150);
    }
    SDL_RenderFillRect(renderer, &rect);

    // Render button border
    if (button.enabled) {
        SDL_SetRenderDrawColor(renderer,
            button.hovered ? titleColor_.r : accentColor_.r,
            button.hovered ? titleColor_.g : accentColor_.g,
            button.hovered ? titleColor_.b : accentColor_.b,
            255);

        for (int i = 0; i < (button.hovered ? 3 : 2); i++) {
            SDL_RenderDrawRect(renderer, &rect);
            rect.x++; rect.y++;
            rect.w -= 2; rect.h -= 2;
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }

    // Render button label (placeholder - would use SDL_ttf in real implementation)
    SDL_Rect labelRect = {
        button.rect.x + 20,
        button.rect.y + button.rect.h / 2 - 10,
        button.rect.w - 40,
        20
    };

    if (button.enabled) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    }
    SDL_RenderDrawRect(renderer, &labelRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void MainMenu::RenderVersionInfo(SDL_Renderer* renderer) {
    SDL_Rect versionRect = {
        20,
        screenHeight_ - 40,
        200,
        30
    };

    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 150);
    SDL_RenderDrawRect(renderer, &versionRect);
}

void MainMenu::RenderGamepadHints(SDL_Renderer* renderer) {
    SDL_Rect hintRect = {
        screenWidth_ - 320,
        screenHeight_ - 120,
        300,
        100
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 180);
    SDL_RenderFillRect(renderer, &hintRect);

    SDL_SetRenderDrawColor(renderer, accentColor_.r, accentColor_.g, accentColor_.b, 200);
    SDL_RenderDrawRect(renderer, &hintRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void MainMenu::UpdateAnimations(float deltaTime) {
    // Update title pulse
    titlePulseTimer_ += deltaTime * 2.0f;

    // Update button animation timer
    buttonAnimTimer_ += deltaTime;

    // Update fade animation
    if (fadeAnimation_.active) {
        fadeAnimation_.time += deltaTime;
        float progress = std::min(1.0f, fadeAnimation_.time / fadeAnimation_.duration);
        fadeAnimation_.currentValue = static_cast<int>(
            fadeAnimation_.startValue +
            (fadeAnimation_.endValue - fadeAnimation_.startValue) * progress
        );

        if (progress >= 1.0f) {
            fadeAnimation_.active = false;
        }
    }

    // Update slide animation
    if (slideAnimation_.active) {
        slideAnimation_.time += deltaTime;
        float progress = std::min(1.0f, slideAnimation_.time / slideAnimation_.duration);

        // Ease out cubic
        progress = 1.0f - std::pow(1.0f - progress, 3.0f);

        slideAnimation_.currentValue = static_cast<int>(
            slideAnimation_.startValue +
            (slideAnimation_.endValue - slideAnimation_.startValue) * progress
        );

        if (progress >= 1.0f) {
            slideAnimation_.active = false;
        }
    }
}

void MainMenu::UpdateButtons(float deltaTime) {
    for (auto& button : buttons_) {
        if (!button.enabled) continue;

        // Smooth scale transitions
        float targetScale = button.hovered ? 1.05f : 1.0f;
        button.scale += (targetScale - button.scale) * deltaTime * 10.0f;

        // Smooth glow transitions
        float targetGlow = button.hovered ? 1.0f : 0.0f;
        button.glowIntensity += (targetGlow - button.glowIntensity) * deltaTime * 8.0f;
    }
}

void MainMenu::UpdateBackground(float deltaTime) {
    backgroundScrollOffset_ += deltaTime * 0.5f;
}

void MainMenu::HandleKeyboardInput(const SDL_Event& event) {
    if (event.type != SDL_KEYDOWN) return;

    switch (event.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            SelectPreviousButton();
            break;

        case SDLK_DOWN:
        case SDLK_s:
            SelectNextButton();
            break;

        case SDLK_RETURN:
        case SDLK_SPACE:
            ActivateSelectedButton();
            break;

        case SDLK_ESCAPE:
            // Go back or exit
            if (buttons_.back().enabled && buttons_.back().onClick) {
                buttons_.back().onClick();
            }
            break;
    }
}

void MainMenu::HandleGamepadInput(const SDL_Event& event) {
    if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        switch (event.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                SelectPreviousButton();
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                SelectNextButton();
                break;

            case SDL_CONTROLLER_BUTTON_A:
                ActivateSelectedButton();
                break;

            case SDL_CONTROLLER_BUTTON_B:
                // Go back
                if (buttons_.back().enabled && buttons_.back().onClick) {
                    buttons_.back().onClick();
                }
                break;
        }
    } else if (event.type == SDL_CONTROLLERAXISMOTION) {
        // Handle analog stick
        if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
            static bool wasNeutral = true;
            if (event.caxis.value < -16384 && wasNeutral) {
                SelectPreviousButton();
                wasNeutral = false;
            } else if (event.caxis.value > 16384 && wasNeutral) {
                SelectNextButton();
                wasNeutral = false;
            } else if (abs(event.caxis.value) < 8192) {
                wasNeutral = true;
            }
        }
    }
}

void MainMenu::HandleMouseInput(const SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION) {
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;
        SelectButtonAtPosition(mouseX, mouseY);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            int mouseX = event.button.x;
            int mouseY = event.button.y;

            for (size_t i = 0; i < buttons_.size(); ++i) {
                const auto& button = buttons_[i];
                if (!button.enabled) continue;

                SDL_Rect rect = button.rect;
                if (mouseX >= rect.x && mouseX < rect.x + rect.w &&
                    mouseY >= rect.y && mouseY < rect.y + rect.h) {
                    PlayButtonClickEffect(static_cast<int>(i));
                    if (button.onClick) {
                        button.onClick();
                    }
                    break;
                }
            }
        }
    }
}

void MainMenu::SelectNextButton() {
    int startIndex = selectedButtonIndex_;
    do {
        selectedButtonIndex_ = (selectedButtonIndex_ + 1) % static_cast<int>(buttons_.size());
        if (buttons_[selectedButtonIndex_].enabled) {
            PlayButtonHoverEffect(selectedButtonIndex_);
            break;
        }
    } while (selectedButtonIndex_ != startIndex);
}

void MainMenu::SelectPreviousButton() {
    int startIndex = selectedButtonIndex_;
    do {
        selectedButtonIndex_--;
        if (selectedButtonIndex_ < 0) {
            selectedButtonIndex_ = static_cast<int>(buttons_.size()) - 1;
        }
        if (buttons_[selectedButtonIndex_].enabled) {
            PlayButtonHoverEffect(selectedButtonIndex_);
            break;
        }
    } while (selectedButtonIndex_ != startIndex);
}

void MainMenu::ActivateSelectedButton() {
    if (selectedButtonIndex_ >= 0 && selectedButtonIndex_ < static_cast<int>(buttons_.size())) {
        const auto& button = buttons_[selectedButtonIndex_];
        if (button.enabled && button.onClick) {
            PlayButtonClickEffect(selectedButtonIndex_);
            button.onClick();
        }
    }
}

void MainMenu::SelectButtonAtPosition(int x, int y) {
    for (size_t i = 0; i < buttons_.size(); ++i) {
        auto& button = buttons_[i];
        if (!button.enabled) {
            button.hovered = false;
            continue;
        }

        SDL_Rect rect = button.rect;
        bool nowHovered = (x >= rect.x && x < rect.x + rect.w &&
                          y >= rect.y && y < rect.y + rect.h);

        if (nowHovered && !button.hovered) {
            selectedButtonIndex_ = static_cast<int>(i);
            PlayButtonHoverEffect(static_cast<int>(i));
        }

        button.hovered = nowHovered;
    }
}

void MainMenu::PlayButtonHoverEffect(int buttonIndex) {
    if (buttonIndex < 0 || buttonIndex >= static_cast<int>(buttons_.size())) return;

    // In a real implementation, play hover sound
    SDL_Log("Button hover: %s", buttons_[buttonIndex].label.c_str());
}

void MainMenu::PlayButtonClickEffect(int buttonIndex) {
    if (buttonIndex < 0 || buttonIndex >= static_cast<int>(buttons_.size())) return;

    // In a real implementation, play click sound
    ShakeScreen(2.0f);
    SDL_Log("Button clicked: %s", buttons_[buttonIndex].label.c_str());
}

void MainMenu::ShakeScreen(float intensity) {
    screenShakeIntensity_ = intensity;
    screenShakeTimer_ = 0.0f;
}

} // namespace Launcher
