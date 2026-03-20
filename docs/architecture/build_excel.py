"""
Build Eterna IP Camera Architecture Excel workbook for leadership.

Setup (one-time):
    pip install -r requirements.txt
      — or —
    pip install openpyxl Pillow

Run:
    python build_excel.py
    python3 build_excel.py          # macOS / Linux

Output:
    eterna_ipcamera_architecture.xlsx   (written next to this script)
"""

import os
import sys

try:
    from openpyxl import Workbook
    from openpyxl.styles import (
        Font, PatternFill, Alignment, Border, Side, GradientFill
    )
    from openpyxl.drawing.image import Image as XLImage
    from openpyxl.utils import get_column_letter
    from openpyxl.worksheet.page import PageMargins
except ModuleNotFoundError:
    sys.exit(
        "\n[ERROR] Missing dependency.\n"
        "Run:  pip install openpyxl Pillow\n"
        "  or: pip install -r requirements.txt\n"
    )

# ── Colour palette ────────────────────────────────────────────────────────────
NAVY       = "1B2A4A"   # header background
GOLD       = "C8A84B"   # accent
LIGHT_BLUE = "E8F4FD"   # row tint
MID_GREY   = "F5F5F5"   # alternating row
DARK_GREY  = "4A4A4A"   # body text
WHITE      = "FFFFFF"
RED_FATAL  = "FADBD8"   # fatal steps
GREEN_OK   = "D5F5E3"   # ok/non-fatal

BASE_DIR  = os.path.dirname(os.path.abspath(__file__))
DIAGRAMS  = os.path.join(BASE_DIR, "diagrams")
OUT_FILE  = os.path.join(BASE_DIR, "eterna_ipcamera_architecture.xlsx")


# ── Helper utilities ──────────────────────────────────────────────────────────

def hdr_font(size=11, bold=True, color=WHITE):
    return Font(name="Calibri", size=size, bold=bold, color=color)

def body_font(size=10, bold=False, color=DARK_GREY):
    return Font(name="Calibri", size=size, bold=bold, color=color)

def title_font(size=20, bold=True, color=WHITE):
    return Font(name="Calibri", size=size, bold=bold, color=color)

def fill(hex_color):
    return PatternFill("solid", fgColor=hex_color)

def center(wrap=False):
    return Alignment(horizontal="center", vertical="center", wrap_text=wrap)

def left(wrap=True):
    return Alignment(horizontal="left", vertical="center", wrap_text=wrap)

def thin_border(sides="all"):
    s = Side(style="thin", color="CCCCCC")
    n = None
    if sides == "all":
        return Border(left=s, right=s, top=s, bottom=s)
    if sides == "bottom":
        return Border(bottom=s)
    return Border(left=s, right=s, top=s, bottom=s)

def write_header_row(ws, row, cols, bg=NAVY, text_color=WHITE, size=11):
    """Write a full-width header row with navy fill."""
    for col_idx, text in enumerate(cols, start=1):
        c = ws.cell(row=row, column=col_idx, value=text)
        c.font = Font(name="Calibri", size=size, bold=True, color=text_color)
        c.fill = fill(bg)
        c.alignment = center(wrap=True)
        c.border = thin_border()

def write_data_row(ws, row, values, alt=False, bold_first=False, bg_override=None):
    bg = bg_override or (MID_GREY if alt else WHITE)
    for col_idx, val in enumerate(values, start=1):
        c = ws.cell(row=row, column=col_idx, value=val)
        c.font = Font(name="Calibri", size=10,
                      bold=(bold_first and col_idx == 1), color=DARK_GREY)
        c.fill = fill(bg)
        c.alignment = left()
        c.border = thin_border()

def section_label(ws, row, text, col_span, bg=GOLD):
    c = ws.cell(row=row, column=1, value=text)
    c.font = Font(name="Calibri", size=11, bold=True, color=WHITE)
    c.fill = fill(bg)
    c.alignment = left(wrap=False)
    c.border = thin_border()
    if col_span > 1:
        ws.merge_cells(start_row=row, start_column=1,
                       end_row=row, end_column=col_span)

def set_col_widths(ws, widths):
    for col_idx, w in enumerate(widths, start=1):
        ws.column_dimensions[get_column_letter(col_idx)].width = w

def freeze(ws, cell="A2"):
    ws.freeze_panes = cell

def add_image(ws, path, anchor, width_px=700, height_px=500):
    if not os.path.exists(path):
        return
    img = XLImage(path)
    # Scale to fit while preserving aspect ratio
    ow, oh = img.width, img.height
    if ow and oh:
        scale = min(width_px / ow, height_px / oh)
        img.width  = int(ow * scale)
        img.height = int(oh * scale)
    ws.add_image(img, anchor)


# ── Sheet builders ────────────────────────────────────────────────────────────

def build_cover(wb):
    ws = wb.active
    ws.title = "Overview"
    ws.sheet_view.showGridLines = False

    # Banner rows
    ws.row_dimensions[1].height = 15
    ws.row_dimensions[2].height = 60
    ws.row_dimensions[3].height = 30
    ws.row_dimensions[4].height = 15

    ws.column_dimensions["A"].width = 3
    for col in range(2, 9):
        ws.column_dimensions[get_column_letter(col)].width = 22

    # Title banner
    ws.merge_cells("B2:H2")
    t = ws["B2"]
    t.value = "Eterna IP Camera — Application Architecture"
    t.font = title_font(size=22)
    t.fill = fill(NAVY)
    t.alignment = center()

    ws.merge_cells("B3:H3")
    s = ws["B3"]
    s.value = "ipcamd daemon  ·  code/application/ipcamera/  ·  Architecture Review"
    s.font = Font(name="Calibri", size=11, italic=True, color=WHITE)
    s.fill = fill(NAVY)
    s.alignment = center()

    row = 5
    # ── What is ipcamd? ──────────────────────────────────────────────────────
    ws.merge_cells(f"B{row}:H{row}")
    c = ws.cell(row=row, column=2, value="What is ipcamd?")
    c.font = Font(name="Calibri", size=13, bold=True, color=NAVY)
    c.alignment = left(wrap=False)
    row += 1

    desc = (
        "ipcamd is the single Linux daemon that owns the complete lifecycle of an Eterna IP camera. "
        "It captures sensor data via the Novatek HDAL SDK, encodes up to four simultaneous H.264/H.265 "
        "video streams, distributes video to RTSP / recording / WebRTC consumers, runs on-device AI "
        "analytics on the NPU, dispatches configurable event-driven actions, and exposes a REST API, "
        "ONVIF service, and mDNS/DNS-SD — all as a single coordinated process."
    )
    ws.merge_cells(f"B{row}:H{row}")
    ws.row_dimensions[row].height = 52
    d = ws.cell(row=row, column=2, value=desc)
    d.font = body_font(size=10)
    d.alignment = left(wrap=True)
    d.fill = fill(LIGHT_BLUE)
    d.border = thin_border()
    row += 2

    # ── Key Numbers ───────────────────────────────────────────────────────────
    ws.merge_cells(f"B{row}:H{row}")
    c = ws.cell(row=row, column=2, value="Key Numbers")
    c.font = Font(name="Calibri", size=13, bold=True, color=NAVY)
    row += 1

    metrics = [
        ("Video streams",        "4 simultaneous (H.264 + H.265)"),
        ("Audio stream",         "1 (G.711 µ-law, 8 kHz)"),
        ("Singleton classes",    "35 across 11 modules"),
        ("Config files",         "18 factory-default JSON files"),
        ("Ring buffer (MediaHub)","30 slots per channel = 1 second at 30 fps"),
        ("AI analytics",         "NPU (YOLOv5s) + motion + tamper + LPR + face + audio"),
        ("Event types",          "40+ typed EventType values"),
        ("Action handlers",      "9 (recording, snapshot, email, webhook, MQTT, FTP, alarm, light, siren)"),
        ("Startup steps",        "23 ordered; 4 are fatal (config, storage, webserver, ONVIF)"),
        ("Recording encryption", "AES-256-CTR per MP4 segment"),
    ]
    write_header_row(ws, row, ["Metric", "Value", "", "", "", "", "", ""], bg=NAVY)
    ws.merge_cells(start_row=row, start_column=3, end_row=row, end_column=8)
    row += 1
    for i, (metric, value) in enumerate(metrics):
        ws.merge_cells(start_row=row, start_column=3, end_row=row, end_column=8)
        write_data_row(ws, row, [metric, value], alt=(i % 2 == 1), bold_first=True)
        row += 1
    row += 1

    # ── Design Patterns at a Glance ───────────────────────────────────────────
    ws.merge_cells(f"B{row}:H{row}")
    c = ws.cell(row=row, column=2, value="Design Patterns at a Glance")
    c.font = Font(name="Calibri", size=13, bold=True, color=NAVY)
    row += 1

    write_header_row(ws, row, ["Pattern", "Where", "Purpose", "", "", "", "", ""], bg=NAVY)
    ws.merge_cells(start_row=row, start_column=4, end_row=row, end_column=8)
    row += 1

    patterns = [
        ("Singleton  X::Instance()",
         "All 11 modules (35 classes)",
         "Single shared instance per manager/engine; C++11 Meyers Singleton; thread-safe without extra locking."),
        ("Coordinator  main.cpp",
         "apps/ipcamd/src/main.cpp",
         "Ordered 23-step startup, reverse shutdown, cross-subsystem wiring. No business logic in main."),
        ("Publish–Subscribe  EventManager",
         "modules/events/",
         "Decouples AI/hardware event sources from notification/action consumers via typed event queue."),
        ("JSON Config per Module",
         "configs/config.factory.d/*.json",
         "18 factory-default files; 3-layer merge (factory → deployment → user delta); delta-only save."),
        ("MediaHub Ring Buffer",
         "modules/media/",
         "SPMC ring buffer (30 slots = 1 s) distributes encoded video to RTSP, recording, WebRTC consumers."),
    ]
    for i, (pat, where, purpose) in enumerate(patterns):
        ws.merge_cells(start_row=row, start_column=4, end_row=row, end_column=8)
        ws.row_dimensions[row].height = 36
        write_data_row(ws, row, [pat, where, purpose], alt=(i % 2 == 1), bold_first=True)
        row += 1

    ws.page_margins = PageMargins(left=0.5, right=0.5, top=0.75, bottom=0.75)
    ws.print_area = f"A1:H{row}"


def build_diagrams(wb):
    ws = wb.create_sheet("Architecture Diagrams")
    ws.sheet_view.showGridLines = False
    ws.column_dimensions["A"].width = 3

    diagram_specs = [
        ("01_startup.png",        "1 — Startup & Shutdown Sequence",
         "Shows the 23-step ordered startup dependency chain and the reverse shutdown order."),
        ("02_video_pipeline.png", "2 — Video Pipeline Data Flow",
         "Sensor → ISP → VideoProc → HDAL encoders → MediaHub ring buffer → RTSP / Recording / WebRTC consumers."),
        ("03_event_system.png",   "3 — Event System Data Flow",
         "AI analytics callbacks → EventManager queue → rule matching → action handlers (recording, email, MQTT, FTP…)."),
        ("04_config_layering.png","4 — Configuration Layering",
         "Three-layer JSON merge: factory defaults → deployment overrides → user delta. Delta-only atomic save."),
        ("05_dependency_map.png", "5 — Module Dependency Map",
         "All 11 modules and their relationships. Config is read by every module. main.cpp wires AI to EventManager."),
    ]

    row = 1
    for filename, title, description in diagram_specs:
        ws.row_dimensions[row].height = 22
        ws.merge_cells(f"B{row}:L{row}")
        c = ws.cell(row=row, column=2, value=title)
        c.font = Font(name="Calibri", size=14, bold=True, color=WHITE)
        c.fill = fill(NAVY)
        c.alignment = left(wrap=False)
        row += 1

        ws.row_dimensions[row].height = 40
        ws.merge_cells(f"B{row}:L{row}")
        d = ws.cell(row=row, column=2, value=description)
        d.font = body_font(size=10)
        d.fill = fill(LIGHT_BLUE)
        d.alignment = left(wrap=True)
        d.border = thin_border()
        row += 1

        img_path = os.path.join(DIAGRAMS, filename)
        if os.path.exists(img_path):
            anchor_row = row
            img = XLImage(img_path)
            ow, oh = img.width, img.height
            target_w = 900
            scale = target_w / ow if ow else 1
            img.width  = int(ow * scale)
            img.height = int(oh * scale)
            img_rows = max(30, int(img.height / 15) + 2)
            for r in range(anchor_row, anchor_row + img_rows):
                ws.row_dimensions[r].height = 15
            ws.add_image(img, f"B{anchor_row}")
            row += img_rows
        else:
            ws.row_dimensions[row].height = 20
            ws.merge_cells(f"B{row}:L{row}")
            m = ws.cell(row=row, column=2, value=f"[Image not found: {filename}]")
            m.font = body_font(size=10)
            row += 1

        row += 2  # gap between diagrams

    for col in range(2, 13):
        ws.column_dimensions[get_column_letter(col)].width = 12


def build_design_patterns(wb):
    ws = wb.create_sheet("Design Patterns")
    ws.sheet_view.showGridLines = False
    set_col_widths(ws, [3, 22, 18, 60, 28])
    freeze(ws, "B3")

    row = 1
    ws.merge_cells("B1:E1")
    t = ws.cell(row=1, column=2, value="Design Patterns — Detailed Reference")
    t.font = Font(name="Calibri", size=16, bold=True, color=WHITE)
    t.fill = fill(NAVY)
    t.alignment = center()
    ws.row_dimensions[1].height = 32

    row = 2
    write_header_row(ws, row, ["", "Pattern", "Location", "Description", "Key Implementation Detail"])
    row += 1

    patterns_detail = [
        # (pattern_name, location, description, key_detail)
        ("Singleton\nX::Instance()",
         "All 11 modules\n35 classes total",
         "Every manager and engine exposes a single static Instance() method returning a reference "
         "to a function-local static object. The C++11 standard guarantees thread-safe initialisation "
         "without additional locking. Private constructor + deleted copy/move prevent accidental "
         "duplication. Used for: all managers, all AI engines, all platform controllers.",
         "static ClassName& Instance() {\n  static ClassName instance;\n  return instance;\n}\n"
         "Exception: MdnsResponder uses GetInstance() — identical body."),

        ("Coordinator\nmain.cpp",
         "apps/ipcamd/src/\nmain.cpp (859 lines)",
         "main.cpp contains zero business logic. Its sole job is: (1) initialise subsystems in "
         "dependency order, (2) wire cross-cutting callbacks (AI analytics → EventManager), "
         "(3) block in a signal-driven main loop, (4) tear down in reverse order. "
         "This ensures no two modules import each other directly, eliminating circular dependencies.",
         "23 ordered startup steps; 4 fatal exits (config, storage, webserver, ONVIF). "
         "Shutdown uses _Exit(0) to bypass C++ static destructors — required for SQLCipher safety."),

        ("Publish–Subscribe\nEventManager",
         "modules/events/\nevent_manager.h/.cpp",
         "Thread-safe std::queue<Event> fed by non-blocking Publish*() calls from any thread. "
         "A single background ProcessingThread dequeues events, calls all matching free listeners "
         "(EventListener = std::function<void(const Event&)>), and evaluates EventRule list from "
         "events.json. Matching rules dispatch ActionHandler::Execute() in detached threads so "
         "the processing thread is never blocked. 40+ EventType values, 9 ActionHandlers.",
         "Analytics → EventManager bridge wired in main.cpp only:\n"
         "analytics.SetMotionCallback([&em](...){ em.PublishMotionStart(...); });\n"
         "8 callback types; 9 action handler classes."),

        ("JSON Config\nper Module",
         "modules/config/\nconfigs/config.factory.d/",
         "nlohmann/json with dot-path API: ipcam::config::Get<int>(\"network.rtsp.port\", 554). "
         "Three layers merged at boot: (1) config.factory.d/*.json — 18 per-module files shipped "
         "with firmware; (2) config.d/*.json — optional deployment overrides; (3) config.json — "
         "user delta only (runtime). Save() diffs runtime vs factory baseline and writes only "
         "changed keys atomically via rename(). Factory reset = delete config.json.",
         "Array index notation: \"storage.channels[0].id\"\n"
         "All 18 factory files + user delta fits in <50 KB.\n"
         "PBKDF2 / AES-256-GCM used for user credentials (UserManager)."),

        ("MediaHub\nRing Buffer",
         "modules/media/\nmedia_hub.h/.cpp",
         "Single-Producer Multiple-Consumer ring buffer per video channel. One ProducerThread "
         "calls hd_videoenc_pull_out_buf() in a loop and writes to 30-slot ring buffer "
         "(1 second at 30 fps, 512 KB/slot for main stream = 15 MB). Consumers each hold a "
         "next_sequence_ cursor and call WaitForFrame() which blocks on a condition_variable. "
         "Odd/even version field prevents torn reads. Late-join: ResetToKeyframe() scans for "
         "nearest IDR. Codec changes: PauseChannel → HDAL change → ResumeChannel + IDR request.",
         "SPMC; no locks on read path (version versioning).\n"
         "Consumers: RTSP (HdalVideoSource/Live555), RecordingService (MP4Recorder/minimp4), "
         "Go2rtcManager (WebRTC/HLS).\n"
         "AES-256-CTR encryption applied per recording segment."),
    ]

    for i, (pat, loc, desc, detail) in enumerate(patterns_detail):
        ws.row_dimensions[row].height = 110
        alt = i % 2 == 1
        bg = MID_GREY if alt else WHITE

        ws.cell(row=row, column=2, value=pat).font = Font(
            name="Calibri", size=10, bold=True, color=NAVY)
        ws.cell(row=row, column=2).fill = fill(bg)
        ws.cell(row=row, column=2).alignment = left(wrap=True)
        ws.cell(row=row, column=2).border = thin_border()

        ws.cell(row=row, column=3, value=loc).font = body_font(size=9)
        ws.cell(row=row, column=3).fill = fill(bg)
        ws.cell(row=row, column=3).alignment = left(wrap=True)
        ws.cell(row=row, column=3).border = thin_border()

        ws.cell(row=row, column=4, value=desc).font = body_font(size=10)
        ws.cell(row=row, column=4).fill = fill(bg)
        ws.cell(row=row, column=4).alignment = left(wrap=True)
        ws.cell(row=row, column=4).border = thin_border()

        ws.cell(row=row, column=5, value=detail).font = Font(
            name="Courier New", size=8, color=DARK_GREY)
        ws.cell(row=row, column=5).fill = fill(LIGHT_BLUE if not alt else "EBF5FB")
        ws.cell(row=row, column=5).alignment = left(wrap=True)
        ws.cell(row=row, column=5).border = thin_border()

        row += 1


def build_modules(wb):
    ws = wb.create_sheet("Module Catalogue")
    ws.sheet_view.showGridLines = False
    set_col_widths(ws, [3, 20, 22, 18, 55])
    freeze(ws, "B3")

    ws.merge_cells("B1:E1")
    t = ws.cell(row=1, column=2, value="Module Catalogue — All Subsystems")
    t.font = Font(name="Calibri", size=16, bold=True, color=WHITE)
    t.fill = fill(NAVY)
    t.alignment = center()
    ws.row_dimensions[1].height = 32

    row = 2
    write_header_row(ws, row, ["", "Module", "Class / Component", "Namespace", "Role"])
    row += 1

    modules = [
        # (module_group, class_name, namespace, role)
        ("ai", "AnalyticsEngine", "ipcam::ai", "Orchestrates all AI sub-engines; wires NPU, motion, tamper, LPR, face, audio; fires typed callbacks."),
        ("ai", "NpuInference", "ipcam::ai", "Runs YOLOv5s ONNX model on the NPU; 640×360 YUV input from HDAL VideoProc path."),
        ("ai", "MotionDetectionEngine", "ipcam::ai", "Pixel-diff motion detection on 160×120 YUV; configurable zones, threshold, schedule."),
        ("ai", "ObjectTracker", "ipcam::ai", "Multi-object tracking; emits first-seen events per track ID to EventManager."),
        ("ai", "TamperDetectionEngine", "ipcam::ai", "Laplacian variance + scene change; detects camera blocking, defocus, or physical shift."),
        ("ai", "VqaEngine", "ipcam::ai", "Video quality analysis on 320×180 VQA path."),
        ("ai", "PrivacyMosaicEngine", "ipcam::ai", "Applies mosaic blur to configured screen regions before encoding."),
        ("ai", "AiispEngine", "ipcam::ai", "AI-based ISP enhancement (denoising, HDR recovery) for low-light scenes."),
        ("platform", "HdalPipeline", "ipcam::platform", "Sole owner of all HDAL path IDs. Opens sensor, ISP, VideoProc, 4 encoder paths, 1 audio, AI/MD/VQA paths."),
        ("platform", "HdalWrapper", "ipcam::platform", "Wraps vendor_isp_* APIs; normalises all 0-100 values to vendor SDK enums."),
        ("platform", "ISPControl", "ipcam::platform", "High-level typed structs (ImageAdjustment, WhiteBalance, ExposureSettings…) over HdalWrapper. Loads/saves isp.json."),
        ("platform", "IRControl", "ipcam::platform", "IR LED via PWM ch.11; IR cut filter via dual-GPIO H-bridge pulse; auto day/night via SW-CDS thresholds or schedule."),
        ("platform", "OsdOverlay", "ipcam::platform", "FreeType timestamp/text/logo overlays; 4 privacy masks per stream via HDAL OSG; font auto-scales with resolution."),
        ("platform", "ResetButton", "ipcam::platform", "Polls GPIO12 at 100 ms; 10-second hold triggers factory reset callback."),
        ("platform", "SystemLogger", "ipcam::platform", "Replaces system_logger.sh; collects dmesg, SoC temp, CPU/mem/disk; SD card (30-day) + flash (3 MB cap)."),
        ("media", "MediaHub", "ipcam::media", "SPMC ring buffer hub — 30 slots per channel (1 sec @ 30 fps). Producer threads + consumer cursors. See ring buffer sheet."),
        ("media", "VideoControl", "ipcam::media", "Serialises concurrent API calls; orchestrates pause/resume for codec changes; fires codec_change_callback_."),
        ("media", "AudioControl", "ipcam::media", "Audio capture config; provides G.711 / AAC-LC frames to AudioFrameBroadcaster."),
        ("streaming", "RtspServer (Live555)", "ipcam::streaming", "HdalVideoSource is a Live555 FramedSource; WaitForFrame(); strips Annex-B; RTP timestamps from HDAL HW clock."),
        ("streaming", "Go2rtcManager", "ipcam::streaming", "Manages go2rtc process for WebRTC and HLS output alongside RTSP."),
        ("streaming", "AudioFrameBroadcaster", "ipcam::streaming", "Distributes G.711 (RTSP) and AAC-LC (recording) audio frames to all registered consumers."),
        ("streaming", "OnvifMetadataGenerator", "ipcam::streaming", "ONVIF metadata stream with bounding-box XML; fed from AnalyticsEngine DetectionCallback."),
        ("recording", "RecordingService", "ipcam::recording", "One RecordingThread per channel; VideoFrameConsumer from MediaHub; MP4Recorder (minimp4); AES-256-CTR per segment."),
        ("recording", "ScheduleManager", "ipcam::recording", "Polls schedule config; calls RecordingService Start/Stop on schedule boundary crossings."),
        ("recording", "MP4Recorder", "ipcam::recording", "Handles Annex-B stripping, hvcC/avcC box creation, fragmented MP4 writes, audio track muxing."),
        ("storage", "NasManager", "ipcam::storage", "NFS/SMB network-attached storage mount and access management."),
        ("storage", "FtpManager", "ipcam::storage", "FTP upload queue; triggered by FtpUploadHandler action."),
        ("events", "EventManager", "ipcam::events", "Thread-safe event queue; single ProcessingThread; free listeners + rule-based action dispatch in detached threads."),
        ("events", "ActionHandlerFactory", "ipcam::events", "RegisterAll() registers 9 concrete ActionHandler subclasses; SubstitutePlaceholders() for template strings."),
        ("networking", "NetworkManager", "ipcam::networking", "Facade: IPv4/v6, DNS, hostname, SSL certs, nginx, NTP, SMTP, SNMP, UPnP, WiFi, link monitor thread."),
        ("networking", "NtpManager", "ipcam::networking", "Pure C++ RFC 5905 NTP — no ntpd. Fallback: pool servers → Google/Cloudflare/NIST/Apple hardcoded IPs."),
        ("networking", "NginxManager", "ipcam::networking", "Generates nginx.conf from current network settings; manages nginx process lifecycle."),
        ("networking", "SslManager", "ipcam::networking", "Self-signed cert generation (2048-bit RSA, 10 yr) or custom PEM install with validation."),
        ("networking", "MdnsResponder", "(global)", "Raw socket mDNS/DNS-SD — no avahi/dbus; announces _http, _https, _rtsp, _onvif TCP services."),
        ("config", "ipcam::config API", "ipcam::config", "Free-function Get<T>/Set<T>/Save() backed by nlohmann/json; dot-path + array index notation."),
        ("config", "UserManager", "ipcam::config", "SQLite+SQLCipher; PBKDF2-HMAC-SHA256 (210k iterations); AES-256-GCM for ONVIF passwords; brute-force lockout."),
        ("onvif", "ONVIF Service", "ipcam::onvif", "WS-Discovery (UDP 3702) + SOAP service (TCP 5000) via gSOAP. Device, Media, Events, PTZ stub."),
        ("webserver", "Webserver", "ipcam::webserver", "Embedded HTTP/HTTPS REST API (port 8082); handlers for all config, streaming, recording, analytics, system endpoints."),
    ]

    group_colors = {
        "ai": "EAF2FF",
        "platform": "F0F4F8",
        "media": "E8F8F5",
        "streaming": "FEF9E7",
        "recording": "FDF2F8",
        "storage": "F9EBEA",
        "events": "EAFAF1",
        "networking": "EBF5FB",
        "config": "FEF5E7",
        "onvif": "F5EEF8",
        "webserver": "FDFEFE",
    }
    current_group = None
    for item in modules:
        group, cls, ns, role = item
        if group != current_group:
            section_label(ws, row, f"  {group.upper()}  MODULE", 5, bg=NAVY)
            row += 1
            current_group = group
        bg = group_colors.get(group, WHITE)
        ws.row_dimensions[row].height = 40
        write_data_row(ws, row, ["", cls, ns, role], bg_override=bg, bold_first=False)
        ws.cell(row=row, column=2).value = cls
        ws.cell(row=row, column=2).font = Font(name="Calibri", size=10, bold=True, color=NAVY)
        ws.cell(row=row, column=3).value = ns
        ws.cell(row=row, column=3).font = body_font(size=9)
        ws.cell(row=row, column=4).value = role
        ws.cell(row=row, column=4).alignment = left(wrap=True)
        row += 1


def build_startup(wb):
    ws = wb.create_sheet("Startup Sequence")
    ws.sheet_view.showGridLines = False
    set_col_widths(ws, [3, 6, 28, 14, 42, 22])
    freeze(ws, "B3")

    ws.merge_cells("B1:F1")
    t = ws.cell(row=1, column=2, value="Startup & Shutdown Sequence")
    t.font = Font(name="Calibri", size=16, bold=True, color=WHITE)
    t.fill = fill(NAVY)
    t.alignment = center()
    ws.row_dimensions[1].height = 32

    row = 2
    write_header_row(ws, row, ["", "#", "Subsystem Call", "Fatal?", "Key Dependency / Notes", "Shutdown Order"])
    row += 1

    steps = [
        (1,  "Logging (spdlog multi-sink)",              "No",      "None — must be first",                                          "25 — after all modules"),
        (2,  "Signal handlers (SIGINT / SIGTERM)",        "No",      "None",                                                          "24 — inherent to process exit"),
        (3,  "CLI argument parsing",                      "No",      "—",                                                             "—"),
        (4,  "ipcam::config::Init()",                    "YES (2)", "None — all other modules read config",                          "1 — last to shut down"),
        (5,  "NetworkManager::InitializeNetwork()",      "No",      "Config",                                                        "—"),
        (6,  "SystemLogger::Instance().Init()",          "No",      "Config",                                                        "8"),
        (7,  "HdalPipeline::Instance().Init() + Start()", "No",     "Config — initialises sensor, ISP, encoders",                   "4 — after MediaHub"),
        (8,  "MediaHub::Instance().Initialize() + Start()", "No",   "HdalPipeline must be running",                                  "5 — after RTSP + Recording"),
        (9,  "AnalyticsEngine (lazy init)",              "No",      "Connects to HdalPipeline VideoProc paths",                      "7"),
        (10, "VideoControl::Instance().Init()",          "No",      "HdalPipeline",                                                  "—"),
        (11, "IRControl::Instance().Init()",             "No",      "Config — starts auto day/night thread",                         "9"),
        (12, "ResetButton::Instance().Init()",           "No",      "Config — starts GPIO poll thread",                              "10"),
        (13, "streaming::Initialize() + Start() (RTSP)", "No",      "MediaHub must be running",                                      "6 — after go2rtc"),
        (14, "Go2rtcManager::Instance().Start()",        "No",      "RTSP server must be running",                                   "11 — before RTSP server"),
        (15, "storage::Init() (SQLite)",                 "YES (3)", "Config — opens user/recording/secure DBs",                     "2"),
        (16, "NasManager::Instance().Init()",            "No",      "Storage",                                                       "13"),
        (17, "EventManager::Instance().Init() + Start()","No",      "All above — wires all analytics callbacks; publishes kSystemStartup", "3"),
        (18, "FtpManager::Instance().Init()",            "No",      "Storage",                                                       "14"),
        (19, "ScheduleManager::Instance().Init()",       "No",      "Storage",                                                       "12"),
        (20, "VideoControl::Instance().Init() (2nd)",    "No",      "HdalPipeline — explicit re-init marker in source",              "—"),
        (21, "RecordingService::Instance().Initialize()","No",      "Storage + MediaHub",                                            "15"),
        (22, "webserver::Init()",                        "YES (4)", "All above — starts REST API server",                           "16"),
        (23, "onvif::Init()",                            "YES (5)", "Network + Storage — WS-Discovery + SOAP service",              "17"),
    ]

    for i, (num, call, fatal, dep, shutdown) in enumerate(steps):
        ws.row_dimensions[row].height = 36
        is_fatal = "YES" in fatal
        bg = RED_FATAL if is_fatal else (MID_GREY if i % 2 == 1 else WHITE)
        for col_idx, val in enumerate(["", num, call, fatal, dep, shutdown], start=1):
            c = ws.cell(row=row, column=col_idx, value=val)
            c.fill = fill(bg)
            c.border = thin_border()
            c.alignment = left(wrap=True)
            if col_idx == 2:  # number
                c.font = Font(name="Calibri", size=11, bold=True,
                              color="C0392B" if is_fatal else NAVY)
                c.alignment = center()
            elif col_idx == 3:  # call
                c.font = Font(name="Calibri", size=10, bold=True, color=DARK_GREY)
            elif col_idx == 4:  # fatal
                c.font = Font(name="Calibri", size=10, bold=True,
                              color="C0392B" if is_fatal else "27AE60")
                c.alignment = center()
            else:
                c.font = body_font(size=9)
        row += 1

    row += 1
    section_label(ws, row, "  SHUTDOWN SEQUENCE (reverse dependency order)", 6, bg=NAVY)
    row += 1

    shutdown_seq = [
        "1.  EventManager.Stop()           — drain queue; publish kSystemShutdown first",
        "2.  RecordingService.Shutdown()   — flush and close open MP4 segments",
        "3.  ScheduleManager.Shutdown()",
        "4.  FtpManager.Shutdown()",
        "5.  NasManager.Shutdown()",
        "6.  AnalyticsEngine.Shutdown()    — stop NPU inference threads",
        "7.  SystemLogger.Shutdown()",
        "8.  IRControl.Shutdown()          — turn off IR LED, set IR cut to day mode",
        "9.  ResetButton.Shutdown()",
        "10. Go2rtcManager.Stop()          — must stop before RTSP server",
        "11. streaming::Stop()             — RTSP server",
        "12. MediaHub.Shutdown()           — stop producer threads, release ring buffers",
        "13. HdalPipeline.Shutdown()       — stop encoders",
        "14. onvif::Shutdown()",
        "15. webserver::Shutdown()",
        "16. storage::Shutdown()",
        "17. config::Shutdown()",
        "     _Exit(0)                     — bypasses C++ static destructors (SQLCipher safety)",
    ]
    for line in shutdown_seq:
        ws.row_dimensions[row].height = 18
        ws.merge_cells(f"B{row}:F{row}")
        c = ws.cell(row=row, column=2, value=line)
        c.font = Font(name="Courier New", size=9, color=DARK_GREY)
        c.fill = fill(LIGHT_BLUE)
        c.border = thin_border("bottom")
        row += 1


def build_config(wb):
    ws = wb.create_sheet("Configuration")
    ws.sheet_view.showGridLines = False
    set_col_widths(ws, [3, 32, 18, 60])
    freeze(ws, "B3")

    ws.merge_cells("B1:D1")
    t = ws.cell(row=1, column=2,
                value="JSON Configuration System — 18 Factory-Default Files")
    t.font = Font(name="Calibri", size=16, bold=True, color=WHITE)
    t.fill = fill(NAVY)
    t.alignment = center()
    ws.row_dimensions[1].height = 32

    row = 2
    write_header_row(ws, row, ["", "File", "Module", "What it configures"])
    row += 1

    configs = [
        ("device.json", "Device Identity",
         "Device name ('Eterna'), model, serial number, firmware/encoder/web/plugin versions, "
         "channel count, sensor capabilities (GC5603, MIPI, 1920×1080, AIQ/NPU enable flags)."),
        ("media.json", "Video & Audio",
         "Sensor driver (nvt_sen_gc5603), ISP tuning path, video processing flags (AE/AWB/AF/WDR/defog/3DNR), "
         "memory pools, 4 encoder configs: H.265 main 2944×1664 30fps 6Mbps, H.264 sub 1280×720, "
         "H.264 third 640×480, H.264 fourth 640×360 (disabled). JPEG snapshot, G.711µ 8kHz audio."),
        ("isp.json", "Image Quality",
         "ISP scenario, all image sliders (brightness/contrast/saturation/sharpness/hue/gamma), "
         "white balance (AWB mode/preset/gains), orientation (mirror/flip/rotation), "
         "anti-flicker 50Hz, exposure (auto AE with manual time/gain/ISO), day/night mode, "
         "BLC/WDR/HDR/HLC, 2D+3D NR, defog, edge enhancement, DPC, lens shading."),
        ("osd.json", "On-Screen Display",
         "Per-stream OSD: timestamp format, font sizes, positions, logo position, stamp buffer "
         "dimensions for 3 streams, plus 4 privacy mask regions."),
        ("streaming.json", "RTSP / WebRTC",
         "RTSP server (Live555, port 554), go2rtc restreamer (binary path, config path, init script)."),
        ("network.json", "Network",
         "Interfaces (eth0, loopback), RTMP/RTSP/HTTP/HTTPS/ONVIF ports, IPv4 (static or DHCP with "
         "fallback link-local), IPv6 (auto/SLAAC), DNS, hostname (auto from MAC prefix 'hon'), "
         "mDNS services, multicast, DDNS, SNMP, UPnP, SMTP, gratuitous ARP."),
        ("onvif.json", "ONVIF",
         "WS-Discovery (UDP port 3702), services TCP port 5000, device info fields, interface name."),
        ("ir.json", "IR LED / Day-Night",
         "IR LED (PWM channel 11, brightness 80, frequency 1kHz), IR cut filter (GPIO pins 115/114, "
         "pulse delay 100ms), auto day/night (SW CDS thresholds, schedule, brightness mode)."),
        ("analytics.json", "AI Analytics",
         "Motion detection (zones, schedule, actions), smart detection (YOLOv5s, pedestrian/vehicle/animal), "
         "line crossing, zone intrusion, people counting, video tampering, face detection, LPR, "
         "heat map, pose estimation, face attribute, audio classification, global AI engine settings "
         "(640×360 YUV input, DLA), MQTT/email/FTP notification server configs."),
        ("events.json", "Event Rules & Actions",
         "5 pre-defined rules (motion→recording+snapshot, person detected→ONVIF+email+webhook+MQTT, "
         "vehicle→ONVIF, line crossing, zone intrusion alarm), global notification config "
         "(email, push, ONVIF), webhook/MQTT/Home Assistant/FTP/cloud integration, "
         "alarm I/O (GPIO pins, siren, white light), storage paths for event snapshots/videos."),
        ("recording.json", "Recording Schedule",
         "Schedule enable flag, active profile ID ('default-24x7'), paths for factory/user profile "
         "directories, NAS (SMB), FTP upload config."),
        ("recording/profiles/\ndefault-24x7.json", "24×7 Schedule Profile",
         "24/7 continuous recording schedule: all 7 days × 24 hours, each slot enabled:true, "
         "type:'continuous'."),
        ("storage.json", "Storage & Encryption",
         "SD card mount path, persistent app path, recording type/base dir, FIFO mode, "
         "AES-256-CTR encryption flag, pre/post record seconds, 3 SQLite DB paths "
         "(user management, recordings, secure config), 3 channel slots."),
        ("auth.json", "Authentication",
         "SQLite DB path (/mnt/app/ipcamera/db/users.db), session token expiry (24h), "
         "password policy (PBKDF2 100k iterations), default admin credentials."),
        ("logging.json", "Logging",
         "Console + file logging (spdlog), access log, security log, audit log, remote syslog "
         "(RFC 5424), system logger subsystem (kernel/thermal/sysdiag/app_mirror sources; "
         "SD card + flash destinations with size/retention limits)."),
        ("system.json", "System",
         "Timezone (UTC), NTP (pool.ntp.org + many fallback servers + direct IPs), DST, language, "
         "watchdog (30s interval), memory monitor (85% threshold), scheduled maintenance, alarm thresholds."),
        ("web_portal.json", "Web API Server",
         "Embedded HTTP server (port 8082, 4 threads, 200MB max upload), authentication "
         "(session 30min, 5 failed attempts → 15min lockout), CORS, rate limiting (60 req/min), "
         "optional Unix socket."),
        ("reset_button.json", "Reset Button",
         "GPIO pin 12, hold 10s for factory reset, active-low, 50ms debounce, 100ms poll interval."),
    ]

    for i, (fname, module, desc) in enumerate(configs):
        ws.row_dimensions[row].height = 52
        bg = MID_GREY if i % 2 == 1 else WHITE
        write_data_row(ws, row, ["", fname, module, desc], bg_override=bg)
        ws.cell(row=row, column=2).font = Font(
            name="Calibri", size=10, bold=True, color=NAVY)
        ws.cell(row=row, column=3).font = Font(
            name="Calibri", size=9, bold=True, color=DARK_GREY)
        ws.cell(row=row, column=4).font = body_font(size=9)
        row += 1

    # Layering explanation
    row += 1
    section_label(ws, row, "  CONFIG LAYERING STRATEGY", 4, bg=NAVY)
    row += 1

    layering = [
        ("Layer 1 — Factory (lowest)",  "config.factory.d/*.json",  "18 per-module files shipped with firmware; sorted alphabetically; merged via json::merge_patch()."),
        ("Layer 2 — Deployment",        "config.d/*.json",          "Optional per-SKU/deployment overrides. Applied on top of factory."),
        ("Baseline snapshot",           "(internal: g_factory_tree)","After layers 1+2 are merged, a snapshot is taken as the 'factory baseline' for delta computation."),
        ("Layer 3 — User (highest)",    "config.json",              "Only user-changed keys. Written by Save(). If empty, deleted so factory defaults apply on next boot."),
        ("Save() strategy",             "atomic rename()",          "diff(g_factory_tree, g_config_tree) → write only delta to config.json.tmp → rename. Factory reset = delete config.json."),
    ]
    write_header_row(ws, row, ["", "Layer", "Path", "Description"])
    row += 1
    for i, (layer, path, desc) in enumerate(layering):
        ws.row_dimensions[row].height = 36
        bg = LIGHT_BLUE if i % 2 == 0 else WHITE
        write_data_row(ws, row, ["", layer, path, desc], bg_override=bg)
        ws.cell(row=row, column=2).font = Font(
            name="Calibri", size=10, bold=True, color=NAVY)
        row += 1


def build_concurrency(wb):
    ws = wb.create_sheet("Concurrency & Safety")
    ws.sheet_view.showGridLines = False
    freeze(ws, "B3")

    # ── Title ────────────────────────────────────────────────────────────────
    ws.merge_cells("B1:J1")
    t = ws.cell(row=1, column=2, value="Concurrency & Thread-Safety Analysis")
    t.font = Font(name="Calibri", size=16, bold=True, color=WHITE)
    t.fill = fill("922B21")
    t.alignment = center()
    ws.row_dimensions[1].height = 32

    set_col_widths(ws, [3, 20, 18, 22, 52, 46])

    # ── Section 1: Thread Inventory ───────────────────────────────────────────
    row = 2
    section_label(ws, row, "  THREAD INVENTORY", 6, bg=NAVY)
    row += 1
    write_header_row(ws, row, ["", "Thread", "Owner", "Count", "Purpose", "Shutdown mechanism"])
    row += 1

    threads = [
        ("main",                "main.cpp",         "1",    "Signal-wait loop; startup/shutdown coordinator",        "SIGTERM → g_running=false"),
        ("ProducerThread",      "MediaHub",         "1 per active channel (max 4)",  "Pulls encoded frames from HDAL; writes to ring buffer", "stop_requested.store(true) + join()"),
        ("ProcessingThread",    "EventManager",     "1",    "Single consumer of event queue; dispatches rules and listeners", "running_=false + notify_all + join()"),
        ("Action threads",      "EventManager",     "N (detached)", "One per triggered action (recording, email, webhook, MQTT, FTP, I/O)", "Detached — no explicit drain (known risk R2)"),
        ("ProcessingThread",    "AnalyticsEngine",  "1",    "Main AI inference loop (NPU + object tracking)",       "running_.store(false) + join()"),
        ("MdProcessingThread",  "AnalyticsEngine",  "1",    "Motion detection on 160×120 YUV path",                 "md_thread_running_.store(false) + join()"),
        ("AudioProcessingThread","AnalyticsEngine", "1",    "Audio classification",                                  "audio_thread_running_.store(false) + join()"),
        ("RecordingThread",     "RecordingService", "1 per active channel", "MP4 write loop; consumes from MediaHub ring buffer", "stop_requested=true + join()"),
        ("MonitorThread",       "RecordingService", "1",    "Polls SD card availability every 5 s",                  "shutdown_requested_=true + join()"),
        ("UploadWorker",        "FtpManager",       "1",    "Queued FTP upload processing (libcurl)",                "shutdown_requested_.store(true) + notify_all + join()"),
        ("AutoDetectionLoop",   "IRControl",        "0 or 1","Luma-threshold day/night switching",                  "auto_thread_running_.store(false) + notify_all + join()"),
        ("ScheduleMonitorLoop", "IRControl",        "0 or 1","Time-based day/night schedule",                       "schedule_thread_running_.store(false) + notify_all + join()"),
        ("SwCdsDetectionLoop",  "IRControl",        "0 or 1","Software CDS (ISP EV-value) day/night detection",    "sw_cds_thread_running_.store(false) + notify_all + join()"),
        ("Live555 event loop",  "RtspServer",       "1",    "Single-threaded RTSP/RTP dispatch (Live555 framework)","live555::Medium::close()"),
        ("Link monitor",        "NetworkManager",   "1",    "Periodic interface link-state polling",                 "Atomic flag + join()"),
        ("SystemLogger worker", "SystemLogger",     "1",    "Periodic dmesg/temp/stats collection",                  "Atomic flag + join()"),
    ]
    for i, (thr, owner, count, purpose, shutdown) in enumerate(threads):
        ws.row_dimensions[row].height = 30
        bg = MID_GREY if i % 2 == 1 else WHITE
        for col_idx, val in enumerate(["", thr, owner, count, purpose, shutdown], start=1):
            c = ws.cell(row=row, column=col_idx, value=val)
            c.fill = fill(bg)
            c.border = thin_border()
            c.alignment = left(wrap=True)
            c.font = Font(name="Calibri", size=9,
                          bold=(col_idx == 2), color=NAVY if col_idx == 2 else DARK_GREY)
        row += 1

    # ── Section 2: Key Mechanisms ─────────────────────────────────────────────
    row += 1
    section_label(ws, row, "  KEY CONCURRENCY MECHANISMS", 6, bg=NAVY)
    row += 1
    write_header_row(ws, row, ["", "Mechanism", "Where", "Pattern", "Race Condition Prevented", "Notes"])
    row += 1

    mechs = [
        ("Seqlock-style ring buffer",
         "MediaHub\nmedia_hub.cpp",
         "atomic<uint64_t> version per slot\nOdd = writing, Even = ready\nRelease store / Acquire load",
         "Torn reads of encoded video frames by multiple concurrent consumers",
         "No mutex on read path — lock-free SPMC.\nTrailing re-read catches overwrites that started mid-memcpy."),
        ("False-sharing prevention",
         "MediaHub\nmedia_hub.h",
         "alignas(64) on write_pos_ and frames_written_",
         "Cache-line ping-pong between producer writes and consumer reads on multi-core SoC",
         "Avoids cache-line invalidation on every frame written."),
        ("Atomic pause/resume for codec change",
         "MediaHub\nVideoControl",
         "paused.store(true, release) → HDAL param change → paused.store(false, release)\nframe_mutex guards cached_sps/pps/vps",
         "ProducerThread accessing HDAL encoder handle while parameters are being changed",
         "PauseChannel sleeps kVideoPullTimeoutMs+50ms to let producer exit any in-progress HDAL call before the change."),
        ("running_.exchange for idempotent stop",
         "MediaHub",
         "running_.exchange(false, memory_order_release)",
         "Two concurrent Stop() calls: both attempt to join producer threads",
         "exchange atomically reads-and-clears — only effective stopper triggers the join path."),
        ("Copy-before-iterate (listeners)",
         "EventManager\nevent_manager.cpp",
         "Copy listeners_ under lock, release lock, iterate copy",
         "Deadlock: listener callback calling AddListener/RemoveListener which acquires listeners_mutex_",
         "Trade-off: a listener removed mid-iteration receives one final notification."),
        ("Lock released before callbacks",
         "AnalyticsEngine\nIRControl\nRecordingService",
         "Narrow lock scope for stats update; callback fired after scope closes",
         "Deadlock: callback re-entering any method that acquires the same non-recursive mutex_",
         "Pattern applied to 7 of 8 analytics callbacks. audio_event_callback_ is an exception (Risk R3)."),
        ("unique_lock.unlock() before re-entrant call",
         "HdalPipeline\nIRControl",
         "unique_lock lock(mutex); ...; lock.unlock(); CallFuncThatAcquiresMutex(); lock.lock();",
         "Deadlock from non-recursive std::mutex + call chain that re-acquires the same lock",
         "Rollback path re-acquires the lock via a fresh lock_guard."),
        ("Shutdown() defers lock until after Stop()",
         "HdalPipeline",
         "Shutdown() calls Stop() (which locks), then acquires lock for cleanup",
         "Deadlock: Shutdown holding lock → calls Stop() → tries to acquire same lock",
         "Explicitly commented in source."),
        ("Interruptible sleep via condition_variable",
         "IRControl\nFtpManager\nRecordingService",
         "cv.wait_for(lock, timeout, [this]{ return stop_flag.load(); })\ncv.notify_all() in Shutdown()",
         "Up to 30-second join delay if thread uses plain sleep_for(30s)",
         "RecordingService MonitorThread uses 50×100ms chunked sleep instead."),
        ("Thread join as happens-before fence",
         "MediaHub, AnalyticsEngine\nEventManager, FtpManager, IRControl",
         "flag.store(false); thread.join(); // then access shared resources",
         "Use-after-free of HDAL VB pool pages; concurrent MD engine access; in-flight curl transfer during cleanup",
         "All join sites guarded with joinable() to prevent UB from joining a non-joinable thread."),
        ("Atomic rename() for file safety",
         "Config module\nNetworkManager",
         "write to .tmp → rename() atomically replaces target",
         "Partial-write corruption: reader sees truncated JSON or certificate on crash/power loss",
         "POSIX guarantees rename() on same filesystem is atomic."),
        ("force_segment_cut acq_rel exchange",
         "RecordingService",
         "store(true, release); exchange(false, acq_rel)",
         "Double-cut: two CutNow() calls resulting in two segments being cut; missed preceding encoder-settings writes",
         "Release on store ensures codec-change writes visible; acq_rel exchange atomically reads-and-clears."),
    ]
    for i, (mech, where, pattern, prevented, notes) in enumerate(mechs):
        ws.row_dimensions[row].height = 70
        bg = MID_GREY if i % 2 == 1 else WHITE
        for col_idx, val in enumerate(["", mech, where, pattern, prevented, notes], start=1):
            c = ws.cell(row=row, column=col_idx, value=val)
            c.fill = fill(bg if col_idx != 4 else (LIGHT_BLUE if i % 2 == 0 else "EBF5FB"))
            c.border = thin_border()
            c.alignment = left(wrap=True)
            if col_idx == 2:
                c.font = Font(name="Calibri", size=10, bold=True, color=NAVY)
            elif col_idx == 4:
                c.font = Font(name="Courier New", size=8, color=DARK_GREY)
            else:
                c.font = body_font(size=9)
        row += 1

    # ── Section 3: Mutex Hierarchy ────────────────────────────────────────────
    row += 1
    section_label(ws, row, "  MUTEX HIERARCHY & LOCK ORDERING", 6, bg=NAVY)
    row += 1
    write_header_row(ws, row, ["", "Module", "Mutex / Lock", "Protects", "Lock-ordering constraint", "Notes"])
    row += 1

    mutexes = [
        ("Config",          "g_mutex (anon ns)",        "g_config_tree, g_factory_tree, g_cache, g_initialized", "None — sole lock in module", ""),
        ("HdalPipeline",    "g_pipeline_mutex (global)", "All HDAL path handles, stream configs, encoder state",  "None — sole lock in module", "unique_lock.unlock() before ReinitWithConfig()"),
        ("IRControl",       "g_mutex (anon ns)",        "ir_led_settings_, ir_cut_settings_, auto_settings_, is_night_mode_, callbacks", "None — sole lock in module", "Manual unlock before SwitchTo*() calls"),
        ("MediaHub",        "frame_mutex (per channel)", "cached_sps/pps/vps, cached_codec; pairs with frame_cv", "None — sole lock per channel", ""),
        ("EventManager",    "queue_mutex_",             "event_queue_",                                           "queue_mutex_ must never be held when acquiring others", ""),
        ("EventManager",    "rules_mutex_",             "rules_ vector",                                          "Independent of queue_mutex_", "Raw pointer escape from MatchRules() — Risk R3"),
        ("EventManager",    "listeners_mutex_",         "listeners_ vector",                                      "Independent of queue_mutex_", "Copy-before-iterate to prevent deadlock on re-entrant callbacks"),
        ("EventManager",    "config_mutex_",            "EventManagerConfig struct",                              "config_mutex_ → rules_mutex_ (in LoadConfig)", ""),
        ("EventManager",    "history_mutex_",           "event_history_ deque",                                   "Independent",                ""),
        ("EventManager",    "stats_mutex_",             "EventStats struct + maps",                               "Independent — also acquired inside detached action threads", ""),
        ("AnalyticsEngine", "mutex_ (instance)",        "config_, stats_, all 8 callback members",               "None — sole lock in class", "Do NOT hold during callbacks — deadlock risk"),
        ("RecordingService","config_mutex_",            "RecordingConfig, channel configs",                       "config_mutex_ → callback_mutex_ (in TryReconnectStorage)", ""),
        ("RecordingService","callback_mutex_",          "segment_callback_, status_callback_, storage_callback_", "Must be acquired AFTER config_mutex_ if both needed", ""),
        ("RecordingService","playback_mutex_",          "playback_sessions_ map",                                 "Independent",                ""),
        ("FtpManager",      "mutex_",                   "config_, stats_, callbacks",                             "mutex_ → queue_mutex_ (in GetStats)", ""),
        ("FtpManager",      "queue_mutex_",             "upload_queue_, upload_history_",                         "Must be acquired AFTER mutex_ if both needed", "UploadWorker reads config_ under queue_mutex_ only — Risk R6"),
        ("NasManager",      "mutex_",                   "config_, initialized_, mount state",                     "None — sole lock in class", "TestConnection() writes config_ without lock — Risk R5"),
    ]
    for i, (module, mutex, protects, ordering, notes) in enumerate(mutexes):
        ws.row_dimensions[row].height = 40
        bg = MID_GREY if i % 2 == 1 else WHITE
        for col_idx, val in enumerate(["", module, mutex, protects, ordering, notes], start=1):
            c = ws.cell(row=row, column=col_idx, value=val)
            c.fill = fill(bg)
            c.border = thin_border()
            c.alignment = left(wrap=True)
            c.font = Font(name="Calibri", size=9,
                          bold=(col_idx == 2), color=NAVY if col_idx == 2 else DARK_GREY)
        row += 1

    # ── Section 4: Residual Risks ─────────────────────────────────────────────
    row += 1
    section_label(ws, row, "  KNOWN RESIDUAL RISKS", 6, bg="922B21")
    row += 1
    write_header_row(ws, row, ["", "ID", "Module", "Description", "Severity", "Recommended Fix"])
    row += 1

    risks = [
        ("R1", "AnalyticsEngine",
         "audio_event_callback_ is fired while mutex_ is held. If the callback calls any AnalyticsEngine API, deadlock occurs. All other 7 callbacks correctly release the lock first.",
         "Medium\n(deadlock if callback re-enters)",
         "Move audio callback invocation outside the lock scope, identical to detection_callback_ pattern."),
        ("R2", "EventManager",
         "Detached action threads capture 'this' (EventManager pointer). If EventManager is destroyed while a detached thread is still in handler->Execute(), the subsequent stats_mutex_ access is use-after-free.",
         "Low\n(singleton lives for process lifetime)",
         "Track outstanding threads with atomic counter; drain in Shutdown(). Or capture std::shared_ptr<EventManager>."),
        ("R3", "EventManager",
         "MatchRules() returns std::vector<const EventRule*> pointing into rules_ vector. After the lock releases, a concurrent AddRule/DeleteRule can reallocate the vector, causing dangling pointer dereference in ExecuteActions().",
         "Medium\n(rare — rule changes at runtime)",
         "Return std::vector<EventRule> by value from MatchRules() instead of raw pointers."),
        ("R4", "EventManager",
         "PublishEvent() reads config_.queue_max_size and AddToHistory() reads config_.max_history_size without holding config_mutex_. SetConfig() writes these under the lock — a formal C++ data race.",
         "Low\n(int reads are effectively atomic on ARM)",
         "Cache as std::atomic<int>, or snapshot config fields under lock at start of PublishEvent()."),
        ("R5", "NasManager",
         "TestConnection(nullptr) writes config_.last_error and config_.status at lines ~179-180 without holding mutex_. Concurrent GetConfig() / SetConfig() create a data race.",
         "Low\n(test operation, not on hot path)",
         "Hold mutex_ for the full duration of TestConnection when config==nullptr."),
        ("R6", "FtpManager",
         "UploadWorker() reads config_.upload_schedule and config_.enabled while holding only queue_mutex_. SetConfig() holds mutex_, so these reads race with concurrent config changes.",
         "Low\n(upload scheduler, not on hot path)",
         "Snapshot relevant config fields under mutex_ at the start of each worker iteration."),
        ("R7", "IRControl",
         "SwCdsDetectionLoop() reads is_night_mode_ and auto_settings_.mode without g_mutex after the wait_for lock releases. Both are plain non-atomic fields written under g_mutex by other threads — formally a C++ data race.",
         "Low\n(read-only check, worst case: one missed transition)",
         "Make is_night_mode_ std::atomic<bool> and auto_settings_.mode std::atomic<DayNightAutoMode>."),
        ("R8", "EventManager",
         "Init() and Start() check initialized_/running_ with load() then set them separately — a TOCTOU window. Two concurrent Init() calls could both pass the guard.",
         "Very Low\n(Init() only called from main during single-threaded startup)",
         "Use initialized_.exchange(true) to atomically check-and-set."),
    ]
    for i, (rid, module, desc, severity, fix) in enumerate(risks):
        ws.row_dimensions[row].height = 65
        bg = RED_FATAL if "Medium" in severity else (MID_GREY if i % 2 == 1 else WHITE)
        for col_idx, val in enumerate(["", rid, module, desc, severity, fix], start=1):
            c = ws.cell(row=row, column=col_idx, value=val)
            c.fill = fill(bg)
            c.border = thin_border()
            c.alignment = left(wrap=True)
            if col_idx == 2:
                c.font = Font(name="Calibri", size=10, bold=True, color="C0392B")
            elif col_idx == 5:
                c.font = Font(name="Calibri", size=9, bold=True,
                              color="C0392B" if "Medium" in severity else "27AE60")
            else:
                c.font = body_font(size=9)
        row += 1


def build_singletons(wb):
    ws = wb.create_sheet("Singleton Inventory")
    ws.sheet_view.showGridLines = False
    set_col_widths(ws, [3, 30, 20, 16, 40])
    freeze(ws, "B3")

    ws.merge_cells("B1:E1")
    t = ws.cell(row=1, column=2,
                value="Singleton Inventory — All 35 X::Instance() Classes")
    t.font = Font(name="Calibri", size=16, bold=True, color=WHITE)
    t.fill = fill(NAVY)
    t.alignment = center()
    ws.row_dimensions[1].height = 32

    row = 2
    write_header_row(ws, row, ["", "Class", "Namespace", "Module", "Role"])
    row += 1

    singletons = [
        ("EventManager",            "ipcam::events",    "events",    "Event queue, rule matching, action dispatch"),
        ("AnalyticsEngine",         "ipcam::ai",        "ai",        "AI inference orchestration"),
        ("NpuInference",            "ipcam::ai",        "ai",        "NPU model runner (YOLOv5s)"),
        ("TamperDetectionEngine",   "ipcam::ai",        "ai",        "Video tamper detection (Laplacian variance + scene change)"),
        ("MotionDetectionEngine",   "ipcam::ai",        "ai",        "Pixel-based motion detection"),
        ("ObjectTracker",           "ipcam::ai",        "ai",        "Multi-object tracking"),
        ("VqaEngine",               "ipcam::ai",        "ai",        "Video quality analysis"),
        ("PrivacyMosaicEngine",     "ipcam::ai",        "ai",        "Privacy mask / mosaic blur"),
        ("AiispEngine",             "ipcam::ai",        "ai",        "AI-based ISP enhancement"),
        ("HdalPipeline",            "ipcam::platform",  "platform",  "Novatek HDAL sensor/ISP/encoder pipeline"),
        ("HdalWrapper",             "ipcam::platform",  "platform",  "Vendor ISP tuning API wrapper"),
        ("ISPControl",              "ipcam::platform",  "platform",  "High-level ISP image quality API"),
        ("IRControl",               "ipcam::platform",  "platform",  "IR LED, IR cut filter, day/night control"),
        ("OsdOverlay",              "ipcam::platform",  "platform",  "On-screen display (timestamp, logo, privacy masks)"),
        ("ResetButton",             "ipcam::platform",  "platform",  "GPIO factory-reset button monitor"),
        ("SystemLogger",            "ipcam::platform",  "platform",  "Periodic system data collector"),
        ("MediaHub",                "ipcam::media",     "media",     "SPMC ring buffer + consumer notification"),
        ("VideoControl",            "ipcam::media",     "media",     "Encoder parameter API bridge"),
        ("AudioControl",            "ipcam::media",     "media",     "Audio parameter API bridge"),
        ("RecordingService",        "ipcam::recording", "recording", "MP4/raw segment recording to SD card"),
        ("ScheduleManager",         "ipcam::recording", "recording", "Scheduled recording triggers"),
        ("NasManager",              "ipcam::storage",   "storage",   "NFS/SMB network storage"),
        ("FtpManager",              "ipcam::storage",   "storage",   "FTP upload queue"),
        ("NetworkManager",          "ipcam::networking","networking","Network interface and service facade"),
        ("NtpManager",              "ipcam::networking","networking","Pure-C++ NTP sync (no ntpd)"),
        ("NginxManager",            "ipcam::networking","networking","nginx config and process control"),
        ("SslManager",              "ipcam::networking","networking","SSL certificate lifecycle"),
        ("MdnsResponder",           "(global)",         "networking","mDNS/DNS-SD via raw sockets  [uses GetInstance()]"),
        ("Go2rtcManager",           "ipcam::streaming", "streaming", "WebRTC/HLS restreamer process"),
        ("AudioFrameBroadcaster",   "ipcam::streaming", "streaming", "Audio consumer distribution"),
        ("OnvifMetadataGenerator",  "ipcam::streaming", "streaming", "ONVIF metadata stream (bounding-box XML)"),
        ("AuditLogger",             "(utils)",          "utils",     "Security audit log"),
        ("PasswordCrypto",          "(webserver)",      "webserver", "Password hashing helper"),
        ("ConfigLoader",            "ipcam::config",    "config",    "YAML-based config loader (secondary/legacy path)"),
    ]

    module_colors = {
        "events": "EAFAF1", "ai": "EAF2FF", "platform": "F0F4F8",
        "media": "E8F8F5", "recording": "FDF2F8", "storage": "F9EBEA",
        "networking": "EBF5FB", "streaming": "FEF9E7",
        "config": "FEF5E7", "utils": "F5EEF8", "webserver": "FDFEFE",
    }
    current_mod = None
    for i, (cls, ns, mod, role) in enumerate(singletons):
        if mod != current_mod:
            section_label(ws, row, f"  {mod.upper()}", 5, bg=NAVY)
            row += 1
            current_mod = mod
        bg = module_colors.get(mod, WHITE)
        ws.row_dimensions[row].height = 22
        write_data_row(ws, row, ["", cls, ns, mod, role], bg_override=bg)
        ws.cell(row=row, column=2).font = Font(
            name="Calibri", size=10, bold=True, color=NAVY)
        row += 1

    row += 1
    ws.merge_cells(f"B{row}:E{row}")
    note = ws.cell(row=row, column=2,
                   value="Pattern (identical in all 34 classes): "
                         "static ClassName& Instance() { static ClassName instance; return instance; }")
    note.font = Font(name="Courier New", size=9, color=DARK_GREY)
    note.fill = fill(LIGHT_BLUE)
    note.border = thin_border()


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    wb = Workbook()

    build_cover(wb)
    build_diagrams(wb)
    build_design_patterns(wb)
    build_modules(wb)
    build_startup(wb)
    build_config(wb)
    build_singletons(wb)
    build_concurrency(wb)

    # Tab colours
    tab_colors = {
        "Overview":             "1B2A4A",
        "Architecture Diagrams":"2E86C1",
        "Design Patterns":      "1A8C4E",
        "Module Catalogue":     "7D3C98",
        "Startup Sequence":     "BA4A00",
        "Configuration":        "B7950B",
        "Singleton Inventory":  "2E4053",
        "Concurrency & Safety": "922B21",
    }
    for ws in wb.worksheets:
        if ws.title in tab_colors:
            ws.sheet_properties.tabColor = tab_colors[ws.title]

    wb.save(OUT_FILE)
    print(f"Saved: {OUT_FILE}")


if __name__ == "__main__":
    main()
