import React from 'react';
import Panel from '@components/shared/Panel/Panel';
import { useOverlay } from '@contexts/OverlayContext';
import './SettingsPanel.scss';

const SettingsPanel: React.FC = () => {
  const { settings, updateSettings, resetLayout, togglePanel } = useOverlay();

  const handleOpacityChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    updateSettings({ opacity: parseFloat(e.target.value) });
  };

  const handleThemeChange = (theme: 'dark' | 'light') => {
    updateSettings({ theme });
  };

  const handleAlwaysOnTopToggle = () => {
    const newValue = !settings.alwaysOnTop;
    updateSettings({ alwaysOnTop: newValue });

    // Send to Electron main process
    const { ipcRenderer } = window.require('electron');
    ipcRenderer.send('set-always-on-top', newValue);
  };

  const handleClickThroughToggle = () => {
    const newValue = !settings.clickThrough;
    updateSettings({ clickThrough: newValue });

    const { ipcRenderer } = window.require('electron');
    ipcRenderer.send('set-click-through', newValue);
  };

  const handleCompactModeToggle = () => {
    updateSettings({ compactMode: !settings.compactMode });
  };

  const handleResetLayout = () => {
    if (confirm('Are you sure you want to reset all panel positions and sizes?')) {
      resetLayout();
    }
  };

  const handleTogglePanel = (panelId: string) => {
    togglePanel(panelId);
  };

  return (
    <Panel id="settings" title="Settings" className="settings-panel">
      <div className="settings-section">
        <h3 className="section-title">Appearance</h3>

        <div className="setting-item">
          <label className="setting-label">Theme</label>
          <div className="theme-buttons">
            <button
              className={`theme-btn ${settings.theme === 'dark' ? 'active' : ''}`}
              onClick={() => handleThemeChange('dark')}
            >
              Dark
            </button>
            <button
              className={`theme-btn ${settings.theme === 'light' ? 'active' : ''}`}
              onClick={() => handleThemeChange('light')}
            >
              Light
            </button>
          </div>
        </div>

        <div className="setting-item">
          <label className="setting-label">
            Opacity: {Math.round(settings.opacity * 100)}%
          </label>
          <input
            type="range"
            min="0.3"
            max="1"
            step="0.05"
            value={settings.opacity}
            onChange={handleOpacityChange}
            className="opacity-slider"
          />
        </div>

        <div className="setting-item">
          <label className="checkbox-label">
            <input
              type="checkbox"
              checked={settings.compactMode}
              onChange={handleCompactModeToggle}
            />
            <span>Compact Mode</span>
          </label>
        </div>
      </div>

      <div className="settings-section">
        <h3 className="section-title">Window Behavior</h3>

        <div className="setting-item">
          <label className="checkbox-label">
            <input
              type="checkbox"
              checked={settings.alwaysOnTop}
              onChange={handleAlwaysOnTopToggle}
            />
            <span>Always on Top</span>
          </label>
        </div>

        <div className="setting-item">
          <label className="checkbox-label">
            <input
              type="checkbox"
              checked={settings.clickThrough}
              onChange={handleClickThroughToggle}
            />
            <span>Click Through (allows clicking through overlay)</span>
          </label>
        </div>
      </div>

      <div className="settings-section">
        <h3 className="section-title">Panels</h3>

        <div className="panel-toggles">
          {Object.entries(settings.panels).map(([key, panel]) => (
            <div key={key} className="panel-toggle-item">
              <label className="checkbox-label">
                <input
                  type="checkbox"
                  checked={panel.visible}
                  onChange={() => handleTogglePanel(key)}
                />
                <span>{key.charAt(0).toUpperCase() + key.slice(1)}</span>
              </label>
            </div>
          ))}
        </div>
      </div>

      <div className="settings-section">
        <h3 className="section-title">Layout</h3>

        <div className="setting-item">
          <button className="btn-danger full-width" onClick={handleResetLayout}>
            Reset Layout to Default
          </button>
        </div>
      </div>

      <div className="settings-info">
        <div className="info-box">
          <div className="info-icon">ⓘ</div>
          <div className="info-text">
            Settings are automatically saved. Panel positions and sizes
            are preserved between sessions.
          </div>
        </div>
      </div>
    </Panel>
  );
};

export default SettingsPanel;
