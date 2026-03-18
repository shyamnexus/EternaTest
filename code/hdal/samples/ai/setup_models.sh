#!/bin/bash
#
# NT98538 Model Setup Script
# Locates available AI models in the SDK and stages them for installation
#
# Usage: ./setup_models.sh
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_ROOT="${SCRIPT_DIR}/../../../.."
DEST_DIR="${SCRIPT_DIR}/models"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}=== NT98538 AI Model Staging Script ===${NC}"

# Create destination structure
mkdir -p "${DEST_DIR}/pvdcnn"
mkdir -p "${DEST_DIR}/fdcnn"
mkdir -p "${DEST_DIR}/public"

echo -e "${GREEN}Created structure in ${DEST_DIR}${NC}"

# Helper to find and copy
copy_model() {
    local name=$1
    local dest=$2
    local found=$(find "${SDK_ROOT}" -name "${name}" -type f | head -n 1)
    
    if [ -n "$found" ]; then
        echo "Found ${name} at $(dirname "${found}")"
        cp "${found}" "${DEST_DIR}/${dest}/${name}"
        echo -e "${GREEN}  -> Copied to ${dest}/${name}${NC}"
    else
        echo -e "${YELLOW}  X ${name} not found in SDK${NC}"
    fi
}

echo ""
echo "Searching for PVDCNN models..."
# Note: The specific pruned models might not be in the source tree, 
# but we look for what's available
copy_model "nvt_model_416_prune37.bin" "pvdcnn" || true
copy_model "nvt_model_320_prune37.bin" "pvdcnn" || true
copy_model "pvdcnn_para_config.txt" "pvdcnn" || true

echo ""
echo "Searching for FDCNN models..."
copy_model "nvt_model.bin" "fdcnn" || true
copy_model "nvt_model_light.bin" "fdcnn" || true

echo ""
echo "Searching for Shared models..."
# Sometimes nvt_model.bin is used as a generic name
if [ ! -f "${DEST_DIR}/fdcnn/nvt_model.bin" ]; then
     # Try to find any nvt_model.bin in ai_pattern if not found for fdcnn specifically
     GENERIC_MODEL=$(find "${SDK_ROOT}/code/hdal/samples/ai_pattern" -name "nvt_model.bin" -type f | head -n 1)
     if [ -n "$GENERIC_MODEL" ]; then
         cp "$GENERIC_MODEL" "${DEST_DIR}/public/"
         echo -e "${GREEN}  -> Copied generic nvt_model.bin to public/${NC}"
     fi
fi

# Copy combin_model if available (usually in rootfs)
COMBIN_MODEL="${SDK_ROOT}/BSP/root-fs/rootfs/etc_Model/etc_COMMON_AOS/combin_model.bin"
if [ -f "${COMBIN_MODEL}" ]; then
    cp "${COMBIN_MODEL}" "${DEST_DIR}/"
    echo -e "${GREEN}  -> Copied combin_model.bin${NC}"
fi

echo ""
echo -e "${BLUE}Staging complete.${NC}"
echo "Models are ready in ${DEST_DIR}/"
echo "You can now run ./install_models.sh to push them to the device."
