#include "ItemDatabase.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

// NOTE: In a real implementation, this would use a proper JSON library
// like nlohmann/json or rapidjson. For this example, we provide the
// structure and interface.

namespace Inventory {

ItemDatabase::ItemDatabase() {
}

ItemDatabase::~ItemDatabase() {
}

bool ItemDatabase::LoadAll(const std::string& dataDirectory) {
    bool success = true;

    success &= LoadWeapons(dataDirectory + "/items/weapons.json");
    success &= LoadArmor(dataDirectory + "/items/armor.json");
    success &= LoadConsumables(dataDirectory + "/items/consumables.json");
    success &= LoadCraftingMaterials(dataDirectory + "/items/crafting_materials.json");
    success &= LoadAffixes(dataDirectory + "/item_affixes.json");
    success &= LoadSets(dataDirectory + "/item_sets.json");

    UpdateCachedLists();

    return success;
}

bool ItemDatabase::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonData = buffer.str();

    return ParseItemFromJson(jsonData);
}

bool ItemDatabase::LoadWeapons(const std::string& filepath) {
    // In production, would parse JSON file
    // For now, return true to indicate structure is ready
    return LoadFromFile(filepath);
}

bool ItemDatabase::LoadArmor(const std::string& filepath) {
    return LoadFromFile(filepath);
}

bool ItemDatabase::LoadConsumables(const std::string& filepath) {
    return LoadFromFile(filepath);
}

bool ItemDatabase::LoadCraftingMaterials(const std::string& filepath) {
    return LoadFromFile(filepath);
}

bool ItemDatabase::LoadAffixes(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonData = buffer.str();

    return ParseAffixFromJson(jsonData);
}

bool ItemDatabase::LoadSets(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonData = buffer.str();

    return ParseSetFromJson(jsonData);
}

std::shared_ptr<Item> ItemDatabase::GetItemTemplate(const std::string& itemId) const {
    auto it = m_items.find(itemId);
    return (it != m_items.end()) ? it->second : nullptr;
}

std::shared_ptr<Item> ItemDatabase::CreateItem(const std::string& itemId) const {
    auto templ = GetItemTemplate(itemId);
    return templ ? templ->Clone() : nullptr;
}

std::shared_ptr<Item> ItemDatabase::GetRandomWeapon(int level) {
    if (m_weaponIds.empty()) return nullptr;

    std::random_device rd;
    std::mt19937 gen(rd());

    if (level >= 0) {
        // Filter by level
        std::vector<std::string> validIds;
        for (const auto& id : m_weaponIds) {
            auto item = GetItemTemplate(id);
            if (item && std::abs(item->GetLevel() - level) <= 5) {
                validIds.push_back(id);
            }
        }

        if (!validIds.empty()) {
            std::uniform_int_distribution<> dist(0, validIds.size() - 1);
            return CreateItem(validIds[dist(gen)]);
        }
    }

    std::uniform_int_distribution<> dist(0, m_weaponIds.size() - 1);
    return CreateItem(m_weaponIds[dist(gen)]);
}

std::shared_ptr<Item> ItemDatabase::GetRandomArmor(ItemType armorType, int level) {
    int armorIndex = -1;
    switch (armorType) {
        case ItemType::Helmet: armorIndex = 0; break;
        case ItemType::Chest: armorIndex = 1; break;
        case ItemType::Legs: armorIndex = 2; break;
        case ItemType::Boots: armorIndex = 3; break;
        default: return nullptr;
    }

    if (m_armorIds[armorIndex].empty()) return nullptr;

    std::random_device rd;
    std::mt19937 gen(rd());

    if (level >= 0) {
        std::vector<std::string> validIds;
        for (const auto& id : m_armorIds[armorIndex]) {
            auto item = GetItemTemplate(id);
            if (item && std::abs(item->GetLevel() - level) <= 5) {
                validIds.push_back(id);
            }
        }

        if (!validIds.empty()) {
            std::uniform_int_distribution<> dist(0, validIds.size() - 1);
            return CreateItem(validIds[dist(gen)]);
        }
    }

    std::uniform_int_distribution<> dist(0, m_armorIds[armorIndex].size() - 1);
    return CreateItem(m_armorIds[armorIndex][dist(gen)]);
}

std::shared_ptr<Item> ItemDatabase::GetRandomAccessory(int level) {
    if (m_accessoryIds.empty()) return nullptr;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, m_accessoryIds.size() - 1);
    return CreateItem(m_accessoryIds[dist(gen)]);
}

std::shared_ptr<Item> ItemDatabase::GetRandomConsumable(int level) {
    if (m_consumableIds.empty()) return nullptr;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, m_consumableIds.size() - 1);
    return CreateItem(m_consumableIds[dist(gen)]);
}

std::vector<std::shared_ptr<Item>> ItemDatabase::GetItemsByType(ItemType type) const {
    std::vector<std::shared_ptr<Item>> items;

    for (const auto& [_, item] : m_items) {
        if (item->GetType() == type) {
            items.push_back(item);
        }
    }

    return items;
}

std::vector<std::shared_ptr<Item>> ItemDatabase::GetItemsByLevel(int minLevel, int maxLevel) const {
    std::vector<std::shared_ptr<Item>> items;

    for (const auto& [_, item] : m_items) {
        if (item->GetLevel() >= minLevel && item->GetLevel() <= maxLevel) {
            items.push_back(item);
        }
    }

    return items;
}

std::vector<std::shared_ptr<Item>> ItemDatabase::GetItemsByRarity(ItemRarity rarity) const {
    std::vector<std::shared_ptr<Item>> items;

    for (const auto& [_, item] : m_items) {
        if (item->GetRarity() == rarity) {
            items.push_back(item);
        }
    }

    return items;
}

std::vector<std::string> ItemDatabase::GetAllItemIds() const {
    std::vector<std::string> ids;

    for (const auto& [id, _] : m_items) {
        ids.push_back(id);
    }

    return ids;
}

ItemAffix ItemDatabase::GetRandomPrefix(int level) const {
    if (m_prefixes.empty()) return ItemAffix();

    std::vector<ItemAffix> valid;
    for (const auto& affix : m_prefixes) {
        if (level < 0 || (level >= affix.minLevel && level <= affix.maxLevel)) {
            valid.push_back(affix);
        }
    }

    if (valid.empty()) return m_prefixes[0];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, valid.size() - 1);
    return valid[dist(gen)];
}

ItemAffix ItemDatabase::GetRandomSuffix(int level) const {
    if (m_suffixes.empty()) return ItemAffix();

    std::vector<ItemAffix> valid;
    for (const auto& affix : m_suffixes) {
        if (level < 0 || (level >= affix.minLevel && level <= affix.maxLevel)) {
            valid.push_back(affix);
        }
    }

    if (valid.empty()) return m_suffixes[0];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, valid.size() - 1);
    return valid[dist(gen)];
}

const ItemSet* ItemDatabase::GetSet(const std::string& setId) const {
    auto it = m_sets.find(setId);
    return (it != m_sets.end()) ? &it->second : nullptr;
}

std::vector<std::string> ItemDatabase::GetAllSetIds() const {
    std::vector<std::string> ids;
    for (const auto& [id, _] : m_sets) {
        ids.push_back(id);
    }
    return ids;
}

bool ItemDatabase::RegisterSet(const ItemSet& set) {
    m_sets[set.setName] = set;
    return true;
}

std::shared_ptr<Item> ItemDatabase::GetUniqueItem(const std::string& uniqueId) const {
    auto item = GetItemTemplate(uniqueId);
    return (item && item->IsUnique()) ? item : nullptr;
}

std::vector<std::string> ItemDatabase::GetAllUniqueIds() const {
    std::vector<std::string> ids;

    for (const auto& [id, item] : m_items) {
        if (item->IsUnique()) {
            ids.push_back(id);
        }
    }

    return ids;
}

int ItemDatabase::GetWeaponCount() const {
    return m_weaponIds.size();
}

int ItemDatabase::GetArmorCount() const {
    int count = 0;
    for (int i = 0; i < 4; i++) {
        count += m_armorIds[i].size();
    }
    return count;
}

int ItemDatabase::GetConsumableCount() const {
    return m_consumableIds.size();
}

void ItemDatabase::Clear() {
    m_items.clear();
    m_sets.clear();
    m_prefixes.clear();
    m_suffixes.clear();

    m_weaponIds.clear();
    for (int i = 0; i < 4; i++) {
        m_armorIds[i].clear();
    }
    m_accessoryIds.clear();
    m_consumableIds.clear();
}

bool ItemDatabase::ParseItemFromJson(const std::string& jsonData) {
    // In production, would use a proper JSON library
    // This is a placeholder for the structure

    // Example parsing logic:
    // auto json = nlohmann::json::parse(jsonData);
    // for (auto& itemJson : json["items"]) {
    //     auto item = CreateItemFromJsonObject(itemJson);
    //     RegisterItem(item);
    // }

    return true;
}

bool ItemDatabase::ParseAffixFromJson(const std::string& jsonData) {
    // Similar to above, would parse affix JSON
    return true;
}

bool ItemDatabase::ParseSetFromJson(const std::string& jsonData) {
    // Similar to above, would parse set JSON
    return true;
}

void ItemDatabase::RegisterItem(std::shared_ptr<Item> item) {
    if (!item) return;

    m_items[item->GetId()] = item;
    UpdateCachedLists();
}

void ItemDatabase::UpdateCachedLists() {
    m_weaponIds.clear();
    for (int i = 0; i < 4; i++) {
        m_armorIds[i].clear();
    }
    m_accessoryIds.clear();
    m_consumableIds.clear();

    for (const auto& [id, item] : m_items) {
        switch (item->GetType()) {
            case ItemType::Weapon:
                m_weaponIds.push_back(id);
                break;
            case ItemType::Helmet:
                m_armorIds[0].push_back(id);
                break;
            case ItemType::Chest:
                m_armorIds[1].push_back(id);
                break;
            case ItemType::Legs:
                m_armorIds[2].push_back(id);
                break;
            case ItemType::Boots:
                m_armorIds[3].push_back(id);
                break;
            case ItemType::Ring:
            case ItemType::Amulet:
            case ItemType::Accessory:
                m_accessoryIds.push_back(id);
                break;
            case ItemType::Consumable:
                m_consumableIds.push_back(id);
                break;
            default:
                break;
        }
    }
}

std::shared_ptr<Item> ItemDatabase::CreateItemFromJsonObject(const std::string& json) {
    // Placeholder for JSON object parsing
    // Would extract all item properties from JSON and create Item
    return nullptr;
}

ItemStats ItemDatabase::ParseStatsFromJson(const std::string& json) {
    // Placeholder for stats parsing
    return ItemStats();
}

} // namespace Inventory
