/**
 * ItemRecommendations - Display recommended items and gear for a build
 */

import React, { useState } from 'react';
import './ItemRecommendations.css';

const ItemRecommendations = ({ items }) => {
  const [selectedSlot, setSelectedSlot] = useState('all');

  if (!items || items.length === 0) {
    return (
      <div className="item-recommendations">
        <p className="no-data">No item recommendations available.</p>
      </div>
    );
  }

  const slots = ['all', ...new Set(items.map(item => item.slot).filter(Boolean))];

  const filteredItems = selectedSlot === 'all'
    ? items
    : items.filter(item => item.slot === selectedSlot);

  const getRarityColor = (rarity) => {
    switch (rarity?.toLowerCase()) {
      case 'normal': return '#c8c8c8';
      case 'magic': return '#8888ff';
      case 'rare': return '#ffff77';
      case 'unique': return '#af6025';
      default: return '#fff';
    }
  };

  const getImportanceColor = (importance) => {
    switch (importance?.toLowerCase()) {
      case 'required': return '#e74c3c';
      case 'high': return '#f39c12';
      case 'medium': return '#3498db';
      case 'low': return '#95a5a6';
      default: return '#95a5a6';
    }
  };

  const formatPrice = (price) => {
    if (!price) return 'N/A';
    if (price.divine > 0) {
      return `${price.divine} Divine`;
    }
    if (price.chaos > 0) {
      return `${price.chaos} Chaos`;
    }
    return 'Budget';
  };

  return (
    <div className="item-recommendations">
      <h2>Recommended Items & Gear</h2>

      <div className="slot-filters">
        {slots.map(slot => (
          <button
            key={slot}
            className={`slot-filter ${selectedSlot === slot ? 'active' : ''}`}
            onClick={() => setSelectedSlot(slot)}
          >
            {slot === 'all' ? 'All Items' : slot}
          </button>
        ))}
      </div>

      <div className="items-container">
        {filteredItems.map((item, index) => (
          <div key={item.id || index} className="item-card">
            <div className="item-header">
              <h3
                className="item-name"
                style={{ color: getRarityColor(item.rarity) }}
              >
                {item.name}
              </h3>
              <div className="item-badges">
                <span
                  className="importance-badge"
                  style={{ backgroundColor: getImportanceColor(item.importance) }}
                >
                  {item.importance}
                </span>
                <span className="rarity-badge">{item.rarity}</span>
              </div>
            </div>

            <div className="item-type">
              {item.type} {item.basetype && `(${item.basetype})`}
            </div>

            {item.implicits && item.implicits.length > 0 && (
              <div className="item-mods implicits">
                {item.implicits.map((mod, idx) => (
                  <div key={idx} className="mod implicit">
                    {mod}
                  </div>
                ))}
              </div>
            )}

            {item.explicits && item.explicits.length > 0 && (
              <div className="item-mods explicits">
                {item.explicits.map((mod, idx) => (
                  <div key={idx} className="mod explicit">
                    {mod}
                  </div>
                ))}
              </div>
            )}

            {item.crafted && item.crafted.length > 0 && (
              <div className="item-mods crafted">
                {item.crafted.map((mod, idx) => (
                  <div key={idx} className="mod crafted-mod">
                    {mod} (Crafted)
                  </div>
                ))}
              </div>
            )}

            <div className="item-footer">
              <div className="item-price">
                <strong>Price:</strong> {formatPrice(item.price)}
              </div>
              {item.corrupted && (
                <span className="corrupted-badge">Corrupted</span>
              )}
            </div>

            {item.alternatives && item.alternatives.length > 0 && (
              <div className="alternatives">
                <details>
                  <summary>Budget Alternatives</summary>
                  <ul>
                    {item.alternatives.map((alt, idx) => (
                      <li key={idx}>{alt}</li>
                    ))}
                  </ul>
                </details>
              </div>
            )}
          </div>
        ))}
      </div>

      <div className="item-notes">
        <h3>Gearing Tips</h3>
        <ul>
          <li><strong>Required</strong> items are essential for the build to function properly</li>
          <li><strong>High</strong> importance items significantly improve performance</li>
          <li><strong>Medium/Low</strong> importance items are nice-to-have upgrades</li>
          <li>Prices are estimates and may vary by league and time</li>
          <li>Check trade sites like poe.trade or the official Path of Exile trade site for current prices</li>
        </ul>
      </div>
    </div>
  );
};

export default ItemRecommendations;
