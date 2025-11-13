#pragma once

#include "../Inventory/Item.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace Crafting {

// Enchantment types
enum class EnchantmentType {
    // Weapon enchants
    WeaponDamage,
    ElementalDamage,
    LifeSteal,
    ManaSteal,
    CriticalStrike,
    AttackSpeed,
    ArmorPenetration,

    // Armor enchants
    Defense,
    Resistance,
    Health,
    Mana,
    Regeneration,
    MovementSpeed,

    // Utility enchants
    GoldFind,
    MagicFind,
    ExperienceGain,
    SkillBonus,

    // Special enchants
    Thorns,
    Reflect,
    Immunity,
    Proc,

    // Gem enchants
    GemSlot
};

// Enchantment slot
enum class EnchantmentSlot {
    Weapon,
    Helmet,
    Chest,
    Legs,
    Boots,
    Ring,
    Amulet,
    Any
};

// Rune types
enum class RuneType {
    // Basic runes
    Fire,
    Ice,
    Lightning,
    Earth,
    Wind,
    Holy,
    Dark,

    // Combination runes
    Storm,      // Lightning + Wind
    Magma,      // Fire + Earth
    Frost,      // Ice + Wind

    // Special runes
    Power,
    Defense,
    Life,
    Energy,
    Chaos,
    Order,

    // Ancient runes
    Ancient,
    Legendary,
    Mythic
};

// Rune quality
enum class RuneQuality {
    Cracked,
    Flawed,
    Normal,
    Flawless,
    Perfect,
    Radiant
};

// Gem types
enum class GemType {
    Ruby,           // Strength/Fire
    Sapphire,       // Intelligence/Ice
    Emerald,        // Dexterity/Poison
    Diamond,        // All stats
    Topaz,          // Lightning
    Amethyst,       // Magic/Mana
    Opal,           // Mixed elemental
    Onyx,           // Dark damage
    Pearl,          // Holy damage
    Obsidian,       // Defense
    Citrine,        // Gold find
    Jade            // Health
};

/**
 * @brief Represents an enchantment that can be applied to items
 */
class Enchantment {
public:
    Enchantment();
    Enchantment(const std::string& id, const std::string& name, EnchantmentType type);
    ~Enchantment();

    // Getters
    const std::string& GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    const std::string& GetDescription() const { return m_description; }
    EnchantmentType GetType() const { return m_type; }
    EnchantmentSlot GetSlot() const { return m_slot; }
    int GetLevel() const { return m_level; }
    int GetMaxLevel() const { return m_maxLevel; }

    // Stats provided by enchantment
    const Inventory::ItemStats& GetBonusStats() const { return m_bonusStats; }

    // Requirements
    int GetRequiredLevel() const { return m_requiredLevel; }
    int GetRequiredEnchantingSkill() const { return m_requiredEnchantingSkill; }
    const std::vector<std::pair<std::string, int>>& GetMaterialCost() const { return m_materialCost; }
    int GetGoldCost() const { return m_goldCost; }
    int GetManaCost() const { return m_manaCost; }

    // Special effects
    const std::string& GetSpecialEffect() const { return m_specialEffect; }
    float GetProcChance() const { return m_procChance; }
    const std::string& GetProcEffect() const { return m_procEffect; }

    // Compatibility
    bool CanEnchantItem(const Inventory::Item& item) const;
    bool ConflictsWith(const Enchantment& other) const;

    // Setters
    void SetId(const std::string& id) { m_id = id; }
    void SetName(const std::string& name) { m_name = name; }
    void SetDescription(const std::string& desc) { m_description = desc; }
    void SetType(EnchantmentType type) { m_type = type; }
    void SetSlot(EnchantmentSlot slot) { m_slot = slot; }
    void SetLevel(int level) { m_level = level; }
    void SetMaxLevel(int maxLevel) { m_maxLevel = maxLevel; }
    void SetBonusStats(const Inventory::ItemStats& stats) { m_bonusStats = stats; }
    void SetRequiredLevel(int level) { m_requiredLevel = level; }
    void SetRequiredEnchantingSkill(int skill) { m_requiredEnchantingSkill = skill; }
    void SetGoldCost(int cost) { m_goldCost = cost; }
    void SetManaCost(int cost) { m_manaCost = cost; }
    void SetSpecialEffect(const std::string& effect) { m_specialEffect = effect; }
    void SetProcChance(float chance) { m_procChance = chance; }
    void SetProcEffect(const std::string& effect) { m_procEffect = effect; }

    void AddMaterialCost(const std::string& itemId, int quantity);
    void AddConflictingEnchant(const std::string& enchantId);

private:
    std::string m_id;
    std::string m_name;
    std::string m_description;
    EnchantmentType m_type;
    EnchantmentSlot m_slot;
    int m_level;
    int m_maxLevel;
    Inventory::ItemStats m_bonusStats;

    // Requirements
    int m_requiredLevel;
    int m_requiredEnchantingSkill;
    std::vector<std::pair<std::string, int>> m_materialCost;
    int m_goldCost;
    int m_manaCost;

    // Special effects
    std::string m_specialEffect;
    float m_procChance;
    std::string m_procEffect;

    // Compatibility
    std::vector<std::string> m_conflictingEnchants;
};

/**
 * @brief Represents a rune that can be combined or used in enchanting
 */
class Rune {
public:
    Rune();
    Rune(RuneType type, RuneQuality quality);
    ~Rune();

    RuneType GetType() const { return m_type; }
    RuneQuality GetQuality() const { return m_quality; }
    int GetLevel() const { return m_level; }
    const std::string& GetName() const;
    const Inventory::ItemStats& GetStats() const { return m_stats; }

    void SetType(RuneType type) { m_type = type; }
    void SetQuality(RuneQuality quality) { m_quality = quality; }
    void SetLevel(int level) { m_level = level; }
    void SetStats(const Inventory::ItemStats& stats) { m_stats = stats; }

    // Rune combining
    static std::shared_ptr<Rune> CombineRunes(const Rune& rune1, const Rune& rune2);
    static bool CanCombine(const Rune& rune1, const Rune& rune2);

private:
    RuneType m_type;
    RuneQuality m_quality;
    int m_level;
    Inventory::ItemStats m_stats;
};

/**
 * @brief Represents a socketed gem
 */
class Gem {
public:
    Gem();
    Gem(GemType type, int tier);
    ~Gem();

    GemType GetType() const { return m_type; }
    int GetTier() const { return m_tier; }
    const std::string& GetName() const;
    const Inventory::ItemStats& GetStats() const { return m_stats; }

    void SetType(GemType type) { m_type = type; }
    void SetTier(int tier) { m_tier = tier; }
    void SetStats(const Inventory::ItemStats& stats) { m_stats = stats; }

    // Gem upgrading
    static std::shared_ptr<Gem> UpgradeGem(const Gem& gem);
    static int GetMaxTier() { return 10; }

private:
    GemType m_type;
    int m_tier;
    Inventory::ItemStats m_stats;
};

/**
 * @brief Manages enchanting, socketing, and item upgrading
 */
class EnchantingSystem {
public:
    EnchantingSystem();
    ~EnchantingSystem();

    // Enchantment management
    void RegisterEnchantment(std::shared_ptr<Enchantment> enchant);
    std::shared_ptr<Enchantment> GetEnchantment(const std::string& enchantId);
    std::vector<std::shared_ptr<Enchantment>> GetEnchantmentsForSlot(EnchantmentSlot slot);
    std::vector<std::shared_ptr<Enchantment>> GetAvailableEnchantments(int skillLevel);

    // Enchanting
    struct EnchantResult {
        bool success;
        std::string message;
        std::shared_ptr<Inventory::Item> enchantedItem;
        int xpGained;
    };

    EnchantResult EnchantItem(std::shared_ptr<Inventory::Item> item,
                             const std::string& enchantId,
                             int enchantingSkill);

    bool CanEnchantItem(const Inventory::Item& item, const std::string& enchantId) const;
    int GetEnchantmentCount(const Inventory::Item& item) const;
    int GetMaxEnchantments(const Inventory::Item& item) const;

    // Disenchanting
    struct DisenchantResult {
        bool success;
        std::vector<std::pair<std::string, int>> materials;  // Recovered materials
        int xpGained;
    };

    DisenchantResult DisenchantItem(std::shared_ptr<Inventory::Item> item, int enchantingSkill);

    // Socketing
    bool SocketGem(std::shared_ptr<Inventory::Item> item, std::shared_ptr<Gem> gem, int socketIndex);
    bool RemoveGem(std::shared_ptr<Inventory::Item> item, int socketIndex);
    bool AddSocketToItem(std::shared_ptr<Inventory::Item> item, int enchantingSkill);

    // Rune crafting
    std::shared_ptr<Rune> CombineRunes(std::shared_ptr<Rune> rune1, std::shared_ptr<Rune> rune2);
    std::shared_ptr<Enchantment> RuneToEnchantment(std::shared_ptr<Rune> rune);

    // Item upgrading/refinement
    struct UpgradeResult {
        bool success;
        std::shared_ptr<Inventory::Item> upgradedItem;
        float successChance;
        bool itemDestroyed;
        std::string message;
    };

    UpgradeResult UpgradeItem(std::shared_ptr<Inventory::Item> item,
                             const std::vector<std::pair<std::string, int>>& materials,
                             int enchantingSkill);

    // Quality improvement
    UpgradeResult ImproveQuality(std::shared_ptr<Inventory::Item> item,
                                const std::vector<std::pair<std::string, int>>& materials,
                                int enchantingSkill);

    // Reforging (reroll affixes)
    struct ReforgeResult {
        bool success;
        std::shared_ptr<Inventory::Item> reforgedItem;
        std::vector<Inventory::ItemAffix> newAffixes;
    };

    ReforgeResult ReforgeItem(std::shared_ptr<Inventory::Item> item,
                             const std::vector<std::pair<std::string, int>>& materials);

    // Load data
    void LoadEnchantmentsFromFile(const std::string& filePath);

    // Statistics
    struct EnchantingStats {
        int totalEnchants = 0;
        int successfulEnchants = 0;
        int failedEnchants = 0;
        int itemsDisenchanted = 0;
        int gemsSocketed = 0;
        int socketsAdded = 0;
        int itemsUpgraded = 0;
        int itemsDestroyed = 0;
    };

    const EnchantingStats& GetStats() const { return m_stats; }
    void ResetStats();

    // Callbacks
    void SetEnchantCompleteCallback(std::function<void(const EnchantResult&)> callback) {
        m_enchantCompleteCallback = callback;
    }

    void SetUpgradeCompleteCallback(std::function<void(const UpgradeResult&)> callback) {
        m_upgradeCompleteCallback = callback;
    }

private:
    float CalculateEnchantSuccessChance(const Enchantment& enchant, int skillLevel) const;
    float CalculateUpgradeSuccessChance(const Inventory::Item& item, int skillLevel) const;
    int CalculateEnchantingXP(const Enchantment& enchant, bool success) const;

private:
    std::map<std::string, std::shared_ptr<Enchantment>> m_enchantments;
    EnchantingStats m_stats;

    std::function<void(const EnchantResult&)> m_enchantCompleteCallback;
    std::function<void(const UpgradeResult&)> m_upgradeCompleteCallback;
};

// Helper functions
std::string EnchantmentTypeToString(EnchantmentType type);
std::string EnchantmentSlotToString(EnchantmentSlot slot);
std::string RuneTypeToString(RuneType type);
std::string RuneQualityToString(RuneQuality quality);
std::string GemTypeToString(GemType type);

EnchantmentType StringToEnchantmentType(const std::string& str);
EnchantmentSlot StringToEnchantmentSlot(const std::string& str);
RuneType StringToRuneType(const std::string& str);
RuneQuality StringToRuneQuality(const std::string& str);
GemType StringToGemType(const std::string& str);

float GetRuneQualityMultiplier(RuneQuality quality);
int GetGemTierValue(int tier);

} // namespace Crafting
