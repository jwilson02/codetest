/**
 * BuildCard - Display a single build in card format
 */

import React from 'react';
import { useNavigate } from 'react-router-dom';
import './BuildCard.css';

const BuildCard = ({ build }) => {
  const navigate = useNavigate();

  const handleClick = () => {
    navigate(`/build/${build.id}`);
  };

  const getDifficultyColor = (difficulty) => {
    switch (difficulty.toLowerCase()) {
      case 'easy': return '#28a745';
      case 'medium': return '#ffc107';
      case 'hard': return '#dc3545';
      default: return '#6c757d';
    }
  };

  const getBudgetColor = (budget) => {
    switch (budget.toLowerCase()) {
      case 'low': return '#28a745';
      case 'medium': return '#ffc107';
      case 'high': return '#dc3545';
      default: return '#6c757d';
    }
  };

  return (
    <div className="build-card" onClick={handleClick}>
      <div className="build-card-header">
        <h3 className="build-title">{build.title}</h3>
        <div className="build-rating">
          <span className="rating-stars">{'★'.repeat(Math.round(build.rating))}</span>
          <span className="rating-value">{build.rating.toFixed(1)}</span>
        </div>
      </div>

      <div className="build-card-body">
        <div className="build-class-info">
          <span className="class-name">{build.className}</span>
          {build.ascendancy && (
            <>
              <span className="separator">›</span>
              <span className="ascendancy">{build.ascendancy}</span>
            </>
          )}
        </div>

        <div className="build-main-skill">
          <strong>Main Skill:</strong> {build.mainSkill}
        </div>

        <p className="build-description">{build.description}</p>

        <div className="build-tags">
          <span
            className="tag difficulty-tag"
            style={{ backgroundColor: getDifficultyColor(build.difficulty) }}
          >
            {build.difficulty}
          </span>
          <span
            className="tag budget-tag"
            style={{ backgroundColor: getBudgetColor(build.budget) }}
          >
            {build.budget} Budget
          </span>
          <span className="tag patch-tag">
            {build.patch}
          </span>
        </div>
      </div>

      <div className="build-card-footer">
        <div className="popularity">
          <span className="popularity-icon">👥</span>
          <span>{build.popularity}% popularity</span>
        </div>
        <div className="view-build">
          View Build →
        </div>
      </div>
    </div>
  );
};

export default BuildCard;
