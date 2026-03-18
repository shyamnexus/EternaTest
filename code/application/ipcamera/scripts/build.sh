#!/bin/bash
set -e

# Script to build ipcamera for different targets
# Usage: ./build.sh [x64|system|all]

IPCAMERA_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_TARGET="${1:-x64}"

# Show help
if [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ "$1" = "help" ]; then
    echo "IPCamera Build Script"
    echo ""
    echo "Usage: $0 [aarch64-ca53|aarch64-gnu|x64|all]"
    echo ""
    echo "  aarch64-ca53 - ARM CA53 cross-compile (NVT SDK default)"
    echo "  aarch64-gnu  - ARM aarch64 cross-compile (generic toolchain)"
    echo "  x64          - Ubuntu x64 native build (default)"
    echo "  all          - Build all targets"
    echo ""
    exit 0
fi

echo "=========================================="
echo "IPCamera Build Script"
echo "=========================================="

build_aarch64_ca53() {
    echo ""
    echo "Building for ARM CA53 (NVT SDK default)..."
    echo "------------------------------------------"
    
    mkdir -p "${IPCAMERA_ROOT}/build/aarch64-ca53"
    cd "${IPCAMERA_ROOT}/build/aarch64-ca53"
    
    cmake ../.. \
        -G "Ninja" \
        -DCMAKE_TOOLCHAIN_FILE="${IPCAMERA_ROOT}/cmake/toolchains/aarch64-ca53.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF
    
    ninja
    
    echo ""
    echo "✓ aarch64-ca53 build complete!"
    echo "  Binary: build/aarch64-ca53/apps/ipcamd/ipcamd"
    file apps/ipcamd/ipcamd | grep -o "ARM aarch64" > /dev/null && echo "  Architecture: ARM aarch64 ✓"
}

build_aarch64_gnu() {
    echo ""
    echo "Building for ARM aarch64 (generic toolchain)..."
    echo "------------------------------------------"
    
    mkdir -p "${IPCAMERA_ROOT}/build/aarch64-gnu"
    cd "${IPCAMERA_ROOT}/build/aarch64-gnu"
    
    cmake ../.. \
        -G "Ninja" \
        -DCMAKE_TOOLCHAIN_FILE="${IPCAMERA_ROOT}/cmake/toolchains/aarch64-linux-gnu.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF
    
    ninja
    
    echo ""
    echo "✓ aarch64-gnu build complete!"
    echo "  Binary: build/aarch64-gnu/apps/ipcamd/ipcamd"
    file apps/ipcamd/ipcamd | grep -o "ARM aarch64" > /dev/null && echo "  Architecture: ARM aarch64 ✓"
}

build_x64() {
    echo ""
    echo "Building for Ubuntu x64 (native)..."
    echo "------------------------------------------"
    
    mkdir -p "${IPCAMERA_ROOT}/build/x64"
    cd "${IPCAMERA_ROOT}/build/x64"
    
    cmake ../.. \
        -G "Ninja" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF \
        -DIPCAMERA_USE_SYSTEM_COMPILER=ON
    
    ninja
    
    echo ""
    echo "✓ x64 build complete!"
    echo "  Binary: build/x64/apps/ipcamd/ipcamd"
    file apps/ipcamd/ipcamd | grep -o "x86-64" > /dev/null && echo "  Architecture: x86-64 ✓"
}

case "$BUILD_TARGET" in
    aarch64-ca53|ca53|default)
        build_aarch64_ca53
        ;;
    aarch64-gnu|gnu)
        build_aarch64_gnu
        ;;
    x64)
        build_x64
        ;;
    all)
        build_aarch64_ca53
        build_aarch64_gnu
        build_x64
        ;;
    *)
        echo "Error: Unknown build target '$BUILD_TARGET'"
        echo ""
        echo "Usage: $0 [aarch64-ca53|aarch64-gnu|x64|all]"
        echo ""
        echo "  aarch64-ca53 - ARM CA53 cross-compile (NVT SDK default)"
        echo "  aarch64-gnu  - ARM aarch64 cross-compile (generic toolchain)"
        echo "  x64          - Ubuntu x64 native build"
        echo "  all          - Build all targets"
        echo ""
        exit 1
        ;;
esac

echo ""
echo "=========================================="
echo "Build Complete!"
echo "=========================================="
