/**
 * Currency Service - Fetches and manages POE currency exchange rates
 * Data source: POE.ninja API
 */

const axios = require('axios');

const POE_NINJA_BASE_URL = 'https://poe.ninja/api/data';
const CACHE_DURATION = 5 * 60 * 1000; // 5 minutes

class CurrencyService {
  constructor() {
    this.cache = {
      currency: null,
      fragments: null,
      lastUpdate: null
    };
    this.updateInterval = null;
  }

  /**
   * Start auto-refresh of currency data
   */
  startAutoRefresh() {
    // Initial fetch
    this.fetchCurrencyRates();

    // Set up interval for auto-refresh every 5 minutes
    this.updateInterval = setInterval(() => {
      this.fetchCurrencyRates();
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
   * Fetch currency exchange rates from POE.ninja
   * @param {string} league - The league to fetch data for (default: current temp league)
   * @returns {Promise<Object>}
   */
  async fetchCurrencyRates(league = null) {
    try {
      // If no league specified, try to determine current league
      if (!league) {
        league = await this.getCurrentLeague();
      }

      // Fetch currency overview
      const currencyResponse = await axios.get(`${POE_NINJA_BASE_URL}/currencyoverview`, {
        params: {
          league: league,
          type: 'Currency'
        }
      });

      // Fetch fragment data (includes scarabs, etc.)
      const fragmentResponse = await axios.get(`${POE_NINJA_BASE_URL}/currencyoverview`, {
        params: {
          league: league,
          type: 'Fragment'
        }
      });

      // Update cache
      this.cache.currency = currencyResponse.data;
      this.cache.fragments = fragmentResponse.data;
      this.cache.lastUpdate = Date.now();

      console.log(`Currency rates updated for league: ${league}`);

      return {
        currency: this.cache.currency,
        fragments: this.cache.fragments,
        league: league,
        lastUpdate: this.cache.lastUpdate
      };
    } catch (error) {
      console.error('Error fetching currency rates:', error);

      // Return cached data if available
      if (this.cache.currency) {
        console.log('Returning cached currency data');
        return {
          currency: this.cache.currency,
          fragments: this.cache.fragments,
          league: league,
          lastUpdate: this.cache.lastUpdate,
          cached: true
        };
      }

      throw new Error(`Failed to fetch currency rates: ${error.message}`);
    }
  }

  /**
   * Get current league name from POE API
   * @returns {Promise<string>}
   */
  async getCurrentLeague() {
    try {
      // Try to fetch from POE.ninja main page or use hardcoded current league
      // For now, using a common league name - this should be made configurable
      return 'Affliction'; // Update this based on current league
    } catch (error) {
      console.error('Error getting current league:', error);
      return 'Standard'; // Fallback to Standard
    }
  }

  /**
   * Get exchange rate for a specific currency
   * @param {string} currencyName - Name of the currency (e.g., 'Divine Orb', 'Exalted Orb')
   * @returns {Object|null}
   */
  getCurrencyRate(currencyName) {
    if (!this.cache.currency) {
      console.warn('Currency data not loaded');
      return null;
    }

    const lines = this.cache.currency.lines || [];
    const currency = lines.find(c =>
      c.currencyTypeName === currencyName ||
      c.currencyTypeName.toLowerCase().includes(currencyName.toLowerCase())
    );

    return currency ? {
      name: currency.currencyTypeName,
      chaosEquivalent: currency.chaosEquivalent,
      change: currency.receiveSparkLine?.totalChange || 0,
      icon: currency.icon
    } : null;
  }

  /**
   * Convert between currencies
   * @param {number} amount - Amount to convert
   * @param {string} fromCurrency - Source currency name
   * @param {string} toCurrency - Target currency name
   * @returns {number|null}
   */
  convertCurrency(amount, fromCurrency, toCurrency) {
    const from = this.getCurrencyRate(fromCurrency);
    const to = this.getCurrencyRate(toCurrency);

    if (!from || !to) {
      console.warn('Currency not found for conversion');
      return null;
    }

    // Convert from -> chaos -> to
    const chaosValue = amount * from.chaosEquivalent;
    const targetValue = chaosValue / to.chaosEquivalent;

    return Math.round(targetValue * 100) / 100; // Round to 2 decimal places
  }

  /**
   * Get popular currency exchange rates
   * @returns {Array}
   */
  getPopularRates() {
    const popularCurrencies = [
      'Divine Orb',
      'Exalted Orb',
      'Orb of Alchemy',
      'Chaos Orb',
      'Orb of Alteration',
      'Chromatic Orb',
      'Jeweller\'s Orb',
      'Orb of Fusing',
      'Vaal Orb',
      'Gemcutter\'s Prism',
      'Cartographer\'s Chisel',
      'Orb of Regret'
    ];

    return popularCurrencies
      .map(name => this.getCurrencyRate(name))
      .filter(rate => rate !== null);
  }

  /**
   * Get cache status
   * @returns {Object}
   */
  getCacheStatus() {
    return {
      loaded: this.cache.currency !== null,
      lastUpdate: this.cache.lastUpdate,
      age: this.cache.lastUpdate ? Date.now() - this.cache.lastUpdate : null,
      stale: this.cache.lastUpdate ? (Date.now() - this.cache.lastUpdate) > CACHE_DURATION : true
    };
  }

  /**
   * Get bulk exchange info for scarabs, fragments, etc.
   * @param {string} itemName - Name of the fragment/scarab
   * @returns {Object|null}
   */
  getFragmentRate(itemName) {
    if (!this.cache.fragments) {
      console.warn('Fragment data not loaded');
      return null;
    }

    const lines = this.cache.fragments.lines || [];
    const fragment = lines.find(f =>
      f.currencyTypeName === itemName ||
      f.currencyTypeName.toLowerCase().includes(itemName.toLowerCase())
    );

    return fragment ? {
      name: fragment.currencyTypeName,
      chaosValue: fragment.chaosEquivalent,
      change: fragment.receiveSparkLine?.totalChange || 0,
      icon: fragment.icon
    } : null;
  }
}

// Export singleton instance
const currencyService = new CurrencyService();
module.exports = currencyService;
module.exports.CurrencyService = CurrencyService;
