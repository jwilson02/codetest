/**
 * Mock Electron Environment for Testing
 * Provides mock implementations of Electron APIs for testing without Electron installed
 */

// Mock clipboard API
const mockClipboard = {
  _content: '',

  readText() {
    return this._content;
  },

  writeText(text) {
    this._content = text;
  },

  // Helper for testing
  _setContent(text) {
    this._content = text;
  }
};

// Mock ipcRenderer API
const mockIpcRenderer = {
  _handlers: {},
  _listeners: {},

  on(channel, callback) {
    if (!this._listeners[channel]) {
      this._listeners[channel] = [];
    }
    this._listeners[channel].push(callback);
  },

  send(channel, ...args) {
    const listeners = this._listeners[channel] || [];
    listeners.forEach(callback => {
      callback({ sender: this }, ...args);
    });
  },

  invoke(channel, ...args) {
    return new Promise((resolve, reject) => {
      const handler = this._handlers[channel];
      if (handler) {
        try {
          const result = handler(...args);
          resolve(result);
        } catch (error) {
          reject(error);
        }
      } else {
        resolve(null);
      }
    });
  },

  // Helper for testing
  _registerHandler(channel, handler) {
    this._handlers[channel] = handler;
  },

  // Helper for testing
  _trigger(channel, ...args) {
    const listeners = this._listeners[channel] || [];
    listeners.forEach(callback => {
      callback({ sender: this }, ...args);
    });
  }
};

// Mock ipcMain API
const mockIpcMain = {
  _handlers: {},

  on(channel, callback) {
    this._handlers[channel] = callback;
  },

  handle(channel, handler) {
    this._handlers[channel] = handler;
  },

  // Helper for testing
  _trigger(channel, event, ...args) {
    const handler = this._handlers[channel];
    if (handler) {
      handler(event || { sender: { send: () => {} } }, ...args);
    }
  }
};

// Mock contextBridge API
const mockContextBridge = {
  exposeInMainWorld(apiKey, api) {
    if (typeof window !== 'undefined') {
      window[apiKey] = api;
    }
  }
};

// Mock app API
const mockApp = {
  _ready: false,
  _handlers: {},

  on(event, callback) {
    this._handlers[event] = callback;
  },

  quit() {
    console.log('[MOCK] App quit called');
  },

  getPath(name) {
    return `/mock/path/${name}`;
  },

  // Helper for testing
  _triggerReady() {
    this._ready = true;
    if (this._handlers['ready']) {
      this._handlers['ready']();
    }
  }
};

// Mock BrowserWindow API
class MockBrowserWindow {
  constructor(options) {
    this.options = options;
    this.isDestroyed = false;
    this._handlers = {};
    this.webContents = {
      send: (channel, ...args) => {
        console.log(`[MOCK] WebContents send: ${channel}`, args);
      },
      openDevTools: () => {
        console.log('[MOCK] DevTools opened');
      },
      on: (event, callback) => {
        this._handlers[event] = callback;
      }
    };
  }

  loadURL(url) {
    console.log(`[MOCK] Loading URL: ${url}`);
    return Promise.resolve();
  }

  setIgnoreMouseEvents(ignore, options) {
    console.log(`[MOCK] Set ignore mouse events: ${ignore}`);
  }

  setAlwaysOnTop(flag) {
    console.log(`[MOCK] Set always on top: ${flag}`);
  }

  minimize() {
    console.log('[MOCK] Window minimized');
  }

  close() {
    this.isDestroyed = true;
    console.log('[MOCK] Window closed');
    if (this._handlers['closed']) {
      this._handlers['closed']();
    }
  }

  on(event, callback) {
    this._handlers[event] = callback;
  }
}

// Mock globalShortcut API
const mockGlobalShortcut = {
  _shortcuts: {},

  register(accelerator, callback) {
    this._shortcuts[accelerator] = callback;
    console.log(`[MOCK] Registered shortcut: ${accelerator}`);
    return true;
  },

  unregister(accelerator) {
    delete this._shortcuts[accelerator];
    console.log(`[MOCK] Unregistered shortcut: ${accelerator}`);
  },

  unregisterAll() {
    this._shortcuts = {};
    console.log('[MOCK] Unregistered all shortcuts');
  },

  // Helper for testing
  _trigger(accelerator) {
    const callback = this._shortcuts[accelerator];
    if (callback) {
      callback();
    }
  }
};

// Export mock Electron module
const mockElectron = {
  clipboard: mockClipboard,
  ipcRenderer: mockIpcRenderer,
  ipcMain: mockIpcMain,
  contextBridge: mockContextBridge,
  app: mockApp,
  BrowserWindow: MockBrowserWindow,
  globalShortcut: mockGlobalShortcut
};

// If running in Node.js, export as module
if (typeof module !== 'undefined' && module.exports) {
  module.exports = mockElectron;
}

// If running in browser, attach to window
try {
  if (typeof window !== 'undefined') {
    window.electron = mockElectron;
  }
} catch (e) {
  // Not in browser environment
}

// Usage example:
console.log('Mock Electron environment loaded');
console.log('Available mocks:');
console.log('  - clipboard');
console.log('  - ipcRenderer');
console.log('  - ipcMain');
console.log('  - contextBridge');
console.log('  - app');
console.log('  - BrowserWindow');
console.log('  - globalShortcut');
