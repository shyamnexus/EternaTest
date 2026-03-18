#include "soapH.h"
#include "soapStub.h"
#include "onvif_main.h"
#include "wsaapi.h"
#include "onvif_imaging.h"
// #include "wsdd.nsmap"
#include "wsddapi.h"
#include "ipcam/image_adjustment_wrapper.h"

#ifndef ONVIF_TEST

typedef const char *				CCHARPTR;
typedef char *					    CHARPTR;
typedef char CHAR;

#define TOKEN_LENGTH 64

// Novatek ISP range: 0-200, ONVIF range: 1-100
// Conversion macros
#define ONVIF_TO_NOVATEK(val) ((int)(((val) - 1) * 200.0 / 99.0))
#define NOVATEK_TO_ONVIF(val) ((int)(((val) * 99.0 / 200.0) + 1))




/** Web service operation '__timg__GetServiceCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetServiceCapabilities(struct soap* soap, struct _timg__GetServiceCapabilities *timg__GetServiceCapabilities, struct _timg__GetServiceCapabilitiesResponse *timg__GetServiceCapabilitiesResponse){

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    timg__GetServiceCapabilitiesResponse->Capabilities = (struct timg__Capabilities *)soap_malloc(soap, sizeof(struct timg__Capabilities));
    MEMSET(timg__GetServiceCapabilitiesResponse->Capabilities, 0x00, sizeof(struct timg__Capabilities));

    /** Required element 'trt:ProfileCapabilities' of XML schema type 'trt:ProfileCapabilities' */
    timg__GetServiceCapabilitiesResponse->Capabilities->ImageStabilization = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *timg__GetServiceCapabilitiesResponse->Capabilities->ImageStabilization = xsd__boolean__false_;

    timg__GetServiceCapabilitiesResponse->Capabilities->Presets = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *timg__GetServiceCapabilitiesResponse->Capabilities->Presets = xsd__boolean__false_;

    timg__GetServiceCapabilitiesResponse->Capabilities->AdaptablePreset = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *timg__GetServiceCapabilitiesResponse->Capabilities->AdaptablePreset = xsd__boolean__false_;

    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    timg__GetServiceCapabilitiesResponse->Capabilities->__size = 0;
    timg__GetServiceCapabilitiesResponse->Capabilities->__any = NULL;
    //dom_att(&timg__GetServiceCapabilitiesResponse->Capabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    return SOAP_OK;
}

/** Web service operation '__timg__GetImagingSettings' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetImagingSettings(struct soap* soap, struct _timg__GetImagingSettings *timg__GetImagingSettings, struct _timg__GetImagingSettingsResponse *timg__GetImagingSettingsResponse)
{
    printf("-------------------------__timg__GetImagingSettings------------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    if ((NULL == timg__GetImagingSettings) || (NULL == timg__GetImagingSettings->VideoSourceToken)){
        LOG_ERROR("Invalid argument. timg__GetImagingSettings or VideoSourceToken is NULL");
        return SOAP_FAULT;
    }
    if (0 != strcmp("VideoSourceToken", timg__GetImagingSettings->VideoSourceToken))
    {
        LOG_ERROR("Invalid argument. VideoSourceToken is not a valid token. Try again!!!!!!!! ");
        return SOAP_FAULT;     
    }
    LOG_INFO("VideoSourceToken is %s and is a valid token", timg__GetImagingSettings->VideoSourceToken);
    int cam_id = 0;
    LOG_DEBUG("Fetching Imaging Settings\n");
    
    // Get image adjustment parameters from Novatek ISP with default values
    int brightness = 100;  // Default to middle value (50% on 0-200 scale)
    int contrast = 100;
    int saturation = 100;
    int sharpness = 100;
    int wb_mode = 0;  // 0=auto, 1=manual
    int wb_r_gain = 128, wb_g_gain = 128, wb_b_gain = 128;
    BOOL wdr_enabled = FALSE;
    int wdr_strength = 50;
    
    // Try to get values from ISP, but continue with defaults if it fails
    HD_RESULT ret;
    if ((ret = get_camera_brightness(cam_id, &brightness)) != HD_OK) {
        LOG_DEBUG("Failed to get brightness, using default\n");
        brightness = 100;
    }
    if ((ret = get_camera_contrast(cam_id, &contrast)) != HD_OK) {
        LOG_DEBUG("Failed to get contrast, using default\n");
        contrast = 100;
    }
    if ((ret = get_camera_saturation(cam_id, &saturation)) != HD_OK) {
        LOG_DEBUG("Failed to get saturation, using default\n");
        saturation = 100;
    }
    if ((ret = get_camera_sharpness(cam_id, &sharpness)) != HD_OK) {
        LOG_DEBUG("Failed to get sharpness, using default\n");
        sharpness = 100;
    }
    get_camera_wb_mode(cam_id, &wb_mode);  // Optional, ignore errors
    get_camera_manual_wb_gain(cam_id, &wb_r_gain, &wb_g_gain, &wb_b_gain);  // Optional
    get_wdr_enable(cam_id, &wdr_enabled);  // Optional
    get_manual_wdr_strength(cam_id, &wdr_strength);  // Optional
    
    LOG_DEBUG("Got imaging settings - Brightness:%d, Contrast:%d, Saturation:%d, Sharpness:%d\n",
              brightness, contrast, saturation, sharpness);


    timg__GetImagingSettingsResponse->ImagingSettings = (struct tt__ImagingSettings20 *)soap_malloc(soap, sizeof(struct tt__ImagingSettings20));
    MEMSET(timg__GetImagingSettingsResponse->ImagingSettings, 0x00, sizeof(struct tt__ImagingSettings20));
 
    /** Optional element 'tt:BacklightCompensation' of XML schema type 'tt:BacklightCompensation20' */
    timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation = (struct tt__BacklightCompensation20 *)soap_malloc(soap,
        sizeof(struct tt__BacklightCompensation20));
    MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation, 0x00, sizeof(struct tt__BacklightCompensation20));
  
    // Using WDR as backlight compensation equivalent
    if(wdr_enabled){
        timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Mode = tt__BacklightCompensationMode__ON;
    }
    else{
        timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
    }
 
    timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Level = (float *)soap_malloc(soap, sizeof(float));
    *timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Level = wdr_strength;
    timg__GetImagingSettingsResponse->ImagingSettings->Brightness = (float *)soap_malloc(soap, sizeof(float));
    *timg__GetImagingSettingsResponse->ImagingSettings->Brightness = NOVATEK_TO_ONVIF(brightness);
    timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = (float *)soap_malloc(soap, sizeof(float));
    *timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = NOVATEK_TO_ONVIF(saturation);
    timg__GetImagingSettingsResponse->ImagingSettings->Contrast = (float *)soap_malloc(soap, sizeof(float));
    *timg__GetImagingSettingsResponse->ImagingSettings->Contrast = NOVATEK_TO_ONVIF(contrast);
    timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = (float *)soap_malloc(soap, sizeof(float));
    *timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = NOVATEK_TO_ONVIF(sharpness);
 
    /** Optional element 'tt:Exposure' of XML schema type 'tt:Exposure20' */
    // timg__GetImagingSettingsResponse->ImagingSettings->Exposure =
    //     (struct tt__Exposure20 *)soap_malloc(soap, sizeof(struct tt__Exposure20));
    // MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->Exposure, 0x00, sizeof(struct tt__Exposure20));
    /** Required element 'tt:Mode' of XML schema type 'tt:ExposureMode' */
    //timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Mode = tt__ExposureMode__MANUAL;
    timg__GetImagingSettingsResponse->ImagingSettings->Exposure = NULL;
    /** Optional element 'tt:MinExposureTime' of XML schema type 'xsd:float' */
    // timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinExposureTime = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinExposureTime = 1.0/10.0;
    // timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxExposureTime = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxExposureTime = 1.0/10.0;
    // timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinGain = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinGain = 10.0;
    // timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxGain = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxGain = 10.0;
    // timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Iris = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Exposure->Iris = 10.0;
    // timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinIris = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MinIris = 10.0;
    // timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxIris = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Exposure->MaxIris = 10.0;
 
    /** Optional element 'tt:Focus' of XML schema type 'tt:FocusConfiguration20' */
    // timg__GetImagingSettingsResponse->ImagingSettings->Focus = (struct tt__FocusConfiguration20 *)soap_malloc(soap, sizeof(struct tt__FocusConfiguration20));
    // MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->Focus, 0x00, sizeof(struct tt__FocusConfiguration20));
    //timg__GetImagingSettingsResponse->ImagingSettings->Focus->AutoFocusMode = tt__AutoFocusMode__MANUAL;
    timg__GetImagingSettingsResponse->ImagingSettings->Focus = NULL;
    // timg__GetImagingSettingsResponse->ImagingSettings->Focus->DefaultSpeed = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Focus->DefaultSpeed = 10.0;
 
   
    /** Optional element 'tt:WideDynamicRange' of XML schema type 'tt:WideDynamicRange20' */

    timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange =
        (struct tt__WideDynamicRange20 *)soap_malloc(soap, sizeof(struct tt__WideDynamicRange20));
    MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange, 0x00, sizeof(struct tt__WideDynamicRange20));
    if(wdr_enabled){
        timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Mode = tt__WideDynamicMode__ON;
    }
    else{
        timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Mode = tt__WideDynamicMode__OFF;
    }
    timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Level = (float *)soap_malloc(soap, sizeof(float)); 
    *timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Level = wdr_strength;
   
    /** Optional element 'tt:WhiteBalance' of XML schema type 'tt:WhiteBalance20' */
    timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance =
        (struct tt__WhiteBalance20 *)soap_malloc(soap, sizeof(struct tt__WhiteBalance20));
    MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance, 0x00, sizeof(struct tt__WhiteBalance20));
    /** Required element 'tt:Mode' of XML schema type 'tt:WhiteBalanceMode' */
    if(wb_mode == 1){  // 1 = manual
        timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->Mode = tt__WhiteBalanceMode__MANUAL;
    }
    else{
        timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->Mode = tt__WhiteBalanceMode__AUTO;
    }
    timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CrGain = (float *)soap_malloc(soap, sizeof(float));
    *timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CrGain = wb_r_gain;
    timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CbGain = (float *)soap_malloc(soap, sizeof(float));
    *timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CbGain = wb_b_gain;
    /** Optional element 'tt:Extension' of XML schema type 'tt:WhiteBalance20Extension' */
    timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->Extension = NULL;
    //dom_att(&timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

 
    /** Optional element 'tt:Extension' of XML schema type 'tt:ImagingSettingsExtension20' */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension =
        (struct tt__ImagingSettingsExtension20 *)soap_malloc(soap, sizeof(struct tt__ImagingSettingsExtension20));
    MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->Extension, 0x00, sizeof(struct tt__ImagingSettingsExtension20));
 
    /** Optional element 'tt:ImageStabilization' of XML schema type 'tt:ImageStabilization' */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->ImageStabilization = NULL;
 
    /** Optional element 'tt:Extension' of XML schema type 'tt:ImagingSettingsExtension202' */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension =
        (struct tt__ImagingSettingsExtension202 *)soap_malloc(soap, sizeof(struct tt__ImagingSettingsExtension202));
    MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension, 0x00, sizeof(struct tt__ImagingSettingsExtension202));
 
    /** Sequence of elements 'tt:IrCutFilterAutoAdjustment' of XML schema type 'tt:IrCutFilterAutoAdjustment' stored in dynamic array IrCutFilterAutoAdjustment of length __sizeIrCutFilterAutoAdjustment */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->__sizeIrCutFilterAutoAdjustment = 0;
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment = NULL;
 
    /** Optional element 'tt:Extension' of XML schema type 'tt:ImagingSettingsExtension203' */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension =
        (struct tt__ImagingSettingsExtension203 *)soap_malloc(soap, sizeof(struct tt__ImagingSettingsExtension203));
    MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension, 0x00, sizeof(struct tt__ImagingSettingsExtension203));
 
    /** Optional element 'tt:ToneCompensation' of XML schema type 'tt:ToneCompensation' */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->ToneCompensation = NULL;
 
    /** Optional element 'tt:Defogging' of XML schema type 'tt:Defogging' */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging = NULL;
    
    // timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging =
    //     (struct tt__Defogging *)soap_malloc(soap, sizeof(struct tt__Defogging));
    // MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging, 0x00, sizeof(struct tt__Defogging));
    // timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging->Mode =
    // (char *)soap_malloc(soap, sizeof(char) * 10);
    // MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging->Mode, 0x00, sizeof(char) * 10);
   
    // snprintf(timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging->Mode, 10, "OFF");
   
    // timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging->Level = (float *)soap_malloc(soap, sizeof(float));
    // *timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging->Level = 10.0;
    // timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging->Extension = NULL;
    //dom_att(&timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Defogging->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
 
    /** Optional element 'tt:NoiseReduction' of XML schema type 'tt:NoiseReduction' */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->NoiseReduction =
        (struct tt__NoiseReduction *)soap_malloc(soap, sizeof(struct tt__NoiseReduction));
    MEMSET(timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->NoiseReduction, 0x00, sizeof(struct tt__NoiseReduction));
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->NoiseReduction->Level = 10.0;
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->NoiseReduction->__size = 0;
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->NoiseReduction->__any = NULL;
    //dom_att(&timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->NoiseReduction->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
 
    /** Optional element 'tt:Extension' of XML schema type 'tt:ImagingSettingsExtension204' */
    timg__GetImagingSettingsResponse->ImagingSettings->Extension->Extension->Extension->Extension = NULL;
 
    /** XML DOM attribute list */
    //dom_att(&timg__GetImagingSettingsResponse->ImagingSettings->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
 
    return SOAP_OK;
}

/** Web service operation '__timg__SetImagingSettings' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__SetImagingSettings(struct soap* soap, struct _timg__SetImagingSettings *timg__SetImagingSettings, struct _timg__SetImagingSettingsResponse *timg__SetImagingSettingsResponse)
{
    printf("-------------------------__timg__SetImagingSettings------------------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    int cam_id = 0;
    int backlight_mode;
    int backlightLevel;

//request validation start
    if (NULL == timg__SetImagingSettings->VideoSourceToken){
        LOG_ERROR("Invalid argument. VideoSourceToken[%p]", timg__SetImagingSettings->VideoSourceToken);
        return SOAP_FAULT;
    }

    if (NULL == timg__SetImagingSettings->ImagingSettings)
    {
        LOG_ERROR("Invalid argument. ImagingSettings[%p]", timg__SetImagingSettings->ImagingSettings);
        return SOAP_FAULT;
    }

    if (0 != strcmp("VideoSourceToken", timg__SetImagingSettings->VideoSourceToken))
    {
        LOG_ERROR("Invalid VideoSourceToken ->  %s", timg__SetImagingSettings->VideoSourceToken);
        return SOAP_FAULT;
    }

//request validation end

    if (NULL != timg__SetImagingSettings->ImagingSettings->BacklightCompensation)
    {
        printf("-----------BacklightCompensation (WDR) Start----------\n");
        backlight_mode = timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Mode;
        if(backlight_mode == 0){
            printf("backlight_mode is 0 - WDR turned off\n");
            set_wdr_enable(cam_id, FALSE);
        }
        else{
            printf("backlight_mode is 1 - WDR turned on\n");
            set_wdr_enable(cam_id, TRUE);
        }
        
        if(NULL != timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Level){
            backlightLevel = *timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Level;
            printf("WDR strength is %d\n", backlightLevel);
            set_manual_wdr_strength(cam_id, backlightLevel);
        }
        printf("-----------BacklightCompensation (WDR) End----------\n");
    }
    if (NULL != timg__SetImagingSettings->ImagingSettings->IrCutFilter)
    {
        printf("-----------IrCutFilter Start----------\n");
        // TODO: Implement IR cut filter control for Novatek platform
        printf("IrCutFilter setting not yet implemented for Novatek\n");
        printf("-----------IrCutFilter End----------\n");
    }
    
    if (NULL != timg__SetImagingSettings->ImagingSettings->Brightness)
    {
        if((*(timg__SetImagingSettings->ImagingSettings->Brightness) >= 1) &&
            (*(timg__SetImagingSettings->ImagingSettings->Brightness) <= 100))
        {
            int onvif_value = *timg__SetImagingSettings->ImagingSettings->Brightness;
            int nvt_value = ONVIF_TO_NOVATEK(onvif_value);
            printf("brightness ONVIF=%d, Novatek=%d\n", onvif_value, nvt_value);
            HD_RESULT ret = set_camera_brightness(cam_id, nvt_value);
            if (ret != HD_OK) {
                printf("Warning: Failed to set brightness\n");
            }
        }
        else
        {
            return SOAP_FAULT;
        }
    }
    if (NULL != timg__SetImagingSettings->ImagingSettings->ColorSaturation)
    {
        if((*(timg__SetImagingSettings->ImagingSettings->ColorSaturation) >= 1) &&
            (*(timg__SetImagingSettings->ImagingSettings->ColorSaturation) <= 100))
        {
            int onvif_value = *timg__SetImagingSettings->ImagingSettings->ColorSaturation;
            int nvt_value = ONVIF_TO_NOVATEK(onvif_value);
            printf("saturation ONVIF=%d, Novatek=%d\n", onvif_value, nvt_value);
            HD_RESULT ret = set_camera_saturation(cam_id, nvt_value);
            if (ret != HD_OK) {
                printf("Warning: Failed to set saturation\n");
            }
        }
        else
        {
            printf("color saturation is %d\n and is Invalid", *timg__SetImagingSettings->ImagingSettings->ColorSaturation);

            //onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:SettingsInvalid", "The requested settings are incorrect.");
            return SOAP_FAULT;
        }
    }
    if (NULL != timg__SetImagingSettings->ImagingSettings->Contrast)
    {
        if((*(timg__SetImagingSettings->ImagingSettings->Contrast) >= 1) &&
            (*(timg__SetImagingSettings->ImagingSettings->Contrast) <= 100))
        {   
            int onvif_value = *timg__SetImagingSettings->ImagingSettings->Contrast;
            int nvt_value = ONVIF_TO_NOVATEK(onvif_value);
            printf("contrast ONVIF=%d, Novatek=%d\n", onvif_value, nvt_value);
            HD_RESULT ret = set_camera_contrast(cam_id, nvt_value);
            if (ret != HD_OK) {
                printf("Warning: Failed to set contrast\n");
            }
        }
        else
        {
            return SOAP_FAULT;
        }
    }
    if (NULL != timg__SetImagingSettings->ImagingSettings->Sharpness)
    {
        if((*(timg__SetImagingSettings->ImagingSettings->Sharpness) >= 1) &&
            (*(timg__SetImagingSettings->ImagingSettings->Sharpness) <= 100))
        {
            int onvif_value = *timg__SetImagingSettings->ImagingSettings->Sharpness;
            int nvt_value = ONVIF_TO_NOVATEK(onvif_value);
            printf("sharpness ONVIF=%d, Novatek=%d\n", onvif_value, nvt_value);
            HD_RESULT ret = set_camera_sharpness(cam_id, nvt_value);
            if (ret != HD_OK) {
                printf("Warning: Failed to set sharpness\n");
            }
        }
        else
        {
            return SOAP_FAULT;
        }
    }

        /** Optional element 'tt:WideDynamicRange' of XML schema type 'tt:WideDynamicRange20' */
    if (NULL != timg__SetImagingSettings->ImagingSettings->WideDynamicRange)
    {
        int wdrmode = timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Mode;
        if(wdrmode == tt__WideDynamicMode__ON){
            printf("wdrmode is ON - turned on\n");
            set_wdr_enable(cam_id, TRUE);
        }
        else{
            printf("wdrmode is OFF - turned off\n");
            set_wdr_enable(cam_id, FALSE);
        }
        if (NULL != timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level){
            int WDRstrength = *timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level;
            printf("WDRstrength is %d\n", WDRstrength);
            set_manual_wdr_strength(cam_id, WDRstrength);
        }
    }

    /* Camera-setup -> Image Settings -> White balance covered */
    if (NULL != timg__SetImagingSettings->ImagingSettings->WhiteBalance)
    {
        if (tt__WhiteBalanceMode__AUTO == timg__SetImagingSettings->ImagingSettings->WhiteBalance->Mode)
        {
            printf("WhiteBalance is AUTO\n");
            set_camera_wb_mode(cam_id, 0);  // 0 = auto
        }
        else if (tt__WhiteBalanceMode__MANUAL == timg__SetImagingSettings->ImagingSettings->WhiteBalance->Mode)
        {
            printf("WhiteBalance is MANUAL\n");
            set_camera_wb_mode(cam_id, 1);  // 1 = manual
            
            // Get current gains first
            int r_gain = 128, g_gain = 128, b_gain = 128;
            get_camera_manual_wb_gain(cam_id, &r_gain, &g_gain, &b_gain);
            
            if (NULL != timg__SetImagingSettings->ImagingSettings->WhiteBalance->CrGain){
                r_gain = *timg__SetImagingSettings->ImagingSettings->WhiteBalance->CrGain;
                printf("Red AWB gain is %d\n", r_gain);
            }
            if (NULL != timg__SetImagingSettings->ImagingSettings->WhiteBalance->CbGain){
                b_gain = *timg__SetImagingSettings->ImagingSettings->WhiteBalance->CbGain;
                printf("Blue AWB gain is %d\n", b_gain);
            }
            
            set_camera_manual_wb_gain(cam_id, r_gain, g_gain, b_gain);
        }
    }
    
    return 0;
}

/** Web service operation '__timg__GetOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetOptions(struct soap* soap, struct _timg__GetOptions *timg__GetOptions, struct _timg__GetOptionsResponse *timg__GetOptionsResponse)
{
    printf("--------------------__timg__GetOptions--------------------\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (0 != strcmp("VideoSourceToken", timg__GetOptions->VideoSourceToken)){
            LOG_ERROR("Invalid VideoSourceToken ->  %s", timg__GetOptions->VideoSourceToken);
            return SOAP_FAULT;
    }
    /** Required element 'timg:ImagingOptions' of XML schema type 'tt:ImagingOptions20' */
    timg__GetOptionsResponse->ImagingOptions = (struct tt__ImagingOptions20 *)soap_malloc(soap, sizeof(struct tt__ImagingOptions20));
    MEMSET(timg__GetOptionsResponse->ImagingOptions, 0x00, sizeof(struct tt__ImagingOptions20));

    /** Optional element 'tt:BacklightCompensation' of XML schema type 'tt:BacklightCompensationOptions20' */
    timg__GetOptionsResponse->ImagingOptions->Brightness = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
    timg__GetOptionsResponse->ImagingOptions->Brightness->Min = BRIGHTNESS.MIN_VALUE;
    timg__GetOptionsResponse->ImagingOptions->Brightness->Max = BRIGHTNESS.MAX_VALUE;
    timg__GetOptionsResponse->ImagingOptions->ColorSaturation = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
    timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Min = SATURATION.MIN_VALUE;
    timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Max = SATURATION.MAX_VALUE;
    timg__GetOptionsResponse->ImagingOptions->Contrast = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
    timg__GetOptionsResponse->ImagingOptions->Contrast->Min = CONTRAST.MIN_VALUE;
    timg__GetOptionsResponse->ImagingOptions->Contrast->Max = CONTRAST.MAX_VALUE;
    timg__GetOptionsResponse->ImagingOptions->Sharpness = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
    timg__GetOptionsResponse->ImagingOptions->Sharpness->Min = SHARPNESS.MIN_VALUE;
    timg__GetOptionsResponse->ImagingOptions->Sharpness->Max = SHARPNESS.MAX_VALUE;
    timg__GetOptionsResponse->ImagingOptions->BacklightCompensation = (struct tt__BacklightCompensationOptions20 *)soap_malloc(soap, sizeof(struct tt__BacklightCompensationOptions20));
    MEMSET(timg__GetOptionsResponse->ImagingOptions->BacklightCompensation, 0x00, sizeof(struct tt__BacklightCompensationOptions20));
    timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->__sizeMode = 2;
    timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode = (enum tt__BacklightCompensationMode *)soap_malloc(soap, sizeof(enum tt__BacklightCompensationMode) * 2);
    timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode[0] = tt__BacklightCompensationMode__OFF;
    timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode[1] = tt__BacklightCompensationMode__ON;
    timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Level = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
    timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Level->Min = MIN_BLC_LEVEL;
    timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Level->Max = MAX_BLC_LEVEL;

    /** Optional element 'tt:Exposure' of XML schema type 'tt:ExposureOptions20' */
    timg__GetOptionsResponse->ImagingOptions->Exposure = NULL; // TODO: implement exposure options
    
    /** Optional element 'tt:Focus' of XML schema type 'tt:FocusOptions20' */
    timg__GetOptionsResponse->ImagingOptions->Focus = NULL; //Depends on camera

    timg__GetOptionsResponse->ImagingOptions->__sizeIrCutFilterModes = 2;
    timg__GetOptionsResponse->ImagingOptions->IrCutFilterModes = (enum tt__IrCutFilterMode *)soap_malloc(soap, sizeof(enum tt__IrCutFilterMode) * 3);
    timg__GetOptionsResponse->ImagingOptions->IrCutFilterModes[0] = tt__IrCutFilterMode__ON;
    timg__GetOptionsResponse->ImagingOptions->IrCutFilterModes[1] = tt__IrCutFilterMode__OFF;

    /** Optional element 'tt:WideDynamicRange' of XML schema type 'tt:WideDynamicRangeOptions20' */
    timg__GetOptionsResponse->ImagingOptions->WideDynamicRange =
        (struct tt__WideDynamicRangeOptions20 *)soap_malloc(soap, sizeof(struct tt__WideDynamicRangeOptions20));
    MEMSET(timg__GetOptionsResponse->ImagingOptions->WideDynamicRange, 0x00, sizeof(struct tt__WideDynamicRangeOptions20));
    /** Sequence of at least 1 elements 'tt:Mode' of XML schema type 'tt:WideDynamicMode' stored in dynamic array Mode of length __sizeMode */
    timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->__sizeMode = 2;
    timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Mode = (enum tt__WideDynamicMode *)soap_malloc(soap, sizeof(enum tt__WideDynamicMode) * 2);
    timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Mode[0] = tt__WideDynamicMode__OFF;
    timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Mode[1] = tt__WideDynamicMode__ON;
    /** Optional element 'tt:Level' of XML schema type 'tt:FloatRange' */
    timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
    timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level->Min = MIN_WDR_LEVEL;
    timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level->Max = MAX_WDR_LEVEL;

    /** Optional element 'tt:WhiteBalance' of XML schema type 'tt:WhiteBalanceOptions20' */
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance =
        (struct tt__WhiteBalanceOptions20 *)soap_malloc(soap, sizeof(struct tt__WhiteBalanceOptions20));
    MEMSET(timg__GetOptionsResponse->ImagingOptions->WhiteBalance, 0x00, sizeof(struct tt__WhiteBalanceOptions20));
    /** Sequence of at least 1 elements 'tt:Mode' of XML schema type 'tt:WhiteBalanceMode' stored in dynamic array Mode of length __sizeMode */
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->__sizeMode = 2;
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->Mode = (enum tt__WhiteBalanceMode *)soap_malloc(soap, sizeof(enum tt__WhiteBalanceMode) * 2);
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->Mode[0] = tt__WhiteBalanceMode__AUTO;
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->Mode[1] = tt__WhiteBalanceMode__MANUAL;
    /** Optional element 'tt:YrGain' of XML schema type 'tt:FloatRange' */
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YrGain = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YrGain->Min = MIN_RGAIN;
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YrGain->Max = MAX_RGAIN;
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YbGain = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YbGain->Min = MIN_BGAIN;
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->YbGain->Max = MAX_BGAIN;
    /** Optional element 'tt:Extension' of XML schema type 'tt:WhiteBalanceOptions20Extension' */
    timg__GetOptionsResponse->ImagingOptions->WhiteBalance->Extension = NULL;

    /** Optional element 'tt:Extension' of XML schema type 'tt:ImagingOptions20Extension' */
    timg__GetOptionsResponse->ImagingOptions->Extension =
        (struct tt__ImagingOptions20Extension *)soap_malloc(soap, sizeof(struct tt__ImagingOptions20Extension));
    MEMSET(timg__GetOptionsResponse->ImagingOptions->Extension, 0x00, sizeof(struct tt__ImagingOptions20Extension));
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    timg__GetOptionsResponse->ImagingOptions->Extension->__size = 0;
    timg__GetOptionsResponse->ImagingOptions->Extension->__any = NULL;

    /** Optional element 'tt:ImageStabilization' of XML schema type 'tt:ImageStabilizationOptions' */
    timg__GetOptionsResponse->ImagingOptions->Extension->ImageStabilization = NULL;

    /** Optional element 'tt:Extension' of XML schema type 'tt:ImagingOptions20Extension2' */
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension =
        (struct tt__ImagingOptions20Extension2 *)soap_malloc(soap, sizeof(struct tt__ImagingOptions20Extension2));
    MEMSET(timg__GetOptionsResponse->ImagingOptions->Extension->Extension, 0x00, sizeof(struct tt__ImagingOptions20Extension2));

    /** Optional element 'tt:IrCutFilterAutoAdjustment' of XML schema type 'tt:IrCutFilterAutoAdjustmentOptions' */
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->IrCutFilterAutoAdjustment = NULL;

    /** Optional element 'tt:Extension' of XML schema type 'tt:ImagingOptions20Extension3' */
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension =
        (struct tt__ImagingOptions20Extension3 *)soap_malloc(soap, sizeof(struct tt__ImagingOptions20Extension3));
    MEMSET(timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension, 0x00, sizeof(struct tt__ImagingOptions20Extension3));

    /** Optional element 'tt:ToneCompensationOptions' of XML schema type 'tt:ToneCompensationOptions' */
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->ToneCompensationOptions = NULL;

    /** Optional element 'tt:DefoggingOptions' of XML schema type 'tt:DefoggingOptions' */
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions = NULL;
    // timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions =
    //     (struct tt__DefoggingOptions *)soap_malloc(soap, sizeof(struct tt__DefoggingOptions));
    // MEMSET(timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions, 0x00, sizeof(struct tt__DefoggingOptions));
    // /** Sequence of at least 1 elements 'tt:Mode' of XML schema type 'xsd:string' stored in dynamic array Mode of length __sizeMode */
    // timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->__sizeMode = 2;
    // timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode = (char* *)soap_malloc(soap, sizeof(char*) * 2);
    // timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode[0] = (char*)soap_malloc(soap, sizeof(char) * VSMALL_BUFFER_LENGTH);
    // MEMSET(timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode[0], 0x00, sizeof(char) * VSMALL_BUFFER_LENGTH);
    // snprintf(timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode[0], VSMALL_BUFFER_LENGTH, "OFF");
    // timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode[1] = (char*)soap_malloc(soap, sizeof(char) * VSMALL_BUFFER_LENGTH);
    // MEMSET(timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode[1], 0x00, sizeof(char) * VSMALL_BUFFER_LENGTH);
    // snprintf(timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode[1], VSMALL_BUFFER_LENGTH, "ON");
    // /** Required element 'tt:Level' of XML schema type 'xsd:boolean' */
    // timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Level = xsd__boolean__true_;
    // /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    // timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->__size = 0;
    // timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->DefoggingOptions->__any = NULL;
    /** Optional element 'tt:NoiseReductionOptions' of XML schema type 'tt:NoiseReductionOptions' */
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->NoiseReductionOptions =
        (struct tt__NoiseReductionOptions *)soap_malloc(soap, sizeof(struct tt__NoiseReductionOptions));
    MEMSET(timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->NoiseReductionOptions, 0x00, sizeof(struct tt__NoiseReductionOptions));
    /** Required element 'tt:Level' of XML schema type 'xsd:boolean' */
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->NoiseReductionOptions->Level = xsd__boolean__true_;
    /** Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size */
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->NoiseReductionOptions->__size = 0;
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->NoiseReductionOptions->__any = NULL;
    timg__GetOptionsResponse->ImagingOptions->Extension->Extension->Extension->Extension = NULL;
    return 0;
}

/** Web service operation '__timg__Move' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__Move(struct soap* soap, struct _timg__Move *timg__Move, struct _timg__MoveResponse *timg__MoveResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    onvif_fault(soap , 0,"ter:ActionNotSupported", "ter:ActionNotSupported" ,"Optional Action Not Implemented" );
    return SOAP_FAULT;
}

/** Web service operation '__timg__Stop' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__Stop(struct soap* soap, struct _timg__Stop *timg__Stop, struct _timg__StopResponse *timg__StopResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    onvif_fault(soap ,ONVIF_ENV_SENDER,"ter:ActionNotSupported", "ter:ActionNotSupported" ,"Optional Action Not Implemented" );
    return SOAP_FAULT;
}

/** Web service operation '__timg__GetStatus' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetStatus(struct soap* soap, struct _timg__GetStatus *timg__GetStatus, struct _timg__GetStatusResponse *timg__GetStatusResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    onvif_fault(soap ,ONVIF_ENV_SENDER,"ter:ActionNotSupported", "ter:ActionNotSupported" ,"Optional Action Not Implemented" );
    return SOAP_FAULT;
}

/** Web service operation '__timg__GetMoveOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetMoveOptions(struct soap* soap, struct _timg__GetMoveOptions *timg__GetMoveOptions, struct _timg__GetMoveOptionsResponse *timg__GetMoveOptionsResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    onvif_fault(soap ,ONVIF_ENV_SENDER,"ter:ActionNotSupported", "ter:ActionNotSupported" ,"Optional Action Not Implemented" );
    return SOAP_FAULT;
}

/** Web service operation '__timg__GetPresets' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetPresets(struct soap* soap, struct _timg__GetPresets *timg__GetPresets, struct _timg__GetPresetsResponse *timg__GetPresetsResponse)
{
    onvif_fault(soap ,ONVIF_ENV_SENDER,"ter:ActionNotSupported", "ter:ActionNotSupported" ,"Optional Action Not Implemented" );
    return SOAP_FAULT;
}

/** Web service operation '__timg__GetCurrentPreset' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetCurrentPreset(struct soap* soap, struct _timg__GetCurrentPreset *timg__GetCurrentPreset, struct _timg__GetCurrentPresetResponse *timg__GetCurrentPresetResponse)
{
    onvif_fault(soap , ONVIF_ENV_SENDER,"ter:ActionNotSupported", "ter:ActionNotSupported" ,"Optional Action Not Implemented" );
    return SOAP_FAULT;
}

/** Web service operation '__timg__SetCurrentPreset' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__SetCurrentPreset(struct soap* soap, struct _timg__SetCurrentPreset *timg__SetCurrentPreset, struct _timg__SetCurrentPresetResponse *timg__SetCurrentPresetResponse)
{
    onvif_fault(soap , ONVIF_ENV_SENDER,"ter:ActionNotSupported", "ter:ActionNotSupported" ,"Optional Action Not Implemented" );
    return SOAP_FAULT;
}


#endif
