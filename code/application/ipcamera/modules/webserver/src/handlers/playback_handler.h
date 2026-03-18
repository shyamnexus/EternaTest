/**
 * @file playback_handler.h
 * @brief Playback API HTTP handler declarations
 * 
 * APIs for video playback page:
 * - Search recordings by date/time/type
 * - Get recordings grouped by date (calendar view)
 * - Get recording timeline data
 * - Stream/download recordings
 * - Thumbnail retrieval
 */

#pragma once

#include "../api_handlers.h"

namespace ipcam {
namespace webserver {
namespace handlers {

// ============================================================================
// Playback Search and Discovery
// ============================================================================

/**
 * Search recordings with advanced filters
 * GET /api/v1/playback/search
 * 
 * Query Parameters:
 * - start_date: YYYY-MM-DD (required)
 * - end_date: YYYY-MM-DD (optional, defaults to start_date)
 * - start_time: HH:MM (optional, defaults to 00:00)
 * - end_time: HH:MM (optional, defaults to 23:59)
 * - channel: Channel ID (optional, -1 for all)
 * - type: Recording type filter (optional: continuous, motion, event, manual, schedule)
 * - limit: Max results (optional, default 100)
 * - offset: Pagination offset (optional, default 0)
 * - sort: Sort order (optional: newest, oldest, default newest)
 * 
 * Response:
 * {
 *   "recordings": [...],
 *   "total_count": 150,
 *   "returned_count": 100,
 *   "offset": 0,
 *   "has_more": true
 * }
 */
api::Response HandlePlaybackSearch(const api::RequestContext& ctx);

/**
 * Get recordings grouped by date for calendar view
 * GET /api/v1/playback/calendar
 * 
 * Query Parameters:
 * - year: Year (required)
 * - month: Month 1-12 (required)
 * - channel: Channel ID (optional, -1 for all)
 * 
 * Response:
 * {
 *   "year": 2025,
 *   "month": 12,
 *   "days": {
 *     "1": { "count": 24, "total_duration_sec": 86400, "types": ["continuous"] },
 *     "15": { "count": 48, "total_duration_sec": 7200, "types": ["motion", "manual"] },
 *     ...
 *   }
 * }
 */
api::Response HandlePlaybackCalendar(const api::RequestContext& ctx);

/**
 * Get recording timeline for a specific date
 * GET /api/v1/playback/timeline
 * 
 * Query Parameters:
 * - date: YYYY-MM-DD (required)
 * - channel: Channel ID (optional, -1 for all)
 * 
 * Response:
 * {
 *   "date": "2025-12-21",
 *   "segments": [
 *     { "start": "00:00:00", "end": "01:00:00", "type": "continuous", "recording_id": "..." },
 *     { "start": "01:00:00", "end": "01:05:00", "type": "motion", "recording_id": "..." },
 *     ...
 *   ],
 *   "gaps": [
 *     { "start": "01:05:00", "end": "02:00:00" }
 *   ],
 *   "total_recorded_sec": 3900,
 *   "coverage_percent": 5.4
 * }
 */
api::Response HandlePlaybackTimeline(const api::RequestContext& ctx);

/**
 * Get dates with recordings in a range
 * GET /api/v1/playback/dates
 * 
 * Query Parameters:
 * - start_date: YYYY-MM-DD (required)
 * - end_date: YYYY-MM-DD (required)
 * - channel: Channel ID (optional, -1 for all)
 * 
 * Response:
 * {
 *   "dates": ["2025-12-01", "2025-12-05", "2025-12-21"],
 *   "count": 3
 * }
 */
api::Response HandlePlaybackDates(const api::RequestContext& ctx);

// ============================================================================
// Recording Details and Streaming
// ============================================================================

/**
 * Get detailed recording info with playback URL
 * GET /api/v1/playback/recordings/{id}
 * 
 * Response:
 * {
 *   "id": "...",
 *   "filename": "...",
 *   "channel": 0,
 *   "type": "continuous",
 *   "start_time": 1734789600000,
 *   "end_time": 1734793200000,
 *   "duration_sec": 3600,
 *   "file_size": 524288000,
 *   "width": 1920,
 *   "height": 1080,
 *   "fps": 30,
 *   "codec": "H264",
 *   "encrypted": true,
 *   "playback_url": "/api/v1/playback/stream/{id}",
 *   "download_url": "/api/v1/playback/download/{id}",
 *   "thumbnail_url": "/api/v1/playback/thumbnail/{id}"
 * }
 */
api::Response HandlePlaybackRecordingInfo(const api::RequestContext& ctx);

/**
 * Stream recording for playback (handles Range requests)
 * GET /api/v1/playback/stream/{id}
 * 
 * Supports HTTP Range requests for seeking.
 * Handles decryption transparently.
 * 
 * Response: Binary video data with appropriate Content-Type
 */
api::Response HandlePlaybackStream(const api::RequestContext& ctx);

/**
 * Download recording file
 * GET /api/v1/playback/download/{id}
 * 
 * Response: File download with Content-Disposition header
 */
api::Response HandlePlaybackDownload(const api::RequestContext& ctx);

/**
 * Get recording thumbnail
 * GET /api/v1/playback/thumbnail/{id}
 * 
 * Response: JPEG image
 */
api::Response HandlePlaybackThumbnail(const api::RequestContext& ctx);

// ============================================================================
// Quick Access
// ============================================================================

/**
 * Get recent recordings
 * GET /api/v1/playback/recent
 * 
 * Query Parameters:
 * - limit: Max results (optional, default 20)
 * - channel: Channel ID (optional, -1 for all)
 * 
 * Response:
 * {
 *   "recordings": [...],
 *   "count": 20
 * }
 */
api::Response HandlePlaybackRecent(const api::RequestContext& ctx);

/**
 * Get recording statistics
 * GET /api/v1/playback/stats
 * 
 * Query Parameters:
 * - days: Number of days to analyze (optional, default 30)
 * 
 * Response:
 * {
 *   "total_recordings": 1500,
 *   "total_duration_hours": 720,
 *   "total_size_gb": 150.5,
 *   "by_type": {
 *     "continuous": { "count": 1200, "duration_hours": 600 },
 *     "motion": { "count": 200, "duration_hours": 100 },
 *     ...
 *   },
 *   "by_channel": {
 *     "0": { "count": 800, "duration_hours": 400 },
 *     ...
 *   },
 *   "oldest_recording": "2025-11-21T00:00:00Z",
 *   "newest_recording": "2025-12-21T15:30:00Z"
 * }
 */
api::Response HandlePlaybackStats(const api::RequestContext& ctx);

} // namespace handlers
} // namespace webserver
} // namespace ipcam
