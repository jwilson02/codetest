import React, { useState, useEffect } from 'react';
import './MapModChecker.scss';

interface DangerousMod {
  text: string;
  severity: 'deadly' | 'high' | 'medium' | 'low';
  category: string;
  name: string;
  description: string;
  warning: string;
  solution: string;
}

interface MapAnalysis {
  mods: string[];
  dangerous: DangerousMod[];
  safe: boolean;
  runnable: boolean;
  dangerLevel: 'deadly' | 'high' | 'medium' | 'safe';
  warnings: string[];
  solutions: string[];
}

interface MapModCheckerProps {
  onClose?: () => void;
}

const MapModChecker: React.FC<MapModCheckerProps> = ({ onClose }) => {
  const [mapText, setMapText] = useState('');
  const [analysis, setAnalysis] = useState<MapAnalysis | null>(null);
  const [buildType, setBuildType] = useState({
    damageType: 'elemental',
    usesLeech: false,
    usesRegen: false,
  });

  const handleAnalyze = () => {
    // This would call the mapModService via IPC in a real implementation
    // For now, using mock analysis
    const mockAnalysis: MapAnalysis = {
      mods: mapText.split('\n').filter(line => line.trim()),
      dangerous: [],
      safe: true,
      runnable: true,
      dangerLevel: 'safe',
      warnings: [],
      solutions: []
    };

    // Check for dangerous mods in the text
    if (mapText.toLowerCase().includes('reflect')) {
      mockAnalysis.dangerous.push({
        text: 'Monsters reflect X% of Elemental Damage',
        severity: 'deadly',
        category: 'reflect',
        name: 'Elemental Reflect',
        description: 'Reflects elemental damage back to you',
        warning: 'DEADLY for elemental damage builds!',
        solution: 'Skip this map or use Sextant of Reflection'
      });
      mockAnalysis.safe = false;
      mockAnalysis.runnable = false;
      mockAnalysis.dangerLevel = 'deadly';
      mockAnalysis.warnings.push('DEADLY for elemental damage builds!');
      mockAnalysis.solutions.push('Skip this map or use Sextant of Reflection');
    }

    if (mapText.toLowerCase().includes('no regen') || mapText.toLowerCase().includes('cannot regenerate')) {
      mockAnalysis.dangerous.push({
        text: 'Players cannot regenerate Life, Mana or Energy Shield',
        severity: 'high',
        category: 'regen',
        name: 'No Regeneration',
        description: 'Prevents all forms of regeneration',
        warning: 'Very difficult for regen-based builds',
        solution: 'Use leech or flasks instead'
      });
      mockAnalysis.safe = false;
      if (mockAnalysis.dangerLevel === 'safe') mockAnalysis.dangerLevel = 'high';
      mockAnalysis.warnings.push('Very difficult for regen-based builds');
      mockAnalysis.solutions.push('Use leech or flasks instead');
    }

    setAnalysis(mockAnalysis);
  };

  const handlePasteFromClipboard = async () => {
    try {
      const text = await navigator.clipboard.readText();
      setMapText(text);
    } catch (error) {
      console.error('Failed to read clipboard:', error);
    }
  };

  const getSeverityColor = (severity: string) => {
    switch (severity) {
      case 'deadly': return '#ff1744';
      case 'high': return '#ff9800';
      case 'medium': return '#ffc107';
      case 'low': return '#4caf50';
      default: return '#fff';
    }
  };

  return (
    <div className="map-mod-checker">
      <div className="map-mod-checker__header">
        <h3>Map Mod Checker</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>×</button>
        )}
      </div>

      <div className="map-mod-checker__build-config">
        <h4>Build Configuration</h4>
        <div className="config-options">
          <label>
            Damage Type:
            <select
              value={buildType.damageType}
              onChange={(e) => setBuildType({ ...buildType, damageType: e.target.value })}
            >
              <option value="physical">Physical</option>
              <option value="elemental">Elemental</option>
              <option value="chaos">Chaos</option>
              <option value="mixed">Mixed</option>
            </select>
          </label>

          <label className="checkbox-label">
            <input
              type="checkbox"
              checked={buildType.usesLeech}
              onChange={(e) => setBuildType({ ...buildType, usesLeech: e.target.checked })}
            />
            Uses Leech
          </label>

          <label className="checkbox-label">
            <input
              type="checkbox"
              checked={buildType.usesRegen}
              onChange={(e) => setBuildType({ ...buildType, usesRegen: e.target.checked })}
            />
            Uses Regeneration
          </label>
        </div>
      </div>

      <div className="map-mod-checker__input">
        <h4>Map Mods</h4>
        <div className="input-actions">
          <button onClick={handlePasteFromClipboard} className="paste-btn">
            Paste from Clipboard
          </button>
          <button onClick={handleAnalyze} className="analyze-btn">
            Analyze Map
          </button>
        </div>
        <textarea
          value={mapText}
          onChange={(e) => setMapText(e.target.value)}
          placeholder="Paste map item text here or use Ctrl+C on the map in-game and click 'Paste from Clipboard'"
          rows={8}
        />
      </div>

      {analysis && (
        <div className="map-mod-checker__results">
          <div className={`danger-level ${analysis.dangerLevel}`}>
            <div className="level-indicator">
              Danger Level: <span>{analysis.dangerLevel.toUpperCase()}</span>
            </div>
            <div className="runnable-status">
              {analysis.runnable ? (
                <span className="can-run">✓ Map is runnable</span>
              ) : (
                <span className="cannot-run">✗ DO NOT RUN THIS MAP</span>
              )}
            </div>
          </div>

          {analysis.dangerous.length > 0 && (
            <div className="dangerous-mods">
              <h4>Dangerous Mods Found:</h4>
              {analysis.dangerous.map((mod, index) => (
                <div
                  key={index}
                  className="mod-item"
                  style={{ borderLeft: `4px solid ${getSeverityColor(mod.severity)}` }}
                >
                  <div className="mod-header">
                    <span className="mod-name">{mod.name}</span>
                    <span className="mod-severity" style={{ color: getSeverityColor(mod.severity) }}>
                      {mod.severity.toUpperCase()}
                    </span>
                  </div>
                  <div className="mod-text">{mod.text}</div>
                  <div className="mod-description">{mod.description}</div>
                  <div className="mod-warning">⚠️ {mod.warning}</div>
                  <div className="mod-solution">💡 {mod.solution}</div>
                </div>
              ))}
            </div>
          )}

          {analysis.safe && (
            <div className="safe-message">
              <span className="icon">✓</span>
              <span>No dangerous mods detected. Map is safe to run!</span>
            </div>
          )}
        </div>
      )}

      <div className="map-mod-checker__info">
        <h4>Common Dangerous Mods:</h4>
        <ul>
          <li><strong>Reflect:</strong> Instantly deadly for corresponding damage types</li>
          <li><strong>No Regen:</strong> Difficult for RF and regen builds</li>
          <li><strong>No Leech:</strong> Dangerous for leech-dependent builds</li>
          <li><strong>Curses:</strong> Temporal Chains, Vulnerability, Elemental Weakness</li>
          <li><strong>-Max Res:</strong> Makes it harder to cap resistances</li>
        </ul>
      </div>
    </div>
  );
};

export default MapModChecker;
