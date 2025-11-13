#pragma once

#include "Item.h"
#include <vector>
#include <memory>
#include <functional>
#include <map>

namespace Inventory {

// Inventory slot
struct InventorySlot {
    std::shared_ptr<Item> item;
    int x;
    int y;
    bool locked;

    InventorySlot() : item(nullptr), x(0), y(0), locked(false) {}
    InventorySlot(int px, int py) : item(nullptr), x(px), y(py), locked(false) {}
};

// Sort criteria
enum class SortCriteria {
    Name,
    Type,
    Rarity,
    Level,
    Value,
    Weight
};

// Filter criteria
struct ItemFilter {
    ItemType type;
    ItemRarity minRarity;
    ItemRarity maxRarity;
    int minLevel;
    int maxLevel;
    std::string nameContains;
    bool equippableOnly;

    ItemFilter()
        : type(ItemType::Misc),
          minRarity(ItemRarity::Common),
          maxRarity(ItemRarity::Mythic),
          minLevel(0),
          maxLevel(999),
          nameContains(""),
          equippableOnly(false) {}
};

// Inventory system class
class InventorySystem {
public:
    InventorySystem(int width = 10, int height = 10);
    ~InventorySystem();

    // Basic operations
    bool AddItem(std::shared_ptr<Item> item);
    bool AddItemAt(std::shared_ptr<Item> item, int x, int y);
    bool RemoveItem(int x, int y);
    bool RemoveItem(std::shared_ptr<Item> item);
    bool MoveItem(int fromX, int fromY, int toX, int toY);
    bool SwapItems(int x1, int y1, int x2, int y2);

    // Stack operations
    bool StackItem(std::shared_ptr<Item> item);
    bool SplitStack(int x, int y, int amount);
    bool MergeStacks(int fromX, int fromY, int toX, int toY);

    // Queries
    std::shared_ptr<Item> GetItem(int x, int y) const;
    bool IsSlotEmpty(int x, int y) const;
    bool IsSlotLocked(int x, int y) const;
    void LockSlot(int x, int y, bool locked);
    int GetItemCount() const;
    int GetFreeSlots() const;
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    // Find operations
    bool HasItem(const std::string& itemId) const;
    std::shared_ptr<Item> FindItem(const std::string& itemId);
    std::vector<std::shared_ptr<Item>> FindAllItems(const std::string& itemId);
    std::vector<std::shared_ptr<Item>> FindItemsByType(ItemType type);
    std::vector<std::shared_ptr<Item>> FindItemsByRarity(ItemRarity rarity);
    std::vector<std::shared_ptr<Item>> FilterItems(const ItemFilter& filter);

    // Sorting
    void Sort(SortCriteria criteria, bool ascending = true);
    void AutoSort();
    void CompactInventory();

    // Capacity management
    void Resize(int newWidth, int newHeight);
    void SetCapacity(int slots);
    int GetCapacity() const { return m_width * m_height; }
    bool IsFull() const;

    // Weight and value
    float GetTotalWeight() const;
    int GetTotalValue() const;
    void SetMaxWeight(float maxWeight) { m_maxWeight = maxWeight; }
    float GetMaxWeight() const { return m_maxWeight; }
    bool IsOverweight() const;

    // Trading
    bool CanAfford(int cost) const;
    void AddGold(int amount);
    bool RemoveGold(int amount);
    int GetGold() const { return m_gold; }
    void SetGold(int gold) { m_gold = gold; }

    // Quick access
    void SetQuickSlot(int quickSlotIndex, int invX, int invY);
    std::shared_ptr<Item> GetQuickSlot(int quickSlotIndex) const;
    void ClearQuickSlot(int quickSlotIndex);

    // Callbacks
    using ItemAddedCallback = std::function<void(std::shared_ptr<Item>)>;
    using ItemRemovedCallback = std::function<void(std::shared_ptr<Item>)>;
    using ItemMovedCallback = std::function<void(std::shared_ptr<Item>, int oldX, int oldY, int newX, int newY)>;

    void SetItemAddedCallback(ItemAddedCallback callback) { m_onItemAdded = callback; }
    void SetItemRemovedCallback(ItemRemovedCallback callback) { m_onItemRemoved = callback; }
    void SetItemMovedCallback(ItemMovedCallback callback) { m_onItemMoved = callback; }

    // Serialization
    std::string Serialize() const;
    bool Deserialize(const std::string& data);

    // Clear
    void Clear();

    // Get all slots
    const std::vector<std::vector<InventorySlot>>& GetSlots() const { return m_slots; }

    // Statistics
    std::map<ItemType, int> GetItemTypeCount() const;
    std::map<ItemRarity, int> GetItemRarityCount() const;

private:
    int m_width;
    int m_height;
    std::vector<std::vector<InventorySlot>> m_slots;

    int m_gold;
    float m_maxWeight;

    // Quick slots (hotbar)
    std::map<int, std::pair<int, int>> m_quickSlots;

    // Callbacks
    ItemAddedCallback m_onItemAdded;
    ItemRemovedCallback m_onItemRemoved;
    ItemMovedCallback m_onItemMoved;

    // Helper methods
    bool IsValidPosition(int x, int y) const;
    std::pair<int, int> FindFirstEmptySlot() const;
    std::pair<int, int> FindItemPosition(std::shared_ptr<Item> item) const;
    void NotifyItemAdded(std::shared_ptr<Item> item);
    void NotifyItemRemoved(std::shared_ptr<Item> item);
    void NotifyItemMoved(std::shared_ptr<Item> item, int oldX, int oldY, int newX, int newY);
};

// Bank/Storage system (extended inventory)
class StorageSystem {
public:
    StorageSystem(int width = 20, int height = 20);
    ~StorageSystem();

    // Use the same interface as InventorySystem
    InventorySystem* GetInventory() { return &m_inventory; }
    const InventorySystem* GetInventory() const { return &m_inventory; }

    // Transfer between inventories
    bool TransferToStorage(InventorySystem& playerInv, int x, int y);
    bool TransferFromStorage(InventorySystem& playerInv, int x, int y);
    bool TransferAll(InventorySystem& playerInv);

    // Storage tabs
    void CreateTab(const std::string& tabName, int width, int height);
    bool SelectTab(const std::string& tabName);
    std::vector<std::string> GetTabNames() const;
    const std::string& GetCurrentTab() const { return m_currentTab; }

    // Shared storage (between characters)
    void SetShared(bool shared) { m_isShared = shared; }
    bool IsShared() const { return m_isShared; }

private:
    InventorySystem m_inventory;
    std::map<std::string, InventorySystem> m_tabs;
    std::string m_currentTab;
    bool m_isShared;
};

// Trade system
class TradeSystem {
public:
    TradeSystem();
    ~TradeSystem();

    // Initialize trade between two players
    bool StartTrade(const std::string& player1Id, const std::string& player2Id);
    void CancelTrade();
    bool CompleteTrade();

    // Add/remove items
    bool AddItemToTrade(const std::string& playerId, std::shared_ptr<Item> item, int quantity);
    bool RemoveItemFromTrade(const std::string& playerId, int slotIndex);
    bool SetGoldOffer(const std::string& playerId, int gold);

    // Confirmation
    void SetReady(const std::string& playerId, bool ready);
    bool IsReady(const std::string& playerId) const;
    bool AreBothReady() const;

    // Get trade info
    const std::vector<std::pair<std::shared_ptr<Item>, int>>& GetPlayerItems(const std::string& playerId) const;
    int GetPlayerGold(const std::string& playerId) const;

    // State
    bool IsTrading() const { return m_isActive; }
    const std::string& GetPlayer1() const { return m_player1Id; }
    const std::string& GetPlayer2() const { return m_player2Id; }

private:
    bool m_isActive;
    std::string m_player1Id;
    std::string m_player2Id;

    std::vector<std::pair<std::shared_ptr<Item>, int>> m_player1Items;
    std::vector<std::pair<std::shared_ptr<Item>, int>> m_player2Items;

    int m_player1Gold;
    int m_player2Gold;

    bool m_player1Ready;
    bool m_player2Ready;

    bool ValidatePlayer(const std::string& playerId) const;
};

} // namespace Inventory
