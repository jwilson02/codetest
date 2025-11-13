#!/bin/bash

# Build script for SCUMM VM-Style Story System
# This builds the C++ narrative engine

set -e

echo "=========================================="
echo "SCUMM VM-Style Story System Builder"
echo "=========================================="
echo ""

# Create build directory
BUILD_DIR="build_story"
mkdir -p ${BUILD_DIR}

# CMake configuration
echo "Configuring build..."
cd ${BUILD_DIR}

cmake -S .. -B . \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_TESTS=OFF \
    -DENABLE_LOCALIZATION=ON

echo ""
echo "Building..."
cmake --build . --config Release

echo ""
echo "=========================================="
echo "Build complete!"
echo "=========================================="
echo ""
echo "Executables:"
echo "  Demo: ${BUILD_DIR}/bin/StorySystemDemo"
echo ""
echo "Libraries:"
echo "  Story: ${BUILD_DIR}/lib/libStory.a"
echo ""
echo "To run the demo:"
echo "  cd ${BUILD_DIR} && ./bin/StorySystemDemo"
echo ""
