# POE Overlay - Quick Start Guide

## Installation

### 1. Install Dependencies

```bash
cd /home/user/codetest
npm install
```

This will install all required packages:
- React 18
- Electron 27
- Webpack 5
- TypeScript
- SCSS loaders
- react-draggable & react-resizable

### 2. Start Development Mode

```bash
npm start
```

This command will:
1. Start the Webpack dev server on port 3000
2. Wait for the dev server to be ready
3. Launch the Electron window

Alternatively, run them separately:
```bash
# Terminal 1
npm run start:react

# Terminal 2 (after webpack is ready)
npm run start:electron
```

### 3. Using the Overlay

#### Default Hotkeys:
- **Alt + D**: Toggle entire overlay
- **Alt + T**: Toggle trade search panel
- **Alt + P**: Toggle price check panel
- **Alt + B**: Toggle build guide panel
- **Ctrl + C**: Copy item (placeholder)

#### Panel Controls:
- **Drag**: Click and drag the panel header
- **Resize**: Drag any corner or edge
- **Minimize**: Click the `_` button
- **Close**: Click the `×` button

#### Opening Panels:
Since this is first startup, use the Settings panel to toggle panels:
1. Press `Alt + D` to show the overlay
2. Open Settings panel from code or add a UI trigger
3. Check the boxes for panels you want visible

Or programmatically toggle panels in the browser console:
```javascript
// Open all panels for testing
['search', 'info', 'price', 'build', 'settings', 'hotkeys'].forEach(id => {
  localStorage.setItem('poe-overlay-settings', JSON.stringify({
    ...JSON.parse(localStorage.getItem('poe-overlay-settings') || '{}'),
    panels: {
      ...JSON.parse(localStorage.getItem('poe-overlay-settings') || '{}').panels,
      [id]: { ...panels[id], visible: true }
    }
  }));
});
```

## Building for Production

### Build the React App
```bash
npm run build
```

This creates optimized bundles in the `build/` directory.

### Package as Electron App
```bash
npm run dist
```

This creates distributable packages:
- **Windows**: `.exe` installer
- **Linux**: `.AppImage` or `.deb`

Output will be in the `dist/` directory.

## Development Tips

### Hot Reload
- Changes to React components auto-reload
- Changes to Electron main process require restart
- SCSS changes auto-reload

### DevTools
- Automatically opens in development mode
- Press `Ctrl+Shift+I` to toggle
- Use React DevTools extension for component inspection

### Debugging
- **Main Process**: Add `console.log` in `electron/main.js`
- **Renderer Process**: Use browser DevTools console
- **IPC Messages**: Log in both main.js and components

### State Inspection
Access global state in DevTools console:
```javascript
// View current settings
JSON.parse(localStorage.getItem('poe-overlay-settings'))

// Reset settings
localStorage.removeItem('poe-overlay-settings')
```

## File Structure Reference

```
src/renderer/
├── components/           # UI Components
│   ├── OverlayContainer/ # Main container
│   ├── SearchPanel/      # Trade search
│   ├── InfoPanel/        # Item info
│   ├── PriceDisplay/     # Price checking
│   ├── BuildGuideViewer/ # Build guides
│   ├── SettingsPanel/    # Settings
│   ├── HotkeyManager/    # Hotkey config
│   └── shared/
│       └── Panel/        # Reusable panel
├── contexts/             # React Context
├── styles/               # SCSS files
├── types/                # TypeScript types
├── App.tsx               # Root component
└── index.js              # Entry point
```

## Customization

### Theme Colors
Edit `/home/user/codetest/src/renderer/styles/_variables.scss`:
```scss
$poe-gold: #c8aa6e;     // Primary color
$poe-dark-bg: #0a0a0a;  // Background
```

### Panel Defaults
Edit `/home/user/codetest/src/renderer/contexts/OverlayContext.tsx`:
```typescript
const defaultSettings = {
  theme: 'dark',
  opacity: 0.95,
  // ... modify defaults
};
```

### Hotkeys
Edit `/home/user/codetest/electron/main.js`:
```javascript
globalShortcut.register('Alt+D', () => {
  // Your custom action
});
```

## Common Issues

### Port 3000 in use
```bash
# Change port in webpack.config.js
devServer: {
  port: 3001, // Change this
}
```

### Panels not showing
1. Check localStorage: `localStorage.getItem('poe-overlay-settings')`
2. Reset layout from Settings panel
3. Clear storage and restart: `localStorage.clear()`

### TypeScript errors
```bash
# Rebuild TypeScript
npx tsc --noEmit
```

### Build fails
```bash
# Clean build
rm -rf build/ dist/ node_modules/
npm install
npm run build
```

## Next Steps

### API Integration
The app is ready to integrate:
1. **POE Trade API** - Add API calls in SearchPanel
2. **Price Checking** - Integrate poe.ninja or similar
3. **Build Guides** - Connect to Maxroll API
4. **Item Parsing** - Add clipboard monitoring

### Example API Integration
```typescript
// In SearchPanel.tsx
const handleSearch = async () => {
  const response = await fetch('https://www.pathofexile.com/api/trade/search/...');
  const data = await response.json();
  setSearchResults(data.result);
};
```

### Clipboard Monitoring
```typescript
// In App.tsx or OverlayContainer
useEffect(() => {
  const { clipboard } = window.require('electron');
  const interval = setInterval(() => {
    const text = clipboard.readText();
    // Parse POE item text
  }, 500);
  return () => clearInterval(interval);
}, []);
```

## Resources

- [Electron Docs](https://www.electronjs.org/docs)
- [React Docs](https://react.dev)
- [POE Trade API](https://www.pathofexile.com/developer/docs)
- [Webpack Docs](https://webpack.js.org)

## Support

For issues or questions:
1. Check POE_OVERLAY_README.md for architecture details
2. Review component source code
3. Check browser DevTools console for errors
4. Verify Electron main process logs

Happy coding!
