/**
 * @file onvif_services_stubs.c
 * @brief Stub implementations for ONVIF services not yet ported
 * 
 * This file provides minimal stub implementations for ONVIF Media2 and Events
 * services. These return "Not Implemented" errors until they are properly
 * ported to the Novatek platform.
 * 
 * NOTE: Imaging service has been implemented in onvif_imaging.c
 * TODO: Port Media2 and Events services incrementally
 */

#include "soapH.h"
#include "onvif_main.h"

// ============================================================================
// ONVIF Media2 Service Stubs
// ============================================================================

SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetVideoEncoderConfigurations(struct soap* soap, 
    struct tr2__GetConfiguration *tr2__GetVideoEncoderConfigurations, 
    struct _tr2__GetVideoEncoderConfigurationsResponse *tr2__GetVideoEncoderConfigurationsResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Media2 service not yet ported");
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAnalyticsConfigurations(struct soap* soap, 
    struct tr2__GetConfiguration *tr2__GetAnalyticsConfigurations, 
    struct _tr2__GetAnalyticsConfigurationsResponse *tr2__GetAnalyticsConfigurationsResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Media2 service not yet ported");
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetVideoEncoderConfiguration(struct soap* soap, 
    struct _tr2__SetVideoEncoderConfiguration *tr2__SetVideoEncoderConfiguration, 
    struct tr2__SetConfigurationResponse *tr2__SetVideoEncoderConfigurationResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Media2 service not yet ported");
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetOSDs(struct soap* soap, 
    struct _tr2__GetOSDs *tr2__GetOSDs, 
    struct _tr2__GetOSDsResponse *tr2__GetOSDsResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Media2 service not yet ported");
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetOSDOptions(struct soap* soap, 
    struct _tr2__GetOSDOptions *tr2__GetOSDOptions, 
    struct _tr2__GetOSDOptionsResponse *tr2__GetOSDOptionsResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Media2 service not yet ported");
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetOSD(struct soap* soap, 
    struct _tr2__SetOSD *tr2__SetOSD, 
    struct tr2__SetConfigurationResponse *tr2__SetOSDResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Media2 service not yet ported");
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__CreateOSD(struct soap* soap, 
    struct _tr2__CreateOSD *tr2__CreateOSD, 
    struct _tr2__CreateOSDResponse *tr2__CreateOSDResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Media2 service not yet ported");
}

SOAP_FMAC5 int SOAP_FMAC6 __tr2__DeleteOSD(struct soap* soap, 
    struct _tr2__DeleteOSD *tr2__DeleteOSD, 
    struct tr2__SetConfigurationResponse *tr2__DeleteOSDResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Media2 service not yet ported");
}

// ============================================================================
// ONVIF Analytics Service Stubs (if needed)
// ============================================================================

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetVideoAnalyticsConfiguration(struct soap* soap, 
    struct _tad__SetVideoAnalyticsConfiguration *tad__SetVideoAnalyticsConfiguration, 
    struct _tad__SetVideoAnalyticsConfigurationResponse *tad__SetVideoAnalyticsConfigurationResponse) {
    return soap_receiver_fault(soap, "Not Implemented", "Analytics service not yet ported");
}

// ============================================================================
// ONVIF Events Service — implemented in onvif_events.c
// ============================================================================
// All __tev__ functions removed from here to avoid linker symbol conflicts.
// See onvif_events.c for: CreatePullPointSubscription, GetEventProperties,
// GetServiceCapabilities, PullMessages, Renew, Unsubscribe, SetSynchronizationPoint

// NOTE: __timg__GetServiceCapabilities is now implemented in onvif_imaging.c
