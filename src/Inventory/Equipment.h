#pragma once

#include "Item.h"
#include <map>
#include <memory>
#include <functional>

namespace Inventory {

// Equipment slot types
enum class EquipmentSlot {
    MainHand,
    OffHand,
    Helmet,
    Chest,
    Legs,
    Boots,
    Gloves,
    Belt,
    Amulet,
    Ring1,
    Ring2,
    Earring1,
    Earring2,
    Cape,
    Pet,
    Mount
};

// Equipment manager
class EquipmentManager {
public:
    EquipmentManager();
    ~EquipmentManager();

    // Equip/Unequip
    bool EquipItem(std::shared_ptr<Item> item);
    bool EquipItemToSlot(std::shared_ptr<Item> item, EquipmentSlot slot);
    bool UnequipSlot(EquipmentSlot slot);
    bool UnequipItem(std::shared_ptr<Item> item);
    void UnequipAll();

    // Swap equipment
    bool SwapEquipment(EquipmentSlot slot1, EquipmentSlot slot2);

    // Queries
    std::shared_ptr<Item> GetEquippedItem(EquipmentSlot slot) const;
    bool IsSlotOccupied(EquipmentSlot slot) const;
    bool IsItemEquipped(std::shared_ptr<Item> item) const;
    EquipmentSlot GetItemSlot(std::shared_ptr<Item> item) const;

    // Get all equipped items
    std::map<EquipmentSlot, std::shared_ptr<Item>> GetAllEquipment() const;
    std::vector<std::shared_ptr<Item>> GetEquippedItems() const;

    // Stats
    ItemStats GetTotalStats() const;
    ItemStats GetEquipmentStats(EquipmentSlot slot) const;

    // Set bonuses
    std::map<std::string, int> GetActiveSets() const;
    std::vector<SetBonus> GetActiveSetBonuses() const;
    bool HasSetBonus(const std::string& setId, int pieceCount) const;

    // Validation
    bool CanEquip(std::shared_ptr<Item> item) const;
    bool CanEquipToSlot(std::shared_ptr<Item> item, EquipmentSlot slot) const;
    std::string GetEquipValidationError(std::shared_ptr<Item> item) const;

    // Requirements
    void SetPlayerLevel(int level) { m_playerLevel = level; }
    void SetPlayerStats(const ItemStats& stats) { m_playerStats = stats; }
    int GetPlayerLevel() const { return m_playerLevel; }
    const ItemStats& GetPlayerStats() const { return m_playerStats; }

    // Two-handed weapon handling
    bool IsUsingTwoHandedWeapon() const;

    // Durability
    void DamageAllEquipment(int amount);
    void DamageSlot(EquipmentSlot slot, int amount);
    bool HasBrokenEquipment() const;
    std::vector<EquipmentSlot> GetBrokenSlots() const;

    // Callbacks
    using EquipCallback = std::function<void(EquipmentSlot, std::shared_ptr<Item>)>;
    using UnequipCallback = std::function<void(EquipmentSlot, std::shared_ptr<Item>)>;

    void SetEquipCallback(EquipCallback callback) { m_onEquip = callback; }
    void SetUnequipCallback(UnequipCallback callback) { m_onUnequip = callback; }

    // Serialization
    std::string Serialize() const;
    bool Deserialize(const std::string& data);

    // Display
    std::string GetSlotName(EquipmentSlot slot) const;
    std::string GetEquipmentSummary() const;

private:
    std::map<EquipmentSlot, std::shared_ptr<Item>> m_equipment;

    int m_playerLevel;
    ItemStats m_playerStats;

    EquipCallback m_onEquip;
    UnequipCallback m_onUnequip;

    EquipmentSlot GetDefaultSlotForItem(std::shared_ptr<Item> item) const;
    bool MeetsRequirements(std::shared_ptr<Item> item) const;
    void NotifyEquip(EquipmentSlot slot, std::shared_ptr<Item> item);
    void NotifyUnequip(EquipmentSlot slot, std::shared_ptr<Item> item);
};

// Equipment set tracker
class EquipmentSetTracker {
public:
    EquipmentSetTracker();
    ~EquipmentSetTracker();

    // Register sets
    void RegisterSet(const ItemSet& set);
    bool HasSet(const std::string& setId) const;
    const ItemSet* GetSet(const std::string& setId) const;

    // Calculate active sets from equipment
    std::map<std::string, int> CalculateActiveSets(const EquipmentManager& equipment) const;
    std::vector<SetBonus> GetActiveBonuses(const EquipmentManager& equipment) const;
    ItemStats GetSetBonusStats(const EquipmentManager& equipment) const;

    // Display
    std::string GetSetInfo(const std::string& setId, int equippedPieces) const;

private:
    std::map<std::string, ItemSet> m_sets;
};

// Equipment comparison helper
class EquipmentComparator {
public:
    struct ComparisonResult {
        std::shared_ptr<Item> currentItem;
        std::shared_ptr<Item> newItem;
        ItemStats statDifference;
        int overallScore;
        std::string recommendation;
        std::vector<std::string> improvements;
        std::vector<std::string> downgrades;
    };

    static ComparisonResult Compare(std::shared_ptr<Item> current, std::shared_ptr<Item> newItem);
    static ComparisonResult CompareWithSlot(const EquipmentManager& equipment,
                                           std::shared_ptr<Item> newItem,
                                           EquipmentSlot slot);

    // Set weight for different stats (for scoring)
    static void SetStatWeight(const std::string& statName, float weight);
    static float GetStatWeight(const std::string& statName);

private:
    static std::map<std::string, float> s_statWeights;
    static int CalculateItemScore(std::shared_ptr<Item> item);
    static ItemStats CalculateStatDifference(std::shared_ptr<Item> current, std::shared_ptr<Item> newItem);
};

// Helper functions
std::string EquipmentSlotToString(EquipmentSlot slot);
EquipmentSlot StringToEquipmentSlot(const std::string& str);

} // namespace Inventory
