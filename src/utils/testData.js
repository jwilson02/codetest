/**
 * Test Data for POE Overlay
 * Sample POE items in clipboard format for testing
 */

const testItems = {
  rareArmor: `Rarity: Rare
Gale Knuckle
Titanium Spirit Shield
--------
Quality: +20% (augmented)
Chance to Block: 25%
Energy Shield: 120 (augmented)
--------
Requirements:
Level: 68
Int: 159
--------
Sockets: R-G-B
--------
Item Level: 85
--------
+75 to maximum Life (crafted)
+45% to Fire Resistance
+38% to Cold Resistance
+12% to Lightning Resistance
18% increased Spell Damage
--------
Corrupted`,

  uniqueItem: `Rarity: Unique
Tabula Rasa
Simple Robe
--------
Sockets: W-W-W-W-W-W
--------
Item Level: 1
--------
Has no level requirement and energy shield (implicit)
--------`,

  uniqueWeapon: `Rarity: Unique
Starforge
Infernal Sword
--------
Two Handed Sword
Physical Damage: 489-702 (augmented)
Critical Strike Chance: 5.00%
Attacks per Second: 1.35 (augmented)
Weapon Range: 13
--------
Requirements:
Level: 67
Str: 155
Dex: 113
--------
Sockets: R-R-R-R-R-R
--------
Item Level: 83
--------
30% increased Global Accuracy Rating (implicit)
--------
412% increased Physical Damage
6% increased Attack Speed
+97 to maximum Life
Your Physical Damage can Shock
20% increased Area of Effect for Melee Attacks
Deal no Elemental Damage`,

  currency: `Rarity: Currency
Exalted Orb
--------
Stack Size: 10/10
--------
Enchants a rare item with a new random modifier`,

  gem: `Rarity: Gem
Summon Skeletons
--------
Minion, Spell
Level: 20 (Max)
Quality: +20% (augmented)
Mana Cost: 45
Cast Time: 1.00 sec
Critical Strike Chance: 5.00%
--------
Requirements:
Level: 70
Int: 155
--------
Minions have 40% increased maximum Life
Minions deal 40% increased Damage
Maximum 8 Summoned Skeletons
Minions have +9% to all Elemental Resistances
--------
Experience: 4321345/5321345`,

  map: `Rarity: Rare
Doom Chasm
Burial Chambers Map
--------
Map Tier: 16
Item Quantity: +68% (augmented)
Item Rarity: +33% (augmented)
Monster Pack Size: +26% (augmented)
Quality: +20% (augmented)
--------
Item Level: 83
--------
Area is inhabited by Lunaris fanatics
Monsters are Humanoid
--------
37% more Monster Life
Monsters have 54% increased Critical Strike Chance
+33% Monster Critical Strike Multiplier
Monsters fire 2 additional Projectiles
Players are Cursed with Vulnerability
Monsters reflect 18% of Elemental Damage
--------
Unidentified`,

  fracturedItem: `Rarity: Rare
Storm Sever
Harbinger Bow
--------
Bow
Physical Damage: 73-174 (augmented)
Elemental Damage: 15-45 (augmented)
Critical Strike Chance: 5.00%
Attacks per Second: 1.25
--------
Requirements:
Level: 62
Dex: 212
--------
Sockets: G-G-G-G-G-G
--------
Item Level: 86
--------
26% increased Elemental Damage with Attack Skills (implicit)
--------
Adds 15 to 45 Fire Damage to Attacks (fractured)
156% increased Physical Damage
+45% to Cold Resistance
29% increased Critical Strike Chance
--------`,

  influencedItem: `Rarity: Rare
Corruption Fist
Murder Mitts
--------
Evasion Rating: 234 (augmented)
Energy Shield: 45 (augmented)
--------
Requirements:
Level: 67
Dex: 51
Int: 51
--------
Sockets: G-G-G-G
--------
Item Level: 85
--------
Shaper Item
Hunter Item
--------
+48 to Dexterity
+89 to maximum Life
+43% to Fire Resistance
+38% to Cold Resistance
16% increased Attack Speed
Attacks have +1.2% to Critical Strike Chance (crafted)`,

  divinationCard: `Rarity: Divination Card
The Doctor
--------
Stack Size: 1/8
--------
Headhunter
--------
Time cures all.`
};

/**
 * Get random test item
 */
function getRandomTestItem() {
  const keys = Object.keys(testItems);
  const randomKey = keys[Math.floor(Math.random() * keys.length)];
  return testItems[randomKey];
}

/**
 * Get specific test item by type
 */
function getTestItem(type) {
  return testItems[type] || null;
}

/**
 * List all available test items
 */
function listTestItems() {
  return Object.keys(testItems);
}

module.exports = {
  testItems,
  getRandomTestItem,
  getTestItem,
  listTestItems
};
