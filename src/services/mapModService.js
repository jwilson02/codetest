/**
 * Map Mod Service - Analyzes and provides information about map modifiers
 * Helps identify dangerous map mods and provides explanations
 */

class MapModService {
  constructor() {
    // Database of dangerous map mods with severity and explanations
    this.dangerousMods = [
      {
        regex: /reflect.*elemental.*damage/i,
        severity: 'deadly',
        category: 'reflect',
        name: 'Elemental Reflect',
        description: 'Reflects elemental damage back to you',
        warning: 'DEADLY for elemental damage builds! Will kill you instantly.',
        solution: 'Skip this map or use Sextant of Reflection/Pantheon'
      },
      {
        regex: /reflect.*physical.*damage/i,
        severity: 'deadly',
        category: 'reflect',
        name: 'Physical Reflect',
        description: 'Reflects physical damage back to you',
        warning: 'DEADLY for physical damage builds! Will kill you instantly.',
        solution: 'Skip this map or use Sextant of Reflection'
      },
      {
        regex: /players cannot regenerate.*mana/i,
        severity: 'high',
        category: 'regen',
        name: 'No Mana Regen',
        description: 'Prevents mana regeneration',
        warning: 'Very difficult for mana-based builds',
        solution: 'Use mana flasks or mana leech'
      },
      {
        regex: /players cannot regenerate.*life/i,
        severity: 'high',
        category: 'regen',
        name: 'No Life Regen',
        description: 'Prevents life regeneration',
        warning: 'Dangerous for RF or regen-based builds',
        solution: 'Skip if using Righteous Fire or heavy regen'
      },
      {
        regex: /monsters.*temporal.*chains/i,
        severity: 'medium',
        category: 'curse',
        name: 'Temporal Chains',
        description: 'Slows your actions and debuff expiry',
        warning: 'Significantly slows down clear speed',
        solution: 'Use curse immunity flask or avoid'
      },
      {
        regex: /monsters.*enfeeble/i,
        severity: 'medium',
        category: 'curse',
        name: 'Enfeeble',
        description: 'Reduces your damage and critical strike chance',
        warning: 'Lower damage output',
        solution: 'Use curse immunity flask'
      },
      {
        regex: /players are cursed with vulnerability/i,
        severity: 'high',
        category: 'curse',
        name: 'Vulnerability',
        description: 'Increases physical damage taken',
        warning: 'Very dangerous - increases damage taken',
        solution: 'Use curse immunity flask or have high physical mitigation'
      },
      {
        regex: /players are cursed with elemental weakness/i,
        severity: 'high',
        category: 'curse',
        name: 'Elemental Weakness',
        description: 'Reduces elemental resistances',
        warning: 'Can drop resistances below cap',
        solution: 'Use curse immunity flask or overcap resistances'
      },
      {
        regex: /-(\d+)% maximum.*resistance/i,
        severity: 'high',
        category: 'resistance',
        name: 'Reduced Max Resistance',
        description: 'Lowers your maximum resistances',
        warning: 'Very dangerous - harder to cap resistances',
        solution: 'Boost your resistances or avoid'
      },
      {
        regex: /monsters cannot be leeched from/i,
        severity: 'high',
        category: 'leech',
        name: 'Cannot Leech',
        description: 'Prevents life and mana leech',
        warning: 'Deadly for leech-dependent builds',
        solution: 'Use alternative recovery methods or avoid'
      },
      {
        regex: /players have (\d+)% less.*recovery/i,
        severity: 'medium',
        category: 'recovery',
        name: 'Reduced Recovery',
        description: 'Reduces life/ES/mana recovery rate',
        warning: 'Slower recovery from flasks and regen',
        solution: 'Play more carefully'
      },
      {
        regex: /monsters have.*extra.*damage as/i,
        severity: 'medium',
        category: 'damage',
        name: 'Extra Damage',
        description: 'Monsters deal additional damage of another type',
        warning: 'Increased incoming damage',
        solution: 'Ensure all resistances are capped'
      },
      {
        regex: /monsters.*crit/i,
        severity: 'high',
        category: 'damage',
        name: 'Monster Crit',
        description: 'Monsters have increased critical strike chance/multiplier',
        warning: 'Can result in one-shots',
        solution: 'Avoid or have very high defenses'
      },
      {
        regex: /area contains.*additional.*totems/i,
        severity: 'low',
        category: 'mechanics',
        name: 'Tormented Spirits',
        description: 'Area has additional totems that curse players',
        warning: 'Minor annoyance with additional cursing totems',
        solution: 'Destroy totems quickly'
      },
      {
        regex: /players have.*reduced.*block/i,
        severity: 'medium',
        category: 'defense',
        name: 'Reduced Block',
        description: 'Reduces block chance',
        warning: 'Dangerous for block-based builds',
        solution: 'Play carefully or avoid if block-dependent'
      },
      {
        regex: /players have.*reduced.*dodge/i,
        severity: 'medium',
        category: 'defense',
        name: 'Reduced Evasion',
        description: 'Reduces evasion/dodge',
        warning: 'More hits will land',
        solution: 'Be more defensive'
      }
    ];

    // Boss mechanics database
    this.bossMechanics = {
      // Conquerors
      'Baran': {
        name: 'Baran, The Crusader',
        mechanics: [
          'Rune squares on ground - avoid standing in them',
          'Mana siphon - drains mana rapidly',
          'Lightning spells with high damage'
        ],
        tips: 'Keep moving, watch for rune patterns on ground'
      },
      'Veritania': {
        name: 'Veritania, The Redeemer',
        mechanics: [
          'Tornado that follows player',
          'Ice prisons that freeze on contact',
          'Cyclone attack with cold damage'
        ],
        tips: 'Avoid tornado, don\'t get trapped in ice prison'
      },
      'Al-Hezmin': {
        name: 'Al-Hezmin, The Hunter',
        mechanics: [
          'Poison clouds - very dangerous',
          'Cobra pets that poison',
          'Snake arena barriers'
        ],
        tips: 'Have poison immunity or high chaos resistance'
      },
      'Drox': {
        name: 'Drox, The Warlord',
        mechanics: [
          'Banner phases - spawns adds',
          'Slam attacks with large AoE',
          'Rallying cry that buffs nearby enemies'
        ],
        tips: 'Kill adds during banner phase, dodge slams'
      },
      // Elderslayers
      'Sirus': {
        name: 'Sirus, Awakener of Worlds',
        mechanics: [
          'Die beam - instant death if hit',
          'Meteor maze - navigate carefully',
          'Clone phase - find the real one',
          'Corridor phase - dangerous storms'
        ],
        tips: 'Practice the fight, learn die beam tells, keep Sirus on screen'
      },
      // Shaper Guardians
      'Minotaur': {
        name: 'Guardian of the Minotaur',
        mechanics: [
          'Burrow slam - massive damage',
          'Rock fall from ceiling',
          'Adds spawn periodically'
        ],
        tips: 'Dodge when he burrows, watch for falling rocks'
      },
      'Hydra': {
        name: 'Guardian of the Hydra',
        mechanics: [
          'Fork projectiles',
          'Adds spawn that also shoot',
          'Arena fills with water - less space over time'
        ],
        tips: 'Kill adds, position carefully for projectiles'
      },
      'Phoenix': {
        name: 'Guardian of the Phoenix',
        mechanics: [
          'Explosion on death',
          'Fire cyclone',
          'Projectile spam'
        ],
        tips: 'Have fire resistance, run away when boss dies'
      },
      'Chimera': {
        name: 'Guardian of the Chimera',
        mechanics: [
          'Smoke phase - becomes invisible',
          'Adds in smoke clouds',
          'Poison/chaos damage'
        ],
        tips: 'Clear adds in smoke phase, have chaos resistance'
      }
    };
  }

  /**
   * Analyze map mods from clipboard text
   * @param {string} text - Clipboard text containing map info
   * @returns {Object}
   */
  analyzeMapMods(text) {
    if (!text || typeof text !== 'string') {
      return { mods: [], dangerous: [], safe: true };
    }

    const lines = text.split('\n').map(line => line.trim()).filter(line => line);
    const dangerous = [];
    const allMods = [];

    lines.forEach(line => {
      // Skip item headers and separators
      if (line.startsWith('---') || line.startsWith('Rarity:') ||
          line.includes('Map Tier:') || line.includes('Item Quantity:')) {
        return;
      }

      // Check if this line is a map mod
      const modMatch = this.dangerousMods.find(mod => mod.regex.test(line));

      if (modMatch) {
        dangerous.push({
          text: line,
          ...modMatch
        });
      }

      // Collect all potential mod lines (lines that don't match headers)
      if (line.length > 10 && !line.startsWith('Item Level:')) {
        allMods.push(line);
      }
    });

    // Determine overall danger level
    const hasDeadlyMods = dangerous.some(mod => mod.severity === 'deadly');
    const hasHighMods = dangerous.some(mod => mod.severity === 'high');

    return {
      mods: allMods,
      dangerous: dangerous,
      safe: dangerous.length === 0,
      runnable: !hasDeadlyMods,
      dangerLevel: hasDeadlyMods ? 'deadly' : hasHighMods ? 'high' : dangerous.length > 0 ? 'medium' : 'safe',
      warnings: dangerous.map(mod => mod.warning),
      solutions: dangerous.map(mod => mod.solution)
    };
  }

  /**
   * Get boss mechanics for a map
   * @param {string} mapName - Name of the map
   * @returns {Object|null}
   */
  getBossMechanics(mapName) {
    // Extract boss name from map name
    for (const [key, value] of Object.entries(this.bossMechanics)) {
      if (mapName.toLowerCase().includes(key.toLowerCase())) {
        return value;
      }
    }

    return null;
  }

  /**
   * Get mod explanation
   * @param {string} modText - The mod text
   * @returns {Object|null}
   */
  getModExplanation(modText) {
    const modMatch = this.dangerousMods.find(mod => mod.regex.test(modText));
    return modMatch || null;
  }

  /**
   * Check if build can run a map based on mods
   * @param {Array} mods - Array of mod texts
   * @param {Object} buildType - Build configuration
   * @returns {Object}
   */
  canRunMap(mods, buildType = {}) {
    const analysis = this.analyzeMapMods(mods.join('\n'));

    // Build-specific checks
    const warnings = [];

    if (buildType.damageType === 'physical') {
      const hasPhysReflect = analysis.dangerous.some(m => m.category === 'reflect' && m.name.includes('Physical'));
      if (hasPhysReflect) {
        warnings.push('CANNOT RUN - Physical Reflect will kill you');
        return { canRun: false, warnings };
      }
    }

    if (buildType.damageType === 'elemental') {
      const hasEleReflect = analysis.dangerous.some(m => m.category === 'reflect' && m.name.includes('Elemental'));
      if (hasEleReflect) {
        warnings.push('CANNOT RUN - Elemental Reflect will kill you');
        return { canRun: false, warnings };
      }
    }

    if (buildType.usesLeech) {
      const hasNoLeech = analysis.dangerous.some(m => m.category === 'leech');
      if (hasNoLeech) {
        warnings.push('WARNING - Cannot leech, use alternative recovery');
      }
    }

    if (buildType.usesRegen) {
      const hasNoRegen = analysis.dangerous.some(m => m.category === 'regen');
      if (hasNoRegen) {
        warnings.push('WARNING - Cannot regen, difficult to sustain');
      }
    }

    return {
      canRun: !analysis.dangerous.some(m => m.severity === 'deadly'),
      warnings: warnings.concat(analysis.warnings),
      suggestions: analysis.solutions
    };
  }

  /**
   * Get all boss mechanics
   * @returns {Object}
   */
  getAllBossMechanics() {
    return this.bossMechanics;
  }
}

// Export singleton instance
const mapModService = new MapModService();
module.exports = mapModService;
module.exports.MapModService = MapModService;
