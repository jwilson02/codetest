#pragma once

#include "UIElement.h"
#include <vector>
#include <unordered_map>
#include <functional>

namespace UI {

// Item data
struct InventoryItem {
    std::string id;
    std::string name;
    std::string description;
    std::string iconPath;
    std::string type; // Weapon, Armor, Consumable, Quest, Material, etc.
    std::string rarity; // Common, Uncommon, Rare, Epic, Legendary
    int stackSize;
    int maxStackSize;
    int value; // Gold value
    int level; // Required level
    bool isQuestItem;
    bool isSoulbound;

    InventoryItem()
        : stackSize(1), maxStackSize(1), value(0), level(1),
          isQuestItem(false), isSoulbound(false), rarity("Common") {}
};

// Inventory slot
class InventorySlot : public UIElement {
public:
    InventorySlot(int slotIndex);

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    void SetItem(const InventoryItem& item);
    void ClearItem();
    bool HasItem() const { return hasItem_; }
    const InventoryItem& GetItem() const { return item_; }

    int GetSlotIndex() const { return slotIndex_; }

    // Drag and drop
    void OnDragStart(std::function<void(int)> callback) { onDragStartCallback_ = callback; }
    void OnDragEnd(std::function<void(int)> callback) { onDragEndCallback_ = callback; }
    void OnDrop(std::function<void(int, int)> callback) { onDropCallback_ = callback; }

    void SetDragging(bool dragging) { isDragging_ = dragging; }
    bool IsDragging() const { return isDragging_; }

private:
    void RenderItem(SDL_Renderer* renderer);
    void RenderStackSize(SDL_Renderer* renderer);
    void RenderRarityBorder(SDL_Renderer* renderer);

private:
    int slotIndex_;
    bool hasItem_;
    InventoryItem item_;
    bool isDragging_;

    std::function<void(int)> onDragStartCallback_;
    std::function<void(int)> onDragEndCallback_;
    std::function<void(int, int)> onDropCallback_;
};

// Inventory UI
class InventoryUI : public UIElement {
public:
    InventoryUI(int numSlots = 40);
    ~InventoryUI() override;

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    // Inventory management
    bool AddItem(const InventoryItem& item);
    bool AddItem(const InventoryItem& item, int preferredSlot);
    bool RemoveItem(const std::string& itemId, int quantity = 1);
    bool RemoveItemFromSlot(int slot);
    bool HasItem(const std::string& itemId) const;
    int GetItemCount(const std::string& itemId) const;

    const InventoryItem* GetItemInSlot(int slot) const;
    int GetFirstEmptySlot() const;
    int GetNumSlots() const { return numSlots_; }
    int GetUsedSlots() const;

    // Drag and drop
    void SwapSlots(int slot1, int slot2);
    void MoveItem(int fromSlot, int toSlot);

    // Sorting
    void SortByType();
    void SortByRarity();
    void SortByValue();

    // Filters
    void SetFilter(const std::string& type);
    void ClearFilter();

    // Money/Currency
    void SetGold(int amount) { gold_ = amount; }
    int GetGold() const { return gold_; }
    void AddGold(int amount) { gold_ += amount; }
    bool RemoveGold(int amount);

    // Callbacks
    void OnItemUsed(std::function<void(const InventoryItem&)> callback) {
        onItemUsed_ = callback;
    }
    void OnItemSold(std::function<void(const InventoryItem&, int)> callback) {
        onItemSold_ = callback;
    }
    void OnItemDropped(std::function<void(const InventoryItem&)> callback) {
        onItemDropped_ = callback;
    }

    // Context menu
    void ShowContextMenu(int slot, float x, float y);
    void HideContextMenu();

private:
    void CreateSlots();
    void CreateGoldDisplay();
    void CreateFilterButtons();
    void CreateSortButtons();
    void CreateContextMenu();

    void UpdateSlotPositions();
    void RenderDraggedItem(SDL_Renderer* renderer);
    void RenderGoldDisplay(SDL_Renderer* renderer);
    void RenderContextMenu(SDL_Renderer* renderer);

    void HandleDragStart(int slot);
    void HandleDragEnd(int slot);
    void HandleDrop(int fromSlot, int toSlot);

    Color GetRarityColor(const std::string& rarity) const;

    void OnPositionChanged() override;
    void OnSizeChanged() override;

private:
    int numSlots_;
    int slotsPerRow_;
    std::vector<std::shared_ptr<InventorySlot>> slots_;

    // Currency
    int gold_;
    std::shared_ptr<UIElement> goldDisplay_;

    // Drag and drop state
    int draggedSlot_;
    bool isDragging_;
    float dragOffsetX_;
    float dragOffsetY_;

    // Filter and sort
    std::string activeFilter_;
    std::shared_ptr<UIElement> filterPanel_;
    std::shared_ptr<UIElement> sortPanel_;

    // Context menu
    std::shared_ptr<UIElement> contextMenu_;
    int contextMenuSlot_;
    bool showContextMenu_;

    // Callbacks
    std::function<void(const InventoryItem&)> onItemUsed_;
    std::function<void(const InventoryItem&, int)> onItemSold_;
    std::function<void(const InventoryItem&)> onItemDropped_;
};

} // namespace UI
