/**
 * Atlas Tracker Service - Manages POE Atlas progression tracking
 * Tracks map completion, boss kills, Maven invitations, and Atlas passives
 */

class AtlasTrackerService {
  constructor() {
    this.progression = {
      // Voidstones (3.27 mechanics)
      voidstones: {
        total: 4,
        collected: [],
      },
      // Maven progression
      maven: {
        invitations: {
          'The Formed': { completed: false, region: 'Haewark Hamlet' },
          'The Twisted': { completed: false, region: 'Tirn\'s End' },
          'The Forgotten': { completed: false, region: 'Lex Proxima' },
          'The Hidden': { completed: false, region: 'Lex Ejoris' },
          'The Feared': { completed: false, region: 'Valdo\'s Rest' },
          'The Elderslayers': { completed: false, region: 'Lira Arthain' },
          'The Maven\'s Writ': { completed: false, region: 'Special' },
        },
        witnessedBosses: 0,
        totalBosses: 30,
      },
      // Uber bosses
      uberBosses: {
        'The Maven': { killed: false, attempts: 0 },
        'Uber Maven': { killed: false, attempts: 0 },
        'The Searing Exarch': { killed: false, attempts: 0 },
        'The Eater of Worlds': { killed: false, attempts: 0 },
        'Uber Searing Exarch': { killed: false, attempts: 0 },
        'Uber Eater of Worlds': { killed: false, attempts: 0 },
        'Uber Elder': { killed: false, attempts: 0 },
        'The Shaper': { killed: false, attempts: 0 },
        'Uber Shaper': { killed: false, attempts: 0 },
        'Sirus': { killed: false, attempts: 0, awakenLevel: 0 },
        'The Cortex': { killed: false, attempts: 0 },
        'The Feared (all unique)': { killed: false, attempts: 0 },
      },
      // Map completion
      maps: {
        white: { completed: 0, total: 50, bonus: 0, awakened: 0 },
        yellow: { completed: 0, total: 40, bonus: 0, awakened: 0 },
        red: { completed: 0, total: 45, bonus: 0, awakened: 0 },
      },
      // Atlas passive points
      atlasPassives: {
        earned: 0,
        allocated: 0,
        total: 132, // Max available passive points
      },
      // Favorite map slots
      favoriteMapSlots: {
        unlocked: 0,
        total: 12,
      },
      // New 3.27 Voidborn content
      voidborn: {
        encountersCompleted: 0,
        keysUsed: 0,
        uniqueBossesDefeated: [],
      },
      // Completion percentage
      completion: {
        overallAtlas: 0,
        mapCompletion: 0,
        bossKills: 0,
        mavenProgress: 0,
      },
    };

    this.loadProgression();
  }

  /**
   * Update voidstone collection
   * @param {number} index - Voidstone index (0-3)
   * @param {string} source - How it was obtained
   */
  collectVoidstone(index, source) {
    if (index < 0 || index >= 4) return;

    if (!this.progression.voidstones.collected.includes(index)) {
      this.progression.voidstones.collected.push(index);
      this.progression.voidstones.collected.sort();
      console.log(`Voidstone ${index + 1} collected from ${source}`);
      this.calculateCompletion();
      this.saveProgression();
    }
  }

  /**
   * Complete a Maven invitation
   * @param {string} invitationName - Name of the invitation
   */
  completeMavenInvitation(invitationName) {
    if (this.progression.maven.invitations[invitationName]) {
      this.progression.maven.invitations[invitationName].completed = true;
      console.log(`Maven invitation completed: ${invitationName}`);
      this.calculateCompletion();
      this.saveProgression();
    }
  }

  /**
   * Record boss kill
   * @param {string} bossName - Name of the boss
   */
  recordBossKill(bossName) {
    if (this.progression.uberBosses[bossName]) {
      this.progression.uberBosses[bossName].killed = true;
      this.progression.uberBosses[bossName].attempts += 1;
      console.log(`Boss killed: ${bossName}`);
      this.calculateCompletion();
      this.saveProgression();
    }
  }

  /**
   * Record boss attempt (death/fail)
   * @param {string} bossName - Name of the boss
   */
  recordBossAttempt(bossName) {
    if (this.progression.uberBosses[bossName]) {
      this.progression.uberBosses[bossName].attempts += 1;
      this.saveProgression();
    }
  }

  /**
   * Update map completion
   * @param {string} tier - Map tier (white, yellow, red)
   * @param {number} completed - Number of maps completed
   * @param {number} bonus - Number of bonus objectives
   * @param {number} awakened - Number of awakened bonuses
   */
  updateMapCompletion(tier, completed, bonus = 0, awakened = 0) {
    if (!this.progression.maps[tier]) return;

    this.progression.maps[tier].completed = Math.min(completed, this.progression.maps[tier].total);
    this.progression.maps[tier].bonus = Math.min(bonus, this.progression.maps[tier].total);
    this.progression.maps[tier].awakened = Math.min(awakened, this.progression.maps[tier].total);

    this.calculateCompletion();
    this.saveProgression();
  }

  /**
   * Add map completion
   * @param {string} tier - Map tier
   * @param {boolean} bonus - Bonus objective completed
   * @param {boolean} awakened - Awakened bonus completed
   */
  addMapCompletion(tier, bonus = false, awakened = false) {
    if (!this.progression.maps[tier]) return;

    this.progression.maps[tier].completed = Math.min(
      this.progression.maps[tier].completed + 1,
      this.progression.maps[tier].total
    );

    if (bonus) {
      this.progression.maps[tier].bonus = Math.min(
        this.progression.maps[tier].bonus + 1,
        this.progression.maps[tier].total
      );
    }

    if (awakened) {
      this.progression.maps[tier].awakened = Math.min(
        this.progression.maps[tier].awakened + 1,
        this.progression.maps[tier].total
      );
    }

    this.calculateCompletion();
    this.saveProgression();
  }

  /**
   * Update Atlas passive points
   * @param {number} earned - Total earned points
   * @param {number} allocated - Points allocated to tree
   */
  updateAtlasPassives(earned, allocated) {
    this.progression.atlasPassives.earned = Math.min(earned, this.progression.atlasPassives.total);
    this.progression.atlasPassives.allocated = Math.min(allocated, earned);
    this.saveProgression();
  }

  /**
   * Add Atlas passive point
   */
  addAtlasPassivePoint() {
    if (this.progression.atlasPassives.earned < this.progression.atlasPassives.total) {
      this.progression.atlasPassives.earned += 1;
      this.saveProgression();
    }
  }

  /**
   * Update favorite map slots
   * @param {number} unlocked - Number of slots unlocked
   */
  updateFavoriteMapSlots(unlocked) {
    this.progression.favoriteMapSlots.unlocked = Math.min(unlocked, this.progression.favoriteMapSlots.total);
    this.saveProgression();
  }

  /**
   * Add Voidborn encounter completion
   */
  addVoidbornEncounter() {
    this.progression.voidborn.encountersCompleted += 1;
    this.saveProgression();
  }

  /**
   * Record Voidborn key usage
   */
  useVoidbornKey() {
    this.progression.voidborn.keysUsed += 1;
    this.saveProgression();
  }

  /**
   * Defeat Voidborn unique boss
   * @param {string} bossName - Name of the Voidborn boss
   */
  defeatVoidbornBoss(bossName) {
    if (!this.progression.voidborn.uniqueBossesDefeated.includes(bossName)) {
      this.progression.voidborn.uniqueBossesDefeated.push(bossName);
      this.saveProgression();
    }
  }

  /**
   * Witness a Maven boss
   */
  witnessMavenBoss() {
    if (this.progression.maven.witnessedBosses < this.progression.maven.totalBosses) {
      this.progression.maven.witnessedBosses += 1;
      this.calculateCompletion();
      this.saveProgression();
    }
  }

  /**
   * Calculate overall completion percentages
   */
  calculateCompletion() {
    // Map completion percentage
    const totalMaps = this.progression.maps.white.total +
                      this.progression.maps.yellow.total +
                      this.progression.maps.red.total;
    const completedMaps = this.progression.maps.white.completed +
                          this.progression.maps.yellow.completed +
                          this.progression.maps.red.completed;
    this.progression.completion.mapCompletion = (completedMaps / totalMaps) * 100;

    // Boss kills percentage
    const totalBosses = Object.keys(this.progression.uberBosses).length;
    const killedBosses = Object.values(this.progression.uberBosses).filter(b => b.killed).length;
    this.progression.completion.bossKills = (killedBosses / totalBosses) * 100;

    // Maven progress percentage
    const totalInvitations = Object.keys(this.progression.maven.invitations).length;
    const completedInvitations = Object.values(this.progression.maven.invitations).filter(i => i.completed).length;
    this.progression.completion.mavenProgress = (completedInvitations / totalInvitations) * 100;

    // Overall Atlas completion (weighted average)
    this.progression.completion.overallAtlas = (
      this.progression.completion.mapCompletion * 0.4 +
      this.progression.completion.bossKills * 0.3 +
      this.progression.completion.mavenProgress * 0.2 +
      (this.progression.voidstones.collected.length / 4) * 100 * 0.1
    );
  }

  /**
   * Get missing maps checklist
   * @returns {Array}
   */
  getMissingMaps() {
    const missing = [];

    const addMissing = (tier, tierName) => {
      const remaining = this.progression.maps[tier].total - this.progression.maps[tier].completed;
      if (remaining > 0) {
        missing.push({
          tier: tierName,
          remaining: remaining,
          percentage: (this.progression.maps[tier].completed / this.progression.maps[tier].total) * 100,
        });
      }
    };

    addMissing('white', 'White Maps (T1-T5)');
    addMissing('yellow', 'Yellow Maps (T6-T10)');
    addMissing('red', 'Red Maps (T11-T16)');

    return missing;
  }

  /**
   * Get uncompleted Maven invitations
   * @returns {Array}
   */
  getUncompletedInvitations() {
    return Object.entries(this.progression.maven.invitations)
      .filter(([name, data]) => !data.completed)
      .map(([name, data]) => ({
        name,
        region: data.region,
      }));
  }

  /**
   * Get unkilled bosses
   * @returns {Array}
   */
  getUnkilledBosses() {
    return Object.entries(this.progression.uberBosses)
      .filter(([name, data]) => !data.killed)
      .map(([name, data]) => ({
        name,
        attempts: data.attempts,
      }));
  }

  /**
   * Get recommended progression path
   * @returns {Array}
   */
  getRecommendedPath() {
    const path = [];

    // Check voidstones
    if (this.progression.voidstones.collected.length < 4) {
      path.push({
        step: 'Collect Voidstones',
        description: `Obtain remaining ${4 - this.progression.voidstones.collected.length} voidstone(s) from endgame bosses`,
        priority: 'high',
      });
    }

    // Check map completion
    const mapCompletion = this.progression.completion.mapCompletion;
    if (mapCompletion < 100) {
      path.push({
        step: 'Complete Atlas Maps',
        description: `Complete ${Math.ceil((135 - (this.progression.maps.white.completed + this.progression.maps.yellow.completed + this.progression.maps.red.completed)))} remaining maps`,
        priority: 'medium',
      });
    }

    // Check Maven progression
    const uncompletedInvitations = this.getUncompletedInvitations();
    if (uncompletedInvitations.length > 0) {
      path.push({
        step: 'Complete Maven Invitations',
        description: `Complete ${uncompletedInvitations.length} remaining invitation(s)`,
        priority: 'high',
      });
    }

    // Check boss kills
    const unkilledBosses = this.getUnkilledBosses();
    if (unkilledBosses.length > 0) {
      path.push({
        step: 'Defeat Endgame Bosses',
        description: `Kill ${unkilledBosses.length} remaining boss(es)`,
        priority: 'medium',
      });
    }

    // Check Atlas passives
    if (this.progression.atlasPassives.earned < this.progression.atlasPassives.total) {
      path.push({
        step: 'Earn Atlas Passive Points',
        description: `Earn ${this.progression.atlasPassives.total - this.progression.atlasPassives.earned} more passive points`,
        priority: 'low',
      });
    }

    return path;
  }

  /**
   * Get all progression data
   * @returns {Object}
   */
  getProgression() {
    return this.progression;
  }

  /**
   * Reset all progression
   */
  resetProgression() {
    if (confirm('Are you sure you want to reset all Atlas progression? This cannot be undone.')) {
      this.progression = {
        voidstones: { total: 4, collected: [] },
        maven: {
          invitations: {
            'The Formed': { completed: false, region: 'Haewark Hamlet' },
            'The Twisted': { completed: false, region: 'Tirn\'s End' },
            'The Forgotten': { completed: false, region: 'Lex Proxima' },
            'The Hidden': { completed: false, region: 'Lex Ejoris' },
            'The Feared': { completed: false, region: 'Valdo\'s Rest' },
            'The Elderslayers': { completed: false, region: 'Lira Arthain' },
            'The Maven\'s Writ': { completed: false, region: 'Special' },
          },
          witnessedBosses: 0,
          totalBosses: 30,
        },
        uberBosses: {
          'The Maven': { killed: false, attempts: 0 },
          'Uber Maven': { killed: false, attempts: 0 },
          'The Searing Exarch': { killed: false, attempts: 0 },
          'The Eater of Worlds': { killed: false, attempts: 0 },
          'Uber Searing Exarch': { killed: false, attempts: 0 },
          'Uber Eater of Worlds': { killed: false, attempts: 0 },
          'Uber Elder': { killed: false, attempts: 0 },
          'The Shaper': { killed: false, attempts: 0 },
          'Uber Shaper': { killed: false, attempts: 0 },
          'Sirus': { killed: false, attempts: 0, awakenLevel: 0 },
          'The Cortex': { killed: false, attempts: 0 },
          'The Feared (all unique)': { killed: false, attempts: 0 },
        },
        maps: {
          white: { completed: 0, total: 50, bonus: 0, awakened: 0 },
          yellow: { completed: 0, total: 40, bonus: 0, awakened: 0 },
          red: { completed: 0, total: 45, bonus: 0, awakened: 0 },
        },
        atlasPassives: { earned: 0, allocated: 0, total: 132 },
        favoriteMapSlots: { unlocked: 0, total: 12 },
        voidborn: { encountersCompleted: 0, keysUsed: 0, uniqueBossesDefeated: [] },
        completion: { overallAtlas: 0, mapCompletion: 0, bossKills: 0, mavenProgress: 0 },
      };
      this.saveProgression();
    }
  }

  /**
   * Save progression to localStorage
   */
  saveProgression() {
    if (typeof window !== 'undefined' && window.localStorage) {
      localStorage.setItem('poe-atlas-progression', JSON.stringify(this.progression));
    }
  }

  /**
   * Load progression from localStorage
   */
  loadProgression() {
    if (typeof window !== 'undefined' && window.localStorage) {
      const saved = localStorage.getItem('poe-atlas-progression');
      if (saved) {
        try {
          this.progression = JSON.parse(saved);
          this.calculateCompletion();
        } catch (error) {
          console.error('Error loading Atlas progression:', error);
        }
      }
    }
  }
}

// Export singleton instance
const atlasTrackerService = new AtlasTrackerService();
module.exports = atlasTrackerService;
module.exports.AtlasTrackerService = AtlasTrackerService;
