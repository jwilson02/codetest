/**
 * Divination Card Service - Manages POE divination card tracking and data
 * Data source: POE.ninja API + static card database
 */

const axios = require('axios');

const POE_NINJA_BASE_URL = 'https://poe.ninja/api/data';
const CACHE_DURATION = 10 * 60 * 1000; // 10 minutes

class DivinationCardService {
  constructor() {
    this.cache = {
      cards: null,
      cardData: null,
      lastUpdate: null,
    };
    this.userCollection = {}; // User's card collection: { cardName: stackCount }
    this.updateInterval = null;
    this.initializeCardDatabase();
  }

  /**
   * Initialize the static card database with all known divination cards
   */
  initializeCardDatabase() {
    // Comprehensive database of divination cards (sample - expand as needed)
    this.cardDatabase = [
      {
        name: 'The Doctor',
        stackSize: 8,
        reward: 'Headhunter',
        dropLocations: ['Burial Chambers', 'Spider Forest', 'The Asylum'],
        rarity: 'ultra-rare',
        farmingStrategy: 'Farm Burial Chambers with high quantity and pack size. Use scarabs for extra monsters.',
        baseValue: 1500, // chaos orbs
        tags: ['unique', 'belt', 'headhunter'],
      },
      {
        name: 'The Nurse',
        stackSize: 8,
        reward: 'The Doctor',
        dropLocations: ['Tower Map', 'Defiled Cathedral'],
        rarity: 'ultra-rare',
        farmingStrategy: 'Farm Tower Map. Consider using Maven invitation for better drops.',
        baseValue: 150,
        tags: ['divination', 'doctor'],
      },
      {
        name: 'Apothecary',
        stackSize: 5,
        reward: 'Mageblood',
        dropLocations: ['Crimson Temple'],
        rarity: 'ultra-rare',
        farmingStrategy: 'Farm Crimson Temple with maximum quantity. Use scarabs and atlas passives.',
        baseValue: 3000,
        tags: ['unique', 'belt', 'mageblood'],
      },
      {
        name: 'House of Mirrors',
        stackSize: 1,
        reward: 'Mirror of Kalandra',
        dropLocations: ['Any map (extremely rare)'],
        rarity: 'mirror-tier',
        farmingStrategy: 'No specific farming strategy - extremely rare random drop.',
        baseValue: 50000,
        tags: ['currency', 'mirror'],
      },
      {
        name: 'The Fiend',
        stackSize: 11,
        reward: 'Headhunter (corrupted)',
        dropLocations: ['Shrine Map'],
        rarity: 'ultra-rare',
        farmingStrategy: 'Farm Shrine Map with maximum quantity and pack size.',
        baseValue: 1000,
        tags: ['unique', 'belt', 'headhunter', 'corrupted'],
      },
      {
        name: 'Seven Years Bad Luck',
        stackSize: 13,
        reward: 'Fracturing Orb',
        dropLocations: ['Delirium encounters', 'Simulacrum'],
        rarity: 'rare',
        farmingStrategy: 'Run Delirium maps or Simulacrum encounters.',
        baseValue: 30,
        tags: ['currency', 'fracturing'],
      },
      {
        name: 'The Immortal',
        stackSize: 10,
        reward: 'House of Mirrors',
        dropLocations: ['Augmented distant memories'],
        rarity: 'mirror-tier',
        farmingStrategy: 'Complete augmented synthesis maps in distant memories.',
        baseValue: 40000,
        tags: ['divination', 'mirror'],
      },
      {
        name: 'Unrequited Love',
        stackSize: 9,
        reward: 'Sacrificial Garb (6-link)',
        dropLocations: ['Acts 3-11', 'Maps'],
        rarity: 'uncommon',
        farmingStrategy: 'General farming in any content.',
        baseValue: 5,
        tags: ['armour', '6-link'],
      },
      {
        name: 'Humility',
        stackSize: 9,
        reward: 'Tabula Rasa',
        dropLocations: ['Channel Map', 'Aqueduct (Act 9)'],
        rarity: 'common',
        farmingStrategy: 'Farm Aqueduct in Act 9 or Channel Map for early league currency.',
        baseValue: 10,
        tags: ['unique', 'armour', 'tabula'],
      },
      {
        name: 'The Chains that Bind',
        stackSize: 11,
        reward: '6-link Body Armour',
        dropLocations: ['Cells Map', 'Dungeon Map'],
        rarity: 'uncommon',
        farmingStrategy: 'Farm Cells or Dungeon Map for reliable 6-link cards.',
        baseValue: 12,
        tags: ['armour', '6-link'],
      },
      {
        name: 'The Porcupine',
        stackSize: 6,
        reward: '6-link Short Bow',
        dropLocations: ['Overgrown Shrine', 'Thicket Map'],
        rarity: 'common',
        farmingStrategy: 'Easy to farm in Overgrown Shrine or Thicket Map.',
        baseValue: 8,
        tags: ['weapon', '6-link', 'bow'],
      },
      {
        name: 'Abandoned Wealth',
        stackSize: 5,
        reward: '3x Exalted Orb',
        dropLocations: ['Burial Chambers', 'Toxic Sewer'],
        rarity: 'rare',
        farmingStrategy: 'Farm Burial Chambers alongside The Doctor cards.',
        baseValue: 60,
        tags: ['currency', 'exalted'],
      },
      {
        name: 'The Saint\'s Treasure',
        stackSize: 10,
        reward: '2x Exalted Orb',
        dropLocations: ['Vault Map'],
        rarity: 'rare',
        farmingStrategy: 'Farm Vault Map with quantity modifiers.',
        baseValue: 40,
        tags: ['currency', 'exalted'],
      },
      {
        name: 'Chaotic Disposition',
        stackSize: 5,
        reward: '5x Chaos Orb',
        dropLocations: ['Various maps'],
        rarity: 'common',
        farmingStrategy: 'Common drop in most map content.',
        baseValue: 5,
        tags: ['currency', 'chaos'],
      },
      {
        name: 'The Demon',
        stackSize: 6,
        reward: 'Corrupted 6-link Body Armour (item level 100)',
        dropLocations: ['Vaal Temple Map'],
        rarity: 'rare',
        farmingStrategy: 'Farm Vaal Temple with atlas passives.',
        baseValue: 25,
        tags: ['armour', '6-link', 'corrupted'],
      },
      {
        name: 'The Fortunate',
        stackSize: 4,
        reward: 'Unique item (random)',
        dropLocations: ['Any map'],
        rarity: 'uncommon',
        farmingStrategy: 'General farming - random unique outcome.',
        baseValue: 2,
        tags: ['unique', 'random'],
      },
      {
        name: 'Jack in the Box',
        stackSize: 4,
        reward: 'Unique item (random)',
        dropLocations: ['Various maps'],
        rarity: 'common',
        farmingStrategy: 'Very common card with random unique outcome.',
        baseValue: 1,
        tags: ['unique', 'random'],
      },
      {
        name: 'The Opulent',
        stackSize: 5,
        reward: 'Unique ring (random)',
        dropLocations: ['Arsenal Map', 'Museum Map'],
        rarity: 'uncommon',
        farmingStrategy: 'Farm Arsenal or Museum for unique ring cards.',
        baseValue: 5,
        tags: ['unique', 'ring'],
      },
      {
        name: 'Brother\'s Stash',
        stackSize: 5,
        reward: 'Exalted Orb',
        dropLocations: ['Tropical Island Map'],
        rarity: 'rare',
        farmingStrategy: 'Farm Tropical Island with quantity.',
        baseValue: 20,
        tags: ['currency', 'exalted'],
      },
      {
        name: 'The Hoarder',
        stackSize: 12,
        reward: 'Exalted Orb',
        dropLocations: ['Vault Map', 'Residence Map'],
        rarity: 'uncommon',
        farmingStrategy: 'Farm Vault or Residence consistently.',
        baseValue: 20,
        tags: ['currency', 'exalted'],
      },
      {
        name: 'The Celestial Justicar',
        stackSize: 7,
        reward: '6-link Astral Plate',
        dropLocations: ['Shrine Map', 'Relic Chambers'],
        rarity: 'uncommon',
        farmingStrategy: 'Farm Shrine Map for Astral Plate 6-links.',
        baseValue: 15,
        tags: ['armour', '6-link'],
      },
      {
        name: 'A Dab of Ink',
        stackSize: 9,
        reward: 'Voidborn Relic Key',
        dropLocations: ['Voidborn encounters'],
        rarity: 'rare',
        farmingStrategy: 'Engage with Voidborn content in 3.27 league.',
        baseValue: 40,
        tags: ['key', 'voidborn', 'endgame'],
      },
    ];
  }

  /**
   * Start auto-refresh of card prices
   */
  startAutoRefresh() {
    this.fetchCardPrices();
    this.updateInterval = setInterval(() => {
      this.fetchCardPrices();
    }, CACHE_DURATION);
  }

  /**
   * Stop auto-refresh
   */
  stopAutoRefresh() {
    if (this.updateInterval) {
      clearInterval(this.updateInterval);
      this.updateInterval = null;
    }
  }

  /**
   * Fetch current card prices from POE.ninja
   * @param {string} league - The league to fetch data for
   * @returns {Promise<Object>}
   */
  async fetchCardPrices(league = null) {
    try {
      if (!league) {
        league = await this.getCurrentLeague();
      }

      const response = await axios.get(`${POE_NINJA_BASE_URL}/itemoverview`, {
        params: {
          league: league,
          type: 'DivinationCard',
        },
      });

      this.cache.cardData = response.data;
      this.cache.lastUpdate = Date.now();

      console.log(`Divination card prices updated for league: ${league}`);

      return {
        cards: this.cache.cardData,
        league: league,
        lastUpdate: this.cache.lastUpdate,
      };
    } catch (error) {
      console.error('Error fetching divination card prices:', error);

      if (this.cache.cardData) {
        console.log('Returning cached card data');
        return {
          cards: this.cache.cardData,
          league: league,
          lastUpdate: this.cache.lastUpdate,
          cached: true,
        };
      }

      throw new Error(`Failed to fetch card prices: ${error.message}`);
    }
  }

  /**
   * Get current league name
   * @returns {Promise<string>}
   */
  async getCurrentLeague() {
    // Update this based on current league
    return 'Affliction';
  }

  /**
   * Get all cards from database
   * @returns {Array}
   */
  getAllCards() {
    return this.cardDatabase;
  }

  /**
   * Get card info by name
   * @param {string} cardName - Name of the divination card
   * @returns {Object|null}
   */
  getCardInfo(cardName) {
    return this.cardDatabase.find((card) =>
      card.name.toLowerCase() === cardName.toLowerCase()
    ) || null;
  }

  /**
   * Get current market value for a card
   * @param {string} cardName - Name of the card
   * @returns {number|null}
   */
  getCardPrice(cardName) {
    if (!this.cache.cardData || !this.cache.cardData.lines) {
      const cardInfo = this.getCardInfo(cardName);
      return cardInfo ? cardInfo.baseValue : null;
    }

    const cardData = this.cache.cardData.lines.find((card) =>
      card.name.toLowerCase() === cardName.toLowerCase()
    );

    if (cardData) {
      return cardData.chaosValue || cardData.exaltedValue * 20 || 0;
    }

    // Fallback to base value from database
    const cardInfo = this.getCardInfo(cardName);
    return cardInfo ? cardInfo.baseValue : null;
  }

  /**
   * Add cards to user collection
   * @param {string} cardName - Name of the card
   * @param {number} count - Number to add
   */
  addToCollection(cardName, count = 1) {
    if (!this.userCollection[cardName]) {
      this.userCollection[cardName] = 0;
    }
    this.userCollection[cardName] += count;
    this.saveCollection();
  }

  /**
   * Remove cards from user collection
   * @param {string} cardName - Name of the card
   * @param {number} count - Number to remove
   */
  removeFromCollection(cardName, count = 1) {
    if (!this.userCollection[cardName]) return;

    this.userCollection[cardName] = Math.max(0, this.userCollection[cardName] - count);

    if (this.userCollection[cardName] === 0) {
      delete this.userCollection[cardName];
    }

    this.saveCollection();
  }

  /**
   * Set exact count for a card in collection
   * @param {string} cardName - Name of the card
   * @param {number} count - Exact count
   */
  setCollectionCount(cardName, count) {
    if (count <= 0) {
      delete this.userCollection[cardName];
    } else {
      this.userCollection[cardName] = count;
    }
    this.saveCollection();
  }

  /**
   * Get user's collection
   * @returns {Object}
   */
  getCollection() {
    return this.userCollection;
  }

  /**
   * Calculate completion percentage for a card
   * @param {string} cardName - Name of the card
   * @returns {number}
   */
  getCompletionPercentage(cardName) {
    const cardInfo = this.getCardInfo(cardName);
    if (!cardInfo) return 0;

    const currentCount = this.userCollection[cardName] || 0;
    return Math.min(100, (currentCount / cardInfo.stackSize) * 100);
  }

  /**
   * Get all cards with user's collection info
   * @returns {Array}
   */
  getCardsWithProgress() {
    return this.cardDatabase.map((card) => {
      const currentCount = this.userCollection[card.name] || 0;
      const price = this.getCardPrice(card.name);
      const completion = this.getCompletionPercentage(card.name);

      return {
        ...card,
        currentCount,
        completion,
        currentValue: price ? currentCount * price : currentCount * card.baseValue,
        totalValue: price ? card.stackSize * price : card.stackSize * card.baseValue,
        remaining: Math.max(0, card.stackSize - currentCount),
      };
    });
  }

  /**
   * Get cards sorted by profitability
   * @returns {Array}
   */
  getCardsByProfitability() {
    return this.getCardsWithProgress().sort((a, b) => b.totalValue - a.totalValue);
  }

  /**
   * Get cards by drop location
   * @param {string} location - Map or area name
   * @returns {Array}
   */
  getCardsByLocation(location) {
    return this.cardDatabase.filter((card) =>
      card.dropLocations.some((loc) =>
        loc.toLowerCase().includes(location.toLowerCase())
      )
    );
  }

  /**
   * Search cards by name or reward
   * @param {string} query - Search query
   * @returns {Array}
   */
  searchCards(query) {
    const lowerQuery = query.toLowerCase();
    return this.cardDatabase.filter((card) =>
      card.name.toLowerCase().includes(lowerQuery) ||
      card.reward.toLowerCase().includes(lowerQuery) ||
      card.tags.some((tag) => tag.toLowerCase().includes(lowerQuery))
    );
  }

  /**
   * Filter cards by rarity
   * @param {string} rarity - Rarity tier
   * @returns {Array}
   */
  filterByRarity(rarity) {
    return this.cardDatabase.filter((card) => card.rarity === rarity);
  }

  /**
   * Get total value of user's collection
   * @returns {number}
   */
  getTotalCollectionValue() {
    let total = 0;
    for (const [cardName, count] of Object.entries(this.userCollection)) {
      const price = this.getCardPrice(cardName);
      if (price) {
        total += price * count;
      } else {
        const cardInfo = this.getCardInfo(cardName);
        if (cardInfo) {
          total += cardInfo.baseValue * count;
        }
      }
    }
    return total;
  }

  /**
   * Save collection to localStorage
   */
  saveCollection() {
    if (typeof window !== 'undefined' && window.localStorage) {
      localStorage.setItem('poe-divination-collection', JSON.stringify(this.userCollection));
    }
  }

  /**
   * Load collection from localStorage
   */
  loadCollection() {
    if (typeof window !== 'undefined' && window.localStorage) {
      const saved = localStorage.getItem('poe-divination-collection');
      if (saved) {
        this.userCollection = JSON.parse(saved);
      }
    }
  }

  /**
   * Clear entire collection
   */
  clearCollection() {
    this.userCollection = {};
    this.saveCollection();
  }
}

// Export singleton instance
const divinationCardService = new DivinationCardService();
module.exports = divinationCardService;
module.exports.DivinationCardService = DivinationCardService;
