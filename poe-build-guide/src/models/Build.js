/**
 * Build data model for Path of Exile builds
 */

export class Build {
  constructor(data = {}) {
    this.id = data.id || null;
    this.title = data.title || '';
    this.className = data.className || '';
    this.ascendancy = data.ascendancy || '';
    this.mainSkill = data.mainSkill || '';
    this.author = data.author || 'Maxroll.gg';
    this.league = data.league || 'Current';
    this.patch = data.patch || '';
    this.url = data.url || '';
    this.description = data.description || '';
    this.difficulty = data.difficulty || 'Medium';
    this.budget = data.budget || 'Medium';
    this.playstyle = data.playstyle || '';
    this.rating = data.rating || 0;
    this.popularity = data.popularity || 0;
    this.lastUpdated = data.lastUpdated || new Date().toISOString();

    // Build details
    this.passiveTree = data.passiveTree || null;
    this.gemLinks = data.gemLinks || [];
    this.items = data.items || [];
    this.pantheon = data.pantheon || { major: '', minor: '' };
    this.bandit = data.bandit || '';
    this.levelingGuide = data.levelingGuide || [];
    this.gearProgression = data.gearProgression || [];
    this.flasks = data.flasks || [];
    this.jewels = data.jewels || [];

    // Advanced details
    this.defenseStats = data.defenseStats || {};
    this.offenseStats = data.offenseStats || {};
    this.pros = data.pros || [];
    this.cons = data.cons || [];
    this.videoGuide = data.videoGuide || '';
    this.poeNinjaLink = data.poeNinjaLink || '';
  }

  /**
   * Validate the build data
   */
  validate() {
    const errors = [];

    if (!this.title) errors.push('Title is required');
    if (!this.className) errors.push('Class name is required');
    if (!this.mainSkill) errors.push('Main skill is required');

    return {
      isValid: errors.length === 0,
      errors
    };
  }

  /**
   * Convert to JSON
   */
  toJSON() {
    return {
      id: this.id,
      title: this.title,
      className: this.className,
      ascendancy: this.ascendancy,
      mainSkill: this.mainSkill,
      author: this.author,
      league: this.league,
      patch: this.patch,
      url: this.url,
      description: this.description,
      difficulty: this.difficulty,
      budget: this.budget,
      playstyle: this.playstyle,
      rating: this.rating,
      popularity: this.popularity,
      lastUpdated: this.lastUpdated,
      passiveTree: this.passiveTree,
      gemLinks: this.gemLinks,
      items: this.items,
      pantheon: this.pantheon,
      bandit: this.bandit,
      levelingGuide: this.levelingGuide,
      gearProgression: this.gearProgression,
      flasks: this.flasks,
      jewels: this.jewels,
      defenseStats: this.defenseStats,
      offenseStats: this.offenseStats,
      pros: this.pros,
      cons: this.cons,
      videoGuide: this.videoGuide,
      poeNinjaLink: this.poeNinjaLink
    };
  }
}

export default Build;
