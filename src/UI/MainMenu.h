#pragma once

#include "UIElement.h"
#include <vector>
#include <functional>

namespace UI {

// Character class types for selection
enum class CharacterClass {
    Warrior,
    Mage,
    Rogue,
    Ranger,
    Paladin,
    Necromancer
};

// Character preview data
struct CharacterPreview {
    CharacterClass characterClass;
    std::string name;
    std::string description;
    std::string iconPath;
    Color accentColor;

    // Base stats preview
    int baseHealth;
    int baseMana;
    int baseStamina;
    int baseStrength;
    int baseIntelligence;
    int baseDexterity;
};

// Menu state
enum class MenuState {
    Main,
    CharacterSelect,
    Settings,
    Credits,
    Loading
};

// Main Menu UI
class MainMenu : public UIElement {
public:
    MainMenu();
    ~MainMenu() override;

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    // Menu navigation
    void ShowMainMenu();
    void ShowCharacterSelect();
    void ShowSettings();
    void ShowCredits();

    // Character selection
    void SelectCharacter(CharacterClass characterClass);
    CharacterClass GetSelectedCharacter() const { return selectedCharacter_; }
    const CharacterPreview* GetCharacterPreview(CharacterClass characterClass) const;

    // Callbacks
    void OnNewGame(std::function<void(CharacterClass)> callback) { onNewGame_ = callback; }
    void OnContinueGame(std::function<void()> callback) { onContinueGame_ = callback; }
    void OnQuit(std::function<void()> callback) { onQuit_ = callback; }

    // State
    MenuState GetMenuState() const { return menuState_; }
    void SetMenuState(MenuState state);

    // Settings
    void SetHasSaveGame(bool hasSave) { hasSaveGame_ = hasSave; }
    bool HasSaveGame() const { return hasSaveGame_; }

private:
    void InitializeCharacterPreviews();
    void CreateMainMenuButtons();
    void CreateCharacterSelectUI();
    void CreateSettingsUI();

    void RenderBackground(SDL_Renderer* renderer);
    void RenderTitle(SDL_Renderer* renderer);
    void RenderCharacterPreview(SDL_Renderer* renderer);
    void RenderMainButtons(SDL_Renderer* renderer);

    void OnPositionChanged() override;
    void OnSizeChanged() override;

private:
    MenuState menuState_;
    CharacterClass selectedCharacter_;
    bool hasSaveGame_;

    // Character data
    std::vector<CharacterPreview> characterPreviews_;
    int selectedCharacterIndex_;

    // UI Elements
    std::shared_ptr<UIElement> titleElement_;
    std::shared_ptr<UIElement> mainButtonPanel_;
    std::shared_ptr<UIElement> characterSelectPanel_;
    std::shared_ptr<UIElement> settingsPanel_;
    std::shared_ptr<UIElement> creditsPanel_;

    // Buttons
    std::vector<std::shared_ptr<UIElement>> mainButtons_;
    std::vector<std::shared_ptr<UIElement>> characterButtons_;

    // Character preview
    std::shared_ptr<UIElement> characterPreviewPanel_;
    std::shared_ptr<UIElement> characterStatsPanel_;

    // Callbacks
    std::function<void(CharacterClass)> onNewGame_;
    std::function<void()> onContinueGame_;
    std::function<void()> onQuit_;

    // Animation state
    float titlePulse_;
    float backgroundScroll_;
};

} // namespace UI
