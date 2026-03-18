#!/usr/bin/env python3
"""Visualize YOLOv8 detection results overlaid on the test image."""
import numpy as np
from PIL import Image, ImageDraw, ImageFont
import sys

W, H = 640, 640

# Load NV12 bus image and convert to RGB
nv12 = np.fromfile("bus_640x640_nv12.bin", dtype=np.uint8)
y = nv12[:W*H].reshape(H, W).astype(np.float32)
uv = nv12[W*H:].reshape(H//2, W//2, 2).astype(np.float32)
u = np.repeat(np.repeat(uv[:,:,0], 2, axis=0), 2, axis=1)
v = np.repeat(np.repeat(uv[:,:,1], 2, axis=0), 2, axis=1)

r = np.clip(y + 1.402 * (v - 128), 0, 255).astype(np.uint8)
g = np.clip(y - 0.344136 * (u - 128) - 0.714136 * (v - 128), 0, 255).astype(np.uint8)
b = np.clip(y + 1.772 * (u - 128), 0, 255).astype(np.uint8)

img = Image.fromarray(np.stack([r, g, b], axis=2))

# Detection results from inference output
detections = [
    {"label": "umbrella", "conf": 0.2663, "bbox": (0.084, 0.447, 0.266, 0.603)},
]

draw = ImageDraw.Draw(img)
for det in detections:
    x1 = int(det["bbox"][0] * W)
    y1 = int(det["bbox"][1] * H)
    x2 = int(det["bbox"][2] * W)
    y2 = int(det["bbox"][3] * H)
    conf = det["conf"]
    label = f'{det["label"]} {conf*100:.1f}%'
    
    draw.rectangle([x1, y1, x2, y2], outline="red", width=3)
    draw.text((x1, y1 - 12), label, fill="red")

img.save("detection_result.png")
print(f"Saved detection_result.png ({img.size[0]}x{img.size[1]})")

# Also save the raw NV12->RGB conversion so we can see what the source image looks like
img_clean = Image.fromarray(np.stack([r, g, b], axis=2))
img_clean.save("bus_rgb.png")
print("Saved bus_rgb.png (clean source image)")
