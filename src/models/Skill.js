/**
 * Skill gem data model
 */

class Skill {
  constructor(data = {}) {
    this.id = data.id || null;
    this.name = data.name || '';
    this.type = data.type || 'Active'; // Active, Support, Aura, etc.
    this.color = data.color || 'Red'; // Red, Green, Blue
    this.level = data.level || 1;
    this.quality = data.quality || 0;
    this.description = data.description || '';
    this.tags = data.tags || [];
    this.manaCost = data.manaCost || 0;
    this.cooldown = data.cooldown || 0;
    this.vaalVersion = data.vaalVersion || false;
    this.awakened = data.awakened || false;
    this.alternatives = data.alternatives || [];
    this.url = data.url || '';
  }

  toJSON() {
    return {
      id: this.id,
      name: this.name,
      type: this.type,
      color: this.color,
      level: this.level,
      quality: this.quality,
      description: this.description,
      tags: this.tags,
      manaCost: this.manaCost,
      cooldown: this.cooldown,
      vaalVersion: this.vaalVersion,
      awakened: this.awakened,
      alternatives: this.alternatives,
      url: this.url
    };
  }
}

class GemLink {
  constructor(data = {}) {
    this.id = data.id || null;
    this.mainSkill = data.mainSkill || null;
    this.supports = data.supports || [];
    this.socketCount = data.socketCount || 6;
    this.priority = data.priority || 1; // 1 = main, 2 = secondary, etc.
    this.description = data.description || '';
    this.alternatives = data.alternatives || [];
  }

  toJSON() {
    return {
      id: this.id,
      mainSkill: this.mainSkill,
      supports: this.supports,
      socketCount: this.socketCount,
      priority: this.priority,
      description: this.description,
      alternatives: this.alternatives
    };
  }
}

module.exports = Skill;
module.exports.Skill = Skill;
module.exports.GemLink = GemLink;
