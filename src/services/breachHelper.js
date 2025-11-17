/**
 * Breach Helper Service - POE 3.27 Keepers of the Flame League
 * Manages Breach mechanics, tracking, and Hive features
 */

class BreachHelper {
  constructor() {
    this.breachTypes = {
      physical: {
        name: 'Breach of Tul',
        color: '#4a9eff',
        boss: 'Tul, Creeping Avalanche',
        splinter: 'Splinter of Tul',
        blessingName: "Tul's Breachstone",
        element: 'Physical',
        ailithProtection: 'Physical Damage Reduction',
        hiveLocation: 'Frozen Caverns',
        hiveborn: 'Frostborn Primarch',
        strategies: [
          'Stack cold resistance',
          'Avoid chilled ground',
          'Use movement skills to dodge ice projectiles',
          'Focus on killing rares for splinters'
        ]
      },
      fire: {
        name: 'Breach of Xoph',
        color: '#ff4500',
        boss: 'Xoph, Dark Embers',
        splinter: 'Splinter of Xoph',
        blessingName: "Xoph's Breachstone",
        element: 'Fire',
        ailithProtection: 'Fire Resistance',
        hiveLocation: 'Volcanic Depths',
        hiveborn: 'Emberborn Tyrant',
        strategies: [
          'Cap fire resistance',
          'Watch for burning ground',
          'Clear monster packs quickly',
          'Prioritize rare monsters'
        ]
      },
      cold: {
        name: 'Breach of Tul',
        color: '#00ffff',
        boss: 'Tul, Creeping Avalanche',
        splinter: 'Splinter of Tul',
        blessingName: "Tul's Breachstone",
        element: 'Cold',
        ailithProtection: 'Cold Resistance & Freeze Immunity',
        hiveLocation: 'Glacial Expanse',
        hiveborn: 'Frostbound Sentinel',
        strategies: [
          'Bring freeze immunity or reduced freeze duration',
          'Stack cold resistance',
          'Move constantly to avoid ice patches',
          'Use AoE for dense packs'
        ]
      },
      lightning: {
        name: 'Breach of Esh',
        color: '#ffff00',
        boss: 'Esh, Forked Thought',
        splinter: 'Splinter of Esh',
        blessingName: "Esh's Breachstone",
        element: 'Lightning',
        ailithProtection: 'Lightning Resistance & Shock Immunity',
        hiveLocation: 'Storm Nexus',
        hiveborn: 'Voltaic Archon',
        strategies: [
          'Cap lightning resistance',
          'Get shock immunity if possible',
          'Watch for lightning mirages',
          'Clear in wide circular patterns'
        ]
      },
      chaos: {
        name: 'Breach of Chayula',
        color: '#9933ff',
        boss: 'Chayula, Who Dreamt',
        splinter: 'Splinter of Chayula',
        blessingName: "Chayula's Breachstone",
        element: 'Chaos',
        ailithProtection: 'Chaos Resistance & Cannot be Hindered',
        hiveLocation: 'Void Sanctum',
        hiveborn: 'Voidborn Hegemon',
        strategies: [
          'Stack chaos resistance (75%+)',
          'Use movement skills frequently',
          'Most valuable splinters - prioritize rares',
          'Watch for void zones'
        ]
      },
      uulNetol: {
        name: 'Breach of Uul-Netol',
        color: '#8b0000',
        boss: 'Uul-Netol, Unburdened Flesh',
        splinter: 'Splinter of Uul-Netol',
        blessingName: "Uul-Netol's Breachstone",
        element: 'Physical',
        ailithProtection: 'Armour & Physical Mitigation',
        hiveLocation: 'Crimson Halls',
        hiveborn: 'Fleshbound Colossus',
        strategies: [
          'Build armour or endurance charges',
          'Watch for slam attacks',
          'Kite around breach perimeter',
          'Save movement skills for emergencies'
        ]
      }
    };

    this.graftbloodRewards = {
      breachSplinters: { min: 3, max: 12, weight: 100 },
      stacks: { min: 1, max: 3, weight: 50 },
      breachstone: { chance: 0.05, weight: 10 },
      blessedBreachstone: { chance: 0.01, weight: 5 },
      uniqueItems: { chance: 0.15, weight: 25 }
    };

    // Track current session data
    this.sessionData = {
      breachesCompleted: 0,
      splintersCounted: {
        tul: 0,
        xoph: 0,
        esh: 0,
        'uul-netol': 0,
        chayula: 0
      },
      graftbloodEarned: 0,
      hivesCompleted: [],
      ailithProtectionActive: false,
      currentBreachType: null,
      unstableBreachTimer: null
    };
  }

  /**
   * Start tracking a breach encounter
   * @param {string} breachType - Type of breach (physical, fire, cold, lightning, chaos, uulNetol)
   * @param {boolean} isUnstable - Whether this is an unstable breach
   */
  startBreach(breachType, isUnstable = false) {
    if (!this.breachTypes[breachType]) {
      console.error('Invalid breach type:', breachType);
      return null;
    }

    this.sessionData.currentBreachType = breachType;

    const breachInfo = {
      type: this.breachTypes[breachType],
      isUnstable,
      startTime: Date.now(),
      timer: isUnstable ? 45 : null, // Unstable breaches have 45s timer
      waveCount: 0,
      monstersKilled: 0,
      raresKilled: 0
    };

    if (isUnstable) {
      this.sessionData.unstableBreachTimer = breachInfo.startTime + (45 * 1000);
    }

    console.log(`Started ${isUnstable ? 'Unstable ' : ''}Breach: ${breachInfo.type.name}`);

    return breachInfo;
  }

  /**
   * End breach tracking and calculate rewards
   * @param {number} splinters - Number of splinters dropped
   * @param {number} graftblood - Graftblood currency earned
   */
  endBreach(splinters = 0, graftblood = 0) {
    if (!this.sessionData.currentBreachType) {
      console.warn('No active breach to end');
      return;
    }

    this.sessionData.breachesCompleted++;

    // Update splinter count
    const breachKey = this.getBreachKey(this.sessionData.currentBreachType);
    if (breachKey && this.sessionData.splintersCounted[breachKey] !== undefined) {
      this.sessionData.splintersCounted[breachKey] += splinters;
    }

    this.sessionData.graftbloodEarned += graftblood;
    this.sessionData.currentBreachType = null;
    this.sessionData.unstableBreachTimer = null;

    console.log(`Breach ended. Splinters: ${splinters}, Graftblood: ${graftblood}`);

    return {
      totalBreaches: this.sessionData.breachesCompleted,
      splinters: this.sessionData.splintersCounted,
      totalGraftblood: this.sessionData.graftbloodEarned
    };
  }

  /**
   * Get breach key for splinter tracking
   * @param {string} breachType
   */
  getBreachKey(breachType) {
    const keyMap = {
      cold: 'tul',
      fire: 'xoph',
      lightning: 'esh',
      uulNetol: 'uul-netol',
      chaos: 'chayula'
    };
    return keyMap[breachType] || 'tul';
  }

  /**
   * Check if enough splinters to create breachstone (requires 100)
   * @param {string} breachType
   */
  canCreateBreachstone(breachType) {
    const breachKey = this.getBreachKey(breachType);
    const count = this.sessionData.splintersCounted[breachKey] || 0;
    return {
      canCreate: count >= 100,
      current: count,
      required: 100,
      remaining: Math.max(0, 100 - count)
    };
  }

  /**
   * Mark a Breach Hive as completed
   * @param {string} hiveType - Type of hive (matches breach type)
   */
  completeHive(hiveType) {
    if (!this.breachTypes[hiveType]) {
      console.error('Invalid hive type:', hiveType);
      return false;
    }

    if (!this.sessionData.hivesCompleted.includes(hiveType)) {
      this.sessionData.hivesCompleted.push(hiveType);
      console.log(`Completed ${this.breachTypes[hiveType].hiveLocation} Hive`);
      return true;
    }

    return false;
  }

  /**
   * Get all hives and their completion status
   */
  getHiveProgress() {
    return Object.keys(this.breachTypes).map(type => ({
      type,
      name: this.breachTypes[type].hiveLocation,
      boss: this.breachTypes[type].hiveborn,
      completed: this.sessionData.hivesCompleted.includes(type),
      color: this.breachTypes[type].color
    }));
  }

  /**
   * Toggle Ailith protection status
   * @param {boolean} active
   */
  setAilithProtection(active) {
    this.sessionData.ailithProtectionActive = active;
    return this.sessionData.ailithProtectionActive;
  }

  /**
   * Get current Ailith protection for active breach
   */
  getCurrentAilithProtection() {
    if (!this.sessionData.currentBreachType) {
      return null;
    }

    const breach = this.breachTypes[this.sessionData.currentBreachType];
    return {
      active: this.sessionData.ailithProtectionActive,
      protection: breach.ailithProtection,
      element: breach.element
    };
  }

  /**
   * Get unstable breach remaining time
   */
  getUnstableBreachTime() {
    if (!this.sessionData.unstableBreachTimer) {
      return null;
    }

    const remaining = Math.max(0, this.sessionData.unstableBreachTimer - Date.now());
    return {
      remaining: Math.floor(remaining / 1000),
      expired: remaining <= 0
    };
  }

  /**
   * Calculate expected Graftblood rewards for breach type
   * @param {string} breachType
   * @param {boolean} isUnstable
   */
  estimateGraftbloodReward(breachType, isUnstable = false) {
    const baseReward = isUnstable ? 8 : 3;
    const multiplier = breachType === 'chaos' ? 1.5 : 1;

    return {
      min: Math.floor(baseReward * multiplier * 0.8),
      max: Math.floor(baseReward * multiplier * 1.2),
      average: Math.floor(baseReward * multiplier)
    };
  }

  /**
   * Get breach boss information
   * @param {string} breachType
   */
  getBossInfo(breachType) {
    if (!this.breachTypes[breachType]) {
      return null;
    }

    const breach = this.breachTypes[breachType];
    return {
      name: breach.boss,
      breachstone: breach.blessingName,
      strategies: breach.strategies,
      recommendedResistance: breach.element,
      hiveborn: breach.hiveborn
    };
  }

  /**
   * Get all breach types with details
   */
  getAllBreachTypes() {
    return Object.keys(this.breachTypes).map(key => ({
      id: key,
      ...this.breachTypes[key]
    }));
  }

  /**
   * Get session statistics
   */
  getSessionStats() {
    const totalSplinters = Object.values(this.sessionData.splintersCounted)
      .reduce((sum, count) => sum + count, 0);

    return {
      breachesCompleted: this.sessionData.breachesCompleted,
      totalSplinters,
      splintersByType: { ...this.sessionData.splintersCounted },
      graftbloodEarned: this.sessionData.graftbloodEarned,
      hivesCompleted: this.sessionData.hivesCompleted.length,
      hiveProgress: this.getHiveProgress(),
      ailithActive: this.sessionData.ailithProtectionActive
    };
  }

  /**
   * Reset session data
   */
  resetSession() {
    this.sessionData = {
      breachesCompleted: 0,
      splintersCounted: {
        tul: 0,
        xoph: 0,
        esh: 0,
        'uul-netol': 0,
        chayula: 0
      },
      graftbloodEarned: 0,
      hivesCompleted: [],
      ailithProtectionActive: false,
      currentBreachType: null,
      unstableBreachTimer: null
    };

    console.log('Session data reset');
    return this.sessionData;
  }

  /**
   * Add splinters manually
   * @param {string} breachType
   * @param {number} count
   */
  addSplinters(breachType, count) {
    const breachKey = this.getBreachKey(breachType);
    if (this.sessionData.splintersCounted[breachKey] !== undefined) {
      this.sessionData.splintersCounted[breachKey] += count;
      return this.sessionData.splintersCounted[breachKey];
    }
    return null;
  }

  /**
   * Add Graftblood currency
   * @param {number} amount
   */
  addGraftblood(amount) {
    this.sessionData.graftbloodEarned += amount;
    return this.sessionData.graftbloodEarned;
  }
}

// Export singleton instance
const breachHelper = new BreachHelper();
module.exports = breachHelper;
module.exports.BreachHelper = BreachHelper;
