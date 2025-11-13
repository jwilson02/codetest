#!/bin/bash
# Dependency installation script for Linux and macOS
# Installs all required libraries for building SCUMM Style ARPG

set -e  # Exit on error

echo "==============================================="
echo "SCUMM Style ARPG - Dependency Installer"
echo "==============================================="
echo ""

# Detect OS
OS_TYPE="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS_TYPE="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS_TYPE="macos"
else
    echo "ERROR: Unsupported OS: $OSTYPE"
    echo "This script supports Linux and macOS only."
    echo "For Windows, see docs/BUILDING.md"
    exit 1
fi

echo "Detected OS: $OS_TYPE"
echo ""

# Function to install on Ubuntu/Debian
install_ubuntu() {
    echo "Installing dependencies for Ubuntu/Debian..."
    echo ""

    # Update package list
    echo "Updating package list..."
    sudo apt-get update

    # Install build tools
    echo "Installing build tools..."
    sudo apt-get install -y \
        build-essential \
        cmake \
        git \
        pkg-config

    # Install SDL2 and related libraries
    echo "Installing SDL2 libraries..."
    sudo apt-get install -y \
        libsdl2-dev \
        libsdl2-mixer-dev \
        libsdl2-image-dev \
        libsdl2-ttf-dev

    # Install OpenGL and GLEW
    echo "Installing OpenGL and GLEW..."
    sudo apt-get install -y \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        libglew-dev

    # Install GLM
    echo "Installing GLM..."
    sudo apt-get install -y libglm-dev

    # Optional: Install additional development tools
    echo "Installing optional development tools..."
    sudo apt-get install -y \
        clang \
        clang-format \
        clang-tidy \
        gdb \
        valgrind

    echo ""
    echo "Dependencies installed successfully on Ubuntu/Debian!"
}

# Function to install on Fedora/RHEL
install_fedora() {
    echo "Installing dependencies for Fedora/RHEL..."
    echo ""

    # Install build tools
    echo "Installing build tools..."
    sudo dnf groupinstall -y "Development Tools"
    sudo dnf install -y cmake git pkg-config

    # Install SDL2 and related libraries
    echo "Installing SDL2 libraries..."
    sudo dnf install -y \
        SDL2-devel \
        SDL2_mixer-devel \
        SDL2_image-devel \
        SDL2_ttf-devel

    # Install OpenGL and GLEW
    echo "Installing OpenGL and GLEW..."
    sudo dnf install -y \
        mesa-libGL-devel \
        mesa-libGLU-devel \
        glew-devel

    # Install GLM
    echo "Installing GLM..."
    sudo dnf install -y glm-devel

    # Optional: Install additional development tools
    echo "Installing optional development tools..."
    sudo dnf install -y \
        clang \
        clang-tools-extra \
        gdb \
        valgrind

    echo ""
    echo "Dependencies installed successfully on Fedora/RHEL!"
}

# Function to install on Arch Linux
install_arch() {
    echo "Installing dependencies for Arch Linux..."
    echo ""

    # Update package database
    echo "Updating package database..."
    sudo pacman -Sy

    # Install build tools
    echo "Installing build tools..."
    sudo pacman -S --needed --noconfirm \
        base-devel \
        cmake \
        git \
        pkg-config

    # Install SDL2 and related libraries
    echo "Installing SDL2 libraries..."
    sudo pacman -S --needed --noconfirm \
        sdl2 \
        sdl2_mixer \
        sdl2_image \
        sdl2_ttf

    # Install OpenGL and GLEW
    echo "Installing OpenGL and GLEW..."
    sudo pacman -S --needed --noconfirm \
        mesa \
        glu \
        glew

    # Install GLM
    echo "Installing GLM..."
    sudo pacman -S --needed --noconfirm glm

    # Optional: Install additional development tools
    echo "Installing optional development tools..."
    sudo pacman -S --needed --noconfirm \
        clang \
        gdb \
        valgrind

    echo ""
    echo "Dependencies installed successfully on Arch Linux!"
}

# Function to install on macOS
install_macos() {
    echo "Installing dependencies for macOS..."
    echo ""

    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "ERROR: Homebrew is not installed!"
        echo ""
        echo "Please install Homebrew first:"
        echo '  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
        echo ""
        echo "After installing Homebrew, run this script again."
        exit 1
    fi

    # Update Homebrew
    echo "Updating Homebrew..."
    brew update

    # Install build tools (if not already installed via Xcode)
    echo "Installing build tools..."
    brew install cmake git pkg-config

    # Install SDL2 and related libraries
    echo "Installing SDL2 libraries..."
    brew install sdl2 sdl2_mixer sdl2_image sdl2_ttf

    # Install GLEW
    echo "Installing GLEW..."
    brew install glew

    # Install GLM
    echo "Installing GLM..."
    brew install glm

    # Optional: Install additional development tools
    echo "Installing optional development tools..."
    brew install llvm

    echo ""
    echo "Dependencies installed successfully on macOS!"
    echo ""
    echo "NOTE: Make sure you have Xcode Command Line Tools installed."
    echo "If not, run: xcode-select --install"
}

# Main installation logic
if [ "$OS_TYPE" = "linux" ]; then
    # Detect Linux distribution
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
    else
        echo "ERROR: Cannot detect Linux distribution"
        exit 1
    fi

    echo "Detected Linux distribution: $DISTRO"
    echo ""

    case $DISTRO in
        ubuntu|debian|pop|mint)
            install_ubuntu
            ;;
        fedora|rhel|centos)
            install_fedora
            ;;
        arch|manjaro)
            install_arch
            ;;
        *)
            echo "ERROR: Unsupported Linux distribution: $DISTRO"
            echo ""
            echo "Supported distributions:"
            echo "  - Ubuntu/Debian"
            echo "  - Fedora/RHEL"
            echo "  - Arch Linux"
            echo ""
            echo "For other distributions, please install these packages manually:"
            echo "  - CMake"
            echo "  - SDL2, SDL2_mixer, SDL2_image, SDL2_ttf"
            echo "  - OpenGL, GLEW"
            echo "  - GLM"
            echo ""
            exit 1
            ;;
    esac
elif [ "$OS_TYPE" = "macos" ]; then
    install_macos
fi

echo ""
echo "==============================================="
echo "Installation Complete!"
echo "==============================================="
echo ""
echo "All dependencies have been installed."
echo ""
echo "Next steps:"
echo "  1. Build the project:"
echo "     ./scripts/build_${OS_TYPE}.sh"
echo ""
echo "  2. Or manually:"
echo "     mkdir build && cd build"
echo "     cmake .."
echo "     cmake --build ."
echo ""
echo "For more information, see docs/BUILDING.md"
echo ""
