import React, { useState, useEffect } from 'react';
import './AtlasTracker.scss';

interface MavenInvitation {
  completed: boolean;
  region: string;
}

interface UberBoss {
  killed: boolean;
  attempts: number;
}

interface MapTier {
  completed: number;
  total: number;
  bonus: number;
  awakened: number;
}

interface AtlasProgression {
  voidstones: {
    total: number;
    collected: number[];
  };
  maven: {
    invitations: { [key: string]: MavenInvitation };
    witnessedBosses: number;
    totalBosses: number;
  };
  uberBosses: { [key: string]: UberBoss };
  maps: {
    white: MapTier;
    yellow: MapTier;
    red: MapTier;
  };
  atlasPassives: {
    earned: number;
    allocated: number;
    total: number;
  };
  favoriteMapSlots: {
    unlocked: number;
    total: number;
  };
  voidborn: {
    encountersCompleted: number;
    keysUsed: number;
    uniqueBossesDefeated: string[];
  };
  completion: {
    overallAtlas: number;
    mapCompletion: number;
    bossKills: number;
    mavenProgress: number;
  };
}

interface AtlasTrackerProps {
  onClose?: () => void;
}

const AtlasTracker: React.FC<AtlasTrackerProps> = ({ onClose }) => {
  const [progression, setProgression] = useState<AtlasProgression | null>(null);
  const [loading, setLoading] = useState(true);
  const [activeTab, setActiveTab] = useState<'overview' | 'maps' | 'bosses' | 'maven'>('overview');

  useEffect(() => {
    loadProgression();
  }, []);

  const loadProgression = () => {
    try {
      setLoading(true);

      // Mock data - in production would use IPC to call the service
      const mockProgression: AtlasProgression = {
        voidstones: {
          total: 4,
          collected: [0, 1],
        },
        maven: {
          invitations: {
            'The Formed': { completed: true, region: 'Haewark Hamlet' },
            'The Twisted': { completed: false, region: "Tirn's End" },
            'The Forgotten': { completed: true, region: 'Lex Proxima' },
            'The Hidden': { completed: false, region: 'Lex Ejoris' },
            'The Feared': { completed: false, region: "Valdo's Rest" },
            'The Elderslayers': { completed: false, region: 'Lira Arthain' },
            "The Maven's Writ": { completed: false, region: 'Special' },
          },
          witnessedBosses: 18,
          totalBosses: 30,
        },
        uberBosses: {
          'The Maven': { killed: true, attempts: 3 },
          'Uber Maven': { killed: false, attempts: 2 },
          'The Searing Exarch': { killed: true, attempts: 1 },
          'The Eater of Worlds': { killed: true, attempts: 2 },
          'Uber Searing Exarch': { killed: false, attempts: 1 },
          'Uber Eater of Worlds': { killed: false, attempts: 0 },
          'Uber Elder': { killed: true, attempts: 4 },
          'The Shaper': { killed: true, attempts: 1 },
          'Uber Shaper': { killed: false, attempts: 0 },
          'Sirus': { killed: true, attempts: 5 },
          'The Cortex': { killed: false, attempts: 0 },
          'The Feared (all unique)': { killed: false, attempts: 1 },
        },
        maps: {
          white: { completed: 42, total: 50, bonus: 38, awakened: 35 },
          yellow: { completed: 28, total: 40, bonus: 24, awakened: 20 },
          red: { completed: 15, total: 45, bonus: 10, awakened: 8 },
        },
        atlasPassives: {
          earned: 85,
          allocated: 80,
          total: 132,
        },
        favoriteMapSlots: {
          unlocked: 8,
          total: 12,
        },
        voidborn: {
          encountersCompleted: 12,
          keysUsed: 3,
          uniqueBossesDefeated: ['Voidborn Sentinel', 'Eclipse Guardian'],
        },
        completion: {
          overallAtlas: 58.5,
          mapCompletion: 63.0,
          bossKills: 41.7,
          mavenProgress: 28.6,
        },
      };

      setProgression(mockProgression);
      setLoading(false);
    } catch (error) {
      console.error('Error loading progression:', error);
      setLoading(false);
    }
  };

  const toggleVoidstone = (index: number) => {
    if (!progression) return;

    const newCollected = progression.voidstones.collected.includes(index)
      ? progression.voidstones.collected.filter((i) => i !== index)
      : [...progression.voidstones.collected, index].sort();

    setProgression({
      ...progression,
      voidstones: { ...progression.voidstones, collected: newCollected },
    });
  };

  const toggleBossKill = (bossName: string) => {
    if (!progression) return;

    setProgression({
      ...progression,
      uberBosses: {
        ...progression.uberBosses,
        [bossName]: {
          ...progression.uberBosses[bossName],
          killed: !progression.uberBosses[bossName].killed,
        },
      },
    });
  };

  const toggleMavenInvitation = (invitationName: string) => {
    if (!progression) return;

    setProgression({
      ...progression,
      maven: {
        ...progression.maven,
        invitations: {
          ...progression.maven.invitations,
          [invitationName]: {
            ...progression.maven.invitations[invitationName],
            completed: !progression.maven.invitations[invitationName].completed,
          },
        },
      },
    });
  };

  if (loading || !progression) {
    return (
      <div className="atlas-tracker">
        <div className="atlas-tracker__loading">Loading Atlas progression...</div>
      </div>
    );
  }

  const totalMaps = progression.maps.white.total + progression.maps.yellow.total + progression.maps.red.total;
  const completedMaps = progression.maps.white.completed + progression.maps.yellow.completed + progression.maps.red.completed;

  return (
    <div className="atlas-tracker">
      <div className="atlas-tracker__header">
        <h3>Atlas Progression Tracker</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>
            ×
          </button>
        )}
      </div>

      <div className="atlas-tracker__completion">
        <div className="completion-circle">
          <svg viewBox="0 0 200 200">
            <circle cx="100" cy="100" r="90" className="progress-bg" />
            <circle
              cx="100"
              cy="100"
              r="90"
              className="progress-fill"
              strokeDasharray={`${progression.completion.overallAtlas * 5.65} 565`}
            />
          </svg>
          <div className="completion-text">
            <span className="percentage">{Math.round(progression.completion.overallAtlas)}%</span>
            <span className="label">Complete</span>
          </div>
        </div>
      </div>

      <div className="atlas-tracker__tabs">
        <button className={activeTab === 'overview' ? 'active' : ''} onClick={() => setActiveTab('overview')}>
          Overview
        </button>
        <button className={activeTab === 'maps' ? 'active' : ''} onClick={() => setActiveTab('maps')}>
          Maps
        </button>
        <button className={activeTab === 'bosses' ? 'active' : ''} onClick={() => setActiveTab('bosses')}>
          Bosses
        </button>
        <button className={activeTab === 'maven' ? 'active' : ''} onClick={() => setActiveTab('maven')}>
          Maven
        </button>
      </div>

      <div className="atlas-tracker__content">
        {activeTab === 'overview' && (
          <div className="overview-tab">
            <div className="section">
              <h4>Voidstones ({progression.voidstones.collected.length}/{progression.voidstones.total})</h4>
              <div className="voidstones">
                {[0, 1, 2, 3].map((index) => (
                  <div
                    key={index}
                    className={`voidstone ${progression.voidstones.collected.includes(index) ? 'collected' : ''}`}
                    onClick={() => toggleVoidstone(index)}
                  >
                    {progression.voidstones.collected.includes(index) ? '✓' : index + 1}
                  </div>
                ))}
              </div>
            </div>

            <div className="section">
              <h4>Map Completion</h4>
              <div className="progress-bar-container">
                <div className="progress-bar">
                  <div className="progress-fill" style={{ width: `${progression.completion.mapCompletion}%` }} />
                </div>
                <span className="progress-label">
                  {completedMaps}/{totalMaps} Maps ({Math.round(progression.completion.mapCompletion)}%)
                </span>
              </div>
            </div>

            <div className="section">
              <h4>Boss Completion</h4>
              <div className="progress-bar-container">
                <div className="progress-bar">
                  <div className="progress-fill" style={{ width: `${progression.completion.bossKills}%` }} />
                </div>
                <span className="progress-label">
                  {Object.values(progression.uberBosses).filter((b) => b.killed).length}/
                  {Object.keys(progression.uberBosses).length} Bosses ({Math.round(progression.completion.bossKills)}%)
                </span>
              </div>
            </div>

            <div className="section">
              <h4>Atlas Passives</h4>
              <div className="progress-bar-container">
                <div className="progress-bar">
                  <div
                    className="progress-fill"
                    style={{ width: `${(progression.atlasPassives.earned / progression.atlasPassives.total) * 100}%` }}
                  />
                </div>
                <span className="progress-label">
                  {progression.atlasPassives.earned}/{progression.atlasPassives.total} Points Earned
                </span>
              </div>
            </div>

            <div className="section">
              <h4>3.27 Voidborn Content</h4>
              <div className="voidborn-stats">
                <div className="stat">
                  <span className="stat-value">{progression.voidborn.encountersCompleted}</span>
                  <span className="stat-label">Encounters</span>
                </div>
                <div className="stat">
                  <span className="stat-value">{progression.voidborn.keysUsed}</span>
                  <span className="stat-label">Keys Used</span>
                </div>
                <div className="stat">
                  <span className="stat-value">{progression.voidborn.uniqueBossesDefeated.length}</span>
                  <span className="stat-label">Unique Bosses</span>
                </div>
              </div>
            </div>
          </div>
        )}

        {activeTab === 'maps' && (
          <div className="maps-tab">
            {(['white', 'yellow', 'red'] as const).map((tier) => (
              <div key={tier} className={`map-tier tier-${tier}`}>
                <h4>{tier.charAt(0).toUpperCase() + tier.slice(1)} Maps</h4>
                <div className="tier-stats">
                  <div className="stat-row">
                    <span>Completed:</span>
                    <span>
                      {progression.maps[tier].completed}/{progression.maps[tier].total}
                    </span>
                  </div>
                  <div className="stat-row">
                    <span>Bonus Objectives:</span>
                    <span>
                      {progression.maps[tier].bonus}/{progression.maps[tier].total}
                    </span>
                  </div>
                  <div className="stat-row">
                    <span>Awakened Bonus:</span>
                    <span>
                      {progression.maps[tier].awakened}/{progression.maps[tier].total}
                    </span>
                  </div>
                </div>
                <div className="progress-bar-container">
                  <div className="progress-bar">
                    <div
                      className="progress-fill"
                      style={{ width: `${(progression.maps[tier].completed / progression.maps[tier].total) * 100}%` }}
                    />
                  </div>
                </div>
              </div>
            ))}

            <div className="section">
              <h4>Favorite Map Slots</h4>
              <div className="favorite-slots">
                {Array.from({ length: progression.favoriteMapSlots.total }).map((_, index) => (
                  <div
                    key={index}
                    className={`slot ${index < progression.favoriteMapSlots.unlocked ? 'unlocked' : 'locked'}`}
                  >
                    {index < progression.favoriteMapSlots.unlocked ? '★' : '☆'}
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}

        {activeTab === 'bosses' && (
          <div className="bosses-tab">
            <div className="boss-list">
              {Object.entries(progression.uberBosses).map(([name, data]) => (
                <div key={name} className={`boss-item ${data.killed ? 'killed' : ''}`} onClick={() => toggleBossKill(name)}>
                  <div className="boss-checkbox">{data.killed ? '✓' : ''}</div>
                  <div className="boss-info">
                    <span className="boss-name">{name}</span>
                    <span className="boss-attempts">{data.attempts} attempt(s)</span>
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}

        {activeTab === 'maven' && (
          <div className="maven-tab">
            <div className="section">
              <h4>Witnessed Bosses</h4>
              <div className="progress-bar-container">
                <div className="progress-bar">
                  <div
                    className="progress-fill"
                    style={{ width: `${(progression.maven.witnessedBosses / progression.maven.totalBosses) * 100}%` }}
                  />
                </div>
                <span className="progress-label">
                  {progression.maven.witnessedBosses}/{progression.maven.totalBosses}
                </span>
              </div>
            </div>

            <div className="section">
              <h4>Maven Invitations</h4>
              <div className="invitation-list">
                {Object.entries(progression.maven.invitations).map(([name, data]) => (
                  <div
                    key={name}
                    className={`invitation-item ${data.completed ? 'completed' : ''}`}
                    onClick={() => toggleMavenInvitation(name)}
                  >
                    <div className="invitation-checkbox">{data.completed ? '✓' : ''}</div>
                    <div className="invitation-info">
                      <span className="invitation-name">{name}</span>
                      <span className="invitation-region">{data.region}</span>
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default AtlasTracker;
