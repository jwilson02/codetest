# Comprehensive Inventory and Equipment System

A fully-featured ARPG inventory system with hundreds of items, random generation, equipment management, trading, and storage.

## Features

### 1. Grid-Based Inventory System
- Customizable grid size (default 10x10)
- Auto-stacking for stackable items
- Auto-sorting by multiple criteria (name, type, rarity, level, value, weight)
- Drag-and-drop support
- Quick slots / hotbar system
- Weight management
- Gold/currency tracking

### 2. Equipment System
- Multiple equipment slots:
  - Main Hand / Off Hand (weapon slots)
  - Helmet, Chest, Legs, Boots
  - Gloves, Belt, Cape
  - Amulet
  - Ring 1, Ring 2
  - Earring 1, Earring 2
  - Pet, Mount (optional)
- Two-handed weapon handling
- Equipment comparison tooltips
- Durability system
- Set item bonuses
- Socket system for gems

### 3. Item Rarity System
Six tiers of rarity:
- **Common** (White) - Basic items
- **Uncommon** (Green) - Slightly better stats
- **Rare** (Blue) - Good quality with affixes
- **Epic** (Purple) - Powerful items with multiple affixes
- **Legendary** (Orange) - Extremely powerful unique items
- **Mythic** (Pink) - The rarest and most powerful items

### 4. Item Stat Generation
Over 40 different item stats including:
- **Primary Stats**: Strength, Dexterity, Intelligence, Vitality, Wisdom, Luck
- **Combat Stats**: Physical Damage, Magic Damage, Armor, Magic Resist
- **Critical Stats**: Crit Chance, Crit Damage
- **Elemental Damage**: Fire, Ice, Lightning, Poison, Holy, Dark
- **Elemental Resistance**: Resist for each damage type
- **Special Stats**: Attack Speed, Movement Speed, Gold Find, Magic Find, Experience Gain

### 5. Random Affix System
- **Prefixes**: Add bonuses at the start of item name (e.g., "Mighty Sword")
- **Suffixes**: Add bonuses at the end of item name (e.g., "Sword of Power")
- Rarity determines max affixes:
  - Common: 0 affixes
  - Uncommon: 1 prefix + 1 suffix
  - Rare: 1 prefix + 2 suffixes
  - Epic: 2 prefixes + 2 suffixes
  - Legendary: 2 prefixes + 3 suffixes
  - Mythic: 3 prefixes + 3 suffixes

### 6. Set Items and Set Bonuses
- Items can belong to equipment sets
- Wearing multiple pieces grants progressive bonuses
- Example: Dragon Set
  - 2 pieces: +50 Fire Damage
  - 4 pieces: +100 Fire Damage, +30% Fire Resist
  - 6 pieces: +200 Fire Damage, Immune to Burn, Summon Dragon ability

### 7. Trading System
- Secure player-to-player trading
- Add items and gold to trade window
- Both players must ready up before trade completes
- Anti-scam measures built-in

### 8. Bank/Storage System
- Expanded storage (default 20x20)
- Multiple storage tabs
- Transfer items between inventory and storage
- Shared storage option (between characters)

### 9. Crafting System
- Recipe-based crafting
- Consume materials to create items
- Level requirements for recipes
- Quality tiers based on materials

### 10. Item Upgrade System
- Upgrade items up to +15
- Each upgrade increases stats by 10%
- Success chance decreases at higher levels
- Add sockets to items
- Reroll item affixes
- Upgrade item rarity

## File Structure

```
src/Inventory/
├── Item.h/cpp                 - Core item class with all properties
├── InventorySystem.h/cpp      - Grid inventory, stacking, sorting
├── Equipment.h/cpp            - Equipment management, slots, bonuses
├── ItemGenerator.h/cpp        - Random item generation, loot tables
├── ItemDatabase.h/cpp         - Item definitions loading from JSON
└── README.md                  - This file

data/
├── items/
│   ├── weapons.json           - 200+ weapon definitions
│   ├── armor.json             - 200+ armor piece definitions
│   ├── consumables.json       - 100+ potions, scrolls, food
│   └── crafting_materials.json - Ores, gems, essences, etc.
└── item_affixes.json          - Prefix/suffix stat modifiers
```

## Usage Examples

### Creating an Inventory

```cpp
#include "Inventory/InventorySystem.h"

// Create a 10x10 inventory
Inventory::InventorySystem inventory(10, 10);
inventory.SetMaxWeight(1000.0f);
inventory.SetGold(100);

// Add an item
auto sword = std::make_shared<Inventory::Item>("iron_sword", "Iron Sword", Inventory::ItemType::Weapon);
inventory.AddItem(sword);

// Sort inventory
inventory.Sort(Inventory::SortCriteria::Rarity, false); // Sort by rarity descending

// Auto-compact
inventory.CompactInventory();
```

### Equipment Management

```cpp
#include "Inventory/Equipment.h"

// Create equipment manager
Inventory::EquipmentManager equipment;
equipment.SetPlayerLevel(20);

// Equip an item
auto helmet = database.CreateItem("knights_helm");
if (equipment.CanEquip(helmet)) {
    equipment.EquipItem(helmet);
}

// Get total stats from all equipment
Inventory::ItemStats totalStats = equipment.GetTotalStats();
std::cout << "Total Armor: " << totalStats.armor << std::endl;
std::cout << "Total Strength: " << totalStats.strength << std::endl;

// Check for set bonuses
auto activeSets = equipment.GetActiveSets();
for (const auto& [setId, pieceCount] : activeSets) {
    std::cout << setId << ": " << pieceCount << " pieces equipped\n";
}
```

### Random Item Generation

```cpp
#include "Inventory/ItemGenerator.h"
#include "Inventory/ItemDatabase.h"

// Load item database
Inventory::ItemDatabase database;
database.LoadAll("data");

// Create generator
Inventory::ItemGenerator generator;
generator.SetDatabase(&database);

// Generate random items
Inventory::ItemGenerationParams params;
params.level = 25;
params.minRarity = Inventory::ItemRarity::Rare;
params.maxRarity = Inventory::ItemRarity::Legendary;
params.magicFindBonus = 50.0f; // +50% magic find

auto randomItem = generator.GenerateItem(params);
std::cout << "Generated: " << randomItem->GetFullName() << std::endl;
std::cout << "Rarity: " << Inventory::ItemRarityToString(randomItem->GetRarity()) << std::endl;

// Generate loot from a table
auto loot = generator.GenerateRandomLoot(5, 30, 50.0f); // 5 items, level 30, +50% MF
```

### Item Comparison

```cpp
#include "Inventory/Equipment.h"

// Compare two items
auto currentSword = equipment.GetEquippedItem(Inventory::EquipmentSlot::MainHand);
auto newSword = inventory.FindItem("legendary_sword");

auto comparison = Inventory::EquipmentComparator::Compare(currentSword, newSword);

std::cout << "Comparison Score: " << comparison.overallScore << std::endl;
std::cout << "Recommendation: " << comparison.recommendation << std::endl;

if (comparison.statDifference.physicalDamage > 0) {
    std::cout << "+" << comparison.statDifference.physicalDamage << " Physical Damage\n";
}
```

### Trading

```cpp
#include "Inventory/InventorySystem.h"

// Initialize trade
Inventory::TradeSystem trade;
trade.StartTrade("player1", "player2");

// Player 1 adds items
auto itemToTrade = player1Inv.FindItem("legendary_sword");
trade.AddItemToTrade("player1", itemToTrade, 1);
trade.SetGoldOffer("player1", 1000);

// Player 2 adds items
auto itemToTrade2 = player2Inv.FindItem("epic_armor");
trade.AddItemToTrade("player2", itemToTrade2, 1);

// Both players ready up
trade.SetReady("player1", true);
trade.SetReady("player2", true);

// Complete trade
if (trade.AreBothReady()) {
    trade.CompleteTrade();
}
```

### Bank/Storage

```cpp
#include "Inventory/InventorySystem.h"

// Create storage
Inventory::StorageSystem storage(20, 20);
storage.CreateTab("Weapons", 10, 10);
storage.CreateTab("Armor", 10, 10);

// Transfer item to storage
storage.TransferToStorage(playerInventory, 5, 3); // Transfer item at (5,3)

// Transfer all items
storage.TransferAll(playerInventory);

// Switch storage tabs
storage.SelectTab("Weapons");
```

### Crafting

```cpp
#include "Inventory/ItemGenerator.h"

Inventory::CraftingSystem crafting;

// Define a recipe
Inventory::CraftingSystem::CraftingRecipe recipe;
recipe.id = "craft_iron_sword";
recipe.name = "Craft Iron Sword";
recipe.resultItemId = "iron_sword";
recipe.materials["iron_ore"] = 5;
recipe.materials["leather_scraps"] = 2;
recipe.requiredLevel = 5;
recipe.craftingCost = 50;
recipe.resultRarity = Inventory::ItemRarity::Common;

crafting.RegisterRecipe(recipe);

// Craft the item
if (crafting.CanCraft(recipe, inventory, playerLevel)) {
    auto craftedItem = crafting.Craft(recipe, inventory, generator);
    inventory.AddItem(craftedItem);
}
```

## Item Properties

### Core Properties
- **ID**: Unique identifier
- **Name**: Display name
- **Description**: Tooltip description
- **Type**: Weapon, Armor, Consumable, etc.
- **Rarity**: Common through Mythic
- **Level**: Item level
- **Value**: Gold value
- **Weight**: Affects carrying capacity
- **Stack Size**: Max stack for consumables

### Item Flags
- **IsEquippable**: Can be equipped
- **IsStackable**: Can stack with identical items
- **IsTradeable**: Can be traded to other players
- **IsQuestItem**: Quest-related (usually non-droppable)
- **IsUnique**: Unique/legendary item

### Special Systems
- **Sockets**: Slots for gems (0-4 sockets)
- **Durability**: Equipment degrades with use
- **Requirements**: Level and stat requirements
- **Special Effects**: Unique item abilities

## Item Database JSON Format

### Weapon Example
```json
{
  "id": "flamebrand",
  "name": "Flamebrand",
  "type": "Weapon",
  "weaponType": "Sword",
  "level": 25,
  "rarity": "Epic",
  "description": "A sword wreathed in eternal flames.",
  "stats": {
    "physicalDamage": 55,
    "fireDamage": 25,
    "strength": 10,
    "critDamage": 15
  },
  "value": 800,
  "weight": 4.5,
  "specialEffect": "Attacks have a 20% chance to burn enemies for 5 seconds",
  "maxSockets": 2
}
```

### Affix Example
```json
{
  "name": "Savage",
  "description": "Brutal offensive power",
  "minLevel": 20,
  "maxLevel": 100,
  "weight": 0.7,
  "stats": {
    "physicalDamage": 18,
    "strength": 12,
    "critDamage": 20
  }
}
```

## Performance Considerations

- Items use shared_ptr for memory management
- Inventory uses 2D vector for O(1) slot access
- Item database caches frequently accessed items
- Stacking reduces memory usage for consumables
- Auto-sorting uses efficient std::sort with custom comparators

## Extension Points

### Adding New Item Types
1. Add to `ItemType` enum in Item.h
2. Update `ItemTypeToString()` and `StringToItemType()`
3. Add handling in ItemGenerator
4. Create JSON definitions

### Adding New Stats
1. Add to `ItemStats` struct in Item.h
2. Update `operator+` and `operator+=`
3. Add to tooltip generation in `GetTooltip()`
4. Include in comparison logic

### Creating Custom Loot Tables
```cpp
Inventory::LootTable bossLoot;
bossLoot.id = "dragon_boss";
bossLoot.minItems = 3;
bossLoot.maxItems = 5;

Inventory::LootTableEntry entry;
entry.itemId = "legendary_sword";
entry.weight = 0.05f; // 5% chance
entry.minQuantity = 1;
entry.maxQuantity = 1;
entry.minLevel = 30;
entry.maxLevel = 40;

bossLoot.entries.push_back(entry);
generator.RegisterLootTable(bossLoot);
```

## Notes

- JSON parsing requires a library like nlohmann/json or rapidjson (not included)
- Item sprites/models referenced by paths but not included
- Save/load serialization requires additional implementation
- Network synchronization for multiplayer requires protocol implementation
- The system is designed to be extended with your game's specific needs

## Future Enhancements

Potential additions:
- Item enchanting system
- Transmog/cosmetic overrides
- Item dyeing/coloring
- Rune system (alternative to gems)
- Legendary item quests
- Item evolution/upgrading
- Mythic affixes (special modifiers)
- Corruption system (powerful but risky)
- Item crafting combinations
- Salvaging/disenchanting items

---

Created for ARPG game development. Enjoy building amazing loot systems!
