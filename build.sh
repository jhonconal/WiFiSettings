#!/bin/bash
# WiFiSettings Build Script

set -e

# ==========================================
# 1. Check Dependencies
# ==========================================
echo "[1/4] Checking required tools..."
if ! command -v qmake >/dev/null 2>&1; then
    echo "Error: qmake not found. Please install qtbase5-dev"
    exit 1
fi

if ! command -v lrelease >/dev/null 2>&1 && ! command -v qtchooser >/dev/null 2>&1; then
    echo "Error: lrelease not found. Please install qttools5-dev-tools"
    exit 1
fi

# ==========================================
# 2. Compile Translations (.ts -> .qm)
# ==========================================
echo "[2/4] Compiling translations..."
QT_SELECT=qt5 lrelease resources/translations/wifi_en.ts resources/translations/wifi_zh.ts

# ==========================================
# 3. Setup Build Directory
# ==========================================
echo "[3/4] Setting up build directory..."
mkdir -p build
cd build

# ==========================================
# 4. Compile Project
# ==========================================
echo "[4/4] Compiling project..."
qmake ../WiFiSettings.pro
make -j$(nproc)

echo "=========================================="
echo "Build successful! ✨"
echo "You can run the application using:"
echo "sudo ./build/WiFiSettings"
echo "=========================================="
