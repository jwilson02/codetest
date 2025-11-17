/**
 * BuildDetail - Display full build guide with all details
 */

import React, { useState, useEffect } from 'react';
import { useParams, Link } from 'react-router-dom';
import { buildGuideService } from '../../services/maxroll/buildGuideService';
import GemLinks from '../GemLinks/GemLinks';
import ItemRecommendations from '../ItemRecommendations/ItemRecommendations';
import SkillTree from '../SkillTree/SkillTree';
import './BuildDetail.css';

const BuildDetail = () => {
  const { buildId } = useParams();
  const [build, setBuild] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [activeTab, setActiveTab] = useState('overview');

  useEffect(() => {
    loadBuild();
  }, [buildId]);

  const loadBuild = async () => {
    try {
      setLoading(true);
      const data = await buildGuideService.fetchBuildGuide(buildId);
      setBuild(data);
      setError(null);
    } catch (err) {
      console.error('Error loading build:', err);
      setError('Failed to load build details.');
    } finally {
      setLoading(false);
    }
  };

  if (loading) {
    return (
      <div className="build-detail">
        <div className="loading">
          <div className="spinner"></div>
          <p>Loading build guide...</p>
        </div>
      </div>
    );
  }

  if (error || !build) {
    return (
      <div className="build-detail">
        <div className="error">
          <h2>Error</h2>
          <p>{error || 'Build not found'}</p>
          <Link to="/" className="back-link">← Back to builds</Link>
        </div>
      </div>
    );
  }

  return (
    <div className="build-detail">
      <div className="build-header">
        <Link to="/" className="back-link">← Back to builds</Link>

        <div className="header-content">
          <h1>{build.title}</h1>
          <div className="build-meta">
            <span className="class-info">
              {build.className} › {build.ascendancy}
            </span>
            <span className="rating">
              {'★'.repeat(Math.round(build.rating))} {build.rating.toFixed(1)}
            </span>
            <span className="patch">Patch {build.patch}</span>
          </div>
        </div>

        <div className="external-links">
          <a href={build.url} target="_blank" rel="noopener noreferrer" className="maxroll-link">
            View on Maxroll.gg ↗
          </a>
        </div>
      </div>

      <div className="build-tabs">
        <button
          className={`tab ${activeTab === 'overview' ? 'active' : ''}`}
          onClick={() => setActiveTab('overview')}
        >
          Overview
        </button>
        <button
          className={`tab ${activeTab === 'gems' ? 'active' : ''}`}
          onClick={() => setActiveTab('gems')}
        >
          Gem Links
        </button>
        <button
          className={`tab ${activeTab === 'items' ? 'active' : ''}`}
          onClick={() => setActiveTab('items')}
        >
          Items & Gear
        </button>
        <button
          className={`tab ${activeTab === 'tree' ? 'active' : ''}`}
          onClick={() => setActiveTab('tree')}
        >
          Passive Tree
        </button>
      </div>

      <div className="build-content">
        {activeTab === 'overview' && (
          <div className="overview-tab">
            <div className="build-summary">
              <h2>Build Summary</h2>
              <p className="description">{build.description}</p>

              <div className="build-info-grid">
                <div className="info-card">
                  <h3>Difficulty</h3>
                  <span className="value">{build.difficulty}</span>
                </div>
                <div className="info-card">
                  <h3>Budget</h3>
                  <span className="value">{build.budget}</span>
                </div>
                <div className="info-card">
                  <h3>Main Skill</h3>
                  <span className="value">{build.mainSkill}</span>
                </div>
                <div className="info-card">
                  <h3>Popularity</h3>
                  <span className="value">{build.popularity}%</span>
                </div>
              </div>
            </div>

            <div className="pros-cons">
              <div className="pros">
                <h3>Pros</h3>
                <ul>
                  {build.pros.map((pro, idx) => (
                    <li key={idx}>✓ {pro}</li>
                  ))}
                </ul>
              </div>
              <div className="cons">
                <h3>Cons</h3>
                <ul>
                  {build.cons.map((con, idx) => (
                    <li key={idx}>✗ {con}</li>
                  ))}
                </ul>
              </div>
            </div>

            <div className="build-stats">
              <div className="stats-section">
                <h3>Defense Stats</h3>
                <div className="stats-grid">
                  {Object.entries(build.defenseStats).map(([key, value]) => (
                    <div key={key} className="stat">
                      <span className="stat-name">{formatStatName(key)}</span>
                      <span className="stat-value">{value}</span>
                    </div>
                  ))}
                </div>
              </div>

              <div className="stats-section">
                <h3>Offense Stats</h3>
                <div className="stats-grid">
                  {Object.entries(build.offenseStats).map(([key, value]) => (
                    <div key={key} className="stat">
                      <span className="stat-name">{formatStatName(key)}</span>
                      <span className="stat-value">{value}</span>
                    </div>
                  ))}
                </div>
              </div>
            </div>

            <div className="build-choices">
              <div className="choice-section">
                <h3>Pantheon</h3>
                <div className="pantheon">
                  <div><strong>Major:</strong> {build.pantheon.major}</div>
                  <div><strong>Minor:</strong> {build.pantheon.minor}</div>
                </div>
              </div>
              <div className="choice-section">
                <h3>Bandit</h3>
                <div className="bandit">{build.bandit}</div>
              </div>
            </div>
          </div>
        )}

        {activeTab === 'gems' && (
          <GemLinks gemLinks={build.gemLinks} />
        )}

        {activeTab === 'items' && (
          <ItemRecommendations items={build.items} />
        )}

        {activeTab === 'tree' && (
          <SkillTree passiveTree={build.passiveTree} buildTitle={build.title} />
        )}
      </div>
    </div>
  );
};

const formatStatName = (name) => {
  return name
    .replace(/([A-Z])/g, ' $1')
    .replace(/^./, str => str.toUpperCase())
    .trim();
};

export default BuildDetail;
