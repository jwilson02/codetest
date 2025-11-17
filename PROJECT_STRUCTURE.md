# Project Structure Summary

## Overview
Electron + React overlay application for Path of Exile with TypeScript support.

## Root Configuration Files

- **package.json** - Project dependencies and npm scripts
- **webpack.config.js** - Webpack bundler configuration
- **tsconfig.json** - TypeScript compiler configuration
- **.babelrc** - Babel transpiler configuration
- **.eslintrc.json** - ESLint linting rules
- **.prettierrc** - Prettier code formatting rules
- **.gitignore** - Git ignore patterns
- **README.md** - Project documentation

## Directory Structure

```
/home/user/codetest/
├── src/
│   ├── main/
│   │   └── main.js              # Electron main process
│   ├── preload/
│   │   └── preload.js           # Secure IPC bridge
│   └── renderer/
│       ├── components/          # React components
│       ├── contexts/           # React contexts
│       ├── hooks/              # Custom React hooks
│       ├── styles/             # SCSS/CSS styles
│       ├── types/              # TypeScript type definitions
│       ├── utils/              # Utility functions
│       ├── App.tsx             # Main React component
│       ├── index.html          # HTML template
│       └── index.js            # React entry point
├── public/                      # Build output (generated)
└── build/                       # Electron builder resources

## Key Features Configured

### Electron Main Process (src/main/main.js)
- Transparent, frameless window
- Always-on-top behavior
- Click-through mode toggle
- Global keyboard shortcuts
- Multi-monitor support
- Secure IPC handlers
- Logging with electron-log

### Global Hotkeys
- Ctrl+Shift+O - Toggle overlay visibility
- Ctrl+Shift+C - Toggle click-through mode
- Ctrl+Shift+R - Reload overlay
- Ctrl+Shift+Q - Quit application

### Security Features
- Context isolation enabled
- Node integration disabled in renderer
- Sandbox mode enabled
- Content Security Policy in HTML
- Limited API exposure via contextBridge

### Development Tools
- Webpack dev server with hot reload
- Source maps for debugging
- ESLint for code linting
- Prettier for code formatting
- TypeScript support

## NPM Scripts

- `npm start` - Start development server and Electron app
- `npm run build` - Build production bundle
- `npm run build:win` - Package for Windows
- `npm run build:linux` - Package for Linux
- `npm run build:mac` - Package for macOS
- `npm run lint` - Run ESLint
- `npm run format` - Format code with Prettier

## Next Steps for Other Agents

The base infrastructure is complete. Other agents can now implement:
1. POE-specific features (trade search, price checking, etc.)
2. UI components for overlay panels
3. Item parsing and data fetching
4. Build guide integration
5. Settings and configuration UI
