# Path of Exile Overlay - User Guide (BETA v1.0.0)

## 🎮 Welcome to POE Overlay!

A feature-rich, transparent overlay for Path of Exile that provides real-time item price checking, build guides from Maxroll.gg, currency exchange rates, map mod analysis, and more!

---

## 📋 Table of Contents
1. [Installation](#installation)
2. [Quick Start](#quick-start)
3. [Features](#features)
4. [Hotkeys](#hotkeys)
5. [Feature Guides](#feature-guides)
6. [Troubleshooting](#troubleshooting)
7. [FAQ](#faq)

---

## 🔧 Installation

### Prerequisites
- **Operating System**: Windows 10/11, Linux, or macOS
- **Path of Exile**: Game must be installed
- **Node.js**: Version 16 or higher (for development)

### Installation Steps

1. **Download the latest release** from the releases page (coming soon)
2. **Extract the archive** to your desired location
3. **Run `POE Overlay.exe`** (Windows) or the appropriate executable for your OS

### Building from Source

```bash
# Clone the repository
git clone <repository-url>
cd codetest

# Install dependencies
npm install

# Build the application
npm run build

# Run in development mode
npm start
```

---

## 🚀 Quick Start

### First Launch
1. Launch **POE Overlay**
2. A transparent overlay will appear on your screen
3. Launch **Path of Exile**
4. The overlay will stay on top of the game

### Basic Usage
1. **Price Check an Item**: Hover over an item in POE, press `Ctrl+C` to copy, then press `Ctrl+Shift+P`
2. **Toggle Overlay**: Press `Ctrl+Shift+T` to show/hide the overlay
3. **Access Settings**: Click the gear icon or press `Ctrl+Shift+S`

---

## ✨ Features

### 1. **Item Price Checking** 💰
- Real-time price data from POE Trade API
- Automatic parsing of all item properties
- Price ranges (min/avg/max/median)
- Confidence scoring
- Similar items display

### 2. **Build Guides** 📚
- Integration with Maxroll.gg
- Browse builds by class and ascendancy
- View skill setups and item recommendations
- Passive tree information
- League starter builds

### 3. **Currency Exchange** 💱
- Live exchange rates from POE.ninja
- Chaos/Divine/Exalt conversions
- Bulk exchange calculator
- Auto-refresh every 5 minutes

### 4. **Map Mod Checker** 🗺️
- Analyzes dangerous map modifiers
- Build-specific warnings
- Severity indicators
- Boss mechanics information

### 5. **Hotkey System** ⌨️
- Global hotkeys (work even when POE is focused)
- Customizable key bindings
- Quick access to all features

### 6. **Overlay Management** 🎛️
- Draggable panels
- Resizable windows
- Position persistence
- Click-through mode
- Always-on-top

---

## ⌨️ Hotkeys

### Default Hotkeys

| Hotkey | Action | Description |
|--------|--------|-------------|
| **Ctrl+Shift+P** | Price Check | Show price for copied item |
| **Ctrl+Shift+T** | Toggle Overlay | Show/hide the entire overlay |
| **Ctrl+Shift+B** | Build Guides | Open build guide browser |
| **Ctrl+Shift+M** | Map Mods | Open map mod checker |
| **Ctrl+Shift+C** | Currency | Show currency exchange rates |
| **Ctrl+Shift+X** | Click-Through | Toggle click-through mode |
| **Ctrl+Shift+S** | Settings | Open settings panel |
| **Ctrl+Shift+R** | Reload | Reload overlay (dev mode) |
| **Ctrl+Shift+Q** | Quit | Close the application |

### Customizing Hotkeys
1. Open **Settings** (`Ctrl+Shift+S`)
2. Go to **Hotkeys** tab
3. Click on a hotkey to record a new key combination
4. Press **Save** to apply changes

---

## 📖 Feature Guides

### Price Checking an Item

1. **In Path of Exile**: Hover over any item
2. **Copy**: Press `Ctrl+C` (POE's copy item hotkey)
3. **Price Check**: Press `Ctrl+Shift+P`
4. **View Results**: The Price Display panel shows:
   - Min/Max/Average/Median prices
   - Number of listings
   - Confidence rating
   - Price recommendations (Quick Sell, Fair Price, High Price)
5. **Search Similar**: Click "Search Similar" to see comparable items on trade

### Using Build Guides

1. **Open Builds**: Press `Ctrl+Shift+B`
2. **Filter**: Select class, ascendancy, or difficulty
3. **Search**: Use the search bar to find specific builds
4. **View Details**: Click on a build to see:
   - Skill gem setups
   - Recommended items
   - Passive tree information
   - Build pros/cons
5. **Open in Maxroll**: Click "View on Maxroll.gg" for full guide

### Checking Currency Rates

1. **Open Currency**: Press `Ctrl+Shift+C`
2. **View Rates**: See live exchange rates for:
   - Chaos Orb
   - Divine Orb
   - Exalted Orb
   - And more...
3. **Convert**: Use the calculator to convert between currencies
4. **Auto-Refresh**: Rates update every 5 minutes

### Analyzing Map Mods

1. **Copy Map**: In POE, press `Ctrl+C` on a map
2. **Open Checker**: Press `Ctrl+Shift+M`
3. **Paste Mods**: The map mods are automatically pasted
4. **Configure Build**: Select your damage type and defenses
5. **View Warnings**: See dangerous mods highlighted with:
   - **Red**: Deadly (avoid or reroll)
   - **Orange**: High danger (be careful)
   - **Yellow**: Medium danger (manageable)
   - **Green**: Low danger (safe)
6. **Read Details**: Hover over warnings for explanations and solutions

---

## 🔧 Settings

### General Settings
- **Theme**: Dark (default) or Light
- **Opacity**: Adjust overlay transparency
- **Compact Mode**: Reduce panel sizes
- **Always on Top**: Keep overlay above all windows
- **Click-Through**: Allow clicks to pass through to POE

### Panel Management
- **Show/Hide Panels**: Toggle visibility of each panel
- **Reset Layout**: Restore default panel positions
- **Save Layout**: Automatically saves positions

### Performance
- **Auto-Refresh Intervals**: Customize update frequencies
- **Cache Duration**: Adjust cache lifetimes
- **Rate Limiting**: Respect API limits

---

## 🐛 Troubleshooting

### Overlay Not Appearing
- **Check if running**: Look for POE Overlay in system tray
- **Restart**: Close and reopen the application
- **Check hotkeys**: Press `Ctrl+Shift+T` to toggle visibility
- **Monitor**: Ensure overlay is on the correct monitor

### Hotkeys Not Working
- **Check conflicts**: Ensure no other apps use the same hotkeys
- **Run as admin**: Some games require admin privileges
- **Reconfigure**: Try changing the hotkey combination
- **Restart**: Restart POE Overlay after changing hotkeys

### Price Check Not Working
- **Copy item**: Ensure you pressed `Ctrl+C` on an item in POE
- **Wait**: Price checks may take 1-2 seconds
- **Check internet**: Requires internet for POE Trade API
- **Rate limit**: You may be rate limited if checking too many items quickly

### Build Guides Not Loading
- **Internet connection**: Requires internet to fetch from Maxroll
- **Cache**: Try clearing cache in settings
- **Maxroll status**: Check if Maxroll.gg is accessible

### Performance Issues
- **Close unnecessary panels**: Hide panels you're not using
- **Reduce opacity**: Lower values use less GPU
- **Compact mode**: Enable for better performance
- **Update drivers**: Ensure graphics drivers are up to date

---

## ❓ FAQ

### Is this allowed by GGG?
POE Overlay only reads information from the clipboard and displays it in an overlay. It does not modify game files or automate gameplay. However, use at your own risk.

### Does it work with all item types?
Yes! The overlay supports:
- Equipment (weapons, armor, jewelry)
- Currency items
- Maps
- Skill gems
- Divination cards
- Unique items
- And more...

### Can I use it with multiple monitors?
Yes, the overlay detects your primary monitor. You can drag panels to any monitor.

### Does it affect game performance?
Minimal impact. The overlay is hardware-accelerated and uses very little CPU/GPU.

### How accurate are the prices?
Prices are pulled directly from the official POE Trade API. Accuracy depends on market activity and number of listings.

### Can I suggest features?
Absolutely! Open an issue on GitHub with your suggestions.

### How do I update?
Download the latest release and replace your existing installation. Settings are preserved.

---

## 🤝 Contributing

We welcome contributions! To contribute:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

---

## 📝 License

This project is licensed under the MIT License. See LICENSE file for details.

---

## 🙏 Credits

- **Path of Exile**: Grinding Gear Games
- **Maxroll.gg**: Build guides and game data
- **POE.ninja**: Currency exchange rates
- **Community**: For feedback and suggestions

---

## 📞 Support

- **Issues**: Report bugs on GitHub
- **Discussions**: Join community discussions
- **Discord**: (Coming soon)

---

## 🔄 Changelog

### Version 1.0.0 (BETA) - 2025-11-17
- ✨ Initial beta release
- ✅ Item price checking
- ✅ Build guides integration
- ✅ Currency exchange tracker
- ✅ Map mod analyzer
- ✅ Global hotkey system
- ✅ Draggable/resizable panels
- ✅ Settings persistence

---

**Enjoy your POE experience with the overlay! Happy grinding, Exile! 💎**
