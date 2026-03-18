# Release Notes - Version 1.5.0

**Release Date:** March 5, 2026

---

## Summary

This release delivers a major security hardening of the authentication and credential storage systems, upgrades password encryption from AES-256-CBC to AES-256-GCM, introduces PBKDF2-based password hashing, and adds five new AI-powered analytics modules (Face Detection/Recognition, License Plate Recognition, Heat Map, Pose Estimation, Audio Classification) running on the CNN30 NPU.

---

## Security Hardening

### AES-256-GCM Password Transit Encryption
- **Upgraded from AES-256-CBC to AES-256-GCM** for all password transit encryption
- GCM provides authenticated encryption — tamper detection in addition to confidentiality
- New `auth_tag` field (16-byte Base64) required in all `EncryptedPassword` payloads
- IV size changed from 16 bytes (CBC) to 12 bytes (GCM standard)
- Backward compatibility: clients omitting `auth_tag` fall back to CBC (will be removed in future release)

### PBKDF2-HMAC-SHA256 Password Hashing
- **Replaced single-pass SHA-256 with PBKDF2-HMAC-SHA256** (210,000 iterations)
- Per-user random 32-byte salt for each password hash
- Significantly increased brute-force resistance
- Existing password hashes are migrated on next successful login

### Per-Device Random Salt for SQLCipher
- **SQLCipher database key derivation** now uses a per-device random salt
- Salt generated with `RAND_bytes()` on first boot, stored in protected flash
- Prevents cross-device credential replay attacks

### WSSE Storage Upgrade
- **WSSE credentials now stored with AES-256-GCM** encryption at rest
- Replaced previous XOR-based obfuscation
- WSSE nonce/digest validation hardened

### Additional Security Fixes
- **Hardcoded default password fallback removed** — factory reset now requires initial-setup flow
- **Shell command injection protection** — all user-supplied strings sanitized before system calls
- **Constant-time password comparison** — prevents timing side-channel attacks
- **`std::rand()` replaced with `RAND_bytes()`** for all cryptographic RNG usage

---

## AI Analytics Modules

### 1. Face Detection & Recognition
- **SCRFD face detector** (`scrfd_nosig` model) running on CNN30 NPU
- Configurable minimum face size and confidence threshold
- **Face Recognition** with embedding database at `/mnt/app/face_db`
  - Cosine similarity matching with configurable threshold
  - Gallery capacity up to 1,000 enrolled faces
  - Adjustable face crop margin for embedding extraction
- **Face Attribute Analysis** — age, gender, expression estimation
- REST API: `GET/PUT /api/v1/analytics/face`, `GET /api/v1/analytics/face/gallery`

### 2. License Plate Recognition (LPR)
- **ANPR/ALPR** using `lpr_nvt` model on CNN30 NPU
- Multi-region support (configurable region list, e.g., US, EU)
- Configurable confidence threshold
- Event action support (recording, snapshot, webhook, MQTT, email, FTP)
- REST API: `GET/PUT /api/v1/analytics/lpr`

### 3. Heat Map Analytics
- **Spatial activity density visualization** using object detection results
- Configurable grid resolution (default 32×18)
- Temporal decay rate for accumulated activity fading
- Live grid data retrieval for web UI overlay rendering
- REST API: `GET/PUT /api/v1/analytics/heatmap`, `GET /api/v1/analytics/heatmap/grid`, `POST /api/v1/analytics/heatmap/reset`

### 4. Pose Estimation
- **Human body keypoint detection** using `yolo26n_pose` model on CNN30 NPU
- 17-point COCO keypoint format (nose, eyes, ears, shoulders, elbows, wrists, hips, knees, ankles)
- Configurable confidence and NMS thresholds
- Track up to 16 persons simultaneously
- REST API: `GET/PUT /api/v1/analytics/pose`

### 5. Audio Classification
- **YAMNet-based environmental sound detection** (`yamnet` model)
- Processes 16kHz mono PCM audio from camera microphone
- 1-second analysis windows (15,360 samples)
- Detects events such as glass breaking, gunshot, dog bark, siren, baby cry
- Configurable detection threshold
- REST API: `GET/PUT /api/v1/analytics/audio`

---

## API Changes

### Auth API v1.5.0
- `GET /api/v1/auth/public-key` — `aes_algorithm` now returns `"AES-256-GCM"`
- `EncryptedPassword` schema adds `auth_tag` field for GCM authentication tag
- All password-accepting endpoints updated: login, change-password, forgot-password/reset, users/create

### Analytics API v1.6.0
- 8 new path definitions, 7 new schemas added to OpenAPI spec
- All JSON float responses use `round_json()` to eliminate IEEE 754 precision artifacts
- Model names standardized: `scrfd_nosig`, `lpr_nvt`, `yolo26n_pose`, `face_attribute`, `yamnet`

### New API Endpoints

| Endpoint | Methods | Description |
|----------|---------|-------------|
| `/api/v1/analytics/face` | GET, PUT | Face detection, recognition, and attribute config |
| `/api/v1/analytics/face/gallery` | GET | Face recognition gallery listing |
| `/api/v1/analytics/lpr` | GET, PUT | License plate recognition config |
| `/api/v1/analytics/heatmap` | GET, PUT | Heat map configuration |
| `/api/v1/analytics/heatmap/grid` | GET | Live heat map grid data |
| `/api/v1/analytics/heatmap/reset` | POST | Reset heat map accumulation |
| `/api/v1/analytics/pose` | GET, PUT | Pose estimation config |
| `/api/v1/analytics/audio` | GET, PUT | Audio classification config |

---

## Configuration Changes

### analytics.json — New Sections
```json
{
  "heat_map": {
    "enabled": false,
    "grid_w": 32,
    "grid_h": 18,
    "decay_rate": 0.97
  },
  "pose_estimation": {
    "enabled": false,
    "model": "yolo26n_pose",
    "conf_threshold": 0.35,
    "nms_threshold": 0.50,
    "max_persons": 16
  },
  "face_attribute": {
    "enabled": false,
    "model": "face_attribute",
    "margin": 0.2
  },
  "audio_classification": {
    "enabled": false,
    "model": "yamnet",
    "detection_threshold": 0.3
  }
}
```

### Default Settings
- All new AI modules disabled by default (opt-in activation)
- Face recognition database path: `/mnt/app/face_db`
- Audio classification processes camera microphone input when enabled

---

## Build & Deployment

### NPU Models Required
The following `.bin` model files must be present in `/mnt/app/models/` for each module:

| Module | Model File | Size |
|--------|-----------|------|
| Face Detection | `scrfd_nosig.bin` | ~2 MB |
| License Plate | `lpr_nvt.bin` | ~3 MB |
| Pose Estimation | `yolo26n_pose.bin` | ~4 MB |
| Face Attributes | `face_attribute.bin` | ~1 MB |
| Audio Classification | `yamnet.bin` | ~1 MB |

### Cross-Compilation
- Target: aarch64-ca53 (Cortex-A53)
- Build: `cmake --build build/aarch64-ca53 --parallel`
- All 28 targets build successfully

---

## Known Issues

- AI modules increase CPU/NPU usage when enabled — enable only modules needed for deployment
- Face gallery management (enroll/delete) is read-only in this release; enrollment requires manual database population
- Audio classification requires camera microphone to be enabled and functional
- Heat map data is reset on service restart (not persisted to flash)

---

## Upgrade Notes

⚠️ **Breaking Change (Web Frontend):** Clients using AES-256-CBC for password encryption must migrate to AES-256-GCM. The CBC fallback is deprecated and will be removed in v1.6.0.

- Firmware upgrade preserves existing user accounts and configurations
- Existing passwords are automatically rehashed with PBKDF2 on next login
- New `analytics.json` sections are added by factory defaults if not present
- First boot after upgrade regenerates SQLCipher device salt

---

## Documentation

- Updated OpenAPI spec (`analytics.yaml`, `ipcamera-api.yaml`) with all new endpoints
- API changelog updated in `CHANGELOG.md`
- `SDK_KNOWLEDGE.md` updated with AI module architecture
- Secure Boot Guide updated with GCM notes
