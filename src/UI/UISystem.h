#pragma once

#include "UIElement.h"
#include <string>
#include <unordered_map>
#include <stack>
#include <json/json.h>

struct SDL_Renderer;
struct SDL_Event;

namespace UI {

// Forward declarations
class MainMenu;
class HUD;
class CharacterSheet;
class InventoryUI;
class DialogueUI;
class QuestTracker;
class Tooltip;

// UI Screen types
enum class ScreenType {
    None,
    MainMenu,
    InGame,
    CharacterSheet,
    Inventory,
    Dialogue,
    Pause,
    Settings,
    CraftingUI,
    QuestLog
};

// Theme configuration
struct UITheme {
    // Colors
    Color primaryColor;
    Color secondaryColor;
    Color accentColor;
    Color backgroundColor;
    Color textColor;
    Color textHighlightColor;
    Color successColor;
    Color warningColor;
    Color errorColor;
    Color disabledColor;

    // Fonts (font paths)
    std::string mainFont;
    std::string titleFont;
    std::string iconFont;

    // Font sizes
    int smallFontSize;
    int normalFontSize;
    int largeFontSize;
    int titleFontSize;

    // UI metrics
    float buttonHeight;
    float buttonPadding;
    float panelPadding;
    float elementSpacing;
    float borderRadius;

    // Animation settings
    float transitionSpeed;
    float fadeSpeed;
    float hoverAnimSpeed;

    // Load from JSON file
    bool LoadFromFile(const std::string& filepath);
    Json::Value ToJson() const;
};

// Central UI management system
class UISystem {
public:
    static UISystem& Instance();

    // Initialization
    bool Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight);
    void Shutdown();

    // Update and render
    void Update(float deltaTime);
    void Render();
    void HandleInput(const SDL_Event& event);

    // Screen management
    void PushScreen(ScreenType screen);
    void PopScreen();
    void SetScreen(ScreenType screen);
    ScreenType GetCurrentScreen() const;
    void ClearScreens();

    // UI Element registration
    void RegisterElement(const std::string& id, std::shared_ptr<UIElement> element);
    void UnregisterElement(const std::string& id);
    UIElement* GetElement(const std::string& id);

    // Screen size management
    void SetScreenSize(int width, int height);
    int GetScreenWidth() const { return screenWidth_; }
    int GetScreenHeight() const { return screenHeight_; }

    // Theme management
    bool LoadTheme(const std::string& filepath);
    const UITheme& GetTheme() const { return theme_; }
    void SetTheme(const UITheme& theme) { theme_ = theme; }

    // Utility functions
    void ShowTooltip(const std::string& text, float x, float y);
    void HideTooltip();
    void ShowNotification(const std::string& text, float duration = 3.0f);

    // Input mode
    InputType GetCurrentInputMode() const { return currentInputMode_; }
    void SetInputMode(InputType mode) { currentInputMode_ = mode; }

    // Focus management
    void SetFocusedElement(UIElement* element);
    UIElement* GetFocusedElement() const { return focusedElement_; }

    // Get specific UI components
    HUD* GetHUD() const { return hud_.get(); }
    MainMenu* GetMainMenu() const { return mainMenu_.get(); }
    CharacterSheet* GetCharacterSheet() const { return characterSheet_.get(); }
    InventoryUI* GetInventory() const { return inventory_.get(); }
    DialogueUI* GetDialogue() const { return dialogue_.get(); }
    QuestTracker* GetQuestTracker() const { return questTracker_.get(); }

    // Global UI state
    void SetPaused(bool paused) { paused_ = paused; }
    bool IsPaused() const { return paused_; }

    // Cursor management
    void SetCursorVisible(bool visible);
    bool IsCursorVisible() const { return cursorVisible_; }
    void SetCustomCursor(const std::string& cursorId);

    // Debug
    void SetDebugMode(bool enabled) { debugMode_ = enabled; }
    bool IsDebugMode() const { return debugMode_; }
    void RenderDebugInfo();

private:
    UISystem();
    ~UISystem();

    UISystem(const UISystem&) = delete;
    UISystem& operator=(const UISystem&) = delete;

    // Create UI screens
    void CreateMainMenu();
    void CreateHUD();
    void CreateCharacterSheet();
    void CreateInventory();
    void CreateDialogue();
    void CreateQuestTracker();
    void CreatePauseMenu();
    void CreateSettingsMenu();
    void CreateCraftingUI();

    // Screen stack management
    void ActivateScreen(ScreenType screen);
    void DeactivateScreen(ScreenType screen);

private:
    SDL_Renderer* renderer_;
    int screenWidth_;
    int screenHeight_;

    // UI Theme
    UITheme theme_;

    // Screen stack for UI navigation
    std::stack<ScreenType> screenStack_;
    ScreenType currentScreen_;

    // UI Components
    std::shared_ptr<MainMenu> mainMenu_;
    std::shared_ptr<HUD> hud_;
    std::shared_ptr<CharacterSheet> characterSheet_;
    std::shared_ptr<InventoryUI> inventory_;
    std::shared_ptr<DialogueUI> dialogue_;
    std::shared_ptr<QuestTracker> questTracker_;
    std::shared_ptr<Tooltip> tooltip_;

    // Additional UI screens
    std::shared_ptr<UIElement> pauseMenu_;
    std::shared_ptr<UIElement> settingsMenu_;
    std::shared_ptr<UIElement> craftingUI_;

    // Element registry
    std::unordered_map<std::string, std::shared_ptr<UIElement>> elements_;

    // Input state
    InputType currentInputMode_;
    UIElement* focusedElement_;

    // Global state
    bool paused_;
    bool cursorVisible_;
    bool debugMode_;
    bool initialized_;

    // Notification system
    struct Notification {
        std::string text;
        float duration;
        float elapsed;
        std::shared_ptr<UIElement> element;
    };
    std::vector<Notification> notifications_;
};

} // namespace UI
