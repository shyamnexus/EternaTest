#!/bin/bash
#
# NT98538 AI Demo Model Installation Script
# This script installs AI models to the target device via ADB
#
# Usage: ./install_models.sh [device_ip]
#        If device_ip is not provided, uses USB ADB connection
#
# Author: IP Camera Team
# Date: 2026

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_ROOT="${SCRIPT_DIR}/../../../.."
DEVICE_MODEL_PATH="/mnt/sd/CNNLib/para"
DEVICE_CONFIG_PATH="/mnt/sd/configs"
MODEL_SOURCE_DIR="${SDK_ROOT}/models"  # Local models directory

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_banner() {
    echo -e "${BLUE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║       NT98538 AI Demo Model Installation Script              ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check ADB connection
check_adb() {
    if [ -n "$1" ]; then
        export ADB_CMD="adb -s $1"
        log_info "Using ADB over network: $1"
    else
        export ADB_CMD="adb"
        log_info "Using USB ADB connection"
    fi

    if ! ${ADB_CMD} devices | grep -q "device$"; then
        log_error "No ADB device connected. Please connect device or provide IP address."
        echo ""
        echo "For USB connection:"
        echo "  1. Connect USB cable"
        echo "  2. Run: adb devices"
        echo ""
        echo "For network connection:"
        echo "  1. On device: setprop service.adb.tcp.port 5555"
        echo "  2. On host: adb connect <device_ip>:5555"
        echo "  3. Run: ./install_models.sh <device_ip>:5555"
        exit 1
    fi

    log_info "ADB connection verified"
}

# Create directory structure on device
create_dirs() {
    log_info "Creating directory structure on device..."
    
    ${ADB_CMD} shell "mkdir -p ${DEVICE_MODEL_PATH}/pvdcnn"
    ${ADB_CMD} shell "mkdir -p ${DEVICE_MODEL_PATH}/fdcnn"
    ${ADB_CMD} shell "mkdir -p ${DEVICE_MODEL_PATH}/public"
    ${ADB_CMD} shell "mkdir -p ${DEVICE_CONFIG_PATH}"
    
    log_info "Directory structure created"
}

# Install PVDCNN models
install_pvdcnn() {
    log_info "Installing PVDCNN models..."
    
    local PVDCNN_MODELS=(
        "nvt_model_320_prune37.bin"
        "nvt_model_416_prune37.bin"
        "nvt_model_512_prune37.bin"
        "nvt_model_608_prune37.bin"
    )
    
    local found=0
    for model in "${PVDCNN_MODELS[@]}"; do
        if [ -f "${MODEL_SOURCE_DIR}/pvdcnn/${model}" ]; then
            log_info "  Pushing ${model}..."
            ${ADB_CMD} push "${MODEL_SOURCE_DIR}/pvdcnn/${model}" "${DEVICE_MODEL_PATH}/pvdcnn/"
            found=$((found + 1))
        fi
    done
    
    if [ $found -eq 0 ]; then
        log_warn "No PVDCNN models found in ${MODEL_SOURCE_DIR}/pvdcnn/"
        log_warn "Please obtain models from Novatek SDK package and place them in:"
        log_warn "  ${MODEL_SOURCE_DIR}/pvdcnn/"
    else
        log_info "Installed ${found} PVDCNN models"
    fi
    
    # Install PVDCNN config
    if [ -f "${MODEL_SOURCE_DIR}/pvdcnn/pvdcnn_para_config.txt" ]; then
        ${ADB_CMD} push "${MODEL_SOURCE_DIR}/pvdcnn/pvdcnn_para_config.txt" "${DEVICE_CONFIG_PATH}/"
    fi
}

# Install FDCNN models
install_fdcnn() {
    log_info "Installing FDCNN models..."
    
    local FDCNN_MODELS=(
        "nvt_model.bin"
        "nvt_model_light.bin"
    )
    
    local found=0
    for model in "${FDCNN_MODELS[@]}"; do
        if [ -f "${MODEL_SOURCE_DIR}/fdcnn/${model}" ]; then
            log_info "  Pushing ${model}..."
            ${ADB_CMD} push "${MODEL_SOURCE_DIR}/fdcnn/${model}" "${DEVICE_MODEL_PATH}/fdcnn/"
            found=$((found + 1))
        fi
    done
    
    if [ $found -eq 0 ]; then
        log_warn "No FDCNN models found in ${MODEL_SOURCE_DIR}/fdcnn/"
        log_warn "Please obtain models from Novatek SDK package and place them in:"
        log_warn "  ${MODEL_SOURCE_DIR}/fdcnn/"
    else
        log_info "Installed ${found} FDCNN models"
    fi
}

# Install public/shared models
install_public() {
    log_info "Installing public/shared models..."
    
    if [ -f "${MODEL_SOURCE_DIR}/public/nvt_model.bin" ]; then
        ${ADB_CMD} push "${MODEL_SOURCE_DIR}/public/nvt_model.bin" "${DEVICE_MODEL_PATH}/public/"
        log_info "Installed public model"
    else
        log_warn "No public models found in ${MODEL_SOURCE_DIR}/public/"
    fi
}

# Install combined model from rootfs
install_combin_model() {
    log_info "Checking for combined model in rootfs..."
    
    local COMBIN_MODEL="${SDK_ROOT}/BSP/root-fs/rootfs/etc_Model/etc_COMMON_AOS/combin_model.bin"
    
    if [ -f "${COMBIN_MODEL}" ]; then
        log_info "  Pushing combin_model.bin..."
        ${ADB_CMD} push "${COMBIN_MODEL}" "${DEVICE_MODEL_PATH}/"
        log_info "Installed combined model"
    else
        log_warn "Combined model not found at ${COMBIN_MODEL}"
    fi
}

# Install demo binaries
install_demos() {
    log_info "Installing AI demo binaries..."
    
    local DEMO_DIR="${SCRIPT_DIR}"
    local DEVICE_BIN_PATH="/mnt/sd/ai_demos"
    
    ${ADB_CMD} shell "mkdir -p ${DEVICE_BIN_PATH}"
    
    local demos=(
        "ai_pvdcnn_demo"
        "ai_fdcnn_demo"
        "ai_motion_detection_demo"
        "ai_vqa_demo"
        "ai_line_crossing_demo"
        "ai_intrusion_detection_demo"
        "ai_privacy_mosaic_demo"
        "ai_crowd_counting_demo"
        "ai_object_tracking_demo"
        "ai_cross_counting_demo"
        "ai_heat_map_demo"
    )
    
    local found=0
    for demo in "${demos[@]}"; do
        local binary="${DEMO_DIR}/${demo}/${demo}"
        if [ -f "${binary}" ]; then
            log_info "  Pushing ${demo}..."
            ${ADB_CMD} push "${binary}" "${DEVICE_BIN_PATH}/"
            ${ADB_CMD} shell "chmod +x ${DEVICE_BIN_PATH}/${demo}"
            found=$((found + 1))
        fi
    done
    
    if [ $found -gt 0 ]; then
        log_info "Installed ${found} demo binaries to ${DEVICE_BIN_PATH}"
    else
        log_warn "No demo binaries found. Build them first with 'make all'"
    fi
}

# Create test script on device
create_test_script() {
    log_info "Creating test script on device..."
    
    cat << 'EOF' > /tmp/run_ai_demos.sh
#!/bin/sh
#
# AI Demo Test Runner
#

DEMO_PATH="/mnt/sd/ai_demos"
MODEL_PATH="/mnt/sd/CNNLib/para"

echo "=== NT98538 AI Demo Runner ==="
echo ""
echo "Available demos:"
echo "  1. ai_pvdcnn_demo       - Person/Vehicle Detection"
echo "  2. ai_fdcnn_demo        - Face Detection"
echo "  3. ai_motion_detection_demo - Motion Detection"
echo "  4. ai_vqa_demo          - Video Quality Analysis"
echo "  5. ai_line_crossing_demo - Line Crossing"
echo "  6. ai_intrusion_detection_demo - Intrusion Detection"
echo "  7. ai_privacy_mosaic_demo - Privacy Mosaic"
echo "  8. ai_crowd_counting_demo - Crowd Counting"
echo "  9. ai_object_tracking_demo - Object Tracking"
echo "  10. ai_cross_counting_demo - Cross Counting"
echo "  11. ai_heat_map_demo     - Heat Map"
echo ""

check_models() {
    echo "Model files status:"
    ls -la ${MODEL_PATH}/pvdcnn/ 2>/dev/null || echo "  pvdcnn: NOT INSTALLED"
    ls -la ${MODEL_PATH}/fdcnn/ 2>/dev/null || echo "  fdcnn: NOT INSTALLED"
    ls -la ${MODEL_PATH}/public/ 2>/dev/null || echo "  public: NOT INSTALLED"
}

run_demo() {
    local demo=$1
    local duration=${2:-30}
    
    if [ -x "${DEMO_PATH}/${demo}" ]; then
        echo "Running ${demo} for ${duration} seconds..."
        cd ${DEMO_PATH}
        ./${demo} ${duration}
    else
        echo "Demo not found: ${demo}"
    fi
}

case "$1" in
    1|pvd|pvdcnn)
        run_demo ai_pvdcnn_demo $2
        ;;
    2|fd|fdcnn|face)
        run_demo ai_fdcnn_demo $2
        ;;
    3|md|motion)
        run_demo ai_motion_detection_demo $2
        ;;
    4|vqa|quality)
        run_demo ai_vqa_demo $2
        ;;
    5|line|crossing)
        run_demo ai_line_crossing_demo $2
        ;;
    6|intrusion)
        run_demo ai_intrusion_detection_demo $2
        ;;
    7|privacy|mosaic)
        run_demo ai_privacy_mosaic_demo $2
        ;;
    8|crowd|count)
        run_demo ai_crowd_counting_demo $2
        ;;
    9|track|tracking)
        run_demo ai_object_tracking_demo $2
        ;;
    10|cross)
        run_demo ai_cross_counting_demo $2
        ;;
    11|heat|heatmap)
        run_demo ai_heat_map_demo $2
        ;;
    check|status)
        check_models
        ;;
    all)
        for demo in ai_motion_detection_demo ai_vqa_demo ai_line_crossing_demo; do
            run_demo $demo 10
            echo ""
        done
        ;;
    *)
        echo "Usage: $0 <demo_number|demo_name> [duration]"
        echo ""
        echo "Examples:"
        echo "  $0 1 30        # Run PVDCNN demo for 30 seconds"
        echo "  $0 motion 60   # Run motion detection for 60 seconds"
        echo "  $0 check       # Check model installation status"
        echo "  $0 all         # Run quick test of all demos"
        ;;
esac
EOF
    
    ${ADB_CMD} push /tmp/run_ai_demos.sh /mnt/sd/ai_demos/
    ${ADB_CMD} shell "chmod +x /mnt/sd/ai_demos/run_ai_demos.sh"
    rm /tmp/run_ai_demos.sh
    
    log_info "Test script installed: /mnt/sd/ai_demos/run_ai_demos.sh"
}

# Print model info
print_model_info() {
    echo ""
    echo -e "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}                     AI Model Information                       ${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
    echo "Expected model files and their locations on device:"
    echo ""
    echo "PVDCNN (Person/Vehicle Detection):"
    echo "  ${DEVICE_MODEL_PATH}/pvdcnn/"
    echo "    - nvt_model_320_prune37.bin  (320x320 input, pruned)"
    echo "    - nvt_model_416_prune37.bin  (416x416 input, pruned)"
    echo "    - nvt_model_512_prune37.bin  (512x512 input, pruned)"
    echo "    - nvt_model_608_prune37.bin  (608x608 input, pruned)"
    echo ""
    echo "FDCNN (Face Detection):"
    echo "  ${DEVICE_MODEL_PATH}/fdcnn/"
    echo "    - nvt_model.bin        (Full model)"
    echo "    - nvt_model_light.bin  (Lightweight model)"
    echo ""
    echo "Public/Shared Models:"
    echo "  ${DEVICE_MODEL_PATH}/public/"
    echo "    - nvt_model.bin        (Limit FDET model)"
    echo ""
    echo "Configuration Files:"
    echo "  ${DEVICE_CONFIG_PATH}/"
    echo "    - pvdcnn_para_config.txt"
    echo ""
    echo -e "${YELLOW}NOTE: Model files must be obtained from Novatek SDK package.${NC}"
    echo -e "${YELLOW}      Place them in: ${MODEL_SOURCE_DIR}/${NC}"
    echo ""
}

# Main function
main() {
    print_banner
    
    local device_ip="$1"
    
    check_adb "$device_ip"
    
    case "$2" in
        models)
            create_dirs
            install_pvdcnn
            install_fdcnn
            install_public
            install_combin_model
            ;;
        demos)
            install_demos
            create_test_script
            ;;
        all|"")
            create_dirs
            install_pvdcnn
            install_fdcnn
            install_public
            install_combin_model
            install_demos
            create_test_script
            ;;
        info)
            print_model_info
            ;;
        *)
            echo "Usage: $0 [device_ip] [models|demos|all|info]"
            echo ""
            echo "Options:"
            echo "  models  - Install model files only"
            echo "  demos   - Install demo binaries only"
            echo "  all     - Install everything (default)"
            echo "  info    - Print model file information"
            exit 1
            ;;
    esac
    
    echo ""
    log_info "Installation complete!"
    echo ""
    echo "To run demos on device:"
    echo "  adb shell"
    echo "  cd /mnt/sd/ai_demos"
    echo "  ./run_ai_demos.sh 1 30   # Run PVDCNN for 30 seconds"
    echo ""
}

main "$@"
