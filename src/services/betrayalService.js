/**
 * Betrayal Service - Immortal Syndicate board tracking and reward optimization
 * Helps players optimize syndicate farming and maximize rewards
 */

class BetrayalService {
  constructor() {
    // Syndicate member database with rewards by rank
    this.syndicateMembers = {
      'It That Fled': {
        name: 'It That Fled',
        division: 'Research',
        bestReward: 'Research Captain/Leader',
        rewards: {
          transportation: {
            rank1: 'Breach Splinters',
            rank2: 'Breach Splinters',
            rank3: 'Breach Stones',
            value: 'Low'
          },
          fortification: {
            rank1: 'Breach Splinters',
            rank2: 'Breach Splinters',
            rank3: 'Breach Stones',
            value: 'Low'
          },
          research: {
            rank1: 'Upgrade Breachstone to Charged',
            rank2: 'Upgrade Breachstone to Enriched',
            rank3: 'Upgrade Breachstone to Pure',
            value: 'EXTREME - Best reward in game',
            notes: 'Pure Breachstones are 3-5 divine each!'
          },
          intervention: {
            rank1: 'Timeless Splinters',
            rank2: 'Timeless Splinters',
            rank3: 'Timeless Emblems',
            value: 'Medium'
          }
        },
        strategy: 'Always put in Research at Rank 3. Upgrade Breachstones for huge profit.',
        priority: 'S-Tier'
      },
      'Vorici': {
        name: 'Vorici',
        division: 'Research/Transportation',
        bestReward: 'Research or Transportation Rank 3',
        rewards: {
          transportation: {
            rank1: '1 White Socket',
            rank2: '2 White Sockets',
            rank3: '3 White Sockets',
            value: 'Very High',
            notes: 'White sockets on any item!'
          },
          fortification: {
            rank1: 'Jeweller\'s Orbs',
            rank2: 'Jeweller\'s Orbs',
            rank3: 'Jeweller\'s Orbs',
            value: 'Low'
          },
          research: {
            rank1: '1-3 White Sockets',
            rank2: '1-4 White Sockets',
            rank3: '1-6 White Sockets',
            value: 'EXTREME',
            notes: 'Can roll 1-6 white sockets on any item!'
          },
          intervention: {
            rank1: 'Chromatic Orbs',
            rank2: 'Chromatic Orbs',
            rank3: 'Chromatic Orbs',
            value: 'Low'
          }
        },
        strategy: 'Research Rank 3 for white sockets. Essential for off-color builds.',
        priority: 'S-Tier'
      },
      'Tora': {
        name: 'Tora',
        division: 'Research',
        bestReward: 'Research Rank 3',
        rewards: {
          transportation: {
            rank1: 'Gem XP',
            rank2: 'Gem XP',
            rank3: 'Gem XP',
            value: 'Low'
          },
          fortification: {
            rank1: 'Gem XP',
            rank2: 'Gem XP',
            rank3: 'Gem XP',
            value: 'Low'
          },
          research: {
            rank1: 'Gem Experience 10%',
            rank2: 'Gem Experience 20%',
            rank3: 'Gem Experience 40%',
            value: 'Medium',
            notes: 'Good for leveling alt quality gems'
          },
          intervention: {
            rank1: 'Gem XP',
            rank2: 'Gem XP',
            rank3: 'Gem XP',
            value: 'Low'
          }
        },
        strategy: 'Useful for gem leveling but not top priority',
        priority: 'B-Tier'
      },
      'Aisling': {
        name: 'Aisling',
        division: 'Research',
        bestReward: 'Research Rank 3',
        rewards: {
          transportation: {
            rank1: 'Add Veiled mod',
            rank2: 'Add Veiled mod',
            rank3: 'Add Veiled mod',
            value: 'Low'
          },
          fortification: {
            rank1: 'Add Veiled mod',
            rank2: 'Add Veiled mod',
            rank3: 'Add Veiled mod',
            value: 'Low'
          },
          research: {
            rank1: 'Add Veiled mod (1 prefix OR suffix)',
            rank2: 'Add Veiled mod (1 prefix OR suffix)',
            rank3: 'Remove crafted mod, add Veiled mod (2 mods - 1 prefix AND 1 suffix)',
            value: 'Very High',
            notes: 'Essential for metacrafting - adds 2 veiled mods!'
          },
          intervention: {
            rank1: 'Add Veiled mod',
            rank2: 'Add Veiled mod',
            rank3: 'Add Veiled mod',
            value: 'Low'
          }
        },
        strategy: 'Research Rank 3 only. Removes 1 crafted mod and adds 2 veiled mods.',
        priority: 'A-Tier'
      },
      'Gravicius': {
        name: 'Gravicius',
        division: 'Research',
        bestReward: 'Research Rank 3',
        rewards: {
          transportation: {
            rank1: 'Divination Cards',
            rank2: 'Divination Cards',
            rank3: 'Divination Cards',
            value: 'Low'
          },
          fortification: {
            rank1: 'Divination Cards',
            rank2: 'Divination Cards',
            rank3: 'Divination Cards',
            value: 'Low'
          },
          research: {
            rank1: 'Unveil random modifier',
            rank2: 'Unveil random modifier',
            rank3: 'Unveil all modifiers',
            value: 'Medium',
            notes: 'Unlock all veiled mods at once'
          },
          intervention: {
            rank1: 'Divination Cards',
            rank2: 'Divination Cards',
            rank3: 'Divination Cards',
            value: 'Low'
          }
        },
        strategy: 'Research Rank 3 to unlock all crafts if you need them',
        priority: 'C-Tier'
      },
      'Leo': {
        name: 'Leo',
        division: 'Research',
        bestReward: 'Research Rank 3',
        rewards: {
          transportation: {
            rank1: 'Exalted Shard',
            rank2: 'Exalted Orb',
            rank3: 'Exalted Orb',
            value: 'Low'
          },
          fortification: {
            rank1: 'Exalted Shard',
            rank2: 'Exalted Orb',
            rank3: 'Exalted Orb',
            value: 'Low'
          },
          research: {
            rank1: 'Exalted Orb',
            rank2: '2 Exalted Orbs',
            rank3: 'Slam with Exalted Orb',
            value: 'Medium',
            notes: 'Free exalt slam'
          },
          intervention: {
            rank1: 'Currency',
            rank2: 'Currency',
            rank3: 'Currency',
            value: 'Low'
          }
        },
        strategy: 'Research Rank 3 for free exalt slam, but not priority',
        priority: 'C-Tier'
      },
      'Hillock': {
        name: 'Hillock',
        division: 'Transportation/Fortification',
        bestReward: 'Transportation/Fortification Rank 3',
        rewards: {
          transportation: {
            rank1: '22% Quality on Weapon',
            rank2: '26% Quality on Weapon',
            rank3: '28% Quality on Weapon',
            value: 'Very High',
            notes: '28% quality weapons!'
          },
          fortification: {
            rank1: '22% Quality on Armour',
            rank2: '26% Quality on Armour',
            rank3: '30% Quality on Armour',
            value: 'Very High',
            notes: '30% quality armour - best for ES gear!'
          },
          research: {
            rank1: 'Blacksmith Whetstones',
            rank2: 'Blacksmith Whetstones',
            rank3: 'Blacksmith Whetstones',
            value: 'Low'
          },
          intervention: {
            rank1: 'Armour Scraps',
            rank2: 'Armour Scraps',
            rank3: 'Armour Scraps',
            value: 'Low'
          }
        },
        strategy: 'Fortification Rank 3 for 30% quality armour. Essential for CI builds.',
        priority: 'A-Tier'
      },
      'Vagan': {
        name: 'Vagan',
        division: 'Intervention',
        bestReward: 'Intervention Rank 3',
        rewards: {
          transportation: {
            rank1: 'Legion Splinters',
            rank2: 'Legion Splinters',
            rank3: 'Legion Splinters',
            value: 'Low'
          },
          fortification: {
            rank1: 'Legion Splinters',
            rank2: 'Legion Splinters',
            rank3: 'Legion Splinters',
            value: 'Low'
          },
          research: {
            rank1: 'Legion Scarabs',
            rank2: 'Legion Scarabs',
            rank3: 'Legion Scarabs',
            value: 'Medium'
          },
          intervention: {
            rank1: '1 Legion Scarab',
            rank2: '2 Legion Scarabs',
            rank3: '3 Legion Scarabs',
            value: 'High',
            notes: 'Scarabs for Legion farming'
          }
        },
        strategy: 'Intervention Rank 3 for Legion scarabs',
        priority: 'B-Tier'
      },
      'Cameria': {
        name: 'Cameria',
        division: 'Intervention',
        bestReward: 'Intervention Rank 3',
        rewards: {
          transportation: {
            rank1: 'Harbinger Currency',
            rank2: 'Harbinger Currency',
            rank3: 'Harbinger Currency',
            value: 'Low'
          },
          fortification: {
            rank1: 'Harbinger Currency',
            rank2: 'Harbinger Currency',
            rank3: 'Harbinger Currency',
            value: 'Low'
          },
          research: {
            rank1: 'Harbinger Currency',
            rank2: 'Harbinger Currency',
            rank3: 'Harbinger Currency',
            value: 'Low'
          },
          intervention: {
            rank1: '1 Harbinger Scarab',
            rank2: '2 Harbinger Scarabs',
            rank3: '3 Harbinger Scarabs',
            value: 'High',
            notes: 'Very profitable for Harbinger farming'
          }
        },
        strategy: 'Intervention Rank 3 for Harbinger scarabs',
        priority: 'A-Tier'
      },
      'Elreon': {
        name: 'Elreon',
        division: 'Intervention',
        bestReward: 'Intervention Rank 3',
        rewards: {
          transportation: {
            rank1: 'Relic Keys',
            rank2: 'Relic Keys',
            rank3: 'Relic Keys',
            value: 'Low'
          },
          fortification: {
            rank1: 'Relic Keys',
            rank2: 'Relic Keys',
            rank3: 'Relic Keys',
            value: 'Low'
          },
          research: {
            rank1: 'Relic Keys',
            rank2: 'Relic Keys',
            rank3: 'Relic Keys',
            value: 'Low'
          },
          intervention: {
            rank1: '1 Reliquary Scarab',
            rank2: '2 Reliquary Scarabs',
            rank3: '3 Reliquary Scarabs',
            value: 'Medium'
          }
        },
        strategy: 'Not high priority',
        priority: 'C-Tier'
      },
      'Haku': {
        name: 'Haku',
        division: 'Intervention',
        bestReward: 'Intervention Rank 3',
        rewards: {
          transportation: {
            rank1: 'Strongbox Currency',
            rank2: 'Strongbox Currency',
            rank3: 'Strongbox Currency',
            value: 'Low'
          },
          fortification: {
            rank1: 'Strongbox Currency',
            rank2: 'Strongbox Currency',
            rank3: 'Strongbox Currency',
            value: 'Low'
          },
          research: {
            rank1: 'Strongbox Scarabs',
            rank2: 'Strongbox Scarabs',
            rank3: 'Strongbox Scarabs',
            value: 'Low'
          },
          intervention: {
            rank1: '1 Ambush Scarab',
            rank2: '2 Ambush Scarabs',
            rank3: '3 Ambush Scarabs',
            value: 'Medium'
          }
        },
        strategy: 'Not high priority',
        priority: 'C-Tier'
      },
      'Janus': {
        name: 'Janus Perandus',
        division: 'Intervention',
        bestReward: 'Intervention Rank 3',
        rewards: {
          transportation: {
            rank1: 'Perandus Coins',
            rank2: 'Perandus Coins',
            rank3: 'Perandus Coins',
            value: 'Low'
          },
          fortification: {
            rank1: 'Perandus Coins',
            rank2: 'Perandus Coins',
            rank3: 'Perandus Coins',
            value: 'Low'
          },
          research: {
            rank1: 'Perandus Coins',
            rank2: 'Perandus Coins',
            rank3: 'Perandus Coins',
            value: 'Low'
          },
          intervention: {
            rank1: '1 Perandus Scarab',
            rank2: '2 Perandus Scarabs',
            rank3: '3 Perandus Scarabs',
            value: 'Low'
          }
        },
        strategy: 'Remove from board',
        priority: 'D-Tier'
      }
    };

    // Board management strategies
    this.strategies = {
      'Profit Farming': {
        research: ['It That Fled', 'Vorici', 'Aisling'],
        transportation: ['Hillock'],
        fortification: ['Hillock'],
        intervention: ['Cameria', 'Vagan'],
        notes: 'Focus on most profitable rewards',
        expectedProfit: '5-10 divine per full board'
      },
      'Crafting Focus': {
        research: ['Vorici', 'Aisling', 'It That Fled'],
        transportation: ['Hillock'],
        fortification: ['Hillock'],
        intervention: ['Cameria'],
        notes: 'Best for crafters',
        expectedProfit: 'Variable - depends on crafts'
      },
      'Scarab Farming': {
        research: ['It That Fled'],
        transportation: [],
        fortification: [],
        intervention: ['Cameria', 'Vagan', 'Haku', 'Elreon'],
        notes: 'Farm scarabs for mapping',
        expectedProfit: '3-7 divine per board'
      }
    };

    // Betrayal mechanics cheatsheet
    this.mechanics = {
      'Execute': 'Kill member - drops items, removes from board',
      'Bargain': 'Member reveals info about other members, stays on board',
      'Interrogate': 'Remove member from board, gain intelligence',
      'Betray': 'Member betrays their division, swaps divisions',
      'Trusted': 'Green line - members will help each other',
      'Rival': 'Red line - members will fight each other',
      'Rank Up': 'Execute/Interrogate a rival to rank up',
      'Rank Down': 'Interrogate to rank down one star',
      'Intelligence': 'Fill bar to run safehouse'
    };
  }

  /**
   * Get member data by name
   * @param {string} memberName - Name of the syndicate member
   * @returns {Object|null}
   */
  getMember(memberName) {
    return this.syndicateMembers[memberName] || null;
  }

  /**
   * Get best rewards for a division
   * @param {string} division - Division name
   * @returns {Array}
   */
  getBestRewardsForDivision(division) {
    const members = [];

    Object.entries(this.syndicateMembers).forEach(([name, data]) => {
      const divisionData = data.rewards[division.toLowerCase()];
      if (divisionData && divisionData.value !== 'Low') {
        members.push({
          name,
          rank3Reward: divisionData.rank3,
          value: divisionData.value,
          notes: divisionData.notes || '',
          priority: data.priority
        });
      }
    });

    // Sort by priority
    const priorityOrder = { 'S-Tier': 0, 'A-Tier': 1, 'B-Tier': 2, 'C-Tier': 3, 'D-Tier': 4 };
    members.sort((a, b) => priorityOrder[a.priority] - priorityOrder[b.priority]);

    return members;
  }

  /**
   * Get optimal board setup for strategy
   * @param {string} strategyName - Strategy name
   * @returns {Object|null}
   */
  getStrategy(strategyName) {
    return this.strategies[strategyName] || null;
  }

  /**
   * Recommend action for encounter
   * @param {Object} encounter - Current encounter state
   * @returns {Object}
   */
  recommendAction(encounter) {
    const { member, currentDivision, currentRank, targetDivision, targetRank } = encounter;

    const memberData = this.getMember(member);
    if (!memberData) {
      return { action: 'Execute', reason: 'Unknown member' };
    }

    // Check if member is in correct division
    if (currentDivision !== targetDivision) {
      return {
        action: 'Bargain or Betray',
        reason: `Move ${member} from ${currentDivision} to ${targetDivision}`
      };
    }

    // Check rank
    if (currentRank < targetRank) {
      return {
        action: 'Execute or Rank Up',
        reason: `${member} needs to be rank ${targetRank}, currently rank ${currentRank}`
      };
    }

    if (currentRank === targetRank) {
      return {
        action: 'Keep on board',
        reason: `${member} is in position - don't run safehouse yet`
      };
    }

    return {
      action: 'Interrogate',
      reason: 'Member rank too high, interrogate to reduce'
    };
  }

  /**
   * Get all mechanics explanations
   * @returns {Object}
   */
  getMechanics() {
    return this.mechanics;
  }

  /**
   * Calculate expected profit for board state
   * @param {Object} boardState - Current board configuration
   * @returns {Object}
   */
  calculateProfit(boardState) {
    let totalValue = 0;
    const breakdown = {};

    Object.entries(boardState).forEach(([division, members]) => {
      members.forEach(member => {
        const memberData = this.getMember(member.name);
        if (memberData) {
          const reward = memberData.rewards[division.toLowerCase()];
          if (reward) {
            const value = this.getRewardValue(reward.value);
            totalValue += value * member.rank;

            if (!breakdown[division]) breakdown[division] = [];
            breakdown[division].push({
              member: member.name,
              rank: member.rank,
              value: value * member.rank,
              reward: reward[`rank${member.rank}`]
            });
          }
        }
      });
    });

    return {
      totalValue: `${totalValue}-${totalValue * 1.5} divine`,
      breakdown
    };
  }

  /**
   * Convert reward value text to numeric
   * @param {string} valueText - Value tier
   * @returns {number}
   */
  getRewardValue(valueText) {
    const values = {
      'EXTREME': 5,
      'Very High': 2,
      'High': 1,
      'Medium': 0.5,
      'Low': 0.1
    };
    return values[valueText] || 0;
  }

  /**
   * Get quick reference for all members
   * @returns {Array}
   */
  getQuickReference() {
    return Object.entries(this.syndicateMembers).map(([name, data]) => ({
      name,
      bestDivision: data.bestReward,
      priority: data.priority,
      strategy: data.strategy
    })).sort((a, b) => {
      const order = { 'S-Tier': 0, 'A-Tier': 1, 'B-Tier': 2, 'C-Tier': 3, 'D-Tier': 4 };
      return order[a.priority] - order[b.priority];
    });
  }
}

// Export singleton instance
const betrayalService = new BetrayalService();
module.exports = betrayalService;
module.exports.BetrayalService = BetrayalService;
