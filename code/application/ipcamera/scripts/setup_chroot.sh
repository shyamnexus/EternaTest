#!/bin/bash
set -e

# Script to set up a chroot environment for running ipcamd
# Usage: ./setup_chroot.sh [aarch64-ca53|aarch64-gnu|x64]
# Example: ./setup_chroot.sh x64            (uses build/x64)
#          ./setup_chroot.sh aarch64-ca53   (uses build/aarch64-ca53)
#          ./setup_chroot.sh                (uses build/x64 - default)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IPCAMERA_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CHROOT_DIR="${IPCAMERA_ROOT}/simulated_root"

# Allow override of build directory via command line argument
if [ -n "$1" ]; then
    BUILD_DIR="${IPCAMERA_ROOT}/build/$1"
else
    # Default to x64
    if [ -d "${IPCAMERA_ROOT}/build/x64" ]; then
        BUILD_DIR="${IPCAMERA_ROOT}/build/x64"
        echo "Using x64 build directory (Ubuntu native - default)"
    elif [ -d "${IPCAMERA_ROOT}/build/aarch64-ca53" ]; then
        BUILD_DIR="${IPCAMERA_ROOT}/build/aarch64-ca53"
        echo "Using aarch64-ca53 build directory (ARM CA53)"
    elif [ -d "${IPCAMERA_ROOT}/build/aarch64-gnu" ]; then
        BUILD_DIR="${IPCAMERA_ROOT}/build/aarch64-gnu"
        echo "Using aarch64-gnu build directory (ARM generic)"
    else
        BUILD_DIR="${IPCAMERA_ROOT}/build/x64"
        echo "Using default build directory: x64"
    fi
fi

echo "Setting up chroot environment in: ${CHROOT_DIR}"
echo "Using build directory: ${BUILD_DIR}"

# Create directory structure
echo "Creating directory structure..."
mkdir -p "${CHROOT_DIR}"/{usr/bin,usr/lib,usr/share,lib,lib64,etc/ipcamera,var/log,var/run,tmp,dev,proc,sys}
mkdir -p "${CHROOT_DIR}/tmp/Rockchip"

# Copy ipcamd binary
echo "Copying ipcamd binary..."
if [ -f "${BUILD_DIR}/apps/ipcamd/ipcamd" ]; then
    cp "${BUILD_DIR}/apps/ipcamd/ipcamd" "${CHROOT_DIR}/usr/bin/"
    chmod +x "${CHROOT_DIR}/usr/bin/ipcamd"
else
    echo "ERROR: ipcamd binary not found at ${BUILD_DIR}/apps/ipcamd/ipcamd"
    echo "Please build the project first: cd build/system && make -j\$(nproc)"
    exit 1
fi

# Copy configuration files
echo "Copying configuration files..."
if [ -d "${BUILD_DIR}/configs/config.factory.d" ]; then
    mkdir -p "${CHROOT_DIR}/etc/ipcamera/config.factory.d"
    cp -r "${BUILD_DIR}/configs/config.factory.d"/* "${CHROOT_DIR}/etc/ipcamera/config.factory.d/"
    echo "  ✓ Copied modular factory configs"
else
    echo "WARNING: Modular config files not found at ${BUILD_DIR}/configs/config.factory.d"
fi

# Create empty user config directory
mkdir -p "${CHROOT_DIR}/etc/ipcamera/config.d"
echo "  ✓ Created user config directory"

# Copy module libraries
echo "Copying module libraries..."
if [ -d "${BUILD_DIR}/modules" ]; then
    find "${BUILD_DIR}/modules" -name "*.so*" -exec cp -a {} "${CHROOT_DIR}/usr/lib/" \;
else
    echo "WARNING: Module libraries not found"
fi

# Copy system libraries
echo "Copying system libraries..."
if [ -d "/lib/x86_64-linux-gnu" ]; then
    cp -a /lib/x86_64-linux-gnu "${CHROOT_DIR}/lib/"
else
    echo "WARNING: /lib/x86_64-linux-gnu not found"
fi

# Copy dynamic linker
echo "Copying dynamic linker..."
if [ -f "/lib64/ld-linux-x86-64.so.2" ]; then
    cp /lib64/ld-linux-x86-64.so.2 "${CHROOT_DIR}/lib64/"
else
    echo "WARNING: Dynamic linker not found at /lib64/ld-linux-x86-64.so.2"
fi

# Set permissions
echo "Setting permissions..."
chmod 777 "${CHROOT_DIR}/tmp"
chmod 777 "${CHROOT_DIR}/var/log"
chmod 777 "${CHROOT_DIR}/var/run"
chmod 777 "${CHROOT_DIR}/tmp/Rockchip"

# Update config paths if needed (use /tmp for databases in chroot)
echo "Updating configuration paths for chroot..."
if [ -f "${CHROOT_DIR}/etc/ipcamera/storage.yaml" ]; then
    # Check if paths need updating
    if grep -q "/home/fatninja" "${CHROOT_DIR}/etc/ipcamera/storage.yaml" 2>/dev/null; then
        echo "Updating storage.yaml database paths to /tmp..."
        sed -i 's|path: "/home/fatninja.*users.db"|path: "/tmp/users.db"|g' "${CHROOT_DIR}/etc/ipcamera/storage.yaml"
        sed -i 's|path: "/home/fatninja.*recordings.db"|path: "/tmp/recordings.db"|g' "${CHROOT_DIR}/etc/ipcamera/storage.yaml"
    fi
fi

echo ""
echo "==================================================================="
echo "Chroot environment setup complete!"
echo "==================================================================="
echo ""
echo "To run ipcamd in chroot:"
echo "  sudo chroot ${CHROOT_DIR} /usr/bin/ipcamd -c /etc/ipcamera"
echo ""
echo "To run in background:"
echo "  sudo chroot ${CHROOT_DIR} /usr/bin/ipcamd -c /etc/ipcamera 2>&1 &"
echo ""
echo "To clean up:"
echo "  rm -rf ${CHROOT_DIR}"
echo ""
echo "Database locations inside chroot:"
echo "  /tmp/users.db (unified user management)"
echo "  /tmp/recordings.db (or /usr/share/recordings.db if config uses it)"
echo ""
echo "Default user: admin / admin"
echo "==================================================================="
