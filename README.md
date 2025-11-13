# SCUMM VM-Style ARPG Game Engine

A comprehensive, production-quality C++17 game engine for creating SCUMM VM-style action RPGs. Built with modern software engineering practices, extensive features, and a modular architecture that scales from indie projects to commercial releases.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)
![Build](https://img.shields.io/badge/build-CMake-green.svg)

## Features

### Core Engine Systems
- **Modern ECS Architecture** - Entity-Component-System for flexible game object composition
- **Scene Management** - Hierarchical scene system with lifecycle management
- **Event System** - Type-safe publish-subscribe event handling
- **Fixed Timestep Loop** - Deterministic physics with variable frame rate rendering
- **Resource Management** - Efficient asset loading and caching

### Rendering System
- **OpenGL 3.3+ Renderer** - Hardware-accelerated 2D/3D graphics
- **Sprite Batching** - Optimized rendering with automatic batching
- **Animation System** - Sprite sheets, skeletal animation, and blending
- **Particle Effects** - GPU-accelerated particle systems
- **Camera System** - Multiple camera modes (follow, fixed, pan, zoom)
- **Post-Processing** - Bloom, color grading, and custom shader effects
- **Lighting** - 2D dynamic lighting with shadows
- **Render Layers** - Z-ordering and layer management

### Audio System
- **SDL2_mixer Integration** - Professional audio playback
- **Music Player** - Crossfading, playlists, and adaptive music
- **3D Positional Audio** - Spatial sound with distance attenuation
- **Sound Effects** - Multiple channels with priority system
- **Audio Categories** - Separate volume controls (music, SFX, voice, ambient)
- **Audio Ducking** - Automatic volume reduction during dialogue

### RPG Systems
- **Character System** - Stats, attributes, and progression
- **Class System** - Multiple character classes with unique abilities
- **Leveling & Experience** - XP curves and level-based progression
- **Skill Trees** - Branching talent systems
- **Achievement System** - Unlockables and rewards
- **Prestige System** - New Game+ mechanics

### Combat System
- **Real-time Combat** - Action-based fighting mechanics
- **Weapon System** - Multiple weapon types with unique properties
- **Skill System** - Active and passive abilities
- **Combo System** - Chain attacks for bonus damage
- **Damage Calculator** - Complex damage formulas with resistances
- **Status Effects** - Buffs, debuffs, and damage over time

### Inventory & Items
- **Inventory Management** - Grid-based or list-based inventory
- **Item Database** - Flexible item type system
- **Equipment System** - Equippable items with stats
- **Item Generator** - Procedural loot generation
- **Crafting System** - Recipe-based item creation
- **Gathering System** - Resource harvesting
- **Enchanting** - Item enhancement and modification

### Quest System
- **Quest Manager** - Multi-step quest tracking
- **Quest Generator** - Procedural quest creation
- **Objective Tracker** - Real-time objective updates
- **World Events** - Dynamic events that affect the world
- **Quest Rewards** - Items, XP, and reputation rewards

### Story & Narrative
- **Dialogue System** - Branching conversations with choices
- **Dialogue Trees** - Visual dialogue editor support
- **Narrative Engine** - Story progression and branching narratives
- **Relationship System** - NPC relationships and reputation
- **Journal System** - Quest log and lore entries
- **Cutscene System** - Scripted cinematic sequences

### AI System
- **Behavior Trees** - Flexible AI decision making
- **State Machines** - Character state management
- **Pathfinding** - A* pathfinding with obstacle avoidance
- **Enemy AI** - Combat behaviors and tactics
- **NPC AI** - Schedules, routines, and interactions
- **Boss AI** - Complex multi-phase boss fights

### World & Environment
- **Tile Map System** - 2D tile-based worlds
- **Map System** - World map and mini-map
- **Procedural Generation** - Random dungeon/level generation
- **Collision Detection** - Spatial partitioning and collision resolution
- **Trigger System** - Zone-based event triggers
- **World Regions** - Area transitions and streaming
- **Day/Night Cycle** - Dynamic time of day
- **Weather System** - Rain, snow, fog effects
- **Season System** - Seasonal changes

### UI System
- **Main Menu** - Title screen and game options
- **HUD** - Health bars, mini-map, quick slots
- **Inventory UI** - Drag-and-drop inventory management
- **Character Sheet** - Stats and equipment display
- **Dialogue UI** - Conversation interface
- **Quest Tracker** - Active quest display
- **Tooltips** - Context-sensitive information
- **Customizable UI** - Skinnable and moddable interface

### Save System
- **Save/Load** - Complete game state persistence
- **Auto-Save** - Automatic checkpoint saves
- **Cloud Sync** - Optional cloud save support
- **Serialization** - JSON-based save format
- **Multiple Save Slots** - User-managed save files

### Network (Multiplayer)
- **Network Manager** - Client-server architecture
- **Server Connection** - Robust connection handling
- **Player Sync** - State synchronization
- **Network Ready** - Foundation for multiplayer features

### Development Tools
- **Asset Manager** - Resource loading and hot-reloading
- **Profiler** - Performance monitoring and optimization
- **Memory Tracker** - Memory leak detection and analysis
- **Game Launcher** - Configuration and launcher interface
- **Debug UI** - In-game debugging tools

## Screenshots

```
[Game Screenshot Placeholder - Coming Soon]
- Main game view showing character, environment, and UI
- Combat system in action
- Inventory and character screen
- Dialogue system example
```

## Quick Start

### Prerequisites

- **C++17 Compiler** - GCC 7+, Clang 5+, MSVC 2019+
- **CMake 3.15+**
- **SDL2** - Window and input management
- **SDL2_mixer** - Audio playback
- **OpenGL 3.3+** - Graphics rendering
- **GLEW** - OpenGL extension loading
- **GLM** - Mathematics library

### Installation

#### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo ./scripts/install_deps.sh

# Build the project
./scripts/build_linux.sh

# Run the game
./build-linux/bin/SCUMMStyleARPG
```

#### macOS
```bash
# Install dependencies (requires Homebrew)
./scripts/install_deps.sh

# Build the project
./scripts/build_macos.sh

# Run the game
./build-macos/bin/SCUMMStyleARPG
```

#### Windows
```bat
REM Install dependencies - See scripts/install_deps_windows.bat for instructions
REM Recommended: Use vcpkg

REM Build the project
scripts\build_windows.bat

REM Run the game
build-windows\bin\Release\SCUMMStyleARPG.exe
```

### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Run
./bin/SCUMMStyleARPG
```

## Project Structure

```
SCUMM-Style-ARPG/
├── CMakeLists.txt              # Main build configuration
├── LICENSE                     # MIT License
├── README.md                   # This file
│
├── src/                        # Source code
│   ├── main.cpp               # Entry point
│   ├── Engine/                # Core engine systems
│   │   ├── GameEngine.h/cpp  # Main engine class
│   │   ├── Time.h/cpp        # Time management
│   │   ├── EventSystem.h/cpp # Event handling
│   │   ├── SceneManager.h/cpp # Scene management
│   │   └── ECS/              # Entity-Component-System
│   │
│   ├── Rendering/            # Graphics and rendering
│   ├── Audio/                # Sound and music
│   ├── UI/                   # User interface
│   ├── AI/                   # Artificial intelligence
│   ├── Combat/               # Combat mechanics
│   ├── RPG/                  # RPG systems
│   ├── Quest/                # Quest system
│   ├── Inventory/            # Items and inventory
│   ├── Story/                # Narrative systems
│   ├── World/                # World and environment
│   ├── Crafting/             # Crafting system
│   ├── Save/                 # Save/load system
│   ├── Environment/          # Weather, day/night
│   ├── Network/              # Multiplayer (optional)
│   ├── Assets/               # Asset management
│   ├── Performance/          # Profiling tools
│   └── Launcher/             # Game launcher
│
├── assets/                    # Game assets
│   ├── textures/             # Images and sprites
│   ├── sounds/               # Audio files
│   ├── music/                # Background music
│   ├── fonts/                # Font files
│   └── shaders/              # GLSL shaders
│
├── data/                      # Game data files
│   ├── items/                # Item definitions
│   ├── quests/               # Quest data
│   ├── dialogues/            # Dialogue trees
│   ├── maps/                 # Level maps
│   └── config/               # Configuration files
│
├── scripts/                   # Build and utility scripts
│   ├── build_linux.sh        # Linux build script
│   ├── build_macos.sh        # macOS build script
│   ├── build_windows.bat     # Windows build script
│   ├── install_deps.sh       # Dependency installer
│   └── install_deps_windows.bat
│
├── docs/                      # Documentation
│   ├── BUILDING.md           # Build instructions
│   ├── DEVELOPER.md          # Developer guide
│   ├── ARCHITECTURE.md       # Architecture overview
│   └── CONTRIBUTING.md       # Contribution guidelines
│
├── examples/                  # Example code
│   └── demo.cpp              # Demo scenes and examples
│
└── tools/                     # Development tools
    └── [Various utility tools]
```

## Controls (Demo)

- **WASD / Arrow Keys** - Move character
- **Space** - Action/Interact
- **E** - Open inventory
- **Tab** - Character sheet
- **M** - Map
- **J** - Journal/Quests
- **Escape** - Menu/Quit

## Documentation

- **[Building Instructions](docs/BUILDING.md)** - Detailed build guide for all platforms
- **[Developer Guide](docs/DEVELOPER.md)** - Getting started with development
- **[Architecture Overview](docs/ARCHITECTURE.md)** - Engine design and systems
- **[Contributing](docs/CONTRIBUTING.md)** - How to contribute to the project

## Performance

- **Target Frame Rate**: 60 FPS
- **Minimum Requirements**:
  - CPU: Dual-core 2.0 GHz
  - RAM: 2 GB
  - GPU: OpenGL 3.3 compatible
  - Storage: 500 MB
- **Recommended**:
  - CPU: Quad-core 3.0 GHz
  - RAM: 4 GB
  - GPU: Dedicated GPU with 1 GB VRAM
  - Storage: 1 GB

## Roadmap

### Current Version: 1.0.0

### Planned Features
- [ ] Level editor and tooling
- [ ] Scripting support (Lua/Python)
- [ ] Advanced particle systems
- [ ] Multiplayer support
- [ ] Steam integration
- [ ] Controller support
- [ ] Localization system
- [ ] Asset streaming
- [ ] Advanced AI behaviors
- [ ] Dynamic difficulty scaling

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](docs/CONTRIBUTING.md) for guidelines.

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests and ensure code quality
5. Submit a pull request

## Testing

```bash
# Build with tests enabled
cmake .. -DBUILD_TESTS=ON
cmake --build .

# Run tests
ctest --output-on-failure
```

## Troubleshooting

### Build Issues

**CMake cannot find SDL2**
```bash
# Linux: Make sure SDL2-dev is installed
sudo apt-get install libsdl2-dev

# macOS: Install via Homebrew
brew install sdl2

# Windows: Use vcpkg or set SDL2_DIR
```

**Linking errors**
- Ensure all dependencies are installed
- Check that library versions are compatible
- See docs/BUILDING.md for platform-specific solutions

**Runtime crashes**
- Verify assets directory is copied to build folder
- Check OpenGL driver is up to date
- Enable debug mode for detailed error messages

For more troubleshooting, see [docs/BUILDING.md](docs/BUILDING.md)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

### Development Team
- Engine Architecture & Core Systems
- Rendering & Graphics
- Audio & Sound Design
- Game Systems & Mechanics

### Third-Party Libraries
- **SDL2** - Simple DirectMedia Layer
- **SDL2_mixer** - Audio mixing library
- **OpenGL** - Graphics API
- **GLEW** - OpenGL Extension Wrangler
- **GLM** - OpenGL Mathematics

### Inspiration
- SCUMM VM - Classic adventure game engine
- Unity Engine - Component-based architecture
- Unreal Engine - AAA game engine patterns
- Divinity: Original Sin - RPG mechanics
- The Legend of Zelda - Action RPG gameplay

## Community

- **Issues**: [GitHub Issues](https://github.com/yourusername/scumm-arpg/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/scumm-arpg/discussions)
- **Wiki**: [Project Wiki](https://github.com/yourusername/scumm-arpg/wiki)

## Support

If you encounter any issues or have questions:
1. Check the [documentation](docs/)
2. Search [existing issues](https://github.com/yourusername/scumm-arpg/issues)
3. Create a new issue with detailed information

## Acknowledgments

Thanks to all contributors who have helped make this project possible!

---

**Built with love by the SCUMM ARPG Team**

*Creating memorable gaming experiences, one pixel at a time.*
