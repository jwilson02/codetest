/**
 * Item Data Service - Fetches and manages unique item data from Maxroll.gg
 */

const Item = require('../../models/Item');
const rateLimiter = require('../utils/rateLimiter');
const cacheService = require('../cache/cacheService');

const MAXROLL_BASE_URL = 'https://maxroll.gg/poe';
const CACHE_TTL = 7200; // 2 hours cache for item data

class ItemDataService {
  constructor() {
    this.rateLimiter = rateLimiter;
  }

  /**
   * Fetch all unique items
   * @param {Object} options - Fetch options
   * @returns {Promise<Array<Item>>}
   */
  async fetchAllItems(options = {}) {
    const cacheKey = 'all_items';

    const cached = cacheService.get(cacheKey);
    if (cached && !options.forceRefresh) {
      console.log('Returning cached item list');
      return cached.map(i => new Item(i));
    }

    try {
      await this.rateLimiter.consume('maxroll', 1);

      // In real implementation, scrape unique item database
      const items = this.getMockItemData();

      cacheService.set(cacheKey, items.map(i => i.toJSON()), CACHE_TTL);
      return items;
    } catch (error) {
      console.error('Error fetching item data:', error);

      const staleCache = cacheService.get(cacheKey, true);
      if (staleCache) {
        return staleCache.map(i => new Item(i));
      }

      throw new Error(`Failed to fetch item data: ${error.message}`);
    }
  }

  /**
   * Fetch item by name
   * @param {string} itemName - Item name
   * @returns {Promise<Item>}
   */
  async fetchItem(itemName) {
    const cacheKey = `item_${itemName.toLowerCase().replace(/\s/g, '_')}`;

    const cached = cacheService.get(cacheKey);
    if (cached) {
      return new Item(cached);
    }

    try {
      await this.rateLimiter.consume('maxroll', 1);

      // In real implementation, scrape specific item page
      const item = this.getMockItemDetail(itemName);

      cacheService.set(cacheKey, item.toJSON(), CACHE_TTL);
      return item;
    } catch (error) {
      console.error(`Error fetching item ${itemName}:`, error);
      throw new Error(`Failed to fetch item: ${error.message}`);
    }
  }

  /**
   * Search items by criteria
   * @param {Object} criteria - Search criteria
   * @returns {Promise<Array<Item>>}
   */
  async searchItems(criteria = {}) {
    const allItems = await this.fetchAllItems();

    return allItems.filter(item => {
      if (criteria.type && item.type !== criteria.type) return false;
      if (criteria.rarity && item.rarity !== criteria.rarity) return false;
      if (criteria.slot && item.slot !== criteria.slot) return false;
      if (criteria.name && !item.name.toLowerCase().includes(criteria.name.toLowerCase())) return false;
      if (criteria.minLevel && item.level < criteria.minLevel) return false;
      if (criteria.maxLevel && item.level > criteria.maxLevel) return false;
      return true;
    });
  }

  /**
   * Get item recommendations for a build
   * @param {string} buildClass - Class name
   * @param {string} mainStat - Main stat (strength, dexterity, intelligence)
   * @returns {Promise<Array<Item>>}
   */
  async getItemRecommendations(buildClass, mainStat = null) {
    const allItems = await this.fetchAllItems();

    // Filter items suitable for the class/build
    // This is a simplified recommendation system
    return allItems.filter(item => {
      if (mainStat && item.requirements[mainStat] > 200) return false;
      return true;
    }).slice(0, 20);
  }

  /**
   * Get budget alternatives for an item
   * @param {string} itemName - Item name
   * @returns {Promise<Array<Item>>}
   */
  async getBudgetAlternatives(itemName) {
    const item = await this.fetchItem(itemName);
    const allItems = await this.fetchAllItems();

    // Find items of same type with lower price
    return allItems.filter(i =>
      i.type === item.type &&
      i.slot === item.slot &&
      i.price.chaos < item.price.chaos &&
      i.name !== item.name
    ).slice(0, 5);
  }

  /**
   * Mock item data
   * @private
   */
  getMockItemData() {
    return [
      new Item({
        id: 'covenant',
        name: 'The Covenant',
        type: 'Body Armour',
        rarity: 'Unique',
        basetype: 'Spidersilk Robe',
        level: 53,
        itemLevel: 68,
        slot: 'Body',
        implicits: [],
        explicits: [
          'Socketed Gems are Supported by Level 29 Added Chaos Damage',
          '+250 to Evasion Rating',
          '+70 to maximum Life',
          '+15% to Chaos Resistance',
          'Blood Magic'
        ],
        requirements: { level: 53, strength: 0, dexterity: 86, intelligence: 100 },
        flavorText: 'To this end, I pledge my undying service.',
        price: { chaos: 150, divine: 0 },
        importance: 'Required',
        url: 'https://maxroll.gg/poe/items/the-covenant'
      }),
      new Item({
        id: 'circle-of-nostalgia',
        name: 'Circle of Nostalgia',
        type: 'Ring',
        rarity: 'Unique',
        basetype: 'Synthesised Ring',
        level: 60,
        itemLevel: 75,
        slot: 'Ring',
        implicits: ['Curse Enemies with Level 10 Herald of Agony on Hit'],
        explicits: [
          'Herald of Agony has (40-60)% increased Buff Effect',
          '(40-60)% reduced Herald of Agony Mana Reservation',
          'Added Small Passive Skills grant: 15% increased Chaos Damage',
          '+(10-20) to all Attributes'
        ],
        requirements: { level: 60, strength: 0, dexterity: 40, intelligence: 40 },
        price: { chaos: 0, divine: 2 },
        importance: 'High',
        url: 'https://maxroll.gg/poe/items/circle-of-nostalgia'
      }),
      new Item({
        id: 'heatshiver',
        name: 'Heatshiver',
        type: 'Helmet',
        rarity: 'Unique',
        basetype: 'Leather Hood',
        level: 35,
        itemLevel: 68,
        slot: 'Helmet',
        implicits: [],
        explicits: [
          '+180 to Evasion Rating',
          '+(40-50) to maximum Mana',
          '+(30-40)% to Cold Resistance',
          '+(30-40)% to Lightning Resistance',
          'Damage Penetrates 25% Cold Resistance against Frozen Enemies',
          'Damage Penetrates 25% Lightning Resistance against Shocked Enemies'
        ],
        requirements: { level: 35, strength: 0, dexterity: 39, intelligence: 0 },
        price: { chaos: 20, divine: 0 },
        importance: 'Medium',
        url: 'https://maxroll.gg/poe/items/heatshiver'
      }),
      new Item({
        id: 'shavronne-wrappings',
        name: "Shavronne's Wrappings",
        type: 'Body Armour',
        rarity: 'Unique',
        basetype: 'Occultist\'s Vestment',
        level: 62,
        itemLevel: 68,
        slot: 'Body',
        implicits: [],
        explicits: [
          '+3% to all maximum Resistances',
          '+(200-250) to maximum Energy Shield',
          'Reflects 1 to 250 Lightning Damage to Melee Attackers',
          '+(140-170)% to Lightning Resistance',
          'Chaos Damage does not bypass Energy Shield',
          '+(100-150) to maximum Mana'
        ],
        requirements: { level: 62, strength: 0, dexterity: 0, intelligence: 180 },
        price: { chaos: 0, divine: 12 },
        importance: 'Required',
        url: 'https://maxroll.gg/poe/items/shavronnes-wrappings'
      }),
      new Item({
        id: 'kaoms-heart',
        name: "Kaom's Heart",
        type: 'Body Armour',
        rarity: 'Unique',
        basetype: 'Glorious Plate',
        level: 68,
        itemLevel: 68,
        slot: 'Body',
        implicits: [],
        explicits: [
          'Has no Sockets',
          '+500 to maximum Life',
          '+(20-40)% to Fire Resistance'
        ],
        requirements: { level: 68, strength: 191, dexterity: 0, intelligence: 0 },
        price: { chaos: 0, divine: 5 },
        importance: 'High',
        url: 'https://maxroll.gg/poe/items/kaoms-heart'
      }),
      new Item({
        id: 'astramentis',
        name: 'Astramentis',
        type: 'Amulet',
        rarity: 'Unique',
        basetype: 'Onyx Amulet',
        level: 20,
        itemLevel: 68,
        slot: 'Amulet',
        implicits: ['+(10-16) to all Attributes'],
        explicits: [
          '+(80-100) to all Attributes',
          '-4 Physical Damage taken from Attacks'
        ],
        requirements: { level: 20, strength: 0, dexterity: 0, intelligence: 0 },
        price: { chaos: 45, divine: 0 },
        importance: 'Medium',
        url: 'https://maxroll.gg/poe/items/astramentis'
      }),
      new Item({
        id: 'doryani-invitation',
        name: "Doryani's Invitation",
        type: 'Belt',
        rarity: 'Unique',
        basetype: 'Heavy Belt',
        level: 68,
        itemLevel: 75,
        slot: 'Belt',
        implicits: ['+25 to Strength'],
        explicits: [
          '+(25-35) to Strength',
          '+(30-35)% to Cold Resistance',
          '1% of Elemental Damage Leeched as Life',
          '25% reduced Enemy Stun Threshold'
        ],
        requirements: { level: 68, strength: 0, dexterity: 0, intelligence: 0 },
        price: { chaos: 180, divine: 0 },
        importance: 'High',
        url: 'https://maxroll.gg/poe/items/doryanis-invitation'
      })
    ];
  }

  /**
   * Mock item detail
   * @private
   */
  getMockItemDetail(itemName) {
    const items = this.getMockItemData();
    return items.find(i => i.name.toLowerCase().includes(itemName.toLowerCase())) || items[0];
  }
}

const itemDataService = new ItemDataService();
module.exports = itemDataService;
module.exports.ItemDataService = ItemDataService;
