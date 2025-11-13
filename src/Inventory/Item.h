#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace Inventory {

// Item rarity levels
enum class ItemRarity {
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    Mythic
};

// Item types
enum class ItemType {
    Weapon,
    Helmet,
    Chest,
    Legs,
    Boots,
    Accessory,
    Ring,
    Amulet,
    Consumable,
    CraftingMaterial,
    QuestItem,
    Misc
};

// Weapon types
enum class WeaponType {
    Sword,
    Axe,
    Mace,
    Dagger,
    Bow,
    Crossbow,
    Staff,
    Wand,
    Spear,
    TwoHandedSword,
    TwoHandedAxe,
    TwoHandedMace
};

// Armor types
enum class ArmorType {
    Light,
    Medium,
    Heavy,
    Cloth
};

// Item stats
struct ItemStats {
    // Primary stats
    int strength = 0;
    int dexterity = 0;
    int intelligence = 0;
    int vitality = 0;
    int wisdom = 0;
    int luck = 0;

    // Combat stats
    int physicalDamage = 0;
    int magicDamage = 0;
    int armor = 0;
    int magicResist = 0;
    int critChance = 0;        // Percentage (0-100)
    int critDamage = 0;        // Percentage bonus
    int attackSpeed = 0;       // Percentage modifier

    // Defensive stats
    int blockChance = 0;       // Percentage
    int dodgeChance = 0;       // Percentage
    int parryChance = 0;       // Percentage

    // Resource stats
    int health = 0;
    int mana = 0;
    int healthRegen = 0;
    int manaRegen = 0;

    // Elemental damage
    int fireDamage = 0;
    int iceDamage = 0;
    int lightningDamage = 0;
    int poisonDamage = 0;
    int holyDamage = 0;
    int darkDamage = 0;

    // Elemental resistance
    int fireResist = 0;
    int iceResist = 0;
    int lightningResist = 0;
    int poisonResist = 0;
    int holyResist = 0;
    int darkResist = 0;

    // Special stats
    int movementSpeed = 0;     // Percentage
    int goldFind = 0;          // Percentage bonus
    int magicFind = 0;         // Percentage bonus
    int experienceGain = 0;    // Percentage bonus

    // Combine stats
    ItemStats operator+(const ItemStats& other) const;
    ItemStats& operator+=(const ItemStats& other);
};

// Item affix (random modifier)
struct ItemAffix {
    std::string name;
    std::string description;
    ItemStats stats;
    int minLevel = 1;
    int maxLevel = 100;
    float weight = 1.0f;       // Probability weight
};

// Set bonus information
struct SetBonus {
    int piecesRequired;
    std::string bonusDescription;
    ItemStats bonusStats;
    std::string specialEffect;
};

// Item set information
struct ItemSet {
    std::string setName;
    std::vector<std::string> itemIds;
    std::vector<SetBonus> bonuses;
};

// Core Item class
class Item {
public:
    Item();
    Item(const std::string& id, const std::string& name, ItemType type);
    ~Item();

    // Getters
    const std::string& GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    const std::string& GetDescription() const { return m_description; }
    ItemType GetType() const { return m_type; }
    ItemRarity GetRarity() const { return m_rarity; }
    int GetLevel() const { return m_level; }
    int GetMaxStackSize() const { return m_maxStackSize; }
    int GetStackCount() const { return m_stackCount; }
    int GetValue() const { return m_value; }
    float GetWeight() const { return m_weight; }
    bool IsEquippable() const { return m_isEquippable; }
    bool IsStackable() const { return m_maxStackSize > 1; }
    bool IsQuestItem() const { return m_isQuestItem; }
    bool IsTradeable() const { return m_isTradeable; }
    bool IsUnique() const { return m_isUnique; }
    const std::string& GetIconPath() const { return m_iconPath; }
    const std::string& GetModelPath() const { return m_modelPath; }

    // Item stats
    const ItemStats& GetBaseStats() const { return m_baseStats; }
    const ItemStats& GetBonusStats() const { return m_bonusStats; }
    ItemStats GetTotalStats() const { return m_baseStats + m_bonusStats; }

    // Affixes (prefixes and suffixes)
    const std::vector<ItemAffix>& GetPrefixes() const { return m_prefixes; }
    const std::vector<ItemAffix>& GetSuffixes() const { return m_suffixes; }
    void AddPrefix(const ItemAffix& affix);
    void AddSuffix(const ItemAffix& affix);
    int GetMaxPrefixes() const;
    int GetMaxSuffixes() const;

    // Weapon specific
    WeaponType GetWeaponType() const { return m_weaponType; }
    void SetWeaponType(WeaponType type) { m_weaponType = type; }
    bool IsTwoHanded() const;

    // Armor specific
    ArmorType GetArmorType() const { return m_armorType; }
    void SetArmorType(ArmorType type) { m_armorType = type; }

    // Set items
    bool IsSetItem() const { return !m_setId.empty(); }
    const std::string& GetSetId() const { return m_setId; }
    void SetSetId(const std::string& setId) { m_setId = setId; }

    // Socket system
    int GetSocketCount() const { return m_socketCount; }
    int GetMaxSockets() const { return m_maxSockets; }
    void SetMaxSockets(int count) { m_maxSockets = count; }
    bool AddSocket();
    const std::vector<std::shared_ptr<Item>>& GetSocketedGems() const { return m_socketedGems; }
    bool SocketGem(std::shared_ptr<Item> gem);
    bool UnsocketGem(int socketIndex);

    // Stacking
    bool CanStack(const Item& other) const;
    bool AddToStack(int count);
    bool RemoveFromStack(int count);
    void SetStackCount(int count) { m_stackCount = std::min(count, m_maxStackSize); }

    // Requirements
    int GetLevelRequirement() const { return m_levelRequirement; }
    const std::map<std::string, int>& GetStatRequirements() const { return m_statRequirements; }
    void SetLevelRequirement(int level) { m_levelRequirement = level; }
    void AddStatRequirement(const std::string& stat, int value);

    // Setters
    void SetId(const std::string& id) { m_id = id; }
    void SetName(const std::string& name) { m_name = name; }
    void SetDescription(const std::string& desc) { m_description = desc; }
    void SetType(ItemType type) { m_type = type; }
    void SetRarity(ItemRarity rarity) { m_rarity = rarity; }
    void SetLevel(int level) { m_level = level; }
    void SetMaxStackSize(int size) { m_maxStackSize = size; }
    void SetValue(int value) { m_value = value; }
    void SetWeight(float weight) { m_weight = weight; }
    void SetEquippable(bool equippable) { m_isEquippable = equippable; }
    void SetQuestItem(bool questItem) { m_isQuestItem = questItem; }
    void SetTradeable(bool tradeable) { m_isTradeable = tradeable; }
    void SetUnique(bool unique) { m_isUnique = unique; }
    void SetIconPath(const std::string& path) { m_iconPath = path; }
    void SetModelPath(const std::string& path) { m_modelPath = path; }
    void SetBaseStats(const ItemStats& stats) { m_baseStats = stats; }
    void SetBonusStats(const ItemStats& stats) { m_bonusStats = stats; }

    // Special effects
    const std::string& GetSpecialEffect() const { return m_specialEffect; }
    void SetSpecialEffect(const std::string& effect) { m_specialEffect = effect; }
    const std::string& GetOnUseEffect() const { return m_onUseEffect; }
    void SetOnUseEffect(const std::string& effect) { m_onUseEffect = effect; }

    // Durability system
    int GetDurability() const { return m_durability; }
    int GetMaxDurability() const { return m_maxDurability; }
    void SetMaxDurability(int max) { m_maxDurability = max; m_durability = max; }
    void DamageDurability(int amount);
    void RepairDurability(int amount);
    bool IsBroken() const { return m_durability <= 0; }
    float GetDurabilityPercent() const;

    // Display
    std::string GetFullName() const;
    std::string GetRarityColor() const;
    std::string GetTooltip() const;

    // Comparison
    int CompareTo(const Item& other) const;

    // Cloning
    std::shared_ptr<Item> Clone() const;

private:
    // Basic properties
    std::string m_id;
    std::string m_name;
    std::string m_description;
    ItemType m_type;
    ItemRarity m_rarity;
    int m_level;
    int m_value;
    float m_weight;

    // Stack info
    int m_maxStackSize;
    int m_stackCount;

    // Flags
    bool m_isEquippable;
    bool m_isQuestItem;
    bool m_isTradeable;
    bool m_isUnique;

    // Stats
    ItemStats m_baseStats;
    ItemStats m_bonusStats;

    // Affixes
    std::vector<ItemAffix> m_prefixes;
    std::vector<ItemAffix> m_suffixes;

    // Type specific
    WeaponType m_weaponType;
    ArmorType m_armorType;

    // Set item
    std::string m_setId;

    // Sockets
    int m_socketCount;
    int m_maxSockets;
    std::vector<std::shared_ptr<Item>> m_socketedGems;

    // Requirements
    int m_levelRequirement;
    std::map<std::string, int> m_statRequirements;

    // Resources
    std::string m_iconPath;
    std::string m_modelPath;

    // Special effects
    std::string m_specialEffect;
    std::string m_onUseEffect;

    // Durability
    int m_durability;
    int m_maxDurability;

    void CalculateBonusStats();
};

// Helper functions
std::string ItemRarityToString(ItemRarity rarity);
std::string ItemTypeToString(ItemType type);
std::string WeaponTypeToString(WeaponType type);
std::string ArmorTypeToString(ArmorType type);

ItemRarity StringToItemRarity(const std::string& str);
ItemType StringToItemType(const std::string& str);
WeaponType StringToWeaponType(const std::string& str);
ArmorType StringToArmorType(const std::string& str);

} // namespace Inventory
