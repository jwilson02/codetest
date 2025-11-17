/**
 * Trade Whisper Service
 * Handles parsing and managing POE trade whispers
 * Supports standard and asynchronous trade formats (3.27+)
 */

class TradeWhisperService {
  constructor() {
    this.trades = [];
    this.maxTradeHistory = 50;
    this.templates = this.getDefaultTemplates();
    this.statistics = {
      totalTrades: 0,
      completedTrades: 0,
      cancelledTrades: 0,
      sessionStart: Date.now(),
      tradesPerHour: 0,
    };
  }

  /**
   * Parse standard trade whisper
   * Format: "@From PlayerName: Hi, I would like to buy your [Item] listed for [Price] in [League]"
   */
  parseStandardWhisper(whisperText) {
    // Standard whisper pattern
    const standardPattern = /@From\s+(.+?):\s+Hi,\s+I(?:'d|\s+would)\s+like\s+to\s+buy\s+your\s+(.+?)\s+listed\s+for\s+(.+?)\s+in\s+(.+?)(?:\s+\(stash tab "(.+?)"; position: left (\d+), top (\d+)\))?/i;

    const match = whisperText.match(standardPattern);

    if (match) {
      return {
        type: 'standard',
        playerName: match[1].trim(),
        item: match[2].trim(),
        price: match[3].trim(),
        league: match[4].trim(),
        stashTab: match[5] || null,
        stashLeft: match[6] ? parseInt(match[6]) : null,
        stashTop: match[7] ? parseInt(match[7]) : null,
        rawText: whisperText,
        timestamp: Date.now(),
      };
    }

    return null;
  }

  /**
   * Parse asynchronous trade whisper (new in 3.27)
   * Format: "@From PlayerName: Hi, I would like to purchase your [Item] for [Price] in [League] (Asynchronous trade)"
   */
  parseAsyncWhisper(whisperText) {
    const asyncPattern = /@From\s+(.+?):\s+Hi,\s+I(?:'d|\s+would)\s+like\s+to\s+(?:purchase|buy)\s+your\s+(.+?)\s+(?:for|listed\s+for)\s+(.+?)\s+in\s+(.+?)(?:\s+\((?:A|a)synchronous\s+(?:T|t)rade\))/i;

    const match = whisperText.match(asyncPattern);

    if (match) {
      return {
        type: 'asynchronous',
        playerName: match[1].trim(),
        item: match[2].trim(),
        price: match[3].trim(),
        league: match[4].trim(),
        rawText: whisperText,
        timestamp: Date.now(),
      };
    }

    return null;
  }

  /**
   * Parse bulk trade whisper
   * Format: "@From PlayerName: Hi, I'd like to buy your 10 Chaos Orb for my 1 Divine Orb in League"
   */
  parseBulkWhisper(whisperText) {
    const bulkPattern = /@From\s+(.+?):\s+Hi,\s+I(?:'d|\s+would)\s+like\s+to\s+buy\s+your\s+(.+?)\s+for\s+my\s+(.+?)\s+in\s+(.+)/i;

    const match = whisperText.match(bulkPattern);

    if (match) {
      return {
        type: 'bulk',
        playerName: match[1].trim(),
        wantItem: match[2].trim(),
        offerItem: match[3].trim(),
        league: match[4].trim(),
        rawText: whisperText,
        timestamp: Date.now(),
      };
    }

    return null;
  }

  /**
   * Main whisper parsing - tries all formats
   */
  parseWhisper(whisperText) {
    if (!whisperText || typeof whisperText !== 'string') {
      return null;
    }

    // Try standard format first
    let parsed = this.parseStandardWhisper(whisperText);
    if (parsed) return parsed;

    // Try asynchronous format
    parsed = this.parseAsyncWhisper(whisperText);
    if (parsed) return parsed;

    // Try bulk format
    parsed = this.parseBulkWhisper(whisperText);
    if (parsed) return parsed;

    return null;
  }

  /**
   * Add a new trade from whisper
   */
  addTrade(whisperText) {
    const parsed = this.parseWhisper(whisperText);

    if (!parsed) {
      return null;
    }

    const trade = {
      id: this.generateTradeId(),
      ...parsed,
      status: 'pending', // pending, completed, cancelled
      addedAt: Date.now(),
      completedAt: null,
    };

    this.trades.unshift(trade);

    // Keep only last N trades
    if (this.trades.length > this.maxTradeHistory) {
      this.trades = this.trades.slice(0, this.maxTradeHistory);
    }

    this.updateStatistics();

    return trade;
  }

  /**
   * Generate unique trade ID
   */
  generateTradeId() {
    return `trade_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }

  /**
   * Get all active trades (pending)
   */
  getActiveTrades() {
    return this.trades.filter(trade => trade.status === 'pending');
  }

  /**
   * Get trade history (completed/cancelled)
   */
  getTradeHistory() {
    return this.trades.filter(trade => trade.status !== 'pending');
  }

  /**
   * Get all trades
   */
  getAllTrades() {
    return this.trades;
  }

  /**
   * Update trade status
   */
  updateTradeStatus(tradeId, status) {
    const trade = this.trades.find(t => t.id === tradeId);

    if (trade) {
      trade.status = status;

      if (status === 'completed' || status === 'cancelled') {
        trade.completedAt = Date.now();
      }

      this.updateStatistics();
      return true;
    }

    return false;
  }

  /**
   * Complete a trade
   */
  completeTrade(tradeId) {
    return this.updateTradeStatus(tradeId, 'completed');
  }

  /**
   * Cancel a trade
   */
  cancelTrade(tradeId) {
    return this.updateTradeStatus(tradeId, 'cancelled');
  }

  /**
   * Remove a trade from history
   */
  removeTrade(tradeId) {
    const index = this.trades.findIndex(t => t.id === tradeId);

    if (index !== -1) {
      this.trades.splice(index, 1);
      this.updateStatistics();
      return true;
    }

    return false;
  }

  /**
   * Clear all trade history
   */
  clearHistory() {
    this.trades = this.trades.filter(trade => trade.status === 'pending');
    this.updateStatistics();
  }

  /**
   * Get default response templates
   */
  getDefaultTemplates() {
    return {
      oneSec: {
        id: 'one-sec',
        name: 'One Second',
        message: '1 sec',
        shortcut: '1',
      },
      oneMoment: {
        id: 'one-moment',
        name: 'One Moment',
        message: 'One moment please',
        shortcut: '2',
      },
      sold: {
        id: 'sold',
        name: 'Already Sold',
        message: 'Sold',
        shortcut: '3',
      },
      sorry: {
        id: 'sorry',
        name: 'Sorry Sold',
        message: 'Sorry, already sold',
        shortcut: '4',
      },
      thanks: {
        id: 'thanks',
        name: 'Thanks',
        message: 'ty',
        shortcut: '5',
      },
      thankYou: {
        id: 'thank-you',
        name: 'Thank You',
        message: 'Thanks for trade!',
        shortcut: '6',
      },
      invite: {
        id: 'invite',
        name: 'Invite Sent',
        message: 'inv',
        shortcut: '7',
      },
      inviteSent: {
        id: 'invite-sent',
        name: 'Invite Sent Full',
        message: 'Invite sent',
        shortcut: '8',
      },
      busy: {
        id: 'busy',
        name: 'Busy',
        message: 'Busy atm, 1 min',
        shortcut: '9',
      },
      afk: {
        id: 'afk',
        name: 'AFK',
        message: 'AFK, back soon',
        shortcut: '0',
      },
    };
  }

  /**
   * Get all templates
   */
  getTemplates() {
    return Object.values(this.templates);
  }

  /**
   * Add custom template
   */
  addTemplate(name, message, shortcut = null) {
    const id = `custom-${Date.now()}`;

    this.templates[id] = {
      id,
      name,
      message,
      shortcut,
      custom: true,
    };

    return this.templates[id];
  }

  /**
   * Update template
   */
  updateTemplate(templateId, updates) {
    if (this.templates[templateId]) {
      this.templates[templateId] = {
        ...this.templates[templateId],
        ...updates,
      };
      return true;
    }
    return false;
  }

  /**
   * Delete custom template
   */
  deleteTemplate(templateId) {
    if (this.templates[templateId] && this.templates[templateId].custom) {
      delete this.templates[templateId];
      return true;
    }
    return false;
  }

  /**
   * Generate whisper response
   */
  generateResponse(tradeId, templateId) {
    const trade = this.trades.find(t => t.id === tradeId);
    const template = this.templates[templateId];

    if (!trade || !template) {
      return null;
    }

    return `@${trade.playerName} ${template.message}`;
  }

  /**
   * Update statistics
   */
  updateStatistics() {
    this.statistics.totalTrades = this.trades.length;
    this.statistics.completedTrades = this.trades.filter(t => t.status === 'completed').length;
    this.statistics.cancelledTrades = this.trades.filter(t => t.status === 'cancelled').length;

    // Calculate trades per hour
    const sessionDuration = (Date.now() - this.statistics.sessionStart) / 1000 / 60 / 60; // hours
    this.statistics.tradesPerHour = sessionDuration > 0
      ? Math.round(this.statistics.completedTrades / sessionDuration * 10) / 10
      : 0;
  }

  /**
   * Get statistics
   */
  getStatistics() {
    this.updateStatistics();
    return { ...this.statistics };
  }

  /**
   * Reset statistics
   */
  resetStatistics() {
    this.statistics = {
      totalTrades: 0,
      completedTrades: 0,
      cancelledTrades: 0,
      sessionStart: Date.now(),
      tradesPerHour: 0,
    };
  }

  /**
   * Export trades to JSON
   */
  exportTrades() {
    return JSON.stringify({
      trades: this.trades,
      templates: this.templates,
      statistics: this.statistics,
      exportedAt: new Date().toISOString(),
    }, null, 2);
  }

  /**
   * Import trades from JSON
   */
  importTrades(jsonData) {
    try {
      const data = JSON.parse(jsonData);

      if (data.trades) {
        this.trades = data.trades;
      }
      if (data.templates) {
        this.templates = { ...this.getDefaultTemplates(), ...data.templates };
      }

      this.updateStatistics();
      return true;
    } catch (error) {
      console.error('Import trades error:', error);
      return false;
    }
  }
}

module.exports = TradeWhisperService;
