#!/usr/bin/env python3
"""
Convert sensor register commands from C driver format to device tree byte format.
"""

import re
import sys

def parse_c_commands(c_code):
    """
    Parse C-style sensor commands like:
    {0x0103, 1, {0x01, 0x0}},
    
    Returns list of (address, value) tuples
    """
    commands = []
    # Match patterns like {0x0103, 1, {0x01, 0x0}}
    pattern = r'\{(0x[0-9a-fA-F]+),\s*\d+,\s*\{(0x[0-9a-fA-F]+),'
    
    for match in re.finditer(pattern, c_code):
        addr = int(match.group(1), 16)
        value = int(match.group(2), 16)
        
        # Skip special commands (they start with high values like 0xFF)
        if addr < 0x8000:
            commands.append((addr, value))
    
    return commands

def convert_to_dtsi_format(commands):
    """
    Convert register commands to device tree format:
    03 30 00 00 [ADDR_HIGH] [ADDR_LOW] [VALUE] 00
    """
    dtsi_lines = []
    
    for addr, value in commands:
        addr_high = (addr >> 8) & 0xFF
        addr_low = addr & 0xFF
        
        # Format: 03 30 00 00 [ADDR_HIGH] [ADDR_LOW] [VALUE] 00
        line = f"\t\t\t03 30 00 00 {addr_high:02X} {addr_low:02X} {value:02X} 00"
        dtsi_lines.append(line)
    
    return dtsi_lines

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 convert_sensor_cmds.py <input_c_file> [mode_number]")
        print("Example: python3 convert_sensor_cmds.py sen_sc500ai.c 1")
        sys.exit(1)
    
    input_file = sys.argv[1]
    mode_num = sys.argv[2] if len(sys.argv) > 2 else "1"
    
    try:
        with open(input_file, 'r') as f:
            content = f.read()
        
        # Find the specific mode array
        mode_pattern = f'static CTL_SEN_CMD sc500ai_mode_{mode_num}\\[\\].*?{{(.*?)^}};'
        match = re.search(mode_pattern, content, re.MULTILINE | re.DOTALL)
        
        if not match:
            print(f"Error: Could not find sc500ai_mode_{mode_num} in {input_file}")
            sys.exit(1)
        
        mode_code = match.group(1)
        
        # Parse commands
        commands = parse_c_commands(mode_code)
        print(f"Found {len(commands)} register commands in mode {mode_num}")
        
        # Convert to DTSI format
        dtsi_lines = convert_to_dtsi_format(commands)
        
        # Print output
        print("\n// Device Tree format commands:")
        print("\t\tcmd = [")
        for line in dtsi_lines:
            print(line)
        print("\t\t\t00 00 00 00")
        print("\t\t];")
        
        # Calculate offsets for gain and exposure commands
        print(f"\n// Total commands: {len(dtsi_lines)}")
        print(f"// Each command is 8 bytes")
        print(f"// Gain and exposure commands are typically at the end")
        print(f"// You may need to adjust gain_cmd_oft and expt_cmd_oft values")
        
    except FileNotFoundError:
        print(f"Error: File {input_file} not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
