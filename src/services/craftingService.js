/**
 * Crafting Service - Comprehensive Path of Exile crafting database and helper
 * Covers Fossils, Essences, Harvest, Eldritch, and more crafting methods
 */

class CraftingService {
  constructor() {
    // Fossil database with modifiers and blocking mechanics
    this.fossils = {
      // Elemental
      'Pristine Fossil': {
        category: 'Life/Mana',
        weight: 'More Life modifiers, No Mana modifiers',
        blocking: ['Mana'],
        bestFor: ['Life-based gear', 'Body Armour', 'Helmets'],
        avgCost: '2-5c'
      },
      'Lucent Fossil': {
        category: 'Life/Mana',
        weight: 'More Mana modifiers, No Life modifiers',
        blocking: ['Life'],
        bestFor: ['Mana-based gear', 'Mind Over Matter builds'],
        avgCost: '1-3c'
      },
      'Scorched Fossil': {
        category: 'Elemental',
        weight: 'More Fire modifiers, No Cold modifiers',
        blocking: ['Cold'],
        bestFor: ['Fire damage weapons', 'Fire skill gear'],
        avgCost: '2-4c'
      },
      'Frigid Fossil': {
        category: 'Elemental',
        weight: 'More Cold modifiers, No Fire modifiers',
        blocking: ['Fire'],
        bestFor: ['Cold damage weapons', 'Cold skill gear'],
        avgCost: '2-4c'
      },
      'Metallic Fossil': {
        category: 'Elemental',
        weight: 'More Lightning modifiers, No Physical modifiers',
        blocking: ['Physical'],
        bestFor: ['Lightning weapons', 'Wands', 'Staves'],
        avgCost: '2-4c'
      },
      // Physical/Chaos
      'Jagged Fossil': {
        category: 'Physical/Chaos',
        weight: 'More Physical modifiers, No Chaos modifiers',
        blocking: ['Chaos'],
        bestFor: ['Physical weapons', 'Attack builds'],
        avgCost: '3-6c'
      },
      'Aberrant Fossil': {
        category: 'Physical/Chaos',
        weight: 'More Chaos modifiers, No Lightning modifiers',
        blocking: ['Lightning'],
        bestFor: ['Chaos damage gear', 'Poison builds'],
        avgCost: '3-6c'
      },
      // Defense
      'Dense Fossil': {
        category: 'Defense',
        weight: 'More Defense modifiers (Armour/ES/Eva), No Life modifiers',
        blocking: ['Life'],
        bestFor: ['Body Armour', 'Helmets', 'Gloves', 'Boots'],
        avgCost: '4-8c'
      },
      'Serrated Fossil': {
        category: 'Attack',
        weight: 'More Attack modifiers, No Caster modifiers',
        blocking: ['Caster'],
        bestFor: ['Attack weapons', 'Jewellery'],
        avgCost: '2-5c'
      },
      // Special
      'Shuddering Fossil': {
        category: 'Speed',
        weight: 'More Speed modifiers (Attack/Cast), No Defense',
        blocking: ['Defense'],
        bestFor: ['Gloves', 'Boots', 'Weapons'],
        avgCost: '3-7c'
      },
      'Bound Fossil': {
        category: 'Defense',
        weight: 'More Defense modifiers, No Attack modifiers',
        blocking: ['Attack'],
        bestFor: ['Defensive gear', 'Caster gear'],
        avgCost: '2-4c'
      },
      'Corroded Fossil': {
        category: 'Bleed/Poison',
        weight: 'More Bleed/Poison modifiers, No Elemental',
        blocking: ['Elemental'],
        bestFor: ['Bleed builds', 'Poison builds'],
        avgCost: '3-6c'
      },
      'Sanctified Fossil': {
        category: 'Numeric',
        weight: 'Improved numeric values on mods',
        blocking: [],
        bestFor: ['High-tier crafting', 'Min-maxing'],
        avgCost: '20-40c'
      },
      'Glyphic Fossil': {
        category: 'Corruption',
        weight: 'Adds Corrupted implicit mod, makes item corrupted',
        blocking: [],
        bestFor: ['Double corruption farming', 'Unique items'],
        avgCost: '5-15c'
      },
      'Tangled Fossil': {
        category: 'Minion',
        weight: 'More Minion modifiers, No Attribute modifiers',
        blocking: ['Attributes'],
        bestFor: ['Minion builds', 'Summoner gear'],
        avgCost: '2-5c'
      },
      'Faceted Fossil': {
        category: 'Gem',
        weight: 'More Gem modifiers, No Mana modifiers',
        blocking: ['Mana'],
        bestFor: ['6-links', 'Helmets', 'Gloves'],
        avgCost: '3-7c'
      }
    };

    // Essence database
    this.essences = {
      // Tiers: Whispering -> Muttering -> Weeping -> Wailing -> Screaming -> Shrieking -> Deafening
      'Essence of Greed': {
        tiers: {
          'Whispering': { mod: '(15-19) to maximum Life', ilvl: 1 },
          'Muttering': { mod: '(20-29) to maximum Life', ilvl: 9 },
          'Weeping': { mod: '(30-39) to maximum Life', ilvl: 20 },
          'Wailing': { mod: '(40-49) to maximum Life', ilvl: 30 },
          'Screaming': { mod: '(50-64) to maximum Life', ilvl: 44 },
          'Shrieking': { mod: '(65-69) to maximum Life', ilvl: 58 },
          'Deafening': { mod: '(70-79) to maximum Life', ilvl: 72 }
        },
        bestFor: ['Life-based builds', 'Body Armour', 'Helmets', 'Belts'],
        strategy: 'Spam Deafening on ilvl 85+ bases for high life rolls'
      },
      'Essence of Woe': {
        tiers: {
          'Whispering': { mod: '(16-25) to maximum Mana', ilvl: 1 },
          'Muttering': { mod: '(26-35) to maximum Mana', ilvl: 9 },
          'Weeping': { mod: '(36-45) to maximum Mana', ilvl: 20 },
          'Wailing': { mod: '(46-55) to maximum Mana', ilvl: 30 },
          'Screaming': { mod: '(56-70) to maximum Mana', ilvl: 44 },
          'Shrieking': { mod: '(71-80) to maximum Mana', ilvl: 58 },
          'Deafening': { mod: '(81-95) to maximum Mana', ilvl: 72 }
        },
        bestFor: ['Mana-based builds', 'Archmage', 'Indigon builds'],
        strategy: 'Use on rings, amulets for MoM builds'
      },
      'Essence of Contempt': {
        tiers: {
          'Deafening': { mod: '(38-42)% increased Attack Speed', ilvl: 72 }
        },
        bestFor: ['Attack weapons', 'Gloves', 'Foils', 'Claws'],
        strategy: 'Guaranteed high attack speed - expensive but powerful'
      },
      'Essence of Zeal': {
        tiers: {
          'Deafening': { mod: '(16-18)% increased Cast Speed', ilvl: 72 }
        },
        bestFor: ['Caster weapons', 'Wands', 'Daggers', 'Sceptres'],
        strategy: 'Best for spell damage weapons'
      },
      'Essence of Rage': {
        tiers: {
          'Deafening': { mod: '+2 to Level of Socketed AoE Gems', ilvl: 72 }
        },
        bestFor: ['6-link Body Armour', 'Helmets', 'Gloves'],
        strategy: 'Meta crafting for AoE skill builds'
      },
      'Essence of Wrath': {
        tiers: {
          'Deafening': { mod: 'Adds (36-40) to (75-80) Lightning Damage', ilvl: 72 }
        },
        bestFor: ['Weapons', 'Rings', 'Amulets'],
        strategy: 'Flat lightning damage for attack builds'
      },
      'Essence of Torment': {
        tiers: {
          'Deafening': { mod: '+2 to Level of Socketed Minion Gems', ilvl: 72 }
        },
        bestFor: ['Minion builds', 'Helmets', 'Body Armour'],
        strategy: 'Essential for summoner builds'
      },
      'Essence of Horror': {
        corruption: true,
        mod: '+1 to Level of Socketed Gems (30% increased Elemental Damage)',
        bestFor: ['6-link Body Armour', 'Weapons', 'Helmets'],
        strategy: 'High-tier corruption essence, very expensive'
      },
      'Essence of Delirium': {
        corruption: true,
        mod: '+2 to Level of Socketed Support Gems',
        bestFor: ['6-link Body Armour', 'Weapons'],
        strategy: 'Extremely powerful for main skill setups'
      },
      'Essence of Insanity': {
        corruption: true,
        mod: '(15-16)% More Attack Speed',
        bestFor: ['Gloves', 'Attack builds'],
        strategy: 'BiS for attack speed on gloves'
      },
      'Essence of Hysteria': {
        corruption: true,
        mod: 'Socketed Gems are Supported by Level 20 Added Fire Damage',
        bestFor: ['Gloves', 'Helmets'],
        strategy: 'Pseudo 7-link for fire builds'
      }
    };

    // Harvest craft database
    this.harvestCrafts = {
      // Augment crafts
      'Augment Fire': {
        type: 'Augment',
        effect: 'Add a Fire modifier to a non-influenced item',
        strategy: 'Target crafting fire resistance or fire damage',
        value: 'High - targeted crafting',
        example: 'Aug fire on ring for fire res'
      },
      'Augment Cold': {
        type: 'Augment',
        effect: 'Add a Cold modifier to a non-influenced item',
        strategy: 'Target crafting cold resistance or cold damage',
        value: 'High',
        example: 'Aug cold on boots for cold res'
      },
      'Augment Lightning': {
        type: 'Augment',
        effect: 'Add a Lightning modifier',
        strategy: 'Target crafting lightning resistance or damage',
        value: 'High',
        example: 'Aug lightning for shock effect'
      },
      'Augment Physical': {
        type: 'Augment',
        effect: 'Add a Physical modifier',
        strategy: 'Add physical damage or reduction',
        value: 'Very High',
        example: 'Aug physical on weapon for flat phys'
      },
      'Augment Chaos': {
        type: 'Augment',
        effect: 'Add a Chaos modifier',
        strategy: 'Add chaos resistance or damage',
        value: 'Very High',
        example: 'Aug chaos for chaos res on gear'
      },
      'Augment Life': {
        type: 'Augment',
        effect: 'Add a Life modifier',
        strategy: 'Guaranteed life roll on open affix',
        value: 'Very High',
        example: 'Aug life on chest for T1 life'
      },
      'Augment Crit': {
        type: 'Augment',
        effect: 'Add a Critical modifier',
        strategy: 'Add crit chance/multi',
        value: 'Extreme',
        example: 'Aug crit on chest for +crit chance'
      },
      'Augment Attack': {
        type: 'Augment',
        effect: 'Add an Attack modifier',
        strategy: 'Add attack speed, accuracy',
        value: 'Very High',
        example: 'Aug attack on gloves for attack speed'
      },
      'Augment Caster': {
        type: 'Augment',
        effect: 'Add a Caster modifier',
        strategy: 'Add cast speed, spell damage',
        value: 'Very High',
        example: 'Aug caster on weapon for cast speed'
      },
      'Augment Speed': {
        type: 'Augment',
        effect: 'Add a Speed modifier',
        strategy: 'Add movement speed, attack/cast speed',
        value: 'Extreme',
        example: 'Aug speed on boots for MS'
      },
      'Augment Influence': {
        type: 'Augment',
        effect: 'Add an influenced modifier',
        strategy: 'Target influenced mods on awakened items',
        value: 'Extreme',
        example: 'Aug influence for tailwind boots'
      },
      // Remove crafts
      'Remove Fire': {
        type: 'Remove',
        effect: 'Remove a Fire modifier',
        strategy: 'Clean unwanted fire mods',
        value: 'Medium',
        example: 'Remove fire res to make room'
      },
      'Remove Cold': {
        type: 'Remove',
        effect: 'Remove a Cold modifier',
        strategy: 'Clean unwanted cold mods',
        value: 'Medium',
        example: 'Remove cold mod safely'
      },
      'Remove Lightning': {
        type: 'Remove',
        effect: 'Remove a Lightning modifier',
        strategy: 'Clean unwanted lightning mods',
        value: 'Medium',
        example: 'Remove lightning mod'
      },
      'Remove Non-Fire Add Fire': {
        type: 'Remove-Add',
        effect: 'Remove non-Fire, add Fire',
        strategy: 'Target fire mods while keeping fire mods safe',
        value: 'High',
        example: 'Safe fire aug without removing fire mods'
      },
      // Reforge crafts
      'Reforge Fire': {
        type: 'Reforge',
        effect: 'Reforge item with at least one Fire modifier',
        strategy: 'Spam for fire-based mods',
        value: 'Medium',
        example: 'Reforge weapon for fire damage'
      },
      'Reforge Crit': {
        type: 'Reforge',
        effect: 'Reforge with at least one Critical modifier',
        strategy: 'Guaranteed crit mod',
        value: 'High',
        example: 'Reforge for base crit'
      },
      'Reforge More Likely': {
        type: 'Reforge',
        effect: 'Reforge with specific tag mods more likely',
        strategy: 'Higher chance of hitting desired mods',
        value: 'High',
        example: 'Reforge life more likely for T1 life'
      },
      'Reforge Keep Prefixes': {
        type: 'Reforge Special',
        effect: 'Reforge item, keep all prefixes',
        strategy: 'Craft prefixes, reroll suffixes',
        value: 'Very High',
        example: 'Keep T1 life/res, reroll suffixes'
      },
      'Reforge Keep Suffixes': {
        type: 'Reforge Special',
        effect: 'Reforge item, keep all suffixes',
        strategy: 'Craft suffixes, reroll prefixes',
        value: 'Very High',
        example: 'Keep T1 res, reroll life prefixes'
      },
      // Special
      'Randomise Numeric Values': {
        type: 'Divine',
        effect: 'Randomize numeric values of mods',
        strategy: 'Pseudo-divine orb',
        value: 'Medium',
        example: 'Divine your item for free'
      },
      'Synthesise Item': {
        type: 'Special',
        effect: 'Create synthesised base',
        strategy: 'Create powerful synthesised items',
        value: 'Extreme',
        example: 'Synth ring with onslaught'
      }
    };

    // Eldritch crafting
    this.eldritchCrafting = {
      'Lesser Eldritch Ember': {
        type: 'Implicit Reforge',
        effect: 'Reroll Searing Exarch implicit',
        influence: 'Searing Exarch',
        avgCost: '1-2c',
        strategy: 'Spam for desired implicit mod'
      },
      'Greater Eldritch Ember': {
        type: 'Implicit Reforge',
        effect: 'Reroll with better Searing Exarch implicit odds',
        influence: 'Searing Exarch',
        avgCost: '5-10c',
        strategy: 'Better odds for high-tier implicits'
      },
      'Grand Eldritch Ember': {
        type: 'Implicit Reforge',
        effect: 'Reroll with best Searing Exarch implicit odds',
        influence: 'Searing Exarch',
        avgCost: '15-30c',
        strategy: 'Best odds for T1 implicits'
      },
      'Exceptional Eldritch Ember': {
        type: 'Implicit Reforge',
        effect: 'Guarantees highest tier Searing Exarch implicit',
        influence: 'Searing Exarch',
        avgCost: '100-200c',
        strategy: 'Guaranteed T0 implicit'
      },
      'Lesser Eldritch Ichor': {
        type: 'Implicit Reforge',
        effect: 'Reroll Eater of Worlds implicit',
        influence: 'Eater of Worlds',
        avgCost: '1-2c',
        strategy: 'Spam for desired implicit mod'
      },
      'Greater Eldritch Ichor': {
        type: 'Implicit Reforge',
        effect: 'Reroll with better Eater implicit odds',
        influence: 'Eater of Worlds',
        avgCost: '5-10c',
        strategy: 'Better odds for high-tier implicits'
      },
      'Grand Eldritch Ichor': {
        type: 'Implicit Reforge',
        effect: 'Reroll with best Eater implicit odds',
        influence: 'Eater of Worlds',
        avgCost: '15-30c',
        strategy: 'Best odds for T1 implicits'
      },
      'Exceptional Eldritch Ichor': {
        type: 'Implicit Reforge',
        effect: 'Guarantees highest tier Eater implicit',
        influence: 'Eater of Worlds',
        avgCost: '100-200c',
        strategy: 'Guaranteed T0 implicit'
      },
      'Eldritch Chaos Orb': {
        type: 'Chaos Orb',
        effect: 'Chaos orb that preserves eldritch implicits',
        avgCost: '10-15c',
        strategy: 'Chaos spam without losing implicits'
      },
      'Eldritch Exalted Orb': {
        type: 'Exalted Orb',
        effect: 'Add a mod that matches implicit influence',
        avgCost: '20-40c',
        strategy: 'Targeted exalt based on implicit influence'
      },
      'Eldritch Orb of Annulment': {
        type: 'Annulment',
        effect: 'Remove mod that matches implicit influence',
        avgCost: '30-50c',
        strategy: 'Safer annul - removes specific influence mods'
      }
    };

    // Popular crafting strategies
    this.strategies = {
      'High Life Body Armour': {
        steps: [
          '1. Get ilvl 86+ Astral Plate base',
          '2. Spam Pristine + Sanctified fossils until T1 life',
          '3. Benchcraft prefixes cannot be changed',
          '4. Harvest reforge keep prefixes for suffixes',
          '5. Divine for perfect rolls'
        ],
        estimatedCost: '2-5 divine',
        difficulty: 'Medium'
      },
      'Tailwind/Elusive Boots': {
        steps: [
          '1. Awakener Orb Hunter + Redeemer boots',
          '2. Hit tailwind + elusive',
          '3. Harvest aug speed for movement speed',
          '4. Aug life for T1 life',
          '5. Craft res or benchcraft'
        ],
        estimatedCost: '10-20 divine',
        difficulty: 'Hard'
      },
      '+2 Minion Helm': {
        steps: [
          '1. Essence of Horror on ilvl 85+ base',
          '2. Hit +2 to socketed minion gems',
          '3. Aug minion for minion life/damage',
          '4. Benchcraft life or res'
        ],
        estimatedCost: '5-10 divine',
        difficulty: 'Medium'
      },
      'High DPS Weapon': {
        steps: [
          '1. Alt spam for T1 flat phys',
          '2. Regal for %phys or attack speed',
          '3. Multi-mod if needed',
          '4. Benchcraft remaining affixes',
          '5. Quality to 30% with fossils'
        ],
        estimatedCost: '5-15 divine',
        difficulty: 'Hard'
      }
    };
  }

  /**
   * Search for crafting methods that can give desired mod
   * @param {string} modName - The modifier to search for
   * @returns {Object}
   */
  searchCraftingMethod(modName) {
    const results = {
      fossils: [],
      essences: [],
      harvest: [],
      eldritch: []
    };

    const searchTerm = modName.toLowerCase();

    // Search fossils
    Object.entries(this.fossils).forEach(([name, data]) => {
      if (name.toLowerCase().includes(searchTerm) ||
          data.category.toLowerCase().includes(searchTerm) ||
          data.bestFor.some(use => use.toLowerCase().includes(searchTerm))) {
        results.fossils.push({ name, ...data });
      }
    });

    // Search essences
    Object.entries(this.essences).forEach(([name, data]) => {
      const tierMods = Object.values(data.tiers || {}).map(t => t.mod).join(' ');
      if (name.toLowerCase().includes(searchTerm) ||
          tierMods.toLowerCase().includes(searchTerm) ||
          data.bestFor?.some(use => use.toLowerCase().includes(searchTerm))) {
        results.essences.push({ name, ...data });
      }
    });

    // Search harvest
    Object.entries(this.harvestCrafts).forEach(([name, data]) => {
      if (name.toLowerCase().includes(searchTerm) ||
          data.effect.toLowerCase().includes(searchTerm) ||
          data.type.toLowerCase().includes(searchTerm)) {
        results.harvest.push({ name, ...data });
      }
    });

    // Search eldritch
    Object.entries(this.eldritchCrafting).forEach(([name, data]) => {
      if (name.toLowerCase().includes(searchTerm) ||
          data.effect.toLowerCase().includes(searchTerm) ||
          data.influence?.toLowerCase().includes(searchTerm)) {
        results.eldritch.push({ name, ...data });
      }
    });

    return results;
  }

  /**
   * Get all fossils by category
   * @param {string} category - Category name
   * @returns {Array}
   */
  getFossilsByCategory(category) {
    return Object.entries(this.fossils)
      .filter(([_, data]) => data.category === category)
      .map(([name, data]) => ({ name, ...data }));
  }

  /**
   * Get essence by tier
   * @param {string} essenceName - Name of essence
   * @param {string} tier - Tier level
   * @returns {Object|null}
   */
  getEssenceTier(essenceName, tier = 'Deafening') {
    const essence = this.essences[essenceName];
    if (!essence) return null;

    return {
      name: essenceName,
      tier: tier,
      mod: essence.tiers[tier],
      bestFor: essence.bestFor,
      strategy: essence.strategy
    };
  }

  /**
   * Get crafting strategy by goal
   * @param {string} goal - Crafting goal
   * @returns {Object|null}
   */
  getStrategy(goal) {
    return this.strategies[goal] || null;
  }

  /**
   * Calculate estimated crafting cost
   * @param {string} method - Crafting method
   * @param {number} attempts - Number of attempts
   * @returns {Object}
   */
  calculateCost(method, attempts = 1) {
    // This would do actual cost calculation based on current prices
    // For now, returning mock data
    return {
      method,
      attempts,
      totalCost: `${attempts * 5}-${attempts * 10} chaos`,
      breakdown: {
        materials: `${attempts * 3}-${attempts * 6} chaos`,
        base: '2-4 chaos'
      }
    };
  }

  /**
   * Get all available crafting methods
   * @returns {Object}
   */
  getAllMethods() {
    return {
      fossils: this.fossils,
      essences: this.essences,
      harvest: this.harvestCrafts,
      eldritch: this.eldritchCrafting,
      strategies: this.strategies
    };
  }

  /**
   * Get fossil combinations recommendations
   * @param {string} itemType - Type of item
   * @returns {Array}
   */
  getFossilCombinations(itemType) {
    const combinations = {
      'weapon': [
        {
          fossils: ['Jagged', 'Corroded', 'Metallic'],
          goal: 'High physical DPS',
          notes: 'Blocks elemental, focuses on physical mods'
        },
        {
          fossils: ['Serrated', 'Shuddering', 'Sanctified'],
          goal: 'High attack speed weapon',
          notes: 'Attack mods + speed + better values'
        }
      ],
      'armour': [
        {
          fossils: ['Pristine', 'Dense', 'Sanctified'],
          goal: 'High life + defense',
          notes: 'Best for life-based builds'
        },
        {
          fossils: ['Dense', 'Lucent', 'Bound'],
          goal: 'High ES',
          notes: 'For CI/Low-life builds'
        }
      ],
      'jewelry': [
        {
          fossils: ['Pristine', 'Sanctified'],
          goal: 'High life ring',
          notes: 'More life, better rolls'
        }
      ]
    };

    return combinations[itemType.toLowerCase()] || [];
  }
}

// Export singleton instance
const craftingService = new CraftingService();
module.exports = craftingService;
module.exports.CraftingService = CraftingService;
