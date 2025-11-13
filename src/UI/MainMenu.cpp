#include "MainMenu.h"
#include "UISystem.h"
#include <SDL2/SDL.h>
#include <cmath>

namespace UI {

MainMenu::MainMenu()
    : UIElement("mainMenu"),
      menuState_(MenuState::Main),
      selectedCharacter_(CharacterClass::Warrior),
      hasSaveGame_(false),
      selectedCharacterIndex_(0),
      titlePulse_(0.0f),
      backgroundScroll_(0.0f) {

    // Set full screen size
    auto& uiSystem = UISystem::Instance();
    SetSize(static_cast<float>(uiSystem.GetScreenWidth()),
            static_cast<float>(uiSystem.GetScreenHeight()));
    SetPosition(0, 0);

    // Set background
    SetBackgroundColor(Color::FromHex("#0A0A0A"));

    // Initialize character data
    InitializeCharacterPreviews();

    // Create UI elements
    CreateMainMenuButtons();
    CreateCharacterSelectUI();
    CreateSettingsUI();

    // Start with main menu visible
    ShowMainMenu();
}

MainMenu::~MainMenu() {
}

void MainMenu::Update(float deltaTime) {
    UIElement::Update(deltaTime);

    // Update animations
    titlePulse_ += deltaTime * 2.0f;
    backgroundScroll_ += deltaTime * 10.0f;

    // Update based on current state
    switch (menuState_) {
        case MenuState::Main:
            if (mainButtonPanel_) mainButtonPanel_->Update(deltaTime);
            break;
        case MenuState::CharacterSelect:
            if (characterSelectPanel_) characterSelectPanel_->Update(deltaTime);
            if (characterPreviewPanel_) characterPreviewPanel_->Update(deltaTime);
            break;
        case MenuState::Settings:
            if (settingsPanel_) settingsPanel_->Update(deltaTime);
            break;
        case MenuState::Credits:
            if (creditsPanel_) creditsPanel_->Update(deltaTime);
            break;
        default:
            break;
    }
}

void MainMenu::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    // Render animated background
    RenderBackground(renderer);

    // Render title
    RenderTitle(renderer);

    // Render based on current state
    switch (menuState_) {
        case MenuState::Main:
            RenderMainButtons(renderer);
            if (mainButtonPanel_) mainButtonPanel_->Render(renderer);
            break;
        case MenuState::CharacterSelect:
            if (characterSelectPanel_) characterSelectPanel_->Render(renderer);
            RenderCharacterPreview(renderer);
            break;
        case MenuState::Settings:
            if (settingsPanel_) settingsPanel_->Render(renderer);
            break;
        case MenuState::Credits:
            if (creditsPanel_) creditsPanel_->Render(renderer);
            break;
        default:
            break;
    }

    // Render children
    UIElement::Render(renderer);
}

void MainMenu::HandleInput(const SDL_Event& event) {
    if (!visible_ || !interactive_) return;

    // Handle escape key to go back
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        if (menuState_ != MenuState::Main) {
            ShowMainMenu();
            return;
        }
    }

    UIElement::HandleInput(event);
}

void MainMenu::ShowMainMenu() {
    SetMenuState(MenuState::Main);

    if (mainButtonPanel_) mainButtonPanel_->SetVisible(true);
    if (characterSelectPanel_) characterSelectPanel_->SetVisible(false);
    if (settingsPanel_) settingsPanel_->SetVisible(false);
    if (creditsPanel_) creditsPanel_->SetVisible(false);
}

void MainMenu::ShowCharacterSelect() {
    SetMenuState(MenuState::CharacterSelect);

    if (mainButtonPanel_) mainButtonPanel_->SetVisible(false);
    if (characterSelectPanel_) {
        characterSelectPanel_->SetVisible(true);
        characterSelectPanel_->PlayAnimation(AnimationType::FadeIn, 0.3f);
    }
    if (settingsPanel_) settingsPanel_->SetVisible(false);
    if (creditsPanel_) creditsPanel_->SetVisible(false);
}

void MainMenu::ShowSettings() {
    SetMenuState(MenuState::Settings);

    if (mainButtonPanel_) mainButtonPanel_->SetVisible(false);
    if (characterSelectPanel_) characterSelectPanel_->SetVisible(false);
    if (settingsPanel_) {
        settingsPanel_->SetVisible(true);
        settingsPanel_->PlayAnimation(AnimationType::FadeIn, 0.3f);
    }
    if (creditsPanel_) creditsPanel_->SetVisible(false);
}

void MainMenu::ShowCredits() {
    SetMenuState(MenuState::Credits);

    if (mainButtonPanel_) mainButtonPanel_->SetVisible(false);
    if (characterSelectPanel_) characterSelectPanel_->SetVisible(false);
    if (settingsPanel_) settingsPanel_->SetVisible(false);
    if (creditsPanel_) {
        creditsPanel_->SetVisible(true);
        creditsPanel_->PlayAnimation(AnimationType::FadeIn, 0.3f);
    }
}

void MainMenu::SelectCharacter(CharacterClass characterClass) {
    selectedCharacter_ = characterClass;

    // Update selected index
    for (size_t i = 0; i < characterPreviews_.size(); ++i) {
        if (characterPreviews_[i].characterClass == characterClass) {
            selectedCharacterIndex_ = static_cast<int>(i);
            break;
        }
    }
}

const CharacterPreview* MainMenu::GetCharacterPreview(CharacterClass characterClass) const {
    for (const auto& preview : characterPreviews_) {
        if (preview.characterClass == characterClass) {
            return &preview;
        }
    }
    return nullptr;
}

void MainMenu::SetMenuState(MenuState state) {
    menuState_ = state;
}

void MainMenu::InitializeCharacterPreviews() {
    characterPreviews_.clear();

    // Warrior
    CharacterPreview warrior;
    warrior.characterClass = CharacterClass::Warrior;
    warrior.name = "Warrior";
    warrior.description = "A mighty fighter with unmatched strength and endurance. Masters of melee combat.";
    warrior.iconPath = "assets/ui/icons/warrior.png";
    warrior.accentColor = Color::FromHex("#C41E3A");
    warrior.baseHealth = 150;
    warrior.baseMana = 50;
    warrior.baseStamina = 100;
    warrior.baseStrength = 20;
    warrior.baseIntelligence = 8;
    warrior.baseDexterity = 12;
    characterPreviews_.push_back(warrior);

    // Mage
    CharacterPreview mage;
    mage.characterClass = CharacterClass::Mage;
    mage.name = "Mage";
    mage.description = "A master of arcane arts, wielding devastating spells with unparalleled magical power.";
    mage.iconPath = "assets/ui/icons/mage.png";
    mage.accentColor = Color::FromHex("#3FC7EB");
    mage.baseHealth = 80;
    mage.baseMana = 200;
    mage.baseStamina = 60;
    mage.baseStrength = 8;
    mage.baseIntelligence = 25;
    mage.baseDexterity = 10;
    characterPreviews_.push_back(mage);

    // Rogue
    CharacterPreview rogue;
    rogue.characterClass = CharacterClass::Rogue;
    rogue.name = "Rogue";
    rogue.description = "A cunning assassin who strikes from shadows with deadly precision and agility.";
    rogue.iconPath = "assets/ui/icons/rogue.png";
    rogue.accentColor = Color::FromHex("#FFF468");
    rogue.baseHealth = 100;
    rogue.baseMana = 80;
    rogue.baseStamina = 120;
    rogue.baseStrength = 12;
    rogue.baseIntelligence = 12;
    rogue.baseDexterity = 22;
    characterPreviews_.push_back(rogue);

    // Ranger
    CharacterPreview ranger;
    ranger.characterClass = CharacterClass::Ranger;
    ranger.name = "Ranger";
    ranger.description = "A skilled marksman and nature's guardian, excelling at ranged combat and tracking.";
    ranger.iconPath = "assets/ui/icons/ranger.png";
    ranger.accentColor = Color::FromHex("#AAD372");
    ranger.baseHealth = 110;
    ranger.baseMana = 100;
    ranger.baseStamina = 110;
    ranger.baseStrength = 14;
    ranger.baseIntelligence = 14;
    ranger.baseDexterity = 18;
    characterPreviews_.push_back(ranger);

    // Paladin
    CharacterPreview paladin;
    paladin.characterClass = CharacterClass::Paladin;
    paladin.name = "Paladin";
    paladin.description = "A holy warrior blessed with divine power, balancing offense and healing abilities.";
    paladin.iconPath = "assets/ui/icons/paladin.png";
    paladin.accentColor = Color::FromHex("#F48CBA");
    paladin.baseHealth = 130;
    paladin.baseMana = 120;
    paladin.baseStamina = 90;
    paladin.baseStrength = 16;
    paladin.baseIntelligence = 16;
    paladin.baseDexterity = 10;
    characterPreviews_.push_back(paladin);

    // Necromancer
    CharacterPreview necromancer;
    necromancer.characterClass = CharacterClass::Necromancer;
    necromancer.name = "Necromancer";
    necromancer.description = "A dark sorcerer who commands the undead and drains life from enemies.";
    necromancer.iconPath = "assets/ui/icons/necromancer.png";
    necromancer.accentColor = Color::FromHex("#9382C9");
    necromancer.baseHealth = 90;
    necromancer.baseMana = 180;
    necromancer.baseStamina = 70;
    necromancer.baseStrength = 10;
    necromancer.baseIntelligence = 22;
    necromancer.baseDexterity = 12;
    characterPreviews_.push_back(necromancer);
}

void MainMenu::CreateMainMenuButtons() {
    auto& uiSystem = UISystem::Instance();
    const auto& theme = uiSystem.GetTheme();

    mainButtonPanel_ = std::make_shared<UIElement>("mainButtonPanel");
    mainButtonPanel_->SetSize(400, 500);
    mainButtonPanel_->SetPosition(width_ / 2 - 200, height_ / 2 - 100);
    mainButtonPanel_->SetBackgroundColor(Color(0, 0, 0, 0)); // Transparent

    AddChild(mainButtonPanel_);

    // Button configuration
    struct ButtonConfig {
        std::string label;
        std::function<void()> onClick;
        bool enabled;
    };

    std::vector<ButtonConfig> buttons = {
        {"New Game", [this]() { ShowCharacterSelect(); }, true},
        {"Continue", [this]() { if (onContinueGame_) onContinueGame_(); }, hasSaveGame_},
        {"Settings", [this]() { ShowSettings(); }, true},
        {"Credits", [this]() { ShowCredits(); }, true},
        {"Quit", [this]() { if (onQuit_) onQuit_(); }, true}
    };

    float buttonY = 150;
    for (size_t i = 0; i < buttons.size(); ++i) {
        auto button = std::make_shared<UIElement>("mainButton_" + std::to_string(i));
        button->SetSize(350, theme.buttonHeight);
        button->SetPosition(25, buttonY);
        button->SetBackgroundColor(theme.primaryColor);
        button->SetBorderColor(theme.accentColor);
        button->SetBorderWidth(2.0f);
        button->SetEnabled(buttons[i].enabled);

        if (buttons[i].enabled) {
            button->OnClick(buttons[i].onClick);
            button->OnHover([button, &theme]() {
                button->SetBackgroundColor(theme.secondaryColor);
                button->PlayAnimation(AnimationType::Scale, 0.2f);
            });
        } else {
            button->SetBackgroundColor(theme.disabledColor);
        }

        mainButtonPanel_->AddChild(button);
        mainButtons_.push_back(button);

        buttonY += theme.buttonHeight + theme.elementSpacing;
    }
}

void MainMenu::CreateCharacterSelectUI() {
    auto& uiSystem = UISystem::Instance();
    const auto& theme = uiSystem.GetTheme();

    characterSelectPanel_ = std::make_shared<UIElement>("characterSelectPanel");
    characterSelectPanel_->SetSize(width_ * 0.8f, height_ * 0.8f);
    characterSelectPanel_->SetPosition(width_ * 0.1f, height_ * 0.1f);
    characterSelectPanel_->SetBackgroundColor(Color(20, 20, 20, 240));
    characterSelectPanel_->SetBorderColor(theme.accentColor);
    characterSelectPanel_->SetBorderWidth(3.0f);
    characterSelectPanel_->SetVisible(false);

    AddChild(characterSelectPanel_);

    // Create character selection buttons (grid layout)
    float buttonWidth = 180;
    float buttonHeight = 220;
    float spacing = 20;
    float startX = 50;
    float startY = 100;

    for (size_t i = 0; i < characterPreviews_.size(); ++i) {
        int row = i / 3;
        int col = i % 3;

        auto charButton = std::make_shared<UIElement>("charButton_" + std::to_string(i));
        charButton->SetSize(buttonWidth, buttonHeight);
        charButton->SetPosition(startX + col * (buttonWidth + spacing),
                               startY + row * (buttonHeight + spacing));
        charButton->SetBackgroundColor(Color(40, 40, 40, 255));
        charButton->SetBorderColor(characterPreviews_[i].accentColor);
        charButton->SetBorderWidth(2.0f);

        int index = i; // Capture for lambda
        charButton->OnClick([this, index]() {
            selectedCharacterIndex_ = index;
            SelectCharacter(characterPreviews_[index].characterClass);
        });

        charButton->OnHover([charButton, color = characterPreviews_[i].accentColor]() {
            charButton->SetBackgroundColor(Color(60, 60, 60, 255));
            charButton->SetBorderWidth(4.0f);
        });

        characterSelectPanel_->AddChild(charButton);
        characterButtons_.push_back(charButton);
    }

    // Character preview panel
    characterPreviewPanel_ = std::make_shared<UIElement>("characterPreviewPanel");
    characterPreviewPanel_->SetSize(400, 300);
    characterPreviewPanel_->SetPosition(characterSelectPanel_->GetWidth() - 450, 100);
    characterPreviewPanel_->SetBackgroundColor(Color(30, 30, 30, 255));
    characterPreviewPanel_->SetBorderColor(theme.accentColor);
    characterPreviewPanel_->SetBorderWidth(2.0f);

    characterSelectPanel_->AddChild(characterPreviewPanel_);

    // Start Game button
    auto startButton = std::make_shared<UIElement>("startGameButton");
    startButton->SetSize(300, theme.buttonHeight);
    startButton->SetPosition(characterSelectPanel_->GetWidth() / 2 - 150,
                            characterSelectPanel_->GetHeight() - 100);
    startButton->SetBackgroundColor(theme.successColor);
    startButton->SetBorderColor(theme.accentColor);
    startButton->SetBorderWidth(2.0f);

    startButton->OnClick([this]() {
        if (onNewGame_) {
            onNewGame_(selectedCharacter_);
        }
    });

    characterSelectPanel_->AddChild(startButton);

    // Back button
    auto backButton = std::make_shared<UIElement>("backButton");
    backButton->SetSize(150, theme.buttonHeight);
    backButton->SetPosition(50, characterSelectPanel_->GetHeight() - 100);
    backButton->SetBackgroundColor(theme.secondaryColor);
    backButton->SetBorderColor(theme.accentColor);
    backButton->SetBorderWidth(2.0f);

    backButton->OnClick([this]() { ShowMainMenu(); });

    characterSelectPanel_->AddChild(backButton);
}

void MainMenu::CreateSettingsUI() {
    auto& uiSystem = UISystem::Instance();
    const auto& theme = uiSystem.GetTheme();

    settingsPanel_ = std::make_shared<UIElement>("settingsPanel");
    settingsPanel_->SetSize(800, 600);
    settingsPanel_->SetPosition(width_ / 2 - 400, height_ / 2 - 300);
    settingsPanel_->SetBackgroundColor(Color(20, 20, 20, 240));
    settingsPanel_->SetBorderColor(theme.accentColor);
    settingsPanel_->SetBorderWidth(3.0f);
    settingsPanel_->SetVisible(false);

    AddChild(settingsPanel_);

    // This would contain various settings options
    // Placeholder for graphics, audio, and control settings

    // Back button
    auto backButton = std::make_shared<UIElement>("settingsBackButton");
    backButton->SetSize(150, theme.buttonHeight);
    backButton->SetPosition(50, settingsPanel_->GetHeight() - 100);
    backButton->SetBackgroundColor(theme.secondaryColor);
    backButton->SetBorderColor(theme.accentColor);
    backButton->SetBorderWidth(2.0f);

    backButton->OnClick([this]() { ShowMainMenu(); });

    settingsPanel_->AddChild(backButton);
}

void MainMenu::RenderBackground(SDL_Renderer* renderer) {
    // Render animated gradient background
    // This is a simplified version - could be enhanced with particles, etc.

    SDL_Rect topHalf = {0, 0, static_cast<int>(width_), static_cast<int>(height_ / 2)};
    SDL_Rect bottomHalf = {0, static_cast<int>(height_ / 2),
                           static_cast<int>(width_), static_cast<int>(height_ / 2)};

    // Animated color shift
    uint8_t pulse = static_cast<uint8_t>(128 + std::sin(titlePulse_) * 20);

    SDL_SetRenderDrawColor(renderer, 10, 10, pulse / 8, 255);
    SDL_RenderFillRect(renderer, &topHalf);

    SDL_SetRenderDrawColor(renderer, 5, 5, pulse / 10, 255);
    SDL_RenderFillRect(renderer, &bottomHalf);
}

void MainMenu::RenderTitle(SDL_Renderer* renderer) {
    // Render animated game title
    // This is a placeholder - would typically use TTF fonts

    float pulse = 1.0f + std::sin(titlePulse_) * 0.1f;
    auto& theme = UISystem::Instance().GetTheme();

    SDL_Rect titleBox;
    titleBox.w = static_cast<int>(500 * pulse);
    titleBox.h = static_cast<int>(100 * pulse);
    titleBox.x = static_cast<int>(width_ / 2 - titleBox.w / 2);
    titleBox.y = static_cast<int>(50);

    uint8_t alpha = static_cast<uint8_t>(200 + std::sin(titlePulse_) * 55);

    SDL_SetRenderDrawColor(renderer, theme.accentColor.r, theme.accentColor.g,
                          theme.accentColor.b, alpha);
    SDL_RenderDrawRect(renderer, &titleBox);
}

void MainMenu::RenderCharacterPreview(SDL_Renderer* renderer) {
    if (!characterPreviewPanel_ || !characterPreviewPanel_->IsVisible()) return;

    // Render selected character information
    if (selectedCharacterIndex_ >= 0 &&
        selectedCharacterIndex_ < static_cast<int>(characterPreviews_.size())) {

        const auto& preview = characterPreviews_[selectedCharacterIndex_];

        // Highlight selected character button
        if (selectedCharacterIndex_ < static_cast<int>(characterButtons_.size())) {
            auto& button = characterButtons_[selectedCharacterIndex_];
            button->SetBorderWidth(5.0f);
            button->SetBorderColor(preview.accentColor);
        }
    }
}

void MainMenu::RenderMainButtons(SDL_Renderer* renderer) {
    // Additional rendering for main menu buttons if needed
}

void MainMenu::OnPositionChanged() {
    UIElement::OnPositionChanged();
}

void MainMenu::OnSizeChanged() {
    UIElement::OnSizeChanged();
}

} // namespace UI
