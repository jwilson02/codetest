import React, { useState, useEffect } from 'react';
import './CraftingHelper.scss';

interface CraftingMethod {
  name: string;
  category?: string;
  type?: string;
  weight?: string;
  effect?: string;
  blocking?: string[];
  bestFor?: string[];
  avgCost?: string;
  strategy?: string;
  value?: string;
  tiers?: any;
  influence?: string;
}

interface SearchResults {
  fossils: CraftingMethod[];
  essences: CraftingMethod[];
  harvest: CraftingMethod[];
  eldritch: CraftingMethod[];
}

interface CraftingHelperProps {
  onClose?: () => void;
}

const CraftingHelper: React.FC<CraftingHelperProps> = ({ onClose }) => {
  const [activeTab, setActiveTab] = useState<'fossils' | 'essences' | 'harvest' | 'eldritch' | 'strategies'>('fossils');
  const [searchTerm, setSearchTerm] = useState('');
  const [searchResults, setSearchResults] = useState<SearchResults | null>(null);
  const [selectedCategory, setSelectedCategory] = useState<string>('all');
  const [fossilData, setFossilData] = useState<any>(null);
  const [essenceData, setEssenceData] = useState<any>(null);
  const [harvestData, setHarvestData] = useState<any>(null);
  const [eldritchData, setEldritchData] = useState<any>(null);
  const [strategies, setStrategies] = useState<any>(null);

  useEffect(() => {
    // In a real implementation, this would load data via IPC from craftingService
    loadCraftingData();
  }, []);

  const loadCraftingData = () => {
    // Mock data loading - in real implementation, call service via IPC
    // For now, we'll use inline mock data
    setFossilData({
      'Pristine Fossil': {
        category: 'Life/Mana',
        weight: 'More Life modifiers, No Mana modifiers',
        blocking: ['Mana'],
        bestFor: ['Life-based gear', 'Body Armour', 'Helmets'],
        avgCost: '2-5c'
      },
      'Dense Fossil': {
        category: 'Defense',
        weight: 'More Defense modifiers, No Life modifiers',
        blocking: ['Life'],
        bestFor: ['Body Armour', 'Helmets', 'Gloves'],
        avgCost: '4-8c'
      },
      'Jagged Fossil': {
        category: 'Physical/Chaos',
        weight: 'More Physical modifiers, No Chaos modifiers',
        blocking: ['Chaos'],
        bestFor: ['Physical weapons', 'Attack builds'],
        avgCost: '3-6c'
      }
    });

    setEssenceData({
      'Essence of Greed': {
        mod: '+70-79 to maximum Life (Deafening)',
        bestFor: ['Life-based builds', 'Body Armour', 'Helmets'],
        strategy: 'Spam on ilvl 85+ bases for high life'
      },
      'Essence of Contempt': {
        mod: '+38-42% increased Attack Speed (Deafening)',
        bestFor: ['Attack weapons', 'Gloves'],
        strategy: 'Guaranteed high attack speed'
      },
      'Essence of Horror': {
        mod: '+1 to Level of Socketed Gems',
        bestFor: ['6-link Body Armour', 'Weapons'],
        strategy: 'Corruption essence - very expensive'
      }
    });

    setHarvestData({
      'Augment Life': {
        type: 'Augment',
        effect: 'Add a Life modifier',
        value: 'Very High',
        strategy: 'Guaranteed life roll on open affix'
      },
      'Augment Crit': {
        type: 'Augment',
        effect: 'Add a Critical modifier',
        value: 'Extreme',
        strategy: 'Add crit chance/multi'
      },
      'Reforge Keep Prefixes': {
        type: 'Reforge Special',
        effect: 'Reforge item, keep all prefixes',
        value: 'Very High',
        strategy: 'Craft prefixes, reroll suffixes'
      }
    });

    setEldritchData({
      'Lesser Eldritch Ember': {
        type: 'Implicit Reforge',
        effect: 'Reroll Searing Exarch implicit',
        influence: 'Searing Exarch',
        avgCost: '1-2c'
      },
      'Eldritch Chaos Orb': {
        type: 'Chaos Orb',
        effect: 'Chaos orb that preserves eldritch implicits',
        avgCost: '10-15c'
      },
      'Eldritch Exalted Orb': {
        type: 'Exalted Orb',
        effect: 'Add mod matching implicit influence',
        avgCost: '20-40c'
      }
    });

    setStrategies({
      'High Life Body Armour': {
        steps: [
          '1. Get ilvl 86+ Astral Plate base',
          '2. Spam Pristine + Sanctified fossils until T1 life',
          '3. Benchcraft prefixes cannot be changed',
          '4. Harvest reforge keep prefixes for suffixes',
          '5. Divine for perfect rolls'
        ],
        estimatedCost: '2-5 divine',
        difficulty: 'Medium'
      },
      'Tailwind/Elusive Boots': {
        steps: [
          '1. Awakener Orb Hunter + Redeemer boots',
          '2. Hit tailwind + elusive',
          '3. Harvest aug speed for movement speed',
          '4. Aug life for T1 life',
          '5. Craft res or benchcraft'
        ],
        estimatedCost: '10-20 divine',
        difficulty: 'Hard'
      }
    });
  };

  const handleSearch = () => {
    if (!searchTerm.trim()) {
      setSearchResults(null);
      return;
    }

    const term = searchTerm.toLowerCase();
    const results: SearchResults = {
      fossils: [],
      essences: [],
      harvest: [],
      eldritch: []
    };

    // Search fossils
    if (fossilData) {
      Object.entries(fossilData).forEach(([name, data]: [string, any]) => {
        if (name.toLowerCase().includes(term) ||
            data.category?.toLowerCase().includes(term) ||
            data.bestFor?.some((use: string) => use.toLowerCase().includes(term))) {
          results.fossils.push({ name, ...data });
        }
      });
    }

    // Search essences
    if (essenceData) {
      Object.entries(essenceData).forEach(([name, data]: [string, any]) => {
        if (name.toLowerCase().includes(term) ||
            data.mod?.toLowerCase().includes(term) ||
            data.bestFor?.some((use: string) => use.toLowerCase().includes(term))) {
          results.essences.push({ name, ...data });
        }
      });
    }

    // Search harvest
    if (harvestData) {
      Object.entries(harvestData).forEach(([name, data]: [string, any]) => {
        if (name.toLowerCase().includes(term) ||
            data.effect?.toLowerCase().includes(term) ||
            data.type?.toLowerCase().includes(term)) {
          results.harvest.push({ name, ...data });
        }
      });
    }

    // Search eldritch
    if (eldritchData) {
      Object.entries(eldritchData).forEach(([name, data]: [string, any]) => {
        if (name.toLowerCase().includes(term) ||
            data.effect?.toLowerCase().includes(term) ||
            data.influence?.toLowerCase().includes(term)) {
          results.eldritch.push({ name, ...data });
        }
      });
    }

    setSearchResults(results);
  };

  const renderFossils = () => {
    const data = searchResults ? searchResults.fossils :
      (fossilData ? Object.entries(fossilData).map(([name, data]) => ({ name, ...data as any })) : []);

    return (
      <div className="crafting-list">
        {data.length === 0 ? (
          <div className="no-results">No fossils found</div>
        ) : (
          data.map((fossil, index) => (
            <div key={index} className="crafting-item fossil">
              <div className="item-header">
                <span className="item-name">{fossil.name}</span>
                <span className="item-cost">{fossil.avgCost}</span>
              </div>
              <div className="item-category">{fossil.category}</div>
              <div className="item-detail">
                <strong>Effect:</strong> {fossil.weight}
              </div>
              {fossil.blocking && fossil.blocking.length > 0 && (
                <div className="item-detail blocking">
                  <strong>Blocks:</strong> {fossil.blocking.join(', ')}
                </div>
              )}
              {fossil.bestFor && (
                <div className="item-detail best-for">
                  <strong>Best for:</strong> {fossil.bestFor.join(', ')}
                </div>
              )}
            </div>
          ))
        )}
      </div>
    );
  };

  const renderEssences = () => {
    const data = searchResults ? searchResults.essences :
      (essenceData ? Object.entries(essenceData).map(([name, data]) => ({ name, ...data as any })) : []);

    return (
      <div className="crafting-list">
        {data.length === 0 ? (
          <div className="no-results">No essences found</div>
        ) : (
          data.map((essence, index) => (
            <div key={index} className="crafting-item essence">
              <div className="item-header">
                <span className="item-name">{essence.name}</span>
              </div>
              <div className="item-detail">
                <strong>Guaranteed Mod:</strong> {essence.mod}
              </div>
              {essence.bestFor && (
                <div className="item-detail best-for">
                  <strong>Best for:</strong> {essence.bestFor.join(', ')}
                </div>
              )}
              {essence.strategy && (
                <div className="item-detail strategy">
                  <strong>Strategy:</strong> {essence.strategy}
                </div>
              )}
            </div>
          ))
        )}
      </div>
    );
  };

  const renderHarvest = () => {
    const data = searchResults ? searchResults.harvest :
      (harvestData ? Object.entries(harvestData).map(([name, data]) => ({ name, ...data as any })) : []);

    return (
      <div className="crafting-list">
        {data.length === 0 ? (
          <div className="no-results">No harvest crafts found</div>
        ) : (
          data.map((craft, index) => (
            <div key={index} className="crafting-item harvest">
              <div className="item-header">
                <span className="item-name">{craft.name}</span>
                <span className={`item-value value-${craft.value?.toLowerCase().replace(' ', '-')}`}>
                  {craft.value}
                </span>
              </div>
              <div className="item-category">{craft.type}</div>
              <div className="item-detail">
                <strong>Effect:</strong> {craft.effect}
              </div>
              {craft.strategy && (
                <div className="item-detail strategy">
                  <strong>Strategy:</strong> {craft.strategy}
                </div>
              )}
            </div>
          ))
        )}
      </div>
    );
  };

  const renderEldritch = () => {
    const data = searchResults ? searchResults.eldritch :
      (eldritchData ? Object.entries(eldritchData).map(([name, data]) => ({ name, ...data as any })) : []);

    return (
      <div className="crafting-list">
        {data.length === 0 ? (
          <div className="no-results">No eldritch currency found</div>
        ) : (
          data.map((currency, index) => (
            <div key={index} className="crafting-item eldritch">
              <div className="item-header">
                <span className="item-name">{currency.name}</span>
                <span className="item-cost">{currency.avgCost}</span>
              </div>
              <div className="item-category">{currency.type}</div>
              {currency.influence && (
                <div className="item-influence">{currency.influence}</div>
              )}
              <div className="item-detail">
                <strong>Effect:</strong> {currency.effect}
              </div>
            </div>
          ))
        )}
      </div>
    );
  };

  const renderStrategies = () => {
    if (!strategies) return <div className="no-results">No strategies loaded</div>;

    return (
      <div className="strategies-list">
        {Object.entries(strategies).map(([name, strategy]: [string, any], index) => (
          <div key={index} className="strategy-item">
            <div className="strategy-header">
              <span className="strategy-name">{name}</span>
              <div className="strategy-meta">
                <span className="strategy-cost">{strategy.estimatedCost}</span>
                <span className={`strategy-difficulty ${strategy.difficulty.toLowerCase()}`}>
                  {strategy.difficulty}
                </span>
              </div>
            </div>
            <div className="strategy-steps">
              <strong>Steps:</strong>
              {strategy.steps.map((step: string, i: number) => (
                <div key={i} className="strategy-step">{step}</div>
              ))}
            </div>
          </div>
        ))}
      </div>
    );
  };

  return (
    <div className="crafting-helper">
      <div className="crafting-helper__header">
        <h3>Crafting Helper</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>×</button>
        )}
      </div>

      <div className="crafting-helper__search">
        <input
          type="text"
          placeholder="Search for mods, item types, or crafting methods..."
          value={searchTerm}
          onChange={(e) => setSearchTerm(e.target.value)}
          onKeyPress={(e) => e.key === 'Enter' && handleSearch()}
        />
        <button onClick={handleSearch} className="search-btn">
          Search
        </button>
        {searchResults && (
          <button onClick={() => { setSearchTerm(''); setSearchResults(null); }} className="clear-btn">
            Clear
          </button>
        )}
      </div>

      {searchResults && (
        <div className="search-summary">
          Found: {searchResults.fossils.length} fossils, {searchResults.essences.length} essences,
          {searchResults.harvest.length} harvest crafts, {searchResults.eldritch.length} eldritch currency
        </div>
      )}

      <div className="crafting-helper__tabs">
        <button
          className={`tab ${activeTab === 'fossils' ? 'active' : ''}`}
          onClick={() => setActiveTab('fossils')}
        >
          Fossils
        </button>
        <button
          className={`tab ${activeTab === 'essences' ? 'active' : ''}`}
          onClick={() => setActiveTab('essences')}
        >
          Essences
        </button>
        <button
          className={`tab ${activeTab === 'harvest' ? 'active' : ''}`}
          onClick={() => setActiveTab('harvest')}
        >
          Harvest
        </button>
        <button
          className={`tab ${activeTab === 'eldritch' ? 'active' : ''}`}
          onClick={() => setActiveTab('eldritch')}
        >
          Eldritch
        </button>
        <button
          className={`tab ${activeTab === 'strategies' ? 'active' : ''}`}
          onClick={() => setActiveTab('strategies')}
        >
          Strategies
        </button>
      </div>

      <div className="crafting-helper__content">
        {activeTab === 'fossils' && renderFossils()}
        {activeTab === 'essences' && renderEssences()}
        {activeTab === 'harvest' && renderHarvest()}
        {activeTab === 'eldritch' && renderEldritch()}
        {activeTab === 'strategies' && renderStrategies()}
      </div>

      <div className="crafting-helper__info">
        <div className="info-text">
          {activeTab === 'fossils' && 'Fossils modify crafting outcomes by adding weight to specific mod types and blocking others.'}
          {activeTab === 'essences' && 'Essences guarantee specific modifiers when used on items.'}
          {activeTab === 'harvest' && 'Harvest crafts allow targeted crafting of specific modifier types.'}
          {activeTab === 'eldritch' && 'Eldritch currency crafts implicits on influenced items.'}
          {activeTab === 'strategies' && 'Popular crafting strategies for common endgame items.'}
        </div>
      </div>
    </div>
  );
};

export default CraftingHelper;
