#include "InventorySystem.h"
#include <algorithm>
#include <sstream>

namespace Inventory {

// InventorySystem implementation
InventorySystem::InventorySystem(int width, int height)
    : m_width(width), m_height(height), m_gold(0), m_maxWeight(1000.0f) {

    m_slots.resize(m_height);
    for (int y = 0; y < m_height; y++) {
        m_slots[y].resize(m_width);
        for (int x = 0; x < m_width; x++) {
            m_slots[y][x] = InventorySlot(x, y);
        }
    }
}

InventorySystem::~InventorySystem() {
}

bool InventorySystem::AddItem(std::shared_ptr<Item> item) {
    if (!item) return false;

    // Try to stack with existing items first
    if (item->IsStackable() && StackItem(item)) {
        return true;
    }

    // Find first empty slot
    auto [x, y] = FindFirstEmptySlot();
    if (x == -1 || y == -1) {
        return false; // Inventory full
    }

    return AddItemAt(item, x, y);
}

bool InventorySystem::AddItemAt(std::shared_ptr<Item> item, int x, int y) {
    if (!item || !IsValidPosition(x, y)) return false;
    if (!IsSlotEmpty(x, y) || IsSlotLocked(x, y)) return false;

    m_slots[y][x].item = item;
    NotifyItemAdded(item);
    return true;
}

bool InventorySystem::RemoveItem(int x, int y) {
    if (!IsValidPosition(x, y)) return false;

    auto item = m_slots[y][x].item;
    if (!item) return false;

    m_slots[y][x].item = nullptr;
    NotifyItemRemoved(item);
    return true;
}

bool InventorySystem::RemoveItem(std::shared_ptr<Item> item) {
    auto [x, y] = FindItemPosition(item);
    if (x == -1 || y == -1) return false;

    return RemoveItem(x, y);
}

bool InventorySystem::MoveItem(int fromX, int fromY, int toX, int toY) {
    if (!IsValidPosition(fromX, fromY) || !IsValidPosition(toX, toY)) return false;
    if (IsSlotEmpty(fromX, fromY)) return false;
    if (IsSlotLocked(toX, toY)) return false;

    auto item = m_slots[fromY][fromX].item;

    // If target slot is empty, just move
    if (IsSlotEmpty(toX, toY)) {
        m_slots[toY][toX].item = item;
        m_slots[fromY][fromX].item = nullptr;
        NotifyItemMoved(item, fromX, fromY, toX, toY);
        return true;
    }

    // If both slots have items, try to stack or swap
    auto targetItem = m_slots[toY][toX].item;
    if (item->CanStack(*targetItem)) {
        return MergeStacks(fromX, fromY, toX, toY);
    }

    // Otherwise swap
    return SwapItems(fromX, fromY, toX, toY);
}

bool InventorySystem::SwapItems(int x1, int y1, int x2, int y2) {
    if (!IsValidPosition(x1, y1) || !IsValidPosition(x2, y2)) return false;
    if (IsSlotLocked(x1, y1) || IsSlotLocked(x2, y2)) return false;

    auto item1 = m_slots[y1][x1].item;
    auto item2 = m_slots[y2][x2].item;

    m_slots[y1][x1].item = item2;
    m_slots[y2][x2].item = item1;

    if (item1) NotifyItemMoved(item1, x1, y1, x2, y2);
    if (item2) NotifyItemMoved(item2, x2, y2, x1, y1);

    return true;
}

bool InventorySystem::StackItem(std::shared_ptr<Item> item) {
    if (!item || !item->IsStackable()) return false;

    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            auto slotItem = m_slots[y][x].item;
            if (slotItem && slotItem->CanStack(*item)) {
                int spaceLeft = slotItem->GetMaxStackSize() - slotItem->GetStackCount();
                int toAdd = std::min(spaceLeft, item->GetStackCount());

                if (toAdd > 0) {
                    slotItem->AddToStack(toAdd);
                    item->RemoveFromStack(toAdd);

                    if (item->GetStackCount() == 0) {
                        return true;
                    }
                }
            }
        }
    }

    // If there's still some left, try to add to empty slot
    if (item->GetStackCount() > 0) {
        return AddItem(item);
    }

    return true;
}

bool InventorySystem::SplitStack(int x, int y, int amount) {
    if (!IsValidPosition(x, y)) return false;

    auto item = m_slots[y][x].item;
    if (!item || !item->IsStackable()) return false;
    if (amount <= 0 || amount >= item->GetStackCount()) return false;

    // Create new stack
    auto newStack = item->Clone();
    newStack->SetStackCount(amount);
    item->RemoveFromStack(amount);

    // Find empty slot for new stack
    return AddItem(newStack);
}

bool InventorySystem::MergeStacks(int fromX, int fromY, int toX, int toY) {
    if (!IsValidPosition(fromX, fromY) || !IsValidPosition(toX, toY)) return false;

    auto fromItem = m_slots[fromY][fromX].item;
    auto toItem = m_slots[toY][toX].item;

    if (!fromItem || !toItem || !fromItem->CanStack(*toItem)) return false;

    int spaceLeft = toItem->GetMaxStackSize() - toItem->GetStackCount();
    int toTransfer = std::min(spaceLeft, fromItem->GetStackCount());

    toItem->AddToStack(toTransfer);
    fromItem->RemoveFromStack(toTransfer);

    if (fromItem->GetStackCount() == 0) {
        m_slots[fromY][fromX].item = nullptr;
    }

    return true;
}

std::shared_ptr<Item> InventorySystem::GetItem(int x, int y) const {
    if (!IsValidPosition(x, y)) return nullptr;
    return m_slots[y][x].item;
}

bool InventorySystem::IsSlotEmpty(int x, int y) const {
    if (!IsValidPosition(x, y)) return false;
    return m_slots[y][x].item == nullptr;
}

bool InventorySystem::IsSlotLocked(int x, int y) const {
    if (!IsValidPosition(x, y)) return false;
    return m_slots[y][x].locked;
}

void InventorySystem::LockSlot(int x, int y, bool locked) {
    if (IsValidPosition(x, y)) {
        m_slots[y][x].locked = locked;
    }
}

int InventorySystem::GetItemCount() const {
    int count = 0;
    for (const auto& row : m_slots) {
        for (const auto& slot : row) {
            if (slot.item) {
                count += slot.item->GetStackCount();
            }
        }
    }
    return count;
}

int InventorySystem::GetFreeSlots() const {
    int count = 0;
    for (const auto& row : m_slots) {
        for (const auto& slot : row) {
            if (!slot.item && !slot.locked) {
                count++;
            }
        }
    }
    return count;
}

bool InventorySystem::HasItem(const std::string& itemId) const {
    for (const auto& row : m_slots) {
        for (const auto& slot : row) {
            if (slot.item && slot.item->GetId() == itemId) {
                return true;
            }
        }
    }
    return false;
}

std::shared_ptr<Item> InventorySystem::FindItem(const std::string& itemId) {
    for (auto& row : m_slots) {
        for (auto& slot : row) {
            if (slot.item && slot.item->GetId() == itemId) {
                return slot.item;
            }
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Item>> InventorySystem::FindAllItems(const std::string& itemId) {
    std::vector<std::shared_ptr<Item>> items;
    for (auto& row : m_slots) {
        for (auto& slot : row) {
            if (slot.item && slot.item->GetId() == itemId) {
                items.push_back(slot.item);
            }
        }
    }
    return items;
}

std::vector<std::shared_ptr<Item>> InventorySystem::FindItemsByType(ItemType type) {
    std::vector<std::shared_ptr<Item>> items;
    for (auto& row : m_slots) {
        for (auto& slot : row) {
            if (slot.item && slot.item->GetType() == type) {
                items.push_back(slot.item);
            }
        }
    }
    return items;
}

std::vector<std::shared_ptr<Item>> InventorySystem::FindItemsByRarity(ItemRarity rarity) {
    std::vector<std::shared_ptr<Item>> items;
    for (auto& row : m_slots) {
        for (auto& slot : row) {
            if (slot.item && slot.item->GetRarity() == rarity) {
                items.push_back(slot.item);
            }
        }
    }
    return items;
}

std::vector<std::shared_ptr<Item>> InventorySystem::FilterItems(const ItemFilter& filter) {
    std::vector<std::shared_ptr<Item>> items;

    for (auto& row : m_slots) {
        for (auto& slot : row) {
            if (!slot.item) continue;

            auto item = slot.item;

            // Apply filters
            if (filter.type != ItemType::Misc && item->GetType() != filter.type) continue;
            if (item->GetRarity() < filter.minRarity || item->GetRarity() > filter.maxRarity) continue;
            if (item->GetLevel() < filter.minLevel || item->GetLevel() > filter.maxLevel) continue;
            if (!filter.nameContains.empty() &&
                item->GetName().find(filter.nameContains) == std::string::npos) continue;
            if (filter.equippableOnly && !item->IsEquippable()) continue;

            items.push_back(item);
        }
    }

    return items;
}

void InventorySystem::Sort(SortCriteria criteria, bool ascending) {
    std::vector<std::shared_ptr<Item>> items;

    // Collect all items
    for (auto& row : m_slots) {
        for (auto& slot : row) {
            if (slot.item && !slot.locked) {
                items.push_back(slot.item);
                slot.item = nullptr;
            }
        }
    }

    // Sort items
    std::sort(items.begin(), items.end(), [criteria, ascending](const auto& a, const auto& b) {
        int result = 0;

        switch (criteria) {
            case SortCriteria::Name:
                result = a->GetName().compare(b->GetName());
                break;
            case SortCriteria::Type:
                result = static_cast<int>(a->GetType()) - static_cast<int>(b->GetType());
                break;
            case SortCriteria::Rarity:
                result = static_cast<int>(a->GetRarity()) - static_cast<int>(b->GetRarity());
                break;
            case SortCriteria::Level:
                result = a->GetLevel() - b->GetLevel();
                break;
            case SortCriteria::Value:
                result = a->GetValue() - b->GetValue();
                break;
            case SortCriteria::Weight:
                result = (a->GetWeight() > b->GetWeight()) ? 1 : -1;
                break;
        }

        return ascending ? (result < 0) : (result > 0);
    });

    // Place items back
    int index = 0;
    for (int y = 0; y < m_height && index < static_cast<int>(items.size()); y++) {
        for (int x = 0; x < m_width && index < static_cast<int>(items.size()); x++) {
            if (!m_slots[y][x].locked) {
                m_slots[y][x].item = items[index++];
            }
        }
    }
}

void InventorySystem::AutoSort() {
    // Sort by rarity (descending), then by level (descending)
    Sort(SortCriteria::Rarity, false);
}

void InventorySystem::CompactInventory() {
    std::vector<std::shared_ptr<Item>> items;

    // Collect all items
    for (auto& row : m_slots) {
        for (auto& slot : row) {
            if (slot.item && !slot.locked) {
                items.push_back(slot.item);
                slot.item = nullptr;
            }
        }
    }

    // Place items from top-left
    int index = 0;
    for (int y = 0; y < m_height && index < static_cast<int>(items.size()); y++) {
        for (int x = 0; x < m_width && index < static_cast<int>(items.size()); x++) {
            if (!m_slots[y][x].locked) {
                m_slots[y][x].item = items[index++];
            }
        }
    }
}

void InventorySystem::Resize(int newWidth, int newHeight) {
    // Save existing items
    std::vector<std::shared_ptr<Item>> items;
    for (auto& row : m_slots) {
        for (auto& slot : row) {
            if (slot.item) {
                items.push_back(slot.item);
            }
        }
    }

    // Resize
    m_width = newWidth;
    m_height = newHeight;
    m_slots.clear();
    m_slots.resize(m_height);
    for (int y = 0; y < m_height; y++) {
        m_slots[y].resize(m_width);
        for (int x = 0; x < m_width; x++) {
            m_slots[y][x] = InventorySlot(x, y);
        }
    }

    // Re-add items
    for (auto& item : items) {
        AddItem(item);
    }
}

void InventorySystem::SetCapacity(int slots) {
    int width = 10;
    int height = (slots + width - 1) / width; // Round up
    Resize(width, height);
}

bool InventorySystem::IsFull() const {
    return GetFreeSlots() == 0;
}

float InventorySystem::GetTotalWeight() const {
    float weight = 0.0f;
    for (const auto& row : m_slots) {
        for (const auto& slot : row) {
            if (slot.item) {
                weight += slot.item->GetWeight() * slot.item->GetStackCount();
            }
        }
    }
    return weight;
}

int InventorySystem::GetTotalValue() const {
    int value = 0;
    for (const auto& row : m_slots) {
        for (const auto& slot : row) {
            if (slot.item) {
                value += slot.item->GetValue() * slot.item->GetStackCount();
            }
        }
    }
    return value;
}

bool InventorySystem::IsOverweight() const {
    return GetTotalWeight() > m_maxWeight;
}

bool InventorySystem::CanAfford(int cost) const {
    return m_gold >= cost;
}

void InventorySystem::AddGold(int amount) {
    m_gold += amount;
}

bool InventorySystem::RemoveGold(int amount) {
    if (m_gold < amount) return false;
    m_gold -= amount;
    return true;
}

void InventorySystem::SetQuickSlot(int quickSlotIndex, int invX, int invY) {
    if (IsValidPosition(invX, invY)) {
        m_quickSlots[quickSlotIndex] = {invX, invY};
    }
}

std::shared_ptr<Item> InventorySystem::GetQuickSlot(int quickSlotIndex) const {
    auto it = m_quickSlots.find(quickSlotIndex);
    if (it != m_quickSlots.end()) {
        return GetItem(it->second.first, it->second.second);
    }
    return nullptr;
}

void InventorySystem::ClearQuickSlot(int quickSlotIndex) {
    m_quickSlots.erase(quickSlotIndex);
}

void InventorySystem::Clear() {
    for (auto& row : m_slots) {
        for (auto& slot : row) {
            slot.item = nullptr;
        }
    }
    m_gold = 0;
    m_quickSlots.clear();
}

std::map<ItemType, int> InventorySystem::GetItemTypeCount() const {
    std::map<ItemType, int> counts;

    for (const auto& row : m_slots) {
        for (const auto& slot : row) {
            if (slot.item) {
                counts[slot.item->GetType()] += slot.item->GetStackCount();
            }
        }
    }

    return counts;
}

std::map<ItemRarity, int> InventorySystem::GetItemRarityCount() const {
    std::map<ItemRarity, int> counts;

    for (const auto& row : m_slots) {
        for (const auto& slot : row) {
            if (slot.item) {
                counts[slot.item->GetRarity()]++;
            }
        }
    }

    return counts;
}

bool InventorySystem::IsValidPosition(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

std::pair<int, int> InventorySystem::FindFirstEmptySlot() const {
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            if (IsSlotEmpty(x, y) && !IsSlotLocked(x, y)) {
                return {x, y};
            }
        }
    }
    return {-1, -1};
}

std::pair<int, int> InventorySystem::FindItemPosition(std::shared_ptr<Item> item) const {
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            if (m_slots[y][x].item == item) {
                return {x, y};
            }
        }
    }
    return {-1, -1};
}

void InventorySystem::NotifyItemAdded(std::shared_ptr<Item> item) {
    if (m_onItemAdded) {
        m_onItemAdded(item);
    }
}

void InventorySystem::NotifyItemRemoved(std::shared_ptr<Item> item) {
    if (m_onItemRemoved) {
        m_onItemRemoved(item);
    }
}

void InventorySystem::NotifyItemMoved(std::shared_ptr<Item> item, int oldX, int oldY, int newX, int newY) {
    if (m_onItemMoved) {
        m_onItemMoved(item, oldX, oldY, newX, newY);
    }
}

// StorageSystem implementation
StorageSystem::StorageSystem(int width, int height)
    : m_inventory(width, height), m_currentTab("main"), m_isShared(false) {
}

StorageSystem::~StorageSystem() {
}

bool StorageSystem::TransferToStorage(InventorySystem& playerInv, int x, int y) {
    auto item = playerInv.GetItem(x, y);
    if (!item) return false;

    if (m_inventory.AddItem(item)) {
        playerInv.RemoveItem(x, y);
        return true;
    }

    return false;
}

bool StorageSystem::TransferFromStorage(InventorySystem& playerInv, int x, int y) {
    auto item = m_inventory.GetItem(x, y);
    if (!item) return false;

    if (playerInv.AddItem(item)) {
        m_inventory.RemoveItem(x, y);
        return true;
    }

    return false;
}

bool StorageSystem::TransferAll(InventorySystem& playerInv) {
    bool success = true;

    for (int y = 0; y < playerInv.GetHeight(); y++) {
        for (int x = 0; x < playerInv.GetWidth(); x++) {
            auto item = playerInv.GetItem(x, y);
            if (item && !item->IsQuestItem()) {
                if (!m_inventory.AddItem(item)) {
                    success = false;
                } else {
                    playerInv.RemoveItem(x, y);
                }
            }
        }
    }

    return success;
}

void StorageSystem::CreateTab(const std::string& tabName, int width, int height) {
    m_tabs[tabName] = InventorySystem(width, height);
}

bool StorageSystem::SelectTab(const std::string& tabName) {
    if (tabName == "main") {
        m_currentTab = "main";
        return true;
    }

    auto it = m_tabs.find(tabName);
    if (it != m_tabs.end()) {
        m_currentTab = tabName;
        return true;
    }

    return false;
}

std::vector<std::string> StorageSystem::GetTabNames() const {
    std::vector<std::string> names = {"main"};

    for (const auto& [name, _] : m_tabs) {
        names.push_back(name);
    }

    return names;
}

// TradeSystem implementation
TradeSystem::TradeSystem()
    : m_isActive(false), m_player1Gold(0), m_player2Gold(0),
      m_player1Ready(false), m_player2Ready(false) {
}

TradeSystem::~TradeSystem() {
}

bool TradeSystem::StartTrade(const std::string& player1Id, const std::string& player2Id) {
    if (m_isActive) return false;

    m_isActive = true;
    m_player1Id = player1Id;
    m_player2Id = player2Id;
    m_player1Items.clear();
    m_player2Items.clear();
    m_player1Gold = 0;
    m_player2Gold = 0;
    m_player1Ready = false;
    m_player2Ready = false;

    return true;
}

void TradeSystem::CancelTrade() {
    m_isActive = false;
    m_player1Items.clear();
    m_player2Items.clear();
}

bool TradeSystem::CompleteTrade() {
    if (!m_isActive || !AreBothReady()) return false;

    // Trade would be completed here - items and gold exchanged
    // This requires access to player inventories

    m_isActive = false;
    return true;
}

bool TradeSystem::AddItemToTrade(const std::string& playerId, std::shared_ptr<Item> item, int quantity) {
    if (!m_isActive || !ValidatePlayer(playerId)) return false;
    if (!item || !item->IsTradeable()) return false;

    if (playerId == m_player1Id) {
        m_player1Items.push_back({item, quantity});
        m_player1Ready = false;
    } else {
        m_player2Items.push_back({item, quantity});
        m_player2Ready = false;
    }

    return true;
}

bool TradeSystem::RemoveItemFromTrade(const std::string& playerId, int slotIndex) {
    if (!m_isActive || !ValidatePlayer(playerId)) return false;

    if (playerId == m_player1Id) {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(m_player1Items.size())) return false;
        m_player1Items.erase(m_player1Items.begin() + slotIndex);
        m_player1Ready = false;
    } else {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(m_player2Items.size())) return false;
        m_player2Items.erase(m_player2Items.begin() + slotIndex);
        m_player2Ready = false;
    }

    return true;
}

bool TradeSystem::SetGoldOffer(const std::string& playerId, int gold) {
    if (!m_isActive || !ValidatePlayer(playerId) || gold < 0) return false;

    if (playerId == m_player1Id) {
        m_player1Gold = gold;
        m_player1Ready = false;
    } else {
        m_player2Gold = gold;
        m_player2Ready = false;
    }

    return true;
}

void TradeSystem::SetReady(const std::string& playerId, bool ready) {
    if (!m_isActive || !ValidatePlayer(playerId)) return;

    if (playerId == m_player1Id) {
        m_player1Ready = ready;
    } else {
        m_player2Ready = ready;
    }
}

bool TradeSystem::IsReady(const std::string& playerId) const {
    if (!ValidatePlayer(playerId)) return false;
    return (playerId == m_player1Id) ? m_player1Ready : m_player2Ready;
}

bool TradeSystem::AreBothReady() const {
    return m_player1Ready && m_player2Ready;
}

const std::vector<std::pair<std::shared_ptr<Item>, int>>& TradeSystem::GetPlayerItems(const std::string& playerId) const {
    static std::vector<std::pair<std::shared_ptr<Item>, int>> empty;
    if (!ValidatePlayer(playerId)) return empty;
    return (playerId == m_player1Id) ? m_player1Items : m_player2Items;
}

int TradeSystem::GetPlayerGold(const std::string& playerId) const {
    if (!ValidatePlayer(playerId)) return 0;
    return (playerId == m_player1Id) ? m_player1Gold : m_player2Gold;
}

bool TradeSystem::ValidatePlayer(const std::string& playerId) const {
    return playerId == m_player1Id || playerId == m_player2Id;
}

} // namespace Inventory
