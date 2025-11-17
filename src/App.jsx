/**
 * Main App Component
 * Ties together all POE overlay functionality
 */

import React, { useState, useEffect } from 'react';
import PriceChecker from './components/PriceChecker';
import TradeSearch from './components/TradeSearch';
import CurrencyExchange from './components/CurrencyExchange';
import WhisperGenerator from './components/WhisperGenerator';

const { ipcRenderer } = window.require('electron');

const App = () => {
  const [currentView, setCurrentView] = useState('priceCheck');
  const [priceCheckResult, setPriceCheckResult] = useState(null);
  const [league, setLeague] = useState('Standard');
  const [searchResults, setSearchResults] = useState(null);

  useEffect(() => {
    // Listen for price check results from main process
    ipcRenderer.on('price-check-result', (event, result) => {
      setPriceCheckResult(result);
      setCurrentView('priceCheck');
    });

    ipcRenderer.on('price-check-error', (event, error) => {
      console.error('Price check error:', error);
    });

    return () => {
      ipcRenderer.removeAllListeners('price-check-result');
      ipcRenderer.removeAllListeners('price-check-error');
    };
  }, []);

  const handleLeagueChange = async (newLeague) => {
    const result = await ipcRenderer.invoke('set-league', newLeague);
    if (result.success) {
      setLeague(newLeague);
    }
  };

  const handleManualPriceCheck = async (itemText) => {
    const result = await ipcRenderer.invoke('manual-price-check', itemText);
    if (result.success) {
      setPriceCheckResult(result);
    }
  };

  const handleSearch = async (searchParams) => {
    const result = await ipcRenderer.invoke('search-items', searchParams);
    if (result.success) {
      setSearchResults(result.results);
    }
  };

  const handleClosePriceCheck = () => {
    setPriceCheckResult(null);
  };

  return (
    <div className="poe-overlay-app">
      <div className="app-header">
        <h1>POE Trade Overlay</h1>
        <div className="league-selector">
          <select value={league} onChange={(e) => handleLeagueChange(e.target.value)}>
            <option value="Standard">Standard</option>
            <option value="Hardcore">Hardcore</option>
            <option value="Challenge">Current League</option>
          </select>
        </div>
      </div>

      <div className="app-nav">
        <button
          className={currentView === 'priceCheck' ? 'active' : ''}
          onClick={() => setCurrentView('priceCheck')}
        >
          Price Check
        </button>
        <button
          className={currentView === 'search' ? 'active' : ''}
          onClick={() => setCurrentView('search')}
        >
          Search
        </button>
        <button
          className={currentView === 'currency' ? 'active' : ''}
          onClick={() => setCurrentView('currency')}
        >
          Currency
        </button>
        <button
          className={currentView === 'whisper' ? 'active' : ''}
          onClick={() => setCurrentView('whisper')}
        >
          Whisper
        </button>
      </div>

      <div className="app-content">
        {currentView === 'priceCheck' && priceCheckResult && (
          <PriceChecker
            item={priceCheckResult.item}
            priceData={priceCheckResult.priceData}
            onClose={handleClosePriceCheck}
          />
        )}

        {currentView === 'search' && (
          <TradeSearch
            onSearch={handleSearch}
            searchResults={searchResults}
          />
        )}

        {currentView === 'currency' && (
          <CurrencyExchange />
        )}

        {currentView === 'whisper' && (
          <WhisperGenerator
            item={priceCheckResult?.item}
            listing={null}
            league={league}
          />
        )}
      </div>
    </div>
  );
};

export default App;
