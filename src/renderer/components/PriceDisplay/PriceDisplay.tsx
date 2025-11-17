import React, { useState } from 'react';
import Panel from '@components/shared/Panel/Panel';
import { PriceData } from '@types/index';
import './PriceDisplay.scss';

const PriceDisplay: React.FC = () => {
  const [priceData, setPriceData] = useState<PriceData[]>([
    {
      item: 'Shavronne\'s Wrappings',
      currency: 'Divine Orb',
      amount: 15.5,
      confidence: 85,
      listings: 42,
      source: 'poe.trade',
    },
    {
      item: 'Shavronne\'s Wrappings',
      currency: 'Chaos Orb',
      amount: 2450,
      confidence: 90,
      listings: 156,
      source: 'pathofexile.com/trade',
    },
  ]);

  const [searchHistory, setSearchHistory] = useState<string[]>([
    'Shavronne\'s Wrappings',
    'Headhunter',
    'Mageblood',
  ]);

  const getConfidenceColor = (confidence: number) => {
    if (confidence >= 80) return 'high-confidence';
    if (confidence >= 60) return 'medium-confidence';
    return 'low-confidence';
  };

  return (
    <Panel id="price" title="Price Check" className="price-panel">
      <div className="price-summary">
        {priceData.length > 0 && (
          <>
            <h3 className="item-name">{priceData[0].item}</h3>
            <div className="price-list">
              {priceData.map((price, index) => (
                <div key={index} className="price-item">
                  <div className="price-main">
                    <span className="amount">{price.amount}</span>
                    <span className="currency">{price.currency}</span>
                  </div>
                  <div className="price-details">
                    <div className="detail-row">
                      <span className="label">Confidence:</span>
                      <span className={`value ${getConfidenceColor(price.confidence)}`}>
                        {price.confidence}%
                      </span>
                    </div>
                    <div className="detail-row">
                      <span className="label">Listings:</span>
                      <span className="value">{price.listings}</span>
                    </div>
                    <div className="detail-row">
                      <span className="label">Source:</span>
                      <span className="value source">{price.source}</span>
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </>
        )}
      </div>

      <div className="price-actions">
        <button className="btn-primary">Refresh Prices</button>
        <button className="btn-primary">View Full Listings</button>
      </div>

      <div className="price-history">
        <h4 className="history-title">Recent Searches</h4>
        <div className="history-list">
          {searchHistory.map((item, index) => (
            <div key={index} className="history-item">
              <span className="item-name">{item}</span>
              <button className="quick-check-btn" title="Quick check">
                $
              </button>
            </div>
          ))}
        </div>
      </div>

      <div className="price-info">
        <div className="info-box">
          <div className="info-icon">ⓘ</div>
          <div className="info-text">
            Prices are updated in real-time from multiple sources.
            Higher confidence indicates more reliable pricing data.
          </div>
        </div>
      </div>
    </Panel>
  );
};

export default PriceDisplay;
