const { contextBridge, ipcRenderer } = require('electron');

/**
 * Preload script for secure IPC communication
 * Exposes a limited API to the renderer process via contextBridge
 */

// Expose protected methods that allow the renderer process to use
// ipcRenderer without exposing the entire object
contextBridge.exposeInMainWorld('electronAPI', {
  // Click-through controls
  getClickThroughState: () => ipcRenderer.invoke('get-click-through-state'),
  setClickThrough: (enabled) => ipcRenderer.invoke('set-click-through', enabled),
  toggleClickThrough: () => ipcRenderer.invoke('toggle-click-through'),
  onClickThroughChanged: (callback) => {
    ipcRenderer.on('click-through-changed', (event, isClickThrough) => {
      callback(isClickThrough);
    });
  },

  // Display information
  getDisplayInfo: () => ipcRenderer.invoke('get-display-info'),

  // Version information
  getVersion: () => ipcRenderer.invoke('get-version'),

  // Application controls
  minimize: () => ipcRenderer.send('minimize'),
  maximize: () => ipcRenderer.send('maximize'),
  close: () => ipcRenderer.send('close'),

  // Clipboard operations
  readClipboard: () => ipcRenderer.invoke('read-clipboard'),
  writeClipboard: (text) => ipcRenderer.invoke('write-clipboard', text),

  // Panel controls
  togglePanel: (panelId) => ipcRenderer.invoke('toggle-panel', panelId),

  // Hotkey events
  onHotkeyPressed: (callback) => {
    ipcRenderer.on('hotkey-pressed', (event, hotkeyName) => {
      callback(hotkeyName);
    });
  },

  // Panel toggle events
  onTogglePanel: (callback) => {
    ipcRenderer.on('toggle-panel', (event, panelId) => {
      callback(panelId);
    });
  },

  // Platform information
  platform: process.platform
});

// Expose a safe logger API
contextBridge.exposeInMainWorld('logger', {
  info: (...args) => console.log('[INFO]', ...args),
  warn: (...args) => console.warn('[WARN]', ...args),
  error: (...args) => console.error('[ERROR]', ...args),
  debug: (...args) => console.debug('[DEBUG]', ...args)
});

// Log preload script loaded
console.log('Preload script loaded successfully');
