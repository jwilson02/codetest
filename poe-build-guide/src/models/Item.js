/**
 * Item data model for unique items and gear
 */

export class Item {
  constructor(data = {}) {
    this.id = data.id || null;
    this.name = data.name || '';
    this.type = data.type || ''; // Helmet, Body Armour, Weapon, etc.
    this.rarity = data.rarity || 'Rare'; // Normal, Magic, Rare, Unique
    this.basetype = data.basetype || '';
    this.level = data.level || 1;
    this.itemLevel = data.itemLevel || 1;

    // Stats and modifiers
    this.explicits = data.explicits || [];
    this.implicits = data.implicits || [];
    this.crafted = data.crafted || [];
    this.corrupted = data.corrupted || false;

    // Requirements
    this.requirements = data.requirements || {
      level: 1,
      strength: 0,
      dexterity: 0,
      intelligence: 0
    };

    // Item properties
    this.sockets = data.sockets || [];
    this.links = data.links || 0;
    this.quality = data.quality || 0;

    // Additional info
    this.flavorText = data.flavorText || '';
    this.dropLocation = data.dropLocation || '';
    this.price = data.price || { chaos: 0, divine: 0 };
    this.alternatives = data.alternatives || [];
    this.importance = data.importance || 'Medium'; // Low, Medium, High, Required
    this.slot = data.slot || '';
    this.url = data.url || '';
  }

  toJSON() {
    return {
      id: this.id,
      name: this.name,
      type: this.type,
      rarity: this.rarity,
      basetype: this.basetype,
      level: this.level,
      itemLevel: this.itemLevel,
      explicits: this.explicits,
      implicits: this.implicits,
      crafted: this.crafted,
      corrupted: this.corrupted,
      requirements: this.requirements,
      sockets: this.sockets,
      links: this.links,
      quality: this.quality,
      flavorText: this.flavorText,
      dropLocation: this.dropLocation,
      price: this.price,
      alternatives: this.alternatives,
      importance: this.importance,
      slot: this.slot,
      url: this.url
    };
  }
}

export default Item;
