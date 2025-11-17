/**
 * Main Entry Point for POE Trade Overlay
 */

const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');

const ClipboardMonitor = require('./services/clipboardMonitor');
const PriceChecker = require('./services/priceChecker');

let mainWindow;
let clipboardMonitor;
let priceChecker;

const CURRENT_LEAGUE = 'Standard';

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 400,
    height: 600,
    frame: false,
    transparent: true,
    alwaysOnTop: true,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false
    }
  });

  mainWindow.loadFile(path.join(__dirname, '../public/index.html'));
}

function initializeServices() {
  clipboardMonitor = new ClipboardMonitor();
  priceChecker = new PriceChecker(CURRENT_LEAGUE);

  clipboardMonitor.start();

  clipboardMonitor.onItemDetected(async (itemText) => {
    console.log('Item detected, checking price...');
    
    try {
      const result = await priceChecker.checkPrice(itemText);
      
      if (result.success) {
        mainWindow.webContents.send('price-check-result', result);
      }
    } catch (error) {
      console.error('Error during price check:', error);
    }
  });

  console.log('Services initialized');
}

app.whenReady().then(() => {
  createWindow();
  initializeServices();
});

app.on('window-all-closed', () => {
  if (clipboardMonitor) {
    clipboardMonitor.stop();
  }
  
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

ipcMain.handle('set-league', async (event, league) => {
  if (priceChecker) {
    priceChecker.setLeague(league);
    return { success: true, league };
  }
  return { success: false, error: 'Price checker not initialized' };
});

ipcMain.handle('manual-price-check', async (event, itemText) => {
  if (!priceChecker) {
    return { success: false, error: 'Price checker not initialized' };
  }

  try {
    const result = await priceChecker.checkPrice(itemText);
    return result;
  } catch (error) {
    return { success: false, error: error.message };
  }
});

console.log('POE Trade Overlay starting...');
