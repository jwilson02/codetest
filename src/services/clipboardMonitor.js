/**
 * Clipboard Monitor Service
 * Monitors clipboard for Path of Exile item data (Ctrl+C in game)
 */

const { clipboard } = require('electron');

class ClipboardMonitor {
  constructor() {
    this.lastClipboard = '';
    this.isMonitoring = false;
    this.callbacks = [];
    this.checkInterval = null;
    this.pollInterval = 500; // Check every 500ms
  }

  /**
   * Start monitoring clipboard for POE items
   */
  start() {
    if (this.isMonitoring) {
      console.log('Clipboard monitor already running');
      return;
    }

    this.isMonitoring = true;
    this.lastClipboard = clipboard.readText();

    this.checkInterval = setInterval(() => {
      this.checkClipboard();
    }, this.pollInterval);

    console.log('Clipboard monitor started');
  }

  /**
   * Stop monitoring clipboard
   */
  stop() {
    if (this.checkInterval) {
      clearInterval(this.checkInterval);
      this.checkInterval = null;
    }
    this.isMonitoring = false;
    console.log('Clipboard monitor stopped');
  }

  /**
   * Check clipboard for changes
   */
  checkClipboard() {
    const currentClipboard = clipboard.readText();

    // Check if clipboard changed
    if (currentClipboard !== this.lastClipboard) {
      this.lastClipboard = currentClipboard;

      // Check if it's a POE item
      if (this.isPOEItem(currentClipboard)) {
        console.log('POE item detected in clipboard');
        this.notifyCallbacks(currentClipboard);
      }
    }
  }

  /**
   * Detect if clipboard contains POE item data
   * POE items have specific format markers
   */
  isPOEItem(text) {
    if (!text || typeof text !== 'string') {
      return false;
    }

    // POE items typically start with "Rarity:" or "Item Class:"
    const poeMarkers = [
      'Rarity:',
      'Item Class:',
      '--------',
      'Requirements:',
      'Sockets:',
      'Item Level:'
    ];

    // Check for at least 2 POE markers
    let markerCount = 0;
    for (const marker of poeMarkers) {
      if (text.includes(marker)) {
        markerCount++;
      }
    }

    return markerCount >= 2;
  }

  /**
   * Register callback for when POE item is detected
   */
  onItemDetected(callback) {
    if (typeof callback === 'function') {
      this.callbacks.push(callback);
    }
  }

  /**
   * Remove callback
   */
  removeCallback(callback) {
    this.callbacks = this.callbacks.filter(cb => cb !== callback);
  }

  /**
   * Notify all registered callbacks
   */
  notifyCallbacks(itemText) {
    this.callbacks.forEach(callback => {
      try {
        callback(itemText);
      } catch (error) {
        console.error('Error in clipboard callback:', error);
      }
    });
  }

  /**
   * Manually trigger item check (for testing)
   */
  manualCheck(itemText) {
    if (this.isPOEItem(itemText)) {
      this.notifyCallbacks(itemText);
      return true;
    }
    return false;
  }

  /**
   * Get current clipboard text
   */
  getCurrentClipboard() {
    return clipboard.readText();
  }

  /**
   * Set clipboard text (for testing)
   */
  setClipboard(text) {
    clipboard.writeText(text);
  }
}

module.exports = ClipboardMonitor;
