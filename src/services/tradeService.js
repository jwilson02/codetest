/**
 * POE Trade API Service
 * Integrates with official Path of Exile Trade API
 * Handles price checks, searches, and currency exchange
 */

const axios = require('axios');

class TradeService {
  constructor(league = 'Standard') {
    this.baseURL = 'https://www.pathofexile.com/api/trade';
    this.league = league;
    this.rateLimitDelay = 1000; // 1 second between requests
    this.lastRequestTime = 0;
    this.cache = new Map();
    this.cacheTimeout = 300000; // 5 minutes
  }

  /**
   * Set current league
   */
  setLeague(league) {
    this.league = league;
    console.log(`Trade league set to: ${league}`);
  }

  /**
   * Rate limiting helper
   */
  async waitForRateLimit() {
    const now = Date.now();
    const timeSinceLastRequest = now - this.lastRequestTime;

    if (timeSinceLastRequest < this.rateLimitDelay) {
      const waitTime = this.rateLimitDelay - timeSinceLastRequest;
      await new Promise(resolve => setTimeout(resolve, waitTime));
    }

    this.lastRequestTime = Date.now();
  }

  /**
   * Build search query from parsed item
   */
  buildSearchQuery(parsedItem) {
    const query = {
      query: {
        status: {
          option: 'online'
        },
        stats: [
          {
            type: 'and',
            filters: []
          }
        ]
      },
      sort: {
        price: 'asc'
      }
    };

    // Item name/type filter
    if (parsedItem.rarity === 'Unique') {
      query.query.name = parsedItem.name;
      query.query.type = parsedItem.baseName;
    } else if (parsedItem.rarity === 'Rare') {
      query.query.type = parsedItem.baseName;
    } else {
      query.query.type = parsedItem.name;
    }

    // Item filters
    query.query.filters = {};

    // Sockets and links
    if (parsedItem.links && parsedItem.links >= 5) {
      query.query.filters.socket_filters = {
        filters: {
          links: {
            min: parsedItem.links
          }
        }
      };
    }

    // Item level
    if (parsedItem.itemLevel) {
      query.query.filters.misc_filters = {
        filters: {
          ilvl: {
            min: parsedItem.itemLevel,
            max: parsedItem.itemLevel
          }
        }
      };
    }

    // Quality
    if (parsedItem.quality && parsedItem.quality > 0) {
      if (!query.query.filters.misc_filters) {
        query.query.filters.misc_filters = { filters: {} };
      }
      query.query.filters.misc_filters.filters.quality = {
        min: parsedItem.quality
      };
    }

    // Corrupted status
    if (parsedItem.corrupted) {
      if (!query.query.filters.misc_filters) {
        query.query.filters.misc_filters = { filters: {} };
      }
      query.query.filters.misc_filters.filters.corrupted = {
        option: 'true'
      };
    }

    // Influences
    if (parsedItem.influences && parsedItem.influences.length > 0) {
      if (!query.query.filters.misc_filters) {
        query.query.filters.misc_filters = { filters: {} };
      }
      
      parsedItem.influences.forEach(influence => {
        const influenceKey = this.getInfluenceKey(influence);
        if (influenceKey) {
          query.query.filters.misc_filters.filters[influenceKey] = 'true';
        }
      });
    }

    // Add explicit mods for rare items
    if (parsedItem.rarity === 'Rare' && parsedItem.mods.explicit.length > 0) {
      this.addModFilters(query, parsedItem);
    }

    return query;
  }

  /**
   * Get influence filter key
   */
  getInfluenceKey(influence) {
    const influenceMap = {
      'Shaper Item': 'shaper_item',
      'Elder Item': 'elder_item',
      'Crusader Item': 'crusader_item',
      'Redeemer Item': 'redeemer_item',
      'Hunter Item': 'hunter_item',
      'Warlord Item': 'warlord_item',
      'Synthesised Item': 'synthesised_item',
      'Fractured Item': 'fractured_item'
    };
    return influenceMap[influence];
  }

  /**
   * Add mod filters to query
   */
  addModFilters(query, parsedItem) {
    const statFilters = [];

    // Add important mods with their values
    parsedItem.mods.explicit.forEach(mod => {
      if (mod.values && mod.values.length > 0) {
        const statFilter = {
          id: this.getModStatId(mod.text),
          value: {
            min: mod.values[0] * 0.8, // 80% of the value
            max: mod.values[0] * 1.2  // 120% of the value
          }
        };
        statFilters.push(statFilter);
      }
    });

    if (statFilters.length > 0) {
      query.query.stats[0].filters = statFilters;
    }
  }

  /**
   * Get stat ID for mod (simplified - would need full mapping)
   */
  getModStatId(modText) {
    // This would need a complete mapping of mod text to stat IDs
    // For now, return a placeholder
    return 'pseudo.pseudo-total-life'; // Example
  }

  /**
   * Search for items on trade site
   */
  async searchItems(parsedItem) {
    await this.waitForRateLimit();

    const cacheKey = `search_${parsedItem.name}_${parsedItem.baseName}`;
    const cached = this.getFromCache(cacheKey);
    if (cached) {
      return cached;
    }

    try {
      const query = this.buildSearchQuery(parsedItem);
      
      const response = await axios.post(
        `${this.baseURL}/search/${this.league}`,
        query,
        {
          headers: {
            'Content-Type': 'application/json',
            'User-Agent': 'POE-Trade-Overlay/1.0'
          }
        }
      );

      const searchResult = {
        id: response.data.id,
        total: response.data.total,
        result: response.data.result || []
      };

      this.setCache(cacheKey, searchResult);
      return searchResult;

    } catch (error) {
      console.error('Trade search error:', error.message);
      throw error;
    }
  }

  /**
   * Fetch specific listings
   */
  async fetchListings(searchId, listingIds) {
    await this.waitForRateLimit();

    try {
      const ids = listingIds.slice(0, 10).join(','); // Max 10 at a time
      
      const response = await axios.get(
        `${this.baseURL}/fetch/${ids}`,
        {
          params: { query: searchId },
          headers: {
            'User-Agent': 'POE-Trade-Overlay/1.0'
          }
        }
      );

      return response.data.result || [];

    } catch (error) {
      console.error('Fetch listings error:', error.message);
      throw error;
    }
  }

  /**
   * Get price for item
   */
  async getPriceForItem(parsedItem) {
    try {
      // Search for similar items
      const searchResult = await this.searchItems(parsedItem);

      if (!searchResult.result || searchResult.result.length === 0) {
        return {
          found: false,
          message: 'No listings found'
        };
      }

      // Fetch first 10 listings
      const listings = await this.fetchListings(
        searchResult.id,
        searchResult.result.slice(0, 10)
      );

      // Extract prices
      const prices = this.extractPrices(listings);

      return {
        found: true,
        total: searchResult.total,
        prices: prices,
        listings: listings.slice(0, 5) // Return top 5 for display
      };

    } catch (error) {
      console.error('Get price error:', error);
      return {
        found: false,
        error: error.message
      };
    }
  }

  /**
   * Extract and calculate prices from listings
   */
  extractPrices(listings) {
    const priceList = [];

    listings.forEach(listing => {
      if (listing.listing && listing.listing.price) {
        const price = listing.listing.price;
        priceList.push({
          amount: price.amount,
          currency: price.currency,
          type: price.type
        });
      }
    });

    if (priceList.length === 0) {
      return null;
    }

    // Calculate min, max, avg
    const amounts = priceList.map(p => p.amount);
    const currency = priceList[0].currency; // Assume same currency

    return {
      min: Math.min(...amounts),
      max: Math.max(...amounts),
      avg: amounts.reduce((a, b) => a + b, 0) / amounts.length,
      median: this.calculateMedian(amounts),
      currency: currency,
      count: priceList.length
    };
  }

  /**
   * Calculate median price
   */
  calculateMedian(values) {
    const sorted = [...values].sort((a, b) => a - b);
    const mid = Math.floor(sorted.length / 2);
    
    if (sorted.length % 2 === 0) {
      return (sorted[mid - 1] + sorted[mid]) / 2;
    }
    return sorted[mid];
  }

  /**
   * Get currency exchange rates
   */
  async getCurrencyExchange(want, have) {
    await this.waitForRateLimit();

    const cacheKey = `exchange_${want}_${have}`;
    const cached = this.getFromCache(cacheKey);
    if (cached) {
      return cached;
    }

    try {
      const query = {
        exchange: {
          status: { option: 'online' },
          have: [have],
          want: [want]
        }
      };

      const response = await axios.post(
        `${this.baseURL}/exchange/${this.league}`,
        query,
        {
          headers: {
            'Content-Type': 'application/json',
            'User-Agent': 'POE-Trade-Overlay/1.0'
          }
        }
      );

      const exchangeResult = {
        id: response.data.id,
        result: response.data.result || []
      };

      this.setCache(cacheKey, exchangeResult);
      return exchangeResult;

    } catch (error) {
      console.error('Currency exchange error:', error.message);
      throw error;
    }
  }

  /**
   * Get bulk item prices
   */
  async getBulkPrices(itemName) {
    await this.waitForRateLimit();

    try {
      const query = {
        query: {
          status: { option: 'online' },
          type: itemName
        }
      };

      const response = await axios.post(
        `${this.baseURL}/exchange/${this.league}`,
        query,
        {
          headers: {
            'Content-Type': 'application/json',
            'User-Agent': 'POE-Trade-Overlay/1.0'
          }
        }
      );

      return response.data;

    } catch (error) {
      console.error('Bulk price error:', error.message);
      throw error;
    }
  }

  /**
   * Cache helpers
   */
  getFromCache(key) {
    const cached = this.cache.get(key);
    if (cached && Date.now() - cached.timestamp < this.cacheTimeout) {
      return cached.data;
    }
    return null;
  }

  setCache(key, data) {
    this.cache.set(key, {
      data: data,
      timestamp: Date.now()
    });
  }

  clearCache() {
    this.cache.clear();
  }
}

module.exports = TradeService;
