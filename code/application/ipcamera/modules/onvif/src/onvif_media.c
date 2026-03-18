// Include gSOAP headers first to define struct soap
#include "soapH.h"
#include "soapStub.h"
#include "wsaapi.h"
// #include "wsseapi-lite.h"
// #include "wsdd.nsmap"
#include "wsddapi.h"

// Then include ONVIF headers
#include "onvif_main.h"
#include "onvif_media.h"
#include "onvif_adapter.h"
#include "onvif_token_defs.h"
#include "ipcam/video_control_wrapper.h"
#include "ipcam/rtsp_server_onvif.h"  // C interface for dynamic RTSP stream URI generation

// TODO: Add Novatek-specific includes when implementing media profile
// #include <rockit.h>
//#include "rkadk_media_comm.h"
// #include "isp.h"
// #include "osd.h"
// #include "param.h"
// #include "video.h"
// #include "model.h"
// #include "socket_functions.h"
#include <sqlite3.h>

/* Definitions for extern variables declared in onvif_media.h */
char *timeout = "PT1M";
bool testProfilecreated = false;
bool testProfileAddSourceConfig = false;
bool testProfileAddVideoEncoderConfig = false;
bool testProfileAddMetaDataConfig = false;

//#define TOKEN_LENGTH 64
// #define MAX_BITRATE_CHN_0   1024//8192//1024
// #define MAX_BITRATE_CHN_1   512

#define LUCKFOX
#define INFO_LENGTH 100

typedef char *					    CHARPTR;
typedef char CHAR;


//Video Source data configuration structure 
char vsrc_confToken[TOKEN_LENGTH];
char vsrc_srcToken[TOKEN_LENGTH];
char vsrc_name[TOKEN_LENGTH];
char mdata_confToken[TOKEN_LENGTH];
char mdata_name[TOKEN_LENGTH];
static int onvif_media_tokens_initialized = 0;

/**
 * @brief Initialize global token variables from centralized definitions.
 *        Must be called early in the ONVIF initialization path.
 */
static void onvif_media_init_tokens(void) {
    if (onvif_media_tokens_initialized) return;
    ONVIF_TOKEN_GEN(vsrc_confToken, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_SOURCE_CONFIG, 0);
    ONVIF_TOKEN_GEN(vsrc_srcToken,  TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_SOURCE, 0);
    snprintf(vsrc_name, TOKEN_LENGTH, "%s", ONVIF_VIDEO_SOURCE_CONFIG_NAME);
    ONVIF_TOKEN_GEN(mdata_confToken, TOKEN_LENGTH, ONVIF_TOKEN_FMT_METADATA, 1);
    ONVIF_TOKEN_GEN(mdata_name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_METADATA_NAME, 1);
    onvif_media_tokens_initialized = 1;
}
int vsrc_useCount = 3;
int vsrc_maxNumOfProfiles =3;
int vsrc_boundX =0;
int vsrc_minX =0;
int vsrc_maxX = 10;
int vsrc_boundY = 0;
int vsrc_minY =0;
int vsrc_maxY=10;
int vsrc_boundWidth=4000;
int vsrc_minboundWidth=0;
int vsrc_maxboundWidth= 4000;
int vsrc_boundHeight = 3000;
int vsrc_minboundHeight =0;
int vsrc_maxboundHeight = 3000;

//Meta Data configuration structure
int mdata_useCount = 1;
int mdata_port = 0;
int mdata_TTL = 64;
bool mdata_autoStart = false;
char mdata_IPV4Addr[IPV4_ADDR_STRING_LEN+1];
//char *mdata_sessionTimeout ="PT1M";
char mdata_sessionTimeout[TOKEN_LENGTH] ="PT1M";
char invalid_timeout[TOKEN_LENGTH] = "invalid";


/** Web service operation '__trt__GetVideoSources' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap* soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
     printf("----------------inside trt_GetVideoSources------------------------\n");
    onvif_media_init_tokens();
    /* Also init metadata tokens */
    ONVIF_TOKEN_GEN(mdata_confToken, TOKEN_LENGTH, ONVIF_TOKEN_FMT_METADATA, 1);
    ONVIF_TOKEN_GEN(mdata_name,      TOKEN_LENGTH, ONVIF_TOKEN_FMT_METADATA_NAME, 1);
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    //For Single video source, __sizeVideoSources should be 1 only
    trt__GetVideoSourcesResponse->__sizeVideoSources = 1;
    
       
    trt__GetVideoSourcesResponse->VideoSources =
    (struct tt__VideoSource *)soap_malloc(soap, (trt__GetVideoSourcesResponse->__sizeVideoSources * sizeof(struct tt__VideoSource)));
    MEMSET(trt__GetVideoSourcesResponse->VideoSources, 0x00, trt__GetVideoSourcesResponse->__sizeVideoSources * sizeof(struct tt__VideoSource));

//----------------------------video source 0------------------------------------------------------------------
    trt__GetVideoSourcesResponse->VideoSources->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetVideoSourcesResponse->VideoSources->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetVideoSourcesResponse->VideoSources->token, vsrc_srcToken);

    // Get framerate from video API
    int framerate = 30;
    nvt_adapter_video_get_frame_rate(0, &framerate);
    trt__GetVideoSourcesResponse->VideoSources->Framerate = framerate;

    trt__GetVideoSourcesResponse->VideoSources->Resolution =
        (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    MEMSET(trt__GetVideoSourcesResponse->VideoSources->Resolution, 0x00, sizeof(struct tt__VideoResolution));

    char *value = malloc(20);
    nvt_adapter_video_get_resolution(0, value);  // Fixed: pass value directly, not &value
    sscanf(value, "%d*%d", &trt__GetVideoSourcesResponse->VideoSources[0].Resolution->Width, &trt__GetVideoSourcesResponse->VideoSources[0].Resolution->Height);

    free(value);
   ///////////////////////////
    /** Optional element 'tt:Imaging' of XML schema type 'tt:ImagingSettings' */
   // trt__GetVideoSourcesResponse->VideoSources[0].Imaging = NULL;
       /** Optional element 'tt:Imaging' of XML schema type 'tt:ImagingSettings' */
        trt__GetVideoSourcesResponse->VideoSources->Imaging = (struct tt__ImagingSettings *) soap_malloc(soap, sizeof(struct tt__ImagingSettings));
        MEMSET(trt__GetVideoSourcesResponse->VideoSources->Imaging, 0x00, sizeof(struct tt__ImagingSettings));
 
        // /* Backlight compensation not used currently */
        // /** Optional element 'tt:BacklightCompensation' of XML schema type 'tt:BacklightCompensation' */
        trt__GetVideoSourcesResponse->VideoSources->Imaging->BacklightCompensation = NULL;
        // /** Optional element 'tt:Brightness' of XML schema type 'xsd:float' */
        trt__GetVideoSourcesResponse->VideoSources->Imaging->Brightness = (float *) soap_malloc(soap, sizeof(float));
        *trt__GetVideoSourcesResponse->VideoSources->Imaging->Brightness = 128;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->ColorSaturation = (float *) soap_malloc(soap, sizeof(float));
        *trt__GetVideoSourcesResponse->VideoSources->Imaging->ColorSaturation = 128;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->Contrast = (float *) soap_malloc(soap, sizeof(float));
        *trt__GetVideoSourcesResponse->VideoSources->Imaging->Contrast = 128;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->Sharpness = (float *) soap_malloc(soap, sizeof(float));
        *trt__GetVideoSourcesResponse->VideoSources->Imaging->Sharpness = 128;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure = NULL;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->Focus = NULL;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->IrCutFilter = NULL;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->WideDynamicRange = NULL;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->WhiteBalance = NULL;
        trt__GetVideoSourcesResponse->VideoSources->Imaging->Extension = NULL;
        trt__GetVideoSourcesResponse->VideoSources->Extension = NULL;

    return 0;
}

/** Web service operation '__trt__GetAudioSources' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap* soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
    printf("--------------------------__trt__GetAudioSources---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // Return single audio source (built-in microphone)
    trt__GetAudioSourcesResponse->__sizeAudioSources = 1;
    trt__GetAudioSourcesResponse->AudioSources = 
        (struct tt__AudioSource *)soap_malloc(soap, sizeof(struct tt__AudioSource));
    memset(trt__GetAudioSourcesResponse->AudioSources, 0, sizeof(struct tt__AudioSource));

    trt__GetAudioSourcesResponse->AudioSources[0].token = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetAudioSourcesResponse->AudioSources[0].token, "AudioSource_1");

    trt__GetAudioSourcesResponse->AudioSources[0].Channels = 1;  // Mono

    return SOAP_OK;
}

/** Web service operation '__trt__GetAudioOutputs' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap* soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__trt__GetProfile' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap* soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
    printf("-------------------------------__trt__GetProfile-------------------------------------\n");
    onvif_media_init_tokens();
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int profIndex = -1;
    char *value = malloc(20);
    const char *profile_value;
    sqlite3 *db;
	char *sql;
	int rc;
    int active, rule_count = 0;
	sqlite3_stmt *res;
    const char* AnalyticsModuleTypes[] = {"tt:TripWireDetectionEngine","tt:TrespassingDetectionEngine","tt:MotionDetectionEngine","tt:TamperDetectionEngine"};
	const char* AnalyticsModuleName[] = {"TripWireDetectionModule","TrespassingDetectionModule","MotionDetectionModule","TamperDetectionModule"};

    const char *parameters[] = {"label","active","coordinates"};
    size_t parameter_length = sizeof(parameters) / sizeof(parameters[0]);


    /* LEGACY DB DISABLED: /oem/usr/share/usermng.db no longer exists.
	 * Analytics will return default values. */
	db = NULL;
	rc = SQLITE_OK;
    // rc = sqlite3_open("/oem/usr/share/usermng.db", &db);
	// if( rc ) {
	// 	printf("==========Can't open database: %s (analytics disabled)==========\n", sqlite3_errmsg(db));
	// 	db = NULL;  // Continue without database - analytics will be disabled
	// }

    sql = "SELECT active FROM analytics_modules WHERE name = ?";


    if (NULL != trt__GetProfile)
    {
        if (NULL != trt__GetProfile->ProfileToken)
        {
            printf("Given profile token for __trt__GetProfile is = %s\n", trt__GetProfile->ProfileToken);

            profIndex = onvif_profile_token_to_channel(trt__GetProfile->ProfileToken);
            if (profIndex < 0 && (0 == strncmp("Test_Profile", trt__GetProfile->ProfileToken, TOKEN_LENGTH)) && testProfilecreated == true)
            {
                printf("Profile index match and flag is true\n");
                profIndex = 3;
            }

            
            if (-1 == profIndex)
            {
                onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
                printf("Given profile token not found is = %s\n", trt__GetProfile->ProfileToken);
                return SOAP_FAULT;
            }
            else
            {
                printf("Given profile token found Index = %d\n", profIndex);
            }
            
        }
    }
    if(profIndex == 0)
    {
        trt__GetProfileResponse->Profile = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile));
        MEMSET(trt__GetProfileResponse->Profile, 0x00, sizeof(struct tt__Profile));

    
        trt__GetProfileResponse->Profile->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_PROFILE, 1);

        trt__GetProfileResponse->Profile->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->Name, ONVIF_PROFILE_NAME_MAIN);

         trt__GetProfileResponse->Profile->fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(trt__GetProfileResponse->Profile->fixed) = xsd__boolean__true_;

        /// Video Source Configuration

      //   printf("-------------------------------VideoSourceConfiguration-------------------------------------\n");

        trt__GetProfileResponse->Profile->VideoSourceConfiguration =
                        (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration, 0x00, sizeof(struct tt__VideoSourceConfiguration));

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->token,vsrc_confToken);

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name,vsrc_name);

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->UseCount = vsrc_useCount;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken,vsrc_srcToken);
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
         trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->y = vsrc_boundX;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->x = vsrc_boundY;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->width = vsrc_boundWidth;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->height = vsrc_boundHeight;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->__size = 0;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->__any = NULL;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Extension = NULL; //TODO: Check for rotate

    
        /* AudioSource-Configuration */
        trt__GetProfileResponse->Profile->AudioSourceConfiguration = NULL;
        /* Video Encoder Configuration */
        //   printf("-------------------------------VideoEncoderConfiguration-------------------------------------\n");
    
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration =
            (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 1);

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 1);

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->UseCount = 1;

        /* TODO: GauranteedFrameRate : Currently no need */

        // Detect codec type dynamically (2=H264, 3=H265)
        int codec_type = 2;
        nvt_adapter_video_get_video_codec(0, &codec_type);
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Encoding = codec_type;
        
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution, 0x00, sizeof(struct tt__VideoResolution));
 
        nvt_adapter_video_get_resolution(0, value);
        sscanf(value, "%d*%d", &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Width, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Height);

    
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Quality = 10.0;

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl, 0x00, sizeof(struct tt__VideoRateControl));

        nvt_adapter_video_get_frame_rate(0, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit);
      //  free(value);
        /* TODO: EncodingInterval: Following parameter create confusion */
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = 1;
        nvt_adapter_video_get_max_rate(0,&trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->BitrateLimit);

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4 = NULL;
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264, 0x00, sizeof(struct tt__H264Configuration));
        /* Determines the interval in which the I-Frames will be coded */
        nvt_adapter_video_get_gop(0,&trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->GovLength);

        //  trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main; //tt__H264Profile__Baseline,tt__H264Profile__High,tt__H264Profile__Main;
        int h264_profile_val = 0;
        nvt_adapter_video_get_h264_profile(0, &h264_profile_val);
        printf("H264 PROFILE value profile 0----00 %d\n", h264_profile_val);
        if(h264_profile_val == 0)
        {    
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Baseline;
        }
        else if(h264_profile_val == 1)
        {
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__High;
        }
        else 
        {
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main;
        }
        free(value);
    
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast =
            (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
         
     //  trt__GetProfileResponse->Profile[i].VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //Currently only support of IPv4
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->Type = 0;

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         strcpy(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address,"0.0.0.0");
        
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->SessionTimeout = timeout; 
        

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->__size = 0;
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->__any = NULL;
     //  dom_att(&trt__GetProfileResponse->Profile[0].VideoEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        //Video Encoder Configuration

        /* AudioEncoderConfiguration */
        trt__GetProfileResponse->Profile->AudioEncoderConfiguration = NULL;
        // AudioEncoderConfiguration 

        //Metadata Configurations
        trt__GetProfileResponse->Profile->MetadataConfiguration = NULL;

        // /* VideoAnalyticsConfiguration */
        // Skip analytics configuration if database is not available
        if (db == NULL) {
            printf("Database not available - skipping analytics configuration for Profile %d\n", profIndex);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
            goto skip_analytics_profile;
        }
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration, 0x00, sizeof(struct tt__VideoAnalyticsConfiguration));
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__size = 1;

        printf("%s--------Set Token Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);    
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH); 
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS, profIndex+1);
        
        printf("%s--------Set Name Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS_NAME, profIndex+1);
            
        printf("%s--------Set uSE Count Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->UseCount = 1;
        
        
        printf("%s--------Set AnalyticsEngineConfiguration Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration, 0x00, sizeof(struct tt__AnalyticsEngineConfiguration));
        

        printf("%s--------Set AnalyticsModule Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 4;
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule, 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        


        for(int index = 0; index<trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule; index++)
        {
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name ,INFO_LENGTH,"%s",AnalyticsModuleName[index]);

            printf("\n---------%s--------Set AnalyticsModule Video Analytics Profile0------------5\n",__func__);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type ,INFO_LENGTH,"%s",AnalyticsModuleTypes[index]);

            
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters, 0x00, sizeof( struct tt__ItemList));
            
            //trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem 
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem = 1;
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem*)soap_malloc(soap, (trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem, 0x00, (trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof( struct _tt__ItemList_SimpleItem));
            //trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name, 0x00, INFO_LENGTH*sizeof( CHAR));
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name ,INFO_LENGTH,"%s","Active");
            ////trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value, 0x00, INFO_LENGTH*sizeof( CHAR));
            
            rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                return SOAP_ERR;
            }
            sqlite3_bind_text(res, 1, AnalyticsModuleName[index], -1, SQLITE_STATIC);

            rc = sqlite3_step(res);
            if (rc == SQLITE_ROW) {
                active = sqlite3_column_int(res, 0);
                printf("Active value: %d\n", active);
            } else {
                printf("No record found\n");
            }

            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value ,INFO_LENGTH,"%d",active);

            // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem 
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeElementItem = 0;
            printf("%s--------Set AnalyticsModule Params Extension  Video Analytics Profile0---------\n",__func__);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->Extension = NULL;
            // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__anyAttribute = NULL;

            printf("\n-----------%s--------Set AnalyticsModule Video Analytics Profile0---------------9\n",__func__);
            sqlite3_finalize(res);
        }

        sql = "SELECT COUNT(*) FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }

        // Execute the SQL statement and fetch results
        rc = sqlite3_step(res);
        if (rc == SQLITE_ROW) {
            rule_count = sqlite3_column_int(res, 0);
            // printf("Number of entries in analytics_modules: %d\n", count);
        } else {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(res); // Finalize the prepared statement
            sqlite3_close(db);
            return SOAP_ERR;
        }
        sqlite3_finalize(res);
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__anyAttribute = NULL;
        
        printf("%s--------Set Analytics Engine __anyAttribute  Video Analytics Profile0---------\n",__func__);
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__anyAttribute = NULL;
        printf("%s--------Set Analytics Engine Extension  Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->Extension = NULL;

        printf("%s--------Set Rule Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration = (struct tt__RuleEngineConfiguration*)soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration , 0x00, sizeof(struct tt__RuleEngineConfiguration));
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule = rule_count;


        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule = (struct tt__Config*)soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule , 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));

        sql = "SELECT * FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }
    
        for(int index = 0; index < trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule ; index++)
        {
            rc = sqlite3_step(res);
            if (rc != SQLITE_ROW) {
               fprintf(stderr, "%s Failed to step statement in RuleEngineConfiguration: %s\n", __func__, sqlite3_errmsg(db));
                sqlite3_finalize(res);
                sqlite3_close(db);
                return SOAP_ERR;
            }
            
            // printf("\nsql column count: %s\n", sqlite3_column_text(res, 1));
            
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            // SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s","MotionRegionRule");
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 1));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            // SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s","tt:MotionRegionDetector");
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 2));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters = (struct  tt__ItemList *) soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters, 0x00, sizeof(struct tt__ItemList));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem = parameter_length;
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem = (struct  _tt__ItemList_SimpleItem *) soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem, 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));

            for(int index_i=0; index_i<trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem; index_i++)
            {
                trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, TOKEN_LENGTH,"%s",parameters[index_i]);

                trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 4+index_i));
            }

	    }
	// Finalize the statement to clean up
    sqlite3_finalize(res);
    sqlite3_close(db);


    trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Extension = NULL;
    // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__anyAttribute = NULL;
    
    trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__any = NULL;
    // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__anyAttribute = NULL;

skip_analytics_profile:
    /* PTZ */

    trt__GetProfileResponse->Profile->PTZConfiguration = NULL;

    trt__GetProfileResponse->Profile->Extension = NULL;

    //    dom_att(&trt__GetProfileResponse->Profiles[0].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    //****************************************************************** 
    }
    //@TODO Check here for profile token match
    else if(profIndex == 1)
    {
        trt__GetProfileResponse->Profile = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile));
        MEMSET(trt__GetProfileResponse->Profile, 0x00, sizeof(struct tt__Profile));

    
        trt__GetProfileResponse->Profile->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_PROFILE, 2);

        trt__GetProfileResponse->Profile->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->Name, ONVIF_PROFILE_NAME_SUB);

        trt__GetProfileResponse->Profile->fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(trt__GetProfileResponse->Profile->fixed) = xsd__boolean__true_;

        /// Video Source Configuration

     //   printf("-------------------------------VideoSourceConfiguration-------------------------------------\n");

        trt__GetProfileResponse->Profile->VideoSourceConfiguration =
                        (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration, 0x00, sizeof(struct tt__VideoSourceConfiguration));

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->token,vsrc_confToken);

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name,vsrc_name);

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->UseCount = vsrc_useCount;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken,vsrc_srcToken);
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->y = vsrc_boundX;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->x = vsrc_boundY;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->width = vsrc_boundWidth;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->height = vsrc_boundHeight;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->__size = 0;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->__any = NULL;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Extension = NULL; //TODO: Check for rotate

    
        /* AudioSource-Configuration */
        trt__GetProfileResponse->Profile->AudioSourceConfiguration = NULL;
        /* Video Encoder Configuration */
     //   printf("-------------------------------VideoEncoderConfiguration-------------------------------------\n");
    
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration =
            (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 2);

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 2);

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->UseCount = 1;

        /* TODO: GauranteedFrameRate : Currently no need */

        // Detect codec type dynamically (2=H264, 3=H265)
        int codec_type_ch1 = 2;
        nvt_adapter_video_get_video_codec(1, &codec_type_ch1);
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Encoding = codec_type_ch1;
        
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution, 0x00, sizeof(struct tt__VideoResolution));
         
        nvt_adapter_video_get_resolution(1, value);
        sscanf(value, "%d*%d", &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Width, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Height);
      
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Quality = 10.0;

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl, 0x00, sizeof(struct tt__VideoRateControl));

        nvt_adapter_video_get_frame_rate(1, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit);

        //trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit = fps_chn_1;
      //  free(value);

        /* TODO: EncodingInterval: Following parameter create confusion */
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = 1;
        nvt_adapter_video_get_max_rate(1,&trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->BitrateLimit);
      
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4 = NULL;
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264, 0x00, sizeof(struct tt__H264Configuration));
        /* Determines the interval in which the I-Frames will be coded */
        nvt_adapter_video_get_gop(1, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->GovLength);

      //  trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main; //tt__H264Profile__Baseline,tt__H264Profile__High,tt__H264Profile__Main;

        int h264_profile_val = 0;
        nvt_adapter_video_get_h264_profile(1, &h264_profile_val);
        printf("H264 PROFILE value profile 1----11 %d\n", h264_profile_val);
        if(h264_profile_val == 0)
        {    
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Baseline;
        }
        else if(h264_profile_val == 1)
        {
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__High;
        }
        else 
        {
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main;
        }

        free(value);
        
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast =
            (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

        //  trt__GetProfileResponse->Profile[i].VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //Currently only support of IPv4
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->Type = 0;

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         strcpy(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address,"0.0.0.0");

        
       trt__GetProfileResponse->Profile->VideoEncoderConfiguration->SessionTimeout = timeout;
     

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->__size = 0;
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->__any = NULL;
        //  dom_att(&trt__GetProfileResponse->Profile[0].VideoEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        //Video Encoder Configuration

     /* AudioEncoderConfiguration */
     trt__GetProfileResponse->Profile->AudioEncoderConfiguration = NULL;

        /*uncomment and do proper settings for multicast

        T_MULTICAST_STREAM_RTSP *l_px_aMulticast = &l_x_profs.m_v_profileList[idx].m_x_multicastConfig.m_x_multicastStream[0]; //Get always first
        */
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast =
        //       (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        //    MEMSET(trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address =
        //       (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        //   MEMSET(trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

        //  trt__GetProfileResponse->Profile[i].AudioEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //TODO: This param should be dynamic
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address->Type = 0; //TODO: This param should be dynamic
            
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->SessionTimeout = 60000;
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->__size = 0;
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->__any = NULL;
        //   dom_att(&trt__GetProfileResponse->Profile[0].AudioEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        // AudioEncoderConfiguration 

        //Metadata Configurations
        trt__GetProfileResponse->Profile->MetadataConfiguration = NULL;

        /* VideoAnalyticsConfiguration */
        // Skip analytics configuration if database is not available
        if (db == NULL) {
            printf("Database not available - skipping analytics configuration for Profile %d\n", profIndex);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
            goto skip_analytics_profile;
        }

        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration, 0x00, sizeof(struct tt__VideoAnalyticsConfiguration));
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__size = 1;

        printf("%s--------Set Token Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);    
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH); 
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS, profIndex+1);
        
        printf("%s--------Set Name Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS_NAME, profIndex+1);
            
        printf("%s--------Set uSE Count Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->UseCount = 1;
        
        
        printf("%s--------Set AnalyticsEngineConfiguration Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration, 0x00, sizeof(struct tt__AnalyticsEngineConfiguration));
        

        printf("%s--------Set AnalyticsModule Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 4;
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule, 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        


        for(int index = 0; index<trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule; index++)
        {
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name ,INFO_LENGTH,"%s",AnalyticsModuleName[index]);

            printf("\n---------%s--------Set AnalyticsModule Video Analytics Profile0------------5\n",__func__);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type ,INFO_LENGTH,"%s",AnalyticsModuleTypes[index]);

            
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters, 0x00, sizeof( struct tt__ItemList));
            
            //trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem 
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem = 1;
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem*)soap_malloc(soap, (trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem, 0x00, (trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof( struct _tt__ItemList_SimpleItem));
            //trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name, 0x00, INFO_LENGTH*sizeof( CHAR));
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name ,INFO_LENGTH,"%s","Active");
            ////trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value, 0x00, INFO_LENGTH*sizeof( CHAR));
            
            rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                return SOAP_ERR;
            }
            sqlite3_bind_text(res, 1, AnalyticsModuleName[index], -1, SQLITE_STATIC);

            rc = sqlite3_step(res);
            if (rc == SQLITE_ROW) {
                active = sqlite3_column_int(res, 0);
                printf("Active value: %d\n", active);
            } else {
                printf("No record found\n");
            }

            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value ,INFO_LENGTH,"%d",active);

            // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem 
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeElementItem = 0;
            printf("%s--------Set AnalyticsModule Params Extension  Video Analytics Profile0---------\n",__func__);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->Extension = NULL;
            // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__anyAttribute = NULL;

            printf("\n-----------%s--------Set AnalyticsModule Video Analytics Profile0---------------9\n",__func__);
            sqlite3_finalize(res);
        }

        sql = "SELECT COUNT(*) FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }

        // Execute the SQL statement and fetch results
        rc = sqlite3_step(res);
        if (rc == SQLITE_ROW) {
            rule_count = sqlite3_column_int(res, 0);
            // printf("Number of entries in analytics_modules: %d\n", count);
        } else {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(res); // Finalize the prepared statement
            sqlite3_close(db);
            return SOAP_ERR;
        }
        sqlite3_finalize(res);
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__anyAttribute = NULL;
        
        printf("%s--------Set Analytics Engine __anyAttribute  Video Analytics Profile0---------\n",__func__);
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__anyAttribute = NULL;
        printf("%s--------Set Analytics Engine Extension  Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->Extension = NULL;

        printf("%s--------Set Rule Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration = (struct tt__RuleEngineConfiguration*)soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration , 0x00, sizeof(struct tt__RuleEngineConfiguration));
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule = rule_count;


        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule = (struct tt__Config*)soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule , 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));

        sql = "SELECT * FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }
    
        for(int index = 0; index < trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule ; index++)
        {
            rc = sqlite3_step(res);
            if (rc != SQLITE_ROW) {
               fprintf(stderr, "%s Failed to step statement in RuleEngineConfiguration: %s\n", __func__, sqlite3_errmsg(db));
                sqlite3_finalize(res);
                sqlite3_close(db);
                return SOAP_ERR;
            }
            
            // printf("\nsql column count: %s\n", sqlite3_column_text(res, 1));
            
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            // SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s","MotionRegionRule");
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 1));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            // SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s","tt:MotionRegionDetector");
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 2));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters = (struct  tt__ItemList *) soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters, 0x00, sizeof(struct tt__ItemList));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem = parameter_length;
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem = (struct  _tt__ItemList_SimpleItem *) soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem, 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));

            for(int index_i=0; index_i<trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem; index_i++)
            {
                trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, TOKEN_LENGTH,"%s",parameters[index_i]);

                trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 4+index_i));
            }

	    }
	// Finalize the statement to clean up
    sqlite3_finalize(res);
    sqlite3_close(db);


    trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Extension = NULL;
    // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__anyAttribute = NULL;
    
    trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__any = NULL;
    // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__anyAttribute = NULL;


        /*PTZ Configuration*/
        trt__GetProfileResponse->Profile->PTZConfiguration = NULL;

        trt__GetProfileResponse->Profile->Extension = NULL;

    //    dom_att(&trt__GetProfileResponse->Profiles[0].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    //******************************************************************
            
   //    printf("ProfileToken = %s\n", trt__GetProfileResponse->Profile[1].token);

    }
    else if(profIndex == 2)
    {
        trt__GetProfileResponse->Profile = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile));
        MEMSET(trt__GetProfileResponse->Profile, 0x00, sizeof(struct tt__Profile));

    
        trt__GetProfileResponse->Profile->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_PROFILE, 3);

        trt__GetProfileResponse->Profile->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->Name, ONVIF_PROFILE_NAME_THIRD);

        trt__GetProfileResponse->Profile->fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(trt__GetProfileResponse->Profile->fixed) = xsd__boolean__true_;

     /// Video Source Configuration

     //   printf("-------------------------------VideoSourceConfiguration-------------------------------------\n");

        trt__GetProfileResponse->Profile->VideoSourceConfiguration =
                        (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration, 0x00, sizeof(struct tt__VideoSourceConfiguration));

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->token,vsrc_confToken);

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name,vsrc_name);

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->UseCount = vsrc_useCount;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
            strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken,vsrc_srcToken);
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->y = vsrc_boundX;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->x = vsrc_boundY;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->width = vsrc_boundWidth;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->height = vsrc_boundHeight;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->__size = 0;
        trt__GetProfileResponse->Profile->VideoSourceConfiguration->__any = NULL;

        trt__GetProfileResponse->Profile->VideoSourceConfiguration->Extension = NULL; //TODO: Check for rotate

    
        /* AudioSource-Configuration */
        trt__GetProfileResponse->Profile->AudioSourceConfiguration = NULL;
        /* Video Encoder Configuration */
        //   printf("-------------------------------VideoEncoderConfiguration-------------------------------------\n");
    
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration =
            (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 3);

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 3);

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->UseCount = 1;

        /* TODO: GauranteedFrameRate : Currently no need */

        // Detect codec type dynamically (2=H264, 3=H265)
        int codec_type_ch2 = 2;
        nvt_adapter_video_get_video_codec(2, &codec_type_ch2);
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Encoding = codec_type_ch2;
        
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution, 0x00, sizeof(struct tt__VideoResolution));
        
        nvt_adapter_video_get_resolution(2, value);
        sscanf(value, "%d*%d", &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Width, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Height);
        
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Quality = 10.0;

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl, 0x00, sizeof(struct tt__VideoRateControl));

        nvt_adapter_video_get_frame_rate(2, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit);

        //trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit = fps_chn_1;
      //  free(value);

        /* TODO: EncodingInterval: Following parameter create confusion */
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = 1;
        nvt_adapter_video_get_max_rate(2,&trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->BitrateLimit);
      
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4 = NULL;
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264, 0x00, sizeof(struct tt__H264Configuration));
        /* Determines the interval in which the I-Frames will be coded */
        nvt_adapter_video_get_gop(2, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->GovLength);
     
       // trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main; //tt__H264Profile__Baseline,tt__H264Profile__High,tt__H264Profile__Main;
        int h264_profile_val = 0;
        nvt_adapter_video_get_h264_profile(2, &h264_profile_val);
        printf("H264 PROFILE value profile 2-----22 %d\n", h264_profile_val);
        if(h264_profile_val == 0)
        {    
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Baseline;
        }
        else if(h264_profile_val == 1)
        {
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__High;
        }
        else 
        {
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main;
        }
        free(value);
        
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast =
            (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

        //  trt__GetProfileResponse->Profile[i].VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //Currently only support of IPv4
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->Type = 0;

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         strcpy(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address,"0.0.0.0");

        
      trt__GetProfileResponse->Profile->VideoEncoderConfiguration->SessionTimeout = timeout;
    

        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->__size = 0;
        trt__GetProfileResponse->Profile->VideoEncoderConfiguration->__any = NULL;
     //  dom_att(&trt__GetProfileResponse->Profile[0].VideoEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        //Video Encoder Configuration

      /* AudioEncoderConfiguration */
     trt__GetProfileResponse->Profile->AudioEncoderConfiguration = NULL;

        /*uncomment and do proper settings for multicast

        T_MULTICAST_STREAM_RTSP *l_px_aMulticast = &l_x_profs.m_v_profileList[idx].m_x_multicastConfig.m_x_multicastStream[0]; //Get always first
        */
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast =
        //       (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        //    MEMSET(trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address =
        //       (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        //   MEMSET(trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

        //  trt__GetProfileResponse->Profile[i].AudioEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //TODO: This param should be dynamic
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address->Type = 0; //TODO: This param should be dynamic
            
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->SessionTimeout = 60000;
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->__size = 0;
        //   trt__GetProfileResponse->Profile->AudioEncoderConfiguration->__any = NULL;
        //   dom_att(&trt__GetProfileResponse->Profile[0].AudioEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
            // AudioEncoderConfiguration 

        //Metadata Configurations
        trt__GetProfileResponse->Profile->MetadataConfiguration = NULL;

        /* VideoAnalyticsConfiguration */
        // Skip analytics configuration if database is not available
        if (db == NULL) {
            printf("Database not available - skipping analytics configuration for Profile %d\n", profIndex);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
            goto skip_analytics_profile;
        }

        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration, 0x00, sizeof(struct tt__VideoAnalyticsConfiguration));
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__size = 1;

        printf("%s--------Set Token Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);    
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH); 
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS, profIndex+1);
        
        printf("%s--------Set Name Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS_NAME, profIndex+1);
            
        printf("%s--------Set uSE Count Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->UseCount = 1;
        
        
        printf("%s--------Set AnalyticsEngineConfiguration Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration, 0x00, sizeof(struct tt__AnalyticsEngineConfiguration));
        

        printf("%s--------Set AnalyticsModule Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 4;
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule, 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        


        for(int index = 0; index<trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule; index++)
        {
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name ,INFO_LENGTH,"%s",AnalyticsModuleName[index]);

            printf("\n---------%s--------Set AnalyticsModule Video Analytics Profile0------------5\n",__func__);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type ,INFO_LENGTH,"%s",AnalyticsModuleTypes[index]);

            
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters, 0x00, sizeof( struct tt__ItemList));
            
            //trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem 
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem = 1;
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem*)soap_malloc(soap, (trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem, 0x00, (trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof( struct _tt__ItemList_SimpleItem));
            //trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name, 0x00, INFO_LENGTH*sizeof( CHAR));
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name ,INFO_LENGTH,"%s","Active");
            ////trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value, 0x00, INFO_LENGTH*sizeof( CHAR));
            
            rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                return SOAP_ERR;
            }
            sqlite3_bind_text(res, 1, AnalyticsModuleName[index], -1, SQLITE_STATIC);

            rc = sqlite3_step(res);
            if (rc == SQLITE_ROW) {
                active = sqlite3_column_int(res, 0);
                printf("Active value: %d\n", active);
            } else {
                printf("No record found\n");
            }

            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value ,INFO_LENGTH,"%d",active);

            // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem 
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeElementItem = 0;
            printf("%s--------Set AnalyticsModule Params Extension  Video Analytics Profile0---------\n",__func__);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->Extension = NULL;
            // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__anyAttribute = NULL;

            printf("\n-----------%s--------Set AnalyticsModule Video Analytics Profile0---------------9\n",__func__);
            sqlite3_finalize(res);
        }

        sql = "SELECT COUNT(*) FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }

        // Execute the SQL statement and fetch results
        rc = sqlite3_step(res);
        if (rc == SQLITE_ROW) {
            rule_count = sqlite3_column_int(res, 0);
            // printf("Number of entries in analytics_modules: %d\n", count);
        } else {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(res); // Finalize the prepared statement
            sqlite3_close(db);
            return SOAP_ERR;
        }
        sqlite3_finalize(res);
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__anyAttribute = NULL;
        
        printf("%s--------Set Analytics Engine __anyAttribute  Video Analytics Profile0---------\n",__func__);
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__anyAttribute = NULL;
        printf("%s--------Set Analytics Engine Extension  Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->Extension = NULL;

        printf("%s--------Set Rule Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration = (struct tt__RuleEngineConfiguration*)soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration , 0x00, sizeof(struct tt__RuleEngineConfiguration));
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule = rule_count;


        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule = (struct tt__Config*)soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule , 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));

        sql = "SELECT * FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }
    
        for(int index = 0; index < trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule ; index++)
        {
            rc = sqlite3_step(res);
            if (rc != SQLITE_ROW) {
               fprintf(stderr, "%s Failed to step statement in RuleEngineConfiguration: %s\n", __func__, sqlite3_errmsg(db));
                sqlite3_finalize(res);
                sqlite3_close(db);
                return SOAP_ERR;
            }
            
            // printf("\nsql column count: %s\n", sqlite3_column_text(res, 1));
            
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            // SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s","MotionRegionRule");
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 1));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            // SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s","tt:MotionRegionDetector");
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 2));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters = (struct  tt__ItemList *) soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters, 0x00, sizeof(struct tt__ItemList));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem = parameter_length;
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem = (struct  _tt__ItemList_SimpleItem *) soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem, 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));

            for(int index_i=0; index_i<trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem; index_i++)
            {
                trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, TOKEN_LENGTH,"%s",parameters[index_i]);

                trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 4+index_i));
            }

	    }
        // Finalize the statement to clean up
        sqlite3_finalize(res);
        sqlite3_close(db);


        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Extension = NULL;
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__anyAttribute = NULL;
        
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__any = NULL;
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__anyAttribute = NULL;


        /* PTZ Configuration*/
        trt__GetProfileResponse->Profile->PTZConfiguration = NULL;

        trt__GetProfileResponse->Profile->Extension = NULL;

        //    dom_att(&trt__GetProfileResponse->Profiles[0].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

     //******************************************************************
            
        //    printf("ProfileToken = %s\n", trt__GetProfileResponse->Profile[1].token);

    }
    //Test Profile
    else if(profIndex == 3)
    {
        trt__GetProfileResponse->Profile = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile));
        MEMSET(trt__GetProfileResponse->Profile, 0x00, sizeof(struct tt__Profile));

    
        trt__GetProfileResponse->Profile->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->token,"Test_Profile");

        trt__GetProfileResponse->Profile->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetProfileResponse->Profile->Name, "testMedia");

         trt__GetProfileResponse->Profile->fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(trt__GetProfileResponse->Profile->fixed) = xsd__boolean__false_;

    /// Video Source Configuration

    //   printf("-------------------------------VideoSourceConfiguration-------------------------------------\n");
        if (testProfileAddSourceConfig)
        {
            printf("---------------testProfileAddSourceConfig----------------VideoSourceConfiguration-------------------------------------\n");

            trt__GetProfileResponse->Profile->VideoSourceConfiguration =
                            (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
            MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration, 0x00, sizeof(struct tt__VideoSourceConfiguration));

            trt__GetProfileResponse->Profile->VideoSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
            strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->token,vsrc_confToken);

            trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name,vsrc_name);

            trt__GetProfileResponse->Profile->VideoSourceConfiguration->UseCount = vsrc_useCount+1;
            trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
            strcpy(trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken,vsrc_srcToken);
            trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
            trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->y = vsrc_boundX;
            trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->x = vsrc_boundY;

            trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->width = vsrc_boundWidth;
            trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->height = vsrc_boundHeight;

            trt__GetProfileResponse->Profile->VideoSourceConfiguration->__size = 0;
            trt__GetProfileResponse->Profile->VideoSourceConfiguration->__any = NULL;

            trt__GetProfileResponse->Profile->VideoSourceConfiguration->Extension = NULL; //TODO: Check for rotate

        }
        else{
            printf("----------testProfileAddSourceConfig---------------------VideoSourceConfiguration-------------False------------------------\n");
            trt__GetProfileResponse->Profile->VideoSourceConfiguration =NULL;
        }
        /* AudioSource-Configuration */
        trt__GetProfileResponse->Profile->AudioSourceConfiguration = NULL;
        /* Video Encoder Configuration */
    //   printf("-------------------------------VideoEncoderConfiguration-------------------------------------\n");
        if(testProfileAddVideoEncoderConfig)
        {
            printf("----------testProfileAddVideoEncoderConfig---------------------VideoEncoderConfiguration-------------------------------------\n");
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration =
                (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
            ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 1);

            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
                ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 1);

            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->UseCount = 2;

        /* TODO: GauranteedFrameRate : Currently no need */

            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Encoding = tt__VideoEncoding__H264;
            
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution, 0x00, sizeof(struct tt__VideoResolution));
    
            nvt_adapter_video_get_resolution(0, value);
            sscanf(value, "%d*%d", &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Width, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Height);

        
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Quality = 10.0;

            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl, 0x00, sizeof(struct tt__VideoRateControl));

            nvt_adapter_video_get_frame_rate(0, &trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit);
        //  free(value);
            /* TODO: EncodingInterval: Following parameter create confusion */
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = 1;
            nvt_adapter_video_get_max_rate(0,&trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->BitrateLimit);

            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4 = NULL;
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264, 0x00, sizeof(struct tt__H264Configuration));
            /* Determines the interval in which the I-Frames will be coded */
            nvt_adapter_video_get_gop(0,&trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->GovLength);

        //  trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main; //tt__H264Profile__Baseline,tt__H264Profile__High,tt__H264Profile__Main;
            int h264_profile_val = 0;
            nvt_adapter_video_get_h264_profile(0, &h264_profile_val);
            printf("H264 PROFILE value profile 0----00 %d\n", h264_profile_val);
            if(h264_profile_val == 0)
            {    
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Baseline;
            }
            else if(h264_profile_val == 1)
            {
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__High;
            }
            else 
            {
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main;
            }
            free(value);
    
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast =
                (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address =
                (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
            
        //  trt__GetProfileResponse->Profile[i].VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //Currently only support of IPv4
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->Type = 0;

            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address =
            (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
            MEMSET(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address, 0x00,
            sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
            strcpy(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address,"0.0.0.0");
            
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->SessionTimeout = timeout; 
        

            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->__size = 0;
            trt__GetProfileResponse->Profile->VideoEncoderConfiguration->__any = NULL;
        //  dom_att(&trt__GetProfileResponse->Profile[0].VideoEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
            //Video Encoder Configuration
        }
        else{
            printf("----------testProfileAddVideoEncoderConfig---------------------VideoEncoderConfiguration-------------False------------------------\n");
             trt__GetProfileResponse->Profile->VideoEncoderConfiguration = NULL;
        }

        if(testProfileAddMetaDataConfig)
        {

            printf("----------testProfileAddMetaDataConfig---------------------MetaDataConfiguration-------------------------------------\n");

            trt__GetProfileResponse->Profile->MetadataConfiguration =
            (struct tt__MetadataConfiguration *)soap_malloc(soap, sizeof(struct tt__MetadataConfiguration));
            MEMSET(trt__GetProfileResponse->Profile->MetadataConfiguration, 0x00, sizeof(struct tt__MetadataConfiguration));

            trt__GetProfileResponse->Profile->MetadataConfiguration->token = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->MetadataConfiguration->token, 0x00, sizeof(CHAR)*TOKEN_LENGTH);
            ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->MetadataConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_METADATA, 1);
            // snprintf(trt__GetProfileResponse->Profile->MetadataConfiguration->token,
            //         TOKEN_LENGTH, "%s", "MetaDataConfigToken_1");

            trt__GetProfileResponse->Profile->MetadataConfiguration->Name = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->MetadataConfiguration->Name, 0x00, sizeof(CHAR)*TOKEN_LENGTH);
            ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->MetadataConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_METADATA_NAME, 1);
            // snprintf(trt__GetProfileResponse->Profile->MetadataConfiguration->Name,
            //         TOKEN_LENGTH, "%s", "MetaDataConfig_1");

            printf("----------testProfileAddMetaDataConfig---------------------MetaDataConfiguration-------------111111------------------------\n");
            trt__GetProfileResponse->Profile->MetadataConfiguration->UseCount = 1;

            trt__GetProfileResponse->Profile->MetadataConfiguration->PTZStatus = NULL;
                // No events tag at all
            trt__GetProfileResponse->Profile->MetadataConfiguration->Events = NULL;

            trt__GetProfileResponse->Profile->MetadataConfiguration->GeoLocation = NULL;

            trt__GetProfileResponse->Profile->MetadataConfiguration->Analytics = NULL;

             printf("----------testProfileAddMetaDataConfig---------------------MetaDataConfiguration--------------222222-----------------------\n");

            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast =
                (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
            MEMSET(trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));

            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address =
                (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
            MEMSET(trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //TODO: This param should be dynamic

            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->IPv4Address =
                (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN+1));
            MEMSET(trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->IPv4Address, 0x00,
                sizeof(CHAR) * (IPV4_ADDR_STRING_LEN+1));
            snprintf(trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->IPv4Address, IPV4_ADDR_STRING_LEN+1, "%s","0.0.0.0");
            printf("----------testProfileAddMetaDataConfig---------------------MetaDataConfiguration-------------333333------------------------\n");
            //Port
            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Port = 0;

            //TTL
            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->TTL = 64;

            //AutoStart
            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->AutoStart = 64;

            printf("----------testProfileAddMetaDataConfig---------------------MetaDataConfiguration----------------444444---------------------\n");
            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->__size = 0;
            trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->__any = NULL;

            trt__GetProfileResponse->Profile->MetadataConfiguration->SessionTimeout = timeout;

            trt__GetProfileResponse->Profile->MetadataConfiguration->__size = 0;
            trt__GetProfileResponse->Profile->MetadataConfiguration->__any = NULL;

             printf("----------testProfileAddMetaDataConfig---------------------MetaDataConfiguration-----------555555--------------------------\n");
        }
        else
        {
            printf("----------testProfileAddMetaDataConfig---------------------MetaDataConfiguration-------------False------------------------\n");
            trt__GetProfileResponse->Profile->MetadataConfiguration = NULL;
        }

    /* AudioEncoderConfiguration */
        trt__GetProfileResponse->Profile->AudioEncoderConfiguration = NULL;
        // AudioEncoderConfiguration 

        //Metadata Configurations
      //  trt__GetProfileResponse->Profile->MetadataConfiguration = NULL;

        /* VideoAnalyticsConfiguration */

        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration, 0x00, sizeof(struct tt__VideoAnalyticsConfiguration));
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__size = 1;

        printf("%s--------Set Token Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);    
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH); 
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS, profIndex+1);
        
        printf("%s--------Set Name Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            
        ONVIF_TOKEN_GEN(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS_NAME, profIndex+1);
            
        printf("%s--------Set uSE Count Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->UseCount = 1;
        
        
        printf("%s--------Set AnalyticsEngineConfiguration Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration, 0x00, sizeof(struct tt__AnalyticsEngineConfiguration));
        

        printf("%s--------Set AnalyticsModule Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 4;
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule, 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        


        for(int index = 0; index<trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule; index++)
        {
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name ,INFO_LENGTH,"%s",AnalyticsModuleName[index]);

            printf("\n---------%s--------Set AnalyticsModule Video Analytics Profile0------------5\n",__func__);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type ,INFO_LENGTH,"%s",AnalyticsModuleTypes[index]);

            
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters, 0x00, sizeof( struct tt__ItemList));
            
            //trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem 
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem = 1;
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem*)soap_malloc(soap, (trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem, 0x00, (trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof( struct _tt__ItemList_SimpleItem));
            //trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name, 0x00, INFO_LENGTH*sizeof( CHAR));
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name ,INFO_LENGTH,"%s","Active");
            ////trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value, 0x00, INFO_LENGTH*sizeof( CHAR));
            
            rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                return SOAP_ERR;
            }
            sqlite3_bind_text(res, 1, AnalyticsModuleName[index], -1, SQLITE_STATIC);

            rc = sqlite3_step(res);
            if (rc == SQLITE_ROW) {
                active = sqlite3_column_int(res, 0);
                printf("Active value: %d\n", active);
            } else {
                printf("No record found\n");
            }

            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value ,INFO_LENGTH,"%d",active);

            // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem 
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeElementItem = 0;
            printf("%s--------Set AnalyticsModule Params Extension  Video Analytics Profile0---------\n",__func__);
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->Extension = NULL;
            // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__anyAttribute = NULL;

            printf("\n-----------%s--------Set AnalyticsModule Video Analytics Profile0---------------9\n",__func__);
            sqlite3_finalize(res);
        }

        sql = "SELECT COUNT(*) FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }

        // Execute the SQL statement and fetch results
        rc = sqlite3_step(res);
        if (rc == SQLITE_ROW) {
            rule_count = sqlite3_column_int(res, 0);
            // printf("Number of entries in analytics_modules: %d\n", count);
        } else {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(res); // Finalize the prepared statement
            sqlite3_close(db);
            return SOAP_ERR;
        }
        sqlite3_finalize(res);
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__anyAttribute = NULL;
        
        printf("%s--------Set Analytics Engine __anyAttribute  Video Analytics Profile0---------\n",__func__);
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__anyAttribute = NULL;
        printf("%s--------Set Analytics Engine Extension  Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->Extension = NULL;

        printf("%s--------Set Rule Configuration Video Analytics Profile0---------\n",__func__);
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration = (struct tt__RuleEngineConfiguration*)soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration , 0x00, sizeof(struct tt__RuleEngineConfiguration));
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule = rule_count;


        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule = (struct tt__Config*)soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));
        MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule , 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));

        sql = "SELECT * FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }
    
        for(int index = 0; index < trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule ; index++)
        {
            rc = sqlite3_step(res);
            if (rc != SQLITE_ROW) {
               fprintf(stderr, "%s Failed to step statement in RuleEngineConfiguration: %s\n", __func__, sqlite3_errmsg(db));
                sqlite3_finalize(res);
                sqlite3_close(db);
                return SOAP_ERR;
            }
            
            // printf("\nsql column count: %s\n", sqlite3_column_text(res, 1));
            
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            // SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s","MotionRegionRule");
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 1));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            // SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s","tt:MotionRegionDetector");
            SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 2));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters = (struct  tt__ItemList *) soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters, 0x00, sizeof(struct tt__ItemList));

            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem = parameter_length;
            trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem = (struct  _tt__ItemList_SimpleItem *) soap_malloc(soap, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem, 0x00, trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));

            for(int index_i=0; index_i<trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem; index_i++)
            {
                trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, TOKEN_LENGTH,"%s",parameters[index_i]);

                trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 4+index_i));
            }

	    }
        // Finalize the statement to clean up
        sqlite3_finalize(res);
        sqlite3_close(db);


        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Extension = NULL;
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__anyAttribute = NULL;
        
        trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__any = NULL;
        // trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->__anyAttribute = NULL;




        /* PTZ */
        trt__GetProfileResponse->Profile->PTZConfiguration = NULL;

        trt__GetProfileResponse->Profile->Extension = NULL;

    //    dom_att(&trt__GetProfileResponse->Profiles[0].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    //****************************************************************** 
 
    }
    
    return 0;
}

/** Web service operation '__trt__GetProfiles' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap* soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
     
    printf("-------------------------------__trt__GetProfiles-------------------------------------\n");
    onvif_media_init_tokens();
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
    {
        return l_i32_ret;
    }
    char *value = malloc(20);
      if (value == NULL) {
        fprintf(stderr, "Failed to allocate memory in func %s\n",__func__);
        return 1;
    }
    const char *profile_value;
    int h264_profile_val = 0;
    sqlite3 *db;
	char *sql;
	int rc;
    int active, rule_count = 0;
	sqlite3_stmt *res;
    const char* AnalyticsModuleTypes[] = {"tt:TripWireDetectionEngine","tt:TrespassingDetectionEngine","tt:MotionDetectionEngine","tt:TamperDetectionEngine"};
	const char* AnalyticsModuleName[] = {"TripWireDetectionModule","TrespassingDetectionModule","MotionDetectionModule","TamperDetectionModule"};
    
    /* LEGACY DB DISABLED */
	db = NULL;
	rc = SQLITE_OK;
    // rc = sqlite3_open("/oem/usr/share/usermng.db", &db);
	if( rc ) {
		printf("==========Can't open database: %s (analytics disabled)==========\n", sqlite3_errmsg(db));
		db = NULL;  // Continue without database - analytics will be disabled
	}

    
    
    
    // For 3 profiles
    trt__GetProfilesResponse->__sizeProfiles = 3;

    trt__GetProfilesResponse->Profiles = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile) * (trt__GetProfilesResponse->__sizeProfiles));
    MEMSET(trt__GetProfilesResponse->Profiles, 0x00, sizeof(struct tt__Profile) * (trt__GetProfilesResponse->__sizeProfiles));


 //***********************************Profile 0******************************************************************************

 //   printf("-------------------------------__trt__GetProfiles-------------------Profile 0------------------\n");

    trt__GetProfilesResponse->Profiles[0].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].token, 0x00, sizeof(char) * TOKEN_LENGTH);
    ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[0].token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_PROFILE, 1);

    trt__GetProfilesResponse->Profiles[0].Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[0].Name, ONVIF_PROFILE_NAME_MAIN);
 
    trt__GetProfilesResponse->Profiles[0].fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetProfilesResponse->Profiles[0].fixed) = xsd__boolean__true_;

    // Video Source Configuration

     //   printf("-------------------------------VideoSourceConfiguration-------------Profile 0------------------------\n");

    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration =
                    (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration, 0x00, sizeof(struct tt__VideoSourceConfiguration));

    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->token,vsrc_confToken);

    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Name,vsrc_name);

    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->UseCount = vsrc_useCount;
    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
    //Source token should be same for all profiles for single camera source
    strcpy(trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->SourceToken,vsrc_srcToken);
    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Bounds->x = vsrc_boundX;
     trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Bounds->y = vsrc_boundY;

    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Bounds->width = vsrc_boundWidth;
    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Bounds->height = vsrc_boundHeight;

    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->__size = 0;
    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->__any = NULL;

    trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->Extension = NULL; //TODO: Check for rotate

  //  dom_att(&trt__GetProfilesResponse->Profiles[0].VideoSourceConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);    
   

    /* AudioSource-Configuration */
    trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration = NULL;
 //   printf("-------------------------------AudioSourceConfiguration---------------Profile 0----------------------\n");
 /*
    trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration =
        (struct tt__AudioSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__AudioSourceConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration, 0x00, sizeof(struct tt__AudioSourceConfiguration));

    trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->token,"AudioSourceToken");

    
    trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->Name,"AUDIO_SOURCE_CONFG");
    

    trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->UseCount = 1;

    trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->SourceToken, "AudioSourceToken");

    trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->__size = 0;
    trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->__any = NULL;
  //  dom_att(&trt__GetProfilesResponse->Profiles[0].AudioSourceConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

*/
    // Video Encoder Configuration 
 //   printf("-------------------------------VideoEncoderConfiguration----------------Profile 0---------------------\n");
   
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration =
        (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 1);

    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 1);

    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->UseCount = 1;

    /* TODO: GauranteedFrameRate : Currently no need */

    // Detect codec type dynamically (2=H264, 3=H265)
    int codec_type_p0 = 2;
    nvt_adapter_video_get_video_codec(0, &codec_type_p0);
    // Always use H264 for ONVIF compatibility
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Encoding = tt__VideoEncoding__H264;
     
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Resolution, 0x00, sizeof(struct tt__VideoResolution));
    nvt_adapter_video_get_resolution(0, value);
    sscanf(value, "%d*%d", &trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Resolution->Width, &trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Resolution->Height);
    
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Quality = 10.0;

    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->RateControl, 0x00, sizeof(struct tt__VideoRateControl));

    nvt_adapter_video_get_frame_rate(0, &trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->RateControl->FrameRateLimit);

    /* TODO: EncodingInterval: Following parameter create confusion */
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->RateControl->EncodingInterval = 1;
    nvt_adapter_video_get_max_rate(0, &trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->RateControl->BitrateLimit);


    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->MPEG4 = NULL;
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->H264, 0x00, sizeof(struct tt__H264Configuration));
    /* Determines the interval in which the I-Frames will be coded */
    nvt_adapter_video_get_gop(0,&trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->H264->GovLength);

 
   // trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main; //tt__H264Profile__Baseline,tt__H264Profile__High,tt__H264Profile__Main;
      
    h264_profile_val = 0;
    nvt_adapter_video_get_h264_profile(0, &h264_profile_val);
    printf("H264 PROFILE value profile 0-------0000 %d\n", h264_profile_val);
    if(h264_profile_val == 0)
    {    
            trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Baseline;
    }
    else if(h264_profile_val == 1)
    {
            trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__High;
    }
    else 
    {
            trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main;
    }
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Multicast =
        (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Multicast->Address =
        (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

  //  trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //Currently only support of IPv4
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Multicast->Address->Type = 0;

        trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
    MEMSET(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
    strcpy(trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->Multicast->Address->IPv4Address,"0.0.0.0");


    
     trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->SessionTimeout = timeout;

  //  printf("-------------------------------VideoEncoderConfiguration---After SetSessionTimeout-------------Profile 0---------------------\n");

    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->__size = 0;
    trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->__any = NULL;
  //  dom_att(&trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
    //Video Encoder Configuration

//AudioEncoderConfiguration 
    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration = NULL;

//    printf("-------------------------------AudioEncoderConfiguration--------------------Profile 0-----------------\n");
/*    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration =
        (struct tt__AudioEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__AudioEncoderConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration, 0x00, sizeof(struct tt__AudioEncoderConfiguration));

    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->token, "AudioSourceToken");

    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Name, "AudioEncoderConfgName");
    
    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->UseCount = 1;

    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Encoding = tt__AudioEncoding__G711; //tt__AudioEncoding__G726;
    
    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Bitrate = 1000;
       
    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->SampleRate = 8000;
    */

     //uncomment and do proper settings for multicast

   // T_MULTICAST_STREAM_RTSP *l_px_aMulticast = &l_x_profs.m_v_profileList[idx].m_x_multicastConfig.m_x_multicastStream[0]; //Get always first
    
 //   trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Multicast =
 //       (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
 //   MEMSET(trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
 //   trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Multicast->Address =
 //       (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
 //   MEMSET(trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

  //  trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //TODO: This param should be dynamic
  //  trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->Multicast->Address->Type = 0; //TODO: This param should be dynamic
    /*
    if (tt__IPType__IPv4 == trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->Address->Type)
    {
        trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->Address->IPv4Address =
            (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
        MEMSET(trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->Address->IPv4Address, 0x00,
            sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
        SNPRINTF(trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->Address->IPv4Address,
                IPV4_ADDR_STRING_LEN + 1, "%s", l_px_aMulticast->m_aui8_GroupAddrAudio);
    }
    else
    {
        // TODO: Currently not supported 
        trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->Address->IPv6Address =
            (CHARPTR)soap_malloc(soap, sizeof(CHAR) * MAX_IPV6_IP_ADDR_LEN);
        MEMSET(trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->Address->IPv6Address, 0x00,
            sizeof(CHAR) * MAX_IPV6_IP_ADDR_LEN);
    }
    
    trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->Port = l_px_aMulticast->m_ui16_audioPort;
    trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->TTL = l_px_aMulticast->m_ui16_ttl;
    trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->AutoStart = (xsd__boolean)l_px_aMulticast->m_b_alwaysMulticastState;
    trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->__size = 0;
    trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->__any = NULL;
    dom_att(&trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration->Multicast->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
    */

 //   trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->SessionTimeout = 60000;
 //   trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->__size = 0;
//    trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->__any = NULL;
 //   dom_att(&trt__GetProfilesResponse->Profiles[0].AudioEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
    // AudioEncoderConfiguration 

    //Metadata Configurations
  //  printf("-------------------------------MetadataConfiguration-----------------Profile 0--------------------\n");
    trt__GetProfilesResponse->Profiles[0].MetadataConfiguration = NULL;

    /* VideoAnalyticsConfiguration */
  //  printf("-------------------------------VideoAnalyticsConfiguration-------------Profile 0------------------------\n");
    printf("-------------------------------VideoAnalyticsConfiguration-------------Profile 0------------------------\n");

    const char *parameters[] = {"label","active","coordinates"};
    size_t parameter_length = sizeof(parameters) / sizeof(parameters[0]);

    // Skip analytics configuration if database is not available
    if (db == NULL) {
        printf("Database not available - skipping analytics configuration\n");
        for (int prof = 0; prof<3; prof++) {
            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration = NULL;
        }
        // Continue to configure Profile 1 and Profile 2 - don't return here!
        goto skip_analytics;
    }

    sql = "SELECT active FROM analytics_modules WHERE name = ?";
    for (int prof = 0; prof<3; prof++)
    {
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
        MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration, 0x00, sizeof(struct tt__VideoAnalyticsConfiguration));
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->__size = 1;

        printf("--------Set Token Configuration Video Analytics Profile%d---------\n",prof);
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);    
        MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH); 
        ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS, prof+1);
        
        printf("--------Set Name Configuration Video Analytics Profile%d---------\n",prof);
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
            
        ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ANALYTICS_NAME, prof+1);
            
        printf("--%s------Set uSE Count Configuration Video Analytics Profile%d---------\n",__func__,prof);
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->UseCount = 1;
        
        printf("--%s------Set AnalyticsEngineConfiguration Configuration Video Analytics Profile%d---------\n",__func__,prof);
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
        MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration, 0x00, sizeof(struct tt__AnalyticsEngineConfiguration));
        
        printf("--%s------Set AnalyticsModule Video Analytics Profile%d---------\n",__func__,prof);
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 4;
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap,  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));
        MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule, 0x00,  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule*sizeof(struct tt__Config));


        for(int index = 0; index< trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule; index++)
        {
            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Name ,INFO_LENGTH,"%s",AnalyticsModuleName[index]);

            printf("\n-------------%s------Set AnalyticsModule Video Analytics Profile%d-----------------5\n",__func__,prof);
            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type , 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Type ,INFO_LENGTH,"%s",AnalyticsModuleTypes[index]);


             trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters, 0x00, sizeof( struct tt__ItemList));
            
            // trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem 
             trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem = 1;
             trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem*)soap_malloc(soap, ( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem, 0x00, ( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof( struct _tt__ItemList_SimpleItem));
            // trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name
             trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name, 0x00, INFO_LENGTH*sizeof( CHAR));
            SNPRINTF( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Name ,INFO_LENGTH,"%s","Active");
            //// trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value
             trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value, 0x00, INFO_LENGTH*sizeof( CHAR));
            
            rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                return SOAP_ERR;
            }
            sqlite3_bind_text(res, 1, AnalyticsModuleName[index], -1, SQLITE_TRANSIENT);

            rc = sqlite3_step(res);
            if (rc == SQLITE_ROW) {
                active = sqlite3_column_int(res, 0);
                printf("Active value: %d\n", active);
            } else {
                printf("No record found\n");
            }
            printf("\n-------------%s------set value of simple item Set AnalyticsModule Video Analytics Profile%d-----------------5\n",__func__,prof);
            SNPRINTF( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->SimpleItem[0].Value ,INFO_LENGTH,"%d",active);


            //  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem 
            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__sizeElementItem = 0;
            printf("-%s-------Set AnalyticsModule Params Extension  Video Analytics Profile%d---------\n",__func__,prof);
             trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->Extension = NULL;
            //  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule[index].Parameters->__anyAttribute = NULL;

            printf("\n-----%s---------------Set AnalyticsModule Video Analytics Profile%d------------------9\n",__func__,prof);
            sqlite3_finalize(res);
        }



        sql = "SELECT COUNT(*) FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "%s Failed to execute statement: %s\n", __func__, sqlite3_errmsg(db));
            sqlite3_close(db);
            return SOAP_ERR;
        }

        // Execute the SQL statement and fetch results
        rc = sqlite3_step(res);
        if (rc == SQLITE_ROW) {
            rule_count = sqlite3_column_int(res, 0);
        } else {
            fprintf(stderr, "%s Failed to fetch data: %s\n", __func__, sqlite3_errmsg(db));
            sqlite3_finalize(res); // Finalize the prepared statement
            sqlite3_close(db);
            return SOAP_ERR;
        }
        sqlite3_finalize(res);
        //  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__anyAttribute = NULL;
        
        printf("%s--------Set Analytics Engine __anyAttribute  Video Analytics Profile%d---------\n",__func__,prof);
        //  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__anyAttribute = NULL;
        printf("%s--------Set Analytics Engine Extension  Video Analytics Profile%d---------\n",__func__,prof);
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->Extension = NULL;


        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration = (struct tt__RuleEngineConfiguration*)soap_malloc(soap,  sizeof(struct tt__RuleEngineConfiguration));
        MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration , 0x00,  sizeof(struct tt__RuleEngineConfiguration));
        
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule = rule_count;

        sql = "SELECT * FROM analytics_rules";

        rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }

        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule = (struct tt__Config*)soap_malloc(soap,  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));
        MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule , 0x00,  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));

        for(int index = 0; index <  rule_count ; index++)
        {
            rc = sqlite3_step(res);
            if (rc != SQLITE_ROW) {
               fprintf(stderr, "%s Failed to step statement in RuleEngineConfiguration: %s\n", __func__, sqlite3_errmsg(db));
                sqlite3_finalize(res);
                sqlite3_close(db);
                return SOAP_ERR;
            }
            // printf("\nsql column count: %s\n", sqlite3_column_text(res, 1));

            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            SNPRINTF( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Name,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 1));

            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
            SNPRINTF( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Type,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 2));

            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters = (struct  tt__ItemList *) soap_malloc(soap, sizeof(struct tt__ItemList));
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters, 0x00, sizeof(struct tt__ItemList));

            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem = parameter_length;
            trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem = (struct  _tt__ItemList_SimpleItem *) soap_malloc(soap,  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));
            MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem, 0x00,  trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));

            for(int index_i=0; index_i< trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->__sizeSimpleItem; index_i++)
            {
                printf("\n%s update rule parameters\n",__func__);
                trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Name, TOKEN_LENGTH,"%s",parameters[index_i]);

                trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF( trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule[index].Parameters->SimpleItem[index_i].Value, TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 4+index_i));
            }

	    }
	    // Finalize the statement to clean up
        sqlite3_finalize(res);
        trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->__any = NULL;
    }
    sqlite3_close(db);

skip_analytics:
    // trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->RuleEngineConfiguration->__anyAttribute = NULL;
    
    
    // trt__GetProfilesResponse->Profiles[prof].VideoAnalyticsConfiguration->__anyAttribute = NULL;




 //    printf("-------------------------------PTZConfiguration------------------Profile 0-------------------\n");
     trt__GetProfilesResponse->Profiles[0].PTZConfiguration = NULL;
     
 //    printf("-------------------------------Extension-------------------------Profile 0------------\n");

     trt__GetProfilesResponse->Profiles[0].Extension = NULL;

 //    dom_att(&trt__GetProfilesResponse->Profiles[0].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

 //******************************************************************Profile_0****************************************************************


 //***********************************Profile 1******************************************************************************

 //   printf("-------------------------------__trt__GetProfiles-------------------Profile 1------------------\n");

    trt__GetProfilesResponse->Profiles[1].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[1].token, 0x00, sizeof(char) * TOKEN_LENGTH);
    ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[1].token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_PROFILE, 2);

    trt__GetProfilesResponse->Profiles[1].Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[1].Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[1].Name, ONVIF_PROFILE_NAME_SUB);

    trt__GetProfilesResponse->Profiles[1].fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetProfilesResponse->Profiles[1].fixed) = xsd__boolean__true_;

/// Video Source Configuration

//    printf("-------------------------------VideoSourceConfiguration-------------Profile 1------------------------\n");

    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration =
                    (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration, 0x00, sizeof(struct tt__VideoSourceConfiguration));

    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->token,vsrc_confToken);

    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Name,vsrc_name);

    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->UseCount = vsrc_useCount;
    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->SourceToken,vsrc_srcToken);
    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Bounds->x = vsrc_boundX;
    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Bounds->y = vsrc_boundY;

    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Bounds->width = vsrc_boundWidth;
    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Bounds->height = vsrc_boundHeight;

    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->__size = 0;
    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->__any = NULL;

    trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->Extension = NULL; //TODO: Check for rotate

//   //  dom_att(&trt__GetProfilesResponse->Profiles[1].VideoSourceConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);    
   

//     /* AudioSource-Configuration */
     trt__GetProfilesResponse->Profiles[1].AudioSourceConfiguration = NULL;
//     printf("-------------------------------AudioSourceConfiguration---------------Profile 1----------------------\n");
 
//      Video Encoder Configuration 
//     printf("-------------------------------VideoEncoderConfiguration----------------Profile 1---------------------\n");
   
    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration =
        (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 2);

    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 2);

    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->UseCount = 1;

//     /* TODO: GauranteedFrameRate : Currently no need */

    // Detect codec type dynamically (2=H264, 3=H265)
    int codec_type_p1 = 2;
    nvt_adapter_video_get_video_codec(1, &codec_type_p1);
    // Always use H264 for ONVIF compatibility
    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Encoding = tt__VideoEncoding__H264;
     
    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Resolution, 0x00, sizeof(struct tt__VideoResolution));

    nvt_adapter_video_get_resolution(1, value);
    sscanf(value, "%d*%d", &trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Resolution->Width, & trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Resolution->Height);
   
    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Quality = 10.0;

    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->RateControl, 0x00, sizeof(struct tt__VideoRateControl));

    nvt_adapter_video_get_frame_rate(1, &trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->RateControl->FrameRateLimit);
  //  free(value);

    /* TODO: EncodingInterval: Following parameter create confusion */
    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->RateControl->EncodingInterval = 1;

    nvt_adapter_video_get_max_rate(1,&trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->RateControl->BitrateLimit);
     trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->MPEG4 = NULL;
    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->H264, 0x00, sizeof(struct tt__H264Configuration));
    /* Determines the interval in which the I-Frames will be coded */
    nvt_adapter_video_get_gop(1,&trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->H264->GovLength);

    //trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main; //tt__H264Profile__Baseline,tt__H264Profile__High,tt__H264Profile__Main;

    h264_profile_val = 0;
    nvt_adapter_video_get_h264_profile(1, &h264_profile_val);
    printf("H264 PROFILE value profile 1------1111 %d\n", h264_profile_val);
    if(h264_profile_val == 0)
    {    
            trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Baseline;
    }
    else if(h264_profile_val == 1)
    {
            trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__High;
    }
    else 
    {
            trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main;
    } 

    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Multicast =
        (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
    trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Multicast->Address =
        (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
    MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

   //  trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //Currently only support of IPv4
     trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Multicast->Address->Type = 0;

     trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         MEMSET(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         strcpy(trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->Multicast->Address->IPv4Address,"0.0.0.0");

     trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->SessionTimeout  = timeout;


//     printf("-------------------------------VideoEncoderConfiguration---After SetSessionTimeout-------------Profile 1---------------------\n");

     trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->__size = 0;
     trt__GetProfilesResponse->Profiles[1].VideoEncoderConfiguration->__any = NULL;
   //  dom_att(&trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
//     //Video Encoder Configuration

// /* AudioEncoderConfiguration */
// trt__GetProfilesResponse->Profiles[1].AudioEncoderConfiguration = NULL;

 //    printf("-------------------------------AudioEncoderConfiguration--------------------Profile 1-----------------\n");
//     //Metadata Configurations
  //   printf("-------------------------------MetadataConfiguration-----------------Profile 1--------------------\n");
    trt__GetProfilesResponse->Profiles[1].MetadataConfiguration = NULL;

//     /* VideoAnalyticsConfiguration */
 //     printf("-------------------------------VideoAnalyticsConfiguration-------------Profile 1------------------------\n");
    //  trt__GetProfilesResponse->Profiles[1].VideoAnalyticsConfiguration = NULL;

 //     printf("-------------------------------PTZConfiguration------------------Profile 1-------------------\n");
      trt__GetProfilesResponse->Profiles[1].PTZConfiguration = NULL;
     
  //    printf("-------------------------------Extension-------------------------Profile 1------------\n");

      trt__GetProfilesResponse->Profiles[1].Extension = NULL;
  //    dom_att(&trt__GetProfilesResponse->Profiles[1].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

//  //******************************************************************profile_1*********************************************************
//   //***********************************Profile 2******************************************************************************

//    printf("-------------------------------__trt__GetProfiles-------------------Profile 2------------------\n");

    trt__GetProfilesResponse->Profiles[2].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[2].token, 0x00, sizeof(char) * TOKEN_LENGTH);
    ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[2].token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_PROFILE, 3);

    trt__GetProfilesResponse->Profiles[2].Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[2].Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[2].Name, ONVIF_PROFILE_NAME_THIRD);

    trt__GetProfilesResponse->Profiles[2].fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetProfilesResponse->Profiles[2].fixed) = xsd__boolean__true_;

/// Video Source Configuration

 //   printf("-------------------------------VideoSourceConfiguration-------------Profile 2------------------------\n");

    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration =
                    (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration, 0x00, sizeof(struct tt__VideoSourceConfiguration));

    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->token,vsrc_confToken);

    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Name,vsrc_name);

    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->UseCount = vsrc_useCount;
    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);

  
   strcpy(trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->SourceToken,vsrc_srcToken);
  
    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Bounds->x = vsrc_boundX;
    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Bounds->y = vsrc_boundY;

    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Bounds->width = vsrc_boundWidth;
    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Bounds->height = vsrc_boundHeight;

    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->__size = 0;
    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->__any = NULL;

    trt__GetProfilesResponse->Profiles[2].VideoSourceConfiguration->Extension = NULL; //TODO: Check for rotate


    /* AudioSource-Configuration */
    trt__GetProfilesResponse->Profiles[2].AudioSourceConfiguration = NULL;
 //   printf("-------------------------------AudioSourceConfiguration---------------Profile 2----------------------\n");
 
    /* Video Encoder Configuration */
 //   printf("-------------------------------VideoEncoderConfiguration----------------Profile 2---------------------\n");
   
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration =
        (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 3);

    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 3);

    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->UseCount = 1;

    /* TODO: GauranteedFrameRate : Currently no need */

    // Detect codec type dynamically (2=H264, 3=H265)
    int codec_type_p2 = 2;
    nvt_adapter_video_get_video_codec(2, &codec_type_p2);
    // Always use H264 for ONVIF compatibility
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Encoding = tt__VideoEncoding__H264;
   
     
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Resolution, 0x00, sizeof(struct tt__VideoResolution));

    nvt_adapter_video_get_resolution(2, value);
    sscanf(value, "%d*%d", &trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Resolution->Width, & trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Resolution->Height);
    
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Quality = 10.0;

    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->RateControl, 0x00, sizeof(struct tt__VideoRateControl));

   // trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->RateControl->FrameRateLimit = 20;
     nvt_adapter_video_get_frame_rate(2, &trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->RateControl->FrameRateLimit);
   // trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->RateControl->FrameRateLimit = 20;
    //free(value);
    /* TODO: EncodingInterval: Following parameter create confusion */
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->RateControl->EncodingInterval = 1;

    //@TODO, check bitrate for 640*480p
   // trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->RateControl->BitrateLimit = 1024;
     nvt_adapter_video_get_max_rate(2,&trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->RateControl->BitrateLimit);

    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->MPEG4 = NULL;
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->H264, 0x00, sizeof(struct tt__H264Configuration));
    /* Determines the interval in which the I-Frames will be coded */
    //trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->H264->GovLength = 1;
    nvt_adapter_video_get_gop(2, &trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->H264->GovLength);

   // trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main; //tt__H264Profile__Baseline,tt__H264Profile__High,tt__H264Profile__Main;

    h264_profile_val = 0;
    nvt_adapter_video_get_h264_profile(2, &h264_profile_val);
    printf("H264 PROFILE value profile 2------2222 %d\n", h264_profile_val);
    if(h264_profile_val == 0)
    {    
            trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Baseline;
    }
    else if(h264_profile_val == 1)
    {
            trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__High;
    }
    else 
    {
            trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->H264->H264Profile = tt__H264Profile__Main;
    }

    free(value);
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Multicast =
        (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Multicast->Address =
        (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
    MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

  //  trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4; //Currently only support of IPv4
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Multicast->Address->Type = 0;
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         MEMSET(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         strcpy(trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->Multicast->Address->IPv4Address,"0.0.0.0");

 //   printf("-------------------------------VideoEncoderConfiguration---SetSessionTimeout-------------Profile 2---------------------\n");


    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->SessionTimeout  = timeout;


 //   printf("-------------------------------VideoEncoderConfiguration---After SetSessionTimeout-------------Profile 2---------------------\n");

    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->__size = 0;
    trt__GetProfilesResponse->Profiles[2].VideoEncoderConfiguration->__any = NULL;
  //  dom_att(&trt__GetProfilesResponse->Profiles[0].VideoEncoderConfiguration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
    //Video Encoder Configuration

/* AudioEncoderConfiguration */
trt__GetProfilesResponse->Profiles[2].AudioEncoderConfiguration = NULL;

//    printf("-------------------------------AudioEncoderConfiguration--------------------Profile 2-----------------\n");
    //Metadata Configurations
 //   printf("-------------------------------MetadataConfiguration-----------------Profile 2--------------------\n");
    trt__GetProfilesResponse->Profiles[2].MetadataConfiguration = NULL;

    /* VideoAnalyticsConfiguration */
 //    printf("-------------------------------VideoAnalyticsConfiguration-------------Profile 2------------------------\n");
    // trt__GetProfilesResponse->Profiles[2].VideoAnalyticsConfiguration = NULL;

 //    printf("-------------------------------PTZConfiguration------------------Profile 2-------------------\n");
     trt__GetProfilesResponse->Profiles[2].PTZConfiguration = NULL;
     
 //    printf("-------------------------------Extension-------------------------Profile 2------------\n");

     trt__GetProfilesResponse->Profiles[2].Extension = NULL;

 //    dom_att(&trt__GetProfilesResponse->Profiles[1].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

 //******************************************************************profile_2*********************************************************
        
 //   printf("ProfileToken = %s\n", trt__GetProfilesResponse->Profiles[2].token);
    return SOAP_OK;
}


/** Web service operation '__trt__GetVideoEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(struct soap* soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{

         printf("-------------------------------__trt__GetVideoEncoderConfiguration-------------------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int profIndex = -1;
    char *value = malloc(20);
    const char *profile_value;

    if (NULL != trt__GetVideoEncoderConfiguration)
    {
        if (NULL != trt__GetVideoEncoderConfiguration->ConfigurationToken)
        {
            printf("Given profile token is = %s\n", trt__GetVideoEncoderConfiguration->ConfigurationToken);
            profIndex = onvif_encoder_token_to_channel(trt__GetVideoEncoderConfiguration->ConfigurationToken);

            
            if (-1 == profIndex)
            {
              //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
                printf("Given profile token not found is = %s\n", trt__GetVideoEncoderConfiguration->ConfigurationToken);
                return SOAP_FAULT;
            }
            else
            {
                printf("Given profile token found Index = %d\n", profIndex);
            }
            
        }
    }
    if(profIndex ==0)
    {
        trt__GetVideoEncoderConfigurationResponse->Configuration =
            (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

        trt__GetVideoEncoderConfigurationResponse->Configuration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetVideoEncoderConfigurationResponse->Configuration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 1);

        trt__GetVideoEncoderConfigurationResponse->Configuration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 1);

         if(testProfilecreated == false || testProfileAddVideoEncoderConfig==false)
             trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = 1;
        else  
            trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = 2; 
            
        trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding = tt__VideoEncoding__H264;
    
        
        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution =
            (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));

        nvt_adapter_video_get_resolution(0, value);
        sscanf(value, "%d*%d", &trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width, &trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height);
         
        trt__GetVideoEncoderConfigurationResponse->Configuration->Quality =10.0;

        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl =
            (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));

        nvt_adapter_video_get_frame_rate(0, &trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit);
    //    free(value);

        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = 1;
        nvt_adapter_video_get_max_rate(0,&trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit);
   
        trt__GetVideoEncoderConfigurationResponse->Configuration->MPEG4 = NULL;
        trt__GetVideoEncoderConfigurationResponse->Configuration->H264 =
            (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        /* Determines the interval in which the I-Frames will be coded */
        nvt_adapter_video_get_gop(0,&trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength);
  
       // trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Main;// tt__H264Profile__Baseline; //tt__H264Profile__Baseline;tt__H264Profile__High
        int h264_profile_val = 0;
        nvt_adapter_video_get_h264_profile(0, &h264_profile_val);
        printf("H264 PROFILE value profile 0 ------000000 %d\n", h264_profile_val);
        if(h264_profile_val == 0)
        {    
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Baseline;
        }
        else if(h264_profile_val == 1)
        {
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__High;
        }
        else 
        {
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Main;
        }
        free(value);
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast =
            (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type = 0; //TODO: This param should be dynamic

        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         strcpy(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address,"0.0.0.0");
        
        
      trt__GetVideoEncoderConfigurationResponse->Configuration->SessionTimeout = timeout;


        trt__GetVideoEncoderConfigurationResponse->Configuration->__size = 0;
        trt__GetVideoEncoderConfigurationResponse->Configuration->__any = NULL;
    }
    else if (profIndex ==1)
    {
            trt__GetVideoEncoderConfigurationResponse->Configuration =
            (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

        trt__GetVideoEncoderConfigurationResponse->Configuration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetVideoEncoderConfigurationResponse->Configuration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 2);

        trt__GetVideoEncoderConfigurationResponse->Configuration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 2);

        trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = 1;
            
        trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding = tt__VideoEncoding__H264;
    
        
        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution =
            (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));

        nvt_adapter_video_get_resolution(1, value);
        sscanf(value, "%d*%d", & trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width, & trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height);
      
        trt__GetVideoEncoderConfigurationResponse->Configuration->Quality =10.0;

        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl =
            (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));

        nvt_adapter_video_get_frame_rate(1, &trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit);
        //trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit = fps_chn_1;
       // free(value);
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = 1;
        nvt_adapter_video_get_max_rate(1,&trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit);
    
        trt__GetVideoEncoderConfigurationResponse->Configuration->MPEG4 = NULL;
        trt__GetVideoEncoderConfigurationResponse->Configuration->H264 =
            (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        /* Determines the interval in which the I-Frames will be coded */
        nvt_adapter_video_get_gop(1,&trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength);
       // trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Main;//tt__H264Profile__Baseline; //tt__H264Profile__Baseline;tt__H264Profile__High
        int h264_profile_val = 0;
        nvt_adapter_video_get_h264_profile(1, &h264_profile_val);
        printf("H264 PROFILE value profile 1-------111111 %d\n", h264_profile_val);
        if(h264_profile_val == 0)
        {    
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Baseline;
        }
        else if(h264_profile_val == 1)
        {
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__High;
        }
        else 
        {
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Main;
        }
        free(value);
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast =
            (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type = 0; //TODO: This param should be dynamic

        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         strcpy(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address,"0.0.0.0");
        
        
      trt__GetVideoEncoderConfigurationResponse->Configuration->SessionTimeout = timeout;


        trt__GetVideoEncoderConfigurationResponse->Configuration->__size = 0;
        trt__GetVideoEncoderConfigurationResponse->Configuration->__any = NULL;
    }
    else if (profIndex ==2)
    {
        trt__GetVideoEncoderConfigurationResponse->Configuration =
            (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration, 0x00, sizeof(struct tt__VideoEncoderConfiguration));

        trt__GetVideoEncoderConfigurationResponse->Configuration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetVideoEncoderConfigurationResponse->Configuration->token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 3);

        trt__GetVideoEncoderConfigurationResponse->Configuration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 3);

        trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = 1;
            
        trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding = tt__VideoEncoding__H264;
    
        
        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution =
            (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));

        nvt_adapter_video_get_resolution(2, value);
        sscanf(value, "%d*%d", & trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width, & trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height);
    
        trt__GetVideoEncoderConfigurationResponse->Configuration->Quality =10.0;

        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl =
            (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));

        nvt_adapter_video_get_frame_rate(2, &trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit);
        // free(value);
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = 1;
        nvt_adapter_video_get_max_rate(2,&trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit);
        trt__GetVideoEncoderConfigurationResponse->Configuration->MPEG4 = NULL;
        trt__GetVideoEncoderConfigurationResponse->Configuration->H264 =
            (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        /* Determines the interval in which the I-Frames will be coded */
        nvt_adapter_video_get_gop(2,&trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength);
    
      //  trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Main;//tt__H264Profile__Baseline; //tt__H264Profile__Baseline;tt__H264Profile__High
        int h264_profile_val = 0;
        nvt_adapter_video_get_h264_profile(2, &h264_profile_val);
        printf("H264 PROFILE value profile 2-------222222 %d\n", h264_profile_val);
        if(h264_profile_val == 0)
        {    
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Baseline;
        }
        else if(h264_profile_val == 1)
        {
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__High;
        }
        else 
        {
                trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Main;
        }

        free(value);
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast =
            (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type = 0; //TODO: This param should be dynamic

        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address =
        (char *)soap_malloc(soap, sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         MEMSET(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 0x00,
         sizeof(char) * (IPV4_ADDR_STRING_LEN + 1));
         strcpy(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address,"0.0.0.0");
        
        
      trt__GetVideoEncoderConfigurationResponse->Configuration->SessionTimeout = timeout;

     
        trt__GetVideoEncoderConfigurationResponse->Configuration->__size = 0;
        trt__GetVideoEncoderConfigurationResponse->Configuration->__any = NULL;
    }
    return 0;
}

//****************************For Streaming**************************************************************

/** Web service operation '__trt__GetStreamUri' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap* soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
    printf("--------------------------__trt__GetStreamUri---------------------------\n");
    
    // *** DYNAMIC IP REFRESH ***
    // Ensure we have the current IP address for RTSP URL
    if (get_ip_address() < 0 || ip_address == NULL) {
        printf("ERROR: Failed to get IP address for stream URI\n");
        onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:ConfigurationConflict", "Failed to determine device IP address");
        return SOAP_FAULT;
    }
    printf("GetStreamUri: Using IP address: %s\n", ip_address);
    
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == trt__GetStreamUri)
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        printf("Invalid argument. trt__GetStreamUri[%p]\n", trt__GetStreamUri);
        return SOAP_FAULT;
    }

    trt__GetStreamUriResponse->MediaUri = (struct tt__MediaUri *)soap_malloc(soap, sizeof(struct tt__MediaUri));
    if (trt__GetStreamUriResponse->MediaUri == NULL) {
        printf("ERROR: Failed to allocate MediaUri for stream\n");
        return SOAP_EOM;
    }
    memset(trt__GetStreamUriResponse->MediaUri, 0, sizeof(struct tt__MediaUri));


     int profIndex = -1;

    if (NULL != trt__GetStreamUri)
    {
        if (NULL != trt__GetStreamUri->ProfileToken)
        {
            printf("Given profile token is = %s\n", trt__GetStreamUri->ProfileToken);
            profIndex = onvif_profile_token_to_channel(trt__GetStreamUri->ProfileToken);

            
            if (-1 == profIndex)
            {
                onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
                printf("Given profile token not found is = %s\n", trt__GetStreamUri->ProfileToken);
                return SOAP_FAULT;
            }
            else
            {
                printf("Given profile token found Index = %d\n", profIndex);
            }
            
        }
    }
    
    // Log and validate Stream type (RTP-Unicast/Multicast)
    printf("StreamSetup->Stream = %d (%s)\n", 
           trt__GetStreamUri->StreamSetup->Stream,
           (trt__GetStreamUri->StreamSetup->Stream == tt__StreamType__RTP_Unicast) ? "RTP-Unicast" : "RTP-Multicast");
    
    // Only support RTP-Unicast
    if (trt__GetStreamUri->StreamSetup->Stream != tt__StreamType__RTP_Unicast) {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InternalError", "Only RTP-Unicast is supported");
        printf("Unsupported stream type: %d (only RTP-Unicast supported)\n", trt__GetStreamUri->StreamSetup->Stream);
        return SOAP_FAULT;
    }
    
    // Validate transport protocol
    const char* protocol_names[] = {"UDP", "TCP", "RTSP", "HTTP"};
    printf("StreamSetup->Transport->Protocol = %d (%s)\n",
           trt__GetStreamUri->StreamSetup->Transport->Protocol,
           (trt__GetStreamUri->StreamSetup->Transport->Protocol >= 0 && 
            trt__GetStreamUri->StreamSetup->Transport->Protocol <= 3) ?
           protocol_names[trt__GetStreamUri->StreamSetup->Transport->Protocol] : "Unknown");
    
    // Accept both RTSP and TCP (some clients use TCP for RTSP interleaved mode)
    switch (trt__GetStreamUri->StreamSetup->Transport->Protocol)
    {
    case tt__TransportProtocol__RTSP:  // Value 2
    case tt__TransportProtocol__TCP:   // Value 1 (for RTSP over TCP)
        printf("Transport protocol accepted: %s\n", 
               (trt__GetStreamUri->StreamSetup->Transport->Protocol == tt__TransportProtocol__RTSP) ? "RTSP" : "TCP");
        break;
    case tt__TransportProtocol__HTTP:
    case tt__TransportProtocol__UDP:
    default:
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InternalError","Only RTSP/TCP transport is supported");
        printf("Unsupported transport protocol: %d\n", trt__GetStreamUri->StreamSetup->Transport->Protocol);
        return SOAP_FAULT;
    }
   
    // Use dynamic RTSP server interface to get stream URI
    trt__GetStreamUriResponse->MediaUri->Uri = (char *)soap_malloc(soap, sizeof(char) * 256);
    memset(trt__GetStreamUriResponse->MediaUri->Uri, '\0', sizeof(char) * 256);
    
    // Get the dynamic URI from RTSP server using profile token
    if (!rtsp_get_onvif_stream_uri(trt__GetStreamUri->ProfileToken, ip_address, 
                                   trt__GetStreamUriResponse->MediaUri->Uri, 256)) {
        // Fallback to hardcoded URI if RTSP server is not available
        sprintf(trt__GetStreamUriResponse->MediaUri->Uri, "rtsp://%s:554/stream%d", ip_address, profIndex);
        printf("RTSP server unavailable, using fallback URI: %s\n", trt__GetStreamUriResponse->MediaUri->Uri);
    } else {
        printf("Got dynamic stream URI from RTSP server: %s\n", trt__GetStreamUriResponse->MediaUri->Uri);
    }
    
    trt__GetStreamUriResponse->MediaUri->InvalidAfterConnect = xsd__boolean__true_;
    trt__GetStreamUriResponse->MediaUri->InvalidAfterReboot  = xsd__boolean__true_;
    trt__GetStreamUriResponse->MediaUri->Timeout = timeout;

    return 0;
}
/** Web service operation '__trt__GetVideoSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(struct soap* soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
    printf("--------------------------__trt__GetVideoSourceConfiguration---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    //only one source configuration for single camera
    trt__GetVideoSourceConfigurationResponse->Configuration =
                    (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
    MEMSET(trt__GetVideoSourceConfigurationResponse->Configuration, 0x00, sizeof(struct tt__VideoSourceConfiguration));

   
        trt__GetVideoSourceConfigurationResponse->Configuration->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoSourceConfigurationResponse->Configuration->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetVideoSourceConfigurationResponse->Configuration->token,vsrc_confToken);

        trt__GetVideoSourceConfigurationResponse->Configuration->Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoSourceConfigurationResponse->Configuration->Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetVideoSourceConfigurationResponse->Configuration->Name,vsrc_name);

        
        if(testProfilecreated == false || testProfileAddSourceConfig==false)
            trt__GetVideoSourceConfigurationResponse->Configuration->UseCount = vsrc_useCount;
        else  
            trt__GetVideoSourceConfigurationResponse->Configuration->UseCount = vsrc_useCount+1;


        trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken,vsrc_srcToken);
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->x = vsrc_boundX;
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->y = vsrc_boundY;

        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->width = vsrc_boundWidth;
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->height = vsrc_boundHeight;

        trt__GetVideoSourceConfigurationResponse->Configuration->__size = 0;
        trt__GetVideoSourceConfigurationResponse->Configuration->__any = NULL;

        trt__GetVideoSourceConfigurationResponse->Configuration->Extension = NULL; 
    return 0;
}
/** Web service operation '__trt__GetVideoEncoderConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(struct soap *soap,
                                                                     struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions,
                                                                     struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
     printf("--------------------------__trt__GetVideoEncoderConfigurationOptions---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    int profIndex = -1;

    if (NULL != trt__GetVideoEncoderConfigurationOptions)
    {
//        printf("--------------------------__trt__GetVideoEncoderConfigurationOptions is not null ---------------------------\n");
        if (NULL != trt__GetVideoEncoderConfigurationOptions->ProfileToken)
        {
            printf("Given profile token is = %s\n", trt__GetVideoEncoderConfigurationOptions->ProfileToken);

            profIndex = onvif_profile_token_to_channel(trt__GetVideoEncoderConfigurationOptions->ProfileToken);

            
            if (-1 == profIndex)
            {
              //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
                printf("Given profile token not found is = %s\n", trt__GetVideoEncoderConfigurationOptions->ProfileToken);
                return SOAP_FAULT;
            }
            else
            {
                printf("Given profile token found Index = %d\n", profIndex);
            }
            
        }
        else if(NULL != trt__GetVideoEncoderConfigurationOptions->ConfigurationToken)
        {
            printf("Given ConfigurationToken token is = %s\n", trt__GetVideoEncoderConfigurationOptions->ConfigurationToken);

            profIndex = onvif_encoder_token_to_channel(trt__GetVideoEncoderConfigurationOptions->ConfigurationToken);

            
            if (-1 == profIndex)
            {
              //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
                printf("Given VideoEncoderToken not found is = %s\n", trt__GetVideoEncoderConfigurationOptions->ConfigurationToken);
                return SOAP_FAULT;
            }
            else
            {
                printf("Given VideoEncoderToken found Index = %d\n", profIndex);
            }

        }
        else{
            //no token is provided
            printf("--------------------------__trt__GetVideoEncoderConfigurationOptions no token is provided ---------------------------\n");
             profIndex =0;
        }
    }
    else
    {
        printf("--------------------------_Invalid input argument trt__GetVideoEncoderConfigurationOptions---------------------------\n");
        return SOAP_FAULT;
    }
    if(profIndex == 0)
    {
        trt__GetVideoEncoderConfigurationOptionsResponse->Options =
            (struct tt__VideoEncoderConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfigurationOptions));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options, 0x00, sizeof(struct tt__VideoEncoderConfigurationOptions));

        /** Required element 'tt:QualityRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange, 0x00, sizeof(struct tt__IntRange));
        /** Required element 'tt:Min' of XML schema type 'xsd:int' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min = 6;//1;   //TODO
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max = 14;//100; //TODO

        trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG = NULL;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4 = NULL;

    //-------------------------------------H.264----------------------------------------------------------------
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 =
            (struct tt__H264Options *)soap_malloc(soap, sizeof(struct tt__H264Options));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264, 0x00, sizeof(struct tt__H264Options));

    // trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable = 2;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable = 3;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable =
            (struct tt__VideoResolution *)soap_malloc(soap,
                                                    trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable, 0x00,
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));

        
    //   for (int i = 0; i < trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable; i++)
    //   {
        #ifdef LUCKFOX
        
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Width = 2304;//2592;//2560;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Height = 1296;//1944;//1440;

                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Width = 1920;//1280;//704;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Height = 1080;//720;//576;

                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[2].Width = 1280;//720;//704;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[2].Height = 720;//576;//576;
        #else
             trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Width = 2592;//2592;//2560;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Height = 1944;//1944;//1440;

                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Width = 2048;//1280;//704;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Height = 1536;//720;//576;

                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[2].Width = 1920;//720;//704;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[2].Height = 1440;//576;//576;
      #endif

    //   }

        /** Required element 'tt:GovLengthRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange, 0x00, sizeof(struct tt__IntRange));

            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Min = 1;//10;// 1;
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Max = 60;//100;// 60;//120;
    
        /** Required element 'tt:FrameRateRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange, 0x00, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min = 1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max = 30; //could be 25
        /** Required element 'tt:EncodingIntervalRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange, 0x00, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min = 1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max = 1;// 5;
    /** Sequence of at least 1 elements 'tt:H264ProfilesSupported' of XML schema type 'tt:H264Profile' stored in dynamic array H264ProfilesSupported of length __sizeH264ProfilesSupported */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported = 3;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported =
            (enum tt__H264Profile *)soap_malloc(soap, trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported, 0x00,
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
            /*
        for (INT32 i = 0; i < trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported; i++)
        {
            //Currently we are supporting only Main profile and does not have option to modify profile 
            if (0 == STRCMP(vH264Profile[i].c_str(), "Main"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__Main;
            else if (0 == STRCMP(vH264Profile[i].c_str(), "Baseline"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__Baseline;
            else if (0 == STRCMP(vH264Profile[i].c_str(), "High"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__High;
        }
    */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[0] =  tt__H264Profile__Main;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[1] = tt__H264Profile__Baseline;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[2] = tt__H264Profile__High;

    //-----------------------------------H.264---------------------------------------------------------------------------
    //    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension = NULL;
         /** Optional element 'tt:Extension' of XML schema type 'tt:VideoEncoderOptionsExtension' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension =
        (struct tt__VideoEncoderOptionsExtension *)soap_malloc(soap, sizeof(struct tt__VideoEncoderOptionsExtension));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension, 0x00, sizeof(struct tt__VideoEncoderOptionsExtension));

    /** Optional element 'tt:JPEG' of XML schema type 'tt:JpegOptions2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->JPEG = NULL;
      

    /** Optional element 'tt:H264' of XML schema type 'tt:H264Options2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264 =
        (struct tt__H264Options2 *)soap_malloc(soap, sizeof(struct tt__H264Options2));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264, 0x00, sizeof(struct tt__H264Options2));
    /** Sequence of at least 1 elements 'tt:ResolutionsAvailable' of XML schema type 'tt:VideoResolution' stored in dynamic array ResolutionsAvailable of length __sizeResolutionsAvailable */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable = 3;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable =
        (struct tt__VideoResolution *)soap_malloc(soap,
                                                  trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable, 0x00,
           trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));
    
    #ifdef LUCKFOX

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[0].Width= 2304;//2592;//2560;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[0].Height= 1296;//1944;//1440;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[1].Width = 1920;//1280;//704;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[1].Height = 1080;//720;//576;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[2].Width = 1280;//720;//704;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[2].Height = 720;//576;//576;
    #else
     trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[0].Width= 2592;//2592;//2560;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[0].Height= 1944;//1944;//1440;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[1].Width = 2048;//1280;//704;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[1].Height = 1536;//720;//576;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[2].Width = 1920;//720;//704;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[2].Height = 1440;//576;//576;
    #endif
   

    /** Required element 'tt:GovLengthRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange, 0x00, sizeof(struct tt__IntRange));
    
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange->Min = 1;//10;//1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange->Max = 60;//100;//60;//120;
   
    /** Required element 'tt:FrameRateRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange->Min =1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange->Max = 30; //could be 25
    /** Required element 'tt:EncodingIntervalRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange->Min = 1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange->Max = 1;//5;
    /** Sequence of at least 1 elements 'tt:H264ProfilesSupported' of XML schema type 'tt:H264Profile' stored in dynamic array H264ProfilesSupported of length __sizeH264ProfilesSupported */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported = 3;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported =
        (enum tt__H264Profile *)soap_malloc(soap, trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported, 0x00,
           trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
    
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[0] =  tt__H264Profile__Main;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[1] = tt__H264Profile__Baseline;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[2] = tt__H264Profile__High;
   
    /** Required element 'tt:BitrateRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Min = 32;//1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Max = 16384;//1024;//8192;//1024;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__size = 0;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__any = NULL;

    /** Optional element 'tt:Extension' of XML schema type 'tt:VideoEncoderOptionsExtension2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->Extension = NULL;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->__size = 0;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->__any = NULL;


        /** Optional attribute 'GuaranteedFrameRateSupported' of XML schema type 'xsd:boolean' */
    //    trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported =
    //        (xsd__boolean *)soap_malloc(soap, sizeof(xsd__boolean));
    //    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported, 0x00, sizeof(xsd__boolean));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported = xsd__boolean__false_; //TODO
        /** XML DOM attribute list */
    // dom_att(&trt__GetVideoEncoderConfigurationOptionsResponse->Options->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

        //dest

    }
    else if(profIndex ==1)
    {
        //to avoid frame rate change during switching between main and sub stream
        // rk_isp_set_frame_rate(ctx->stViCfg[1].pipeId, fps_chn_1);

        trt__GetVideoEncoderConfigurationOptionsResponse->Options =
            (struct tt__VideoEncoderConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfigurationOptions));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options, 0x00, sizeof(struct tt__VideoEncoderConfigurationOptions));

        /** Required element 'tt:QualityRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange, 0x00, sizeof(struct tt__IntRange));
        /** Required element 'tt:Min' of XML schema type 'xsd:int' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min = 1;   //TODO
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max = 100; //TODO

        trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG = NULL;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4 = NULL;

    //-------------------------------------H.264----------------------------------------------------------------
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 =
            (struct tt__H264Options *)soap_malloc(soap, sizeof(struct tt__H264Options));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264, 0x00, sizeof(struct tt__H264Options));

    // trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable = 2;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable = 3;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable =
            (struct tt__VideoResolution *)soap_malloc(soap,
                                                    trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable, 0x00,
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));

        
    //   for (int i = 0; i < trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable; i++)
    //   {
        
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Width = 1600;//720;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Height = 1200;//576;

                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Width = 1360;//640;//704;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Height = 768;//360;//576;

                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[2].Width = 1280;//352;//704;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[2].Height = 720;//288;//576;

    //   }

        /** Required element 'tt:GovLengthRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange, 0x00, sizeof(struct tt__IntRange));

            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Min = 1;
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Max = 60;//120;
    
        /** Required element 'tt:FrameRateRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange, 0x00, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min = 1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max = 30;
        /** Required element 'tt:EncodingIntervalRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange, 0x00, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min = 1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max = 5;
    /** Sequence of at least 1 elements 'tt:H264ProfilesSupported' of XML schema type 'tt:H264Profile' stored in dynamic array H264ProfilesSupported of length __sizeH264ProfilesSupported */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported = 3;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported =
            (enum tt__H264Profile *)soap_malloc(soap, trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported, 0x00,
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
            /*
        for (INT32 i = 0; i < trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported; i++)
        {
            //Currently we are supporting only Main profile and does not have option to modify profile 
            if (0 == STRCMP(vH264Profile[i].c_str(), "Main"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__Main;
            else if (0 == STRCMP(vH264Profile[i].c_str(), "Baseline"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__Baseline;
            else if (0 == STRCMP(vH264Profile[i].c_str(), "High"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__High;
        }
    */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[0] =  tt__H264Profile__Main;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[1] = tt__H264Profile__Baseline;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[2] = tt__H264Profile__High;

    //-----------------------------------H.264---------------------------------------------------------------------------
    //    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension = NULL;

          /** Optional element 'tt:Extension' of XML schema type 'tt:VideoEncoderOptionsExtension' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension =
        (struct tt__VideoEncoderOptionsExtension *)soap_malloc(soap, sizeof(struct tt__VideoEncoderOptionsExtension));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension, 0x00, sizeof(struct tt__VideoEncoderOptionsExtension));

    /** Optional element 'tt:JPEG' of XML schema type 'tt:JpegOptions2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->JPEG = NULL;
      

    /** Optional element 'tt:H264' of XML schema type 'tt:H264Options2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264 =
        (struct tt__H264Options2 *)soap_malloc(soap, sizeof(struct tt__H264Options2));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264, 0x00, sizeof(struct tt__H264Options2));
    /** Sequence of at least 1 elements 'tt:ResolutionsAvailable' of XML schema type 'tt:VideoResolution' stored in dynamic array ResolutionsAvailable of length __sizeResolutionsAvailable */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable = 3;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable =
        (struct tt__VideoResolution *)soap_malloc(soap,
                                                  trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable, 0x00,
           trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));

   
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[0].Width= 1600;//720;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[0].Height= 1200;//576;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[1].Width = 1360;//640;//704;;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[1].Height = 768;//360;//576;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[2].Width = 1280;//352;//704;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[2].Height = 720;//288;//576;
   

    /** Required element 'tt:GovLengthRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange, 0x00, sizeof(struct tt__IntRange));
    
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange->Min = 1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange->Max = 60;//120;
   
    /** Required element 'tt:FrameRateRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange->Min =1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange->Max = 30;
    /** Required element 'tt:EncodingIntervalRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange->Min = 1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange->Max = 5;
    /** Sequence of at least 1 elements 'tt:H264ProfilesSupported' of XML schema type 'tt:H264Profile' stored in dynamic array H264ProfilesSupported of length __sizeH264ProfilesSupported */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported = 3;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported =
        (enum tt__H264Profile *)soap_malloc(soap, trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported, 0x00,
           trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
    
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[0] =  tt__H264Profile__Main;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[1] = tt__H264Profile__Baseline;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[2] = tt__H264Profile__High;
   
    /** Required element 'tt:BitrateRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Min = 1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Max = 512;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__size = 0;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__any = NULL;

    /** Optional element 'tt:Extension' of XML schema type 'tt:VideoEncoderOptionsExtension2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->Extension = NULL;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->__size = 0;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->__any = NULL;

    /** Optional attribute 'GuaranteedFrameRateSupported' of XML schema type 'xsd:boolean' */
    //    trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported =
    //        (xsd__boolean *)soap_malloc(soap, sizeof(xsd__boolean));
    //    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported, 0x00, sizeof(xsd__boolean));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported = xsd__boolean__false_; //TODO
        /** XML DOM attribute list */
    // dom_att(&trt__GetVideoEncoderConfigurationOptionsResponse->Options->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

        //dest
        
    }
    else if(profIndex == 2)
    {
        trt__GetVideoEncoderConfigurationOptionsResponse->Options =
            (struct tt__VideoEncoderConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfigurationOptions));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options, 0x00, sizeof(struct tt__VideoEncoderConfigurationOptions));

        /** Required element 'tt:QualityRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange, 0x00, sizeof(struct tt__IntRange));
        /** Required element 'tt:Min' of XML schema type 'xsd:int' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min = 1;   //TODO
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max = 100; //TODO

        trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG = NULL;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4 = NULL;

    //-------------------------------------H.264----------------------------------------------------------------
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 =
            (struct tt__H264Options *)soap_malloc(soap, sizeof(struct tt__H264Options));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264, 0x00, sizeof(struct tt__H264Options));

    //trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable = 2;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable = 3;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable =
            (struct tt__VideoResolution *)soap_malloc(soap,
                                                    trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable, 0x00,
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));

        
    //   for (int i = 0; i < trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable; i++)
    //   {
        
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Width = 704;//640;//960;//800;;//2592;//2560;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Height = 576;//360;//540;//600;///1944;//1440;

                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Width = 640;//352;//704;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Height = 360;//288;//520;//576;///576;

                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[2].Width = 352;//640;//300;//704;
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[2].Height = 288;//360;//200;//576;

    //   }

        /** Required element 'tt:GovLengthRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange, 0x00, sizeof(struct tt__IntRange));

            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Min = 1;
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Max = 60;//120;
    
        /** Required element 'tt:FrameRateRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange, 0x00, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min = 1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max = 30;
        /** Required element 'tt:EncodingIntervalRange' of XML schema type 'tt:IntRange' */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange =
            (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange, 0x00, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min = 1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max = 5;
    /** Sequence of at least 1 elements 'tt:H264ProfilesSupported' of XML schema type 'tt:H264Profile' stored in dynamic array H264ProfilesSupported of length __sizeH264ProfilesSupported */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported = 3;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported =
            (enum tt__H264Profile *)soap_malloc(soap, trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
        MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported, 0x00,
            trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
            /*
        for (INT32 i = 0; i < trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported; i++)
        {
            //Currently we are supporting only Main profile and does not have option to modify profile 
            if (0 == STRCMP(vH264Profile[i].c_str(), "Main"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__Main;
            else if (0 == STRCMP(vH264Profile[i].c_str(), "Baseline"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__Baseline;
            else if (0 == STRCMP(vH264Profile[i].c_str(), "High"))
                trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[i] = tt__H264Profile__High;
        }
    */
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[0] =  tt__H264Profile__Main;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[1] = tt__H264Profile__Baseline;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[2] = tt__H264Profile__High;

    //-----------------------------------H.264---------------------------------------------------------------------------
    //    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension = NULL;
         /** Optional element 'tt:Extension' of XML schema type 'tt:VideoEncoderOptionsExtension' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension =
        (struct tt__VideoEncoderOptionsExtension *)soap_malloc(soap, sizeof(struct tt__VideoEncoderOptionsExtension));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension, 0x00, sizeof(struct tt__VideoEncoderOptionsExtension));

    /** Optional element 'tt:JPEG' of XML schema type 'tt:JpegOptions2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->JPEG = NULL;
      

    /** Optional element 'tt:H264' of XML schema type 'tt:H264Options2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264 =
        (struct tt__H264Options2 *)soap_malloc(soap, sizeof(struct tt__H264Options2));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264, 0x00, sizeof(struct tt__H264Options2));
    /** Sequence of at least 1 elements 'tt:ResolutionsAvailable' of XML schema type 'tt:VideoResolution' stored in dynamic array ResolutionsAvailable of length __sizeResolutionsAvailable */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable = 3;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable =
        (struct tt__VideoResolution *)soap_malloc(soap,
                                                  trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable, 0x00,
           trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));

   
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[0].Width= 704;//960;//640;//2592;//2560;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[0].Height= 576;//540;//360;//1944;//1440;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[1].Width = 640;//720;//352;//704;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[1].Height = 360;//520;//576;//288;//576;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[2].Width = 352;//640;//300;//704;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable[2].Height = 288;//360;//200;//576;
   

    /** Required element 'tt:GovLengthRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange, 0x00, sizeof(struct tt__IntRange));
    
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange->Min = 1;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange->Max = 60;//120;
   
    /** Required element 'tt:FrameRateRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange->Min =1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange->Max = 30;
    /** Required element 'tt:EncodingIntervalRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange->Min = 1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange->Max = 5;
    /** Sequence of at least 1 elements 'tt:H264ProfilesSupported' of XML schema type 'tt:H264Profile' stored in dynamic array H264ProfilesSupported of length __sizeH264ProfilesSupported */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported = 3;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported =
        (enum tt__H264Profile *)soap_malloc(soap, trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported, 0x00,
           trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));
    
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[0] =  tt__H264Profile__Main;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[1] = tt__H264Profile__Baseline;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported[2] = tt__H264Profile__High;
   
    /** Required element 'tt:BitrateRange' of XML schema type 'tt:IntRange' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange =
        (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Min = 1;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Max = 1024;//8192;//1024;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__size = 0;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__any = NULL;

    /** Optional element 'tt:Extension' of XML schema type 'tt:VideoEncoderOptionsExtension2' */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->Extension = NULL;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->__size = 0;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->__any = NULL;


        /** Optional attribute 'GuaranteedFrameRateSupported' of XML schema type 'xsd:boolean' */
    //    trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported =
    //        (xsd__boolean *)soap_malloc(soap, sizeof(xsd__boolean));
    //    MEMSET(trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported, 0x00, sizeof(xsd__boolean));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported = xsd__boolean__false_; //TODO
        /** XML DOM attribute list */
    // dom_att(&trt__GetVideoEncoderConfigurationOptionsResponse->Options->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

        //dest

    }
    return SOAP_OK;
}
/** Web service operation '__trt__SetVideoEncoderConfiguration' implementation, should return SOAP_OK or error code */
/** Web service operation '__trt__SetVideoEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(struct soap *soap,
                                                              struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration,
                                                              struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{

    int32_t s32Ret = 0;
    int profIndex = -1;

     printf("--------------------------__trt__SetVideoEncoderConfiguration---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == trt__SetVideoEncoderConfiguration)
    {
        printf("--------------------------__trt__SetVideoEncoderConfiguration ---------NULL-----------------\n");
        return SOAP_FAULT;
    }

    if (NULL == trt__SetVideoEncoderConfiguration->Configuration)
    {
       printf("--------------------------__trt__SetVideoEncoderConfiguration ---------Config NULL-----------------\n");
       return SOAP_FAULT;
    }

    if ((NULL == trt__SetVideoEncoderConfiguration->Configuration->token) || (0 == strlen(trt__SetVideoEncoderConfiguration->Configuration->token)))
    {
       printf("--------------------------__trt__SetVideoEncoderConfiguration ---------Config token NULL-----------------\n");
       return SOAP_FAULT;
    }
     if (NULL != trt__SetVideoEncoderConfiguration->Configuration->token)
    {
        printf("Given profile token is = %s\n", trt__SetVideoEncoderConfiguration->Configuration->token);
        profIndex = onvif_encoder_token_to_channel(trt__SetVideoEncoderConfiguration->Configuration->token);

        
        if (-1 == profIndex)
        {
            //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
            printf("Given profile token not found is = %s\n", trt__SetVideoEncoderConfiguration->Configuration->token);
            return SOAP_FAULT;
        }
        else
        {
            printf("Given profile token found Index = %d\n", profIndex);
        }
    }
    
  
    if (profIndex == 2)
    {
         s32Ret = nvt_adapter_model_stop();
    }
    
    // Set Resolution
    int width = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
    int height = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
    printf("--------------------Set Resolution: %dx%d---------------------\n", width, height);
    
    if (onvif_set_resolution(profIndex, width, height) != 0) {
        printf("Failed to set resolution\n");
        return SOAP_FAULT;
    }

    // Set Frame Rate
    if( trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit >=1 )
    {
         int fps = trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
         onvif_set_framerate(profIndex, fps);
    }
     
    // Set Bitrate
    int bitrate = trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
    onvif_set_video_bitrate(profIndex, bitrate);

    // Set H264 Settings
    if (trt__SetVideoEncoderConfiguration->Configuration->H264) {
        if(trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength >=1)
        {
            int gop = trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength;
            onvif_set_video_gop(profIndex, gop);
        }

        int h264_profile = trt__SetVideoEncoderConfiguration->Configuration->H264->H264Profile;
        printf("================= h264_profile profile value==================%d\n", h264_profile);
        
        switch(h264_profile)
        {
            case tt__H264Profile__Baseline:
                 onvif_set_h264_profile(profIndex, ONVIF_H264_PROFILE_BASELINE);
                break;
            case tt__H264Profile__Main:
                 onvif_set_h264_profile(profIndex, ONVIF_H264_PROFILE_MAIN);
                break;
            case tt__H264Profile__High:
                 onvif_set_h264_profile(profIndex, ONVIF_H264_PROFILE_HIGH);
                break;
             default:
                 onvif_set_h264_profile(profIndex, ONVIF_H264_PROFILE_MAIN);
                 break;
        }
    }

    if (profIndex == 2)
    {
        if(s32Ret==NVT_SUCCESS)
        {
            nvt_adapter_model_start();
        }
    }
    //restart OSD with updated resolution
    nvt_adapter_osd_restart();
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(struct soap *soap,
                                                               struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations,
                                                               struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
    printf("--------------------------__trt__GetVideoEncoderConfigurations---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int profIndex = 0;
    char *value = malloc(20);
    const char *profile_value;
    
    trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = 3;

    /* TODO: How to deal with H265 codec */
    trt__GetVideoEncoderConfigurationsResponse->Configurations =
        (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoderConfiguration));
    MEMSET(trt__GetVideoEncoderConfigurationsResponse->Configurations, 0x00,
           trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoderConfiguration));

    for (profIndex = 0; profIndex < 3; profIndex++)
    {
         trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
            MEMSET(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].token, 0x00, sizeof(char) * TOKEN_LENGTH);

        ONVIF_TOKEN_GEN(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, profIndex + 1);

        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, profIndex + 1);


        if(testProfilecreated == false || testProfileAddVideoEncoderConfig==false)
            trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].UseCount = 1;
        else  
            trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].UseCount = 2;

        

         trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Encoding = tt__VideoEncoding__H264;
          trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Resolution =
                    (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    
        nvt_adapter_video_get_resolution(profIndex, value);
        sscanf(value, "%d*%d", &trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Resolution->Width,
        &trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Resolution->Height);

        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Quality =  10.0;
            
  
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].SessionTimeout = timeout;
            
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl =
            (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
        MEMSET(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl, 0x00,
                sizeof(struct tt__VideoRateControl));
  
        nvt_adapter_video_get_frame_rate(profIndex, &trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl->FrameRateLimit);
       // free(value);
            
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl->EncodingInterval = 1;

        nvt_adapter_video_get_max_rate(profIndex,& trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl->BitrateLimit);
  
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].MPEG4 = NULL;
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].H264 = 
                            (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        // Determines the interval in which the I-Frames will be coded 
        nvt_adapter_video_get_gop(profIndex,&trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->GovLength);

      // trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->H264Profile = tt__H264Profile__Main;
        int h264_profile_val = 0;
        nvt_adapter_video_get_h264_profile(profIndex, &h264_profile_val);
        printf("H264 PROFILE value %d\n", h264_profile_val);
        if(h264_profile_val == 0)
        {    
             trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->H264Profile = tt__H264Profile__Baseline;
        }
        else if(h264_profile_val == 1)
        {
            trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->H264Profile = tt__H264Profile__High;
        }
        else 
        {
             trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->H264Profile = tt__H264Profile__Main;
        }

        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast =
                (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address =
                                            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address->Type = 0;//tt__IPType__IPv4; //TODO: This param should be dynamic


        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address->IPv4Address =
            (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
        MEMSET(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address->IPv4Address, 0x00,
                sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
        strcpy(trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address->IPv4Address,"0.0.0.0");

             
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].__size = 0;
        trt__GetVideoEncoderConfigurationsResponse->Configurations[profIndex].__any = NULL;
    }
    free(value);
    
     printf("--------------------------return ok---------------------------\n"); 
    return SOAP_OK;
}

// /** Web service operation '__trt__GetServiceCapabilities' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities(struct soap *soap,
                                                        struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities,
                                                        struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
    
    printf("--------------------------__trt__GetServiceCapabilities---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    trt__GetServiceCapabilitiesResponse->Capabilities = (struct trt__Capabilities *)soap_malloc(soap, sizeof(struct trt__Capabilities));
    MEMSET(trt__GetServiceCapabilitiesResponse->Capabilities, 0x00, sizeof(struct trt__Capabilities));

       /** Required element 'trt:ProfileCapabilities' of XML schema type 'trt:ProfileCapabilities' */
    trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities =
        (struct trt__ProfileCapabilities *)soap_malloc(soap, sizeof(struct trt__ProfileCapabilities));
    MEMSET(trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities, 0x00, sizeof(struct trt__ProfileCapabilities));
    /** Optional attribute 'MaximumNumberOfProfiles' of XML schema type 'xsd:int' */
    trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles = (int *)soap_malloc(soap, sizeof(int));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles) = 3 ; // @Todo , change maximum number of profile with some parameter.
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->__size = 0;
    trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->__any = NULL;
 //   dom_att(&trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
    printf("--------------------------__trt__GetServiceCapabilities---------333333333333333333------------------\n");
    /** Required element 'trt:StreamingCapabilities' of XML schema type 'trt:StreamingCapabilities' */
    trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities =
        (struct trt__StreamingCapabilities *)soap_malloc(soap, sizeof(struct trt__StreamingCapabilities));
    MEMSET(trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities, 0x00, sizeof(struct trt__StreamingCapabilities));


    /** Optional attribute 'RTPMulticast' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTPMulticast = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTPMulticast) = xsd__boolean__false_;
        
    /** Optional attribute 'RTP_TCP' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORETCP) = xsd__boolean__true_;
       
    /** Optional attribute 'RTP_RTSP_TCP' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP) = xsd__boolean__true_;
    /** Optional attribute 'NonAggregateControl' of XML schema type 'xsd:boolean' */
     trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NonAggregateControl = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NonAggregateControl) = xsd__boolean__true_;
    /** Optional attribute 'NoRTSPStreaming' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NoRTSPStreaming = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NoRTSPStreaming) = xsd__boolean__false_;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->__size = 0;
    trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->__any = NULL;
  //  dom_att(&trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
printf("--------------------------__trt__GetServiceCapabilities----------555555555555555555555555555-----------------\n");
    /** Optional attribute 'SnapshotUri' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->SnapshotUri = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->SnapshotUri) = xsd__boolean__true_;
    /** Optional attribute 'Rotation' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->Rotation = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->Rotation) = xsd__boolean__false_;
      /** Optional attribute 'VideoSourceMode' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->VideoSourceMode = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->VideoSourceMode) = xsd__boolean__false_;
    /** Optional attribute 'OSD' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->OSD = NULL;
    /** Optional attribute 'TemporaryOSDText' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->TemporaryOSDText = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(trt__GetServiceCapabilitiesResponse->Capabilities->TemporaryOSDText) = xsd__boolean__false_;
     /** Optional attribute 'EXICompression' of XML schema type 'xsd:boolean' */
    trt__GetServiceCapabilitiesResponse->Capabilities->EXICompression = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));     
   *(trt__GetServiceCapabilitiesResponse->Capabilities->EXICompression) = xsd__boolean__false_;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    trt__GetServiceCapabilitiesResponse->Capabilities->__size = 0;
    trt__GetServiceCapabilitiesResponse->Capabilities->__any = NULL;
    /** XML DOM attribute list */
//    dom_att(&trt__GetServiceCapabilitiesResponse->Capabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    return SOAP_OK;
}
/** Web service operation '__trt__GetVideoSourceConfigurationOptions' implementation, should return SOAP_OK or error code */
// SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap *soap,
//                                                                     struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions,
//                                                                     struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
// {
//     typedef char *	CHARPTR;
//     printf("--------------------------__trt__GetVideoSourceConfigurationOptions---------------------------\n");
   
//    int profIndex = -1;

//     if (NULL != trt__GetVideoSourceConfigurationOptions)
//     {
  
//         if (NULL != trt__GetVideoSourceConfigurationOptions->ProfileToken)
//         {

//             printf("Given profile token is = %s\n", trt__GetVideoSourceConfigurationOptions->ProfileToken);
//             if(0 == strncmp("Profile_1", trt__GetVideoSourceConfigurationOptions->ProfileToken, TOKEN_LENGTH))
//                 profIndex =0;
//             else if (0 == strncmp("Profile_2", trt__GetVideoSourceConfigurationOptions->ProfileToken, TOKEN_LENGTH))
//                 profIndex =1;
//             else if (0 == strncmp("Profile_3", trt__GetVideoSourceConfigurationOptions->ProfileToken, TOKEN_LENGTH))
//                 profIndex =2;

            
//             if (-1 == profIndex)
//             {
//               //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
//                 printf("Given profile token not found is = %s\n", trt__GetVideoSourceConfigurationOptions->ProfileToken);
//                 return SOAP_FAULT;
//             }
//             else
//             {
//                 printf("Given profile token found Index = %d\n", profIndex);
//             }
//              if (NULL != trt__GetVideoSourceConfigurationOptions->ConfigurationToken) /* Profile token and Video source token both provided */
//             {
//                  printf("Given configuration token is = %s\n", trt__GetVideoSourceConfigurationOptions->ConfigurationToken);
//             }
//              else /* Only profile token provided */
//             {
//                 printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------only profiletoken given-----------------\n");
//             }
            
//         }
//         else 
//         {
//             if(NULL != trt__GetVideoSourceConfigurationOptions->ConfigurationToken)
//             {
//                 printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------no profile token provided-----------------\n");
//                 printf("Given configuration token is = %s\n", trt__GetVideoSourceConfigurationOptions->ConfigurationToken);
//                 if(0 == strncmp(vsrc_confToken, trt__GetVideoSourceConfigurationOptions->ConfigurationToken, TOKEN_LENGTH))
//                 {
//                     printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------only config token provided-----------------\n");
//                 //  profIndex =0;
//                 }
//                 else { //no token provided
                   
//                    // printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------no token provided-----------------\n");
//                     return SOAP_FAULT;
//                 }
//             }
//             else { //no token provided
//                     printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------no token provided-----------------\n");
//                     return SOAP_FAULT;
//             }
//         }
//     }
//     else /* No any token provided */
//     {
//         printf("--------------------------_Invalid input argument trt__GetVideoSourceConfigurationOptions---------------------------\n");
//         return SOAP_FAULT;
//     }
// ///////////////////@TODO Check configuration token also
   
//     trt__GetVideoSourceConfigurationOptionsResponse->Options =
//         (struct tt__VideoSourceConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfigurationOptions));

//     /** Required element 'tt:BoundsRange' of XML schema type 'tt:IntRectangleRange' */
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange = (struct tt__IntRectangleRange *)soap_malloc(soap, sizeof(struct tt__IntRectangleRange));
//     MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange, 0x00, sizeof(struct tt__IntRectangleRange));

//     /** Required element 'tt:XRange' of XML schema type 'tt:IntRange' */
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Min =vsrc_minX; //0
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Max =vsrc_maxX; //100

//     /** Required element 'tt:YRange' of XML schema type 'tt:IntRange' */
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Min = vsrc_minY; //0
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Max = vsrc_maxY; //100

//     /** Required element 'tt:WidthRange' of XML schema type 'tt:IntRange' */
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Min = vsrc_minboundWidth; //0
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Max = vsrc_maxboundWidth; //2048

//     /** Required element 'tt:HeightRange' of XML schema type 'tt:IntRange' */
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Min = vsrc_minboundHeight; //0
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Max = vsrc_maxboundHeight;//1536


//     trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable = 1;
//     trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable =
//        (CHARPTR *)soap_malloc(soap, sizeof(CHARPTR) * trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable);
//     MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable, 0x00,
//            sizeof(CHARPTR) * trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable);


//     trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0] = (CHARPTR)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
//     MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0], 0x00, sizeof(char) * TOKEN_LENGTH);
//     //strcpy(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0] ,vsrc_srcToken);
//     snprintf(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0], TOKEN_LENGTH, "%s",
//              vsrc_srcToken);

//     trt__GetVideoSourceConfigurationOptionsResponse->Options->MaximumNumberOfProfiles = (int *)soap_malloc(soap, sizeof(int));
//     *(trt__GetVideoSourceConfigurationOptionsResponse->Options->MaximumNumberOfProfiles) = vsrc_maxNumOfProfiles;//2;

//     trt__GetVideoSourceConfigurationOptionsResponse->Options->Extension = NULL;

//     // trt__GetVideoSourceConfigurationOptionsResponse->Options->Extension =
//     //     (struct tt__VideoSourceConfigurationOptionsExtension *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfigurationOptionsExtension));
//     // MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->Extension, 0x00, sizeof(struct tt__VideoSourceConfigurationOptionsExtension));
//     // /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
//     // trt__GetVideoSourceConfigurationOptionsResponse->Options->Extension->__size = 0;
//     // trt__GetVideoSourceConfigurationOptionsResponse->Options->Extension->__any = NULL;
//     // trt__GetVideoSourceConfigurationOptionsResponse->Options->Extension->Rotate = NULL;
//     // trt__GetVideoSourceConfigurationOptionsResponse->Options->Extension->Extension = NULL;
      
   
//     return SOAP_OK;
// }
/** Web service operation '__trt__GetVideoSourceConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap *soap,
                                                                    struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions,
                                                                    struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
    typedef char *	CHARPTR;
    printf("--------------------------__trt__GetVideoSourceConfigurationOptions---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
   int profIndex = -1;

    if (NULL != trt__GetVideoSourceConfigurationOptions)
    {
  
        if (NULL != trt__GetVideoSourceConfigurationOptions->ProfileToken)
        {

            printf("Given profile token is = %s\n", trt__GetVideoSourceConfigurationOptions->ProfileToken);
            profIndex = onvif_profile_token_to_channel(trt__GetVideoSourceConfigurationOptions->ProfileToken);

            
            if (-1 == profIndex)
            {
              //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
                printf("Given profile token not found is = %s\n", trt__GetVideoSourceConfigurationOptions->ProfileToken);
                return SOAP_FAULT;
            }
            else
            {
                printf("Given profile token found Index = %d\n", profIndex);
            }
             if (NULL != trt__GetVideoSourceConfigurationOptions->ConfigurationToken) /* Profile token and Video source token both provided */
            {
                 printf("Given configuration token is = %s\n", trt__GetVideoSourceConfigurationOptions->ConfigurationToken);
            }
             else /* Only profile token provided */
            {
                printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------only profiletoken given-----------------\n");
            }
            
        }
        else 
        {
            if(NULL != trt__GetVideoSourceConfigurationOptions->ConfigurationToken)
            {
                printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------no profile token provided-----------------\n");
                printf("Given configuration token is = %s\n", trt__GetVideoSourceConfigurationOptions->ConfigurationToken);
                if(0 == strncmp(vsrc_srcToken, trt__GetVideoSourceConfigurationOptions->ConfigurationToken, TOKEN_LENGTH))
                {
                    printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------only config token provided-----------------\n");
                //  profIndex =0;
                }
                else { //no token provided
                   
                   // printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------no token provided-----------------\n");
                    return SOAP_FAULT;
                }
            }
            else { //no token provided
                    printf("--------------------------__trt__GetVideoSourceConfigurationOptions----------no token provided-----------------\n");
                    return SOAP_FAULT;
            }
        }
    }
    else /* No any token provided */
    {
        printf("--------------------------_Invalid input argument trt__GetVideoSourceConfigurationOptions---------------------------\n");
        return SOAP_FAULT;
    }
///////////////////@TODO Check configuration token also
   
    trt__GetVideoSourceConfigurationOptionsResponse->Options =
        (struct tt__VideoSourceConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfigurationOptions));
    MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options, 0x00, sizeof(struct tt__VideoSourceConfigurationOptions));

    // Required element 'tt:BoundsRange' of XML schema type 'tt:IntRectangleRange' 
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange = (struct tt__IntRectangleRange *)soap_malloc(soap, sizeof(struct tt__IntRectangleRange));
    MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange, 0x00, sizeof(struct tt__IntRectangleRange));

    // Required element 'tt:XRange' of XML schema type 'tt:IntRange' 
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Min =vsrc_minX; //0
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Max =vsrc_maxX; //100

    // Required element 'tt:YRange' of XML schema type 'tt:IntRange' 
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Min = vsrc_minY; //0
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Max = vsrc_maxY; //100

    // Required element 'tt:WidthRange' of XML schema type 'tt:IntRange' 
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Min = vsrc_minboundWidth; //0
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Max = vsrc_maxboundWidth; //2048

    // Required element 'tt:HeightRange' of XML schema type 'tt:IntRange' 
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Min = vsrc_minboundHeight; //0
    trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Max = vsrc_maxboundHeight;//1536


    trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable = 1;
    
    trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable =
       (CHARPTR *)soap_malloc(soap, sizeof(CHARPTR) * trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable);


    MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable, 0x00,
           sizeof(CHARPTR) * trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable);


    trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0] = (CHARPTR)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);

    MEMSET(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0], 0x00, sizeof(char) * TOKEN_LENGTH);
    //strcpy(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0] ,vsrc_srcToken);
    snprintf(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0], TOKEN_LENGTH, "%s",
             vsrc_srcToken);

    trt__GetVideoSourceConfigurationOptionsResponse->Options->MaximumNumberOfProfiles = (int *)soap_malloc(soap, sizeof(int));
    *(trt__GetVideoSourceConfigurationOptionsResponse->Options->MaximumNumberOfProfiles) = vsrc_maxNumOfProfiles;//2;

    trt__GetVideoSourceConfigurationOptionsResponse->Options->Extension = NULL;

  
    return SOAP_OK;
}

/** Web service operation '__trt__GetSnapshotUri' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap *soap,
                                                struct _trt__GetSnapshotUri *trt__GetSnapshotUri,
                                                struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
    printf("Called __trt__GetSnapshotUri.....\n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == trt__GetSnapshotUri || NULL == trt__GetSnapshotUri->ProfileToken) {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument",
                    "Invalid request — ProfileToken is required");
        return SOAP_FAULT;
    }

    /* Validate profile token */
    int profIndex = onvif_profile_token_to_channel(trt__GetSnapshotUri->ProfileToken);
    if (profIndex < 0) {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile",
                    "The requested profile token does not exist.");
        return SOAP_FAULT;
    }

    /* Build snapshot URI: http://<ip>/onvif/snapshot?token=<ProfileToken> */
    trt__GetSnapshotUriResponse->MediaUri = (struct tt__MediaUri *)
        soap_malloc(soap, sizeof(struct tt__MediaUri));
    memset(trt__GetSnapshotUriResponse->MediaUri, 0, sizeof(struct tt__MediaUri));

    trt__GetSnapshotUriResponse->MediaUri->Uri = (char *)soap_malloc(soap, sizeof(char) * 256);
    snprintf(trt__GetSnapshotUriResponse->MediaUri->Uri, 256,
             "http://%s/onvif/snapshot?token=%s",
             ip_address ? ip_address : "0.0.0.0",
             trt__GetSnapshotUri->ProfileToken);

    trt__GetSnapshotUriResponse->MediaUri->InvalidAfterConnect = xsd__boolean__false_;
    trt__GetSnapshotUriResponse->MediaUri->InvalidAfterReboot  = xsd__boolean__false_;
    trt__GetSnapshotUriResponse->MediaUri->Timeout = soap_strdup(soap, "PT5S");

    printf("GetSnapshotUri: %s\n", trt__GetSnapshotUriResponse->MediaUri->Uri);
    return SOAP_OK;
}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(struct soap *soap,
                                                              struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations,
                                                              struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
    printf("Called __trt__GetVideoSourceConfigurations.....\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

     trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations = 1;
    trt__GetVideoSourceConfigurationsResponse->Configurations =
        (struct tt__VideoSourceConfiguration *)soap_malloc(soap, (trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoSourceConfiguration)));
    MEMSET(trt__GetVideoSourceConfigurationsResponse->Configurations, 0x00, trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoSourceConfiguration));


    trt__GetVideoSourceConfigurationsResponse->Configurations[0].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoSourceConfigurationsResponse->Configurations[0].token, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetVideoSourceConfigurationsResponse->Configurations[0].token, vsrc_confToken);

     trt__GetVideoSourceConfigurationsResponse->Configurations[0].Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoSourceConfigurationsResponse->Configurations[0].Name, 0x00, sizeof(char) * TOKEN_LENGTH);
         strcpy(trt__GetVideoSourceConfigurationsResponse->Configurations[0].Name,vsrc_name);

        trt__GetVideoSourceConfigurationsResponse->Configurations[0].UseCount =vsrc_useCount;

        trt__GetVideoSourceConfigurationsResponse->Configurations[0].ViewMode = NULL;

        trt__GetVideoSourceConfigurationsResponse->Configurations[0].SourceToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetVideoSourceConfigurationsResponse->Configurations[0].SourceToken, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(trt__GetVideoSourceConfigurationsResponse->Configurations[0].SourceToken, vsrc_srcToken);
 
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds->x = vsrc_boundX;
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds->y = vsrc_boundY;

        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds->width = vsrc_boundWidth;
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds->height = vsrc_boundHeight;
 

        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Extension = NULL;
          

        //trt__GetVideoSourceConfigurationsResponse->Configurations[0].Extension->Extension = NULL;

        trt__GetVideoSourceConfigurationsResponse->Configurations[0].__size = 0;
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].__any = NULL;

     return SOAP_OK;
 }
// SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap,
//                                                                             struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances,
//                                                                             struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
// {
//      printf("Called __trt__GetGuaranteedNumberOfVideoEncoderInstances.....\n");

//      return SOAP_OK;
// }
// SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceModes(struct soap *soap,
//                                                      struct _trt__GetVideoSourceModes *trt__GetVideoSourceModes,
//                                                      struct _trt__GetVideoSourceModesResponse *trt__GetVideoSourceModesResponse)
// {
//     printf("Called __trt__GetVideoSourceModes.....\n");

//      return SOAP_OK;
// }
// SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceMode(struct soap *soap,
//                                                     struct _trt__SetVideoSourceMode *trt__SetVideoSourceMode,
//                                                     struct _trt__SetVideoSourceModeResponse *trt__SetVideoSourceModeResponse)
// {
//     printf("Called __trt__SetVideoSourceMode.....\n");

//      return SOAP_OK;
// }
/** Web service operation '__trt__GetAudioEncoderConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
    printf("Called __trt__GetAudioEncoderConfigurationOptions.....\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

     return SOAP_OK;
}


/** Web service operation '__trt__GetOSDOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDOptions(struct soap *soap, struct _trt__GetOSDOptions *trt__GetOSDOptions, struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse)
{
    printf("Called __trt__GetOSDOptions....\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int index = 0;
    int retCode = SOAP_FAULT;
    if (NULL == trt__GetOSDOptions)
    {
        LOG_ERROR("Invalid argument. trt__GetOSDOptions is NULL....\n");
        return SOAP_FAULT;
    }

    if (NULL == trt__GetOSDOptions->ConfigurationToken || 0 == strlen(trt__GetOSDOptions->ConfigurationToken))
    {
        LOG_ERROR("Invalid argument. trt__GetOSDOptions->ConfigurationToken is NULL....\n");
        return SOAP_FAULT;
    }

    if(0 != strcmp(vsrc_srcToken, trt__GetOSDOptions->ConfigurationToken))
    {
        LOG_ERROR("Invalid argument. trt__GetOSDOptions->ConfigurationToken is invalid....\n");
        return SOAP_FAULT;
    }

    //OSDoptions start

    trt__GetOSDOptionsResponse->OSDOptions = (struct tt__OSDConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__OSDConfigurationOptions));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions, 0x00, sizeof(struct tt__OSDConfigurationOptions));

    //MaximumNumberOfOSDs
    trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs = (struct tt__MaximumNumberOfOSDs *)soap_malloc(soap, sizeof(struct tt__MaximumNumberOfOSDs));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs, 0x00, sizeof(struct tt__MaximumNumberOfOSDs));

    //total number of OSDs
    trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Total = 3;

    //Plain Text
    trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->PlainText = (int *)soap_malloc(soap, sizeof(int));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->PlainText, 0x00, sizeof(int));
    *(trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->PlainText) = 2;

    //DateAndTime
    trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->DateAndTime = (int *)soap_malloc(soap, sizeof(int));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->DateAndTime, 0x00, sizeof(int));
    *(trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->DateAndTime) = 1;

    trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Date = NULL;
    trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Time = NULL;
    trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Image = NULL;

    trt__GetOSDOptionsResponse->OSDOptions->Extension = ( struct tt__OSDConfigurationOptionsExtension *)soap_malloc(soap, sizeof(struct tt__OSDConfigurationOptionsExtension));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->Extension, 0x00, sizeof(struct tt__OSDConfigurationOptionsExtension));

    trt__GetOSDOptionsResponse->OSDOptions->Extension->__size = 1;

    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any = (struct soap_dom_element *)soap_malloc(soap, trt__GetOSDOptionsResponse->OSDOptions->Extension->__size*sizeof(struct soap_dom_element));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->Extension->__any, 0x00, trt__GetOSDOptionsResponse->OSDOptions->Extension->__size*sizeof(struct soap_dom_element));

    

    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->name = soap_strdup(soap, "tt:ChannelName");
    
    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->text = soap_strdup(soap, "true");
    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->type = 0;
    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->node = NULL;
    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->tail = NULL;
    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->soap = soap;
    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->code = NULL;
    trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->lead = NULL;
    // trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->atts = (struct soap_dom_attribute *)soap_malloc(soap, sizeof(struct soap_dom_attribute));
    // MEMSET(trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->atts, 0x00, sizeof(struct soap_dom_attribute));
    // trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->atts->name = (char *)soap_malloc(soap, sizeof(char) * 20);
    // trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->atts->name = "";
    dom_att(trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->atts, 0, NULL,NULL, NULL, NULL,soap);
    // trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->nstr = (char *)soap_malloc(soap, sizeof(char) * 20);
    // MEMSET(trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->nstr, 0x00, sizeof(char) * 20);
    // trt__GetOSDOptionsResponse->OSDOptions->Extension->__any->nstr = "";

    //SizeType
    trt__GetOSDOptionsResponse->OSDOptions->__sizeType = 1;

    trt__GetOSDOptionsResponse->OSDOptions->Type = (enum tt__OSDType *)soap_malloc(soap, sizeof(enum tt__OSDType));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->Type, 0x00, sizeof(enum tt__OSDType));
    *(trt__GetOSDOptionsResponse->OSDOptions->Type) = tt__OSDType__Text;

    //SizePositionOption
    trt__GetOSDOptionsResponse->OSDOptions->__sizePositionOption = 6;
    trt__GetOSDOptionsResponse->OSDOptions->PositionOption = (char **)soap_malloc(soap, (sizeof(char *) * trt__GetOSDOptionsResponse->OSDOptions->__sizePositionOption));

    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->PositionOption, 0x00,
           (sizeof(char *) * trt__GetOSDOptionsResponse->OSDOptions->__sizePositionOption));
    for (index = 0; index < trt__GetOSDOptionsResponse->OSDOptions->__sizePositionOption; index++)
    {
        trt__GetOSDOptionsResponse->OSDOptions->PositionOption[index] = (char *)soap_malloc(soap, sizeof(char) * 20);
        MEMSET(trt__GetOSDOptionsResponse->OSDOptions->PositionOption[index], 0x00, sizeof(char) * 20);
        snprintf(trt__GetOSDOptionsResponse->OSDOptions->PositionOption[index], SMALL_BUFFER_LENGTH, "%s", osdPositions[index]);
    }

    //TextOption
    trt__GetOSDOptionsResponse->OSDOptions->TextOption = (struct tt__OSDTextOptions *)soap_malloc(soap, sizeof(struct tt__OSDTextOptions));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->TextOption, 0x00, sizeof(struct tt__OSDTextOptions));

    //sizetype
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeType = 2;
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->Type = (char **)soap_malloc(soap,
                                                                                    (sizeof(char *) * trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeType));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->TextOption->Type, 0x00, (sizeof(char *) * trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeType));
    //char *osdTextTypes[] = {"Plain", "DateAndTime"};

    //type
    index = 0;
    for (index = 0; index < trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeType; index++)
    {
        trt__GetOSDOptionsResponse->OSDOptions->TextOption->Type[index] = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(trt__GetOSDOptionsResponse->OSDOptions->TextOption->Type[index], 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(trt__GetOSDOptionsResponse->OSDOptions->TextOption->Type[index], SMALL_BUFFER_LENGTH, "%s", osdTextTypes[index]);
    }

    //SizeDateFormat
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeDateFormat = 5;
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat = (char **)soap_malloc(soap,
                                                                                      (sizeof(char *) * trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeDateFormat));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat, 0x00, (sizeof(char *) * trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeDateFormat));

    //dateFormat
    //char *osdDateFormat[] = {"YYYY-MM-DD", "MM-DD-YYYY", "DD-MM-YYYY", "YYYY/MM/DD", "MM/DD/YYYY", "DD-MM-YYYY"};
    index = 0;
    for (index = 0; index < trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeDateFormat; index++)
    {
        trt__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat[index] = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(trt__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat[index], 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(trt__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat[index], SMALL_BUFFER_LENGTH, "%s", osdDateFormat[index]);
    }

    //SizeTimeFormat
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeTimeFormat = 2;
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat = (char **)soap_malloc(soap,
                                                                                  (sizeof(char *) * trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeTimeFormat));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat, 0x00, (sizeof(char *) * trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeTimeFormat));  

    //timeFormat
    index = 0;
    for (index = 0; index < trt__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeTimeFormat; index++)
    {
        trt__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat[index] = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(trt__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat[index], 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(trt__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat[index], SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[index]);
    }

    //FontSizeRange
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->FontSizeRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(trt__GetOSDOptionsResponse->OSDOptions->TextOption->FontSizeRange, 0x00, sizeof(struct tt__IntRange));
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->FontSizeRange->Min = 12;
    trt__GetOSDOptionsResponse->OSDOptions->TextOption->FontSizeRange->Max = 48;


    return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSD(struct soap *soap, struct _trt__GetOSD *trt__GetOSD, struct _trt__GetOSDResponse *trt__GetOSDResponse){
    printf("Called __trt__GetOSD....\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int retcode = SOAP_FAULT;
    int index = 0;
    int noOfOSDConfig = 0;
    char entry[128];
    bool isConfigToken = false;
    bool isOSDTokenFound = false;
    int responseSize = 0;
    int OSDTokenID = 0;
    const char *osdToken = NULL;
    int R;
    int G;
    int B;
    float y;
    float cb;
    float cr;

    if (NULL == trt__GetOSD)
    {
        LOG_ERROR("Invalid argument. trt__GetOSD is NULL....\n");
        return SOAP_FAULT;
    }

    for(index = 0; index < 3; index++)
    {
        snprintf(entry, 127, "osd.%d:enabled", index);
        if(nvt_adapter_param_get_int(entry, 0))
        {
            noOfOSDConfig++;
        }
    }
    printf("no of osd enabled are %d\n", noOfOSDConfig);

    if(0 == noOfOSDConfig)
    {
        LOG_ERROR("No OSD configuration found....\n");
        trt__GetOSDResponse->__size = 0;
        trt__GetOSDResponse->OSD = NULL;
        return SOAP_FAULT;
    }

    if(NULL != trt__GetOSD->OSDToken){
        for(index = 0; index < 3; index++){
            snprintf(entry, 127, "osd.%d:enabled", index);
            if(nvt_adapter_param_get_int(entry, 0)){
                snprintf(entry, 127, "osd.%d:token", index);
                osdToken = nvt_adapter_param_get_string(entry, NULL);
                if(NULL != osdToken){
                    if(0 == strcmp(osdToken, trt__GetOSD->OSDToken)){
                        isOSDTokenFound = true;
                        OSDTokenID = index;
                        LOG_INFO("OSD token found at index %d and it is %s\n", index, osdToken);
                        break;
                }
                
                }
                else{
                    LOG_ERROR("OSD token is NULL in config file....\n");
                    return SOAP_FAULT;
                }
            }
            else{
                LOG_ERROR("OSD is not enabled....\n");
                return SOAP_FAULT;
            }
        }
    }

    if(!isOSDTokenFound){
        LOG_ERROR("OSD token not found....\n");
        return SOAP_FAULT;
    }

    LOG_INFO("OSD token is %s\n", trt__GetOSD->OSDToken);
    trt__GetOSDResponse->__size = 1;
    trt__GetOSDResponse->OSD = (struct tt__OSDConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDConfiguration));
    MEMSET(trt__GetOSDResponse->OSD, 0x00, sizeof(struct tt__OSDConfiguration));
    trt__GetOSDResponse->OSD->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(trt__GetOSDResponse->OSD->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    snprintf(trt__GetOSDResponse->OSD->token, TOKEN_LENGTH, "%s", trt__GetOSD->OSDToken);
    trt__GetOSDResponse->OSD->VideoSourceConfigurationToken = (struct tt__OSDReference *)soap_malloc(soap, sizeof(struct tt__OSDReference));
    MEMSET(trt__GetOSDResponse->OSD->VideoSourceConfigurationToken, 0x00, sizeof(struct tt__OSDReference));
    trt__GetOSDResponse->OSD->VideoSourceConfigurationToken->__item = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
    MEMSET(trt__GetOSDResponse->OSD->VideoSourceConfigurationToken->__item, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
    snprintf(trt__GetOSDResponse->OSD->VideoSourceConfigurationToken->__item, TOKEN_LENGTH, "%s", vsrc_srcToken);
    trt__GetOSDResponse->OSD->Type = tt__OSDType__Text;
    trt__GetOSDResponse->OSD->Position = (struct tt__OSDPosConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDPosConfiguration));
    MEMSET(trt__GetOSDResponse->OSD->Position, 0x00, sizeof(struct tt__OSDPosConfiguration));
    trt__GetOSDResponse->OSD->Position->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
    MEMSET(trt__GetOSDResponse->OSD->Position->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
    snprintf(trt__GetOSDResponse->OSD->Position->Type, SMALL_BUFFER_LENGTH, "%s" ,"Custom");
    trt__GetOSDResponse->OSD->Position->Pos = (struct tt__Vector *)soap_malloc(soap, sizeof(struct tt__Vector));
    MEMSET(trt__GetOSDResponse->OSD->Position->Pos, 0x00, sizeof(struct tt__Vector));
    trt__GetOSDResponse->OSD->Position->Pos->x = (float *)soap_malloc(soap, sizeof(float));
    trt__GetOSDResponse->OSD->Position->Pos->y = (float *)soap_malloc(soap, sizeof(float));
    int pos_x = 0;
    int pos_y = 0;
    //snprintf(entry, 127, "osd.%d:position_x", OSDTokenID);
    //pos_x = nvt_adapter_param_get_int(entry, 0);
    // FIX: Was calling get_position_y for both x and y
    nvt_adapter_osd_get_position_x(OSDTokenID, &pos_x);
    //snprintf(entry, 127, "osd.%d:position_y", OSDTokenID);
    //pos_y = nvt_adapter_param_get_int(entry, 0);
    nvt_adapter_osd_get_position_y(OSDTokenID, &pos_y);
    // *(trt__GetOSDResponse->OSD->Position->Pos->x) = pos_x;
    // *(trt__GetOSDResponse->OSD->Position->Pos->y) = pos_y;
    inverseRescaleCoordinates(pos_x, pos_y, trt__GetOSDResponse->OSD->Position->Pos->x, trt__GetOSDResponse->OSD->Position->Pos->y);
    LOG_INFO("Position x = %f, Position y = %f\n", *(trt__GetOSDResponse->OSD->Position->Pos->x), *(trt__GetOSDResponse->OSD->Position->Pos->y));
    trt__GetOSDResponse->OSD->TextString = (struct tt__OSDTextConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDTextConfiguration));
    MEMSET(trt__GetOSDResponse->OSD->TextString, 0x00, sizeof(struct tt__OSDTextConfiguration));    
    trt__GetOSDResponse->OSD->TextString->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
    MEMSET(trt__GetOSDResponse->OSD->TextString->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
    if(0 == strcmp(trt__GetOSDResponse->OSD->token, "time_01")){
        snprintf(trt__GetOSDResponse->OSD->TextString->Type, SMALL_BUFFER_LENGTH, "%s", "DateAndTime");
        trt__GetOSDResponse->OSD->TextString->DateFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(trt__GetOSDResponse->OSD->TextString->DateFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(entry,127,"osd.%d:date_style", OSDTokenID);
        const char *dateFormat = nvt_adapter_param_get_string(entry, NULL);
        omitFirstFourChars(dateFormat, dateFormatWithoutFirstFourChars);
        snprintf(trt__GetOSDResponse->OSD->TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", dateFormatWithoutFirstFourChars);
        LOG_INFO("Date format is %s\n", trt__GetOSDResponse->OSD->TextString->DateFormat);
        trt__GetOSDResponse->OSD->TextString->TimeFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(trt__GetOSDResponse->OSD->TextString->TimeFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(entry,127,"osd.%d:time_style", OSDTokenID);
        trt__GetOSDResponse->OSD->TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));
        MEMSET(trt__GetOSDResponse->OSD->TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));
        trt__GetOSDResponse->OSD->TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));
        MEMSET(trt__GetOSDResponse->OSD->TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));
        // FIX: Use OSD-specific font color instead of hardcoded "osd.2"
        snprintf(entry, 127, "osd.%d:font_color", OSDTokenID);
        const char *fontColor = nvt_adapter_param_get_string(entry, "FFFFFF");
        hexToYCbCr(fontColor, &y, &cb, &cr);
        trt__GetOSDResponse->OSD->TextString->FontColor->Color->X = y;
        trt__GetOSDResponse->OSD->TextString->FontColor->Color->Y = cb;
        trt__GetOSDResponse->OSD->TextString->FontColor->Color->Z = cr; 
        const char *timeFormat = nvt_adapter_param_get_string(entry, NULL);
        if(0 == strcmp(timeFormat, "12hour")){
            snprintf(trt__GetOSDResponse->OSD->TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[1]);
        }
        else{
            snprintf(trt__GetOSDResponse->OSD->TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[0]);
        }
        trt__GetOSDResponse->OSD->TextString->PlainText = NULL;
    }
    else{
        snprintf(trt__GetOSDResponse->OSD->TextString->Type, SMALL_BUFFER_LENGTH, "%s", "Plain");
        trt__GetOSDResponse->OSD->TextString->DateFormat = NULL;
        trt__GetOSDResponse->OSD->TextString->TimeFormat = NULL;
        trt__GetOSDResponse->OSD->TextString->PlainText = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(trt__GetOSDResponse->OSD->TextString->PlainText, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(entry,127,"osd.%d:display_text", OSDTokenID);
        trt__GetOSDResponse->OSD->TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));
        MEMSET(trt__GetOSDResponse->OSD->TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));
        trt__GetOSDResponse->OSD->TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));
        MEMSET(trt__GetOSDResponse->OSD->TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));
        snprintf(entry,127,"osd.%d:font_color", OSDTokenID);
        const char *fontColor = nvt_adapter_param_get_string(entry, NULL);
        hexToYCbCr(fontColor, &y, &cb, &cr);
        trt__GetOSDResponse->OSD->TextString->FontColor->Color->X = y;
        trt__GetOSDResponse->OSD->TextString->FontColor->Color->Y = cb;
        trt__GetOSDResponse->OSD->TextString->FontColor->Color->Z = cr; 
        const char *displayText = nvt_adapter_param_get_string(entry, NULL);
        snprintf(trt__GetOSDResponse->OSD->TextString->PlainText, SMALL_BUFFER_LENGTH, "%s", displayText);
    }
    trt__GetOSDResponse->OSD->TextString->FontSize = (int *)soap_malloc(soap, sizeof(int));
    MEMSET(trt__GetOSDResponse->OSD->TextString->FontSize, 0x00, sizeof(int));
    int fontSize = nvt_adapter_param_get_int("osd.common:font_size", -1);
    *(trt__GetOSDResponse->OSD->TextString->FontSize) = fontSize;
    // trt__GetOSDResponse->OSD->TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));
    // MEMSET(trt__GetOSDResponse->OSD->TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));
    // trt__GetOSDResponse->OSD->TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));
    // MEMSET(trt__GetOSDResponse->OSD->TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));
    // const char *fontColor = nvt_adapter_param_get_string("osd.common:font_color", NULL);
    // // LOG_INFO("Font color is %s\n", fontColor);
    // // unsigned int hexCode = (unsigned int)strtol(fontColor, NULL, 16);
    // // hexToRGB(hexCode, &R, &G, &B);
    // // LOG_INFO("R = %d, G = %d, B = %d\n", R, G, B);
    // // RGBtoYCbCr(R, G, B, &y, &cb, &cr);
    // // LOG_INFO("Y = %f, Cb = %f, Cr = %f\n", y, cb, cr);
    // hexToYCbCr(fontColor, &y, &cb, &cr);
    // trt__GetOSDResponse->OSD->TextString->FontColor->Color->X = y;
    // trt__GetOSDResponse->OSD->TextString->FontColor->Color->Y = cb;
    // trt__GetOSDResponse->OSD->TextString->FontColor->Color->Z = cr; 

    LOG_INFO("End of __trt__GetOSD....\n");
    return SOAP_OK;

}
/** Web service operation '__trt__GetOSDs' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDs(struct soap *soap, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse)
{
    printf("Called __trt__GetOSDs....\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int retcode = SOAP_FAULT;
    int index = 0;
    int noOfOSDConfig = 0;
    char entry[128];
    int responseSize = 0;
    const char *osdToken = NULL;
    int R;
    int G;
    int B;
    float y;
    float cb;
    float cr;

    if (NULL == trt__GetOSDs)
    {
        LOG_ERROR("Invalid argument. trt__GetOSDs is NULL....\n");
        return SOAP_FAULT;
    }
    int nvt_adapter_osd_get_enabled(int id, int *value) {
	char entry[128] = {'\0'};
	snprintf(entry, 127, "osd.%d:enabled", id);
	*value = nvt_adapter_param_get_int(entry, -1);
    int *enableIndices = (int *)malloc(sizeof(int));

	return 0;
    }
    for(index = 0; index < 3; index++)
    {
        int enabled = 0;
        nvt_adapter_osd_get_enabled(index, &enabled);
        if(enabled)
        {
            noOfOSDConfig++;

        }
    }
    printf("no of osd enabled are %d\n", noOfOSDConfig);

    if(0 == noOfOSDConfig)
    {
        LOG_ERROR("No OSD configuration found....\n");
        trt__GetOSDsResponse->__sizeOSDs = 0;
        trt__GetOSDsResponse->OSDs = NULL;
        return SOAP_OK;
    }
    if(NULL != trt__GetOSDs->ConfigurationToken)
    {
        if(0 == strcmp(vsrc_srcToken, trt__GetOSDs->ConfigurationToken))
        {
            printf("Matched configuration token\n");
            responseSize = noOfOSDConfig;
        }
        else{
            printf("Configuration token invalid.\n");
            return SOAP_FAULT;
        }
    }

    
    trt__GetOSDsResponse->__sizeOSDs = responseSize;
    trt__GetOSDsResponse->OSDs = (struct tt__OSDConfiguration *)soap_malloc(soap, responseSize * sizeof(struct tt__OSDConfiguration));
    MEMSET(trt__GetOSDsResponse->OSDs, 0x00, responseSize * sizeof(struct tt__OSDConfiguration));
    int i = 0;
    responseSize = 3;

    for(int j = 0; j < responseSize; j++){
        int enabled = 0;
        nvt_adapter_osd_get_enabled(j, &enabled);
        if(!enabled){
            continue;
        }

        trt__GetOSDsResponse->OSDs[i].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetOSDsResponse->OSDs[i].token, 0x00, sizeof(char) * TOKEN_LENGTH);
        snprintf(entry, 127, "osd.%d:token", j);
        const char *osdToken = nvt_adapter_param_get_string(entry, NULL);
        snprintf(trt__GetOSDsResponse->OSDs[i].token, TOKEN_LENGTH, "%s", osdToken);
        LOG_INFO("OSD token is %s\n", trt__GetOSDsResponse->OSDs[i].token);
        trt__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken = (struct tt__OSDReference *)soap_malloc(soap, sizeof(struct tt__OSDReference));
        MEMSET(trt__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken, 0x00, sizeof(struct tt__OSDReference));
        trt__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        snprintf(trt__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item, TOKEN_LENGTH, "%s", trt__GetOSDs->ConfigurationToken);
        LOG_INFO("OSD VideoSourceConfigurationToken is %s\n", trt__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item);
        trt__GetOSDsResponse->OSDs[i].Type = tt__OSDType__Text;
        trt__GetOSDsResponse->OSDs[i].Position = (struct tt__OSDPosConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDPosConfiguration));
        MEMSET(trt__GetOSDsResponse->OSDs[i].Position, 0x00, sizeof(struct tt__OSDPosConfiguration));
        trt__GetOSDsResponse->OSDs[i].Position->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);  
        MEMSET(trt__GetOSDsResponse->OSDs[i].Position->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(trt__GetOSDsResponse->OSDs[i].Position->Type, SMALL_BUFFER_LENGTH, "%s" ,"Custom");
        trt__GetOSDsResponse->OSDs[i].Position->Pos = (struct tt__Vector *)soap_malloc(soap, sizeof(struct tt__Vector));
        MEMSET(trt__GetOSDsResponse->OSDs[i].Position->Pos, 0x00, sizeof(struct tt__Vector));
        trt__GetOSDsResponse->OSDs[i].Position->Pos->x = (float *)soap_malloc(soap, sizeof(float));
        trt__GetOSDsResponse->OSDs[i].Position->Pos->y = (float *)soap_malloc(soap, sizeof(float));
        int pos_x = 0;
        int pos_y = 0;
        // snprintf(entry, 127, "osd.%d:position_x", i);
        // pos_x = nvt_adapter_param_get_int(entry, 0);
        // snprintf(entry, 127, "osd.%d:position_y", i);
        // pos_y = nvt_adapter_param_get_int(entry, 0);
        nvt_adapter_osd_get_position_x(j, &pos_x);
        LOG_INFO("Value of X from ini is %d\n", pos_x);

        nvt_adapter_osd_get_position_y(j, &pos_y);
        LOG_INFO("Value of Y from ini is %d\n", pos_y);
        
        inverseRescaleCoordinates(pos_x, pos_y, trt__GetOSDsResponse->OSDs[i].Position->Pos->x, trt__GetOSDsResponse->OSDs[i].Position->Pos->y);
        LOG_INFO("OSD position is x = %f, y = %f\n", *(trt__GetOSDsResponse->OSDs[i].Position->Pos->x), *(trt__GetOSDsResponse->OSDs[i].Position->Pos->y));

        trt__GetOSDsResponse->OSDs[i].TextString = (struct tt__OSDTextConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDTextConfiguration));
        MEMSET(trt__GetOSDsResponse->OSDs[i].TextString, 0x00, sizeof(struct tt__OSDTextConfiguration));
        trt__GetOSDsResponse->OSDs[i].TextString->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        if(0 == strcmp(trt__GetOSDsResponse->OSDs[i].token, "time_01")){
            snprintf(trt__GetOSDsResponse->OSDs[i].TextString->Type, SMALL_BUFFER_LENGTH, "%s", "DateAndTime");
            trt__GetOSDsResponse->OSDs[i].TextString->DateFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
            MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->DateFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
            snprintf(entry,127,"osd.%d:date_style", j);
            const char *dateFormat = nvt_adapter_param_get_string(entry, "YYYY-MM-DD");
            if (dateFormat && strlen(dateFormat) > 4) {
                omitFirstFourChars(dateFormat, dateFormatWithoutFirstFourChars);
                snprintf(trt__GetOSDsResponse->OSDs[i].TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", dateFormatWithoutFirstFourChars);
            } else {
                snprintf(trt__GetOSDsResponse->OSDs[i].TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", "YYYY-MM-DD");
            }
            trt__GetOSDsResponse->OSDs[i].TextString->TimeFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
            MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->TimeFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
            snprintf(entry,127,"osd.%d:time_style", j);
            trt__GetOSDsResponse->OSDs[i].TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));
            MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));
            trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));
            MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));
            // FIX: Use OSD-specific font color instead of hardcoded "osd.2"
            snprintf(entry, 127, "osd.%d:font_color", j);
            const char *fontColor = nvt_adapter_param_get_string(entry, "FFFFFF");
            LOG_INFO("Font color is %s\n", fontColor ? fontColor : "(default white)");
            if (fontColor && strlen(fontColor) > 0) {
                hexToYCbCr(fontColor, &y, &cb, &cr);
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->X = y;
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Y = cb;
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Z = cr;
            } else {
                // Default to white
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->X = 1.0;
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Y = 0.5;
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Z = 0.5;
            }
            const char *timeFormat = nvt_adapter_param_get_string(entry, "24hour");
            if(timeFormat && 0 == strcmp(timeFormat, "12hour")){
                snprintf(trt__GetOSDsResponse->OSDs[i].TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[1]);
            }
            else{
                snprintf(trt__GetOSDsResponse->OSDs[i].TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[0]);
            }

        }
        else{
            snprintf(trt__GetOSDsResponse->OSDs[i].TextString->Type, SMALL_BUFFER_LENGTH, "%s", "Plain");
            trt__GetOSDsResponse->OSDs[i].TextString->DateFormat = NULL;
            trt__GetOSDsResponse->OSDs[i].TextString->TimeFormat = NULL;
            trt__GetOSDsResponse->OSDs[i].TextString->PlainText = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
            MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->PlainText, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
            snprintf(entry, 127, "osd.%d:display_text", j);
            const char *displayText = nvt_adapter_param_get_string(entry, "");
            snprintf(trt__GetOSDsResponse->OSDs[i].TextString->PlainText, SMALL_BUFFER_LENGTH, "%s", displayText ? displayText : "");
            trt__GetOSDsResponse->OSDs[i].TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));
            MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));
            trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));
            MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));
            snprintf(entry, 127, "osd.%d:font_color", j);
            const char *fontColor = nvt_adapter_param_get_string(entry, "FFFFFF");
            LOG_INFO("Font color is %s\n", fontColor ? fontColor : "(default white)");
            if (fontColor && strlen(fontColor) > 0) {
                hexToYCbCr(fontColor, &y, &cb, &cr);
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->X = y;
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Y = cb;
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Z = cr;
            } else {
                // Default to white
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->X = 1.0;
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Y = 0.5;
                trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Z = 0.5;
            }


        }

        if(0 == strcmp(trt__GetOSDsResponse->OSDs[i].token, "title_01")){
            trt__GetOSDsResponse->OSDs[i].Extension = (struct tt__OSDConfigurationExtension *)soap_malloc(soap, sizeof(struct tt__OSDConfigurationExtension));
            MEMSET(trt__GetOSDsResponse->OSDs[i].Extension, 0x00, sizeof(struct tt__OSDConfigurationExtension));
            trt__GetOSDsResponse->OSDs[i].Extension->__size = 1;
            trt__GetOSDsResponse->OSDs[i].Extension->__any = (struct soap_dom_element *)soap_malloc(soap, trt__GetOSDsResponse->OSDs[i].Extension->__size*sizeof(struct soap_dom_element));
            MEMSET(trt__GetOSDsResponse->OSDs[i].Extension->__any, 0x00, trt__GetOSDsResponse->OSDs[i].Extension->__size*sizeof(struct soap_dom_element));
            trt__GetOSDsResponse->OSDs[i].Extension->__any->name = soap_strdup(soap, "tt:ChannelName");
            trt__GetOSDsResponse->OSDs[i].Extension->__any->text = soap_strdup(soap, "true");
            trt__GetOSDsResponse->OSDs[i].Extension->__any->type = 0;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->node = NULL;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->tail = NULL;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->soap = soap;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->code = NULL;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->lead = NULL;
            //dom_att(trt__GetOSDsResponse->OSDs[i].Extension->__any->atts, 0, NULL,NULL, NULL, NULL,soap);

        }

        else{
            trt__GetOSDsResponse->OSDs[i].Extension = (struct tt__OSDConfigurationExtension *)soap_malloc(soap, sizeof(struct tt__OSDConfigurationExtension));
            MEMSET(trt__GetOSDsResponse->OSDs[i].Extension, 0x00, sizeof(struct tt__OSDConfigurationExtension));
             trt__GetOSDsResponse->OSDs[i].Extension->__size = 1;
            trt__GetOSDsResponse->OSDs[i].Extension->__any = (struct soap_dom_element *)soap_malloc(soap, trt__GetOSDsResponse->OSDs[i].Extension->__size*sizeof(struct soap_dom_element));
            MEMSET(trt__GetOSDsResponse->OSDs[i].Extension->__any, 0x00, trt__GetOSDsResponse->OSDs[i].Extension->__size*sizeof(struct soap_dom_element));
            trt__GetOSDsResponse->OSDs[i].Extension->__any->name = soap_strdup(soap, "tt:ChannelName");
            trt__GetOSDsResponse->OSDs[i].Extension->__any->text = soap_strdup(soap, "false");
            trt__GetOSDsResponse->OSDs[i].Extension->__any->type = 0;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->node = NULL;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->tail = NULL;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->soap = soap;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->code = NULL;
            trt__GetOSDsResponse->OSDs[i].Extension->__any->lead = NULL;
            //dom_att(trt__GetOSDsResponse->OSDs[i].Extension->__any->atts, 0, NULL,NULL, NULL, NULL,soap);
        }

        trt__GetOSDsResponse->OSDs[i].TextString->FontSize = (int *)soap_malloc(soap, sizeof(int));
        MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->FontSize, 0x00, sizeof(int));
        int fontSize = nvt_adapter_param_get_int("osd.common:font_size", 0);
        *(trt__GetOSDsResponse->OSDs[i].TextString->FontSize) = fontSize;

        // trt__GetOSDsResponse->OSDs[i].TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));
        // MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));
        // trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));
        // MEMSET(trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));
        // const char *fontColor = nvt_adapter_param_get_string("osd.common:font_color", NULL);
        // LOG_INFO("Font color is %s\n", fontColor);
        // unsigned int hexCode = (unsigned int)strtol(fontColor, NULL, 16);
        // // hexToRGB(hexCode, &R, &G, &B);
        // // LOG_INFO("R = %d, G = %d, B = %d\n", R, G, B);
        // // RGBtoYCbCr(R, G, B, &y, &cb, &cr);
        // // LOG_INFO("Y = %f, Cb = %f, Cr = %f\n", y, cb, cr);
        // hexToYCbCr(fontColor, &y, &cb, &cr);
        // trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->X = y;
        // trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Y = cb;
        // trt__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Z = cr;

        LOG_INFO("End of GetOSDs API\n");
        i ++; 

    } 
   
    return SOAP_OK;   
}


/**
 * Sets the OSD (On-Screen Display) settings based on the given parameters.
 * 
 * @param soap The SOAP structure.
 * @param trt__SetOSD The input parameters for setting the OSD.
 * @param trt__SetOSDResponse The response structure for setting the OSD.
 * @return The status code indicating the success or failure of the operation.
 */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetOSD(struct soap *soap, struct _trt__SetOSD *trt__SetOSD, struct _trt__SetOSDResponse *trt__SetOSDResponse)
{
    LOG_INFO("=========Called __trt__SetOSD=========");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int retCode = SOAP_FAULT;
    int enabled = 0;
    bool m_b_isconfig = false;
    const char *osdToken;
    int position_x = 0;
    int position_y = 0;
    char *dateformat;
    char *timeformat;
    int index = 0;
    char* hexColor;
    bool DATE_TIME = false;
    bool PLAINTEXT = false;

    //LOG_INFO("Positio X is %f\n", *(trt__SetOSD->OSD->Position->Pos->x));
    //LOG_INFO("Positio Y is %f\n", *(trt__SetOSD->OSD->Position->Pos->y));

    do
    {
        if (NULL == trt__SetOSD)
        {
            //onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
            LOG_ERROR("Invalid request received.");
            retCode = SOAP_FAULT;
            break;
        }

        if (NULL == trt__SetOSD->OSD)
        {
            //onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
            LOG_ERROR("Invalid request received.");
            retCode = SOAP_FAULT;
            break;
        }

        if (NULL != trt__SetOSD->OSD)
        {
            if (NULL == trt__SetOSD->OSD->token || (0 == strlen(trt__SetOSD->OSD->token)))
            {
                //onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                LOG_ERROR("OSD token null");
                retCode = SOAP_FAULT;
                break;
            }
        }
        //Compare OSD Token
        
        if (NULL != trt__SetOSD->OSD->token)
        {
            for(int i = 0; i < 3; i++){
                char entry[127];    
                snprintf(entry, 127, "osd.%d:token", i);    
                osdToken = nvt_adapter_param_get_string(entry, NULL);
                if (0 == strcmp(osdToken, trt__SetOSD->OSD->token))
                {
                    m_b_isconfig = true;
                    break;
                    
                }
            }
        }

        if(NULL == trt__SetOSD->OSD->VideoSourceConfigurationToken->__item){
            //onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
            LOG_ERROR("VideoSourceConfigurationToken is NULL");
            retCode = SOAP_FAULT;
            break;
        }

        if (false == m_b_isconfig)
        {
            //onvif_fault(soap, ONVIF_ENV_SENDER, " ter:InvalidArgVal", "ter:NoConfig", "The requested configuration indicated with OSDToken does not exist.");
            LOG_ERROR( "The requested configuration indicated with OSDToken does not exist.");
            retCode = SOAP_FAULT;
            break;
        }

        nvt_adapter_osd_get_enable(&enabled);
        if (false == enabled)
        {
            //onvif_fault(soap, ONVIF_ENV_SENDER, " ter:InvalidArgVal", "ter:NoConfig", "The requested OSD does not exist.");
            LOG_ERROR("The requested OSD does not exist.");
            retCode = SOAP_FAULT;
            break;
        }

        if (NULL != trt__SetOSD->OSD)
        {
            //Token
            if (NULL != trt__SetOSD->OSD->token)
            {
                LOG_INFO("Token - %s", trt__SetOSD->OSD->token);
            }

            //VideoSourceConfigurationToken
            // if (NULL != trt__SetOSD->OSD->VideoSourceConfigurationToken)

            //Position
            if (NULL != trt__SetOSD->OSD->Position)
            {
                if (NULL != trt__SetOSD->OSD->Position->Type)
                {
                    LOG_INFO("Type - %s", trt__SetOSD->OSD->Position->Type);
                   
                    //TODO: Convert Position Type
                    if(!strcmp(trt__SetOSD->OSD->Position->Type ,"UpperRight")){
                        LOG_INFO("--------------------UpperRight-----------------\n");
                        position_x = 1700;
                        position_y = 16;
                    }
                    if(!strcmp(trt__SetOSD->OSD->Position->Type,"UpperLeft")){
                        LOG_INFO("--------------------UpperLeft-----------------\n");
                        position_x = 16;
                        position_y = 16;
                    }
                    if(!strcmp(trt__SetOSD->OSD->Position->Type, "LowerRight")){
                        LOG_INFO("--------------------LowerRight-----------------\n");
                        position_x = 1700;
                        position_y = 1000;
                    }
                    if(!strcmp(trt__SetOSD->OSD->Position->Type, "TopCenter")){
                        LOG_INFO("--------------------TopCenter-----------------\n");
                        position_x = 930;
                        position_y = 16;
                    }
                    if(!strcmp(trt__SetOSD->OSD->Position->Type, "BottomCenter")){
                        LOG_INFO("--------------------BottomCenter-----------------\n");
                        position_x = 930;
                        position_y = 1000;
                    }
                    if(!strcmp(trt__SetOSD->OSD->Position->Type, "LowerLeft")){
                        LOG_INFO("--------------------LowerLeft-----------------\n");
                        position_x = 16;
                        position_y = 1000;
                    }

                    if(!strcmp(trt__SetOSD->OSD->Position->Type, "Custom")){
                        LOG_INFO("--------------------Custom-----------------\n");
                        rescaleCoordinates(*(trt__SetOSD->OSD->Position->Pos->x), *(trt__SetOSD->OSD->Position->Pos->y), &position_x, &position_y);
                        LOG_INFO("X and Y are %f and %f \n", *(trt__SetOSD->OSD->Position->Pos->x), *(trt__SetOSD->OSD->Position->Pos->y));
                        LOG_INFO("Scaled X and Y are %d and %d \n", position_x, position_y);
                        if(strcmp(trt__SetOSD->OSD->TextString->Type,"DateAndTime")==0){
                            LOG_INFO("DateAndTime is selected\n");
                            nvt_adapter_osd_set_position_x(OSD_TIME_ID, position_x);
                            nvt_adapter_osd_set_position_y(OSD_TIME_ID, position_y);

                        }

                        
                    }
                    else{
                        LOG_INFO("Position mismatch!!\n");
                        LOG_INFO("trt__SetOSD->OSD->Position->Type is %s", trt__SetOSD->OSD->Position->Type);
                        break;
                    }
                }
            }

            //TextString
            
            if (NULL != trt__SetOSD->OSD->TextString)
            {
                //Type
                if (NULL != trt__SetOSD->OSD->TextString->Type)
                {
                    LOG_INFO("Type - %s", trt__SetOSD->OSD->TextString->Type);
                    if(strcmp(trt__SetOSD->OSD->TextString->Type,"Date")==0){
                        LOG_INFO("--------------------Date-----------------\n");
                        //ONLY_DATE = true;
                     
                    }
                    else if(strcmp(trt__SetOSD->OSD->TextString->Type,"Time")==0){
                        LOG_INFO("--------------------Time-----------------\n");
                        // ONLY_TIME = true;
                       

                    }
                    else if(strcmp(trt__SetOSD->OSD->TextString->Type,"DateAndTime")==0){
                        LOG_INFO("--------------------DateAndTime-----------------\n");
                        DATE_TIME = true;
                       

                    }
                    else if(strcmp(trt__SetOSD->OSD->TextString->Type,"Plain")==0){
                        LOG_INFO("--------------------PlainText-----------------\n");
                        PLAINTEXT = true;
                        LOG_INFO("PLAINTEXT - %s", trt__SetOSD->OSD->TextString->PlainText);
                    
                    }
                    else{
                        LOG_INFO("Type mismatch!!\n");
                        LOG_INFO("trt__SetOSD->OSD->TextString->Type is %s", trt__SetOSD->OSD->TextString->Type);
                        break;
                    }
                }

                //DateFormat
                if (NULL != trt__SetOSD->OSD->TextString->DateFormat)
                {

                    //TODO: define the osdDateFormat in includes to use it without multiple definition
                    LOG_INFO("-----checking date format------");
                    LOG_INFO("DateFormat - %s", trt__SetOSD->OSD->TextString->DateFormat);
                    index = 0;
                    for(index = 0; index < 6; index ++){
                        if(strcmp(trt__SetOSD->OSD->TextString->DateFormat ,osdDateFormat[index])==0){
                            dateformat = trt__SetOSD->OSD->TextString->DateFormat;
                            LOG_INFO("Date format recieved is ------------------------------------------- %s", dateformat);
                            break;
                        }
                        else{
                            dateformat = NULL;
                            continue;
                        }
                    }

                    
                    if(dateformat == NULL){
                        onvif_fault(soap, ONVIF_ENV_SENDER, " ter:InvalidArgVal", "ter:NoConfig", "The requested dateformat doesnot exist");
                        LOG_ERROR("invalid dateformat recived");
                        dateformat = "YYYY-MM-DD";

                    }
                    // FIX: Use properly sized buffer instead of stack-overflow-prone char[]
                    char prefix[128] = "CHR-";
                    strncat(prefix, dateformat, sizeof(prefix) - strlen(prefix) - 1);
                    nvt_adapter_osd_set_date_style(OSD_TIME_ID, prefix);

                    
                }
                //TimeFormat
                if (NULL != trt__SetOSD->OSD->TextString->TimeFormat)
                {
                    LOG_INFO("TimeFormat - %s", trt__SetOSD->OSD->TextString->TimeFormat);
                    //char *osdTimeFormat[] = {"hh:mm:ss", "hh:mm:ss tt"};
                    if(strcmp(trt__SetOSD->OSD->TextString->TimeFormat, osdTimeFormat[0])==0){
                        timeformat = "24hour";
                    }
                    else if(strcmp(trt__SetOSD->OSD->TextString->TimeFormat, osdTimeFormat[1])==0){
                        timeformat = "12hour";
                    }
                    else{
                        timeformat = "12hour";
                    }
                    nvt_adapter_osd_set_time_style(OSD_TIME_ID, timeformat);

                }
                //FontSize
                if (NULL != trt__SetOSD->OSD->TextString->FontSize)
                {
                    int ret = 0;
                    int current_font_size;
                    LOG_INFO("FontSize - %d", *(trt__SetOSD->OSD->TextString->FontSize));
                    ret = nvt_adapter_osd_get_font_size(&current_font_size);
                    if (ret!= 0){
                        LOG_ERROR("Failed to get current font size");
                        return -1;
                    }
                    if(current_font_size!= *(trt__SetOSD->OSD->TextString->FontSize)){
                        LOG_INFO("Current font size is %d, new font size is %d", current_font_size, *(trt__SetOSD->OSD->TextString->FontSize));
                        ret = nvt_adapter_osd_set_font_size(*(trt__SetOSD->OSD->TextString->FontSize));
                        if (ret!= 0){
                            LOG_ERROR("Failed to set font size");
                            return -1;
                        }
                    
                    }
                }
                //TODO: set the font color(identify the color space)
                if (NULL != trt__SetOSD->OSD->TextString->FontColor)
                {
                    //Color
                    if (NULL != trt__SetOSD->OSD->TextString->FontColor->Color)
                    {
                        LOG_INFO("Color X - %f \n", trt__SetOSD->OSD->TextString->FontColor->Color->X);
                        LOG_INFO("Color Y - %f \n", trt__SetOSD->OSD->TextString->FontColor->Color->Y);
                        LOG_INFO("Color Z - %f \n", trt__SetOSD->OSD->TextString->FontColor->Color->Z);

                        hexColor = ycbcrToHex(trt__SetOSD->OSD->TextString->FontColor->Color->X, trt__SetOSD->OSD->TextString->FontColor->Color->Y, trt__SetOSD->OSD->TextString->FontColor->Color->Z);
                        LOG_INFO("Hex color is %s\n", hexColor);
                        int ret = 0;
                        if(DATE_TIME){
                            nvt_adapter_osd_set_font_color_of_dateTime(hexColor);
                        }
                        if(!strcmp(trt__SetOSD->OSD->token, "title_01")){
                            nvt_adapter_osd_set_font_color_of_channelName(hexColor);
                            ret = nvt_adapter_osd_set_display_text(OSD_CHANNEL_ID, trt__SetOSD->OSD->TextString->PlainText);
                            if(ret != 0 ){
                                LOG_ERROR("Failed to set display text");
                                return -1;
                            }
                        }
                        if(!strcmp(trt__SetOSD->OSD->token, "location_01")){
                            nvt_adapter_osd_set_font_color_of_channelLocation(hexColor);
                            ret = nvt_adapter_osd_set_display_text(OSD_CHARACTER_ID, trt__SetOSD->OSD->TextString->PlainText);
                            if(ret != 0 ){
                                LOG_ERROR("Failed to set display text");
                                return -1;
                            }
                        }

                    }
                }

                //PlainText
                if (NULL != trt__SetOSD->OSD->TextString->PlainText)
                {
                    LOG_INFO("PlainText - %s", trt__SetOSD->OSD->TextString->PlainText);

                }

                if (NULL != trt__SetOSD->OSD->TextString->IsPersistentText)
                {
                    LOG_INFO("IsPersistentText - %d", trt__SetOSD->OSD->TextString->IsPersistentText);
                    

                }
            }
            //SetOSDSettings
            if(DATE_TIME == true){
                LOG_INFO("----------Date and Time OSD-----------");
                // char prefix[] = "CHR-";
                // strcat(prefix, dateformat);
                nvt_adapter_osd_set_enabled(OSD_TIME_ID, 1);
                //LOG_INFO("prefix - %s", prefix);
                // nvt_adapter_osd_set_position_x(OSD_TIME_ID, position_x);
                // nvt_adapter_osd_set_position_y(OSD_TIME_ID, position_y);
                LOG_INFO("------------------ Position Y is ----------------%d", position_y);
                // nvt_adapter_osd_set_time_style(OSD_TIME_ID, timeformat);
                // nvt_adapter_osd_set_date_style(OSD_TIME_ID, prefix);
                DATE_TIME = false;
            }
            if(PLAINTEXT == true){
                int ret = 0;
                if(!strcmp(osdToken, "title_01")){
                    LOG_INFO("Title OSD");
                    nvt_adapter_osd_set_enabled(OSD_CHANNEL_ID, 1);
                    nvt_adapter_osd_set_position_x(OSD_CHANNEL_ID, position_x);
                    nvt_adapter_osd_set_position_y(OSD_CHANNEL_ID, position_y);
                    // ret = nvt_adapter_osd_set_display_text(OSD_CHANNEL_ID, trt__SetOSD->OSD->TextString->PlainText);
                    // if(ret != 0 ){
                    //     LOG_ERROR("Failed to set display text");
                    //     return -1;
                    // }
                    //nvt_adapter_osd_set_font_color_of_channelName(hexColor);
                    PLAINTEXT = false;
                }
                else{
                    LOG_INFO("PLAINTEXT - %s", trt__SetOSD->OSD->TextString->PlainText);
                    nvt_adapter_osd_set_enabled(OSD_CHARACTER_ID, 1);
                    nvt_adapter_osd_set_position_x(OSD_CHARACTER_ID, position_x);
                    nvt_adapter_osd_set_position_y(OSD_CHARACTER_ID, position_y);
                    // ret = nvt_adapter_osd_set_display_text(OSD_CHARACTER_ID, trt__SetOSD->OSD->TextString->PlainText);
                    // if(ret != 0 ){
                    // LOG_ERROR("Failed to set display text");
                    // return -1;
                    // }
                    //nvt_adapter_osd_set_font_color_of_channelLocation(hexColor);
                    PLAINTEXT = false;
                }


            }
           
            nvt_adapter_osd_restart();
        }
        
        retCode = SOAP_OK;
    }
    while (false);
    
    return retCode;
    
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateOSD(struct soap *soap, struct _trt__CreateOSD *trt__CreateOSD, struct _trt__CreateOSDResponse *trt__CreateOSDResponse){
    LOG_INFO("=========Called trt__CreateOSD=========");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int retCode = SOAP_FAULT;
    const char *OSD_TOKEN;
    const char *VSCToken;
    const char *OSDType;
    const char *OSDPosition;
    const char *OSDText;
    const char *OSDDateFormat;
    const char *OSDTimeFormat;
    const char *OSDPlainText;
    enum xsd__boolean *OSDIsPersistentText;
    int OSDPosX;
    int OSDPosY;
    int OSDFontSize;
    bool CharID1 = false;
    bool CharID2 = false;
    bool DateTimeID = false;

    
    // Print request structure
    LOG_INFO("Received trt__CreateOSD request:");
    LOG_INFO("OSD Token: %s", trt__CreateOSD->OSD->token);
    if (trt__CreateOSD->OSD->VideoSourceConfigurationToken) {
        LOG_INFO("VideoSourceConfigurationToken: %s", trt__CreateOSD->OSD->VideoSourceConfigurationToken->__item);
    }
    if (trt__CreateOSD->OSD->Position && trt__CreateOSD->OSD->Position->Type) {
        LOG_INFO("Position Type: %s", trt__CreateOSD->OSD->Position->Type);
    }
    if (trt__CreateOSD->OSD->TextString && trt__CreateOSD->OSD->TextString->Type) {
        LOG_INFO("TextString Type: %s", trt__CreateOSD->OSD->TextString->Type);
    }
    if (trt__CreateOSD->OSD->TextString && trt__CreateOSD->OSD->TextString->DateFormat) {
        LOG_INFO("TextString DateFormat: %s", trt__CreateOSD->OSD->TextString->DateFormat);
    }
    if (trt__CreateOSD->OSD->TextString && trt__CreateOSD->OSD->TextString->TimeFormat) {
        LOG_INFO("TextString TimeFormat: %s", trt__CreateOSD->OSD->TextString->TimeFormat);
    }
    if (trt__CreateOSD->OSD->TextString && trt__CreateOSD->OSD->TextString->FontSize) {
        LOG_INFO("TextString FontSize: %d", *(trt__CreateOSD->OSD->TextString->FontSize));
    }
    if (trt__CreateOSD->OSD->TextString && trt__CreateOSD->OSD->TextString->PlainText) {
        LOG_INFO("TextString PlainText: %s", trt__CreateOSD->OSD->TextString->PlainText);
    }
    if (trt__CreateOSD->OSD->TextString && trt__CreateOSD->OSD->TextString->IsPersistentText) {
        LOG_INFO("TextString IsPersistentText: %d", *(trt__CreateOSD->OSD->TextString->IsPersistentText));
    }
    if(NULL == trt__CreateOSD)
    {
        LOG_ERROR("Invalid argument. trt__CreateOSD is NULL....\n");
        return retCode;
    }

    if(NULL == trt__CreateOSD->OSD)
    {
        LOG_ERROR("Invalid argument. trt__CreateOSD->OSD is NULL....\n");
        return retCode;
    }

    // if(NULL == trt__CreateOSD->OSD->token || (0 == strlen(trt__CreateOSD->OSD->token)))
    // {
    //     LOG_ERROR("Invalid argument. trt__CreateOSD->OSD->token is NULL ----- %s\n", trt__CreateOSD->OSD->token);
    //     return retCode;
    
    // }
    LOG_INFO("OSD Token - %s", trt__CreateOSD->OSD->token);
    if (NULL != trt__CreateOSD->OSD->VideoSourceConfigurationToken)
    {
        LOG_INFO("VideoSourceConfigurationToken - %s", trt__CreateOSD->OSD->VideoSourceConfigurationToken->__item);
        if(0 != strcmp(trt__CreateOSD->OSD->VideoSourceConfigurationToken->__item, vsrc_srcToken)){
            LOG_ERROR("Invalid VideoSourceConfigurationToken received.");
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Configuration token error");
            return SOAP_FAULT;
        }
    }

    //Position
    if (NULL != trt__CreateOSD->OSD->Position)
    {
            if (NULL != trt__CreateOSD->OSD->Position->Type)
            {
                LOG_INFO("Type - %s", trt__CreateOSD->OSD->Position->Type);
                if(0 == strcmp(trt__CreateOSD->OSD->Position->Type, "UpperRight")){
                    //LOG_INFO("--------------------UpperRight-----------------\n");
                    OSDPosition = "UpperRight";
                    OSDPosX = 1700;
                    OSDPosY = 16;
                }
                else if(0 == strcmp(trt__CreateOSD->OSD->Position->Type, "UpperLeft")){
                    //LOG_INFO("--------------------UpperLeft-----------------\n");
                    OSDPosition = "UpperLeft";
                    OSDPosX = 16;
                    OSDPosY = 16;
                }
                else if(0 == strcmp(trt__CreateOSD->OSD->Position->Type, "LowerRight")){
                    //LOG_INFO("--------------------LowerRight-----------------\n");
                    OSDPosition = "LowerRight";
                    OSDPosX = 1700;
                    OSDPosY = 1000;
                }
                else if(0 == strcmp(trt__CreateOSD->OSD->Position->Type, "TopCenter")){
                    //LOG_INFO("--------------------TopCenter-----------------\n");
                    OSDPosition = "TopCenter";
                    OSDPosX = 930;
                    OSDPosY = 16;

                }
                else if(0 == strcmp(trt__CreateOSD->OSD->Position->Type, "BottomCenter")){
                    //LOG_INFO("--------------------BottomCenter-----------------\n");
                    OSDPosition = "BottomCenter";
                    OSDPosX = 930;
                    OSDPosY = 1000;

                }
                else if(0 == strcmp(trt__CreateOSD->OSD->Position->Type, "LowerLeft")){
                    //LOG_INFO("--------------------LowerLeft-----------------\n");
                    OSDPosition = "LowerLeft";
                    OSDPosX = 16;
                    OSDPosY = 1000;
                }
                else if(0 == strcmp(trt__CreateOSD->OSD->Position->Type, "Custom")){
                    //LOG_INFO("----------------Custom-----------------\n");
                    OSDPosition = "Custom";
                    if(NULL != trt__CreateOSD->OSD->Position->Pos){
                        rescaleCoordinates(*(trt__CreateOSD->OSD->Position->Pos->x), *(trt__CreateOSD->OSD->Position->Pos->y), &OSDPosX, &OSDPosY);
                        // OSDPosX = *(trt__CreateOSD->OSD->Position->Pos->x);
                        // OSDPosY = *(trt__CreateOSD->OSD->Position->Pos->y);
                    }
                    else{
                        LOG_ERROR("Invalid position received");
                        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                        return SOAP_FAULT;
                    }
                }
                else{
                    LOG_INFO("Type mismatch!!\n");
                    LOG_INFO("trt__CreateOSD->OSD->Position->Type is %s", trt__CreateOSD->OSD->Position->Type);
                    onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                    return SOAP_FAULT;
                }

            }
        }

        //TextString
        if (NULL != trt__CreateOSD->OSD->TextString)
        {
                //Type
                if (NULL != trt__CreateOSD->OSD->TextString->Type)
                {
                    LOG_INFO("Type - %s", trt__CreateOSD->OSD->TextString->Type);
                    if(0 == strcmp(trt__CreateOSD->OSD->TextString->Type, "DateAndTime")){
                        LOG_INFO("--------------------DateAndTime-----------------\n");
                        OSDText = "DateAndTime";
                    }
                    else if(0 == strcmp(trt__CreateOSD->OSD->TextString->Type, "Plain")){
                        LOG_INFO("--------------------PlainText-----------------\n");
                        OSDText = "Plain";
                    }
                    else{
                        LOG_INFO("Type mismatch!!\n");
                        LOG_INFO("trt__CreateOSD->OSD->TextString->Type is %s", trt__CreateOSD->OSD->TextString->Type);
                        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                    }
                }

                //DateFormat
                if (NULL != trt__CreateOSD->OSD->TextString->DateFormat)
                {
                    LOG_INFO("DateFormat - %s", trt__CreateOSD->OSD->TextString->DateFormat);
                    for(int i = 0; i < 6; i++){
                        if(0 == strcmp(trt__CreateOSD->OSD->TextString->DateFormat, osdDateFormat[i])){
                            OSDDateFormat = trt__CreateOSD->OSD->TextString->DateFormat;
                            break;
                        }
                        else{
                            OSDDateFormat = NULL;
                            continue;
                        }
                    }
                    if(OSDDateFormat == NULL){
                        LOG_ERROR("Invalid date format received");
                        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                        return SOAP_FAULT;
                    }
                    
                }

                //TimeFormat
                if (NULL != trt__CreateOSD->OSD->TextString->TimeFormat)
                {
                    LOG_INFO("TimeFormat - %s", trt__CreateOSD->OSD->TextString->TimeFormat);
                    for(int i = 0; i < 2; i++){
                        if(0 == strcmp(trt__CreateOSD->OSD->TextString->TimeFormat, osdTimeFormat[i])){
                            if(0 == i){
                                OSDTimeFormat = "24hour";
                            }
                            else{
                                OSDTimeFormat = "12hour";
                            }
                            break;
                        }
                        else{
                            OSDTimeFormat = NULL;
                            continue;
                        }
                    }
                    if(OSDTimeFormat == NULL){
                        LOG_ERROR("Invalid time format received");
                        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                        return SOAP_FAULT;
                    }
                    
                }

                //FontSize
                if (NULL != trt__CreateOSD->OSD->TextString->FontSize)
                {
                    LOG_INFO("FontSize - %d", *(trt__CreateOSD->OSD->TextString->FontSize));
                    OSDFontSize = *(trt__CreateOSD->OSD->TextString->FontSize);
                }
                else
                {
                    LOG_INFO("Default FontSize will be used");
                    OSDFontSize = OSD_DEFAULT_FONT_SIZE; // Default font size will be used.
                }

                //PlainText
                if (NULL != trt__CreateOSD->OSD->TextString->PlainText)
                {
                    LOG_INFO("PlainText - %s", trt__CreateOSD->OSD->TextString->PlainText);
                    OSDPlainText = trt__CreateOSD->OSD->TextString->PlainText;
                    
                }

                //IsPersistentText
                if (NULL != trt__CreateOSD->OSD->TextString->IsPersistentText)
                {
                    LOG_INFO("IsPersistentText - %d", trt__CreateOSD->OSD->TextString->IsPersistentText);
                    OSDIsPersistentText = trt__CreateOSD->OSD->TextString->IsPersistentText;
                }

                // if(NULL != trt__CreateOSD->OSD->Extension->__size){
                //     LOG_INFO("Extension size - %d", trt__CreateOSD->OSD->Extension->__size);
                // }
            }

        // Create osd

        nvt_adapter_osd_set_font_size(OSDFontSize);
        if(0 == strcmp(trt__CreateOSD->OSD->TextString->Type, "DateAndTime")){
            char entry[128];
            snprintf(entry, 127, "osd.%d:enabled", OSD_TIME_ID);
            if(nvt_adapter_param_get_int(entry, 0) == 1){
                LOG_INFO("OSD time ID already exists");
                onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "OSD time ID already exists");
                return SOAP_FAULT;
            }
            LOG_INFO("Creating DateAndTime OSD");
            nvt_adapter_osd_set_enabled(OSD_TIME_ID, 1);
            nvt_adapter_osd_set_position_x(OSD_TIME_ID, OSDPosX);
            nvt_adapter_osd_set_position_y(OSD_TIME_ID, OSDPosY);
            // FIX: Use properly sized buffer instead of stack-overflow-prone char[]
            char prefix[128] = "CHR-";
            strncat(prefix, OSDDateFormat, sizeof(prefix) - strlen(prefix) - 1);
            nvt_adapter_osd_set_date_style(OSD_TIME_ID, prefix);
            nvt_adapter_osd_set_time_style(OSD_TIME_ID, OSDTimeFormat);
            
        }
        else if(0 == strcmp(trt__CreateOSD->OSD->TextString->Type, "Plain")){
            char entry[128];
            snprintf(entry, 127, "osd.%d:enabled", OSD_CHARACTER_ID);
            if(nvt_adapter_param_get_int(entry, 0) == 1){
                LOG_INFO("OSD charcter ID 1 already exists");
                CharID1 = true;

            }
            snprintf(entry, 127, "osd.%d:enabled", OSD_CHANNEL_ID);
            if(nvt_adapter_param_get_int(entry, 0) == 1){
                LOG_INFO("OSD charcter ID 2 already exists");
                CharID2 = true;
            }
            if(CharID1 == true && CharID2 == true){
                LOG_INFO("Both OSD character IDs are already in use");
                onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Both OSD character IDs are already in use");
                return SOAP_FAULT;
            }
            if(CharID1 == true ){
                LOG_INFO("Creating PlainText OSD");
                nvt_adapter_osd_set_enabled(OSD_CHANNEL_ID, 1);
                nvt_adapter_osd_set_position_x(OSD_CHANNEL_ID, OSDPosX);
                nvt_adapter_osd_set_position_y(OSD_CHANNEL_ID, OSDPosY);
                nvt_adapter_osd_set_display_text(OSD_CHANNEL_ID, OSDPlainText);
                OSD_TOKEN = "title_01";

            }
            if(CharID2 == true){
                LOG_INFO("Creating PlainText OSD");
                nvt_adapter_osd_set_enabled(OSD_CHARACTER_ID, 1);
                nvt_adapter_osd_set_position_x(OSD_CHARACTER_ID, OSDPosX);
                nvt_adapter_osd_set_position_y(OSD_CHARACTER_ID, OSDPosY);
                nvt_adapter_osd_set_display_text(OSD_CHARACTER_ID, OSDPlainText);
                OSD_TOKEN = "location_01";
            }
        }
        else{
            LOG_INFO("Invalid OSD text type");
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid OSD text type");
            return SOAP_FAULT;
        }

        nvt_adapter_osd_restart();
        trt__CreateOSDResponse->OSDToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__CreateOSDResponse->OSDToken, 0x00, sizeof(char) * TOKEN_LENGTH);

        if(0 == strcmp(OSDText, "DateAndTime")){
            snprintf(trt__CreateOSDResponse->OSDToken, TOKEN_LENGTH, "%s", "time_01");
        }
        else{
            snprintf(trt__CreateOSDResponse->OSDToken, TOKEN_LENGTH, "%s", OSD_TOKEN);
        }  
        retCode = SOAP_OK; 
        return retCode;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteOSD(struct soap *soap, struct _trt__DeleteOSD *trt__DeleteOSD, struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse){
    LOG_INFO("----------------Inside __trt__DeleteOSD--------------------");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    char entry[128];
    const char *osdToken = NULL;
    int OSD_ID = -1;
    bool tokenFound = false;
    int retCode = 0;

    if(NULL == trt__DeleteOSD){
        LOG_ERROR("Invalid request received.");
        return SOAP_FAULT;
    }
    if(NULL == trt__DeleteOSD->OSDToken){
        LOG_ERROR("OSD Token is NULL.");
        return SOAP_FAULT;
    }

    for(int i = 0; i<3; i++){
        snprintf(entry, 127, "osd.%d:token", i);
        osdToken = nvt_adapter_param_get_string(entry, NULL);
        if(0 == strcmp(osdToken, "title_01")){
            continue;
        }
        if(0 == strcmp(osdToken, trt__DeleteOSD->OSDToken)){
            LOG_INFO("OSD Token found.");
            snprintf(entry, 127, "osd.%d:enabled", i);
            int enabled = nvt_adapter_param_get_int(entry, 0);
            if(0 == enabled){
                LOG_INFO("The requested OSD does not exist.");  
                retCode = SOAP_FAULT;
                break;         
            }
            else{
                snprintf(entry, 127, "osd.%d:enabled", i);
                nvt_adapter_param_set_int(entry, 0);
                LOG_INFO("OSD deleted successfully.");
                nvt_adapter_osd_restart();
                retCode = SOAP_OK;
                break;
            }
        }
    }
    return retCode;

}


/** Web service operation '__trt__GetCompatibleVideoEncoderConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap,
                                                                         struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations,
                                                                         struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
    int profIndex = -1;
    char *value = malloc(20);
    const char *profile_value;
    printf("Called __trt__GetCompatibleVideoEncoderConfigurations....\n");
    printf("Called __trt__GetCompatibleVideoEncoderConfigurations before null....\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if ((NULL == trt__GetCompatibleVideoEncoderConfigurations) ||
        (NULL == trt__GetCompatibleVideoEncoderConfigurations->ProfileToken))
    {
        printf("Invalid argument. trt__GetCompatibleVideoEncoderConfigurations or ProfileToken is NULL....\n");
        return SOAP_FAULT;
    }
    printf("Called __trt__GetCompatibleVideoEncoderConfigurations not null....\n");
      
    printf("Given trt__GetCompatibleVideoEncoderConfigurations ProfileToken is = %s\n", trt__GetCompatibleVideoEncoderConfigurations->ProfileToken);

    profIndex = onvif_profile_token_to_channel(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken);
    if (profIndex < 0 && (0 == strncmp("Test_Profile", trt__GetCompatibleVideoEncoderConfigurations->ProfileToken, TOKEN_LENGTH)) && testProfilecreated == true)
        profIndex = 3;

    
    if (-1 == profIndex)
    {
        //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
        printf("Given profile token not found is = %s\n", trt__GetCompatibleVideoEncoderConfigurations->ProfileToken);
        return SOAP_FAULT;
    }
    else
    {
        printf("Given profile token found Index = %d\n", profIndex);
        
    }
     if((profIndex >= 0 && profIndex <= 2) || profIndex == 3)

    {
            trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations = 1;
             printf("trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations is 1....\n");
    }
    else{
            trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations = 3;
    }
    //@TODO, proper respose send with check of H.265 also

     trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations =
        (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoderConfiguration));
    MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations, 0x00,
           trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoderConfiguration));

     if(profIndex == 0 || profIndex == 1 || profIndex == 2)
     {

       trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, profIndex + 1);

        trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, profIndex + 1);

        trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].UseCount = 1;

         trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Encoding = tt__VideoEncoding__H264;
          trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Resolution =
                    (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    
         nvt_adapter_video_get_resolution(profIndex, value);
         sscanf(value, "%d*%d", &trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Resolution->Width,
         &trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Resolution->Height);

          trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Quality =  10.0;
               

            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].SessionTimeout = timeout;
               
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl =
                (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
            MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl, 0x00,
                    sizeof(struct tt__VideoRateControl));

            nvt_adapter_video_get_frame_rate(profIndex, &trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl->FrameRateLimit);
           // free(value);
               
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl->EncodingInterval = 1;

            nvt_adapter_video_get_max_rate(profIndex,& trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].RateControl->BitrateLimit);

            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].MPEG4 = NULL;
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].H264 = 
                                (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
            // Determines the interval in which the I-Frames will be coded 
            nvt_adapter_video_get_gop(profIndex,&trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->GovLength);

           // trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->H264Profile = tt__H264Profile__Main;
            int h264_profile_val = 0;
            nvt_adapter_video_get_h264_profile(profIndex, &h264_profile_val);
            printf("H264 PROFILE value profile==================  %d\n", h264_profile_val);
            if(h264_profile_val == 0)
            {    
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->H264Profile = tt__H264Profile__Baseline;
            }
            else if(h264_profile_val == 1)
            {
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->H264Profile = tt__H264Profile__High;
            }
            else 
            {
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].H264->H264Profile = tt__H264Profile__Main;
            }
            free(value);

            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast =
                    (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
            MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address =
                                             (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
            MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address->Type = 0;//tt__IPType__IPv4; //TODO: This param should be dynamic

            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address->IPv4Address =
                (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
            MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address->IPv4Address, 0x00,
                    sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
            strcpy(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Address->IPv4Address,"0.0.0.0");

            // trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->Port =

            // trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->TTL =

            // trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->AutoStart =

            //     trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->__size = 0;
            //     trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].Multicast->__any = NULL;
             
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].__size = 0;
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[profIndex].__any = NULL;
            
      }
      //For Test Profie
      else if (profIndex == 3)
      {
        trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].token, 0x00, sizeof(char) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER, 1);
        
        trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
       
        ONVIF_TOKEN_GEN(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_VIDEO_ENCODER_NAME, 1);
      
        
        trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].UseCount = 2;
       

        trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Encoding = tt__VideoEncoding__H264;
        trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Resolution =
                    (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    
         nvt_adapter_video_get_resolution(0, value);
         sscanf(value, "%d*%d", &trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Resolution->Width,
         &trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Resolution->Height);

          trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Quality =  10.0;
               

            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].SessionTimeout = timeout;
               
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].RateControl =
                (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
            MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].RateControl, 0x00,
                    sizeof(struct tt__VideoRateControl));

            nvt_adapter_video_get_frame_rate(0, &trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit);
           // free(value);
               
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].RateControl->EncodingInterval = 1;

            nvt_adapter_video_get_max_rate(0,& trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].RateControl->BitrateLimit);

            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].MPEG4 = NULL;
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].H264 = 
                                (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
            // Determines the interval in which the I-Frames will be coded 
            nvt_adapter_video_get_gop(0,&trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].H264->GovLength);

           // trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].H264->H264Profile = tt__H264Profile__Main;
            int h264_profile_val = 0;
            nvt_adapter_video_get_h264_profile(0, &h264_profile_val);
            printf("H264 PROFILE value profile for test profile==================  %d\n", h264_profile_val);
            if(h264_profile_val == 0)
            {    
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].H264->H264Profile = tt__H264Profile__Baseline;
            }
            else if(h264_profile_val == 1)
            {
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].H264->H264Profile = tt__H264Profile__High;
            }
            else 
            {
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].H264->H264Profile = tt__H264Profile__Main;
            }
            free(value);

            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast =
                    (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
            MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->Address =
                                             (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
            MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->Address->Type = 0;//tt__IPType__IPv4; //TODO: This param should be dynamic

            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->Address->IPv4Address =
                (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
            MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->Address->IPv4Address, 0x00,
                    sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
            strcpy(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->Address->IPv4Address,"0.0.0.0");

            // trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->Port =

            // trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->TTL =

            // trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->AutoStart =

            //     trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->__size = 0;
            //     trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].Multicast->__any = NULL;
             
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].__size = 0;
            trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[0].__any = NULL;

      }


/*
    T_PROFILE_SETTING l_x_profs; //Don't memset this structure
    UINT32 l_ui32_outLen = sizeof(T_PROFILE_SETTING);
    BOOL l_b_isFound = false;
    BOOL l_b_isConfig = false;

    if (RCF_SUCCESS != getSysServerResp(GET_MEDIA_PROFILE, NULL, 0, &l_x_profs, l_ui32_outLen))
    {
        onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Fail to get Profiles");
        RCF_PRINTF(PRIO_ERROR, "Failed to get profiles from system server");
        return SOAP_FAULT;
    }

    for (INT32 i = 0; i < l_x_profs.m_u8_noOfProfile; i++)
    {
        if (0 == strncmp(l_x_profs.m_v_profileList[i].m_ca_profileToken, trt__GetCompatibleVideoEncoderConfigurations->ProfileToken, TOKEN_LENGTH))
        {
            l_b_isFound = true;
            break;
        }
    }
    if (false == l_b_isFound)
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token does not exist.");
        RCF_PRINTF(PRIO_ERROR, "Given profile token not found[%s]", trt__GetCompatibleVideoEncoderConfigurations->ProfileToken)
        return SOAP_FAULT;
    }

    INT32 i_i32_maxEncoders = STREAM_AUDIO;
    T_STREAM_ENCODER_CONFIGS l_x_vEncConfigs;
    l_ui32_outLen = sizeof(T_STREAM_ENCODER_CONFIGS);
    INT32 l_i32_index = 0;

    MEMSET(&l_x_vEncConfigs, 0x00, sizeof(T_STREAM_ENCODER_CONFIGS));

    if (RCF_SUCCESS != getSysServerResp(GET_STREAM_ENCODER_SETTINGS, NULL, 0, &l_x_vEncConfigs, l_ui32_outLen))
    {
        onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Fail to get Video encoder settings");
        RCF_PRINTF(PRIO_ERROR, "Failed to get video encoder configurations from system server");
        return SOAP_FAULT;
    }
    else
    {
        RCF_PRINTF(PRIO_DEBUG, "Successfully received video encoder configurations from system server");
    }

    if((0 == STRCMP(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken, "ProfileMain1")) ||
        (0 == STRCMP(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken, "ProfileSub2")) ||
        (0 == STRCMP(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken, "ProfileThird3")))
    {
        for (INT32 l_i32_idx = 0; l_i32_idx < i_i32_maxEncoders; l_i32_idx++)
        {
            for (INT32 index = 0; index < (INT32)CODEC_MPEG4; index++)
            {
                if (TRUE == l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_idx][index].m_b_enableCodec)
                {
                    CONSTCHARPTR pToken = &(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken[STRLEN(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken) - 1]);
                    CONSTCHARPTR cToken = &(l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_idx][index].m_ac_streamEncoderToken[STRLEN(l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_idx][index].m_ac_streamEncoderToken) - 1]);

                    if (0 == (STRCMP(pToken, cToken)))
                    {
                        l_i32_index = l_i32_idx;
                        l_b_isConfig = true;
                        break;
                    }
                }
            }
            if(true == l_b_isConfig)
            {
                break;
            }
        }
        if (false == l_b_isConfig)
        {
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration indicated with ConfigurationToken does not exist");
            RCF_PRINTF(PRIO_ERROR, "Config token not found for Profile [%s]", trt__GetCompatibleVideoEncoderConfigurations->ProfileToken)
            return SOAP_FAULT;
        }
        trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations = 1;
    }
    else
    {
        trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations = i_i32_maxEncoders;
        l_i32_index = 0;
    }

    // TODO: How to deal with H265 codec 
    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations =
        (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoderConfiguration));
    MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations, 0x00,
           trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoderConfiguration));

    for (INT32 l_i32_idx = 0; l_i32_idx < trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations; l_i32_idx++)
    {
        for (INT32 index = 0; index < (INT32)CODEC_MPEG4; index++)
        {
            if (TRUE == l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_b_enableCodec)
            {
                if (CODEC_H265 == l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_e_codecType)
                {
                    // Profile S does not support H.265 codec 
                    continue;
                }

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].token = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].token, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].token, TOKEN_LENGTH, "%s",
                         l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_ac_streamEncoderToken);

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
                MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
                SNPRINTF(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Name, TOKEN_LENGTH, "%s",
                         l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_ca_name);

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].UseCount =
                    l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_ui32_usecount;

                // TODO: GauranteedFrameRate : Currently no need 

                //JPEG = 0, MPEG4 = 1, H264 = 2
                switch (l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_e_codecType)
                {
                case CODEC_H264:
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Encoding = tt__VideoEncoding__H264;
                    break;

                case CODEC_MJPEG:
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Encoding = tt__VideoEncoding__JPEG;
                    break;

                default:
                    onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:InvalidArgVal", "ter:InternalError","Fail to get codec");
                    RCF_PRINTF(PRIO_ERROR, "Failed to get video CODEC");
                    return SOAP_FAULT;
                }

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Resolution =
                    (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));

                std::pair<UINT32, UINT32> resolutionPair = GetResolution(l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_idx][index].m_e_resolution);
                if ((0 != resolutionPair.first) && (0 != resolutionPair.second))
                {
                    // Set resolution width 
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Resolution->Width = resolutionPair.first;
                    // Set resolution height 
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Resolution->Height = resolutionPair.second;
                }
                else
                {
                    onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:InvalidArgVal", "ter:InternalError","Fail to get Resolution");
                    RCF_PRINTF(PRIO_DEBUG, "Failed to get resolution - resolution[%d] Encoder[%d]",
                               l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_e_resolution, index);
                    return SOAP_FAULT;
                }

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Quality =
                    GetQualityFromEnum(l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_e_imageQuality,
                                       l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_ui8_customizedImageQuality);

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].SessionTimeout =
                    std::chrono::milliseconds(l_x_vEncConfigs.m_x_multicastConfig.m_ui64_rtspSessionTimeout).count();
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].RateControl =
                    (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
                MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].RateControl, 0x00,
                       sizeof(struct tt__VideoRateControl));
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].RateControl->FrameRateLimit =
                    GetFpsFromEnum(l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_e_fps,
                                   l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_ui8_customizedFPS);

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].RateControl->EncodingInterval = 1;

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].RateControl->BitrateLimit =
                    GetBitrateValueInKbps(l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_e_bitRate,
                                          l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_ui32_customizedBitRate);

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].MPEG4 = NULL;
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].H264 =
                    (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
                // Determines the interval in which the I-Frames will be coded 
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].H264->GovLength =
                l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_ui32_govLength;

                switch (l_x_vEncConfigs.m_x_stream.m_x_stream[l_i32_index][index].m_e_profile)
                {
                case PROFILE_BASELINE:
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].H264->H264Profile = tt__H264Profile__Baseline;
                    break;
                case PROFILE_MAIN:
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].H264->H264Profile = tt__H264Profile__Main;
                    break;
                case PROFILE_HIGH:
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].H264->H264Profile = tt__H264Profile__High;
                    break;
                default:
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].H264->H264Profile = tt__H264Profile__Main;
                    break;
                }

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast =
                    (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
                MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address =
                    (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
                MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->Type = tt__IPType__IPv4; //TODO: This param should be dynamic
                if (tt__IPType__IPv4 == trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->Type)
                {
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address =
                        (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
                    MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address, 0x00,
                           sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
                    SNPRINTF(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address,
                             IPV4_ADDR_STRING_LEN + 1, "%s", l_x_vEncConfigs.m_x_multicastConfig.m_x_multicastStream[l_i32_index].m_aui8_GroupAddr);
                }
                else
                {
                    // TODO: Currently not supported 
                    trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv6Address =
                        (CHARPTR)soap_malloc(soap, sizeof(CHAR) * MAX_IPV6_IP_ADDR_LEN);
                    MEMSET(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv6Address, 0x00,
                           sizeof(CHAR) * MAX_IPV6_IP_ADDR_LEN);
                }

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->Port =
                    l_x_vEncConfigs.m_x_multicastConfig.m_x_multicastStream[l_i32_index].m_ui16_videoPort;
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->TTL =
                    l_x_vEncConfigs.m_x_multicastConfig.m_x_multicastStream[l_i32_index].m_ui16_ttl;
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->AutoStart =
                    (xsd__boolean)l_x_vEncConfigs.m_x_multicastConfig.m_x_multicastStream[l_i32_index].m_b_alwaysMulticastState;
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->__size = 0;
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->__any = NULL;
                //dom_att(&trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].Multicast->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].__size = 0;
                trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].__any = NULL;
                //dom_att(&trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[l_i32_idx].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
            }
        }
        l_i32_index++;
    }

   
   */
    return SOAP_OK;
    

}
/** Web service operation '__trt__CreateProfile' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap *soap,
                                               struct _trt__CreateProfile *trt__CreateProfile,
                                               struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
     printf("Called __trt__CreateProfile....\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    if (NULL == trt__CreateProfile)
    {
        printf("trt__CreateProfile is NULL");
     //   onvif_fault(soap, ONVIF_ENV_SENDER, "ter:OperationProhibited", "ter:InvalidArgument", "Invalid request received");
        return SOAP_FAULT;
    }
    if (NULL == trt__CreateProfile->Name)
    {
        printf("Profile name is NULL");
      //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:OperationProhibited", "ter:InvalidArgument", "Invalid request received");
        return SOAP_FAULT;
    }

    if (testProfilecreated == false)
    {
        T_CREATE_CONFIG_INFO l_x_createProfileInfo;
        unsigned int  l_ui32_createProfileInfo = sizeof(T_CREATE_CONFIG_INFO);
        MEMSET(&l_x_createProfileInfo, 0x00, sizeof(T_CREATE_CONFIG_INFO));

        // Check that Create profile command available or not

        if ((NULL != trt__CreateProfile->Token) && (strlen(trt__CreateProfile->Token) > 0))
        {
            snprintf(l_x_createProfileInfo.m_ca_profileToken, TOKEN_LENGTH, "%s", trt__CreateProfile->Token);
        }

        if((NULL != trt__CreateProfile->Name) && (strlen(trt__CreateProfile->Name) > 0))
        {
            snprintf(l_x_createProfileInfo.m_ca_profileName, TOKEN_LENGTH, "%s", trt__CreateProfile->Name);
        }

        testProfilecreated  = true;
        //@TODO, need to check for Maximum supported profile by device RV1106? and check here for available token and max supoorted token
  //  }

    

    // nretCode = getSysServerResp(CREATE_MEDIA_PROFILE, &l_x_createProfileInfo, sizeof(T_CREATE_CONFIG_INFO), &l_x_createProfileInfo,l_ui32_createProfileInfo);
    // if (RCF_SUCCESS != nretCode)
    // {
    //     if(RCF_MAX_PROFILE_REACHED == nretCode)
    //     {
    //         RCF_PRINTF(PRIO_ERROR, "The maximum number of supported profiles supported by the device has been reached");
    //         onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:MaxNVTProfiles", "The maximum number of supported profiles supported by the device has been reached");
    //     }
    //     else
    //     {
    //         RCF_PRINTF(PRIO_ERROR, "Failed to get create profile information from system server");
    //         onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Failed to get create profile information from system server");
    //     }
    //     return SOAP_FAULT;
    // }
    // else
    // {
    //     RCF_PRINTF(PRIO_DEBUG, "Got create profile information from system server");
    // }

    trt__CreateProfileResponse->Profile = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile));
    MEMSET(trt__CreateProfileResponse->Profile, 0x00, sizeof(struct tt__Profile));

    trt__CreateProfileResponse->Profile->token = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
    MEMSET(trt__CreateProfileResponse->Profile->token, 0x00, sizeof(CHAR) * TOKEN_LENGTH);

    snprintf(l_x_createProfileInfo.m_ca_profileToken,TOKEN_LENGTH,"%s","Test_Profile");

    snprintf(trt__CreateProfileResponse->Profile->token, TOKEN_LENGTH, "%s", l_x_createProfileInfo.m_ca_profileToken);
    

    trt__CreateProfileResponse->Profile->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
    MEMSET(trt__CreateProfileResponse->Profile->Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
    snprintf(trt__CreateProfileResponse->Profile->Name, TOKEN_LENGTH, "%s", l_x_createProfileInfo.m_ca_profileName);
    

     return SOAP_OK;
    }
    else
    {
        return SOAP_FAULT;
    }
}
/** Web service operation '__trt__AddVideoSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(struct soap *soap,
                                                             struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration,
                                                             struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
    printf("--------------Called AddVideoSourceConfiguration---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    if ((NULL == trt__AddVideoSourceConfiguration->ProfileToken) || (0 == strlen(trt__AddVideoSourceConfiguration->ProfileToken)))
    {
      //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        //RCF_PRINTF(PRIO_ERROR, "The requested profile token [%s] does not exist.", trt__AddVideoSourceConfiguration->ProfileToken);
        printf("The requested profile token [%s] does not exist.", trt__AddVideoSourceConfiguration->ProfileToken);
        return SOAP_FAULT;
    }
    if ((NULL == trt__AddVideoSourceConfiguration->ConfigurationToken) || (0 == strlen(trt__AddVideoSourceConfiguration->ConfigurationToken)))
    {
       // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
       // RCF_PRINTF(PRIO_ERROR, "The VideoSourceConfiguration indicated by the ConfigurationToken[%s] does not exist.", trt__AddVideoSourceConfiguration->ConfigurationToken);
        printf("The VideoSourceConfiguration indicated by the ConfigurationToken[%s] does not exist.", trt__AddVideoSourceConfiguration->ConfigurationToken);
        return SOAP_FAULT;
    }

    T_ONVIF_CONFIG_INFO l_x_onvifConfInfo;
    MEMSET(&l_x_onvifConfInfo, 0x00, sizeof(T_ONVIF_CONFIG_INFO));
   // l_x_onvifConfInfo.m_ui16_commandID = (unsigned short int)ADD_VIDEO_SRC_CONF;
    snprintf(l_x_onvifConfInfo.m_ca_tokenName, TOKEN_LENGTH, "%s", trt__AddVideoSourceConfiguration->ConfigurationToken);

    printf("l_x_onvifConfInfo.m_ca_tokenName is (AddVideoSourceConfiguration) = %s\n",l_x_onvifConfInfo.m_ca_tokenName);

    testProfileAddSourceConfig = true;

    // unsigned int l_i32_offset = 0;
    // unsigned char l_ui8_noOfConfigs = 1;
    // unsigned int l_i32_memBytes = (TOKEN_LENGTH * sizeof(CHAR)) + sizeof( unsigned char) + l_ui8_noOfConfigs * sizeof(T_ONVIF_CONFIG_INFO);
    // unsigned char *l_vp_data = ( unsigned char *)soap_malloc(soap, l_i32_memBytes);
    // MEMSET(l_vp_data, 0x00, l_i32_memBytes);

    // memcpy((void *)(l_vp_data + l_i32_offset), trt__AddVideoSourceConfiguration->ProfileToken, TOKEN_LENGTH);
    // l_i32_offset += TOKEN_LENGTH;
    // memcpy((void *)(l_vp_data + l_i32_offset), &l_ui8_noOfConfigs, sizeof( unsigned char));
    // l_i32_offset += sizeof( unsigned char);
    // memcpy((void *)(l_vp_data + l_i32_offset), &l_x_onvifConfInfo, sizeof(T_ONVIF_CONFIG_INFO));

   // unsigned int l_ui32_outLen = 0;
    // if (RCF_SUCCESS != getSysServerResp(MODIFY_PROFILE, l_vp_data, l_i32_memBytes, NULL, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:Action", "ter:InternalError", "Fail Modify Profiles");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to add video source configuration from system server");
    //     return SOAP_FAULT;
    // }

    return SOAP_OK;
}
/** Web service operation '__trt__AddVideoEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(struct soap *soap,
                                                              struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration,
                                                              struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
     printf("--------------Called AddVideoEncoderConfiguration---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // INT32 l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    // if (l_i32_ret != SOAP_OK)
    //     return l_i32_ret;

    if ((NULL == trt__AddVideoEncoderConfiguration->ProfileToken) || (0 == strlen(trt__AddVideoEncoderConfiguration->ProfileToken)))
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "The requested profile token [%s] does not exist.", trt__AddVideoEncoderConfiguration->ProfileToken);
         printf("The requested profile token [%s] does not exist.", trt__AddVideoEncoderConfiguration->ProfileToken);
        return SOAP_FAULT;
    }

    if ((NULL == trt__AddVideoEncoderConfiguration->ConfigurationToken) || (0 == strlen(trt__AddVideoEncoderConfiguration->ConfigurationToken)))
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "The VideoEncoderConfiguration indicated by the ConfigurationToken[%s] does not exist.", trt__AddVideoEncoderConfiguration->ConfigurationToken);
         printf("The VideoEncoderConfiguration indicated by the ConfigurationToken[%s] does not exist.", trt__AddVideoEncoderConfiguration->ConfigurationToken);
        return SOAP_FAULT;
    }

    T_ONVIF_CONFIG_INFO l_x_onvifConfInfo;
    // MEMSET(&l_x_onvifConfInfo, 0x00, sizeof(T_ONVIF_CONFIG_INFO));
    // l_x_onvifConfInfo.m_ui16_commandID = (UINT16)ADD_VIDEO_ENC_CONF;
    //SNPRINTF(l_x_onvifConfInfo.m_ca_tokenName, TOKEN_LENGTH, "%s", trt__AddVideoEncoderConfiguration->ConfigurationToken);
    snprintf(l_x_onvifConfInfo.m_ca_tokenName, TOKEN_LENGTH, "%s", trt__AddVideoEncoderConfiguration->ConfigurationToken);

    printf("l_x_onvifConfInfo.m_ca_tokenName is (AddVideoEncoderConfiguration) = %s\n",l_x_onvifConfInfo.m_ca_tokenName);

    testProfileAddVideoEncoderConfig = true;

    // UINT32 l_i32_offset = 0;
    // UINT8 l_ui8_noOfConfigs = 1;
    // UINT32 l_i32_memBytes = (TOKEN_LENGTH * sizeof(CHAR)) + sizeof(UINT8) + l_ui8_noOfConfigs * sizeof(T_ONVIF_CONFIG_INFO);
    // UINT8 *l_vp_data = (UINT8 *)soap_malloc(soap, l_i32_memBytes);
    // MEMSET(l_vp_data, 0x00, l_i32_memBytes);

    // MEMCPY((void *)(l_vp_data + l_i32_offset), trt__AddVideoEncoderConfiguration->ProfileToken, TOKEN_LENGTH);
    // l_i32_offset += TOKEN_LENGTH;
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_ui8_noOfConfigs, sizeof(UINT8));
    // l_i32_offset += sizeof(UINT8);
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_x_onvifConfInfo, sizeof(T_ONVIF_CONFIG_INFO));

    // UINT32 l_ui32_outLen = 0;
    // if (RCF_SUCCESS != getSysServerResp(MODIFY_PROFILE, l_vp_data, l_i32_memBytes, NULL, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:Action", "ter:InternalError", "Fail Modify Profiles");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to add video encoder configuration from system server");
    //     return SOAP_FAULT;
    // }

    return SOAP_OK;
}
/** Web service operation '__trt__RemoveVideoSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(struct soap *soap,
                                                                struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration,
                                                                struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
    printf("Called RemoveVideoSourceConfiguration");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == trt__RemoveVideoSourceConfiguration)
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. trt__RemoveVideoSourceConfiguration[%p]", trt__RemoveVideoSourceConfiguration);
        printf("Invalid argument. trt__RemoveVideoSourceConfiguration[%p]", trt__RemoveVideoSourceConfiguration);
        return SOAP_FAULT;
    }

    if (NULL == trt__RemoveVideoSourceConfiguration->ProfileToken)
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. ProfileToken[%p]", trt__RemoveVideoSourceConfiguration->ProfileToken);
        printf("Invalid argument. ProfileToken[%p]", trt__RemoveVideoSourceConfiguration->ProfileToken);
        return SOAP_FAULT;
    }

    if (0 == strlen(trt__RemoveVideoSourceConfiguration->ProfileToken))
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. Invalid argument. ProfileToken length is zero");
        printf("Invalid argument. Invalid argument. ProfileToken length is zero");
        return SOAP_FAULT;
    }
      // Remove source configuration also from test profile
    testProfileAddSourceConfig = false;
    
    // T_ONVIF_CONFIG_INFO l_x_onvifConfInfo;
    // MEMSET(&l_x_onvifConfInfo, 0x00, sizeof(T_ONVIF_CONFIG_INFO));
    // l_x_onvifConfInfo.m_ui16_commandID = (UINT16)REMOVE_VIDEO_SRC_CONF;

    // UINT32 l_i32_offset = 0;
    // UINT8 l_ui8_noOfConfigs = 1;
    // UINT32 l_i32_memBytes = TOKEN_LENGTH * sizeof(CHAR) + sizeof(UINT8) + l_ui8_noOfConfigs * sizeof(T_ONVIF_CONFIG_INFO);
    // UINT8 *l_vp_data = (UINT8 *)soap_malloc(soap, l_i32_memBytes);
    // MEMSET(l_vp_data, 0x00, l_i32_memBytes);

    // MEMCPY((void *)(l_vp_data + l_i32_offset), trt__RemoveVideoSourceConfiguration->ProfileToken, TOKEN_LENGTH);
    // l_i32_offset += TOKEN_LENGTH;
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_ui8_noOfConfigs, sizeof(UINT8));
    // l_i32_offset += sizeof(UINT8);
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_x_onvifConfInfo, sizeof(T_ONVIF_CONFIG_INFO));

    // UINT32 l_ui32_outLen = 0;
    // if (RCF_SUCCESS != getSysServerResp(MODIFY_PROFILE, l_vp_data, l_i32_memBytes, NULL, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:Action", "ter:InternalError", "Fail Modify Profiles");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to remove video source configuration from system server");
    //     return SOAP_FAULT;
    // }

    return SOAP_OK;
}
/** Web service operation '__trt__RemoveVideoEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(struct soap *soap,
                                                                 struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration,
                                                                 struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
   printf("Called RemoveVideoEncoderConfiguration");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == trt__RemoveVideoEncoderConfiguration)
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. trt__RemoveVideoEncoderConfiguration[%p]", trt__RemoveVideoEncoderConfiguration);
        printf("Invalid argument. trt__RemoveVideoEncoderConfiguration[%p]", trt__RemoveVideoEncoderConfiguration);
        return SOAP_FAULT;
    }

    if (NULL == trt__RemoveVideoEncoderConfiguration->ProfileToken)
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. ProfileToken[%p]", trt__RemoveVideoEncoderConfiguration->ProfileToken);
        printf("Invalid argument. ProfileToken[%p]", trt__RemoveVideoEncoderConfiguration->ProfileToken);
        return SOAP_FAULT;
    }

    if (0 == strlen(trt__RemoveVideoEncoderConfiguration->ProfileToken))
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. Invalid argument. ProfileToken length is zero");
        printf("Invalid argument. Invalid argument. ProfileToken length is zero");
        return SOAP_FAULT;
    }
    // Remove video encoder configuration from test profile
    testProfileAddVideoEncoderConfig = false;

    // T_ONVIF_CONFIG_INFO l_x_onvifConfInfo;
    // MEMSET(&l_x_onvifConfInfo, 0x00, sizeof(T_ONVIF_CONFIG_INFO));
    // l_x_onvifConfInfo.m_ui16_commandID = (UINT16)REMOVE_VIDEO_ENC_CONF;

    // UINT32 l_i32_offset = 0;
    // UINT8 l_ui8_noOfConfigs = 1;
    // UINT32 l_i32_memBytes = TOKEN_LENGTH * sizeof(CHAR) + sizeof(UINT8) + l_ui8_noOfConfigs * sizeof(T_ONVIF_CONFIG_INFO);
    // UINT8 *l_vp_data = (UINT8 *)soap_malloc(soap, l_i32_memBytes);
    // MEMSET(l_vp_data, 0x00, l_i32_memBytes);

    // MEMCPY((void *)(l_vp_data + l_i32_offset), trt__RemoveVideoEncoderConfiguration->ProfileToken, TOKEN_LENGTH);
    // l_i32_offset += TOKEN_LENGTH;
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_ui8_noOfConfigs, sizeof(UINT8));
    // l_i32_offset += sizeof(UINT8);
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_x_onvifConfInfo, sizeof(T_ONVIF_CONFIG_INFO));

    // UINT32 l_ui32_outLen = 0;
    // if (RCF_SUCCESS != getSysServerResp(MODIFY_PROFILE, l_vp_data, l_i32_memBytes, NULL, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:Action", "ter:InternalError", "Fail Modify Profiles");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to remove video encoder configuration from system server");
    //     return SOAP_FAULT;
    // }

    return SOAP_OK;
}

/** Web service operation '__trt__GetGuaranteedNumberOfVideoEncoderInstances' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap,
                                                                            struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances,
                                                                            struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
    printf("-----------------Called GetGuaranteedNumberOfVideoEncoderInstances---------------------------------");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // INT32 l_i32_idx = 0;
    // T_VIDEO_SRC_CONFIG l_x_vEncInst;
    // UINT32 l_ui32_outLen = sizeof(T_VIDEO_SRC_CONFIG);

    // if (RCF_SUCCESS != getSysServerResp(GET_VIDEO_SRC_CONFIGURATION, NULL, 0, &l_x_vEncInst, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Fail to get Guaranteed Number Of Video Encoder Instances configurations");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to Guaranteed Number Of Video Encoder Instances source configurations from system server");
    //     return SOAP_FAULT;
    // }
    // else
    // {
    //     RCF_PRINTF(PRIO_DEBUG, "Got Guaranteed Number Of Video Encoder Instances configurations from system server successfully");
    // }
    
    if (0 == strcmp(trt__GetGuaranteedNumberOfVideoEncoderInstances->ConfigurationToken,vsrc_confToken))
    {
        printf("VideoSource Config Token is Match");
    }
    else{
         onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration indicated with ConfigurationToken does not exist");
         printf( "Given video source configuration token not found");
        return SOAP_FAULT;
    }

    // for (l_i32_idx = 0; l_i32_idx < l_x_vEncInst.m_ui8_noOfVideoSourceConfigurations; l_i32_idx++)
    // {
    //     if (0 == STRCMP(l_x_vEncInst.m_lx_videoSourceConfigurationParam[l_i32_idx].m_ca_confToken, trt__GetGuaranteedNumberOfVideoEncoderInstances->ConfigurationToken))
    //     {
    //         break;
    //     }
    // }

    // /* Check that video source config token found or not */
    // if (l_i32_idx == l_x_vEncInst.m_ui8_noOfVideoSourceConfigurations)
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration indicated with ConfigurationToken does not exist");
    //     RCF_PRINTF(PRIO_ERROR, "Given video source configuration token not found");
    //     return SOAP_FAULT;
    // }

    trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse->TotalNumber = 3;

    trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse->JPEG = NULL;
    trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse->H264 = (int *)soap_malloc(soap, sizeof(int));
    *(trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse->H264) = 3;
    trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse->MPEG4 = NULL;

    return SOAP_OK;
}
/** Web service operation '__trt__GetCompatibleVideoSourceConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap,
                                                                        struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations,
                                                                        struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
    int profIndex = -1;
    printf("--------------------------Called GetCompatibleVideoSourceConfigurations---------------------------------");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if ((NULL == trt__GetCompatibleVideoSourceConfigurations) ||
        (NULL == trt__GetCompatibleVideoSourceConfigurations->ProfileToken))
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        printf("Invalid argument. trt__GetCompatibleVideoSourceConfigurations or ProfileToken is NULL");
        return SOAP_FAULT;
    }

    profIndex = onvif_profile_token_to_channel(trt__GetCompatibleVideoSourceConfigurations->ProfileToken);
    if (profIndex < 0 && (0 == strncmp("Test_Profile", trt__GetCompatibleVideoSourceConfigurations->ProfileToken, TOKEN_LENGTH)) && testProfilecreated == true)
        profIndex = 3;

    if (-1 == profIndex)
    {
       onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token does not exist.");
        printf("Given profile token not found is = %s\n", trt__GetCompatibleVideoSourceConfigurations->ProfileToken);
        return SOAP_FAULT;
    }
    else
    {
        printf("Given profile token found Index = %d\n", profIndex);
        
    }

        // T_VIDEO_SRC_CONFIG l_x_vSrcConfig;
    // l_ui32_outLen = sizeof(T_VIDEO_SRC_CONFIG);

    // if (RCF_SUCCESS != getSysServerResp(GET_VIDEO_SRC_CONFIGURATION, NULL, 0, &l_x_vSrcConfig, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Fail to get Video Source Configuration");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to get video source configurations from system server");
    //     return SOAP_FAULT;
    // }
    // else
    // {
    //     RCF_PRINTF(PRIO_DEBUG, "Got video source configurations from system server successfully");
    // }

    trt__GetCompatibleVideoSourceConfigurationsResponse->__sizeConfigurations = 1;
    trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations =
        (struct tt__VideoSourceConfiguration *)soap_malloc(soap, (trt__GetCompatibleVideoSourceConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoSourceConfiguration)));
    MEMSET(trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations, 0x00, trt__GetCompatibleVideoSourceConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoSourceConfiguration));

 
        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].token = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].token, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        snprintf(trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].token, TOKEN_LENGTH, "%s",vsrc_confToken);
               

        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        snprintf(trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Name, TOKEN_LENGTH, "%s",vsrc_name);

        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].UseCount = vsrc_useCount;

        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].ViewMode = NULL;

        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].SourceToken = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].SourceToken, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        snprintf(trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].SourceToken, TOKEN_LENGTH, "%s",vsrc_srcToken);


        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds =
            (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds->x = vsrc_boundX;

        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds->y = vsrc_boundY;
           
        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds->width = vsrc_boundWidth;

        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds->height = vsrc_boundHeight;


        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Extension = NULL;
           
        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].__size = 0;
        trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].__any = NULL;
    //    //dom_att(&trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    return SOAP_OK;
}
/** Web service operation '__trt__SetVideoSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(struct soap *soap,
                                                             struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration,
                                                             struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
    printf("----------------Called SetVideoSourceConfiguration------------------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == trt__SetVideoSourceConfiguration)
    {
        printf("Invalid argument. trt__SetVideoSourceConfiguration is NULL");
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        return SOAP_FAULT;
    }

    if (NULL == trt__SetVideoSourceConfiguration->Configuration)
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        printf("Invalid argument. Configuration object pointer is NULL");
        return SOAP_FAULT;
    }

    if ((NULL == trt__SetVideoSourceConfiguration->Configuration->token) ||
        (0 == strlen(trt__SetVideoSourceConfiguration->Configuration->token)))
    {
        printf("Invalid argument. Video source configuration token is NULL or empty");
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        return SOAP_FAULT;
    }

    if ((NULL == trt__SetVideoSourceConfiguration->Configuration->Name) || (0 == strlen(trt__SetVideoSourceConfiguration->Configuration->Name)))
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        printf("Invalid video source configuration name");
        return SOAP_FAULT;
    }

    if (0 == strcmp(vsrc_confToken, trt__SetVideoSourceConfiguration->Configuration->token))
    {
        printf("-------------Given video source configuration token found----------------------------\n");
    }
    else{
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The configuration does not exist.");
        printf("Given video source configuration token not found");
        return SOAP_FAULT;
    }

    if((NULL != trt__SetVideoSourceConfiguration->Configuration->Name) && (strlen(trt__SetVideoSourceConfiguration->Configuration->Name) > 0))
    {
        snprintf(vsrc_name, TOKEN_LENGTH, "%s", trt__SetVideoSourceConfiguration->Configuration->Name);
    }
   
    bool  l_b_invalidParam = true;
    if (NULL != trt__SetVideoSourceConfiguration->Configuration->Bounds)
    {
        if ((trt__SetVideoSourceConfiguration->Configuration->Bounds->x >= vsrc_minX) &&
            (trt__SetVideoSourceConfiguration->Configuration->Bounds->x <= vsrc_maxX))
            vsrc_boundX = trt__SetVideoSourceConfiguration->Configuration->Bounds->x;
        else
            l_b_invalidParam = false;

        if ((trt__SetVideoSourceConfiguration->Configuration->Bounds->y >= vsrc_minY) &&
            (trt__SetVideoSourceConfiguration->Configuration->Bounds->y <= vsrc_maxY))
            vsrc_boundY = trt__SetVideoSourceConfiguration->Configuration->Bounds->y;
        else
            l_b_invalidParam = false;

        if ((trt__SetVideoSourceConfiguration->Configuration->Bounds->width >= vsrc_minboundWidth) &&
            (trt__SetVideoSourceConfiguration->Configuration->Bounds->width <= vsrc_maxboundWidth))
            vsrc_boundWidth = trt__SetVideoSourceConfiguration->Configuration->Bounds->width;
        else
            l_b_invalidParam = false;

        if ((trt__SetVideoSourceConfiguration->Configuration->Bounds->height >= vsrc_minboundHeight) &&
            (trt__SetVideoSourceConfiguration->Configuration->Bounds->height <= vsrc_maxboundHeight))
            vsrc_boundHeight = trt__SetVideoSourceConfiguration->Configuration->Bounds->height;
        else
            l_b_invalidParam = false;
    }

    if (l_b_invalidParam == false)
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:ConfigModify", "The configuration parameters are not possible to set");
        printf("------------------------------Invalid input parameter to set video source configs-----------------------\n");
        return SOAP_FAULT;
    }

    /* No need to take care about Respose */
    return SOAP_OK;
}

/** Web service operation '__trt__DeleteProfile' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap *soap,
                                               struct _trt__DeleteProfile *trt__DeleteProfile,
                                               struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
    printf("----------------------------Called DeleteProfile-------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == trt__DeleteProfile)
    {
        printf("--------trt__DeleteProfile is NULL------------\n");
      //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        return SOAP_FAULT;
    }

    if (NULL == trt__DeleteProfile->ProfileToken)
    {
        printf("---------------Profile token is NULL----------------\n");
     //   onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        return SOAP_FAULT;
    }

    if (0 == strlen(trt__DeleteProfile->ProfileToken))
    {
      //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        printf("------------------Invalid argument. Invalid argument. ProfileToken length is zero-----------------------\n");
        return SOAP_FAULT;
    }
    if(testProfilecreated)
    {
        testProfilecreated = false;
         printf("======after ====testProfilecreated==============%d",testProfilecreated);
    }
    // T_ONVIF_CONFIG_INFO l_x_onvifConfInfo;
    // MEMSET(&l_x_onvifConfInfo, 0x00, sizeof(T_ONVIF_CONFIG_INFO));
    // l_x_onvifConfInfo.m_ui16_commandID = (UINT16)DELETE_MEDIA_PROFILE;

    // UINT32 l_i32_offset = 0;
    // UINT8 l_ui8_noOfConfigs = 1;
    // UINT32 l_i32_memBytes = TOKEN_LENGTH * sizeof(CHAR) + sizeof(UINT8) + l_ui8_noOfConfigs * sizeof(T_ONVIF_CONFIG_INFO);
    // UINT8 *l_vp_data = (UINT8 *)soap_malloc(soap, l_i32_memBytes);
    // MEMSET(l_vp_data, 0x00, l_i32_memBytes);

    // MEMCPY((void *)(l_vp_data + l_i32_offset), trt__DeleteProfile->ProfileToken, TOKEN_LENGTH);
    // l_i32_offset += TOKEN_LENGTH;
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_ui8_noOfConfigs, sizeof(UINT8));
    // l_i32_offset += sizeof(UINT8);
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_x_onvifConfInfo, sizeof(T_ONVIF_CONFIG_INFO));

    // UINT32 l_ui32_outLen = 0;
    // if (RCF_SUCCESS != getSysServerResp(MODIFY_PROFILE, l_vp_data, l_i32_memBytes, NULL, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:Action", "ter:InternalError", "Fail Modify Profiles");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to delete profile from system server");
    //     return SOAP_FAULT;
    // }
    printf("--------return--------------------Called DeleteProfile-------------------\n");
    return SOAP_OK;
}
/** Web service operation '__trt__GetMetadataConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration(struct soap *soap,
                                                          struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration,
                                                          struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
    printf("-------------------Called GetMetadataConfiguration---------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // SIZET l_i32_idx = 0;
    // T_METADATA_CONFIGS l_x_metadataConfigs;
    // UINT32 l_ui32_outLenMetadataConfigs = sizeof(T_METADATA_CONFIGS);

    // // Request for all stream info 
    // if (RCF_SUCCESS != getSysServerResp(GET_METADATA_CONFIGS, NULL, 0, &l_x_metadataConfigs, l_ui32_outLenMetadataConfigs))
    // {
    //     onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:InvalidArgVal", "ter:InternalError","Fail to Get Audio Encoder Configurations");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to get metadata from system server");
    //     return SOAP_FAULT;
    // }
    // else
    // {
    //     RCF_PRINTF(PRIO_DEBUG, "Successfully received metadata from system server");
    // }

    // for (l_i32_idx = 0; l_i32_idx < l_x_metadataConfigs.m_lx_metadataConfigList.size(); l_i32_idx++)
    // {
    //     if (0 == STRCMP(l_x_metadataConfigs.m_lx_metadataConfigList[l_i32_idx].m_ca_configToken, trt__GetMetadataConfiguration->ConfigurationToken))
    //     {
    //         break;
    //     }
    // }

    // if (l_i32_idx == l_x_metadataConfigs.m_lx_metadataConfigList.size())
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration indicated with ConfigurationToken doesnot exist");
    //     RCF_PRINTF(PRIO_ERROR, "Given Metadata configuration token not found");
    //     return SOAP_FAULT;
    // }

     if (0 == strcmp(mdata_confToken, trt__GetMetadataConfiguration->ConfigurationToken))
    {
        printf("-------------------Called GetMetadataConfiguration------conf token Match---------\n");
    }
    else{
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration indicated with ConfigurationToken doesnot exist");
        printf("-----------------Given Metadata configuration token not found----------------------------\n");
         return SOAP_FAULT;
    }

     //Configuration
     trt__GetMetadataConfigurationResponse->Configuration =
         (struct tt__MetadataConfiguration *)soap_malloc(soap, sizeof(struct tt__MetadataConfiguration));
     MEMSET(trt__GetMetadataConfigurationResponse->Configuration, 0x00, sizeof(struct tt__MetadataConfiguration));

     //token
     trt__GetMetadataConfigurationResponse->Configuration->token = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
     MEMSET(trt__GetMetadataConfigurationResponse->Configuration->token, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
   // snprintf(trt__GetMetadataConfigurationResponse->Configuration->token, TOKEN_LENGTH, "%s","MetaDataConfigToken_1");
    snprintf(trt__GetMetadataConfigurationResponse->Configuration->token, TOKEN_LENGTH, "%s",mdata_confToken);
   
     //Name
     trt__GetMetadataConfigurationResponse->Configuration->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
     MEMSET(trt__GetMetadataConfigurationResponse->Configuration->Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
     snprintf(trt__GetMetadataConfigurationResponse->Configuration->Name, TOKEN_LENGTH, "%s",mdata_name);
        
     //UseCount
     trt__GetMetadataConfigurationResponse->Configuration->UseCount = mdata_useCount;//1;


     //Multicast
     trt__GetMetadataConfigurationResponse->Configuration->Multicast =
         (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
     MEMSET(trt__GetMetadataConfigurationResponse->Configuration->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));

     //Address
     trt__GetMetadataConfigurationResponse->Configuration->Multicast->Address =
         (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
     MEMSET(trt__GetMetadataConfigurationResponse->Configuration->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

     trt__GetMetadataConfigurationResponse->Configuration->Multicast->Address->Type = tt__IPType__IPv4; //TODO: This param should be dynamic


    trt__GetMetadataConfigurationResponse->Configuration->Multicast->Address->IPv4Address =
            (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
    MEMSET(trt__GetMetadataConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 0x00,
            sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
   // snprintf(trt__GetMetadataConfigurationResponse->Configuration->Multicast->Address->IPv4Address, IPV4_ADDR_STRING_LEN + 1, "%s","0.0.0.0");
   snprintf(trt__GetMetadataConfigurationResponse->Configuration->Multicast->Address->IPv4Address, IPV4_ADDR_STRING_LEN + 1, "%s",mdata_IPV4Addr);
  //  strcpy(trt__GetMetadataConfigurationResponse->Configuration->Multicast->Address->IPv4Address,"0.0.0.0");
     //Port

     trt__GetMetadataConfigurationResponse->Configuration->Multicast->Port = mdata_port;// 0;

     //TTL
     trt__GetMetadataConfigurationResponse->Configuration->Multicast->TTL = mdata_TTL;//64;

     //AutoStart
     trt__GetMetadataConfigurationResponse->Configuration->Multicast->AutoStart = mdata_autoStart;//false;
    

     trt__GetMetadataConfigurationResponse->Configuration->Multicast->__size = 0;
     trt__GetMetadataConfigurationResponse->Configuration->Multicast->__any = NULL;

     //SessionTimeout

    trt__GetMetadataConfigurationResponse->Configuration->SessionTimeout = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
    MEMSET(trt__GetMetadataConfigurationResponse->Configuration->SessionTimeout, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
    snprintf(trt__GetMetadataConfigurationResponse->Configuration->SessionTimeout, TOKEN_LENGTH, "%s",mdata_sessionTimeout);


    trt__GetMetadataConfigurationResponse->Configuration->__size = 0;
    trt__GetMetadataConfigurationResponse->Configuration->__any = NULL;

    //PTZ 
    //PTZStatus
    trt__GetMetadataConfigurationResponse->Configuration->PTZStatus = NULL;

    // Allocated memory for events tag
    trt__GetMetadataConfigurationResponse->Configuration->Events = NULL;

    //AnalyticsEngineConfiguration
    trt__GetMetadataConfigurationResponse->Configuration->AnalyticsEngineConfiguration = NULL;

    //Extension
    trt__GetMetadataConfigurationResponse->Configuration->Extension = NULL;

    //CompressionType
    trt__GetMetadataConfigurationResponse->Configuration->CompressionType = NULL;

     //GeoLocation
     trt__GetMetadataConfigurationResponse->Configuration->GeoLocation = NULL;
    // //ShapePolygon
     trt__GetMetadataConfigurationResponse->Configuration->ShapePolygon = NULL;

    // //dom_att(&trt__GetMetadataConfigurationResponse->Configuration->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    return SOAP_OK;
}
/** Web service operation '__trt__GetMetadataConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(struct soap *soap,
                                                           struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations,
                                                           struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
    printf("----------------Called GetMetadataConfigurations----------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // T_METADATA_CONFIGS l_x_metadataConfigs;
    // UINT32 l_ui32_outLenMetadataConfigs = sizeof(T_METADATA_CONFIGS);

    // /* Request for all stream info */
    // if (RCF_SUCCESS != getSysServerResp(GET_METADATA_CONFIGS, NULL, 0, &l_x_metadataConfigs, l_ui32_outLenMetadataConfigs))
    // {
    //     onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:InvalidArgVal", "ter:InternalError","Fail to Get MetaData Configurations");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to get metadata from system server");
    //     return SOAP_FAULT;
    // }
    // else
    // {
    //     RCF_PRINTF(PRIO_DEBUG, "Successfully received metadata from system server");
    // }

    trt__GetMetadataConfigurationsResponse->__sizeConfigurations = 1;

    //Configurations
    trt__GetMetadataConfigurationsResponse->Configurations =
        (struct tt__MetadataConfiguration *)soap_malloc(soap,
                                                        trt__GetMetadataConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__MetadataConfiguration));
    MEMSET(trt__GetMetadataConfigurationsResponse->Configurations, 0x00,
           trt__GetMetadataConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__MetadataConfiguration));

    for (int l_i32_idx = 0; l_i32_idx < trt__GetMetadataConfigurationsResponse->__sizeConfigurations; l_i32_idx++)
    {
        //token
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].token = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].token, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        snprintf(trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].token, TOKEN_LENGTH, "%s",mdata_confToken);


        //Name
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        snprintf(trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Name, TOKEN_LENGTH, "%s",mdata_name);


        //UseCount
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].UseCount = mdata_useCount;//1;

        //PTZStatus
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].PTZStatus = NULL;

        // events
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Events = NULL;
        // Fill events tag
          //Analytics
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Analytics = NULL;

        //Multicast
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast =
            (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));

        //Address
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->Type = tt__IPType__IPv4; //TODO: This param should be dynamic
       
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address =
            (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
        MEMSET(trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address, 0x00,
                sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
        snprintf(trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address, IPV4_ADDR_STRING_LEN + 1, "%s",mdata_IPV4Addr);
        //Port
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Port = mdata_port;//0;

        //TTL
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->TTL = mdata_TTL;//64;

        //AutoStart
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->AutoStart = mdata_autoStart;//false;

        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->__size = 0;
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->__any = NULL;
        //dom_att(&trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

       //SessionTimeout
        trt__GetMetadataConfigurationsResponse->Configurations->SessionTimeout = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetMetadataConfigurationsResponse->Configurations->SessionTimeout, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        snprintf(trt__GetMetadataConfigurationsResponse->Configurations->SessionTimeout, TOKEN_LENGTH, "%s",mdata_sessionTimeout);

        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].__size = 0;
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].__any = NULL;

        //AnalyticsEngineConfiguration
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].AnalyticsEngineConfiguration = NULL;

        //Extension
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].Extension = NULL;

        //CompressionType
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].CompressionType = NULL;

        //GeoLocation
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].GeoLocation = NULL;
        
        //ShapePolygon
        trt__GetMetadataConfigurationsResponse->Configurations[l_i32_idx].ShapePolygon = NULL;
    }
    return SOAP_OK;
}
/** Web service operation '__trt__AddMetadataConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration(struct soap *soap,
                                                          struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration,
                                                          struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
    printf("-----------Called AddMetadataConfiguration--------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if ((NULL == trt__AddMetadataConfiguration->ProfileToken) || (0 == strlen(trt__AddMetadataConfiguration->ProfileToken)))
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
       // RCF_PRINTF(PRIO_ERROR, "The requested profile token [%s] does not exist.", trt__AddMetadataConfiguration->ProfileToken);
        printf("The requested profile token [%s] does not exist.", trt__AddMetadataConfiguration->ProfileToken);
        return SOAP_FAULT;
    }

    if ((NULL == trt__AddMetadataConfiguration->ConfigurationToken) || (0 == strlen(trt__AddMetadataConfiguration->ConfigurationToken)))
    {
         onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "The AddMetadataConfiguration indicated by the ConfigurationToken[%s] does not exist.", trt__AddMetadataConfiguration->ConfigurationToken);
        printf("The AddMetadataConfiguration indicated by the ConfigurationToken[%s] does not exist.", trt__AddMetadataConfiguration->ConfigurationToken);
        return SOAP_FAULT;
    }

    testProfileAddMetaDataConfig = true;

     printf("----------testProfileAddMetaDataConfig----------%d\n",testProfileAddMetaDataConfig);
    // T_ONVIF_CONFIG_INFO l_x_onvifConfInfo;
    // MEMSET(&l_x_onvifConfInfo, 0x00, sizeof(T_ONVIF_CONFIG_INFO));
    // l_x_onvifConfInfo.m_ui16_commandID = (UINT16)ADD_METADATA_CONFIG;
    // SNPRINTF(l_x_onvifConfInfo.m_ca_tokenName, TOKEN_LENGTH, "%s", trt__AddMetadataConfiguration->ConfigurationToken);

    // UINT32 l_i32_offset = 0;
    // UINT8 l_ui8_noOfConfigs = 1;
    // UINT32 l_i32_memBytes = (TOKEN_LENGTH * sizeof(CHAR)) + sizeof(UINT8) + l_ui8_noOfConfigs * sizeof(T_ONVIF_CONFIG_INFO);
    // UINT8 *l_vp_data = (UINT8 *)soap_malloc(soap, l_i32_memBytes);
    // MEMSET(l_vp_data, 0x00, l_i32_memBytes);

    // MEMCPY((void *)(l_vp_data + l_i32_offset), trt__AddMetadataConfiguration->ProfileToken, TOKEN_LENGTH);
    // l_i32_offset += TOKEN_LENGTH;
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_ui8_noOfConfigs, sizeof(UINT8));
    // l_i32_offset += sizeof(UINT8);
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_x_onvifConfInfo, sizeof(T_ONVIF_CONFIG_INFO));

    // UINT32 l_ui32_outLen = 0;
    // if (RCF_SUCCESS != getSysServerResp(MODIFY_PROFILE, l_vp_data, l_i32_memBytes, NULL, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:Action", "ter:InternalError", "Fail Modify Profiles");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to add Metadata configuration from system server");
    //     return SOAP_FAULT;
    // }
    return SOAP_OK;
}
/** Web service operation '__trt__RemoveMetadataConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(struct soap *soap,
                                                             struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration,
                                                             struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
    printf("------------------Called RemoveMetadataConfiguration--------------------");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == trt__RemoveMetadataConfiguration)
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. trt__RemoveMetadataConfiguration[%p]", trt__RemoveMetadataConfiguration);
        printf("Invalid argument. trt__RemoveMetadataConfiguration[%p]", trt__RemoveMetadataConfiguration);
        return SOAP_FAULT;
    }

    if (NULL == trt__RemoveMetadataConfiguration->ProfileToken)
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. ProfileToken[%p]", trt__RemoveMetadataConfiguration->ProfileToken);
         printf("Invalid argument. ProfileToken[%p]", trt__RemoveMetadataConfiguration->ProfileToken);
        return SOAP_FAULT;
    }

    if (0 == strlen(trt__RemoveMetadataConfiguration->ProfileToken))
    {
        // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        // RCF_PRINTF(PRIO_ERROR, "Invalid argument. Invalid argument. ProfileToken length is zero");
        printf("Invalid argument. Invalid argument. ProfileToken length is zero");
        return SOAP_FAULT;
    }
    testProfileAddMetaDataConfig= false;
    // T_ONVIF_CONFIG_INFO l_x_onvifConfInfo;
    // MEMSET(&l_x_onvifConfInfo, 0x00, sizeof(T_ONVIF_CONFIG_INFO));
    // l_x_onvifConfInfo.m_ui16_commandID = (UINT16)REMOVE_METADATA_CONFIG;

    // UINT32 l_i32_offset = 0;
    // UINT8 l_ui8_noOfConfigs = 1;
    // UINT32 l_i32_memBytes = TOKEN_LENGTH * sizeof(CHAR) + sizeof(UINT8) + l_ui8_noOfConfigs * sizeof(T_ONVIF_CONFIG_INFO);
    // UINT8 *l_vp_data = (UINT8 *)soap_malloc(soap, l_i32_memBytes);
    // MEMSET(l_vp_data, 0x00, l_i32_memBytes);

    // MEMCPY((void *)(l_vp_data + l_i32_offset), trt__RemoveMetadataConfiguration->ProfileToken, TOKEN_LENGTH);
    // l_i32_offset += TOKEN_LENGTH;
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_ui8_noOfConfigs, sizeof(UINT8));
    // l_i32_offset += sizeof(UINT8);
    // MEMCPY((void *)(l_vp_data + l_i32_offset), &l_x_onvifConfInfo, sizeof(T_ONVIF_CONFIG_INFO));

    // UINT32 l_ui32_outLen = 0;
    // if (RCF_SUCCESS != getSysServerResp(MODIFY_PROFILE, l_vp_data, l_i32_memBytes, NULL, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:Action", "ter:InternalError", "Fail Modify Profiles");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to remove Metadata configuration from system server");
    //     return SOAP_FAULT;
    // }
    return SOAP_OK;
}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration(struct soap *soap,
                                                          struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration,
                                                          struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
    printf("---------------Called SetMetadataConfiguration------------------------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

 //   INT32 i_i32_maxEncoders = 1; //Currently 1 audio stream supported
 

    if(NULL == trt__SetMetadataConfiguration)
    {
        printf( "Invalid argument. trt__SetMetadataConfiguration is NULL");
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        return SOAP_FAULT;
    }

    if (NULL == trt__SetMetadataConfiguration->Configuration)
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        printf("Invalid argument. Configuration object pointer is NULL");
        return SOAP_FAULT;
    }

    if ((NULL == trt__SetMetadataConfiguration->Configuration->token) ||
        (0 == strlen(trt__SetMetadataConfiguration->Configuration->token)))
    {
        printf("Invalid argument. Metadata configuration token is NULL or empty");
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        return SOAP_FAULT;
    }

    if (0 == strncmp(mdata_confToken, trt__SetMetadataConfiguration->Configuration->token, TOKEN_LENGTH))
    {
        printf("-----------------Given profile token found[%s]-------------------\n", trt__SetMetadataConfiguration->Configuration->token);
    }
    else
    {
        
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The configuration does not exist.");
        printf("--------------Given Configuration token not found[%s]-------------\n", trt__SetMetadataConfiguration->Configuration->token);
        return SOAP_FAULT;
    }
    //token
    if (trt__SetMetadataConfiguration->Configuration->token)
    {
        snprintf(mdata_confToken, TOKEN_LENGTH, "%s", trt__SetMetadataConfiguration->Configuration->token);
    }
 printf("---------------Called SetMetadataConfiguration---------------11111111111111111---------------------------\n");
    //Name
    if (NULL != trt__SetMetadataConfiguration->Configuration->Name)
    {
        snprintf(mdata_name, TOKEN_LENGTH, "%s", trt__SetMetadataConfiguration->Configuration->Name);
    }
    //UseCount
    mdata_useCount = trt__SetMetadataConfiguration->Configuration->UseCount;
   
    printf("---------------Called SetMetadataConfiguration-----------------2222222222222222-------------------------\n");
    if (NULL != trt__SetMetadataConfiguration->Configuration->Multicast)
    {

        if(NULL != trt__SetMetadataConfiguration->Configuration->Multicast->Address->IPv4Address)
        {
            snprintf(mdata_IPV4Addr, IPV4_ADDR_STRING_LEN, "%s",trt__SetMetadataConfiguration->Configuration->Multicast->Address->IPv4Address);
        }

        mdata_port = trt__SetMetadataConfiguration->Configuration->Multicast->Port;
        mdata_TTL =  trt__SetMetadataConfiguration->Configuration->Multicast->TTL;
        mdata_autoStart = trt__SetMetadataConfiguration->Configuration->Multicast->AutoStart;
    }
     printf("---------------Called SetMetadataConfiguration------------------3333333333333333------------------------\n");

      printf("SessionTimeout--trt-----%s\n",trt__SetMetadataConfiguration->Configuration->SessionTimeout );

    //if(0 == strcmp("invalid",trt__SetMetadataConfiguration->Configuration->SessionTimeout))
    if(0 == strcmp(invalid_timeout,trt__SetMetadataConfiguration->Configuration->SessionTimeout))
    
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "Validation constraint violation:invalid value in element'session Timeout");
         printf("Invalid Session Timeout\n");
         return SOAP_FAULT;
    }
    else{
        printf("mdata timeout--trt-----%s\n",trt__SetMetadataConfiguration->Configuration->SessionTimeout );
        strcpy(mdata_sessionTimeout,trt__SetMetadataConfiguration->Configuration->SessionTimeout);
      // snprintf(mdata_sessionTimeout, TOKEN_LENGTH, "%s",trt__SetMetadataConfiguration->Configuration->SessionTimeout);
      //  mdata_sessionTimeout = trt__SetMetadataConfiguration->Configuration->SessionTimeout;
        printf("mdata timeout---mdata----%s\n",mdata_sessionTimeout );
    }
    printf("---------------Called SetMetadataConfiguration-------------------444444444444444-----------------------\n");
    return SOAP_OK;
}
/** Web service operation '__trt__GetCompatibleMetadataConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(struct soap *soap,
                                                                     struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations,
                                                                     struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
    int  profIndex = -1;
    
    printf("-------------------------Called GetCompatibleMetadataConfigurations-----------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if ((NULL == trt__GetCompatibleMetadataConfigurations) || (NULL == trt__GetCompatibleMetadataConfigurations->ProfileToken))
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        printf("Invalid argument. trt__GetCompatibleMetadataConfigurations or ProfileToken is NULL");
        return SOAP_FAULT;
    }

    // T_PROFILE_SETTING l_x_profs;
    // UINT32 l_ui32_outLen = sizeof(T_PROFILE_SETTING);
    // BOOL l_b_isFound = false;

    // if (RCF_SUCCESS != getSysServerResp(GET_MEDIA_PROFILE, NULL, 0, &l_x_profs, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Fail to get Profiles");
    //     RCF_PRINTF(PRIO_ERROR, "Failed to get profiles from system server");
    //     return SOAP_FAULT;
    // }

     printf("Given profile token for trt__GetCompatibleMetadataConfigurations is = %s\n", trt__GetCompatibleMetadataConfigurations->ProfileToken);

    profIndex = onvif_profile_token_to_channel(trt__GetCompatibleMetadataConfigurations->ProfileToken);
    if (profIndex < 0 && (0 == strncmp("Test_Profile", trt__GetCompatibleMetadataConfigurations->ProfileToken, TOKEN_LENGTH)) && testProfilecreated == true)
    {
            printf("Profile index match and flag is true\n");
            profIndex = 3;
    }

    
    if (-1 == profIndex)
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token does not exist.");
        //onvif_fault(soap, ONVIF_ENV_SENDER, "InvalidArgVal", "NoProfile", "The requested profile token ProfileToken does not exist.");
        printf("Given profile token not found is = %s\n", trt__GetCompatibleMetadataConfigurations->ProfileToken);
        return SOAP_FAULT;
    }

    else
    {
        printf("Given profile token found Index = %d\n", profIndex);
    }

    // if (0 == strncmp("MetaDataConfigToken_1", trt__GetCompatibleMetadataConfigurations->ProfileToken, TOKEN_LENGTH))
    // {
    //     printf("Given profile token found[%s]", trt__GetCompatibleMetadataConfigurations->ProfileToken);
    // }
    // else
    // {
        
    //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token does not exist.");
    //     printf("Given profile token not found[%s]", trt__GetCompatibleMetadataConfigurations->ProfileToken);
    //     return SOAP_FAULT;
    // }


    trt__GetCompatibleMetadataConfigurationsResponse->__sizeConfigurations = 1;

    //Configurations
    trt__GetCompatibleMetadataConfigurationsResponse->Configurations = (struct tt__MetadataConfiguration *)soap_malloc(soap,
        trt__GetCompatibleMetadataConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__MetadataConfiguration));
    MEMSET(trt__GetCompatibleMetadataConfigurationsResponse->Configurations, 0x00,
           trt__GetCompatibleMetadataConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__MetadataConfiguration));

    for (int l_i32_idx = 0; l_i32_idx < trt__GetCompatibleMetadataConfigurationsResponse->__sizeConfigurations; l_i32_idx++)
    {
        //token
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].token = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].token, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].token, TOKEN_LENGTH, ONVIF_TOKEN_FMT_METADATA, 1);


        //Name
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        ONVIF_TOKEN_GEN(trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Name, TOKEN_LENGTH, ONVIF_TOKEN_FMT_METADATA_NAME, 1);


        //UseCount
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].UseCount = 1;

        //PTZStatus
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].PTZStatus = NULL;
        //Events
        
        // No events tag at all
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Events = NULL;
 
        //Analytics
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Analytics = NULL;

        //Multicast
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast =
            (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
        MEMSET(trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));

        //Address
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
        MEMSET(trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address, 0x00, sizeof(struct tt__IPAddress));

        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->Type = tt__IPType__IPv4; //TODO: This param should be dynamic

     
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address =
            (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
        MEMSET(trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address, 0x00,
                sizeof(CHAR) * (IPV4_ADDR_STRING_LEN + 1));
        snprintf(trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Address->IPv4Address, IPV4_ADDR_STRING_LEN + 1, "%s","0.0.0.0");
        //Port
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->Port = 0;
        //TTL
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->TTL = 64;

        //AutoStart
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->AutoStart = false;

        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->__size = 0;
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->__any = NULL;
     //   //dom_att(&trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Multicast->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

        //SessionTimeout
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].SessionTimeout = timeout;

        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].__size = 0;
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].__any = NULL;

        //AnalyticsEngineConfiguration
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].AnalyticsEngineConfiguration = NULL;

        //Extension
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].Extension = NULL;

        //CompressionType
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].CompressionType = NULL;

        //GeoLocation
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].GeoLocation = NULL;
        //ShapePolygon
        trt__GetCompatibleMetadataConfigurationsResponse->Configurations[l_i32_idx].ShapePolygon = NULL;

    }
    return SOAP_OK;
}
/** Web service operation '__trt__GetMetadataConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(struct soap *soap,
                                                                 struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions,
                                                                 struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{
    printf("-----------------------------Called GetMetadataConfigurationOptions-------------------------------------------\n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // T_METADATA_CONFIGS l_x_metadataConfig;
    // UINT32 l_ui32_outLen = sizeof(T_METADATA_CONFIGS);

    // if (RCF_SUCCESS != getSysServerResp(GET_METADATA_CONFIGS, NULL, 0, &l_x_metadataConfig, l_ui32_outLen))
    // {
    //     onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Fail Get MetaData Configuration");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to get Metadata configuration from system server");
    //     return SOAP_FAULT;
    // }
    // else
    // {
    //     RCF_PRINTF(PRIO_DEBUG, "Got Metadata configuration from system server successfully");
    // }

    int profIndex = -1;
    size_t l_i32_stream =0;
    bool isStreamFound = false;

    if (NULL != trt__GetMetadataConfigurationOptions)
    {
        if (NULL != trt__GetMetadataConfigurationOptions->ProfileToken)
        {
             printf("Given profile token is [%s]", trt__GetMetadataConfigurationOptions->ProfileToken);


            profIndex = onvif_profile_token_to_channel(trt__GetMetadataConfigurationOptions->ProfileToken);

            if (-1 == profIndex)
            {
                onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist");
                printf("Given profile token not found[%s]", trt__GetMetadataConfigurationOptions->ProfileToken);
                return SOAP_FAULT;
            }
            else
            {
                printf("Given profile token found Index[%d]", profIndex);
            }
            if (NULL != trt__GetMetadataConfigurationOptions->ConfigurationToken) /* Profile token and Config token both provided */
            {
                printf("Given Metadata configuration token is [%s]", trt__GetMetadataConfigurationOptions->ConfigurationToken);
               
                if (0 == strcmp(mdata_confToken, trt__GetMetadataConfigurationOptions->ConfigurationToken))
                {
                    l_i32_stream = 0;
                    isStreamFound = true;
                }
                if (false == isStreamFound)
                {
                    onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration does not exist.");
                    printf("Given Metadata configuration not found");
                    return SOAP_FAULT;
                }
            }
            else /* Only profile token provided */
            {
                printf("Audio source configuration token not provided");
                //@TODO, match metadata token with profiletoken metadata configuration.
                // if (true == l_x_profs.m_v_profileList[l_i32_profIndex].m_b_metaDataConfigToken)
                // {
                //     if (0 == STRCMP(l_x_profs.m_v_profileList[l_i32_profIndex].m_px_metadataConfig.m_ca_configToken, l_x_profs.m_v_profileList[l_i32_profIndex].m_ca_metaDataConfigToken))
                //     {
                //         RCF_PRINTF(PRIO_DEBUG, "Given profile has Audio source configuration [%s]", l_x_profs.m_v_profileList[l_i32_profIndex].m_px_metadataConfig.m_ca_configToken);
                //         for (SIZET l_i32_idx = 0; l_i32_idx < l_x_metadataConfig.m_lx_metadataConfigList.size(); l_i32_idx++)
                //         {
                //             if (0 == STRCMP(l_x_metadataConfig.m_lx_metadataConfigList[l_i32_idx].m_ca_configToken, l_x_profs.m_v_profileList[l_i32_profIndex].m_ca_metaDataConfigToken))
                //             {
                //                 l_i32_stream = l_i32_idx;
                //                 isStreamFound = TRUE;
                //                 break;
                //             }
                //         }
                //         if (FALSE == isStreamFound)
                //         {
                //             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration does not exist.");
                //             RCF_PRINTF(PRIO_ERROR, "Given Metadata configuration not found");
                //             return SOAP_FAULT;
                //         }
                //     }
                //     else
                //     {
                //         onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration does not exist.");
                //         RCF_PRINTF(PRIO_ERROR, "This profile does not have proper Metadata configuration token");
                //         return SOAP_FAULT;
                //     }
                // }
                // else
                // {
                //     RCF_PRINTF(PRIO_DEBUG, "Meta Data configuration token not provided");
                //     l_i32_stream = 0;
                // }
            }
        }
        else
        {
            printf("---------------------Profile token not provided--------------------------\n");
            if (NULL != trt__GetMetadataConfigurationOptions->ConfigurationToken) /* Only Video source token provided */
            {
                printf("Given Metadata configuration token [%s]", trt__GetMetadataConfigurationOptions->ConfigurationToken);

                  //@TODO, match metadata token with profiletoken metadata configuration.

                // for (SIZET l_i32_idx = 0; l_i32_idx < l_x_metadataConfig.m_lx_metadataConfigList.size(); l_i32_idx++)
                // {
                //     if (0 == STRCMP(l_x_metadataConfig.m_lx_metadataConfigList[l_i32_idx].m_ca_configToken, trt__GetMetadataConfigurationOptions->ConfigurationToken))
                //     {
                //         l_i32_stream = l_i32_idx;
                //         isStreamFound = TRUE;
                //         break;
                //     }
                // }
                // if (FALSE == isStreamFound)
                // {
                //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The requested configuration does not exist.");
                //     RCF_PRINTF(PRIO_ERROR, "Given Metadata configuration not found");
                //     return SOAP_FAULT;
                // }
            }
            else /* No any token provided */
            {
                printf("Metadata configuration token not provided");
                l_i32_stream = 0;
            }
        }
    }
    else
    {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument","Invalid request received");
        printf("Invalid input argument trt__GetMetadataConfigurationOptions[%p]", trt__GetMetadataConfigurationOptions);
        return SOAP_FAULT;
    }

    printf("-----------------MetadataConfiguration Index[%d]------------------\n", l_i32_stream);

    //Options
    trt__GetMetadataConfigurationOptionsResponse->Options =
        (struct tt__MetadataConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__MetadataConfigurationOptions));
    MEMSET(trt__GetMetadataConfigurationOptionsResponse->Options, 0x00, sizeof(struct tt__MetadataConfigurationOptions));

    //PTZStatusFilterOptions
    trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions =
        (struct tt__PTZStatusFilterOptions *)soap_malloc(soap, sizeof(struct tt__PTZStatusFilterOptions));
    MEMSET(trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions, 0x00, sizeof(struct tt__PTZStatusFilterOptions));

    //PanTiltStatusSupported
    trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->PanTiltStatusSupported =  xsd__boolean__false_;

    //ZoomStatusSupported
    trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->ZoomStatusSupported =  xsd__boolean__false_;

    trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->__size = 0;
    trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->__any = NULL;

    //PanTiltPositionSupported
    trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->PanTiltPositionSupported =
        (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    MEMSET(trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->PanTiltPositionSupported, 0x00, sizeof(enum xsd__boolean));
    *(trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->PanTiltPositionSupported) =  xsd__boolean__false_;

    //ZoomPositionSupported
    trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->ZoomPositionSupported =
        (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    MEMSET(trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->ZoomPositionSupported, 0x00, sizeof(enum xsd__boolean));
    *(trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->ZoomPositionSupported) =  xsd__boolean__false_;

    //Extension
    trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->Extension = NULL;

    trt__GetMetadataConfigurationOptionsResponse->Options->__size = 0;
    trt__GetMetadataConfigurationOptionsResponse->Options->__any = NULL;

    //Extension
    trt__GetMetadataConfigurationOptionsResponse->Options->Extension = NULL;

    //GeoLocation
    trt__GetMetadataConfigurationOptionsResponse->Options->GeoLocation =
        (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    MEMSET(trt__GetMetadataConfigurationOptionsResponse->Options->GeoLocation, 0x00, sizeof(enum xsd__boolean));
    *(trt__GetMetadataConfigurationOptionsResponse->Options->GeoLocation) =  xsd__boolean__false_;
    return SOAP_OK;
}
/** Web service operation '__trt__GetAudioSourceConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(struct soap *soap,
                                                              struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations,
                                                              struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
    printf("--------------------------__trt__GetAudioSourceConfigurations---------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL, false, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // Return single audio source configuration
    trt__GetAudioSourceConfigurationsResponse->__sizeConfigurations = 1;
    trt__GetAudioSourceConfigurationsResponse->Configurations = 
        (struct tt__AudioSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__AudioSourceConfiguration));
    memset(trt__GetAudioSourceConfigurationsResponse->Configurations, 0, sizeof(struct tt__AudioSourceConfiguration));

    trt__GetAudioSourceConfigurationsResponse->Configurations[0].token = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetAudioSourceConfigurationsResponse->Configurations[0].token, "AudioSourceConfig_1");

    trt__GetAudioSourceConfigurationsResponse->Configurations[0].Name = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetAudioSourceConfigurationsResponse->Configurations[0].Name, "AudioSourceConfig");

    trt__GetAudioSourceConfigurationsResponse->Configurations[0].UseCount = 1;

    trt__GetAudioSourceConfigurationsResponse->Configurations[0].SourceToken = 
        (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    strcpy(trt__GetAudioSourceConfigurationsResponse->Configurations[0].SourceToken, "AudioSource_1");

    return SOAP_OK;
}

/** Web service operation '__trt__SetSynchronizationPoint' implementation (stub) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint(struct soap *soap,
                                                          struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint,
                                                          struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
    // This is a stub implementation that just returns success
    // SetSynchronizationPoint is used for metadata/analytics synchronization
    // Since we don't have advanced analytics, we can safely ignore this
    printf("[DEBUG] SetSynchronizationPoint called for profile token: %s\n", 
           trt__SetSynchronizationPoint->ProfileToken);
    return SOAP_OK;
}
