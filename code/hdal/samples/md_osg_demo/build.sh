#!/bin/bash
#
# MD OSG Demo Build and Deploy Script
# Usage: ./build.sh [build|clean|push|run|all]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SDK_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
OUTPUT_DIR="$SDK_ROOT/output"
SAMPLE_NAME="md_osg_demo"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() {
    echo -e "${CYAN}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

show_help() {
    echo "
╔══════════════════════════════════════════════════════════════╗
║           MD Integrated Demo - Build Script                  ║
╚══════════════════════════════════════════════════════════════╝

Usage: $0 [command]

Commands:
    build       Build the demo using SDK build system
    clean       Clean build artifacts
    push        Push binary to device via ADB
    run         Run demo on device (30s, high sensitivity)
    run_custom  Run demo with custom parameters
    test        Quick test run (10s)
    all         Build and push to device

Examples:
    $0 build    # Build the demo
    $0 push     # Push to device
    $0 run      # Run on device for 30 seconds
"
}

do_build() {
    log_info "Building $SAMPLE_NAME..."
    
    cd "$SDK_ROOT"
    
    # Check if envsetup.sh has been sourced
    if [ -z "$NVT_HDAL_DIR" ]; then
        log_info "Sourcing build environment..."
        source build/envsetup.sh
    fi
    
    # Build the sample
    log_info "Compiling..."
    cd "$SCRIPT_DIR"
    make clean 2>/dev/null || true
    make
    
    # Copy output
    if [ -f "$SAMPLE_NAME" ]; then
        log_success "Binary built: $SCRIPT_DIR/$SAMPLE_NAME"
        ls -la "$SAMPLE_NAME"
    else
        log_error "Build failed!"
        exit 1
    fi
}

do_clean() {
    log_info "Cleaning..."
    cd "$SCRIPT_DIR"
    make clean 2>/dev/null || true
    rm -f "$SAMPLE_NAME" *.o
    log_success "Clean complete"
}

do_push() {
    log_info "Pushing to device via ADB..."
    
    # Check ADB connection
    if ! adb devices | grep -q device$; then
        log_error "No ADB device connected!"
        exit 1
    fi
    
    # Find binary
    BINARY=""
    if [ -f "$SCRIPT_DIR/$SAMPLE_NAME" ]; then
        BINARY="$SCRIPT_DIR/$SAMPLE_NAME"
    elif [ -f "$OUTPUT_DIR/bin/$SAMPLE_NAME" ]; then
        BINARY="$OUTPUT_DIR/bin/$SAMPLE_NAME"
    else
        log_error "Binary not found. Run 'build' first."
        exit 1
    fi
    
    adb push "$BINARY" /mnt/sd/
    adb shell chmod +x /mnt/sd/$SAMPLE_NAME
    
    log_success "Pushed to /mnt/sd/$SAMPLE_NAME"
}

do_run() {
    local duration=${1:-30}
    local sensitivity=${2:-2}
    
    log_info "Running demo on device..."
    log_info "Duration: ${duration}s, Sensitivity: $sensitivity (0=low,1=med,2=high,3=super)"
    
    # Check if ipcamd is running
    if adb shell "pidof ipcamd" &>/dev/null; then
        echo ""
        log_error "ipcamd is currently running!"
        echo -e "${YELLOW}The demo requires exclusive access to the camera hardware.${NC}"
        echo ""
        echo "Options:"
        echo "  1. Stop ipcamd:  adb shell 'killall ipcamd'"
        echo "  2. Run demo:     $0 run"
        echo "  3. Restart:      adb shell 'ipcamd &'"
        echo ""
        read -p "Stop ipcamd and continue? [y/N] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            adb shell "killall ipcamd" || true
            sleep 2
        else
            exit 1
        fi
    fi
    
    # Run the demo
    adb shell "cd /mnt/sd && ./$SAMPLE_NAME $duration $sensitivity"
}

do_test() {
    log_info "Quick test run (10 seconds, high sensitivity)..."
    do_run 10 2
}

do_all() {
    do_build
    do_push
}

# Parse command
case "${1:-help}" in
    build)
        do_build
        ;;
    clean)
        do_clean
        ;;
    push)
        do_push
        ;;
    run)
        do_run "${2:-30}" "${3:-2}"
        ;;
    run_custom)
        if [ -z "$2" ] || [ -z "$3" ]; then
            echo "Usage: $0 run_custom <duration> <sensitivity>"
            echo "  sensitivity: 0=low, 1=medium, 2=high, 3=super_high"
            exit 1
        fi
        do_run "$2" "$3"
        ;;
    test)
        do_test
        ;;
    all)
        do_all
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        log_error "Unknown command: $1"
        show_help
        exit 1
        ;;
esac
