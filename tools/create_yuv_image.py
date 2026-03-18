#!/usr/bin/env python3
"""
YUV420 Semi-Planar (NV12) Image Creator
Converts PNG/JPG to YUV420 SP format for AI testing
"""

import sys
import numpy as np
from PIL import Image

def rgb_to_yuv420sp(rgb_image, width, height):
    """Convert RGB image to YUV420 Semi-Planar (NV12) format"""
    # Resize if needed
    if rgb_image.size != (width, height):
        rgb_image = rgb_image.resize((width, height), Image.LANCZOS)
    
    # Convert to numpy array
    rgb = np.array(rgb_image)
    
    # RGB to YUV conversion
    R = rgb[:, :, 0].astype(np.float32)
    G = rgb[:, :, 1].astype(np.float32)
    B = rgb[:, :, 2].astype(np.float32)
    
    # BT.601 conversion
    Y = 0.299 * R + 0.587 * G + 0.114 * B
    U = -0.168736 * R - 0.331264 * G + 0.5 * B + 128
    V = 0.5 * R - 0.418688 * G - 0.081312 * B + 128
    
    Y = np.clip(Y, 0, 255).astype(np.uint8)
    U = np.clip(U, 0, 255).astype(np.uint8)
    V = np.clip(V, 0, 255).astype(np.uint8)
    
    # Downsample U and V (4:2:0)
    U_downsampled = U[::2, ::2]
    V_downsampled = V[::2, ::2]
    
    # Interleave U and V for NV12 format
    uv_height = height // 2
    uv_width = width // 2
    UV = np.empty((uv_height, uv_width, 2), dtype=np.uint8)
    UV[:, :, 0] = U_downsampled
    UV[:, :, 1] = V_downsampled
    
    # Flatten to create output buffer
    y_plane = Y.flatten()
    uv_plane = UV.flatten()
    
    return np.concatenate([y_plane, uv_plane])

def main():
    if len(sys.argv) < 4:
        print("Usage: create_yuv_image.py <input.png/jpg> <width> <height> [output.bin]")
        print("Example: create_yuv_image.py photo.jpg 512 376 YUV420_SP_W512H376.bin")
        sys.exit(1)
    
    input_file = sys.argv[1]
    width = int(sys.argv[2])
    height = int(sys.argv[3])
    output_file = sys.argv[4] if len(sys.argv) > 4 else f"YUV420_SP_W{width}H{height}.bin"
    
    # Check width and height are even (required for YUV420)
    if width % 2 != 0 or height % 2 != 0:
        print("Error: Width and height must be even numbers for YUV420 format")
        sys.exit(1)
    
    print(f"Converting {input_file} to YUV420 SP format...")
    print(f"Target size: {width}x{height}")
    
    try:
        # Load image
        img = Image.open(input_file)
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Convert to YUV420 SP
        yuv_data = rgb_to_yuv420sp(img, width, height)
        
        # Save to file
        yuv_data.tofile(output_file)
        
        expected_size = width * height * 3 // 2
        actual_size = len(yuv_data)
        
        print(f"✓ Created: {output_file}")
        print(f"  Size: {actual_size} bytes (expected: {expected_size})")
        print(f"  Format: YUV420 Semi-Planar (NV12)")
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
