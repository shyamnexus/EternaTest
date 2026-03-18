/**
 * @file recording_handler.h
 * @brief Recording API HTTP handlers
 */

#pragma once

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

/**
 * GET /api/v1/recording
 * Get recording status and configuration
 */
api::Response HandleRecording(const api::RequestContext& ctx);

/**
 * GET /api/v1/recording/sessions
 * Get active recording sessions
 */
api::Response HandleRecordingSessions(const api::RequestContext& ctx);

/**
 * POST /api/v1/recording/start
 * Start recording on a channel
 * Body: { "channel": 0, "type": "manual" }
 */
api::Response HandleRecordingStart(const api::RequestContext& ctx);

/**
 * POST /api/v1/recording/stop
 * Stop recording on a channel
 * Body: { "channel": 0 }
 */
api::Response HandleRecordingStop(const api::RequestContext& ctx);

/**
 * GET /api/v1/recordings
 * List recordings with filtering and pagination
 * Query params: channel, start_time, end_time, type, offset, limit
 */
api::Response HandleRecordingsList(const api::RequestContext& ctx);

/**
 * GET /api/v1/recordings/{id}
 * Get recording details by ID
 */
api::Response HandleRecordingGet(const api::RequestContext& ctx);

/**
 * DELETE /api/v1/recordings/{id}
 * Delete a recording
 */
api::Response HandleRecordingDelete(const api::RequestContext& ctx);

/**
 * DELETE /api/v1/recordings
 * Delete multiple recordings
 * Body: { "ids": ["id1", "id2"] } or { "before": timestamp_ms }
 */
api::Response HandleRecordingsDelete(const api::RequestContext& ctx);

/**
 * GET /api/v1/recordings/{id}/stream
 * Stream recording file (with decryption)
 * Supports HTTP Range requests for seeking
 */
api::Response HandleRecordingStream(const api::RequestContext& ctx);

/**
 * GET /api/v1/recording/storage
 * Get SD card storage status and statistics
 */
api::Response HandleRecordingStorage(const api::RequestContext& ctx);

/**
 * POST /api/v1/recording/storage/format
 * Format SD card
 * Body: { "filesystem": "exfat" }  // or "fat32"
 */
api::Response HandleRecordingStorageFormat(const api::RequestContext& ctx);

/**
 * GET /api/v1/recording/config
 * Get recording configuration
 */
api::Response HandleRecordingConfig(const api::RequestContext& ctx);

/**
 * PUT /api/v1/recording/config
 * Update recording configuration
 */
api::Response HandleRecordingConfigUpdate(const api::RequestContext& ctx);

/**
 * GET /api/v1/recording/channels/{id}
 * Get channel recording configuration
 */
api::Response HandleRecordingChannel(const api::RequestContext& ctx);

/**
 * PUT /api/v1/recording/channels/{id}
 * Update channel recording configuration
 */
api::Response HandleRecordingChannelUpdate(const api::RequestContext& ctx);

// ============================================================================
// SD Card Management APIs
// ============================================================================

/**
 * GET /api/v1/sdcard
 * Get SD card detection status and info
 * Returns: inserted, mounted, filesystem, size, needs_format, etc.
 */
api::Response HandleSdCardStatus(const api::RequestContext& ctx);

/**
 * POST /api/v1/sdcard/mount
 * Mount SD card (if inserted but not mounted)
 */
api::Response HandleSdCardMount(const api::RequestContext& ctx);

/**
 * POST /api/v1/sdcard/unmount
 * Safely unmount SD card (stops recordings first)
 */
api::Response HandleSdCardUnmount(const api::RequestContext& ctx);

/**
 * POST /api/v1/sdcard/format
 * Format SD card with specified filesystem
 * Body: { "filesystem": "exfat", "label": "IPCAM", "confirm": true }
 * Note: exfat recommended for cards > 4GB
 */
api::Response HandleSdCardFormat(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
