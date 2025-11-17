/**
 * Main App Component - Path of Exile Build Guide Application
 */

import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import BuildBrowser from './components/BuildBrowser/BuildBrowser';
import BuildDetail from './components/BuildDetail/BuildDetail';
import './App.css';

function App() {
  return (
    <Router>
      <div className="app">
        <header className="app-header">
          <div className="header-content">
            <h1 className="app-title">
              <a href="/">POE Build Guide</a>
            </h1>
            <p className="app-subtitle">Powered by Maxroll.gg</p>
          </div>
          <nav className="app-nav">
            <a href="/" className="nav-link">Builds</a>
            <a href="https://maxroll.gg/poe" target="_blank" rel="noopener noreferrer" className="nav-link">
              Maxroll.gg
            </a>
          </nav>
        </header>

        <main className="app-main">
          <Routes>
            <Route path="/" element={<BuildBrowser />} />
            <Route path="/build/:buildId" element={<BuildDetail />} />
          </Routes>
        </main>

        <footer className="app-footer">
          <div className="footer-content">
            <p>
              Build data sourced from{' '}
              <a href="https://maxroll.gg/poe" target="_blank" rel="noopener noreferrer">
                Maxroll.gg
              </a>
              . All rights reserved to their respective owners.
            </p>
            <p className="disclaimer">
              This application is for educational purposes and respects Maxroll.gg's terms of service.
              Please visit their site for the most up-to-date and complete build guides.
            </p>
          </div>
        </footer>
      </div>
    </Router>
  );
}

export default App;
