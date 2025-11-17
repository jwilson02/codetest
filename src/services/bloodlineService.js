/**
 * Bloodline Ascendancy Service - Manages Bloodline Ascendancy System for Keepers of the Flame
 * Bloodline Ascendancies are secondary ascendancy trees with unique passive skills
 * Each has specific unlock requirements and synergizes with different main ascendancies
 */

class BloodlineService {
  constructor() {
    // Database of all 10 Bloodline Ascendancy trees
    this.bloodlines = [
      {
        id: 'bloodline_breach_walker',
        name: 'Breach Walker',
        theme: 'Breach',
        description: 'Master of the space between worlds, channeling breach energy',
        icon: 'breach',
        color: '#8b5cf6',
        unlockRequirement: {
          boss: 'Xesht, Breachlord of the Void',
          location: 'Xesht Domain (100 Splinters)',
          difficulty: 'High'
        },
        notables: [
          {
            name: 'Void Touched',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              'Damage Penetrates 15% Chaos Resistance',
              '+20% to Chaos Damage',
              'Enemies near your Breaches are Withered'
            ],
            description: 'Embrace the power of the void between realms'
          },
          {
            name: 'Reality Fracture',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              '+25% increased Breach Radius',
              'Breaches last 5 seconds longer',
              '+10% chance to spawn Breach Monsters'
            ],
            description: 'Tear larger rifts in the fabric of reality'
          },
          {
            name: 'Splinter Harvest',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              '+30% increased Splinter Drop Rate',
              'Breach Monsters have +15% chance to drop additional items',
              '+10% increased Item Quantity in Breaches'
            ],
            description: 'The void rewards those who delve deeper'
          },
          {
            name: 'Unstable Vortex',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'Breaches cannot close while you are inside',
              'Take 30% increased Damage while in Breaches',
              'Deal 40% more Damage while in Breaches'
            ],
            description: 'Risk and reward merge at the edge of oblivion'
          }
        ],
        bestCombinations: ['Occultist', 'Trickster', 'Assassin'],
        synergyBonus: 'Grants +1 to Maximum Breach Encounters per Map'
      },
      {
        id: 'bloodline_genesis_seeker',
        name: 'Genesis Seeker',
        theme: 'Genesis Tree',
        description: 'Channel the primordial energies of the Genesis Tree',
        icon: 'genesis',
        color: '#10b981',
        unlockRequirement: {
          boss: 'The Primordial Warden',
          location: 'Heart of the Genesis Tree',
          difficulty: 'Extreme'
        },
        notables: [
          {
            name: 'Primordial Bond',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              '+1 to All Attributes per 10 Strength, Dexterity, or Intelligence',
              'Regenerate 1% of Life and Mana per second',
              '+5% to all Elemental Resistances'
            ],
            description: 'The first tree still remembers the dawn of creation'
          },
          {
            name: 'Seeds of Power',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              '+15% increased Experience Gain',
              '+10% increased Skill Effect Duration',
              'Skills have +1 Chain'
            ],
            description: 'From small seeds, mighty power grows'
          },
          {
            name: 'Ancient Growth',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              '+200 to maximum Life',
              '+100 to maximum Mana',
              '+8% increased maximum Life and Mana'
            ],
            description: 'The old growth shelters all who seek it'
          },
          {
            name: 'Eternal Bloom',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'Regeneration effects are doubled',
              'Cannot Leech Life or Mana',
              'Life and Mana Recovery from Flasks applies instantly'
            ],
            description: 'Life renews eternally at the tree\'s heart'
          }
        ],
        bestCombinations: ['Guardian', 'Hierophant', 'Chieftain'],
        synergyBonus: 'Passive Skills grant 5% increased Effect'
      },
      {
        id: 'bloodline_crimson_pact',
        name: 'Crimson Pact',
        theme: 'Blood Magic',
        description: 'Master the forbidden art of blood sorcery',
        icon: 'blood',
        color: '#dc2626',
        unlockRequirement: {
          boss: 'The Blood Sovereign',
          location: 'Crimson Cathedral',
          difficulty: 'High'
        },
        notables: [
          {
            name: 'Sanguine Mastery',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              'Skills Cost Life instead of Mana',
              'Life Cost of Skills is 50% of Mana Cost',
              '15% of Damage Leeched as Life'
            ],
            description: 'Blood is the ultimate currency of power'
          },
          {
            name: 'Hemomancy',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              'Gain 1 Rage when you spend Life for a Skill',
              '+25% increased Damage while you have Rage',
              'Lose 5 Rage per second'
            ],
            description: 'Fury flows from spilled blood'
          },
          {
            name: 'Vital Sacrifice',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              '+30% increased Damage when on Low Life',
              '+300 to maximum Life',
              'Effects on Low Life also apply while on Full Life'
            ],
            description: 'The price of power is always paid in blood'
          },
          {
            name: 'Exsanguination',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'Deal 10% of your Maximum Life as Extra Chaos Damage',
              'Lose 5% of your Maximum Life per second',
              'You are immune to Bleeding'
            ],
            description: 'Your very essence becomes a weapon'
          }
        ],
        bestCombinations: ['Berserker', 'Slayer', 'Gladiator'],
        synergyBonus: 'Life Costs can Overleech'
      },
      {
        id: 'bloodline_temporal_weaver',
        name: 'Temporal Weaver',
        theme: 'Time Manipulation',
        description: 'Bend time to your will',
        icon: 'time',
        color: '#06b6d4',
        unlockRequirement: {
          boss: 'Chronos, the Time Keeper',
          location: 'Temporal Sanctum',
          difficulty: 'Very High'
        },
        notables: [
          {
            name: 'Temporal Flux',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              '+35% Cooldown Recovery Rate',
              'Skills used Recently have 25% increased Effect',
              'Debuffs on You expire 25% faster'
            ],
            description: 'Time flows differently for those who understand it'
          },
          {
            name: 'Accelerated Evolution',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              '+30% increased Attack and Cast Speed',
              '+20% increased Movement Speed',
              'Buffs on You expire 15% slower'
            ],
            description: 'Move through time as water flows downhill'
          },
          {
            name: 'Moment of Clarity',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              'Skills Repeat an additional time',
              '+15% chance to avoid Stun and Freeze',
              'Temporal Chains has no effect on you'
            ],
            description: 'See all moments at once'
          },
          {
            name: 'Paradox',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'Your Skills trigger a second time 2 seconds later',
              'Cannot deal Damage with Hits',
              'Damage over Time you inflict is Doubled'
            ],
            description: 'Cause and effect become one'
          }
        ],
        bestCombinations: ['Assassin', 'Raider', 'Deadeye'],
        synergyBonus: 'Temporal Buffs apply to nearby Allies'
      },
      {
        id: 'bloodline_void_herald',
        name: 'Void Herald',
        theme: 'Chaos & Void',
        description: 'Become a conduit for the primordial chaos',
        icon: 'void',
        color: '#7c3aed',
        unlockRequirement: {
          boss: 'The Voidborn Matriarch',
          location: 'Abyssal Depths Level 5',
          difficulty: 'Very High'
        },
        notables: [
          {
            name: 'Void Essence',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              '+40% to Chaos Damage',
              'Chaos Damage can Poison',
              'Your Chaos Damage Ignores Energy Shield'
            ],
            description: 'The void hungers for all things'
          },
          {
            name: 'Entropic Touch',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              'Enemies you Kill have a 25% chance to Explode',
              'Explosions deal 10% of Enemy Maximum Life as Chaos Damage',
              '+15% to Chaos Resistance'
            ],
            description: 'Entropy claims all in the end'
          },
          {
            name: 'Withering Presence',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              'Enemies near you are Withered',
              'Withered Enemies take 5% increased Chaos Damage per stack',
              'You can apply up to 20 Withered Debuffs'
            ],
            description: 'Your mere presence drains vitality'
          },
          {
            name: 'Abyssal Consumption',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'You gain Energy Shield equal to 50% of Chaos Damage dealt',
              'Enemies cannot Regenerate Life',
              'Take 20% increased Chaos Damage'
            ],
            description: 'Feed the void and it feeds you'
          }
        ],
        bestCombinations: ['Occultist', 'Trickster', 'Pathfinder'],
        synergyBonus: 'Chaos Skills gain +2 Chain'
      },
      {
        id: 'bloodline_storm_sovereign',
        name: 'Storm Sovereign',
        theme: 'Lightning & Weather',
        description: 'Command the fury of the storm',
        icon: 'lightning',
        color: '#eab308',
        unlockRequirement: {
          boss: 'Tempest Lord Zephyros',
          location: 'Eye of the Storm',
          difficulty: 'High'
        },
        notables: [
          {
            name: 'Lightning Sovereignty',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              '+40% to Lightning Damage',
              'Lightning Damage can Shock Enemies regardless of Life total',
              'Shocks you inflict have +25% Effect'
            ],
            description: 'The storm answers to your command'
          },
          {
            name: 'Chain Reaction',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              'Lightning Skills Chain +3 times',
              '+20% increased Shock Duration',
              'Shocked Enemies take 15% increased Damage'
            ],
            description: 'Lightning seeks all paths'
          },
          {
            name: 'Conductivity',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              '+12% to Lightning Resistance',
              '30% of Lightning Damage taken is Converted to Mana',
              'Cannot be Shocked'
            ],
            description: 'Become one with the tempest'
          },
          {
            name: 'Thunderous Rebuke',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'When Hit, Shock nearby Enemies',
              'Reflected Damage cannot Shock you',
              'Deal 50% more Lightning Damage to Shocked Enemies'
            ],
            description: 'Return lightning with lightning'
          }
        ],
        bestCombinations: ['Elementalist', 'Inquisitor', 'Assassin'],
        synergyBonus: 'Lightning Damage has 20% chance to Freeze'
      },
      {
        id: 'bloodline_infernal_compact',
        name: 'Infernal Compact',
        theme: 'Fire & Destruction',
        description: 'Embrace the purifying flames of destruction',
        icon: 'fire',
        color: '#f97316',
        unlockRequirement: {
          boss: 'Ignis, the Eternal Flame',
          location: 'Infernal Crucible',
          difficulty: 'High'
        },
        notables: [
          {
            name: 'Blazing Covenant',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              '+40% to Fire Damage',
              'Ignites you inflict deal Damage 30% faster',
              '+20% chance to Ignite'
            ],
            description: 'Fire purifies all it touches'
          },
          {
            name: 'Pyroclasm',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              'Ignited Enemies you Kill Explode',
              'Explosion deals 15% of their Life as Fire Damage',
              '+10% to Fire Resistance'
            ],
            description: 'From ashes, more flames arise'
          },
          {
            name: 'Phoenix Rebirth',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              'Regenerate 3% of Life per second',
              '+100 to maximum Life',
              'Cannot be Ignited'
            ],
            description: 'Rise from your own ashes'
          },
          {
            name: 'Inferno',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'Deal 100% more Fire Damage',
              'You are always Ignited',
              'Regenerate 3% of Life per second while Ignited'
            ],
            description: 'Become the flame itself'
          }
        ],
        bestCombinations: ['Chieftain', 'Elementalist', 'Inquisitor'],
        synergyBonus: 'Fire Damage Leeches Life'
      },
      {
        id: 'bloodline_glacial_tyrant',
        name: 'Glacial Tyrant',
        theme: 'Ice & Frost',
        description: 'Rule over the frozen wastelands',
        icon: 'ice',
        color: '#06b6d4',
        unlockRequirement: {
          boss: 'Boreas, the Frost King',
          location: 'Frozen Citadel',
          difficulty: 'High'
        },
        notables: [
          {
            name: 'Absolute Zero',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              '+40% to Cold Damage',
              '+30% increased Freeze Duration',
              'Frozen Enemies take 20% increased Damage from your Hits'
            ],
            description: 'At absolute zero, all things cease'
          },
          {
            name: 'Permafrost',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              'Chilled Ground you create also Freezes',
              '+25% increased Area of Effect of Cold Skills',
              '+12% to Cold Resistance'
            ],
            description: 'The eternal ice claims all'
          },
          {
            name: 'Glacial Fortress',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              '+300 to maximum Energy Shield',
              'Regenerate 2% of Energy Shield per second',
              'Cannot be Chilled or Frozen'
            ],
            description: 'Protected by walls of eternal ice'
          },
          {
            name: 'Shatter',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'Frozen Enemies you Shatter Explode',
              'Explosion deals 20% of Enemy Maximum Life as Cold Damage',
              'Cannot deal Damage to Enemies that are not Chilled or Frozen'
            ],
            description: 'All things must break'
          }
        ],
        bestCombinations: ['Elementalist', 'Occultist', 'Assassin'],
        synergyBonus: 'Cold Damage cannot be Reflected'
      },
      {
        id: 'bloodline_necro_architect',
        name: 'Necrotic Architect',
        theme: 'Death & Undeath',
        description: 'Master the forces of death and resurrection',
        icon: 'necro',
        color: '#10b981',
        unlockRequirement: {
          boss: 'Mortis, the Death Architect',
          location: 'Necropolis Prime',
          difficulty: 'Very High'
        },
        notables: [
          {
            name: 'Death\'s Design',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              '+2 to Maximum number of Raised Zombies',
              '+2 to Maximum number of Raised Spectres',
              'Minions have +30% increased Maximum Life'
            ],
            description: 'Death is but a doorway'
          },
          {
            name: 'Grave Domain',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              'Minions deal 40% increased Damage',
              'Minions have +15% to all Resistances',
              '25% increased Minion Duration'
            ],
            description: 'The grave yields endless servants'
          },
          {
            name: 'Life from Death',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              'Regenerate 2% of Life when a Minion Dies',
              '+5% to Chaos Resistance',
              'Cannot be Cursed'
            ],
            description: 'Every death feeds the living'
          },
          {
            name: 'Unholy Congregation',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'Minions count as Rare for modifiers',
              'Maximum of 20 Raised Minions',
              'Minions have 50% less Maximum Life'
            ],
            description: 'Quality over quantity in the army of the damned'
          }
        ],
        bestCombinations: ['Necromancer', 'Guardian', 'Occultist'],
        synergyBonus: 'Minions gain your Ascendancy Bonuses'
      },
      {
        id: 'bloodline_astral_conduit',
        name: 'Astral Conduit',
        theme: 'Celestial & Stars',
        description: 'Channel the power of the stars themselves',
        icon: 'astral',
        color: '#a855f7',
        unlockRequirement: {
          boss: 'Celestia, the Star Weaver',
          location: 'Astral Observatory',
          difficulty: 'Extreme'
        },
        notables: [
          {
            name: 'Stellar Alignment',
            position: { x: 0, y: 0 },
            type: 'keystone',
            bonuses: [
              '+1 to Maximum Power, Frenzy, and Endurance Charges',
              'Gain a Random Charge every 3 seconds',
              '+15% to all Elemental Resistances'
            ],
            description: 'The stars align for those who watch'
          },
          {
            name: 'Cosmic Ray',
            position: { x: -100, y: 50 },
            type: 'notable',
            bonuses: [
              '+25% to all Elemental Damage',
              'Skills Pierce 3 additional Targets',
              '+20% increased Projectile Speed'
            ],
            description: 'Light from distant stars pierces all'
          },
          {
            name: 'Astral Protection',
            position: { x: 100, y: 50 },
            type: 'notable',
            bonuses: [
              '+200 to maximum Energy Shield',
              '20% of Elemental Damage taken as Chaos Damage',
              'Regenerate 1% of Energy Shield per second per Power Charge'
            ],
            description: 'The cosmos shields its chosen'
          },
          {
            name: 'Supernova',
            position: { x: 0, y: 100 },
            type: 'keystone',
            bonuses: [
              'When you consume Charges, deal AoE Damage equal to 200% of your Maximum Charges',
              'Charges are consumed automatically when they reach Maximum',
              '+50% to Critical Strike Multiplier when you have Maximum Charges'
            ],
            description: 'Stars die to birth new galaxies'
          }
        ],
        bestCombinations: ['Inquisitor', 'Hierophant', 'Elementalist'],
        synergyBonus: 'Elemental Skills gain additional effects based on active charges'
      }
    ];

    // Currently selected bloodline
    this.selectedBloodline = null;
    this.allocatedNodes = [];
  }

  /**
   * Get all bloodline ascendancies
   * @returns {Array}
   */
  getAllBloodlines() {
    return this.bloodlines;
  }

  /**
   * Get bloodline by ID
   * @param {string} id - Bloodline ID
   * @returns {Object|null}
   */
  getBloodlineById(id) {
    return this.bloodlines.find(b => b.id === id) || null;
  }

  /**
   * Get bloodlines by theme
   * @param {string} theme - Theme name
   * @returns {Array}
   */
  getBloodlinesByTheme(theme) {
    return this.bloodlines.filter(b => b.theme.toLowerCase().includes(theme.toLowerCase()));
  }

  /**
   * Get best bloodlines for a main ascendancy
   * @param {string} ascendancy - Main ascendancy name
   * @returns {Array}
   */
  getBestBloodlinesForAscendancy(ascendancy) {
    return this.bloodlines.filter(b =>
      b.bestCombinations.includes(ascendancy)
    ).sort((a, b) => {
      // Prioritize based on how early the ascendancy appears in bestCombinations
      const aIndex = a.bestCombinations.indexOf(ascendancy);
      const bIndex = b.bestCombinations.indexOf(ascendancy);
      return aIndex - bIndex;
    });
  }

  /**
   * Select a bloodline
   * @param {string} bloodlineId - Bloodline ID to select
   * @returns {Object}
   */
  selectBloodline(bloodlineId) {
    const bloodline = this.getBloodlineById(bloodlineId);
    if (!bloodline) {
      return {
        success: false,
        message: 'Bloodline not found'
      };
    }

    this.selectedBloodline = bloodline;
    this.allocatedNodes = [];

    return {
      success: true,
      message: `${bloodline.name} selected`,
      bloodline: this.selectedBloodline
    };
  }

  /**
   * Allocate a notable node
   * @param {string} nodeName - Name of the notable to allocate
   * @returns {Object}
   */
  allocateNode(nodeName) {
    if (!this.selectedBloodline) {
      return {
        success: false,
        message: 'No bloodline selected'
      };
    }

    const node = this.selectedBloodline.notables.find(n => n.name === nodeName);
    if (!node) {
      return {
        success: false,
        message: 'Notable not found'
      };
    }

    if (this.allocatedNodes.includes(nodeName)) {
      return {
        success: false,
        message: 'Notable already allocated'
      };
    }

    this.allocatedNodes.push(nodeName);

    return {
      success: true,
      message: `${nodeName} allocated`,
      allocated: this.allocatedNodes
    };
  }

  /**
   * Deallocate a notable node
   * @param {string} nodeName - Name of the notable to deallocate
   * @returns {Object}
   */
  deallocateNode(nodeName) {
    const index = this.allocatedNodes.indexOf(nodeName);
    if (index === -1) {
      return {
        success: false,
        message: 'Notable not allocated'
      };
    }

    this.allocatedNodes.splice(index, 1);

    return {
      success: true,
      message: `${nodeName} deallocated`,
      allocated: this.allocatedNodes
    };
  }

  /**
   * Get currently selected bloodline
   * @returns {Object|null}
   */
  getSelectedBloodline() {
    return this.selectedBloodline;
  }

  /**
   * Get allocated nodes
   * @returns {Array}
   */
  getAllocatedNodes() {
    return this.allocatedNodes;
  }

  /**
   * Calculate synergy with main ascendancy
   * @param {string} bloodlineId - Bloodline ID
   * @param {string} mainAscendancy - Main ascendancy name
   * @returns {Object}
   */
  calculateSynergy(bloodlineId, mainAscendancy) {
    const bloodline = this.getBloodlineById(bloodlineId);
    if (!bloodline) {
      return {
        rating: 0,
        reason: 'Bloodline not found'
      };
    }

    const isBestMatch = bloodline.bestCombinations.includes(mainAscendancy);
    const matchIndex = bloodline.bestCombinations.indexOf(mainAscendancy);

    let rating = 0;
    let reason = '';

    if (matchIndex === 0) {
      rating = 5;
      reason = 'Excellent synergy - Top recommended combination';
    } else if (matchIndex === 1) {
      rating = 4;
      reason = 'Very good synergy - Highly recommended';
    } else if (matchIndex === 2) {
      rating = 3;
      reason = 'Good synergy - Solid combination';
    } else {
      rating = 2;
      reason = 'Moderate synergy - Can work but not optimal';
    }

    return {
      rating,
      reason,
      synergyBonus: bloodline.synergyBonus,
      compatible: isBestMatch
    };
  }

  /**
   * Get unlock requirements for a bloodline
   * @param {string} bloodlineId - Bloodline ID
   * @returns {Object|null}
   */
  getUnlockRequirements(bloodlineId) {
    const bloodline = this.getBloodlineById(bloodlineId);
    if (!bloodline) return null;

    return {
      name: bloodline.name,
      requirement: bloodline.unlockRequirement,
      tips: this.getUnlockTips(bloodline.unlockRequirement.boss)
    };
  }

  /**
   * Get tips for unlocking a bloodline
   * @param {string} boss - Boss name
   * @returns {string}
   */
  getUnlockTips(boss) {
    const tips = {
      'Xesht, Breachlord of the Void': 'Collect 100 Splinters of Xesht from Breach encounters. Focus on Xesht-specific breaches.',
      'The Primordial Warden': 'Complete the Genesis Tree storyline. Final boss requires high DPS and mobility.',
      'The Blood Sovereign': 'Located in the Crimson Cathedral map. Bring life flasks and chaos resistance.',
      'Chronos, the Time Keeper': 'Found in Temporal Sanctum. Be prepared for time-dilating mechanics.',
      'The Voidborn Matriarch': 'Reach Abyssal Depths Level 5. Requires clearing many Abyss encounters.',
      'Tempest Lord Zephyros': 'Access the Eye of the Storm through storm-themed maps. Need high lightning resistance.',
      'Ignis, the Eternal Flame': 'Enter the Infernal Crucible. Bring maximum fire resistance and anti-ignite mechanics.',
      'Boreas, the Frost King': 'Challenge the Frozen Citadel. Bring freeze immunity or high cold resistance.',
      'Mortis, the Death Architect': 'Complete Necropolis Prime. Prepare for waves of undead minions.',
      'Celestia, the Star Weaver': 'Unlock the Astral Observatory. Requires completing celestial challenges.'
    };

    return tips[boss] || 'No specific tips available. Prepare thoroughly before attempting.';
  }

  /**
   * Get recommended bloodlines for build type
   * @param {Object} buildConfig - Build configuration
   * @returns {Array}
   */
  recommendBloodlines(buildConfig) {
    const { damageType, ascendancy, playstyle } = buildConfig;

    let recommendations = [];

    // Recommend based on ascendancy first
    if (ascendancy) {
      recommendations.push(...this.getBestBloodlinesForAscendancy(ascendancy));
    }

    // Add theme-based recommendations
    if (damageType === 'chaos') {
      recommendations.push(...this.getBloodlinesByTheme('Chaos'));
      recommendations.push(...this.getBloodlinesByTheme('Void'));
    } else if (damageType === 'fire') {
      recommendations.push(...this.getBloodlinesByTheme('Fire'));
    } else if (damageType === 'cold') {
      recommendations.push(...this.getBloodlinesByTheme('Ice'));
    } else if (damageType === 'lightning') {
      recommendations.push(...this.getBloodlinesByTheme('Lightning'));
    }

    if (playstyle === 'minion') {
      recommendations.push(this.getBloodlineById('bloodline_necro_architect'));
    } else if (playstyle === 'fast') {
      recommendations.push(this.getBloodlineById('bloodline_temporal_weaver'));
    }

    // Remove duplicates and null values
    const uniqueRecommendations = [...new Set(recommendations)].filter(b => b !== null);
    return uniqueRecommendations.slice(0, 5);
  }

  /**
   * Reset bloodline selection
   */
  reset() {
    this.selectedBloodline = null;
    this.allocatedNodes = [];
  }

  /**
   * Get all allocated bonuses
   * @returns {Array}
   */
  getAllocatedBonuses() {
    if (!this.selectedBloodline) return [];

    const bonuses = [];
    this.allocatedNodes.forEach(nodeName => {
      const node = this.selectedBloodline.notables.find(n => n.name === nodeName);
      if (node) {
        bonuses.push({
          node: nodeName,
          bonuses: node.bonuses
        });
      }
    });

    return bonuses;
  }

  /**
   * Get statistics
   * @returns {Object}
   */
  getStatistics() {
    return {
      totalBloodlines: this.bloodlines.length,
      selected: this.selectedBloodline ? this.selectedBloodline.name : 'None',
      allocatedNodes: this.allocatedNodes.length,
      availableNodes: this.selectedBloodline ? this.selectedBloodline.notables.length : 0
    };
  }
}

// Export singleton instance
const bloodlineService = new BloodlineService();
module.exports = bloodlineService;
module.exports.BloodlineService = BloodlineService;
