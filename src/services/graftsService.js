/**
 * Grafts Service - Manages Graft System for Keepers of the Flame
 * Grafts are powerful modular enhancements that provide unique bonuses and abilities
 * Players can equip up to 2 grafts once the Genesis Tree is leveled
 */

class GraftsService {
  constructor() {
    // Database of all 16 graft types
    this.grafts = [
      // OFFENSIVE GRAFTS
      {
        id: 'graft_crimson_fury',
        name: 'Crimson Fury',
        type: 'offensive',
        rarity: 'unique',
        description: 'Amplifies damage output through blood magic',
        bonuses: [
          '+25% to Global Critical Strike Multiplier',
          '+15% increased Attack and Cast Speed',
          'Skills Cost Life instead of Mana',
          '+10% to Fire Damage'
        ],
        activeSkill: null,
        compatible: ['Berserker', 'Juggernaut', 'Chieftain'],
        acquisition: 'Drops from Breach Lords',
        lore: 'Forged in the blood of ancient warriors, this graft pulses with primal fury.'
      },
      {
        id: 'graft_void_strike',
        name: 'Void Strike',
        type: 'offensive',
        rarity: 'unique',
        description: 'Channels void energy to amplify critical strikes',
        bonuses: [
          '+40% to Global Critical Strike Chance',
          '+20% increased Damage with Hits and Ailments',
          'Crits have 25% chance to apply Withered',
          '+15% to Chaos Damage'
        ],
        activeSkill: null,
        compatible: ['Assassin', 'Trickster', 'Occultist'],
        acquisition: 'Drops from Synthesis Bosses',
        lore: 'The void whispers secrets of destruction to those who dare listen.'
      },
      {
        id: 'graft_lightning_conduit',
        name: 'Lightning Conduit',
        type: 'offensive',
        rarity: 'rare',
        description: 'Harnesses electrical energy for devastating attacks',
        bonuses: [
          '+30% increased Lightning Damage',
          '+20% chance to Shock',
          'Lightning Damage can Shock Chilled Enemies',
          'Shocks you inflict spread to nearby enemies'
        ],
        activeSkill: {
          name: 'Chain Lightning',
          cooldown: 8,
          description: 'Unleash a bolt that chains between 5 enemies',
          damage: 'High'
        },
        compatible: ['Elementalist', 'Inquisitor', 'Hierophant'],
        acquisition: 'Crafted from Storm-Touched Essences',
        lore: 'Lightning courses through veins of living metal.'
      },
      {
        id: 'graft_temporal_blade',
        name: 'Temporal Blade',
        type: 'offensive',
        rarity: 'unique',
        description: 'Manipulates time to enhance attack sequences',
        bonuses: [
          '+25% increased Attack Speed',
          'Attacks Repeat an additional time',
          '+15% chance to gain Haste on Kill',
          '20% reduced Skill Effect Duration'
        ],
        activeSkill: {
          name: 'Time Dilation',
          cooldown: 12,
          description: 'Slow nearby enemies by 50% for 4 seconds',
          damage: 'None'
        },
        compatible: ['Deadeye', 'Raider', 'Pathfinder'],
        acquisition: 'Obtained from Delirium Encounters',
        lore: 'Time bends to the will of those who master this ancient art.'
      },

      // DEFENSIVE GRAFTS
      {
        id: 'graft_stone_skin',
        name: 'Stone Skin',
        type: 'defensive',
        rarity: 'rare',
        description: 'Hardens the body against physical damage',
        bonuses: [
          '+1500 to Armour',
          '+8% additional Physical Damage Reduction',
          '+200 to maximum Life',
          'Regenerate 2% of Life per second'
        ],
        activeSkill: {
          name: 'Fortification',
          cooldown: 10,
          description: 'Gain 75% reduced damage taken for 3 seconds',
          damage: 'None'
        },
        compatible: ['Juggernaut', 'Gladiator', 'Champion'],
        acquisition: 'Drops from Breach: Physical Domain',
        lore: 'The earth itself lends its strength to protect the bearer.'
      },
      {
        id: 'graft_spectral_ward',
        name: 'Spectral Ward',
        type: 'defensive',
        rarity: 'unique',
        description: 'Creates a barrier of pure energy',
        bonuses: [
          '+250 to maximum Energy Shield',
          '+15% increased Energy Shield Recharge Rate',
          'Energy Shield Recharge is not interrupted by Damage if Recharge began Recently',
          '+5% to all Elemental Resistances'
        ],
        activeSkill: null,
        compatible: ['Occultist', 'Trickster', 'Guardian'],
        acquisition: 'Drops from Maven Witnessing',
        lore: 'Between life and death lies a shield of pure will.'
      },
      {
        id: 'graft_phoenix_heart',
        name: 'Phoenix Heart',
        type: 'defensive',
        rarity: 'unique',
        description: 'Grants incredible regenerative capabilities',
        bonuses: [
          'Regenerate 5% of Life per second',
          '+10% to Fire Resistance',
          '+8% increased maximum Life',
          '25% of Life Regeneration applies to Energy Shield'
        ],
        activeSkill: {
          name: 'Resurrection',
          cooldown: 120,
          description: 'Automatically revive once with 50% Life',
          damage: 'None'
        },
        compatible: ['Chieftain', 'Inquisitor', 'Guardian'],
        acquisition: 'Drops from The Phoenix Guardian',
        lore: 'Death is but a temporary setback for those who carry the phoenix flame.'
      },
      {
        id: 'graft_glacial_bulwark',
        name: 'Glacial Bulwark',
        type: 'defensive',
        rarity: 'rare',
        description: 'Surrounds the bearer with protective ice',
        bonuses: [
          '+12% to Cold Resistance',
          '+20% chance to Avoid Elemental Ailments',
          '+150 to maximum Life',
          '30% chance to Freeze attackers on Block'
        ],
        activeSkill: {
          name: 'Ice Barrier',
          cooldown: 15,
          description: 'Create a wall of ice that blocks projectiles for 5 seconds',
          damage: 'None'
        },
        compatible: ['Elementalist', 'Occultist', 'Hierophant'],
        acquisition: 'Drops from Shaper Guardians',
        lore: 'The eternal frost protects those who embrace the cold.'
      },

      // UTILITY GRAFTS
      {
        id: 'graft_shadow_step',
        name: 'Shadow Step',
        type: 'utility',
        rarity: 'rare',
        description: 'Enhances movement and evasion capabilities',
        bonuses: [
          '+35% increased Movement Speed',
          '+25% chance to Evade Attacks',
          '15% increased Attack and Cast Speed during any Flask Effect',
          'Cannot be Stunned while moving'
        ],
        activeSkill: {
          name: 'Phase Dash',
          cooldown: 6,
          description: 'Dash through enemies, gaining Phasing for 3 seconds',
          damage: 'Low'
        },
        compatible: ['Raider', 'Trickster', 'Assassin'],
        acquisition: 'Drops from Vaal Side Areas',
        lore: 'To move is to live; to be still is to die.'
      },
      {
        id: 'graft_arcane_reservoir',
        name: 'Arcane Reservoir',
        type: 'utility',
        rarity: 'rare',
        description: 'Vastly improves resource management',
        bonuses: [
          '+250 to maximum Mana',
          '+40% increased Mana Regeneration Rate',
          'Skills Cost 15% less Mana',
          '8% of Damage taken Recouped as Mana'
        ],
        activeSkill: null,
        compatible: ['Hierophant', 'Inquisitor', 'Necromancer'],
        acquisition: 'Crafted with Essence of Insanity',
        lore: 'The well of power never runs dry for those who know its secrets.'
      },
      {
        id: 'graft_hunters_focus',
        name: "Hunter's Focus",
        type: 'utility',
        rarity: 'unique',
        description: 'Sharpens perception and tracking abilities',
        bonuses: [
          '+30% increased Flask Charges gained',
          'Flasks apply to you 25% faster',
          '+20% to Critical Strike Chance against Rare and Unique Enemies',
          'Culling Strike against Enemies on Full Life'
        ],
        activeSkill: {
          name: 'Mark Prey',
          cooldown: 10,
          description: 'Mark an enemy, taking 15% increased damage from all sources',
          damage: 'None'
        },
        compatible: ['Deadeye', 'Pathfinder', 'Slayer'],
        acquisition: 'Drops from The Hunter Conqueror',
        lore: 'The hunt ends only when the prey falls.'
      },
      {
        id: 'graft_cosmic_insight',
        name: 'Cosmic Insight',
        type: 'utility',
        rarity: 'unique',
        description: 'Grants supernatural awareness and cooldown recovery',
        bonuses: [
          '+20% Cooldown Recovery Rate',
          '+15% increased Area of Effect',
          'Skills Chain an additional time',
          '+1 to Maximum number of Summoned Totems'
        ],
        activeSkill: null,
        compatible: ['Hierophant', 'Necromancer', 'Elementalist'],
        acquisition: 'Drops from Elder Guardians',
        lore: 'The cosmos reveals its patterns to those who observe closely.'
      },

      // ACTIVE SKILL GRAFTS
      {
        id: 'graft_genesis_bloom',
        name: 'Genesis Bloom',
        type: 'active',
        rarity: 'mythic',
        description: 'Channels the power of creation itself',
        bonuses: [
          '+10% to all Elemental Resistances',
          '+100 to maximum Life',
          '+100 to maximum Mana',
          'Regenerate 1% of Life and Mana per second'
        ],
        activeSkill: {
          name: 'Genesis Pulse',
          cooldown: 20,
          description: 'Release a wave that heals allies and damages enemies in a large radius',
          damage: 'Very High'
        },
        compatible: ['All Classes'],
        acquisition: 'Drops from Genesis Tree Final Boss',
        lore: 'At the dawn of creation, all things begin anew.'
      },
      {
        id: 'graft_breach_anchor',
        name: 'Breach Anchor',
        type: 'active',
        rarity: 'mythic',
        description: 'Tears rifts in reality to summon portals',
        bonuses: [
          '+20% increased Damage',
          'Damage Penetrates 10% Elemental Resistances',
          '+100 to maximum Energy Shield',
          'Cannot be Cursed'
        ],
        activeSkill: {
          name: 'Breach Portal',
          cooldown: 30,
          description: 'Open a breach portal that spawns allies and deals chaos damage over time',
          damage: 'Extreme'
        },
        compatible: ['Occultist', 'Trickster', 'Necromancer'],
        acquisition: 'Obtained from Breach Unique Encounters',
        lore: 'The space between worlds is yours to command.'
      },
      {
        id: 'graft_blood_harvest',
        name: 'Blood Harvest',
        type: 'active',
        rarity: 'unique',
        description: 'Converts enemy life force into power',
        bonuses: [
          '10% of Damage Leeched as Life',
          'Life Leech effects are not removed at Full Life',
          '+15% increased Damage per nearby Enemy',
          'Gain Rage on Hit'
        ],
        activeSkill: {
          name: 'Sanguine Nova',
          cooldown: 15,
          description: 'Drain life from all nearby enemies, healing for the total amount',
          damage: 'High'
        },
        compatible: ['Slayer', 'Berserker', 'Gladiator'],
        acquisition: 'Drops from Bloodline Ascendancy Trials',
        lore: 'Their blood becomes your strength.'
      },
      {
        id: 'graft_ancient_pact',
        name: 'Ancient Pact',
        type: 'active',
        rarity: 'mythic',
        description: 'Binds the bearer to primordial forces',
        bonuses: [
          '+1 to Maximum Power Charges',
          '+1 to Maximum Frenzy Charges',
          '+1 to Maximum Endurance Charges',
          'Gain a random Charge every 4 seconds'
        ],
        activeSkill: {
          name: 'Primordial Surge',
          cooldown: 25,
          description: 'Consume all charges to gain massive temporary bonuses based on charges consumed',
          damage: 'Variable'
        },
        compatible: ['All Classes'],
        acquisition: 'Ultimate Endgame Boss Drop',
        lore: 'The old gods still walk, and their power flows through those they favor.'
      }
    ];

    // Track equipped grafts (max 2)
    this.equippedGrafts = [];
    this.maxGrafts = 2;
  }

  /**
   * Get all available grafts
   * @returns {Array}
   */
  getAllGrafts() {
    return this.grafts;
  }

  /**
   * Get grafts by type
   * @param {string} type - offensive, defensive, utility, active
   * @returns {Array}
   */
  getGraftsByType(type) {
    return this.grafts.filter(graft => graft.type === type);
  }

  /**
   * Get graft by ID
   * @param {string} id - Graft ID
   * @returns {Object|null}
   */
  getGraftById(id) {
    return this.grafts.find(graft => graft.id === id) || null;
  }

  /**
   * Get grafts compatible with a specific ascendancy
   * @param {string} ascendancy - Ascendancy name
   * @returns {Array}
   */
  getCompatibleGrafts(ascendancy) {
    return this.grafts.filter(graft =>
      graft.compatible.includes(ascendancy) || graft.compatible.includes('All Classes')
    );
  }

  /**
   * Equip a graft
   * @param {string} graftId - ID of graft to equip
   * @returns {Object} - Result with success status and message
   */
  equipGraft(graftId) {
    if (this.equippedGrafts.length >= this.maxGrafts) {
      return {
        success: false,
        message: `Maximum of ${this.maxGrafts} grafts can be equipped. Unequip one first.`
      };
    }

    const graft = this.getGraftById(graftId);
    if (!graft) {
      return {
        success: false,
        message: 'Graft not found'
      };
    }

    if (this.equippedGrafts.some(g => g.id === graftId)) {
      return {
        success: false,
        message: 'Graft is already equipped'
      };
    }

    this.equippedGrafts.push(graft);
    return {
      success: true,
      message: `${graft.name} equipped successfully`,
      equipped: this.equippedGrafts
    };
  }

  /**
   * Unequip a graft
   * @param {string} graftId - ID of graft to unequip
   * @returns {Object}
   */
  unequipGraft(graftId) {
    const index = this.equippedGrafts.findIndex(g => g.id === graftId);
    if (index === -1) {
      return {
        success: false,
        message: 'Graft is not equipped'
      };
    }

    const graft = this.equippedGrafts.splice(index, 1)[0];
    return {
      success: true,
      message: `${graft.name} unequipped successfully`,
      equipped: this.equippedGrafts
    };
  }

  /**
   * Get currently equipped grafts
   * @returns {Array}
   */
  getEquippedGrafts() {
    return this.equippedGrafts;
  }

  /**
   * Recommend grafts based on build type
   * @param {Object} buildConfig - Build configuration
   * @returns {Array}
   */
  recommendGrafts(buildConfig) {
    const { damageType, defenseType, ascendancy, playstyle } = buildConfig;

    let recommendations = [];

    // Filter by ascendancy compatibility
    let compatible = ascendancy
      ? this.getCompatibleGrafts(ascendancy)
      : this.grafts;

    // Recommend based on damage type
    if (damageType === 'physical') {
      recommendations.push(
        ...compatible.filter(g =>
          g.bonuses.some(b => b.includes('Physical') || b.includes('Attack'))
        )
      );
    } else if (damageType === 'elemental' || damageType === 'fire' || damageType === 'cold' || damageType === 'lightning') {
      recommendations.push(
        ...compatible.filter(g =>
          g.bonuses.some(b => b.includes('Elemental') || b.includes('Fire') || b.includes('Cold') || b.includes('Lightning'))
        )
      );
    } else if (damageType === 'chaos') {
      recommendations.push(
        ...compatible.filter(g =>
          g.bonuses.some(b => b.includes('Chaos') || b.includes('Withered'))
        )
      );
    }

    // Recommend based on defense type
    if (defenseType === 'armour') {
      recommendations.push(
        ...compatible.filter(g => g.type === 'defensive' && g.bonuses.some(b => b.includes('Armour')))
      );
    } else if (defenseType === 'evasion') {
      recommendations.push(
        ...compatible.filter(g => g.type === 'defensive' && g.bonuses.some(b => b.includes('Evade')))
      );
    } else if (defenseType === 'energy_shield') {
      recommendations.push(
        ...compatible.filter(g => g.type === 'defensive' && g.bonuses.some(b => b.includes('Energy Shield')))
      );
    }

    // Recommend based on playstyle
    if (playstyle === 'fast') {
      recommendations.push(
        ...compatible.filter(g =>
          g.bonuses.some(b => b.includes('Movement Speed') || b.includes('Attack Speed') || b.includes('Cast Speed'))
        )
      );
    } else if (playstyle === 'tank') {
      recommendations.push(
        ...compatible.filter(g => g.type === 'defensive')
      );
    } else if (playstyle === 'crit') {
      recommendations.push(
        ...compatible.filter(g =>
          g.bonuses.some(b => b.includes('Critical'))
        )
      );
    }

    // Remove duplicates and limit to top 8
    const uniqueRecommendations = [...new Set(recommendations)];
    return uniqueRecommendations.slice(0, 8);
  }

  /**
   * Get acquisition guide for a graft
   * @param {string} graftId - Graft ID
   * @returns {Object|null}
   */
  getAcquisitionGuide(graftId) {
    const graft = this.getGraftById(graftId);
    if (!graft) return null;

    return {
      name: graft.name,
      acquisition: graft.acquisition,
      rarity: graft.rarity,
      tips: this.getAcquisitionTips(graft.acquisition)
    };
  }

  /**
   * Get tips for acquiring a graft
   * @param {string} acquisition - Acquisition source
   * @returns {string}
   */
  getAcquisitionTips(acquisition) {
    const tips = {
      'Breach Lords': 'Farm Breaches and collect Splinters to access Breach Lord domains',
      'Synthesis Bosses': 'Complete Synthesis Maps to encounter unique bosses',
      'Storm-Touched Essences': 'Use Remnants of Corruption on Screaming Essences',
      'Delirium Encounters': 'Push Delirium as far as possible for better rewards',
      'Breach: Physical Domain': 'Target Tul Breach encounters',
      'Maven Witnessing': 'Invite Maven to witness boss fights',
      'The Phoenix Guardian': 'Complete The Forge of the Phoenix map',
      'Shaper Guardians': 'Defeat all four Shaper Guardians',
      'Vaal Side Areas': 'Look for corrupted zones in maps',
      'Essence of Insanity': 'Farm high-tier essences and corrupt them',
      'The Hunter Conqueror': 'Defeat Al-Hezmin in his Citadel',
      'Elder Guardians': 'Complete Elder-influenced maps',
      'Genesis Tree Final Boss': 'Complete the Genesis Tree storyline',
      'Breach Unique Encounters': 'Farm Breachstones with maximum upgrades',
      'Bloodline Ascendancy Trials': 'Complete all Bloodline trials',
      'Ultimate Endgame Boss Drop': 'Defeat the final endgame boss'
    };

    return tips[acquisition] || 'No specific tips available';
  }

  /**
   * Clear all equipped grafts
   */
  clearEquipped() {
    this.equippedGrafts = [];
  }

  /**
   * Get graft statistics
   * @returns {Object}
   */
  getStatistics() {
    return {
      total: this.grafts.length,
      offensive: this.getGraftsByType('offensive').length,
      defensive: this.getGraftsByType('defensive').length,
      utility: this.getGraftsByType('utility').length,
      active: this.getGraftsByType('active').length,
      equipped: this.equippedGrafts.length,
      maxEquippable: this.maxGrafts
    };
  }
}

// Export singleton instance
const graftsService = new GraftsService();
module.exports = graftsService;
module.exports.GraftsService = GraftsService;
