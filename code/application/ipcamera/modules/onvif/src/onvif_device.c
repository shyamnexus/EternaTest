//#include "onvif_services.h"
// Include gSOAP headers first to define struct soap
#include "soapH.h"
#include "soapStub.h"
#include "wsaapi.h"
// #include "wsseapi-lite.h"
// #include "wsdd.nsmap"
#include "wsddapi.h"

// Then include ONVIF headers
#include "onvif_main.h"
#include "onvif_adapter.h"
#include "ipcam/network_wrapper.h"

#include <time.h>
#include <sys/time.h>
#define VSMALL_BUFFER_LENGTH 10
#define MAX_SUPPORTED_CONF_LIST_STR 200
#define INFO_LENGTH          100
#include <sqlite3.h>

// #include "isp.h"
// #include <rk_aiq_user_api2_acsm.h>
// #include <rk_aiq_user_api2_camgroup.h>
// #include <rk_aiq_user_api2_imgproc.h>
// #include <rk_aiq_user_api2_sysctl.h>
// FIX: Unified DB path to match user_wrapper.cpp (was /mnt/app/ipcamera/users.db)
#define DATABASE_PATH "/mnt/app/ipcamera/db/users.db"
#define TOKEN_LENGTH 64
typedef const char *				CCHARPTR;
typedef char *					    CHARPTR;
typedef char CHAR;
#define INFO_BUFFER_LENGTH 1024
#define INFO_LENGTH 100
#define MEMSET(s, c, n) (memset((void *)s, (int)c, (size_t)n))


#define SPRINTF(s, f, a...) (sprintf((char *)s, (const char *)f, ##a))
#define SNPRINTF(s,n, f, a...) (snprintf((char *)s,n, (const char *)f, ##a))
#define STRCMP(s1, s2) (strcmp((const char *)s1, (const char *)s2))
#define MEMSET(s, c, n) (memset((void *)s, (int)c, (size_t)n))

#define MAX_SCOPE_STR_LEN 128

typedef struct
{
    bool m_b_isFixed;
    CHAR m_ca_scopeIteam[MAX_SCOPE_STR_LEN];
}T_SCOPE_CONFIG;

// interface, method, address, netmask, gateway are now declared in onvif_main.h

char test_search_domain[SMALL_BUFFER_LENGTH] = "test.com";
char dns_mode = 0;
// isDiscoveryOn is now declared in onvif_main.h

/** Web service operation '__tds__GetServices' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices(struct soap* soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
    printf("-------------------------------__tds__GetServices-------------------------------------\n");

    // *** DYNAMIC IP REFRESH ***
    get_ip_address();

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    int size = 6;  // Added Recording and Replay services for Profile G
    tds__GetServicesResponse->__sizeService = size;
    
    const char* namespaces[] = {
        "http://www.onvif.org/ver10/device/wsdl",
        "http://www.onvif.org/ver10/media/wsdl",
        "http://www.onvif.org/ver10/imaging/wsdl",
        "http://www.onvif.org/ver20/analytics/wsdl",
        "http://www.onvif.org/ver10/recording/wsdl",   // Recording Control (Profile G)
        "http://www.onvif.org/ver10/replay/wsdl",      // Replay (Profile G)
        // "http://www.onvif.org/ver20/media/wsdl",  // Media2 disabled - NVR will use Media1 instead
    };

  
    tds__GetServicesResponse->Service = (struct tds__Service *)soap_malloc(soap, sizeof(struct tds__Service) * tds__GetServicesResponse->__sizeService);
    MEMSET(tds__GetServicesResponse->Service, 0x00, tds__GetServicesResponse->__sizeService * sizeof(struct tds__Service));

     for (int i = 0; i < tds__GetServicesResponse->__sizeService; i++)
    {
        if (tds__GetServices->IncludeCapability)
        {


            printf("-------------------------------__tds__GetServices loop-------------------------------------\n");
            tds__GetServicesResponse->Service[i].Namespace = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * 100);
            MEMSET(tds__GetServicesResponse->Service[i].Namespace, 0x00, sizeof(CHAR) * 100);
            SNPRINTF(tds__GetServicesResponse->Service[i].Namespace, 100, "%s", namespaces[i]);

            tds__GetServicesResponse->Service[i].XAddr = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * 100);
            MEMSET(tds__GetServicesResponse->Service[i].XAddr, 0x00, sizeof(CHAR) * 100);


            tds__GetServicesResponse->Service[i].Capabilities = (struct _tds__Service_Capabilities *)soap_malloc(soap, sizeof(struct _tds__Service_Capabilities));
            MEMSET(tds__GetServicesResponse->Service[i].Capabilities, 0x00, sizeof(struct _tds__Service_Capabilities));
            
            tds__GetServicesResponse->Service[i].Version = (struct tt__OnvifVersion *)soap_malloc(soap, sizeof(struct tt__OnvifVersion));
            MEMSET(tds__GetServicesResponse->Service[i].Version, 0x00, sizeof(struct tt__OnvifVersion));

            if (strcmp(namespaces[i], "http://www.onvif.org/ver10/device/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/device_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 1;
                tds__GetServicesResponse->Service[i].Version->Minor = 10;
                tds__GetServicesResponse->Service[i].__size = 0;
                tds__GetServicesResponse->Service[i].__any = NULL;
                

                struct tds__DeviceServiceCapabilities *tdsdeviceservicecapabilities = soap_new_tds__DeviceServiceCapabilities(soap, -1);

                //<Device><Network>
                tdsdeviceservicecapabilities->Network = (struct tds__NetworkCapabilities *) soap_malloc (soap, sizeof(struct tds__NetworkCapabilities));
                MEMSET(tdsdeviceservicecapabilities->Network , 0x00, sizeof(struct tds__NetworkCapabilities));

                 tdsdeviceservicecapabilities->Network->IPFilter = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Network->IPFilter) = xsd__boolean__false_;                
                tdsdeviceservicecapabilities->Network->ZeroConfiguration= (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Network->ZeroConfiguration) = xsd__boolean__false_;        
                tdsdeviceservicecapabilities->Network->IPVersion6 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Network->IPVersion6) = xsd__boolean__false_;              
                tdsdeviceservicecapabilities->Network->DynDNS = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Network->DynDNS) = xsd__boolean__false_;  
                tdsdeviceservicecapabilities->Network->Dot11Configuration = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Network->Dot11Configuration) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Network->Dot1XConfigurations = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Network->Dot1XConfigurations) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Network->HostnameFromDHCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Network->HostnameFromDHCP) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Network->NTP = (int *)soap_malloc(soap, sizeof(int));
                *(tdsdeviceservicecapabilities->Network->NTP) = 0;
                tdsdeviceservicecapabilities->Network->DHCPv6 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Network->DHCPv6) = xsd__boolean__false_;

                //<Device><Security>
                  tdsdeviceservicecapabilities->Security = (struct tds__SecurityCapabilities *) soap_malloc (soap, sizeof(struct tds__SecurityCapabilities));
                  MEMSET(tdsdeviceservicecapabilities->Security , 0x00, sizeof(struct tds__SecurityCapabilities));

                tdsdeviceservicecapabilities->Security->TLS1_x002e0 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->TLS1_x002e0) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Security->TLS1_x002e1= (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->TLS1_x002e1) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Security->TLS1_x002e2 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->TLS1_x002e2) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Security->OnboardKeyGeneration = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->OnboardKeyGeneration) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Security->AccessPolicyConfig = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->AccessPolicyConfig) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Security->DefaultAccessPolicy = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->DefaultAccessPolicy) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Security->Dot1X = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->Dot1X) = xsd__boolean__false_;
                 tdsdeviceservicecapabilities->Security->RemoteUserHandling = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->RemoteUserHandling) = xsd__boolean__false_;
                 tdsdeviceservicecapabilities->Security->X_x002e509Token = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->X_x002e509Token) = xsd__boolean__false_;
                  tdsdeviceservicecapabilities->Security->SAMLToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->SAMLToken) = xsd__boolean__false_;
                  tdsdeviceservicecapabilities->Security->KerberosToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->KerberosToken) = xsd__boolean__false_;
                //@TODO , make below flags true as per the implementation.
                 tdsdeviceservicecapabilities->Security->UsernameToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->UsernameToken) = xsd__boolean__true_;
                tdsdeviceservicecapabilities->Security->HttpDigest = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->HttpDigest) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Security->RELToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->RELToken) = xsd__boolean__false_;
                tdsdeviceservicecapabilities->Security->JsonWebToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->Security->JsonWebToken) = xsd__boolean__false_;

                // //@TODO, change the No. Of max users, max user name length and max password length as per inplementation.
                tdsdeviceservicecapabilities->Security->MaxUsers = (int *)soap_malloc(soap, sizeof(int));
                *(tdsdeviceservicecapabilities->Security->MaxUsers) = 10;
                tdsdeviceservicecapabilities->Security->MaxUserNameLength = (int *)soap_malloc(soap, sizeof(int));
                *(tdsdeviceservicecapabilities->Security->MaxUserNameLength) = 32;
                tdsdeviceservicecapabilities->Security->MaxPasswordLength= (int *)soap_malloc(soap, sizeof(int));
                *(tdsdeviceservicecapabilities->Security->MaxPasswordLength) = 32;

           
                //<Device><Systerm>
                 tdsdeviceservicecapabilities->System = (struct tds__SystemCapabilities *) soap_malloc (soap, sizeof(struct tds__SystemCapabilities));
                 MEMSET(tdsdeviceservicecapabilities->System , 0x00, sizeof(struct tds__SystemCapabilities));

                tdsdeviceservicecapabilities->System->DiscoveryResolve = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->DiscoveryResolve) = xsd__boolean__true_;
                 tdsdeviceservicecapabilities->System->DiscoveryBye = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->DiscoveryBye) = xsd__boolean__true_;
                 tdsdeviceservicecapabilities->System->RemoteDiscovery = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->RemoteDiscovery) = xsd__boolean__true_;
                 tdsdeviceservicecapabilities->System->SystemBackup = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->SystemBackup) = xsd__boolean__true_;
                 tdsdeviceservicecapabilities->System->SystemLogging = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->SystemLogging) = xsd__boolean__false_;
                 tdsdeviceservicecapabilities->System->FirmwareUpgrade = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->FirmwareUpgrade) = xsd__boolean__true_;

                 tdsdeviceservicecapabilities->System->HttpFirmwareUpgrade = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->HttpFirmwareUpgrade) = xsd__boolean__true_;
                 tdsdeviceservicecapabilities->System->HttpSystemBackup = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->HttpSystemBackup) = xsd__boolean__true_;
                 tdsdeviceservicecapabilities->System->HttpSystemLogging = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->HttpSystemLogging) = xsd__boolean__true_;
                 tdsdeviceservicecapabilities->System->HttpSupportInformation = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tdsdeviceservicecapabilities->System->HttpSupportInformation) = xsd__boolean__true_;
               soap_elt_set(&tds__GetServicesResponse->Service[i].Capabilities->__any, NULL, "tds:DeviceServiceCapabilities"); 
               soap_elt_node(&tds__GetServicesResponse->Service[i].Capabilities->__any, tdsdeviceservicecapabilities, SOAP_TYPE_tds__DeviceServiceCapabilities);

              
            }
            if (strcmp(namespaces[i], "http://www.onvif.org/ver10/media/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/media_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 1;
                tds__GetServicesResponse->Service[i].Version->Minor = 10;
                tds__GetServicesResponse->Service[i].__size = 0;
                tds__GetServicesResponse->Service[i].__any = NULL;
                // tds__GetServicesResponse->Service[i].__anyAttribute = NULL;

                struct trt__Capabilities *trtcapabilities = soap_new_trt__Capabilities(soap, -1);

                trtcapabilities->ProfileCapabilities = (struct trt__ProfileCapabilities *)soap_malloc(soap, sizeof(struct trt__ProfileCapabilities));
                MEMSET(trtcapabilities->ProfileCapabilities, 0x00, sizeof(struct trt__ProfileCapabilities));

                // Optional attribute 'MaximumNumberOfProfiles' of XML schema type 'xsd:int' 
                trtcapabilities->ProfileCapabilities->MaximumNumberOfProfiles = (int *)soap_malloc(soap, sizeof(int));
                *(trtcapabilities->ProfileCapabilities->MaximumNumberOfProfiles) = 3;

                // Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size 
                trtcapabilities->ProfileCapabilities->__size = 0;
                trtcapabilities->ProfileCapabilities->__any = NULL;
                //dom_att(&trtcapabilities->ProfileCapabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

                 // Required element 'trt:StreamingCapabilities' of XML schema type 'trt:StreamingCapabilities'
                trtcapabilities->StreamingCapabilities =  (struct trt__StreamingCapabilities *)soap_malloc(soap, sizeof(struct trt__StreamingCapabilities));
                MEMSET(trtcapabilities->StreamingCapabilities, 0x00, sizeof(struct trt__StreamingCapabilities));

                // Optional attribute 'RTPMulticast' of XML schema type 'xsd:boolean'
                trtcapabilities->StreamingCapabilities->RTPMulticast = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->StreamingCapabilities->RTPMulticast) = xsd__boolean__false_;

                // Optional attribute 'RTP_TCP' of XML schema type 'xsd:boolean' 
                trtcapabilities->StreamingCapabilities->RTP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->StreamingCapabilities->RTP_USCORETCP) = xsd__boolean__true_;

                //Optional attribute 'RTP_RTSP_TCP' of XML schema type 'xsd:boolean' 
                trtcapabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP) = xsd__boolean__true_;

                // Optional attribute 'NonAggregateControl' of XML schema type 'xsd:boolean' 
                trtcapabilities->StreamingCapabilities->NonAggregateControl = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->StreamingCapabilities->NonAggregateControl) =  xsd__boolean__true_ ;

                // Optional attribute 'NoRTSPStreaming' of XML schema type 'xsd:boolean'
                trtcapabilities->StreamingCapabilities->NoRTSPStreaming = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->StreamingCapabilities->NoRTSPStreaming) = xsd__boolean__false_;

                // Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size 
                trtcapabilities->StreamingCapabilities->__size = 0;
                trtcapabilities->StreamingCapabilities->__any = NULL;
                //dom_att(&trtcapabilities->StreamingCapabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

                // Optional attribute 'SnapshotUri' of XML schema type 'xsd:boolean' 
                trtcapabilities->SnapshotUri = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->SnapshotUri) = xsd__boolean__true_;
                // Optional attribute 'Rotation' of XML schema type 'xsd:boolean' 
                trtcapabilities->Rotation = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->Rotation) = xsd__boolean__false_;
                // Optional attribute 'VideoSourceMode' of XML schema type 'xsd:boolean'
                trtcapabilities->VideoSourceMode = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->VideoSourceMode) = xsd__boolean__false_;
                // Optional attribute 'OSD' of XML schema type 'xsd:boolean'
                //@TODO, Make OSD true as per implementation
                trtcapabilities->OSD = NULL;
                // Optional attribute 'TemporaryOSDText' of XML schema type 'xsd:boolean'
                 trtcapabilities->TemporaryOSDText = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->TemporaryOSDText) = xsd__boolean__false_;
                //Optional attribute 'EXICompression' of XML schema type 'xsd:boolean'
                trtcapabilities->EXICompression = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(trtcapabilities->EXICompression) =  xsd__boolean__false_;

                // Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size 
                trtcapabilities->__size = 0;
                trtcapabilities->__any = NULL;

                soap_elt_set(&tds__GetServicesResponse->Service[i].Capabilities->__any, NULL, "trt:Capabilities");
                soap_elt_node(&tds__GetServicesResponse->Service[i].Capabilities->__any, trtcapabilities, SOAP_TYPE_trt__Capabilities);

            }
            if (strcmp(namespaces[i], "http://www.onvif.org/ver10/imaging/wsdl") == 0)
            {
                
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/image_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 1;
                tds__GetServicesResponse->Service[i].Version->Minor = 10;
                tds__GetServicesResponse->Service[i].__size = 0;
                tds__GetServicesResponse->Service[i].__any = NULL;
                // tds__GetServicesResponse->Service[i].__anyAttribute = NULL;

                 struct timg__Capabilities *timgcapabilities = soap_new_timg__Capabilities(soap, -1);

                 timgcapabilities->AdaptablePreset = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
                *(timgcapabilities->AdaptablePreset) = xsd__boolean__false_;

                timgcapabilities->ImageStabilization = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
                *(timgcapabilities->ImageStabilization) = xsd__boolean__false_;

                 timgcapabilities->Presets = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
                *(timgcapabilities->ImageStabilization) = xsd__boolean__false_;          

                 timgcapabilities->__any = NULL;
                //timgcapabilities->__size =0;

                soap_elt_set(&tds__GetServicesResponse->Service[i].Capabilities->__any, NULL, "timg:Capabilities");
                soap_elt_node(&tds__GetServicesResponse->Service[i].Capabilities->__any, timgcapabilities, SOAP_TYPE_timg__Capabilities);

            }

            if (strcmp(namespaces[i], "http://www.onvif.org/ver20/analytics/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/analytics_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 20;
                tds__GetServicesResponse->Service[i].Version->Minor = 06;
                tds__GetServicesResponse->Service[i].__size = 0;
                tds__GetServicesResponse->Service[i].__any = NULL;
                // tds__GetServicesResponse->Service[i].__anyAttribute = NULL;
             
                struct tan__Capabilities *tancapabilities = soap_new_tan__Capabilities(soap, -1);

                tancapabilities->RuleSupport = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tancapabilities->RuleSupport) = xsd__boolean__true_;

                tancapabilities->AnalyticsModuleOptionsSupported = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tancapabilities->AnalyticsModuleOptionsSupported) = xsd__boolean__true_;

                tancapabilities->AnalyticsModuleSupport = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tancapabilities->AnalyticsModuleSupport) = xsd__boolean__true_;

                tancapabilities->CellBasedSceneDescriptionSupported = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tancapabilities->CellBasedSceneDescriptionSupported) = xsd__boolean__false_;

                tancapabilities->SupportedMetadata = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tancapabilities->SupportedMetadata) = xsd__boolean__false_;

                soap_elt_set(&tds__GetServicesResponse->Service[i].Capabilities->__any, NULL, "tan:Capabilities");
                soap_elt_node(&tds__GetServicesResponse->Service[i].Capabilities->__any, tancapabilities, SOAP_TYPE_tan__Capabilities);
            }

            // Recording Control Service (Profile G) - IncludeCapability=true path
            if (strcmp(namespaces[i], "http://www.onvif.org/ver10/recording/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/recording_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 0;
                tds__GetServicesResponse->Service[i].Version->Minor = 0;
                tds__GetServicesResponse->Service[i].__size = 0;
                tds__GetServicesResponse->Service[i].__any = NULL;
                // Note: Recording capabilities structure would go here if needed
                // For now, empty capabilities to indicate service exists but minimal support
            }

            // Replay Service (Profile G) - IncludeCapability=true path
            if (strcmp(namespaces[i], "http://www.onvif.org/ver10/replay/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/replay_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 0;
                tds__GetServicesResponse->Service[i].Version->Minor = 0;
                tds__GetServicesResponse->Service[i].__size = 0;
                tds__GetServicesResponse->Service[i].__any = NULL;
                // Note: Replay capabilities structure would go here if needed
                // For now, empty capabilities to indicate service exists but minimal support
            }

            if (strcmp(namespaces[i], "http://www.onvif.org/ver20/media/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/media2_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 20;
                tds__GetServicesResponse->Service[i].Version->Minor = 06;
                tds__GetServicesResponse->Service[i].__size = 0;
                tds__GetServicesResponse->Service[i].__any = NULL;
                // tds__GetServicesResponse->Service[i].__anyAttribute = NULL;

                struct tr2__Capabilities2 *tr2capabilities = soap_new_tr2__Capabilities2(soap, -1);

                tr2capabilities->ProfileCapabilities = (struct tr2__ProfileCapabilities *)soap_malloc(soap, sizeof(struct tr2__ProfileCapabilities));
                MEMSET(tr2capabilities->ProfileCapabilities, 0x00, sizeof(struct tr2__ProfileCapabilities));

                // Optional attribute 'MaximumNumberOfProfiles' of XML schema type 'xsd:int' 
                tr2capabilities->ProfileCapabilities->MaximumNumberOfProfiles = (int *)soap_malloc(soap, sizeof(int));
                *(tr2capabilities->ProfileCapabilities->MaximumNumberOfProfiles) = 3;

                // Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size 
                tr2capabilities->ProfileCapabilities->__size = 0;
                tr2capabilities->ProfileCapabilities->__any = NULL;

                 // Required element 'trt:StreamingCapabilities' of XML schema type 'trt:StreamingCapabilities'
                tr2capabilities->StreamingCapabilities =  (struct tr2__StreamingCapabilities *)soap_malloc(soap, sizeof(struct tr2__StreamingCapabilities));
                MEMSET(tr2capabilities->StreamingCapabilities, 0x00, sizeof(struct tr2__StreamingCapabilities));

                // Optional attribute 'RTPMulticast' of XML schema type 'xsd:boolean'
                tr2capabilities->StreamingCapabilities->RTPMulticast = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->StreamingCapabilities->RTPMulticast) = xsd__boolean__false_;

                // Optional attribute 'RTP_TCP' of XML schema type 'xsd:boolean' 
                tr2capabilities->StreamingCapabilities->RTSPStreaming = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->StreamingCapabilities->RTSPStreaming) = xsd__boolean__true_;

                //Optional attribute 'RTP_RTSP_TCP' of XML schema type 'xsd:boolean' 
                tr2capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP) = xsd__boolean__true_;

                // Optional attribute 'NonAggregateControl' of XML schema type 'xsd:boolean' 
                tr2capabilities->StreamingCapabilities->NonAggregateControl = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->StreamingCapabilities->NonAggregateControl) =  xsd__boolean__true_ ;

                // Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size 
                tr2capabilities->StreamingCapabilities->__size = 0;
                tr2capabilities->StreamingCapabilities->__any = NULL;
                //dom_att(&trtcapabilities->StreamingCapabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

                // Optional attribute 'SnapshotUri' of XML schema type 'xsd:boolean' 
                tr2capabilities->SnapshotUri = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->SnapshotUri) = xsd__boolean__false_;
                // Optional attribute 'Rotation' of XML schema type 'xsd:boolean' 
                tr2capabilities->Rotation = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->Rotation) = xsd__boolean__false_;
                // Optional attribute 'VideoSourceMode' of XML schema type 'xsd:boolean'
                tr2capabilities->VideoSourceMode = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->VideoSourceMode) = xsd__boolean__false_;
                // Optional attribute 'OSD' of XML schema type 'xsd:boolean'
                //@TODO, Make OSD true as per implementation
                tr2capabilities->OSD = NULL;
                // Optional attribute 'TemporaryOSDText' of XML schema type 'xsd:boolean'
                 tr2capabilities->TemporaryOSDText = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->TemporaryOSDText) = xsd__boolean__false_;
                //Optional attribute 'EXICompression' of XML schema type 'xsd:boolean'
                tr2capabilities->Mask = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->Mask) =  xsd__boolean__false_;
                tr2capabilities->SourceMask = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
                *(tr2capabilities->SourceMask) =  xsd__boolean__false_;


                // Sequence of elements '-any' of XML schema type 'xsd:anyType' stored in dynamic array __any of length __size 
                tr2capabilities->__size = 0;
                tr2capabilities->__any = NULL;
                // XML DOM attribute list 
               // dom_att(&trtcapabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

                soap_elt_set(&tds__GetServicesResponse->Service[i].Capabilities->__any, NULL, "tr2:Capabilities2");
                soap_elt_node(&tds__GetServicesResponse->Service[i].Capabilities->__any, tr2capabilities, SOAP_TYPE_tr2__Capabilities2);
           }
            tds__GetServicesResponse->Service[i].__size = 0;
            tds__GetServicesResponse->Service[i].__any = NULL;
           // dom_att(&tds__GetServicesResponse->Service[i].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        }
        else
        {
            tds__GetServicesResponse->Service[i].Namespace = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(tds__GetServicesResponse->Service[i].Namespace, 0x00, sizeof(CHAR) * INFO_LENGTH);
            SNPRINTF(tds__GetServicesResponse->Service[i].Namespace, INFO_LENGTH, "%s", namespaces[i]);
            tds__GetServicesResponse->Service[i].XAddr = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            MEMSET(tds__GetServicesResponse->Service[i].XAddr, 0x00, sizeof(CHAR) * INFO_LENGTH);

            tds__GetServicesResponse->Service[i].Capabilities = NULL;
            tds__GetServicesResponse->Service[i].Version = (struct tt__OnvifVersion *)soap_malloc(soap, sizeof(struct tt__OnvifVersion));
            MEMSET(tds__GetServicesResponse->Service[i].Version, 0x00, sizeof(struct tt__OnvifVersion));

            if (STRCMP(namespaces[i], "http://www.onvif.org/ver10/device/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/device_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 19;
                tds__GetServicesResponse->Service[i].Version->Minor = 12;
            }
            if (STRCMP(namespaces[i], "http://www.onvif.org/ver10/media/wsdl") == 0)
            {

                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/media_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 19;
                tds__GetServicesResponse->Service[i].Version->Minor = 06;
            }
            if (STRCMP(namespaces[i], "http://www.onvif.org/ver20/media/wsdl") == 0)
            {
                // SNPRINTF(tds__GetServicesResponse->Service[i].XAddr, INFO_LENGTH, "%s/media2_service", l_ca_serviceAddr);
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/media2_service", ip_address, ONVIF_TCP_PORT);

                tds__GetServicesResponse->Service[i].Version->Major = 19;
                tds__GetServicesResponse->Service[i].Version->Minor = 06;
            }
            if (STRCMP(namespaces[i], "http://www.onvif.org/ver20/imaging/wsdl") == 0)
            {
                // SNPRINTF(tds__GetServicesResponse->Service[i].XAddr, INFO_LENGTH, "%s/imaging_service", l_ca_serviceAddr);
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/imaging_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 19;
                tds__GetServicesResponse->Service[i].Version->Minor = 06;
            }

            if (STRCMP(namespaces[i], "http://www.onvif.org/ver20/analytics/wsdl") == 0)
            {
                // SNPRINTF(tds__GetServicesResponse->Service[i].XAddr, INFO_LENGTH, "%s/analytics_service", l_ca_serviceAddr);
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/analytics_service", ip_address, ONVIF_TCP_PORT);

                tds__GetServicesResponse->Service[i].Version->Major = 20;
                tds__GetServicesResponse->Service[i].Version->Minor = 06;
            }

            // Recording Control Service (Profile G)
            if (STRCMP(namespaces[i], "http://www.onvif.org/ver10/recording/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/recording_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 17;
                tds__GetServicesResponse->Service[i].Version->Minor = 06;
            }

            // Replay Service (Profile G)
            if (STRCMP(namespaces[i], "http://www.onvif.org/ver10/replay/wsdl") == 0)
            {
                sprintf(tds__GetServicesResponse->Service[i].XAddr, "http://%s:%d/onvif/replay_service", ip_address, ONVIF_TCP_PORT);
                tds__GetServicesResponse->Service[i].Version->Major = 17;
                tds__GetServicesResponse->Service[i].Version->Minor = 06;
            }

            tds__GetServicesResponse->Service[i].__size = 0;
            tds__GetServicesResponse->Service[i].__any = NULL;
          //  dom_att(&tds__GetServicesResponse->Service[i].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        }

    }
    printf("__tds__GetServices is over\n");

    return 0;
}


/** Web service operation '__tds__GetServiceCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServiceCapabilities(struct soap* soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{
    printf("Called __tds__GetServiceCapabilities ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    

    tds__GetServiceCapabilitiesResponse->Capabilities = (struct tds__DeviceServiceCapabilities *)soap_malloc(soap, sizeof(struct tds__DeviceServiceCapabilities));
    MEMSET(tds__GetServiceCapabilitiesResponse->Capabilities, 0x00, sizeof(struct tds__DeviceServiceCapabilities));

        /*Network Capabilites*/
    tds__GetServiceCapabilitiesResponse->Capabilities->Network = (struct tds__NetworkCapabilities *)soap_malloc(soap, sizeof(struct tds__NetworkCapabilities));
    MEMSET(tds__GetServiceCapabilitiesResponse->Capabilities->Network, 0x00, sizeof(struct tds__NetworkCapabilities));
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot11Configuration = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot11Configuration) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->HostnameFromDHCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->HostnameFromDHCP) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->DHCPv6 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->DHCPv6) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot1XConfigurations = (int32_t*)soap_malloc(soap, sizeof(int32_t));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot1XConfigurations) = 0; // l_x_caps.m_ui32_dot1xConfigs;
    tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP = (int32_t*)soap_malloc(soap, sizeof(int32_t));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP) = 1; //l_x_caps.m_ui32_NTP;

    /*System Capabilities*/
    tds__GetServiceCapabilitiesResponse->Capabilities->System = (struct tds__SystemCapabilities *)soap_malloc(soap, sizeof(struct tds__SystemCapabilities));
    MEMSET(tds__GetServiceCapabilitiesResponse->Capabilities->System, 0x00, sizeof(struct tds__SystemCapabilities));

    tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryResolve = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryResolve) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->RemoteDiscovery = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->RemoteDiscovery) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemBackup = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemBackup) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemLogging = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemLogging) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->FirmwareUpgrade = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->FirmwareUpgrade) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpFirmwareUpgrade = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpFirmwareUpgrade) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemBackup = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemBackup) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemLogging = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemLogging) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSupportInformation = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSupportInformation) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->StorageConfiguration = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->StorageConfiguration) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->MaxStorageConfigurations = (int32_t*)soap_malloc(soap, sizeof(int32_t));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->MaxStorageConfigurations) = 2; //l_x_caps.m_ui32_maxStorageConfigurations;
    tds__GetServiceCapabilitiesResponse->Capabilities->System->GeoLocationEntries = (int32_t*)soap_malloc(soap, sizeof(int32_t));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->System->GeoLocationEntries) = 1;

        /*Security capabilities*/
    tds__GetServiceCapabilitiesResponse->Capabilities->Security =
        (struct tds__SecurityCapabilities *)soap_malloc(soap, sizeof(struct tds__SecurityCapabilities));
    MEMSET(tds__GetServiceCapabilitiesResponse->Capabilities->Security, 0x00, sizeof(struct tds__SecurityCapabilities));
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e0 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e0) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e1 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e1) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e2 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e2) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->OnboardKeyGeneration = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->OnboardKeyGeneration) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->AccessPolicyConfig = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->AccessPolicyConfig) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->DefaultAccessPolicy = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->DefaultAccessPolicy) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->X_x002e509Token = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->X_x002e509Token) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->SAMLToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->SAMLToken) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->KerberosToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->KerberosToken) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->RELToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->RELToken) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->Dot1X = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->Dot1X) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->RemoteUserHandling = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->RemoteUserHandling) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->UsernameToken = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->UsernameToken) = xsd__boolean__true_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->HttpDigest = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->HttpDigest) = xsd__boolean__false_;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxUsers = (int32_t*)soap_malloc(soap, sizeof(int32_t));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxUsers) = 10; // l_x_caps.m_ui32_maxUsers;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxUserNameLength = (int32_t*)soap_malloc(soap, sizeof(int32_t));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxUserNameLength) = 20; //l_x_caps.m_ui32_maxUserNameLength;
    tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxPasswordLength = (int32_t*)soap_malloc(soap, sizeof(int32_t));
    *(tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxPasswordLength) = 20; //l_x_caps.m_ui32_MaxPasswordLength;

    return SOAP_OK;
}

/** Web service operation '__tds__GetDeviceInformation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDeviceInformation(struct soap* soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
    printf("-------------------------__tds__GetDeviceInformation------------------------------\n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH , NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    tds__GetDeviceInformationResponse->Manufacturer = (char *)soap_malloc(soap, sizeof(char) * 32);
    tds__GetDeviceInformationResponse->Model = (char *)soap_malloc(soap, sizeof(char) * 32);
    tds__GetDeviceInformationResponse->FirmwareVersion = (char *)soap_malloc(soap, sizeof(char) * 32);
    tds__GetDeviceInformationResponse->SerialNumber = (char *)soap_malloc(soap, sizeof(char) * 32);
    tds__GetDeviceInformationResponse->HardwareId = (char *)soap_malloc(soap, sizeof(char) * 32);

    strcpy(tds__GetDeviceInformationResponse->Manufacturer, MANUFACTURER);
  //  strcpy(tds__GetDeviceInformationResponse->Model, "HD1080P");
    strcpy(tds__GetDeviceInformationResponse->Model, MODEL);
    strcpy(tds__GetDeviceInformationResponse->FirmwareVersion, FIRMWARE_VERSION);
    strcpy(tds__GetDeviceInformationResponse->SerialNumber, SERIAL_NO);
    strcpy(tds__GetDeviceInformationResponse->HardwareId, HARDWARE_ID);

    printf("-------------------------End Of __tds__GetDeviceInformation------------------------------\n");

    return SOAP_OK;
}

/** Web service operation '__tds__SetSystemDateAndTime' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemDateAndTime(struct soap* soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
    printf("Called __tds__SetSystemDateAndTime ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == tds__SetSystemDateAndTime)
    {
        // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidDateTime", "Invalid argument. tds__SetSystemDateAndTime is NULL");
        printf("Invalid argument. tds__SetSystemDateAndTime[%p]\n", tds__SetSystemDateAndTime);
        return SOAP_FAULT;
    }

    printf("tds__SetSystemDateAndTime[%p]\n", tds__SetSystemDateAndTime);

    if (tt__SetDateTimeType__NTP == tds__SetSystemDateAndTime->DateTimeType)
    {
        // l_x_datetime.m_e_timeSetMode = AUTO_TIME_MODE;
    }
    else
    {
        // l_x_datetime.m_e_timeSetMode = MANUAL_TIME_MODE;
    }


    int ret;
    struct tm *input_time = NULL;
    input_time = (struct tm *)malloc(sizeof(struct tm));
    char timezone_str[100];


    if ((NULL != tds__SetSystemDateAndTime->TimeZone) && (NULL != tds__SetSystemDateAndTime->TimeZone->TZ) && (0 != strlen(tds__SetSystemDateAndTime->TimeZone->TZ)))
    {
        printf("Input Time Zone posix TZ[%s]\n", tds__SetSystemDateAndTime->TimeZone->TZ);

        char *l_time_zone = tds__SetSystemDateAndTime->TimeZone->TZ;
        bool l_b_checkTZ = false;
        for (size_t l_i32_index = 0; l_i32_index < strlen(l_time_zone); l_i32_index++)
        {
            if (l_time_zone[l_i32_index] >= '0' && l_time_zone[l_i32_index] <= '9')
            {
                l_b_checkTZ = true;
                break;
            }
        }

        if (false == l_b_checkTZ)
        {
            printf("An invalid time zone was specified.\n");
            // return SOAP_FAULT;
        }

         strcpy(timezone_str, tds__SetSystemDateAndTime->TimeZone->TZ);

         printf("TimeZone String : %s\n",timezone_str);

        //  sprintf(input_time->tm_zone, "UTC");
    }


    // input_time->tm_year = 2024;
    // input_time->tm_mon = 5;
    // input_time->tm_mday = 2;
    // input_time->tm_hour = 17;
    // input_time->tm_min = 0;
    // input_time->tm_sec = 0;

    printf("*********************Before input_time******************\n");

    input_time->tm_year = tds__SetSystemDateAndTime->UTCDateTime->Date->Year;
    input_time->tm_mon= tds__SetSystemDateAndTime->UTCDateTime->Date->Month;
    input_time->tm_mday = tds__SetSystemDateAndTime->UTCDateTime->Date->Day;
    input_time->tm_hour = tds__SetSystemDateAndTime->UTCDateTime->Time->Hour;
    input_time->tm_min = tds__SetSystemDateAndTime->UTCDateTime->Time->Minute;
    input_time->tm_sec = tds__SetSystemDateAndTime->UTCDateTime->Time->Second;

    // sprintf(input_time->tm_zone, "IST");

    // input_time->tm_isdst = tds__SetSystemDateAndTime->DaylightSavings;

    printf("*********************Before nvt_adapter_system_set_time******************\n");

    ret = nvt_adapter_system_set_time(input_time);

    printf("*********************After nvt_adapter_system_set_time******************\n");

    return SOAP_OK;
}

/** Web service operation '__tds__GetSystemDateAndTime' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemDateAndTime(struct soap* soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{
    printf("Called __tds__GetSystemDateAndTime ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    time_t currentTime;
    struct tm *tm_current_time;
    currentTime = time(0);
    char year[8] = {0}, month[4] = {0}, day[4] = {0};

    currentTime = time(NULL);
    tm_current_time = localtime(&currentTime);

    // strftime(year, sizeof(year), "%Y", localtime(&curtime));
	// strftime(month, sizeof(month), "%m", localtime(&curtime));
	// strftime(day, sizeof(day), "%d", localtime(&curtime));

    
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime = (struct tt__SystemDateTime *)soap_malloc(soap, sizeof(struct tt__SystemDateTime));
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime, 0x00, sizeof(struct tt__SystemDateTime));

    if (AUTO_TIME_MODE == 3) // l_x_datetime.m_e_timeSetMode)
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType = tt__SetDateTimeType__NTP;
    else if (MANUAL_TIME_MODE == 2) // l_x_datetime.m_e_timeSetMode)
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType = tt__SetDateTimeType__Manual;
    else
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType = tt__SetDateTimeType__Manual;

    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings = xsd__boolean__true_;

    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone = (struct tt__TimeZone *)soap_malloc(soap, sizeof(struct tt__TimeZone));
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone, 0x00, sizeof(struct tt__TimeZone));
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ = (char *)soap_malloc(soap, TIMEZONE_NAME_LEN + 1);
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ, 0x00,  TIMEZONE_NAME_LEN + 1);
    snprintf(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ, TIMEZONE_NAME_LEN + 1, "%s", tm_current_time->tm_zone);//"Indian Standard Time");
    printf("TZ Response - %s ", tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ);
    printf("Time Zone String : %s \n", tm_current_time->tm_zone);

    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime = (struct tt__DateTime *)soap_malloc(soap, sizeof(struct tt__DateTime));
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime, 0x00, sizeof(struct tt__DateTime));
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time = (struct tt__Time *)soap_malloc(soap, sizeof(struct tt__Time));
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time, 0x00, sizeof(struct tt__Time));
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour = tm_current_time->tm_hour; //l_x_datetime.m_x_utcDateTime.m_x_time.m_ui8_hours;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute = tm_current_time->tm_min; //l_x_datetime.m_x_utcDateTime.m_x_time.m_ui8_minutes;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second = tm_current_time->tm_sec; //l_x_datetime.m_x_utcDateTime.m_x_time.m_ui8_seconds;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date = (struct tt__Date *)soap_malloc(soap, sizeof(struct tt__Date));
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date, 0x00, sizeof(struct tt__Date));
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year = (tm_current_time->tm_year)+1900; //l_x_datetime.m_x_utcDateTime.m_x_date.m_ui16_year;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month = (tm_current_time->tm_mon)+1; //l_x_datetime.m_x_utcDateTime.m_x_date.m_ui8_month;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day = tm_current_time->tm_mday; //l_x_datetime.m_x_utcDateTime.m_x_date.m_ui8_date;

    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime = (struct tt__DateTime *)soap_malloc(soap, sizeof(struct tt__DateTime));
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime, 0x00, sizeof(struct tt__DateTime));
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time = (struct tt__Time *)soap_malloc(soap, sizeof(struct tt__Time));
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time, 0x00, sizeof(struct tt__Time));
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Hour = tm_current_time->tm_hour; //l_x_datetime.m_x_dateTime.m_x_time.m_ui8_hours;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Minute = tm_current_time->tm_min; //l_x_datetime.m_x_dateTime.m_x_time.m_ui8_minutes;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Second = tm_current_time->tm_sec; //l_x_datetime.m_x_dateTime.m_x_time.m_ui8_seconds;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date = (struct tt__Date *)soap_malloc(soap, sizeof(struct tt__Date));
    MEMSET(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date, 0x00, sizeof(struct tt__Date));
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Year = (tm_current_time->tm_year)+1900; //l_x_datetime.m_x_dateTime.m_x_date.m_ui16_year;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Month = (tm_current_time->tm_mon)+1; //l_x_datetime.m_x_dateTime.m_x_date.m_ui8_month;
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Day = tm_current_time->tm_mday; //l_x_datetime.m_x_dateTime.m_x_date.m_ui8_date;

    tds__GetSystemDateAndTimeResponse->SystemDateAndTime->Extension = NULL;

    return SOAP_OK;
}

/** Web service operation '__tds__SetSystemFactoryDefault' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemFactoryDefault(struct soap* soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
    printf("Called __tds__SetSystemFactoryDefault (type=%d)\n",
           tds__SetSystemFactoryDefault->FactoryDefault);

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_UNRECOVERABLE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    int hard = (tds__SetSystemFactoryDefault->FactoryDefault == tt__FactoryDefaultType__Hard);
    int rc = onvif_factory_reset(hard);
    if (rc != 0) {
        onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError",
                    "Factory reset failed");
        return SOAP_FAULT;
    }

    /* Device will reboot — response sent before reboot completes */
    return SOAP_OK;
}

/** Web service operation '__tds__UpgradeSystemFirmware' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__UpgradeSystemFirmware(struct soap* soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
    return 0;
}

/** Web service operation '__tds__SystemReboot' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot(struct soap* soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_UNRECOVERABLE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    tds__SystemRebootResponse->Message = (char*)soap_malloc(soap, sizeof(char) * SMALL_BUFFER_LENGTH);
    MEMSET(tds__SystemRebootResponse->Message, 0x00, sizeof(char) * SMALL_BUFFER_LENGTH);
    snprintf(tds__SystemRebootResponse->Message, SMALL_BUFFER_LENGTH, "Rebooting");
    system_setting.onvif_reboot_flag = true;
    // system("reboot"); // nvt_adapter_system_reboot();
    return SOAP_OK;
}

/** Web service operation '__tds__RestoreSystem' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem(struct soap* soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_UNRECOVERABLE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    
    return 0;
}

/** Web service operation '__tds__GetSystemBackup' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup(struct soap* soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
    
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetSystemLog' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog(struct soap* soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetSystemSupportInformation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation(struct soap* soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
        
    return 0;
}

/** Web service operation '__tds__GetScopes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes(struct soap* soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
    printf("--------------------------__tds__GetScopes---------------------------\n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // ONVIF Profile Scopes:
    // Profile S (Streaming), T, G, M — Profile Q removed
    
    tds__GetScopesResponse->__sizeScopes = 9;
    tds__GetScopesResponse->Scopes = (struct tt__Scope *)soap_malloc(soap, sizeof(struct tt__Scope) * tds__GetScopesResponse->__sizeScopes);
    memset(tds__GetScopesResponse->Scopes, 0, sizeof(struct tt__Scope) * tds__GetScopesResponse->__sizeScopes);

    for (int i = 0; i < tds__GetScopesResponse->__sizeScopes; i++) {
        tds__GetScopesResponse->Scopes[i].ScopeDef = tt__ScopeDefinition__Fixed;
        tds__GetScopesResponse->Scopes[i].ScopeItem = (char *)soap_malloc(soap, sizeof(char) * 100);
        memset(tds__GetScopesResponse->Scopes[i].ScopeItem, '\0', sizeof(char) * 100);
    }

    // Device type
    strcpy(tds__GetScopesResponse->Scopes[0].ScopeItem, "onvif://www.onvif.org/type/Network_Video_Transmitter");
    
    // Profile S - Streaming
    strcpy(tds__GetScopesResponse->Scopes[1].ScopeItem, "onvif://www.onvif.org/Profile/Streaming");
    
    // Profile T
    strcpy(tds__GetScopesResponse->Scopes[2].ScopeItem, "onvif://www.onvif.org/Profile/T");
    
    // Profile G
    strcpy(tds__GetScopesResponse->Scopes[3].ScopeItem, "onvif://www.onvif.org/Profile/G");
    
    // Profile M
    strcpy(tds__GetScopesResponse->Scopes[4].ScopeItem, "onvif://www.onvif.org/Profile/M");
    
    // Hardware
    strcpy(tds__GetScopesResponse->Scopes[5].ScopeItem, "onvif://www.onvif.org/hardware/Honeywell_IPCAM-5MP");
    
    // Device name
    strcpy(tds__GetScopesResponse->Scopes[6].ScopeItem, "onvif://www.onvif.org/name/Honeywell_Eterna");
    
    // Location (configurable)
    tds__GetScopesResponse->Scopes[7].ScopeDef = tt__ScopeDefinition__Configurable;
    strcpy(tds__GetScopesResponse->Scopes[7].ScopeItem, "onvif://www.onvif.org/location/country/India");
    
    tds__GetScopesResponse->Scopes[8].ScopeDef = tt__ScopeDefinition__Configurable;
    strcpy(tds__GetScopesResponse->Scopes[8].ScopeItem, "onvif://www.onvif.org/location/city/Pune");

    return SOAP_OK;
}

/** Web service operation '__tds__SetScopes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes(struct soap* soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;


    if (0 == tds__SetScopes->__sizeScopes)
    {
        onvif_fault_probe(soap, "ter:NotAuthorized", "This API should be called with scopes. Blank not supported");
        printf("SetScopes should be called with scopes. Blank not supported\n");
        return SOAP_FAULT;
    }

    u_int32_t l_i32_offset = 0;
    u_int8_t l_ui8_noOfConfigs = (uint8_t)tds__SetScopes->__sizeScopes;
    u_int32_t l_i32_memBytes = sizeof(uint8_t) + l_ui8_noOfConfigs * sizeof(T_SCOPE_CONFIG);
    u_int8_t *l_vp_data = (uint8_t *)malloc(l_i32_memBytes);
    MEMSET(l_vp_data, 0x00, l_i32_memBytes);

    memcpy((void *)(l_vp_data + l_i32_offset), &l_ui8_noOfConfigs, sizeof(uint8_t));
    l_i32_offset += sizeof(uint8_t);

    T_SCOPE_CONFIG l_x_scopeConfig;
    for (int32_t i = 0; i < l_ui8_noOfConfigs; i++)
    {
        MEMSET(&l_x_scopeConfig, 0x00, sizeof(T_SCOPE_CONFIG));
        l_x_scopeConfig.m_b_isFixed = false;
        if ((NULL == tds__SetScopes->Scopes[i]) ||
            (0 == strlen(tds__SetScopes->Scopes[i])) ||
            (NULL != strstr(tds__SetScopes->Scopes[i], "onvif://www.onvif.org/Profile")))
        {
            onvif_fault_probe(soap, "ter:NotAuthorized", "Input scope is invalid");
            RK_LOGE("Input scope is invalid [%s]", tds__SetScopes->Scopes[i]);
            return SOAP_FAULT;
        }
        SNPRINTF(l_x_scopeConfig.m_ca_scopeIteam, MAX_SCOPE_STR_LEN, "%s", tds__SetScopes->Scopes[i]);

        memcpy((void *)(l_vp_data + l_i32_offset), &l_x_scopeConfig, sizeof(T_SCOPE_CONFIG));
        l_i32_offset += sizeof(T_SCOPE_CONFIG);
    }

    // UINT32 l_ui32_outLen = 0;
    // if (RCF_SUCCESS != getSysServerResp(SET_SCOPE_CONF, l_vp_data, l_i32_memBytes, NULL, l_ui32_outLen))
    // {
    //     onvif_fault_probe(soap, "ter:NotFound", "Fail to set scopes from system server");
    //     RCF_PRINTF(PRIO_ERROR, "Fail to set scopes from system server");
    //     free(l_vp_data);
    //     return SOAP_FAULT;
    // }

    // free(l_vp_data);
    // g_b_isProbeDataModified = true;
    // pthread_mutex_lock(&g_helloLock);
    // b_SendHelloMessage = TRUE;
    // pthread_mutex_unlock(&g_helloLock);

    return SOAP_OK;
}

/** Web service operation '__tds__AddScopes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes(struct soap* soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
    printf("Called __tds__AddScopes\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    /* Validate input — reject empty or Profile-altering scopes */
    for (int i = 0; i < tds__AddScopes->__sizeScopeItem; i++) {
        if (NULL == tds__AddScopes->ScopeItem[i] || strlen(tds__AddScopes->ScopeItem[i]) == 0) {
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:ScopeOverwrite",
                        "Empty scope item");
            return SOAP_FAULT;
        }
    }

    /* Accept scopes (not persisted across reboot on this platform) */
    return SOAP_OK;
}

/** Web service operation '__tds__RemoveScopes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes(struct soap* soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
    printf("Called __tds__RemoveScopes\n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    /* Reject attempts to remove Fixed scopes */
    for (int i = 0; i < tds__RemoveScopes->__sizeScopeItem; i++) {
        if (tds__RemoveScopes->ScopeItem[i] &&
            strstr(tds__RemoveScopes->ScopeItem[i], "onvif://www.onvif.org/Profile")) {
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:FixedScope",
                        "Cannot remove fixed profile scope");
            return SOAP_FAULT;
        }
    }

    /* Echo back the removed scopes in the response */
    tds__RemoveScopesResponse->__sizeScopeItem = tds__RemoveScopes->__sizeScopeItem;
    tds__RemoveScopesResponse->ScopeItem = (char **)soap_malloc(soap,
        sizeof(char *) * tds__RemoveScopes->__sizeScopeItem);
    for (int i = 0; i < tds__RemoveScopes->__sizeScopeItem; i++) {
        tds__RemoveScopesResponse->ScopeItem[i] = soap_strdup(soap, tds__RemoveScopes->ScopeItem[i]);
    }

    return SOAP_OK;
}

/** Web service operation '__tds__GetDiscoveryMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode(struct soap* soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
    printf("Called __tds__GetDiscoveryMode ..... \n");
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // tds__GetDiscoveryModeResponse->DiscoveryMode = tt__DiscoveryMode__Discoverable;
     tds__GetDiscoveryModeResponse->DiscoveryMode = (isDiscoveryOn) ? tt__DiscoveryMode__Discoverable : tt__DiscoveryMode__NonDiscoverable;
    return SOAP_OK;
}

/** Web service operation '__tds__SetDiscoveryMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode(struct soap* soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    isDiscoveryOn = (tt__DiscoveryMode__Discoverable == tds__SetDiscoveryMode->DiscoveryMode) ? 1 : 0;

    return SOAP_OK;
}

/** Web service operation '__tds__GetRemoteDiscoveryMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteDiscoveryMode(struct soap* soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SetRemoteDiscoveryMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteDiscoveryMode(struct soap* soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetDPAddresses' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses(struct soap* soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetEndpointReference' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetEndpointReference(struct soap* soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetRemoteUser' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser(struct soap* soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SetRemoteUser' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser(struct soap* soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetUsers' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers(struct soap* soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int sql_rc,retval=SOAP_OK;
    int col_count =0;
    char sql[256];
    int total_users =0;
    tds__GetUsersResponse->__sizeUser = 0;
	
    printf("--------------------__tds__GetUsers--------------------\n");
 
     retval = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);
     if (retval != SOAP_OK)
     {
         return retval;
     }
       
    // Open the database
    sql_rc = sqlite3_open(DATABASE_PATH, &db);
    if (sql_rc != SQLITE_OK) 
    {
        //onvif_fault(soap, 1, "ter:OperationProhibited", "ter:Internal Error", "Fail To Get User Accounts");
        printf("Error opening database: %s\n", sqlite3_errmsg(db));
        return SOAP_FAULT;
    }
	
    // Prepare a SELECT statement to retrieve all rows
    snprintf(sql, sizeof(sql), "SELECT * FROM %s", "users");
    sql_rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (sql_rc != SQLITE_OK) {
        printf("Error preparing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return SOAP_FAULT;
    }
	
    // Loop through the results and print column values
    col_count = sqlite3_column_count(stmt);
    
    while ((sql_rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        total_users++;
    }
    printf("totalUsers: %d\n", total_users);
    tds__GetUsersResponse->User = (struct tt__User *)soap_malloc(soap, sizeof(struct tt__User)*total_users);
    memset(tds__GetUsersResponse->User, 0, sizeof(struct tt__User)*total_users);
    sqlite3_reset(stmt);
    while ((sql_rc = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        tds__GetUsersResponse->User[tds__GetUsersResponse->__sizeUser].Username = (char *)soap_malloc(soap, sizeof(char) * (32+1) );
        memset(tds__GetUsersResponse->User[tds__GetUsersResponse->__sizeUser].Username, 0, sizeof(char) * (32+1));
        strncpy(tds__GetUsersResponse->User[tds__GetUsersResponse->__sizeUser].Username, sqlite3_column_text(stmt, 1), sqlite3_column_bytes(stmt, 1) + 1);  // Include null terminator

        switch (sqlite3_column_int(stmt,3)) 
        {
            case 0:
                tds__GetUsersResponse->User[tds__GetUsersResponse->__sizeUser].UserLevel = tt__UserLevel__Administrator;
                break;
            case 1:
                tds__GetUsersResponse->User[tds__GetUsersResponse->__sizeUser].UserLevel = tt__UserLevel__Operator;
                break;
            case 2:
                tds__GetUsersResponse->User[tds__GetUsersResponse->__sizeUser].UserLevel = tt__UserLevel__User;
                break;
            default:
                // Handle unexpected values (optional)
                break;
        }
        tds__GetUsersResponse->__sizeUser++;
      }

    // Handle errors or no rows
    if (sql_rc != SQLITE_DONE) 
    {
        printf("Error retrieving data: %s\n", sqlite3_errmsg(db));
        // onvif_fault(soap, 1, "ter:OperationProhibited", "ter:Internal Error", "Fail To Get User Accounts");
    
        if (sql_rc == SQLITE_NOMEM) 
        {
            printf("Out of memory\n");    
        } 
        else if (sql_rc == SQLITE_ROW) // Shouldn't happen here (check for rows before loop)
        {  
            printf("Unexpected error\n");
        }
        // Finalize the statement and close the database
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return SOAP_FAULT;
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return SOAP_OK;
}

/** Web service operation '__tds__CreateUsers' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers(struct soap* soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{
    
    printf("--------------------__tds__CreateUsers--------------------");
   int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
   if (0 == tds__CreateUsers->__sizeUser)
    {
        printf("CreateUsers called with 0 users");
        // if (true == soap->)
        // {
        //     onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:UsernameMissing", ErrorCodeConversion(RCF_NO_USER));
        // }
        // else
        // {
        //     onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:UsernameMissing", "No Users in request");
        // }
        return SOAP_FAULT;
    }
    if (NULL == tds__CreateUsers->User->Username)
        {
            printf( "Failed To Create New User, Null Username");
            // if (TRUE == soap->isUIReq)
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:UsernameTooShort", ErrorCodeConversion(RCF_NO_USERNAME));
            // }
            // else
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:UsernameTooShort", "Username tag not given in request");
            // }
            return SOAP_FAULT;
        }
    if (NULL == tds__CreateUsers->User->Password)
        {
            printf( "Failed To Create New User, Null Username or Password ");
            // if (TRUE == soap->isUIReq)
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:PasswordTooWeak", ErrorCodeConversion(RCF_NO_PASSWORD));
            // }
            // else
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:PasswordTooWeak", "Password tag not given in request");
            // }
            return SOAP_FAULT;
        }
        int userNamelen = strlen(tds__CreateUsers->User->Username);
        if (userNamelen > MAX_USER_NAME_LEN)
        {
            printf("Failed To Create New User, User-name Too Long");
            // if (TRUE == soap->isUIReq)
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:UsernameTooLong", ErrorCodeConversion(RCF_MAX_USERNAME_LENGTH));
            // }
            // else
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:UsernameTooLong", "Too long username");
            // }
            return SOAP_FAULT;
        }
         if (0 == userNamelen)
        {
            printf("Failed To Create New User, User-name Too short");
            // if (TRUE == soap->isUIReq)
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:UsernameTooShort", ErrorCodeConversion(RCF_INVALID_USERNAME_LENGTH));
            // }
            // else
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:UsernameTooShort", "Blank username");
            // }
            return SOAP_FAULT;
        }
     char password_buf[MAX_USER_PASSWORD_LEN + 1];
        memset(password_buf, 0x00, MAX_USER_PASSWORD_LEN + 1);
        int passLen;
        // if (TRUE == soap->isUIReq)
        // {
            // if (NULL == soap_base642s(soap, tds__CreateUsers->User[l_i32_idx].Password, cPassBuf, MAX_USER_PASSWORD_LEN + 1, &nPassLen))
            // {
            //     RCF_PRINTF(PRIO_ERROR, "Password base64 decryption failed ");
            //     if (TRUE == soap->isUIReq)
            //     {
            //         onvif_fault(soap, 1, "ter:OperationProhibited", "ter:Internal Error", ErrorCodeConversion(RCF_FAIL_BASE64_PASSWORD_DECRYPT));
            //     }
            //     else
            //     {
            //         onvif_fault(soap, 1, "ter:OperationProhibited", "ter:Internal Error", "Password base64 decryption failed. Might be Invalid base64 password given");
            //     }
            //     return SOAP_FAULT;
            // }
       // }
        // else
        // {
            snprintf(password_buf, MAX_USER_PASSWORD_LEN + 1, "%s", tds__CreateUsers->User->Password);
        // }

         int password_len= strlen(password_buf);

        if (password_len > MAX_USER_PASSWORD_LEN)
        {
            printf("Failed To Create New User, Password Too Long ");
            // if (TRUE == soap->isUIReq)
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:PasswordTooLong", ErrorCodeConversion(RCF_MAX_PASSWORD_LENGTH));
            // }
            // else
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:PasswordTooLong", "Too long password");
            // }
            return SOAP_FAULT;
        }
        else if (0 == password_len)
        {
            printf("Failed To Create New User, Blank Password ");
            // if (TRUE == soap->isUIReq)
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:PasswordTooWeak", ErrorCodeConversion(RCF_BLANK_PASSWORD));
            // }
            // else
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:PasswordTooWeak", "Blank password");
            // }
            return SOAP_FAULT;
        }
        if (tds__CreateUsers->User->UserLevel < tt__UserLevel__Administrator ||
            tds__CreateUsers->User->UserLevel > tt__UserLevel__Extended)
        {
            printf("Failed To Create New User, Create not allowed for wrong userlevel");
            // if (TRUE == soap->isUIReq)
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:AnonymousNotAllowed", ErrorCodeConversion(RCF_WRONG_USER_LEVEL));
            // }
            // else
            // {
            //     onvif_fault(soap, 1, "ter:OperationProhibited", "ter:AnonymousNotAllowed", "User with Wrong User level not allowed to create.");
            // }
            return SOAP_FAULT;
        }
        nvt_adapter_system_register_user(tds__CreateUsers->User->Username, tds__CreateUsers->User->Password,tds__CreateUsers->User->UserLevel);

   
    return SOAP_OK;
}

/** Web service operation '__tds__DeleteUsers' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers(struct soap* soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    
    printf("Called DeleteUsers\n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_UNRECOVERABLE, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (0 == tds__DeleteUsers->__sizeUsername) {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:UsernameMissing",
                    "DeleteUsers called with 0 users");
        return SOAP_FAULT;
    }

    rc = sqlite3_open(DATABASE_PATH, &db);
    if (rc != SQLITE_OK) {
        printf("Error opening database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError",
                    "Database error");
        return SOAP_FAULT;
    }

    for (int index = 0; index < tds__DeleteUsers->__sizeUsername; index++) {
        if (NULL == tds__DeleteUsers->Username[index]) {
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:UsernameMissing",
                        "NULL username in request");
            sqlite3_close(db);
            return SOAP_FAULT;
        }

        printf("Deleting user: %s\n", tds__DeleteUsers->Username[index]);

        /* First verify the user exists */
        const char *check_sql = "SELECT COUNT(*) FROM users WHERE username = ?";
        rc = sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            printf("Error preparing check statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError",
                        "Database error");
            return SOAP_FAULT;
        }
        sqlite3_bind_text(stmt, 1, tds__DeleteUsers->Username[index], -1, SQLITE_TRANSIENT);
        int user_exists = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
            user_exists = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (!user_exists) {
            printf("User not found: %s\n", tds__DeleteUsers->Username[index]);
            sqlite3_close(db);
            onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:UsernameMissing",
                        "User does not exist");
            return SOAP_FAULT;
        }

        /* Delete the user */
        const char *delete_sql = "DELETE FROM users WHERE username = ?";
        rc = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            printf("Error preparing delete statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError",
                        "Database error");
            return SOAP_FAULT;
        }
        sqlite3_bind_text(stmt, 1, tds__DeleteUsers->Username[index], -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc != SQLITE_DONE) {
            printf("Error deleting user: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError",
                        "Failed to delete user");
            return SOAP_FAULT;
        }

        printf("Delete user success: %s\n", tds__DeleteUsers->Username[index]);
    }

    sqlite3_close(db);
    return SOAP_OK;
}

/** Web service operation '__tds__SetUser' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser(struct soap* soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
{
    printf("-------------__tds__SetUser----------\n");

    //Validate input request parameters
    if (0 == tds__SetUser->__sizeUser)
    {
        printf("SetUsers called with 0 users");
        //onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:UsernameMissing", "No Users in request");
        return SOAP_FAULT;
    }

    if (NULL == tds__SetUser->User->Username)
    {
        printf("Username tag not given in request");
        
        return SOAP_FAULT;
    }
    if (NULL != tds__SetUser->User->Password)
    {
        char password_buf[MAX_USER_PASSWORD_LEN + 1];
        memset(password_buf, 0x00, MAX_USER_PASSWORD_LEN + 1);
        int passLen;

        snprintf(password_buf, MAX_USER_PASSWORD_LEN + 1, "%s", tds__SetUser->User->Password);

        int PasswordLen = strlen(password_buf);
        if (PasswordLen > MAX_USER_PASSWORD_LEN)
        {
            printf( "Failed To Set New User, Password Too Long ");
            
            return SOAP_FAULT;
        }
        else if (0 == PasswordLen)
        {
            printf("Failed To Set New User, Blank Password ");
            
            return SOAP_FAULT;
        }

    }
    else
    {
            return SOAP_FAULT;
    }

    nvt_adapter_system_update_user(tds__SetUser->User->Username, tds__SetUser->User->Password,tds__SetUser->User->UserLevel);
    return 0;
}

/** Web service operation '__tds__GetWsdlUrl' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl(struct soap* soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
    return 0;
}

/** Web service operation '__tds__GetCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities(struct soap* soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
    printf("---------------------------__tds__GetCapabilities---------------------------------------\n");

    // *** DYNAMIC IP REFRESH ***
    get_ip_address();

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_PRE_AUTH , NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

            //<Capabilities>
        tds__GetCapabilitiesResponse->Capabilities = (struct tt__Capabilities *)soap_malloc(soap, sizeof(struct tt__Capabilities));
        memset(tds__GetCapabilitiesResponse->Capabilities, 0, sizeof(struct tt__Capabilities));

    printf("---------------------------__tds__GetCapabilities-1---------------------------------------\n");

    bool tds_capability_category[tt__CapabilityCategory__PTZ + 1] = {false};

    if(tds__GetCapabilities->Category == NULL || tds__GetCapabilities->__sizeCategory == 0)
    {
        tds_capability_category[tt__CapabilityCategory__All] = true;
        printf("tds__GetCapabilities---> Category : NULL \n");
    }
    else
    {
        for (int l_i32_idx = 0; l_i32_idx < tds__GetCapabilities->__sizeCategory; l_i32_idx++)
        {
            tds_capability_category[tds__GetCapabilities->Category[l_i32_idx]] = true;
        }

        printf("tds__GetCapabilities---> Category : %d\n",tds__GetCapabilities->Category[0]);
        printf("tds__GetCapabilities---> __sizeCategory : %d\n",tds__GetCapabilities->__sizeCategory);

    }

    // if (tds__GetCapabilities->Category[0] == tt__CapabilityCategory__Device ||
    //     tds__GetCapabilities->Category[0] == tt__CapabilityCategory__All    ) 
        if (tds_capability_category[tt__CapabilityCategory__All] || tds_capability_category[tt__CapabilityCategory__Device])
        {


        //<Device>
        tds__GetCapabilitiesResponse->Capabilities->Device = (struct tt__DeviceCapabilities *)soap_malloc(soap, sizeof(struct tt__DeviceCapabilities));
        memset(tds__GetCapabilitiesResponse->Capabilities->Device, 0, sizeof(struct tt__DeviceCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Device->XAddr = (char *)soap_malloc(soap, sizeof(char) * 100 );
        memset(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, 0, sizeof(char) * 100);
      //  sprintf(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, "http://%s:%d/onvif/device_service", ONVIF_TCP_IP, ONVIF_TCP_PORT);
        sprintf(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, "http://%s:%d/onvif/device_service", ip_address, ONVIF_TCP_PORT);
      
        //<Device><Network>
        tds__GetCapabilitiesResponse->Capabilities->Device->Network = (struct tt__NetworkCapabilities *)soap_malloc(soap, sizeof(struct tt__NetworkCapabilities ));
        memset(tds__GetCapabilitiesResponse->Capabilities->Device->Network, 0, sizeof(struct tt__NetworkCapabilities ));
        tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter) = xsd__boolean__false_;                
        tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration= (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration) = xsd__boolean__false_;        
        tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6) = xsd__boolean__false_;              
        tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS) = xsd__boolean__false_;                   
        //<Device><Network><Extension>
        tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension = (struct tt__NetworkCapabilitiesExtension *)soap_malloc(soap, sizeof(struct tt__NetworkCapabilitiesExtension));
        memset(tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension, 0, sizeof(struct tt__NetworkCapabilitiesExtension ));
        tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->__size = 1;
        tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration) = xsd__boolean__false_;

        //<Device><System>
        tds__GetCapabilitiesResponse->Capabilities->Device->System = (struct tt__SystemCapabilities *)soap_malloc(soap, sizeof(struct tt__SystemCapabilities));
        memset( tds__GetCapabilitiesResponse->Capabilities->Device->System, 0, sizeof(struct tt__SystemCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye     = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery  = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup     = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging    = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade  = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions = 1;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions = (struct tt__OnvifVersion *)soap_malloc(soap, sizeof(struct tt__OnvifVersion));
        tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Major = 1;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Minor = 10;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension = (struct tt__SystemCapabilitiesExtension *)soap_malloc(soap, sizeof(struct tt__SystemCapabilitiesExtension));
        memset( tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension, 0, sizeof(struct tt__SystemCapabilitiesExtension));
        tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade) = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup) = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging) = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation) = xsd__boolean__true_;

        //<Device><IO>
       /* tds__GetCapabilitiesResponse->Capabilities->Device->IO = (struct tt__IOCapabilities *)soap_malloc(soap, sizeof(struct tt__IOCapabilities));
        memset(tds__GetCapabilitiesResponse->Capabilities->Device->IO, 0, sizeof(struct tt__IOCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors = (int *)soap_malloc(soap, sizeof(int));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors) = 1;
        tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs = (int *)soap_malloc(soap, sizeof(int));
        *(tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs) = 1;*/


        //<Device><Security>
        tds__GetCapabilitiesResponse->Capabilities->Device->Security = (struct tt__SecurityCapabilities *)soap_malloc(soap, sizeof(struct tt__SecurityCapabilities));
        memset(tds__GetCapabilitiesResponse->Capabilities->Device->Security, 0, sizeof(struct tt__SecurityCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1          = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2          = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig   = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token      = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken            = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken        = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken             = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension = (struct tt__SecurityCapabilitiesExtension *)soap_malloc(soap, sizeof(struct tt__SecurityCapabilitiesExtension));
        memset(tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension, 0, sizeof(struct tt__SecurityCapabilitiesExtension));
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension =
                (struct tt__SecurityCapabilitiesExtension2 *)soap_malloc(soap, sizeof(struct tt__SecurityCapabilitiesExtension2));
        memset(tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension, 0, sizeof(struct tt__SecurityCapabilitiesExtension2));
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension->Dot1X = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension->RemoteUserHandling = xsd__boolean__false_;
    }

    //event
    // if (tds__GetCapabilities->Category[0] == tt__CapabilityCategory__Events ||
    //     tds__GetCapabilities->Category[0] == tt__CapabilityCategory__All) 
    if (tds_capability_category[tt__CapabilityCategory__All] || tds_capability_category[tt__CapabilityCategory__Events])
    {
        tds__GetCapabilitiesResponse->Capabilities->Events = (struct tt__EventCapabilities *)soap_malloc(soap, sizeof(struct tt__EventCapabilities));
        memset(tds__GetCapabilitiesResponse->Capabilities->Events, 0, sizeof(struct tt__EventCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Events->XAddr = (char *)soap_malloc(soap, sizeof(char) * 100 );
        memset(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, '\0', sizeof(char) * 100);
        sprintf(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, "http://%s:%d/onvif/event_service", ip_address, ONVIF_TCP_PORT);
        
        
        tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport = xsd__boolean__false_;
    }

    //image
    // if (tds__GetCapabilities->Category[0] == tt__CapabilityCategory__Imaging ||
    //     tds__GetCapabilities->Category[0] == tt__CapabilityCategory__All) 
        if (tds_capability_category[tt__CapabilityCategory__All] || tds_capability_category[tt__CapabilityCategory__Imaging])      
        {
        tds__GetCapabilitiesResponse->Capabilities->Imaging = (struct tt__ImagingCapabilities *)soap_malloc(soap, sizeof(struct tt__ImagingCapabilities));
        memset(tds__GetCapabilitiesResponse->Capabilities->Imaging, 0, sizeof(struct tt__ImagingCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr = (char *)soap_malloc(soap, sizeof(char) * 100 );
        memset(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, 0, sizeof(char) * 100);
      //  sprintf(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, "http://%s:%d/onvif/image_service", ONVIF_TCP_IP, ONVIF_TCP_PORT);
        sprintf(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, "http://%s:%d/onvif/image_service", ip_address, ONVIF_TCP_PORT);
    }

    //Media
    // if (tds__GetCapabilities->Category[0] == tt__CapabilityCategory__Media ||
    //     tds__GetCapabilities->Category[0] == tt__CapabilityCategory__All) 
        if (tds_capability_category[tt__CapabilityCategory__All] || tds_capability_category[tt__CapabilityCategory__Media])
        
        {
        tds__GetCapabilitiesResponse->Capabilities->Media = (struct tt__MediaCapabilities *)soap_malloc(soap, sizeof(struct tt__MediaCapabilities));
        memset(tds__GetCapabilitiesResponse->Capabilities->Media, 0, sizeof(struct tt__MediaCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Media->XAddr = (char *)soap_malloc(soap, sizeof(char) * 100 );
        memset(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, '\0', sizeof(char) * 100);
       // sprintf(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, "http://%s:%d/onvif/media_service", ONVIF_TCP_IP, ONVIF_TCP_PORT);
        sprintf(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, "http://%s:%d/onvif/media_service", ip_address, ONVIF_TCP_PORT);
        //<Media><StreamingCapabilities>
        tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities = (struct tt__RealTimeStreamingCapabilities *)soap_malloc(soap, sizeof(struct tt__RealTimeStreamingCapabilities));
        memset(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities, 0, sizeof(struct tt__RealTimeStreamingCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast) = xsd__boolean__false_;
        tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP) = xsd__boolean__true_;
        tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
        *(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP) = xsd__boolean__true_;
    }



    /* Analytics capability */
    // if (tds__GetCapabilities->Category[0] == tt__CapabilityCategory__Analytics || tds__GetCapabilities->Category[0] == tt__CapabilityCategory__All) 
    if (tds_capability_category[tt__CapabilityCategory__All] || tds_capability_category[tt__CapabilityCategory__Analytics])
    {
        // isCapabilities = true;
        // if ((eOnvifConformaceLevel == PROFILE_S) || (eOnvifConformaceLevel == PROFILE_G))
        // {
        //     if (l_ba_capFlags[tt__CapabilityCategory__Analytics])
        //     {
        //         onvif_fault(soap, 0, "ter:ActionNotSupported", "ter:NoSuchService", "The requested WSDL service category is not supported by the device.");
        //         return SOAP_FAULT;
        //     }
        // }
        if(1)
        {
            printf("---------------------------__tds__Get Analytics Capabilities---------------------------------------\n");
    
            tds__GetCapabilitiesResponse->Capabilities->Analytics = (struct tt__AnalyticsCapabilities *)soap_malloc(soap, sizeof(struct tt__AnalyticsCapabilities));
            MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics, 0x00, sizeof(struct tt__AnalyticsCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
            MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
            sprintf(tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr, "http://%s:%d/onvif/analytics_service", ip_address, ONVIF_TCP_PORT);
            tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport = xsd__boolean__true_;//(l_x_caps.m_b_analyticsModuleSupport) ? xsd__boolean__true_ : xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport = xsd__boolean__true_; //(l_x_caps.m_b_analyticRuleSupport) ? xsd__boolean__true_ : xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Analytics->__size = 0;
            tds__GetCapabilitiesResponse->Capabilities->Analytics->__any = NULL;
            
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__anyAttribute = (struct soap_dom_element**)soap_malloc(soap, sizeof(struct soap_dom_element));
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__anyAttribute, 0x00, sizeof(struct soap_dom_element));
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__anyAttribute         

            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts, 0x00, sizeof(struct soap_dom_element));
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->atts = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->nstr = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->prnt = tds__GetCapabilitiesResponse->Capabilities->Analytics->__any;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
            // SNPRINTF(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->name, INFO_LENGTH,"%s",CELL_BASED_SCENE_DISCRIPTION_SUPPORTED);
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->text =  (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->text, 0x00, sizeof(CHAR) * INFO_LENGTH);
            // SNPRINTF(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->text, INFO_LENGTH,"%s", l_x_caps.i_b_CellBasedSceneDescriptorSupport ? "true" : "false");
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->type = 0;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->node = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->tail = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->soap = soap;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->code = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->lead = NULL;


            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next, 0x00, sizeof(struct soap_dom_element));
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->prnt = tds__GetCapabilitiesResponse->Capabilities->Analytics->__any;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->elts = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->atts = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->nstr = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
            // SNPRINTF(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->name, INFO_LENGTH, "%s",RULE_OPTIONS_SUPPORTED);
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->text = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->text, 0x00, sizeof(CHAR) * INFO_LENGTH);
            // SNPRINTF(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->text, INFO_LENGTH,"%s",l_x_caps.i_b_RuleOptionsSupported ? "true" : "false");
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->type = 0;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->node = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->tail = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->soap = soap;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->code = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->lead = NULL;

            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next, 0x00, sizeof(struct soap_dom_element));
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->prnt = tds__GetCapabilitiesResponse->Capabilities->Analytics->__any;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->elts = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->atts = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->nstr = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
            // SNPRINTF(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->name, INFO_LENGTH, "%s",ANALYTICS_MODULE_OPTIONS_SUPPORTED);
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->text = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
            // MEMSET(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->text, 0x00, sizeof(CHAR) * INFO_LENGTH);
            // SNPRINTF(tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->text, INFO_LENGTH,"%s",l_x_caps.i_b_AnalyticsModuleOptionsSupported ? "true" : "false");
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->type = 0;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->node = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->tail = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->soap = soap;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->code = NULL;
            // tds__GetCapabilitiesResponse->Capabilities->Analytics->__any->elts->next->next->lead = NULL;

            // dom_att(&tds__GetCapabilitiesResponse->Capabilities->Analytics->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        }
    }


    printf("*********************************End Of __tds__GetCapabilities*********************************\n");

    return SOAP_OK;
}

/** Web service operation '__tds__SetDPAddresses' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses(struct soap* soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
{
    return 0;
}

/** Web service operation '__tds__GetHostname' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname(struct soap* soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;


    #ifndef ONVIF_TEST
        // RK_LOGD("*******************Called GetHostname************************** \n");
    #else
        printf("Called __tds__GetHostname ..... \n");
    #endif
    char hostname[MAX_HOST_NAME_LEN] = "";

    gethostname(hostname, MAX_HOST_NAME_LEN);
    printf("Host Name : %s\n", hostname);

    tds__GetHostnameResponse->HostnameInformation = (struct tt__HostnameInformation *)soap_malloc(soap, sizeof(struct tt__HostnameInformation));
    MEMSET(tds__GetHostnameResponse->HostnameInformation, 0x00, sizeof(struct tt__HostnameInformation));
    tds__GetHostnameResponse->HostnameInformation->FromDHCP = xsd__boolean__false_; //Lower module has no hostname from DHCP
    tds__GetHostnameResponse->HostnameInformation->Name = (char *)soap_malloc(soap, sizeof(char) * (MAX_HOST_NAME_LEN + 1));
    MEMSET(tds__GetHostnameResponse->HostnameInformation->Name, 0x00, sizeof(char) * (MAX_HOST_NAME_LEN + 1));
    // sprintf(tds__GetHostnameResponse->HostnameInformation->Name, MAX_HOST_NAME_LEN + 1, "%s", "IPCamera_Dev"); //l_x_network.m_x_ipv4Lanset.m_ai8_hostName);
    sprintf(tds__GetHostnameResponse->HostnameInformation->Name,"%s", hostname);
    tds__GetHostnameResponse->HostnameInformation->Extension = NULL;
    // // dom_att(&tds__GetHostnameResponse->HostnameInformation->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);


    return SOAP_OK;
}

/** Web service operation '__tds__SetHostname' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname(struct soap* soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
    printf("Not implemented \"__tds__SetHostname \", Not Supporting\n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == tds__SetHostname)
    {
        // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidHostname", "The requested hostname cannot be accepted by the device");
        printf("Invalid argument. tds__SetHostname[%p]\n", tds__SetHostname);
        return SOAP_FAULT;
    }

    if (NULL == tds__SetHostname->Name)
    {
        // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidHostname", "The requested hostname cannot be accepted by the device");
        printf("Invalid argument. Hostname[%p]\n", tds__SetHostname->Name);
        return SOAP_FAULT;
    }

    if ((0 == strlen(tds__SetHostname->Name)) || (MAX_HOST_NAME_LEN < strlen(tds__SetHostname->Name)))
    {
        // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidHostname", "The requested hostname cannot be accepted by the device");
        printf("Invalid argument. Invalid argument. Hostname len[%d]\n", strlen(tds__SetHostname->Name));
        return SOAP_FAULT;
    }

        // As per RFC952, Valid Host Name (allowed characters are alphabet (A-Z) (a-z), digits (0-9) and minus sign (-) and .(Dot))
    uint8_t hoatNameLength = strlen(tds__SetHostname->Name);
    for (uint8_t index = 0; index < hoatNameLength; index++)
    {
        if ((0 == isalnum(tds__SetHostname->Name[index])) && (tds__SetHostname->Name[index] != '-') && (tds__SetHostname->Name[index] != '.'))
        {
            onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidHostname", "The requested hostname cannot be accepted by the device");
            printf("Invalid argument. Invalid Hostname. Hostname [%s]\n", tds__SetHostname->Name);
            return SOAP_FAULT;
        }
    }

    printf("Host Name : %s \n", tds__SetHostname->Name);
    int ret_val = sethostname(tds__SetHostname->Name, strlen(tds__SetHostname->Name));

    printf("*********************************sethostname return value : %d **************************\n",ret_val);


    return SOAP_OK;
}

/** Web service operation '__tds__SetHostnameFromDHCP' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostnameFromDHCP(struct soap* soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{
    printf("Not implemented \"__tds__SetHostnameFromDHCP \", Not Supporting\n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return SOAP_FAULT;
}

/** Web service operation '__tds__GetDNS' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS(struct soap* soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{

    // #ifndef ONVIF_TEST
    // // RK_LOGD("Called GetDNS \n");
    // #else
    printf("****************Called GetDNS ****************************\n");
    // #endif

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    char* ethernet_v4_dns = NULL;
	char* ethernet_v4_dns1 = NULL;
	ethernet_v4_dns = malloc(64);
	ethernet_v4_dns1 = malloc(64);

    nvt_adapter_network_dns_get(ethernet_v4_dns,ethernet_v4_dns1);

    printf("ethernet_v4_dns : %s\n",ethernet_v4_dns);
    printf("ethernet_v4_dns1 : %s\n",ethernet_v4_dns1);

    if(0 == isValidIp4(ethernet_v4_dns))
    {
        sprintf(ethernet_v4_dns,"255.255.255.255");
    }

    if(0 == isValidIp4(ethernet_v4_dns1))
    {
        sprintf(ethernet_v4_dns1,"255.255.255.255");
    }


    printf("ethernet_v4_dns : %s\n",ethernet_v4_dns);
    printf("ethernet_v4_dns1 : %s\n",ethernet_v4_dns1);

    
    tds__GetDNSResponse->DNSInformation = (struct tt__DNSInformation *)soap_malloc(soap, sizeof(struct tt__DNSInformation));
    MEMSET(tds__GetDNSResponse->DNSInformation, 0x00, sizeof(struct tt__DNSInformation));


    if (DNS_AUTO == dns_mode)
        tds__GetDNSResponse->DNSInformation->FromDHCP = xsd__boolean__true_;
    else if (DNS_STATIC == dns_mode)
        tds__GetDNSResponse->DNSInformation->FromDHCP = xsd__boolean__false_;

    tds__GetDNSResponse->DNSInformation->__sizeSearchDomain = 1;
    tds__GetDNSResponse->DNSInformation->SearchDomain = (char *)soap_malloc(soap, sizeof(char *) * tds__GetDNSResponse->DNSInformation->__sizeSearchDomain);
    tds__GetDNSResponse->DNSInformation->SearchDomain[0] = (char *)soap_malloc(soap, sizeof(char *) * (SMALL_BUFFER_LENGTH));
    MEMSET(tds__GetDNSResponse->DNSInformation->SearchDomain[0], 0x00, sizeof(char *) * (SMALL_BUFFER_LENGTH));
    // SNPRINTF(tds__GetDNSResponse->DNSInformation->SearchDomain[0], SMALL_BUFFER_LENGTH, "%s", "honeywell.com");    // SNPRINTF(tds__GetDNSResponse->DNSInformation->SearchDomain[0], SMALL_BUFFER_LENGTH, "%s", "honeywell.com");

    // SNPRINTF(tds__GetDNSResponse->DNSInformation->SearchDomain[0], SMALL_BUFFER_LENGTH, "%s", test_search_domain);

    sprintf(tds__GetDNSResponse->DNSInformation->SearchDomain[0],"%s", test_search_domain); // "test.com"

    if (DNS_AUTO == dns_mode)
    {
        printf("***********DNS_AUTO****************\n");
        tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 1; //2;
        tds__GetDNSResponse->DNSInformation->DNSFromDHCP =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress) * tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP);
        MEMSET(tds__GetDNSResponse->DNSInformation->DNSFromDHCP, 0x00, sizeof(struct tt__IPAddress) * tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP);

        tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].Type = tt__IPType__IPv4;
        // if(l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[0] != 0)
        {
            tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].IPv4Address = (char *)soap_malloc(soap, sizeof(char) * IPV4_ADDR_STRING_LEN);
            MEMSET(tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].IPv4Address, 0x00, sizeof(char) * IPV4_ADDR_STRING_LEN);
            snprintf(tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].IPv4Address, IPV4_ADDR_STRING_LEN, "%s", ethernet_v4_dns); //"%d.%d.%d.%d",
                    // ip_address[0], //10,// l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[0],
                    // ip_address[1],//67,// l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[1],
                    // ip_address[2],//97,// l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[2],
                    // ip_address[3]); //35);// l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[3]);
        }
        // else
        // {
        //     tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].IPv4Address = NULL;
        // }

        tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].Type = tt__IPType__IPv4;
        {
            tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].IPv4Address = (char *)soap_malloc(soap, sizeof(char) * IPV4_ADDR_STRING_LEN);
            MEMSET(tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].IPv4Address, 0x00, sizeof(char) * IPV4_ADDR_STRING_LEN);
            snprintf(tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].IPv4Address, IPV4_ADDR_STRING_LEN, "%s", ethernet_v4_dns1); //"%d.%d.%d.%d",
                // ip_address[0], //10,// l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[0],
                // ip_address[1],//67,// l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[1],
                // ip_address[2],//97,// l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[2],
                // ip_address[3]); //35);// l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[3]);
        }

        // {
        //     tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].IPv4Address = NULL;
        // }

        // if(l_x_network.m_x_ipv6Lanset.m_ui8_ipv6Enable == xsd__boolean__true_ )
        // {
        //     tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].IPv6Address = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (MAX_DNS_IPV6_ADDR_LEN + 1));
        //     MEMSET(tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].IPv6Address, 0x00, sizeof(CHAR) * (MAX_DNS_IPV6_ADDR_LEN + 1));
        //     SNPRINTF(tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].IPv6Address, MAX_DNS_IPV6_ADDR_LEN + 1, "%s", l_x_network.m_x_ipv6Lanset.m_x_dns.m_aui8_preferrdAddr);

        //     tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].IPv6Address = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * (MAX_DNS_IPV6_ADDR_LEN + 1));
        //     MEMSET(tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].IPv6Address, 0x00, sizeof(CHAR) * (MAX_DNS_IPV6_ADDR_LEN + 1));
        //     SNPRINTF(tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].IPv6Address, MAX_DNS_IPV6_ADDR_LEN + 1, "%s", l_x_network.m_x_ipv6Lanset.m_x_dns.m_aui8_alternateAddr);
        // }
        // else
        {
            tds__GetDNSResponse->DNSInformation->DNSFromDHCP[0].IPv6Address = NULL;
            tds__GetDNSResponse->DNSInformation->DNSFromDHCP[1].IPv6Address = NULL;
        }
    }
    else
    {
        printf("***********DNS_STATIC****************\n");
        int dns_count = 0;
        if (isValidIp4(ethernet_v4_dns)) dns_count++;
        if (isValidIp4(ethernet_v4_dns1)) dns_count++;
        if (dns_count == 0) dns_count = 1;
        tds__GetDNSResponse->DNSInformation->__sizeDNSManual = dns_count;
        tds__GetDNSResponse->DNSInformation->DNSManual =
            (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress) * dns_count);
        MEMSET(tds__GetDNSResponse->DNSInformation->DNSManual, 0x00, sizeof(struct tt__IPAddress) * dns_count);

        tds__GetDNSResponse->DNSInformation->DNSManual[0].Type = tt__IPType__IPv4;
        tds__GetDNSResponse->DNSInformation->DNSManual[0].IPv4Address = (char *)soap_malloc(soap, sizeof(char) * IPV4_ADDR_STRING_LEN);
        MEMSET(tds__GetDNSResponse->DNSInformation->DNSManual[0].IPv4Address, 0x00, sizeof(char) * IPV4_ADDR_STRING_LEN);
        snprintf(tds__GetDNSResponse->DNSInformation->DNSManual[0].IPv4Address, IPV4_ADDR_STRING_LEN, "%s", ethernet_v4_dns);//"%d.%d.%d.%d",
                // ip_address[0], //10,//  l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[0],
                // ip_address[1], //67,//  l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[1],
                // ip_address[2], //97,//  l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[2],
                // ip_address[3]); //35);//  l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[3]);
        tds__GetDNSResponse->DNSInformation->DNSManual[0].IPv6Address = NULL;

        if (dns_count > 1) {
        tds__GetDNSResponse->DNSInformation->DNSManual[1].Type = tt__IPType__IPv4;
        tds__GetDNSResponse->DNSInformation->DNSManual[1].IPv4Address = (char *)soap_malloc(soap, sizeof(char) * IPV4_ADDR_STRING_LEN);
        MEMSET(tds__GetDNSResponse->DNSInformation->DNSManual[1].IPv4Address, 0x00, sizeof(char) * IPV4_ADDR_STRING_LEN);
        snprintf(tds__GetDNSResponse->DNSInformation->DNSManual[1].IPv4Address, IPV4_ADDR_STRING_LEN, "%s", ethernet_v4_dns1);
        tds__GetDNSResponse->DNSInformation->DNSManual[1].IPv6Address = NULL;
        }

        // tds__GetDNSResponse->DNSInformation->DNSManual[2].Type = tt__IPType__IPv6;
        // tds__GetDNSResponse->DNSInformation->DNSManual[2].IPv6Address = (char *)soap_malloc(soap, sizeof(char) * (MAX_DNS_IPV6_ADDR_LEN + 1));
        // MEMSET(tds__GetDNSResponse->DNSInformation->DNSManual[2].IPv6Address, 0x00, sizeof(char) * (MAX_DNS_IPV6_ADDR_LEN + 1));
        // snprintf(tds__GetDNSResponse->DNSInformation->DNSManual[2].IPv6Address, MAX_DNS_IPV6_ADDR_LEN + 1, "%s", ip_address); //"10.67.97.35");//l_x_network.m_x_ipv6Lanset.m_x_dns.m_aui8_alternateAddr);
        // tds__GetDNSResponse->DNSInformation->DNSManual[2].IPv4Address = NULL;

        // tds__GetDNSResponse->DNSInformation->DNSManual[3].Type = tt__IPType__IPv6;
        // tds__GetDNSResponse->DNSInformation->DNSManual[3].IPv6Address = (char *)soap_malloc(soap, sizeof(char) * (MAX_DNS_IPV6_ADDR_LEN + 1));
        // MEMSET(tds__GetDNSResponse->DNSInformation->DNSManual[3].IPv6Address, 0x00, sizeof(char) * (MAX_DNS_IPV6_ADDR_LEN + 1));
        // snprintf(tds__GetDNSResponse->DNSInformation->DNSManual[3].IPv6Address, MAX_DNS_IPV6_ADDR_LEN + 1, "%s", ip_address); //"10.67.97.35"); //l_x_network.m_x_ipv6Lanset.m_x_dns.m_aui8_preferrdAddr);
        // tds__GetDNSResponse->DNSInformation->DNSManual[3].IPv4Address = NULL;
    }

    tds__GetDNSResponse->DNSInformation->Extension = NULL;

    printf("***********Before free****************\n");
    // dom_att(&tds__GetDNSResponse->DNSInformation->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

	free(ethernet_v4_dns);
	free(ethernet_v4_dns1);

    printf("***********After free****************\n");

    return SOAP_OK;
}

/** Web service operation '__tds__SetDNS' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS(struct soap* soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    
    printf("****************************Called __tds__SetDNS ****************************\n");

    dns_mode = (xsd__boolean__true_ == tds__SetDNS->FromDHCP) ? DNS_AUTO : DNS_STATIC;

    // if((xsd__boolean__true_ == tds__SetDNS->FromDHCP))
    // {
    //     printf("Set DNS from DHCP\n");
    // }
    // else
    // {
        if ((tds__SetDNS->__sizeDNSManual > 0) && (NULL != tds__SetDNS->DNSManual)) 
        {
            if (tt__IPType__IPv4 == tds__SetDNS->DNSManual[0].Type)
            {
                if ((NULL == tds__SetDNS->DNSManual[0].IPv4Address) || (0 == isValidIp4(tds__SetDNS->DNSManual[0].IPv4Address)))
                {
                    printf("Invalid IPv4 Address IPv4Address[%s]\n", tds__SetDNS->DNSManual[0].IPv4Address);
                    // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "The suggested IPv4 address is invalid");
                    return SOAP_FAULT;
                }

                // sscanf(tds__SetDNS->DNSManual[0].IPv4Address, "%d.%d.%d.%d", &l_i32_ip[0], &l_i32_ip[1], &l_i32_ip[2], &l_i32_ip[3]);
                // RCF_PRINTF(PRIO_INFO, "Set DNS IP Address - %d.%d.%d.%d", l_i32_ip[0], l_i32_ip[1], l_i32_ip[2], l_i32_ip[3]);

                // l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[0] = (UINT8)l_i32_ip[0];
                // l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[1] = (UINT8)l_i32_ip[1];
                // l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[2] = (UINT8)l_i32_ip[2];
                // l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_preferrdAddr[3] = (UINT8)l_i32_ip[3];
            }
            else{}
        }

        if ((tds__SetDNS->__sizeDNSManual > 1) && (NULL != tds__SetDNS->DNSManual)) //if size more then 1 then set second address
        {
            if (tt__IPType__IPv4 == tds__SetDNS->DNSManual[1].Type)
            {
                if ((NULL == tds__SetDNS->DNSManual[1].IPv4Address) || (!isValidIp4(tds__SetDNS->DNSManual[1].IPv4Address)))
                {
                    printf("Invalid IPv4 Address IPv4Address[%s]\n", tds__SetDNS->DNSManual[1].IPv4Address);
                    // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "The suggested IPv4 address is invalid");
                    return SOAP_FAULT;
                }

                // sscanf(tds__SetDNS->DNSManual[1].IPv4Address, "%d.%d.%d.%d", &l_i32_ip[0], &l_i32_ip[1], &l_i32_ip[2], &l_i32_ip[3]);
                // RCF_PRINTF(PRIO_INFO, "Set DNS IP Address - %d.%d.%d.%d", l_i32_ip[0], l_i32_ip[1], l_i32_ip[2], l_i32_ip[3]);

                // l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_alternateAddr[0] = (UINT8)l_i32_ip[0];
                // l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_alternateAddr[1] = (UINT8)l_i32_ip[1];
                // l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_alternateAddr[2] = (UINT8)l_i32_ip[2];
                // l_x_network.m_x_ipv4Lanset.m_x_dns.m_aui8_alternateAddr[3] = (UINT8)l_i32_ip[3];
            }
            else{}
        }

        if (tds__SetDNS->__sizeSearchDomain > 0)
        {
            printf("test_search_domain : %s \n", tds__SetDNS->SearchDomain[0]);

            snprintf(test_search_domain, SMALL_BUFFER_LENGTH, "%s", tds__SetDNS->SearchDomain[0]);

            printf("test_search_domain : %s \n", tds__SetDNS->SearchDomain[0]);
            printf("test_search_domain : %s \n", test_search_domain);
        }

        if(NULL != tds__SetDNS->DNSManual)
        {
            const char *dns2 = (tds__SetDNS->__sizeDNSManual > 1) ? tds__SetDNS->DNSManual[1].IPv4Address : NULL;
            nvt_adapter_network_dns_set(tds__SetDNS->DNSManual[0].IPv4Address, dns2); 

            printf("****************************Set DNS Manually ****************************\n");
        }
        else{

            printf("****************************Set DNS from DHCP ****************************\n");
        }


    // }
    
    return SOAP_OK;
}

/** Web service operation '__tds__GetNTP' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP(struct soap* soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
    printf("Called __tds__GetNTP ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    tds__GetNTPResponse->NTPInformation = (struct tt__NTPInformation *)soap_malloc(soap, sizeof(struct tt__NTPInformation));
    MEMSET(tds__GetNTPResponse->NTPInformation, 0x00, sizeof(struct tt__NTPInformation));

    tds__GetNTPResponse->NTPInformation->FromDHCP = ((0 == ntp_server.imp_ui8_IsDhcp) ? xsd__boolean__false_ : xsd__boolean__true_); // ((0 == l_x_ntp.m_ui8_IsDhcp) ? xsd__boolean__false_ : xsd__boolean__true_);

    if(xsd__boolean__true_ == tds__GetNTPResponse->NTPInformation->FromDHCP)
    {
        tds__GetNTPResponse->NTPInformation->NTPManual = NULL;
        tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP = 1;
        tds__GetNTPResponse->NTPInformation->NTPFromDHCP =
            ((struct tt__NetworkHost *)soap_malloc(soap, tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP * sizeof(struct tt__NetworkHost)));
        MEMSET(tds__GetNTPResponse->NTPInformation->NTPFromDHCP, 0x00, tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP * sizeof(struct tt__NetworkHost));

        for(int i = 0; i < tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP; i++)
        {
            if(E_NTP_IPV4 == 0) //l_x_ntp.m_e_ntpType)
            {
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv6Address = NULL;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].DNSname = NULL;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].Type = tt__NetworkHostType__IPv4;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv4Address = (char *)soap_malloc(soap, (MAX_NTP_SERVER_LEN + 1));
                MEMSET(tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv4Address, 0x00, (MAX_NTP_SERVER_LEN + 1));
                snprintf(tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv4Address, (MAX_NTP_SERVER_LEN + 1), "%s",
                            ntp_server.imp_ua_ntpAddress); //"10.67.97.35"); //l_x_ntp.m_ua_ntpAddress);
            }
            else if(E_NTP_IPV6 == 1) //l_x_ntp.m_e_ntpType)
            {
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv4Address = NULL;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].DNSname = NULL;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].Type = tt__NetworkHostType__IPv6;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv6Address = (char *)soap_malloc(soap, (MAX_NTP_SERVER_LEN + 1));
                MEMSET(tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv6Address, 0x00, (MAX_NTP_SERVER_LEN + 1));
                snprintf(tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv6Address, (MAX_NTP_SERVER_LEN + 1), "%s",
                            ntp_server.imp_ua_ntpAddress); //"10.67.97.35"); // l_x_ntp.m_ua_ntpAddress);
            }
            else
            {
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv4Address = NULL;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].IPv6Address = NULL;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].Type = tt__NetworkHostType__DNS;
                tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].DNSname = (char *)soap_malloc(soap, (MAX_NTP_SERVER_LEN + 1));
                MEMSET(tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].DNSname, 0x00, (MAX_NTP_SERVER_LEN + 1));
                snprintf(tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].DNSname, (MAX_NTP_SERVER_LEN + 1), "%s",
                            ntp_server.imp_ua_ntpAddress); //"10.67.97.35"); // l_x_ntp.m_ua_ntpAddress);
            }

            tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].Extension = NULL;
         //   dom_att(&tds__GetNTPResponse->NTPInformation->NTPFromDHCP[i].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        }
    }
    else
    {
        tds__GetNTPResponse->NTPInformation->NTPFromDHCP = NULL;
        tds__GetNTPResponse->NTPInformation->__sizeNTPManual = 1;
        tds__GetNTPResponse->NTPInformation->NTPManual =
            ((struct tt__NetworkHost *)soap_malloc(soap, tds__GetNTPResponse->NTPInformation->__sizeNTPManual * sizeof(struct tt__NetworkHost)));
        MEMSET(tds__GetNTPResponse->NTPInformation->NTPManual, 0x00, tds__GetNTPResponse->NTPInformation->__sizeNTPManual * sizeof(struct tt__NetworkHost));

        for(int i = 0; i < tds__GetNTPResponse->NTPInformation->__sizeNTPManual; i++)
        {
            if(E_NTP_IPV4 == 0) //l_x_ntp.m_e_ntpType)
            {
                tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv6Address = NULL;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].DNSname = NULL;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].Type = tt__NetworkHostType__IPv4;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv4Address = (char *)soap_malloc(soap, (MAX_NTP_SERVER_LEN + 1));
                MEMSET(tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv4Address, 0x00, (MAX_NTP_SERVER_LEN + 1));
                snprintf(tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv4Address, (MAX_NTP_SERVER_LEN + 1), "%s",
                            ntp_server.imp_ua_ntpAddress); //"10.67.97.35"); // l_x_ntp.m_ua_ntpAddress);
            }
            else if(E_NTP_IPV6 == 1) // l_x_ntp.m_e_ntpType)
            {
                tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv4Address = NULL;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].DNSname = NULL;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].Type = tt__NetworkHostType__IPv6;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv6Address = (char *)soap_malloc(soap, (MAX_NTP_SERVER_LEN + 1));
                MEMSET(tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv6Address, 0x00, (MAX_NTP_SERVER_LEN + 1));
                snprintf(tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv6Address, (MAX_NTP_SERVER_LEN + 1), "%s",
                            ntp_server.imp_ua_ntpAddress); //"10.67.97.35"); // l_x_ntp.m_ua_ntpAddress);
            }
            else
            {
                tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv4Address = NULL;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].IPv6Address = NULL;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].Type = tt__NetworkHostType__DNS;
                tds__GetNTPResponse->NTPInformation->NTPManual[i].DNSname = (char *)soap_malloc(soap, (MAX_NTP_SERVER_LEN + 1));
                MEMSET(tds__GetNTPResponse->NTPInformation->NTPManual[i].DNSname, 0x00, (MAX_NTP_SERVER_LEN + 1));
                snprintf(tds__GetNTPResponse->NTPInformation->NTPManual[i].DNSname, (MAX_NTP_SERVER_LEN + 1), "%s",
                            ntp_server.imp_ua_ntpAddress); //"10.67.97.35"); // l_x_ntp.m_ua_ntpAddress);
            }

            tds__GetNTPResponse->NTPInformation->NTPManual[i].Extension = NULL;
          //  dom_att(&tds__GetNTPResponse->NTPInformation->NTPManual[i].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
        }
    }

    tds__GetNTPResponse->NTPInformation->Extension = NULL;



    return SOAP_OK;
}

/** Web service operation '__tds__SetNTP' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP(struct soap* soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (tds__SetNTP->__sizeNTPManual >= 2)
    {
        printf("Set NTP from DHCP not allowed more than one\n");
        // onvif_fault_probe(soap, "ter:InvalidArgVal", "Invalid Argunment Provided");
        return SOAP_FAULT;
    }

    if (NULL != tds__SetNTP->NTPManual)
    {
        if ((tds__SetNTP->FromDHCP == xsd__boolean__true_))
        {
            printf("Set NTP from DHCP not allowed\n");
            // onvif_fault_probe(soap, "ter:InvalidArgVal", "Invalid Argunment Provided");
            return SOAP_FAULT;
        }
        else
        {
            ntp_server.imp_ui8_IsDhcp = tds__SetNTP->FromDHCP;
            for (int32_t i = 0; i < tds__SetNTP->__sizeNTPManual; i++)
            {
                if ((tt__NetworkHostType__IPv4 == tds__SetNTP->NTPManual[i].Type) &&
                    (NULL != tds__SetNTP->NTPManual[i].IPv4Address))
                {
                    if (isValidIp4(tds__SetNTP->NTPManual[i].IPv4Address) == false)
                    {
                        onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "The suggested IPv4 address is invalid");
                        printf("The suggested IPv4 address is invalid\n");
                        return SOAP_FAULT;
                    }

                    ntp_server.imp_e_ntpType = E_NTP_IPV4;
                    snprintf(ntp_server.imp_ua_ntpAddress, MAX_NTP_SERVER_LEN, "%s", tds__SetNTP->NTPManual[i].IPv4Address);
                }
                else if ((tt__NetworkHostType__IPv6 == tds__SetNTP->NTPManual[i].Type) &&
                         (NULL != tds__SetNTP->NTPManual[i].IPv6Address))
                {
                    // if (isValidIp6(tds__SetNTP->NTPManual[i].IPv6Address) == false)
                    // {
                    //     // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidIPv6Address", "The suggested IPv6 address is invalid");
                    //     printf("The suggested IPv6 address is invalid\n");
                    //     return SOAP_FAULT;
                    // }

                    ntp_server.imp_e_ntpType = E_NTP_IPV6;
                    snprintf(ntp_server.imp_ua_ntpAddress, MAX_NTP_SERVER_LEN, "%s", tds__SetNTP->NTPManual[i].IPv6Address);
                }
                else if ((tt__NetworkHostType__DNS == tds__SetNTP->NTPManual[i].Type) &&
                         (NULL != tds__SetNTP->NTPManual[i].DNSname))
                {
                    ntp_server.imp_e_ntpType = E_NTP_DNS;
                    snprintf(ntp_server.imp_ua_ntpAddress, MAX_NTP_SERVER_LEN, "%s", tds__SetNTP->NTPManual[i].DNSname);
                }
                else
                {
                    // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "Invalid data provided");
                    printf("Invaild data provided\n");
                    return SOAP_FAULT;
                }
            }
        }
    }
    else
    {
        if ((tds__SetNTP->FromDHCP == xsd__boolean__false_))
        {
            printf("Service Not Supported, Set NTP from DHCP not allowed\n");
            // onvif_fault_probe(soap, "ter:InvalidArgVal", "Invalid Argunment Provided");
            return SOAP_FAULT;
        }
        else
        {
            ntp_server.imp_ui8_IsDhcp = tds__SetNTP->FromDHCP;
        }
    }


    return SOAP_OK;
}

/** Web service operation '__tds__GetDynamicDNS' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS(struct soap* soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return SOAP_OK;
}

/** Web service operation '__tds__SetDynamicDNS' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS(struct soap* soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetNetworkInterfaces' (returns SOAP_OK or error code) */
/** Web service operation '__tds__GetNetworkInterfaces' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkInterfaces(struct soap* soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
    printf("Called __tds__GetNetworkInterfaces ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if(ip_address_update_request)
    {
        printf("Set network settings process is inprogress, Get Operation Prohibited\n");
        onvif_fault_probe(soap, "ter:OperationProhibited", "Failed to get network settings from system server");
        return SOAP_FAULT;
    }

    // Get network info using wrapper
    OnvifNetworkInterface info;
    if (onvif_get_network_interface("eth0", &info) != 0) {
        printf("Failed to get network interface info\n");
        return SOAP_FAULT;
    }

    printf("Network Info: Interface=%s, MAC=%s, IP=%s, Method=%s\n", 
           info.name, info.mac_address, info.ipv4.address, info.ipv4.method);

    // Update global ip_address buffer if it exists (for compatibility)
    // snprintf(ip_address, sizeof(ip_address), "%s", info.ipv4.address);

    tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = 1;

    tds__GetNetworkInterfacesResponse->NetworkInterfaces =
        (struct tt__NetworkInterface *)soap_malloc(soap, tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces * sizeof(struct tt__NetworkInterface));
    MEMSET(tds__GetNetworkInterfacesResponse->NetworkInterfaces, 0x00, tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces * sizeof(struct tt__NetworkInterface));
    
    struct tt__NetworkInterface *iface = tds__GetNetworkInterfacesResponse->NetworkInterfaces;

    // Token
    iface->token = (char *)soap_malloc(soap, TOKEN_LENGTH);
    MEMSET(iface->token, 0x00, TOKEN_LENGTH);
    sprintf(iface->token, "%s", "eth0");
    iface->Enabled = xsd__boolean__true_;

    // Info
    iface->Info = (struct tt__NetworkInterfaceInfo *)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceInfo));
    MEMSET(iface->Info, 0x00, sizeof(struct tt__NetworkInterfaceInfo));
    
    iface->Info->Name = (char *)soap_malloc(soap, MAX_HOST_NAME_LEN+1);
    MEMSET(iface->Info->Name, 0x00, MAX_HOST_NAME_LEN+1);
    sprintf(iface->Info->Name,"%s", "eth0");
    
    iface->Info->HwAddress = (char *)soap_malloc(soap, MAC_ADDR_STRING_LEN);
    MEMSET(iface->Info->HwAddress, 0x00, MAC_ADDR_STRING_LEN);
    snprintf(iface->Info->HwAddress, MAC_ADDR_STRING_LEN, "%s", info.mac_address);
                                                                   
    iface->Info->MTU = (int *)soap_malloc(soap, sizeof(int));
    *iface->Info->MTU = 1500;

    // Link
    iface->Link = (struct tt__NetworkInterfaceLink *)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceLink));
    MEMSET(iface->Link, 0x00, sizeof(struct tt__NetworkInterfaceLink));

    iface->Link->AdminSettings = (struct tt__NetworkInterfaceConnectionSetting *)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceConnectionSetting));
    MEMSET(iface->Link->AdminSettings, 0x00, sizeof(struct tt__NetworkInterfaceConnectionSetting));
    iface->Link->AdminSettings->AutoNegotiation = xsd__boolean__true_;
    iface->Link->AdminSettings->Speed = 0;
    iface->Link->AdminSettings->Duplex = tt__Duplex__Full;

    iface->Link->OperSettings = (struct tt__NetworkInterfaceConnectionSetting *)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceConnectionSetting));
    MEMSET(iface->Link->OperSettings, 0x00, sizeof(struct tt__NetworkInterfaceConnectionSetting));
    iface->Link->OperSettings->AutoNegotiation = xsd__boolean__true_;
    iface->Link->OperSettings->Speed = 0;
    iface->Link->OperSettings->Duplex = tt__Duplex__Full;

    iface->Link->InterfaceType = 6;

    // IPv4
    iface->IPv4 = (struct tt__IPv4NetworkInterface *)soap_malloc(soap, sizeof(struct tt__IPv4NetworkInterface));
    MEMSET(iface->IPv4, 0x00, sizeof(struct tt__IPv4NetworkInterface));
    iface->IPv4->Enabled = xsd__boolean__true_;
    
    iface->IPv4->Config = (struct tt__IPv4Configuration *)soap_malloc(soap, sizeof(struct tt__IPv4Configuration));
    MEMSET(iface->IPv4->Config, 0x00, sizeof(struct tt__IPv4Configuration));

    iface->IPv4->Config->LinkLocal = (struct tt__PrefixedIPv4Address *)soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
    MEMSET(iface->IPv4->Config->LinkLocal, 0x00, sizeof(struct tt__PrefixedIPv4Address));
    iface->IPv4->Config->LinkLocal->Address = (char *)soap_malloc(soap, IPV4_ADDR_STRING_LEN + 1);
    MEMSET(iface->IPv4->Config->LinkLocal->Address, 0x00, IPV4_ADDR_STRING_LEN + 1);
    snprintf(iface->IPv4->Config->LinkLocal->Address, IPV4_ADDR_STRING_LEN + 1, "%s", info.ipv4.address);
    iface->IPv4->Config->LinkLocal->PrefixLength = 16;

    if(strcmp(info.ipv4.method, "dhcp") == 0)
    {
        iface->IPv4->Config->FromDHCP = (struct tt__PrefixedIPv4Address *)soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
        MEMSET(iface->IPv4->Config->FromDHCP, 0x00, sizeof(struct tt__PrefixedIPv4Address));
        iface->IPv4->Config->FromDHCP->Address = (char *)soap_malloc(soap, IPV4_ADDR_STRING_LEN);
        MEMSET(iface->IPv4->Config->FromDHCP->Address, 0x00, IPV4_ADDR_STRING_LEN);
        snprintf(iface->IPv4->Config->FromDHCP->Address, IPV4_ADDR_STRING_LEN, "%s", info.ipv4.address);
        iface->IPv4->Config->FromDHCP->PrefixLength = 24;
        
        iface->IPv4->Config->__sizeManual = 0;
        iface->IPv4->Config->Manual = NULL;
        iface->IPv4->Config->DHCP = xsd__boolean__true_;
    }
    else     
    {
        iface->IPv4->Config->FromDHCP = NULL;
        
        iface->IPv4->Config->__sizeManual = 1;
        iface->IPv4->Config->Manual = (struct tt__PrefixedIPv4Address *)soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
        MEMSET(iface->IPv4->Config->Manual, 0x00, sizeof(struct tt__PrefixedIPv4Address));
        
        iface->IPv4->Config->Manual->Address = (char *)soap_malloc(soap, IPV4_ADDR_STRING_LEN);
        MEMSET(iface->IPv4->Config->Manual->Address, 0x00, IPV4_ADDR_STRING_LEN);
        snprintf(iface->IPv4->Config->Manual->Address, IPV4_ADDR_STRING_LEN, "%s", info.ipv4.address);
        iface->IPv4->Config->Manual->PrefixLength = 24;
        
        iface->IPv4->Config->DHCP = xsd__boolean__false_;
    }

    iface->IPv4->Config->__size = 0;
    iface->IPv4->Config->__any = NULL;

    return SOAP_OK;
}

/** Web service operation '__tds__SetNetworkInterfaces' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkInterfaces(struct soap* soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{
    printf("Called __tds__SetNetworkInterfaces ..... \n");


    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == tds__SetNetworkInterfaces)
    {
        onvif_fault_probe(soap, "ter:NotFound", "Invalid argument. tds__SetNetworkInterfaces is NULL");
        printf("Invalid argument. tds__SetNetworkProtocols[%p]\n", tds__SetNetworkInterfaces);
        return SOAP_FAULT;
    }

    if (NULL != tds__SetNetworkInterfaces->InterfaceToken)
    {
        if (strcmp(tds__SetNetworkInterfaces->InterfaceToken, "eth0"))
        {
            printf("Invalid Network Token \n");
            onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidNetworkInterface", "Invalid network token");
            return SOAP_FAULT;
        }
    }

    if (NULL != tds__SetNetworkInterfaces->NetworkInterface->IPv4)
    {
        if (NULL != tds__SetNetworkInterfaces->NetworkInterface->IPv4->Enabled)
        {
            ; //Here IPv4 always enabled we'll ignore its flag
        }

        if (xsd__boolean__true_ == *(tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP))
        {
            printf("****DHCP Setting Paramaters****\n");
            // nvt_adapter_network_ipv4_set("eth0", "dhcp", "127.1.0.0", "255.255.0.0", "199.63.104.254");
            // usleep(1000*1000);
            // nvt_adapter_network_ipv4_get("eth0", method, ip_address, netmask, gateway); 
            // if ((!isValidIp4(ip_address)) || (0 == strcmp(ip_address, "0.0.0.0")))
            // {
            //     printf("Failed to retrieve IP address of eth0\n");
            //     printf("*********************************************Failed to retrieve IP address of eth0*************************************************************... \n");
            //     usleep(1000*1000llu);
            // }
            // snprintf(method, strlen("dhcp")+1,"dhcp");
            sprintf(method,"dhcp");

            nvt_adapter_param_set_string("network.ipv4:method", "dhcp");
            printf("Method is : %s\n",method);

            sprintf(temp_method, "dhcp");

            printf("ip_assign_method : %s \n",ip_assign_method);

            

            // sprintf(previous_ip_address, "%s",ip_address);

            // if(0 != strcmp(ip_assign_method, "dhcp"))
            {
                sprintf(ip_assign_method, "dhcp");
                ip_address_update_request = true;
            }
            
            // nvt_adapter_network_ipv4_set("eth0", method, ip_address, netmask, gateway);
            // printf("**********Network Set Successfully ******************************\n");
            // usleep(1000*1000);

            // ip_address_update_flag = 1;
            // while(1)
            // {
            //     usleep(1000*1000);
            //     // usleep(1000*1000);
            //     // usleep(1000*1000);
            //     printf("**********Before nvt_adapter_network_ipv4_get ******************************\n");
            //     nvt_adapter_network_ipv4_get("eth0",method, ip_address, netmask, gateway); 
            //     printf("**********After nvt_adapter_network_ipv4_get ******************************\n");
            //     // if(ret_v == 0 && ip_address != NULL)
            //     if ((!isValidIp4(ip_address)) || (0 == strcmp(ip_address, "0.0.0.0")))
            //     {
            //         printf("Failed to retrieve IP address of eth0\n");
            //         printf("*********************************************Failed to retrieve IP address of eth0*************************************************************... \n");
            //         usleep(1000*1000llu);
            //     }
            //     else
            //     {
            //         // sprintf(ip_address,"%s",address);
            //         printf("IP address : %s\n",ip_address);
            //         get_hw_addr("eth0", macAddress);
            //         printf("MAC Address :%02X%02X%02X%02X%02X%02X\n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
            //         // nvt_adapter_network_get_mac("eth0", macAddress);
            //         // printf("MAC Address :%02X%02X%02X%02X%02X%02X\n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
            //         //************ONVIF ******************************
            //         if(0 != strcmp(ip_address, previous_ip_address))
            //             ip_address_update_request = true;
            //         break;
            //     }
            // }
        }

        else if (NULL != tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual)
        {
             printf("****Manual Setting Paramaters****\n");
            if (NULL != tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address)
            {
                if (isValidIp4(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address) == 0) // Check IP address
                {
                    onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "Invalid IPv4 address");
                    return SOAP_FAULT;
                }
                printf("User Selected Ip Address - %s\n",
                           tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address);
            }

           printf("User Selected Ip Token - %s\n", tds__SetNetworkInterfaces->InterfaceToken);

           printf("ip_assign_method : %s \n",ip_assign_method);

            // if(0 != strcmp(ip_assign_method, "static"))
            // {

                if((0 == strcmp(ip_address, tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address)))
                {
                        printf("**************Request for IP Address update is same***********\n");
                }
                else
                {
                        printf("**************Request for IP Address update is different***********\n");
                        // nvt_adapter_param_set_string("network.ipv4:method", "static");
                        // nvt_adapter_param_set_string("network.ipv4:ipv4_address", ip_address);
                        // nvt_adapter_param_set_string("network.ipv4:ipv4_netmask", netmask);
                        // nvt_adapter_param_set_string("network.ipv4:ipv4_gateway", gateway);
                }

                snprintf(method, strlen("static")+1,"static");
                printf("Method is : %s\n",method);
                // nvt_adapter_network_ipv4_set("eth0", method, tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address, netmask, gateway);
                sprintf(temp_ip_address, "%s", tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address);
                sprintf(temp_method, "static");
                sprintf(ip_assign_method, "static");
                ip_address_update_request = true;

            // }

            // nvt_adapter_network_ipv4_get("eth0", method, ip_address, netmask, gateway); 
            // if ((!isValidIp4(ip_address)) || (0 == strcmp(ip_address, "0.0.0.0")))
            // {
            //     printf("Failed to retrieve IP address of eth0\n");
            //     printf("*********************************************Failed to retrieve IP address of eth0*************************************************************... \n");
            //     usleep(1000*1000llu);
            // }
        }
        else{}
    }

    if(ip_address_update_request)
        tds__SetNetworkInterfacesResponse->RebootNeeded = xsd__boolean__true_;
    else
        tds__SetNetworkInterfacesResponse->RebootNeeded = xsd__boolean__false_;


    return SOAP_OK;
}

/** Web service operation '__tds__GetNetworkProtocols' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkProtocols(struct soap* soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
    printf("Called __tds__GetNetworkProtocols ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols = 3;
    tds__GetNetworkProtocolsResponse->NetworkProtocols =
        (struct tt__NetworkProtocol *)soap_malloc(soap, tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols * sizeof(struct tt__NetworkProtocol));
    MEMSET(tds__GetNetworkProtocolsResponse->NetworkProtocols, 0x00,
           tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols * sizeof(struct tt__NetworkProtocol));

    /*HTTP Protocols*/
    tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Name = tt__NetworkProtocolType__HTTP; // HTTP = 0, HTTPS = 1, RTSP = 2
    tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Enabled = ((0 == network_protocol.imp_ui8_httpState) ? xsd__boolean__false_ : xsd__boolean__true_);
    tds__GetNetworkProtocolsResponse->NetworkProtocols[0].__sizePort = 1;
    tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Port = (int *)soap_malloc(soap, sizeof(int));
    *tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Port = network_protocol.imp_ui16_httpPort; // 5000; //l_x_protocol.m_ui16_httpPort;
    tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Extension = NULL;
    // dom_att(&tds__GetNetworkProtocolsResponse->NetworkProtocols[0].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
    /*HTTPS Protocols*/
    tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Name = tt__NetworkProtocolType__HTTPS;
    tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Enabled = ((0 == network_protocol.imp_ui8_httpsState) ? xsd__boolean__false_ : xsd__boolean__true_);
    tds__GetNetworkProtocolsResponse->NetworkProtocols[1].__sizePort = 1;
    tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port = (int *)soap_malloc(soap, sizeof(int));
    *tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port = network_protocol.imp_ui16_httpsPort; //1000; //l_x_protocol.m_ui16_httpsPort;
    tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Extension = NULL;
    // dom_att(&tds__GetNetworkProtocolsResponse->NetworkProtocols[1].__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);
    /*RTSP Protocols*/
    tds__GetNetworkProtocolsResponse->NetworkProtocols[2].Name = tt__NetworkProtocolType__RTSP;
    tds__GetNetworkProtocolsResponse->NetworkProtocols[2].Enabled = ((0 == network_protocol.imp_ui8_rtspState) ? xsd__boolean__false_ : xsd__boolean__true_);
    tds__GetNetworkProtocolsResponse->NetworkProtocols[2].__sizePort = 1;
    tds__GetNetworkProtocolsResponse->NetworkProtocols[2].Port = (int *)soap_malloc(soap, sizeof(int));
    *tds__GetNetworkProtocolsResponse->NetworkProtocols[2].Port = network_protocol.imp_ui16_rtspPort; //DEFAULT_RTSP_PORT; //l_x_protocol.m_ui16_rtspPort;
    tds__GetNetworkProtocolsResponse->NetworkProtocols[2].Extension = NULL;

    return SOAP_OK;
}

/** Web service operation '__tds__SetNetworkProtocols' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkProtocols(struct soap* soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{
    printf("Called __tds__SetNetworkProtocols ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    // if (NULL == tds__SetNetworkProtocols)
    // {
    //     // onvif_fault_probe(soap, "ter:InvalidArgVal", "tds__SetNetworkProtocols is NULL");
    //     printf("Invalid argument. tds__SetNetworkProtocols[%p]\n", tds__SetNetworkProtocols);
    //     return SOAP_FAULT;
    // }

    // if ((NULL == tds__SetNetworkProtocols->NetworkProtocols))
    // {
    //     // onvif_fault_probe(soap, "ter:InvalidArgVal", "NetworkProtocols is NULL");
    //     printf("Invalid argument. NetworkProtocols is NULL \n");
    //     return SOAP_FAULT;
    // }

    // //INT32 l_i32_ip[4] = {0};
    // UINT32 l_ui32_outLen = sizeof(T_NETWORK_PROTOCOL_SETTINGS);
    // T_NETWORK_PROTOCOL_SETTINGS l_x_protocol;
    // MEMSET(&l_x_protocol, 0x00, sizeof(T_NETWORK_PROTOCOL_SETTINGS));


    for (int32_t l_i32_idx = 0; l_i32_idx < tds__SetNetworkProtocols->__sizeNetworkProtocols; l_i32_idx++)
    {
        printf("User Selected Port Is - %d\n", *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port);
        switch (tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Name)
        {
        case tt__NetworkProtocolType__HTTP:
            if ((1 < (*tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port)) &&
                (65535 > (*tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port)) &&
                (network_protocol.imp_ui16_httpsPort!= *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port) &&
                (network_protocol.imp_ui16_rtspPort != *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port))
            {
                network_protocol.imp_ui16_httpPort = *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port;
                network_protocol.imp_ui8_httpState = (xsd__boolean__true_ == tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Enabled) ? xsd__boolean__true_ : xsd__boolean__false_;
                nvt_adapter_param_set_int("network.ports:enable_http", network_protocol.imp_ui8_httpState);
                nvt_adapter_param_set_int("network.ports:http_port", network_protocol.imp_ui16_httpPort);
            }
            else
            {
                printf("Invalid Parameter, Port No is invalid");
                // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter::PortAlreadyInUse", "Port No is invalid");
                return SOAP_FAULT;
            }
            break;

        case tt__NetworkProtocolType__HTTPS:
            if ((1 < (*tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port)) &&
                (65535 > (*tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port)) &&
                (network_protocol.imp_ui16_httpPort != *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port) &&
                (network_protocol.imp_ui16_rtspPort != *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port))
            {
                network_protocol.imp_ui16_httpsPort = *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port;
                network_protocol.imp_ui8_httpsState = (xsd__boolean__true_ == tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Enabled) ? xsd__boolean__true_ : xsd__boolean__false_;
                nvt_adapter_param_set_int("network.ports:enable_https", network_protocol.imp_ui8_httpsState);
                nvt_adapter_param_set_int("network.ports:https_port", network_protocol.imp_ui16_httpsPort);
            }
            else
            {
                printf("Invalid Parameter, Port No is invalid\n");
                // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter::PortAlreadyInUse", "Port No is invalid");
                return SOAP_FAULT;
            }
            break;

        case tt__NetworkProtocolType__RTSP:
            if ((1 < (*tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port)) &&
                (65535 > (*tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port)) &&
                (network_protocol.imp_ui16_httpPort != *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port) &&
                (network_protocol.imp_ui16_httpsPort != *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port))
            {
                network_protocol.imp_ui16_rtspPort = *tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Port;
                network_protocol.imp_ui8_rtspState = (xsd__boolean__true_ == tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Enabled) ? xsd__boolean__true_ : xsd__boolean__false_;
                nvt_adapter_param_set_int("network.ports:enable_rtsp", network_protocol.imp_ui8_rtspState);
                nvt_adapter_param_set_int("network.ports:rtsp_port", network_protocol.imp_ui16_rtspPort );
            }
            else
            {
                printf("Invalid Parameter, Port No is invalid\n");
                // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter::PortAlreadyInUse", "Port No is invalid");
                return SOAP_FAULT;
            }
            break;

        default:
            printf("Invalid Protocol[%d]\n", tds__SetNetworkProtocols->NetworkProtocols[l_i32_idx].Name);
            // onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:ServiceNotSupported", "The supplied network Protocol settings is not supported");
            return SOAP_FAULT;
        }
    }

    return SOAP_OK;
}

/** Web service operation '__tds__GetNetworkDefaultGateway' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkDefaultGateway(struct soap* soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
    printf("Called __tds__GetNetworkDefaultGateway ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

        tds__GetNetworkDefaultGatewayResponse->NetworkGateway = ((struct tt__NetworkGateway *)soap_malloc(soap, sizeof(struct tt__NetworkGateway)));
    MEMSET(tds__GetNetworkDefaultGatewayResponse->NetworkGateway, 0x00, sizeof(struct tt__NetworkGateway));
    tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address = 1;
    tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address =
        (char **)soap_malloc(soap, sizeof(char *) * tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address);
    MEMSET(tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address, 0x00, sizeof(char *) * tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address);
    tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address[0] = (char *)soap_malloc(soap, IPV4_ADDR_STRING_LEN);
    MEMSET(tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address[0], 0x00, IPV4_ADDR_STRING_LEN);
    snprintf(tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address[0], IPV4_ADDR_STRING_LEN, "%s", gateway); //"%d.%d.%d.%d", 
            // 255, //l_x_network.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_gateway[0],
            // 255, //l_x_network.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_gateway[1],
            // 255, //l_x_network.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_gateway[2],
            // 255); //l_x_network.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_gateway[3]);
    // if(0 != ((0 != l_x_network.m_x_ipv6Lanset.m_ui8_ipv6Enable) && ( STRLEN(l_x_network.m_x_ipv6Lanset.m_e_ip6Config.m_aui8_getway) != 0 )))
    // {
    //     tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv6Address = 1;
    //     tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv6Address =
    //         (char **)soap_malloc(soap, sizeof(char *)*tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv6Address);
    //     MEMSET(tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv6Address, 0x00, sizeof(CHARPTR)*tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv6Address);
    //     tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv6Address[0] = (CHARPTR)soap_malloc(soap, MAX_IPV6_GETWAY_ADDR_LEN + 1);
    //     MEMSET(tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv6Address[0], 0x00, MAX_IPV6_GETWAY_ADDR_LEN + 1);
    //     SNPRINTF(tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv6Address[0], MAX_IPV6_GETWAY_ADDR_LEN + 1, "%s", l_x_network.m_x_ipv6Lanset.m_e_ip6Config.m_aui8_getway);
    // }

    // free(gateway);

    return SOAP_OK;
}

/** Web service operation '__tds__SetNetworkDefaultGateway' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkDefaultGateway(struct soap* soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    if (NULL == tds__SetNetworkDefaultGateway)
    {
        onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidGatewayAddress", "The supplied gateway address was invalid.");
        printf("Invalid argument. tds__SetNetworkDefaultGateway[%p]\n", tds__SetNetworkDefaultGateway);
        return SOAP_FAULT;
    }

    if ((tds__SetNetworkDefaultGateway->__sizeIPv4Address <= 0) && (tds__SetNetworkDefaultGateway->__sizeIPv6Address <= 0))
    {
        return SOAP_OK;
    }

    if (tds__SetNetworkDefaultGateway->__sizeIPv4Address > 0)
    {
        if (!isValidIp4(tds__SetNetworkDefaultGateway->IPv4Address[0])) // Check IP address
        {
            printf("InvalidIPv4Address[%s]\n", tds__SetNetworkDefaultGateway->IPv4Address[0]);
            onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "The suggested IPv4 address is invalid.");
            return SOAP_FAULT;
        }
        else
        {
            printf("Provided Network Gateway IPv4[%s]\n", tds__SetNetworkDefaultGateway->IPv4Address[0]);
            sprintf(gateway,"%s",tds__SetNetworkDefaultGateway->IPv4Address[0]);

            // sscanf(tds__SetNetworkDefaultGateway->IPv4Address[0], "%d.%d.%d.%d", &l_i32_ip[0], &l_i32_ip[1], &l_i32_ip[2], &l_i32_ip[3]);

            // l_x_network.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_gateway[0] = (UINT8)l_i32_ip[0];
            // l_x_network.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_gateway[1] = (UINT8)l_i32_ip[1];
            // l_x_network.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_gateway[2] = (UINT8)l_i32_ip[2];
            // l_x_network.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_gateway[3] = (UINT8)l_i32_ip[3];
        }
    }

    if (tds__SetNetworkDefaultGateway->__sizeIPv6Address > 0)
    {
        onvif_fault(soap, 1, "ter:InvalidArgVal", "ter:InvalidGatewayAddress", "The supplied gateway address was invalid.");
        printf("Invalid argument. tds__SetNetworkDefaultGateway[%p]\n", tds__SetNetworkDefaultGateway);
        return SOAP_FAULT;
    }

    return SOAP_OK;
}

/** Web service operation '__tds__GetZeroConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetZeroConfiguration(struct soap* soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
    printf("Called __tds__GetZeroConfiguration ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    tds__GetZeroConfigurationResponse->ZeroConfiguration =
        (struct tt__NetworkZeroConfiguration *)soap_malloc(soap, (sizeof(struct tt__NetworkZeroConfiguration)));
    MEMSET(tds__GetZeroConfigurationResponse->ZeroConfiguration, 0x00, sizeof(struct tt__NetworkZeroConfiguration));
    //InterfaceToken
    tds__GetZeroConfigurationResponse->ZeroConfiguration->InterfaceToken = (char *)soap_malloc(soap, MAX_TOKEN_SIZE + 1);
    snprintf(tds__GetZeroConfigurationResponse->ZeroConfiguration->InterfaceToken, MAX_TOKEN_SIZE + 1, "%s","Zero Config1"); //l_x_zeroConfig.m_ca_InterfaceToken);
    //Enabled
    tds__GetZeroConfigurationResponse->ZeroConfiguration->Enabled = xsd__boolean__true_; // (l_x_zeroConfig.m_b_enable) ? xsd__boolean__true_ : xsd__boolean__false_;
    //__sizeAddresses
    tds__GetZeroConfigurationResponse->ZeroConfiguration->__sizeAddresses = 1; //l_x_zeroConfig.m_ui8_size;
    tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses = (char **)soap_malloc(soap, sizeof(char *) * (IPV4_ADDR_STRING_LEN + 1));

    for (int l_i32_idx = 0; l_i32_idx < tds__GetZeroConfigurationResponse->ZeroConfiguration->__sizeAddresses; l_i32_idx++)
    {
        tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[l_i32_idx] = (char *)soap_malloc(soap, (IPV4_ADDR_STRING_LEN + 1));
        MEMSET(tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[l_i32_idx], 0x00, (IPV4_ADDR_STRING_LEN + 1));
        snprintf(tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[l_i32_idx], IPV4_ADDR_STRING_LEN + 1, "%s",ip_address); //"10.67.97.35"); //l_x_zeroConfig.m_v_ipAddrList[l_i32_idx].m_aui8_ipAddress);
    }
    tds__GetZeroConfigurationResponse->ZeroConfiguration->Extension = NULL;

    return SOAP_OK;
}

/** Web service operation '__tds__SetZeroConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetZeroConfiguration(struct soap* soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetIPAddressFilter' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetIPAddressFilter(struct soap* soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SetIPAddressFilter' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetIPAddressFilter(struct soap* soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__AddIPAddressFilter' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__AddIPAddressFilter(struct soap* soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
    printf("Called __tds__AddIPAddressFilter ..... \n");

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return SOAP_OK;
}

/** Web service operation '__tds__RemoveIPAddressFilter' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveIPAddressFilter(struct soap* soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetAccessPolicy' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy(struct soap* soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__SetAccessPolicy' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy(struct soap* soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__CreateCertificate' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateCertificate(struct soap* soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__GetCertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates(struct soap* soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__GetCertificatesStatus' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificatesStatus(struct soap* soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__SetCertificatesStatus' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetCertificatesStatus(struct soap* soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__DeleteCertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteCertificates(struct soap* soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__GetPkcs10Request' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request(struct soap* soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__LoadCertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates(struct soap* soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__GetClientCertificateMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetClientCertificateMode(struct soap* soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SetClientCertificateMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetClientCertificateMode(struct soap* soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetRelayOutputs' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs(struct soap* soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SetRelayOutputSettings' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputSettings(struct soap* soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    
    return 0;
}

/** Web service operation '__tds__SetRelayOutputState' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputState(struct soap* soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SendAuxiliaryCommand' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SendAuxiliaryCommand(struct soap* soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetCACertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCACertificates(struct soap* soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    return 0;
}

/** Web service operation '__tds__LoadCertificateWithPrivateKey' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey(struct soap* soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;
    return 0;
}

/** Web service operation '__tds__GetCertificateInformation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation(struct soap* soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__LoadCACertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCACertificates(struct soap* soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__CreateDot1XConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateDot1XConfiguration(struct soap* soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SetDot1XConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDot1XConfiguration(struct soap* soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetDot1XConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfiguration(struct soap* soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetDot1XConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfigurations(struct soap* soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM_SECRET, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__DeleteDot1XConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteDot1XConfiguration(struct soap* soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetDot11Capabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Capabilities(struct soap* soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetDot11Status' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status(struct soap* soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__ScanAvailableDot11Networks' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks(struct soap* soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}


/** Web service operation '__tds__GetSystemUris' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris(struct soap* soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__StartFirmwareUpgrade' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__StartFirmwareUpgrade(struct soap* soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_UNRECOVERABLE, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__StartSystemRestore' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__StartSystemRestore(struct soap* soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_UNRECOVERABLE, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetStorageConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetStorageConfigurations(struct soap* soap, struct _tds__GetStorageConfigurations *tds__GetStorageConfigurations, struct _tds__GetStorageConfigurationsResponse *tds__GetStorageConfigurationsResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__CreateStorageConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateStorageConfiguration(struct soap* soap, struct _tds__CreateStorageConfiguration *tds__CreateStorageConfiguration, struct _tds__CreateStorageConfigurationResponse *tds__CreateStorageConfigurationResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetStorageConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetStorageConfiguration(struct soap* soap, struct _tds__GetStorageConfiguration *tds__GetStorageConfiguration, struct _tds__GetStorageConfigurationResponse *tds__GetStorageConfigurationResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SetStorageConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetStorageConfiguration(struct soap* soap, struct _tds__SetStorageConfiguration *tds__SetStorageConfiguration, struct _tds__SetStorageConfigurationResponse *tds__SetStorageConfigurationResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__DeleteStorageConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteStorageConfiguration(struct soap* soap, struct _tds__DeleteStorageConfiguration *tds__DeleteStorageConfiguration, struct _tds__DeleteStorageConfigurationResponse *tds__DeleteStorageConfigurationResponse)
{

    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__GetGeoLocation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetGeoLocation(struct soap* soap, struct _tds__GetGeoLocation *tds__GetGeoLocation, struct _tds__GetGeoLocationResponse *tds__GetGeoLocationResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__SetGeoLocation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetGeoLocation(struct soap* soap, struct _tds__SetGeoLocation *tds__SetGeoLocation, struct _tds__SetGeoLocationResponse *tds__SetGeoLocationResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}

/** Web service operation '__tds__DeleteGeoLocation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteGeoLocation(struct soap* soap, struct _tds__DeleteGeoLocation *tds__DeleteGeoLocation, struct _tds__DeleteGeoLocationResponse *tds__DeleteGeoLocationResponse)
{
    int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_WRITE_SYSTEM, NULL, NULL);  // ACCESS_READ_SYSTEM
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;

    return 0;
}
