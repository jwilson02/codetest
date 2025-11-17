/**
 * POE Item Parser
 * Parses Path of Exile item data from clipboard format
 * Handles all item types: uniques, rares, currency, maps, gems, etc.
 */

class ItemParser {
  constructor() {
    this.rarityMap = {
      'Normal': 0,
      'Magic': 1,
      'Rare': 2,
      'Unique': 3,
      'Gem': 4,
      'Currency': 5,
      'Divination Card': 6
    };

    this.influenceTypes = [
      'Shaper Item',
      'Elder Item',
      'Crusader Item',
      'Redeemer Item',
      'Hunter Item',
      'Warlord Item',
      'Synthesised Item',
      'Fractured Item'
    ];
  }

  /**
   * Main parsing function - parse POE item from clipboard text
   */
  parseItem(itemText) {
    if (!itemText || typeof itemText !== 'string') {
      return null;
    }

    const lines = itemText.split('\n').map(line => line.trim()).filter(line => line);
    
    if (lines.length < 2) {
      return null;
    }

    const item = {
      rawText: itemText,
      rarity: null,
      rarityTier: null,
      name: null,
      baseName: null,
      itemClass: null,
      identified: true,
      corrupted: false,
      mirrored: false,
      influences: [],
      itemLevel: null,
      quality: null,
      sockets: null,
      links: null,
      requirements: {},
      properties: [],
      mods: {
        implicit: [],
        explicit: [],
        crafted: [],
        fractured: [],
        enchant: []
      },
      stats: {},
      note: null
    };

    let currentSection = 'header';
    let lineIndex = 0;

    // Parse rarity (first line)
    if (lines[lineIndex].startsWith('Rarity:')) {
      item.rarity = lines[lineIndex].replace('Rarity:', '').trim();
      item.rarityTier = this.rarityMap[item.rarity] || 0;
      lineIndex++;
    }

    // Parse name and base type
    if (item.rarity === 'Rare' || item.rarity === 'Magic') {
      // For rare/magic: first line is name, second is base
      item.name = lines[lineIndex];
      lineIndex++;
      if (lines[lineIndex] && lines[lineIndex] !== '--------') {
        item.baseName = lines[lineIndex];
        lineIndex++;
      }
    } else {
      // For normal/unique: just the name
      item.name = lines[lineIndex];
      item.baseName = item.name;
      lineIndex++;
    }

    // Parse remaining sections
    while (lineIndex < lines.length) {
      const line = lines[lineIndex];

      // Section separator
      if (line === '--------') {
        currentSection = this.getNextSection(currentSection);
        lineIndex++;
        continue;
      }

      // Parse based on current section
      if (line.startsWith('Item Class:')) {
        item.itemClass = line.replace('Item Class:', '').trim();
      } else if (line.startsWith('Unidentified')) {
        item.identified = false;
      } else if (line.startsWith('Corrupted')) {
        item.corrupted = true;
      } else if (line.startsWith('Mirrored')) {
        item.mirrored = true;
      } else if (this.isInfluence(line)) {
        item.influences.push(line);
      } else if (line.startsWith('Item Level:')) {
        item.itemLevel = parseInt(line.replace('Item Level:', '').trim());
      } else if (line.startsWith('Quality:')) {
        item.quality = this.parseNumericValue(line);
      } else if (line.startsWith('Sockets:')) {
        this.parseSockets(line, item);
      } else if (line.startsWith('Requirements:') || line.startsWith('Level:') || 
                 line.startsWith('Str:') || line.startsWith('Dex:') || line.startsWith('Int:')) {
        this.parseRequirement(line, item);
      } else if (line.includes('Note:')) {
        item.note = line.replace('Note:', '').trim();
      } else if (currentSection === 'properties') {
        this.parseProperty(line, item);
      } else if (currentSection === 'mods') {
        this.parseMod(line, item);
      }

      lineIndex++;
    }

    // Post-process: calculate total links
    this.calculateLinks(item);

    // Extract numeric stats from mods
    this.extractStats(item);

    return item;
  }

  /**
   * Determine next section based on current
   */
  getNextSection(current) {
    const sections = ['header', 'properties', 'requirements', 'mods', 'flavor'];
    const currentIndex = sections.indexOf(current);
    return currentIndex < sections.length - 1 ? sections[currentIndex + 1] : 'mods';
  }

  /**
   * Check if line indicates influence
   */
  isInfluence(line) {
    return this.influenceTypes.some(inf => line === inf);
  }

  /**
   * Parse sockets and links
   */
  parseSockets(line, item) {
    const socketStr = line.replace('Sockets:', '').trim();
    
    // Count sockets by color
    const sockets = {
      R: 0, // Red (Strength)
      G: 0, // Green (Dexterity)
      B: 0, // Blue (Intelligence)
      W: 0, // White (Any)
      total: 0
    };

    // Count links
    let maxLinks = 0;
    const groups = socketStr.split(' ');
    
    for (const group of groups) {
      const groupSockets = group.split('-');
      maxLinks = Math.max(maxLinks, groupSockets.length);
      
      for (const socket of groupSockets) {
        if (sockets[socket] !== undefined) {
          sockets[socket]++;
          sockets.total++;
        }
      }
    }

    item.sockets = sockets;
    item.links = maxLinks;
    item.socketString = socketStr;
  }

  /**
   * Parse requirement line
   */
  parseRequirement(line, item) {
    if (line.startsWith('Level:')) {
      item.requirements.level = parseInt(line.replace('Level:', '').trim());
    } else if (line.startsWith('Str:')) {
      item.requirements.str = parseInt(line.replace('Str:', '').trim());
    } else if (line.startsWith('Dex:')) {
      item.requirements.dex = parseInt(line.replace('Dex:', '').trim());
    } else if (line.startsWith('Int:')) {
      item.requirements.int = parseInt(line.replace('Int:', '').trim());
    }
  }

  /**
   * Parse property line
   */
  parseProperty(line, item) {
    const property = {
      name: '',
      values: []
    };

    // Split on colon
    const parts = line.split(':');
    if (parts.length >= 2) {
      property.name = parts[0].trim();
      property.values = parts[1].trim();
    } else {
      property.name = line;
    }

    item.properties.push(property);
  }

  /**
   * Parse mod (affix) line
   */
  parseMod(line, item) {
    // Determine mod type
    let modType = 'explicit';

    if (line.includes('(implicit)')) {
      modType = 'implicit';
      line = line.replace('(implicit)', '').trim();
    } else if (line.includes('(crafted)')) {
      modType = 'crafted';
      line = line.replace('(crafted)', '').trim();
    } else if (line.includes('(fractured)')) {
      modType = 'fractured';
      line = line.replace('(fractured)', '').trim();
    } else if (line.includes('(enchant)')) {
      modType = 'enchant';
      line = line.replace('(enchant)', '').trim();
    }

    // Extract numeric values and ranges
    const mod = {
      text: line,
      values: this.extractNumericValues(line),
      type: modType
    };

    item.mods[modType].push(mod);
  }

  /**
   * Extract numeric values from text
   */
  extractNumericValues(text) {
    const values = [];
    const regex = /([+-]?\d+(?:\.\d+)?)/g;
    let match;

    while ((match = regex.exec(text)) !== null) {
      values.push(parseFloat(match[1]));
    }

    return values;
  }

  /**
   * Parse single numeric value (for Quality, etc.)
   */
  parseNumericValue(line) {
    const match = line.match(/([+-]?\d+(?:\.\d+)?)/);
    return match ? parseFloat(match[1]) : null;
  }

  /**
   * Calculate maximum link group
   */
  calculateLinks(item) {
    if (!item.socketString) {
      return;
    }

    const groups = item.socketString.split(' ');
    let maxLinks = 0;

    for (const group of groups) {
      const linkCount = group.split('-').length;
      maxLinks = Math.max(maxLinks, linkCount);
    }

    item.links = maxLinks;
  }

  /**
   * Extract stats from mods for easier searching
   */
  extractStats(item) {
    const stats = {};

    // Combine all mods
    const allMods = [
      ...item.mods.implicit,
      ...item.mods.explicit,
      ...item.mods.crafted,
      ...item.mods.fractured
    ];

    // Extract common stats
    for (const mod of allMods) {
      // Life
      if (mod.text.includes('maximum Life')) {
        stats.life = (stats.life || 0) + (mod.values[0] || 0);
      }
      // Energy Shield
      if (mod.text.includes('Energy Shield')) {
        stats.energyShield = (stats.energyShield || 0) + (mod.values[0] || 0);
      }
      // Resistances
      if (mod.text.includes('Fire Resistance')) {
        stats.fireRes = (stats.fireRes || 0) + (mod.values[0] || 0);
      }
      if (mod.text.includes('Cold Resistance')) {
        stats.coldRes = (stats.coldRes || 0) + (mod.values[0] || 0);
      }
      if (mod.text.includes('Lightning Resistance')) {
        stats.lightningRes = (stats.lightningRes || 0) + (mod.values[0] || 0);
      }
      if (mod.text.includes('Chaos Resistance')) {
        stats.chaosRes = (stats.chaosRes || 0) + (mod.values[0] || 0);
      }
    }

    item.stats = stats;
  }

  /**
   * Check if item is currency
   */
  isCurrency(item) {
    return item.rarity === 'Currency' || item.itemClass === 'Currency';
  }

  /**
   * Check if item is gem
   */
  isGem(item) {
    return item.itemClass && (
      item.itemClass.includes('Gem') ||
      item.itemClass === 'Active Skill Gems' ||
      item.itemClass === 'Support Skill Gems'
    );
  }

  /**
   * Check if item is map
   */
  isMap(item) {
    return item.itemClass && item.itemClass.includes('Map');
  }

  /**
   * Check if item is divination card
   */
  isDivinationCard(item) {
    return item.itemClass === 'Divination Card';
  }

  /**
   * Get item category for trade search
   */
  getItemCategory(item) {
    if (this.isCurrency(item)) return 'currency';
    if (this.isGem(item)) return 'gem';
    if (this.isMap(item)) return 'map';
    if (this.isDivinationCard(item)) return 'card';
    return 'equipment';
  }
}

module.exports = ItemParser;
