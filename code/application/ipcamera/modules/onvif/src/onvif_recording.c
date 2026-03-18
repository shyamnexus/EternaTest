/**
 * @file onvif_recording.c
 * @brief ONVIF Recording Control and Replay Service Implementation (Profile G)
 * 
 * Implements the Recording Control (trc) and Replay (trp) services
 * with actual integration to ipcamera RecordingService.
 */

#include "soapH.h"
#include "soapStub.h"
#include "onvif_main.h"
#include "ipcam/onvif_recording_wrapper.h"

#include <time.h>
#include <string.h>

// Helper to convert Unix timestamp to XSD datetime
static void unix_to_datetime(int64_t unix_ms, struct tm* tm_out) {
    time_t t = unix_ms / 1000;
    gmtime_r(&t, tm_out);
}

// Helper to format datetime as ISO string
static void format_datetime(int64_t unix_ms, char* buffer, int size) {
    struct tm tm;
    unix_to_datetime(unix_ms, &tm);
    strftime(buffer, size, "%Y-%m-%dT%H:%M:%SZ", &tm);
}

// ============================================================================
// Recording Control Service (trc)
// ============================================================================

/** Get Recording Service Capabilities */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetServiceCapabilities(
    struct soap* soap,
    struct _trc__GetServiceCapabilities *trc__GetServiceCapabilities,
    struct _trc__GetServiceCapabilitiesResponse *trc__GetServiceCapabilitiesResponse)
{
    printf("--------------------------__trc__GetServiceCapabilities---------------------------\n");
    
    trc__GetServiceCapabilitiesResponse->Capabilities = 
        (struct trc__Capabilities *)soap_malloc(soap, sizeof(struct trc__Capabilities));
    memset(trc__GetServiceCapabilitiesResponse->Capabilities, 0, sizeof(struct trc__Capabilities));
    
    trc__GetServiceCapabilitiesResponse->Capabilities->DynamicRecordings = 
        (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trc__GetServiceCapabilitiesResponse->Capabilities->DynamicRecordings) = xsd__boolean__true_;
    
    trc__GetServiceCapabilitiesResponse->Capabilities->DynamicTracks = 
        (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trc__GetServiceCapabilitiesResponse->Capabilities->DynamicTracks) = xsd__boolean__true_;
    
    return SOAP_OK;
}

/** Get Recordings - Returns actual recordings from SD card */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordings(
    struct soap* soap,
    struct _trc__GetRecordings *trc__GetRecordings,
    struct _trc__GetRecordingsResponse *trc__GetRecordingsResponse)
{
    printf("--------------------------__trc__GetRecordings---------------------------\n");
    
    // Check if recording service is available
    if (!onvif_rec_is_available()) {
        // Return empty list if no storage
        trc__GetRecordingsResponse->__sizeRecordingItem = 0;
        trc__GetRecordingsResponse->RecordingItem = NULL;
        return SOAP_OK;
    }
    
    // Get recordings from service
    OnvifRecordingInfo recordings[100];
    int count = onvif_rec_list_recordings(recordings, 100, -1, 0, 0);
    
    if (count <= 0) {
        trc__GetRecordingsResponse->__sizeRecordingItem = 0;
        trc__GetRecordingsResponse->RecordingItem = NULL;
        return SOAP_OK;
    }
    
    trc__GetRecordingsResponse->__sizeRecordingItem = count;
    trc__GetRecordingsResponse->RecordingItem = 
        (struct tt__GetRecordingsResponseItem *)soap_malloc(soap, 
            sizeof(struct tt__GetRecordingsResponseItem) * count);
    memset(trc__GetRecordingsResponse->RecordingItem, 0, 
           sizeof(struct tt__GetRecordingsResponseItem) * count);
    
    for (int i = 0; i < count; i++) {
        OnvifRecordingInfo* rec = &recordings[i];
        struct tt__GetRecordingsResponseItem* item = &trc__GetRecordingsResponse->RecordingItem[i];
        
        // Recording token
        item->RecordingToken = (char *)soap_malloc(soap, TOKEN_LENGTH);
        snprintf(item->RecordingToken, TOKEN_LENGTH, "Rec_%s", rec->id);
        
        // Configuration
        item->Configuration = (struct tt__RecordingConfiguration *)soap_malloc(soap, 
            sizeof(struct tt__RecordingConfiguration));
        memset(item->Configuration, 0, sizeof(struct tt__RecordingConfiguration));
        
        // Source information
        item->Configuration->Source = (struct tt__RecordingSourceInformation *)soap_malloc(soap, 
            sizeof(struct tt__RecordingSourceInformation));
        memset(item->Configuration->Source, 0, sizeof(struct tt__RecordingSourceInformation));
        
        item->Configuration->Source->SourceId = (char *)soap_malloc(soap, TOKEN_LENGTH);
        snprintf(item->Configuration->Source->SourceId, TOKEN_LENGTH, "VideoSource_%d", rec->channel);
        
        item->Configuration->Source->Name = (char *)soap_malloc(soap, TOKEN_LENGTH);
        snprintf(item->Configuration->Source->Name, TOKEN_LENGTH, "Channel %d", rec->channel + 1);
        
        item->Configuration->Source->Location = (char *)soap_malloc(soap, TOKEN_LENGTH);
        strcpy(item->Configuration->Source->Location, "Local");
        
        item->Configuration->Source->Description = (char *)soap_malloc(soap, TOKEN_LENGTH);
        snprintf(item->Configuration->Source->Description, TOKEN_LENGTH, 
                 "%dx%d %s %dfps", rec->width, rec->height, rec->codec, rec->fps);
        
        item->Configuration->Source->Address = (char *)soap_malloc(soap, TOKEN_LENGTH);
        sprintf(item->Configuration->Source->Address, "rtsp://%s:554/stream%d", ip_address, rec->channel);
        
        item->Configuration->Content = (char *)soap_malloc(soap, TOKEN_LENGTH);
        strcpy(item->Configuration->Content, rec->encrypted ? "Encrypted Video" : "Video");
        
        item->Configuration->MaximumRetentionTime = "P30D";
        
        // Tracks is optional and has incompatible struct in this gSOAP build - skip
        item->Tracks = NULL;
    }
    
    return SOAP_OK;
}

/** Create Recording */
SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateRecording(
    struct soap* soap,
    struct _trc__CreateRecording *trc__CreateRecording,
    struct _trc__CreateRecordingResponse *trc__CreateRecordingResponse)
{
    printf("--------------------------__trc__CreateRecording---------------------------\n");
    
    static int recording_counter = 1;
    trc__CreateRecordingResponse->RecordingToken = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    sprintf(trc__CreateRecordingResponse->RecordingToken, "RecordingToken_%d", recording_counter++);
    
    return SOAP_OK;
}

/** Delete Recording */
SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteRecording(
    struct soap* soap,
    struct _trc__DeleteRecording *trc__DeleteRecording,
    struct _trc__DeleteRecordingResponse *trc__DeleteRecordingResponse)
{
    printf("--------------------------__trc__DeleteRecording---------------------------\n");
    
    // Extract ID from token (format: "Rec_<id>")
    if (trc__DeleteRecording->RecordingToken) {
        const char* id = trc__DeleteRecording->RecordingToken;
        if (strncmp(id, "Rec_", 4) == 0) {
            id += 4;
        }
        onvif_rec_delete_recording(id);
    }
    
    return SOAP_OK;
}

/** Get Recording Configuration */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingConfiguration(
    struct soap* soap,
    struct _trc__GetRecordingConfiguration *trc__GetRecordingConfiguration,
    struct _trc__GetRecordingConfigurationResponse *trc__GetRecordingConfigurationResponse)
{
    printf("--------------------------__trc__GetRecordingConfiguration---------------------------\n");
    
    trc__GetRecordingConfigurationResponse->RecordingConfiguration = 
        (struct tt__RecordingConfiguration *)soap_malloc(soap, sizeof(struct tt__RecordingConfiguration));
    memset(trc__GetRecordingConfigurationResponse->RecordingConfiguration, 0, 
           sizeof(struct tt__RecordingConfiguration));
    
    trc__GetRecordingConfigurationResponse->RecordingConfiguration->Source = 
        (struct tt__RecordingSourceInformation *)soap_malloc(soap, 
            sizeof(struct tt__RecordingSourceInformation));
    memset(trc__GetRecordingConfigurationResponse->RecordingConfiguration->Source, 0, 
           sizeof(struct tt__RecordingSourceInformation));
    
    trc__GetRecordingConfigurationResponse->RecordingConfiguration->Source->SourceId = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trc__GetRecordingConfigurationResponse->RecordingConfiguration->Source->SourceId, "VideoSource_0");
    
    trc__GetRecordingConfigurationResponse->RecordingConfiguration->Source->Name = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trc__GetRecordingConfigurationResponse->RecordingConfiguration->Source->Name, "Main Camera");
    
    trc__GetRecordingConfigurationResponse->RecordingConfiguration->Content = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trc__GetRecordingConfigurationResponse->RecordingConfiguration->Content, "Video recording");
    
    trc__GetRecordingConfigurationResponse->RecordingConfiguration->MaximumRetentionTime = "P30D";
    
    return SOAP_OK;
}

/** Set Recording Configuration */
SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingConfiguration(
    struct soap* soap,
    struct _trc__SetRecordingConfiguration *trc__SetRecordingConfiguration,
    struct _trc__SetRecordingConfigurationResponse *trc__SetRecordingConfigurationResponse)
{
    printf("--------------------------__trc__SetRecordingConfiguration---------------------------\n");
    return SOAP_OK;
}

/** Get Recording Options */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingOptions(
    struct soap* soap,
    struct _trc__GetRecordingOptions *trc__GetRecordingOptions,
    struct _trc__GetRecordingOptionsResponse *trc__GetRecordingOptionsResponse)
{
    printf("--------------------------__trc__GetRecordingOptions---------------------------\n");
    trc__GetRecordingOptionsResponse->Options = NULL;
    return SOAP_OK;
}

/** Create Track */
SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateTrack(
    struct soap* soap,
    struct _trc__CreateTrack *trc__CreateTrack,
    struct _trc__CreateTrackResponse *trc__CreateTrackResponse)
{
    printf("--------------------------__trc__CreateTrack---------------------------\n");
    
    static int track_counter = 1;
    trc__CreateTrackResponse->TrackToken = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    sprintf(trc__CreateTrackResponse->TrackToken, "TrackToken_%d", track_counter++);
    
    return SOAP_OK;
}

/** Delete Track */
SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteTrack(
    struct soap* soap,
    struct _trc__DeleteTrack *trc__DeleteTrack,
    struct _trc__DeleteTrackResponse *trc__DeleteTrackResponse)
{
    printf("--------------------------__trc__DeleteTrack---------------------------\n");
    return SOAP_OK;
}

/** Get Track Configuration */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetTrackConfiguration(
    struct soap* soap,
    struct _trc__GetTrackConfiguration *trc__GetTrackConfiguration,
    struct _trc__GetTrackConfigurationResponse *trc__GetTrackConfigurationResponse)
{
    printf("--------------------------__trc__GetTrackConfiguration---------------------------\n");
    
    trc__GetTrackConfigurationResponse->TrackConfiguration = 
        (struct tt__TrackConfiguration *)soap_malloc(soap, sizeof(struct tt__TrackConfiguration));
    memset(trc__GetTrackConfigurationResponse->TrackConfiguration, 0, sizeof(struct tt__TrackConfiguration));
    
    trc__GetTrackConfigurationResponse->TrackConfiguration->TrackType = tt__TrackType__Video;
    trc__GetTrackConfigurationResponse->TrackConfiguration->Description = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trc__GetTrackConfigurationResponse->TrackConfiguration->Description, "Video track");
    
    return SOAP_OK;
}

/** Set Track Configuration */
SOAP_FMAC5 int SOAP_FMAC6 __trc__SetTrackConfiguration(
    struct soap* soap,
    struct _trc__SetTrackConfiguration *trc__SetTrackConfiguration,
    struct _trc__SetTrackConfigurationResponse *trc__SetTrackConfigurationResponse)
{
    printf("--------------------------__trc__SetTrackConfiguration---------------------------\n");
    return SOAP_OK;
}

/** Create Recording Job - Starts actual recording */
SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateRecordingJob(
    struct soap* soap,
    struct _trc__CreateRecordingJob *trc__CreateRecordingJob,
    struct _trc__CreateRecordingJobResponse *trc__CreateRecordingJobResponse)
{
    printf("--------------------------__trc__CreateRecordingJob---------------------------\n");
    
    // Parse channel from source token (e.g., "VideoSource_0")
    int channel = 0;
    if (trc__CreateRecordingJob->JobConfiguration && 
        trc__CreateRecordingJob->JobConfiguration->__sizeSource > 0) {
        // Try to extract channel from first source
        // Expected format: VideoSource_N
        const char* src = trc__CreateRecordingJob->JobConfiguration->Source[0].SourceToken->Token;
        if (src && sscanf(src, "VideoSource_%d", &channel) != 1) {
            channel = 0;
        }
    }
    
    // Start recording
    OnvifRecordingType type = ONVIF_REC_TYPE_MANUAL;
    if (trc__CreateRecordingJob->JobConfiguration && 
        trc__CreateRecordingJob->JobConfiguration->Mode) {
        if (strcmp(trc__CreateRecordingJob->JobConfiguration->Mode, "Active") == 0 ||
            strcmp(trc__CreateRecordingJob->JobConfiguration->Mode, "Continuous") == 0) {
            type = ONVIF_REC_TYPE_CONTINUOUS;
        }
    }
    
    onvif_rec_start_recording(channel, type);
    
    static int job_counter = 1;
    trc__CreateRecordingJobResponse->JobToken = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    sprintf(trc__CreateRecordingJobResponse->JobToken, "RecordingJob_%d", job_counter++);
    
    trc__CreateRecordingJobResponse->JobConfiguration = 
        (struct tt__RecordingJobConfiguration *)soap_malloc(soap, sizeof(struct tt__RecordingJobConfiguration));
    memset(trc__CreateRecordingJobResponse->JobConfiguration, 0, sizeof(struct tt__RecordingJobConfiguration));
    
    if (trc__CreateRecordingJob->JobConfiguration && 
        trc__CreateRecordingJob->JobConfiguration->RecordingToken) {
        trc__CreateRecordingJobResponse->JobConfiguration->RecordingToken = 
            (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        strcpy(trc__CreateRecordingJobResponse->JobConfiguration->RecordingToken, 
               trc__CreateRecordingJob->JobConfiguration->RecordingToken);
        trc__CreateRecordingJobResponse->JobConfiguration->Mode = "Active";
        trc__CreateRecordingJobResponse->JobConfiguration->Priority = 
            trc__CreateRecordingJob->JobConfiguration->Priority > 0 ? 
            trc__CreateRecordingJob->JobConfiguration->Priority : 1;
    }
    
    return SOAP_OK;
}

/** Delete Recording Job - Stops recording */
SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteRecordingJob(
    struct soap* soap,
    struct _trc__DeleteRecordingJob *trc__DeleteRecordingJob,
    struct _trc__DeleteRecordingJobResponse *trc__DeleteRecordingJobResponse)
{
    printf("--------------------------__trc__DeleteRecordingJob---------------------------\n");
    
    // Parse job token to get channel (format: "RecordingJob_N")
    int job_id = 0;
    if (trc__DeleteRecordingJob->JobToken) {
        sscanf(trc__DeleteRecordingJob->JobToken, "RecordingJob_%d", &job_id);
    }
    
    // Stop all channels for now (job-to-channel mapping would need persistent storage)
    for (int ch = 0; ch < 3; ch++) {
        onvif_rec_stop_recording(ch);
    }
    
    return SOAP_OK;
}

/** Get Recording Jobs - Returns active recording sessions */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobs(
    struct soap* soap,
    struct _trc__GetRecordingJobs *trc__GetRecordingJobs,
    struct _trc__GetRecordingJobsResponse *trc__GetRecordingJobsResponse)
{
    printf("--------------------------__trc__GetRecordingJobs---------------------------\n");
    
    int active_count = onvif_rec_get_active_job_count();
    
    if (active_count <= 0) {
        trc__GetRecordingJobsResponse->__sizeJobItem = 0;
        trc__GetRecordingJobsResponse->JobItem = NULL;
        return SOAP_OK;
    }
    
    trc__GetRecordingJobsResponse->__sizeJobItem = active_count;
    trc__GetRecordingJobsResponse->JobItem = 
        (struct tt__GetRecordingJobsResponseItem *)soap_malloc(soap, 
            sizeof(struct tt__GetRecordingJobsResponseItem) * active_count);
    memset(trc__GetRecordingJobsResponse->JobItem, 0, 
           sizeof(struct tt__GetRecordingJobsResponseItem) * active_count);
    
    int job_idx = 0;
    for (int ch = 0; ch < 3 && job_idx < active_count; ch++) {
        OnvifRecordingSession session;
        if (onvif_rec_get_session(ch, &session) == 0 && 
            session.status == ONVIF_REC_STATUS_RECORDING) {
            
            struct tt__GetRecordingJobsResponseItem* item = 
                &trc__GetRecordingJobsResponse->JobItem[job_idx];
            
            item->JobToken = (char *)soap_malloc(soap, TOKEN_LENGTH);
            sprintf(item->JobToken, "RecordingJob_CH%d", ch);
            
            item->JobConfiguration = (struct tt__RecordingJobConfiguration *)soap_malloc(soap, 
                sizeof(struct tt__RecordingJobConfiguration));
            memset(item->JobConfiguration, 0, sizeof(struct tt__RecordingJobConfiguration));
            
            item->JobConfiguration->RecordingToken = (char *)soap_malloc(soap, TOKEN_LENGTH);
            sprintf(item->JobConfiguration->RecordingToken, "Recording_CH%d", ch);
            
            item->JobConfiguration->Mode = "Active";
            item->JobConfiguration->Priority = 1;
            
            job_idx++;
        }
    }
    
    // Adjust count if some channels weren't recording
    trc__GetRecordingJobsResponse->__sizeJobItem = job_idx;
    
    return SOAP_OK;
}

/** Get Recording Job Configuration */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobConfiguration(
    struct soap* soap,
    struct _trc__GetRecordingJobConfiguration *trc__GetRecordingJobConfiguration,
    struct _trc__GetRecordingJobConfigurationResponse *trc__GetRecordingJobConfigurationResponse)
{
    printf("--------------------------__trc__GetRecordingJobConfiguration---------------------------\n");
    
    trc__GetRecordingJobConfigurationResponse->JobConfiguration = 
        (struct tt__RecordingJobConfiguration *)soap_malloc(soap, sizeof(struct tt__RecordingJobConfiguration));
    memset(trc__GetRecordingJobConfigurationResponse->JobConfiguration, 0, 
           sizeof(struct tt__RecordingJobConfiguration));
    
    trc__GetRecordingJobConfigurationResponse->JobConfiguration->RecordingToken = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trc__GetRecordingJobConfigurationResponse->JobConfiguration->RecordingToken, "RecordingToken_1");
    
    trc__GetRecordingJobConfigurationResponse->JobConfiguration->Mode = "Active";
    trc__GetRecordingJobConfigurationResponse->JobConfiguration->Priority = 1;
    
    return SOAP_OK;
}

/** Set Recording Job Configuration */
SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingJobConfiguration(
    struct soap* soap,
    struct _trc__SetRecordingJobConfiguration *trc__SetRecordingJobConfiguration,
    struct _trc__SetRecordingJobConfigurationResponse *trc__SetRecordingJobConfigurationResponse)
{
    printf("--------------------------__trc__SetRecordingJobConfiguration---------------------------\n");
    
    trc__SetRecordingJobConfigurationResponse->JobConfiguration = 
        (struct tt__RecordingJobConfiguration *)soap_malloc(soap, sizeof(struct tt__RecordingJobConfiguration));
    memset(trc__SetRecordingJobConfigurationResponse->JobConfiguration, 0, 
           sizeof(struct tt__RecordingJobConfiguration));
    
    if (trc__SetRecordingJobConfiguration->JobConfiguration && 
        trc__SetRecordingJobConfiguration->JobConfiguration->RecordingToken) {
        trc__SetRecordingJobConfigurationResponse->JobConfiguration->RecordingToken = 
            (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        strcpy(trc__SetRecordingJobConfigurationResponse->JobConfiguration->RecordingToken, 
               trc__SetRecordingJobConfiguration->JobConfiguration->RecordingToken);
        trc__SetRecordingJobConfigurationResponse->JobConfiguration->Mode = 
            trc__SetRecordingJobConfiguration->JobConfiguration->Mode;
        trc__SetRecordingJobConfigurationResponse->JobConfiguration->Priority = 
            trc__SetRecordingJobConfiguration->JobConfiguration->Priority;
    }
    
    return SOAP_OK;
}

/** Set Recording Job Mode */
SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingJobMode(
    struct soap* soap,
    struct _trc__SetRecordingJobMode *trc__SetRecordingJobMode,
    struct _trc__SetRecordingJobModeResponse *trc__SetRecordingJobModeResponse)
{
    printf("--------------------------__trc__SetRecordingJobMode---------------------------\n");
    return SOAP_OK;
}

/** Get Recording Job State - Returns actual recording state */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobState(
    struct soap* soap,
    struct _trc__GetRecordingJobState *trc__GetRecordingJobState,
    struct _trc__GetRecordingJobStateResponse *trc__GetRecordingJobStateResponse)
{
    printf("--------------------------__trc__GetRecordingJobState---------------------------\n");
    
    // Parse channel from job token (format: "RecordingJob_CH0")
    int channel = 0;
    if (trc__GetRecordingJobState->JobToken) {
        if (sscanf(trc__GetRecordingJobState->JobToken, "RecordingJob_CH%d", &channel) != 1) {
            sscanf(trc__GetRecordingJobState->JobToken, "RecordingJob_%d", &channel);
        }
    }
    
    OnvifRecordingSession session;
    onvif_rec_get_session(channel, &session);
    
    trc__GetRecordingJobStateResponse->State = 
        (struct tt__RecordingJobStateInformation *)soap_malloc(soap, 
            sizeof(struct tt__RecordingJobStateInformation));
    memset(trc__GetRecordingJobStateResponse->State, 0, sizeof(struct tt__RecordingJobStateInformation));
    
    trc__GetRecordingJobStateResponse->State->RecordingToken = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    sprintf(trc__GetRecordingJobStateResponse->State->RecordingToken, "Recording_CH%d", channel);
    
    switch (session.status) {
        case ONVIF_REC_STATUS_RECORDING:
            trc__GetRecordingJobStateResponse->State->State = "Active";
            break;
        case ONVIF_REC_STATUS_PAUSED:
            trc__GetRecordingJobStateResponse->State->State = "Paused";
            break;
        case ONVIF_REC_STATUS_ERROR:
            trc__GetRecordingJobStateResponse->State->State = "Error";
            break;
        default:
            trc__GetRecordingJobStateResponse->State->State = "Idle";
            break;
    }
    
    // Note: Error field is not available in this gSOAP struct definition
    
    return SOAP_OK;
}

/** Export Recorded Data */
SOAP_FMAC5 int SOAP_FMAC6 __trc__ExportRecordedData(
    struct soap* soap,
    struct _trc__ExportRecordedData *trc__ExportRecordedData,
    struct _trc__ExportRecordedDataResponse *trc__ExportRecordedDataResponse)
{
    printf("--------------------------__trc__ExportRecordedData---------------------------\n");
    
    static int export_counter = 1;
    trc__ExportRecordedDataResponse->OperationToken = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    sprintf(trc__ExportRecordedDataResponse->OperationToken, "Export_%d", export_counter++);
    
    trc__ExportRecordedDataResponse->__sizeFileNames = 0;
    trc__ExportRecordedDataResponse->FileNames = NULL;
    
    return SOAP_OK;
}

/** Stop Export Recorded Data */
SOAP_FMAC5 int SOAP_FMAC6 __trc__StopExportRecordedData(
    struct soap* soap,
    struct _trc__StopExportRecordedData *trc__StopExportRecordedData,
    struct _trc__StopExportRecordedDataResponse *trc__StopExportRecordedDataResponse)
{
    printf("--------------------------__trc__StopExportRecordedData---------------------------\n");
    return SOAP_OK;
}

/** Get Export Recorded Data State */
SOAP_FMAC5 int SOAP_FMAC6 __trc__GetExportRecordedDataState(
    struct soap* soap,
    struct _trc__GetExportRecordedDataState *trc__GetExportRecordedDataState,
    struct _trc__GetExportRecordedDataStateResponse *trc__GetExportRecordedDataStateResponse)
{
    printf("--------------------------__trc__GetExportRecordedDataState---------------------------\n");
    trc__GetExportRecordedDataStateResponse->Progress = 100.0f;
    return SOAP_OK;
}

// ============================================================================
// Replay Service (trp)
// ============================================================================

/** Get Replay Service Capabilities */
SOAP_FMAC5 int SOAP_FMAC6 __trp__GetServiceCapabilities(
    struct soap* soap,
    struct _trp__GetServiceCapabilities *trp__GetServiceCapabilities,
    struct _trp__GetServiceCapabilitiesResponse *trp__GetServiceCapabilitiesResponse)
{
    printf("--------------------------__trp__GetServiceCapabilities---------------------------\n");
    
    trp__GetServiceCapabilitiesResponse->Capabilities = 
        (struct trp__Capabilities *)soap_malloc(soap, sizeof(struct trp__Capabilities));
    memset(trp__GetServiceCapabilitiesResponse->Capabilities, 0, sizeof(struct trp__Capabilities));
    
    trp__GetServiceCapabilitiesResponse->Capabilities->ReversePlayback = 
        (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trp__GetServiceCapabilitiesResponse->Capabilities->ReversePlayback) = xsd__boolean__false_;
    
    trp__GetServiceCapabilitiesResponse->Capabilities->SessionTimeoutRange = 
        (char *)soap_malloc(soap, sizeof(char) * 32);
    strcpy(trp__GetServiceCapabilitiesResponse->Capabilities->SessionTimeoutRange, "PT10S PT300S");
    
    return SOAP_OK;
}

/** Get Replay URI - Returns actual playback URI */
SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayUri(
    struct soap* soap,
    struct _trp__GetReplayUri *trp__GetReplayUri,
    struct _trp__GetReplayUriResponse *trp__GetReplayUriResponse)
{
    printf("--------------------------__trp__GetReplayUri---------------------------\n");
    
    // *** DYNAMIC IP REFRESH ***
    get_ip_address();
    
    trp__GetReplayUriResponse->Uri = (char *)soap_malloc(soap, sizeof(char) * 256);
    
    // Extract recording ID from token
    const char* rec_token = trp__GetReplayUri->RecordingToken;
    if (rec_token && strncmp(rec_token, "Rec_", 4) == 0) {
        onvif_rec_get_replay_uri(rec_token + 4, trp__GetReplayUriResponse->Uri, 256);
    } else if (rec_token) {
        onvif_rec_get_replay_uri(rec_token, trp__GetReplayUriResponse->Uri, 256);
    } else {
        sprintf(trp__GetReplayUriResponse->Uri, "rtsp://%s:554/playback", ip_address);
    }
    
    return SOAP_OK;
}

/** Get Replay Configuration */
SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayConfiguration(
    struct soap* soap,
    struct _trp__GetReplayConfiguration *trp__GetReplayConfiguration,
    struct _trp__GetReplayConfigurationResponse *trp__GetReplayConfigurationResponse)
{
    printf("--------------------------__trp__GetReplayConfiguration---------------------------\n");
    
    trp__GetReplayConfigurationResponse->Configuration = 
        (struct tt__ReplayConfiguration *)soap_malloc(soap, sizeof(struct tt__ReplayConfiguration));
    memset(trp__GetReplayConfigurationResponse->Configuration, 0, sizeof(struct tt__ReplayConfiguration));
    
    trp__GetReplayConfigurationResponse->Configuration->SessionTimeout = "PT60S";
    
    return SOAP_OK;
}

/** Set Replay Configuration */
SOAP_FMAC5 int SOAP_FMAC6 __trp__SetReplayConfiguration(
    struct soap* soap,
    struct _trp__SetReplayConfiguration *trp__SetReplayConfiguration,
    struct _trp__SetReplayConfigurationResponse *trp__SetReplayConfigurationResponse)
{
    printf("--------------------------__trp__SetReplayConfiguration---------------------------\n");
    return SOAP_OK;
}
