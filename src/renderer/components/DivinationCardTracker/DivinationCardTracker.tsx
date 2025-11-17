import React, { useState, useEffect } from 'react';
import './DivinationCardTracker.scss';

interface DivinationCard {
  name: string;
  stackSize: number;
  reward: string;
  dropLocations: string[];
  rarity: string;
  farmingStrategy: string;
  baseValue: number;
  tags: string[];
  currentCount?: number;
  completion?: number;
  currentValue?: number;
  totalValue?: number;
  remaining?: number;
}

interface DivinationCardTrackerProps {
  onClose?: () => void;
}

const DivinationCardTracker: React.FC<DivinationCardTrackerProps> = ({ onClose }) => {
  const [cards, setCards] = useState<DivinationCard[]>([]);
  const [filteredCards, setFilteredCards] = useState<DivinationCard[]>([]);
  const [loading, setLoading] = useState(true);
  const [searchQuery, setSearchQuery] = useState('');
  const [rarityFilter, setRarityFilter] = useState('all');
  const [sortBy, setSortBy] = useState('name');
  const [selectedCard, setSelectedCard] = useState<DivinationCard | null>(null);
  const [totalValue, setTotalValue] = useState(0);

  useEffect(() => {
    loadCardData();
  }, []);

  useEffect(() => {
    filterAndSortCards();
  }, [cards, searchQuery, rarityFilter, sortBy]);

  const loadCardData = () => {
    try {
      setLoading(true);

      // In production, this would use IPC to call the service
      // For now, using mock data similar to the service
      const mockCards: DivinationCard[] = [
        {
          name: 'The Doctor',
          stackSize: 8,
          reward: 'Headhunter',
          dropLocations: ['Burial Chambers', 'Spider Forest'],
          rarity: 'ultra-rare',
          farmingStrategy: 'Farm Burial Chambers with high quantity and pack size.',
          baseValue: 1500,
          tags: ['unique', 'belt', 'headhunter'],
          currentCount: 2,
          completion: 25,
          currentValue: 3000,
          totalValue: 12000,
          remaining: 6,
        },
        {
          name: 'Apothecary',
          stackSize: 5,
          reward: 'Mageblood',
          dropLocations: ['Crimson Temple'],
          rarity: 'ultra-rare',
          farmingStrategy: 'Farm Crimson Temple with maximum quantity.',
          baseValue: 3000,
          tags: ['unique', 'belt', 'mageblood'],
          currentCount: 0,
          completion: 0,
          currentValue: 0,
          totalValue: 15000,
          remaining: 5,
        },
        {
          name: 'Humility',
          stackSize: 9,
          reward: 'Tabula Rasa',
          dropLocations: ['Channel Map', 'Aqueduct (Act 9)'],
          rarity: 'common',
          farmingStrategy: 'Farm Aqueduct in Act 9 or Channel Map.',
          baseValue: 10,
          tags: ['unique', 'armour', 'tabula'],
          currentCount: 9,
          completion: 100,
          currentValue: 90,
          totalValue: 90,
          remaining: 0,
        },
        {
          name: 'The Nurse',
          stackSize: 8,
          reward: 'The Doctor',
          dropLocations: ['Tower Map'],
          rarity: 'ultra-rare',
          farmingStrategy: 'Farm Tower Map consistently.',
          baseValue: 150,
          tags: ['divination', 'doctor'],
          currentCount: 4,
          completion: 50,
          currentValue: 600,
          totalValue: 1200,
          remaining: 4,
        },
      ];

      setCards(mockCards);
      calculateTotalValue(mockCards);
      setLoading(false);
    } catch (error) {
      console.error('Error loading card data:', error);
      setLoading(false);
    }
  };

  const calculateTotalValue = (cardList: DivinationCard[]) => {
    const total = cardList.reduce((sum, card) => sum + (card.currentValue || 0), 0);
    setTotalValue(total);
  };

  const filterAndSortCards = () => {
    let filtered = [...cards];

    // Apply search filter
    if (searchQuery) {
      const query = searchQuery.toLowerCase();
      filtered = filtered.filter(
        (card) =>
          card.name.toLowerCase().includes(query) ||
          card.reward.toLowerCase().includes(query) ||
          card.dropLocations.some((loc) => loc.toLowerCase().includes(query))
      );
    }

    // Apply rarity filter
    if (rarityFilter !== 'all') {
      filtered = filtered.filter((card) => card.rarity === rarityFilter);
    }

    // Apply sorting
    filtered.sort((a, b) => {
      switch (sortBy) {
        case 'name':
          return a.name.localeCompare(b.name);
        case 'value':
          return (b.totalValue || 0) - (a.totalValue || 0);
        case 'completion':
          return (b.completion || 0) - (a.completion || 0);
        case 'rarity':
          const rarityOrder = { 'mirror-tier': 0, 'ultra-rare': 1, 'rare': 2, 'uncommon': 3, 'common': 4 };
          return (rarityOrder[a.rarity] || 5) - (rarityOrder[b.rarity] || 5);
        default:
          return 0;
      }
    });

    setFilteredCards(filtered);
  };

  const updateCardCount = (cardName: string, count: number) => {
    setCards((prevCards) =>
      prevCards.map((card) => {
        if (card.name === cardName) {
          const newCount = Math.max(0, Math.min(card.stackSize, count));
          const completion = (newCount / card.stackSize) * 100;
          return {
            ...card,
            currentCount: newCount,
            completion,
            currentValue: newCount * card.baseValue,
            remaining: card.stackSize - newCount,
          };
        }
        return card;
      })
    );
  };

  const getRarityClass = (rarity: string) => {
    switch (rarity) {
      case 'mirror-tier':
        return 'rarity-mirror';
      case 'ultra-rare':
        return 'rarity-ultra';
      case 'rare':
        return 'rarity-rare';
      case 'uncommon':
        return 'rarity-uncommon';
      default:
        return 'rarity-common';
    }
  };

  const getRarityDisplay = (rarity: string) => {
    return rarity.charAt(0).toUpperCase() + rarity.slice(1).replace('-', ' ');
  };

  return (
    <div className="divination-tracker">
      <div className="divination-tracker__header">
        <h3>Divination Card Tracker</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>
            ×
          </button>
        )}
      </div>

      <div className="divination-tracker__stats">
        <div className="stat-box">
          <span className="stat-label">Total Cards</span>
          <span className="stat-value">{cards.length}</span>
        </div>
        <div className="stat-box">
          <span className="stat-label">In Progress</span>
          <span className="stat-value">
            {cards.filter((c) => (c.currentCount || 0) > 0 && (c.completion || 0) < 100).length}
          </span>
        </div>
        <div className="stat-box">
          <span className="stat-label">Completed</span>
          <span className="stat-value">{cards.filter((c) => (c.completion || 0) === 100).length}</span>
        </div>
        <div className="stat-box">
          <span className="stat-label">Collection Value</span>
          <span className="stat-value">{totalValue}c</span>
        </div>
      </div>

      <div className="divination-tracker__filters">
        <input
          type="text"
          className="search-input"
          placeholder="Search cards..."
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
        />
        <select className="filter-select" value={rarityFilter} onChange={(e) => setRarityFilter(e.target.value)}>
          <option value="all">All Rarities</option>
          <option value="mirror-tier">Mirror Tier</option>
          <option value="ultra-rare">Ultra Rare</option>
          <option value="rare">Rare</option>
          <option value="uncommon">Uncommon</option>
          <option value="common">Common</option>
        </select>
        <select className="sort-select" value={sortBy} onChange={(e) => setSortBy(e.target.value)}>
          <option value="name">Sort by Name</option>
          <option value="value">Sort by Value</option>
          <option value="completion">Sort by Completion</option>
          <option value="rarity">Sort by Rarity</option>
        </select>
      </div>

      {loading ? (
        <div className="divination-tracker__loading">Loading divination cards...</div>
      ) : (
        <div className="divination-tracker__cards">
          {filteredCards.map((card) => (
            <div
              key={card.name}
              className={`card-item ${selectedCard?.name === card.name ? 'selected' : ''}`}
              onClick={() => setSelectedCard(selectedCard?.name === card.name ? null : card)}
            >
              <div className="card-header">
                <span className={`card-name ${getRarityClass(card.rarity)}`}>{card.name}</span>
                <span className="card-value">{card.totalValue}c</span>
              </div>
              <div className="card-reward">Reward: {card.reward}</div>
              <div className="card-progress">
                <div className="progress-bar">
                  <div className="progress-fill" style={{ width: `${card.completion}%` }} />
                </div>
                <div className="progress-text">
                  {card.currentCount}/{card.stackSize} ({Math.round(card.completion || 0)}%)
                </div>
              </div>
              <div className="card-controls">
                <button onClick={(e) => { e.stopPropagation(); updateCardCount(card.name, (card.currentCount || 0) - 1); }}>-</button>
                <input
                  type="number"
                  value={card.currentCount || 0}
                  onChange={(e) => updateCardCount(card.name, parseInt(e.target.value) || 0)}
                  onClick={(e) => e.stopPropagation()}
                  min="0"
                  max={card.stackSize}
                />
                <button onClick={(e) => { e.stopPropagation(); updateCardCount(card.name, (card.currentCount || 0) + 1); }}>+</button>
              </div>

              {selectedCard?.name === card.name && (
                <div className="card-details">
                  <div className="detail-row">
                    <strong>Drop Locations:</strong>
                    <ul>
                      {card.dropLocations.map((loc, idx) => (
                        <li key={idx}>{loc}</li>
                      ))}
                    </ul>
                  </div>
                  <div className="detail-row">
                    <strong>Farming Strategy:</strong>
                    <p>{card.farmingStrategy}</p>
                  </div>
                  <div className="detail-row">
                    <strong>Rarity:</strong>
                    <span className={getRarityClass(card.rarity)}>{getRarityDisplay(card.rarity)}</span>
                  </div>
                  <div className="detail-row">
                    <strong>Current Value:</strong>
                    <span>{card.currentValue}c</span>
                  </div>
                  <div className="detail-row">
                    <strong>Cards Remaining:</strong>
                    <span>{card.remaining}</span>
                  </div>
                </div>
              )}
            </div>
          ))}
        </div>
      )}
    </div>
  );
};

export default DivinationCardTracker;
