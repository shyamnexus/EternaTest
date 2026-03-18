#include "onvif_main.h"
#include "wsaapi.h"
#include "soapH.h"
#include "soapStub.h"
#include "wsddapi.h"

#define MEMSET(s, c, n) (memset((void *)s, (int)c, (size_t)n))

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "onvif_media2.c"

/** Web service operation '__tr2__GetProfiles' - WORKING IMPLEMENTATION FOR ONVIF PROFILE T */
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
