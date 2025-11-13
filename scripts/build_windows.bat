@echo off
REM Build script for Windows
REM Requires: CMake, Visual Studio (or MinGW), and all dependencies installed

echo ===============================================
echo SCUMM Style ARPG - Windows Build Script
echo ===============================================
echo.

REM Check if CMake is available
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found! Please install CMake and add it to PATH.
    echo Download from: https://cmake.org/download/
    exit /b 1
)

REM Determine build directory
set BUILD_DIR=build-windows
set BUILD_TYPE=Release

REM Parse command-line arguments
:parse_args
if "%~1"=="" goto end_parse_args
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if /i "%~1"=="--clean" (
    echo Cleaning build directory...
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
    shift
    goto parse_args
)
if /i "%~1"=="--help" (
    echo Usage: build_windows.bat [options]
    echo.
    echo Options:
    echo   --debug        Build in Debug mode (default: Release)
    echo   --clean        Clean build directory before building
    echo   --generator G  Specify CMake generator (default: auto-detect)
    echo   --help         Show this help message
    echo.
    exit /b 0
)
if /i "%~1"=="--generator" (
    set CMAKE_GENERATOR=%~2
    shift
    shift
    goto parse_args
)
shift
goto parse_args
:end_parse_args

echo Build Type: %BUILD_TYPE%
echo Build Directory: %BUILD_DIR%
echo.

REM Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

REM Change to build directory
cd %BUILD_DIR%

REM Configure with CMake
echo ===============================================
echo Configuring project with CMake...
echo ===============================================
echo.

if defined CMAKE_GENERATOR (
    cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G "%CMAKE_GENERATOR%"
) else (
    REM Auto-detect Visual Studio version
    cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Common issues:
    echo   1. Missing dependencies - Run: scripts\install_deps_windows.bat
    echo   2. Wrong CMake generator - Try: --generator "Visual Studio 16 2019"
    echo   3. See docs\BUILDING.md for detailed instructions
    echo.
    cd ..
    exit /b 1
)

REM Build the project
echo.
echo ===============================================
echo Building project...
echo ===============================================
echo.

cmake --build . --config %BUILD_TYPE% --parallel

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    echo Check the error messages above for details.
    echo.
    cd ..
    exit /b 1
)

REM Return to root directory
cd ..

echo.
echo ===============================================
echo Build completed successfully!
echo ===============================================
echo.
echo Executable location: %BUILD_DIR%\bin\%BUILD_TYPE%\SCUMMStyleARPG.exe
echo.
echo To run the game:
echo   cd %BUILD_DIR%\bin\%BUILD_TYPE%
echo   SCUMMStyleARPG.exe
echo.
echo Or use the launcher script:
echo   scripts\run_windows.bat
echo.

exit /b 0
