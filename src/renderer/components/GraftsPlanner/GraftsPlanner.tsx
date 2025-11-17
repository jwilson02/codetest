import React, { useState, useEffect } from 'react';
import './GraftsPlanner.scss';

interface ActiveSkill {
  name: string;
  cooldown: number;
  description: string;
  damage: string;
}

interface Graft {
  id: string;
  name: string;
  type: 'offensive' | 'defensive' | 'utility' | 'active';
  rarity: 'rare' | 'unique' | 'mythic';
  description: string;
  bonuses: string[];
  activeSkill: ActiveSkill | null;
  compatible: string[];
  acquisition: string;
  lore: string;
}

interface GraftsPlannerProps {
  onClose?: () => void;
}

const GraftsPlanner: React.FC<GraftsPlannerProps> = ({ onClose }) => {
  const [grafts, setGrafts] = useState<Graft[]>([]);
  const [equippedGrafts, setEquippedGrafts] = useState<Graft[]>([]);
  const [selectedType, setSelectedType] = useState<string>('all');
  const [selectedGraft, setSelectedGraft] = useState<Graft | null>(null);
  const [buildConfig, setBuildConfig] = useState({
    damageType: 'elemental',
    defenseType: 'energy_shield',
    ascendancy: '',
    playstyle: 'balanced'
  });
  const [showRecommendations, setShowRecommendations] = useState(false);

  // Mock data - in a real implementation, this would come from the graftsService via IPC
  useEffect(() => {
    const mockGrafts: Graft[] = [
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
      }
    ];
    setGrafts(mockGrafts);
  }, []);

  const filteredGrafts = selectedType === 'all'
    ? grafts
    : grafts.filter(g => g.type === selectedType);

  const handleEquipGraft = (graft: Graft) => {
    if (equippedGrafts.length >= 2) {
      alert('Maximum of 2 grafts can be equipped. Unequip one first.');
      return;
    }

    if (equippedGrafts.some(g => g.id === graft.id)) {
      alert('Graft is already equipped');
      return;
    }

    setEquippedGrafts([...equippedGrafts, graft]);
  };

  const handleUnequipGraft = (graftId: string) => {
    setEquippedGrafts(equippedGrafts.filter(g => g.id !== graftId));
  };

  const getRarityColor = (rarity: string) => {
    switch (rarity) {
      case 'mythic': return '#af52de';
      case 'unique': return '#af6025';
      case 'rare': return '#ffff00';
      default: return '#fff';
    }
  };

  const getTypeIcon = (type: string) => {
    switch (type) {
      case 'offensive': return '⚔️';
      case 'defensive': return '🛡️';
      case 'utility': return '⚡';
      case 'active': return '✨';
      default: return '📦';
    }
  };

  return (
    <div className="grafts-planner">
      <div className="grafts-planner__header">
        <h3>Grafts System Planner</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>×</button>
        )}
      </div>

      {/* Equipped Grafts Display */}
      <div className="grafts-planner__equipped">
        <h4>Equipped Grafts ({equippedGrafts.length}/2)</h4>
        <div className="equipped-slots">
          {[0, 1].map(index => (
            <div key={index} className={`equipped-slot ${equippedGrafts[index] ? 'filled' : 'empty'}`}>
              {equippedGrafts[index] ? (
                <div className="equipped-graft">
                  <div className="graft-name" style={{ color: getRarityColor(equippedGrafts[index].rarity) }}>
                    {getTypeIcon(equippedGrafts[index].type)} {equippedGrafts[index].name}
                  </div>
                  <div className="graft-bonuses">
                    {equippedGrafts[index].bonuses.map((bonus, i) => (
                      <div key={i} className="bonus-line">{bonus}</div>
                    ))}
                  </div>
                  {equippedGrafts[index].activeSkill && (
                    <div className="active-skill">
                      <strong>{equippedGrafts[index].activeSkill!.name}</strong> (CD: {equippedGrafts[index].activeSkill!.cooldown}s)
                      <div className="skill-desc">{equippedGrafts[index].activeSkill!.description}</div>
                    </div>
                  )}
                  <button
                    className="unequip-btn"
                    onClick={() => handleUnequipGraft(equippedGrafts[index].id)}
                  >
                    Unequip
                  </button>
                </div>
              ) : (
                <div className="empty-slot-text">Empty Slot {index + 1}</div>
              )}
            </div>
          ))}
        </div>
      </div>

      {/* Build Configuration */}
      <div className="grafts-planner__config">
        <h4>Build Configuration</h4>
        <div className="config-grid">
          <label>
            Damage Type:
            <select
              value={buildConfig.damageType}
              onChange={(e) => setBuildConfig({ ...buildConfig, damageType: e.target.value })}
            >
              <option value="physical">Physical</option>
              <option value="elemental">Elemental</option>
              <option value="fire">Fire</option>
              <option value="cold">Cold</option>
              <option value="lightning">Lightning</option>
              <option value="chaos">Chaos</option>
            </select>
          </label>

          <label>
            Defense Type:
            <select
              value={buildConfig.defenseType}
              onChange={(e) => setBuildConfig({ ...buildConfig, defenseType: e.target.value })}
            >
              <option value="armour">Armour</option>
              <option value="evasion">Evasion</option>
              <option value="energy_shield">Energy Shield</option>
              <option value="hybrid">Hybrid</option>
            </select>
          </label>

          <label>
            Ascendancy:
            <input
              type="text"
              placeholder="e.g., Occultist"
              value={buildConfig.ascendancy}
              onChange={(e) => setBuildConfig({ ...buildConfig, ascendancy: e.target.value })}
            />
          </label>

          <label>
            Playstyle:
            <select
              value={buildConfig.playstyle}
              onChange={(e) => setBuildConfig({ ...buildConfig, playstyle: e.target.value })}
            >
              <option value="fast">Fast Clear</option>
              <option value="tank">Tank</option>
              <option value="crit">Critical Strike</option>
              <option value="balanced">Balanced</option>
            </select>
          </label>
        </div>
        <button
          className="recommend-btn"
          onClick={() => setShowRecommendations(!showRecommendations)}
        >
          {showRecommendations ? 'Hide' : 'Show'} Recommendations
        </button>
      </div>

      {/* Type Filter */}
      <div className="grafts-planner__filters">
        <h4>Filter by Type</h4>
        <div className="filter-buttons">
          <button
            className={selectedType === 'all' ? 'active' : ''}
            onClick={() => setSelectedType('all')}
          >
            All ({grafts.length})
          </button>
          <button
            className={selectedType === 'offensive' ? 'active' : ''}
            onClick={() => setSelectedType('offensive')}
          >
            ⚔️ Offensive
          </button>
          <button
            className={selectedType === 'defensive' ? 'active' : ''}
            onClick={() => setSelectedType('defensive')}
          >
            🛡️ Defensive
          </button>
          <button
            className={selectedType === 'utility' ? 'active' : ''}
            onClick={() => setSelectedType('utility')}
          >
            ⚡ Utility
          </button>
          <button
            className={selectedType === 'active' ? 'active' : ''}
            onClick={() => setSelectedType('active')}
          >
            ✨ Active Skills
          </button>
        </div>
      </div>

      {/* Grafts List */}
      <div className="grafts-planner__list">
        <h4>Available Grafts</h4>
        <div className="grafts-grid">
          {filteredGrafts.map(graft => (
            <div
              key={graft.id}
              className={`graft-card ${selectedGraft?.id === graft.id ? 'selected' : ''}`}
              onClick={() => setSelectedGraft(graft)}
            >
              <div className="graft-header">
                <span className="graft-icon">{getTypeIcon(graft.type)}</span>
                <span className="graft-name" style={{ color: getRarityColor(graft.rarity) }}>
                  {graft.name}
                </span>
                <span className="graft-rarity" style={{ color: getRarityColor(graft.rarity) }}>
                  {graft.rarity}
                </span>
              </div>
              <div className="graft-description">{graft.description}</div>
              <div className="graft-bonuses">
                {graft.bonuses.map((bonus, i) => (
                  <div key={i} className="bonus">{bonus}</div>
                ))}
              </div>
              {graft.activeSkill && (
                <div className="graft-skill">
                  <strong>Active:</strong> {graft.activeSkill.name} (CD: {graft.activeSkill.cooldown}s)
                </div>
              )}
              <div className="graft-compatible">
                <strong>Compatible:</strong> {graft.compatible.join(', ')}
              </div>
              <button
                className="equip-btn"
                onClick={(e) => {
                  e.stopPropagation();
                  handleEquipGraft(graft);
                }}
                disabled={equippedGrafts.some(g => g.id === graft.id)}
              >
                {equippedGrafts.some(g => g.id === graft.id) ? 'Equipped' : 'Equip'}
              </button>
            </div>
          ))}
        </div>
      </div>

      {/* Selected Graft Details */}
      {selectedGraft && (
        <div className="grafts-planner__details">
          <h4>Graft Details</h4>
          <div className="details-panel">
            <div className="detail-section">
              <h5 style={{ color: getRarityColor(selectedGraft.rarity) }}>
                {getTypeIcon(selectedGraft.type)} {selectedGraft.name}
              </h5>
              <p className="lore">{selectedGraft.lore}</p>
            </div>
            <div className="detail-section">
              <h6>Acquisition</h6>
              <p>{selectedGraft.acquisition}</p>
            </div>
            {selectedGraft.activeSkill && (
              <div className="detail-section">
                <h6>Active Skill: {selectedGraft.activeSkill.name}</h6>
                <p>Cooldown: {selectedGraft.activeSkill.cooldown} seconds</p>
                <p>Damage: {selectedGraft.activeSkill.damage}</p>
                <p>{selectedGraft.activeSkill.description}</p>
              </div>
            )}
          </div>
        </div>
      )}

      {/* Info Section */}
      <div className="grafts-planner__info">
        <h4>About Grafts</h4>
        <ul>
          <li>Grafts are powerful modular enhancements for your character</li>
          <li>You can equip up to 2 grafts once the Genesis Tree is leveled</li>
          <li>Each graft provides unique bonuses and some grant active skills</li>
          <li>Choose grafts that synergize with your build and ascendancy</li>
          <li>Rarity: <span style={{ color: '#ffff00' }}>Rare</span> / <span style={{ color: '#af6025' }}>Unique</span> / <span style={{ color: '#af52de' }}>Mythic</span></li>
        </ul>
      </div>
    </div>
  );
};

export default GraftsPlanner;
