/**
 * GemLinks - Display gem setups for a build
 */

import React from 'react';
import './GemLinks.css';

const GemLinks = ({ gemLinks }) => {
  if (!gemLinks || gemLinks.length === 0) {
    return (
      <div className="gem-links">
        <p className="no-data">No gem link information available.</p>
      </div>
    );
  }

  const getGemColor = (color) => {
    switch (color?.toLowerCase()) {
      case 'red': return '#e74c3c';
      case 'green': return '#2ecc71';
      case 'blue': return '#3498db';
      default: return '#95a5a6';
    }
  };

  const getPriorityLabel = (priority) => {
    switch (priority) {
      case 1: return 'Main Setup';
      case 2: return 'Secondary';
      case 3: return 'Tertiary';
      default: return `Priority ${priority}`;
    }
  };

  return (
    <div className="gem-links">
      <h2>Gem Links</h2>
      <p className="section-description">
        Below are the recommended gem setups for this build, ordered by priority.
      </p>

      <div className="gem-links-container">
        {gemLinks.map((link, index) => (
          <div key={link.id || index} className="gem-link-card">
            <div className="link-header">
              <h3>{getPriorityLabel(link.priority)}</h3>
              <span className="socket-count">{link.socketCount} Sockets</span>
            </div>

            {link.description && (
              <p className="link-description">{link.description}</p>
            )}

            <div className="gems-container">
              {/* Main Skill */}
              {link.mainSkill && (
                <div className="gem main-gem">
                  <div
                    className="gem-icon"
                    style={{ backgroundColor: getGemColor(link.mainSkill.color) }}
                  >
                    {link.mainSkill.name.charAt(0)}
                  </div>
                  <div className="gem-info">
                    <div className="gem-name">{link.mainSkill.name}</div>
                    <div className="gem-meta">
                      {link.mainSkill.level && <span>Level {link.mainSkill.level}</span>}
                      {link.mainSkill.quality && <span>{link.mainSkill.quality}% Quality</span>}
                    </div>
                  </div>
                </div>
              )}

              {/* Support Gems */}
              {link.supports && link.supports.length > 0 && (
                <div className="support-gems">
                  {link.supports.map((support, idx) => (
                    <div key={idx} className="gem support-gem">
                      <div
                        className="gem-icon"
                        style={{ backgroundColor: getGemColor(support.color) }}
                      >
                        {support.name.charAt(0)}
                      </div>
                      <div className="gem-info">
                        <div className="gem-name">{support.name}</div>
                        <div className="gem-meta">
                          {support.level && <span>Level {support.level}</span>}
                          {support.quality && <span>{support.quality}% Quality</span>}
                        </div>
                      </div>
                    </div>
                  ))}
                </div>
              )}
            </div>

            {link.alternatives && link.alternatives.length > 0 && (
              <div className="alternatives">
                <h4>Alternatives:</h4>
                <ul>
                  {link.alternatives.map((alt, idx) => (
                    <li key={idx}>{alt}</li>
                  ))}
                </ul>
              </div>
            )}
          </div>
        ))}
      </div>

      <div className="gem-notes">
        <h3>Notes</h3>
        <ul>
          <li>Quality on gems can be obtained through Gemcutter's Prisms</li>
          <li>Level gems that don't have stat requirements to 20, then use a Vaal Orb for a chance at 21</li>
          <li>Awakened support gems are expensive but provide significant damage increases</li>
          <li>Check the Maxroll.gg guide for detailed gem progression and alternatives</li>
        </ul>
      </div>
    </div>
  );
};

export default GemLinks;
