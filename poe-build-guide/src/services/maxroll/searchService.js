/**
 * Search Service - Provides comprehensive search and filter functionality
 * for builds, skills, and items from Maxroll.gg
 */

import { buildGuideService } from './buildGuideService';
import { skillDataService } from './skillDataService';
import { itemDataService } from './itemDataService';
import { cacheService } from '../cache/cacheService';

const CACHE_TTL = 1800; // 30 minutes cache for search results

class SearchService {
  /**
   * Search builds by various criteria
   * @param {Object} criteria - Search criteria
   * @returns {Promise<Array<Build>>}
   */
  async searchBuilds(criteria = {}) {
    const cacheKey = `search_builds_${JSON.stringify(criteria)}`;

    const cached = cacheService.get(cacheKey);
    if (cached) {
      console.log('Returning cached search results');
      return cached;
    }

    try {
      const allBuilds = await buildGuideService.fetchAllBuilds();

      let results = allBuilds.filter(build => {
        // Filter by class
        if (criteria.className && build.className !== criteria.className) {
          return false;
        }

        // Filter by ascendancy
        if (criteria.ascendancy && build.ascendancy !== criteria.ascendancy) {
          return false;
        }

        // Filter by main skill
        if (criteria.mainSkill && !build.mainSkill.toLowerCase().includes(criteria.mainSkill.toLowerCase())) {
          return false;
        }

        // Filter by league
        if (criteria.league && build.league !== criteria.league) {
          return false;
        }

        // Filter by patch
        if (criteria.patch && build.patch !== criteria.patch) {
          return false;
        }

        // Filter by difficulty
        if (criteria.difficulty && build.difficulty !== criteria.difficulty) {
          return false;
        }

        // Filter by budget
        if (criteria.budget && build.budget !== criteria.budget) {
          return false;
        }

        // Filter by minimum rating
        if (criteria.minRating && build.rating < criteria.minRating) {
          return false;
        }

        // Text search in title and description
        if (criteria.searchText) {
          const searchLower = criteria.searchText.toLowerCase();
          const inTitle = build.title.toLowerCase().includes(searchLower);
          const inDescription = build.description.toLowerCase().includes(searchLower);
          if (!inTitle && !inDescription) {
            return false;
          }
        }

        return true;
      });

      // Apply sorting
      results = this.sortBuilds(results, criteria.sortBy || 'popularity', criteria.sortOrder || 'desc');

      // Apply pagination
      if (criteria.limit) {
        const offset = criteria.offset || 0;
        results = results.slice(offset, offset + criteria.limit);
      }

      // Cache the results
      cacheService.set(cacheKey, results, CACHE_TTL);

      return results;
    } catch (error) {
      console.error('Error searching builds:', error);
      throw new Error(`Failed to search builds: ${error.message}`);
    }
  }

  /**
   * Get builds by class
   * @param {string} className - Class name
   * @returns {Promise<Array<Build>>}
   */
  async getBuildsByClass(className) {
    return this.searchBuilds({ className });
  }

  /**
   * Get builds by skill
   * @param {string} skillName - Skill name
   * @returns {Promise<Array<Build>>}
   */
  async getBuildsBySkill(skillName) {
    return this.searchBuilds({ mainSkill: skillName });
  }

  /**
   * Get popular builds
   * @param {number} limit - Number of builds to return
   * @returns {Promise<Array<Build>>}
   */
  async getPopularBuilds(limit = 10) {
    return this.searchBuilds({
      sortBy: 'popularity',
      sortOrder: 'desc',
      limit
    });
  }

  /**
   * Get top-rated builds
   * @param {number} limit - Number of builds to return
   * @returns {Promise<Array<Build>>}
   */
  async getTopRatedBuilds(limit = 10) {
    return this.searchBuilds({
      sortBy: 'rating',
      sortOrder: 'desc',
      limit
    });
  }

  /**
   * Get league starter builds
   * @returns {Promise<Array<Build>>}
   */
  async getLeagueStarterBuilds() {
    return this.searchBuilds({
      budget: 'Low',
      difficulty: 'Easy',
      sortBy: 'rating',
      sortOrder: 'desc'
    });
  }

  /**
   * Get budget builds
   * @returns {Promise<Array<Build>>}
   */
  async getBudgetBuilds() {
    return this.searchBuilds({
      budget: 'Low',
      sortBy: 'rating',
      sortOrder: 'desc'
    });
  }

  /**
   * Search all data types (builds, skills, items)
   * @param {string} searchText - Search text
   * @returns {Promise<Object>}
   */
  async searchAll(searchText) {
    const [builds, skills, items] = await Promise.all([
      this.searchBuilds({ searchText, limit: 10 }),
      skillDataService.searchSkills({ name: searchText }),
      itemDataService.searchItems({ name: searchText })
    ]);

    return {
      builds: builds.slice(0, 5),
      skills: skills.slice(0, 5),
      items: items.slice(0, 5)
    };
  }

  /**
   * Get available filter options
   * @returns {Promise<Object>}
   */
  async getFilterOptions() {
    const cacheKey = 'filter_options';

    const cached = cacheService.get(cacheKey);
    if (cached) {
      return cached;
    }

    try {
      const allBuilds = await buildGuideService.fetchAllBuilds();

      const options = {
        classes: [...new Set(allBuilds.map(b => b.className))].sort(),
        ascendancies: [...new Set(allBuilds.map(b => b.ascendancy))].sort(),
        skills: [...new Set(allBuilds.map(b => b.mainSkill))].sort(),
        leagues: [...new Set(allBuilds.map(b => b.league))].sort(),
        patches: [...new Set(allBuilds.map(b => b.patch))].sort(),
        difficulties: ['Easy', 'Medium', 'Hard'],
        budgets: ['Low', 'Medium', 'High'],
        sortOptions: [
          { value: 'popularity', label: 'Popularity' },
          { value: 'rating', label: 'Rating' },
          { value: 'title', label: 'Name' },
          { value: 'lastUpdated', label: 'Last Updated' }
        ]
      };

      cacheService.set(cacheKey, options, 3600); // Cache for 1 hour
      return options;
    } catch (error) {
      console.error('Error getting filter options:', error);
      throw new Error(`Failed to get filter options: ${error.message}`);
    }
  }

  /**
   * Sort builds by criteria
   * @private
   */
  sortBuilds(builds, sortBy, sortOrder = 'desc') {
    const sorted = [...builds].sort((a, b) => {
      let aVal, bVal;

      switch (sortBy) {
        case 'popularity':
          aVal = a.popularity;
          bVal = b.popularity;
          break;
        case 'rating':
          aVal = a.rating;
          bVal = b.rating;
          break;
        case 'title':
          aVal = a.title.toLowerCase();
          bVal = b.title.toLowerCase();
          break;
        case 'lastUpdated':
          aVal = new Date(a.lastUpdated);
          bVal = new Date(b.lastUpdated);
          break;
        default:
          return 0;
      }

      if (aVal < bVal) return sortOrder === 'asc' ? -1 : 1;
      if (aVal > bVal) return sortOrder === 'asc' ? 1 : -1;
      return 0;
    });

    return sorted;
  }
}

export const searchService = new SearchService();
export default searchService;
