#include "ItemGenerator.h"
#include "ItemDatabase.h"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace Inventory {

// ItemGenerator implementation
ItemGenerator::ItemGenerator()
    : m_database(nullptr) {

    // Initialize RNG with current time
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    m_rng.seed(static_cast<unsigned int>(seed));

    // Set default rarity weights
    m_rarityWeights[ItemRarity::Common] = 50.0f;
    m_rarityWeights[ItemRarity::Uncommon] = 25.0f;
    m_rarityWeights[ItemRarity::Rare] = 15.0f;
    m_rarityWeights[ItemRarity::Epic] = 7.0f;
    m_rarityWeights[ItemRarity::Legendary] = 2.5f;
    m_rarityWeights[ItemRarity::Mythic] = 0.5f;
}

ItemGenerator::~ItemGenerator() {
}

void ItemGenerator::SetSeed(unsigned int seed) {
    m_rng.seed(seed);
}

std::shared_ptr<Item> ItemGenerator::GenerateItem(const ItemGenerationParams& params) {
    ItemRarity rarity = RollRarity(params.minRarity, params.maxRarity, params.magicFindBonus);

    if (params.type != ItemType::Misc) {
        return GenerateItemOfType(params.type, params.level, rarity);
    }

    // Random type
    std::vector<ItemType> types = {
        ItemType::Weapon, ItemType::Helmet, ItemType::Chest,
        ItemType::Legs, ItemType::Boots, ItemType::Ring,
        ItemType::Amulet, ItemType::Accessory
    };

    ItemType randomType = types[RandomInt(0, types.size() - 1)];
    return GenerateItemOfType(randomType, params.level, rarity);
}

std::shared_ptr<Item> ItemGenerator::GenerateItemOfType(ItemType type, int level, ItemRarity rarity) {
    switch (type) {
        case ItemType::Weapon:
            return GenerateWeapon(level, rarity);
        case ItemType::Helmet:
        case ItemType::Chest:
        case ItemType::Legs:
        case ItemType::Boots:
            return GenerateArmor(type, level, rarity);
        case ItemType::Ring:
        case ItemType::Amulet:
        case ItemType::Accessory:
            return GenerateAccessory(level, rarity);
        case ItemType::Consumable:
            return GenerateConsumable(level);
        default:
            return nullptr;
    }
}

std::shared_ptr<Item> ItemGenerator::GenerateWeapon(int level, ItemRarity rarity) {
    // Generate weapon from database if available
    if (m_database) {
        auto baseWeapon = m_database->GetRandomWeapon(level);
        if (baseWeapon) {
            return EnhanceItem(baseWeapon, rarity);
        }
    }

    // Fallback: generate generic weapon
    auto weapon = std::make_shared<Item>("weapon_" + std::to_string(RandomInt(1, 1000)),
                                        "Weapon", ItemType::Weapon);

    weapon->SetLevel(level);
    weapon->SetRarity(rarity);
    weapon->SetLevelRequirement(level);

    // Generate base stats
    ItemStats stats;
    ItemQuality quality = RollQuality();

    stats.physicalDamage = GenerateStatValue(10, level, rarity, quality);
    stats.critChance = GenerateStatValue(5, level / 5, rarity, quality);

    weapon->SetBaseStats(stats);

    // Add affixes based on rarity
    AddRandomAffixes(weapon);

    // Calculate value
    int value = 10 + level * 5 + static_cast<int>(rarity) * 50;
    weapon->SetValue(value);
    weapon->SetWeight(3.0f + level * 0.1f);

    return weapon;
}

std::shared_ptr<Item> ItemGenerator::GenerateArmor(ItemType armorType, int level, ItemRarity rarity) {
    // Generate armor from database if available
    if (m_database) {
        auto baseArmor = m_database->GetRandomArmor(armorType, level);
        if (baseArmor) {
            return EnhanceItem(baseArmor, rarity);
        }
    }

    // Fallback: generate generic armor
    std::string typeName = ItemTypeToString(armorType);
    auto armor = std::make_shared<Item>("armor_" + std::to_string(RandomInt(1, 1000)),
                                       typeName, armorType);

    armor->SetLevel(level);
    armor->SetRarity(rarity);
    armor->SetLevelRequirement(level);

    // Generate base stats
    ItemStats stats;
    ItemQuality quality = RollQuality();

    stats.armor = GenerateStatValue(15, level, rarity, quality);
    stats.health = GenerateStatValue(20, level, rarity, quality);

    armor->SetBaseStats(stats);

    // Add affixes
    AddRandomAffixes(armor);

    // Calculate value
    int value = 8 + level * 4 + static_cast<int>(rarity) * 40;
    armor->SetValue(value);
    armor->SetWeight(5.0f + level * 0.15f);

    return armor;
}

std::shared_ptr<Item> ItemGenerator::GenerateAccessory(int level, ItemRarity rarity) {
    if (m_database) {
        auto baseAccessory = m_database->GetRandomAccessory(level);
        if (baseAccessory) {
            return EnhanceItem(baseAccessory, rarity);
        }
    }

    // Fallback
    auto accessory = std::make_shared<Item>("accessory_" + std::to_string(RandomInt(1, 1000)),
                                           "Accessory", ItemType::Accessory);

    accessory->SetLevel(level);
    accessory->SetRarity(rarity);
    accessory->SetLevelRequirement(level);

    ItemStats stats;
    ItemQuality quality = RollQuality();

    // Accessories have varied stats
    if (RandomChance(0.5f)) stats.strength = GenerateStatValue(5, level, rarity, quality);
    if (RandomChance(0.5f)) stats.dexterity = GenerateStatValue(5, level, rarity, quality);
    if (RandomChance(0.5f)) stats.intelligence = GenerateStatValue(5, level, rarity, quality);
    if (RandomChance(0.3f)) stats.critChance = GenerateStatValue(3, level / 5, rarity, quality);
    if (RandomChance(0.3f)) stats.magicFind = GenerateStatValue(5, level / 2, rarity, quality);

    accessory->SetBaseStats(stats);
    AddRandomAffixes(accessory);

    int value = 15 + level * 6 + static_cast<int>(rarity) * 60;
    accessory->SetValue(value);
    accessory->SetWeight(0.5f);

    return accessory;
}

std::shared_ptr<Item> ItemGenerator::GenerateConsumable(int level) {
    if (m_database) {
        auto consumable = m_database->GetRandomConsumable(level);
        if (consumable) {
            return consumable->Clone();
        }
    }

    // Fallback: health potion
    auto potion = std::make_shared<Item>("health_potion", "Health Potion", ItemType::Consumable);
    potion->SetMaxStackSize(99);
    potion->SetValue(10 + level * 2);
    potion->SetDescription("Restores " + std::to_string(50 + level * 10) + " health.");
    potion->SetOnUseEffect("restore_health_" + std::to_string(50 + level * 10));

    return potion;
}

std::shared_ptr<Item> ItemGenerator::EnhanceItem(std::shared_ptr<Item> baseItem, ItemRarity targetRarity) {
    if (!baseItem) return nullptr;

    auto enhanced = baseItem->Clone();
    enhanced->SetRarity(targetRarity);

    // Scale base stats
    ItemStats scaledStats = ScaleStats(baseItem->GetBaseStats(), baseItem->GetLevel(), targetRarity);
    enhanced->SetBaseStats(scaledStats);

    // Add affixes
    AddRandomAffixes(enhanced);

    // Increase value
    int valueMultiplier = 1 + static_cast<int>(targetRarity) * 2;
    enhanced->SetValue(baseItem->GetValue() * valueMultiplier);

    return enhanced;
}

void ItemGenerator::AddRandomAffixes(std::shared_ptr<Item> item) {
    if (!item || m_prefixes.empty() || m_suffixes.empty()) return;

    int maxPrefixes = item->GetMaxPrefixes();
    int maxSuffixes = item->GetMaxSuffixes();

    // Add prefixes
    for (int i = 0; i < maxPrefixes; i++) {
        if (RandomChance(0.7f)) {
            AddRandomPrefix(item, item->GetLevel());
        }
    }

    // Add suffixes
    for (int i = 0; i < maxSuffixes; i++) {
        if (RandomChance(0.7f)) {
            AddRandomSuffix(item, item->GetLevel());
        }
    }
}

void ItemGenerator::AddRandomPrefix(std::shared_ptr<Item> item, int level) {
    if (!item || m_prefixes.empty()) return;

    ItemAffix prefix = SelectRandomAffix(m_prefixes, level);
    item->AddPrefix(prefix);
}

void ItemGenerator::AddRandomSuffix(std::shared_ptr<Item> item, int level) {
    if (!item || m_suffixes.empty()) return;

    ItemAffix suffix = SelectRandomAffix(m_suffixes, level);
    item->AddSuffix(suffix);
}

std::vector<std::shared_ptr<Item>> ItemGenerator::GenerateLoot(const LootTable& table, int level, float magicFind) {
    std::vector<std::shared_ptr<Item>> loot;

    int itemCount = RandomInt(table.minItems, table.maxItems);

    for (int i = 0; i < itemCount; i++) {
        // Select random entry based on weights
        float totalWeight = 0.0f;
        for (const auto& entry : table.entries) {
            if (level >= entry.minLevel && level <= entry.maxLevel) {
                totalWeight += entry.weight;
            }
        }

        float roll = RandomFloat(0.0f, totalWeight);
        float currentWeight = 0.0f;

        for (const auto& entry : table.entries) {
            if (level < entry.minLevel || level > entry.maxLevel) continue;

            currentWeight += entry.weight;
            if (roll <= currentWeight) {
                // Generate item
                if (m_database) {
                    auto baseItem = m_database->GetItemTemplate(entry.itemId);
                    if (baseItem) {
                        ItemRarity rarity = DetermineRarity(level, magicFind);
                        auto item = EnhanceItem(baseItem, rarity);

                        int quantity = RandomInt(entry.minQuantity, entry.maxQuantity);
                        item->SetStackCount(quantity);

                        loot.push_back(item);
                    }
                }
                break;
            }
        }
    }

    return loot;
}

std::vector<std::shared_ptr<Item>> ItemGenerator::GenerateRandomLoot(int count, int level, float magicFind) {
    std::vector<std::shared_ptr<Item>> loot;

    for (int i = 0; i < count; i++) {
        ItemGenerationParams params;
        params.level = level;
        params.magicFindBonus = magicFind;

        auto item = GenerateItem(params);
        if (item) {
            loot.push_back(item);
        }
    }

    return loot;
}

ItemRarity ItemGenerator::DetermineRarity(int level, float magicFind) {
    return RollRarity(ItemRarity::Common, ItemRarity::Mythic, magicFind);
}

ItemRarity ItemGenerator::RollRarity(ItemRarity minRarity, ItemRarity maxRarity, float magicFind) {
    // Calculate total weight
    float totalWeight = 0.0f;
    for (int r = static_cast<int>(minRarity); r <= static_cast<int>(maxRarity); r++) {
        ItemRarity rarity = static_cast<ItemRarity>(r);
        float weight = m_rarityWeights[rarity] * (1.0f + magicFind / 100.0f);
        totalWeight += weight;
    }

    // Roll
    float roll = RandomFloat(0.0f, totalWeight);
    float currentWeight = 0.0f;

    for (int r = static_cast<int>(maxRarity); r >= static_cast<int>(minRarity); r--) {
        ItemRarity rarity = static_cast<ItemRarity>(r);
        float weight = m_rarityWeights[rarity] * (1.0f + magicFind / 100.0f);
        currentWeight += weight;

        if (roll <= currentWeight) {
            return rarity;
        }
    }

    return minRarity;
}

ItemStats ItemGenerator::ScaleStats(const ItemStats& baseStats, int level, ItemRarity rarity) {
    ItemStats scaled = baseStats;

    float levelScale = 1.0f + (level - 1) * 0.1f;
    float rarityScale = 1.0f + static_cast<float>(rarity) * 0.25f;
    float totalScale = levelScale * rarityScale;

    scaled.physicalDamage = static_cast<int>(scaled.physicalDamage * totalScale);
    scaled.magicDamage = static_cast<int>(scaled.magicDamage * totalScale);
    scaled.armor = static_cast<int>(scaled.armor * totalScale);
    scaled.magicResist = static_cast<int>(scaled.magicResist * totalScale);
    scaled.health = static_cast<int>(scaled.health * totalScale);
    scaled.mana = static_cast<int>(scaled.mana * totalScale);

    scaled.strength = static_cast<int>(scaled.strength * totalScale);
    scaled.dexterity = static_cast<int>(scaled.dexterity * totalScale);
    scaled.intelligence = static_cast<int>(scaled.intelligence * totalScale);
    scaled.vitality = static_cast<int>(scaled.vitality * totalScale);

    return scaled;
}

int ItemGenerator::ScaleDamage(int baseDamage, int level, ItemRarity rarity) {
    float scale = 1.0f + (level - 1) * 0.1f + static_cast<float>(rarity) * 0.25f;
    return static_cast<int>(baseDamage * scale);
}

int ItemGenerator::ScaleArmor(int baseArmor, int level, ItemRarity rarity) {
    float scale = 1.0f + (level - 1) * 0.08f + static_cast<float>(rarity) * 0.2f;
    return static_cast<int>(baseArmor * scale);
}

ItemGenerator::ItemQuality ItemGenerator::RollQuality() {
    float roll = RandomFloat(0.0f, 100.0f);

    if (roll < 5.0f) return ItemQuality::Poor;
    if (roll < 80.0f) return ItemQuality::Normal;
    if (roll < 98.0f) return ItemQuality::Superior;
    return ItemQuality::Perfect;
}

float ItemGenerator::GetQualityMultiplier(ItemQuality quality) {
    switch (quality) {
        case ItemQuality::Poor: return 0.7f;
        case ItemQuality::Normal: return 1.0f;
        case ItemQuality::Superior: return 1.15f;
        case ItemQuality::Perfect: return 1.3f;
        default: return 1.0f;
    }
}

ItemAffix ItemGenerator::SelectRandomAffix(const std::vector<ItemAffix>& affixes, int level) {
    // Filter affixes by level
    std::vector<ItemAffix> validAffixes;
    for (const auto& affix : affixes) {
        if (level >= affix.minLevel && level <= affix.maxLevel) {
            validAffixes.push_back(affix);
        }
    }

    if (validAffixes.empty()) {
        return affixes[0]; // Fallback
    }

    // Select based on weight
    float totalWeight = 0.0f;
    for (const auto& affix : validAffixes) {
        totalWeight += affix.weight;
    }

    float roll = RandomFloat(0.0f, totalWeight);
    float currentWeight = 0.0f;

    for (const auto& affix : validAffixes) {
        currentWeight += affix.weight;
        if (roll <= currentWeight) {
            return affix;
        }
    }

    return validAffixes[0];
}

int ItemGenerator::GenerateStatValue(int baseValue, int level, ItemRarity rarity, ItemQuality quality) {
    float scale = 1.0f + (level * 0.1f) + (static_cast<int>(rarity) * 0.3f);
    float qualityMult = GetQualityMultiplier(quality);

    int value = static_cast<int>(baseValue * scale * qualityMult);
    return std::max(1, value);
}

int ItemGenerator::RandomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_rng);
}

float ItemGenerator::RandomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_rng);
}

bool ItemGenerator::RandomChance(float probability) {
    return RandomFloat(0.0f, 1.0f) < probability;
}

void ItemGenerator::RegisterLootTable(const LootTable& table) {
    m_lootTables[table.id] = table;
}

const LootTable* ItemGenerator::GetLootTable(const std::string& id) const {
    auto it = m_lootTables.find(id);
    return (it != m_lootTables.end()) ? &it->second : nullptr;
}

// CraftingSystem implementation
CraftingSystem::CraftingSystem() {
}

CraftingSystem::~CraftingSystem() {
}

void CraftingSystem::RegisterRecipe(const CraftingRecipe& recipe) {
    m_recipes[recipe.id] = recipe;
}

bool CraftingSystem::CanCraft(const CraftingRecipe& recipe, InventorySystem& inventory, int playerLevel) {
    if (playerLevel < recipe.requiredLevel) return false;
    if (!inventory.CanAfford(recipe.craftingCost)) return false;

    // Check materials
    for (const auto& [materialId, required] : recipe.materials) {
        int count = 0;
        auto items = inventory.FindAllItems(materialId);
        for (const auto& item : items) {
            count += item->GetStackCount();
        }

        if (count < required) return false;
    }

    return true;
}

std::shared_ptr<Item> CraftingSystem::Craft(const CraftingRecipe& recipe,
                                            InventorySystem& inventory,
                                            ItemGenerator& generator) {
    if (!CanCraft(recipe, inventory, 1)) return nullptr;

    // Consume materials
    if (!ConsumeMaterials(recipe, inventory)) return nullptr;

    // Pay cost
    inventory.RemoveGold(recipe.craftingCost);

    // Generate result item
    ItemGenerationParams params;
    params.level = recipe.requiredLevel;
    params.minRarity = recipe.resultRarity;
    params.maxRarity = recipe.resultRarity;

    return generator.GenerateItem(params);
}

const CraftingSystem::CraftingRecipe* CraftingSystem::GetRecipe(const std::string& id) const {
    auto it = m_recipes.find(id);
    return (it != m_recipes.end()) ? &it->second : nullptr;
}

std::vector<const CraftingSystem::CraftingRecipe*> CraftingSystem::GetAvailableRecipes(int playerLevel) const {
    std::vector<const CraftingRecipe*> available;

    for (const auto& [_, recipe] : m_recipes) {
        if (playerLevel >= recipe.requiredLevel) {
            available.push_back(&recipe);
        }
    }

    return available;
}

bool CraftingSystem::ConsumeMaterials(const CraftingRecipe& recipe, InventorySystem& inventory) {
    for (const auto& [materialId, required] : recipe.materials) {
        int remaining = required;

        for (int y = 0; y < inventory.GetHeight() && remaining > 0; y++) {
            for (int x = 0; x < inventory.GetWidth() && remaining > 0; x++) {
                auto item = inventory.GetItem(x, y);
                if (item && item->GetId() == materialId) {
                    int toRemove = std::min(remaining, item->GetStackCount());
                    item->RemoveFromStack(toRemove);

                    if (item->GetStackCount() == 0) {
                        inventory.RemoveItem(x, y);
                    }

                    remaining -= toRemove;
                }
            }
        }

        if (remaining > 0) return false;
    }

    return true;
}

// ItemUpgradeSystem implementation
ItemUpgradeSystem::ItemUpgradeSystem() {
}

ItemUpgradeSystem::~ItemUpgradeSystem() {
}

bool ItemUpgradeSystem::UpgradeItem(std::shared_ptr<Item> item, int materialCost) {
    if (!item) return false;

    int currentLevel = GetUpgradeLevel(item);
    if (currentLevel >= MAX_UPGRADE_LEVEL) return false;

    // Calculate success chance
    float successChance = GetUpgradeSuccessChance(item);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    if (dist(gen) > successChance) {
        return false; // Upgrade failed
    }

    // Increase stats
    ItemStats current = item->GetBaseStats();
    ItemStats increased;

    increased.physicalDamage = static_cast<int>(current.physicalDamage * 1.1f);
    increased.magicDamage = static_cast<int>(current.magicDamage * 1.1f);
    increased.armor = static_cast<int>(current.armor * 1.1f);
    increased.health = static_cast<int>(current.health * 1.1f);

    item->SetBaseStats(increased);
    m_upgradeLevels[item] = currentLevel + 1;

    return true;
}

int ItemUpgradeSystem::GetUpgradeLevel(std::shared_ptr<Item> item) const {
    auto it = m_upgradeLevels.find(item);
    return (it != m_upgradeLevels.end()) ? it->second : 0;
}

int ItemUpgradeSystem::GetUpgradeCost(std::shared_ptr<Item> item) const {
    int level = GetUpgradeLevel(item);
    return 100 * (level + 1) * (level + 1);
}

float ItemUpgradeSystem::GetUpgradeSuccessChance(std::shared_ptr<Item> item) const {
    int level = GetUpgradeLevel(item);
    float baseChance = 1.0f;

    if (level < 5) baseChance = 0.95f;
    else if (level < 10) baseChance = 0.75f;
    else if (level < 15) baseChance = 0.50f;
    else baseChance = 0.25f;

    return baseChance;
}

bool ItemUpgradeSystem::AddSocketToItem(std::shared_ptr<Item> item, int cost) {
    if (!item) return false;
    return item->AddSocket();
}

} // namespace Inventory
