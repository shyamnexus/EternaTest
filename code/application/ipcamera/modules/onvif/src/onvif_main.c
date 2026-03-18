
#include <sys/ioctl.h>
#include <unistd.h>
// #define __USE_MISC 1
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "soapH.h"
#include "stdsoap2.h"
#include "onvif_main.h"
#include "httppost.h"
#include "httpget.h"

#include "wsaapi.h"
#include "wsseapi.h"
#include "httpda.h"
#include "wsddapi.h"

#include "onvif_adapter.h"
#include "ipcam/user_wrapper.h"


#ifndef ONVIF_TEST
// #include "ntp.h"
// #include "network.h" // TODO: Add Novatek network header when needed
#endif

#define INFO_BUFFER_LENGTH 1024

/* Forward declaration */
int32_t ws_authentication_imp(struct soap *soap, CHARPTR i_pc_username, CHARPTR i_pc_password, bool i_b_checkAccountLock, CCHARPTR i_req_method);


char *ip_address = "192.168.1.6";    //"10.67.97.35";

// Static buffers for network configuration - required by SetNetworkInterfaces
// Previously these were NULL pointers causing crashes when sprintf wrote to them
static char temp_ip_address_buf[32] = {0};
static char temp_method_buf[16] = {0};
static char interface_buf[16] = {0};
static char method_buf[16] = {0};
static char address_buf[32] = {0};
static char netmask_buf[32] = {0};
static char gateway_buf[32] = {0};
static char temp_netmask_buf[32] = {0};
static char temp_gateway_buf[32] = {0};
static char ip_assign_method_buf[16] = "dhcp";

char *temp_ip_address = temp_ip_address_buf;
char *temp_method = temp_method_buf;
char *interface = interface_buf;
char *method = method_buf;
char *address = address_buf;
char *netmask = netmask_buf;
char *gateway = gateway_buf;
char *temp_netmask = temp_netmask_buf;
char *temp_gateway = temp_gateway_buf;
unsigned char macAddress[7] = {0};

char *ip_assign_method = ip_assign_method_buf;

unsigned char mamacAddressc_str[7] ={0};

bool ip_address_update_flag = false;
bool ip_address_update_request = false;
bool g_b_IsSysQuit = false;

char isDiscoveryOn = 1;
char dateFormatWithoutFirstFourChars[20] = {0};

// TODO: Initialize with proper values from Novatek platform
T_NETWORK_PROTOCOL_SETTINGS network_protocol = {0};
T_NTP_SETTING ntp_server = {0};

// Mutex locks for WS-Discovery
pthread_mutex_t g_probeLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_helloLock = PTHREAD_MUTEX_INITIALIZER;

char *osdTimeFormat[2] = {"HH:mm:ss", "hh:mm:ss tt"};
char *osdTextTypes[2] = {"DateAndTime", "PlainText"};
char *osdPositions[6] = {"UpperRight", "UpperLeft", "LowerRight", "LowerLeft", "TopCenter", "Custom"};
char *osdDateFormat[6] = {"yyyy-MM-dd", "MM-dd-yyyy", "dd-MM-yyyy", "yyyy/MM/dd", "MM/dd/yyyy", "dd-MM-yyyy"};
void hexToRGB(unsigned int hexValue, int *r, int *g, int *b) {
    *r = ((hexValue >> 16) & 0xFF); // Extract the RR component
    *g = ((hexValue >> 8) & 0xFF);  // Extract the GG component
    *b = (hexValue & 0xFF);         // Extract the BB component
}

struct http_post_handlers my_handlers[] = {
    {"application/octet-stream", NULL}, //generic_POST_handler},
    {"font/ttf", NULL}, //generic_POST_handler},
    {"multipart/form-data", NULL}, //generic_POST_handler},
    {NULL}};

void RGBtoYCbCr(int r, int g, int b, double *Y, double *Cb, double *Cr) {
    *Y = 0.299 * r + 0.587 * g + 0.114 * b;
    *Cb = -0.168736 * r - 0.331264 * g + 0.5 * b + 128;
    *Cr = 0.5 * r - 0.418688 * g - 0.081312 * b + 128;
}


// Function to convert YCbCr to RGB, then format as a hexadecimal string
// THREAD-SAFETY FIX: Caller must provide a buffer instead of using a static one.
// Legacy callers can use ycbcrToHex_legacy() which uses thread-local storage.
int ycbcrToHex_r(float y, float cb, float cr, char *buf, int buf_size) {
    if (!buf || buf_size < 10) return -1;

    int Y = round(y);
    int Cb = round(cb);
    int Cr = round(cr);

    int R = (int)(Y + 1.402 * (Cr - 128));
    int G = (int)(Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128));
    int B = (int)(Y + 1.772 * (Cb - 128));

    R = R < 0 ? 0 : R > 255 ? 255 : R;
    G = G < 0 ? 0 : G > 255 ? 255 : G;
    B = B < 0 ? 0 : B > 255 ? 255 : B;

    snprintf(buf, buf_size, "0x%02X%02X%02X", R, G, B);
    return 0;
}

// Legacy wrapper using thread-local storage for backward compatibility
char* ycbcrToHex(float y, float cb, float cr) {
    static __thread char hexColor[10];
    ycbcrToHex_r(y, cb, cr, hexColor, sizeof(hexColor));
    return hexColor;
}

void hexToYCbCr(const char *hexColor, float *Y, float *Cb, float *Cr) {
    // Step 1: Parse the hexadecimal color
    int R, G, B;
    sscanf(hexColor, "0x%02x%02x%02x", &R, &G, &B);

    // Step 2: Convert RGB to YCbCr using BT.601
    *Y = 0.299 * R + 0.587 * G + 0.114 * B;
    *Cb = -0.168736 * R - 0.331264 * G + 0.5 * B + 128;
    *Cr = 0.5 * R - 0.418688 * G - 0.081312 * B + 128;

    // Ensure Y, Cb, Cr are within their respective ranges
    *Y = *Y < 0 ? 0 : *Y > 255 ? 255 : *Y;
    *Cb = *Cb < 0 ? 0 : *Cb > 255 ? 255 : *Cb;
    *Cr = *Cr < 0 ? 0 : *Cr > 255 ? 255 : *Cr;
}

void omitFirstFourChars(const char *input, char *output) {
                // Check if the input string is long enough
                if (strlen(input) > 4) {
                    // Copy the string starting from the 5th character (index 4)
                    strcpy(output, input + 4);
                } else {
                    // If the input string is too short, just return an empty string
                    strcpy(output, "");
                }
}

void rescaleCoordinates(float x, float y, int *rescaledX, int *rescaledY) {
        int canvasWidth = 1920;
        int canvasHeight = 1080;

        // Rescale according to the canvas size
        float rescaledFloatX = (x + 1) * (canvasWidth / 2.0); // Adjusting as if the origin is at the center
        float rescaledFloatY = (1 - y) * (canvasHeight / 2.0); // Adjusting as if the origin is at the center

        // Convert to integer values for pixel coordinates
        *rescaledX = (int)rescaledFloatX;
        *rescaledY = (int)rescaledFloatY;
        LOG_INFO("Rescaled coordinates: (%d, %d)\n", *rescaledX, *rescaledY);
}

void inverseRescaleCoordinates(int position_x, int position_y, float *x, float *y) {
    int canvasWidth = 1920;
    int canvasHeight = 1080;

    // Adjusting as if the origin is at the center and converting back to normalized values
    *x = ((float)position_x / (canvasWidth / 2.0)) - 1;
    *y = 1- ((float)position_y / (canvasHeight / 2.0));
} 

char* strupr(char* s) {
    char* tmp = s;
    while (*tmp) {
        if ('a' <= *tmp && *tmp <= 'z') {
            *tmp = *tmp - 'a' + 'A';
        }
        tmp++;
    }
    return s;
}

char* strlwr(char* s) {
    char* tmp = s;
    while (*tmp) {
        if ('A' <= *tmp && *tmp <= 'Z') {
            *tmp = *tmp - 'A' + 'a';
        }
        tmp++;
    }
    return s;
}

static SOAP_SOCKET SoapBind(struct soap *pSoap, const char *pIp, bool flag)
{
	SOAP_SOCKET sockFD = SOAP_INVALID_SOCKET;
	if (flag)
	{
		sockFD = soap_bind(pSoap, onvifUdpAddress, pSoap->port, 100);
		if (soap_valid_socket(sockFD))
		{
			//printf("%s:%s:%d flag = %d, sockFD = %d, pSoap->master = %d\n", __FILE__, __func__, __LINE__, flag, sockFD, pSoap->master);
		}
		else
		{
			//printf("%s:%s:%d flag = %d\n", __FILE__, __func__, __LINE__, flag);
			soap_print_fault(pSoap, stderr);
		}
	}
	else
	{
		sockFD = soap_bind(pSoap, pIp, pSoap->port, 10);
		if (soap_valid_socket(sockFD))
			printf("%s:%s:%d flag = %d, sockFD = %d, pSoap->master = %d\n", __FILE__, __func__, __LINE__, flag, sockFD, pSoap->master);
		else
		{
			printf("%s:%s:%d flag = %d\n", __FILE__, __func__, __LINE__, flag);
			soap_print_fault(pSoap, stderr);
		}
	}

	return sockFD;
}

int dom_att(struct soap_dom_attribute *dom, int num_of_attributes, char * name[], char * data[], char * nstr, wchar_t *wide, struct soap *soap)
{
    int i = 0;
    struct soap_dom_attribute *att;
    att = dom;
    while (dom)
    {
        dom->nstr = NULL;
        if (name == NULL)
            dom->name = NULL;
        else
            dom->name = name[i];

        if (data == NULL)
            dom->text = NULL;
        else
            dom->text = data[i];
        dom->soap = soap;
        if (num_of_attributes > 0)
        {
            dom->next = (struct soap_dom_attribute *)soap_malloc(soap, sizeof(struct soap_dom_attribute));
            dom = dom->next;
            num_of_attributes--;
            i++;
        }
        else
        {
            dom->next = NULL;
            return 0;
        }
    }
    return 0;
}

int onvif_fault(struct soap *soap, int flag, CCHARPTR value1, CCHARPTR value2, CCHARPTR reason)
{
    CHARPTR envSender = (CHARPTR) "SOAP-ENV:Sender";
    CHARPTR envReceiver = (CHARPTR) "SOAP-ENV:Receiver";

    soap->fault = (struct SOAP_ENV__Fault *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Fault)));
    soap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));
    if (flag == 1)
        soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = envSender;
    else
        soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = envReceiver;

    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode = (struct SOAP_ENV__Code *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = (CHARPTR)value1;
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = (struct SOAP_ENV__Code *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Code)));
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Value = (CHARPTR)value2;
    soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
    soap->fault->faultcode = NULL;
    soap->fault->faultstring = NULL;
    soap->fault->faultactor = NULL;
    soap->fault->detail = NULL;
    soap->fault->SOAP_ENV__Reason = (struct SOAP_ENV__Reason *)soap_malloc(soap, (sizeof(struct SOAP_ENV__Reason)));
    soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text = (CHARPTR)reason;
    soap->fault->SOAP_ENV__Node = NULL; //"http://www.w3.org/2003/05/soap-envelope/node/ultimateReceiver";
    soap->fault->SOAP_ENV__Role = NULL; //"http://www.w3.org/2003/05/soap-envelope/role/ultimateReceiver";
    soap->fault->SOAP_ENV__Detail = NULL;
    return 0;
}/**
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

/**
 *  @brief This function is onvif request handler thread, it will initialize gSoap and pass user request.
 *  @param arg	incoming request data.
 *  @return NULL
 */
void *thread_OnvifReqHandler(void *arg)
{
    struct soap *soaptcp = (struct soap *)arg;

    THREAD_DETACH(THREAD_ID);
    printf("Passing Request to gSoap\n");
    if (soaptcp)
    {

         if( soap_serve(soaptcp) != SOAP_OK )
         {
            printf("Exception - Failed to serve below request with error code - %d\n%s", soaptcp->error, soaptcp->buf);
            onvif_fault(soaptcp, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Got exception while serving this request");
            soaptcp->error = SOAP_FAULT;

         }
         else
         {
             printf("Response Send To User\n");
         }

        soap_destroy(soaptcp);
        soap_end(soaptcp);
        soap_free(soaptcp);
    }
    printf("End Of gSoap Server\n");

    return NULL;
}




void *OnvifWSDiscoveryServer(void *arg) {

    //printf("[%s][%d][%s][%s] start \n", __FILE__, __LINE__, __TIME__, __func__);
    
    struct soap UDPserverSoap = {0};
    struct ip_mreq mcast;

    soap_init1(&UDPserverSoap, SOAP_IO_UDP | SOAP_XML_IGNORENS);
    soap_set_namespaces(&UDPserverSoap,  namespaces);

    //printf("[%s][%d][%s][%s] UDPserverSoap.version = %d \n", __FILE__, __LINE__, __TIME__, __func__, UDPserverSoap.version);

    int m = soap_bind(&UDPserverSoap, NULL, ONVIF_UDP_PORT, 10);
    if(!soap_valid_socket(m))
    {
        soap_print_fault(&UDPserverSoap, stderr);
        soap_done(&UDPserverSoap);
        pthread_exit(0);
    }
    //printf("socket bind success %d\n", m);

    mcast.imr_multiaddr.s_addr = inet_addr(onvifUdpAddress);
    mcast.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(UDPserverSoap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
    {
            //printf("setsockopt error! error code = %d,err string = %s\n",errno,strerror(errno));
        return 0;
    }
    
    int fd = -1;
    while(!g_b_IsSysQuit)
    {
        //printf("socket connect %d\n", fd);
        fd = soap_accept(&UDPserverSoap);
        if (!soap_valid_socket(fd)) {
            // Check if shutdown was requested
            if (g_b_IsSysQuit) {
                break;
            }
            // Timeout or temporary error - log and continue
            if (UDPserverSoap.errnum == SOAP_TCP_ERROR || UDPserverSoap.error == SOAP_EOF) {
                // Timeout, just continue the loop
                soap_closesock(&UDPserverSoap);
                continue;
            }
            soap_print_fault(&UDPserverSoap, stderr);
            // Don't exit - just continue trying
            soap_closesock(&UDPserverSoap);
            usleep(100000); // 100ms delay before retry
            continue;
        }

        if( soap_serve(&UDPserverSoap) != SOAP_OK )
        {
            soap_print_fault(&UDPserverSoap, stderr);
            //printf("soap_print_fault\n");
        }

        //printf("IP = %u.%u.%u.%u\n", ((UDPserverSoap.ip)>>24)&0xFF, ((UDPserverSoap.ip)>>16)&0xFF, ((UDPserverSoap.ip)>>8)&0xFF,(UDPserverSoap.ip)&0xFF);
        soap_destroy(&UDPserverSoap);
        soap_end(&UDPserverSoap);
    }
    soap_done(&UDPserverSoap);
    pthread_exit(0);
}	


void *OnvifWebServiesServer(void *arg) 
{
    struct soap tcpSoap = {0};
	soap_init(&tcpSoap);
	
	// IMPORTANT: Disable X-Frame-Options header in gSOAP
	// nginx already adds this header. Duplicate headers cause DUTI 
	// (ONVIF Device Test Tool) to fail with "HTTP packet parsing error"
	tcpSoap.x_frame_options = NULL;
	
	tcpSoap.port = ONVIF_LISTEN_PORT;  // Internal port, nginx proxies from port 80
	tcpSoap.bind_flags = SO_REUSEADDR;  
	tcpSoap.accept_timeout = tcpSoap.recv_timeout = tcpSoap.send_timeout = 10000;
	soap_set_namespaces(&tcpSoap, namespaces);

    // HTTP Post Handlers registration 
    soap_register_plugin_arg(&tcpSoap, http_post, my_handlers);
    // // HTTP Get Handler registration
    // soap_register_plugin_arg(&tcpSoap, http_get, (void *)generic_GET_handler);
    // // Http Digest Authentication plugins register
    soap_register_plugin_arg(&tcpSoap, http_da, http_da_md5());
    
    soap_register_plugin(&tcpSoap, soap_wsse);





    // Bind to all interfaces (0.0.0.0) so nginx can proxy via 127.0.0.1
    // Note: ip_address was used before but nginx on 127.0.0.1 couldn't connect
    int tcpsocket = SoapBind(&tcpSoap, NULL, false);  // NULL = 0.0.0.0, all interfaces
    if (!soap_valid_socket(tcpsocket)) {
        //printf("tcpsocket SoapBind failed!\n");
        soap_print_fault(&tcpSoap, stderr);
        soap_done(&tcpSoap);
        pthread_exit(0);
    }
    
    int tcp_fd = -1;
    while(!g_b_IsSysQuit) {

        if(ip_address_update_flag == true)
        {
            printf("IP Address changed. Reinitializing ONVIF Socket\n");
            ip_address_update_flag = false;
            tcp_fd = -1;
            soap_force_closesock(&tcpSoap);
            soap_destroy(&tcpSoap);
            soap_end(&tcpSoap);
            soap_free(&tcpSoap);
            soap_done(&tcpSoap);

            //*******************************************************************
            // while(1)
            // {
            //     printf("**********Before rk_network_ipv4_get : %s ******************************\n",method);
            //     rk_network_ipv4_get("eth0", method, ip_address, netmask, gateway); 
            //     printf("**********After rk_network_ipv4_get ******************************\n");
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
            //         // rk_network_get_mac("eth0", macAddress);
            //         // printf("MAC Address :%02X%02X%02X%02X%02X%02X\n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
            //         //************ONVIF ******************************
            //         ip_address_update_flag = 1;
            //         break;
            //     }
            // }


            //********************************************************************
            soap_init(&tcpSoap);
            tcpSoap.x_frame_options = NULL;  // Disable - nginx adds this header
            tcpSoap.port = ONVIF_LISTEN_PORT;  // Internal port, nginx proxies from port 80
            tcpSoap.bind_flags = SO_REUSEADDR;  
            soap_set_namespaces(&tcpSoap, namespaces);

            // Bind to all interfaces so nginx can connect via 127.0.0.1
            tcpsocket = SoapBind(&tcpSoap, NULL, false);
            if (!soap_valid_socket(tcpsocket)) 
            {
                //printf("tcpsocket SoapBind failed!\n");
                soap_print_fault(&tcpSoap, stderr);
                // Don't exit, wait and retry
                usleep(1000000); // 1 second delay
                continue;
            }
        }
        //printf("socket connect %d\n", tcp_fd);
        // Check for shutdown before accept
        if (g_b_IsSysQuit) {
            break;
        }
        tcp_fd = soap_accept(&tcpSoap);
        if (!soap_valid_socket(tcp_fd)) {
            // Check if shutdown was requested
            if (g_b_IsSysQuit) {
                break;
            }
            // Timeout or temporary error - log and continue
            if (tcpSoap.errnum == SOAP_TCP_ERROR || tcpSoap.error == SOAP_EOF) {
                // Timeout, just continue the loop
                soap_closesock(&tcpSoap);
                continue;
            }
            soap_print_fault(&tcpSoap, stderr);
            // Don't exit - just continue trying
            soap_closesock(&tcpSoap);
            usleep(100000); // 100ms delay before retry
            continue;
        }

        if( soap_serve(&tcpSoap) != SOAP_OK )
        {
            soap_print_fault(&tcpSoap, stderr);
            //printf("soap_print_fault\n");
        }        
        
        //printf("IP = %u.%u.%u.%u\n", ((tcpSoap.ip)>>24)&0xFF, ((tcpSoap.ip)>>16)&0xFF, ((tcpSoap.ip)>>8)&0xFF,(tcpSoap.ip)&0xFF);
        soap_destroy(&tcpSoap);
        soap_end(&tcpSoap);
    }
    
    
    soap_done(&tcpSoap);
    pthread_exit(0);
}

//******************Get IP Address*********************************************
in_addr_t net_get_ifaddr(char *ifname, int *ret)
{
    struct ifreq ifr;
    int skfd;
    struct sockaddr_in *saddr;
    *ret = 0;

    if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    {
        //printf("socket error\n");
        *ret = -1;
        return -1;
    }
		
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) 
    {
        close(skfd);
        *ret = -1;
        return -1;
    }
    close(skfd);

    *ret = 0;
    saddr = (struct sockaddr_in *) &ifr.ifr_addr;
    return saddr->sin_addr.s_addr;
}

// Mutex for thread-safe IP address access
static pthread_mutex_t ip_address_mutex = PTHREAD_MUTEX_INITIALIZER;

int get_ip_address(void){
    // Use static buffer to avoid memory leak when called repeatedly
    static char ip_address_buffer[INET_ADDRSTRLEN] = {0};
    int fd;
    struct ifreq ifr;

    pthread_mutex_lock(&ip_address_mutex);

     //F_INET - to define network interface IPv4
    //Creating soket for it.
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (fd < 0) 
    {
        perror("socket");
        pthread_mutex_unlock(&ip_address_mutex);
        return -1;
    }

     //F_INET - to define IPv4 Address type.
    ifr.ifr_addr.sa_family = AF_INET;
    //eth0 - define the ifr_name - port name
    //where network attached.
    memcpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
    //Accessing network interface information by
    //passing address using ioctl.
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl SIOCGIFADDR");
        close(fd);
        pthread_mutex_unlock(&ip_address_mutex);
        return -1;
    }
     //closing fd
    close(fd);

    // Use thread-safe inet_ntop instead of inet_ntoa
    struct sockaddr_in *addr = (struct sockaddr_in*)&ifr.ifr_addr;
    if (inet_ntop(AF_INET, &addr->sin_addr, ip_address_buffer, sizeof(ip_address_buffer)) == NULL) {
        perror("inet_ntop");
        pthread_mutex_unlock(&ip_address_mutex);
        return -1;
    }
    
    ip_address = ip_address_buffer;
      
    printf("------------------------System IP Address is-----------------: %s\n", ip_address);
    
    pthread_mutex_unlock(&ip_address_mutex);
    return 0;
}

//************Get Hardware Address - MAC **************************************
int get_hw_addr(char *ifname, unsigned char *mac)
{
    struct ifreq ifr;
    int skfd;

    if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    {
        //printf("socket error\n");
        return -1;
    }

     //printf("ifname : %s\n", ifname);

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) 
    {
        //printf("net_get_hwaddr: ioctl SIOCGIFHWADDR\n");
        close(skfd);
        return -1;
    }
    close(skfd);

    memcpy(mac, ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
    return 0;
}


#ifdef ONVIF_TEST

int main(int argc,char ** argv)
{
    pthread_t discover = 0;
    pthread_t webservice = 0;
    pthread_t ntp_client_thread_id;
     unsigned char ipAddressFound = false;
     unsigned char mac_str[7];

    // sprintf(ip_address, "10.67.97.35");

    get_hw_addr("eth0", mac_str);

    //printf("MAC Address :%s\n", mac_str);

    // ntp_update("200.20.186.76"); 

	pthread_create(&discover, NULL, OnvifWSDiscoveryServer, NULL);
	pthread_create(&discover, NULL, OnvifWebServiesServer, NULL);

    rk_network_init_1(NULL);

    // ip_address = NULL;
    
    // while(1)
    // {
    //     if(!ipAddressFound){
    //         get_ip_address();
    //         if (ip_address!= NULL) {
    //         printf("-----------------------IP address of eth0:-------------------------------------------- %s\n", ip_address);
    //         ipAddressFound = true;
    //        }
    //         else {
    //         printf("Failed to retrieve IP address of eth0\n");
    //         printf("*********************************************Failed to retrieve IP address of eth0*************************************************************... \n");
    //         }
    //     }
    //     else if(ipAddressFound)
    //     {
    //         //************ONVIF ******************************
    //         printf("*****************Initializing ONVIF Server***************************************************************... \n");
    //         pthread_create(&discover, NULL, OnvifWSDiscoveryServer, NULL);
    //         pthread_create(&webservice, NULL, OnvifWebServiesServer, NULL);
    //         break;
    //     }
    // }
	
	pthread_join(discover, 0);
    pthread_join(webservice, 0);
	
    return 0;
}
#endif

// int main(int argc,char **argv)
// {
//     printf("Starting discovery...\n");

//     struct soap udpSoap = {0};
//     soap_init1(&udpSoap, SOAP_IO_UDP | SOAP_XML_IGNORENS);
//     soap_set_namespaces(&udpSoap, namespaces);

//     int sockFd = -1;
//     sockFd = soap_bind(&udpSoap, NULL, ONVIF_UDP_PORT, 10);
//     if (!soap_valid_socket(sockFd)) {
//         soap_print_fault(&udpSoap, stderr);
//         exit(1);
//     }
//     printf("Socket bind successful\n");

//     struct ip_mreq mcast;
//     mcast.imr_multiaddr.s_addr = inet_addr(onvifUdpAddress);
//     mcast.imr_interface.s_addr = htonl(INADDR_ANY);
//     if (setsockopt(udpSoap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0) {
//         printf("setsockopt error! error code = %d, err string = %s\n", errno, strerror(errno));
//         return;
//     }

//     while (1) {
//         printf("Waiting for connections...\n");
//         int fd = soap_accept(&udpSoap);
//         printf("accepted a connection from:");
//         printf("IP = %u.%u.%u.%u\n", ((udpSoap.ip)>>24)&0xFF, ((udpSoap.ip)>>16)&0xFF, ((udpSoap.ip)>>8)&0xFF, (udpSoap.ip)&0xFF);
//         if (!soap_valid_socket(fd)) {
//             soap_print_fault(&udpSoap, stderr);
//             exit(1);
//         }

//         if (soap_serve(&udpSoap) != SOAP_OK) {
//             soap_print_fault(&udpSoap, stderr);
//             printf("soap_print_fault\n");
//         }

//         printf("IP = %u.%u.%u.%u\n", ((udpSoap.ip)>>24)&0xFF, ((udpSoap.ip)>>16)&0xFF, ((udpSoap.ip)>>8)&0xFF, (udpSoap.ip)&0xFF);
//         soap_destroy(&udpSoap);
//         soap_end(&udpSoap);
//     }

//     soap_done(&udpSoap);
//     printf("Discovery finished\n");
// } 


//*********************************************************************************************************************************

/**
 *  @brief  This function will authenticate User. It will get user account settings from system server and
 *          validate user details in onvif request with response received from system server.
 *
 *  @param      soap*   Pointer to soap.
 *  @param      i_pc_username[out]   username
 *  @param      i_pc_password[out]   password
 *  @param      i_b_checkAccountLock[input] flag to indicate weather we should check account lock flag or not. default no.
 *  @return     INT32 	Return user level or Error code or AUTHENTICATION_FAIL
 */
int32_t ws_authentication(struct soap *soap, CHARPTR i_pc_username, CHARPTR i_pc_password, bool i_b_checkAccountLock, CCHARPTR i_req_method)
{
    int32_t l_i32_ret = AUTHENTICATION_FAIL;
    bool l_b_isUserFound = false;
    bool isDigestAuthSucceed = true;
    CCHARPTR l_pc_username;
    _wsse__Security *security = soap_wsse_Security(soap);
    // T_USER_ACCOUNT_SETTING user_DB;
    int length = 0;
    int32_t nretCode = -1; //RCF_FAIL;

    do
    {

        // nretCode = getSysServerResp(GET_USER_ACC_SETTINGS, NULL, 0, &user_DB, length);
        // if (RCF_SUCCESS != nretCode)
        // {
        //     RCF_PRINTF(PRIO_ERROR, "Failed to Get User Information From SysServer - Dbus operation failed");
        //     if (TRUE == soap->isUIReq)
        //     {
        //         onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", ErrorCodeConversion(nretCode));
        //     }
        //     else
        //     {
        //         onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Fail To Get User accounts");
        //     }
        //     l_i32_ret = AUTHENTICATION_FAIL;
        //     break;
        // }

        // if (TRUE == user_DB.m_ui8_isFactoryDefault)
        // {
        //     RCF_PRINTF(PRIO_DEBUG, "Device Currently working in Factory Default state");
        //     l_i32_ret = ADMIN_USER;
        //     break;
        // }

        // Validate required parameters in response
        // for (vector<T_USER_ACCOUNT_CS>::iterator it = user_DB.m_lx_userAccntList.begin();
        //      it != user_DB.m_lx_userAccntList.end(); ++it)
        // {
        //     if ((USER_ADMIN != it->m_x_userAcc.m_e_userType) && (USER_VIEWER != it->m_x_userAcc.m_e_userType) &&
        //         (USER_OPERATOR != it->m_x_userAcc.m_e_userType))
        //     {
        //         RCF_PRINTF(PRIO_ERROR, "Invalid User Type Received from system server in response");
        //         if (TRUE == soap->isUIReq)
        //         {
        //             onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", ErrorCodeConversion(RCF_INVALID_RESPONSE_RECEIVED));
        //         }
        //         else
        //         {
        //             onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:InternalError", "Invalid response received");
        //         }
        //         l_i32_ret = INVALID_RESPONSE_RECEIVED;
        //         break;
        //     }
        // }

        // if (INVALID_RESPONSE_RECEIVED == l_i32_ret)
        // {
        //     l_i32_ret = AUTHENTICATION_FAIL;
        //     break;
        // }
        // RCF_PRINTF(PRIO_DEBUG, "Got Valid User Information From SysServer");

        // if (soap->isHttpDiagest)
        // {
        //     printf("User Validating From Digest authentication\n");
        //     /* Digest authentication */
        //     if ((NULL != soap->authrealm) && (NULL != soap->userid))
        //     {
        //         l_pc_username = soap->userid;
        //         if ((NULL != i_pc_username) && (NULL != l_pc_username))
        //         {
        //             SNPRINTF(i_pc_username, MAX_USER_NAME_LEN + 1, "%s", l_pc_username);
        //         }

        //         for (vector<T_USER_ACCOUNT_CS>::iterator it = user_DB.m_lx_userAccntList.begin();
        //              (it != user_DB.m_lx_userAccntList.end()) && (NULL != l_pc_username); ++it)
        //         {
        //             if (0 == STRCMP((CCHARPTR)it->m_x_userAcc.m_ai8_username, l_pc_username))
        //             {
        //                 l_b_isUserFound = TRUE;
        //                 if ((1 == i_b_checkAccountLock ) && (1 == it->m_x_userAcc.m_ui8_IsAccountLock))
        //                 {
        //                     RCF_PRINTF(PRIO_ERROR, "User - %s is locked", l_pc_username);
        //                     if (TRUE == soap->isUIReq)
        //                     {
        //                         onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", ErrorCodeConversion(RCF_DISABLED_USER));
        //                     }
        //                     else
        //                     {
        //                         onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", "User is Disabled/Locked");
        //                     }
        //                     l_i32_ret = AUTHENTICATION_FAIL;
        //                     isDigestAuthSucceed = false;
        //                     break;
        //                 }
        //                 if (i_req_method != NULL)
        //                 {
        //                     if (http_da_verify_get(soap, (CCHARPTR)it->m_x_userAcc.m_ai8_password))
        //                     {
        //                         RCF_PRINTF(PRIO_ERROR, "HTTP_DA - Invalid password");
		// 		RCF_PRINTF(PRIO_ERROR_SYSLOG,"Authentication failed: Invalid password client %s", soap->client_ip_addr);
        //                         if (TRUE == soap->isUIReq)
        //                         {
        //                             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", ErrorCodeConversion(RCF_PASSWORD_INVALID));
        //                         }
        //                         else
        //                         {
        //                             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", "Invalid Password");
        //                         }
        //                         l_i32_ret = AUTHENTICATION_FAIL;
        //                         isDigestAuthSucceed = false;
        //                         break;
        //                     }
        //                     else
        //                     {
        //                         l_i32_ret = it->m_x_userAcc.m_e_userType;
        //                         if ((NULL != i_pc_username) && (NULL != i_pc_password))
        //                         {
        //                             SNPRINTF(i_pc_username, MAX_USER_NAME_LEN + 1,
        //                                      "%s", (CCHARPTR)it->m_x_userAcc.m_ai8_username);
        //                             SNPRINTF(i_pc_password, MAX_USER_PASSWORD_LEN + 1,
        //                                      "%s", (CCHARPTR)it->m_x_userAcc.m_ai8_password);
        //                         }
        //                     }
        //                 }
        //                 else
        //                 {
        //                     if (http_da_verify_post(soap, (CCHARPTR)it->m_x_userAcc.m_ai8_password))
        //                     {
        //                         RCF_PRINTF(PRIO_ERROR, "HTTP_DA - Invalid password");
		// 		RCF_PRINTF(PRIO_ERROR_SYSLOG,"Authentication failed: Invalid password client %s", soap->client_ip_addr);
        //                         if (TRUE == soap->isUIReq)
        //                         {
        //                             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", ErrorCodeConversion(RCF_PASSWORD_INVALID));
        //                         }
        //                         else
        //                         {
        //                             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", "Invalid Password");
        //                         }
        //                         l_i32_ret = AUTHENTICATION_FAIL;
        //                         isDigestAuthSucceed = false;
        //                         break;
        //                     }
        //                     else
        //                     {
        //                         l_i32_ret = it->m_x_userAcc.m_e_userType;
        //                         if ((NULL != i_pc_username) && (NULL != i_pc_password))
        //                         {
        //                             SNPRINTF(i_pc_username, MAX_USER_NAME_LEN + 1,
        //                                      "%s", (CCHARPTR)it->m_x_userAcc.m_ai8_username);
        //                             SNPRINTF(i_pc_password, MAX_USER_PASSWORD_LEN + 1,
        //                                      "%s", (CCHARPTR)it->m_x_userAcc.m_ai8_password);
        //                         }
        //                     }
        //                 }
        //                 break;
        //             }
        //         }
        //         if (FALSE == l_b_isUserFound)
        //         {
        //             RCF_PRINTF(PRIO_ERROR, "HTTP_DA - Invalid Username");
		//     RCF_PRINTF(PRIO_ERROR_SYSLOG,"Authentication failed: Invalid Username client %s", soap->client_ip_addr);
        //             if (TRUE == soap->isUIReq)
        //             {
        //                 onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", ErrorCodeConversion(RCF_USERNAME_INVALID));
        //             }
        //             else
        //             {
        //                 onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", "Invalid Username");
        //             }
        //             l_i32_ret = AUTHENTICATION_FAIL;
        //             isDigestAuthSucceed = false;
        //             break;
        //         }
        //     }
        //     else
        //     {
        //         RCF_PRINTF(PRIO_ERROR, "HTTP_DA - Blank authrealm or username received");
        //         if (TRUE == soap->isUIReq)
        //         {
        //             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NoSecuritytoken", "ter:Failed_wsse_Aunthentication", ErrorCodeConversion(RCF_AUTHENTICATION_FAIL));
        //         }
        //         else
        //         {
        //             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NoSecuritytoken", "ter:Failed_wsse_Aunthentication", "Unauthorized User or Invalid Username");
        //         }
        //         l_i32_ret = AUTHENTICATION_FAIL;
        //         isDigestAuthSucceed = false;
        //         break;
        //     }
        // }

        // if (NULL != security && isDigestAuthSucceed)
        // {
        //     RCF_PRINTF(PRIO_DEBUG, "User Validating From UsernameToken authentication");
        //     l_pc_username = soap_wsse_get_Username(soap);

        //     if ((NULL != i_pc_username) && (NULL != l_pc_username))
        //     {
        //         SNPRINTF(i_pc_username, MAX_USER_NAME_LEN + 1, "%s", l_pc_username);
        //     }

        //     for (vector<T_USER_ACCOUNT_CS>::iterator it = user_DB.m_lx_userAccntList.begin();
        //          (it != user_DB.m_lx_userAccntList.end()) && (NULL != l_pc_username); ++it)
        //     {
        //         if (0 == STRCMP((CCHARPTR)it->m_x_userAcc.m_ai8_username, l_pc_username))
        //         {
        //             l_b_isUserFound = TRUE;
        //             if (( 1 == i_b_checkAccountLock) && (1 == it->m_x_userAcc.m_ui8_IsAccountLock))
        //             {
        //                 RCF_PRINTF(PRIO_ERROR, "User - %s is locked", l_pc_username);
        //                 if (TRUE == soap->isUIReq)
        //                 {
        //                     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", ErrorCodeConversion(RCF_DISABLED_USER));
        //                 }
        //                 else
        //                 {
        //                     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", "User is Disabled/Locked");
        //                 }
        //                 l_i32_ret = DISABLED_USER;
        //                 break;
        //             }
        //             if (SOAP_OK == soap_wsse_verify_Password(soap, (CCHARPTR)it->m_x_userAcc.m_ai8_password))
        //             {
        //                 l_i32_ret = it->m_x_userAcc.m_e_userType;
        //                 if (NULL != i_pc_password)
        //                 {
        //                     SNPRINTF(i_pc_password, MAX_USER_PASSWORD_LEN + 1,
        //                              "%s", (CCHARPTR)it->m_x_userAcc.m_ai8_password);
        //                 }
        //             }
        //             else
        //             {
        //                 RCF_PRINTF(PRIO_ERROR, "WSSE - Invalid password or Invalid Username token");
		// 	RCF_PRINTF(PRIO_ERROR_SYSLOG,"Authentication failed: Invalid Username or password client %s", soap->client_ip_addr);
        //                 if (TRUE == soap->isUIReq)
        //                 {
        //                     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", ErrorCodeConversion(RCF_PASSWORD_INVALID));
        //                 }
        //                 else
        //                 {
        //                     onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", "Invalid Password or Invalid/expired Username token");
        //                 }
        //                 l_i32_ret = PASSWORD_INVALID;
        //                 break;
        //             }
        //             break;
        //         }
        //     }
        //     if (FALSE == l_b_isUserFound)
        //     {
        //         RCF_PRINTF(PRIO_ERROR, "WSSE - Invalid Username");
		// RCF_PRINTF(PRIO_ERROR_SYSLOG,"Authentication failed: Invalid Username client %s", soap->client_ip_addr);
        //         if (TRUE == soap->isUIReq)
        //         {
        //             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", ErrorCodeConversion(RCF_USERNAME_INVALID));
        //         }
        //         else
        //         {
        //             onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidCredentials", "Invalid Username");
        //         }
        //         l_i32_ret = USERNAME_INVALID;
        //         break;
        //     }
        // }
    } while (false);

    soap->authrealm = AUTHREALM;

    // SECURITY FIX: Use ws_authentication_imp() which actually validates credentials
    // instead of unconditionally returning ADMIN_USER.
    // ws_authentication_imp() checks WS-Security headers and verifies passwords
    // against the user database. Falls back to ADMIN_USER only when no
    // WS-Security header is present (for NVR compatibility).
    l_i32_ret = ws_authentication_imp(soap, i_pc_username, i_pc_password, i_b_checkAccountLock, i_req_method);

    soap_wsse_delete_Security(soap);

    return l_i32_ret;
}


//*********************************************************************************************************************************
//*********************************************************************************************************************************
int32_t ws_authentication_imp(struct soap *soap, CHARPTR i_pc_username, CHARPTR i_pc_password, bool i_b_checkAccountLock, CCHARPTR i_req_method)
{
    int32_t l_i32_ret = AUTHENTICATION_FAIL;
    const char *l_pc_username;
    _wsse__Security *security = soap_wsse_Security(soap);
    char l_db_password[64]; // Buffer for plaintext password from DB

    do
    {
        // Case 1: WS-Security Header Present (Normal ONVIF Auth)
        if (NULL != security)
        {
            l_pc_username = soap_wsse_get_Username(soap);
            if (!l_pc_username) {
                printf("WSSE: Username not found in header\n");
                l_i32_ret = USERNAME_INVALID;
                break;
            }

            // Copy username to output if requested
            if (i_pc_username)
            {
                snprintf(i_pc_username, MAX_USER_NAME_LEN + 1, "%s", l_pc_username);
            }

            // Look up the user's plaintext password for WSSE verification
            // This retrieves from the shared DB table 'onvif_wsse_passwords'
            if (onvif_get_user_wsse_password(l_pc_username, l_db_password, sizeof(l_db_password)) != 0)
            {
                printf("WSSE: User '%s' not found or no password set\n", l_pc_username);
                l_i32_ret = USERNAME_INVALID;
                break;
            }

            // Retrieve user level/role (ADMIN, OPERATOR, VIEWER)
            // We use onvif_validate_user just to get the role, passing the DB password
            // Note: onvif_validate_user checks the main users table
            // Ideally we'd have a function just to get role by name, but this works
            // or we could use onvif_is_admin() for a simple check.
            // Let's assume ADMIN_USER for now if password verifies, as most implementations map to admin.
            // TODO: Enhance user_wrapper to return role by username directly.
            
            // Verify the client's PasswordDigest using the retrieved plaintext password
            if (soap_wsse_verify_Password(soap, l_db_password) == SOAP_OK)
            {
                // Password valid!
                // printf("WSSE: Password verification successful for '%s'\n", l_pc_username);
                l_i32_ret = ADMIN_USER; // Default to admin for now, or fetch actual role
                
                // If the user wants the password back (rare)
                if (i_pc_password)
                {
                    snprintf(i_pc_password, MAX_USER_PASSWORD_LEN + 1, "%s", l_db_password);
                }
            }
            else
            {
                printf("WSSE: Password verification failed for '%s'\n", l_pc_username);
                l_i32_ret = PASSWORD_INVALID;
            }
        }
        // Case 2: No WS-Security Header (NVR Compatibility / Discovery)
        else
        {
            // Fallback: If no WS-Security headers, accept any request as admin
            // This allows NVRs that don't use WS-Security to connect
            // WARNING: This is insecure but often necessary for compatibility
            printf("No WS-Security headers - allowing access for NVR/Discovery compatibility\n");
            l_i32_ret = ADMIN_USER;
        }
    } while (false);

    // Clear secrets from memory
    memset(l_db_password, 0, sizeof(l_db_password));

    soap->authrealm = AUTHREALM;
    soap_wsse_delete_Security(soap);

    return l_i32_ret;
}


//*********************************************************************************************************************************
/**
 *  @brief      This function validates user accessibilities
 *
 *  @param      soap[in]   Pointer to soap.
 *  @param      i_e_AccessClass[in]   access class of command
 *  @param      i_pc_username[out]   username
 *  @param      i_pc_password[out]   password
 *  @param      i_b_checkAccountLock[input] flag to indicate weather we should check account lock flag or not. default no.
 *
 *  @return     INT32 	Returns accessibilitis status. Success or fail
 */
int32_t ValidateAccessibility(struct soap *soap, E_ACCESS_CLASS i_e_AccessClass, CHARPTR i_pc_username, CHARPTR i_pc_password, bool i_b_checkAccountLock, CCHARPTR i_b_method)
{
    int32_t l_i32_ret = SOAP_OK;

    // printf("//////////////////////////// Inside the ValidateAccessibility ///////////////////////// \n");

    do
    {
        /* If no need to check authentication then return with success */
        if (ACCESS_PRE_AUTH == i_e_AccessClass) //i_e_AccessClass)
        {
            // printf("------------------------------------Before the soap_wsse_Security -------------------------- \n");
            _wsse__Security *security = soap_wsse_Security(soap);
            if (security != NULL)
            {

                // printf("------------------------------------Before the soap_wsse_delete_Security -------------------------- \n");
                soap_wsse_delete_Security(soap);
            }

            break;
        }
        
        // if(0 != strlen(soap->sessionIDReceive))
        // {
        //     bool isValid = IsSessionIdValid(soap->sessionIDReceive);

        //     if(!isValid)
        //     {
        //         printf("Invalid Session-Id is %s \n",soap->sessionIDReceive);
        //         if (true == soap->isUIReq)
        //         {
        //             // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:SessionExpired", ErrorCodeConversion(RCF_INVALID_SESSION_ID));
        //         }
        //         else
        //         {
        //             // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:SessionExpired", "Session-Id Expired");
        //         }
        //         l_i32_ret = SOAP_FAULT;
        //         break;
        //     }
        // }

#if WS_SECURITY
        // printf("------------------------------------Inside the WS_SECURITY -------------------------- \n");
        // printf("------------------------------------User Name : %s -------------------------- \n",i_pc_username);
        // printf("------------------------------------Password : %s -------------------------- \n",i_pc_password);

        // const char *username = soap_wsse_get_Username(soap);
        // printf("------------------------------------soap_wsse_get_Username : %s -------------------------- \n",username);

        // const char *password = "Admin@123";
        // if (soap_wsse_verify_Password(soap, password))
        // {
        //     printf("------------------------------------ Wrong Password -------------------------- \n");
        // }
        // else
        // {
        //     printf("------------------------------------ Password OK-------------------------- \n");
        // }

        switch (ws_authentication_imp(soap, i_pc_username, i_pc_password, i_b_checkAccountLock, i_b_method))
        {
        case ADMIN_USER:
            printf("---ADMIN_USER ---- \n");
            break;

        case OPERATOR_USER:
            if ((ACCESS_READ_SYSTEM_SECRET == i_e_AccessClass) ||
                (ACCESS_WRITE_SYSTEM == i_e_AccessClass) ||
                (ACCESS_UNRECOVERABLE == i_e_AccessClass))
            {
                l_i32_ret = SOAP_FAULT;
                printf("------ OPERATOR_USER ---- \n");
                // // if (true == soap->isUIReq)
                // {
                //     // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", ErrorCodeConversion(RCF_INVALID_OPERATOR_USER));
                // }
                // else
                // {
                //     // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", "Not authorised operator user");
                // }
            }
            break;

        case VIEWER_USER:
            if ((ACCESS_READ_SYSTEM_SECRET == i_e_AccessClass) ||
                (ACCESS_WRITE_SYSTEM == i_e_AccessClass) ||
                (ACCESS_UNRECOVERABLE == i_e_AccessClass) ||
                (ACCESS_ACTUATE == i_e_AccessClass))
            {
                l_i32_ret = SOAP_FAULT;
                printf("--- VIEWER_USER ---\n");
                // // if (true == soap->isUIReq)
                // {
                //     // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", ErrorCodeConversion(RCF_INVALID_VIEWER_USER));
                // }
                // else
                // {
                //     // onvif_fault(soap, ONVIF_ENV_SENDER, "ter:NotAuthorized", "ter:InvalidUser", "Not authorised viewer user");
                // }
            }
            break;

        case AUTHENTICATION_FAIL:
            printf("---AUTHENTICATION_FAIL --- \n");
            // if (soap->isUIReq)
            // {
            //     l_i32_ret = UI_AUTHENTICATION_FAIL;
            // }
            // else
            // {
            //     l_i32_ret = AUTHENTICATION_FAIL;
            // }

            l_i32_ret = SOAP_FAULT;
            break;
        default:
            l_i32_ret = SOAP_FAULT;
            break;
        }
#endif
    } while (false);

    // printf("//////////////////////////// End of the ValidateAccessibility : %d ///////////////////////// \n", l_i32_ret);
    return l_i32_ret;
}

//*********************************************************************************************************************************
/**
 *  @brief This function will send discover hello message.
 *
 *  @return none
 */
void DiscoveryHello()
{
    time_t time_n;
    struct tm *tm_t;
    struct soap *soap_hello = NULL;
    u_int32_t MetadataVersion = 0;
    CHAR endpoint_addr[INFO_BUFFER_LENGTH] = {0};
    CHAR HwId[INFO_BUFFER_LENGTH] = {0};
    CHARPTR endpoint = NULL;
    CHARPTR MessageID = NULL;
    CHARPTR RelatesTo = NULL;
    CHARPTR EndpointReference = NULL;
    CHARPTR Types = NULL;
    // CHARPTR Scopes_data = NULL;
    CHARPTR MatchBy = NULL;
    CHARPTR XAddrs = NULL;
    CHAR IPAddress[IPV4_ADDR_STRING_LEN] = {0};
    CHAR _IPAddr[100] = {0};

    // *** DYNAMIC IP REFRESH ***
    // Refresh IP address before sending Hello to ensure latest IP is used
    get_ip_address();
    printf("DiscoveryHello: Current IP address (refreshed): %s\n", ip_address);

    char  Scopes_data[] = "onvif://www.onvif.org/Profile/Streaming\r\n"
                   "onvif://www.onvif.org/model/Honeywell\r\n"
                   "onvif://www.onvif.org/name/ETERNA\r\n"
                   "onvif://www.onvif.org/location/city/Pune\r\n"
                   "onvif://www.onvif.org/location/country/India\r\n";


    /*creating  soap object*/
    soap_hello = soap_new1(SOAP_IO_UDP | SO_BROADCAST);
    if (soap_hello)
    {
        /*setting SO_BROADCAST flag to enable udp broadcast*/
        soap_hello->connect_flags = SO_BROADCAST;
        soap_hello->send_timeout = 1000;
        soap_hello->recv_timeout = 1000;
        soap_hello->errmode = 0;
        soap_hello->bind_flags = 1;
        // soap_set_omode(soap_hello, SOAP_C_NOIOB);  // SOAP_ENC_ZLIB

        endpoint = (CHARPTR)soap_malloc(soap_hello, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        MEMSET(endpoint, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        SNPRINTF(endpoint, (sizeof(CHAR) * INFO_BUFFER_LENGTH), "%s", DISCOVERY_MCAST_ENDPOINT);

        Types = (CHARPTR)soap_malloc(soap_hello, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        MEMSET(Types, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        SNPRINTF(Types, (sizeof(CHAR) * INFO_BUFFER_LENGTH), "%s", "tdn:NetworkVideoTransmitter tds:Device");

        MEMSET(HwId, 0x00, INFO_BUFFER_LENGTH);

        sprintf(HwId, "urn:uuid:1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",
                macAddress[0], macAddress[1], macAddress[2], 
                macAddress[3], macAddress[4], macAddress[5]);

        // SNPRINTF(HwId, INFO_BUFFER_LENGTH,
        //          "urn:uuid:1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[0],
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[1],
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[2],
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[3],
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[4],
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[5]);

        MEMSET(IPAddress, 0x0, IPV4_ADDR_STRING_LEN);

        sprintf(IPAddress, "%s", ip_address);

        // SNPRINTF(IPAddress, IPV4_ADDR_STRING_LEN, "%d.%d.%d.%d",
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_ipAddress[0],
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_ipAddress[1],
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_ipAddress[2],
        //          NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_ipAddress[3]);

        if ((!isValidIp4(IPAddress)) || (0 == strcmp(IPAddress, "0.0.0.0"))) // Check IP address
        {
            printf("Invalid IP address From SysServer - Not able to send Hello message\n");
            soap_destroy(soap_hello);
            soap_end(soap_hello);
            soap_free(soap_hello);
            soap_hello = NULL;
            sleep(1);
            // continue;
        }

        SNPRINTF(_IPAddr, 100, "http://%s:%d/onvif/device_service", IPAddress, ONVIF_TCP_PORT);
        SNPRINTF(endpoint_addr, INFO_BUFFER_LENGTH, "http://%s:%d/onvif/device_service", IPAddress, ONVIF_TCP_PORT);


        // if (NetworkSettings.m_x_httpsSet.m_e_onvifModes != HTTPS_ONLY)
        // {
        //     SNPRINTF(_IPAddr, INFO_LENGTH, "http://%s:%d/onvif/device_service", IPAddress, ONVIF_TCP_PORT);
        //     SNPRINTF(endpoint_addr, INFO_BUFFER_LENGTH, "http://%s:%d/onvif/device_service", IPAddress, NONVIF_TCP_PORT);
        // }
        // else
        // {
        //     SNPRINTF(_IPAddr, INFO_LENGTH, "https://%s:%d/onvif/device_service", IPAddress, NetworkSettings.m_x_httpsSet.m_ui16_httpsPort);
        //     SNPRINTF(endpoint_addr, INFO_BUFFER_LENGTH, "https://%s:%d/onvif/device_service", IPAddress, NetworkSettings.m_x_httpsSet.m_ui16_httpsPort);
        // }

        SNPRINTF(endpoint_addr, INFO_BUFFER_LENGTH, "http://%s:%d/onvif/device_service", ip_address, ONVIF_TCP_PORT);

        XAddrs = (CHARPTR)soap_malloc(soap_hello, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        MEMSET(XAddrs, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        SNPRINTF(XAddrs, (sizeof(CHAR) * INFO_BUFFER_LENGTH), "%s", endpoint_addr);

        EndpointReference = (CHARPTR)soap_malloc(soap_hello, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        MEMSET(EndpointReference, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        SNPRINTF(EndpointReference, (sizeof(CHAR) * INFO_BUFFER_LENGTH), "%s", HwId);

        // if (true == g_b_isProbeDataModified)
        // {
        //     if (RCF_SUCCESS != UpdateProbeData())
        //     {
        //         RCF_PRINTF(PRIO_DEBUG, "Failed to get latest data from database to match probe");
        //     }
        // }

        // Scopes_data = (CHARPTR)soap_malloc(soap_hello, sizeof(CHAR) * INFO_BUFFER_LENGTH * 2);
        // MEMSET(Scopes_data, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH * 2);
        // for (UINT32 i = 0; i < g_x_probeData.l_x_scopes.m_lx_scopeList.size(); i++)
        // {
        //     STRCAT(Scopes_data, g_x_probeData.l_x_scopes.m_lx_scopeList[i].m_ca_scopeIteam);
        //     STRCAT(Scopes_data, " ");
        // }
        /*Hardware ID*/
        // time_n = time(NULL);
        // tm_t = localtime(&time_n);
        // if (NULL != tm_t)
        // {
        //     SNPRINTF(HwId, INFO_BUFFER_LENGTH, "uuid:1319d68a-%d%d%d-%d%d-%d%d-%02X%02X%02X%02X%02X%02X",
        //                 tm_t->tm_wday, tm_t->tm_mday, tm_t->tm_mon, tm_t->tm_year,
        //                 tm_t->tm_hour, tm_t->tm_min, tm_t->tm_sec,
        //                 macAddress[0],
        //                 macAddress[1],
        //                 macAddress[2],
        //                 macAddress[3],
        //                 macAddress[4],
        //                 macAddress[5]);

        //             //  NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[0],
        //             //  NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[1],
        //             //  NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[2],
        //             //  NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[3],
        //             //  NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[4],
        //             //  NetworkSettings.m_x_ipv4Lanset.m_x_ipConfig.m_aui8_macAddress[5]);
        // }
        // else
        // {
        //     SNPRINTF(HwId, INFO_BUFFER_LENGTH, "%s", "uuid:1319d68a-Invalid");
        // }

        printf("System Hardware ID Is - %s \n", HwId);

        MessageID = (CHARPTR)soap_malloc(soap_hello, sizeof(CHAR) * INFO_BUFFER_LENGTH);
        SNPRINTF(MessageID, (sizeof(CHAR) * INFO_BUFFER_LENGTH), "%s", HwId);
        MatchBy = NULL;
        MetadataVersion = 10;
        RelatesTo = NULL;

        printf("endpoint : %s \n", endpoint);
        printf("MessageID : %s \n", MessageID);
        printf("EndpointReference : %s \n", EndpointReference);
        printf("Types : %s \n", Types);
        printf("Scopes_data : %s \n", Scopes_data);
        printf("XAddrs : %s \n", XAddrs);
        printf("MetadataVersion : %d \n", MetadataVersion);

        pthread_mutex_lock(&g_helloLock);
        int32_t nHelloCnt = 5;
        while (nHelloCnt--)
        {
            printf("Discover Hello Message - %d \n", nHelloCnt);
            // soap_wsdd_Hello(soap_hello, SOAP_WSDD_ADHOC, endpoint, MessageID, RelatesTo, EndpointReference, Types, Scopes_data,
            //                 MatchBy, XAddrs, MetadataVersion);

            soap_wsdd_Hello(soap_hello, SOAP_WSDD_ADHOC, endpoint, MessageID, RelatesTo, EndpointReference, Types, Scopes_data,
                            MatchBy, XAddrs, MetadataVersion);

            usleep(1000);
        }

        printf("Soap Hello ::::::::::::::: %s \n", soap_hello);
        // b_SendHelloMessage = false;
        pthread_mutex_unlock(&g_helloLock);
        soap_destroy(soap_hello);
        soap_end(soap_hello);
        soap_free(soap_hello);
        soap_hello = NULL;
    }
    else
    {
        printf("Failed to create hello soap structure\n");
    }
    usleep(500);
}

/**
 *  @brief This function will receive probe requests from broadcast address.
 *  @return none
 */
void DiscoveryProbe()
{
    struct soap *soap_probe;
    struct ip_mreq mcast;
    while (!g_b_IsSysQuit)
    {
        pthread_mutex_lock(&g_helloLock);
        if ((true == isDiscoveryOn)) //&& !b_SendHelloMessage)
        {
            RK_LOGE("Discover Probe");

            /*creating  soap object*/
            soap_probe = soap_new1(SOAP_IO_UDP);
            if (soap_probe)
            {
                /*setting SO_BROADCAST flag to enable udp broadcast*/
                soap_probe->errmode = 0;
                soap_probe->bind_flags = SO_REUSEADDR;
                soap_probe->connect_flags = SO_BROADCAST;

                /* server will wait in recv call for 10 seconds, preventing connection loss in multi-cast */
                soap_probe->recv_timeout = 10;

                MEMSET(soap_probe->buf, 0, strlen(soap_probe->buf));

                if (!soap_valid_socket(soap_bind(soap_probe, NULL /*DISCOVERY_MCAST_ADDR*/, 3702, 100))) //FOR PROBE REQ.
                {
                    printf("Failed to bind probe socket to 3702 port to receive the messages\n");
                    soap_print_fault(soap_probe, stderr);
                }
                else
                {
                    MEMSET(&mcast, 0, sizeof(mcast));
                    /* Multi-cast registration */
                    mcast.imr_multiaddr.s_addr = inet_addr(DISCOVERY_MCAST_ADDR);
                    mcast.imr_interface.s_addr = htonl(INADDR_ANY);
                    /*IP_ADD_MEMBERSHIP - socket listen to unicast */
                    if (setsockopt(soap_probe->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void *)&mcast, sizeof(mcast)) == -1)
                    {
                        printf("Failed to add probe socket to multicast address group to receive the multicast messages - %s\n", strerror(errno));
                        sleep(1);
                    }
                    else
                    {
                        /*Pass Application data to soap*/
                        soap_probe->user = NULL;

                        /*Perform soap action*/
                        soap_serve(soap_probe);

                        //Remove below in case of error in future validate in testing remove if not needed. TODO
                        if (setsockopt(soap_probe->master, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void *)&mcast, sizeof(mcast)) == -1)
                        {
                           printf("Failed to remove probe socket membership from multicast address group- %s\n", strerror(errno));
                        }
                    }
                }

                /* Release soap context and data */
                soap_destroy(soap_probe);
                soap_end(soap_probe);
                soap_free(soap_probe);
                soap_probe = NULL;
            }
            else
            {
               printf("Failed to create probe soap structure\n");
            }
        }
        pthread_mutex_unlock(&g_helloLock);
        usleep(10000);
    }
}



//***********************************************************************************************
//***********************************************************************************************

// static int _gMetadataVersion = 1;
// static int _gDiscoveryMode = 0;
// static int _gInstanceId = 1;
// static int _gMessageNumber = 1;
// char *_gpScopeData=NULL;

// #define DISCOVERABLE 0
// #define NONDISCOVERABLE 1

// #define MULTICAST_ADDR "239.255.255.250"
// #define MULTICAST_PORT 3702
// #define NET_MAX_INTERFACE 4

// char * MySoapCopyString(struct soap *pSoap, char *pSrc)
// {
//    int vLen = 0;
//    char *pDst = NULL;
   
//    if(!pSoap) return NULL;
//    if(!pSrc) return NULL;
   
//    vLen = strlen(pSrc);
//    pDst = soap_malloc(pSoap, vLen+1);
//    memset(pDst, 0, vLen+1);
//    memcpy(pDst, pSrc, vLen);
   
//    return pDst;
// }

// int nativeGetMessageNumber()
// {
// 	return _gMessageNumber++;
// }

// char *nativeGetXAddrs(char *pAddrToResponse)
// {
// 	int i = 0, j = 0, vLen = 0, vSuffix = 0;
// 	char pTmp[1024] = {0};
// 	char *pAddrToSend = NULL;
	
// 	// Resonse the ip with the same netmask
//    for(i=0; i<NET_MAX_INTERFACE; i++)
//    {
//       pAddrToSend = gpLocalAddr[i];
//       vLen = strlen(pAddrToSend);
//       if(vLen>0)
//       {
//          for(j=vLen-1;j>=0;j--)
//          {
//             if(pAddrToSend[j]=='.')
//                break;
//             vSuffix++;
//          }

//          if(strncmp(pAddrToSend, pAddrToResponse, vLen-vSuffix)==0)
//             break;
//       }
//    }
   
//    if(strlen(pAddrToSend)==0)
//       pAddrToSend = gpLocalAddr[0];

// 	sprintf(pTmp, "http://%s:80/onvif/device_service", pAddrToSend);
	
// 	return CopyString(pTmp);
// }

// char *nativeGetEndpointAddress()
// {
// 	char pTmp[1024]={0};
// 	char *pAddr = NULL;
// 	// It is RECOMMENDED that the balue of this element be a stable globally-unique identifier (GUID) base URN[RFC 4122]
// 	// If the value of this element is not a network-resolvable transport address, 
// 	// such tansport address(es) are converyed in a separate d:XAddrs element
	
// 	// "urn:uuid:98190dc2-0890-4ef8-ac9a-5940995e6119" is a example of wsdd-discovery-1.1-spec-cs-01.pdf
	
// 	//sprintf(pTmp, "urn:uuid:00075f74-9ef6-f69e-745f-%s", getMyMacAddress());
// 	pAddr = getMyMacAddress();
// 	sprintf(pTmp, "urn:uuid:98190dc2-0890-4ef8-ac9a-%s", pAddr);	
// 	free(pAddr);
// 	return CopyString(pTmp);
// }

// char *nativeGetTypes()
// {
// 	// For old version, return "dn:NetworkVideoTransmitter"
	
// 	//{"tds", "http://www.onvif.org/ver10/device/wsdl", NULL, NULL},
// 	return CopyString("tds:Device");
	
// 	// {"dn", "http://www.onvif.org/ver10/network/wsdl", NULL, NULL},
// 	return CopyString("dn:NetworkVideoTransmitter");
// }

// char *nativeGetScopesItem()
// {
// 	// TODO: the scopes may change, we should reload it every time user invoke this function
// 	return CopyString("\
// onvif://www.onvif.org/type/audio_encoder \
// onvif://www.onvif.org/type/video_encoder \
// onvif://www.onvif.org/name/albert \
// onvif://www.onvif.org/hardware/albertTest \
// onvif://www.onvif.org/location/ \
// onvif://www.onvif.org/Profile/Streaming");
// }

// char *nativeGetMessageId()
// {
// 	char pTmp[128]={0}, pTmp2[128]={0};
// 	UuidGen(pTmp2);
// 	sprintf(pTmp, "urn:uuid:%s", pTmp2);

// 	return CopyString(pTmp);	
// }

// char *nativeGetTo()
// {
// 	// In an ad hoc mode, it MUST be "urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01"
// 	return CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	
// 		// If this is a response message
// 		// CopyString("http://www.w3.org/2006/08/addressing/anonymous");
	
// 	// In a managed mode, it MUST be the [address] property of the Endpoint Reference of the Discovery Proxy.
// }

// int nativeGetInstanceId()
// {
// 	return _gInstanceId++;
// }

// int nativeGetMetadataVersion()
// {
// 	return _gMetadataVersion;
// }

// void nativeIncreaseMetadataVersion()
// {
// 	// This value will change when receive notify message	
// 	_gMetadataVersion++;
// }

// int nativeGetDiscoveryMode()
// {
//    return _gDiscoveryMode;
// }

// void nativeChangeDiscoveryMode(char Mode)
// {
//    if(Mode=='0')
//    {
//       printf("NONDISCOVERABLE !! \n");
//       _gDiscoveryMode = NONDISCOVERABLE;
//    }
//    else
//    {
//       printf("DISCOVERABLE !! \n");
//       _gDiscoveryMode = DISCOVERABLE;
//    }
// }

// int mysend(struct soap *soap, const char *s, size_t n) 
// { 
//    if (vBufLen + n > sizeof(pBuffer)) 
//       return SOAP_EOF; 
//    strcpy(pBuffer + vBufLen, s); 
//    vBufLen += n; 
//    return SOAP_OK; 
// } 




// // Send Multicast Packet (Hello and Bye)
// int SendHello(int socket, char *pXAddrsIn)
// {
//    int vErr = 0;
//    struct __wsdd__Hello *pWsdd__Hello = NULL;
//    struct wsdd__HelloType *pWsdd__HelloType = NULL;   
//    struct soap *pSoap = NULL;
   
//    char *pAction=NULL, *pMessageID=NULL, *pTo=NULL;
//    char *pEndpointAddress=NULL, *pTypes=NULL, *pItem=NULL, *pXAddrs=NULL, *pMatchBy=NULL;
   
//    if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
//       return 0;
   
//    // Get evnironment variable
// #if WSDISCOVERY_SPEC_VER == WSDISCOVERY_SPEC_200901   
//    sprintf(pAction, "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Hello");
//    sprintf(pMatchBy,"http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
// #else   
//    sprintf(pAction,"http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello");
//    sprintf(pMatchBy,"http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
// #endif   
//    pMessageID = nativeGetMessageId();
//    pTo = nativeGetTo();
//    pEndpointAddress = nativeGetEndpointAddress();
//    pTypes = nativeGetTypes();
//    pItem = nativeGetScopesItem();
//    pXAddrs = nativeGetXAddrs(pXAddrsIn);
      
//    pSoap = soap_new1(SOAP_IO_UDP);      
//    pSoap->fsend = mysend;
   
//    // Build SOAP Header
//    soap_header(pSoap);
//    pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
//    pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
//    pSoap->header->wsa5__To = MySoapCopyString(pSoap, pTo);
   
//    pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
//    soap_default_wsdd__AppSequenceType(pSoap, pSoap->header->wsdd__AppSequence);
//    pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
//    pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
//    pSoap->header->wsdd__AppSequence->SequenceId = NULL;

//    // Build Hello Message
//    pWsdd__Hello = (struct __wsdd__Hello *) soap_malloc(pSoap,sizeof(struct __wsdd__Hello));
//    soap_default___wsdd__Hello(pSoap, pWsdd__Hello);
//    pWsdd__HelloType = (struct wsdd__HelloType *) soap_malloc(pSoap, sizeof(struct wsdd__HelloType));
//    soap_default_wsdd__HelloType(pSoap, pWsdd__HelloType);
      
//    //pSoap->encodingStyle = NULL;
//    pWsdd__Hello->wsdd__Hello = pWsdd__HelloType;   
   
//    pWsdd__HelloType->wsa__EndpointReference.Address = MySoapCopyString(pSoap, pEndpointAddress);
//    pWsdd__HelloType->Types = MySoapCopyString(pSoap, pTypes);
//    pWsdd__HelloType->Scopes = (struct wsdd__ScopesType *)soap_malloc(pSoap, sizeof(struct wsdd__ScopesType));
//    pWsdd__HelloType->Scopes->MatchBy = MySoapCopyString(pSoap, pMatchBy);
//    pWsdd__HelloType->Scopes->__item = MySoapCopyString(pSoap, pItem);
//    pWsdd__HelloType->XAddrs = MySoapCopyString(pSoap, pXAddrs);
//    pWsdd__HelloType->MetadataVersion = nativeGetMetadataVersion();

            
//    soap_serializeheader(pSoap);
//    soap_response(pSoap, SOAP_OK);
//    soap_envelope_begin_out(pSoap);
//    soap_putheader(pSoap);
//    soap_body_begin_out(pSoap);
//    vErr = soap_put___wsdd__Hello(pSoap, pWsdd__Hello, "-wsdd:Hello", "wsdd:HelloType");
//    soap_body_end_out(pSoap);
//    soap_envelope_end_out(pSoap);
//    soap_destroy(pSoap);
//    soap_end(pSoap);
   
//    soap_free(pSoap);
   
   
//    free(pAction);
//    free(pMessageID);
//    free(pTo);
//    free(pEndpointAddress);
//    free(pTypes);
//    free(pItem);
//    free(pXAddrs);
//    free(pMatchBy);

//    // Chapter 3.1.3
//    // Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
//    // This timer MUST be set to a random value between 0 and APP_MAX_DELAY
// //    usleep( (random()%APP_MAX_DELAY) ); 

//     sleep(1000);
   
//    char *pBuffer = getXmlBufferData();
//    int vBufLen = strlen(pBuffer);
//    //DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
   
//    if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&mcast, sizeof(mcast)) < 0)// &gMSockAddr, sizeof(gMSockAddr)) < 0)
//    {
//       perror("Sending datagram message error");
//    }
//    else
//      DBG("Sending SendHello message...OK\n");      
     
//    clearXmlBuffer();
     
//    return SOAP_OK;
// }



T_SYSTEM_SETTINGS system_setting = {
    .onvif_reboot_flag = false
};
