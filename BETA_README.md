# Path of Exile Overlay - BETA v1.0.0

## 🎯 Overview

A feature-rich, transparent overlay application for Path of Exile built with Electron, React, and TypeScript. Provides real-time item price checking, build guides from Maxroll.gg, currency exchange rates, map mod analysis, and more!

![POE Overlay](https://via.placeholder.com/800x400?text=POE+Overlay+Screenshot)

---

## ✨ Key Features

### 🔍 **Item Price Checking**
- Parse items directly from clipboard (`Ctrl+C` in game)
- Real-time prices from official POE Trade API
- Price ranges with confidence scoring
- Similar items search
- Support for all item types

### 📚 **Build Guides (Maxroll Integration)**
- Browse builds by class/ascendancy
- View skill setups and gem links
- Item recommendations
- Passive tree information
- League starter builds

### 💱 **Currency Exchange**
- Live rates from POE.ninja
- Chaos/Divine/Exalt conversions
- Currency calculator
- Auto-refresh every 5 minutes

### 🗺️ **Map Mod Checker**
- Dangerous mod warnings
- Build-specific analysis
- Severity indicators (deadly/high/medium/low)
- Boss mechanics database

### ⌨️ **Global Hotkeys**
- `Ctrl+Shift+P` - Price check
- `Ctrl+Shift+B` - Build guides
- `Ctrl+Shift+C` - Currency rates
- `Ctrl+Shift+M` - Map mods
- `Ctrl+Shift+T` - Toggle overlay
- And more... (all customizable!)

### 🎨 **Overlay Features**
- Transparent, always-on-top window
- Draggable and resizable panels
- Click-through mode
- Position persistence
- POE-themed dark UI
- Customizable opacity

---

## 🚀 Quick Start

### Installation

```bash
# Install dependencies
npm install

# Build the application
npm run build

# Run in development mode
npm start
```

### For End Users
- Download the latest release from the releases page
- Extract and run `POE Overlay.exe`
- Press `Ctrl+Shift+T` to toggle the overlay

---

## 📦 Project Structure

```
codetest/
├── src/
│   ├── main/                 # Electron main process
│   │   └── main.js           # Main Electron window & IPC
│   ├── preload/              # Preload scripts
│   │   └── preload.js        # Secure IPC bridge
│   ├── renderer/             # React application
│   │   ├── components/       # React components
│   │   │   ├── OverlayContainer/
│   │   │   ├── PriceDisplay/
│   │   │   ├── BuildGuideViewer/
│   │   │   ├── CurrencyTracker/
│   │   │   ├── MapModChecker/
│   │   │   ├── SearchPanel/
│   │   │   ├── InfoPanel/
│   │   │   ├── SettingsPanel/
│   │   │   ├── HotkeyManager/
│   │   │   └── shared/Panel/
│   │   ├── contexts/         # React contexts
│   │   │   └── OverlayContext.tsx
│   │   ├── styles/           # SCSS styles
│   │   │   ├── _variables.scss
│   │   │   ├── _mixins.scss
│   │   │   └── global.scss
│   │   ├── types/            # TypeScript types
│   │   ├── App.tsx           # Main React component
│   │   └── index.js          # React entry point
│   ├── services/             # Business logic
│   │   ├── clipboardMonitor.js
│   │   ├── itemParser.js
│   │   ├── tradeService.js
│   │   ├── priceChecker.js
│   │   ├── currencyService.js
│   │   ├── mapModService.js
│   │   ├── maxroll/          # Maxroll integration
│   │   │   ├── buildGuideService.js
│   │   │   ├── skillDataService.js
│   │   │   ├── itemDataService.js
│   │   │   └── searchService.js
│   │   ├── cache/            # Caching system
│   │   │   └── cacheService.js
│   │   └── utils/
│   │       └── rateLimiter.js
│   └── models/               # Data models
│       ├── Build.js
│       ├── Skill.js
│       └── Item.js
├── build/                    # Webpack output
├── electron/                 # Electron config (deprecated)
├── public/                   # Static assets
├── docs/                     # Documentation
│   ├── USER_GUIDE.md        # User documentation
│   ├── INTEGRATION_COMPLETE.md
│   ├── BUG_FIXES.md
│   └── HOTKEYS_REFERENCE.md
├── package.json              # Dependencies
├── webpack.config.js         # Webpack configuration
├── tsconfig.json             # TypeScript configuration
└── .babelrc                  # Babel configuration
```

---

## 🛠️ Tech Stack

- **Electron** 27.0.0 - Desktop framework
- **React** 18.2.0 - UI library
- **TypeScript** 5.2.2 - Type safety
- **Webpack** 5 - Module bundler
- **Sass** - CSS preprocessing
- **Axios** - HTTP client
- **react-draggable** - Draggable panels
- **react-resizable** - Resizable panels
- **electron-log** - Logging
- **electron-store** - Settings persistence

---

## 🏗️ Architecture

### Main Process (Electron)
- Manages application window
- Handles global hotkeys
- IPC communication with renderer
- Security: context isolation, sandboxing

### Preload Script
- Secure bridge between main and renderer
- Exposes safe IPC methods
- Clipboard access
- Logging interface

### Renderer Process (React)
- UI components
- State management (React Context)
- Service integration
- User interactions

### Services Layer
- **clipboardMonitor**: Monitors clipboard for POE items
- **itemParser**: Parses POE item format
- **tradeService**: POE Trade API integration
- **priceChecker**: Price aggregation and recommendations
- **currencyService**: POE.ninja currency rates
- **mapModService**: Map modifier analysis
- **maxroll/**: Maxroll.gg build guide integration
- **cacheService**: Smart caching with TTL
- **rateLimiter**: API rate limiting

---

## 🔒 Security

- ✅ **Context Isolation**: Enabled
- ✅ **Node Integration**: Disabled in renderer
- ✅ **Sandbox**: Enabled
- ✅ **Preload Script**: Secure IPC bridge
- ✅ **CSP Headers**: Content Security Policy
- ✅ **No Remote Module**: Disabled for security

---

## 📊 Statistics

- **Total Files**: 60+
- **Lines of Code**: ~8,000+
- **Components**: 11 React components
- **Services**: 12 service modules
- **Models**: 3 data models
- **Supported Item Types**: All POE items
- **API Integrations**: 3 (POE Trade, POE.ninja, Maxroll)

---

## 🧪 Testing

### Code Validation
```bash
# Run syntax validation
node validate-code.js

# All 22 JavaScript files pass validation
```

### Build Test
```bash
# Build production bundle
npm run build

# Output: build/main.bundle.js, build/vendors.bundle.js
```

---

## 📝 Development

### Scripts

```json
{
  "start": "npm run start:react && npm run start:electron",
  "start:react": "webpack serve --mode development",
  "start:electron": "electron .",
  "build": "webpack --mode production",
  "build:electron": "electron-builder",
  "pack": "electron-builder --dir",
  "dist": "npm run build && electron-builder"
}
```

### Environment Variables

```env
NODE_ENV=development  # or production
```

### Adding New Features

1. Create service in `src/services/`
2. Create React component in `src/renderer/components/`
3. Add to `OverlayContainer`
4. Update `OverlayContext` for state
5. Add hotkey in `main.js`
6. Update documentation

---

## 🚨 Known Issues (Beta)

### Limitations
- **Electron Binary**: May fail to download in restricted networks (use `npm install --ignore-scripts`)
- **Maxroll Data**: Currently uses mock data (implement scraping for production)
- **POE.ninja**: Requires internet connection
- **Rate Limits**: Aggressive price checking may hit API limits

### Warnings
- **SASS Deprecations**: `@import` rules are deprecated (migrate to `@use` in future)
- **Security**: Use at own risk, may violate POE ToS regarding automated tools

---

## 🔮 Future Enhancements

### Planned Features
- [ ] Stash tab organization
- [ ] Trade whisper management
- [ ] Divination card tracking
- [ ] Atlas strategy recommendations
- [ ] Crafting calculator
- [ ] Delve fossil planner
- [ ] Harvest craft database
- [ ] Custom sound alerts
- [ ] Multi-language support
- [ ] Cloud settings sync

### Technical Improvements
- [ ] Migrate SASS `@import` to `@use`
- [ ] Add comprehensive unit tests
- [ ] Implement E2E testing
- [ ] CI/CD pipeline
- [ ] Auto-updates
- [ ] Installer creation
- [ ] Code signing
- [ ] Performance optimizations

---

## 🤝 Contributing

We welcome contributions! Here's how:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/AmazingFeature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/AmazingFeature`)
5. **Open** a Pull Request

### Contribution Guidelines
- Follow existing code style
- Add tests for new features
- Update documentation
- Ensure all validation passes

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **Grinding Gear Games** - Path of Exile
- **Maxroll.gg** - Build guides and game data
- **POE.ninja** - Currency exchange rates
- **POE Community** - Feedback and support
- **Electron Team** - Desktop framework
- **React Team** - UI library

---

## 📞 Support

- **GitHub Issues**: Report bugs and request features
- **Discussions**: Community discussions and Q&A
- **Discord**: (Coming soon)
- **Email**: (Coming soon)

---

## 📋 Changelog

### v1.0.0-beta (2025-11-17)

**Initial Beta Release**

#### Features
- ✨ Item price checking with POE Trade API
- ✨ Build guides from Maxroll.gg
- ✨ Currency exchange tracker (POE.ninja)
- ✨ Map mod analyzer with danger warnings
- ✨ Global hotkey system
- ✨ Draggable/resizable panels
- ✨ Settings persistence
- ✨ Click-through mode
- ✨ POE-themed dark UI

#### Technical
- ✅ Electron 27 + React 18
- ✅ TypeScript support
- ✅ Webpack 5 bundling
- ✅ SCSS styling
- ✅ Secure IPC communication
- ✅ Code validation passed (22/22 files)
- ✅ Production build successful

#### Bug Fixes
- 🐛 Fixed module system inconsistencies
- 🐛 Fixed Electron security configuration
- 🐛 Fixed Babel configuration
- 🐛 Fixed webpack output conflicts
- 🐛 Added missing SASS mixin

---

## 🎯 Beta Testing Feedback

We need your help to make POE Overlay better! Please test and report:

1. **Functionality Issues**: Features not working as expected
2. **Performance**: Lag, crashes, memory leaks
3. **UI/UX**: Confusing layouts, missing information
4. **Compatibility**: Issues with specific POE versions or OS
5. **Feature Requests**: What would you like to see?

**Report via GitHub Issues with:**
- POE Overlay version
- Operating System
- POE version
- Steps to reproduce
- Screenshots/logs if possible

---

**Made with ❤️ for the Path of Exile community**

**Happy grinding, Exiles! 💎⚔️🔥**
