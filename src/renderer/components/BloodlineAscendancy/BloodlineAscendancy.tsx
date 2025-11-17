import React, { useState, useEffect } from 'react';
import './BloodlineAscendancy.scss';

interface Notable {
  name: string;
  position: { x: number; y: number };
  type: 'keystone' | 'notable';
  bonuses: string[];
  description: string;
}

interface UnlockRequirement {
  boss: string;
  location: string;
  difficulty: string;
}

interface Bloodline {
  id: string;
  name: string;
  theme: string;
  description: string;
  icon: string;
  color: string;
  unlockRequirement: UnlockRequirement;
  notables: Notable[];
  bestCombinations: string[];
  synergyBonus: string;
}

interface BloodlineAscendancyProps {
  onClose?: () => void;
}

const BloodlineAscendancy: React.FC<BloodlineAscendancyProps> = ({ onClose }) => {
  const [bloodlines, setBloodlines] = useState<Bloodline[]>([]);
  const [selectedBloodline, setSelectedBloodline] = useState<Bloodline | null>(null);
  const [allocatedNodes, setAllocatedNodes] = useState<string[]>([]);
  const [mainAscendancy, setMainAscendancy] = useState('');
  const [showSynergy, setShowSynergy] = useState(false);

  // Mock data - in a real implementation, this would come from bloodlineService via IPC
  useEffect(() => {
    const mockBloodlines: Bloodline[] = [
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
            description: "Life renews eternally at the tree's heart"
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
      }
    ];
    setBloodlines(mockBloodlines);
  }, []);

  const handleSelectBloodline = (bloodline: Bloodline) => {
    setSelectedBloodline(bloodline);
    setAllocatedNodes([]);
  };

  const handleAllocateNode = (nodeName: string) => {
    if (allocatedNodes.includes(nodeName)) {
      setAllocatedNodes(allocatedNodes.filter(n => n !== nodeName));
    } else {
      setAllocatedNodes([...allocatedNodes, nodeName]);
    }
  };

  const calculateSynergyRating = (bloodline: Bloodline, ascendancy: string) => {
    if (!ascendancy) return { rating: 0, text: 'Not calculated' };

    const index = bloodline.bestCombinations.indexOf(ascendancy);
    if (index === 0) return { rating: 5, text: 'Excellent' };
    if (index === 1) return { rating: 4, text: 'Very Good' };
    if (index === 2) return { rating: 3, text: 'Good' };
    return { rating: 2, text: 'Moderate' };
  };

  const getDifficultyColor = (difficulty: string) => {
    switch (difficulty.toLowerCase()) {
      case 'extreme': return '#dc2626';
      case 'very high': return '#f97316';
      case 'high': return '#fbbf24';
      default: return '#10b981';
    }
  };

  return (
    <div className="bloodline-ascendancy">
      <div className="bloodline-ascendancy__header">
        <h3>Bloodline Ascendancy Planner</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>×</button>
        )}
      </div>

      {/* Main Ascendancy Input */}
      <div className="bloodline-ascendancy__main">
        <h4>Your Main Ascendancy</h4>
        <div className="main-config">
          <input
            type="text"
            placeholder="Enter your main ascendancy (e.g., Occultist)"
            value={mainAscendancy}
            onChange={(e) => setMainAscendancy(e.target.value)}
          />
          <button
            className="synergy-btn"
            onClick={() => setShowSynergy(!showSynergy)}
          >
            {showSynergy ? 'Hide' : 'Show'} Synergy Ratings
          </button>
        </div>
      </div>

      {/* Bloodline Selection */}
      <div className="bloodline-ascendancy__selection">
        <h4>Select Bloodline Ascendancy</h4>
        <div className="bloodline-grid">
          {bloodlines.map(bloodline => {
            const synergy = mainAscendancy ? calculateSynergyRating(bloodline, mainAscendancy) : null;
            return (
              <div
                key={bloodline.id}
                className={`bloodline-card ${selectedBloodline?.id === bloodline.id ? 'selected' : ''}`}
                style={{ borderColor: bloodline.color }}
                onClick={() => handleSelectBloodline(bloodline)}
              >
                <div className="bloodline-header">
                  <h5 style={{ color: bloodline.color }}>{bloodline.name}</h5>
                  <span className="bloodline-theme">{bloodline.theme}</span>
                </div>
                <p className="bloodline-description">{bloodline.description}</p>

                {showSynergy && synergy && (
                  <div className="synergy-rating">
                    <span className="rating-label">Synergy:</span>
                    <span className="rating-stars">
                      {'★'.repeat(synergy.rating)}{'☆'.repeat(5 - synergy.rating)}
                    </span>
                    <span className="rating-text">{synergy.text}</span>
                  </div>
                )}

                <div className="bloodline-combos">
                  <strong>Best with:</strong> {bloodline.bestCombinations.join(', ')}
                </div>

                <div className="unlock-preview">
                  <strong>Boss:</strong> {bloodline.unlockRequirement.boss}
                  <span
                    className="difficulty-badge"
                    style={{ backgroundColor: getDifficultyColor(bloodline.unlockRequirement.difficulty) }}
                  >
                    {bloodline.unlockRequirement.difficulty}
                  </span>
                </div>
              </div>
            );
          })}
        </div>
      </div>

      {/* Selected Bloodline Details */}
      {selectedBloodline && (
        <div className="bloodline-ascendancy__details">
          <div className="details-header" style={{ borderColor: selectedBloodline.color }}>
            <h4 style={{ color: selectedBloodline.color }}>{selectedBloodline.name} - Passive Tree</h4>
            <div className="allocated-count">
              Allocated: {allocatedNodes.length}/{selectedBloodline.notables.length}
            </div>
          </div>

          {/* Unlock Requirements */}
          <div className="unlock-requirements">
            <h5>Unlock Requirements</h5>
            <div className="requirement-details">
              <div className="req-item">
                <strong>Boss:</strong> {selectedBloodline.unlockRequirement.boss}
              </div>
              <div className="req-item">
                <strong>Location:</strong> {selectedBloodline.unlockRequirement.location}
              </div>
              <div className="req-item">
                <strong>Difficulty:</strong>
                <span
                  className="difficulty-badge"
                  style={{ backgroundColor: getDifficultyColor(selectedBloodline.unlockRequirement.difficulty) }}
                >
                  {selectedBloodline.unlockRequirement.difficulty}
                </span>
              </div>
            </div>
          </div>

          {/* Passive Tree */}
          <div className="passive-tree">
            <h5>Notable Passives</h5>
            <div className="notables-list">
              {selectedBloodline.notables.map((notable, index) => (
                <div
                  key={index}
                  className={`notable-node ${notable.type} ${allocatedNodes.includes(notable.name) ? 'allocated' : ''}`}
                  onClick={() => handleAllocateNode(notable.name)}
                  style={{
                    borderColor: allocatedNodes.includes(notable.name) ? selectedBloodline.color : '#555'
                  }}
                >
                  <div className="node-header">
                    <span className="node-name" style={{ color: selectedBloodline.color }}>
                      {notable.name}
                    </span>
                    <span className="node-type">
                      {notable.type === 'keystone' ? '◆ Keystone' : '● Notable'}
                    </span>
                  </div>
                  <p className="node-description">{notable.description}</p>
                  <div className="node-bonuses">
                    {notable.bonuses.map((bonus, i) => (
                      <div key={i} className="bonus-line">{bonus}</div>
                    ))}
                  </div>
                  <div className="node-action">
                    {allocatedNodes.includes(notable.name) ? 'Click to Deallocate' : 'Click to Allocate'}
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* Synergy Bonus */}
          <div className="synergy-bonus" style={{ borderColor: selectedBloodline.color }}>
            <h5>Synergy Bonus</h5>
            <p>{selectedBloodline.synergyBonus}</p>
          </div>

          {/* Allocated Bonuses Summary */}
          {allocatedNodes.length > 0 && (
            <div className="allocated-summary">
              <h5>Your Allocated Bonuses</h5>
              <div className="summary-list">
                {allocatedNodes.map(nodeName => {
                  const node = selectedBloodline.notables.find(n => n.name === nodeName);
                  return node ? (
                    <div key={nodeName} className="summary-node">
                      <strong style={{ color: selectedBloodline.color }}>{node.name}:</strong>
                      <ul>
                        {node.bonuses.map((bonus, i) => (
                          <li key={i}>{bonus}</li>
                        ))}
                      </ul>
                    </div>
                  ) : null;
                })}
              </div>
            </div>
          )}
        </div>
      )}

      {/* Info Section */}
      <div className="bloodline-ascendancy__info">
        <h4>About Bloodline Ascendancies</h4>
        <ul>
          <li>Bloodline Ascendancies are secondary ascendancy trees with powerful unique passives</li>
          <li>Each bloodline has 4 notable passives, including keystones that drastically alter playstyle</li>
          <li>Must defeat specific endgame bosses to unlock each bloodline</li>
          <li>Choose bloodlines that synergize with your main ascendancy for maximum effectiveness</li>
          <li>Hotkey: <strong>Ctrl+Shift+A</strong> to open this planner</li>
        </ul>
      </div>
    </div>
  );
};

export default BloodlineAscendancy;
