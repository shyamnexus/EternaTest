# Web API Changelog — Frontend

## v1.7.0 (2026-03-05)

### New AI Analytics Endpoints

Five new analytics feature APIs have been added:

| Endpoint | Methods | Description |
|----------|---------|-------------|
| `/api/v1/analytics/face` | GET, PUT | Face detection, recognition, and attribute analysis |
| `/api/v1/analytics/face/gallery` | GET | Face recognition gallery listing |
| `/api/v1/analytics/lpr` | GET, PUT | License plate recognition (ANPR/ALPR) |
| `/api/v1/analytics/heatmap` | GET, PUT | Heat map configuration |
| `/api/v1/analytics/heatmap/grid` | GET | Live heat map grid data (2D float array) |
| `/api/v1/analytics/heatmap/reset` | POST | Reset accumulated heat map data |
| `/api/v1/analytics/pose` | GET, PUT | Pose estimation (body keypoints) |
| `/api/v1/analytics/audio` | GET, PUT | Audio classification (YAMNet) |

### Models Used

| Feature | Model | NPU |
|---------|-------|-----|
| Face Detection | `scrfd_nosig` | CNN30 |
| License Plate | `lpr_nvt` | CNN30 |
| Pose Estimation | `yolo26n_pose` | CNN30 |
| Face Attributes | `face_attribute` | CNN30 |
| Audio Classification | `yamnet` | CNN30 |

---

## v1.6.0 (2026-03-04)

### Breaking Change — AES-256-CBC Removed

`auth_tag` is now **required** in all `EncryptedPassword` payloads. The AES-256-CBC fallback has been removed from the backend. Requests without `auth_tag` are rejected with HTTP 400.

| Field | Status | Notes |
|-------|--------|-------|
| `auth_tag` | **Required** | Base64-encoded 16-byte GCM authentication tag |
| `iv` | **Must be 12 bytes** | 16-byte CBC IVs are no longer accepted |

### Affected Endpoints

| Endpoint | What changes |
|----------|-------------|
| `POST /auth/login` | `auth_tag` required in password object |
| `POST /auth/change-password` | `auth_tag` required in `old_password` and `new_password` |
| `POST /auth/forgot-password/reset` | `auth_tag` required in `new_password` |
| `POST /users` | `auth_tag` required in password object |

### What to Change in `encryption.service.ts`

1. Remove `crypto-js` dependency — use Web Crypto API (`crypto.subtle`) for AES-GCM
2. Change IV from 16 bytes to **12 bytes**
3. Split GCM output: ciphertext (all but last 16 bytes) + auth tag (last 16 bytes)
4. Send `auth_tag` field in all encrypted password payloads
5. Pass raw AES key bytes to RSA encrypt (not Base64 string)
6. Remove legacy `encrypt()` method and `secretKey` field

---

## v1.5.0 (2026-02-25)

### What Changed

`GET /api/v1/auth/public-key` now returns `"aes_algorithm": "AES-256-GCM"` instead of `"AES-256-CBC"`.

The `EncryptedPassword` object has a new `auth_tag` field (Base64-encoded 16-byte GCM tag).
The `iv` field expects **12 bytes** (GCM) instead of 16 bytes (CBC).
