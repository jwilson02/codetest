#!/bin/bash
# Build script for Linux
# Requires: CMake, GCC/Clang, and all dependencies installed

set -e  # Exit on error

echo "==============================================="
echo "SCUMM Style ARPG - Linux Build Script"
echo "==============================================="
echo ""

# Default values
BUILD_DIR="build-linux"
BUILD_TYPE="Release"
CLEAN_BUILD=false
JOBS=$(nproc)

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
            echo "  --jobs N      Number of parallel build jobs (default: $(nproc))"
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

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found! Please install CMake."
    echo "  Ubuntu/Debian: sudo apt-get install cmake"
    echo "  Fedora: sudo dnf install cmake"
    echo "  Arch: sudo pacman -S cmake"
    exit 1
fi

# Check if build tools are installed
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "ERROR: No C++ compiler found! Please install GCC or Clang."
    echo "  Ubuntu/Debian: sudo apt-get install build-essential"
    echo "  Fedora: sudo dnf groupinstall 'Development Tools'"
    echo "  Arch: sudo pacman -S base-devel"
    exit 1
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
    echo "  1. Missing dependencies - Run: sudo scripts/install_deps.sh"
    echo "  2. See docs/BUILDING.md for detailed instructions"
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
