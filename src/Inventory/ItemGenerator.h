#pragma once

#include "Item.h"
#include <random>
#include <vector>
#include <memory>

namespace Inventory {

// Forward declaration
class ItemDatabase;

// Item generation parameters
struct ItemGenerationParams {
    int level;
    ItemRarity minRarity;
    ItemRarity maxRarity;
    ItemType type;
    bool forceUnique;
    float magicFindBonus;

    ItemGenerationParams()
        : level(1),
          minRarity(ItemRarity::Common),
          maxRarity(ItemRarity::Mythic),
          type(ItemType::Misc),
          forceUnique(false),
          magicFindBonus(0.0f) {}
};

// Loot table entry
struct LootTableEntry {
    std::string itemId;
    float weight;
    int minQuantity;
    int maxQuantity;
    int minLevel;
    int maxLevel;

    LootTableEntry()
        : itemId(""), weight(1.0f),
          minQuantity(1), maxQuantity(1),
          minLevel(1), maxLevel(100) {}
};

// Loot table
struct LootTable {
    std::string id;
    std::vector<LootTableEntry> entries;
    int minItems;
    int maxItems;

    LootTable()
        : id(""), minItems(1), maxItems(1) {}
};

// Item generator class
class ItemGenerator {
public:
    ItemGenerator();
    ~ItemGenerator();

    // Set database reference
    void SetDatabase(ItemDatabase* database) { m_database = database; }

    // Set seed for reproducible generation
    void SetSeed(unsigned int seed);

    // Generate items
    std::shared_ptr<Item> GenerateItem(const ItemGenerationParams& params);
    std::shared_ptr<Item> GenerateItemOfType(ItemType type, int level, ItemRarity rarity);
    std::shared_ptr<Item> GenerateWeapon(int level, ItemRarity rarity);
    std::shared_ptr<Item> GenerateArmor(ItemType armorType, int level, ItemRarity rarity);
    std::shared_ptr<Item> GenerateAccessory(int level, ItemRarity rarity);
    std::shared_ptr<Item> GenerateConsumable(int level);

    // Generate from base item
    std::shared_ptr<Item> EnhanceItem(std::shared_ptr<Item> baseItem, ItemRarity targetRarity);

    // Affix generation
    void AddRandomAffixes(std::shared_ptr<Item> item);
    void AddRandomPrefix(std::shared_ptr<Item> item, int level);
    void AddRandomSuffix(std::shared_ptr<Item> item, int level);

    // Loot generation
    std::vector<std::shared_ptr<Item>> GenerateLoot(const LootTable& table, int level, float magicFind = 0.0f);
    std::vector<std::shared_ptr<Item>> GenerateRandomLoot(int count, int level, float magicFind = 0.0f);

    // Rarity determination
    ItemRarity DetermineRarity(int level, float magicFind = 0.0f);
    ItemRarity RollRarity(ItemRarity minRarity, ItemRarity maxRarity, float magicFind = 0.0f);

    // Unique item generation
    std::shared_ptr<Item> GenerateUniqueItem(const std::string& uniqueId, int level);
    bool CanGenerateUnique(int level, float magicFind);

    // Set item generation
    std::shared_ptr<Item> GenerateSetItem(const std::string& setId, const std::string& itemId, int level);

    // Stat scaling
    ItemStats ScaleStats(const ItemStats& baseStats, int level, ItemRarity rarity);
    int ScaleDamage(int baseDamage, int level, ItemRarity rarity);
    int ScaleArmor(int baseArmor, int level, ItemRarity rarity);

    // Affix management
    void LoadAffixesFromFile(const std::string& filepath);
    void RegisterAffix(const ItemAffix& affix);
    const std::vector<ItemAffix>& GetPrefixes() const { return m_prefixes; }
    const std::vector<ItemAffix>& GetSuffixes() const { return m_suffixes; }

    // Loot table management
    void RegisterLootTable(const LootTable& table);
    const LootTable* GetLootTable(const std::string& id) const;

    // Item quality/roll
    enum class ItemQuality {
        Poor,       // Below average stats
        Normal,     // Average stats
        Superior,   // Above average stats
        Perfect     // Maximum stats
    };

    ItemQuality RollQuality();
    float GetQualityMultiplier(ItemQuality quality);

private:
    std::mt19937 m_rng;
    ItemDatabase* m_database;

    std::vector<ItemAffix> m_prefixes;
    std::vector<ItemAffix> m_suffixes;
    std::map<std::string, LootTable> m_lootTables;

    // Rarity weights (can be adjusted)
    std::map<ItemRarity, float> m_rarityWeights;

    // Helper methods
    ItemAffix SelectRandomAffix(const std::vector<ItemAffix>& affixes, int level);
    float GetRarityChance(ItemRarity rarity, float magicFind);
    void ApplyQualityToStats(ItemStats& stats, ItemQuality quality);

    // Random number generation
    int RandomInt(int min, int max);
    float RandomFloat(float min, float max);
    bool RandomChance(float probability);

    // Stat generation helpers
    int GenerateStatValue(int baseValue, int level, ItemRarity rarity, ItemQuality quality);
};

// Crafting system (item generation from materials)
class CraftingSystem {
public:
    struct CraftingRecipe {
        std::string id;
        std::string name;
        std::string resultItemId;
        std::map<std::string, int> materials;  // itemId -> quantity
        int requiredLevel;
        int craftingCost;
        ItemRarity resultRarity;

        CraftingRecipe()
            : id(""), name(""), resultItemId(""),
              requiredLevel(1), craftingCost(0),
              resultRarity(ItemRarity::Common) {}
    };

    CraftingSystem();
    ~CraftingSystem();

    // Register recipes
    void RegisterRecipe(const CraftingRecipe& recipe);
    void LoadRecipesFromFile(const std::string& filepath);

    // Crafting
    bool CanCraft(const CraftingRecipe& recipe, InventorySystem& inventory, int playerLevel);
    std::shared_ptr<Item> Craft(const CraftingRecipe& recipe, InventorySystem& inventory,
                                ItemGenerator& generator);

    // Recipe queries
    const CraftingRecipe* GetRecipe(const std::string& id) const;
    std::vector<const CraftingRecipe*> GetAvailableRecipes(int playerLevel) const;
    std::vector<const CraftingRecipe*> GetRecipesForMaterial(const std::string& materialId) const;

private:
    std::map<std::string, CraftingRecipe> m_recipes;

    bool ConsumeMaterials(const CraftingRecipe& recipe, InventorySystem& inventory);
};

// Item upgrading/enhancing
class ItemUpgradeSystem {
public:
    ItemUpgradeSystem();
    ~ItemUpgradeSystem();

    // Upgrade levels
    static const int MAX_UPGRADE_LEVEL = 15;

    // Upgrade an item (increases stats)
    bool UpgradeItem(std::shared_ptr<Item> item, int materialCost);
    int GetUpgradeLevel(std::shared_ptr<Item> item) const;
    int GetUpgradeCost(std::shared_ptr<Item> item) const;
    float GetUpgradeSuccessChance(std::shared_ptr<Item> item) const;

    // Add sockets
    bool AddSocketToItem(std::shared_ptr<Item> item, int cost);

    // Reroll affixes
    bool RerollAffixes(std::shared_ptr<Item> item, ItemGenerator& generator, int cost);

    // Change rarity
    bool UpgradeRarity(std::shared_ptr<Item> item, ItemGenerator& generator, int cost);

private:
    std::map<std::shared_ptr<Item>, int> m_upgradeLevels;

    float CalculateUpgradeMultiplier(int upgradeLevel);
};

} // namespace Inventory
