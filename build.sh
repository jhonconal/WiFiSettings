#!/bin/bash
# WiFiSettings Build Script

set -e

# 构建架构参数 (默认: X86)
ARCH=${1:-X86}

echo "Building for architecture: $ARCH"

if [ "$ARCH" = "ARM" ]; then
    # ARM 构建 - source Yocto 环境
    echo "ARM build: Sourcing Yocto environment..."
    source /opt/fsl-imx-xwayland/5.10-gatesgarth/environment-setup-cortexa53-crypto-poky-linux
    CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=../../tools/toolchain.cmake"
else
    # X86 构建 (默认)
    echo "X86 build: Native compilation"
    CMAKE_ARGS=""
fi

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
[ -d build ] && rm -rf build
mkdir -p build && cd build

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
