#!/bin/bash

# Script to cross-compile live555 for Novatek platform
# This script extracts, configures, and builds live555

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIVE_DIR="${SCRIPT_DIR}/live"
SDK_ROOT="${SCRIPT_DIR}/../../../.."

echo "=========================================="
echo "  Live555 Cross-Compilation Script"
echo "=========================================="
echo "Script directory: ${SCRIPT_DIR}"
echo "Live555 directory: ${LIVE_DIR}"
echo "SDK root: ${SDK_ROOT}"
echo ""

# Check if SDK environment is set up
if [ -z "${CROSS_COMPILE}" ]; then
    echo "SDK environment not set up. Setting up now..."
    cd "${SDK_ROOT}"
    source build/envsetup.sh
    cd "${SCRIPT_DIR}"
    echo ""
fi

# Display cross-compilation environment
echo "Cross-compilation environment:"
echo "  CROSS_COMPILE: ${CROSS_COMPILE}"
echo "  CC: ${CC}"
echo "  CXX: ${CXX}"
echo "  AR: ${AR}"
echo ""

# Extract live555 source if not already extracted
LIVE_TARBALL="${SCRIPT_DIR}/live555-latest.tar.gz"
if [ ! -d "${LIVE_DIR}" ]; then
    if [ -f "${LIVE_TARBALL}" ]; then
        echo "Extracting live555 source from ${LIVE_TARBALL}..."
        tar -xzf "${LIVE_TARBALL}" -C "${SCRIPT_DIR}"
        if [ $? -eq 0 ]; then
            echo "Extraction completed successfully!"
            echo ""
        else
            echo "Error: Failed to extract ${LIVE_TARBALL}"
            exit 1
        fi
    else
        echo "Error: Live555 tarball not found at ${LIVE_TARBALL}"
        echo "Please place live.2025.10.13.tar.gz in ${SCRIPT_DIR}"
        exit 1
    fi
fi

# Verify live directory exists
if [ ! -d "${LIVE_DIR}" ]; then
    echo "Error: Live555 source directory not found at ${LIVE_DIR} after extraction"
    exit 1
fi

# Navigate to live555 directory
cd "${LIVE_DIR}"

# Clean previous build if any
echo "Cleaning previous build..."
if [ -f Makefile ]; then
    make clean 2>/dev/null || true
fi

# Set SDK library include path for OpenSSL headers
export SDK_LIB_INCLUDE="${SDK_ROOT}/code/lib/include"
echo "SDK_LIB_INCLUDE: ${SDK_LIB_INCLUDE}"

# Configure for Novatek platform
echo "Configuring live555 for Novatek aarch64 platform..."
./genMakefiles nvt-aarch64

# Build live555
echo ""
echo "Building live555..."
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "  Build completed successfully!"
    echo "=========================================="
    echo ""
    echo "Libraries built:"
    find . -name "*.a" -type f
    echo ""
    echo "Test programs built:"
    find testProgs -name "*Server" -o -name "*Client" 2>/dev/null | head -10
    echo ""
    echo "Install locations:"
    echo "  Libraries: ${SCRIPT_DIR}/lib/"
    echo "  Headers: ${SCRIPT_DIR}/include/"
    echo ""
    
    # Copy libraries and headers to parent directories
    echo "Installing libraries and headers..."
    mkdir -p "${SCRIPT_DIR}/lib"
    mkdir -p "${SCRIPT_DIR}/include"
    
    # Copy libraries
    for lib_dir in liveMedia groupsock UsageEnvironment BasicUsageEnvironment; do
        if [ -f "${lib_dir}/lib${lib_dir}.a" ]; then
            cp "${lib_dir}/lib${lib_dir}.a" "${SCRIPT_DIR}/lib/"
            echo "  Copied lib${lib_dir}.a"
        fi
    done
    
    # Copy headers (remove existing read-only files first to avoid permission errors)
    echo "  Removing existing read-only headers..."
    rm -rf "${SCRIPT_DIR}/include/"*
    
    for inc_dir in liveMedia groupsock UsageEnvironment BasicUsageEnvironment; do
        if [ -d "${inc_dir}/include" ]; then
            cp -r "${inc_dir}/include/"* "${SCRIPT_DIR}/include/"
            echo "  Copied headers from ${inc_dir}/include/"
        fi
    done
    
    # Make headers writable for future updates
    chmod -R u+w "${SCRIPT_DIR}/include/"
    
    echo ""
    echo "Installation complete!"
else
    echo ""
    echo "=========================================="
    echo "  Build failed!"
    echo "=========================================="
    exit 1
fi
