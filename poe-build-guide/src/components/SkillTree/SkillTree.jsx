/**
 * SkillTree - Display passive skill tree information
 */

import React from 'react';
import './SkillTree.css';

const SkillTree = ({ passiveTree, buildTitle }) => {
  if (!passiveTree) {
    return (
      <div className="skill-tree">
        <h2>Passive Skill Tree</h2>
        <div className="tree-placeholder">
          <p>Passive tree data not available in current implementation.</p>
          <p className="note">
            For the full interactive passive skill tree, please visit the build guide on Maxroll.gg
          </p>
          <div className="tree-links">
            <a
              href="https://www.pathofexile.com/passive-skill-tree"
              target="_blank"
              rel="noopener noreferrer"
              className="poe-tree-link"
            >
              Path of Exile Skill Tree ↗
            </a>
            <a
              href="https://poeplanner.com"
              target="_blank"
              rel="noopener noreferrer"
              className="planner-link"
            >
              PoE Planner ↗
            </a>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="skill-tree">
      <h2>Passive Skill Tree</h2>

      <div className="tree-info">
        <div className="info-section">
          <h3>Tree Overview</h3>
          <div className="tree-stats">
            <div className="stat">
              <span className="stat-label">Total Points:</span>
              <span className="stat-value">{passiveTree.totalPoints || 'N/A'}</span>
            </div>
            <div className="stat">
              <span className="stat-label">Keystones:</span>
              <span className="stat-value">{passiveTree.keystones?.length || 0}</span>
            </div>
            <div className="stat">
              <span className="stat-label">Notables:</span>
              <span className="stat-value">{passiveTree.notables?.length || 0}</span>
            </div>
          </div>
        </div>

        {passiveTree.keystones && passiveTree.keystones.length > 0 && (
          <div className="info-section">
            <h3>Key Passive Nodes</h3>
            <div className="keystones">
              {passiveTree.keystones.map((keystone, idx) => (
                <div key={idx} className="keystone">
                  <strong>{keystone.name}</strong>
                  {keystone.description && <p>{keystone.description}</p>}
                </div>
              ))}
            </div>
          </div>
        )}

        {passiveTree.url && (
          <div className="tree-link-section">
            <a
              href={passiveTree.url}
              target="_blank"
              rel="noopener noreferrer"
              className="view-tree-button"
            >
              View Interactive Tree ↗
            </a>
          </div>
        )}
      </div>

      <div className="tree-notes">
        <h3>Leveling Path</h3>
        <p>
          For detailed passive tree leveling progression (level 30, 50, 70, 90+),
          please refer to the full build guide on Maxroll.gg.
        </p>
        <p>
          The passive tree should be planned carefully as respeccing can be expensive.
          Focus on life/energy shield nodes early, then add damage as you level.
        </p>
      </div>
    </div>
  );
};

export default SkillTree;
