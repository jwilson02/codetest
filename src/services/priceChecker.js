/**
 * Price Checker Service
 * Combines item parsing and trade API to provide price checks
 */

const ItemParser = require('./itemParser');
const TradeService = require('./tradeService');

class PriceChecker {
  constructor(league = 'Standard') {
    this.itemParser = new ItemParser();
    this.tradeService = new TradeService(league);
    this.priceHistory = new Map();
  }

  /**
   * Set current league
   */
  setLeague(league) {
    this.tradeService.setLeague(league);
  }

  /**
   * Main price check function
   */
  async checkPrice(itemText) {
    try {
      // Parse the item
      const parsedItem = this.itemParser.parseItem(itemText);
      
      if (!parsedItem) {
        return {
          success: false,
          error: 'Failed to parse item'
        };
      }

      // Get item category
      const category = this.itemParser.getItemCategory(parsedItem);

      // Handle different item types
      let priceData;
      
      if (category === 'currency') {
        priceData = await this.checkCurrencyPrice(parsedItem);
      } else if (category === 'gem') {
        priceData = await this.checkGemPrice(parsedItem);
      } else if (category === 'map') {
        priceData = await this.checkMapPrice(parsedItem);
      } else if (category === 'card') {
        priceData = await this.checkCardPrice(parsedItem);
      } else {
        priceData = await this.checkEquipmentPrice(parsedItem);
      }

      // Store in history
      this.addToPriceHistory(parsedItem, priceData);

      return {
        success: true,
        item: parsedItem,
        category: category,
        priceData: priceData
      };

    } catch (error) {
      console.error('Price check error:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  /**
   * Check price for equipment (armor, weapons, jewelry)
   */
  async checkEquipmentPrice(parsedItem) {
    const result = await this.tradeService.getPriceForItem(parsedItem);

    if (!result.found) {
      return {
        available: false,
        message: result.message || 'No price data available'
      };
    }

    return {
      available: true,
      total: result.total,
      prices: result.prices,
      listings: result.listings,
      recommendation: this.getPriceRecommendation(result.prices)
    };
  }

  /**
   * Check price for currency
   */
  async checkCurrencyPrice(parsedItem) {
    // For currency, we check exchange rates
    // This is a simplified version
    return {
      available: true,
      type: 'currency',
      message: 'Use currency exchange for accurate rates'
    };
  }

  /**
   * Check price for gems
   */
  async checkGemPrice(parsedItem) {
    const result = await this.tradeService.getPriceForItem(parsedItem);

    if (!result.found) {
      return {
        available: false,
        message: 'No gem listings found'
      };
    }

    return {
      available: true,
      total: result.total,
      prices: result.prices,
      listings: result.listings,
      recommendation: this.getPriceRecommendation(result.prices)
    };
  }

  /**
   * Check price for maps
   */
  async checkMapPrice(parsedItem) {
    const result = await this.tradeService.getPriceForItem(parsedItem);

    if (!result.found) {
      return {
        available: false,
        message: 'No map listings found'
      };
    }

    return {
      available: true,
      total: result.total,
      prices: result.prices,
      listings: result.listings,
      recommendation: this.getPriceRecommendation(result.prices)
    };
  }

  /**
   * Check price for divination cards
   */
  async checkCardPrice(parsedItem) {
    const result = await this.tradeService.getPriceForItem(parsedItem);

    if (!result.found) {
      return {
        available: false,
        message: 'No card listings found'
      };
    }

    return {
      available: true,
      total: result.total,
      prices: result.prices,
      listings: result.listings,
      recommendation: this.getPriceRecommendation(result.prices)
    };
  }

  /**
   * Get price recommendation based on market data
   */
  getPriceRecommendation(prices) {
    if (!prices || !prices.avg) {
      return null;
    }

    const { min, max, avg, median, currency } = prices;

    // Calculate confidence based on price spread
    const spread = max - min;
    const spreadPercent = (spread / avg) * 100;

    let confidence = 'high';
    if (spreadPercent > 50) {
      confidence = 'low';
    } else if (spreadPercent > 25) {
      confidence = 'medium';
    }

    // Suggest price ranges
    return {
      quick_sell: Math.floor(min * 0.9 * 100) / 100,
      fair_price: Math.floor(median * 100) / 100,
      high_price: Math.floor(avg * 100) / 100,
      currency: currency,
      confidence: confidence,
      notes: this.getPriceNotes(spreadPercent, prices.count)
    };
  }

  /**
   * Get notes about price data quality
   */
  getPriceNotes(spreadPercent, count) {
    const notes = [];

    if (count < 3) {
      notes.push('Limited market data - price may be unreliable');
    } else if (count < 10) {
      notes.push('Moderate market data available');
    } else {
      notes.push('Good market data available');
    }

    if (spreadPercent > 50) {
      notes.push('High price variance - market is volatile');
    } else if (spreadPercent > 25) {
      notes.push('Moderate price variance');
    } else {
      notes.push('Stable pricing');
    }

    return notes;
  }

  /**
   * Add to price history
   */
  addToPriceHistory(item, priceData) {
    const key = `${item.name}_${item.baseName}`;
    const history = this.priceHistory.get(key) || [];

    history.push({
      timestamp: Date.now(),
      item: item,
      priceData: priceData
    });

    // Keep last 10 checks
    if (history.length > 10) {
      history.shift();
    }

    this.priceHistory.set(key, history);
  }

  /**
   * Get price history for item
   */
  getPriceHistory(itemName, baseName) {
    const key = `${itemName}_${baseName}`;
    return this.priceHistory.get(key) || [];
  }

  /**
   * Clear price history
   */
  clearHistory() {
    this.priceHistory.clear();
  }

  /**
   * Generate trade whisper message
   */
  generateWhisper(listing) {
    if (!listing || !listing.listing) {
      return null;
    }

    const { account, price, item } = listing.listing;
    
    // Standard POE trade whisper format
    return `@${account.name} Hi, I would like to buy your ${item.name} listed for ${price.amount} ${price.currency} in ${this.tradeService.league}`;
  }

  /**
   * Get similar items from market
   */
  async getSimilarItems(parsedItem, limit = 20) {
    try {
      const searchResult = await this.tradeService.searchItems(parsedItem);

      if (!searchResult.result || searchResult.result.length === 0) {
        return [];
      }

      const listingIds = searchResult.result.slice(0, limit);
      const listings = await this.tradeService.fetchListings(
        searchResult.id,
        listingIds
      );

      return listings;

    } catch (error) {
      console.error('Get similar items error:', error);
      return [];
    }
  }

  /**
   * Estimate item value (for unpriced items)
   */
  estimateValue(parsedItem) {
    let value = 0;
    let confidence = 0;

    // Basic estimation based on item properties
    if (parsedItem.rarity === 'Unique') {
      value = 1; // Default chaos orb
      confidence = 0.3;
    } else if (parsedItem.rarity === 'Rare') {
      // Estimate based on mods and stats
      value = this.estimateRareValue(parsedItem);
      confidence = 0.5;
    }

    return {
      estimated: true,
      value: value,
      currency: 'chaos',
      confidence: confidence,
      message: 'Estimated value - check market for accurate pricing'
    };
  }

  /**
   * Estimate rare item value based on stats
   */
  estimateRareValue(parsedItem) {
    let value = 0;

    // High links add value
    if (parsedItem.links >= 6) {
      value += 10;
    } else if (parsedItem.links === 5) {
      value += 2;
    }

    // Good life roll
    if (parsedItem.stats.life > 80) {
      value += 5;
    } else if (parsedItem.stats.life > 60) {
      value += 2;
    }

    // Resistances
    const totalRes = (parsedItem.stats.fireRes || 0) +
                     (parsedItem.stats.coldRes || 0) +
                     (parsedItem.stats.lightningRes || 0);
    
    if (totalRes > 100) {
      value += 5;
    } else if (totalRes > 60) {
      value += 2;
    }

    return Math.max(value, 1); // Minimum 1 chaos
  }
}

module.exports = PriceChecker;
