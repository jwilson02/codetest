#include "UISystem.h"
#include "MainMenu.h"
#include "HUD.h"
#include "CharacterSheet.h"
#include "InventoryUI.h"
#include "DialogueUI.h"
#include "QuestTracker.h"
#include "Tooltip.h"
#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>

namespace UI {

// UITheme implementation
bool UITheme::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open theme file: " << filepath << std::endl;
        return false;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;

    if (!Json::parseFromStream(builder, file, &root, &errs)) {
        std::cerr << "Failed to parse theme JSON: " << errs << std::endl;
        return false;
    }

    // Parse colors
    if (root.isMember("colors")) {
        const Json::Value& colors = root["colors"];
        primaryColor = Color::FromHex(colors.get("primary", "#4A90E2").asString());
        secondaryColor = Color::FromHex(colors.get("secondary", "#7B68EE").asString());
        accentColor = Color::FromHex(colors.get("accent", "#FFD700").asString());
        backgroundColor = Color::FromHex(colors.get("background", "#1E1E1E").asString());
        textColor = Color::FromHex(colors.get("text", "#FFFFFF").asString());
        textHighlightColor = Color::FromHex(colors.get("textHighlight", "#FFD700").asString());
        successColor = Color::FromHex(colors.get("success", "#4CAF50").asString());
        warningColor = Color::FromHex(colors.get("warning", "#FF9800").asString());
        errorColor = Color::FromHex(colors.get("error", "#F44336").asString());
        disabledColor = Color::FromHex(colors.get("disabled", "#757575").asString());
    }

    // Parse fonts
    if (root.isMember("fonts")) {
        const Json::Value& fonts = root["fonts"];
        mainFont = fonts.get("main", "assets/fonts/Roboto-Regular.ttf").asString();
        titleFont = fonts.get("title", "assets/fonts/Roboto-Bold.ttf").asString();
        iconFont = fonts.get("icon", "assets/fonts/FontAwesome.ttf").asString();
    }

    // Parse font sizes
    if (root.isMember("fontSizes")) {
        const Json::Value& sizes = root["fontSizes"];
        smallFontSize = sizes.get("small", 12).asInt();
        normalFontSize = sizes.get("normal", 16).asInt();
        largeFontSize = sizes.get("large", 20).asInt();
        titleFontSize = sizes.get("title", 32).asInt();
    }

    // Parse metrics
    if (root.isMember("metrics")) {
        const Json::Value& metrics = root["metrics"];
        buttonHeight = metrics.get("buttonHeight", 40.0).asFloat();
        buttonPadding = metrics.get("buttonPadding", 15.0).asFloat();
        panelPadding = metrics.get("panelPadding", 20.0).asFloat();
        elementSpacing = metrics.get("elementSpacing", 10.0).asFloat();
        borderRadius = metrics.get("borderRadius", 5.0).asFloat();
    }

    // Parse animation settings
    if (root.isMember("animation")) {
        const Json::Value& anim = root["animation"];
        transitionSpeed = anim.get("transitionSpeed", 0.3).asFloat();
        fadeSpeed = anim.get("fadeSpeed", 0.5).asFloat();
        hoverAnimSpeed = anim.get("hoverAnimSpeed", 0.15).asFloat();
    }

    return true;
}

Json::Value UITheme::ToJson() const {
    Json::Value root;

    // Colors
    Json::Value colors;
    colors["primary"] = primaryColor.ToHex();
    colors["secondary"] = secondaryColor.ToHex();
    colors["accent"] = accentColor.ToHex();
    colors["background"] = backgroundColor.ToHex();
    colors["text"] = textColor.ToHex();
    colors["textHighlight"] = textHighlightColor.ToHex();
    colors["success"] = successColor.ToHex();
    colors["warning"] = warningColor.ToHex();
    colors["error"] = errorColor.ToHex();
    colors["disabled"] = disabledColor.ToHex();
    root["colors"] = colors;

    // Fonts
    Json::Value fonts;
    fonts["main"] = mainFont;
    fonts["title"] = titleFont;
    fonts["icon"] = iconFont;
    root["fonts"] = fonts;

    // Font sizes
    Json::Value fontSizes;
    fontSizes["small"] = smallFontSize;
    fontSizes["normal"] = normalFontSize;
    fontSizes["large"] = largeFontSize;
    fontSizes["title"] = titleFontSize;
    root["fontSizes"] = fontSizes;

    // Metrics
    Json::Value metrics;
    metrics["buttonHeight"] = buttonHeight;
    metrics["buttonPadding"] = buttonPadding;
    metrics["panelPadding"] = panelPadding;
    metrics["elementSpacing"] = elementSpacing;
    metrics["borderRadius"] = borderRadius;
    root["metrics"] = metrics;

    // Animation
    Json::Value animation;
    animation["transitionSpeed"] = transitionSpeed;
    animation["fadeSpeed"] = fadeSpeed;
    animation["hoverAnimSpeed"] = hoverAnimSpeed;
    root["animation"] = animation;

    return root;
}

// UISystem implementation
UISystem& UISystem::Instance() {
    static UISystem instance;
    return instance;
}

UISystem::UISystem()
    : renderer_(nullptr), screenWidth_(1920), screenHeight_(1080),
      currentScreen_(ScreenType::None), currentInputMode_(InputType::Mouse),
      focusedElement_(nullptr), paused_(false), cursorVisible_(true),
      debugMode_(false), initialized_(false) {
}

UISystem::~UISystem() {
    Shutdown();
}

bool UISystem::Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    if (initialized_) {
        std::cerr << "UISystem already initialized!" << std::endl;
        return false;
    }

    renderer_ = renderer;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;

    // Load default theme
    if (!LoadTheme("assets/ui/theme.json")) {
        std::cout << "Warning: Could not load theme, using defaults" << std::endl;
        // Set default theme values
        theme_.primaryColor = Color::FromHex("#4A90E2");
        theme_.secondaryColor = Color::FromHex("#7B68EE");
        theme_.accentColor = Color::FromHex("#FFD700");
        theme_.backgroundColor = Color::FromHex("#1E1E1E");
        theme_.textColor = Color::FromHex("#FFFFFF");
        theme_.transitionSpeed = 0.3f;
        theme_.buttonHeight = 40.0f;
        theme_.elementSpacing = 10.0f;
    }

    // Create UI components
    CreateMainMenu();
    CreateHUD();
    CreateCharacterSheet();
    CreateInventory();
    CreateDialogue();
    CreateQuestTracker();
    CreatePauseMenu();
    CreateSettingsMenu();
    CreateCraftingUI();

    // Create tooltip
    tooltip_ = std::make_shared<Tooltip>();

    initialized_ = true;
    std::cout << "UISystem initialized successfully" << std::endl;
    return true;
}

void UISystem::Shutdown() {
    if (!initialized_) return;

    // Clear all UI components
    mainMenu_.reset();
    hud_.reset();
    characterSheet_.reset();
    inventory_.reset();
    dialogue_.reset();
    questTracker_.reset();
    tooltip_.reset();
    pauseMenu_.reset();
    settingsMenu_.reset();
    craftingUI_.reset();

    // Clear registrations
    elements_.clear();
    notifications_.clear();

    // Clear screen stack
    while (!screenStack_.empty()) {
        screenStack_.pop();
    }

    renderer_ = nullptr;
    initialized_ = false;

    std::cout << "UISystem shut down" << std::endl;
}

void UISystem::Update(float deltaTime) {
    if (!initialized_) return;

    // Update current screen
    switch (currentScreen_) {
        case ScreenType::MainMenu:
            if (mainMenu_) mainMenu_->Update(deltaTime);
            break;
        case ScreenType::InGame:
            if (hud_) hud_->Update(deltaTime);
            if (questTracker_ && questTracker_->IsVisible()) {
                questTracker_->Update(deltaTime);
            }
            break;
        case ScreenType::CharacterSheet:
            if (characterSheet_) characterSheet_->Update(deltaTime);
            break;
        case ScreenType::Inventory:
            if (inventory_) inventory_->Update(deltaTime);
            break;
        case ScreenType::Dialogue:
            if (dialogue_) dialogue_->Update(deltaTime);
            break;
        case ScreenType::Pause:
            if (pauseMenu_) pauseMenu_->Update(deltaTime);
            break;
        case ScreenType::Settings:
            if (settingsMenu_) settingsMenu_->Update(deltaTime);
            break;
        case ScreenType::CraftingUI:
            if (craftingUI_) craftingUI_->Update(deltaTime);
            break;
        default:
            break;
    }

    // Update tooltip
    if (tooltip_) {
        tooltip_->Update(deltaTime);
    }

    // Update notifications
    for (auto it = notifications_.begin(); it != notifications_.end();) {
        it->elapsed += deltaTime;
        if (it->element) {
            it->element->Update(deltaTime);
        }

        if (it->elapsed >= it->duration) {
            it = notifications_.erase(it);
        } else {
            ++it;
        }
    }
}

void UISystem::Render() {
    if (!initialized_ || !renderer_) return;

    // Render current screen
    switch (currentScreen_) {
        case ScreenType::MainMenu:
            if (mainMenu_) mainMenu_->Render(renderer_);
            break;
        case ScreenType::InGame:
            if (hud_) hud_->Render(renderer_);
            if (questTracker_ && questTracker_->IsVisible()) {
                questTracker_->Render(renderer_);
            }
            break;
        case ScreenType::CharacterSheet:
            if (characterSheet_) characterSheet_->Render(renderer_);
            break;
        case ScreenType::Inventory:
            if (inventory_) inventory_->Render(renderer_);
            break;
        case ScreenType::Dialogue:
            if (dialogue_) dialogue_->Render(renderer_);
            break;
        case ScreenType::Pause:
            if (pauseMenu_) pauseMenu_->Render(renderer_);
            break;
        case ScreenType::Settings:
            if (settingsMenu_) settingsMenu_->Render(renderer_);
            break;
        case ScreenType::CraftingUI:
            if (craftingUI_) craftingUI_->Render(renderer_);
            break;
        default:
            break;
    }

    // Render notifications
    for (auto& notification : notifications_) {
        if (notification.element) {
            notification.element->Render(renderer_);
        }
    }

    // Render tooltip on top of everything
    if (tooltip_ && tooltip_->IsVisible()) {
        tooltip_->Render(renderer_);
    }

    // Render debug info
    if (debugMode_) {
        RenderDebugInfo();
    }
}

void UISystem::HandleInput(const SDL_Event& event) {
    if (!initialized_) return;

    // Detect input mode changes
    if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN) {
        currentInputMode_ = InputType::Mouse;
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        currentInputMode_ = InputType::Keyboard;
    } else if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERAXISMOTION) {
        currentInputMode_ = InputType::Gamepad;
    }

    // Pass input to current screen
    switch (currentScreen_) {
        case ScreenType::MainMenu:
            if (mainMenu_) mainMenu_->HandleInput(event);
            break;
        case ScreenType::InGame:
            if (hud_) hud_->HandleInput(event);
            if (questTracker_ && questTracker_->IsVisible()) {
                questTracker_->HandleInput(event);
            }
            break;
        case ScreenType::CharacterSheet:
            if (characterSheet_) characterSheet_->HandleInput(event);
            break;
        case ScreenType::Inventory:
            if (inventory_) inventory_->HandleInput(event);
            break;
        case ScreenType::Dialogue:
            if (dialogue_) dialogue_->HandleInput(event);
            break;
        case ScreenType::Pause:
            if (pauseMenu_) pauseMenu_->HandleInput(event);
            break;
        case ScreenType::Settings:
            if (settingsMenu_) settingsMenu_->HandleInput(event);
            break;
        case ScreenType::CraftingUI:
            if (craftingUI_) craftingUI_->HandleInput(event);
            break;
        default:
            break;
    }
}

void UISystem::PushScreen(ScreenType screen) {
    if (currentScreen_ != ScreenType::None) {
        screenStack_.push(currentScreen_);
        DeactivateScreen(currentScreen_);
    }
    currentScreen_ = screen;
    ActivateScreen(screen);
}

void UISystem::PopScreen() {
    if (!screenStack_.empty()) {
        DeactivateScreen(currentScreen_);
        currentScreen_ = screenStack_.top();
        screenStack_.pop();
        ActivateScreen(currentScreen_);
    } else {
        currentScreen_ = ScreenType::None;
    }
}

void UISystem::SetScreen(ScreenType screen) {
    if (currentScreen_ != ScreenType::None) {
        DeactivateScreen(currentScreen_);
    }
    currentScreen_ = screen;
    ActivateScreen(screen);
}

ScreenType UISystem::GetCurrentScreen() const {
    return currentScreen_;
}

void UISystem::ClearScreens() {
    while (!screenStack_.empty()) {
        screenStack_.pop();
    }
    if (currentScreen_ != ScreenType::None) {
        DeactivateScreen(currentScreen_);
    }
    currentScreen_ = ScreenType::None;
}

void UISystem::RegisterElement(const std::string& id, std::shared_ptr<UIElement> element) {
    elements_[id] = element;
}

void UISystem::UnregisterElement(const std::string& id) {
    elements_.erase(id);
}

UIElement* UISystem::GetElement(const std::string& id) {
    auto it = elements_.find(id);
    return (it != elements_.end()) ? it->second.get() : nullptr;
}

void UISystem::SetScreenSize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;

    // Notify all UI elements of screen resize
    // This could be enhanced with a proper event system
}

bool UISystem::LoadTheme(const std::string& filepath) {
    return theme_.LoadFromFile(filepath);
}

void UISystem::ShowTooltip(const std::string& text, float x, float y) {
    if (tooltip_) {
        tooltip_->Show(text, x, y);
    }
}

void UISystem::HideTooltip() {
    if (tooltip_) {
        tooltip_->Hide();
    }
}

void UISystem::ShowNotification(const std::string& text, float duration) {
    Notification notif;
    notif.text = text;
    notif.duration = duration;
    notif.elapsed = 0.0f;

    // Create a simple notification element
    auto element = std::make_shared<UIElement>("notification_" + std::to_string(notifications_.size()));
    element->SetPosition(screenWidth_ / 2.0f - 150, 50 + notifications_.size() * 60);
    element->SetSize(300, 50);
    element->SetBackgroundColor(theme_.primaryColor);
    element->PlayAnimation(AnimationType::FadeIn, 0.3f);

    notif.element = element;
    notifications_.push_back(notif);
}

void UISystem::SetFocusedElement(UIElement* element) {
    if (focusedElement_ != element) {
        if (focusedElement_) {
            focusedElement_->SetState(UIState::Normal);
        }
        focusedElement_ = element;
        if (focusedElement_) {
            focusedElement_->SetState(UIState::Focused);
        }
    }
}

void UISystem::SetCursorVisible(bool visible) {
    cursorVisible_ = visible;
    SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

void UISystem::SetCustomCursor(const std::string& cursorId) {
    // Implementation would load and set a custom cursor
    // For now, this is a placeholder
}

void UISystem::RenderDebugInfo() {
    // Render debug information
    // This would typically show FPS, screen info, input mode, etc.
    // Placeholder for now
}

void UISystem::CreateMainMenu() {
    mainMenu_ = std::make_shared<MainMenu>();
    RegisterElement("mainMenu", mainMenu_);
}

void UISystem::CreateHUD() {
    hud_ = std::make_shared<HUD>();
    RegisterElement("hud", hud_);
}

void UISystem::CreateCharacterSheet() {
    characterSheet_ = std::make_shared<CharacterSheet>();
    RegisterElement("characterSheet", characterSheet_);
}

void UISystem::CreateInventory() {
    inventory_ = std::make_shared<InventoryUI>();
    RegisterElement("inventory", inventory_);
}

void UISystem::CreateDialogue() {
    dialogue_ = std::make_shared<DialogueUI>();
    RegisterElement("dialogue", dialogue_);
}

void UISystem::CreateQuestTracker() {
    questTracker_ = std::make_shared<QuestTracker>();
    RegisterElement("questTracker", questTracker_);
}

void UISystem::CreatePauseMenu() {
    pauseMenu_ = std::make_shared<UIElement>("pauseMenu");
    pauseMenu_->SetSize(400, 500);
    pauseMenu_->SetPosition(screenWidth_ / 2.0f - 200, screenHeight_ / 2.0f - 250);
    pauseMenu_->SetBackgroundColor(theme_.backgroundColor);
    pauseMenu_->SetVisible(false);
    RegisterElement("pauseMenu", pauseMenu_);
}

void UISystem::CreateSettingsMenu() {
    settingsMenu_ = std::make_shared<UIElement>("settingsMenu");
    settingsMenu_->SetSize(600, 700);
    settingsMenu_->SetPosition(screenWidth_ / 2.0f - 300, screenHeight_ / 2.0f - 350);
    settingsMenu_->SetBackgroundColor(theme_.backgroundColor);
    settingsMenu_->SetVisible(false);
    RegisterElement("settingsMenu", settingsMenu_);
}

void UISystem::CreateCraftingUI() {
    craftingUI_ = std::make_shared<UIElement>("craftingUI");
    craftingUI_->SetSize(800, 600);
    craftingUI_->SetPosition(screenWidth_ / 2.0f - 400, screenHeight_ / 2.0f - 300);
    craftingUI_->SetBackgroundColor(theme_.backgroundColor);
    craftingUI_->SetVisible(false);
    RegisterElement("craftingUI", craftingUI_);
}

void UISystem::ActivateScreen(ScreenType screen) {
    switch (screen) {
        case ScreenType::MainMenu:
            if (mainMenu_) mainMenu_->SetVisible(true);
            break;
        case ScreenType::InGame:
            if (hud_) hud_->SetVisible(true);
            break;
        case ScreenType::CharacterSheet:
            if (characterSheet_) characterSheet_->SetVisible(true);
            break;
        case ScreenType::Inventory:
            if (inventory_) inventory_->SetVisible(true);
            break;
        case ScreenType::Dialogue:
            if (dialogue_) dialogue_->SetVisible(true);
            break;
        case ScreenType::Pause:
            if (pauseMenu_) pauseMenu_->SetVisible(true);
            break;
        case ScreenType::Settings:
            if (settingsMenu_) settingsMenu_->SetVisible(true);
            break;
        case ScreenType::CraftingUI:
            if (craftingUI_) craftingUI_->SetVisible(true);
            break;
        default:
            break;
    }
}

void UISystem::DeactivateScreen(ScreenType screen) {
    switch (screen) {
        case ScreenType::MainMenu:
            if (mainMenu_) mainMenu_->SetVisible(false);
            break;
        case ScreenType::InGame:
            if (hud_) hud_->SetVisible(false);
            break;
        case ScreenType::CharacterSheet:
            if (characterSheet_) characterSheet_->SetVisible(false);
            break;
        case ScreenType::Inventory:
            if (inventory_) inventory_->SetVisible(false);
            break;
        case ScreenType::Dialogue:
            if (dialogue_) dialogue_->SetVisible(false);
            break;
        case ScreenType::Pause:
            if (pauseMenu_) pauseMenu_->SetVisible(false);
            break;
        case ScreenType::Settings:
            if (settingsMenu_) settingsMenu_->SetVisible(false);
            break;
        case ScreenType::CraftingUI:
            if (craftingUI_) craftingUI_->SetVisible(false);
            break;
        default:
            break;
    }
}

} // namespace UI
