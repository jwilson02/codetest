@echo off
REM Dependency installation guide for Windows
REM This script provides instructions for installing dependencies on Windows

echo ===============================================
echo SCUMM Style ARPG - Windows Dependencies
echo ===============================================
echo.
echo This script will guide you through installing dependencies on Windows.
echo.
echo ===============================================
echo Required Software
echo ===============================================
echo.
echo 1. Visual Studio 2019 or later (Community Edition is free)
echo    Download: https://visualstudio.microsoft.com/downloads/
echo    Make sure to install "Desktop development with C++"
echo.
echo 2. CMake (version 3.15 or later)
echo    Download: https://cmake.org/download/
echo    Make sure to add CMake to system PATH during installation
echo.
echo 3. Git (for cloning repositories)
echo    Download: https://git-scm.com/download/win
echo.
echo ===============================================
echo Option 1: Using vcpkg (Recommended)
echo ===============================================
echo.
echo vcpkg is a C++ package manager that makes installing dependencies easy.
echo.
echo Step 1: Install vcpkg
echo   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
echo   cd C:\vcpkg
echo   bootstrap-vcpkg.bat
echo   vcpkg integrate install
echo.
echo Step 2: Install dependencies
echo   vcpkg install sdl2:x64-windows
echo   vcpkg install sdl2-mixer:x64-windows
echo   vcpkg install sdl2-image:x64-windows
echo   vcpkg install sdl2-ttf:x64-windows
echo   vcpkg install glew:x64-windows
echo   vcpkg install glm:x64-windows
echo.
echo Step 3: Configure CMake to use vcpkg
echo   When running CMake, add this option:
echo   -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
echo.
echo   Or set it as an environment variable:
echo   set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
echo.
echo ===============================================
echo Option 2: Manual Installation
echo ===============================================
echo.
echo If you prefer to install libraries manually:
echo.
echo 1. SDL2
echo    Download: https://www.libsdl.org/download-2.0.php
echo    Get the "Development Libraries" (Visual C++)
echo    Extract to: C:\SDL2
echo.
echo 2. SDL2_mixer
echo    Download: https://www.libsdl.org/projects/SDL_mixer/
echo    Extract to: C:\SDL2_mixer
echo.
echo 3. SDL2_image
echo    Download: https://www.libsdl.org/projects/SDL_image/
echo    Extract to: C:\SDL2_image
echo.
echo 4. SDL2_ttf
echo    Download: https://www.libsdl.org/projects/SDL_ttf/
echo    Extract to: C:\SDL2_ttf
echo.
echo 5. GLEW
echo    Download: http://glew.sourceforge.net/
echo    Extract to: C:\GLEW
echo.
echo 6. GLM
echo    Download: https://github.com/g-truc/glm/releases
echo    Extract to: C:\GLM
echo.
echo 7. Configure CMake paths
echo    When running CMake, you'll need to specify library paths:
echo    -DSDL2_DIR=C:\SDL2
echo    -DGLEW_DIR=C:\GLEW
echo    etc.
echo.
echo ===============================================
echo After Installing Dependencies
echo ===============================================
echo.
echo To build the project:
echo   1. Open "x64 Native Tools Command Prompt for VS 2019"
echo   2. Navigate to the project directory
echo   3. Run: scripts\build_windows.bat
echo.
echo Or using CMake GUI:
echo   1. Open CMake GUI
echo   2. Set source directory to the project root
echo   3. Set build directory to: project_root\build
echo   4. Click "Configure" and select your Visual Studio version
echo   5. If using vcpkg, set CMAKE_TOOLCHAIN_FILE
echo   6. Click "Generate"
echo   7. Click "Open Project" to open in Visual Studio
echo   8. Build from Visual Studio (Ctrl+Shift+B)
echo.
echo ===============================================
echo Troubleshooting
echo ===============================================
echo.
echo If CMake cannot find libraries:
echo   1. Make sure all dependencies are installed
echo   2. Check that CMake can find vcpkg toolchain file
echo   3. Or manually specify library paths with -D flags
echo.
echo If build fails with "SDL.h not found":
echo   1. Verify SDL2 is installed correctly
echo   2. Check include paths in CMake configuration
echo.
echo For more help, see: docs\BUILDING.md
echo.
echo ===============================================

pause
