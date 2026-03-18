# TFTP Boot Guide for ETERNA IP Camera

This guide explains how to set up and use TFTP boot for firmware updates on the ETERNA IP Camera board.

## Prerequisites

### 1. TFTP Server Setup on Host Machine

#### Install TFTP Server (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y tftpd-hpa
```

#### Configure TFTP Server
```bash
# Create TFTP root directory
sudo mkdir -p /var/lib/tftpboot

# Configure TFTP server settings
sudo tee /etc/default/tftpd-hpa > /dev/null << 'EOF'
# /etc/default/tftpd-hpa
TFTP_USERNAME="tftp"
TFTP_DIRECTORY="/var/lib/tftpboot"
TFTP_ADDRESS=":69"
TFTP_OPTIONS="--secure"
EOF

# Start and enable TFTP server
sudo systemctl restart tftpd-hpa
sudo systemctl enable tftpd-hpa
```

#### Copy Firmware to TFTP Directory
```bash
# Copy the all-in-one firmware binary to TFTP root
sudo cp /path/to/your/sdk/output/packed/FW98538A.bin /var/lib/tftpboot/

# Set appropriate permissions
sudo chmod 644 /var/lib/tftpboot/FW98538A.bin
```

#### Verify TFTP Server Status
```bash
# Check server status
sudo systemctl status tftpd-hpa

# Verify file is accessible
ls -la /var/lib/tftpboot/
```

#### Install TFTP Server (Windows)

**Option 1: Tftpd64 (Recommended)**

1. **Download Tftpd64** from https://pjo2.github.io/tftpd64/
2. **Extract** the ZIP file to a folder (e.g., `C:\tftpd64`)
3. **Run** `tftpd64.exe` as Administrator
4. **Configure** the server:
   - Go to **Settings** > **TFTP** tab
   - Set **Base Directory** to your firmware folder (e.g., `C:\tftpboot`)
   - Enable **TFTP Server**
   - Click **OK** to save
5. **Create the TFTP directory** and copy firmware:
   ```cmd
   mkdir C:\tftpboot
   copy C:\path\to\FW98538A.bin C:\tftpboot\
   ```
6. **Select the correct network interface** from the dropdown in the main window
7. **Verify** the server shows "TFTP Server started" in the status

**Option 2: SolarWinds Free TFTP Server**

1. **Download** from https://www.solarwinds.com/free-tools/free-tftp-server
2. **Install** and run the application
3. **Configure** the root directory in **File** > **Configure** > **TFTP Server**
4. **Copy** `FW98538A.bin` to the configured root directory
5. **Start** the TFTP service

**Windows Firewall Configuration:**
```cmd
# Run as Administrator - Allow TFTP through firewall
netsh advfirewall firewall add rule name="TFTP" dir=in action=allow protocol=udp localport=69
```

**Verify Windows IP Address:**
```cmd
ipconfig
```

### 2. Network Configuration

- Ensure your host machine and the camera board are on the same network segment
- Note your host machine's IP address:
```bash
ip addr show | grep -E 'inet.*global'
```

## TFTP Boot Procedure

### Step 1: Interrupt Boot Sequence
1. **Power on** the camera board
2. **Immediately press `Ctrl+C`** before the device boots completely
3. You should see the **nvt U-Boot shell** prompt: `nvt:`

### Step 2: Configure Network Settings
```bash
# Set TFTP server IP (your host machine's IP)
nvt: setenv serverip <your_host_machine_ip>

# Set board IP address (any unused IP in the same network range)
nvt: setenv ipaddr <board_ip_address>
```

**Example:**
```bash
nvt: setenv serverip 192.168.1.100
nvt: setenv ipaddr 192.168.1.101
```

### Step 3: Download Firmware via TFTP
```bash
# Download firmware to all_in_one memory area
nvt: tftp 0x10000000 FW98538A.bin
```

**Expected Output:**
```
Using eth0@2,f01b0000 device
TFTP from server 192.168.1.100; our IP address is 192.168.1.101
Filename 'FW98538A.bin'.
Load address: 0x10000000
Loading: ################################################################
         ################################################################
         [... progress bars ...]
         ################################################################
         ####################################
         XX.X MiB/s
done
Bytes transferred: XXXXXXXX (XXXXXXX hex)
```

### Step 4: Update Firmware
```bash
# Flash the downloaded firmware
nvt: nvt_update_all 0x10000000 0
```

### Step 5: Boot the System
```bash
# Boot the board with new firmware
nvt: nvt_boot
```

> **Important:** Always use `0x10000000` for TFTP downloads. The `linuxtmp` area at `0x02000000` is protected and will cause "reserved memory" errors.

## Quick Reference Commands

```bash
# Complete TFTP boot sequence
nvt: setenv serverip 192.168.1.100
nvt: setenv ipaddr 192.168.1.101
nvt: tftp 0x10000000 FW98538A.bin
nvt: nvt_update_all 0x10000000 0
nvt: nvt_boot
```

## File Locations

- **Firmware binary:** `output/packed/FW98538A.bin`
- **Memory configuration:** `configs/Linux/cfg_539A_IPC_NAND_RAMDISK_EVB/nvt-mem-tbl.dtsi`
- **TFTP root:** `/var/lib/tftpboot/`

## Notes

- The firmware binary name is `FW98538A.bin` for ETERNA hardware
- Ensure you're using the correct binary from the `packed` folder
- The all_in_one memory area is specifically designed for firmware updates
- Always verify the download completed successfully before running `nvt_update_all`
