/**
 * Genesis Tree Service - POE 3.27 Keepers of the Flame League
 * Manages Genesis Tree passive skills, Graftblood currency, and rewards
 */

class GenesisTreeService {
  constructor() {
    // Genesis Tree passive skill nodes
    this.passiveNodes = {
      // Core Nodes
      core1: {
        id: 'core1',
        name: 'Breach Foundation',
        description: '+10% increased Breach Monster pack size',
        type: 'core',
        cost: 5,
        tier: 1,
        unlocked: false,
        prerequisites: []
      },
      core2: {
        id: 'core2',
        name: 'Graftblood Sustenance',
        description: '+15% increased Graftblood drop rate',
        type: 'core',
        cost: 8,
        tier: 1,
        unlocked: false,
        prerequisites: []
      },

      // Breach Enhancement Branch
      breach1: {
        id: 'breach1',
        name: 'Extended Breach',
        description: 'Breaches last 3 seconds longer',
        type: 'breach',
        cost: 10,
        tier: 2,
        unlocked: false,
        prerequisites: ['core1']
      },
      breach2: {
        id: 'breach2',
        name: 'Breach Density',
        description: 'Breach monsters spawn 25% faster',
        type: 'breach',
        cost: 15,
        tier: 2,
        unlocked: false,
        prerequisites: ['breach1']
      },
      breach3: {
        id: 'breach3',
        name: 'Splinter Fortune',
        description: '+20% increased Breach Splinter drop rate',
        type: 'breach',
        cost: 20,
        tier: 3,
        unlocked: false,
        prerequisites: ['breach2']
      },
      breach4: {
        id: 'breach4',
        name: 'Unstable Mastery',
        description: 'Unstable Breaches appear 50% more often',
        type: 'breach',
        cost: 30,
        tier: 3,
        unlocked: false,
        prerequisites: ['breach3']
      },

      // Hive Exploration Branch
      hive1: {
        id: 'hive1',
        name: 'Hive Seeker',
        description: 'Breach Hives appear 20% more frequently',
        type: 'hive',
        cost: 12,
        tier: 2,
        unlocked: false,
        prerequisites: ['core1']
      },
      hive2: {
        id: 'hive2',
        name: 'Hiveborn Bounty',
        description: 'Hiveborn bosses drop 1 additional reward',
        type: 'hive',
        cost: 18,
        tier: 2,
        unlocked: false,
        prerequisites: ['hive1']
      },
      hive3: {
        id: 'hive3',
        name: 'Hive Mastery',
        description: 'Completing Hives grants 50% more Graftblood',
        type: 'hive',
        cost: 25,
        tier: 3,
        unlocked: false,
        prerequisites: ['hive2']
      },

      // Ailith Protection Branch
      ailith1: {
        id: 'ailith1',
        name: 'Ailith\'s Favor',
        description: 'Ailith protection duration increased by 30%',
        type: 'ailith',
        cost: 10,
        tier: 2,
        unlocked: false,
        prerequisites: ['core2']
      },
      ailith2: {
        id: 'ailith2',
        name: 'Divine Shield',
        description: '+5% to all maximum Resistances during Ailith protection',
        type: 'ailith',
        cost: 20,
        tier: 3,
        unlocked: false,
        prerequisites: ['ailith1']
      },
      ailith3: {
        id: 'ailith3',
        name: 'Eternal Guardian',
        description: 'Ailith protection also grants immunity to Elemental Ailments',
        type: 'ailith',
        cost: 35,
        tier: 4,
        unlocked: false,
        prerequisites: ['ailith2']
      },

      // Reward Enhancement Branch
      reward1: {
        id: 'reward1',
        name: 'Greater Spoils',
        description: '+10% increased quantity of items from Breach monsters',
        type: 'reward',
        cost: 15,
        tier: 2,
        unlocked: false,
        prerequisites: ['core2']
      },
      reward2: {
        id: 'reward2',
        name: 'Blessed Drops',
        description: '5% chance for Breachstones to drop as Blessed',
        type: 'reward',
        cost: 25,
        tier: 3,
        unlocked: false,
        prerequisites: ['reward1']
      },
      reward3: {
        id: 'reward3',
        name: 'Chaos Affinity',
        description: 'Chayula Breaches appear 100% more often',
        type: 'reward',
        cost: 40,
        tier: 4,
        unlocked: false,
        prerequisites: ['reward2']
      },

      // Unique Branch - Final Nodes
      unique1: {
        id: 'unique1',
        name: 'Genesis Awakening',
        description: 'Unlock Genesis unique item crafting',
        type: 'unique',
        cost: 50,
        tier: 4,
        unlocked: false,
        prerequisites: ['breach4', 'hive3']
      },
      unique2: {
        id: 'unique2',
        name: 'Flame Keeper\'s Legacy',
        description: 'League-specific unique items drop 50% more often',
        type: 'unique',
        cost: 50,
        tier: 4,
        unlocked: false,
        prerequisites: ['ailith3', 'reward3']
      }
    };

    // Genesis unique items that can drop
    this.genesisUniques = [
      {
        name: 'Ailith\'s Grace',
        type: 'Amulet',
        description: 'Grants Ailith\'s protection permanently with reduced effect',
        rarity: 'Very Rare'
      },
      {
        name: 'Hiveborn Crown',
        type: 'Helmet',
        description: 'Summons a spectral Hiveborn to fight alongside you',
        rarity: 'Rare'
      },
      {
        name: 'Breach Keeper\'s Embrace',
        type: 'Gloves',
        description: 'Your attacks have a chance to open mini-breaches',
        rarity: 'Rare'
      },
      {
        name: 'Graftblood Reservoir',
        type: 'Belt',
        description: 'Convert Graftblood into life and mana recovery',
        rarity: 'Uncommon'
      },
      {
        name: 'Splinter Storm',
        type: 'Wand',
        description: 'Attacks fire additional projectiles per splinter type collected',
        rarity: 'Very Rare'
      }
    ];

    // Available crafts unlocked via Genesis Tree
    this.genesisCrafts = {
      basic: {
        name: 'Graftblood Infusion',
        description: 'Add Breach modifiers to items',
        cost: 20,
        unlocked: false
      },
      advanced: {
        name: 'Hiveborn Essence',
        description: 'Guarantee Breach unique from boss',
        cost: 50,
        unlocked: false
      },
      master: {
        name: 'Genesis Transcendence',
        description: 'Upgrade Breachstone to Flawless',
        cost: 100,
        unlocked: false
      }
    };

    // Track player progress
    this.playerProgress = {
      graftbloodBalance: 0,
      totalGraftbloodEarned: 0,
      totalGraftbloodSpent: 0,
      unlockedNodes: [],
      completedTiers: [],
      availableCrafts: [],
      uniquesFound: [],
      rewardTiersUnlocked: 0
    };

    // Reward tiers based on nodes unlocked
    this.rewardTiers = [
      {
        tier: 1,
        nodesRequired: 2,
        rewards: ['Basic Graftblood drops increase', 'Access to Hive tracking']
      },
      {
        tier: 2,
        nodesRequired: 5,
        rewards: ['Unlock Genesis crafts', 'Increased splinter rates']
      },
      {
        tier: 3,
        nodesRequired: 10,
        rewards: ['Advanced crafting options', 'Breach boss map device options']
      },
      {
        tier: 4,
        nodesRequired: 15,
        rewards: ['Genesis unique item drops', 'Maximum Genesis power']
      }
    ];
  }

  /**
   * Get current Graftblood balance
   */
  getGraftbloodBalance() {
    return this.playerProgress.graftbloodBalance;
  }

  /**
   * Add Graftblood currency
   * @param {number} amount
   */
  addGraftblood(amount) {
    this.playerProgress.graftbloodBalance += amount;
    this.playerProgress.totalGraftbloodEarned += amount;
    return this.playerProgress.graftbloodBalance;
  }

  /**
   * Spend Graftblood (for unlocking nodes or crafts)
   * @param {number} amount
   */
  spendGraftblood(amount) {
    if (this.playerProgress.graftbloodBalance >= amount) {
      this.playerProgress.graftbloodBalance -= amount;
      this.playerProgress.totalGraftbloodSpent += amount;
      return true;
    }
    return false;
  }

  /**
   * Check if a node can be unlocked
   * @param {string} nodeId
   */
  canUnlockNode(nodeId) {
    const node = this.passiveNodes[nodeId];

    if (!node) {
      return { canUnlock: false, reason: 'Node not found' };
    }

    if (node.unlocked) {
      return { canUnlock: false, reason: 'Already unlocked' };
    }

    // Check prerequisites
    for (const prereqId of node.prerequisites) {
      if (!this.passiveNodes[prereqId].unlocked) {
        return {
          canUnlock: false,
          reason: `Requires ${this.passiveNodes[prereqId].name}`
        };
      }
    }

    // Check Graftblood cost
    if (this.playerProgress.graftbloodBalance < node.cost) {
      return {
        canUnlock: false,
        reason: `Insufficient Graftblood (need ${node.cost}, have ${this.playerProgress.graftbloodBalance})`
      };
    }

    return { canUnlock: true };
  }

  /**
   * Unlock a passive node
   * @param {string} nodeId
   */
  unlockNode(nodeId) {
    const canUnlock = this.canUnlockNode(nodeId);

    if (!canUnlock.canUnlock) {
      console.error(`Cannot unlock node: ${canUnlock.reason}`);
      return { success: false, reason: canUnlock.reason };
    }

    const node = this.passiveNodes[nodeId];

    // Spend Graftblood
    this.spendGraftblood(node.cost);

    // Unlock the node
    node.unlocked = true;
    this.playerProgress.unlockedNodes.push(nodeId);

    // Check if tier is completed
    this.checkTierCompletion(node.tier);

    // Check for craft unlocks
    this.checkCraftUnlocks();

    console.log(`Unlocked: ${node.name} for ${node.cost} Graftblood`);

    return {
      success: true,
      node: node,
      remainingGraftblood: this.playerProgress.graftbloodBalance
    };
  }

  /**
   * Check if a tier is completed
   * @param {number} tier
   */
  checkTierCompletion(tier) {
    if (this.playerProgress.completedTiers.includes(tier)) {
      return false;
    }

    const tierNodes = Object.values(this.passiveNodes).filter(n => n.tier === tier);
    const unlockedTierNodes = tierNodes.filter(n => n.unlocked);

    if (unlockedTierNodes.length === tierNodes.length) {
      this.playerProgress.completedTiers.push(tier);
      console.log(`Tier ${tier} completed!`);
      return true;
    }

    return false;
  }

  /**
   * Check and unlock available crafts based on nodes
   */
  checkCraftUnlocks() {
    const unlockedCount = this.playerProgress.unlockedNodes.length;

    // Basic craft at 5 nodes
    if (unlockedCount >= 5 && !this.genesisCrafts.basic.unlocked) {
      this.genesisCrafts.basic.unlocked = true;
      this.playerProgress.availableCrafts.push('basic');
      console.log('Unlocked: Graftblood Infusion craft');
    }

    // Advanced craft at 10 nodes
    if (unlockedCount >= 10 && !this.genesisCrafts.advanced.unlocked) {
      this.genesisCrafts.advanced.unlocked = true;
      this.playerProgress.availableCrafts.push('advanced');
      console.log('Unlocked: Hiveborn Essence craft');
    }

    // Master craft at 15 nodes
    if (unlockedCount >= 15 && !this.genesisCrafts.master.unlocked) {
      this.genesisCrafts.master.unlocked = true;
      this.playerProgress.availableCrafts.push('master');
      console.log('Unlocked: Genesis Transcendence craft');
    }
  }

  /**
   * Get all passive nodes with their status
   */
  getAllNodes() {
    return Object.values(this.passiveNodes).map(node => ({
      ...node,
      canUnlock: this.canUnlockNode(node.id)
    }));
  }

  /**
   * Get nodes by type
   * @param {string} type
   */
  getNodesByType(type) {
    return Object.values(this.passiveNodes)
      .filter(node => node.type === type)
      .map(node => ({
        ...node,
        canUnlock: this.canUnlockNode(node.id)
      }));
  }

  /**
   * Get available nodes (prerequisites met, not yet unlocked)
   */
  getAvailableNodes() {
    return Object.values(this.passiveNodes)
      .filter(node => {
        if (node.unlocked) return false;
        const check = this.canUnlockNode(node.id);
        return check.canUnlock || check.reason.includes('Insufficient Graftblood');
      })
      .map(node => ({
        ...node,
        canUnlock: this.canUnlockNode(node.id)
      }));
  }

  /**
   * Get player progress summary
   */
  getProgressSummary() {
    const totalNodes = Object.keys(this.passiveNodes).length;
    const unlockedNodes = this.playerProgress.unlockedNodes.length;
    const progressPercent = Math.round((unlockedNodes / totalNodes) * 100);

    // Calculate current reward tier
    let currentTier = 0;
    for (const tier of this.rewardTiers) {
      if (unlockedNodes >= tier.nodesRequired) {
        currentTier = tier.tier;
      }
    }

    return {
      graftbloodBalance: this.playerProgress.graftbloodBalance,
      totalEarned: this.playerProgress.totalGraftbloodEarned,
      totalSpent: this.playerProgress.totalGraftbloodSpent,
      nodesUnlocked: unlockedNodes,
      totalNodes,
      progressPercent,
      currentTier,
      tiersCompleted: this.playerProgress.completedTiers,
      availableCrafts: this.playerProgress.availableCrafts,
      uniquesFound: this.playerProgress.uniquesFound.length
    };
  }

  /**
   * Get recommended passive path suggestions
   */
  getRecommendedPaths() {
    return {
      breachFarmer: {
        name: 'Breach Farmer',
        description: 'Maximize breach encounters and splinters',
        path: ['core1', 'breach1', 'breach2', 'breach3', 'breach4'],
        benefits: 'Best for farming splinters and breachstones'
      },
      hiveExplorer: {
        name: 'Hive Explorer',
        description: 'Focus on Breach Hive completion',
        path: ['core1', 'hive1', 'hive2', 'hive3'],
        benefits: 'Maximize Graftblood from Hives'
      },
      ailithDefender: {
        name: 'Ailith Defender',
        description: 'Maximize protection benefits',
        path: ['core2', 'ailith1', 'ailith2', 'ailith3'],
        benefits: 'Best for survivability in difficult content'
      },
      rewardMaximizer: {
        name: 'Reward Maximizer',
        description: 'Focus on item drops and valuable rewards',
        path: ['core2', 'reward1', 'reward2', 'reward3'],
        benefits: 'Best for finding valuable items'
      },
      balanced: {
        name: 'Balanced Build',
        description: 'Well-rounded progression',
        path: ['core1', 'core2', 'breach1', 'hive1', 'reward1', 'ailith1'],
        benefits: 'Versatile approach for all content'
      }
    };
  }

  /**
   * Get reward tier information
   * @param {number} tier
   */
  getRewardTier(tier) {
    return this.rewardTiers[tier - 1] || null;
  }

  /**
   * Get all unlockable crafts
   */
  getAvailableCrafts() {
    return Object.entries(this.genesisCrafts)
      .filter(([_, craft]) => craft.unlocked)
      .map(([key, craft]) => ({
        id: key,
        ...craft
      }));
  }

  /**
   * Get Genesis unique items
   */
  getGenesisUniques() {
    return this.genesisUniques.map(unique => ({
      ...unique,
      found: this.playerProgress.uniquesFound.includes(unique.name)
    }));
  }

  /**
   * Record finding a Genesis unique
   * @param {string} uniqueName
   */
  foundUnique(uniqueName) {
    if (!this.playerProgress.uniquesFound.includes(uniqueName)) {
      this.playerProgress.uniquesFound.push(uniqueName);
      console.log(`Found Genesis unique: ${uniqueName}`);
      return true;
    }
    return false;
  }

  /**
   * Calculate Graftblood needed for next milestone
   */
  getNextMilestone() {
    const availableNodes = this.getAvailableNodes();
    if (availableNodes.length === 0) {
      return { completed: true, message: 'All nodes unlocked!' };
    }

    // Find cheapest available node
    const cheapestNode = availableNodes.reduce((min, node) =>
      node.cost < min.cost ? node : min
    );

    const needed = cheapestNode.cost - this.playerProgress.graftbloodBalance;

    return {
      completed: false,
      nextNode: cheapestNode.name,
      cost: cheapestNode.cost,
      graftbloodNeeded: Math.max(0, needed),
      canAfford: needed <= 0
    };
  }

  /**
   * Reset all progress (for testing or new league)
   */
  resetProgress() {
    // Reset all nodes
    Object.values(this.passiveNodes).forEach(node => {
      node.unlocked = false;
    });

    // Reset crafts
    Object.values(this.genesisCrafts).forEach(craft => {
      craft.unlocked = false;
    });

    // Reset player progress
    this.playerProgress = {
      graftbloodBalance: 0,
      totalGraftbloodEarned: 0,
      totalGraftbloodSpent: 0,
      unlockedNodes: [],
      completedTiers: [],
      availableCrafts: [],
      uniquesFound: [],
      rewardTiersUnlocked: 0
    };

    console.log('Genesis Tree progress reset');
    return this.playerProgress;
  }

  /**
   * Export progress data (for saving)
   */
  exportProgress() {
    return {
      nodes: Object.entries(this.passiveNodes).reduce((acc, [id, node]) => {
        acc[id] = node.unlocked;
        return acc;
      }, {}),
      progress: { ...this.playerProgress }
    };
  }

  /**
   * Import progress data (for loading)
   * @param {Object} data
   */
  importProgress(data) {
    if (data.nodes) {
      Object.entries(data.nodes).forEach(([id, unlocked]) => {
        if (this.passiveNodes[id]) {
          this.passiveNodes[id].unlocked = unlocked;
        }
      });
    }

    if (data.progress) {
      this.playerProgress = { ...data.progress };
    }

    console.log('Genesis Tree progress imported');
    return this.playerProgress;
  }
}

// Export singleton instance
const genesisTreeService = new GenesisTreeService();
module.exports = genesisTreeService;
module.exports.GenesisTreeService = GenesisTreeService;
