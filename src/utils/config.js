/**
 * Configuration Manager for POE Overlay
 * Handles user settings and preferences
 */

const Store = require('electron-store');

class Config {
  constructor() {
    this.store = new Store({
      name: 'poe-overlay-config',
      defaults: {
        league: 'Standard',
        hotkey: 'Ctrl+D',
        overlay: {
          alwaysOnTop: true,
          opacity: 0.95,
          width: 400,
          height: 600,
          x: null,
          y: null
        },
        api: {
          rateLimitDelay: 1000,
          cacheTimeout: 300000
        },
        priceCheck: {
          autoShow: true,
          confidenceThreshold: 0.7,
          minListings: 3
        },
        display: {
          showItemDetails: true,
          showSimilarListings: true,
          maxListingsToShow: 5,
          theme: 'dark'
        }
      }
    });
  }

  /**
   * Get configuration value
   */
  get(key) {
    return this.store.get(key);
  }

  /**
   * Set configuration value
   */
  set(key, value) {
    this.store.set(key, value);
  }

  /**
   * Get current league
   */
  getLeague() {
    return this.get('league');
  }

  /**
   * Set current league
   */
  setLeague(league) {
    this.set('league', league);
  }

  /**
   * Get overlay settings
   */
  getOverlaySettings() {
    return this.get('overlay');
  }

  /**
   * Update overlay settings
   */
  updateOverlaySettings(settings) {
    const current = this.getOverlaySettings();
    this.set('overlay', { ...current, ...settings });
  }

  /**
   * Get API settings
   */
  getApiSettings() {
    return this.get('api');
  }

  /**
   * Get price check settings
   */
  getPriceCheckSettings() {
    return this.get('priceCheck');
  }

  /**
   * Reset to defaults
   */
  reset() {
    this.store.clear();
  }

  /**
   * Export settings
   */
  export() {
    return this.store.store;
  }

  /**
   * Import settings
   */
  import(settings) {
    Object.keys(settings).forEach(key => {
      this.set(key, settings[key]);
    });
  }
}

module.exports = Config;
