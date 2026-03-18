#include "onvif_main.h"
#include "wsaapi.h"
// #include "wsseapi-lite.h"
#include "soapH.h"
#include "soapStub.h"
#include "wsdd.nsmap"
#include "wsddapi.h"

// #include "../import/wsdd10.h"

// #include "isp.h"
// #include <rk_aiq_user_api2_acsm.h>
// #include <rk_aiq_user_api2_camgroup.h>
// #include <rk_aiq_user_api2_imgproc.h>
// #include <rk_aiq_user_api2_sysctl.h>

#define TOKEN_LENGTH 64


#define MEMSET(s, c, n) (memset((void *)s, (int)c, (size_t)n))



//todo:add  below  to utils
// Scopes include device information that ONVIF test tools parse during WS-Discovery
// Format: onvif://www.onvif.org/{scope_type}/{value}
// Notes:
// - Scopes are space-separated (RFC 3986 compliant URIs)
// - Spaces in names must be replaced with underscores
// - ONVIF test tools display: hardware -> Model, name -> Device Name
// - mfr is not a standard scope (manufacturer comes from GetDeviceInformation)
char  g_scopes[] = "onvif://www.onvif.org/type/Network_Video_Transmitter "
                   "onvif://www.onvif.org/type/video_encoder "
                   "onvif://www.onvif.org/Profile/Streaming "
                   "onvif://www.onvif.org/Profile/T "
                   "onvif://www.onvif.org/Profile/G "
                   "onvif://www.onvif.org/Profile/M "
                   "onvif://www.onvif.org/hardware/Honeywell_IPCAM-5MP "
                   "onvif://www.onvif.org/name/Honeywell_Eterna "
                   "onvif://www.onvif.org/location/city/Pune "
                   "onvif://www.onvif.org/location/country/India";

char g_types[] = "tdn:NetworkVideoTransmitter";

#ifndef ONVIF_TEST
char end_point_reference[64] = "urn:uuid:464A4854-4656-5242-4530-313035394100";
#else
char end_point_reference[64] = "urn:uuid:464A4855-4656-5242-4530-313035394101";
#endif

//Todo: add method to set ONVIF_TCP_IP of the device in utils
char XAddrs[256];
// snprintf(XAddrs,sizeof(XAddrs) ,"http://%s:%d/onvif/device_service", ONVIF_TCP_IP, ONVIF_TCP_PORT);

/** Web service one-way operation 'SOAP_ENV__Fault' implementation, should return value of soap_send_empty_response() to send HTTP Accept acknowledgment, or return an error code, or return SOAP_OK to immediately return without sending an HTTP response message */
    SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap* soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail){
        printf("file:%s func:%s line:%d at here\n",__FILE__,__func__,__LINE__);
        return SOAP_OK;
    }
    /** Web service one-way operation '__wsdd__Hello' implementation, should return value of soap_send_empty_response() to send HTTP Accept acknowledgment, or return an error code, or return SOAP_OK to immediately return without sending an HTTP response message */
    void wsdd_event_Hello (struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
    {
        printf("EndpointReference %s\n",EndpointReference==NULL?"NULL":EndpointReference);
        printf("Types %s\n",Types==NULL?"NULL":Types);
        printf("Scopes %s\n",Scopes==NULL?"NULL":Scopes);
        printf("MatchBy %s\n",MatchBy==NULL?"NULL":MatchBy);
        printf("XAddrs %s\n",XAddrs==NULL?"NULL":XAddrs);
        printf("MetadataVersion %d\n",MetadataVersion);

    }
    /** Web service one-way operation '__wsdd__Bye' implementation, should return value of soap_send_empty_response() to send HTTP Accept acknowledgment, or return an error code, or return SOAP_OK to immediately return without sending an HTTP response message */
    void wsdd_event_Bye (struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
    {
        printf("file:%s func:%s line:%d at here\n",__FILE__,__func__,__LINE__);
        
    }
    /** Web service one-way operation '__wsdd__Probe' implementation, should return value of soap_send_empty_response() to send HTTP Accept acknowledgment, or return an error code, or return SOAP_OK to immediately return without sending an HTTP response message */
    soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID,
		const char *ReplyTo, const char *Types, const char *Scopes,
		const char *MatchBy, struct wsdd__ProbeMatchesType *matches) {
        printf("%s,%d\n", __FUNCTION__, __LINE__);
        printf("MessageID:%s\n", MessageID);
        printf("ReplyTo:%s\n", ReplyTo);
        printf("Types:%s\n", Types);
        printf("Scopes:%s\n", Scopes);
        printf("MatchBy:%s\n", MatchBy);
        
        // Debug: Show peer IP address (who is probing)
        printf("Probe from IP: %u.%u.%u.%u:%d\n", 
               ((soap->ip)>>24)&0xFF, ((soap->ip)>>16)&0xFF, 
               ((soap->ip)>>8)&0xFF, (soap->ip)&0xFF, soap->port);
        printf("\n");

        // *** DYNAMIC IP REFRESH ***
        // Refresh the IP address on each probe to handle IP changes
        get_ip_address();
        printf("Current IP address (refreshed): %s\n", ip_address);

        soap->header->wsa__RelatesTo = (struct wsa__Relationship*) soap_malloc(
                soap, sizeof(struct wsa__Relationship));

        if (soap->header->wsa__ReplyTo) {
            soap->header->wsa__To = soap->header->wsa__ReplyTo->Address;
        } else {
            soap->header->wsa__To = NULL;
        }
        soap->header->wsa__RelatesTo->__item = soap->header->wsa__MessageID;
        soap->header->wsa__RelatesTo->RelationshipType = NULL;
        soap->header->wsa__RelatesTo->__anyAttribute = NULL;
        soap->header->wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";
        soap->header->wsa__ReplyTo = NULL;

         sprintf(XAddrs, "http://%s:%d/onvif/device_service", ip_address, ONVIF_TCP_PORT);
        printf("Device Service Address : %s\n", XAddrs);

        // Format macAddress bytes as hex string for EndpointReference UUID
        sprintf(end_point_reference,
            "urn:uuid:1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",
                                                            macAddress[0],
                                                            macAddress[1],
                                                            macAddress[2],
                                                            macAddress[3],
                                                            macAddress[4],
                                                            macAddress[5]);

        printf("End Point Reference : %s\n", end_point_reference);

        soap_wsdd_init_ProbeMatches(soap, matches);
        int result = soap_wsdd_add_ProbeMatch(soap, matches,
                    end_point_reference, // "urn:uuid:464A4854-4656-5242-4530-313035394100",
                    "tdn:NetworkVideoTransmitter", g_scopes,
                    NULL,
                    XAddrs, // "http://10.67.97.35:5000/onvif/device_service", // "http://192.168.1.6:5000/onvif/device_service",
                    10);
        
        printf("ProbeMatch added, result=%d, match_count=%d\n", result, matches->__sizeProbeMatch);
        printf("Returning SOAP_WSDD_MANAGED to send ProbeMatches response\n");

        return SOAP_WSDD_MANAGED;
    }



//**************************For Testing***********************************************************
static char g_uuid[64] = {0};
SOAP_FMAC5 int SOAP_FMAC6  __wsdd__Probe1(struct soap* soap, struct wsdd__ProbeType *wsdd__Probe)
{
    printf("---------------------------------------------\n");
        char                            ip_addr[32] = {0};
        char                            mac_addr[13] = {0};
        struct wsdd__ScopesType         *pScopes = NULL;
        char                            str_tmp[256] = {0};

        char scopes_message[] =
            "onvif://www.onvif.org/type/NetworkVideoTransmitter\r\n"
            "onvif://www.onvif.org/Profile/Streaming\r\n"
            "onvif://www.onvif.org/Profile/Q/Operational\r\n"
            "onvif://www.onvif.org/hardware/HD1080P\r\n"
            "onvif://www.onvif.org/name/ETERNA\r\n"
            "onvif://www.onvif.org/location/city/Pune\r\n"
            "onvif://www.onvif.org/location/country/India\r\n";

        sprintf(ip_addr, "%u.%u.%u.%u", ((soap->ip)>>24)&0xFF, ((soap->ip)>>16)&0xFF, ((soap->ip)>>8)&0xFF,(soap->ip)&0xFF);
        sprintf(mac_addr, "000c29c9338f");

        // verify scropes
        if( wsdd__Probe->Scopes && wsdd__Probe->Scopes->__item )
        {
            if( wsdd__Probe->Scopes->MatchBy )
            {
            }
            else
            {
            }
        }

        // response ProbeMatches
        struct wsdd__ProbeMatchesType   wsdd__ProbeMatches = {0};
        struct wsdd__ProbeMatchType     *pProbeMatchType = NULL;
        struct wsa__Relationship        *pWsa__RelatesTo = NULL;
        char                            *pMessageID = NULL;

        pProbeMatchType = (struct wsdd__ProbeMatchType*)soap_malloc(soap, sizeof(struct wsdd__ProbeMatchType));
        soap_default_wsdd__ProbeMatchType(soap, pProbeMatchType);

        //sprintf(str_tmp, "http://%s/onvif/device_service", ip_addr);
        sprintf(str_tmp, "http://%s:%d/onvif/device_service", ip_address, ONVIF_TCP_PORT);
        pProbeMatchType->XAddrs = soap_strdup(soap, str_tmp);
        if( wsdd__Probe->Types && strlen(wsdd__Probe->Types) )
            pProbeMatchType->Types  = soap_strdup(soap, wsdd__Probe->Types);
        else
            pProbeMatchType->Types  = soap_strdup(soap, "dn:NetworkVideoTransmitter tds:Device");

        pProbeMatchType->MetadataVersion = 1;

        // Build Scopes Message
        pScopes = (struct wsdd__ScopesType *)soap_malloc(soap, sizeof(struct wsdd__ScopesType));
        soap_default_wsdd__ScopesType(soap, pScopes);
        //pScopes->MatchBy = soap_strdup(soap, "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
        pScopes->MatchBy = NULL;
        pScopes->__item  = soap_strdup(soap, scopes_message);
        pProbeMatchType->Scopes = pScopes;

        if( !strlen(g_uuid) )
            snprintf(g_uuid, 64, "%s", soap_wsa_rand_uuid(soap));
        pMessageID = g_uuid;
       // snprintf(str_tmp, 256, "%s-%s", pMessageID, mac_addr);
        sprintf(str_tmp, "%s", pMessageID);
        printf("g_uuid: %s\n", pMessageID);

        pProbeMatchType->wsa__EndpointReference.Address = soap_strdup(soap, pMessageID);

        wsdd__ProbeMatches.__sizeProbeMatch = 1;
        wsdd__ProbeMatches.ProbeMatch       = pProbeMatchType;

        // Build SOAP Header
        pWsa__RelatesTo = (struct wsa__Relationship*)soap_malloc(soap, sizeof(struct wsa__Relationship));
        soap_default__wsa__RelatesTo(soap, pWsa__RelatesTo);
        pWsa__RelatesTo->__item = soap->header->wsa__MessageID;
        soap->header->wsa__RelatesTo = pWsa__RelatesTo;
        soap->header->wsa__Action      = soap_strdup(soap, "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches");
        soap->header->wsa__To          = soap_strdup(soap, "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous");

        soap_send___wsdd__ProbeMatches(soap, "http://", NULL, &wsdd__ProbeMatches);

	return SOAP_OK;
        
}


    //***********************************************************************************************************
    /** Web service one-way operation '__wsdd__ProbeMatches' implementation, should return value of soap_send_empty_response() to send HTTP Accept acknowledgment, or return an error code, or return SOAP_OK to immediately return without sending an HTTP response message */
    void wsdd_event_ProbeMatches (struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *matches)
    {
        
        if(matches!=NULL)
        {
            int i;
            printf("matches count:%d\n",matches->__sizeProbeMatch);
            for( i = 0;i<matches->__sizeProbeMatch;i++)
            {
                printf("Endpoint Address:%s\n",matches->ProbeMatch[i].wsa__EndpointReference.Address==NULL?"NULL":matches->ProbeMatch[i].wsa__EndpointReference.Address);
                printf("Types:%s\n",matches->ProbeMatch[i].Types==NULL?"NULL":matches->ProbeMatch[i].Types);
                if(matches->ProbeMatch[i].Scopes!=NULL)
                {
                    printf("Scopes item:%s\n",matches->ProbeMatch[i].Scopes->__item==NULL?"NULL":matches->ProbeMatch[i].Scopes->__item);
                    printf("Scopes MatchBy:%s\n",matches->ProbeMatch[i].Scopes->MatchBy==NULL?"NULL":matches->ProbeMatch[i].Scopes->MatchBy);
                }
                else
                {
                    printf("Scopes is NULL\n");
                }
                printf("XAddr:%s\n",matches->ProbeMatch[i].XAddrs==NULL?"NULL":matches->ProbeMatch[i].XAddrs);
                printf("MetadataVersion:%d\n",matches->ProbeMatch[i].MetadataVersion);	
            }
        }
        else
        {
            printf("matches is NULL\n");
        }
        printf("file:%s func:%s line:%d at here,end\n",__FILE__,__func__,__LINE__);
    }
    /** Web service one-way operation '__wsdd__Resolve' implementation, should return value of soap_send_empty_response() to send HTTP Accept acknowledgment, or return an error code, or return SOAP_OK to immediately return without sending an HTTP response message */
    soap_wsdd_mode wsdd_event_Resolve (struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match)
    {
        printf("file:%s func:%s line:%d at here\n",__FILE__,__func__,__LINE__);
        return SOAP_WSDD_ADHOC;
    }
    /** Web service one-way operation '__wsdd__ResolveMatches' implementation, should return value of soap_send_empty_response() to send HTTP Accept acknowledgment, or return an error code, or return SOAP_OK to immediately return without sending an HTTP response message */
    void wsdd_event_ResolveMatches (struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
    {
        printf("file:%s func:%s line:%d at here\n",__FILE__,__func__,__LINE__);
    }
    /** Web service operation '__tdn__Hello' implementation, should return SOAP_OK or error code */
    SOAP_FMAC5 int SOAP_FMAC6 __tdn__Hello(struct soap* soap, struct wsdd__HelloType tdn__Hello, struct wsdd__ResolveType *tdn__HelloResponse){
    printf("sending a hello message");
    
    tdn__HelloResponse->wsa__EndpointReference.Address = (char*)soap_malloc(soap, 256);
    
    if(tdn__HelloResponse->wsa__EndpointReference.Address == NULL) {
        return SOAP_EOM;
    }
    snprintf(tdn__HelloResponse->wsa__EndpointReference.Address, 256, "http://%s:%d/onvif/device_service", onvifUdpAddress, ONVIF_UDP_PORT);
    tdn__HelloResponse->wsa__EndpointReference.PortType = "tdn:NetworkVideoTransmitter";
    tdn__HelloResponse->wsa__EndpointReference.ServiceName = "ImpactIpCamera";
    tdn__HelloResponse->wsa__EndpointReference.ReferenceParameters = NULL;
    tdn__HelloResponse->wsa__EndpointReference.ReferenceProperties = NULL;
    tdn__HelloResponse->wsa__EndpointReference.__any= NULL;
    tdn__HelloResponse->wsa__EndpointReference.__anyAttribute = NULL;
    tdn__HelloResponse->wsa__EndpointReference.__size = 0;
    return SOAP_OK;
}
    /** Web service operation '__tdn__Bye' implementation, should return SOAP_OK or error code */
    SOAP_FMAC5 int SOAP_FMAC6 __tdn__Bye(struct soap* soap, struct wsdd__ByeType tdn__Bye, struct wsdd__ResolveType *tdn__ByeResponse){
        printf("sending a bye message");
        // FIX: Was using comma operator instead of sprintf - format string was returned as literal
        {
            char bye_addr[256];
            snprintf(bye_addr, sizeof(bye_addr), "http://%s:%d/onvif/device_service", ip_address, ONVIF_TCP_PORT);
            tdn__ByeResponse->wsa__EndpointReference.Address = soap_strdup(soap, bye_addr);
        }
        tdn__ByeResponse->wsa__EndpointReference.PortType = "tdn:NetworkVideoTransmitter";
        tdn__ByeResponse->wsa__EndpointReference.ReferenceParameters = NULL;
        tdn__ByeResponse->wsa__EndpointReference.ReferenceProperties = NULL;
        tdn__ByeResponse->wsa__EndpointReference.ServiceName = "ImpactIpCamera";
        tdn__ByeResponse->wsa__EndpointReference.__any = NULL;
        tdn__ByeResponse->wsa__EndpointReference.__anyAttribute = NULL;
        tdn__ByeResponse->wsa__EndpointReference.__size = 0;
        return SOAP_OK;
    }
    /** Web service operation '__tdn__Probe' implementation, should return SOAP_OK or error code */
    SOAP_FMAC5 int SOAP_FMAC6 __tdn__Probe(struct soap* soap, struct wsdd__ProbeType tdn__Probe, struct wsdd__ProbeMatchesType *tdn__ProbeResponse){
        printf("recieved a probe  message");
        return SOAP_OK;
    }
