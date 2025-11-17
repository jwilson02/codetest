/**
 * Build Guide Service - Fetches and parses build guides from Maxroll.gg
 */

import axios from 'axios';
import Build from '../../models/Build';
import { rateLimiter } from '../utils/rateLimiter';
import { cacheService } from '../cache/cacheService';

const MAXROLL_BASE_URL = 'https://maxroll.gg/poe';
const CACHE_TTL = 3600; // 1 hour cache

class BuildGuideService {
  constructor() {
    this.rateLimiter = rateLimiter;
  }

  /**
   * Fetch all available build guides
   * @param {Object} options - Fetch options
   * @returns {Promise<Array<Build>>}
   */
  async fetchAllBuilds(options = {}) {
    const cacheKey = 'all_builds';

    // Check cache first
    const cached = cacheService.get(cacheKey);
    if (cached && !options.forceRefresh) {
      console.log('Returning cached build list');
      return cached.map(b => new Build(b));
    }

    try {
      // Rate limit the request
      await this.rateLimiter.consume('maxroll', 1);

      // In a real implementation, this would scrape the Maxroll build guide page
      // For now, we'll return mock data structure
      const response = await this.fetchBuildListPage();
      const builds = this.parseBuildList(response);

      // Cache the results
      cacheService.set(cacheKey, builds.map(b => b.toJSON()), CACHE_TTL);

      return builds;
    } catch (error) {
      console.error('Error fetching build guides:', error);

      // Return cached data if available, even if expired
      const staleCache = cacheService.get(cacheKey, true);
      if (staleCache) {
        console.log('Returning stale cache due to error');
        return staleCache.map(b => new Build(b));
      }

      throw new Error(`Failed to fetch build guides: ${error.message}`);
    }
  }

  /**
   * Fetch a specific build guide by ID or URL
   * @param {string} buildId - Build identifier or URL
   * @returns {Promise<Build>}
   */
  async fetchBuildGuide(buildId) {
    const cacheKey = `build_${buildId}`;

    // Check cache first
    const cached = cacheService.get(cacheKey);
    if (cached) {
      console.log(`Returning cached build: ${buildId}`);
      return new Build(cached);
    }

    try {
      // Rate limit the request
      await this.rateLimiter.consume('maxroll', 1);

      const buildUrl = this.getBuildUrl(buildId);
      const response = await this.fetchBuildPage(buildUrl);
      const build = this.parseBuildDetail(response, buildId);

      // Cache the result
      cacheService.set(cacheKey, build.toJSON(), CACHE_TTL);

      return build;
    } catch (error) {
      console.error(`Error fetching build ${buildId}:`, error);

      // Return stale cache if available
      const staleCache = cacheService.get(cacheKey, true);
      if (staleCache) {
        console.log('Returning stale cache due to error');
        return new Build(staleCache);
      }

      throw new Error(`Failed to fetch build guide: ${error.message}`);
    }
  }

  /**
   * Fetch build list page (to be implemented with actual scraping)
   * @private
   */
  async fetchBuildListPage() {
    // In a real implementation, this would use axios + cheerio to scrape
    // the Maxroll build guide list page

    // Mock implementation returning sample data structure
    return this.getMockBuildList();
  }

  /**
   * Fetch individual build page (to be implemented with actual scraping)
   * @private
   */
  async fetchBuildPage(url) {
    // In a real implementation, this would scrape the build detail page
    // using axios + cheerio

    // For now, return mock data
    return this.getMockBuildDetail();
  }

  /**
   * Parse build list HTML into Build objects
   * @private
   */
  parseBuildList(html) {
    // In real implementation, this would parse HTML with cheerio
    // For now, return mock builds
    return this.getMockBuildList();
  }

  /**
   * Parse build detail HTML into a Build object
   * @private
   */
  parseBuildDetail(html, buildId) {
    // In real implementation, this would parse the build detail page
    // extracting gem links, items, passive tree, etc.

    return this.getMockBuildDetail(buildId);
  }

  /**
   * Get build URL from ID
   * @private
   */
  getBuildUrl(buildId) {
    if (buildId.startsWith('http')) {
      return buildId;
    }
    return `${MAXROLL_BASE_URL}/build-guides/${buildId}`;
  }

  /**
   * Mock data for development - Replace with actual scraping
   * @private
   */
  getMockBuildList() {
    return [
      new Build({
        id: 'poison-spark-pathfinder',
        title: 'Poison Spark Pathfinder',
        className: 'Ranger',
        ascendancy: 'Pathfinder',
        mainSkill: 'Spark',
        league: 'Affliction',
        patch: '3.23',
        difficulty: 'Medium',
        budget: 'Medium',
        rating: 4.5,
        popularity: 85,
        description: 'A fast-mapping build that uses Poison Spark for great clear speed.',
        url: 'https://maxroll.gg/poe/build-guides/poison-spark-pathfinder',
        pros: ['Excellent clear speed', 'Good single target', 'Zoom zoom'],
        cons: ['Squishy', 'Expensive to min-max']
      }),
      new Build({
        id: 'righteous-fire-juggernaut',
        title: 'Righteous Fire Juggernaut',
        className: 'Marauder',
        ascendancy: 'Juggernaut',
        mainSkill: 'Righteous Fire',
        league: 'Affliction',
        patch: '3.23',
        difficulty: 'Easy',
        budget: 'Low',
        rating: 4.8,
        popularity: 92,
        description: 'A tanky RF build perfect for beginners and HC players.',
        url: 'https://maxroll.gg/poe/build-guides/righteous-fire-juggernaut',
        pros: ['Very tanky', 'Great for beginners', 'Budget friendly'],
        cons: ['Slow clear', 'Lower DPS ceiling']
      }),
      new Build({
        id: 'summon-raging-spirits-necromancer',
        title: 'SRS Necromancer',
        className: 'Witch',
        ascendancy: 'Necromancer',
        mainSkill: 'Summon Raging Spirit',
        league: 'Affliction',
        patch: '3.23',
        difficulty: 'Easy',
        budget: 'Low',
        rating: 4.3,
        popularity: 78,
        description: 'Classic minion build with great scaling potential.',
        url: 'https://maxroll.gg/poe/build-guides/srs-necromancer',
        pros: ['Safe playstyle', 'Good scaling', 'Minions do the work'],
        cons: ['AI issues', 'Boss targeting']
      }),
      new Build({
        id: 'lightning-arrow-deadeye',
        title: 'Lightning Arrow Deadeye',
        className: 'Ranger',
        ascendancy: 'Deadeye',
        mainSkill: 'Lightning Arrow',
        league: 'Affliction',
        patch: '3.23',
        difficulty: 'Medium',
        budget: 'High',
        rating: 4.6,
        popularity: 88,
        description: 'Fast bow build with excellent clear and good bossing.',
        url: 'https://maxroll.gg/poe/build-guides/lightning-arrow-deadeye',
        pros: ['Great clear', 'Fast movement', 'Scales well'],
        cons: ['Expensive', 'Glass cannon']
      }),
      new Build({
        id: 'cyclone-slayer',
        title: 'Cyclone Slayer',
        className: 'Duelist',
        ascendancy: 'Slayer',
        mainSkill: 'Cyclone',
        league: 'Affliction',
        patch: '3.23',
        difficulty: 'Easy',
        budget: 'Medium',
        rating: 4.4,
        popularity: 80,
        description: 'Spin to win! A classic melee build with great life leech.',
        url: 'https://maxroll.gg/poe/build-guides/cyclone-slayer',
        pros: ['Satisfying gameplay', 'Good survivability', 'League starter viable'],
        cons: ['Melee range', 'Requires investment for endgame']
      })
    ];
  }

  /**
   * Mock detailed build data
   * @private
   */
  getMockBuildDetail(buildId = 'poison-spark-pathfinder') {
    const baseBuilds = this.getMockBuildList();
    const baseBuild = baseBuilds.find(b => b.id === buildId) || baseBuilds[0];

    return new Build({
      ...baseBuild.toJSON(),
      gemLinks: [
        {
          id: 1,
          mainSkill: { name: 'Spark', level: 21, quality: 23, color: 'Blue' },
          supports: [
            { name: 'Awakened Added Chaos Damage', level: 5, color: 'Green' },
            { name: 'Awakened Void Manipulation', level: 5, color: 'Green' },
            { name: 'Awakened Unbound Ailments', level: 5, color: 'Green' },
            { name: 'Divergent Inspiration', level: 20, quality: 20, color: 'Blue' },
            { name: 'Empower', level: 4, color: 'Red' }
          ],
          socketCount: 6,
          priority: 1,
          description: 'Main damage setup'
        },
        {
          id: 2,
          mainSkill: { name: 'Sniper\'s Mark', level: 20, color: 'Green' },
          supports: [
            { name: 'Mark on Hit', level: 20, color: 'Red' },
            { name: 'Lifetap', level: 1, color: 'Red' }
          ],
          socketCount: 3,
          priority: 2,
          description: 'Curse setup'
        }
      ],
      items: [
        {
          name: 'Covenant',
          type: 'Body Armour',
          rarity: 'Unique',
          slot: 'Body',
          importance: 'Required',
          explicits: ['Socketed Gems are Supported by Level 29 Added Chaos Damage'],
          price: { chaos: 150, divine: 0 }
        },
        {
          name: 'Circle of Nostalgia',
          type: 'Ring',
          rarity: 'Unique',
          slot: 'Ring',
          importance: 'High',
          explicits: ['Herald of Agony has increased Buff Effect', 'Reduced Herald of Agony Mana Reservation'],
          price: { chaos: 0, divine: 2 }
        }
      ],
      pantheon: {
        major: 'Soul of Lunaris',
        minor: 'Soul of Shakari'
      },
      bandit: 'Kill All',
      defenseStats: {
        life: 4500,
        energyShield: 1200,
        armour: 5000,
        evasion: 15000,
        blockChance: 15,
        spellSuppressionChance: 100
      },
      offenseStats: {
        dps: 8000000,
        critChance: 5,
        critMultiplier: 150,
        attackSpeed: 0,
        castSpeed: 4.5
      }
    });
  }
}

export const buildGuideService = new BuildGuideService();
export default buildGuideService;
