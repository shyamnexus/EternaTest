#include "onvif_main.h"
#include "wsaapi.h"
// #include "wsseapi-lite.h"
#include "soapH.h"
#include "soapStub.h"
// #include "wsdd.nsmap"
#include "wsddapi.h"
// TODO: Add Novatek-specific includes when implementing media2 profile
// #include <rockit.h>
//#include "rkadk_media_comm.h"
// #include "isp.h"
// #include "osd.h"
//#include "onvif.h"
// #include "web_socket.h"
// #include "video.h"
// #include "param.h"

#define MEMSET(s, c, n) (memset((void *)s, (int)c, (size_t)n))
#define LAYOUT "tt:Layout"
#define INFO_LENGTH 100
#define ANALYTIC_NAME "MotionConfiguration"
#define MOTION_REGION "MotionRegion"
typedef const char *				CCHARPTR;
typedef char *					    CHARPTR;
typedef char CHAR;
#define MAX_OSD_COUNT               3

#define SPRINTF(s, f, a...) (sprintf((char *)s, (const char *)f, ##a))
#define SNPRINTF(s,n, f, a...) (snprintf((char *)s,n, (const char *)f, ##a))
#define MEMSET(s, c, n) (memset((void *)s, (int)c, (size_t)n))



bool ConstantBitRate_chn_0 = true;
bool ConstantBitRate_chn_1 = true;
float FrameRateLimit_chn_0 = 25;
float FrameRateLimit_chn_1 = 25;
int BitrateLimit_chn_0 = 1000;
int BitrateLimit_chn_1 = 1000;
#define OSD_POSITION_COUNT              6
#define SMALL_BUFFER_LENGTH             128
#define OSD_TEXT_TYPE_COUNT             2
#define MAX_SUPPORTED_DATE_FORMAT       64
#define MAX_SUPPORTED_TIME_FORMAT       64
bool    ONLY_DATE                       = false;
bool    ONLY_TIME                       = false;
bool    DATE_TIME                       = false;
bool    IMAGE                           = false;
bool    PLAINTEXT                       = false;

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "onvif_media2.c"

/** Web service operation '__tr2__GetVideoEncoderConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetVideoEncoderConfigurations(struct soap *soap,
                                                               struct tr2__GetConfiguration *tr2__GetVideoEncoderConfigurations,
                                                               struct _tr2__GetVideoEncoderConfigurationsResponse *tr2__GetVideoEncoderConfigurationsResponse)
{

    printf("----------------inside __tr2__GetVideoEnocoderConfigurations------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

     int profIndex = -1;

    if (NULL != tr2__GetVideoEncoderConfigurations)
    {
        if (NULL != tr2__GetVideoEncoderConfigurations->ConfigurationToken)
        {
            printf("Given profile token is = %s\n", tr2__GetVideoEncoderConfigurations->ConfigurationToken);
            if(0 == strncmp("VideoEncoderToken_1", tr2__GetVideoEncoderConfigurations->ConfigurationToken, TOKEN_LENGTH))
                profIndex =0;
            else if (0 == strncmp("VideoEncoderToken_2", tr2__GetVideoEncoderConfigurations->ConfigurationToken, TOKEN_LENGTH))
                profIndex =1;
            else if (0 == strncmp("VideoEncoderToken_3", tr2__GetVideoEncoderConfigurations->ConfigurationToken, TOKEN_LENGTH))
                profIndex =2;

            
            if (-1 == profIndex)
            {
              //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
                printf("Given profile token not found is = %s\n", tr2__GetVideoEncoderConfigurations->ConfigurationToken);
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
        tr2__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = 1;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations =
        (struct tt__VideoEncoder2Configuration *)soap_malloc(soap, tr2__GetVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoder2Configuration));
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations, 0x00,
            tr2__GetVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoder2Configuration));

        //Name
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name,"MyProfile");
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name = %s\n", tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name);

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token, 0x00, sizeof(char) * TOKEN_LENGTH);

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token = tr2__GetVideoEncoderConfigurations->ConfigurationToken;
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token = %s\n", tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token);

        //Encoding
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding = (char *)soap_malloc(soap, sizeof(char) * 10);
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding, 0x00, sizeof(char) * 10);

        //tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding = videoencoder2configuration->Encoding;
        strcpy(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding,"H264");
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding = %s\n", tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding);

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Resolution = NULL;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl =
        (struct tt__VideoRateControl2 *)soap_malloc(soap, sizeof(struct tt__VideoRateControl2));
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl, 0x00,
            sizeof(struct tt__VideoRateControl2));
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->ConstantBitRate = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));

        *(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->ConstantBitRate) =  ConstantBitRate_chn_0;
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->ConstantBitRate = %d\n",*(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->ConstantBitRate));
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit = FrameRateLimit_chn_0;
       
       //strcpy(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit,"FrameRateLimit_chn_0");
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit = %f\n",tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit);
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->BitrateLimit = BitrateLimit_chn_0;
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->BitrateLimit = %d\n",tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->BitrateLimit);

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->__size = 0;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->__any = NULL;
       // tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->__anyAttribute = NULL;

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Multicast = NULL;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].GovLength = NULL;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].AnchorFrameDistance = NULL;

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].GuaranteedFrameRate = NULL;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].GuaranteedFrameRate = false;

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Profile = NULL;
    }
    else
    {
        tr2__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = 1;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations =
        (struct tt__VideoEncoder2Configuration *)soap_malloc(soap, tr2__GetVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoder2Configuration));
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations, 0x00,
            tr2__GetVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoder2Configuration));

        //Name
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name, 0x00, sizeof(char) * TOKEN_LENGTH);
        strcpy(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name,"MyProfile");
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name = %s\n", tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Name);

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token, 0x00, sizeof(char) * TOKEN_LENGTH);

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token = tr2__GetVideoEncoderConfigurations->ConfigurationToken;
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token = %s\n", tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].token);

        //Encoding
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding = (char *)soap_malloc(soap, sizeof(char) * 10);
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding, 0x00, sizeof(char) * 10);

        //tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding = videoencoder2configuration->Encoding;
        strcpy(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding,"H264");
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding = %s\n", tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding);

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Resolution = NULL;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl =
        (struct tt__VideoRateControl2 *)soap_malloc(soap, sizeof(struct tt__VideoRateControl2));
        MEMSET(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl, 0x00,
            sizeof(struct tt__VideoRateControl2));
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->ConstantBitRate = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));

        *(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->ConstantBitRate) = ConstantBitRate_chn_1;
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->ConstantBitRate = %d\n",*(tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->ConstantBitRate));
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit = FrameRateLimit_chn_1;
       
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit = %f\n",tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit);
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->BitrateLimit = BitrateLimit_chn_1;
        printf("tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->BitrateLimit = %d\n",tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->BitrateLimit);

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->__size = 0;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->__any = NULL;
      //  tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->__anyAttribute = NULL;

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Multicast = NULL;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].GovLength = NULL;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].AnchorFrameDistance = NULL;

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].GuaranteedFrameRate = NULL;
        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].GuaranteedFrameRate = false;

        tr2__GetVideoEncoderConfigurationsResponse->Configurations[0].Profile = NULL;
    }
       

    return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetVideoEncoderConfiguration(struct soap *soap,
                                                              struct _tr2__SetVideoEncoderConfiguration *tr2__SetVideoEncoderConfiguration,
                                                              struct tr2__SetConfigurationResponse *tr2__SetVideoEncoderConfigurationResponse)
{
    int32_t s32Ret;
    int profIndex = -1;

     printf("--------------------------__tr2__SetVideoEncoderConfiguration---------------------------\n");
     int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
     if (NULL == tr2__SetVideoEncoderConfiguration)
    {
        printf("--------------------------tr2__SetVideoEncoderConfiguration ---------NULL-----------------\n");
        return SOAP_FAULT;
    }

    if (NULL == tr2__SetVideoEncoderConfiguration->Configuration)
    {
       printf("--------------------------tr2__SetVideoEncoderConfiguration ---------Config NULL-----------------\n");
       return SOAP_FAULT;
    }
    if ((NULL == tr2__SetVideoEncoderConfiguration->Configuration->token) || (0 == strlen(tr2__SetVideoEncoderConfiguration->Configuration->token)))
    {
       printf("--------------------------tr2__SetVideoEncoderConfiguration ---------Config token NULL-----------------\n");
       return SOAP_FAULT;
    }
     if (NULL != tr2__SetVideoEncoderConfiguration->Configuration->token)
    {
        printf("Given profile token is = %s\n", tr2__SetVideoEncoderConfiguration->Configuration->token);
        if(0 == strncmp("VideoEncoderToken_1", tr2__SetVideoEncoderConfiguration->Configuration->token, TOKEN_LENGTH))
            profIndex =0;
        else if (0 == strncmp("VideoEncoderToken_2", tr2__SetVideoEncoderConfiguration->Configuration->token, TOKEN_LENGTH))
            profIndex =1;
        else if (0 == strncmp("VideoEncoderToken_3", tr2__SetVideoEncoderConfiguration->Configuration->token, TOKEN_LENGTH))
            profIndex =2;

        if (-1 == profIndex)
        {
            //  onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested profile token ProfileToken does not exist.");
            printf("Given profile token not found is = %s\n", tr2__SetVideoEncoderConfiguration->Configuration->token);
            return SOAP_FAULT;
        }
        else
        {
            printf("Given profile token found Index = %d\n", profIndex);
        }
    }
    
    MPP_CHN_S stViChn, stVencChn;

    stViChn.enModId = RK_ID_VI;
    stViChn.s32DevId = ctx->stViCfg[profIndex].devId;
    stViChn.s32ChnId = ctx->stViCfg[profIndex].channelId;

    stVencChn.enModId = RK_ID_VENC;
    stVencChn.s32DevId = 0;
    stVencChn.s32ChnId = ctx->stVencCfg[profIndex].u32ChnId;
    int ret;
	ret = RK_MPI_SYS_UnBind(&stViChn, &stVencChn);

    if (ret)
         printf("--------------------------Unbind VI and VENC error! ---------------------------\n");
		
	else
        printf("--------------------------Unbind VI and VENC success---------------------------\n");
    
   	VENC_CHN_ATTR_S venc_chn_attr;
	ret = RK_MPI_VENC_GetChnAttr(stVencChn.s32ChnId, &venc_chn_attr);
     if (ret)
         printf("--------------------------RK_MPI_VENC_GetChnAttr error! ---------------------------\n");
		
	else
        printf("--------------------------RK_MPI_VENC_GetChnAttr success---------------------------\n");

    printf("-----------stVencChn.s32ChnId---------- = %d\n", stVencChn.s32ChnId);	
    printf("--------- from ONVIF ConstantBitRate------%d\n",*(tr2__SetVideoEncoderConfiguration->Configuration->RateControl->ConstantBitRate));
    
    if (stViChn.s32ChnId == 0)
    {
        ConstantBitRate_chn_0 = *(tr2__SetVideoEncoderConfiguration->Configuration->RateControl->ConstantBitRate);
        printf("======================ConstantBitRate_chn_0=======================%d\n",ConstantBitRate_chn_0);
        if(ConstantBitRate_chn_0)
        {
            ctx->stVencCfg[profIndex].enRcMode = VENC_RC_MODE_H264CBR;
            venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        }
        else
        {
             ctx->stVencCfg[profIndex].enRcMode = VENC_RC_MODE_H264VBR;
             venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
        }
        FrameRateLimit_chn_0 = tr2__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
        BitrateLimit_chn_0 = tr2__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
    }
    else{
        ConstantBitRate_chn_1 = *(tr2__SetVideoEncoderConfiguration->Configuration->RateControl->ConstantBitRate);
        printf("======================ConstantBitRate_chn_1=======================%d\n",ConstantBitRate_chn_1);
        if(ConstantBitRate_chn_1)
        {
            ctx->stVencCfg[profIndex].enRcMode = VENC_RC_MODE_H264CBR;
            venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        }
        else
        {
            ctx->stVencCfg[profIndex].enRcMode = VENC_RC_MODE_H264VBR;
            venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
        }
        FrameRateLimit_chn_1 = tr2__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
        BitrateLimit_chn_1 = tr2__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
    }
      ret = RK_MPI_VENC_SetChnAttr(stVencChn.s32ChnId, &venc_chn_attr);
     if (ret)
          printf("--------------------------RK_MPI_VENC_SetChnAttr error!---------------------------\n");

    ret = RK_MPI_SYS_Bind(&stViChn, &stVencChn);
    
	if (ret)
        printf("--------------------------bind VI and VENC error!---------------------------\n");

        printf("---------ConstantBitRate_chn_0------%d\n",ctx->stVencCfg[0].enRcMode);
        printf("---------ConstantBitRate_chn_1------%d\n",ctx->stVencCfg[1].enRcMode);
        printf("---------venc_chn_attr.stRcAttr.enRcMode------%d\n",venc_chn_attr.stRcAttr.enRcMode);
        
    return SOAP_OK;
}

bool fillOSDresponse(struct soap *soap, struct tt__OSDConfiguration *OvfOSD, int index)
{

    printf("----------------inside fillOSDresponse------------------------\n");
    //Token 
    OvfOSD->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(OvfOSD->token, 0x00, sizeof(char) * TOKEN_LENGTH);
    char entry[128];
    snprintf(entry, 127, "osd.%d:token", index);
    const char *osdToken = rk_param_get_string(entry, NULL);
    printf("OSD token is %s\n", osdToken);
    snprintf(OvfOSD->token, TOKEN_LENGTH, "%s", osdToken);

    //VideoSourceConfigurationToken
    OvfOSD->VideoSourceConfigurationToken = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(OvfOSD->VideoSourceConfigurationToken, 0x00, sizeof(char) * TOKEN_LENGTH);
    snprintf(OvfOSD->VideoSourceConfigurationToken, TOKEN_LENGTH, "%s", "VideoSourceToken");


    //Type
    // OvfOSD.Type = (enum tt__OSDType *)soap_malloc(soap, sizeof(enum tt__OSDType));
    // MEMSET(OvfOSD.Type, 0x00, sizeof(enum tt__OSDType));
    OvfOSD->Type = tt__OSDType__Text;

    //position
    OvfOSD->Position = NULL;

    //TextString
    OvfOSD->TextString = NULL;
    OvfOSD->Extension = NULL;
    OvfOSD->Image = NULL;
    printf("End of fillOSDresponse\n");


    return true;

  
}

/**
 * @brief Retrieves OSD configurations based on the provided request.
 * Description:
 * This operation lists existing OSD configurations for the device.
 * If an OSD token is provided the device shall respond with the requested configuration or provide an error if it does not exist.
 * In case only a video source configuration token is provided the device shall respond with all configurations that exist for the video source configuration.
 * If no tokens are provided the device shall respond with all available OSD configurations.
 * SOAP action:
 * http://www.onvif.org/ver20/media/wsdl/GetOSDs
 *
 * @param soap The pointer to the SOAP structure.
 * @param tr2__GetOSDs The pointer to the request structure containing the configuration token or OSD token.
 * @param tr2__GetOSDsResponse The pointer to the response structure to store the retrieved OSD configurations.
 * @return The SOAP_OK if the OSD configurations are retrieved successfully, otherwise returns the SOAP_FAULT.
 */

SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetOSDs(struct soap *soap, struct _tr2__GetOSDs *tr2__GetOSDs, struct _tr2__GetOSDsResponse *tr2__GetOSDsResponse){
    printf("----------------inside __tr2__GetOSDs------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
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

    //validation
    printf("Starting validation\n");
    if(NULL == tr2__GetOSDs)
    {
        printf("Invalid request received.\n");
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "Invalid request received.");
        return retcode;
    }
    for(index = 0; index < MAX_OSD_COUNT; index++)
    {
        snprintf(entry, 127, "osd.%d:enabled", index);
        if(rk_param_get_int(entry, 0))
        {
            noOfOSDConfig++;
        }
    }
    printf("no of osd enabled are %d\n", noOfOSDConfig);

    if(noOfOSDConfig == 0)
    {
        printf("No OSD configuration found.\n");
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "No OSD configuration found.");
        tr2__GetOSDsResponse->__sizeOSDs = 0;
        tr2__GetOSDsResponse->OSDs = NULL;
        return SOAP_FAULT;
    }
    // Check for configuration token first
    if(NULL != tr2__GetOSDs->ConfigurationToken)
    {
        if(0 == strcmp(tr2__GetOSDs->ConfigurationToken, "VideoSourceToken"))
        {
            printf("Matched configuration token\n");
            isConfigToken = true;
            responseSize = noOfOSDConfig;
        }
        else
        {
            printf("Configuration token invalid.\n");
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested configuration token does not exist.");
            return retcode;
        }
        
    }
    else if(NULL != tr2__GetOSDs->OSDToken)
    {
        
        for(index = 0; index < MAX_OSD_COUNT; index++)
        {
            snprintf(entry, 127, "osd.%d:enabled", index);
            if(rk_param_get_int(entry, 0))
            {
                snprintf(entry, 127, "osd.%d:token", index);
                //const char *osdToken = rk_param_get_string(entry, NULL);
                osdToken = rk_param_get_string(entry, NULL);
                if (strcmp(tr2__GetOSDs->OSDToken, osdToken) == 0) {
                    isOSDTokenFound = true;
                    printf("OSD token found which is %s \n", osdToken);
                    responseSize = 1;
                    OSDTokenID = index;
                    printf("OSD token ID is %d which corresponds to %s\n", OSDTokenID, osdToken);
                    break;
                }
            }
        }
        if(!isOSDTokenFound)
        {
            printf("OSD token not found.\n");
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoProfile", "The requested OSD token does not exist.");
            return retcode;
        }
    }
    else
    {
        //all OSDs are requested as no token is provided
        responseSize = noOfOSDConfig;
    }

    if(isConfigToken){
        /**
         * This code block is responsible for populating the `tr2__GetOSDsResponse` structure with OSD configurations.
         * It iterates over the `responseSize` and assigns values to the corresponding fields of each OSD configuration.
         * 
         * The OSD configuration includes the OSD token, video source configuration token, OSD type, position, text string,
         * font size, font color, and background color.
         * 
         * After assigning the values, debug print statements are used to display the OSD token, video source configuration token,
         * OSD type, and the end of the configuration token.
         */
        // Set the size of the OSDs array in the response structure
        tr2__GetOSDsResponse->__sizeOSDs = responseSize;

        // Allocate memory for the OSDs array
        tr2__GetOSDsResponse->OSDs = (struct tt__OSDConfiguration *)soap_malloc(soap, responseSize * sizeof(struct tt__OSDConfiguration));

        // Initialize the allocated memory with zeros
        MEMSET(tr2__GetOSDsResponse->OSDs, 0x00, responseSize * sizeof(struct tt__OSDConfiguration));
        

        // Iterate over the responseSize and populate each OSD configuration
        for(int i = 0; i < responseSize; i++){
            // Allocate memory for the OSD token
            tr2__GetOSDsResponse->OSDs[i].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].token, 0x00, sizeof(char) * TOKEN_LENGTH);

            // Generate the OSD token using the index
            snprintf(entry, 127, "osd.%d:token", i);
            const char *osdToken = rk_param_get_string(entry, NULL);

            // Copy the generated OSD token to the response structure
            snprintf(tr2__GetOSDsResponse->OSDs[i].token, TOKEN_LENGTH, "%s", osdToken);

            // Print the OSD token for debugging
            LOG_INFO("OSD token is %s\n", tr2__GetOSDsResponse->OSDs[i].token);

            // Allocate memory for the VideoSourceConfigurationToken
            tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken = (struct tt__OSDReference *)soap_malloc(soap, sizeof(struct tt__OSDReference));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken, 0x00, sizeof(struct tt__OSDReference));

            // Allocate memory for the VideoSourceConfigurationToken->__item
            tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item, 0x00, sizeof(CHAR) * TOKEN_LENGTH);

            // Set the VideoSourceConfigurationToken value
            snprintf(tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item, TOKEN_LENGTH, "%s", "VideoSourceToken");

            // Print the VideoSourceConfigurationToken for debugging
            printf("VideoSourceConfigurationToken is %s\n", tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item);

            // Set the OSD Type
            tr2__GetOSDsResponse->OSDs[i].Type = tt__OSDType__Text;

            // Print the OSD Type for debugging
            printf("OSD Type is %d\n", tr2__GetOSDsResponse->OSDs->Type);

            // Allocate memory for the OSD Position
            tr2__GetOSDsResponse->OSDs[i].Position = (struct tt__OSDPosConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDPosConfiguration));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].Position, 0x00, sizeof(struct tt__OSDPosConfiguration));

            // Allocate memory for the OSD Position->Type
            tr2__GetOSDsResponse->OSDs[i].Position->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].Position->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);

            // Set the OSD Position->Type value
            snprintf(tr2__GetOSDsResponse->OSDs[i].Position->Type, SMALL_BUFFER_LENGTH, "%s" ,"Custom");

            // Allocate memory for the OSD Position->Pos
            tr2__GetOSDsResponse->OSDs[i].Position->Pos = (struct tt__Vector *)soap_malloc(soap, sizeof(struct tt__Vector));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].Position->Pos, 0x00, sizeof(struct tt__Vector));

            // Allocate memory for the OSD Position->Pos->x and OSD Position->Pos->y
            tr2__GetOSDsResponse->OSDs[i].Position->Pos->x = (float *)soap_malloc(soap, sizeof(float));
            tr2__GetOSDsResponse->OSDs[i].Position->Pos->y = (float *)soap_malloc(soap, sizeof(float));

            // Set the OSD Position->Pos->x and OSD Position->Pos->y values
            int pos_x = 0.0;
            int pos_y = 0.0;
            snprintf(entry, 127, "osd.%d:position_x", i);
            pos_x = rk_param_get_int(entry, 0);
            snprintf(entry, 127, "osd.%d:position_y", i);
            pos_y = rk_param_get_int(entry, 0);

            inverseRescaleCoordinates(pos_x, pos_y, tr2__GetOSDsResponse->OSDs->Position->Pos->x, tr2__GetOSDsResponse->OSDs->Position->Pos->y);




            // *(tr2__GetOSDsResponse->OSDs[i].Position->Pos->x) = pos_x;
            // *(tr2__GetOSDsResponse->OSDs[i].Position->Pos->y) = pos_y;

            // Allocate memory for the OSD TextString
            tr2__GetOSDsResponse->OSDs[i].TextString =  (struct tt__OSDTextConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDTextConfiguration)); 

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString, 0x00, sizeof(struct tt__OSDTextConfiguration));

            // Allocate memory for the OSD TextString->Type
            tr2__GetOSDsResponse->OSDs[i].TextString->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);     

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);

            // Set the OSD TextString->Type value
            if(0 == strcmp(tr2__GetOSDsResponse->OSDs[i].token, "time_01")){
                snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->Type, SMALL_BUFFER_LENGTH, "%s", "DateTime");
                tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);

                // Initialize the allocated memory with zeros
                MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);

                // Set the OSD TextString->DateFormat value
                //todo : get the date format from the config file
                snprintf(entry,127,"osd.%d:date_style", i);
                const char *dateFormat = rk_param_get_string(entry, NULL);
                omitFirstFourChars(dateFormat, dateFormatWithoutFirstFourChars);
                snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", dateFormatWithoutFirstFourChars);
                // char *date;
                // date = strtok(dateFormat, "-");
                // date = strtok(NULL, "-");

                // if(date != NULL){
                //     LOG_INFO("Date format is %s\n", date);
                //     snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", date);
                // }
                // else{
                //     LOG_INFO("Date format is NULL\n");
                // }

                // Allocate memory for the OSD TextString->TimeFormat
                tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);

                // Initialize the allocated memory with zeros
                MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);

                // Set the OSD TextString->TimeFormat value
                //todo : get the time format from the config file
                snprintf(entry,127,"osd.%d:time_style", i);
                const char *timeFormat = rk_param_get_string(entry, NULL);
                if(0 == strcmp(timeFormat, "12hour")){
                    snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[1]);
                }
                else{
                    snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[0]);
                }
            }
            else{
                snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->Type, SMALL_BUFFER_LENGTH, "%s", "Plain");
                tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat = NULL;
                tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat = NULL;
                tr2__GetOSDsResponse->OSDs[i].TextString->PlainText = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
                MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->PlainText, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
                snprintf(entry, 127, "osd.%d:display_text", i);
				const char *display_text = rk_param_get_string(entry, NULL);
                snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->PlainText, SMALL_BUFFER_LENGTH, "%s", display_text);
            }

            // Allocate memory for the OSD TextString->FontSize
            tr2__GetOSDsResponse->OSDs[i].TextString->FontSize = (int *)soap_malloc(soap, sizeof(int));

            // Set the OSD TextString->FontSize value
            int fontSize = rk_param_get_int("osd.common:font_size", -1);
            *(tr2__GetOSDsResponse->OSDs[i].TextString->FontSize) = fontSize;

            // Allocate memory for the OSD TextString->FontColor
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));

            // Allocate memory for the OSD TextString->FontColor->Color
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));

            // Set the OSD TextString->FontColor->Color values
           
            const char *fontColor = rk_param_get_string("osd.common:font_color", NULL);
            LOG_INFO("Font color is %s\n", fontColor);
            // unsigned int hexCode = (unsigned int)strtol(fontColor, NULL, 16);
            // hexToRGB(hexCode, &R, &G, &B);
            // LOG_INFO("R = %d, G = %d, B = %d\n", R, G, B);
            // RGBtoYCbCr(R, G, B, &y, &cb, &cr);
            // LOG_INFO("Y = %f, Cb = %f, Cr = %f\n", y, cb, cr);
            hexToYCbCr(fontColor, &y, &cb, &cr);
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->X = y;
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Y = cb;
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Z = cr;
            // tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Colorspace = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
            // MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Colorspace, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
            // tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Colorspace = 'http://www.onvif.org/ver10/colorspace/YCbCr';

            // Allocate memory for the OSD TextString->BackgroundColor
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor, 0x00, sizeof(struct tt__OSDColor));

            // Allocate memory for the OSD TextString->BackgroundColor->Color
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color)); 

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color, 0x00, sizeof(struct tt__Color));

            // Set the OSD TextString->BackgroundColor->Color values
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color->X = 0;
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color->Y = 0;
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color->Z = 0;

            // Print the end of the configuration token for debugging
            printf("End of Config token\n");
        }
    }

    else if(isOSDTokenFound){
        //TODO: Fill the OSD response for the OSD token
        printf("OSD token is found\n");
        tr2__GetOSDsResponse->__sizeOSDs = 1;
        tr2__GetOSDsResponse->OSDs = (struct tt__OSDConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDConfiguration));
        MEMSET(tr2__GetOSDsResponse->OSDs, 0x00, sizeof(struct tt__OSDConfiguration));
        tr2__GetOSDsResponse->OSDs->token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
        MEMSET(tr2__GetOSDsResponse->OSDs->token, 0x00, sizeof(char) * TOKEN_LENGTH);
        snprintf(tr2__GetOSDsResponse->OSDs->token, TOKEN_LENGTH, "%s", osdToken);
        tr2__GetOSDsResponse->OSDs->VideoSourceConfigurationToken = (struct tt__OSDReference *)soap_malloc(soap, sizeof(struct tt__OSDReference));
        MEMSET(tr2__GetOSDsResponse->OSDs->VideoSourceConfigurationToken, 0x00, sizeof(struct tt__OSDReference));
        tr2__GetOSDsResponse->OSDs->VideoSourceConfigurationToken->__item = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(tr2__GetOSDsResponse->OSDs->VideoSourceConfigurationToken->__item, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
        snprintf(tr2__GetOSDsResponse->OSDs->VideoSourceConfigurationToken->__item, TOKEN_LENGTH, "%s", "VideoSourceToken");
        tr2__GetOSDsResponse->OSDs->Type = tt__OSDType__Text;
        tr2__GetOSDsResponse->OSDs->Position = (struct tt__OSDPosConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDPosConfiguration));
        MEMSET(tr2__GetOSDsResponse->OSDs->Position, 0x00, sizeof(struct tt__OSDPosConfiguration));
        tr2__GetOSDsResponse->OSDs->Position->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(tr2__GetOSDsResponse->OSDs->Position->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(tr2__GetOSDsResponse->OSDs->Position->Type, SMALL_BUFFER_LENGTH, "%s" ,"Custom");
        tr2__GetOSDsResponse->OSDs->Position->Pos = (struct tt__Vector *)soap_malloc(soap, sizeof(struct tt__Vector));
        MEMSET(tr2__GetOSDsResponse->OSDs->Position->Pos, 0x00, sizeof(struct tt__Vector));
        tr2__GetOSDsResponse->OSDs->Position->Pos->x = (float *)soap_malloc(soap, sizeof(float));
        tr2__GetOSDsResponse->OSDs->Position->Pos->y = (float *)soap_malloc(soap, sizeof(float));
        float pos_x = 0.0;
        float pos_y = 0.0;
        snprintf(entry, 127, "osd.%d:position_x", OSDTokenID);
        pos_x = rk_param_get_int(entry, 0);
        snprintf(entry, 127, "osd.%d:position_y", OSDTokenID);
        pos_y = rk_param_get_int(entry, 0);
        *(tr2__GetOSDsResponse->OSDs->Position->Pos->x) = pos_x;
        *(tr2__GetOSDsResponse->OSDs->Position->Pos->y) = pos_y;
        tr2__GetOSDsResponse->OSDs->TextString =  (struct tt__OSDTextConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDTextConfiguration));
        MEMSET(tr2__GetOSDsResponse->OSDs->TextString, 0x00, sizeof(struct tt__OSDTextConfiguration));
        tr2__GetOSDsResponse->OSDs->TextString->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(tr2__GetOSDsResponse->OSDs->TextString->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        if(0 == strcmp(tr2__GetOSDsResponse->OSDs->token, "time_01")){
            snprintf(tr2__GetOSDsResponse->OSDs->TextString->Type, SMALL_BUFFER_LENGTH, "%s", "DateTime");
            tr2__GetOSDsResponse->OSDs->TextString->DateFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
            MEMSET(tr2__GetOSDsResponse->OSDs->TextString->DateFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
            //snprintf(tr2__GetOSDsResponse->OSDs->TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", "yyyy-mm-dd");
            snprintf(entry,127,"osd.%d:date_style", OSDTokenID);
            const char *dateFormat = rk_param_get_string(entry, NULL);

            // Create a buffer to store the date format without the first four characters
            // Call the function to omit the first four characters
            omitFirstFourChars(dateFormat, dateFormatWithoutFirstFourChars);
            snprintf(tr2__GetOSDsResponse->OSDs->TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", dateFormatWithoutFirstFourChars);
            printf("Date format is %s\n", tr2__GetOSDsResponse->OSDs->TextString->DateFormat);
            tr2__GetOSDsResponse->OSDs->TextString->TimeFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
            MEMSET(tr2__GetOSDsResponse->OSDs->TextString->TimeFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
            // snprintf(tr2__GetOSDsResponse->OSDs->TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", "hh:mm:ss");

            snprintf(entry,127,"osd.%d:time_style", OSDTokenID);
            const char *timeFormat = rk_param_get_string(entry, NULL);
            if(0 == strcmp(timeFormat, "12hour")){
                snprintf(tr2__GetOSDsResponse->OSDs->TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[1]);
            }
            else{
                snprintf(tr2__GetOSDsResponse->OSDs->TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[0]);                
                }
        }
        else{
            snprintf(tr2__GetOSDsResponse->OSDs->TextString->Type, SMALL_BUFFER_LENGTH, "%s", "Plain");
            tr2__GetOSDsResponse->OSDs->TextString->DateFormat = NULL;
            tr2__GetOSDsResponse->OSDs->TextString->TimeFormat = NULL;
            tr2__GetOSDsResponse->OSDs->TextString->PlainText = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
            MEMSET(tr2__GetOSDsResponse->OSDs->TextString->PlainText, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
            snprintf(entry, 127, "osd.%d:display_text", OSDTokenID);
            const char *display_text = rk_param_get_string(entry, NULL);
            snprintf(tr2__GetOSDsResponse->OSDs->TextString->PlainText, SMALL_BUFFER_LENGTH, "%s", display_text);
        }
        tr2__GetOSDsResponse->OSDs->TextString->FontSize = (int *)soap_malloc(soap, sizeof(int));
        int fontSize = rk_param_get_int("osd.common:font_size", -1);
        *(tr2__GetOSDsResponse->OSDs->TextString->FontSize) = fontSize;
        tr2__GetOSDsResponse->OSDs->TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));
        MEMSET(tr2__GetOSDsResponse->OSDs->TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));
        tr2__GetOSDsResponse->OSDs->TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));
        MEMSET(tr2__GetOSDsResponse->OSDs->TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));
        const char *fontColor = rk_param_get_string("osd.common:font_color", NULL);
        LOG_INFO("Font color is %s\n", fontColor);
        // unsigned int hexCode = (unsigned int)strtol(fontColor, NULL, 16);
        // hexToRGB(hexCode, &R, &G, &B);
        // LOG_INFO("R = %d, G = %d, B = %d\n", R, G, B);
        // RGBtoYCbCr(R, G, B, &y, &cb, &cr);
        // LOG_INFO("Y = %f, Cb = %f, Cr = %f\n", y, cb, cr);
        hexToYCbCr(fontColor, &y, &cb, &cr);
        tr2__GetOSDsResponse->OSDs->TextString->FontColor->Color->X = y;
        tr2__GetOSDsResponse->OSDs->TextString->FontColor->Color->Y = cb;
        tr2__GetOSDsResponse->OSDs->TextString->FontColor->Color->Z = cr;
        // tr2__GetOSDsResponse->OSDs->TextString->FontColor->Color->Colorspace = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
        // MEMSET(tr2__GetOSDsResponse->OSDs->TextString->FontColor->Color->Colorspace, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        // tr2__GetOSDsResponse->OSDs->TextString->FontColor->Color->Colorspace = 'http://www.onvif.org/ver10/colorspace/YCbCr';
        tr2__GetOSDsResponse->OSDs->TextString->BackgroundColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));
        MEMSET(tr2__GetOSDsResponse->OSDs->TextString->BackgroundColor, 0x00, sizeof(struct tt__OSDColor));
        tr2__GetOSDsResponse->OSDs->TextString->BackgroundColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));
        MEMSET(tr2__GetOSDsResponse->OSDs->TextString->BackgroundColor->Color, 0x00, sizeof(struct tt__Color));
        tr2__GetOSDsResponse->OSDs->TextString->BackgroundColor->Color->X = 0;
        tr2__GetOSDsResponse->OSDs->TextString->BackgroundColor->Color->Y = 0;
        tr2__GetOSDsResponse->OSDs->TextString->BackgroundColor->Color->Z = 0;

        printf("End of OSD token\n");
    }
    else{
         /**
         * This code block is responsible for populating the `tr2__GetOSDsResponse` structure with OSD configurations.
         * It iterates over the `responseSize` and assigns values to the corresponding fields of each OSD configuration.
         * 
         * The OSD configuration includes the OSD token, video source configuration token, OSD type, position, text string,
         * font size, font color, and background color.
         * 
         * After assigning the values, debug print statements are used to display the OSD token, video source configuration token,
         * OSD type, and the end of the configuration token.
         */
        // Set the size of the OSDs array in the response structure
        tr2__GetOSDsResponse->__sizeOSDs = responseSize;

        // Allocate memory for the OSDs array
        tr2__GetOSDsResponse->OSDs = (struct tt__OSDConfiguration *)soap_malloc(soap, responseSize * sizeof(struct tt__OSDConfiguration));

        // Initialize the allocated memory with zeros
        MEMSET(tr2__GetOSDsResponse->OSDs, 0x00, responseSize * sizeof(struct tt__OSDConfiguration));

        // Iterate over the responseSize and populate each OSD configuration
        for(int i = 0; i < responseSize; i++){
            // Allocate memory for the OSD token
            tr2__GetOSDsResponse->OSDs[i].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].token, 0x00, sizeof(char) * TOKEN_LENGTH);

            // Generate the OSD token using the index
            snprintf(entry, 127, "osd.%d:token", i);
            const char *osdToken = rk_param_get_string(entry, NULL);

            // Copy the generated OSD token to the response structure
            snprintf(tr2__GetOSDsResponse->OSDs[i].token, TOKEN_LENGTH, "%s", osdToken);

            // Print the OSD token for debugging
            printf("OSD token is %s\n", tr2__GetOSDsResponse->OSDs[i].token);

            // Allocate memory for the VideoSourceConfigurationToken
            tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken = (struct tt__OSDReference *)soap_malloc(soap, sizeof(struct tt__OSDReference));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken, 0x00, sizeof(struct tt__OSDReference));

            // Allocate memory for the VideoSourceConfigurationToken->__item
            tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item, 0x00, sizeof(CHAR) * TOKEN_LENGTH);

            // Set the VideoSourceConfigurationToken value
            snprintf(tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item, TOKEN_LENGTH, "%s", "VideoSourceToken");

            // Print the VideoSourceConfigurationToken for debugging
            printf("VideoSourceConfigurationToken is %s\n", tr2__GetOSDsResponse->OSDs[i].VideoSourceConfigurationToken->__item);

            // Set the OSD Type
            tr2__GetOSDsResponse->OSDs[i].Type = tt__OSDType__Text;

            // Print the OSD Type for debugging
            printf("OSD Type is %d\n", tr2__GetOSDsResponse->OSDs->Type);

            // Allocate memory for the OSD Position
            tr2__GetOSDsResponse->OSDs[i].Position = (struct tt__OSDPosConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDPosConfiguration));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].Position, 0x00, sizeof(struct tt__OSDPosConfiguration));

            // Allocate memory for the OSD Position->Type
            tr2__GetOSDsResponse->OSDs[i].Position->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].Position->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);

            // Set the OSD Position->Type value
            snprintf(tr2__GetOSDsResponse->OSDs[i].Position->Type, SMALL_BUFFER_LENGTH, "%s" ,"Custom");

            // Allocate memory for the OSD Position->Pos
            tr2__GetOSDsResponse->OSDs[i].Position->Pos = (struct tt__Vector *)soap_malloc(soap, sizeof(struct tt__Vector));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].Position->Pos, 0x00, sizeof(struct tt__Vector));

            // Allocate memory for the OSD Position->Pos->x and OSD Position->Pos->y
            tr2__GetOSDsResponse->OSDs[i].Position->Pos->x = (float *)soap_malloc(soap, sizeof(float));
            tr2__GetOSDsResponse->OSDs[i].Position->Pos->y = (float *)soap_malloc(soap, sizeof(float));

            // Set the OSD Position->Pos->x and OSD Position->Pos->y values
            float pos_x = 0.0;
            float pos_y = 0.0;
            snprintf(entry, 127, "osd.%d:position_x", i);
            pos_x = rk_param_get_int(entry, 0);
            snprintf(entry, 127, "osd.%d:position_y", i);
            pos_y = rk_param_get_int(entry, 0);

            *(tr2__GetOSDsResponse->OSDs[i].Position->Pos->x) = pos_x;
            *(tr2__GetOSDsResponse->OSDs[i].Position->Pos->y) = pos_y;

            // Allocate memory for the OSD TextString
            tr2__GetOSDsResponse->OSDs[i].TextString =  (struct tt__OSDTextConfiguration *)soap_malloc(soap, sizeof(struct tt__OSDTextConfiguration)); 

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString, 0x00, sizeof(struct tt__OSDTextConfiguration));

            // Allocate memory for the OSD TextString->Type
            tr2__GetOSDsResponse->OSDs[i].TextString->Type = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);     

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->Type, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);

            // Set the OSD TextString->Type value
            if(0 == strcmp(tr2__GetOSDsResponse->OSDs[i].token, "time_01")){

                snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->Type, SMALL_BUFFER_LENGTH, "%s", "DateTime");
                tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);

                // Initialize the allocated memory with zeros
                MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);

                // Set the OSD TextString->DateFormat value
                //todo : get the date format from the config file

                snprintf(entry,127,"osd.%d:date_style", i);
                const char *dateFormat = rk_param_get_string(entry, NULL);
                // char *date;
                // date = strtok(dateFormat, "-");
                // date = strtok(NULL, "-");
                omitFirstFourChars(dateFormat, dateFormatWithoutFirstFourChars);
                snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", dateFormatWithoutFirstFourChars);
                // if(date != NULL){
                //     LOG_INFO("Date format is %s\n", date);
                //     snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", date);
                // }
                // else{
                //     LOG_INFO("Date format is NULL\n");
                //     snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", "yyyy-mm-dd");
                // }
                //snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat, SMALL_BUFFER_LENGTH, "%s", "yyyy-mm-dd");

                // Allocate memory for the OSD TextString->TimeFormat
                tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);

                // Initialize the allocated memory with zeros
                MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);

                // Set the OSD TextString->TimeFormat value
                //todo : get the time format from the config file
                snprintf(entry,127,"osd.%d:time_style", i);
                const char *timeFormat = rk_param_get_string(entry, NULL);
                if(0 == strcmp(timeFormat, "12hour")){
                    snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[1]);
                }
                else{
                    snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat, SMALL_BUFFER_LENGTH, "%s", osdTimeFormat[0]);
                }
            }
            else{
                snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->Type, SMALL_BUFFER_LENGTH, "%s", "Plain");
                tr2__GetOSDsResponse->OSDs[i].TextString->DateFormat = NULL;
                tr2__GetOSDsResponse->OSDs[i].TextString->TimeFormat = NULL;
                tr2__GetOSDsResponse->OSDs[i].TextString->PlainText = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
                MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->PlainText, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
                snprintf(entry, 127, "osd.%d:display_text", i);
				const char *display_text = rk_param_get_string(entry, NULL);
                snprintf(tr2__GetOSDsResponse->OSDs[i].TextString->PlainText, SMALL_BUFFER_LENGTH, "%s", display_text);
            }

            // Allocate memory for the OSD TextString->FontSize
            tr2__GetOSDsResponse->OSDs[i].TextString->FontSize = (int *)soap_malloc(soap, sizeof(int));

            // Set the OSD TextString->FontSize value
            int fontSize = rk_param_get_int("osd.common:font_size", -1);
            *(tr2__GetOSDsResponse->OSDs[i].TextString->FontSize) = fontSize;

            // Allocate memory for the OSD TextString->FontColor
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->FontColor, 0x00, sizeof(struct tt__OSDColor));

            // Allocate memory for the OSD TextString->FontColor->Color
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color, 0x00, sizeof(struct tt__Color));

            // Set the OSD TextString->FontColor->Color values
            const char *fontColor = rk_param_get_string("osd.common:font_color", NULL);
            LOG_INFO("Font color is %s\n", fontColor);
            // unsigned int hexCode = (unsigned int)strtol(fontColor, NULL, 16);
            // hexToRGB(hexCode, &R, &G, &B);
            // LOG_INFO("R = %d, G = %d, B = %d\n", R, G, B);
            // RGBtoYCbCr(R, G, B, &y, &cb, &cr);
            // LOG_INFO("Y = %f, Cb = %f, Cr = %f\n", y, cb, cr);
            hexToYCbCr(fontColor, &y, &cb, &cr);
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->X = y;
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Y = cb;
            tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Z = cr;
            // tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Colorspace = (char *)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
            // MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Colorspace, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
            // tr2__GetOSDsResponse->OSDs[i].TextString->FontColor->Color->Colorspace = 'http://www.onvif.org/ver10/colorspace/YCbCr';

            // Allocate memory for the OSD TextString->BackgroundColor
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor = (struct tt__OSDColor *)soap_malloc(soap, sizeof(struct tt__OSDColor));

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor, 0x00, sizeof(struct tt__OSDColor));

            // Allocate memory for the OSD TextString->BackgroundColor->Color
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color = (struct tt__Color *)soap_malloc(soap, sizeof(struct tt__Color)); 

            // Initialize the allocated memory with zeros
            MEMSET(tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color, 0x00, sizeof(struct tt__Color));

            // Set the OSD TextString->BackgroundColor->Color values
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color->X = 0;
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color->Y = 0;
            tr2__GetOSDsResponse->OSDs[i].TextString->BackgroundColor->Color->Z = 0;

            // Print the end of the configuration token for debugging
            printf("End of Config token\n");
        }

    }

    return SOAP_OK;
}

/**
 * Web service operation '__tr2__GetOSDOptions' implementation, which returns the available OSD configuration options.
 *
 * @param soap The SOAP context.
 * @param tr2__GetOSDOptions The input parameters for the GetOSDOptions operation.
 * @param tr2__GetOSDOptionsResponse The output parameters for the GetOSDOptions operation.
 * @return SOAP_OK on success, or a SOAP fault code on error.
 */


SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetOSDOptions(struct soap *soap, struct _tr2__GetOSDOptions *tr2__GetOSDOptions, struct _tr2__GetOSDOptionsResponse *tr2__GetOSDOptionsResponse)
{
    LOG_INFO("----------------------------Inside GetOSDOptions----------------------------");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    //const char *configuration_token = "12345";

    int index = 0;
    int retCode = SOAP_FAULT;
    if (NULL == tr2__GetOSDOptions)
    {
        LOG_ERROR("Invalid request received.");
        return retCode;
    }

    if (NULL == tr2__GetOSDOptions->ConfigurationToken || (0 == strlen(tr2__GetOSDOptions->ConfigurationToken)))
    {
        LOG_ERROR("ConfigToken is NULL");
        return retCode;
    }

    if (0 != strcmp(tr2__GetOSDOptions->ConfigurationToken, "VideoSourceToken"))
    {
        LOG_ERROR("Invalid OSD Token received.");
        return retCode;
    }

    // OSDOptions
    tr2__GetOSDOptionsResponse->OSDOptions = (struct tt__OSDConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__OSDConfigurationOptions));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions, 0x00, sizeof(struct tt__OSDConfigurationOptions));

    // MaximumNumberOfOSDs
    tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs = (struct tt__MaximumNumberOfOSDs *)soap_malloc(soap, sizeof(struct tt__MaximumNumberOfOSDs));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs, 0x00, sizeof(struct tt__MaximumNumberOfOSDs));

    // Total
    tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Total = 3;

    // PlainText
    tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->PlainText = (int *)soap_malloc(soap, sizeof(int));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->PlainText, 0x00, sizeof(int));
    *(tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->PlainText) = 2;

    // DateAndTime
    tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->DateAndTime = (int *)soap_malloc(soap, sizeof(int));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->DateAndTime, 0x00, sizeof(int));
    *(tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->DateAndTime) = 1;

    tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Date = NULL;
    tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Image = NULL;
    tr2__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Time = NULL;

    // SizeType
    tr2__GetOSDOptionsResponse->OSDOptions->__sizeType = 1;

    // Type
    tr2__GetOSDOptionsResponse->OSDOptions->Type = (enum tt__OSDType *)soap_malloc(soap, sizeof(enum tt__OSDType));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->Type, 0x00, sizeof(enum tt__OSDType));
    *(tr2__GetOSDOptionsResponse->OSDOptions->Type) = tt__OSDType__Text;
    // LOG_INFO("---------Type is set to Text------------\n");

    // tr2__GetOSDOptionsResponse->OSDOptions->Type = NULL;

    // SizePositionOption
    tr2__GetOSDOptionsResponse->OSDOptions->__sizePositionOption = 6;
    tr2__GetOSDOptionsResponse->OSDOptions->PositionOption = (char **)soap_malloc(soap,
                                                                                  (sizeof(char *) * tr2__GetOSDOptionsResponse->OSDOptions->__sizePositionOption));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->PositionOption, 0x00, (sizeof(char *) * tr2__GetOSDOptionsResponse->OSDOptions->__sizePositionOption));
    //char *osdPositions[] = {"UpperRight", "UpperLeft", "LowerRight", "LowerLeft", "TopCenter", "Custom"};
    for (index = 0; index < OSD_POSITION_COUNT; index++)
    {
        // PositionOption
        tr2__GetOSDOptionsResponse->OSDOptions->PositionOption[index] = (char *)malloc(sizeof(char) * SMALL_BUFFER_LENGTH);
        MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->PositionOption[index], 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
        snprintf(tr2__GetOSDOptionsResponse->OSDOptions->PositionOption[index], SMALL_BUFFER_LENGTH, "%s", osdPositions[index]);
    }

    // TextOption
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption = (struct tt__OSDTextOptions *)soap_malloc(soap, sizeof(struct tt__OSDTextOptions));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->TextOption, 0x00, sizeof(struct tt__OSDTextOptions));

    // SizeType
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeType = 2;
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->Type = (char **)soap_malloc(soap,
                                                                                    (sizeof(char *) * tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeType));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->Type, 0x00, (sizeof(char *) * tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeType));

    // Type
    index = 0;
    

    for (index = 0; index < OSD_TEXT_TYPE_COUNT; index++)
    {
        // Allocate memory for storing OSD text type
        tr2__GetOSDOptionsResponse->OSDOptions->TextOption->Type[index] = (char *)soap_malloc(soap, (sizeof(char) * SMALL_BUFFER_LENGTH));
        // Initialize memory with null bytes
        MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->Type[index], 0x00, (sizeof(char) * SMALL_BUFFER_LENGTH));
        // Copy OSD text type into allocated memory
        snprintf(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->Type[index], SMALL_BUFFER_LENGTH, "%s", osdTextTypes[index]);
    }

    // SizeDateFormat
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeDateFormat = 5;
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat = (char **)soap_malloc(soap,
                                                                                          (sizeof(char *) * tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeDateFormat));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat, 0x00,
           (sizeof(char *) * tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeDateFormat));

    // DateFormat

    
    index = 0;
    for (index = 0; index < 6; index++)
    {
        tr2__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat[index] = (char *)soap_malloc(soap, (sizeof(char) * SMALL_BUFFER_LENGTH));
        MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat[index], 0x00, (sizeof(char) * SMALL_BUFFER_LENGTH));

        snprintf(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->DateFormat[index], SMALL_BUFFER_LENGTH, "%s", osdDateFormat[index]);
    }
    // SizeTimeFormat
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeTimeFormat = 2;
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat = (char **)soap_malloc(soap,
                                                                                          (sizeof(char *) * tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeTimeFormat));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat, 0x00,
           (sizeof(char *) * tr2__GetOSDOptionsResponse->OSDOptions->TextOption->__sizeTimeFormat));

    // TimeFormat

    
    index = 0;
    for (index = 0; index < 2; index++)
    {
        tr2__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat[index] = (char *)soap_malloc(soap, (sizeof(char) * MAX_SUPPORTED_TIME_FORMAT));
        MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat[index], 0x00, (sizeof(char) * MAX_SUPPORTED_TIME_FORMAT));
        snprintf(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->TimeFormat[index], MAX_SUPPORTED_TIME_FORMAT, "%s", osdTimeFormat[index]);
    }

    // FontSizeRange
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->FontSizeRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    MEMSET(tr2__GetOSDOptionsResponse->OSDOptions->TextOption->FontSizeRange, 0x00, sizeof(struct tt__IntRange));
    // MinFontSize
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->FontSizeRange->Min = 12;
    // MaxFontSize
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->FontSizeRange->Max = 48;

    // FontColor
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->FontColor = NULL;
    // BackgroundColor
    tr2__GetOSDOptionsResponse->OSDOptions->TextOption->BackgroundColor = NULL;

    // Extension
    tr2__GetOSDOptionsResponse->OSDOptions->Extension = NULL;
    // dom_att(&tr2__GetOSDOptionsResponse->OSDOptions->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    retCode = SOAP_OK;
    return retCode;
}

/** Web service operation '__tr2__SetOSD' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetOSD(struct soap *soap, struct _tr2__SetOSD *tr2__SetOSD, struct tr2__SetConfigurationResponse *tr2__SetOSDResponse)
{
    LOG_INFO("=========Called __tr2__SetOSD=========");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int retCode = SOAP_FAULT;
    int enabled = 0;
    bool m_b_isconfig = false;
    const char *osdToken;
    int position_x;
    int position_y;
    char *dateformat;
    char *timeformat;
    int index = 0;

    //LOG_INFO("Positio X is %f\n", *(tr2__SetOSD->OSD->Position->Pos->x));
    //LOG_INFO("Positio Y is %f\n", *(tr2__SetOSD->OSD->Position->Pos->y));

    do
    {
        if (NULL == tr2__SetOSD)
        {
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
            LOG_ERROR("Invalid request received.");
            retCode = SOAP_FAULT;
            break;
        }

        if (NULL == tr2__SetOSD->OSD)
        {
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
            LOG_ERROR("Invalid request received.");
            retCode = SOAP_FAULT;
            break;
        }

        if (NULL != tr2__SetOSD->OSD)
        {
            if (NULL == tr2__SetOSD->OSD->token || (0 == strlen(tr2__SetOSD->OSD->token)))
            {
                onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                LOG_ERROR("OSD token null");
                retCode = SOAP_FAULT;
                break;
            }
        }
        //Compare OSD Token
        
        if (NULL != tr2__SetOSD->OSD->token)
        {
            for(int i =0; i < MAX_OSD_COUNT; i++){
                char entry[127];    
                snprintf(entry, 127, "osd.%d:token", i);    
                osdToken = rk_param_get_string(entry, NULL);
                if (0 == strcmp(osdToken, tr2__SetOSD->OSD->token))
                {
                    m_b_isconfig = true;
                    break;
                    
                }
            }
        }

        if(NULL == tr2__SetOSD->OSD->VideoSourceConfigurationToken->__item){
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
            LOG_ERROR("VideoSourceConfigurationToken is NULL");
            retCode = SOAP_FAULT;
            break;
        }

        if (false == m_b_isconfig)
        {
            onvif_fault(soap, ONVIF_ENV_SENDER, " ter:InvalidArgVal", "ter:NoConfig", "The requested configuration indicated with OSDToken does not exist.");
            LOG_ERROR( "The requested configuration indicated with OSDToken does not exist.");
            retCode = SOAP_FAULT;
            break;
        }

        rk_osd_get_enable(&enabled);
        if (false == enabled)
        {
            onvif_fault(soap, ONVIF_ENV_SENDER, " ter:InvalidArgVal", "ter:NoConfig", "The requested OSD does not exist.");
            //RCF_PRINTF(PRIO_ERROR, "The requested OSD does not exist.");
            LOG_ERROR("The requested OSD does not exist.");
            retCode = SOAP_FAULT;
            break;
        }

        if (NULL != tr2__SetOSD->OSD)
        {
            //Token
            if (NULL != tr2__SetOSD->OSD->token)
            {
                LOG_INFO("Token - %s", tr2__SetOSD->OSD->token);
            }

            //VideoSourceConfigurationToken
            // if (NULL != tr2__SetOSD->OSD->VideoSourceConfigurationToken)

            //Position
            if (NULL != tr2__SetOSD->OSD->Position)
            {
                if (NULL != tr2__SetOSD->OSD->Position->Type)
                {
                    LOG_INFO("Type - %s", tr2__SetOSD->OSD->Position->Type);
                   
                    //TODO: Convert Position Type
                    if(!strcmp(tr2__SetOSD->OSD->Position->Type ,"UpperRight")){
                        LOG_INFO("--------------------UpperRight-----------------\n");
                        position_x = 1700;
                        position_y = 16;
                    }
                    if(!strcmp(tr2__SetOSD->OSD->Position->Type,"UpperLeft")){
                        LOG_INFO("--------------------UpperLeft-----------------\n");
                        position_x = 16;
                        position_y = 16;
                    }
                    if(!strcmp(tr2__SetOSD->OSD->Position->Type, "LowerRight")){
                        LOG_INFO("--------------------LowerRight-----------------\n");
                        position_x = 1700;
                        position_y = 1000;
                    }
                    if(!strcmp(tr2__SetOSD->OSD->Position->Type, "TopCenter")){
                        LOG_INFO("--------------------TopCenter-----------------\n");
                        position_x = 930;
                        position_y = 16;
                    }
                    if(!strcmp(tr2__SetOSD->OSD->Position->Type, "BottomCenter")){
                        LOG_INFO("--------------------BottomCenter-----------------\n");
                        position_x = 930;
                        position_y = 1000;
                    }
                    if(!strcmp(tr2__SetOSD->OSD->Position->Type, "LowerLeft")){
                        LOG_INFO("--------------------LowerLeft-----------------\n");
                        position_x = 16;
                        position_y = 1000;
                    }

                    if(!strcmp(tr2__SetOSD->OSD->Position->Type, "Custom")){
                        LOG_INFO("--------------------Custom-----------------\n");
                        rescaleCoordinates(*(tr2__SetOSD->OSD->Position->Pos->x), *(tr2__SetOSD->OSD->Position->Pos->y), &position_x, &position_y);
                        // position_x = *(tr2__SetOSD->OSD->Position->Pos->x);
                        // position_y = *(tr2__SetOSD->OSD->Position->Pos->y);
                    }
                    // else{
                    //     LOG_INFO("Position mismatch!!\n");
                    //     LOG_INFO("tr2__SetOSD->OSD->Position->Type is %s", tr2__SetOSD->OSD->Position->Type);
                    //     break;
                    // }
                }
            }

            //TextString
            
            if (NULL != tr2__SetOSD->OSD->TextString)
            {
                //Type
                if (NULL != tr2__SetOSD->OSD->TextString->Type)
                {
                    LOG_INFO("Type - %s", tr2__SetOSD->OSD->TextString->Type);
                    if(strcmp(tr2__SetOSD->OSD->TextString->Type,"Date")==0){
                        LOG_INFO("--------------------Date-----------------\n");
                        ONLY_DATE = true;
                     
                    }
                    else if(strcmp(tr2__SetOSD->OSD->TextString->Type,"Time")==0){
                        LOG_INFO("--------------------Time-----------------\n");
                        ONLY_TIME = true;
                       

                    }
                    else if(strcmp(tr2__SetOSD->OSD->TextString->Type,"DateAndTime")==0){
                        LOG_INFO("--------------------DateAndTime-----------------\n");
                        DATE_TIME = true;
                       

                    }
                    else if(strcmp(tr2__SetOSD->OSD->TextString->Type,"Plain")==0){
                        LOG_INFO("--------------------PlainText-----------------\n");
                        PLAINTEXT = true;
                        LOG_INFO("PLAINTEXT - %s", tr2__SetOSD->OSD->TextString->PlainText);
                    
                    }
                    else{
                        LOG_INFO("Type mismatch!!\n");
                        LOG_INFO("tr2__SetOSD->OSD->TextString->Type is %s", tr2__SetOSD->OSD->TextString->Type);
                        break;
                    }
                }

                //DateFormat
                if (NULL != tr2__SetOSD->OSD->TextString->DateFormat)
                {

                    //TODO: define the osdDateFormat in includes to use it without multiple definition
                    LOG_INFO("-----checking date format------");
                    LOG_INFO("DateFormat - %s", tr2__SetOSD->OSD->TextString->DateFormat);
                    index = 0;
                    for(index = 0; index < 6; index ++){
                        if(strcmp(tr2__SetOSD->OSD->TextString->DateFormat ,osdDateFormat[index])==0){
                            dateformat = tr2__SetOSD->OSD->TextString->DateFormat;
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
            
                        return SOAP_FAULT;

                    }
                }
                //TimeFormat
                if (NULL != tr2__SetOSD->OSD->TextString->TimeFormat)
                {
                    LOG_INFO("TimeFormat - %s", tr2__SetOSD->OSD->TextString->TimeFormat);
                    //char *osdTimeFormat[] = {"hh:mm:ss", "hh:mm:ss tt"};
                    if(strcmp(tr2__SetOSD->OSD->TextString->TimeFormat, osdTimeFormat[0])==0){
                        timeformat = "24hour";
                    }
                    if(strcmp(tr2__SetOSD->OSD->TextString->TimeFormat, osdTimeFormat[1])==0){
                        timeformat = "12hour";
                    }

                }
                //FontSize
                if (NULL != tr2__SetOSD->OSD->TextString->FontSize)
                {
                    int ret = 0;
                    int current_font_size;
                    LOG_INFO("FontSize - %d", *(tr2__SetOSD->OSD->TextString->FontSize));
                    ret = rk_osd_get_font_size(&current_font_size);
                    if (ret!= 0){
                        LOG_ERROR("Failed to get current font size");
                        return -1;
                    }
                    if(current_font_size!= *(tr2__SetOSD->OSD->TextString->FontSize)){
                        LOG_INFO("Current font size is %d, new font size is %d", current_font_size, *(tr2__SetOSD->OSD->TextString->FontSize));
                        ret = rk_osd_set_font_size(*(tr2__SetOSD->OSD->TextString->FontSize));
                        if (ret!= 0){
                            LOG_ERROR("Failed to set font size");
                            return -1;
                        }
                    
                    }
                }
                //TODO: set the font color(identify the color space)
                if (NULL != tr2__SetOSD->OSD->TextString->FontColor)
                {
                    //Color
                    if (NULL != tr2__SetOSD->OSD->TextString->FontColor->Color)
                    {
                        LOG_INFO("Color - %d", tr2__SetOSD->OSD->TextString->FontColor->Color->X);
                        LOG_INFO("Color - %d", tr2__SetOSD->OSD->TextString->FontColor->Color->Y);
                        LOG_INFO("Color - %d", tr2__SetOSD->OSD->TextString->FontColor->Color->Z);
                    }
                }

                //PlainText
                if (NULL != tr2__SetOSD->OSD->TextString->PlainText)
                {
                    LOG_INFO("PlainText - %s", tr2__SetOSD->OSD->TextString->PlainText);

                }

                if (NULL != tr2__SetOSD->OSD->TextString->IsPersistentText)
                {
                    LOG_INFO("IsPersistentText - %d", tr2__SetOSD->OSD->TextString->IsPersistentText);
                    

                }
            }
            //SetOSDSettings

            if(ONLY_DATE == true){
                rk_osd_set_enabled(OSD_TIME_ID, 1);
                rk_osd_set_position_x(OSD_TIME_ID, position_x);
                rk_osd_set_position_y(OSD_TIME_ID, position_y);
                rk_osd_set_date_style(OSD_TIME_ID, dateformat);
                ONLY_DATE = false;
            }
            if(ONLY_TIME == true){
                rk_osd_set_enabled(OSD_TIME_ID, 1);
                rk_osd_set_position_x(OSD_TIME_ID, position_x);
                rk_osd_set_position_y(OSD_TIME_ID, position_y);
                rk_osd_set_date_style(OSD_TIME_ID, "CHR");
                rk_osd_set_time_style(OSD_TIME_ID, timeformat);
                ONLY_TIME = false;
            }
            if(DATE_TIME == true){
                char prefix[] = "CHR-";
                strcat(prefix, dateformat);
                rk_osd_set_enabled(OSD_TIME_ID, 1);
                LOG_INFO("prefix - %s", prefix);
                rk_osd_set_position_x(OSD_TIME_ID, position_x);
                rk_osd_set_position_y(OSD_TIME_ID, position_y);
                rk_osd_set_time_style(OSD_TIME_ID, timeformat);
                rk_osd_set_date_style(OSD_TIME_ID, prefix);
                DATE_TIME = false;
            }
            if(PLAINTEXT == true){
                int ret = 0;
                if(!strcmp(osdToken, "title_01")){
                    LOG_INFO("Title OSD");
                    rk_osd_set_enabled(OSD_CHANNEL_ID, 1);
                    rk_osd_set_position_x(OSD_CHANNEL_ID, position_x);
                    rk_osd_set_position_y(OSD_CHANNEL_ID, position_y);
                    ret = rk_osd_set_display_text(OSD_CHANNEL_ID, tr2__SetOSD->OSD->TextString->PlainText);
                    if(ret != 0 ){
                        LOG_ERROR("Failed to set display text");
                        return -1;
                    }
                    PLAINTEXT = false;
                }
                else{
                    LOG_INFO("PLAINTEXT - %s", tr2__SetOSD->OSD->TextString->PlainText);
                    rk_osd_set_enabled(OSD_CHARACTER_ID, 1);
                    rk_osd_set_position_x(OSD_CHARACTER_ID, position_x);
                    rk_osd_set_position_y(OSD_CHARACTER_ID, position_y);
                    ret = rk_osd_set_display_text(OSD_CHARACTER_ID, tr2__SetOSD->OSD->TextString->PlainText);
                    if(ret != 0 ){
                    LOG_ERROR("Failed to set display text");
                    return -1;
                    }
                PLAINTEXT = false;
                }


            }
            if(IMAGE == true){
                rk_osd_set_enabled(OSD_IMAGE_ID, 1);
                rk_osd_set_position_x(OSD_IMAGE_ID, position_x);
                rk_osd_set_position_y(OSD_IMAGE_ID, position_y);
                IMAGE = false;

            }
            rkipc_osd_restart();

        }
        retCode = SOAP_OK;
    }
    while (false);
    return retCode;
    
}



/** Web service operation '__tr2__CreateOSD' implementation, should return SOAP_OK or error code */

SOAP_FMAC5 int SOAP_FMAC6 __tr2__CreateOSD(struct soap *soap, struct _tr2__CreateOSD *tr2__CreateOSD, struct _tr2__CreateOSDResponse *tr2__CreateOSDResponse)
{
    LOG_INFO("=========Called CreateOSD=========");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);
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
    const char *OSDIsPersistentText;
    int OSDPosX;
    int OSDPosY;
    int OSDFontSize;
    bool CharID1 = false;
    bool CharID2 = false;
    bool DateTimeID = false;

    if (NULL == tr2__CreateOSD)
    {
        LOG_ERROR("Invalid request received.");
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
        return retCode;
    }

    if (NULL != tr2__CreateOSD){
        if(NULL == tr2__CreateOSD->OSD->token || (0 == strlen(tr2__CreateOSD->OSD->token))){
            LOG_ERROR("OSD is NULL.");
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "OSD is NULL.");
            return retCode;
        }
    }
    if (NULL != tr2__CreateOSD->OSD){
        if (NULL != tr2__CreateOSD->OSD->token){
            LOG_INFO("Token - %s", tr2__CreateOSD->OSD->token);

        }
        if (NULL != tr2__CreateOSD->OSD->VideoSourceConfigurationToken)
        {
                LOG_INFO("VideoSourceConfigurationToken - %s", tr2__CreateOSD->OSD->VideoSourceConfigurationToken->__item);
                if(0 != strcmp(tr2__CreateOSD->OSD->VideoSourceConfigurationToken->__item, "VideoSourceToken")){
                    LOG_ERROR("Invalid VideoSourceConfigurationToken received.");
                    onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Configuration token error");
                    return SOAP_FAULT;
                }
        }

        //type
        //LOG_INFO("Type - %s", tr2__CreateOSD->OSD->Type);


        //Position
        if (NULL != tr2__CreateOSD->OSD->Position)
        {
            if (NULL != tr2__CreateOSD->OSD->Position->Type)
            {
                LOG_INFO("Type - %s", tr2__CreateOSD->OSD->Position->Type);
                if(0 == strcmp(tr2__CreateOSD->OSD->Position->Type, "UpperRight")){
                    //LOG_INFO("--------------------UpperRight-----------------\n");
                    OSDPosition = "UpperRight";
                    OSDPosX = 1700;
                    OSDPosY = 16;
                }
                else if(0 == strcmp(tr2__CreateOSD->OSD->Position->Type, "UpperLeft")){
                    //LOG_INFO("--------------------UpperLeft-----------------\n");
                    OSDPosition = "UpperLeft";
                    OSDPosX = 16;
                    OSDPosY = 16;
                }
                else if(0 == strcmp(tr2__CreateOSD->OSD->Position->Type, "LowerRight")){
                    //LOG_INFO("--------------------LowerRight-----------------\n");
                    OSDPosition = "LowerRight";
                    OSDPosX = 1700;
                    OSDPosY = 1000;
                }
                else if(0 == strcmp(tr2__CreateOSD->OSD->Position->Type, "TopCenter")){
                    //LOG_INFO("--------------------TopCenter-----------------\n");
                    OSDPosition = "TopCenter";
                    OSDPosX = 930;
                    OSDPosY = 16;

                }
                else if(0 == strcmp(tr2__CreateOSD->OSD->Position->Type, "BottomCenter")){
                    //LOG_INFO("--------------------BottomCenter-----------------\n");
                    OSDPosition = "BottomCenter";
                    OSDPosX = 930;
                    OSDPosY = 1000;

                }
                else if(0 == strcmp(tr2__CreateOSD->OSD->Position->Type, "LowerLeft")){
                    //LOG_INFO("--------------------LowerLeft-----------------\n");
                    OSDPosition = "LowerLeft";
                    OSDPosX = 16;
                    OSDPosY = 1000;
                }
                else if(0 == strcmp(tr2__CreateOSD->OSD->Position->Type, "Custom")){
                    //LOG_INFO("----------------Custom-----------------\n");
                    OSDPosition = "Custom";
                    if(NULL != tr2__CreateOSD->OSD->Position->Pos){
                        OSDPosX = *(tr2__CreateOSD->OSD->Position->Pos->x);
                        OSDPosY = *(tr2__CreateOSD->OSD->Position->Pos->y);
                    }
                    else{
                        LOG_ERROR("Invalid position received");
                        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                        return SOAP_FAULT;
                    }
                }
                else{
                    LOG_INFO("Type mismatch!!\n");
                    LOG_INFO("tr2__CreateOSD->OSD->Position->Type is %s", tr2__CreateOSD->OSD->Position->Type);
                    onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                    return SOAP_FAULT;
                }

            }
        }

        //TextString
        if (NULL != tr2__CreateOSD->OSD->TextString)
            {
                //Type
                if (NULL != tr2__CreateOSD->OSD->TextString->Type)
                {
                    LOG_INFO("Type - %s", tr2__CreateOSD->OSD->TextString->Type);
                    if(0 == strcmp(tr2__CreateOSD->OSD->TextString->Type, "DateAndTime")){
                        LOG_INFO("--------------------DateAndTime-----------------\n");
                        OSDText = "DateAndTime";
                    }
                    else if(0 == strcmp(tr2__CreateOSD->OSD->TextString->Type, "Plain")){
                        LOG_INFO("--------------------PlainText-----------------\n");
                        OSDText = "Plain";
                    }
                    else{
                        LOG_INFO("Type mismatch!!\n");
                        LOG_INFO("tr2__CreateOSD->OSD->TextString->Type is %s", tr2__CreateOSD->OSD->TextString->Type);
                        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid request received");
                    }
                }

                //DateFormat
                if (NULL != tr2__CreateOSD->OSD->TextString->DateFormat)
                {
                    LOG_INFO("DateFormat - %s", tr2__CreateOSD->OSD->TextString->DateFormat);
                    for(int i = 0; i < 6; i++){
                        if(0 == strcmp(tr2__CreateOSD->OSD->TextString->DateFormat, osdDateFormat[i])){
                            OSDDateFormat = tr2__CreateOSD->OSD->TextString->DateFormat;
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
                if (NULL != tr2__CreateOSD->OSD->TextString->TimeFormat)
                {
                    LOG_INFO("TimeFormat - %s", tr2__CreateOSD->OSD->TextString->TimeFormat);
                    for(int i = 0; i < 2; i++){
                        if(0 == strcmp(tr2__CreateOSD->OSD->TextString->TimeFormat, osdTimeFormat[i])){
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
                if (NULL != tr2__CreateOSD->OSD->TextString->FontSize)
                {
                    LOG_INFO("FontSize - %d", *(tr2__CreateOSD->OSD->TextString->FontSize));
                    OSDFontSize = *(tr2__CreateOSD->OSD->TextString->FontSize);
                }
                else
                {
                    LOG_INFO("Default FontSize will be used");
                    OSDFontSize = OSD_DEFAULT_FONT_SIZE; // Default font size will be used.
                }

                //PlainText
                if (NULL != tr2__CreateOSD->OSD->TextString->PlainText)
                {
                    LOG_INFO("PlainText - %s", tr2__CreateOSD->OSD->TextString->PlainText);
                    OSDPlainText = tr2__CreateOSD->OSD->TextString->PlainText;
                    
                }

                //IsPersistentText
                if (NULL != tr2__CreateOSD->OSD->TextString->IsPersistentText)
                {
                    LOG_INFO("IsPersistentText - %d", tr2__CreateOSD->OSD->TextString->IsPersistentText);
                    OSDIsPersistentText = tr2__CreateOSD->OSD->TextString->IsPersistentText;
                }
            }
        // Create osd

        rk_osd_set_font_size(OSDFontSize);
        if(0 == strcmp(OSDText, "DateAndTime")){
            char entry[128];
            snprintf(entry, 127, "osd.%d:enabled", OSD_TIME_ID);
            if(rk_param_get_int(entry, 0) == 1){
                LOG_INFO("OSD time ID already exists");
                onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "OSD time ID already exists");
                return SOAP_FAULT;
            }
            LOG_INFO("Creating DateAndTime OSD");
            rk_osd_set_enabled(OSD_TIME_ID, 1);
            rk_osd_set_position_x(OSD_TIME_ID, OSDPosX);
            rk_osd_set_position_y(OSD_TIME_ID, OSDPosY);
            char prefix[] = "CHR-";
            strcat(prefix, OSDDateFormat);
            rk_osd_set_date_style(OSD_TIME_ID, prefix);
            rk_osd_set_time_style(OSD_TIME_ID, OSDTimeFormat);
            
        }
        else if(0 == strcmp(OSDText, "Plain")){
            char entry[128];
            snprintf(entry, 127, "osd.%d:enabled", OSD_CHARACTER_ID);
            if(rk_param_get_int(entry, 0) == 1){
                LOG_INFO("OSD charcter ID 1 already exists");
                CharID1 = true;

            }
            snprintf(entry, 127, "osd.%d:enabled", OSD_CHANNEL_ID);
            if(rk_param_get_int(entry, 0) == 1){
                LOG_INFO("OSD charcter ID 2 already exists");
                CharID2 = true;
            }
            if(CharID1 == true && CharID2 == true){
                LOG_INFO("Both OSD character IDs are already in use");
                onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Both OSD character IDs are already in use");
                return SOAP_FAULT;
            }
            if(CharID1 == true){
                LOG_INFO("Creating PlainText OSD");
                rk_osd_set_enabled(OSD_CHANNEL_ID, 1);
                rk_osd_set_position_x(OSD_CHANNEL_ID, OSDPosX);
                rk_osd_set_position_y(OSD_CHANNEL_ID, OSDPosY);
                rk_osd_set_display_text(OSD_CHANNEL_ID, OSDPlainText);
                OSD_TOKEN = "title_01";

            }
            else{
                LOG_INFO("Creating PlainText OSD");
                rk_osd_set_enabled(OSD_CHARACTER_ID, 1);
                rk_osd_set_position_x(OSD_CHARACTER_ID, OSDPosX);
                rk_osd_set_position_y(OSD_CHARACTER_ID, OSDPosY);
                rk_osd_set_display_text(OSD_CHARACTER_ID, OSDPlainText);
                OSD_TOKEN = "location_01";
            }
        }
        else{
            LOG_INFO("Invalid OSD text type");
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidArgument", "Invalid OSD text type");
            return SOAP_FAULT;
        }

        tr2__CreateOSDResponse->OSDToken = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
        MEMSET(tr2__CreateOSDResponse->OSDToken, 0x00, sizeof(CHAR) * TOKEN_LENGTH);

        if (0 == strcmp("Plain", tr2__CreateOSD->OSD->TextString->Type))
        {
            snprintf(tr2__CreateOSDResponse->OSDToken, TOKEN_LENGTH, "%s",OSD_TOKEN);
        }
        else
        {
            snprintf(tr2__CreateOSDResponse->OSDToken, TOKEN_LENGTH, "%s", "time_01");
        }
        LOG_INFO("OSD created successfully.");
        rkipc_osd_restart();

    }
    return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__DeleteOSD(struct soap *soap, struct _tr2__DeleteOSD *tr2__DeleteOSD, struct tr2__SetConfigurationResponse *tr2__DeleteOSDResponse)
{
    LOG_INFO("----------------Inside __tr2__DeleteOSD--------------------");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    char entry[128];
    const char *osdToken = NULL;
    int OSD_ID = -1;
    bool tokenFound = false;

    if(NULL == tr2__DeleteOSD){
        LOG_ERROR("Invalid request received.");
        return SOAP_FAULT;
    }
    if(NULL == tr2__DeleteOSD->OSDToken){
        LOG_ERROR("OSD Token is NULL.");
        return SOAP_FAULT;
    }

    for(int i = 0; i<3; i++){
        snprintf(entry, 127, "osd.%d:token", i);
        osdToken = rk_param_get_string(entry, NULL);
        if(0 == strcmp(osdToken, tr2__DeleteOSD->OSDToken)){
            LOG_INFO("OSD Token found.");
            OSD_ID = i;
            tokenFound = true;
            break;
        }
    }
    if(!tokenFound){
        LOG_INFO("The requested OSD does not exist.");
        return SOAP_FAULT;
    }
    snprintf(entry, 127, "osd.%d:enabled", OSD_ID);
    int enabled = rk_param_get_int(entry, 0);
    if(0 == enabled){
        LOG_INFO("The requested OSD does not exist.");
        return SOAP_FAULT;
    }
    else{
        snprintf(entry, 127, "osd.%d:enabled", OSD_ID);
        rk_param_set_int(entry, 0);
        LOG_INFO("OSD deleted successfully.");
        rkipc_osd_restart();
    }

    

    return SOAP_OK;
}



SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetProfiles(struct soap *soap,
                                             struct _tr2__GetProfiles *tr2__GetProfiles,
                                             struct _tr2__GetProfilesResponse *tr2__GetProfilesResponse)
{
    printf("\n================__tr2__GetProfiles in Media 2 gets called==========\n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    printf("tr2__GetProfiles->__sizeType[%d]\n", tr2__GetProfiles->__sizeType);
    for (int i = 0; i < tr2__GetProfiles->__sizeType; i++)
    {
        printf("tr2__GetProfiles->Type[%d][%s]\n", i, tr2__GetProfiles->Type[i]);
        
    }



    // Return 1 profile for simplicity
    tr2__GetProfilesResponse->__sizeProfiles = 1;

    tr2__GetProfilesResponse->Profiles = (struct tr2__MediaProfile *)soap_malloc(soap, sizeof(struct tr2__MediaProfile) * (tr2__GetProfilesResponse->__sizeProfiles));
    MEMSET(tr2__GetProfilesResponse->Profiles, 0x00, sizeof(struct tr2__MediaProfile) * (tr2__GetProfilesResponse->__sizeProfiles));

    // Profile 0
    tr2__GetProfilesResponse->Profiles[0].token = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(tr2__GetProfilesResponse->Profiles[0].token, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(tr2__GetProfilesResponse->Profiles[0].token, "Profile_1");
    
    tr2__GetProfilesResponse->Profiles[0].fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tr2__GetProfilesResponse->Profiles[0].fixed) = xsd__boolean__false_;

    tr2__GetProfilesResponse->Profiles[0].Name = (char *)soap_malloc(soap, sizeof(char) * TOKEN_LENGTH);
    MEMSET(tr2__GetProfilesResponse->Profiles[0].Name, 0x00, sizeof(char) * TOKEN_LENGTH);
    strcpy(tr2__GetProfilesResponse->Profiles[0].Name, "MainProfile");

    tr2__GetProfilesResponse->Profiles[0].Configurations = (struct tr2__ConfigurationSet *)soap_malloc(soap, sizeof(struct tr2__ConfigurationSet));
    MEMSET(tr2__GetProfilesResponse->Profiles[0].Configurations, 0x00, sizeof(struct tr2__ConfigurationSet));

    // Set all optional configurations to NULL (minimal implementation)
    tr2__GetProfilesResponse->Profiles[0].Configurations->VideoSource = NULL;
    tr2__GetProfilesResponse->Profiles[0].Configurations->AudioSource = NULL;
    tr2__GetProfilesResponse->Profiles[0].Configurations->VideoEncoder = NULL;
    tr2__GetProfilesResponse->Profiles[0].Configurations->AudioEncoder = NULL;
    tr2__GetProfilesResponse->Profiles[0].Configurations->Analytics = NULL;
    tr2__GetProfilesResponse->Profiles[0].Configurations->PTZ = NULL;
    tr2__GetProfilesResponse->Profiles[0].Configurations->Metadata = NULL;
    tr2__GetProfilesResponse->Profiles[0].Configurations->AudioOutput = NULL;
    tr2__GetProfilesResponse->Profiles[0].Configurations->AudioDecoder = NULL;

    printf("__tr2__GetProfiles completed successfully with 1 profile\n");
    return SOAP_OK;
}

// /** Web service operation '__tr2__GetAnalyticsConfigurations' implementation, should return SOAP_OK or error code */

SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAnalyticsConfigurations(struct soap *soap, struct tr2__GetConfiguration *tr2__GetAnalyticsConfigurations, struct _tr2__GetAnalyticsConfigurationsResponse *tr2__GetAnalyticsConfigurationsResponse)
{
    printf( "------------------------__tr2__Called GetAnalyticsConfigurations----------------- ");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // if(tr2__GetAnalyticsConfigurations->ConfigurationToken != ANALYTIC_NAME)
    // {
	//     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The GetAnalyticsConfigurations indicated by the ConfigurationToken does not exist.");
	//     RCF_PRINTF(PRIO_ERROR, "The GetRules indicated by the ConfigurationToken[%s] does not exist.", tr2__GetAnalyticsConfigurations->ConfigurationToken);
	//     return SOAP_FAULT;
    // }

    tr2__GetAnalyticsConfigurationsResponse->__sizeConfigurations = 1;

    //Configurations
    tr2__GetAnalyticsConfigurationsResponse->Configurations = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap,
                    tr2__GetAnalyticsConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoAnalyticsConfiguration));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations, 0x00, tr2__GetAnalyticsConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoAnalyticsConfiguration));

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].token = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*MAX_TOKEN_SIZE);
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].token, 0x00, sizeof(CHAR)*MAX_TOKEN_SIZE);
    SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].token, MAX_TOKEN_SIZE, "%s", "12345");

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*MAX_TOKEN_SIZE);
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].Name, 0x00, sizeof(CHAR)*MAX_TOKEN_SIZE);
    SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].Name, MAX_TOKEN_SIZE, "%s", "VIDEO_ANALYTICS_CONFG");

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].UseCount = 1;

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].__size = 1;
    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration, 0x00, sizeof(struct tt__AnalyticsEngineConfiguration));

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->__sizeAnalyticsModule = 1;

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, sizeof(struct tt__Config));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule, 0x00, sizeof(struct tt__Config));

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Name = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*TOKEN_LENGTH);
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Name, 0x00, sizeof(CHAR)*TOKEN_LENGTH);
    SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Name, TOKEN_LENGTH, "%s", "ObjectDetectionModule");
    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Type = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*TOKEN_LENGTH);
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Type, 0x00, sizeof(CHAR)*TOKEN_LENGTH);
    SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Type, TOKEN_LENGTH, "%s", "tt:ObjectDetectionEngine");

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters = (struct tt__ItemList *)soap_malloc(soap, sizeof(struct tt__ItemList));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters, 0x00, sizeof(struct tt__ItemList));

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem = 1;

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem *)soap_malloc(soap, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem*sizeof(struct _tt__ItemList_SimpleItem));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem, 0x00, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem*sizeof(struct _tt__ItemList_SimpleItem));

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Name = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Name, 0x00, sizeof(CHAR)*INFO_LENGTH);
    SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Name, INFO_LENGTH, "%s", "SimpleItem1");

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Value = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Value, 0x00, sizeof(CHAR)*INFO_LENGTH);
    SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Value, INFO_LENGTH, "%s", "SimpleItem1Value");

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem = 0;
    // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem = (struct _tt__ItemList_ElementItem *)soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem));
    // MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem, 0x00, sizeof(struct _tt__ItemList_ElementItem));

    // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->Name = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
    // MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->Name, 0x00, sizeof(CHAR)*INFO_LENGTH);
    // SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->Name, INFO_LENGTH, "%s", "ElementItem1");

    // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->__any = NULL;

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->Extension = NULL;
    // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__anyAttribute = NULL;

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->Extension = NULL;
    // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->__anyAttribute = NULL;

    // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration = NULL;
    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration = (struct tt__RuleEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration, 0x00, sizeof(struct tt__RuleEngineConfiguration));

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->__sizeRule = 1;
    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule = (struct tt__Config* )soap_malloc(soap,tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule, 0x00, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
	SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Name ,INFO_LENGTH,"%s","ObjectDetectionRule");


    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Type , 0x00, sizeof(CHAR) * INFO_LENGTH);
	SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Type ,INFO_LENGTH,"%s","tt:ObjectDetector");

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters  , 0x00, sizeof(struct tt__ItemList));


    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->__sizeSimpleItem = 1;
    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->SimpleItem = (struct  _tt__ItemList_SimpleItem *) soap_malloc(soap, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->SimpleItem, 0x00, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->SimpleItem->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->SimpleItem->Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
    SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->SimpleItem->Name, TOKEN_LENGTH,"%s","SimpleItem1");

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->SimpleItem->Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
    MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->SimpleItem->Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
    SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->SimpleItem->Value, TOKEN_LENGTH,"%s","SimpleItem1Value");
    

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->__sizeElementItem = 0;
    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule[0].Parameters->Extension = NULL;
    




    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Extension = NULL;
    // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->__anyAttribute = NULL;

    tr2__GetAnalyticsConfigurationsResponse->Configurations[0].__any = NULL;
    // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].__anyAttribute = NULL;

    printf( "------------------------__tr2__Called GetAnalyticsConfigurations Over----------------- ");

    return SOAP_OK;
}



/** Web service operation '__tr2__GetAnalyticsConfigurations' implementation, should return SOAP_OK or error code */

// SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAnalyticsConfigurations(struct soap *soap, struct tr2__GetConfiguration *tr2__GetAnalyticsConfigurations, struct _tr2__GetAnalyticsConfigurationsResponse *tr2__GetAnalyticsConfigurationsResponse)
// {
//     printf( "------------------------__tr2__Called GetAnalyticsConfigurations----------------- ");


//     // if(tr2__GetAnalyticsConfigurations->ConfigurationToken != ANALYTIC_NAME)
//     // {
// 	//     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:NoConfig", "The GetAnalyticsConfigurations indicated by the ConfigurationToken does not exist.");
// 	//     RCF_PRINTF(PRIO_ERROR, "The GetRules indicated by the ConfigurationToken[%s] does not exist.", tr2__GetAnalyticsConfigurations->ConfigurationToken);
// 	//     return SOAP_FAULT;
//     // }

//     tr2__GetAnalyticsConfigurationsResponse->__sizeConfigurations = 1;

//     //Configurations
//     tr2__GetAnalyticsConfigurationsResponse->Configurations = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap,
//                     tr2__GetAnalyticsConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoAnalyticsConfiguration));
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations, 0x00, tr2__GetAnalyticsConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoAnalyticsConfiguration));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].token = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*MAX_TOKEN_SIZE);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].token, 0x00, sizeof(CHAR)*MAX_TOKEN_SIZE);
//     SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].token, MAX_TOKEN_SIZE, "%s", "12345");

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*MAX_TOKEN_SIZE);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].Name, 0x00, sizeof(CHAR)*MAX_TOKEN_SIZE);
//     SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].Name, MAX_TOKEN_SIZE, "%s", "VIDEO_ANALYTICS_CONFG");

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].UseCount = 1;

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].__size = 1;
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration, 0x00, sizeof(struct tt__AnalyticsEngineConfiguration));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->__sizeAnalyticsModule = 1;

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, sizeof(struct tt__Config));
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule, 0x00, sizeof(struct tt__Config));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Name = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*TOKEN_LENGTH);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Name, 0x00, sizeof(CHAR)*TOKEN_LENGTH);
//     SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Name, TOKEN_LENGTH, "%s", "MyCellMotionModule");
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Type = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*TOKEN_LENGTH);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Type, 0x00, sizeof(CHAR)*TOKEN_LENGTH);
//     SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Type, TOKEN_LENGTH, "%s", "tt:CellMotionEngine");

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters = (struct tt__ItemList *)soap_malloc(soap, sizeof(struct tt__ItemList));
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters, 0x00, sizeof(struct tt__ItemList));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem = 1;

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem *)soap_malloc(soap, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem*sizeof(struct _tt__ItemList_SimpleItem));
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem, 0x00, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem*sizeof(struct _tt__ItemList_SimpleItem));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Name = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Name, 0x00, sizeof(CHAR)*INFO_LENGTH);
//     SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Name, INFO_LENGTH, "%s", "Sensitivity");

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Value = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Value, 0x00, sizeof(CHAR)*INFO_LENGTH);
//     SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Value, INFO_LENGTH, "%s", "100");

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem = 1;
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem = (struct _tt__ItemList_ElementItem *)soap_malloc(soap, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem*sizeof(struct _tt__ItemList_ElementItem));
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem, 0x00, tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem*sizeof(struct _tt__ItemList_ElementItem));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].Name = (CHARPTR) soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].Name, 0x00, sizeof(CHAR)*INFO_LENGTH);
//     SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].Name, INFO_LENGTH, "%s", "Layout");









// 	tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.name ,INFO_LENGTH,"%s","tt:CellLayout");

// 	soap_att_text(soap_att(&tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any, NULL, "Columns"), "22");
// 	soap_att_text(soap_att(&tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any, NULL, "Rows"), "18");

// 	tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
// 	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts, 0x00, sizeof(struct soap_dom_element));

// 	tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->name ,INFO_LENGTH,"%s","tt:Transformation");

// 	tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
// 	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts, 0x00, sizeof(struct soap_dom_element));

// 	tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->name ,INFO_LENGTH,"%s","tt:Translate");

// 	soap_att_text(soap_att(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts, NULL, "x"), "-1.000");
// 	soap_att_text(soap_att(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts, NULL, "y"), "1.000");


// 	tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->next = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
// 	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->next, 0x00, sizeof(struct soap_dom_element));


// 	tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->next->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->next->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->next->name ,INFO_LENGTH,"%s","tt:Scale");

// 	soap_att_text(soap_att(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->next, NULL, "x"), "0.0909");
// 	soap_att_text(soap_att(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem[0].__any.elts->elts->next, NULL, "y"), "-0.111");



//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->Extension = NULL;
//     // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__anyAttribute = NULL;

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->Extension = NULL;
//     // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->__anyAttribute = NULL;

//     // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration = NULL;
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration = (struct tt__RuleEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration, 0x00, sizeof(struct tt__RuleEngineConfiguration));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->__sizeRule = 0;
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule = (struct tt__Config* )soap_malloc(soap,tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->__sizeRule*sizeof(struct tt__Config));
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule, 0x00, sizeof(struct tt__Config));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Name = NULL;
//     // MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Name, 0x00, sizeof(CHAR)*TOKEN_LENGTH);

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Type = NULL;
//     // MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Type, 0x00, sizeof(CHAR)*TOKEN_LENGTH);
    
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters = (struct tt__ItemList*) soap_malloc(soap, sizeof(struct tt__ItemList)*TOKEN_LENGTH);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters, 0x00, sizeof(struct tt__ItemList));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->__sizeElementItem = 0;
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->ElementItem = (struct _tt__ItemList_ElementItem*) soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem)*TOKEN_LENGTH);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->ElementItem, 0x00, sizeof(struct _tt__ItemList_ElementItem));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->ElementItem->Name = NULL;
//     // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->ElementItem->__any = NULL;


//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->Extension = NULL;

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->__sizeSimpleItem = 0;
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem*) soap_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem)*TOKEN_LENGTH);
//     MEMSET(tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->SimpleItem, 0x00, sizeof(struct _tt__ItemList_SimpleItem));

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->SimpleItem->Name = NULL;
//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->SimpleItem->Value = NULL;

//     // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Rule->Parameters->__anyAttribute = NULL;
    

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->Extension = NULL;
//     // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].RuleEngineConfiguration->__anyAttribute = NULL;

//     tr2__GetAnalyticsConfigurationsResponse->Configurations[0].__any = NULL;
//     // tr2__GetAnalyticsConfigurationsResponse->Configurations[0].__anyAttribute = NULL;

//     printf( "------------------------__tr2__Called GetAnalyticsConfigurations Over----------------- ");

//     return SOAP_OK;
// }





