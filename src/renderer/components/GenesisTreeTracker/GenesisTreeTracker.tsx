import React, { useState, useEffect } from 'react';
import './GenesisTreeTracker.scss';

interface PassiveNode {
  id: string;
  name: string;
  description: string;
  type: string;
  cost: number;
  tier: number;
  unlocked: boolean;
  prerequisites: string[];
  canUnlock: {
    canUnlock: boolean;
    reason?: string;
  };
}

interface ProgressSummary {
  graftbloodBalance: number;
  totalEarned: number;
  totalSpent: number;
  nodesUnlocked: number;
  totalNodes: number;
  progressPercent: number;
  currentTier: number;
  tiersCompleted: number[];
  availableCrafts: string[];
  uniquesFound: number;
}

interface RecommendedPath {
  name: string;
  description: string;
  path: string[];
  benefits: string;
}

interface GenesisCraft {
  id: string;
  name: string;
  description: string;
  cost: number;
  unlocked: boolean;
}

interface GenesisTreeTrackerProps {
  onClose?: () => void;
}

const GenesisTreeTracker: React.FC<GenesisTreeTrackerProps> = ({ onClose }) => {
  const [progress, setProgress] = useState<ProgressSummary | null>(null);
  const [nodes, setNodes] = useState<PassiveNode[]>([]);
  const [selectedTab, setSelectedTab] = useState<'progress' | 'nodes' | 'paths' | 'crafts'>('progress');
  const [selectedType, setSelectedType] = useState<string>('all');
  const [graftbloodInput, setGraftbloodInput] = useState<string>('');
  const [recommendedPaths, setRecommendedPaths] = useState<{ [key: string]: RecommendedPath }>({});
  const [crafts, setCrafts] = useState<GenesisCraft[]>([]);

  useEffect(() => {
    loadGenesisData();
  }, []);

  const loadGenesisData = () => {
    // In a real implementation, this would communicate with genesisTreeService via IPC
    // For now, using mock data
    const mockProgress: ProgressSummary = {
      graftbloodBalance: 145,
      totalEarned: 320,
      totalSpent: 175,
      nodesUnlocked: 6,
      totalNodes: 20,
      progressPercent: 30,
      currentTier: 2,
      tiersCompleted: [1, 2],
      availableCrafts: ['basic'],
      uniquesFound: 1
    };

    const mockNodes: PassiveNode[] = [
      {
        id: 'core1',
        name: 'Breach Foundation',
        description: '+10% increased Breach Monster pack size',
        type: 'core',
        cost: 5,
        tier: 1,
        unlocked: true,
        prerequisites: [],
        canUnlock: { canUnlock: false, reason: 'Already unlocked' }
      },
      {
        id: 'core2',
        name: 'Graftblood Sustenance',
        description: '+15% increased Graftblood drop rate',
        type: 'core',
        cost: 8,
        tier: 1,
        unlocked: true,
        prerequisites: [],
        canUnlock: { canUnlock: false, reason: 'Already unlocked' }
      },
      {
        id: 'breach1',
        name: 'Extended Breach',
        description: 'Breaches last 3 seconds longer',
        type: 'breach',
        cost: 10,
        tier: 2,
        unlocked: true,
        prerequisites: ['core1'],
        canUnlock: { canUnlock: false, reason: 'Already unlocked' }
      },
      {
        id: 'breach2',
        name: 'Breach Density',
        description: 'Breach monsters spawn 25% faster',
        type: 'breach',
        cost: 15,
        tier: 2,
        unlocked: false,
        prerequisites: ['breach1'],
        canUnlock: { canUnlock: true }
      },
      {
        id: 'hive1',
        name: 'Hive Seeker',
        description: 'Breach Hives appear 20% more frequently',
        type: 'hive',
        cost: 12,
        tier: 2,
        unlocked: true,
        prerequisites: ['core1'],
        canUnlock: { canUnlock: false, reason: 'Already unlocked' }
      },
      {
        id: 'ailith1',
        name: "Ailith's Favor",
        description: 'Ailith protection duration increased by 30%',
        type: 'ailith',
        cost: 10,
        tier: 2,
        unlocked: true,
        prerequisites: ['core2'],
        canUnlock: { canUnlock: false, reason: 'Already unlocked' }
      },
      {
        id: 'reward1',
        name: 'Greater Spoils',
        description: '+10% increased quantity of items from Breach monsters',
        type: 'reward',
        cost: 15,
        tier: 2,
        unlocked: true,
        prerequisites: ['core2'],
        canUnlock: { canUnlock: false, reason: 'Already unlocked' }
      },
      {
        id: 'breach3',
        name: 'Splinter Fortune',
        description: '+20% increased Breach Splinter drop rate',
        type: 'breach',
        cost: 20,
        tier: 3,
        unlocked: false,
        prerequisites: ['breach2'],
        canUnlock: { canUnlock: false, reason: 'Requires Breach Density' }
      }
    ];

    const mockPaths: { [key: string]: RecommendedPath } = {
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
      }
    };

    const mockCrafts: GenesisCraft[] = [
      {
        id: 'basic',
        name: 'Graftblood Infusion',
        description: 'Add Breach modifiers to items',
        cost: 20,
        unlocked: true
      },
      {
        id: 'advanced',
        name: 'Hiveborn Essence',
        description: 'Guarantee Breach unique from boss',
        cost: 50,
        unlocked: false
      },
      {
        id: 'master',
        name: 'Genesis Transcendence',
        description: 'Upgrade Breachstone to Flawless',
        cost: 100,
        unlocked: false
      }
    ];

    setProgress(mockProgress);
    setNodes(mockNodes);
    setRecommendedPaths(mockPaths);
    setCrafts(mockCrafts);
  };

  const handleAddGraftblood = () => {
    const amount = parseInt(graftbloodInput);
    if (!isNaN(amount) && amount > 0 && progress) {
      setProgress({
        ...progress,
        graftbloodBalance: progress.graftbloodBalance + amount,
        totalEarned: progress.totalEarned + amount
      });
      setGraftbloodInput('');
    }
  };

  const handleUnlockNode = (nodeId: string) => {
    const node = nodes.find(n => n.id === nodeId);
    if (!node || !progress) return;

    if (node.canUnlock.canUnlock && progress.graftbloodBalance >= node.cost) {
      // Update node
      const updatedNodes = nodes.map(n =>
        n.id === nodeId ? { ...n, unlocked: true, canUnlock: { canUnlock: false, reason: 'Already unlocked' } } : n
      );
      setNodes(updatedNodes);

      // Update progress
      setProgress({
        ...progress,
        graftbloodBalance: progress.graftbloodBalance - node.cost,
        totalSpent: progress.totalSpent + node.cost,
        nodesUnlocked: progress.nodesUnlocked + 1,
        progressPercent: Math.round(((progress.nodesUnlocked + 1) / progress.totalNodes) * 100)
      });

      console.log(`Unlocked: ${node.name}`);
    }
  };

  const getNodesByType = (type: string) => {
    if (type === 'all') return nodes;
    return nodes.filter(n => n.type === type);
  };

  const getNodeColor = (type: string): string => {
    const colors: { [key: string]: string } = {
      core: '#ffd700',
      breach: '#9933ff',
      hive: '#ff4500',
      ailith: '#c084fc',
      reward: '#4ade80',
      unique: '#ff6b9d'
    };
    return colors[type] || '#c8aa6e';
  };

  const getTierColor = (tier: number): string => {
    const colors = ['#8b8b8b', '#ffffff', '#4ade80', '#c084fc', '#ffd700'];
    return colors[tier] || '#ffffff';
  };

  return (
    <div className="genesis-tree-tracker">
      <div className="genesis-tree-tracker__header">
        <div className="header-left">
          <h3>Genesis Tree</h3>
          <span className="league-tag">Keepers of the Flame</span>
        </div>
        {onClose && (
          <button className="close-btn" onClick={onClose}>×</button>
        )}
      </div>

      {progress && (
        <div className="genesis-progress-bar">
          <div className="progress-info">
            <span className="progress-label">Tree Progress</span>
            <span className="progress-value">{progress.nodesUnlocked}/{progress.totalNodes} nodes</span>
          </div>
          <div className="progress-track">
            <div
              className="progress-fill"
              style={{ width: `${progress.progressPercent}%` }}
            />
          </div>
          <div className="graftblood-display">
            <span className="graftblood-icon">🔮</span>
            <span className="graftblood-amount">{progress.graftbloodBalance}</span>
            <span className="graftblood-label">Graftblood</span>
          </div>
        </div>
      )}

      <div className="genesis-tree-tracker__tabs">
        <button
          className={`tab ${selectedTab === 'progress' ? 'active' : ''}`}
          onClick={() => setSelectedTab('progress')}
        >
          Progress
        </button>
        <button
          className={`tab ${selectedTab === 'nodes' ? 'active' : ''}`}
          onClick={() => setSelectedTab('nodes')}
        >
          Passive Nodes
        </button>
        <button
          className={`tab ${selectedTab === 'paths' ? 'active' : ''}`}
          onClick={() => setSelectedTab('paths')}
        >
          Recommended Paths
        </button>
        <button
          className={`tab ${selectedTab === 'crafts' ? 'active' : ''}`}
          onClick={() => setSelectedTab('crafts')}
        >
          Crafts
        </button>
      </div>

      <div className="genesis-tree-tracker__content">
        {selectedTab === 'progress' && progress && (
          <div className="progress-section">
            <div className="stats-overview">
              <div className="stat-card">
                <span className="stat-label">Current Tier</span>
                <span className="stat-value tier" style={{ color: getTierColor(progress.currentTier) }}>
                  Tier {progress.currentTier}
                </span>
              </div>
              <div className="stat-card">
                <span className="stat-label">Total Earned</span>
                <span className="stat-value graftblood">{progress.totalEarned}</span>
              </div>
              <div className="stat-card">
                <span className="stat-label">Total Spent</span>
                <span className="stat-value spent">{progress.totalSpent}</span>
              </div>
              <div className="stat-card">
                <span className="stat-label">Uniques Found</span>
                <span className="stat-value uniques">{progress.uniquesFound}</span>
              </div>
            </div>

            <div className="graftblood-input-section">
              <h4>Add Graftblood</h4>
              <div className="input-group">
                <input
                  type="number"
                  value={graftbloodInput}
                  onChange={(e) => setGraftbloodInput(e.target.value)}
                  placeholder="Amount..."
                  min="0"
                />
                <button onClick={handleAddGraftblood} className="add-btn">
                  Add
                </button>
              </div>
            </div>

            <div className="crafts-preview">
              <h4>Available Crafts</h4>
              {progress.availableCrafts.length > 0 ? (
                <div className="crafts-list">
                  {crafts.filter(c => c.unlocked).map(craft => (
                    <div key={craft.id} className="craft-item">
                      <span className="craft-name">{craft.name}</span>
                      <span className="craft-cost">{craft.cost} Graftblood</span>
                    </div>
                  ))}
                </div>
              ) : (
                <p className="no-crafts">Unlock 5 nodes to access Genesis crafts</p>
              )}
            </div>

            <div className="tier-info">
              <h4>Reward Tiers</h4>
              <div className="tiers-list">
                {[1, 2, 3, 4].map(tier => (
                  <div
                    key={tier}
                    className={`tier-item ${progress.tiersCompleted.includes(tier) ? 'completed' : ''}`}
                  >
                    <span className="tier-number" style={{ color: getTierColor(tier) }}>
                      Tier {tier}
                    </span>
                    {progress.currentTier >= tier && (
                      <span className="tier-status completed">✓</span>
                    )}
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}

        {selectedTab === 'nodes' && (
          <div className="nodes-section">
            <div className="node-filters">
              <button
                className={`filter-btn ${selectedType === 'all' ? 'active' : ''}`}
                onClick={() => setSelectedType('all')}
              >
                All
              </button>
              <button
                className={`filter-btn ${selectedType === 'core' ? 'active' : ''}`}
                onClick={() => setSelectedType('core')}
                style={{ color: getNodeColor('core') }}
              >
                Core
              </button>
              <button
                className={`filter-btn ${selectedType === 'breach' ? 'active' : ''}`}
                onClick={() => setSelectedType('breach')}
                style={{ color: getNodeColor('breach') }}
              >
                Breach
              </button>
              <button
                className={`filter-btn ${selectedType === 'hive' ? 'active' : ''}`}
                onClick={() => setSelectedType('hive')}
                style={{ color: getNodeColor('hive') }}
              >
                Hive
              </button>
              <button
                className={`filter-btn ${selectedType === 'ailith' ? 'active' : ''}`}
                onClick={() => setSelectedType('ailith')}
                style={{ color: getNodeColor('ailith') }}
              >
                Ailith
              </button>
              <button
                className={`filter-btn ${selectedType === 'reward' ? 'active' : ''}`}
                onClick={() => setSelectedType('reward')}
                style={{ color: getNodeColor('reward') }}
              >
                Reward
              </button>
            </div>

            <div className="nodes-list">
              {getNodesByType(selectedType).map(node => (
                <div
                  key={node.id}
                  className={`node-card ${node.unlocked ? 'unlocked' : ''} ${node.canUnlock.canUnlock ? 'available' : ''}`}
                  style={{ borderLeftColor: getNodeColor(node.type) }}
                >
                  <div className="node-header">
                    <h5 style={{ color: getNodeColor(node.type) }}>{node.name}</h5>
                    <span className="node-tier" style={{ color: getTierColor(node.tier) }}>
                      T{node.tier}
                    </span>
                  </div>
                  <p className="node-description">{node.description}</p>
                  <div className="node-footer">
                    <span className="node-cost">{node.cost} Graftblood</span>
                    {!node.unlocked && (
                      <button
                        className="unlock-btn"
                        onClick={() => handleUnlockNode(node.id)}
                        disabled={!node.canUnlock.canUnlock}
                      >
                        {node.canUnlock.canUnlock ? 'Unlock' : 'Locked'}
                      </button>
                    )}
                    {node.unlocked && (
                      <span className="unlocked-badge">Unlocked</span>
                    )}
                  </div>
                  {!node.canUnlock.canUnlock && !node.unlocked && node.canUnlock.reason && (
                    <div className="node-requirement">{node.canUnlock.reason}</div>
                  )}
                </div>
              ))}
            </div>
          </div>
        )}

        {selectedTab === 'paths' && (
          <div className="paths-section">
            <h4>Recommended Passive Paths</h4>
            <div className="paths-list">
              {Object.entries(recommendedPaths).map(([key, path]) => (
                <div key={key} className="path-card">
                  <h5>{path.name}</h5>
                  <p className="path-description">{path.description}</p>
                  <div className="path-nodes">
                    <span className="path-label">Path:</span>
                    <div className="node-sequence">
                      {path.path.map((nodeId, index) => {
                        const node = nodes.find(n => n.id === nodeId);
                        return (
                          <span key={nodeId} className="path-node">
                            {node?.name || nodeId}
                            {index < path.path.length - 1 && <span className="arrow">→</span>}
                          </span>
                        );
                      })}
                    </div>
                  </div>
                  <div className="path-benefits">
                    <span className="benefits-label">Benefits:</span>
                    <span className="benefits-text">{path.benefits}</span>
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}

        {selectedTab === 'crafts' && (
          <div className="crafts-section">
            <h4>Genesis Crafts</h4>
            <div className="crafts-grid">
              {crafts.map(craft => (
                <div
                  key={craft.id}
                  className={`craft-card ${craft.unlocked ? 'unlocked' : 'locked'}`}
                >
                  <h5>{craft.name}</h5>
                  <p className="craft-description">{craft.description}</p>
                  <div className="craft-footer">
                    <span className="craft-cost">{craft.cost} Graftblood</span>
                    {craft.unlocked ? (
                      <span className="craft-status unlocked">Unlocked</span>
                    ) : (
                      <span className="craft-status locked">Locked</span>
                    )}
                  </div>
                </div>
              ))}
            </div>
            <div className="craft-unlock-info">
              <p>Unlock crafts by allocating passive nodes:</p>
              <ul>
                <li>5 nodes: Basic craft</li>
                <li>10 nodes: Advanced craft</li>
                <li>15 nodes: Master craft</li>
              </ul>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default GenesisTreeTracker;
