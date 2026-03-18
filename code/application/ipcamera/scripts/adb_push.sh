#!/bin/bash
#
# ADB Push Script for IPCamera Application
# Pushes ipcamd binary, libraries, configs, and init scripts to device
#
# Device Storage Architecture (after S20ipcamera_config runs):
#   /mnt/app/ipcamera/     - NAND persistent storage
#     ├── configs/         - All config files
#     │   ├── config.json  - User settings
#     │   ├── config.d/    - User config overrides
#     │   └── config.factory.d/ - Factory defaults
#     ├── ssl/             - SSL certificates (persistent)
#     └── db/              - Auth databases
#   /etc/ipcamera          - Symlink to /mnt/app/ipcamera
#   /etc/nginx/ssl/        - Symlinks to /mnt/app/ipcamera/ssl/*
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Build configuration - SDK uses aarch64-ca53 directory
BUILD_DIR="${PROJECT_ROOT}/build/aarch64-ca53"
BUILD_TYPE="${BUILD_TYPE:-aarch64-ca53}"

# Device paths - MUST match SDK Makefile (code/application/Makefile)
# SDK installs to: rootfs/bin/ipcamd, rootfs/lib/libipcam_*.so
DEVICE_BIN_DIR="/bin"
DEVICE_LIB_DIR="/lib"
DEVICE_CONFIG_DIR="/etc"
DEVICE_PERSISTENT_DIR="/mnt/app/ipcamera"
DEVICE_CONFIG_SYMLINK="/etc/ipcamera"
DEVICE_CONFIG_BASE_DIR="${DEVICE_PERSISTENT_DIR}/configs"
DEVICE_SSL_DIR="${DEVICE_PERSISTENT_DIR}/ssl"
DEVICE_NGINX_SSL_DIR="/etc/nginx/ssl"
DEVICE_INIT_DIR="/etc/init.d"
DEVICE_WWW_DIR="/var/www"

# ADB device selection
ADB_DEVICE=""

# Source files - paths match SDK Makefile structure
IPCAMD_BIN="${BUILD_DIR}/apps/ipcamd/ipcamd"
MODULES_DIR="${BUILD_DIR}/modules"
CONFIGS_DIR="${PROJECT_ROOT}/configs"
INIT_SCRIPTS_DIR="${PROJECT_ROOT}/init.d"
WEB_UI_DIST_DIR="${PROJECT_ROOT}/../web/www"

# Module libraries
MODULE_LIBS=(
    "ai/libipcam_ai.so"
    "config/libipcam_config.so"
    "events/libipcam_events.so"
    "media/libipcam_media.so"
    "networking/libipcam_networking.so"
    "onvif/libipcam_onvif.so"
    "platform/libipcam_platform.so"
    "recording/libipcam_recording.so"
    "storage/libipcam_storage.so"
    "streaming/libipcam_streaming.so"
    "upgrade/libipcam_upgrade.so"
    "utils/libipcam_utils.so"
    "webserver/libipcam_webserver.so"
)

# SDK libraries (from code/lib/output or synced sdk_libs directory)
SDK_LIB_DIR="${PROJECT_ROOT}/../../lib/output"
SDK_LIB_DIR_ALT="${PROJECT_ROOT}/sdk_libs"  # Alternative path when synced via deploy_via_fatninja.sh
SDK_LIBS=(
    "libaec.so"
    "libagc.so"
    "libaudfilt.so"
)

# Init scripts
INIT_SCRIPTS=(
    "S30ipcamd"
    "S26ntpd"
)

# OSD Assets directory
ASSETS_OSD_DIR="${PROJECT_ROOT}/assets/osd"
DEVICE_OSD_DIR="/etc/ipcamera/assets/osd"

# Function to print colored messages
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if adb is available
check_adb() {
    if ! command -v adb &> /dev/null; then
        print_error "adb command not found. Please install Android Debug Bridge."
        exit 1
    fi
}

# Function to check if device is connected
check_device() {
    local devices=$(adb devices | grep -v "List of devices" | grep "device$" | wc -l)
    if [ "$devices" -eq 0 ]; then
        print_error "No device connected. Please connect a device via adb."
        exit 1
    elif [ "$devices" -gt 1 ]; then
        print_warn "Multiple devices connected. Selecting first IP-based device..."
        # Prefer IP-based connection (192.168.x.x) over hostname
        local ip_device=$(adb devices | grep "device$" | grep -E "^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:" | head -1 | awk '{print $1}')
        if [ -n "$ip_device" ]; then
            ADB_DEVICE="-s $ip_device"
            print_info "Selected device: $ip_device"
        else
            # Fallback to first device
            local first_device=$(adb devices | grep "device$" | head -1 | awk '{print $1}')
            ADB_DEVICE="-s $first_device"
            print_info "Selected device: $first_device"
        fi
    else
        local device=$(adb devices | grep 'device$' | head -1 | awk '{print $1}')
        ADB_DEVICE="-s $device"
        print_info "Device connected: $device"
    fi
}

# Wrapper for adb commands with device selection
adb_cmd() {
    adb $ADB_DEVICE "$@"
}

# Function to check if build artifacts exist
check_build() {
    if [ ! -f "${IPCAMD_BIN}" ]; then
        print_error "ipcamd binary not found at: ${IPCAMD_BIN}"
        print_error "Please build the project first."
        exit 1
    fi
    
    print_info "Build directory: ${BUILD_DIR}"
    print_info "ipcamd binary: ${IPCAMD_BIN}"
}

# Function to remount device filesystem as read-write
remount_rw() {
    print_info "Remounting device filesystem as read-write..."
    adb_cmd shell "mount -o remount,rw /" 2>/dev/null || true
    adb_cmd root 2>/dev/null || true
    sleep 1
    adb_cmd shell "mount -o remount,rw /" 2>/dev/null || true
}

# Function to create device directories
create_device_dirs() {
    print_info "Creating device directories..."
    adb_cmd shell "mkdir -p ${DEVICE_BIN_DIR}" || true
    adb_cmd shell "mkdir -p ${DEVICE_LIB_DIR}" || true
    adb_cmd shell "mkdir -p ${DEVICE_PERSISTENT_DIR}/configs/config.d" || true
    adb_cmd shell "mkdir -p ${DEVICE_PERSISTENT_DIR}/configs/config.factory.d" || true
    adb_cmd shell "mkdir -p ${DEVICE_SSL_DIR}" || true
    adb_cmd shell "mkdir -p ${DEVICE_NGINX_SSL_DIR}" || true
    adb_cmd shell "mkdir -p ${DEVICE_INIT_DIR}" || true
    adb_cmd shell "mkdir -p ${DEVICE_WWW_DIR}" || true
    adb_cmd shell "mkdir -p ${DEVICE_PERSISTENT_DIR}/assets/osd" || true
    
    # Ensure symlink exists: /etc/ipcamera -> /mnt/app/ipcamera
    adb_cmd shell "if [ ! -L ${DEVICE_CONFIG_SYMLINK} ]; then rm -rf ${DEVICE_CONFIG_SYMLINK} 2>/dev/null; ln -sf ${DEVICE_PERSISTENT_DIR} ${DEVICE_CONFIG_SYMLINK}; fi" || true
    
    # Ensure nginx SSL symlinks
    adb_cmd shell "ln -sf ${DEVICE_SSL_DIR}/Server.crt ${DEVICE_NGINX_SSL_DIR}/Server.crt 2>/dev/null" || true
    adb_cmd shell "ln -sf ${DEVICE_SSL_DIR}/Server.key ${DEVICE_NGINX_SSL_DIR}/Server.key 2>/dev/null" || true
}

# Function to stop ipcamd service
stop_service() {
    print_info "Stopping ipcamd service..."
    adb_cmd shell "killall ipcamd" 2>/dev/null || true
    adb_cmd shell "${DEVICE_INIT_DIR}/S30ipcamd stop" 2>/dev/null || true
    sleep 1
}

# Function to push ipcamd binary
push_binary() {
    print_info "Pushing ipcamd binary..."
    adb_cmd push "${IPCAMD_BIN}" "${DEVICE_BIN_DIR}/ipcamd"
    adb_cmd shell "chmod +x ${DEVICE_BIN_DIR}/ipcamd"
    print_info "✓ ipcamd binary pushed"
}

# Function to push module libraries
push_libraries() {
    print_info "Pushing module libraries..."
    local count=0
    for lib in "${MODULE_LIBS[@]}"; do
        local lib_path="${MODULES_DIR}/${lib}"
        if [ -f "${lib_path}" ]; then
            local lib_name=$(basename "${lib}")
            adb_cmd push "${lib_path}" "${DEVICE_LIB_DIR}/${lib_name}"
            count=$((count + 1))
        else
            print_warn "Library not found: ${lib_path}"
        fi
    done
    print_info "✓ Pushed ${count} module libraries"
    
    # Update library cache on device
    print_info "Updating library cache..."
    adb_cmd shell "ldconfig" 2>/dev/null || true
}

# Function to push configuration files
push_configs() {
    print_info "Pushing configuration files..."
    
    # Push factory configs to persistent storage config.factory.d
    if [ -d "${CONFIGS_DIR}/config.factory.d" ]; then
        adb_cmd push "${CONFIGS_DIR}/config.factory.d/." "${DEVICE_PERSISTENT_DIR}/configs/config.factory.d/"
        local config_count=$(find "${CONFIGS_DIR}/config.factory.d" -type f -name "*.json" | wc -l)
        print_info "✓ Pushed ${config_count} factory config files"
    fi
    
    # Push deployment override configs (config.d/) — these override factory defaults
    if [ -d "${CONFIGS_DIR}/config.d" ]; then
        local override_count=$(find "${CONFIGS_DIR}/config.d" -type f -name "*.json" | wc -l)
        if [ "${override_count}" -gt 0 ]; then
            adb_cmd push "${CONFIGS_DIR}/config.d/." "${DEVICE_PERSISTENT_DIR}/configs/config.d/"
            print_info "✓ Pushed ${override_count} deployment override config files (config.d/)"
        fi
    fi
    
    # Remove stale monolithic config.json so updated factory defaults take effect.
    # Save() now writes only user-modified delta, so any old full-dump config.json
    # from a previous firmware version will override new factory values.
    if adb_cmd shell "test -f ${DEVICE_PERSISTENT_DIR}/configs/config.json && echo exists" 2>/dev/null | grep -q "exists"; then
        adb_cmd shell "rm -f ${DEVICE_PERSISTENT_DIR}/configs/config.json"
        print_info "✓ Removed stale config.json (factory defaults will apply on next boot)"
    fi
    
    # Push NTP config to /etc/ntp.conf if exists
    if [ -f "${CONFIGS_DIR}/ntp.conf" ]; then
        adb_cmd push "${CONFIGS_DIR}/ntp.conf" "${DEVICE_CONFIG_DIR}/ntp.conf"
        print_info "✓ Pushed ntp.conf to ${DEVICE_CONFIG_DIR}"
    fi
}

# Function to push init scripts
push_init_scripts() {
    print_info "Pushing init scripts..."
    local count=0
    for script in "${INIT_SCRIPTS[@]}"; do
        local script_path="${INIT_SCRIPTS_DIR}/${script}"
        if [ -f "${script_path}" ]; then
            adb_cmd push "${script_path}" "${DEVICE_INIT_DIR}/${script}"
            adb_cmd shell "chmod +x ${DEVICE_INIT_DIR}/${script}"
            count=$((count + 1))
        else
            print_warn "Init script not found: ${script_path}"
        fi
    done
    print_info "✓ Pushed ${count} init scripts"
}

# Function to push SDK libraries (optional)
push_sdk_libs() {
    local lib_dir=""
    
    # Check primary SDK lib directory
    if [ -d "${SDK_LIB_DIR}" ]; then
        lib_dir="${SDK_LIB_DIR}"
    # Check alternative path (synced via deploy_via_fatninja.sh)
    elif [ -d "${SDK_LIB_DIR_ALT}" ]; then
        lib_dir="${SDK_LIB_DIR_ALT}"
    else
        print_warn "SDK lib directory not found: ${SDK_LIB_DIR} or ${SDK_LIB_DIR_ALT}"
        return
    fi
    
    print_info "Pushing SDK audio libraries from ${lib_dir}..."
    local count=0
    
    for lib in "${SDK_LIBS[@]}"; do
        local lib_path="${lib_dir}/${lib}"
        if [ -f "${lib_path}" ]; then
            adb_cmd push "${lib_path}" "${DEVICE_LIB_DIR}/${lib}"
            count=$((count + 1))
            print_info "✓ Pushed ${lib}"
        else
            print_warn "SDK library not found: ${lib_path}"
        fi
    done
    
    if [ ${count} -gt 0 ]; then
        print_info "✓ Pushed ${count} SDK audio libraries"
        # Update library cache
        adb_cmd shell "ldconfig" 2>/dev/null || true
    fi
}

# Function to push web UI dist files
push_web_ui() {
    if [ ! -d "${WEB_UI_DIST_DIR}" ]; then
        print_warn "Web UI dist directory not found: ${WEB_UI_DIST_DIR}"
        print_warn "Skipping web UI deployment. Build the web UI first if needed."
        return
    fi
    
    print_info "Pushing Web UI files to ${DEVICE_WWW_DIR}..."
    
    # Clear existing web files on device
    adb_cmd shell "rm -rf ${DEVICE_WWW_DIR}/*" 2>/dev/null || true
    
    # Push all web UI files
    adb_cmd push "${WEB_UI_DIST_DIR}/." "${DEVICE_WWW_DIR}/"
    
    # Count files pushed
    local file_count=$(find "${WEB_UI_DIST_DIR}" -type f | wc -l)
    print_info "✓ Pushed ${file_count} Web UI files to ${DEVICE_WWW_DIR}"
    
    # Set appropriate permissions
    adb_cmd shell "chmod -R 755 ${DEVICE_WWW_DIR}" 2>/dev/null || true
}

# Function to push OSD assets (logo, fonts)
push_osd_assets() {
    local ASSETS_OSD_DIR="${PROJECT_ROOT}/assets/osd"
    local DEVICE_OSD_DIR="${DEVICE_PERSISTENT_DIR}/assets/osd"
    
    if [ ! -d "${ASSETS_OSD_DIR}" ]; then
        print_warn "OSD assets directory not found: ${ASSETS_OSD_DIR}"
        return
    fi
    
    print_info "Pushing OSD assets to ${DEVICE_OSD_DIR}..."
    
    # Push logo.bin
    if [ -f "${ASSETS_OSD_DIR}/logo.bin" ]; then
        adb_cmd push "${ASSETS_OSD_DIR}/logo.bin" "${DEVICE_OSD_DIR}/logo.bin"
        print_info "✓ Pushed logo.bin"
    else
        print_warn "logo.bin not found in ${ASSETS_OSD_DIR}"
    fi
    
    # Push font file
    if [ -f "${ASSETS_OSD_DIR}/arial.ttf" ]; then
        adb_cmd push "${ASSETS_OSD_DIR}/arial.ttf" "${DEVICE_OSD_DIR}/arial.ttf"
        print_info "✓ Pushed arial.ttf"
    else
        print_warn "arial.ttf not found in ${ASSETS_OSD_DIR}"
    fi
    
    # Push original logo PNG for reference
    if [ -f "${ASSETS_OSD_DIR}/IMPACT-by-honeywell.png" ]; then
        adb_cmd push "${ASSETS_OSD_DIR}/IMPACT-by-honeywell.png" "${DEVICE_OSD_DIR}/IMPACT-by-honeywell.png"
        print_info "✓ Pushed IMPACT-by-honeywell.png"
    fi
    
    print_info "OSD assets deployment complete"
}

# Function to regenerate SSL certificates
regen_ssl() {
    print_info "Removing old SSL certificates to trigger regeneration..."
    adb_cmd shell "rm -f ${DEVICE_SSL_DIR}/Server.crt ${DEVICE_SSL_DIR}/Server.key" 2>/dev/null || true
    adb_cmd shell "rm -f ${DEVICE_NGINX_SSL_DIR}/Server.crt ${DEVICE_NGINX_SSL_DIR}/Server.key" 2>/dev/null || true
    print_info "✓ SSL certificates removed, will regenerate on next ipcamd start"
}

# Function to verify installation
verify_installation() {
    print_info "Verifying installation..."
    
    # Check if binary exists and is executable
    if adb_cmd shell "test -x ${DEVICE_BIN_DIR}/ipcamd && echo exists" | grep -q "exists"; then
        print_info "✓ ipcamd binary is installed and executable"
    else
        print_error "✗ ipcamd binary verification failed"
        return 1
    fi
    
    # Check if libraries exist
    local lib_ok=true
    for lib in "${MODULE_LIBS[@]}"; do
        local lib_name=$(basename "${lib}")
        if ! adb_cmd shell "test -f ${DEVICE_LIB_DIR}/${lib_name} && echo exists" | grep -q "exists"; then
            print_warn "✗ Library not found: ${lib_name}"
            lib_ok=false
        fi
    done
    
    if [ "$lib_ok" = true ]; then
        print_info "✓ All module libraries verified"
    fi
    
    # Check symlink setup
    if adb_cmd shell "test -L ${DEVICE_CONFIG_SYMLINK} && echo exists" | grep -q "exists"; then
        print_info "✓ Config symlink verified: ${DEVICE_CONFIG_SYMLINK} -> ${DEVICE_PERSISTENT_DIR}"
    else
        print_warn "✗ Config symlink not setup: ${DEVICE_CONFIG_SYMLINK}"
    fi
    
    # Check ipcamd version
    print_info "ipcamd version:"
    adb_cmd shell "${DEVICE_BIN_DIR}/ipcamd --version" 2>/dev/null || print_warn "Could not get version"
}

# Function to start service
start_service() {
    print_info "Starting ipcamd service..."
    # Use start-stop-daemon to properly daemonize the process
    # This survives ADB shell exit unlike simple backgrounding
    adb_cmd shell "start-stop-daemon -S -b -m -p /run/ipcamd.pid -x ${DEVICE_BIN_DIR}/ipcamd"
    sleep 2
    
    # Check if service is running
    local pid=$(adb_cmd shell "pidof ipcamd" 2>/dev/null | tr -d '\r\n')
    if [ -n "$pid" ] && [ "$pid" != "" ]; then
        print_info "✓ ipcamd service started successfully (PID: $pid)"
    else
        print_warn "ipcamd service may not be running. Check device logs."
        print_info "Try starting manually via UART: /etc/init.d/S30ipcamd start"
    fi
}

# Function to show logs
show_logs() {
    print_info "Recent logs (press Ctrl+C to stop):"
    echo "----------------------------------------"
    adb_cmd shell "tail -f /var/log/ipcamd.log" 2>/dev/null || \
    print_warn "Could not access /var/log/ipcamd.log"
}

# Main deployment function
deploy() {
    print_info "========================================"
    print_info "IPCamera ADB Deployment Script"
    print_info "========================================"
    
    check_adb
    check_device
    check_build
    
    remount_rw
    stop_service
    create_device_dirs
    
    push_binary
    push_libraries
    push_configs
    push_init_scripts
    push_sdk_libs
    push_web_ui
    push_osd_assets
    
    verify_installation
    
    print_info "========================================"
    print_info "Deployment completed successfully!"
    print_info "========================================"
    print_info "To start the service, run: ./adb_push.sh start"
    print_info "To view logs, run: ./adb_push.sh logs"
}

# Parse command line arguments
case "${1:-deploy}" in
    deploy)
        deploy
        ;;
    push-only)
        check_adb
        check_device
        check_build
        remount_rw
        stop_service
        create_device_dirs
        push_binary
        push_libraries
        push_configs
        push_init_scripts
        push_web_ui
        push_osd_assets
        print_info "Push completed. Service not started."
        ;;
    binary-only)
        check_adb
        check_device
        check_build
        remount_rw
        stop_service
        push_binary
        push_libraries
        print_info "Binary and libraries pushed. Run 'start' to restart service."
        ;;
    web-only)
        check_adb
        check_device
        remount_rw
        create_device_dirs
        push_web_ui
        print_info "Web UI push completed."
        ;;
    ssl-regen)
        check_adb
        check_device
        remount_rw
        regen_ssl
        print_info "SSL certs will regenerate on next ipcamd restart."
        ;;
    start)
        check_adb
        check_device
        start_service
        ;;
    stop)
        check_adb
        check_device
        stop_service
        ;;
    restart)
        check_adb
        check_device
        stop_service
        start_service
        ;;
    logs)
        check_adb
        check_device
        show_logs
        ;;
    verify)
        check_adb
        check_device
        verify_installation
        ;;
    status)
        check_adb
        check_device
        print_info "Device status:"
        adb_cmd shell "pidof ipcamd && echo 'ipcamd: running' || echo 'ipcamd: stopped'"
        adb_cmd shell "cat /proc/uptime | awk '{print \"Uptime: \" int(\$1/3600) \"h \" int(\$1%3600/60) \"m\"}'"
        adb_cmd shell "hostname"
        adb_cmd shell "ip addr show eth0 | grep 'inet '"
        ;;
    *)
        echo "Usage: $0 {deploy|push-only|binary-only|web-only|ssl-regen|start|stop|restart|logs|verify|status}"
        echo ""
        echo "Commands:"
        echo "  deploy      - Full deployment (stop, push, verify)"
        echo "  push-only   - Push all files without starting service"
        echo "  binary-only - Push only ipcamd and libraries (quick update)"
        echo "  web-only    - Push only Web UI files to /var/www"
        echo "  ssl-regen   - Remove SSL certs to trigger regeneration"
        echo "  start       - Start ipcamd service"
        echo "  stop        - Stop ipcamd service"
        echo "  restart     - Restart ipcamd service"
        echo "  logs        - Show ipcamd logs"
        echo "  verify      - Verify installation"
        echo "  status      - Show device status (PID, uptime, IP)"
        exit 1
        ;;
esac
