#pragma once

#include "UIElement.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace UI {

// Character stats
struct CharacterStats {
    // Core attributes
    int strength;
    int intelligence;
    int dexterity;
    int vitality;
    int wisdom;
    int luck;

    // Derived stats
    int maxHealth;
    int maxMana;
    int maxStamina;
    int armor;
    int magicResist;
    int attackPower;
    int spellPower;
    int critChance;
    int critDamage;
    int attackSpeed;
    int movementSpeed;

    // Character info
    std::string name;
    std::string className;
    int level;
    int experience;
    int experienceToNextLevel;
    int skillPoints;
    int attributePoints;

    CharacterStats()
        : strength(10), intelligence(10), dexterity(10),
          vitality(10), wisdom(10), luck(10),
          maxHealth(100), maxMana(100), maxStamina(100),
          armor(0), magicResist(0), attackPower(10),
          spellPower(10), critChance(5), critDamage(150),
          attackSpeed(100), movementSpeed(100),
          level(1), experience(0), experienceToNextLevel(100),
          skillPoints(0), attributePoints(0) {}
};

// Equipment slots
enum class EquipmentSlot {
    Head,
    Shoulders,
    Chest,
    Hands,
    Legs,
    Feet,
    MainHand,
    OffHand,
    Neck,
    Ring1,
    Ring2,
    Trinket1,
    Trinket2,
    Back
};

// Item data for equipment
struct EquipmentItem {
    std::string id;
    std::string name;
    std::string iconPath;
    EquipmentSlot slot;
    int level;
    std::string rarity; // Common, Uncommon, Rare, Epic, Legendary

    // Stats provided
    std::unordered_map<std::string, int> stats;

    EquipmentItem() : level(1), rarity("Common") {}
};

// Character sheet UI
class CharacterSheet : public UIElement {
public:
    CharacterSheet();
    ~CharacterSheet() override;

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    // Stats management
    void SetStats(const CharacterStats& stats);
    const CharacterStats& GetStats() const { return stats_; }

    // Equipment management
    void EquipItem(EquipmentSlot slot, const EquipmentItem& item);
    void UnequipItem(EquipmentSlot slot);
    const EquipmentItem* GetEquippedItem(EquipmentSlot slot) const;

    // Attribute allocation
    void AllocateAttribute(const std::string& attribute);
    bool CanAllocateAttributes() const { return stats_.attributePoints > 0; }

    // Callbacks
    void OnAttributeAllocated(std::function<void(const std::string&)> callback) {
        onAttributeAllocated_ = callback;
    }
    void OnEquipmentChanged(std::function<void(EquipmentSlot, const EquipmentItem&)> callback) {
        onEquipmentChanged_ = callback;
    }

    // Tab management
    enum class Tab {
        Stats,
        Equipment,
        Skills,
        Achievements
    };

    void SetActiveTab(Tab tab);
    Tab GetActiveTab() const { return activeTab_; }

private:
    void CreateLayout();
    void CreateStatsPanel();
    void CreateEquipmentPanel();
    void CreateSkillsPanel();
    void CreateTabButtons();

    void RenderStatsPanel(SDL_Renderer* renderer);
    void RenderEquipmentPanel(SDL_Renderer* renderer);
    void RenderSkillsPanel(SDL_Renderer* renderer);
    void RenderEquipmentSlot(SDL_Renderer* renderer, EquipmentSlot slot);
    void RenderStatBar(SDL_Renderer* renderer, const std::string& label,
                       int value, float x, float y);
    void RenderAttributeLine(SDL_Renderer* renderer, const std::string& label,
                            int value, float x, float y, bool canIncrease);

    std::string GetSlotName(EquipmentSlot slot) const;
    Color GetRarityColor(const std::string& rarity) const;

    void OnPositionChanged() override;
    void OnSizeChanged() override;

private:
    CharacterStats stats_;
    Tab activeTab_;

    // Equipment
    std::unordered_map<EquipmentSlot, EquipmentItem> equipment_;
    std::vector<std::shared_ptr<UIElement>> equipmentSlots_;

    // UI Panels
    std::shared_ptr<UIElement> statsPanel_;
    std::shared_ptr<UIElement> equipmentPanel_;
    std::shared_ptr<UIElement> skillsPanel_;
    std::shared_ptr<UIElement> achievementsPanel_;

    // Tab buttons
    std::vector<std::shared_ptr<UIElement>> tabButtons_;

    // Character model display
    std::shared_ptr<UIElement> characterModel_;

    // Callbacks
    std::function<void(const std::string&)> onAttributeAllocated_;
    std::function<void(EquipmentSlot, const EquipmentItem&)> onEquipmentChanged_;

    // Hover state
    EquipmentSlot hoveredSlot_;
    bool isHoveringSlot_;
};

} // namespace UI
