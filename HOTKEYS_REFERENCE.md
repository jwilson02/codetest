# POE Overlay - Global Hotkeys Reference

## Quick Reference

All hotkeys work globally, even when Path of Exile is focused.

| Hotkey | Action | Description |
|--------|--------|-------------|
| **Ctrl+Shift+P** | Price Check | Opens price display panel and reads clipboard for item |
| **Ctrl+Shift+B** | Build Guide | Opens build guide viewer panel |
| **Ctrl+Shift+M** | Map Mod Checker | Opens map mod analyzer panel |
| **Ctrl+Shift+C** | Currency Tracker | Opens currency exchange rate tracker |
| **Ctrl+Shift+T** | Toggle Overlay | Shows/hides the entire overlay |
| **Ctrl+Shift+X** | Click-Through | Toggles click-through mode (mouse passes through overlay) |
| **Ctrl+Shift+R** | Reload | Reloads the overlay (development only) |
| **Ctrl+Shift+Q** | Quit | Closes the application |

## Usage Tips

### Price Checking
1. In POE, hover over an item and press **Ctrl+C** to copy it
2. Press **Ctrl+Shift+P** to open the price checker
3. The overlay will automatically read the clipboard and analyze the item

### Build Guides
1. Press **Ctrl+Shift+B** to browse available builds
2. Click on a build to view detailed information
3. See gem links, items, passive tree, and more

### Map Mod Analysis
1. In POE, hover over a map and press **Ctrl+C** to copy it
2. Press **Ctrl+Shift+M** to open the map mod checker
3. Click "Paste from Clipboard" to analyze the map
4. Set your build type for personalized warnings

### Currency Tracking
1. Press **Ctrl+Shift+C** to open the currency tracker
2. View real-time exchange rates for all currencies
3. Use the converter to calculate currency exchanges

### Click-Through Mode
1. Press **Ctrl+Shift+X** to enable click-through
2. Your clicks will pass through the overlay to POE
3. Press **Ctrl+Shift+X** again to interact with the overlay

### Toggle Visibility
1. Press **Ctrl+Shift+T** to hide the overlay completely
2. Press **Ctrl+Shift+T** again to show it
3. Useful when you need full screen visibility

## Customization

To change hotkeys, edit `/home/user/codetest/src/main/main.js` in the `registerShortcuts()` function (lines 115-188).

Example:
```javascript
// Change price check from Ctrl+Shift+P to Alt+P
globalShortcut.register('Alt+P', () => {
  log.info('Price check shortcut pressed');
  if (mainWindow) {
    mainWindow.webContents.send('hotkey-pressed', 'price-check');
  }
});
```

## Troubleshooting

### Hotkey Not Working
- Ensure no other application is using the same combination
- Check the console for "Global shortcuts registered" message
- Try restarting the overlay

### Multiple Hotkeys Conflict
- Some applications may capture hotkeys before the overlay
- Try alternative key combinations
- Close conflicting applications

### Hotkeys Work But Panel Doesn't Open
- Open DevTools (F12) and check for errors
- Verify the panel is not already open but minimized
- Try toggling the panel twice

## Platform Differences

### Windows
- Uses **Ctrl** key as shown above

### macOS
- Uses **Command (⌘)** instead of Ctrl
- Example: **Cmd+Shift+P** for price check

### Linux
- Uses **Ctrl** key as shown above
- Some desktop environments may have conflicting shortcuts

---

**Remember:** All hotkeys are global and work even when Path of Exile is the active window. This is the main advantage of using an overlay!
