#include "InventoryUI.h"
#include "UISystem.h"
#include <SDL2/SDL.h>
#include <algorithm>

namespace UI {

// InventorySlot implementation
InventorySlot::InventorySlot(int slotIndex)
    : UIElement("invSlot_" + std::to_string(slotIndex)),
      slotIndex_(slotIndex), hasItem_(false), isDragging_(false) {

    SetSize(50, 50);
    SetBackgroundColor(Color(40, 40, 45, 220));
    SetBorderColor(Color(80, 80, 90, 255));
    SetBorderWidth(2.0f);
}

void InventorySlot::Update(float deltaTime) {
    UIElement::Update(deltaTime);
}

void InventorySlot::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    // Don't render if being dragged (will be rendered separately)
    if (isDragging_) {
        // Render empty slot
        UIElement::Render(renderer);
        return;
    }

    UIElement::Render(renderer);

    if (hasItem_) {
        RenderRarityBorder(renderer);
        RenderItem(renderer);
        if (item_.stackSize > 1) {
            RenderStackSize(renderer);
        }
    }
}

void InventorySlot::HandleInput(const SDL_Event& event) {
    UIElement::HandleInput(event);

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (hasItem_ && ContainsPoint(static_cast<float>(event.button.x),
                                      static_cast<float>(event.button.y))) {
            if (onDragStartCallback_) {
                onDragStartCallback_(slotIndex_);
            }
        }
    }
}

void InventorySlot::SetItem(const InventoryItem& item) {
    item_ = item;
    hasItem_ = true;
}

void InventorySlot::ClearItem() {
    hasItem_ = false;
    item_ = InventoryItem();
}

void InventorySlot::RenderItem(SDL_Renderer* renderer) {
    // Render item icon
    // Placeholder - would load and render texture from item_.iconPath

    SDL_Rect bounds = GetBounds();
    SDL_Rect iconRect = bounds;
    iconRect.x += 5;
    iconRect.y += 5;
    iconRect.w -= 10;
    iconRect.h -= 10;

    // Simple colored square as placeholder
    SDL_SetRenderDrawColor(renderer, 200, 180, 100, 255);
    SDL_RenderFillRect(renderer, &iconRect);
}

void InventorySlot::RenderStackSize(SDL_Renderer* renderer) {
    // Render stack size text in bottom-right corner
    // Would use TTF fonts to render item_.stackSize
    // Placeholder
}

void InventorySlot::RenderRarityBorder(SDL_Renderer* renderer) {
    // Render colored border based on rarity
    Color rarityColor;
    if (item_.rarity == "Common") rarityColor = Color::FromHex("#9D9D9D");
    else if (item_.rarity == "Uncommon") rarityColor = Color::FromHex("#1EFF00");
    else if (item_.rarity == "Rare") rarityColor = Color::FromHex("#0070DD");
    else if (item_.rarity == "Epic") rarityColor = Color::FromHex("#A335EE");
    else if (item_.rarity == "Legendary") rarityColor = Color::FromHex("#FF8000");
    else rarityColor = Color::FromHex("#FFFFFF");

    SDL_Rect bounds = GetBounds();
    SDL_SetRenderDrawColor(renderer, rarityColor.r, rarityColor.g, rarityColor.b, rarityColor.a);

    for (int i = 0; i < 2; ++i) {
        SDL_Rect borderRect = bounds;
        borderRect.x -= i;
        borderRect.y -= i;
        borderRect.w += i * 2;
        borderRect.h += i * 2;
        SDL_RenderDrawRect(renderer, &borderRect);
    }
}

// InventoryUI implementation
InventoryUI::InventoryUI(int numSlots)
    : UIElement("inventory"),
      numSlots_(numSlots), slotsPerRow_(8),
      gold_(0), draggedSlot_(-1), isDragging_(false),
      dragOffsetX_(0), dragOffsetY_(0),
      contextMenuSlot_(-1), showContextMenu_(false) {

    auto& uiSystem = UISystem::Instance();
    const auto& theme = uiSystem.GetTheme();

    // Calculate size based on number of slots
    int rows = (numSlots_ + slotsPerRow_ - 1) / slotsPerRow_;
    float slotSize = 50;
    float spacing = 5;
    float width = slotsPerRow_ * (slotSize + spacing) + spacing + 40;
    float height = rows * (slotSize + spacing) + spacing + 120;

    SetSize(width, height);
    SetPosition(uiSystem.GetScreenWidth() * 0.5f - width / 2,
                uiSystem.GetScreenHeight() * 0.5f - height / 2);

    SetBackgroundColor(Color(25, 25, 30, 240));
    SetBorderColor(theme.accentColor);
    SetBorderWidth(3.0f);
    SetVisible(false);

    CreateSlots();
    CreateGoldDisplay();
    CreateFilterButtons();
    CreateSortButtons();
    CreateContextMenu();
}

InventoryUI::~InventoryUI() {
}

void InventoryUI::Update(float deltaTime) {
    if (!visible_) return;

    UIElement::Update(deltaTime);

    for (auto& slot : slots_) {
        if (slot) slot->Update(deltaTime);
    }
}

void InventoryUI::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    UIElement::Render(renderer);

    // Render slots
    for (auto& slot : slots_) {
        if (slot) slot->Render(renderer);
    }

    RenderGoldDisplay(renderer);

    // Render dragged item on top
    if (isDragging_) {
        RenderDraggedItem(renderer);
    }

    // Render context menu
    if (showContextMenu_) {
        RenderContextMenu(renderer);
    }
}

void InventoryUI::HandleInput(const SDL_Event& event) {
    if (!visible_) return;

    // Close with ESC or 'I' key
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_i) {
            SetVisible(false);
            return;
        }
    }

    // Handle drag and drop
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (showContextMenu_) {
            HideContextMenu();
        }

        // Check if clicking on a slot
        for (auto& slot : slots_) {
            if (slot && slot->HasItem() &&
                slot->ContainsPoint(static_cast<float>(event.button.x),
                                   static_cast<float>(event.button.y))) {
                HandleDragStart(slot->GetSlotIndex());
                break;
            }
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (isDragging_) {
            // Find drop target
            int targetSlot = -1;
            for (auto& slot : slots_) {
                if (slot && slot->ContainsPoint(static_cast<float>(event.button.x),
                                               static_cast<float>(event.button.y))) {
                    targetSlot = slot->GetSlotIndex();
                    break;
                }
            }

            HandleDrop(draggedSlot_, targetSlot);
            isDragging_ = false;
            draggedSlot_ = -1;
        }
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
        // Right-click for context menu
        for (auto& slot : slots_) {
            if (slot && slot->HasItem() &&
                slot->ContainsPoint(static_cast<float>(event.button.x),
                                   static_cast<float>(event.button.y))) {
                ShowContextMenu(slot->GetSlotIndex(),
                              static_cast<float>(event.button.x),
                              static_cast<float>(event.button.y));
                return;
            }
        }
        HideContextMenu();
    }

    UIElement::HandleInput(event);

    for (auto& slot : slots_) {
        if (slot) slot->HandleInput(event);
    }
}

bool InventoryUI::AddItem(const InventoryItem& item) {
    // Try to stack first
    if (item.maxStackSize > 1) {
        for (auto& slot : slots_) {
            if (slot && slot->HasItem() &&
                slot->GetItem().id == item.id &&
                slot->GetItem().stackSize < slot->GetItem().maxStackSize) {

                InventoryItem stackedItem = slot->GetItem();
                int spaceLeft = stackedItem.maxStackSize - stackedItem.stackSize;
                int toAdd = std::min(spaceLeft, item.stackSize);

                stackedItem.stackSize += toAdd;
                slot->SetItem(stackedItem);

                if (toAdd >= item.stackSize) {
                    return true; // All items stacked
                }
                // Continue stacking remaining items
            }
        }
    }

    // Find empty slot
    int emptySlot = GetFirstEmptySlot();
    if (emptySlot >= 0) {
        slots_[emptySlot]->SetItem(item);
        return true;
    }

    return false; // Inventory full
}

bool InventoryUI::AddItem(const InventoryItem& item, int preferredSlot) {
    if (preferredSlot >= 0 && preferredSlot < numSlots_) {
        if (!slots_[preferredSlot]->HasItem()) {
            slots_[preferredSlot]->SetItem(item);
            return true;
        }
    }
    return AddItem(item);
}

bool InventoryUI::RemoveItem(const std::string& itemId, int quantity) {
    int remaining = quantity;

    for (auto& slot : slots_) {
        if (slot && slot->HasItem() && slot->GetItem().id == itemId) {
            InventoryItem item = slot->GetItem();

            if (item.stackSize <= remaining) {
                remaining -= item.stackSize;
                slot->ClearItem();
            } else {
                item.stackSize -= remaining;
                slot->SetItem(item);
                remaining = 0;
            }

            if (remaining == 0) return true;
        }
    }

    return remaining == 0;
}

bool InventoryUI::RemoveItemFromSlot(int slot) {
    if (slot >= 0 && slot < numSlots_ && slots_[slot]->HasItem()) {
        slots_[slot]->ClearItem();
        return true;
    }
    return false;
}

bool InventoryUI::HasItem(const std::string& itemId) const {
    for (const auto& slot : slots_) {
        if (slot && slot->HasItem() && slot->GetItem().id == itemId) {
            return true;
        }
    }
    return false;
}

int InventoryUI::GetItemCount(const std::string& itemId) const {
    int count = 0;
    for (const auto& slot : slots_) {
        if (slot && slot->HasItem() && slot->GetItem().id == itemId) {
            count += slot->GetItem().stackSize;
        }
    }
    return count;
}

const InventoryItem* InventoryUI::GetItemInSlot(int slot) const {
    if (slot >= 0 && slot < numSlots_ && slots_[slot]->HasItem()) {
        return &slots_[slot]->GetItem();
    }
    return nullptr;
}

int InventoryUI::GetFirstEmptySlot() const {
    for (int i = 0; i < numSlots_; ++i) {
        if (!slots_[i]->HasItem()) {
            return i;
        }
    }
    return -1;
}

int InventoryUI::GetUsedSlots() const {
    int used = 0;
    for (const auto& slot : slots_) {
        if (slot && slot->HasItem()) {
            used++;
        }
    }
    return used;
}

void InventoryUI::SwapSlots(int slot1, int slot2) {
    if (slot1 == slot2) return;
    if (slot1 < 0 || slot1 >= numSlots_ || slot2 < 0 || slot2 >= numSlots_) return;

    bool has1 = slots_[slot1]->HasItem();
    bool has2 = slots_[slot2]->HasItem();

    if (has1 && has2) {
        InventoryItem temp = slots_[slot1]->GetItem();
        slots_[slot1]->SetItem(slots_[slot2]->GetItem());
        slots_[slot2]->SetItem(temp);
    } else if (has1) {
        slots_[slot2]->SetItem(slots_[slot1]->GetItem());
        slots_[slot1]->ClearItem();
    } else if (has2) {
        slots_[slot1]->SetItem(slots_[slot2]->GetItem());
        slots_[slot2]->ClearItem();
    }
}

void InventoryUI::MoveItem(int fromSlot, int toSlot) {
    if (fromSlot == toSlot) return;
    if (fromSlot < 0 || fromSlot >= numSlots_) return;
    if (toSlot < 0 || toSlot >= numSlots_) return;

    if (!slots_[fromSlot]->HasItem()) return;

    if (!slots_[toSlot]->HasItem()) {
        slots_[toSlot]->SetItem(slots_[fromSlot]->GetItem());
        slots_[fromSlot]->ClearItem();
    } else {
        SwapSlots(fromSlot, toSlot);
    }
}

void InventoryUI::SortByType() {
    // Collect all items
    std::vector<InventoryItem> items;
    for (auto& slot : slots_) {
        if (slot && slot->HasItem()) {
            items.push_back(slot->GetItem());
            slot->ClearItem();
        }
    }

    // Sort by type
    std::sort(items.begin(), items.end(),
              [](const InventoryItem& a, const InventoryItem& b) {
                  return a.type < b.type;
              });

    // Place back
    for (size_t i = 0; i < items.size() && i < slots_.size(); ++i) {
        slots_[i]->SetItem(items[i]);
    }
}

void InventoryUI::SortByRarity() {
    std::vector<InventoryItem> items;
    for (auto& slot : slots_) {
        if (slot && slot->HasItem()) {
            items.push_back(slot->GetItem());
            slot->ClearItem();
        }
    }

    // Sort by rarity (Legendary > Epic > Rare > Uncommon > Common)
    std::unordered_map<std::string, int> rarityOrder = {
        {"Legendary", 5}, {"Epic", 4}, {"Rare", 3}, {"Uncommon", 2}, {"Common", 1}
    };

    std::sort(items.begin(), items.end(),
              [&rarityOrder](const InventoryItem& a, const InventoryItem& b) {
                  return rarityOrder[a.rarity] > rarityOrder[b.rarity];
              });

    for (size_t i = 0; i < items.size() && i < slots_.size(); ++i) {
        slots_[i]->SetItem(items[i]);
    }
}

void InventoryUI::SortByValue() {
    std::vector<InventoryItem> items;
    for (auto& slot : slots_) {
        if (slot && slot->HasItem()) {
            items.push_back(slot->GetItem());
            slot->ClearItem();
        }
    }

    std::sort(items.begin(), items.end(),
              [](const InventoryItem& a, const InventoryItem& b) {
                  return a.value > b.value;
              });

    for (size_t i = 0; i < items.size() && i < slots_.size(); ++i) {
        slots_[i]->SetItem(items[i]);
    }
}

void InventoryUI::SetFilter(const std::string& type) {
    activeFilter_ = type;
    // Would hide/show slots based on filter
}

void InventoryUI::ClearFilter() {
    activeFilter_.clear();
}

bool InventoryUI::RemoveGold(int amount) {
    if (gold_ >= amount) {
        gold_ -= amount;
        return true;
    }
    return false;
}

void InventoryUI::ShowContextMenu(int slot, float x, float y) {
    if (!contextMenu_) return;

    contextMenuSlot_ = slot;
    showContextMenu_ = true;

    contextMenu_->SetPosition(x, y);
    contextMenu_->SetVisible(true);
}

void InventoryUI::HideContextMenu() {
    showContextMenu_ = false;
    if (contextMenu_) {
        contextMenu_->SetVisible(false);
    }
}

void InventoryUI::CreateSlots() {
    float slotSize = 50;
    float spacing = 5;
    float startX = 20;
    float startY = 50;

    for (int i = 0; i < numSlots_; ++i) {
        int row = i / slotsPerRow_;
        int col = i % slotsPerRow_;

        auto slot = std::make_shared<InventorySlot>(i);
        slot->SetPosition(startX + col * (slotSize + spacing),
                         startY + row * (slotSize + spacing));

        slot->OnDragStart([this](int slotIdx) {
            HandleDragStart(slotIdx);
        });

        AddChild(slot);
        slots_.push_back(slot);
    }
}

void InventoryUI::CreateGoldDisplay() {
    goldDisplay_ = std::make_shared<UIElement>("goldDisplay");
    goldDisplay_->SetSize(200, 30);
    goldDisplay_->SetPosition(20, height_ - 50);
    goldDisplay_->SetBackgroundColor(Color(40, 40, 45, 220));
    goldDisplay_->SetBorderColor(Color::FromHex("#FFD700"));
    goldDisplay_->SetBorderWidth(2.0f);

    AddChild(goldDisplay_);
}

void InventoryUI::CreateFilterButtons() {
    filterPanel_ = std::make_shared<UIElement>("filterPanel");
    filterPanel_->SetSize(width_ - 40, 30);
    filterPanel_->SetPosition(20, 10);
    filterPanel_->SetBackgroundColor(Color(0, 0, 0, 0)); // Transparent

    AddChild(filterPanel_);

    // Would create filter buttons here (All, Weapons, Armor, Consumables, etc.)
}

void InventoryUI::CreateSortButtons() {
    sortPanel_ = std::make_shared<UIElement>("sortPanel");
    sortPanel_->SetSize(200, 30);
    sortPanel_->SetPosition(width_ - 220, height_ - 50);
    sortPanel_->SetBackgroundColor(Color(0, 0, 0, 0)); // Transparent

    AddChild(sortPanel_);

    // Would create sort buttons here
}

void InventoryUI::CreateContextMenu() {
    auto& theme = UISystem::Instance().GetTheme();

    contextMenu_ = std::make_shared<UIElement>("contextMenu");
    contextMenu_->SetSize(150, 120);
    contextMenu_->SetBackgroundColor(Color(30, 30, 35, 250));
    contextMenu_->SetBorderColor(theme.accentColor);
    contextMenu_->SetBorderWidth(2.0f);
    contextMenu_->SetVisible(false);
    contextMenu_->SetLayer(1000); // Render on top

    // Would create context menu buttons here (Use, Drop, Sell, etc.)

    AddChild(contextMenu_);
}

void InventoryUI::UpdateSlotPositions() {
    float slotSize = 50;
    float spacing = 5;
    float startX = 20;
    float startY = 50;

    for (int i = 0; i < numSlots_; ++i) {
        int row = i / slotsPerRow_;
        int col = i % slotsPerRow_;

        if (slots_[i]) {
            slots_[i]->SetPosition(startX + col * (slotSize + spacing),
                                  startY + row * (slotSize + spacing));
        }
    }
}

void InventoryUI::RenderDraggedItem(SDL_Renderer* renderer) {
    if (draggedSlot_ < 0 || draggedSlot_ >= numSlots_) return;
    if (!slots_[draggedSlot_]->HasItem()) return;

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    const auto& item = slots_[draggedSlot_]->GetItem();

    // Render item following cursor
    SDL_Rect itemRect;
    itemRect.x = mouseX - 25;
    itemRect.y = mouseY - 25;
    itemRect.w = 50;
    itemRect.h = 50;

    // Semi-transparent
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Background
    SDL_SetRenderDrawColor(renderer, 40, 40, 45, 180);
    SDL_RenderFillRect(renderer, &itemRect);

    // Rarity border
    Color rarityColor = GetRarityColor(item.rarity);
    SDL_SetRenderDrawColor(renderer, rarityColor.r, rarityColor.g,
                          rarityColor.b, 200);
    SDL_RenderDrawRect(renderer, &itemRect);

    // Item icon (placeholder)
    SDL_Rect iconRect = itemRect;
    iconRect.x += 5;
    iconRect.y += 5;
    iconRect.w -= 10;
    iconRect.h -= 10;
    SDL_SetRenderDrawColor(renderer, 200, 180, 100, 180);
    SDL_RenderFillRect(renderer, &iconRect);
}

void InventoryUI::RenderGoldDisplay(SDL_Renderer* renderer) {
    if (!goldDisplay_) return;

    goldDisplay_->Render(renderer);

    // Would render gold amount using TTF fonts
    // Format: "Gold: 12,345"
}

void InventoryUI::RenderContextMenu(SDL_Renderer* renderer) {
    if (!contextMenu_ || !showContextMenu_) return;

    contextMenu_->Render(renderer);

    // Would render menu options here
}

void InventoryUI::HandleDragStart(int slot) {
    if (slot >= 0 && slot < numSlots_ && slots_[slot]->HasItem()) {
        draggedSlot_ = slot;
        isDragging_ = true;
        slots_[slot]->SetDragging(true);
    }
}

void InventoryUI::HandleDragEnd(int slot) {
    if (slot >= 0 && slot < numSlots_) {
        slots_[slot]->SetDragging(false);
    }
}

void InventoryUI::HandleDrop(int fromSlot, int toSlot) {
    if (fromSlot >= 0 && fromSlot < numSlots_) {
        slots_[fromSlot]->SetDragging(false);
    }

    if (fromSlot >= 0 && toSlot >= 0 && fromSlot != toSlot) {
        // Try to stack if same item
        if (toSlot < numSlots_ && slots_[toSlot]->HasItem() && slots_[fromSlot]->HasItem()) {
            const auto& fromItem = slots_[fromSlot]->GetItem();
            const auto& toItem = slots_[toSlot]->GetItem();

            if (fromItem.id == toItem.id && toItem.stackSize < toItem.maxStackSize) {
                InventoryItem mergedItem = toItem;
                int spaceLeft = mergedItem.maxStackSize - mergedItem.stackSize;
                int toMove = std::min(spaceLeft, fromItem.stackSize);

                mergedItem.stackSize += toMove;
                slots_[toSlot]->SetItem(mergedItem);

                if (toMove >= fromItem.stackSize) {
                    slots_[fromSlot]->ClearItem();
                } else {
                    InventoryItem remainingItem = fromItem;
                    remainingItem.stackSize -= toMove;
                    slots_[fromSlot]->SetItem(remainingItem);
                }
                return;
            }
        }

        // Otherwise swap
        SwapSlots(fromSlot, toSlot);
    }
}

Color InventoryUI::GetRarityColor(const std::string& rarity) const {
    if (rarity == "Common") return Color::FromHex("#9D9D9D");
    if (rarity == "Uncommon") return Color::FromHex("#1EFF00");
    if (rarity == "Rare") return Color::FromHex("#0070DD");
    if (rarity == "Epic") return Color::FromHex("#A335EE");
    if (rarity == "Legendary") return Color::FromHex("#FF8000");
    return Color::FromHex("#FFFFFF");
}

void InventoryUI::OnPositionChanged() {
    UIElement::OnPositionChanged();
}

void InventoryUI::OnSizeChanged() {
    UIElement::OnSizeChanged();
    UpdateSlotPositions();
}

} // namespace UI
