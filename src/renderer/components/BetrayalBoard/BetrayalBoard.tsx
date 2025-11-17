import React, { useState, useEffect } from 'react';
import './BetrayalBoard.scss';

interface SyndicateMember {
  name: string;
  division: string;
  rank: number;
  relationships: {
    trusted: string[];
    rival: string[];
  };
}

interface MemberReward {
  name: string;
  rank3Reward: string;
  value: string;
  notes: string;
  priority: string;
}

interface BetrayalBoardProps {
  onClose?: () => void;
}

const BetrayalBoard: React.FC<BetrayalBoardProps> = ({ onClose }) => {
  const [activeTab, setActiveTab] = useState<'tracker' | 'rewards' | 'cheatsheet' | 'mechanics'>('tracker');
  const [selectedDivision, setSelectedDivision] = useState<string>('research');
  const [boardState, setBoardState] = useState<{ [key: string]: SyndicateMember[] }>({
    research: [],
    transportation: [],
    fortification: [],
    intervention: []
  });
  const [quickReference, setQuickReference] = useState<any[]>([]);
  const [mechanics, setMechanics] = useState<any>({});

  useEffect(() => {
    loadBetrayalData();
  }, []);

  const loadBetrayalData = () => {
    // Mock data - in real implementation, load from betrayalService via IPC
    setQuickReference([
      {
        name: 'It That Fled',
        bestDivision: 'Research Captain/Leader',
        priority: 'S-Tier',
        strategy: 'Always put in Research at Rank 3. Upgrade Breachstones for huge profit.'
      },
      {
        name: 'Vorici',
        bestDivision: 'Research or Transportation Rank 3',
        priority: 'S-Tier',
        strategy: 'Research Rank 3 for white sockets. Essential for off-color builds.'
      },
      {
        name: 'Aisling',
        bestDivision: 'Research Rank 3',
        priority: 'A-Tier',
        strategy: 'Research Rank 3 only. Removes 1 crafted mod and adds 2 veiled mods.'
      },
      {
        name: 'Hillock',
        bestDivision: 'Transportation/Fortification Rank 3',
        priority: 'A-Tier',
        strategy: 'Fortification Rank 3 for 30% quality armour. Essential for CI builds.'
      },
      {
        name: 'Cameria',
        bestDivision: 'Intervention Rank 3',
        priority: 'A-Tier',
        strategy: 'Intervention Rank 3 for Harbinger scarabs'
      }
    ]);

    setMechanics({
      'Execute': 'Kill member - drops items, removes from board',
      'Bargain': 'Member reveals info about other members, stays on board',
      'Interrogate': 'Remove member from board, gain intelligence',
      'Betray': 'Member betrays their division, swaps divisions',
      'Trusted': 'Green line - members will help each other',
      'Rival': 'Red line - members will fight each other',
      'Rank Up': 'Execute/Interrogate a rival to rank up',
      'Intelligence': 'Fill bar to run safehouse'
    });
  };

  const divisions = ['research', 'transportation', 'fortification', 'intervention'];

  const divisionRewards: { [key: string]: MemberReward[] } = {
    research: [
      {
        name: 'It That Fled',
        rank3Reward: 'Upgrade Breachstone to Pure',
        value: 'EXTREME',
        notes: 'Pure Breachstones are 3-5 divine each!',
        priority: 'S-Tier'
      },
      {
        name: 'Vorici',
        rank3Reward: '1-6 White Sockets',
        value: 'EXTREME',
        notes: 'Can roll 1-6 white sockets on any item!',
        priority: 'S-Tier'
      },
      {
        name: 'Aisling',
        rank3Reward: 'Remove crafted mod, add 2 Veiled mods',
        value: 'Very High',
        notes: 'Essential for metacrafting',
        priority: 'A-Tier'
      }
    ],
    transportation: [
      {
        name: 'Hillock',
        rank3Reward: '28% Quality on Weapon',
        value: 'Very High',
        notes: '28% quality weapons!',
        priority: 'A-Tier'
      },
      {
        name: 'Vorici',
        rank3Reward: '3 White Sockets',
        value: 'Very High',
        notes: 'White sockets on any item!',
        priority: 'S-Tier'
      }
    ],
    fortification: [
      {
        name: 'Hillock',
        rank3Reward: '30% Quality on Armour',
        value: 'Very High',
        notes: '30% quality armour - best for ES gear!',
        priority: 'A-Tier'
      }
    ],
    intervention: [
      {
        name: 'Cameria',
        rank3Reward: '3 Harbinger Scarabs',
        value: 'High',
        notes: 'Very profitable for Harbinger farming',
        priority: 'A-Tier'
      },
      {
        name: 'Vagan',
        rank3Reward: '3 Legion Scarabs',
        value: 'High',
        notes: 'Scarabs for Legion farming',
        priority: 'B-Tier'
      }
    ]
  };

  const getPriorityColor = (priority: string) => {
    switch (priority) {
      case 'S-Tier': return '#ff1744';
      case 'A-Tier': return '#ff9800';
      case 'B-Tier': return '#ffc107';
      case 'C-Tier': return '#4caf50';
      case 'D-Tier': return '#666';
      default: return '#fff';
    }
  };

  const getValueColor = (value: string) => {
    switch (value) {
      case 'EXTREME': return '#ff1744';
      case 'Very High': return '#ff9800';
      case 'High': return '#ffc107';
      case 'Medium': return '#4caf50';
      case 'Low': return '#666';
      default: return '#fff';
    }
  };

  const renderTracker = () => {
    return (
      <div className="board-tracker">
        <div className="tracker-info">
          <p>Track your current syndicate board state and get recommendations for optimal rewards.</p>
          <div className="board-tip">
            Tip: Focus on getting high-priority members (S-Tier and A-Tier) to Rank 3 in their best divisions.
          </div>
        </div>

        <div className="division-selector">
          {divisions.map(div => (
            <button
              key={div}
              className={`division-btn ${selectedDivision === div ? 'active' : ''}`}
              onClick={() => setSelectedDivision(div)}
            >
              {div.charAt(0).toUpperCase() + div.slice(1)}
            </button>
          ))}
        </div>

        <div className="division-board">
          <h4>{selectedDivision.charAt(0).toUpperCase() + selectedDivision.slice(1)} Division</h4>

          <div className="best-members">
            <div className="section-header">Best Members for {selectedDivision}:</div>
            {divisionRewards[selectedDivision].map((member, index) => (
              <div key={index} className="member-card">
                <div className="member-header">
                  <span className="member-name">{member.name}</span>
                  <span
                    className="member-priority"
                    style={{ backgroundColor: getPriorityColor(member.priority) }}
                  >
                    {member.priority}
                  </span>
                </div>
                <div className="member-reward">
                  <strong>Rank 3:</strong> {member.rank3Reward}
                </div>
                <div className="member-value" style={{ color: getValueColor(member.value) }}>
                  Value: {member.value}
                </div>
                {member.notes && (
                  <div className="member-notes">{member.notes}</div>
                )}
              </div>
            ))}
          </div>
        </div>
      </div>
    );
  };

  const renderRewards = () => {
    return (
      <div className="rewards-guide">
        <div className="rewards-info">
          <p>Complete reward breakdown for all syndicate members by division and rank.</p>
        </div>

        <div className="rewards-grid">
          {divisions.map(division => (
            <div key={division} className="division-rewards">
              <h4 className="division-title">
                {division.charAt(0).toUpperCase() + division.slice(1)}
              </h4>
              <div className="reward-list">
                {divisionRewards[division].map((member, index) => (
                  <div key={index} className="reward-card">
                    <div className="reward-member">
                      <span>{member.name}</span>
                      <span
                        className="priority-badge"
                        style={{ backgroundColor: getPriorityColor(member.priority) }}
                      >
                        {member.priority}
                      </span>
                    </div>
                    <div className="reward-text">{member.rank3Reward}</div>
                    <div className="reward-value" style={{ color: getValueColor(member.value) }}>
                      {member.value}
                    </div>
                  </div>
                ))}
              </div>
            </div>
          ))}
        </div>
      </div>
    );
  };

  const renderCheatsheet = () => {
    return (
      <div className="cheatsheet">
        <div className="cheatsheet-info">
          <p>Quick reference guide for syndicate member priorities and optimal positions.</p>
        </div>

        <div className="priority-legend">
          <div className="legend-title">Priority Tiers:</div>
          <div className="legend-items">
            <span className="legend-item" style={{ color: getPriorityColor('S-Tier') }}>
              S-Tier: Must have
            </span>
            <span className="legend-item" style={{ color: getPriorityColor('A-Tier') }}>
              A-Tier: Very valuable
            </span>
            <span className="legend-item" style={{ color: getPriorityColor('B-Tier') }}>
              B-Tier: Good
            </span>
            <span className="legend-item" style={{ color: getPriorityColor('C-Tier') }}>
              C-Tier: Situational
            </span>
            <span className="legend-item" style={{ color: getPriorityColor('D-Tier') }}>
              D-Tier: Remove from board
            </span>
          </div>
        </div>

        <div className="member-list">
          {quickReference.map((member, index) => (
            <div key={index} className="member-cheat-card">
              <div className="cheat-header">
                <span className="cheat-name">{member.name}</span>
                <span
                  className="cheat-priority"
                  style={{ backgroundColor: getPriorityColor(member.priority) }}
                >
                  {member.priority}
                </span>
              </div>
              <div className="cheat-division">
                <strong>Best Position:</strong> {member.bestDivision}
              </div>
              <div className="cheat-strategy">
                <strong>Strategy:</strong> {member.strategy}
              </div>
            </div>
          ))}
        </div>

        <div className="farming-strategies">
          <h4>Farming Strategies</h4>
          <div className="strategy-cards">
            <div className="strategy-card">
              <div className="strategy-title">Profit Farming</div>
              <div className="strategy-content">
                <div><strong>Research:</strong> It That Fled, Vorici, Aisling</div>
                <div><strong>Fort:</strong> Hillock</div>
                <div><strong>Intervention:</strong> Cameria, Vagan</div>
                <div className="strategy-profit">Expected: 5-10 divine per board</div>
              </div>
            </div>
            <div className="strategy-card">
              <div className="strategy-title">Crafting Focus</div>
              <div className="strategy-content">
                <div><strong>Research:</strong> Vorici, Aisling, It That Fled</div>
                <div><strong>Transport/Fort:</strong> Hillock</div>
                <div><strong>Intervention:</strong> Cameria</div>
                <div className="strategy-profit">Variable profit based on crafts</div>
              </div>
            </div>
          </div>
        </div>
      </div>
    );
  };

  const renderMechanics = () => {
    return (
      <div className="mechanics-guide">
        <div className="mechanics-info">
          <p>Understanding betrayal mechanics and optimal decision making.</p>
        </div>

        <div className="mechanics-list">
          {Object.entries(mechanics).map(([action, description], index) => (
            <div key={index} className="mechanic-item">
              <div className="mechanic-action">{action}</div>
              <div className="mechanic-description">{description}</div>
            </div>
          ))}
        </div>

        <div className="decision-guide">
          <h4>Decision Making Guide</h4>
          <div className="decision-cards">
            <div className="decision-card">
              <div className="decision-title">When to Execute</div>
              <ul>
                <li>Member is at Rank 3 in target division</li>
                <li>Ready to run safehouse</li>
                <li>Member is low priority (C/D-Tier)</li>
              </ul>
            </div>
            <div className="decision-card">
              <div className="decision-title">When to Bargain</div>
              <ul>
                <li>Member not in target division</li>
                <li>Want to create relationships</li>
                <li>Need to move members around</li>
              </ul>
            </div>
            <div className="decision-card">
              <div className="decision-title">When to Interrogate</div>
              <ul>
                <li>Remove low-priority members</li>
                <li>Gain intelligence for safehouse</li>
                <li>Member rank is too high</li>
              </ul>
            </div>
          </div>
        </div>

        <div className="tips-section">
          <h4>Pro Tips</h4>
          <ul className="tips-list">
            <li>Always keep It That Fled in Research at Rank 3</li>
            <li>Vorici is best in Research for white sockets</li>
            <li>Create rival relationships to rank up members</li>
            <li>Don't run safehouses until all members are positioned</li>
            <li>Remove low-value members (Janus, Haku, Elreon)</li>
            <li>Focus on 2-3 high-value members per division</li>
          </ul>
        </div>
      </div>
    );
  };

  return (
    <div className="betrayal-board">
      <div className="betrayal-board__header">
        <h3>Betrayal Board Assistant</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>×</button>
        )}
      </div>

      <div className="betrayal-board__tabs">
        <button
          className={`tab ${activeTab === 'tracker' ? 'active' : ''}`}
          onClick={() => setActiveTab('tracker')}
        >
          Board Tracker
        </button>
        <button
          className={`tab ${activeTab === 'rewards' ? 'active' : ''}`}
          onClick={() => setActiveTab('rewards')}
        >
          Rewards
        </button>
        <button
          className={`tab ${activeTab === 'cheatsheet' ? 'active' : ''}`}
          onClick={() => setActiveTab('cheatsheet')}
        >
          Cheatsheet
        </button>
        <button
          className={`tab ${activeTab === 'mechanics' ? 'active' : ''}`}
          onClick={() => setActiveTab('mechanics')}
        >
          Mechanics
        </button>
      </div>

      <div className="betrayal-board__content">
        {activeTab === 'tracker' && renderTracker()}
        {activeTab === 'rewards' && renderRewards()}
        {activeTab === 'cheatsheet' && renderCheatsheet()}
        {activeTab === 'mechanics' && renderMechanics()}
      </div>
    </div>
  );
};

export default BetrayalBoard;
