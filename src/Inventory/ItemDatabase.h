#pragma once

#include "Item.h"
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace Inventory {

// Item database - loads and manages item definitions
class ItemDatabase {
public:
    ItemDatabase();
    ~ItemDatabase();

    // Loading
    bool LoadFromFile(const std::string& filepath);
    bool LoadWeapons(const std::string& filepath);
    bool LoadArmor(const std::string& filepath);
    bool LoadConsumables(const std::string& filepath);
    bool LoadCraftingMaterials(const std::string& filepath);
    bool LoadAffixes(const std::string& filepath);
    bool LoadSets(const std::string& filepath);
    bool LoadAll(const std::string& dataDirectory);

    // Get item templates
    std::shared_ptr<Item> GetItemTemplate(const std::string& itemId) const;
    std::shared_ptr<Item> CreateItem(const std::string& itemId) const;

    // Random selection
    std::shared_ptr<Item> GetRandomWeapon(int level = -1);
    std::shared_ptr<Item> GetRandomArmor(ItemType armorType, int level = -1);
    std::shared_ptr<Item> GetRandomAccessory(int level = -1);
    std::shared_ptr<Item> GetRandomConsumable(int level = -1);

    // Queries
    std::vector<std::shared_ptr<Item>> GetItemsByType(ItemType type) const;
    std::vector<std::shared_ptr<Item>> GetItemsByLevel(int minLevel, int maxLevel) const;
    std::vector<std::shared_ptr<Item>> GetItemsByRarity(ItemRarity rarity) const;
    std::vector<std::string> GetAllItemIds() const;

    // Affixes
    const std::vector<ItemAffix>& GetPrefixes() const { return m_prefixes; }
    const std::vector<ItemAffix>& GetSuffixes() const { return m_suffixes; }
    ItemAffix GetRandomPrefix(int level = -1) const;
    ItemAffix GetRandomSuffix(int level = -1) const;

    // Sets
    const ItemSet* GetSet(const std::string& setId) const;
    std::vector<std::string> GetAllSetIds() const;
    bool RegisterSet(const ItemSet& set);

    // Unique items
    std::shared_ptr<Item> GetUniqueItem(const std::string& uniqueId) const;
    std::vector<std::string> GetAllUniqueIds() const;

    // Statistics
    int GetItemCount() const { return m_items.size(); }
    int GetWeaponCount() const;
    int GetArmorCount() const;
    int GetConsumableCount() const;

    // Clear
    void Clear();

private:
    std::map<std::string, std::shared_ptr<Item>> m_items;
    std::map<std::string, ItemSet> m_sets;
    std::vector<ItemAffix> m_prefixes;
    std::vector<ItemAffix> m_suffixes;

    // Cached lists for quick random access
    std::vector<std::string> m_weaponIds;
    std::vector<std::string> m_armorIds[4]; // Helmet, Chest, Legs, Boots
    std::vector<std::string> m_accessoryIds;
    std::vector<std::string> m_consumableIds;

    // Helper methods
    bool ParseItemFromJson(const std::string& jsonData);
    bool ParseAffixFromJson(const std::string& jsonData);
    bool ParseSetFromJson(const std::string& jsonData);
    void RegisterItem(std::shared_ptr<Item> item);
    void UpdateCachedLists();

    // JSON parsing helpers (simplified - would use a JSON library in production)
    std::shared_ptr<Item> CreateItemFromJsonObject(const std::string& json);
    ItemStats ParseStatsFromJson(const std::string& json);
};

} // namespace Inventory
