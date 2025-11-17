/**
 * BuildFilters - Filter and search builds
 */

import React from 'react';
import './BuildFilters.css';

const BuildFilters = ({ filters, filterOptions, onChange, onClear }) => {
  return (
    <div className="build-filters">
      <div className="filters-container">
        <div className="filter-group">
          <label htmlFor="search">Search</label>
          <input
            id="search"
            type="text"
            placeholder="Search builds..."
            value={filters.searchText}
            onChange={(e) => onChange('searchText', e.target.value)}
            className="filter-input"
          />
        </div>

        <div className="filter-group">
          <label htmlFor="class">Class</label>
          <select
            id="class"
            value={filters.className}
            onChange={(e) => onChange('className', e.target.value)}
            className="filter-select"
          >
            <option value="">All Classes</option>
            {filterOptions.classes.map(cls => (
              <option key={cls} value={cls}>{cls}</option>
            ))}
          </select>
        </div>

        <div className="filter-group">
          <label htmlFor="ascendancy">Ascendancy</label>
          <select
            id="ascendancy"
            value={filters.ascendancy}
            onChange={(e) => onChange('ascendancy', e.target.value)}
            className="filter-select"
          >
            <option value="">All Ascendancies</option>
            {filterOptions.ascendancies.map(asc => (
              <option key={asc} value={asc}>{asc}</option>
            ))}
          </select>
        </div>

        <div className="filter-group">
          <label htmlFor="difficulty">Difficulty</label>
          <select
            id="difficulty"
            value={filters.difficulty}
            onChange={(e) => onChange('difficulty', e.target.value)}
            className="filter-select"
          >
            <option value="">All Difficulties</option>
            {filterOptions.difficulties.map(diff => (
              <option key={diff} value={diff}>{diff}</option>
            ))}
          </select>
        </div>

        <div className="filter-group">
          <label htmlFor="budget">Budget</label>
          <select
            id="budget"
            value={filters.budget}
            onChange={(e) => onChange('budget', e.target.value)}
            className="filter-select"
          >
            <option value="">All Budgets</option>
            {filterOptions.budgets.map(budget => (
              <option key={budget} value={budget}>{budget}</option>
            ))}
          </select>
        </div>

        <div className="filter-group">
          <label htmlFor="sortBy">Sort By</label>
          <select
            id="sortBy"
            value={filters.sortBy}
            onChange={(e) => onChange('sortBy', e.target.value)}
            className="filter-select"
          >
            {filterOptions.sortOptions.map(opt => (
              <option key={opt.value} value={opt.value}>{opt.label}</option>
            ))}
          </select>
        </div>

        <div className="filter-actions">
          <button onClick={onClear} className="clear-button">
            Clear Filters
          </button>
        </div>
      </div>
    </div>
  );
};

export default BuildFilters;
