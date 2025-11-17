import React, { useState } from 'react';
import Panel from '@components/shared/Panel/Panel';
import './SearchPanel.scss';

interface SearchFilter {
  minPrice?: string;
  maxPrice?: string;
  online: boolean;
  corruptedOnly: boolean;
}

const SearchPanel: React.FC = () => {
  const [searchQuery, setSearchQuery] = useState('');
  const [filters, setFilters] = useState<SearchFilter>({
    online: true,
    corruptedOnly: false,
  });
  const [searchResults, setSearchResults] = useState<any[]>([]);

  const handleSearch = () => {
    // Placeholder for trade search functionality
    console.log('Searching for:', searchQuery, filters);
    // This would integrate with POE Trade API
  };

  const updateFilter = (key: keyof SearchFilter, value: any) => {
    setFilters(prev => ({ ...prev, [key]: value }));
  };

  return (
    <Panel id="search" title="Trade Search" className="search-panel">
      <div className="search-section">
        <h3 className="section-title">Item Search</h3>
        <input
          type="text"
          className="search-input"
          placeholder="Enter item name or paste item..."
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
        />
        <button className="btn-primary search-btn" onClick={handleSearch}>
          Search Trade
        </button>
      </div>

      <div className="filters-section">
        <h3 className="section-title">Filters</h3>

        <div className="filter-group">
          <label className="filter-label">Price Range</label>
          <div className="price-inputs">
            <input
              type="text"
              className="price-input"
              placeholder="Min"
              value={filters.minPrice || ''}
              onChange={(e) => updateFilter('minPrice', e.target.value)}
            />
            <span className="price-separator">-</span>
            <input
              type="text"
              className="price-input"
              placeholder="Max"
              value={filters.maxPrice || ''}
              onChange={(e) => updateFilter('maxPrice', e.target.value)}
            />
          </div>
        </div>

        <div className="filter-group">
          <label className="checkbox-label">
            <input
              type="checkbox"
              checked={filters.online}
              onChange={(e) => updateFilter('online', e.target.checked)}
            />
            <span>Online Only</span>
          </label>
        </div>

        <div className="filter-group">
          <label className="checkbox-label">
            <input
              type="checkbox"
              checked={filters.corruptedOnly}
              onChange={(e) => updateFilter('corruptedOnly', e.target.checked)}
            />
            <span>Corrupted Only</span>
          </label>
        </div>
      </div>

      <div className="results-section">
        <h3 className="section-title">
          Results {searchResults.length > 0 && `(${searchResults.length})`}
        </h3>
        <div className="results-list">
          {searchResults.length === 0 ? (
            <div className="no-results">No results yet. Start a search above.</div>
          ) : (
            searchResults.map((result, index) => (
              <div key={index} className="result-item">
                {/* Result items would be rendered here */}
              </div>
            ))
          )}
        </div>
      </div>
    </Panel>
  );
};

export default SearchPanel;
