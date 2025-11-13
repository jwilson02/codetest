#include "Equipment.h"
#include <sstream>
#include <algorithm>

namespace Inventory {

// EquipmentManager implementation
EquipmentManager::EquipmentManager()
    : m_playerLevel(1) {
}

EquipmentManager::~EquipmentManager() {
}

bool EquipmentManager::EquipItem(std::shared_ptr<Item> item) {
    if (!item || !CanEquip(item)) return false;

    EquipmentSlot slot = GetDefaultSlotForItem(item);
    return EquipItemToSlot(item, slot);
}

bool EquipmentManager::EquipItemToSlot(std::shared_ptr<Item> item, EquipmentSlot slot) {
    if (!item || !CanEquipToSlot(item, slot)) return false;

    // If two-handed weapon, unequip offhand
    if (item->IsTwoHanded() && slot == EquipmentSlot::MainHand) {
        UnequipSlot(EquipmentSlot::OffHand);
    }

    // If equipping offhand and main hand has two-handed weapon, unequip main hand
    if (slot == EquipmentSlot::OffHand && IsUsingTwoHandedWeapon()) {
        UnequipSlot(EquipmentSlot::MainHand);
    }

    // Unequip current item if slot is occupied
    if (IsSlotOccupied(slot)) {
        UnequipSlot(slot);
    }

    m_equipment[slot] = item;
    NotifyEquip(slot, item);
    return true;
}

bool EquipmentManager::UnequipSlot(EquipmentSlot slot) {
    auto it = m_equipment.find(slot);
    if (it == m_equipment.end()) return false;

    auto item = it->second;
    m_equipment.erase(it);
    NotifyUnequip(slot, item);
    return true;
}

bool EquipmentManager::UnequipItem(std::shared_ptr<Item> item) {
    for (auto& [slot, equippedItem] : m_equipment) {
        if (equippedItem == item) {
            return UnequipSlot(slot);
        }
    }
    return false;
}

void EquipmentManager::UnequipAll() {
    std::vector<EquipmentSlot> slots;
    for (const auto& [slot, _] : m_equipment) {
        slots.push_back(slot);
    }

    for (auto slot : slots) {
        UnequipSlot(slot);
    }
}

bool EquipmentManager::SwapEquipment(EquipmentSlot slot1, EquipmentSlot slot2) {
    auto item1 = GetEquippedItem(slot1);
    auto item2 = GetEquippedItem(slot2);

    if (!item1 && !item2) return false;

    if (item1) UnequipSlot(slot1);
    if (item2) UnequipSlot(slot2);

    if (item1) EquipItemToSlot(item1, slot2);
    if (item2) EquipItemToSlot(item2, slot1);

    return true;
}

std::shared_ptr<Item> EquipmentManager::GetEquippedItem(EquipmentSlot slot) const {
    auto it = m_equipment.find(slot);
    return (it != m_equipment.end()) ? it->second : nullptr;
}

bool EquipmentManager::IsSlotOccupied(EquipmentSlot slot) const {
    return m_equipment.find(slot) != m_equipment.end();
}

bool EquipmentManager::IsItemEquipped(std::shared_ptr<Item> item) const {
    for (const auto& [_, equippedItem] : m_equipment) {
        if (equippedItem == item) return true;
    }
    return false;
}

EquipmentSlot EquipmentManager::GetItemSlot(std::shared_ptr<Item> item) const {
    for (const auto& [slot, equippedItem] : m_equipment) {
        if (equippedItem == item) return slot;
    }
    return EquipmentSlot::MainHand; // Default
}

std::map<EquipmentSlot, std::shared_ptr<Item>> EquipmentManager::GetAllEquipment() const {
    return m_equipment;
}

std::vector<std::shared_ptr<Item>> EquipmentManager::GetEquippedItems() const {
    std::vector<std::shared_ptr<Item>> items;
    for (const auto& [_, item] : m_equipment) {
        items.push_back(item);
    }
    return items;
}

ItemStats EquipmentManager::GetTotalStats() const {
    ItemStats total;

    for (const auto& [_, item] : m_equipment) {
        if (item) {
            total += item->GetTotalStats();
        }
    }

    return total;
}

ItemStats EquipmentManager::GetEquipmentStats(EquipmentSlot slot) const {
    auto item = GetEquippedItem(slot);
    return item ? item->GetTotalStats() : ItemStats();
}

std::map<std::string, int> EquipmentManager::GetActiveSets() const {
    std::map<std::string, int> sets;

    for (const auto& [_, item] : m_equipment) {
        if (item && item->IsSetItem()) {
            sets[item->GetSetId()]++;
        }
    }

    return sets;
}

std::vector<SetBonus> EquipmentManager::GetActiveSetBonuses() const {
    // This would need access to ItemDatabase to get set definitions
    // For now, return empty vector
    return std::vector<SetBonus>();
}

bool EquipmentManager::HasSetBonus(const std::string& setId, int pieceCount) const {
    auto sets = GetActiveSets();
    auto it = sets.find(setId);
    return it != sets.end() && it->second >= pieceCount;
}

bool EquipmentManager::CanEquip(std::shared_ptr<Item> item) const {
    if (!item || !item->IsEquippable()) return false;
    return MeetsRequirements(item);
}

bool EquipmentManager::CanEquipToSlot(std::shared_ptr<Item> item, EquipmentSlot slot) const {
    if (!CanEquip(item)) return false;

    // Check if item type matches slot
    ItemType type = item->GetType();

    switch (slot) {
        case EquipmentSlot::MainHand:
        case EquipmentSlot::OffHand:
            return type == ItemType::Weapon;

        case EquipmentSlot::Helmet:
            return type == ItemType::Helmet;

        case EquipmentSlot::Chest:
            return type == ItemType::Chest;

        case EquipmentSlot::Legs:
            return type == ItemType::Legs;

        case EquipmentSlot::Boots:
            return type == ItemType::Boots;

        case EquipmentSlot::Ring1:
        case EquipmentSlot::Ring2:
            return type == ItemType::Ring;

        case EquipmentSlot::Amulet:
            return type == ItemType::Amulet;

        case EquipmentSlot::Gloves:
        case EquipmentSlot::Belt:
        case EquipmentSlot::Cape:
        case EquipmentSlot::Earring1:
        case EquipmentSlot::Earring2:
            return type == ItemType::Accessory;

        default:
            return false;
    }
}

std::string EquipmentManager::GetEquipValidationError(std::shared_ptr<Item> item) const {
    if (!item) return "Invalid item";
    if (!item->IsEquippable()) return "Item is not equippable";

    if (item->GetLevelRequirement() > m_playerLevel) {
        return "Level requirement not met (Requires level " +
               std::to_string(item->GetLevelRequirement()) + ")";
    }

    const auto& statReqs = item->GetStatRequirements();
    for (const auto& [stat, value] : statReqs) {
        // Would need to check player stats here
        // Simplified for now
    }

    return "";
}

bool EquipmentManager::IsUsingTwoHandedWeapon() const {
    auto mainHand = GetEquippedItem(EquipmentSlot::MainHand);
    return mainHand && mainHand->IsTwoHanded();
}

void EquipmentManager::DamageAllEquipment(int amount) {
    for (auto& [_, item] : m_equipment) {
        if (item) {
            item->DamageDurability(amount);
        }
    }
}

void EquipmentManager::DamageSlot(EquipmentSlot slot, int amount) {
    auto item = GetEquippedItem(slot);
    if (item) {
        item->DamageDurability(amount);
    }
}

bool EquipmentManager::HasBrokenEquipment() const {
    for (const auto& [_, item] : m_equipment) {
        if (item && item->IsBroken()) {
            return true;
        }
    }
    return false;
}

std::vector<EquipmentSlot> EquipmentManager::GetBrokenSlots() const {
    std::vector<EquipmentSlot> broken;

    for (const auto& [slot, item] : m_equipment) {
        if (item && item->IsBroken()) {
            broken.push_back(slot);
        }
    }

    return broken;
}

std::string EquipmentManager::GetSlotName(EquipmentSlot slot) const {
    return EquipmentSlotToString(slot);
}

std::string EquipmentManager::GetEquipmentSummary() const {
    std::stringstream ss;
    ss << "=== Equipment Summary ===\n\n";

    const std::vector<EquipmentSlot> allSlots = {
        EquipmentSlot::MainHand, EquipmentSlot::OffHand,
        EquipmentSlot::Helmet, EquipmentSlot::Chest,
        EquipmentSlot::Legs, EquipmentSlot::Boots,
        EquipmentSlot::Gloves, EquipmentSlot::Belt,
        EquipmentSlot::Amulet, EquipmentSlot::Ring1,
        EquipmentSlot::Ring2, EquipmentSlot::Cape
    };

    for (auto slot : allSlots) {
        ss << GetSlotName(slot) << ": ";
        auto item = GetEquippedItem(slot);
        if (item) {
            ss << item->GetFullName() << " (" << ItemRarityToString(item->GetRarity()) << ")";
            if (item->IsBroken()) {
                ss << " [BROKEN]";
            }
        } else {
            ss << "[Empty]";
        }
        ss << "\n";
    }

    ss << "\nTotal Stats:\n";
    ItemStats total = GetTotalStats();
    if (total.strength > 0) ss << "  Strength: " << total.strength << "\n";
    if (total.dexterity > 0) ss << "  Dexterity: " << total.dexterity << "\n";
    if (total.intelligence > 0) ss << "  Intelligence: " << total.intelligence << "\n";
    if (total.vitality > 0) ss << "  Vitality: " << total.vitality << "\n";
    if (total.armor > 0) ss << "  Armor: " << total.armor << "\n";
    if (total.physicalDamage > 0) ss << "  Physical Damage: " << total.physicalDamage << "\n";
    if (total.magicDamage > 0) ss << "  Magic Damage: " << total.magicDamage << "\n";

    auto sets = GetActiveSets();
    if (!sets.empty()) {
        ss << "\nActive Sets:\n";
        for (const auto& [setId, count] : sets) {
            ss << "  " << setId << ": " << count << " pieces\n";
        }
    }

    return ss.str();
}

EquipmentSlot EquipmentManager::GetDefaultSlotForItem(std::shared_ptr<Item> item) const {
    if (!item) return EquipmentSlot::MainHand;

    switch (item->GetType()) {
        case ItemType::Weapon:
            return EquipmentSlot::MainHand;
        case ItemType::Helmet:
            return EquipmentSlot::Helmet;
        case ItemType::Chest:
            return EquipmentSlot::Chest;
        case ItemType::Legs:
            return EquipmentSlot::Legs;
        case ItemType::Boots:
            return EquipmentSlot::Boots;
        case ItemType::Ring:
            return IsSlotOccupied(EquipmentSlot::Ring1) ? EquipmentSlot::Ring2 : EquipmentSlot::Ring1;
        case ItemType::Amulet:
            return EquipmentSlot::Amulet;
        case ItemType::Accessory:
            return EquipmentSlot::Belt;
        default:
            return EquipmentSlot::MainHand;
    }
}

bool EquipmentManager::MeetsRequirements(std::shared_ptr<Item> item) const {
    if (!item) return false;

    // Check level requirement
    if (item->GetLevelRequirement() > m_playerLevel) {
        return false;
    }

    // Check stat requirements
    const auto& statReqs = item->GetStatRequirements();
    for (const auto& [stat, required] : statReqs) {
        // Would check player stats here
        // Simplified for now
    }

    return true;
}

void EquipmentManager::NotifyEquip(EquipmentSlot slot, std::shared_ptr<Item> item) {
    if (m_onEquip) {
        m_onEquip(slot, item);
    }
}

void EquipmentManager::NotifyUnequip(EquipmentSlot slot, std::shared_ptr<Item> item) {
    if (m_onUnequip) {
        m_onUnequip(slot, item);
    }
}

// EquipmentSetTracker implementation
EquipmentSetTracker::EquipmentSetTracker() {
}

EquipmentSetTracker::~EquipmentSetTracker() {
}

void EquipmentSetTracker::RegisterSet(const ItemSet& set) {
    m_sets[set.setName] = set;
}

bool EquipmentSetTracker::HasSet(const std::string& setId) const {
    return m_sets.find(setId) != m_sets.end();
}

const ItemSet* EquipmentSetTracker::GetSet(const std::string& setId) const {
    auto it = m_sets.find(setId);
    return (it != m_sets.end()) ? &it->second : nullptr;
}

std::map<std::string, int> EquipmentSetTracker::CalculateActiveSets(const EquipmentManager& equipment) const {
    return equipment.GetActiveSets();
}

std::vector<SetBonus> EquipmentSetTracker::GetActiveBonuses(const EquipmentManager& equipment) const {
    std::vector<SetBonus> bonuses;
    auto activeSets = equipment.GetActiveSets();

    for (const auto& [setId, equippedCount] : activeSets) {
        const ItemSet* set = GetSet(setId);
        if (!set) continue;

        for (const auto& bonus : set->bonuses) {
            if (equippedCount >= bonus.piecesRequired) {
                bonuses.push_back(bonus);
            }
        }
    }

    return bonuses;
}

ItemStats EquipmentSetTracker::GetSetBonusStats(const EquipmentManager& equipment) const {
    ItemStats total;
    auto bonuses = GetActiveBonuses(equipment);

    for (const auto& bonus : bonuses) {
        total += bonus.bonusStats;
    }

    return total;
}

std::string EquipmentSetTracker::GetSetInfo(const std::string& setId, int equippedPieces) const {
    const ItemSet* set = GetSet(setId);
    if (!set) return "";

    std::stringstream ss;
    ss << set->setName << " (" << equippedPieces << "/" << set->itemIds.size() << ")\n";

    for (const auto& bonus : set->bonuses) {
        ss << "  (" << bonus.piecesRequired << ") " << bonus.bonusDescription;
        if (equippedPieces >= bonus.piecesRequired) {
            ss << " [ACTIVE]";
        }
        ss << "\n";
    }

    return ss.str();
}

// EquipmentComparator implementation
std::map<std::string, float> EquipmentComparator::s_statWeights = {
    {"physicalDamage", 1.0f},
    {"magicDamage", 1.0f},
    {"armor", 0.8f},
    {"health", 0.5f},
    {"strength", 0.7f},
    {"dexterity", 0.7f},
    {"intelligence", 0.7f},
    {"critChance", 1.2f}
};

EquipmentComparator::ComparisonResult EquipmentComparator::Compare(
    std::shared_ptr<Item> current,
    std::shared_ptr<Item> newItem) {

    ComparisonResult result;
    result.currentItem = current;
    result.newItem = newItem;

    if (!newItem) {
        result.recommendation = "Invalid item";
        return result;
    }

    if (!current) {
        result.overallScore = 100;
        result.recommendation = "Equip this item (slot is empty)";
        return result;
    }

    result.statDifference = CalculateStatDifference(current, newItem);

    int currentScore = CalculateItemScore(current);
    int newScore = CalculateItemScore(newItem);
    result.overallScore = newScore - currentScore;

    // Generate recommendations
    if (result.overallScore > 10) {
        result.recommendation = "Upgrade! This is significantly better.";
    } else if (result.overallScore > 0) {
        result.recommendation = "Slight upgrade.";
    } else if (result.overallScore < -10) {
        result.recommendation = "Downgrade. Keep current equipment.";
    } else {
        result.recommendation = "Similar power level. Personal preference.";
    }

    return result;
}

EquipmentComparator::ComparisonResult EquipmentComparator::CompareWithSlot(
    const EquipmentManager& equipment,
    std::shared_ptr<Item> newItem,
    EquipmentSlot slot) {

    auto current = equipment.GetEquippedItem(slot);
    return Compare(current, newItem);
}

void EquipmentComparator::SetStatWeight(const std::string& statName, float weight) {
    s_statWeights[statName] = weight;
}

float EquipmentComparator::GetStatWeight(const std::string& statName) {
    auto it = s_statWeights.find(statName);
    return (it != s_statWeights.end()) ? it->second : 1.0f;
}

int EquipmentComparator::CalculateItemScore(std::shared_ptr<Item> item) {
    if (!item) return 0;

    int score = 0;
    ItemStats stats = item->GetTotalStats();

    score += stats.physicalDamage * GetStatWeight("physicalDamage");
    score += stats.magicDamage * GetStatWeight("magicDamage");
    score += stats.armor * GetStatWeight("armor");
    score += stats.health * GetStatWeight("health") * 0.1f;
    score += stats.strength * GetStatWeight("strength");
    score += stats.dexterity * GetStatWeight("dexterity");
    score += stats.intelligence * GetStatWeight("intelligence");
    score += stats.critChance * GetStatWeight("critChance");

    // Rarity bonus
    score += static_cast<int>(item->GetRarity()) * 10;

    return score;
}

ItemStats EquipmentComparator::CalculateStatDifference(
    std::shared_ptr<Item> current,
    std::shared_ptr<Item> newItem) {

    ItemStats diff;
    if (!current || !newItem) return diff;

    ItemStats currentStats = current->GetTotalStats();
    ItemStats newStats = newItem->GetTotalStats();

    diff.physicalDamage = newStats.physicalDamage - currentStats.physicalDamage;
    diff.magicDamage = newStats.magicDamage - currentStats.magicDamage;
    diff.armor = newStats.armor - currentStats.armor;
    diff.strength = newStats.strength - currentStats.strength;
    diff.dexterity = newStats.dexterity - currentStats.dexterity;
    diff.intelligence = newStats.intelligence - currentStats.intelligence;
    diff.health = newStats.health - currentStats.health;

    return diff;
}

// Helper functions
std::string EquipmentSlotToString(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::MainHand: return "Main Hand";
        case EquipmentSlot::OffHand: return "Off Hand";
        case EquipmentSlot::Helmet: return "Helmet";
        case EquipmentSlot::Chest: return "Chest";
        case EquipmentSlot::Legs: return "Legs";
        case EquipmentSlot::Boots: return "Boots";
        case EquipmentSlot::Gloves: return "Gloves";
        case EquipmentSlot::Belt: return "Belt";
        case EquipmentSlot::Amulet: return "Amulet";
        case EquipmentSlot::Ring1: return "Ring 1";
        case EquipmentSlot::Ring2: return "Ring 2";
        case EquipmentSlot::Earring1: return "Earring 1";
        case EquipmentSlot::Earring2: return "Earring 2";
        case EquipmentSlot::Cape: return "Cape";
        case EquipmentSlot::Pet: return "Pet";
        case EquipmentSlot::Mount: return "Mount";
        default: return "Unknown";
    }
}

EquipmentSlot StringToEquipmentSlot(const std::string& str) {
    if (str == "Main Hand" || str == "MainHand") return EquipmentSlot::MainHand;
    if (str == "Off Hand" || str == "OffHand") return EquipmentSlot::OffHand;
    if (str == "Helmet") return EquipmentSlot::Helmet;
    if (str == "Chest") return EquipmentSlot::Chest;
    if (str == "Legs") return EquipmentSlot::Legs;
    if (str == "Boots") return EquipmentSlot::Boots;
    if (str == "Gloves") return EquipmentSlot::Gloves;
    if (str == "Belt") return EquipmentSlot::Belt;
    if (str == "Amulet") return EquipmentSlot::Amulet;
    if (str == "Ring 1" || str == "Ring1") return EquipmentSlot::Ring1;
    if (str == "Ring 2" || str == "Ring2") return EquipmentSlot::Ring2;
    if (str == "Cape") return EquipmentSlot::Cape;
    return EquipmentSlot::MainHand;
}

} // namespace Inventory
