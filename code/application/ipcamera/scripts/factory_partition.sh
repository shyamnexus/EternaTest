#!/bin/sh
#
# Factory Partition Utility
# Manages device-specific data in the factory partition
#
# Copyright (C) 2026 HON-BA
# SPDX-License-Identifier: GPL-2.0+
#
# Factory partition layout (from nvt-storage-partition.dtsi):
#   Offset: 0x7FC0000 (133693440 bytes from flash start)
#   Size:   0x40000 (256KB = 262144 bytes)
#   MTD:    /dev/mtd11 (factory)
#
# Data structure (224 bytes):
#   Offset  Size  Field
#   0       4     Magic (0x46414354 = "FACT")
#   4       4     Version (1)
#   8       6     MAC eth0
#   14      6     MAC eth1
#   20      6     MAC WiFi
#   26      32    Serial number (null-terminated)
#   58      32    Model name (null-terminated)
#   90      4     Manufacturing date (YYYYMMDD)
#   94      2     Hardware revision
#   96      124   Reserved
#   220     4     CRC32 checksum

set -e

# Factory partition MTD device
FACTORY_MTD="/dev/mtd11"
FACTORY_MTD_BLOCK="/dev/mtdblock11"
FACTORY_MTDCHAR="${FACTORY_MTD}"

# Alternative: Find partition by name
find_factory_mtd() {
    local mtd_num
    mtd_num=$(grep -w "factory" /proc/mtd | cut -d: -f1 | sed 's/mtd//')
    if [ -n "$mtd_num" ]; then
        FACTORY_MTD="/dev/mtd${mtd_num}"
        FACTORY_MTD_BLOCK="/dev/mtdblock${mtd_num}"
        FACTORY_MTDCHAR="/dev/mtd${mtd_num}"
        return 0
    fi
    return 1
}

# Data structure offsets
MAGIC_OFFSET=0
VERSION_OFFSET=4
MAC_ETH0_OFFSET=8
MAC_ETH1_OFFSET=14
MAC_WIFI_OFFSET=20
SERIAL_OFFSET=26
MODEL_OFFSET=58
MFGDATE_OFFSET=90
HWREV_OFFSET=94
CRC_OFFSET=220

# Constants
FACTORY_MAGIC="FACT"
FACTORY_VERSION=1
DATA_SIZE=224

# Temporary file for operations
TMPFILE="/tmp/factory_data.bin"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

# Calculate CRC32 of data (first 220 bytes)
calc_crc32() {
    local file="$1"
    # Extract first 220 bytes and calculate CRC32
    dd if="$file" bs=1 count=220 2>/dev/null | cksum | awk '{print $1}'
}

# Read factory data to temp file
read_factory_data() {
    find_factory_mtd || {
        log_error "Factory partition not found!"
        return 1
    }
    
    if [ ! -c "$FACTORY_MTDCHAR" ]; then
        log_error "Factory MTD device $FACTORY_MTDCHAR not found!"
        return 1
    fi
    
    # Read factory data (first 224 bytes)
    dd if="$FACTORY_MTDCHAR" of="$TMPFILE" bs=1 count=$DATA_SIZE 2>/dev/null
    
    # Verify magic
    local magic
    magic=$(dd if="$TMPFILE" bs=1 skip=$MAGIC_OFFSET count=4 2>/dev/null | cat)
    if [ "$magic" != "$FACTORY_MAGIC" ]; then
        log_warn "Factory partition not initialized (magic: '$magic')"
        return 2
    fi
    
    return 0
}

# Write factory data from temp file
write_factory_data() {
    find_factory_mtd || {
        log_error "Factory partition not found!"
        return 1
    }
    
    # Erase factory partition
    log_info "Erasing factory partition..."
    # Try flash_erase first, then flash_eraseall as fallback
    if command -v flash_erase >/dev/null 2>&1; then
        flash_erase "$FACTORY_MTDCHAR" 0 0 2>/dev/null || {
            log_warn "flash_erase failed, trying flash_eraseall..."
            flash_eraseall "$FACTORY_MTDCHAR" 2>/dev/null || {
                log_error "Failed to erase factory partition!"
                return 1
            }
        }
    elif command -v flash_eraseall >/dev/null 2>&1; then
        flash_eraseall "$FACTORY_MTDCHAR" 2>/dev/null || {
            log_error "Failed to erase factory partition!"
            return 1
        }
    else
        log_error "No flash erase tool found (flash_erase or flash_eraseall)!"
        return 1
    fi
    
    # Pad data to page size (2048 bytes) for NAND alignment
    local PAGE_SIZE=2048
    local PADDED_FILE="/tmp/factory_data_padded.bin"
    dd if=/dev/zero of="$PADDED_FILE" bs=$PAGE_SIZE count=1 2>/dev/null
    dd if="$TMPFILE" of="$PADDED_FILE" bs=1 count=$DATA_SIZE conv=notrunc 2>/dev/null
    
    # Write data using block device for proper NAND handling
    log_info "Writing factory data..."
    dd if="$PADDED_FILE" of="$FACTORY_MTD_BLOCK" bs=$PAGE_SIZE count=1 2>/dev/null || {
        log_error "Failed to write factory data!"
        rm -f "$PADDED_FILE"
        return 1
    }
    
    rm -f "$PADDED_FILE"
    sync
    
    log_info "Factory data written successfully!"
    return 0
}

# Initialize factory data with default values
init_factory_data() {
    # Create empty data file
    dd if=/dev/zero of="$TMPFILE" bs=1 count=$DATA_SIZE 2>/dev/null
    
    # Write magic "FACT"
    printf "FACT" | dd of="$TMPFILE" bs=1 seek=$MAGIC_OFFSET conv=notrunc 2>/dev/null
    
    # Write version (little-endian uint32)
    printf '\x01\x00\x00\x00' | dd of="$TMPFILE" bs=1 seek=$VERSION_OFFSET conv=notrunc 2>/dev/null
    
    # Calculate and write CRC32
    local crc
    crc=$(calc_crc32 "$TMPFILE")
    printf '%08x' "$crc" | xxd -r -p | dd of="$TMPFILE" bs=1 seek=$CRC_OFFSET conv=notrunc 2>/dev/null
}

# Get MAC address from factory data
get_mac() {
    local iface="$1"
    local offset
    
    case "$iface" in
        eth0|0) offset=$MAC_ETH0_OFFSET ;;
        eth1|1) offset=$MAC_ETH1_OFFSET ;;
        wifi|wlan0) offset=$MAC_WIFI_OFFSET ;;
        *) log_error "Unknown interface: $iface"; return 1 ;;
    esac
    
    read_factory_data || return 1
    
    # Read 6 bytes and format as MAC
    dd if="$TMPFILE" bs=1 skip=$offset count=6 2>/dev/null | \
        xxd -p | sed 's/\(..\)/\1:/g' | sed 's/:$//'
}

# Set MAC address in factory data
set_mac() {
    local iface="$1"
    local mac="$2"
    local offset
    
    # Validate MAC format
    if ! echo "$mac" | grep -qE '^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$'; then
        log_error "Invalid MAC format: $mac (expected XX:XX:XX:XX:XX:XX)"
        return 1
    fi
    
    case "$iface" in
        eth0|0) offset=$MAC_ETH0_OFFSET ;;
        eth1|1) offset=$MAC_ETH1_OFFSET ;;
        wifi|wlan0) offset=$MAC_WIFI_OFFSET ;;
        *) log_error "Unknown interface: $iface"; return 1 ;;
    esac
    
    # Read existing data or initialize
    read_factory_data
    local ret=$?
    if [ $ret -eq 2 ]; then
        log_info "Initializing factory data..."
        init_factory_data
    elif [ $ret -ne 0 ]; then
        return 1
    fi
    
    # Convert MAC to binary and write
    echo "$mac" | tr -d ':' | xxd -r -p | \
        dd of="$TMPFILE" bs=1 seek=$offset conv=notrunc 2>/dev/null
    
    # Recalculate CRC
    local crc
    crc=$(calc_crc32 "$TMPFILE")
    printf '%08x' "$crc" | xxd -r -p | \
        dd of="$TMPFILE" bs=1 seek=$CRC_OFFSET conv=notrunc 2>/dev/null
    
    # Write to flash
    write_factory_data
}

# Get serial number
get_serial() {
    read_factory_data || return 1
    dd if="$TMPFILE" bs=1 skip=$SERIAL_OFFSET count=32 2>/dev/null | tr -d '\0'
}

# Set serial number
set_serial() {
    local serial="$1"
    
    if [ ${#serial} -gt 31 ]; then
        log_error "Serial number too long (max 31 chars)"
        return 1
    fi
    
    # Read existing data or initialize
    read_factory_data
    local ret=$?
    if [ $ret -eq 2 ]; then
        log_info "Initializing factory data..."
        init_factory_data
    elif [ $ret -ne 0 ]; then
        return 1
    fi
    
    # Clear serial field and write new value
    dd if=/dev/zero bs=1 count=32 2>/dev/null | \
        dd of="$TMPFILE" bs=1 seek=$SERIAL_OFFSET conv=notrunc 2>/dev/null
    printf '%s' "$serial" | \
        dd of="$TMPFILE" bs=1 seek=$SERIAL_OFFSET conv=notrunc 2>/dev/null
    
    # Recalculate CRC
    local crc
    crc=$(calc_crc32 "$TMPFILE")
    printf '%08x' "$crc" | xxd -r -p | \
        dd of="$TMPFILE" bs=1 seek=$CRC_OFFSET conv=notrunc 2>/dev/null
    
    write_factory_data
}

# Get model name
get_model() {
    read_factory_data || return 1
    dd if="$TMPFILE" bs=1 skip=$MODEL_OFFSET count=32 2>/dev/null | tr -d '\0'
}

# Set model name
set_model() {
    local model="$1"
    
    if [ ${#model} -gt 31 ]; then
        log_error "Model name too long (max 31 chars)"
        return 1
    fi
    
    # Read existing data or initialize
    read_factory_data
    local ret=$?
    if [ $ret -eq 2 ]; then
        log_info "Initializing factory data..."
        init_factory_data
    elif [ $ret -ne 0 ]; then
        return 1
    fi
    
    # Clear model field and write new value
    dd if=/dev/zero bs=1 count=32 2>/dev/null | \
        dd of="$TMPFILE" bs=1 seek=$MODEL_OFFSET conv=notrunc 2>/dev/null
    printf '%s' "$model" | \
        dd of="$TMPFILE" bs=1 seek=$MODEL_OFFSET conv=notrunc 2>/dev/null
    
    # Recalculate CRC
    local crc
    crc=$(calc_crc32 "$TMPFILE")
    printf '%08x' "$crc" | xxd -r -p | \
        dd of="$TMPFILE" bs=1 seek=$CRC_OFFSET conv=notrunc 2>/dev/null
    
    write_factory_data
}

# Show all factory data
show_all() {
    echo "=========================================="
    echo "       Factory Partition Data"
    echo "=========================================="
    
    if ! read_factory_data; then
        if [ $? -eq 2 ]; then
            echo "Status: NOT INITIALIZED"
            echo ""
            echo "Use 'factory_partition.sh init' to initialize"
        fi
        return
    fi
    
    echo "Status: VALID"
    echo ""
    echo "MAC Addresses:"
    printf "  eth0:  %s\n" "$(get_mac eth0)"
    printf "  eth1:  %s\n" "$(get_mac eth1)"
    printf "  wifi:  %s\n" "$(get_mac wifi)"
    echo ""
    echo "Device Info:"
    printf "  Serial: %s\n" "$(get_serial)"
    printf "  Model:  %s\n" "$(get_model)"
    echo ""
    echo "=========================================="
}

# Initialize factory partition with test data
init() {
    log_info "Initializing factory partition..."
    init_factory_data
    write_factory_data
    log_info "Factory partition initialized!"
}

# Erase factory partition (dangerous!)
erase() {
    echo "WARNING: This will erase all factory data!"
    echo "Type 'YES' to confirm: "
    read -r confirm
    
    if [ "$confirm" != "YES" ]; then
        log_error "Aborted."
        return 1
    fi
    
    find_factory_mtd || {
        log_error "Factory partition not found!"
        return 1
    }
    
    log_info "Erasing factory partition..."
    flash_erase "$FACTORY_MTDCHAR" 0 0 2>/dev/null
    log_info "Factory partition erased!"
}

# Export factory data to file
export_data() {
    local outfile="${1:-/tmp/factory_export.bin}"
    
    read_factory_data || return 1
    
    cp "$TMPFILE" "$outfile"
    log_info "Factory data exported to: $outfile"
}

# Import factory data from file
import_data() {
    local infile="$1"
    
    if [ ! -f "$infile" ]; then
        log_error "File not found: $infile"
        return 1
    fi
    
    # Verify file magic
    local magic
    magic=$(dd if="$infile" bs=1 count=4 2>/dev/null | cat)
    if [ "$magic" != "$FACTORY_MAGIC" ]; then
        log_error "Invalid factory data file (bad magic)"
        return 1
    fi
    
    cp "$infile" "$TMPFILE"
    write_factory_data
    log_info "Factory data imported successfully!"
}

# Verify factory data integrity
verify() {
    if read_factory_data; then
        log_info "Factory data is VALID"
        return 0
    else
        log_error "Factory data is INVALID or not initialized"
        return 1
    fi
}

# Usage help
usage() {
    cat << EOF
Factory Partition Utility

Usage: $0 <command> [arguments]

Commands:
  show                    Show all factory data
  init                    Initialize factory partition with defaults
  verify                  Verify factory data integrity
  
  get-mac <iface>         Get MAC address (eth0, eth1, wifi)
  set-mac <iface> <MAC>   Set MAC address (format: XX:XX:XX:XX:XX:XX)
  
  get-serial              Get serial number
  set-serial <serial>     Set serial number (max 31 chars)
  
  get-model               Get model name
  set-model <model>       Set model name (max 31 chars)
  
  export [file]           Export factory data to file
  import <file>           Import factory data from file
  
  erase                   Erase factory partition (requires confirmation)

Factory Partition Info:
  MTD Device: /dev/mtd11 (factory)
  Offset:     0x7FC0000 (133693440)
  Size:       256KB (262144 bytes)

Examples:
  $0 show
  $0 set-mac eth0 00:11:22:33:44:55
  $0 set-serial "CAM-2026-001234"
  $0 set-model "ETERNA-IP-001"
  $0 export /backup/factory.bin

EOF
}

# Main
case "$1" in
    show)
        show_all
        ;;
    init)
        init
        ;;
    verify)
        verify
        ;;
    get-mac)
        get_mac "$2"
        ;;
    set-mac)
        set_mac "$2" "$3"
        ;;
    get-serial)
        get_serial
        ;;
    set-serial)
        set_serial "$2"
        ;;
    get-model)
        get_model
        ;;
    set-model)
        set_model "$2"
        ;;
    export)
        export_data "$2"
        ;;
    import)
        import_data "$2"
        ;;
    erase)
        erase
        ;;
    *)
        usage
        ;;
esac

# Cleanup
rm -f "$TMPFILE" 2>/dev/null

exit 0
