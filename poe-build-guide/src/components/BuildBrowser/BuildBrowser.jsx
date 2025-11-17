/**
 * BuildBrowser - Browse available Path of Exile builds from Maxroll.gg
 */

import React, { useState, useEffect } from 'react';
import { buildGuideService } from '../../services/maxroll/buildGuideService';
import { searchService } from '../../services/maxroll/searchService';
import BuildCard from './BuildCard';
import BuildFilters from './BuildFilters';
import './BuildBrowser.css';

const BuildBrowser = () => {
  const [builds, setBuilds] = useState([]);
  const [filteredBuilds, setFilteredBuilds] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [filters, setFilters] = useState({
    className: '',
    ascendancy: '',
    mainSkill: '',
    difficulty: '',
    budget: '',
    searchText: '',
    sortBy: 'popularity',
    sortOrder: 'desc'
  });
  const [filterOptions, setFilterOptions] = useState(null);

  useEffect(() => {
    loadBuilds();
    loadFilterOptions();
  }, []);

  useEffect(() => {
    applyFilters();
  }, [filters, builds]);

  const loadBuilds = async () => {
    try {
      setLoading(true);
      const data = await buildGuideService.fetchAllBuilds();
      setBuilds(data);
      setFilteredBuilds(data);
      setError(null);
    } catch (err) {
      console.error('Error loading builds:', err);
      setError('Failed to load builds. Using cached data if available.');
    } finally {
      setLoading(false);
    }
  };

  const loadFilterOptions = async () => {
    try {
      const options = await searchService.getFilterOptions();
      setFilterOptions(options);
    } catch (err) {
      console.error('Error loading filter options:', err);
    }
  };

  const applyFilters = async () => {
    try {
      const results = await searchService.searchBuilds(filters);
      setFilteredBuilds(results);
    } catch (err) {
      console.error('Error applying filters:', err);
      setFilteredBuilds(builds);
    }
  };

  const handleFilterChange = (key, value) => {
    setFilters(prev => ({ ...prev, [key]: value }));
  };

  const handleClearFilters = () => {
    setFilters({
      className: '',
      ascendancy: '',
      mainSkill: '',
      difficulty: '',
      budget: '',
      searchText: '',
      sortBy: 'popularity',
      sortOrder: 'desc'
    });
  };

  if (loading) {
    return (
      <div className="build-browser">
        <div className="loading">
          <div className="spinner"></div>
          <p>Loading builds from Maxroll.gg...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="build-browser">
      <div className="browser-header">
        <h1>Path of Exile Build Guides</h1>
        <p className="subtitle">
          Browse top-rated builds from Maxroll.gg
        </p>
        {error && (
          <div className="error-banner">
            {error}
          </div>
        )}
      </div>

      {filterOptions && (
        <BuildFilters
          filters={filters}
          filterOptions={filterOptions}
          onChange={handleFilterChange}
          onClear={handleClearFilters}
        />
      )}

      <div className="build-results">
        <div className="results-header">
          <span className="result-count">
            {filteredBuilds.length} {filteredBuilds.length === 1 ? 'build' : 'builds'} found
          </span>
          <button onClick={loadBuilds} className="refresh-button">
            Refresh
          </button>
        </div>

        <div className="build-grid">
          {filteredBuilds.length === 0 ? (
            <div className="no-results">
              <p>No builds found matching your criteria.</p>
              <button onClick={handleClearFilters} className="clear-filters-button">
                Clear Filters
              </button>
            </div>
          ) : (
            filteredBuilds.map(build => (
              <BuildCard key={build.id} build={build} />
            ))
          )}
        </div>
      </div>
    </div>
  );
};

export default BuildBrowser;
