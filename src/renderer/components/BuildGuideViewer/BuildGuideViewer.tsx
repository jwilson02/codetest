import React, { useState } from 'react';
import Panel from '@components/shared/Panel/Panel';
import { BuildGuide } from '@types/index';
import './BuildGuideViewer.scss';

const BuildGuideViewer: React.FC = () => {
  const [selectedBuild, setSelectedBuild] = useState<BuildGuide | null>(null);
  const [builds] = useState<BuildGuide[]>([
    {
      id: '1',
      name: 'Poison Blade Vortex',
      class: 'Shadow',
      ascendancy: 'Assassin',
      skills: ['Blade Vortex', 'Plague Bearer', 'Whirling Blades'],
      items: ['Obliteration', 'Despair on Hit Ring', 'Cospri\'s Will'],
      url: 'https://maxroll.gg/poe/build-guides/poison-blade-vortex',
    },
    {
      id: '2',
      name: 'Lightning Arrow Deadeye',
      class: 'Ranger',
      ascendancy: 'Deadeye',
      skills: ['Lightning Arrow', 'Barrage', 'Blink Arrow'],
      items: ['Hyrri\'s Ire', 'Brood Twine', 'Omeyocan'],
      url: 'https://maxroll.gg/poe/build-guides/lightning-arrow-deadeye',
    },
  ]);

  const handleBuildSelect = (build: BuildGuide) => {
    setSelectedBuild(build);
  };

  const openBuildUrl = () => {
    if (selectedBuild?.url) {
      window.require('electron').shell.openExternal(selectedBuild.url);
    }
  };

  return (
    <Panel id="build" title="Build Guides" className="build-panel">
      <div className="build-list">
        <h3 className="section-title">Popular Builds</h3>
        {builds.map((build) => (
          <div
            key={build.id}
            className={`build-item ${selectedBuild?.id === build.id ? 'active' : ''}`}
            onClick={() => handleBuildSelect(build)}
          >
            <div className="build-header">
              <div className="build-name">{build.name}</div>
              <div className="build-class">{build.ascendancy}</div>
            </div>
            <div className="build-meta">
              {build.class} - {build.ascendancy}
            </div>
          </div>
        ))}
      </div>

      {selectedBuild && (
        <div className="build-details">
          <h3 className="section-title">Build Details</h3>

          <div className="detail-section">
            <h4 className="detail-title">Main Skills</h4>
            <div className="skill-list">
              {selectedBuild.skills.map((skill, index) => (
                <div key={index} className="skill-tag">
                  {skill}
                </div>
              ))}
            </div>
          </div>

          <div className="detail-section">
            <h4 className="detail-title">Key Items</h4>
            <div className="item-list">
              {selectedBuild.items.map((item, index) => (
                <div key={index} className="item-tag">
                  {item}
                </div>
              ))}
            </div>
          </div>

          <div className="build-actions">
            <button className="btn-primary" onClick={openBuildUrl}>
              Open Full Guide
            </button>
            <button className="btn-primary">
              Import to PoB
            </button>
          </div>
        </div>
      )}

      <div className="build-search">
        <input
          type="text"
          className="search-input"
          placeholder="Search builds by name, class, or skill..."
        />
      </div>
    </Panel>
  );
};

export default BuildGuideViewer;
