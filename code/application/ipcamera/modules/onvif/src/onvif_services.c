#ifdef FALSE
/******************************************************************************
 *
 * File Name : onvif_services.c
 * Date : June 25, 2024
 * Author : H447264
 *****************************************************************************/

/*****************************************************************************************************
 *                               onvif_services INCLUDE
 ****************************************************************************************************/
#include "onvif_services.h"

/**
 *  @brief      This function validates user accessibilities
 *
 *  @param      soap[in]   Pointer to soap.
 *  @param      AccessClass[in]   access class of command
 *  @param      username[out]   username
 *  @param      password[out]   password
 *  @param      checkAccountLock[input] flag to indicate weather we should check account lock flag or not. default no.
 *
 *  @return     int 	Returns accessibilitis status. Success or fail
 */
int ValidateAccessibility(struct soap *soap, ACCESS_CLASS AccessClass, char* username, char* password, bool checkAccountLock, char* method)
{
    int ret = SOAP_OK; 

    do
    {
        /* If no need to check authentication then return with success */
        if (ACCESS_PRE_AUTH == AccessClass)
        {
            _wsse__Security *security = soap_wsse_Security(soap);
            if (security != NULL)
            {
                soap_wsse_delete_Security(soap);
            }

            break;
        }

        // if(0 != strlen(soap->sessionIDReceive))
        // {
        //     bool isValid = IsSessionIdValid(soap->sessionIDReceive);

        //     if(!isValid)
        //     {
        //         printf("Invalid Session-Id is %s",soap->sessionIDReceive);
                // if (TRUE == soap->isUIReq)
                // {
                //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:SessionExpired", ErrorCodeConversion(RCF_INVALID_SESSION_ID));
                // }
                // else
                // {
                //     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:SessionExpired", "Session-Id Expired");
                // }
        //         l_i32_ret = SOAP_FAULT;
        //         break;
        //     }
        // }

#if WS_SECURITY
        switch (ws_authentication(soap, username, password, checkAccountLock, method))
        {
        case ADMIN_USER:
            break;

        case OPERATOR_USER:
            if ((ACCESS_READ_SYSTEM_SECRET == AccessClass) ||
                (ACCESS_WRITE_SYSTEM == AccessClass) ||
                (ACCESS_UNRECOVERABLE == AccessClass))
            {
                ret = SOAP_FAULT;
             
                // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", "Not authorised operator user");
            }
            break;

        case VIEWER_USER:
            if ((ACCESS_READ_SYSTEM_SECRET == AccessClass) ||
                (ACCESS_WRITE_SYSTEM == AccessClass) ||
                (ACCESS_UNRECOVERABLE == AccessClass) ||
                (ACCESS_ACTUATE == AccessClass))
            {
                ret = SOAP_FAULT;
                // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", "Not authorised viewer user");
            }
            break;

        case AUTHENTICATION_FAIL:
            ret = AUTHENTICATION_FAIL;
            break;
        default:
            ret = SOAP_FAULT;
            break;
        }
#endif
    } while (false);

    return ret;
}

/**
 * @brief   This function generate fault XML.
 * @param soap
 * @param flag
 * @param value1
 * @param value2
 * @param reason
 * @return
 */
int onvif_fault(struct soap *soap, int flag, char* value1, char* value2, char* reason)
{
    char* envSender = (char*) "SOAP-ENV:Sender";
    char* envReceiver = (char*) "SOAP-ENV:Receiver";

    soap->fault = (struct SOAP_ENV__Fault *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Fault)));
    soap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));
    if (flag == 1)
        soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = envSender;
    else
        soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = envReceiver;

    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode = (struct SOAP_ENV__Code *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = (char*)value1;
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = (struct SOAP_ENV__Code *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Value = (char*)value2;
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
    soap->fault->faultcode = NULL;
    soap->fault->faultstring = NULL;
    soap->fault->faultactor = NULL;
    soap->fault->detail = NULL;
    soap->fault->SOAP_ENV__Reason = (struct SOAP_ENV__Reason *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Reason)));
    soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text = (char*)reason;
    soap->fault->SOAP_ENV__Node = NULL; //"http://www.w3.org/2003/05/soap-envelope/node/ultimateReceiver";
    soap->fault->SOAP_ENV__Role = NULL; //"http://www.w3.org/2003/05/soap-envelope/role/ultimateReceiver";
    soap->fault->SOAP_ENV__Detail = NULL;
    return 0;
}

/**
 * @brief   This function generate fault XML.
 * @param soap
 * @param value1
 * @param reason
 * @return NONE
 */
void onvif_fault_probe(struct soap *soap, char* value1, char* reason)
{
    char* envSender = (char*) "SOAP-ENV:Sender";
    soap->fault = (struct SOAP_ENV__Fault *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Fault)));
    soap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = envSender;

    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode = (struct SOAP_ENV__Code *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));
    ;
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = (char*)value1;
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
    soap->fault->faultcode = NULL;
    soap->fault->faultstring = NULL;
    soap->fault->faultactor = NULL;
    soap->fault->detail = NULL;
    soap->fault->SOAP_ENV__Reason = (struct SOAP_ENV__Reason *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Reason)));
    soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text = (char*)reason;
    soap->fault->SOAP_ENV__Node = NULL;
    soap->fault->SOAP_ENV__Role = NULL;
    soap->fault->SOAP_ENV__Detail = NULL;
}
#endif
