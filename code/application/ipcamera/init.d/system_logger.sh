#!/bin/sh
#
# system_logger.sh - Early-boot dmesg capture (one-shot)
#
# All periodic system data collection (dmesg, thermal, sysdiag, app log mirror)
# and destination management (SD card, flash) is now handled by the in-app
# SystemLogger component (modules/platform/src/system_logger.cpp).
#
# This script only runs at boot to capture kernel messages that appear before
# ipcamd starts. It writes to flash only (SD may not be mounted yet).
#
# Usage: ./system_logger.sh start
#

FLASH_LOG_DIR="/mnt/app/logs"
BOOT_LOG="$FLASH_LOG_DIR/boot_dmesg.log"
MAX_BOOT_LOG_KB=256

do_boot_capture() {
    mkdir -p "$FLASH_LOG_DIR" 2>/dev/null

    # Rotate boot log if it's too large
    if [ -f "$BOOT_LOG" ]; then
        size_kb=$(du -k "$BOOT_LOG" 2>/dev/null | cut -f1)
        if [ "${size_kb:-0}" -gt "$MAX_BOOT_LOG_KB" ]; then
            # Keep last half
            tail -n 500 "$BOOT_LOG" > "${BOOT_LOG}.tmp" 2>/dev/null
            mv "${BOOT_LOG}.tmp" "$BOOT_LOG" 2>/dev/null
        fi
    fi

    # Capture boot dmesg with timestamp header
    {
        echo ""
        echo "=== Boot dmesg captured at $(date '+%Y-%m-%d %H:%M:%S') ==="
        dmesg 2>/dev/null
        echo "=== End boot dmesg ==="
        echo ""
    } >> "$BOOT_LOG"

    echo "Boot dmesg captured to $BOOT_LOG"
}

case "${1:-start}" in
    start)
        do_boot_capture
        ;;
    stop|restart|status)
        # No-op — this is a one-shot script, nothing to stop
        echo "system_logger.sh is a one-shot boot capture (periodic collection handled by ipcamd SystemLogger)"
        ;;
    *)
        echo "Usage: $0 {start|stop|restart|status}"
        exit 1
        ;;
esac

exit 0
