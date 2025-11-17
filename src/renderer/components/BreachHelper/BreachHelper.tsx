import React, { useState, useEffect } from 'react';
import './BreachHelper.scss';

interface BreachType {
  id: string;
  name: string;
  color: string;
  boss: string;
  splinter: string;
  element: string;
  ailithProtection: string;
  hiveLocation: string;
  hiveborn: string;
  strategies: string[];
}

interface HiveProgress {
  type: string;
  name: string;
  boss: string;
  completed: boolean;
  color: string;
}

interface SessionStats {
  breachesCompleted: number;
  totalSplinters: number;
  splintersByType: {
    tul: number;
    xoph: number;
    esh: number;
    'uul-netol': number;
    chayula: number;
  };
  graftbloodEarned: number;
  hivesCompleted: number;
  hiveProgress: HiveProgress[];
  ailithActive: boolean;
}

interface BreachHelperProps {
  onClose?: () => void;
}

const BreachHelper: React.FC<BreachHelperProps> = ({ onClose }) => {
  const [breachTypes, setBreachTypes] = useState<BreachType[]>([]);
  const [sessionStats, setSessionStats] = useState<SessionStats | null>(null);
  const [selectedBreach, setSelectedBreach] = useState<string>('');
  const [activeTab, setActiveTab] = useState<'overview' | 'hives' | 'splinters'>('overview');
  const [ailithActive, setAilithActive] = useState(false);
  const [unstableTimer, setUnstableTimer] = useState<number | null>(null);

  useEffect(() => {
    // In a real implementation, this would communicate with the breach helper service
    // via Electron IPC. For now, using mock data.
    loadBreachData();

    // Set up timer update for unstable breaches
    const interval = setInterval(() => {
      updateUnstableTimer();
    }, 1000);

    return () => clearInterval(interval);
  }, []);

  const loadBreachData = () => {
    // Mock data - would come from breachHelper service
    const mockBreachTypes: BreachType[] = [
      {
        id: 'cold',
        name: 'Breach of Tul',
        color: '#00ffff',
        boss: 'Tul, Creeping Avalanche',
        splinter: 'Splinter of Tul',
        element: 'Cold',
        ailithProtection: 'Cold Resistance & Freeze Immunity',
        hiveLocation: 'Glacial Expanse',
        hiveborn: 'Frostbound Sentinel',
        strategies: [
          'Bring freeze immunity or reduced freeze duration',
          'Stack cold resistance',
          'Move constantly to avoid ice patches',
          'Use AoE for dense packs'
        ]
      },
      {
        id: 'fire',
        name: 'Breach of Xoph',
        color: '#ff4500',
        boss: 'Xoph, Dark Embers',
        splinter: 'Splinter of Xoph',
        element: 'Fire',
        ailithProtection: 'Fire Resistance',
        hiveLocation: 'Volcanic Depths',
        hiveborn: 'Emberborn Tyrant',
        strategies: [
          'Cap fire resistance',
          'Watch for burning ground',
          'Clear monster packs quickly',
          'Prioritize rare monsters'
        ]
      },
      {
        id: 'lightning',
        name: 'Breach of Esh',
        color: '#ffff00',
        boss: 'Esh, Forked Thought',
        splinter: 'Splinter of Esh',
        element: 'Lightning',
        ailithProtection: 'Lightning Resistance & Shock Immunity',
        hiveLocation: 'Storm Nexus',
        hiveborn: 'Voltaic Archon',
        strategies: [
          'Cap lightning resistance',
          'Get shock immunity if possible',
          'Watch for lightning mirages',
          'Clear in wide circular patterns'
        ]
      },
      {
        id: 'uulNetol',
        name: 'Breach of Uul-Netol',
        color: '#8b0000',
        boss: 'Uul-Netol, Unburdened Flesh',
        splinter: 'Splinter of Uul-Netol',
        element: 'Physical',
        ailithProtection: 'Armour & Physical Mitigation',
        hiveLocation: 'Crimson Halls',
        hiveborn: 'Fleshbound Colossus',
        strategies: [
          'Build armour or endurance charges',
          'Watch for slam attacks',
          'Kite around breach perimeter',
          'Save movement skills for emergencies'
        ]
      },
      {
        id: 'chaos',
        name: 'Breach of Chayula',
        color: '#9933ff',
        boss: 'Chayula, Who Dreamt',
        splinter: 'Splinter of Chayula',
        element: 'Chaos',
        ailithProtection: 'Chaos Resistance & Cannot be Hindered',
        hiveLocation: 'Void Sanctum',
        hiveborn: 'Voidborn Hegemon',
        strategies: [
          'Stack chaos resistance (75%+)',
          'Use movement skills frequently',
          'Most valuable splinters - prioritize rares',
          'Watch for void zones'
        ]
      }
    ];

    const mockStats: SessionStats = {
      breachesCompleted: 12,
      totalSplinters: 48,
      splintersByType: {
        tul: 15,
        xoph: 10,
        esh: 8,
        'uul-netol': 12,
        chayula: 3
      },
      graftbloodEarned: 145,
      hivesCompleted: 2,
      hiveProgress: [
        { type: 'cold', name: 'Glacial Expanse', boss: 'Frostbound Sentinel', completed: true, color: '#00ffff' },
        { type: 'fire', name: 'Volcanic Depths', boss: 'Emberborn Tyrant', completed: true, color: '#ff4500' },
        { type: 'lightning', name: 'Storm Nexus', boss: 'Voltaic Archon', completed: false, color: '#ffff00' },
        { type: 'uulNetol', name: 'Crimson Halls', boss: 'Fleshbound Colossus', completed: false, color: '#8b0000' },
        { type: 'chaos', name: 'Void Sanctum', boss: 'Voidborn Hegemon', completed: false, color: '#9933ff' }
      ],
      ailithActive: false
    };

    setBreachTypes(mockBreachTypes);
    setSessionStats(mockStats);
    if (mockBreachTypes.length > 0) {
      setSelectedBreach(mockBreachTypes[0].id);
    }
  };

  const updateUnstableTimer = () => {
    // Would check actual timer from service
    // For demo purposes, just decrement if active
    if (unstableTimer !== null && unstableTimer > 0) {
      setUnstableTimer(unstableTimer - 1);
    }
  };

  const toggleAilithProtection = () => {
    setAilithActive(!ailithActive);
    // Would call service method here
  };

  const formatSplinterProgress = (count: number, required: number = 100) => {
    const percent = Math.min(100, (count / required) * 100);
    return { count, required, percent };
  };

  const getBreachstoneStatus = (splinterCount: number) => {
    if (splinterCount >= 100) {
      return { ready: true, remaining: 0 };
    }
    return { ready: false, remaining: 100 - splinterCount };
  };

  const selectedBreachData = breachTypes.find(b => b.id === selectedBreach);

  return (
    <div className="breach-helper">
      <div className="breach-helper__header">
        <h3>Breach Helper</h3>
        <div className="header-info">
          <span className="league-tag">Keepers of the Flame</span>
          {onClose && (
            <button className="close-btn" onClick={onClose}>×</button>
          )}
        </div>
      </div>

      <div className="breach-helper__tabs">
        <button
          className={`tab ${activeTab === 'overview' ? 'active' : ''}`}
          onClick={() => setActiveTab('overview')}
        >
          Overview
        </button>
        <button
          className={`tab ${activeTab === 'hives' ? 'active' : ''}`}
          onClick={() => setActiveTab('hives')}
        >
          Breach Hives
        </button>
        <button
          className={`tab ${activeTab === 'splinters' ? 'active' : ''}`}
          onClick={() => setActiveTab('splinters')}
        >
          Splinters
        </button>
      </div>

      {activeTab === 'overview' && (
        <div className="breach-helper__content">
          <div className="session-stats">
            <h4>Session Statistics</h4>
            {sessionStats && (
              <div className="stats-grid">
                <div className="stat-card">
                  <span className="stat-label">Breaches Completed</span>
                  <span className="stat-value">{sessionStats.breachesCompleted}</span>
                </div>
                <div className="stat-card">
                  <span className="stat-label">Total Splinters</span>
                  <span className="stat-value">{sessionStats.totalSplinters}</span>
                </div>
                <div className="stat-card">
                  <span className="stat-label">Graftblood Earned</span>
                  <span className="stat-value graftblood">{sessionStats.graftbloodEarned}</span>
                </div>
                <div className="stat-card">
                  <span className="stat-label">Hives Completed</span>
                  <span className="stat-value">{sessionStats.hivesCompleted}/5</span>
                </div>
              </div>
            )}
          </div>

          <div className="ailith-protection">
            <h4>Ailith Protection</h4>
            <div className="protection-control">
              <button
                className={`protection-toggle ${ailithActive ? 'active' : ''}`}
                onClick={toggleAilithProtection}
              >
                {ailithActive ? 'Protection Active' : 'Protection Inactive'}
              </button>
              {ailithActive && selectedBreachData && (
                <div className="protection-info">
                  <span className="protection-type" style={{ color: selectedBreachData.color }}>
                    {selectedBreachData.ailithProtection}
                  </span>
                </div>
              )}
            </div>
          </div>

          <div className="breach-selector">
            <h4>Select Breach Type</h4>
            <div className="breach-buttons">
              {breachTypes.map(breach => (
                <button
                  key={breach.id}
                  className={`breach-btn ${selectedBreach === breach.id ? 'selected' : ''}`}
                  style={{
                    borderColor: breach.color,
                    backgroundColor: selectedBreach === breach.id ? `${breach.color}20` : 'transparent'
                  }}
                  onClick={() => setSelectedBreach(breach.id)}
                >
                  <span className="breach-element" style={{ color: breach.color }}>
                    {breach.element}
                  </span>
                </button>
              ))}
            </div>
          </div>

          {selectedBreachData && (
            <div className="breach-details">
              <div className="breach-header" style={{ borderLeftColor: selectedBreachData.color }}>
                <h4 style={{ color: selectedBreachData.color }}>{selectedBreachData.name}</h4>
                <span className="breach-boss">{selectedBreachData.boss}</span>
              </div>

              <div className="breach-info-section">
                <h5>Strategies</h5>
                <ul className="strategy-list">
                  {selectedBreachData.strategies.map((strategy, index) => (
                    <li key={index}>{strategy}</li>
                  ))}
                </ul>
              </div>

              <div className="breach-rewards">
                <div className="reward-item">
                  <span className="reward-label">Splinter:</span>
                  <span className="reward-value">{selectedBreachData.splinter}</span>
                </div>
                <div className="reward-item">
                  <span className="reward-label">Hive Location:</span>
                  <span className="reward-value">{selectedBreachData.hiveLocation}</span>
                </div>
                <div className="reward-item">
                  <span className="reward-label">Hiveborn Boss:</span>
                  <span className="reward-value">{selectedBreachData.hiveborn}</span>
                </div>
              </div>
            </div>
          )}

          {unstableTimer !== null && unstableTimer > 0 && (
            <div className="unstable-breach-timer">
              <span className="timer-label">Unstable Breach Timer:</span>
              <span className="timer-value">{unstableTimer}s</span>
            </div>
          )}
        </div>
      )}

      {activeTab === 'hives' && sessionStats && (
        <div className="breach-helper__content">
          <div className="hives-section">
            <h4>Breach Hive Completion</h4>
            <div className="hive-list">
              {sessionStats.hiveProgress.map(hive => (
                <div
                  key={hive.type}
                  className={`hive-card ${hive.completed ? 'completed' : ''}`}
                  style={{ borderLeftColor: hive.color }}
                >
                  <div className="hive-info">
                    <h5 style={{ color: hive.color }}>{hive.name}</h5>
                    <span className="hive-boss">{hive.boss}</span>
                  </div>
                  <div className="hive-status">
                    {hive.completed ? (
                      <span className="status-completed">Completed</span>
                    ) : (
                      <span className="status-pending">Pending</span>
                    )}
                  </div>
                </div>
              ))}
            </div>
          </div>
        </div>
      )}

      {activeTab === 'splinters' && sessionStats && (
        <div className="breach-helper__content">
          <div className="splinters-section">
            <h4>Splinter Progress</h4>
            <div className="splinter-list">
              {Object.entries(sessionStats.splintersByType).map(([type, count]) => {
                const progress = formatSplinterProgress(count);
                const status = getBreachstoneStatus(count);
                const breach = breachTypes.find(b => b.id === type || b.splinter.toLowerCase().includes(type));
                const color = breach?.color || '#ffffff';

                return (
                  <div key={type} className="splinter-card">
                    <div className="splinter-header">
                      <span className="splinter-name" style={{ color }}>
                        {type.charAt(0).toUpperCase() + type.slice(1).replace('-', ' ')}
                      </span>
                      <span className="splinter-count">
                        {count}/100
                      </span>
                    </div>
                    <div className="splinter-progress">
                      <div
                        className="progress-bar"
                        style={{
                          width: `${progress.percent}%`,
                          backgroundColor: color
                        }}
                      />
                    </div>
                    <div className="splinter-status">
                      {status.ready ? (
                        <span className="status-ready">Ready for Breachstone!</span>
                      ) : (
                        <span className="status-remaining">{status.remaining} more needed</span>
                      )}
                    </div>
                  </div>
                );
              })}
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default BreachHelper;
