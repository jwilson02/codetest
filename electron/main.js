const { app, BrowserWindow, globalShortcut, ipcMain } = require('electron');
const path = require('path');

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    transparent: true,
    frame: false,
    alwaysOnTop: true,
    skipTaskbar: true,
    resizable: true,
    webPreferences: {
      preload: path.join(__dirname, '../src/preload/preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
      enableRemoteModule: false,
    },
  });

  // In development, load from webpack dev server
  // In production, load from built files
  const startUrl = process.env.ELECTRON_START_URL || `file://${path.join(__dirname, '../build/index.html')}`;
  mainWindow.loadURL(startUrl);

  // Set window to be click-through when not focused
  mainWindow.setIgnoreMouseEvents(false);

  // Open DevTools in development
  if (process.env.NODE_ENV === 'development') {
    mainWindow.webContents.openDevTools();
  }

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

app.on('ready', () => {
  createWindow();

  // Register global hotkeys
  globalShortcut.register('Alt+D', () => {
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'toggle-overlay');
    }
  });

  globalShortcut.register('Ctrl+C', () => {
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'copy-item');
    }
  });

  globalShortcut.register('Alt+T', () => {
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'toggle-trade-search');
    }
  });

  globalShortcut.register('Alt+B', () => {
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'toggle-build-guide');
    }
  });

  globalShortcut.register('Alt+P', () => {
    if (mainWindow) {
      mainWindow.webContents.send('hotkey-pressed', 'toggle-price-check');
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (mainWindow === null) {
    createWindow();
  }
});

app.on('will-quit', () => {
  globalShortcut.unregisterAll();
});

// IPC handlers
ipcMain.on('set-click-through', (event, enabled) => {
  if (mainWindow) {
    mainWindow.setIgnoreMouseEvents(enabled, { forward: true });
  }
});

ipcMain.on('set-always-on-top', (event, enabled) => {
  if (mainWindow) {
    mainWindow.setAlwaysOnTop(enabled);
  }
});

ipcMain.on('minimize-window', () => {
  if (mainWindow) {
    mainWindow.minimize();
  }
});

ipcMain.on('close-window', () => {
  if (mainWindow) {
    mainWindow.close();
  }
});
