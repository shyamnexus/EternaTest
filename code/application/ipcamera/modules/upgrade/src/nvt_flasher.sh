#!/bin/sh
# nvt_flasher.sh - Firmware flasher launcher script
#
# This script is launched in background by the main application.
# All services are already stopped by the main app before this script runs.
# It waits briefly for the main app to exit, then runs the flasher binary.
#
# Usage: sh nvt_flasher.sh [--no-reboot]

FLASHER_BIN="/tmp/upgrade/nvt_flasher"
LOG_FILE="/tmp/upgrade/flasher.log"

log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $*" >> "$LOG_FILE"
    echo "[FLASHER_SH] $*"
}

log "=== Flasher script started ==="
log "PID: $$"
log "Args: $*"

# Brief wait for main app to fully exit (services already stopped by main app)
log "Waiting 2 seconds for main app to exit..."
sleep 2

# Double-check main app is dead
if pgrep -x "ipcam" > /dev/null 2>&1; then
    log "Main app still running, killing..."
    killall -9 ipcam 2>/dev/null
    sleep 1
fi

# Verify flasher binary exists and is executable
if [ ! -x "$FLASHER_BIN" ]; then
    log "ERROR: Flasher binary not found or not executable: $FLASHER_BIN"
    exit 1
fi

# Verify upgrade info exists
if [ ! -f "/tmp/upgrade/upgradeinfo" ]; then
    log "ERROR: Upgrade info not found"
    exit 1
fi

log "Starting flasher binary..."

# Run the flasher
if [ "$1" = "--no-reboot" ]; then
    "$FLASHER_BIN" -v -n >> "$LOG_FILE" 2>&1
else
    "$FLASHER_BIN" -v >> "$LOG_FILE" 2>&1
fi

RESULT=$?
log "Flasher exited with code: $RESULT"

# If we get here without reboot, something went wrong
if [ $RESULT -ne 0 ]; then
    log "ERROR: Flashing failed!"
    # Don't reboot on failure - leave system in current state
    exit 1
fi

# Reboot should have happened in flasher binary
# If we reach here, try manual reboot
log "Flasher didn't reboot, forcing reboot..."
sync
sync
reboot -f
