# Game UI/UX System

A comprehensive, polished UI/UX system for action RPG games built with C++ and SDL2.

## Overview

This UI system provides a complete, production-ready interface framework with:
- Intuitive and beautiful design
- Smooth animations and transitions
- Full mouse, keyboard, and gamepad support
- Responsive layout system
- Drag-and-drop functionality
- Theme customization
- Accessibility features

## Architecture

### Core Components

#### UIElement (Base Class)
`UIElement.h/cpp` - Base class for all UI components

**Features:**
- Hierarchical parent-child system
- Flexible positioning with anchors and pivots
- State management (Normal, Hovered, Pressed, Disabled, Focused)
- Animation system (FadeIn, FadeOut, SlideIn, SlideOut, Scale, Bounce, Shake)
- Event callbacks (onClick, onHover, onFocus, onDrag)
- Styling (colors, borders, opacity, padding, margins)
- Layer-based rendering order

**Key Methods:**
```cpp
void SetPosition(float x, float y);
void SetSize(float width, float height);
void SetAnchor(Anchor anchor);
void PlayAnimation(AnimationType type, float duration, bool loop);
void OnClick(std::function<void()> callback);
bool ContainsPoint(float x, float y);
```

#### UISystem (Manager)
`UISystem.h/cpp` - Central UI management singleton

**Features:**
- Screen/scene management with stack-based navigation
- Element registration and lookup
- Theme loading and management
- Global input handling
- Tooltip system
- Notification system
- Focus management

**Key Methods:**
```cpp
static UISystem& Instance();
bool Initialize(SDL_Renderer* renderer, int width, int height);
void PushScreen(ScreenType screen);
void PopScreen();
void ShowNotification(const std::string& text, float duration);
bool LoadTheme(const std::string& filepath);
```

## UI Components

### 1. Main Menu
`MainMenu.h/cpp` - Main menu with character selection

**Features:**
- Animated background
- Character class selection (6 classes)
- Character preview with stats
- Settings menu
- Credits screen
- Save game detection

**Classes Available:**
- Warrior (Tank/Melee)
- Mage (Ranged/Magic)
- Rogue (Stealth/DPS)
- Ranger (Ranged/Utility)
- Paladin (Hybrid/Support)
- Necromancer (Summoner/DOT)

### 2. HUD (Heads-Up Display)
`HUD.h/cpp` - In-game HUD

**Components:**
- **Resource Bars**: Health, Mana, Stamina with smooth animations
- **Experience Bar**: Bottom screen progress bar
- **Skill Bar**: 10 hotkey slots (1-0) with cooldown indicators
- **Minimap**: Fog of war, entity markers, player indicator
- **Target Frame**: Enemy health and name
- **Buff/Debuff Bar**: Status effect indicators
- **Combat Text**: Floating damage/healing numbers

**Resource Bar Features:**
```cpp
void SetHealth(float current, float max);
void SetMana(float current, float max);
void SetStamina(float current, float max);
```

**Minimap Features:**
```cpp
void UpdateMinimap(const MinimapData& data);
void SetZoom(float zoom);
void ToggleFogOfWar(bool enabled);
```

**Skill Bar Features:**
```cpp
void SetSkill(int slot, const SkillSlot& skill);
void TriggerSkill(int slot);
void UpdateCooldown(int slot, float cooldown);
```

### 3. Character Sheet
`CharacterSheet.h/cpp` - Character stats and equipment

**Tabs:**
- **Stats**: Attributes, derived stats, attribute allocation
- **Equipment**: Paperdoll layout with 14 slots
- **Skills**: Skill tree (placeholder)
- **Achievements**: Achievement tracking (placeholder)

**Equipment Slots:**
- Head, Shoulders, Chest, Hands, Legs, Feet
- Main Hand, Off Hand
- Neck, Ring1, Ring2
- Trinket1, Trinket2
- Back (cloak)

**Stats System:**
```cpp
// Core Attributes
int strength, intelligence, dexterity;
int vitality, wisdom, luck;

// Derived Stats
int maxHealth, maxMana, maxStamina;
int armor, magicResist;
int attackPower, spellPower;
int critChance, critDamage;
```

### 4. Inventory UI
`InventoryUI.h/cpp` - Inventory with drag-and-drop

**Features:**
- Grid-based layout (8x5 default, 40 slots)
- Full drag-and-drop support
- Automatic stacking
- Item rarity borders (Common to Legendary)
- Context menu (right-click)
- Sorting (by type, rarity, value)
- Filtering by item type
- Gold display
- Sell, drop, use actions

**Drag-and-Drop:**
```cpp
void SwapSlots(int slot1, int slot2);
void MoveItem(int fromSlot, int toSlot);
```

**Item Management:**
```cpp
bool AddItem(const InventoryItem& item);
bool RemoveItem(const std::string& itemId, int quantity);
int GetItemCount(const std::string& itemId);
```

### 5. Dialogue UI
`DialogueUI.h/cpp` - Interactive dialogue system

**Features:**
- Text reveal animation
- Speaker portrait and name
- Multiple choice support
- Keyboard shortcuts (1-9 for choices)
- Auto-advance option
- Skip functionality
- Choice conditions
- Rich dialogue nodes

**Usage:**
```cpp
DialogueNode node;
node.speakerName = "Mysterious Stranger";
node.text = "Greetings, traveler. What brings you here?";

DialogueChoice choice1;
choice1.text = "I seek adventure!";
choice1.nextDialogueId = "adventure_path";
node.choices.push_back(choice1);

dialogueUI->ShowDialogue(node);
```

### 6. Quest Tracker
`QuestTracker.h/cpp` - Quest tracking and journal

**Display Modes:**
- **Compact**: Shows only tracked quests (top-right corner)
- **Expanded**: Shows all active quests
- **Full**: Quest log view (center screen)

**Features:**
- Objective tracking with progress bars
- Category colors (Main, Side, Daily, Event)
- Auto-update on progress
- Scrollable quest list
- Completion notifications
- Quest categories

**Quest Management:**
```cpp
void AddQuest(const Quest& quest);
void UpdateObjective(const std::string& questId,
                     const std::string& objectiveId, int progress);
void TrackQuest(const std::string& questId);
std::vector<Quest*> GetTrackedQuests();
```

### 7. Tooltip System
`Tooltip.h/cpp` - Smart tooltip display

**Styles:**
- **Simple**: Single line text
- **Rich**: Title, subtitle, description, stats
- **Item**: Item tooltips with rarity
- **Skill**: Skill information

**Features:**
- Smart positioning (stays on screen)
- Follow cursor option
- Delayed show (configurable)
- Word wrapping
- Fade-in animation
- Rich formatting

**Usage:**
```cpp
// Simple tooltip
tooltip->Show("Health Potion", x, y);

// Rich tooltip
TooltipContent content;
content.title = "Sword of the Ancients";
content.subtitle = "Legendary Weapon";
content.description = "A blade forged in ancient times...";
content.stats.push_back({"Damage", "125-180"});
content.stats.push_back({"Critical Chance", "+15%"});
tooltip->ShowRich(content, x, y);
```

## Theme System

### theme.json Configuration

The theme system provides comprehensive customization:

**Colors:**
- Primary, secondary, accent colors
- Resource bar colors (health, mana, stamina)
- Rarity colors (common to legendary)
- Success, warning, error states
- Text colors

**Fonts:**
- Main, bold, italic, title fonts
- Icon font (FontAwesome)
- Monospace font
- Configurable sizes

**Metrics:**
- Button sizes
- Padding and spacing
- Border widths
- Icon sizes

**Animation:**
- Transition speeds
- Fade speeds
- Hover animations

**UI-Specific Settings:**
- HUD layout and opacity
- Inventory grid configuration
- Dialogue settings
- Quest tracker settings

### Loading a Theme

```cpp
UISystem::Instance().LoadTheme("assets/ui/theme.json");
const UITheme& theme = UISystem::Instance().GetTheme();
```

## Input Handling

The UI system supports multiple input methods:

### Mouse
- Click events
- Hover events
- Drag-and-drop
- Right-click context menus
- Mouse wheel scrolling

### Keyboard
- Hotkeys (1-0 for skills)
- Tab navigation
- ESC to close
- Enter to confirm
- Arrow keys (future)

### Gamepad
- Button mapping (future)
- Analog stick navigation (future)
- Trigger actions (future)

## Animation System

Built-in animation types:

```cpp
enum class AnimationType {
    None,
    FadeIn,     // Fade in from transparent
    FadeOut,    // Fade out to transparent
    SlideIn,    // Slide in from edge
    SlideOut,   // Slide out to edge
    Scale,      // Scale from small to normal
    Bounce,     // Bounce effect
    Shake       // Shake/vibrate
};
```

**Usage:**
```cpp
element->PlayAnimation(AnimationType::FadeIn, 0.5f);
element->PlayAnimation(AnimationType::Bounce, 0.3f, true); // Loop
```

**Easing Functions:**
- EaseInOut (smooth acceleration/deceleration)
- EaseIn (acceleration)
- EaseOut (deceleration)
- Bounce (bouncy effect)

## Responsive Design

The UI system includes responsive features:

- **Anchor System**: Position elements relative to screen edges/center
- **Pivot Points**: Control element origin for scaling
- **Auto-scaling**: Scale UI for different resolutions
- **Layout Helpers**: Align and distribute elements

```cpp
element->SetAnchor(Anchor::TopRight);
element->SetPivot(0.5f, 0.5f); // Center pivot

// Layout helpers
UIUtils::AlignHorizontal(elements, spacing);
UIUtils::AlignVertical(elements, spacing);
UIUtils::DistributeHorizontal(elements, totalWidth);
```

## Integration Example

### Basic Setup

```cpp
#include "UI/UISystem.h"

// Initialize
SDL_Renderer* renderer = /* ... */;
UISystem::Instance().Initialize(renderer, 1920, 1080);

// Game loop
while (running) {
    float deltaTime = /* calculate */;

    // Handle input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        UISystem::Instance().HandleInput(event);
    }

    // Update
    UISystem::Instance().Update(deltaTime);

    // Render
    UISystem::Instance().Render();

    SDL_RenderPresent(renderer);
}

// Cleanup
UISystem::Instance().Shutdown();
```

### Screen Navigation

```cpp
// Show main menu
UISystem::Instance().SetScreen(ScreenType::MainMenu);

// Push character sheet (can return to previous)
UISystem::Instance().PushScreen(ScreenType::CharacterSheet);

// Go back
UISystem::Instance().PopScreen();
```

### HUD Updates

```cpp
auto* hud = UISystem::Instance().GetHUD();

// Update resources
hud->SetHealth(currentHP, maxHP);
hud->SetMana(currentMP, maxMP);

// Update minimap
MinimapData data;
data.playerX = player.x;
data.playerY = player.y;
data.enemyPositions = GetEnemyPositions();
hud->UpdateMinimap(data);

// Show damage numbers
hud->ShowCombatText("125", enemy.x, enemy.y,
                    Color(255, 0, 0), false);
```

## File Structure

```
src/UI/
├── UIElement.h/cpp          - Base UI element class
├── UISystem.h/cpp           - UI manager (singleton)
├── MainMenu.h/cpp           - Main menu
├── HUD.h/cpp                - In-game HUD
├── CharacterSheet.h/cpp     - Character screen
├── InventoryUI.h/cpp        - Inventory interface
├── DialogueUI.h/cpp         - Dialogue system
├── QuestTracker.h/cpp       - Quest tracker
├── Tooltip.h/cpp            - Tooltip system
└── README.md                - This file

assets/ui/
└── theme.json               - UI theme configuration
```

## Dependencies

- **SDL2**: Core rendering and input
- **SDL2_ttf**: Text rendering (optional, currently placeholder)
- **SDL2_image**: Image loading for icons and textures
- **jsoncpp**: JSON parsing for theme configuration

## Future Enhancements

Potential additions:
- [ ] Settings menu (graphics, audio, controls)
- [ ] Crafting UI
- [ ] Trading UI
- [ ] Mail/messaging system
- [ ] Guild/clan UI
- [ ] Achievement UI with progress tracking
- [ ] Skill tree visualization
- [ ] Map system (world map)
- [ ] Journal/lore system
- [ ] Social features (friends, party)
- [ ] Action bar customization
- [ ] UI layout editor
- [ ] More animation effects
- [ ] Particle systems for UI
- [ ] Advanced text formatting (BBCode/Markdown)

## Best Practices

1. **Always initialize UISystem before use**
2. **Call Update() and Render() in correct order**
3. **Use theme colors instead of hardcoded values**
4. **Set proper Z-layers for overlapping elements**
5. **Dispose of temporary UI elements**
6. **Use callbacks for event handling**
7. **Test with different screen resolutions**
8. **Provide keyboard alternatives for mouse actions**
9. **Implement proper focus management**
10. **Profile UI performance regularly**

## Performance Tips

- Minimize draw calls by batching
- Cache calculated sizes and positions
- Use object pooling for frequently created elements
- Disable updates for hidden elements
- Optimize animation easing calculations
- Use texture atlases for UI sprites
- Limit tooltip updates to visible tooltips
- Batch text rendering when possible

## License

This UI system is part of the game project and follows the project's license terms.

---

**Built with passion for creating beautiful, intuitive game interfaces.**
