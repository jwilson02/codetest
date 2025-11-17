# Path of Exile Overlay - v1.1.0 (Keepers of the Flame)

<div align="center">

**A feature-rich, transparent overlay for Path of Exile**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Electron](https://img.shields.io/badge/Electron-27.0-blue.svg)](https://www.electronjs.org/)
[![React](https://img.shields.io/badge/React-18.2-blue.svg)](https://reactjs.org/)
[![TypeScript](https://img.shields.io/badge/TypeScript-5.2-blue.svg)](https://www.typescriptlang.org/)

</div>

---

## 🎯 Overview

POE Overlay is a powerful desktop application that provides real-time **item price checking**, **build guides from Maxroll.gg**, **currency exchange rates**, and **map mod analysis** for Path of Exile. Built with Electron, React, and TypeScript for a smooth, professional experience.

## ✨ Key Features

### **🆕 v1.1.0 - Keepers of the Flame (POE 3.27) - 11 NEW FEATURES!**
- 💜 **Breach Helper** - Track Ailith, Hives, Unstable Breaches, splinters
- 🌳 **Genesis Tree** - 20 passives, Graftblood currency, 4 reward tiers
- 🧬 **Grafts System** - 16 graft types, 2 slots, active skills
- ⚔️ **Bloodline Ascendancy** - 10 secondary ascendancies with synergy calculator
- 💬 **Trade Whisper Manager** - Auto-parse whispers, quick responses, async trading
- 🏪 **Vendor Recipes** - 30+ recipes with profitability indicators
- 🃏 **Divination Cards** - Track 22+ cards with farming locations
- 🗺️ **Atlas Tracker** - Voidstones, Maven, 135 maps, 12 uber bosses
- 🔨 **Crafting Helper** - Fossils, essences, harvest, eldritch crafting
- 🎭 **Betrayal Board** - Syndicate optimization with profit strategies

### **Core Features (v1.0.0)**
- 🔍 **Item Price Checking** - Real-time prices from POE Trade API with confidence scoring
- 📚 **Build Guides** - Integration with Maxroll.gg for class builds and skill setups
- 💱 **Currency Exchange** - Live rates from POE.ninja with auto-refresh
- 🗺️ **Map Mod Checker** - Dangerous mod warnings with build-specific analysis
- ⌨️ **Global Hotkeys** - 18 system-wide hotkeys that work even when POE is focused
- 🎨 **Professional UI** - Transparent overlay with draggable/resizable panels

## 📦 Quick Start

### Installation

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

## ⌨️ Default Hotkeys

| Hotkey | Action |
|--------|--------|
| **Ctrl+Shift+P** | Price check copied item |
| **Ctrl+Shift+B** | Open build guides |
| **Ctrl+Shift+C** | Show currency rates |
| **Ctrl+Shift+M** | Map mod checker |
| **Ctrl+Shift+T** | Toggle overlay |
| **Ctrl+Shift+X** | Toggle click-through |

All hotkeys are customizable in settings!

## 📚 Documentation

- **[Changelog v1.1.0](CHANGELOG_v1.1.0.md)** - ⭐ NEW: Complete changelog for Keepers of the Flame update
- **[Features v1.1.0](FEATURES_v1.1.0.md)** - ⭐ NEW: Detailed guide for all 11 new features
- **[User Guide](USER_GUIDE.md)** - Complete user documentation
- **[Beta README](BETA_README.md)** - Technical details and architecture
- **[Integration Guide](INTEGRATION_GUIDE.md)** - API integration documentation
- **[Hotkeys Reference](HOTKEYS_REFERENCE.md)** - Complete hotkey guide (18 total)
- **[Bug Fixes](BUG_FIXES.md)** - List of fixes applied

## 🏗️ Project Structure

```
codetest/
├── src/
│   ├── main/                  # Electron main process
│   ├── preload/               # Secure IPC bridge
│   ├── renderer/              # React application
│   │   ├── components/        # 11 React components
│   │   ├── contexts/          # State management
│   │   └── styles/            # SCSS styling
│   ├── services/              # Business logic
│   │   ├── clipboardMonitor.js
│   │   ├── itemParser.js
│   │   ├── tradeService.js
│   │   ├── priceChecker.js
│   │   ├── currencyService.js
│   │   ├── mapModService.js
│   │   └── maxroll/           # Maxroll integration
│   └── models/                # Data models
├── build/                     # Webpack output
├── docs/                      # Documentation
└── package.json               # Dependencies
```

## 🛠️ Tech Stack

- **Electron** 27.0 - Desktop framework
- **React** 18.2 - UI library
- **TypeScript** 5.2 - Type safety
- **Webpack** 5 - Module bundler
- **Sass** - CSS preprocessing

## 🔒 Security

- ✅ Context Isolation enabled
- ✅ Node Integration disabled
- ✅ Sandbox enabled
- ✅ Secure IPC communication
- ✅ Content Security Policy

## 🚀 Development

### Available Scripts

```bash
npm start          # Start development server with hot reload
npm run build      # Build production bundle
npm run pack       # Package application (no installer)
npm run dist       # Build and create installer
```

### Building for Production

```bash
# Build React application
npm run build

# Package for Windows
npm run build:win

# Package for Linux
npm run build:linux

# Package for all platforms
npm run build:all
```

## 🐛 Known Issues (Beta)

- Electron may fail to install in restricted networks (use `npm install --ignore-scripts`)
- Maxroll integration uses mock data (implement scraping for production)
- SASS deprecation warnings (migrate to `@use` in future)

## 🔮 Roadmap

- [ ] Stash tab organization
- [ ] Trade whisper management
- [ ] Divination card tracking
- [ ] Crafting calculator
- [ ] Atlas strategy recommendations
- [ ] Multi-language support

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Grinding Gear Games** - Path of Exile
- **Maxroll.gg** - Build guides
- **POE.ninja** - Currency rates
- **POE Community** - Support and feedback

## 📞 Support

- **GitHub Issues** - Report bugs and request features
- **Discussions** - Community Q&A

---

## 📋 Changelog

### v1.1.0 (2025-11-17) - Keepers of the Flame Update

**Major Feature Release - 11 NEW Features!**

#### **Keepers of the Flame (POE 3.27) Features:**
- ✨ Breach Helper - Track Ailith, Hives, Unstable Breaches, splinters
- ✨ Genesis Tree Tracker - 20 passives, Graftblood, 4 reward tiers
- ✨ Grafts System Planner - 16 graft types with active skills
- ✨ Bloodline Ascendancy Planner - 10 secondary ascendancies

#### **Trading & Economy Features:**
- ✨ Trade Whisper Manager - Auto-parse, quick responses, async trading (3.27)
- ✨ Vendor Recipe Database - 30+ recipes with profit indicators

#### **Progression Tracking Features:**
- ✨ Divination Card Tracker - 22+ cards with farming guides
- ✨ Atlas Progression Tracker - Voidstones, Maven, 135 maps, 12 bosses

#### **Crafting & Endgame Features:**
- ✨ Crafting Helper - Fossils, essences, harvest, eldritch
- ✨ Betrayal Board Assistant - Syndicate optimization

#### **System Improvements:**
- ✨ Added 10 new global hotkeys (18 total)
- ✨ Updated for POE 3.27 league content
- ✨ All new panels with POE-themed styling
- ✨ LocalStorage persistence for all new features

**See [CHANGELOG_v1.1.0.md](CHANGELOG_v1.1.0.md) for complete details.**

---

### v1.0.0-beta (2025-11-17)

**Initial Beta Release**

- ✨ Item price checking with POE Trade API
- ✨ Build guides from Maxroll.gg
- ✨ Currency exchange tracker
- ✨ Map mod analyzer
- ✨ Global hotkey system
- ✨ Draggable/resizable panels
- ✨ Settings persistence
- ✨ POE-themed dark UI

---

**Made with ❤️ for the Path of Exile community**

**Happy grinding, Exiles! 💎⚔️🔥**
