/**
 * Vendor Recipe Service
 * Comprehensive database of Path of Exile vendor recipes
 * Includes currency, gem, flask, map, and unique item recipes
 */

class VendorRecipeService {
  constructor() {
    this.recipes = this.initializeRecipes();
    this.favorites = new Set();
  }

  /**
   * Initialize all vendor recipes
   */
  initializeRecipes() {
    return [
      // CURRENCY RECIPES
      {
        id: 'chaos-rare-set',
        name: 'Chaos Orb Recipe',
        category: 'currency',
        result: '2 Chaos Orbs (if all items are unidentified), 1 Chaos Orb (if identified)',
        requirements: [
          'Full set of rare items (ilvl 60-74)',
          '1x Amulet',
          '2x Rings',
          '1x Belt',
          '1x Helmet',
          '1x Body Armour',
          '1x Gloves',
          '1x Boots',
          '1x Weapon (2x for dual wield, or 1x two-handed)',
        ],
        notes: 'All items must be rare (yellow). Unidentified items give 2 chaos.',
        profitability: 'high',
        difficulty: 'easy',
        tags: ['chaos', 'currency', 'rare', 'popular'],
      },
      {
        id: 'regal-rare-set',
        name: 'Regal Orb Recipe',
        category: 'currency',
        result: '1 Regal Orb',
        requirements: [
          'Full set of rare items (ilvl 75+)',
          'Same requirements as Chaos recipe but ilvl 75+',
        ],
        notes: 'Higher item level version of Chaos recipe.',
        profitability: 'medium',
        difficulty: 'easy',
        tags: ['regal', 'currency', 'rare'],
      },
      {
        id: 'divine-6link',
        name: 'Divine Orb Recipe',
        category: 'currency',
        result: '1 Divine Orb',
        requirements: ['1x 6-linked item (any rarity)'],
        notes: 'Any item with 6 linked sockets. Best way to get guaranteed Divine Orb.',
        profitability: 'very-high',
        difficulty: 'hard',
        tags: ['divine', 'currency', '6-link', 'valuable'],
      },
      {
        id: 'chromatic-rgb',
        name: 'Chromatic Orb Recipe',
        category: 'currency',
        result: '1 Chromatic Orb',
        requirements: ['1x item with linked Red, Green, and Blue sockets'],
        notes: 'Easiest way to farm Chromatic Orbs. Common drop from early maps.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['chromatic', 'currency', 'sockets'],
      },
      {
        id: 'jeweller-6socket',
        name: "Jeweller's Orb Recipe",
        category: 'currency',
        result: "7 Jeweller's Orbs",
        requirements: ['1x item with 6 sockets'],
        notes: 'Any 6-socket item gives 7 jewellers.',
        profitability: 'medium',
        difficulty: 'easy',
        tags: ['jeweller', 'currency', 'sockets'],
      },
      {
        id: 'gcp-quality-gems',
        name: "Gemcutter's Prism Recipe",
        category: 'currency',
        result: "1 Gemcutter's Prism",
        requirements: ['Gems with total quality of 40% or more'],
        notes: 'Multiple gems can be combined. Example: 4x gems at 10% quality each.',
        profitability: 'medium',
        difficulty: 'easy',
        tags: ['gcp', 'currency', 'gems', 'quality'],
      },
      {
        id: 'chisel-quality-hammer',
        name: "Cartographer's Chisel Recipe",
        category: 'currency',
        result: "1 Cartographer's Chisel",
        requirements: [
          '1x any normal rarity map',
          '1x Stone Hammer/Rock Breaker/Gavel with 20% quality',
        ],
        notes: 'Use quality currency on hammer first, then vendor with normal map.',
        profitability: 'medium',
        difficulty: 'medium',
        tags: ['chisel', 'currency', 'maps'],
      },
      {
        id: 'chance-magic-to-rare',
        name: 'Orb of Chance Recipe',
        category: 'currency',
        result: '5 Orbs of Chance',
        requirements: ['Full set of magic items (same base types as chaos recipe)'],
        notes: 'Not commonly used, better to sell magic items for alteration shards.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['chance', 'currency', 'magic'],
      },

      // SKILL GEM RECIPES
      {
        id: 'quality-gem-level',
        name: 'Quality Gem (Level Reset)',
        category: 'gems',
        result: 'Level 1 gem with quality = (gem level + quality)',
        requirements: [
          '1x gem',
          "1x Gemcutter's Prism",
        ],
        notes: 'Resets gem to level 1 but adds quality. Not recommended unless planning to relevel.',
        profitability: 'situational',
        difficulty: 'easy',
        tags: ['gems', 'quality'],
      },
      {
        id: 'vaal-gem',
        name: 'Vaal Gem Recipe',
        category: 'gems',
        result: 'Vaal version of skill gem',
        requirements: [
          '1x normal skill gem',
          '1x Vaal Orb',
          '1x Sacrifice fragment',
        ],
        notes: 'Only works for gems that have Vaal versions. Not commonly used.',
        profitability: 'low',
        difficulty: 'medium',
        tags: ['gems', 'vaal'],
      },

      // FLASK RECIPES
      {
        id: 'flask-upgrade-quality',
        name: 'Flask Quality Upgrade',
        category: 'flasks',
        result: '1 flask with 5% more quality (max 20%)',
        requirements: [
          '1x flask',
          "1x Glassblower's Bauble",
        ],
        notes: 'Use on white/normal flasks for better value (20% per bauble vs 5% on magic/rare).',
        profitability: 'medium',
        difficulty: 'easy',
        tags: ['flasks', 'quality'],
      },
      {
        id: 'flask-full-set',
        name: 'Glassblower Bauble Recipe',
        category: 'flasks',
        result: "1 Glassblower's Bauble",
        requirements: ['Flasks with combined quality of 40%'],
        notes: 'Similar to GCP recipe but for flasks.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['flasks', 'quality', 'currency'],
      },

      // MAP RECIPES
      {
        id: 'map-upgrade-3to1',
        name: 'Map Upgrade (3:1)',
        category: 'maps',
        result: '1 map of next tier (same region)',
        requirements: ['3x maps of same tier and region'],
        notes: 'Upgrades to random map of next tier. Useful for filling atlas.',
        profitability: 'situational',
        difficulty: 'easy',
        tags: ['maps', 'upgrade'],
      },
      {
        id: 'map-same-tier',
        name: 'Map Reroll (Same Tier)',
        category: 'maps',
        result: '1 different map of same tier',
        requirements: ['3x same map (same tier)'],
        notes: 'Get different map of same tier. Useful if you hate a specific map.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['maps', 'reroll'],
      },
      {
        id: 'map-quality',
        name: 'Map Quality Recipe',
        category: 'maps',
        result: '1 map with +5% quality',
        requirements: [
          '1x map',
          "1x Cartographer's Chisel",
        ],
        notes: 'Use on white maps for 5% quality per chisel, magic/rare get less.',
        profitability: 'medium',
        difficulty: 'easy',
        tags: ['maps', 'quality'],
      },

      // UNIQUE ITEM RECIPES
      {
        id: 'unique-5to1',
        name: 'Unique Item Gamble (5:1)',
        category: 'uniques',
        result: '1 random unique of same item class',
        requirements: ['5x same base unique items'],
        notes: 'Gambling recipe. Can be profitable for expensive unique bases.',
        profitability: 'high-risk',
        difficulty: 'medium',
        tags: ['uniques', 'gamble'],
      },
      {
        id: 'loreweave',
        name: 'Loreweave Recipe',
        category: 'uniques',
        result: '1 Loreweave (unique body armour)',
        requirements: ['60x unique rings'],
        notes: 'Very powerful unique. Save up unique rings throughout league.',
        profitability: 'very-high',
        difficulty: 'medium',
        tags: ['uniques', 'loreweave', 'rings', 'valuable'],
      },
      {
        id: 'the-anima-stone',
        name: 'The Anima Stone Recipe',
        category: 'uniques',
        result: '1 The Anima Stone (unique jewel)',
        requirements: [
          '1x Primordial Might',
          '1x Primordial Eminence',
          '1x Primordial Harmony',
        ],
        notes: 'Golem build enabler. Vendor 3 specific primordial jewels.',
        profitability: 'medium',
        difficulty: 'hard',
        tags: ['uniques', 'jewels', 'golems'],
      },

      // SPECIAL RECIPES
      {
        id: 'quality-weapon-20',
        name: '20% Quality Weapon Recipe',
        category: 'crafting',
        result: 'Weapon with 20% quality',
        requirements: [
          '1x normal (white) weapon',
          '1x Blacksmith\'s Whetstone',
        ],
        notes: 'Use on white weapons before crafting. Each whetstone gives 5% quality.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['crafting', 'quality', 'weapons'],
      },
      {
        id: 'quality-armour-20',
        name: '20% Quality Armour Recipe',
        category: 'crafting',
        result: 'Armour with 20% quality',
        requirements: [
          "1x normal (white) armour piece",
          "1x Armourer's Scrap",
        ],
        notes: 'Use on white armour before crafting. Each scrap gives 5% quality.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['crafting', 'quality', 'armour'],
      },
      {
        id: 'whetstone-weapon-quality',
        name: "Blacksmith's Whetstone Recipe",
        category: 'currency',
        result: "1 Blacksmith's Whetstone",
        requirements: ['Weapons with combined quality of 40%'],
        notes: 'Similar to GCP recipe but for weapons.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['currency', 'quality', 'weapons'],
      },
      {
        id: 'scrap-armour-quality',
        name: "Armourer's Scrap Recipe",
        category: 'currency',
        result: "1 Armourer's Scrap",
        requirements: ['Armour pieces with combined quality of 40%'],
        notes: 'Similar to GCP recipe but for armour.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['currency', 'quality', 'armour'],
      },
      {
        id: 'same-name-magic',
        name: 'Orb of Alchemy Recipe',
        category: 'currency',
        result: '1 Orb of Alchemy',
        requirements: ['3x same base type magic items with same name'],
        notes: 'Not commonly used. Better to use alchs directly on items.',
        profitability: 'low',
        difficulty: 'medium',
        tags: ['currency', 'alchemy', 'magic'],
      },

      // RESISTANCE RECIPES
      {
        id: 'elemental-resistance-ring',
        name: 'Elemental Resistance Ring',
        category: 'crafting',
        result: '1 magic ring with resistance',
        requirements: [
          '1x Iron Ring + Ruby Ring = Fire Resistance Ring',
          '1x Iron Ring + Sapphire Ring = Cold Resistance Ring',
          '1x Iron Ring + Topaz Ring = Lightning Resistance Ring',
        ],
        notes: 'Useful for early game resistance capping.',
        profitability: 'low',
        difficulty: 'easy',
        tags: ['crafting', 'resistance', 'rings'],
      },
      {
        id: 'all-resistance-amulet',
        name: 'Prismatic Ring Recipe',
        category: 'crafting',
        result: '1 Prismatic Ring (+8-10% to all Elemental Resistances)',
        requirements: [
          '1x Iron Ring',
          '1x Ruby Ring',
          '1x Sapphire Ring',
          '1x Topaz Ring',
        ],
        notes: 'Creates base prismatic ring. Good for early game.',
        profitability: 'medium',
        difficulty: 'medium',
        tags: ['crafting', 'resistance', 'rings', 'prismatic'],
      },

      // INFLUENCED ITEM RECIPES (3.27+)
      {
        id: 'maven-orb',
        name: 'Maven\'s Orb Application',
        category: 'crafting',
        result: 'Item with elevated influenced mod',
        requirements: [
          '1x item with influenced mods',
          "1x Maven's Orb",
        ],
        notes: 'Elevates one influenced mod to higher tier. High-end crafting.',
        profitability: 'very-high',
        difficulty: 'hard',
        tags: ['crafting', 'influenced', 'maven', 'end-game'],
      },
    ];
  }

  /**
   * Get all recipes
   */
  getAllRecipes() {
    return this.recipes;
  }

  /**
   * Search recipes by name or result
   */
  searchRecipes(query) {
    if (!query) {
      return this.recipes;
    }

    const lowerQuery = query.toLowerCase();

    return this.recipes.filter(recipe => {
      return (
        recipe.name.toLowerCase().includes(lowerQuery) ||
        recipe.result.toLowerCase().includes(lowerQuery) ||
        recipe.requirements.some(req => req.toLowerCase().includes(lowerQuery)) ||
        recipe.tags.some(tag => tag.toLowerCase().includes(lowerQuery))
      );
    });
  }

  /**
   * Get recipes by category
   */
  getRecipesByCategory(category) {
    if (!category || category === 'all') {
      return this.recipes;
    }

    return this.recipes.filter(recipe => recipe.category === category);
  }

  /**
   * Get recipe by ID
   */
  getRecipeById(id) {
    return this.recipes.find(recipe => recipe.id === id);
  }

  /**
   * Get all categories
   */
  getCategories() {
    const categories = new Set(this.recipes.map(r => r.category));
    return Array.from(categories).sort();
  }

  /**
   * Get recipes by profitability
   */
  getRecipesByProfitability(profitability) {
    return this.recipes.filter(recipe => recipe.profitability === profitability);
  }

  /**
   * Get valuable recipes (high profitability)
   */
  getValuableRecipes() {
    return this.recipes.filter(recipe =>
      recipe.profitability === 'high' ||
      recipe.profitability === 'very-high'
    );
  }

  /**
   * Add recipe to favorites
   */
  addFavorite(recipeId) {
    this.favorites.add(recipeId);
    return true;
  }

  /**
   * Remove recipe from favorites
   */
  removeFavorite(recipeId) {
    return this.favorites.delete(recipeId);
  }

  /**
   * Toggle favorite status
   */
  toggleFavorite(recipeId) {
    if (this.favorites.has(recipeId)) {
      this.favorites.delete(recipeId);
      return false;
    } else {
      this.favorites.add(recipeId);
      return true;
    }
  }

  /**
   * Check if recipe is favorited
   */
  isFavorite(recipeId) {
    return this.favorites.has(recipeId);
  }

  /**
   * Get favorite recipes
   */
  getFavoriteRecipes() {
    return this.recipes.filter(recipe => this.favorites.has(recipe.id));
  }

  /**
   * Export favorites
   */
  exportFavorites() {
    return JSON.stringify(Array.from(this.favorites));
  }

  /**
   * Import favorites
   */
  importFavorites(jsonData) {
    try {
      const favorites = JSON.parse(jsonData);
      this.favorites = new Set(favorites);
      return true;
    } catch (error) {
      console.error('Import favorites error:', error);
      return false;
    }
  }

  /**
   * Get recipe statistics
   */
  getStatistics() {
    return {
      totalRecipes: this.recipes.length,
      categoryCounts: this.getCategoryCounts(),
      profitabilityCounts: this.getProfitabilityCounts(),
      totalFavorites: this.favorites.size,
    };
  }

  /**
   * Get recipe count by category
   */
  getCategoryCounts() {
    const counts = {};

    this.recipes.forEach(recipe => {
      counts[recipe.category] = (counts[recipe.category] || 0) + 1;
    });

    return counts;
  }

  /**
   * Get recipe count by profitability
   */
  getProfitabilityCounts() {
    const counts = {};

    this.recipes.forEach(recipe => {
      counts[recipe.profitability] = (counts[recipe.profitability] || 0) + 1;
    });

    return counts;
  }

  /**
   * Get difficulty indicator
   */
  getDifficultyColor(difficulty) {
    const colors = {
      'easy': '#4caf50',
      'medium': '#ff9800',
      'hard': '#d32f2f',
    };

    return colors[difficulty] || '#666666';
  }

  /**
   * Get profitability indicator
   */
  getProfitabilityColor(profitability) {
    const colors = {
      'very-high': '#ffd700',
      'high': '#4caf50',
      'medium': '#2196f3',
      'low': '#666666',
      'situational': '#9c27b0',
      'high-risk': '#f44336',
    };

    return colors[profitability] || '#666666';
  }
}

module.exports = VendorRecipeService;
