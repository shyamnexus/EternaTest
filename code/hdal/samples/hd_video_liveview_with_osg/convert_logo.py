#!/usr/bin/env python3
"""
Convert IMPACT-by-honeywell.png to ARGB4444 raw format for OSD overlay.
Output format: 8-byte header (width U32, height U32) + raw ARGB4444 pixels
"""

from PIL import Image
import struct
import sys

def convert_to_argb4444(image_path, output_path, max_width=300):
    """Convert PNG to ARGB4444 binary format."""
    img = Image.open(image_path).convert("RGBA")
    
    # Resize maintaining aspect ratio
    if img.size[0] > max_width:
        w_percent = (max_width / float(img.size[0]))
        target_height = int((float(img.size[1]) * float(w_percent)))
        img = img.resize((max_width, target_height), Image.Resampling.LANCZOS)
    
    width, height = img.size
    print(f"Converting: {image_path}")
    print(f"Output dimensions: {width}x{height}")
    print(f"Output file: {output_path}")

    with open(output_path, 'wb') as f:
        # Write header: width (4 bytes LE), height (4 bytes LE)
        f.write(struct.pack('<II', width, height))
        
        for y in range(height):
            for x in range(width):
                r, g, b, a = img.getpixel((x, y))
                
                # Convert 8-bit to 4-bit per channel
                a4 = (a >> 4) & 0xF
                r4 = (r >> 4) & 0xF
                g4 = (g >> 4) & 0xF
                b4 = (b >> 4) & 0xF
                
                # ARGB4444: Alpha in MSB nibble
                val = (a4 << 12) | (r4 << 8) | (g4 << 4) | b4
                
                # Write as little-endian unsigned short
                f.write(struct.pack('<H', val))
    
    total_bytes = 8 + (width * height * 2)
    print(f"Total bytes written: {total_bytes}")

if __name__ == "__main__":
    input_file = "IMPACT-by-honeywell.png"
    output_file = "logo.bin"
    
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    if len(sys.argv) > 2:
        output_file = sys.argv[2]
    
    convert_to_argb4444(input_file, output_file)
    print("Conversion complete!")
