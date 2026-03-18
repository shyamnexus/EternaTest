/**
 * @file onvif_recording_wrapper.cpp
 * @brief C wrapper implementation for ipcamera RecordingService
 */

#include "ipcam/onvif_recording_wrapper.h"
#include <ipcam/recording.h>
#include <cstring>

using namespace ipcam::recording;

extern "C" {

bool onvif_rec_is_available(void) {
    return RecordingService::Instance().IsStorageAvailable();
}

int onvif_rec_get_storage_stats(OnvifStorageStats* stats) {
    if (!stats) return -1;
    
    memset(stats, 0, sizeof(OnvifStorageStats));
    
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return -1;
    }
    
    StorageStats s = svc.GetStorageStats();
    stats->storage_available = (s.status == StorageStatus::Mounted);
    stats->total_bytes = s.total_bytes;
    stats->used_bytes = s.used_bytes;
    stats->free_bytes = s.free_bytes;
    stats->recordings_bytes = s.recordings_bytes;
    stats->total_files = s.total_files;
    strncpy(stats->filesystem, s.filesystem_type.c_str(), sizeof(stats->filesystem) - 1);
    
    return 0;
}

int onvif_rec_get_recording_count(void) {
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return 0;
    }
    
    auto recordings = svc.ListRecordings(-1, 0, 0, std::nullopt, 0, 0);
    return static_cast<int>(recordings.size());
}

int onvif_rec_list_recordings(OnvifRecordingInfo* recordings, int max_count,
                               int channel, int64_t start_time, int64_t end_time) {
    if (!recordings || max_count <= 0) return -1;
    
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return 0;
    }
    
    auto list = svc.ListRecordings(channel, start_time, end_time, std::nullopt, 0, max_count);
    
    int count = 0;
    for (const auto& rec : list) {
        if (count >= max_count) break;
        
        OnvifRecordingInfo* info = &recordings[count];
        memset(info, 0, sizeof(OnvifRecordingInfo));
        
        strncpy(info->id, rec.id.c_str(), sizeof(info->id) - 1);
        strncpy(info->filename, rec.filename.c_str(), sizeof(info->filename) - 1);
        info->channel = rec.channel;
        info->type = static_cast<OnvifRecordingType>(rec.type);
        info->start_time = rec.start_time;
        info->end_time = rec.end_time;
        info->duration_ms = rec.duration_ms;
        info->file_size = rec.file_size;
        info->width = rec.width;
        info->height = rec.height;
        info->fps = rec.fps;
        strncpy(info->codec, rec.codec.c_str(), sizeof(info->codec) - 1);
        info->encrypted = rec.encrypted;
        strncpy(info->status, rec.status.c_str(), sizeof(info->status) - 1);
        
        count++;
    }
    
    return count;
}

int onvif_rec_get_recording(const char* id, OnvifRecordingInfo* info) {
    if (!id || !info) return -1;
    
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return -1;
    }
    
    auto rec = svc.GetRecording(id);
    if (!rec.has_value()) {
        return -1;
    }
    
    memset(info, 0, sizeof(OnvifRecordingInfo));
    strncpy(info->id, rec->id.c_str(), sizeof(info->id) - 1);
    strncpy(info->filename, rec->filename.c_str(), sizeof(info->filename) - 1);
    info->channel = rec->channel;
    info->type = static_cast<OnvifRecordingType>(rec->type);
    info->start_time = rec->start_time;
    info->end_time = rec->end_time;
    info->duration_ms = rec->duration_ms;
    info->file_size = rec->file_size;
    info->width = rec->width;
    info->height = rec->height;
    info->fps = rec->fps;
    strncpy(info->codec, rec->codec.c_str(), sizeof(info->codec) - 1);
    info->encrypted = rec->encrypted;
    strncpy(info->status, rec->status.c_str(), sizeof(info->status) - 1);
    
    return 0;
}

int onvif_rec_get_session(int channel, OnvifRecordingSession* session) {
    if (!session || channel < 0 || channel > 2) return -1;
    
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return -1;
    }
    
    RecordingSession s = svc.GetSession(channel);
    
    memset(session, 0, sizeof(OnvifRecordingSession));
    session->channel = s.channel;
    session->status = static_cast<OnvifRecordingStatus>(s.status);
    session->type = static_cast<OnvifRecordingType>(s.type);
    strncpy(session->current_file, s.current_file.c_str(), sizeof(session->current_file) - 1);
    session->start_time = s.start_time;
    session->bytes_written = s.bytes_written;
    session->segments_completed = s.segments_completed;
    strncpy(session->error_message, s.error_message.c_str(), sizeof(session->error_message) - 1);
    
    return 0;
}

int onvif_rec_get_active_job_count(void) {
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return 0;
    }
    
    auto sessions = svc.GetActiveSessions();
    return static_cast<int>(sessions.size());
}

int onvif_rec_start_recording(int channel, OnvifRecordingType type) {
    if (channel < 0 || channel > 2) return -1;
    
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return -1;
    }
    
    return svc.StartRecording(channel, static_cast<RecordingType>(type)) ? 0 : -1;
}

int onvif_rec_stop_recording(int channel) {
    if (channel < 0 || channel > 2) return -1;
    
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return -1;
    }
    
    return svc.StopRecording(channel) ? 0 : -1;
}

int onvif_rec_delete_recording(const char* id) {
    if (!id) return -1;
    
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return -1;
    }
    
    return svc.DeleteRecording(id) ? 0 : -1;
}

int onvif_rec_get_replay_uri(const char* recording_id, char* uri, int uri_size) {
    if (!recording_id || !uri || uri_size <= 0) return -1;
    
    auto& svc = RecordingService::Instance();
    if (!svc.IsInitialized()) {
        return -1;
    }
    
    auto rec = svc.GetRecording(recording_id);
    if (!rec.has_value()) {
        return -1;
    }
    
    // Get IP address for RTSP URL
    extern char ip_address[32];
    snprintf(uri, uri_size, "rtsp://%s:554/playback?id=%s", ip_address, recording_id);
    
    return 0;
}

} // extern "C"
