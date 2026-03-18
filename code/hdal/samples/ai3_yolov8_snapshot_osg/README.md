# AI3 YOLOv8 Snapshot + OSG BBox Sample

Live sensor capture with AI3 NPU inference (YOLOv8/SCRFD) + bounding box drawing on YUV frames via `hd_gfx_draw_rect` + JPEG snapshot encoding.

## Overview

This sample combines three Novatek HDAL features:

1. **Live Sensor Pipeline** — videocap → videoproc (GC5603 sensor @ 2960×1664)
2. **AI3 NPU Inference** — YOLOv8 object detection or SCRFD face detection
3. **OSG / BBox Drawing** — `hd_gfx_draw_rect()` draws colored bounding boxes directly on YUV frames
4. **JPEG Snapshot** — annotated frames encoded via videoenc and saved to SD card
5. **VLC Smart BBox** — `VENDOR_VIDEOENC_SMART_BBOX` for live RTSP bbox overlay

## Architecture

```
Sensor (GC5603)
    │
    ▼
 videocap ──bind──▶ videoproc ──pull──▶ AI thread (NPU inference)
                        │                    │
                        │              ┌─────┘ results
                        │              ▼
                    pull_out_buf ──▶ draw_bbox (hd_gfx_draw_rect)
                        │                    │
                        ▼              ┌─────┘ annotated frame
                    videoenc ──push──▶ JPEG encode
                        │
                        ▼
                   /mnt/sd/snap_bbox_XXXX.jpg
```

## Files

| File | Description |
|------|-------------|
| `ai3_yolov8_snapshot_osg.c` | Main: sensor pipeline + AI + GFX bbox + JPEG snapshot |
| `yolov8_postproc.c/h` | YOLOv8 post-processing (NMS, decode) |
| `scrfd_postproc.c/h` | SCRFD face detection post-processing |
| `Makefile` | Build configuration |
| `models/coco_labels.txt` | COCO 80-class label file |

## Building

```bash
cd /path/to/ns02302_linux_sdk
source build/envsetup.sh

cd code/hdal/samples/ai3_yolov8_snapshot_osg
make clean && make
```

## Usage

```bash
# YOLOv8 object detection + snapshot with bbox
./ai3_yolov8_snapshot_osg <model.bin> [labels.txt]

# SCRFD face detection + snapshot with bbox
./ai3_yolov8_snapshot_osg --scrfd <model.bin>
```

### Interactive Keys

| Key | Action |
|-----|--------|
| `s` | Take JPEG snapshot with bounding boxes drawn on frame |
| `r` | Toggle AI inference ON/OFF |
| `d` | Enter HDAL debug menu |
| `q` | Quit |

### Example

```bash
# Deploy to camera
adb push ai3_yolov8_snapshot_osg /mnt/sd/
adb push models/yolov8n.bin /mnt/sd/models/
adb push models/coco_labels.txt /mnt/sd/models/

# Run on camera
cd /mnt/sd
./ai3_yolov8_snapshot_osg models/yolov8n.bin models/coco_labels.txt

# Press 's' to save annotated JPEG snapshots to /mnt/sd/snap_bbox_XXXX.jpg
```

## How BBox Drawing Works

### 1. GFX Draw (on YUV frame — visible in JPEG snapshots)

Uses `hd_gfx_draw_rect()` to draw colored hollow rectangles directly on the YUV420 buffer:

```c
HD_GFX_DRAW_RECT dr = {0};
dr.dst_img.dim       = frame_dim;
dr.dst_img.format    = HD_VIDEO_PXLFMT_YUV420;
dr.dst_img.p_phy_addr[0] = frame->phy_addr[0];
dr.color     = Y | (U << 8) | (V << 16);
dr.type      = HD_GFX_RECT_HOLLOW;
dr.thickness = 4;
dr.rect      = {x, y, w, h};
hd_gfx_draw_rect(&dr);
```

**Color scheme (YUV):**
- Green = person (class 0)
- Red = vehicles (class 1-7)
- Blue = other / faces

### 2. Smart BBox (VLC overlay — visible in RTSP stream)

Uses `vendor_videoenc_set(enc_path, VENDOR_VIDEOENC_PARAM_SMART_BBOX, ...)` from `vendor_videoenc.h`:

```c
VENDOR_VIDEOENC_SMART_BBOX sb = {0};
sb.base_resolution = {VDO_SIZE_W, VDO_SIZE_H};
sb.bbox[i].positions[TOP_LEFT]     = {x1, y1};
sb.bbox[i].positions[BOTTOM_RIGHT] = {x2, y2};
sb.bbox[i].class_id = VENDOR_VIDEOENC_SMART_ROI_CLASS0;
vendor_videoenc_set(enc_path, VENDOR_VIDEOENC_PARAM_SMART_BBOX, &sb);
```

## Configuration

| Parameter | Default | Location |
|-----------|---------|----------|
| Sensor resolution | 2960×1664 | `VDO_SIZE_W/H` defines |
| AI input size | 640×640 | `AI_INPUT_W/H` defines |
| Confidence threshold | 0.25 (YOLO) / 0.3 (SCRFD) | `ai3_infer_frame()` |
| NMS threshold | 0.45 (YOLO) / 0.4 (SCRFD) | `ai3_infer_frame()` |
| BBox line thickness | 4 px | `BBOX_THICKNESS` |
| JPEG quality | 90 | `set_enc_param()` |
| Sensor driver | nvt_sen_gc5603 | `set_cap_cfg()` |

## Differences from ai3_custom_yolov8

| Feature | ai3_custom_yolov8 | ai3_yolov8_snapshot_osg |
|---------|-------------------|------------------------|
| Input | Static file (.bin) | Live sensor (GC5603) |
| Output | Console text | JPEG file + VLC bbox overlay |
| BBox drawing | None | hd_gfx_draw_rect on YUV |
| Smart BBox | None | VENDOR_VIDEOENC_SMART_BBOX |
| Pipeline | AI only | videocap → videoproc → videoenc |
| Threads | Single | AI thread + Snapshot thread |

## Related Samples

- `ai3_custom_yolov8/` — Static file AI inference (base for this sample)
- `hd_video_snapshot/` — Snapshot pipeline reference (no AI)
- `hd_video_snapshot_with_osg/` — Snapshot + OSG stamps/masks
- `hd_video_record_with_smart_roi/` — Smart ROI bbox with encoder
- `ai3_turnkey_sample_stream_ipc/` — Turnkey AI + IPC bbox overlay
