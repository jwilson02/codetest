const { app, BrowserWindow, screen, globalShortcut, ipcMain } = require('electron');
const path = require('path');
const log = require('electron-log');

// Set up logging
log.transports.file.level = 'info';
log.info('Application starting...');

let mainWindow = null;
let isClickThrough = false;

// Development mode check
const isDev = process.env.NODE_ENV === 'development' || !app.isPackaged;

function createWindow() {
  // Get primary display dimensions
  const primaryDisplay = screen.getPrimaryDisplay();
  const { width, height } = primaryDisplay.workAreaSize;

  log.info(`Creating window with dimensions: ${width}x${height}`);

  // Create the browser window with overlay configuration
  mainWindow = new BrowserWindow({
    width: width,
    height: height,
    x: 0,
    y: 0,
    transparent: true,
    frame: false,
    alwaysOnTop: true,
    skipTaskbar: true,
    resizable: false,
    movable: false,
    minimizable: false,
    maximizable: false,
    closable: true,
    hasShadow: false,
    focusable: true,
    show: false, // Don't show until ready
    webPreferences: {
      preload: path.join(__dirname, '../preload/preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
      enableRemoteModule: false,
      sandbox: true
    }
  });

  // Set window to be click-through initially
  setClickThrough(true);

  // Load the app
  if (isDev) {
    mainWindow.loadURL('http://localhost:3000');
    // Open DevTools in development
    mainWindow.webContents.openDevTools({ mode: 'detach' });
  } else {
    mainWindow.loadFile(path.join(__dirname, '../../build/index.html'));
  }

  // Show window when ready
  mainWindow.once('ready-to-show', () => {
    log.info('Window ready to show');
    mainWindow.show();
  });

  // Handle window closed
  mainWindow.on('closed', () => {
    log.info('Window closed');
    mainWindow = null;
  });

  // Log any web contents errors
  mainWindow.webContents.on('crashed', () => {
    log.error('Window crashed');
  });

  mainWindow.webContents.on('unresponsive', () => {
    log.error('Window unresponsive');
  });
}

/**
 * Set click-through mode for the overlay
 * @param {boolean} enabled - Whether to enable click-through
 */
function setClickThrough(enabled) {
  if (!mainWindow) return;

  isClickThrough = enabled;

  if (enabled) {
    // Make window click-through
    mainWindow.setIgnoreMouseEvents(true, { forward: true });
    log.info('Click-through enabled');
  } else {
    // Make window interactive
    mainWindow.setIgnoreMouseEvents(false);
    log.info('Click-through disabled');
  }
}

/**
 * Toggle click-through mode
 */
function toggleClickThrough() {
  setClickThrough(!isClickThrough);

  // Notify renderer process of the change
  if (mainWindow) {
    mainWindow.webContents.send('click-through-changed', isClickThrough);
  }
}

/**
 * Register global shortcuts/hotkeys
 */
function registerShortcuts() {
  // Toggle overlay visibility (Ctrl+Shift+T)
  globalShortcut.register('CommandOrControl+Shift+T', () => {
    log.info('Toggle visibility shortcut pressed');
    if (mainWindow) {
      if (mainWindow.isVisible()) {
        mainWindow.hide();
      } else {
        mainWindow.show();
      }
    }
    // Notify renderer
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'toggle-overlay');
    }
  });

  // Price check (Ctrl+Shift+P) - trigger clipboard read + price check
  globalShortcut.register('CommandOrControl+Shift+P', () => {
    log.info('Price check shortcut pressed');
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'price-check');
    }
  });

  // Build guide (Ctrl+Shift+B)
  globalShortcut.register('CommandOrControl+Shift+B', () => {
    log.info('Build guide shortcut pressed');
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'build-guide');
    }
  });

  // Map info/checker (Ctrl+Shift+M)
  globalShortcut.register('CommandOrControl+Shift+M', () => {
    log.info('Map info shortcut pressed');
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'map-info');
    }
  });

  // Currency converter (Ctrl+Shift+C)
  globalShortcut.register('CommandOrControl+Shift+C', () => {
    log.info('Currency converter shortcut pressed');
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'currency');
    }
  });

  // Toggle click-through mode (Ctrl+Shift+X)
  globalShortcut.register('CommandOrControl+Shift+X', () => {
    log.info('Toggle click-through shortcut pressed');
    toggleClickThrough();
  });

  // Reload overlay (Ctrl+Shift+R) - useful for development
  globalShortcut.register('CommandOrControl+Shift+R', () => {
    log.info('Reload shortcut pressed');
    if (mainWindow) {
      mainWindow.reload();
    }
  });

  // Quit application (Ctrl+Shift+Q)
  globalShortcut.register('CommandOrControl+Shift+Q', () => {
    log.info('Quit shortcut pressed');
    app.quit();
  });

  log.info('Global shortcuts registered');
}

/**
 * Unregister all global shortcuts
 */
function unregisterShortcuts() {
  globalShortcut.unregisterAll();
  log.info('Global shortcuts unregistered');
}

// App event handlers
app.whenReady().then(() => {
  log.info('App ready');
  createWindow();
  registerShortcuts();

  app.on('activate', () => {
    // On macOS, re-create window when dock icon is clicked
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

// Quit when all windows are closed (except on macOS)
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

// Clean up shortcuts before quitting
app.on('will-quit', () => {
  log.info('App will quit');
  unregisterShortcuts();
});

// Handle display changes (monitor connection/disconnection)
screen.on('display-added', (event, newDisplay) => {
  log.info('Display added:', newDisplay.id);
  // Could reposition window if needed
});

screen.on('display-removed', (event, oldDisplay) => {
  log.info('Display removed:', oldDisplay.id);
  // Could reposition window if needed
});

screen.on('display-metrics-changed', (event, display, changedMetrics) => {
  log.info('Display metrics changed:', display.id, changedMetrics);
  // Could resize window if needed
});

// IPC handlers
ipcMain.handle('get-click-through-state', () => {
  return isClickThrough;
});

ipcMain.handle('set-click-through', (event, enabled) => {
  setClickThrough(enabled);
  return isClickThrough;
});

ipcMain.handle('toggle-click-through', () => {
  toggleClickThrough();
  return isClickThrough;
});

ipcMain.handle('get-display-info', () => {
  return {
    primary: screen.getPrimaryDisplay(),
    all: screen.getAllDisplays()
  };
});

ipcMain.handle('get-version', () => {
  return {
    app: app.getVersion(),
    electron: process.versions.electron,
    chrome: process.versions.chrome,
    node: process.versions.node
  };
});

// Clipboard IPC handlers
ipcMain.handle('read-clipboard', () => {
  const { clipboard } = require('electron');
  return clipboard.readText();
});

ipcMain.handle('write-clipboard', (event, text) => {
  const { clipboard } = require('electron');
  clipboard.writeText(text);
  return true;
});

// Panel toggle handlers
ipcMain.handle('toggle-panel', (event, panelId) => {
  log.info(`Toggle panel: ${panelId}`);
  if (mainWindow) {
    mainWindow.webContents.send('toggle-panel', panelId);
  }
  return true;
});

// Log unhandled errors
process.on('uncaughtException', (error) => {
  log.error('Uncaught exception:', error);
});

process.on('unhandledRejection', (reason, promise) => {
  log.error('Unhandled rejection at:', promise, 'reason:', reason);
});
