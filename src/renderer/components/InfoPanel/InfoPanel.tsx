import React, { useState } from 'react';
import Panel from '@components/shared/Panel/Panel';
import { ItemData } from '@types/index';
import './InfoPanel.scss';

const InfoPanel: React.FC = () => {
  const [itemData, setItemData] = useState<ItemData | null>(null);

  // Mock data for demonstration
  const exampleItem: ItemData = {
    name: 'Shavronne\'s Wrappings',
    type: 'Occultist\'s Vestment',
    rarity: 'unique',
    corrupted: false,
    quality: 20,
    stats: [
      '+150 to maximum Energy Shield',
      '+30 to all Attributes',
      '10% increased Movement Speed',
    ],
    mods: [
      'Chaos Damage does not bypass Energy Shield',
      '+1 to Maximum Jewel Sockets',
    ],
  };

  const getRarityClass = (rarity: string) => {
    return `rarity-${rarity}`;
  };

  const displayItem = itemData || exampleItem;

  return (
    <Panel id="info" title="Item Information" className="info-panel">
      <div className="item-display">
        <div className={`item-header ${getRarityClass(displayItem.rarity)}`}>
          <div className="item-name">{displayItem.name}</div>
          <div className="item-type">{displayItem.type}</div>
        </div>

        {displayItem.quality !== undefined && (
          <div className="item-property">
            Quality: +{displayItem.quality}%
          </div>
        )}

        <div className="item-separator"></div>

        <div className="item-stats">
          {displayItem.stats.map((stat, index) => (
            <div key={index} className="stat-line">
              {stat}
            </div>
          ))}
        </div>

        {displayItem.mods.length > 0 && (
          <>
            <div className="item-separator"></div>
            <div className="item-mods">
              {displayItem.mods.map((mod, index) => (
                <div key={index} className="mod-line">
                  {mod}
                </div>
              ))}
            </div>
          </>
        )}

        {displayItem.corrupted && (
          <div className="item-corrupted">Corrupted</div>
        )}
      </div>

      <div className="item-actions">
        <button className="btn-primary">Copy Item Text</button>
        <button className="btn-primary">Search Similar</button>
        <button className="btn-primary">Check Price</button>
      </div>

      <div className="item-notes">
        <h4 className="notes-title">Notes</h4>
        <textarea
          className="notes-textarea"
          placeholder="Add notes about this item..."
        ></textarea>
      </div>
    </Panel>
  );
};

export default InfoPanel;
