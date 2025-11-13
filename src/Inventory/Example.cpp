// Example usage of the Inventory and Equipment System
// Compile with: g++ -std=c++17 Example.cpp Item.cpp InventorySystem.cpp Equipment.cpp ItemGenerator.cpp ItemDatabase.cpp -o inventory_demo

#include "Item.h"
#include "InventorySystem.h"
#include "Equipment.h"
#include "ItemGenerator.h"
#include "ItemDatabase.h"
#include <iostream>
#include <memory>

using namespace Inventory;

void printSeparator() {
    std::cout << "\n========================================\n\n";
}

void demoBasicInventory() {
    std::cout << "=== BASIC INVENTORY DEMO ===\n\n";

    // Create a 10x10 inventory
    InventorySystem inventory(10, 10);
    inventory.SetGold(500);
    inventory.SetMaxWeight(1000.0f);

    std::cout << "Created inventory: " << inventory.GetWidth() << "x" << inventory.GetHeight() << "\n";
    std::cout << "Starting gold: " << inventory.GetGold() << "\n";

    // Create some items
    auto sword = std::make_shared<Item>("iron_sword", "Iron Sword", ItemType::Weapon);
    sword->SetLevel(10);
    sword->SetRarity(ItemRarity::Uncommon);
    sword->SetValue(150);

    ItemStats swordStats;
    swordStats.physicalDamage = 25;
    swordStats.strength = 5;
    sword->SetBaseStats(swordStats);

    auto potion = std::make_shared<Item>("health_potion", "Health Potion", ItemType::Consumable);
    potion->SetMaxStackSize(99);
    potion->SetStackCount(10);
    potion->SetValue(20);

    // Add items to inventory
    inventory.AddItem(sword);
    inventory.AddItem(potion);

    std::cout << "\nAdded items to inventory\n";
    std::cout << "Free slots: " << inventory.GetFreeSlots() << "/" << inventory.GetCapacity() << "\n";
    std::cout << "Total value: " << inventory.GetTotalValue() + inventory.GetGold() << " gold\n";

    // Find item
    auto foundSword = inventory.FindItem("iron_sword");
    if (foundSword) {
        std::cout << "\nFound item: " << foundSword->GetName() << "\n";
        std::cout << "Tooltip:\n" << foundSword->GetTooltip() << "\n";
    }

    printSeparator();
}

void demoEquipment() {
    std::cout << "=== EQUIPMENT SYSTEM DEMO ===\n\n";

    EquipmentManager equipment;
    equipment.SetPlayerLevel(20);

    // Create helmet
    auto helmet = std::make_shared<Item>("steel_helmet", "Steel Helmet", ItemType::Helmet);
    helmet->SetLevel(15);
    helmet->SetRarity(ItemRarity::Rare);
    helmet->SetArmorType(ArmorType::Heavy);

    ItemStats helmetStats;
    helmetStats.armor = 35;
    helmetStats.health = 60;
    helmetStats.strength = 8;
    helmet->SetBaseStats(helmetStats);

    // Create weapon
    auto weapon = std::make_shared<Item>("flame_sword", "Flame Sword", ItemType::Weapon);
    weapon->SetLevel(18);
    weapon->SetRarity(ItemRarity::Epic);
    weapon->SetWeaponType(WeaponType::Sword);

    ItemStats weaponStats;
    weaponStats.physicalDamage = 45;
    weaponStats.fireDamage = 20;
    weaponStats.strength = 10;
    weapon->SetBaseStats(weaponStats);

    // Equip items
    std::cout << "Equipping items...\n";

    if (equipment.CanEquip(helmet)) {
        equipment.EquipItem(helmet);
        std::cout << "Equipped: " << helmet->GetName() << "\n";
    }

    if (equipment.CanEquip(weapon)) {
        equipment.EquipItem(weapon);
        std::cout << "Equipped: " << weapon->GetName() << "\n";
    }

    // Get total stats
    ItemStats totalStats = equipment.GetTotalStats();
    std::cout << "\nTotal Equipment Stats:\n";
    std::cout << "  Physical Damage: " << totalStats.physicalDamage << "\n";
    std::cout << "  Fire Damage: " << totalStats.fireDamage << "\n";
    std::cout << "  Armor: " << totalStats.armor << "\n";
    std::cout << "  Health: " << totalStats.health << "\n";
    std::cout << "  Strength: " << totalStats.strength << "\n";

    // Print full equipment summary
    std::cout << "\n" << equipment.GetEquipmentSummary() << "\n";

    printSeparator();
}

void demoItemGeneration() {
    std::cout << "=== ITEM GENERATION DEMO ===\n\n";

    ItemGenerator generator;
    generator.SetSeed(12345); // For reproducible results

    std::cout << "Generating random items...\n\n";

    // Generate items at different levels
    for (int level : {5, 15, 25, 35}) {
        ItemGenerationParams params;
        params.level = level;
        params.magicFindBonus = 20.0f;

        auto item = generator.GenerateWeapon(level, ItemRarity::Rare);
        if (item) {
            std::cout << "Level " << level << " Weapon:\n";
            std::cout << "  " << item->GetFullName() << "\n";
            std::cout << "  Rarity: " << ItemRarityToString(item->GetRarity()) << "\n";
            std::cout << "  Physical Damage: " << item->GetTotalStats().physicalDamage << "\n";
            std::cout << "\n";
        }
    }

    // Generate loot pile
    std::cout << "Generating random loot pile (5 items, level 30):\n";
    auto loot = generator.GenerateRandomLoot(5, 30, 50.0f);

    for (size_t i = 0; i < loot.size(); i++) {
        std::cout << (i + 1) << ". " << loot[i]->GetFullName()
                  << " (" << ItemRarityToString(loot[i]->GetRarity()) << ")\n";
    }

    printSeparator();
}

void demoAffixes() {
    std::cout << "=== AFFIX SYSTEM DEMO ===\n\n";

    // Create a base item
    auto sword = std::make_shared<Item>("base_sword", "Sword", ItemType::Weapon);
    sword->SetLevel(20);
    sword->SetRarity(ItemRarity::Epic);

    ItemStats baseStats;
    baseStats.physicalDamage = 40;
    sword->SetBaseStats(baseStats);

    std::cout << "Base item: " << sword->GetName() << "\n";
    std::cout << "Base damage: " << sword->GetBaseStats().physicalDamage << "\n\n";

    // Add prefixes and suffixes
    ItemAffix prefix;
    prefix.name = "Savage";
    prefix.description = "Brutal offensive power";
    prefix.stats.physicalDamage = 18;
    prefix.stats.strength = 12;
    prefix.stats.critDamage = 20;

    ItemAffix suffix;
    suffix.name = "of Slaying";
    suffix.description = "Deadly against foes";
    suffix.stats.physicalDamage = 30;
    suffix.stats.critChance = 12;
    suffix.stats.critDamage = 30;

    sword->AddPrefix(prefix);
    sword->AddSuffix(suffix);

    std::cout << "Enhanced item: " << sword->GetFullName() << "\n";
    std::cout << "Total damage: " << sword->GetTotalStats().physicalDamage << "\n";
    std::cout << "Total crit chance: " << sword->GetTotalStats().critChance << "%\n";
    std::cout << "Total crit damage: " << sword->GetTotalStats().critDamage << "%\n";
    std::cout << "Total strength: " << sword->GetTotalStats().strength << "\n";

    printSeparator();
}

void demoInventoryManagement() {
    std::cout << "=== INVENTORY MANAGEMENT DEMO ===\n\n";

    InventorySystem inventory(5, 5);

    // Add various items
    for (int i = 0; i < 8; i++) {
        auto item = std::make_shared<Item>(
            "item_" + std::to_string(i),
            "Item " + std::to_string(i),
            ItemType::Misc
        );
        item->SetLevel(i * 5 + 1);
        item->SetRarity(static_cast<ItemRarity>(i % 6));
        item->SetValue((i + 1) * 100);
        inventory.AddItem(item);
    }

    std::cout << "Added 8 items to 5x5 inventory\n";
    std::cout << "Item count: " << inventory.GetItemCount() << "\n";
    std::cout << "Free slots: " << inventory.GetFreeSlots() << "\n\n";

    // Get item statistics
    auto rarityCount = inventory.GetItemRarityCount();
    std::cout << "Items by rarity:\n";
    for (const auto& [rarity, count] : rarityCount) {
        std::cout << "  " << ItemRarityToString(rarity) << ": " << count << "\n";
    }

    std::cout << "\n--- Sorting by rarity (descending) ---\n";
    inventory.Sort(SortCriteria::Rarity, false);
    std::cout << "Inventory sorted!\n";

    std::cout << "\n--- Compacting inventory ---\n";
    inventory.CompactInventory();
    std::cout << "Inventory compacted!\n";

    printSeparator();
}

void demoComparison() {
    std::cout << "=== ITEM COMPARISON DEMO ===\n\n";

    // Current equipped sword
    auto currentSword = std::make_shared<Item>("current_sword", "Iron Blade", ItemType::Weapon);
    currentSword->SetLevel(15);
    currentSword->SetRarity(ItemRarity::Uncommon);
    ItemStats currentStats;
    currentStats.physicalDamage = 30;
    currentStats.strength = 8;
    currentStats.critChance = 5;
    currentSword->SetBaseStats(currentStats);

    // New sword found
    auto newSword = std::make_shared<Item>("new_sword", "Steel Slayer", ItemType::Weapon);
    newSword->SetLevel(20);
    newSword->SetRarity(ItemRarity::Rare);
    ItemStats newStats;
    newStats.physicalDamage = 48;
    newStats.strength = 12;
    newStats.critChance = 10;
    newStats.critDamage = 25;
    newSword->SetBaseStats(newStats);

    // Compare items
    auto comparison = EquipmentComparator::Compare(currentSword, newSword);

    std::cout << "Comparing items:\n";
    std::cout << "Current: " << currentSword->GetName()
              << " (" << ItemRarityToString(currentSword->GetRarity()) << ")\n";
    std::cout << "New: " << newSword->GetName()
              << " (" << ItemRarityToString(newSword->GetRarity()) << ")\n\n";

    std::cout << "Stat Differences:\n";
    if (comparison.statDifference.physicalDamage != 0) {
        std::cout << "  Physical Damage: "
                  << (comparison.statDifference.physicalDamage > 0 ? "+" : "")
                  << comparison.statDifference.physicalDamage << "\n";
    }
    if (comparison.statDifference.strength != 0) {
        std::cout << "  Strength: "
                  << (comparison.statDifference.strength > 0 ? "+" : "")
                  << comparison.statDifference.strength << "\n";
    }

    std::cout << "\nOverall Score: " << comparison.overallScore << "\n";
    std::cout << "Recommendation: " << comparison.recommendation << "\n";

    printSeparator();
}

void demoTrading() {
    std::cout << "=== TRADING SYSTEM DEMO ===\n\n";

    TradeSystem trade;

    // Start trade between two players
    trade.StartTrade("Alice", "Bob");
    std::cout << "Trade started between Alice and Bob\n\n";

    // Alice offers items
    auto aliceSword = std::make_shared<Item>("legendary_sword", "Legendary Sword", ItemType::Weapon);
    aliceSword->SetRarity(ItemRarity::Legendary);
    aliceSword->SetValue(1500);

    trade.AddItemToTrade("Alice", aliceSword, 1);
    trade.SetGoldOffer("Alice", 500);

    std::cout << "Alice offers:\n";
    std::cout << "  - " << aliceSword->GetName() << "\n";
    std::cout << "  - 500 gold\n\n";

    // Bob offers items
    auto bobArmor = std::make_shared<Item>("epic_armor", "Epic Chestplate", ItemType::Chest);
    bobArmor->SetRarity(ItemRarity::Epic);
    bobArmor->SetValue(800);

    trade.AddItemToTrade("Bob", bobArmor, 1);
    trade.SetGoldOffer("Bob", 0);

    std::cout << "Bob offers:\n";
    std::cout << "  - " << bobArmor->GetName() << "\n\n";

    // Ready up
    trade.SetReady("Alice", true);
    std::cout << "Alice is ready\n";

    trade.SetReady("Bob", true);
    std::cout << "Bob is ready\n\n";

    // Complete trade
    if (trade.AreBothReady()) {
        if (trade.CompleteTrade()) {
            std::cout << "Trade completed successfully!\n";
        }
    }

    printSeparator();
}

int main() {
    std::cout << "======================================\n";
    std::cout << "  INVENTORY SYSTEM DEMONSTRATION\n";
    std::cout << "======================================\n";

    demoBasicInventory();
    demoEquipment();
    demoItemGeneration();
    demoAffixes();
    demoInventoryManagement();
    demoComparison();
    demoTrading();

    std::cout << "\n=== DEMO COMPLETE ===\n";
    std::cout << "\nFor more examples, see README.md\n";

    return 0;
}
