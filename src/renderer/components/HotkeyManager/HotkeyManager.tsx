import React, { useState, useEffect } from 'react';
import Panel from '@components/shared/Panel/Panel';
import { Hotkey } from '@types/index';
import { useOverlay } from '@contexts/OverlayContext';
import './HotkeyManager.scss';

const HotkeyManager: React.FC = () => {
  const { togglePanel } = useOverlay();

  const [hotkeys, setHotkeys] = useState<Hotkey[]>([
    {
      id: 'toggle-overlay',
      name: 'Toggle Overlay',
      description: 'Show/hide the entire overlay',
      key: 'D',
      modifiers: ['Alt'],
      action: () => console.log('Toggle overlay'),
    },
    {
      id: 'toggle-trade-search',
      name: 'Toggle Trade Search',
      description: 'Open/close trade search panel',
      key: 'T',
      modifiers: ['Alt'],
      action: () => togglePanel('search'),
    },
    {
      id: 'toggle-price-check',
      name: 'Toggle Price Check',
      description: 'Open/close price check panel',
      key: 'P',
      modifiers: ['Alt'],
      action: () => togglePanel('price'),
    },
    {
      id: 'toggle-build-guide',
      name: 'Toggle Build Guide',
      description: 'Open/close build guide panel',
      key: 'B',
      modifiers: ['Alt'],
      action: () => togglePanel('build'),
    },
    {
      id: 'copy-item',
      name: 'Copy Item',
      description: 'Copy item under cursor',
      key: 'C',
      modifiers: ['Ctrl'],
      action: () => console.log('Copy item'),
    },
  ]);

  const [editingHotkey, setEditingHotkey] = useState<string | null>(null);
  const [recordingKey, setRecordingKey] = useState(false);

  useEffect(() => {
    // Listen for Electron IPC hotkey events
    const { ipcRenderer } = window.require('electron');

    const handleHotkey = (_event: any, action: string) => {
      const hotkey = hotkeys.find(h => h.id === action);
      if (hotkey) {
        hotkey.action();
      }
    };

    ipcRenderer.on('hotkey-pressed', handleHotkey);

    return () => {
      ipcRenderer.removeListener('hotkey-pressed', handleHotkey);
    };
  }, [hotkeys]);

  const startRecording = (hotkeyId: string) => {
    setEditingHotkey(hotkeyId);
    setRecordingKey(true);
  };

  const stopRecording = () => {
    setEditingHotkey(null);
    setRecordingKey(false);
  };

  const formatHotkey = (hotkey: Hotkey) => {
    return [...hotkey.modifiers, hotkey.key].join(' + ');
  };

  const resetHotkeys = () => {
    if (confirm('Reset all hotkeys to default?')) {
      // Reset logic would go here
      console.log('Reset hotkeys');
    }
  };

  return (
    <Panel id="hotkeys" title="Hotkey Manager" className="hotkey-panel">
      <div className="hotkey-list">
        {hotkeys.map((hotkey) => (
          <div key={hotkey.id} className="hotkey-item">
            <div className="hotkey-info">
              <div className="hotkey-name">{hotkey.name}</div>
              <div className="hotkey-description">{hotkey.description}</div>
            </div>
            <div className="hotkey-binding">
              {editingHotkey === hotkey.id && recordingKey ? (
                <div className="recording-indicator">
                  Press any key...
                </div>
              ) : (
                <div className="hotkey-display">
                  {formatHotkey(hotkey)}
                </div>
              )}
              <button
                className="edit-btn"
                onClick={() => startRecording(hotkey.id)}
              >
                Edit
              </button>
            </div>
          </div>
        ))}
      </div>

      <div className="hotkey-actions">
        <button className="btn-primary" onClick={resetHotkeys}>
          Reset to Defaults
        </button>
      </div>

      <div className="hotkey-info">
        <div className="info-box">
          <div className="info-icon">ⓘ</div>
          <div className="info-text">
            Global hotkeys work even when POE is in focus.
            Click "Edit" and press your desired key combination.
          </div>
        </div>
      </div>
    </Panel>
  );
};

export default HotkeyManager;
