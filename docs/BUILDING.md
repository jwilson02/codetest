# Building SCUMM Style ARPG

This document provides detailed instructions for building the SCUMM Style ARPG game engine on all supported platforms.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Platform-Specific Instructions](#platform-specific-instructions)
  - [Linux](#linux)
  - [macOS](#macos)
  - [Windows](#windows)
- [CMake Options](#cmake-options)
- [Build Types](#build-types)
- [Troubleshooting](#troubleshooting)
- [Advanced Topics](#advanced-topics)

## Prerequisites

### All Platforms

- **CMake 3.15 or later**
- **C++17 compatible compiler**:
  - GCC 7+ (Linux)
  - Clang 5+ (macOS/Linux)
  - MSVC 2019+ (Windows)

### Required Libraries

- **SDL2** (2.0.10+) - Window management and input
- **SDL2_mixer** (2.0.4+) - Audio playback
- **OpenGL** (3.3+) - Graphics rendering
- **GLEW** (2.1.0+) - OpenGL extension loading
- **GLM** (0.9.9+) - Mathematics library

### Optional Libraries

- **SDL2_image** - Image loading (PNG, JPG)
- **SDL2_ttf** - TrueType font rendering

## Quick Start

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo ./scripts/install_deps.sh

# Build the project
./scripts/build_linux.sh

# Run the game
./build-linux/bin/SCUMMStyleARPG
```

### macOS

```bash
# Install dependencies (requires Homebrew)
./scripts/install_deps.sh

# Build the project
./scripts/build_macos.sh

# Run the game
./build-macos/bin/SCUMMStyleARPG
```

### Windows

```bat
REM See Windows section below for dependency installation

REM Build the project
scripts\build_windows.bat

REM Run the game
build-windows\bin\Release\SCUMMStyleARPG.exe
```

## Platform-Specific Instructions

### Linux

#### Ubuntu/Debian

1. **Install Dependencies**

```bash
# Update package list
sudo apt-get update

# Install build tools
sudo apt-get install -y build-essential cmake git pkg-config

# Install SDL2 libraries
sudo apt-get install -y \
    libsdl2-dev \
    libsdl2-mixer-dev \
    libsdl2-image-dev \
    libsdl2-ttf-dev

# Install OpenGL and GLEW
sudo apt-get install -y \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libglew-dev

# Install GLM
sudo apt-get install -y libglm-dev
```

2. **Build the Project**

```bash
# Using the build script (recommended)
./scripts/build_linux.sh

# Or manually
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel $(nproc)
```

3. **Run the Game**

```bash
./build-linux/bin/SCUMMStyleARPG
```

#### Fedora/RHEL

```bash
# Install dependencies
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git pkg-config
sudo dnf install SDL2-devel SDL2_mixer-devel SDL2_image-devel SDL2_ttf-devel
sudo dnf install mesa-libGL-devel mesa-libGLU-devel glew-devel glm-devel

# Build
./scripts/build_linux.sh
```

#### Arch Linux

```bash
# Install dependencies
sudo pacman -S base-devel cmake git pkg-config
sudo pacman -S sdl2 sdl2_mixer sdl2_image sdl2_ttf
sudo pacman -S mesa glu glew glm

# Build
./scripts/build_linux.sh
```

### macOS

#### Prerequisites

1. **Install Xcode Command Line Tools**

```bash
xcode-select --install
```

2. **Install Homebrew** (if not already installed)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### Install Dependencies

```bash
# Install build tools
brew install cmake git pkg-config

# Install SDL2 libraries
brew install sdl2 sdl2_mixer sdl2_image sdl2_ttf

# Install GLEW and GLM
brew install glew glm
```

#### Build the Project

```bash
# Using the build script (recommended)
./scripts/build_macos.sh

# Or manually
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel $(sysctl -n hw.ncpu)
```

#### Run the Game

```bash
./build-macos/bin/SCUMMStyleARPG
```

### Windows

Windows has two main approaches: vcpkg (recommended) or manual installation.

#### Option 1: Using vcpkg (Recommended)

1. **Install Visual Studio 2019 or later**
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Install "Desktop development with C++"

2. **Install CMake**
   - Download from: https://cmake.org/download/
   - Add CMake to system PATH

3. **Install vcpkg**

```bat
REM Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg

REM Bootstrap vcpkg
bootstrap-vcpkg.bat

REM Integrate with Visual Studio
vcpkg integrate install
```

4. **Install Dependencies**

```bat
REM Install all required libraries
vcpkg install sdl2:x64-windows
vcpkg install sdl2-mixer:x64-windows
vcpkg install sdl2-image:x64-windows
vcpkg install sdl2-ttf:x64-windows
vcpkg install glew:x64-windows
vcpkg install glm:x64-windows
```

5. **Set Environment Variable**

```bat
REM Set CMAKE_TOOLCHAIN_FILE environment variable
setx CMAKE_TOOLCHAIN_FILE "C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

6. **Build the Project**

```bat
REM Using the build script
scripts\build_windows.bat

REM Or manually
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

#### Option 2: Manual Installation

1. **Download and Install Visual Studio** (as above)

2. **Download Libraries Manually**

   - **SDL2**: https://www.libsdl.org/download-2.0.php
     - Download "Development Libraries" (Visual C++)
     - Extract to `C:\SDL2`

   - **SDL2_mixer**: https://www.libsdl.org/projects/SDL_mixer/
     - Extract to `C:\SDL2_mixer`

   - **SDL2_image**: https://www.libsdl.org/projects/SDL_image/
     - Extract to `C:\SDL2_image`

   - **SDL2_ttf**: https://www.libsdl.org/projects/SDL_ttf/
     - Extract to `C:\SDL2_ttf`

   - **GLEW**: http://glew.sourceforge.net/
     - Extract to `C:\GLEW`

   - **GLM**: https://github.com/g-truc/glm/releases
     - Extract to `C:\GLM`

3. **Build with CMake**

```bat
mkdir build
cd build
cmake .. ^
    -DSDL2_DIR=C:\SDL2\cmake ^
    -DGLEW_DIR=C:\GLEW\lib\cmake\glew ^
    -DCMAKE_PREFIX_PATH="C:\SDL2_mixer;C:\SDL2_image;C:\SDL2_ttf;C:\GLM"
cmake --build . --config Release
```

#### Run the Game

```bat
build\bin\Release\SCUMMStyleARPG.exe
```

## CMake Options

### Build Type

```bash
# Debug build (with debug symbols)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release build (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Release with debug info
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Minimum size release
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel
```

### Optional Features

```bash
# Build examples
cmake .. -DBUILD_EXAMPLES=ON

# Build development tools
cmake .. -DBUILD_TOOLS=ON

# Build unit tests
cmake .. -DBUILD_TESTS=ON

# Combine multiple options
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON
```

### Custom Installation Path

```bash
# Install to custom directory
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/scumm-arpg

# Build and install
cmake --build . --config Release
cmake --install .
```

## Build Types

### Debug Build

```bash
# Build with debug symbols and no optimization
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Run with debugger
gdb ./build/bin/SCUMMStyleARPG
```

**Debug build features:**
- Full debug symbols
- No optimization (-O0)
- Assertions enabled
- Verbose logging
- Slower performance

### Release Build

```bash
# Build with full optimization
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**Release build features:**
- Full optimization (-O3)
- No debug symbols
- Assertions disabled
- Fast performance

### Profile Build

```bash
# Build with profiling support
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build .
```

**Profile build features:**
- Optimization enabled
- Debug symbols included
- Use with profiling tools (gprof, valgrind, perf)

## Troubleshooting

### CMake Configuration Issues

**Problem**: `CMake Error: Could not find SDL2`

**Solution**:
```bash
# Linux: Install SDL2 development package
sudo apt-get install libsdl2-dev

# macOS: Install via Homebrew
brew install sdl2

# Windows: Use vcpkg or set SDL2_DIR
set SDL2_DIR=C:\SDL2\cmake
```

**Problem**: `CMake Error: Could not find GLEW`

**Solution**:
```bash
# Linux
sudo apt-get install libglew-dev

# macOS
brew install glew

# Windows (vcpkg)
vcpkg install glew:x64-windows
```

### Build Errors

**Problem**: `error: 'std::filesystem' has not been declared`

**Solution**: Ensure you're using a C++17 compatible compiler:
- GCC 8+ or Clang 7+
- Update compiler: `sudo apt-get install g++-8`

**Problem**: `undefined reference to SDL_Init`

**Solution**: Linking issue - make sure SDL2 libraries are properly linked:
```bash
# Check CMake output for SDL2 library paths
cmake .. --trace-expand | grep SDL2
```

### Runtime Errors

**Problem**: `error while loading shared libraries: libSDL2-2.0.so.0`

**Solution**:
```bash
# Linux: Update library cache
sudo ldconfig

# Or add library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

**Problem**: Game window doesn't open

**Solution**: Check OpenGL support:
```bash
# Linux: Check OpenGL version
glxinfo | grep "OpenGL version"

# Update graphics drivers if version < 3.3
```

**Problem**: Missing assets error

**Solution**: Assets should be automatically copied. If not:
```bash
# Manually copy assets
cp -r assets build/bin/
cp -r data build/bin/
```

### Platform-Specific Issues

#### Linux

**Problem**: Black screen on Intel integrated graphics

**Solution**: Update mesa drivers:
```bash
sudo apt-get update
sudo apt-get install mesa-utils
```

#### macOS

**Problem**: "App is damaged and can't be opened"

**Solution**: Remove quarantine attribute:
```bash
xattr -cr /path/to/SCUMMStyleARPG
```

#### Windows

**Problem**: MSVCR140.dll missing

**Solution**: Install Visual C++ Redistributable:
- Download from Microsoft website
- Or install via vcpkg: `vcpkg install vcredist`

**Problem**: OpenGL initialization fails

**Solution**: Update graphics drivers from manufacturer website
- NVIDIA: https://www.nvidia.com/drivers
- AMD: https://www.amd.com/drivers
- Intel: https://downloadcenter.intel.com

## Advanced Topics

### Cross-Compilation

#### Linux to Windows (MinGW)

```bash
# Install MinGW cross-compiler
sudo apt-get install mingw-w64

# Configure for Windows
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .
```

### Static Linking

```bash
# Build with static libraries (Linux/macOS)
cmake .. \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_EXE_LINKER_FLAGS="-static"
```

### Custom Compiler

```bash
# Use Clang instead of GCC
cmake .. -DCMAKE_CXX_COMPILER=clang++

# Use specific GCC version
cmake .. -DCMAKE_CXX_COMPILER=g++-10
```

### Parallel Builds

```bash
# Linux/macOS - use all CPU cores
cmake --build . --parallel $(nproc)

# Windows
cmake --build . --parallel

# Or use Ninja for faster builds
cmake .. -GNinja
ninja
```

### IDE Integration

#### Visual Studio Code

1. Install CMake Tools extension
2. Open project folder
3. Select kit (compiler)
4. Press F7 to build

#### CLion

1. Open CMakeLists.txt as project
2. CMake will configure automatically
3. Build from menu or Ctrl+F9

#### Visual Studio

```bash
# Generate Visual Studio solution
cmake .. -G "Visual Studio 16 2019"

# Open generated .sln file
start SCUMMStyleARPG.sln
```

### Clean Build

```bash
# Remove build directory
rm -rf build

# Or clean build artifacts
cd build
cmake --build . --target clean
```

### Verbose Build Output

```bash
# See full compiler commands
cmake --build . --verbose

# Or with make
make VERBOSE=1
```

## Performance Optimization

### Compiler Optimizations

```bash
# Maximum optimization (may increase compile time)
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"

# Link-time optimization
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

### Profile-Guided Optimization (PGO)

1. Build with profiling:
```bash
cmake .. -DCMAKE_CXX_FLAGS="-fprofile-generate"
cmake --build .
```

2. Run game to generate profile data

3. Rebuild with profile data:
```bash
cmake .. -DCMAKE_CXX_FLAGS="-fprofile-use"
cmake --build .
```

## Getting Help

If you encounter issues not covered here:

1. Check the [main README](../README.md)
2. Search [GitHub Issues](https://github.com/yourusername/scumm-arpg/issues)
3. Ask in [Discussions](https://github.com/yourusername/scumm-arpg/discussions)
4. Create a new issue with:
   - Your platform and compiler version
   - Full error messages
   - Steps to reproduce

## Next Steps

After successfully building:

- Read [DEVELOPER.md](DEVELOPER.md) for development guidelines
- Check [ARCHITECTURE.md](ARCHITECTURE.md) for system overview
- See [CONTRIBUTING.md](CONTRIBUTING.md) to contribute

---

**Happy Building!**
