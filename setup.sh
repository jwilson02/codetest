#!/bin/bash
# Setup script for Amazon Deal Tracker

echo "========================================="
echo "Amazon Deal Tracker - Setup"
echo "========================================="
echo ""

# Check Python version
echo "[1/5] Checking Python version..."
if command -v python3 &> /dev/null; then
    PYTHON_VERSION=$(python3 --version | cut -d' ' -f2)
    echo "✓ Python 3 found: $PYTHON_VERSION"
else
    echo "❌ Python 3 not found. Please install Python 3.8 or higher."
    exit 1
fi

# Create virtual environment
echo ""
echo "[2/5] Creating virtual environment..."
if [ ! -d "venv" ]; then
    python3 -m venv venv
    echo "✓ Virtual environment created"
else
    echo "✓ Virtual environment already exists"
fi

# Activate virtual environment
echo ""
echo "[3/5] Activating virtual environment..."
source venv/bin/activate
echo "✓ Virtual environment activated"

# Install dependencies
echo ""
echo "[4/5] Installing dependencies..."
pip install --upgrade pip
pip install -r requirements.txt
echo "✓ Dependencies installed"

# Setup environment file
echo ""
echo "[5/5] Setting up environment configuration..."
if [ ! -f ".env" ]; then
    cp .env.example .env
    echo "✓ Created .env file from template"
    echo ""
    echo "⚠️  IMPORTANT: Please edit .env and add your Amazon credentials:"
    echo "   nano .env"
else
    echo "✓ .env file already exists"
fi

# Create data directory
mkdir -p data

echo ""
echo "========================================="
echo "Setup Complete!"
echo "========================================="
echo ""
echo "Next steps:"
echo "1. Edit .env and add your Amazon credentials"
echo "2. Run: source venv/bin/activate"
echo "3. Run: python main.py scrape"
echo ""
