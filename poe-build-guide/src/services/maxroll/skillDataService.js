/**
 * Skill Data Service - Fetches and manages skill gem data from Maxroll.gg
 */

import Skill from '../../models/Skill';
import { rateLimiter } from '../utils/rateLimiter';
import { cacheService } from '../cache/cacheService';

const MAXROLL_BASE_URL = 'https://maxroll.gg/poe';
const CACHE_TTL = 7200; // 2 hours cache for skill data

class SkillDataService {
  constructor() {
    this.rateLimiter = rateLimiter;
  }

  /**
   * Fetch all skill gems
   * @param {Object} options - Fetch options
   * @returns {Promise<Array<Skill>>}
   */
  async fetchAllSkills(options = {}) {
    const cacheKey = 'all_skills';

    const cached = cacheService.get(cacheKey);
    if (cached && !options.forceRefresh) {
      console.log('Returning cached skill list');
      return cached.map(s => new Skill(s));
    }

    try {
      await this.rateLimiter.consume('maxroll', 1);

      // In real implementation, scrape skill gem database
      const skills = this.getMockSkillData();

      cacheService.set(cacheKey, skills.map(s => s.toJSON()), CACHE_TTL);
      return skills;
    } catch (error) {
      console.error('Error fetching skill data:', error);

      const staleCache = cacheService.get(cacheKey, true);
      if (staleCache) {
        return staleCache.map(s => new Skill(s));
      }

      throw new Error(`Failed to fetch skill data: ${error.message}`);
    }
  }

  /**
   * Fetch skill by name
   * @param {string} skillName - Skill name
   * @returns {Promise<Skill>}
   */
  async fetchSkill(skillName) {
    const cacheKey = `skill_${skillName.toLowerCase().replace(/\s/g, '_')}`;

    const cached = cacheService.get(cacheKey);
    if (cached) {
      return new Skill(cached);
    }

    try {
      await this.rateLimiter.consume('maxroll', 1);

      // In real implementation, scrape specific skill page
      const skill = this.getMockSkillDetail(skillName);

      cacheService.set(cacheKey, skill.toJSON(), CACHE_TTL);
      return skill;
    } catch (error) {
      console.error(`Error fetching skill ${skillName}:`, error);
      throw new Error(`Failed to fetch skill: ${error.message}`);
    }
  }

  /**
   * Search skills by criteria
   * @param {Object} criteria - Search criteria
   * @returns {Promise<Array<Skill>>}
   */
  async searchSkills(criteria = {}) {
    const allSkills = await this.fetchAllSkills();

    return allSkills.filter(skill => {
      if (criteria.type && skill.type !== criteria.type) return false;
      if (criteria.color && skill.color !== criteria.color) return false;
      if (criteria.tags && !criteria.tags.some(tag => skill.tags.includes(tag))) return false;
      if (criteria.name && !skill.name.toLowerCase().includes(criteria.name.toLowerCase())) return false;
      return true;
    });
  }

  /**
   * Get skill recommendations for a build
   * @param {string} mainSkill - Main skill name
   * @returns {Promise<Array<Skill>>}
   */
  async getSkillRecommendations(mainSkill) {
    const skill = await this.fetchSkill(mainSkill);

    // Return support gems that work well with this skill
    const supports = await this.searchSkills({ type: 'Support' });

    // Filter based on skill tags (simplified)
    return supports.filter(support =>
      skill.tags.some(tag => support.tags.includes(tag))
    ).slice(0, 10);
  }

  /**
   * Mock skill data
   * @private
   */
  getMockSkillData() {
    return [
      new Skill({
        id: 'spark',
        name: 'Spark',
        type: 'Active',
        color: 'Blue',
        level: 1,
        description: 'Launches unpredictable sparks that move randomly until they hit an enemy or expire.',
        tags: ['Spell', 'Projectile', 'Duration', 'Lightning'],
        manaCost: 12,
        cooldown: 0,
        vaalVersion: true,
        url: 'https://maxroll.gg/poe/skill-gems/spark'
      }),
      new Skill({
        id: 'righteous-fire',
        name: 'Righteous Fire',
        type: 'Active',
        color: 'Red',
        level: 16,
        description: 'Engulfs you in magical fire that rapidly burns you and nearby enemies.',
        tags: ['Spell', 'AoE', 'Fire', 'Duration'],
        manaCost: 25,
        cooldown: 0,
        vaalVersion: true,
        url: 'https://maxroll.gg/poe/skill-gems/righteous-fire'
      }),
      new Skill({
        id: 'summon-raging-spirit',
        name: 'Summon Raging Spirit',
        type: 'Active',
        color: 'Red',
        level: 4,
        description: 'Summons a short-lived flaming skull that rushes at enemies.',
        tags: ['Spell', 'Minion', 'Duration', 'Fire'],
        manaCost: 16,
        cooldown: 0,
        vaalVersion: false,
        url: 'https://maxroll.gg/poe/skill-gems/summon-raging-spirit'
      }),
      new Skill({
        id: 'added-chaos-damage',
        name: 'Added Chaos Damage Support',
        type: 'Support',
        color: 'Green',
        level: 8,
        description: 'Adds chaos damage to supported attacks and spells.',
        tags: ['Support', 'Chaos'],
        manaCost: 0,
        cooldown: 0,
        awakened: true,
        url: 'https://maxroll.gg/poe/skill-gems/added-chaos-damage-support'
      }),
      new Skill({
        id: 'void-manipulation',
        name: 'Void Manipulation Support',
        type: 'Support',
        color: 'Green',
        level: 8,
        description: 'Increases chaos damage but prevents dealing non-chaos damage.',
        tags: ['Support', 'Chaos'],
        manaCost: 0,
        cooldown: 0,
        awakened: true,
        url: 'https://maxroll.gg/poe/skill-gems/void-manipulation-support'
      }),
      new Skill({
        id: 'inspiration',
        name: 'Inspiration Support',
        type: 'Support',
        color: 'Blue',
        level: 18,
        description: 'Grants charges that reduce mana cost and increase critical strike chance.',
        tags: ['Support', 'Critical'],
        manaCost: 0,
        cooldown: 0,
        awakened: false,
        url: 'https://maxroll.gg/poe/skill-gems/inspiration-support'
      }),
      new Skill({
        id: 'empower',
        name: 'Empower Support',
        type: 'Support',
        color: 'Red',
        level: 1,
        description: 'Increases the level of supported active skill gems.',
        tags: ['Support'],
        manaCost: 0,
        cooldown: 0,
        awakened: false,
        url: 'https://maxroll.gg/poe/skill-gems/empower-support'
      })
    ];
  }

  /**
   * Mock skill detail
   * @private
   */
  getMockSkillDetail(skillName) {
    const skills = this.getMockSkillData();
    return skills.find(s => s.name.toLowerCase() === skillName.toLowerCase()) || skills[0];
  }
}

export const skillDataService = new SkillDataService();
export default skillDataService;
