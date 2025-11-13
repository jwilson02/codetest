#include "CharacterSheet.h"
#include "UISystem.h"
#include <SDL2/SDL.h>
#include <algorithm>

namespace UI {

CharacterSheet::CharacterSheet()
    : UIElement("characterSheet"),
      activeTab_(Tab::Stats),
      hoveredSlot_(EquipmentSlot::Head),
      isHoveringSlot_(false) {

    auto& uiSystem = UISystem::Instance();
    const auto& theme = uiSystem.GetTheme();

    // Set size and position (centered, 80% of screen)
    float width = uiSystem.GetScreenWidth() * 0.8f;
    float height = uiSystem.GetScreenHeight() * 0.8f;
    SetSize(width, height);
    SetPosition(uiSystem.GetScreenWidth() * 0.1f, uiSystem.GetScreenHeight() * 0.1f);

    SetBackgroundColor(Color(25, 25, 30, 240));
    SetBorderColor(theme.accentColor);
    SetBorderWidth(3.0f);
    SetVisible(false);

    CreateLayout();
}

CharacterSheet::~CharacterSheet() {
}

void CharacterSheet::Update(float deltaTime) {
    if (!visible_) return;

    UIElement::Update(deltaTime);

    // Update active panel
    switch (activeTab_) {
        case Tab::Stats:
            if (statsPanel_) statsPanel_->Update(deltaTime);
            break;
        case Tab::Equipment:
            if (equipmentPanel_) equipmentPanel_->Update(deltaTime);
            break;
        case Tab::Skills:
            if (skillsPanel_) skillsPanel_->Update(deltaTime);
            break;
        case Tab::Achievements:
            if (achievementsPanel_) achievementsPanel_->Update(deltaTime);
            break;
    }
}

void CharacterSheet::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    UIElement::Render(renderer);

    // Render active panel
    switch (activeTab_) {
        case Tab::Stats:
            RenderStatsPanel(renderer);
            break;
        case Tab::Equipment:
            RenderEquipmentPanel(renderer);
            break;
        case Tab::Skills:
            RenderSkillsPanel(renderer);
            break;
        default:
            break;
    }
}

void CharacterSheet::HandleInput(const SDL_Event& event) {
    if (!visible_) return;

    // Close with ESC or 'C' key
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_c) {
            SetVisible(false);
            return;
        }

        // Tab shortcuts
        if (event.key.keysym.sym == SDLK_1) SetActiveTab(Tab::Stats);
        else if (event.key.keysym.sym == SDLK_2) SetActiveTab(Tab::Equipment);
        else if (event.key.keysym.sym == SDLK_3) SetActiveTab(Tab::Skills);
        else if (event.key.keysym.sym == SDLK_4) SetActiveTab(Tab::Achievements);
    }

    UIElement::HandleInput(event);
}

void CharacterSheet::SetStats(const CharacterStats& stats) {
    stats_ = stats;
}

void CharacterSheet::EquipItem(EquipmentSlot slot, const EquipmentItem& item) {
    equipment_[slot] = item;
    if (onEquipmentChanged_) {
        onEquipmentChanged_(slot, item);
    }
}

void CharacterSheet::UnequipItem(EquipmentSlot slot) {
    equipment_.erase(slot);
}

const EquipmentItem* CharacterSheet::GetEquippedItem(EquipmentSlot slot) const {
    auto it = equipment_.find(slot);
    return (it != equipment_.end()) ? &it->second : nullptr;
}

void CharacterSheet::AllocateAttribute(const std::string& attribute) {
    if (stats_.attributePoints <= 0) return;

    if (attribute == "strength") stats_.strength++;
    else if (attribute == "intelligence") stats_.intelligence++;
    else if (attribute == "dexterity") stats_.dexterity++;
    else if (attribute == "vitality") stats_.vitality++;
    else if (attribute == "wisdom") stats_.wisdom++;
    else if (attribute == "luck") stats_.luck++;
    else return;

    stats_.attributePoints--;

    if (onAttributeAllocated_) {
        onAttributeAllocated_(attribute);
    }
}

void CharacterSheet::SetActiveTab(Tab tab) {
    activeTab_ = tab;

    // Hide all panels
    if (statsPanel_) statsPanel_->SetVisible(false);
    if (equipmentPanel_) equipmentPanel_->SetVisible(false);
    if (skillsPanel_) skillsPanel_->SetVisible(false);
    if (achievementsPanel_) achievementsPanel_->SetVisible(false);

    // Show active panel
    switch (tab) {
        case Tab::Stats:
            if (statsPanel_) {
                statsPanel_->SetVisible(true);
                statsPanel_->PlayAnimation(AnimationType::FadeIn, 0.2f);
            }
            break;
        case Tab::Equipment:
            if (equipmentPanel_) {
                equipmentPanel_->SetVisible(true);
                equipmentPanel_->PlayAnimation(AnimationType::FadeIn, 0.2f);
            }
            break;
        case Tab::Skills:
            if (skillsPanel_) {
                skillsPanel_->SetVisible(true);
                skillsPanel_->PlayAnimation(AnimationType::FadeIn, 0.2f);
            }
            break;
        case Tab::Achievements:
            if (achievementsPanel_) {
                achievementsPanel_->SetVisible(true);
                achievementsPanel_->PlayAnimation(AnimationType::FadeIn, 0.2f);
            }
            break;
    }

    // Update tab button states
    for (size_t i = 0; i < tabButtons_.size(); ++i) {
        if (tabButtons_[i]) {
            auto& theme = UISystem::Instance().GetTheme();
            if (static_cast<int>(tab) == static_cast<int>(i)) {
                tabButtons_[i]->SetBackgroundColor(theme.primaryColor);
                tabButtons_[i]->SetBorderWidth(3.0f);
            } else {
                tabButtons_[i]->SetBackgroundColor(theme.secondaryColor);
                tabButtons_[i]->SetBorderWidth(2.0f);
            }
        }
    }
}

void CharacterSheet::CreateLayout() {
    CreateTabButtons();
    CreateStatsPanel();
    CreateEquipmentPanel();
    CreateSkillsPanel();

    // Set initial tab
    SetActiveTab(Tab::Stats);
}

void CharacterSheet::CreateTabButtons() {
    auto& theme = UISystem::Instance().GetTheme();

    std::vector<std::string> tabNames = {"Stats", "Equipment", "Skills", "Achievements"};
    float tabWidth = 150;
    float tabHeight = 40;
    float spacing = 10;
    float startX = 20;
    float startY = 20;

    for (size_t i = 0; i < tabNames.size(); ++i) {
        auto tabButton = std::make_shared<UIElement>("tab_" + std::to_string(i));
        tabButton->SetSize(tabWidth, tabHeight);
        tabButton->SetPosition(startX + i * (tabWidth + spacing), startY);
        tabButton->SetBackgroundColor(theme.secondaryColor);
        tabButton->SetBorderColor(theme.accentColor);
        tabButton->SetBorderWidth(2.0f);

        Tab tabType = static_cast<Tab>(i);
        tabButton->OnClick([this, tabType]() {
            SetActiveTab(tabType);
        });

        tabButton->OnHover([tabButton, &theme]() {
            tabButton->SetBorderWidth(3.0f);
        });

        AddChild(tabButton);
        tabButtons_.push_back(tabButton);
    }
}

void CharacterSheet::CreateStatsPanel() {
    auto& theme = UISystem::Instance().GetTheme();

    statsPanel_ = std::make_shared<UIElement>("statsPanel");
    statsPanel_->SetSize(width_ - 40, height_ - 100);
    statsPanel_->SetPosition(20, 80);
    statsPanel_->SetBackgroundColor(Color(15, 15, 20, 200));
    statsPanel_->SetBorderColor(theme.primaryColor);
    statsPanel_->SetBorderWidth(1.0f);

    AddChild(statsPanel_);

    // Character model preview (left side)
    characterModel_ = std::make_shared<UIElement>("characterModel");
    characterModel_->SetSize(300, 400);
    characterModel_->SetPosition(30, 30);
    characterModel_->SetBackgroundColor(Color(30, 30, 35, 255));
    characterModel_->SetBorderColor(theme.accentColor);
    characterModel_->SetBorderWidth(2.0f);

    statsPanel_->AddChild(characterModel_);
}

void CharacterSheet::CreateEquipmentPanel() {
    auto& theme = UISystem::Instance().GetTheme();

    equipmentPanel_ = std::make_shared<UIElement>("equipmentPanel");
    equipmentPanel_->SetSize(width_ - 40, height_ - 100);
    equipmentPanel_->SetPosition(20, 80);
    equipmentPanel_->SetBackgroundColor(Color(15, 15, 20, 200));
    equipmentPanel_->SetBorderColor(theme.primaryColor);
    equipmentPanel_->SetBorderWidth(1.0f);
    equipmentPanel_->SetVisible(false);

    AddChild(equipmentPanel_);

    // Create equipment slots
    float slotSize = 60;
    float spacing = 10;

    // Define slot positions (paperdoll layout)
    struct SlotPosition {
        EquipmentSlot slot;
        float x, y;
    };

    std::vector<SlotPosition> positions = {
        {EquipmentSlot::Head, 200, 50},
        {EquipmentSlot::Shoulders, 200, 120},
        {EquipmentSlot::Chest, 200, 190},
        {EquipmentSlot::Hands, 130, 190},
        {EquipmentSlot::Legs, 200, 260},
        {EquipmentSlot::Feet, 200, 330},
        {EquipmentSlot::MainHand, 130, 260},
        {EquipmentSlot::OffHand, 270, 260},
        {EquipmentSlot::Neck, 270, 50},
        {EquipmentSlot::Ring1, 130, 120},
        {EquipmentSlot::Ring2, 270, 120},
        {EquipmentSlot::Trinket1, 60, 190},
        {EquipmentSlot::Trinket2, 340, 190},
        {EquipmentSlot::Back, 270, 330}
    };

    for (const auto& pos : positions) {
        auto slotElement = std::make_shared<UIElement>("equipSlot_" + GetSlotName(pos.slot));
        slotElement->SetSize(slotSize, slotSize);
        slotElement->SetPosition(pos.x, pos.y);
        slotElement->SetBackgroundColor(Color(40, 40, 45, 255));
        slotElement->SetBorderColor(Color(80, 80, 90, 255));
        slotElement->SetBorderWidth(2.0f);

        EquipmentSlot slot = pos.slot;
        slotElement->OnHover([this, slot, slotElement, &theme]() {
            hoveredSlot_ = slot;
            isHoveringSlot_ = true;
            slotElement->SetBorderColor(theme.accentColor);
            slotElement->SetBorderWidth(3.0f);
        });

        equipmentPanel_->AddChild(slotElement);
        equipmentSlots_.push_back(slotElement);
    }
}

void CharacterSheet::CreateSkillsPanel() {
    auto& theme = UISystem::Instance().GetTheme();

    skillsPanel_ = std::make_shared<UIElement>("skillsPanel");
    skillsPanel_->SetSize(width_ - 40, height_ - 100);
    skillsPanel_->SetPosition(20, 80);
    skillsPanel_->SetBackgroundColor(Color(15, 15, 20, 200));
    skillsPanel_->SetBorderColor(theme.primaryColor);
    skillsPanel_->SetBorderWidth(1.0f);
    skillsPanel_->SetVisible(false);

    AddChild(skillsPanel_);

    // Skill tree would go here
    // For now, placeholder
}

void CharacterSheet::RenderStatsPanel(SDL_Renderer* renderer) {
    if (!statsPanel_ || !statsPanel_->IsVisible()) return;

    statsPanel_->Render(renderer);

    // Render character stats on the right side
    float startX = statsPanel_->GetX() + 350;
    float startY = statsPanel_->GetY() + 40;
    float lineHeight = 30;

    auto& theme = UISystem::Instance().GetTheme();

    // Character info
    // Would render: Name, Class, Level using TTF fonts

    // Core attributes
    float y = startY + 60;
    bool canIncrease = stats_.attributePoints > 0;

    RenderAttributeLine(renderer, "Strength", stats_.strength, startX, y, canIncrease);
    y += lineHeight;
    RenderAttributeLine(renderer, "Intelligence", stats_.intelligence, startX, y, canIncrease);
    y += lineHeight;
    RenderAttributeLine(renderer, "Dexterity", stats_.dexterity, startX, y, canIncrease);
    y += lineHeight;
    RenderAttributeLine(renderer, "Vitality", stats_.vitality, startX, y, canIncrease);
    y += lineHeight;
    RenderAttributeLine(renderer, "Wisdom", stats_.wisdom, startX, y, canIncrease);
    y += lineHeight;
    RenderAttributeLine(renderer, "Luck", stats_.luck, startX, y, canIncrease);
    y += lineHeight + 20;

    // Derived stats
    RenderStatBar(renderer, "Health", stats_.maxHealth, startX, y);
    y += lineHeight;
    RenderStatBar(renderer, "Mana", stats_.maxMana, startX, y);
    y += lineHeight;
    RenderStatBar(renderer, "Stamina", stats_.maxStamina, startX, y);
    y += lineHeight + 10;

    RenderStatBar(renderer, "Armor", stats_.armor, startX, y);
    y += lineHeight;
    RenderStatBar(renderer, "Magic Resist", stats_.magicResist, startX, y);
    y += lineHeight;
    RenderStatBar(renderer, "Attack Power", stats_.attackPower, startX, y);
    y += lineHeight;
    RenderStatBar(renderer, "Spell Power", stats_.spellPower, startX, y);
}

void CharacterSheet::RenderEquipmentPanel(SDL_Renderer* renderer) {
    if (!equipmentPanel_ || !equipmentPanel_->IsVisible()) return;

    equipmentPanel_->Render(renderer);

    // Render equipped items in slots
    for (const auto& slotElement : equipmentSlots_) {
        slotElement->Render(renderer);
    }

    // Render item stats on the right side
    float statsX = equipmentPanel_->GetX() + 450;
    float statsY = equipmentPanel_->GetY() + 50;

    // Total stats from equipment
    // Would calculate and display total bonuses
}

void CharacterSheet::RenderSkillsPanel(SDL_Renderer* renderer) {
    if (!skillsPanel_ || !skillsPanel_->IsVisible()) return;

    skillsPanel_->Render(renderer);

    // Render skill tree
    // Placeholder
}

void CharacterSheet::RenderEquipmentSlot(SDL_Renderer* renderer, EquipmentSlot slot) {
    const EquipmentItem* item = GetEquippedItem(slot);
    if (!item) return;

    // Would render item icon and rarity border
    // Placeholder
}

void CharacterSheet::RenderStatBar(SDL_Renderer* renderer, const std::string& label,
                                   int value, float x, float y) {
    // Render stat label and value
    // Would use TTF fonts
    // Format: "Stat Name: 123"

    SDL_Rect labelRect;
    labelRect.x = static_cast<int>(x);
    labelRect.y = static_cast<int>(y);
    labelRect.w = 200;
    labelRect.h = 20;

    auto& theme = UISystem::Instance().GetTheme();
    SDL_SetRenderDrawColor(renderer, theme.textColor.r, theme.textColor.g,
                          theme.textColor.b, 180);
    SDL_RenderDrawRect(renderer, &labelRect);
}

void CharacterSheet::RenderAttributeLine(SDL_Renderer* renderer, const std::string& label,
                                        int value, float x, float y, bool canIncrease) {
    // Render attribute with +/- buttons if points available
    RenderStatBar(renderer, label, value, x, y);

    if (canIncrease) {
        // Render + button
        SDL_Rect buttonRect;
        buttonRect.x = static_cast<int>(x + 220);
        buttonRect.y = static_cast<int>(y);
        buttonRect.w = 20;
        buttonRect.h = 20;

        auto& theme = UISystem::Instance().GetTheme();
        SDL_SetRenderDrawColor(renderer, theme.successColor.r, theme.successColor.g,
                              theme.successColor.b, 255);
        SDL_RenderFillRect(renderer, &buttonRect);
    }
}

std::string CharacterSheet::GetSlotName(EquipmentSlot slot) const {
    switch (slot) {
        case EquipmentSlot::Head: return "Head";
        case EquipmentSlot::Shoulders: return "Shoulders";
        case EquipmentSlot::Chest: return "Chest";
        case EquipmentSlot::Hands: return "Hands";
        case EquipmentSlot::Legs: return "Legs";
        case EquipmentSlot::Feet: return "Feet";
        case EquipmentSlot::MainHand: return "MainHand";
        case EquipmentSlot::OffHand: return "OffHand";
        case EquipmentSlot::Neck: return "Neck";
        case EquipmentSlot::Ring1: return "Ring1";
        case EquipmentSlot::Ring2: return "Ring2";
        case EquipmentSlot::Trinket1: return "Trinket1";
        case EquipmentSlot::Trinket2: return "Trinket2";
        case EquipmentSlot::Back: return "Back";
        default: return "Unknown";
    }
}

Color CharacterSheet::GetRarityColor(const std::string& rarity) const {
    if (rarity == "Common") return Color::FromHex("#9D9D9D");
    if (rarity == "Uncommon") return Color::FromHex("#1EFF00");
    if (rarity == "Rare") return Color::FromHex("#0070DD");
    if (rarity == "Epic") return Color::FromHex("#A335EE");
    if (rarity == "Legendary") return Color::FromHex("#FF8000");
    return Color::FromHex("#FFFFFF");
}

void CharacterSheet::OnPositionChanged() {
    UIElement::OnPositionChanged();
}

void CharacterSheet::OnSizeChanged() {
    UIElement::OnSizeChanged();
}

} // namespace UI
