#!/bin/bash
set -e

# Script to setup chroot and run ipcamd
# Usage: ./run_chroot.sh [x64|system] [foreground|background]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IPCAMERA_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CHROOT_DIR="${IPCAMERA_ROOT}/simulated_root"
BUILD_TARGET="${1:-x64}"
RUN_MODE="${2:-foreground}"

# Show help
if [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ "$1" = "help" ]; then
    echo "IPCamera Chroot Runner"
    echo ""
    echo "Usage: $0 [aarch64-ca53|aarch64-gnu|x64] [foreground|background]"
    echo ""
    echo "  Build target:"
    echo "    aarch64-ca53 - ARM CA53 cross-compile"
    echo "    aarch64-gnu  - ARM aarch64 cross-compile"
    echo "    x64          - Ubuntu x64 native build (default)"
    echo ""
    echo "  Run mode:"
    echo "    foreground (fg) - Run in foreground (default)"
    echo "    background (bg) - Run in background"
    echo ""
    echo "Examples:"
    echo "  $0                      # Run x64 build in foreground"
    echo "  $0 x64 bg               # Run x64 build in background"
    echo "  $0 aarch64-ca53 fg      # Run aarch64-ca53 build in foreground"
    echo "  $0 aarch64-gnu bg       # Run aarch64-gnu build in background"
    echo ""
    exit 0
fi

echo "=========================================="
echo "IPCamera Chroot Runner"
echo "=========================================="
echo "Build target: $BUILD_TARGET"
echo "Run mode: $RUN_MODE"
echo ""

# Check if build exists
BUILD_DIR="${IPCAMERA_ROOT}/build/${BUILD_TARGET}"
if [ ! -f "${BUILD_DIR}/apps/ipcamd/ipcamd" ]; then
    echo "Error: Build not found at ${BUILD_DIR}/apps/ipcamd/ipcamd"
    echo ""
    echo "Please build first:"
    echo "  ./build.sh ${BUILD_TARGET}"
    echo ""
    exit 1
fi

# Setup chroot
echo "Setting up chroot environment..."
echo "------------------------------------------"
"${SCRIPT_DIR}/setup_chroot.sh" "${BUILD_TARGET}"

echo ""
echo "=========================================="
echo "Starting ipcamd in chroot..."
echo "=========================================="
echo ""

# Check if already running
if pgrep -f "chroot.*ipcamd" > /dev/null; then
    echo "Warning: ipcamd appears to be already running in chroot"
    echo ""
    read -p "Kill existing process? [y/N] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo pkill -f "chroot.*ipcamd" || true
        sleep 1
    else
        echo "Exiting..."
        exit 1
    fi
fi

case "$RUN_MODE" in
    foreground|fg)
        echo "Running in foreground (Ctrl+C to stop)..."
        echo ""
        sudo chroot "${CHROOT_DIR}" /usr/bin/ipcamd -c /etc/ipcamera 2>&1
        ;;
    background|bg)
        echo "Running in background..."
        sudo chroot "${CHROOT_DIR}" /usr/bin/ipcamd -c /etc/ipcamera > /tmp/ipcamd_chroot.log 2>&1 &
        PID=$!
        sleep 1
        
        if ps -p $PID > /dev/null; then
            echo "✓ ipcamd started successfully (PID: $PID)"
            echo ""
            echo "Log file: /tmp/ipcamd_chroot.log"
            echo ""
            echo "To view logs:"
            echo "  tail -f /tmp/ipcamd_chroot.log"
            echo ""
            echo "To stop:"
            echo "  sudo kill $PID"
            echo "  or"
            echo "  sudo pkill -f 'chroot.*ipcamd'"
        else
            echo "✗ Failed to start ipcamd"
            echo "Check log: /tmp/ipcamd_chroot.log"
            exit 1
        fi
        ;;
    *)
        echo "Error: Unknown run mode '$RUN_MODE'"
        echo ""
        echo "Usage: $0 [aarch64-ca53|aarch64-gnu|x64] [foreground|background]"
        echo ""
        echo "  Build target:"
        echo "    aarch64-ca53 - ARM CA53 cross-compile"
        echo "    aarch64-gnu  - ARM aarch64 cross-compile"
        echo "    x64          - Ubuntu x64 native build (default)"
        echo ""
        echo "  Run mode:"
        echo "    foreground (fg) - Run in foreground (default)"
        echo "    background (bg) - Run in background"
        echo ""
        exit 1
        ;;
esac

echo ""
echo "=========================================="
