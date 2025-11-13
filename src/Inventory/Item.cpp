#include "Item.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace Inventory {

// ItemStats implementation
ItemStats ItemStats::operator+(const ItemStats& other) const {
    ItemStats result = *this;
    result += other;
    return result;
}

ItemStats& ItemStats::operator+=(const ItemStats& other) {
    strength += other.strength;
    dexterity += other.dexterity;
    intelligence += other.intelligence;
    vitality += other.vitality;
    wisdom += other.wisdom;
    luck += other.luck;

    physicalDamage += other.physicalDamage;
    magicDamage += other.magicDamage;
    armor += other.armor;
    magicResist += other.magicResist;
    critChance += other.critChance;
    critDamage += other.critDamage;
    attackSpeed += other.attackSpeed;

    blockChance += other.blockChance;
    dodgeChance += other.dodgeChance;
    parryChance += other.parryChance;

    health += other.health;
    mana += other.mana;
    healthRegen += other.healthRegen;
    manaRegen += other.manaRegen;

    fireDamage += other.fireDamage;
    iceDamage += other.iceDamage;
    lightningDamage += other.lightningDamage;
    poisonDamage += other.poisonDamage;
    holyDamage += other.holyDamage;
    darkDamage += other.darkDamage;

    fireResist += other.fireResist;
    iceResist += other.iceResist;
    lightningResist += other.lightningResist;
    poisonResist += other.poisonResist;
    holyResist += other.holyResist;
    darkResist += other.darkResist;

    movementSpeed += other.movementSpeed;
    goldFind += other.goldFind;
    magicFind += other.magicFind;
    experienceGain += other.experienceGain;

    return *this;
}

// Item implementation
Item::Item()
    : m_id(""), m_name("Unknown Item"), m_description(""),
      m_type(ItemType::Misc), m_rarity(ItemRarity::Common),
      m_level(1), m_value(0), m_weight(0.0f),
      m_maxStackSize(1), m_stackCount(1),
      m_isEquippable(false), m_isQuestItem(false),
      m_isTradeable(true), m_isUnique(false),
      m_weaponType(WeaponType::Sword), m_armorType(ArmorType::Light),
      m_socketCount(0), m_maxSockets(0),
      m_levelRequirement(1),
      m_durability(100), m_maxDurability(100) {
}

Item::Item(const std::string& id, const std::string& name, ItemType type)
    : m_id(id), m_name(name), m_description(""),
      m_type(type), m_rarity(ItemRarity::Common),
      m_level(1), m_value(0), m_weight(0.0f),
      m_maxStackSize(1), m_stackCount(1),
      m_isEquippable(type != ItemType::Consumable && type != ItemType::CraftingMaterial && type != ItemType::Misc),
      m_isQuestItem(false), m_isTradeable(true), m_isUnique(false),
      m_weaponType(WeaponType::Sword), m_armorType(ArmorType::Light),
      m_socketCount(0), m_maxSockets(0),
      m_levelRequirement(1),
      m_durability(100), m_maxDurability(100) {

    if (type == ItemType::Consumable || type == ItemType::CraftingMaterial) {
        m_maxStackSize = 99;
    }
}

Item::~Item() {
}

void Item::AddPrefix(const ItemAffix& affix) {
    int maxPrefixes = GetMaxPrefixes();
    if (m_prefixes.size() < static_cast<size_t>(maxPrefixes)) {
        m_prefixes.push_back(affix);
        CalculateBonusStats();
    }
}

void Item::AddSuffix(const ItemAffix& affix) {
    int maxSuffixes = GetMaxSuffixes();
    if (m_suffixes.size() < static_cast<size_t>(maxSuffixes)) {
        m_suffixes.push_back(affix);
        CalculateBonusStats();
    }
}

int Item::GetMaxPrefixes() const {
    switch (m_rarity) {
        case ItemRarity::Common: return 0;
        case ItemRarity::Uncommon: return 1;
        case ItemRarity::Rare: return 1;
        case ItemRarity::Epic: return 2;
        case ItemRarity::Legendary: return 2;
        case ItemRarity::Mythic: return 3;
        default: return 0;
    }
}

int Item::GetMaxSuffixes() const {
    switch (m_rarity) {
        case ItemRarity::Common: return 0;
        case ItemRarity::Uncommon: return 1;
        case ItemRarity::Rare: return 2;
        case ItemRarity::Epic: return 2;
        case ItemRarity::Legendary: return 3;
        case ItemRarity::Mythic: return 3;
        default: return 0;
    }
}

bool Item::IsTwoHanded() const {
    if (m_type != ItemType::Weapon) return false;

    return m_weaponType == WeaponType::TwoHandedSword ||
           m_weaponType == WeaponType::TwoHandedAxe ||
           m_weaponType == WeaponType::TwoHandedMace ||
           m_weaponType == WeaponType::Bow ||
           m_weaponType == WeaponType::Crossbow ||
           m_weaponType == WeaponType::Staff;
}

bool Item::AddSocket() {
    if (m_socketCount < m_maxSockets) {
        m_socketCount++;
        return true;
    }
    return false;
}

bool Item::SocketGem(std::shared_ptr<Item> gem) {
    if (!gem || m_socketedGems.size() >= static_cast<size_t>(m_socketCount)) {
        return false;
    }

    m_socketedGems.push_back(gem);
    CalculateBonusStats();
    return true;
}

bool Item::UnsocketGem(int socketIndex) {
    if (socketIndex < 0 || socketIndex >= static_cast<int>(m_socketedGems.size())) {
        return false;
    }

    m_socketedGems.erase(m_socketedGems.begin() + socketIndex);
    CalculateBonusStats();
    return true;
}

bool Item::CanStack(const Item& other) const {
    if (!IsStackable() || !other.IsStackable()) {
        return false;
    }

    // Must be same item ID and have identical properties
    return m_id == other.m_id &&
           m_rarity == other.m_rarity &&
           m_level == other.m_level &&
           m_prefixes.size() == other.m_prefixes.size() &&
           m_suffixes.size() == other.m_suffixes.size();
}

bool Item::AddToStack(int count) {
    if (count <= 0) return false;

    int newCount = m_stackCount + count;
    if (newCount <= m_maxStackSize) {
        m_stackCount = newCount;
        return true;
    }

    m_stackCount = m_maxStackSize;
    return false;
}

bool Item::RemoveFromStack(int count) {
    if (count <= 0 || count > m_stackCount) return false;

    m_stackCount -= count;
    return true;
}

void Item::AddStatRequirement(const std::string& stat, int value) {
    m_statRequirements[stat] = value;
}

void Item::DamageDurability(int amount) {
    m_durability = std::max(0, m_durability - amount);
}

void Item::RepairDurability(int amount) {
    m_durability = std::min(m_maxDurability, m_durability + amount);
}

float Item::GetDurabilityPercent() const {
    if (m_maxDurability <= 0) return 0.0f;
    return static_cast<float>(m_durability) / static_cast<float>(m_maxDurability);
}

std::string Item::GetFullName() const {
    std::stringstream ss;

    // Add prefixes
    for (const auto& prefix : m_prefixes) {
        ss << prefix.name << " ";
    }

    // Add base name
    ss << m_name;

    // Add suffixes
    for (const auto& suffix : m_suffixes) {
        ss << " " << suffix.name;
    }

    return ss.str();
}

std::string Item::GetRarityColor() const {
    switch (m_rarity) {
        case ItemRarity::Common: return "#FFFFFF";     // White
        case ItemRarity::Uncommon: return "#00FF00";   // Green
        case ItemRarity::Rare: return "#0080FF";       // Blue
        case ItemRarity::Epic: return "#A040FF";       // Purple
        case ItemRarity::Legendary: return "#FF8000";  // Orange
        case ItemRarity::Mythic: return "#FF0080";     // Pink
        default: return "#FFFFFF";
    }
}

std::string Item::GetTooltip() const {
    std::stringstream ss;

    // Name and rarity
    ss << GetFullName() << "\n";
    ss << ItemRarityToString(m_rarity) << " " << ItemTypeToString(m_type) << "\n";

    // Level and requirements
    if (m_levelRequirement > 1) {
        ss << "Requires Level " << m_levelRequirement << "\n";
    }

    // Type specific info
    if (m_type == ItemType::Weapon) {
        ss << WeaponTypeToString(m_weaponType);
        if (IsTwoHanded()) ss << " (Two-Handed)";
        ss << "\n";
    } else if (m_type == ItemType::Helmet || m_type == ItemType::Chest ||
               m_type == ItemType::Legs || m_type == ItemType::Boots) {
        ss << ArmorTypeToString(m_armorType) << " Armor\n";
    }

    ss << "\n";

    // Stats
    ItemStats totalStats = GetTotalStats();

    if (totalStats.physicalDamage > 0)
        ss << "Physical Damage: " << totalStats.physicalDamage << "\n";
    if (totalStats.magicDamage > 0)
        ss << "Magic Damage: " << totalStats.magicDamage << "\n";
    if (totalStats.armor > 0)
        ss << "Armor: " << totalStats.armor << "\n";
    if (totalStats.magicResist > 0)
        ss << "Magic Resist: " << totalStats.magicResist << "\n";

    // Primary stats
    if (totalStats.strength > 0)
        ss << "+" << totalStats.strength << " Strength\n";
    if (totalStats.dexterity > 0)
        ss << "+" << totalStats.dexterity << " Dexterity\n";
    if (totalStats.intelligence > 0)
        ss << "+" << totalStats.intelligence << " Intelligence\n";
    if (totalStats.vitality > 0)
        ss << "+" << totalStats.vitality << " Vitality\n";
    if (totalStats.wisdom > 0)
        ss << "+" << totalStats.wisdom << " Wisdom\n";
    if (totalStats.luck > 0)
        ss << "+" << totalStats.luck << " Luck\n";

    // Combat stats
    if (totalStats.critChance > 0)
        ss << "+" << totalStats.critChance << "% Critical Hit Chance\n";
    if (totalStats.critDamage > 0)
        ss << "+" << totalStats.critDamage << "% Critical Hit Damage\n";
    if (totalStats.attackSpeed > 0)
        ss << "+" << totalStats.attackSpeed << "% Attack Speed\n";

    // Resource stats
    if (totalStats.health > 0)
        ss << "+" << totalStats.health << " Health\n";
    if (totalStats.mana > 0)
        ss << "+" << totalStats.mana << " Mana\n";

    // Elemental damage
    if (totalStats.fireDamage > 0)
        ss << "+" << totalStats.fireDamage << " Fire Damage\n";
    if (totalStats.iceDamage > 0)
        ss << "+" << totalStats.iceDamage << " Ice Damage\n";
    if (totalStats.lightningDamage > 0)
        ss << "+" << totalStats.lightningDamage << " Lightning Damage\n";
    if (totalStats.poisonDamage > 0)
        ss << "+" << totalStats.poisonDamage << " Poison Damage\n";

    // Special stats
    if (totalStats.magicFind > 0)
        ss << "+" << totalStats.magicFind << "% Magic Find\n";
    if (totalStats.goldFind > 0)
        ss << "+" << totalStats.goldFind << "% Gold Find\n";

    // Sockets
    if (m_maxSockets > 0) {
        ss << "\nSockets: " << m_socketedGems.size() << "/" << m_socketCount << " (" << m_maxSockets << " max)\n";
    }

    // Set info
    if (IsSetItem()) {
        ss << "\nSet: " << m_setId << "\n";
    }

    // Special effects
    if (!m_specialEffect.empty()) {
        ss << "\n" << m_specialEffect << "\n";
    }

    // Description
    if (!m_description.empty()) {
        ss << "\n" << m_description << "\n";
    }

    // Value and weight
    ss << "\nValue: " << m_value << " gold";
    if (m_weight > 0) {
        ss << " | Weight: " << m_weight << " lbs";
    }

    return ss.str();
}

int Item::CompareTo(const Item& other) const {
    // Compare by rarity first
    if (m_rarity != other.m_rarity) {
        return static_cast<int>(m_rarity) - static_cast<int>(other.m_rarity);
    }

    // Then by level
    if (m_level != other.m_level) {
        return m_level - other.m_level;
    }

    // Then by value
    return m_value - other.m_value;
}

std::shared_ptr<Item> Item::Clone() const {
    auto clone = std::make_shared<Item>();

    clone->m_id = m_id;
    clone->m_name = m_name;
    clone->m_description = m_description;
    clone->m_type = m_type;
    clone->m_rarity = m_rarity;
    clone->m_level = m_level;
    clone->m_value = m_value;
    clone->m_weight = m_weight;

    clone->m_maxStackSize = m_maxStackSize;
    clone->m_stackCount = m_stackCount;

    clone->m_isEquippable = m_isEquippable;
    clone->m_isQuestItem = m_isQuestItem;
    clone->m_isTradeable = m_isTradeable;
    clone->m_isUnique = m_isUnique;

    clone->m_baseStats = m_baseStats;
    clone->m_bonusStats = m_bonusStats;

    clone->m_prefixes = m_prefixes;
    clone->m_suffixes = m_suffixes;

    clone->m_weaponType = m_weaponType;
    clone->m_armorType = m_armorType;

    clone->m_setId = m_setId;

    clone->m_socketCount = m_socketCount;
    clone->m_maxSockets = m_maxSockets;

    clone->m_levelRequirement = m_levelRequirement;
    clone->m_statRequirements = m_statRequirements;

    clone->m_iconPath = m_iconPath;
    clone->m_modelPath = m_modelPath;

    clone->m_specialEffect = m_specialEffect;
    clone->m_onUseEffect = m_onUseEffect;

    clone->m_durability = m_durability;
    clone->m_maxDurability = m_maxDurability;

    return clone;
}

void Item::CalculateBonusStats() {
    m_bonusStats = ItemStats();

    // Add prefix stats
    for (const auto& prefix : m_prefixes) {
        m_bonusStats += prefix.stats;
    }

    // Add suffix stats
    for (const auto& suffix : m_suffixes) {
        m_bonusStats += suffix.stats;
    }

    // Add socketed gem stats
    for (const auto& gem : m_socketedGems) {
        if (gem) {
            m_bonusStats += gem->GetTotalStats();
        }
    }
}

// Helper function implementations
std::string ItemRarityToString(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::Common: return "Common";
        case ItemRarity::Uncommon: return "Uncommon";
        case ItemRarity::Rare: return "Rare";
        case ItemRarity::Epic: return "Epic";
        case ItemRarity::Legendary: return "Legendary";
        case ItemRarity::Mythic: return "Mythic";
        default: return "Unknown";
    }
}

std::string ItemTypeToString(ItemType type) {
    switch (type) {
        case ItemType::Weapon: return "Weapon";
        case ItemType::Helmet: return "Helmet";
        case ItemType::Chest: return "Chest Armor";
        case ItemType::Legs: return "Leg Armor";
        case ItemType::Boots: return "Boots";
        case ItemType::Accessory: return "Accessory";
        case ItemType::Ring: return "Ring";
        case ItemType::Amulet: return "Amulet";
        case ItemType::Consumable: return "Consumable";
        case ItemType::CraftingMaterial: return "Crafting Material";
        case ItemType::QuestItem: return "Quest Item";
        case ItemType::Misc: return "Miscellaneous";
        default: return "Unknown";
    }
}

std::string WeaponTypeToString(WeaponType type) {
    switch (type) {
        case WeaponType::Sword: return "Sword";
        case WeaponType::Axe: return "Axe";
        case WeaponType::Mace: return "Mace";
        case WeaponType::Dagger: return "Dagger";
        case WeaponType::Bow: return "Bow";
        case WeaponType::Crossbow: return "Crossbow";
        case WeaponType::Staff: return "Staff";
        case WeaponType::Wand: return "Wand";
        case WeaponType::Spear: return "Spear";
        case WeaponType::TwoHandedSword: return "Two-Handed Sword";
        case WeaponType::TwoHandedAxe: return "Two-Handed Axe";
        case WeaponType::TwoHandedMace: return "Two-Handed Mace";
        default: return "Unknown";
    }
}

std::string ArmorTypeToString(ArmorType type) {
    switch (type) {
        case ArmorType::Light: return "Light";
        case ArmorType::Medium: return "Medium";
        case ArmorType::Heavy: return "Heavy";
        case ArmorType::Cloth: return "Cloth";
        default: return "Unknown";
    }
}

ItemRarity StringToItemRarity(const std::string& str) {
    if (str == "Common") return ItemRarity::Common;
    if (str == "Uncommon") return ItemRarity::Uncommon;
    if (str == "Rare") return ItemRarity::Rare;
    if (str == "Epic") return ItemRarity::Epic;
    if (str == "Legendary") return ItemRarity::Legendary;
    if (str == "Mythic") return ItemRarity::Mythic;
    return ItemRarity::Common;
}

ItemType StringToItemType(const std::string& str) {
    if (str == "Weapon") return ItemType::Weapon;
    if (str == "Helmet") return ItemType::Helmet;
    if (str == "Chest" || str == "Chest Armor") return ItemType::Chest;
    if (str == "Legs" || str == "Leg Armor") return ItemType::Legs;
    if (str == "Boots") return ItemType::Boots;
    if (str == "Accessory") return ItemType::Accessory;
    if (str == "Ring") return ItemType::Ring;
    if (str == "Amulet") return ItemType::Amulet;
    if (str == "Consumable") return ItemType::Consumable;
    if (str == "CraftingMaterial" || str == "Crafting Material") return ItemType::CraftingMaterial;
    if (str == "QuestItem" || str == "Quest Item") return ItemType::QuestItem;
    if (str == "Misc" || str == "Miscellaneous") return ItemType::Misc;
    return ItemType::Misc;
}

WeaponType StringToWeaponType(const std::string& str) {
    if (str == "Sword") return WeaponType::Sword;
    if (str == "Axe") return WeaponType::Axe;
    if (str == "Mace") return WeaponType::Mace;
    if (str == "Dagger") return WeaponType::Dagger;
    if (str == "Bow") return WeaponType::Bow;
    if (str == "Crossbow") return WeaponType::Crossbow;
    if (str == "Staff") return WeaponType::Staff;
    if (str == "Wand") return WeaponType::Wand;
    if (str == "Spear") return WeaponType::Spear;
    if (str == "TwoHandedSword" || str == "Two-Handed Sword") return WeaponType::TwoHandedSword;
    if (str == "TwoHandedAxe" || str == "Two-Handed Axe") return WeaponType::TwoHandedAxe;
    if (str == "TwoHandedMace" || str == "Two-Handed Mace") return WeaponType::TwoHandedMace;
    return WeaponType::Sword;
}

ArmorType StringToArmorType(const std::string& str) {
    if (str == "Light") return ArmorType::Light;
    if (str == "Medium") return ArmorType::Medium;
    if (str == "Heavy") return ArmorType::Heavy;
    if (str == "Cloth") return ArmorType::Cloth;
    return ArmorType::Light;
}

} // namespace Inventory
