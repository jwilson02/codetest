# Quick Build Guide - SCUMM Style ARPG Engine

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install libsdl2-dev libgl1-mesa-dev libglm-dev
```

### macOS
```bash
brew install cmake sdl2 glm
```

### Windows
1. Install Visual Studio 2019+ with C++ development tools
2. Install CMake (https://cmake.org/download/)
3. Install vcpkg for dependencies:
```powershell
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install sdl2 glm
```

## Building

### Linux/macOS

```bash
# Navigate to project directory
cd /home/user/codetest

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build (use -j for parallel compilation)
cmake --build . -j$(nproc)

# Run the engine
./bin/SCUMMStyleARPG
```

### Windows (Visual Studio)

```powershell
# Navigate to project directory
cd C:\path\to\codetest

# Create build directory
mkdir build
cd build

# Configure with CMake (vcpkg integration)
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake

# Build
cmake --build . --config Release

# Run
.\bin\Release\SCUMMStyleARPG.exe
```

## Project Structure

```
codetest/
├── CMakeLists.txt                 # Build configuration
├── BUILD_GUIDE.md                 # This file
├── ENGINE_README.md               # Detailed architecture documentation
└── src/
    ├── main.cpp                   # Entry point with demo
    └── Engine/
        ├── GameEngine.h           # Main engine (196 lines)
        ├── GameEngine.cpp         # Engine implementation (338 lines)
        ├── Time.h                 # Time management (97 lines)
        ├── Time.cpp               # Time implementation (90 lines)
        ├── EventSystem.h          # Event system (274 lines)
        ├── EventSystem.cpp        # Event implementation (12 lines)
        ├── SceneManager.h         # Scene management (227 lines)
        ├── SceneManager.cpp       # Scene implementation (185 lines)
        └── ECS/
            ├── Component.h        # Component base (95 lines)
            ├── Entity.h           # Entity system (241 lines)
            └── Entity.cpp         # Entity implementation (42 lines)

Total: ~2,080 lines of production-quality C++17 code
```

## Running the Demo

The included demo showcases all engine features:

### What to Expect
1. Engine initialization with SDL2 and OpenGL
2. Window creation (1280x720)
3. Game scene with multiple entities
4. Player entity with movement controls
5. FPS counter (displayed every second in console)
6. Event system demonstration

### Controls
- **W/A/S/D or Arrow Keys** - Move player entity
- **SPACE** - Trigger action
- **ESC** - Exit application

### Console Output
You'll see detailed logging of:
- Engine initialization
- OpenGL information
- Scene creation and switching
- Entity creation
- FPS statistics
- Input events

Example output:
```
===========================================
  SCUMM VM-Style ARPG Game Engine
  Modern C++17 Architecture Demo
===========================================

[Engine] Initializing game engine...
[Engine] Initialization complete

========================================
  SCUMM Style ARPG Engine
========================================
OpenGL Vendor:   [Your GPU Vendor]
OpenGL Renderer: [Your GPU Model]
OpenGL Version:  [OpenGL Version]
Window Size:     1280x720
VSync:           ON
Fixed Timestep:  0.0166667s
========================================

[SceneManager] Added scene: GameScene
[SceneManager] Entering scene: GameScene
[GameScene] Creating game world...
[GameScene] Player entity created with ID: 1
[GameScene] Created 4 entities

[Engine] Starting game loop...
[GameScene] FPS: 60.0 | Frame: 60 | Time: 1.0s
[Player] Moving right
[GameScene] FPS: 60.0 | Frame: 120 | Time: 2.0s
```

## Troubleshooting

### CMake can't find SDL2
```bash
# Linux: Ensure pkg-config is installed
sudo apt-get install pkg-config

# Set SDL2_DIR manually
cmake .. -DSDL2_DIR=/path/to/sdl2
```

### OpenGL errors
```bash
# Linux: Install Mesa drivers
sudo apt-get install mesa-utils

# Check OpenGL version
glxinfo | grep "OpenGL version"
```

### Linker errors
```bash
# Clean build directory
rm -rf build/*
cd build && cmake .. && cmake --build .
```

### Window doesn't appear
- Check if display server is running (Linux)
- Verify GPU drivers are installed
- Try disabling VSync in EngineConfig

## Customizing the Engine

### Change Window Settings
Edit `main.cpp`:
```cpp
EngineConfig config;
config.windowTitle = "Your Game Title";
config.windowWidth = 1920;
config.windowHeight = 1080;
config.fullscreen = true;  // Start in fullscreen
config.vsync = false;      // Disable VSync
config.targetFPS = 144;    // Target 144 FPS
```

### Add More Components
Create new component classes:
```cpp
class YourComponent : public ComponentBase<YourComponent> {
public:
    void Update(double deltaTime) override {
        // Your logic here
    }
};
```

### Create New Scenes
```cpp
class YourScene : public Scene {
public:
    YourScene() : Scene("YourScene") {}

    void OnEnter() override {
        // Setup scene
    }
};

// In main():
sceneManager.CreateScene<YourScene>("YourScene");
sceneManager.SwitchScene("YourScene");
```

## Performance Tips

### Build for Release
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```
This enables optimizations (10-100x faster than Debug builds)

### Profile Your Game
The engine includes FPS tracking. Monitor with:
```cpp
std::cout << "FPS: " << Time::GetFPS() << std::endl;
```

### Memory Profiling
Use Valgrind (Linux):
```bash
valgrind --leak-check=full ./bin/SCUMMStyleARPG
```

## Next Steps

1. Read `ENGINE_README.md` for detailed architecture documentation
2. Explore the example code in `main.cpp`
3. Create your own components and scenes
4. Add rendering systems (sprites, animations)
5. Implement game logic (combat, inventory, quests)

## Support

For issues or questions:
1. Check the detailed documentation in `ENGINE_README.md`
2. Review the example code in `main.cpp`
3. Ensure all dependencies are properly installed
4. Verify CMake version is 3.15 or higher

## Architecture Highlights

✅ **Fixed Timestep Loop** - Deterministic physics simulation
✅ **ECS Architecture** - Flexible component composition
✅ **Event System** - Decoupled communication
✅ **Scene Management** - Easy level organization
✅ **Smart Pointers** - Automatic memory management
✅ **Modern C++17** - Template metaprogramming, lambdas
✅ **Cross-Platform** - Windows, Linux, macOS support
✅ **Production Quality** - Error handling, logging, documentation

Happy game development! 🎮
