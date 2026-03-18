/**
 * @file onvif_recording_wrapper.h
 * @brief C wrapper for ipcamera RecordingService for ONVIF integration
 */

#ifndef ONVIF_RECORDING_WRAPPER_H
#define ONVIF_RECORDING_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Types
// ============================================================================

typedef enum {
    ONVIF_REC_TYPE_CONTINUOUS = 0,
    ONVIF_REC_TYPE_MOTION = 1,
    ONVIF_REC_TYPE_EVENT = 2,
    ONVIF_REC_TYPE_MANUAL = 3,
    ONVIF_REC_TYPE_SCHEDULE = 4
} OnvifRecordingType;

typedef enum {
    ONVIF_REC_STATUS_IDLE = 0,
    ONVIF_REC_STATUS_RECORDING = 1,
    ONVIF_REC_STATUS_PAUSED = 2,
    ONVIF_REC_STATUS_ERROR = 3
} OnvifRecordingStatus;

typedef struct {
    char id[64];
    char filename[256];
    int channel;
    OnvifRecordingType type;
    int64_t start_time;
    int64_t end_time;
    int64_t duration_ms;
    int64_t file_size;
    int width;
    int height;
    int fps;
    char codec[16];
    bool encrypted;
    char status[16];
} OnvifRecordingInfo;

typedef struct {
    bool storage_available;
    int64_t total_bytes;
    int64_t used_bytes;
    int64_t free_bytes;
    int64_t recordings_bytes;
    int total_files;
    char filesystem[16];
} OnvifStorageStats;

typedef struct {
    int channel;
    OnvifRecordingStatus status;
    OnvifRecordingType type;
    char current_file[256];
    int64_t start_time;
    int64_t bytes_written;
    int segments_completed;
    char error_message[256];
} OnvifRecordingSession;

// ============================================================================
// Functions
// ============================================================================

/**
 * Check if recording service is available
 */
bool onvif_rec_is_available(void);

/**
 * Get storage statistics
 */
int onvif_rec_get_storage_stats(OnvifStorageStats* stats);

/**
 * Get number of recordings
 */
int onvif_rec_get_recording_count(void);

/**
 * List recordings
 * @param recordings Output array (caller allocates)
 * @param max_count Maximum number to return
 * @param channel Filter by channel (-1 for all)
 * @param start_time Filter by start time (0 for no filter)
 * @param end_time Filter by end time (0 for no filter)
 * @return Number of recordings returned
 */
int onvif_rec_list_recordings(OnvifRecordingInfo* recordings, int max_count,
                               int channel, int64_t start_time, int64_t end_time);

/**
 * Get recording by ID
 */
int onvif_rec_get_recording(const char* id, OnvifRecordingInfo* info);

/**
 * Get current recording session for channel
 */
int onvif_rec_get_session(int channel, OnvifRecordingSession* session);

/**
 * Get number of active recording jobs
 */
int onvif_rec_get_active_job_count(void);

/**
 * Start recording on channel
 */
int onvif_rec_start_recording(int channel, OnvifRecordingType type);

/**
 * Stop recording on channel
 */
int onvif_rec_stop_recording(int channel);

/**
 * Delete a recording by ID
 */
int onvif_rec_delete_recording(const char* id);

/**
 * Get replay URI for a recording
 */
int onvif_rec_get_replay_uri(const char* recording_id, char* uri, int uri_size);

#ifdef __cplusplus
}
#endif

#endif // ONVIF_RECORDING_WRAPPER_H
