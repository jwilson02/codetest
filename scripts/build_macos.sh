#!/bin/bash
# Build script for macOS
# Requires: CMake, Xcode Command Line Tools, and all dependencies installed

set -e  # Exit on error

echo "==============================================="
echo "SCUMM Style ARPG - macOS Build Script"
echo "==============================================="
echo ""

# Default values
BUILD_DIR="build-macos"
BUILD_TYPE="Release"
CLEAN_BUILD=false
JOBS=$(sysctl -n hw.ncpu)

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --debug       Build in Debug mode (default: Release)"
            echo "  --clean       Clean build directory before building"
            echo "  --jobs N      Number of parallel build jobs (default: $(sysctl -n hw.ncpu))"
            echo "  --help        Show this help message"
            echo ""
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo "Build Type: $BUILD_TYPE"
echo "Build Directory: $BUILD_DIR"
echo "Parallel Jobs: $JOBS"
echo ""

# Check if Xcode Command Line Tools are installed
if ! command -v xcode-select &> /dev/null || ! xcode-select -p &> /dev/null; then
    echo "ERROR: Xcode Command Line Tools not found!"
    echo "Please install them by running:"
    echo "  xcode-select --install"
    exit 1
fi

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found! Please install CMake."
    echo "Using Homebrew: brew install cmake"
    echo "Or download from: https://cmake.org/download/"
    exit 1
fi

# Check if Homebrew is installed (recommended for dependencies)
if ! command -v brew &> /dev/null; then
    echo "WARNING: Homebrew not found!"
    echo "It's recommended to install Homebrew for easier dependency management:"
    echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    echo ""
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    echo ""
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Change to build directory
cd "$BUILD_DIR"

# Configure with CMake
echo "==============================================="
echo "Configuring project with CMake..."
echo "==============================================="
echo ""

cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: CMake configuration failed!"
    echo ""
    echo "Common issues:"
    echo "  1. Missing dependencies - Run: scripts/install_deps.sh"
    echo "  2. Make sure you have Homebrew installed"
    echo "  3. See docs/BUILDING.md for detailed instructions"
    echo ""
    cd ..
    exit 1
fi

# Build the project
echo ""
echo "==============================================="
echo "Building project..."
echo "==============================================="
echo ""

cmake --build . --config "$BUILD_TYPE" --parallel "$JOBS"

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed!"
    echo "Check the error messages above for details."
    echo ""
    cd ..
    exit 1
fi

# Return to root directory
cd ..

echo ""
echo "==============================================="
echo "Build completed successfully!"
echo "==============================================="
echo ""
echo "Executable location: $BUILD_DIR/bin/SCUMMStyleARPG"
echo ""
echo "To run the game:"
echo "  cd $BUILD_DIR/bin"
echo "  ./SCUMMStyleARPG"
echo ""
echo "Or use:"
echo "  $BUILD_DIR/bin/SCUMMStyleARPG"
echo ""
