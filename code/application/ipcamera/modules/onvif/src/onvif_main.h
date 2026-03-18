#ifndef ONVIF_MAIN_H
#define ONVIF_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Platform compatibility - use spdlog for logging instead of Rockchip specific headers
#ifndef RK_LOGE
#define RK_LOGE(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#endif
#ifndef RK_LOGI
#define RK_LOGI(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#endif
#ifndef RK_LOGD
#define RK_LOGD(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#endif
#ifndef RK_LOGW
#define RK_LOGW(fmt, ...) printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#endif

#ifndef LOG_INFO
#define LOG_INFO(fmt, ...) printf("[INFO] " fmt, ##__VA_ARGS__)
#endif
#ifndef LOG_ERROR
#define LOG_ERROR(fmt, ...) printf("[ERROR] " fmt, ##__VA_ARGS__)
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#endif
//#define ONVIF_TEST              // uncomment to test ONVIF in ubuntu PC

#define WS_SECURITY             1   

typedef const char *				CCHARPTR;
typedef char *					    CHARPTR;
typedef char CHAR;

//ports to use
#define ONVIF_UDP_PORT 3702 //as defined in onvif ws-discovery
#define ONVIF_TCP_PORT 80   // Advertised port (standard HTTP) - use nginx to proxy if needed
#define ONVIF_LISTEN_PORT 5000 // Internal listen port for nginx reverse proxy
//addresses to use
#define onvifUdpAddress "239.255.255.250" //as defined in onvif ws-discovery
//#define onvifTcpAddress "10.67.97.35" //"192.168.1.6" //got  from ifconfig in raspberry pi
#define SPRINTF(s, f, a...) (sprintf((char *)s, (const char *)f, ##a))
#define SNPRINTF(s,n, f, a...) (snprintf((char *)s,n, (const char *)f, ##a))
#define MEMSET(s, c, n) (memset((void *)s, (int)c, (size_t)n))

#define ONVIF_ENV_SENDER    1


#define ACCPET_TIMEOUT 1
#define SEND_RECV_TIMEOUT 5 // in seconds
#define ACCEPT_PORT 9999
#define ACCEPT_QUEUE 100
#define LOOPBACK_IP "127.0.0.1"

#define ONVIF_ENV_RECEIVER                      0

# define THREAD_CREATE(x,y,z)	pthread_create((x), NULL, (y), (z))

#define MANUFACTURER        "Honeywell"
#define MODEL               "Honeywell IPCAM-5MP"
#define FIRMWARE_VERSION    "v1.0.0"
#define SERIAL_NO           "IP123456H"
#define HARDWARE_ID         "Rev C"
#define OSD_DEFAULT_FONT_SIZE       48

extern bool g_b_IsSysQuit; 
// struct http_post_handlers my_handlers[];

extern char *ip_address, *temp_ip_address, *temp_method;
extern char *interface, *method,*address, *netmask, *gateway, *temp_netmask, *temp_gateway;
extern unsigned char macAddress[7];

extern char *ip_assign_method; 

extern char *osdTimeFormat[2];
extern char *osdTextTypes[2];
extern char *osdPositions[6];
extern char *osdDateFormat[6];
extern char dateFormatWithoutFirstFourChars[20];
char* strupr(char* s);
char* strlwr(char* s);

void *OnvifWSDiscoveryServer(void *arg); 
void *OnvifWebServiesServer(void *arg); 
void DiscoveryHello();
void DiscoveryProbe();
int get_ip_address(void);
int get_hw_addr(char *ifname, unsigned char *mac);
in_addr_t net_get_ifaddr(char *ifname, int *ret);
int ntp_update(const char *ntp_server_addr); 

void *thread_OnvifReqHandler(void *arg);

extern bool ip_address_update_flag;
extern bool ip_address_update_request;

extern char isDiscoveryOn;

extern pthread_mutex_t g_probeLock;
extern pthread_mutex_t g_helloLock;

#define DEFAULT_RTSP_PORT   554




#define TOKEN_LENGTH                64
#define SMALL_BUFFER_LENGTH         20


// //**********Netwrok Settings **********************

#define		MAX_HOST_NAME_LEN				    64
#define 	MAC_ADDR_STRING_LEN				    18
#define 	IPV4_ADDR_STRING_LEN				16
#define		MAX_IPV4_IP_ADDR_LEN				4
#define		MAX_IPV4_SUBNET_MASK_LEN			4
#define		MAX_IPV4_GATEWAY_ADDRESS_LEN		4
#define		MAX_DNS_IPV4_ADDR_LEN				4
#define		MAX_MAC_ADDR_LEN				    6
// #define		MIN_IP_ADDR_RANGE				    0
// #define		MAX_IP_ADDR_RANGE				    255
// #define	 	MAX_MAC_ADDRESS_LEN				    18
#define 	MAX_DNS_IPV6_ADDR_LEN				45
#define 	MAX_IPV6_IP_ADDR_LEN				45
#define 	MAX_IPV6_GETWAY_ADDR_LEN			45

#define MAX_NTP_SERVER_LEN      (48)
#define     MAX_TOKEN_SIZE          		100
// #define 	IPV4_ADDR_STRING_LEN				16
// #define 	MAC_ADDR_STRING_LEN				    18
// #define 	MAX_FILE_NAME_SIZE			        (1000)
// #define     DEFAULT_ABOUT_PRODUCT_NAME	    "Honeywell"
// #define     DFLT_INTERFACE_IP_STR			"0.0.0.0"
// #define     DFLT_GW_ADDR_FOR_MULTICAST		"224.0.0.0"

#define TIMEZONE_NAME_LEN       			(100)

#define DISCOVERY_MCAST_ADDR                    "239.255.255.250"
#define DISCOVERY_MCAST_ENDPOINT                "soap.udp://" DISCOVERY_MCAST_ADDR ":3702"
#define AUTHREALM                               "Silvan_http_digest"


//************************************ User Account Setting start ************************************
#define		MAX_WEB_SESSION						10
#define		MAX_USER_NAME_LEN					32
#define		MAX_USER_PASSWORD_LEN					64
#define		MAX_USER_ACCOUNT					20
#define     SESSION_ID_LENGTH                   50

// typedef enum
// {
//     IP_ASSIGN_STATIC = 0,	// Static IP assignment
//     IP_ASSIGN_DHCP,			// assignment through DHCP
//     IP6_ASSIGN_STATIC,
//     IP6_ASSIGN_DHCP,
//     IP6_ASSIGN_DHCP_STATELESS,
//     IP6_ASSIGN_DHCP_STATEFUL,
//     MAX_IP_ASSIGN_MODE		// max number of ip assignment modes
// }E_IP_ASSIGN_MODE;


// typedef enum
// {
// 	WLAN0 = 0,
// 	PHY_LINK_ID_ETH,
//     PHY_LINK_ID_ETH0_0,
// 	MAX_DEFAULT_GATEWAY
// }E_INTERFACE;

// #pragma pack(push, 1)


typedef struct
{
    u_int8_t 	m_aui8_ipAddressDhcp[IPV4_ADDR_STRING_LEN + 1];		// For static, DHCP, VLAN
    u_int8_t 	m_aui8_ipAddressZero[IPV4_ADDR_STRING_LEN + 1];		// For zero configuration

}T_NW_IP_ADDR_LIST;

// #pragma pack(pop)

typedef enum
{
    DNS_STATIC = 0,		// Static DNS address
    DNS_AUTO,		// Auto DNS address
    MAX_DNS_MODE		// max number of dns address mode
}E_DNS_MODE;

// #pragma pack(push, 1)
typedef struct
{
    E_DNS_MODE 		m_e_mode;					// DNS addressing mode
    u_int8_t 			m_aui8_preferrdAddr[MAX_DNS_IPV4_ADDR_LEN];	// primary DNS address(preferred)
    u_int8_t 			m_aui8_alternateAddr[MAX_DNS_IPV4_ADDR_LEN];	// secondary DNS address(alternate)
}T_DNS_IPV4_PARAMETER;

typedef struct
{
	bool						m_e_autoneg;		// Auto negotiation on/off
	int						m_ai32_speed;		// Speed (Mbps)
	bool						m_e_duplex;		// Duplex type, Half or Full
}T_NETWORK_INTERFACE_LINK_SETTING;

typedef struct
{
	T_NETWORK_INTERFACE_LINK_SETTING		m_x_adminSetting;		// Configured link settings
	T_NETWORK_INTERFACE_LINK_SETTING		m_x_operSetting;		// Current active link settings
	int8_t						m_ai8_interface_type;		// Integer indicating interface type, for example: 6 is ethernet
}T_NETWORK_INTERFACE_LINK;

typedef struct
{
	char 						m_nwi_ref_token[TOKEN_LENGTH];				// Unique identifier referencing the physical entity
	bool						m_nwi_enabled;						// Indicates whether or not an interface is enabled
	int8_t						m_ai8_name[MAX_HOST_NAME_LEN+1];			// Network interface name, for example eth0
	int						m_ai32_mtu;						// Maximum transmission unit
	T_NETWORK_INTERFACE_LINK			m_x_link;						// Link configuration
}T_NETWORK_INTERFACES;

// typedef struct
// {
// 	int8_t					m_ai8_hostName[MAX_HOST_NAME_LEN+1];
// 	u_int32_t				m_ui32_default_gateway;
// 	E_IP_ASSIGN_MODE			m_e_ipAssignMode;
// 	T_NW_IPV4_PARAMETER			m_x_ipConfig;
// 	T_DNS_IPV4_PARAMETER			m_x_dns;
// }T_NETWORK_IPV4_LANSETTING;


typedef enum
{
    E_NTP_IPV4,
    E_NTP_IPV6,
    E_NTP_DNS
}E_NTP_TYPE;

typedef enum
{
    CURRENT_TIME_MODE,			//set current system time
    SYNC_COMUTER_TIME_MODE,		//set host machine time
    MANUAL_TIME_MODE,			//set manual time
    AUTO_TIME_MODE,			//set ntp server time
    MAX_MODE
}E_TIME_SET_MODE;

//**************Network IPV4 Setting *******************

typedef struct
{
    u_int8_t 	imp_aui8_ipAddress[MAX_IPV4_IP_ADDR_LEN];			// ip address
    u_int8_t 	imp_aui8_subnetMask[MAX_IPV4_SUBNET_MASK_LEN];	// Subnet mask
    u_int8_t 	imp_aui8_gateway[MAX_IPV4_GATEWAY_ADDRESS_LEN];	// gateway address
    u_int8_t	imp_aui8_macAddress[MAX_MAC_ADDR_LEN];			// MAC address
    u_int8_t	imp_aui8_ipv4PrefixLength;						// Prefix Length
}T_NETWORK_IPV4_PARAMETER;

extern T_NETWORK_IPV4_PARAMETER network_ipv4_param;


//***********Network Protocols ***************************
typedef struct
{
    uint8_t         imp_ui8_httpState;
    uint16_t		imp_ui16_httpPort;

    uint8_t         imp_ui8_httpsState;
    u_int16_t		imp_ui16_httpsPort;

    uint8_t         imp_ui8_rtspState;
    uint16_t		imp_ui16_rtspPort;

}T_NETWORK_PROTOCOL_SETTINGS;

extern T_NETWORK_PROTOCOL_SETTINGS network_protocol;

//*********** NTP Server ********************************
typedef struct
{
    uint8_t imp_ui8_IsDhcp;
    E_NTP_TYPE imp_e_ntpType;
    uint8_t imp_ua_ntpAddress[MAX_NTP_SERVER_LEN + 1];
}T_NTP_SETTING;

extern T_NTP_SETTING ntp_server;

typedef enum
{
    VIEWER_USER                     = 0,
    OPERATOR_USER                   = 1,
    ADMIN_USER                      = 2,
    DISABLED_USER                   = 3,
    USERNAME_INVALID                = 4,
    PASSWORD_INVALID                = 5,
    INVALID_RESPONSE_RECEIVED       = 6,
    DBUS_OPERATION_FAILED           = 7,
    INVALID_INPUT_ARG               = 8,
    AUTHENTICATION_FAIL             = 401,
    UI_AUTHENTICATION_FAIL          = 470
}E_ONVIF_AUTHENTICATION;

typedef enum
{
    ACCESS_PRE_AUTH = 0,           //Admin, opr, user, anonymous
    ACCESS_READ_SYSTEM,		    //Admin, opr, user
    ACCESS_READ_SYSTEM_SENSITIVE,	//Admin, opr, user
    ACCESS_READ_SYSTEM_SECRET,	    //Admin
    ACCESS_WRITE_SYSTEM,		    //Admin
    ACCESS_UNRECOVERABLE,		    //Admin
    ACCESS_READ_MEDIA,		        //Admin, opr, user
    ACCESS_ACTUATE			        //Admin, opr
}E_ACCESS_CLASS;

//***********System Flags ***************************
typedef struct
{
    bool onvif_reboot_flag;

}T_SYSTEM_SETTINGS;

extern T_SYSTEM_SETTINGS system_setting;

// // /*********************************** UPNP Setting start *********************************/
// // #define		MAX_FRIENDLY_NAME_LEN			64
// // #define     MIN_PORT_HTTP                   		1
// // #define     MAX_PORT_HTTP     				65535
// // #define     MIN_PORT_RTSP                   		1
// // #define     MAX_PORT_RTSP     				65535

// // typedef enum
// // {
// //     UPNP_DISABLE=0,
// //     UPNP_ENABLE,
// //     MAX_NW_UPNP_STATE
// // }E_NW_UPNP_STATE;

// // typedef enum
// // {
// //     UPNP_PORT_DISABLE=0,
// //     UPNP_PORT_ENABLE,
// //     MAX_NW_UPNP_PORT_STATE
// // }E_NW_UPNP_PORT_STATE;

// // typedef enum
// // {
// //     PORT_MODE_AUTO=0,
// //     PORT_MODE_MANUAL,
// //     MAX_PORT_MODE
// // }E_PORT_MODE;

// // #pragma pack(push, 1)
// // typedef struct
// // {
// //     E_NW_UPNP_STATE 	    m_e_enable;   					//enable upnp
// //     UINT8				    m_ai8_friendlyName[MAX_FRIENDLY_NAME_LEN+1];	//Friendly Name
// //     E_NW_UPNP_PORT_STATE    m_e_portFwdEnable;				//enable port forwarding
// //     E_PORT_MODE			    m_e_portFwdMode;				//port forwarding Mode
// //     UINT32				    m_ui32_extHttpPort;				//external http port
// //     UINT32				    m_ui32_extRtspPort;				//external rtsp port
// // }T_UPNP_SETTING;
// // #pragma pack(pop)
// // /************************************ UPNP Setting end **********************************/


// // /**************************************** DDNS Setting start **************************************/
// // #define		MAX_DDNS_USERNAME_LEN				32
// // #define		MAX_DDNS_PASSWORD_LEN				16
// // #define		MAX_DDNS_HOSTNAME_LEN				48
// // #define		MAX_DDNS_SERVER_LEN					48
// // #define		MAX_DDNS_SERVER_LIST_LEN			(MAX_DDNS_SERVER_LEN * 10)
// // #define		MIN_DDNS_UPDATE_DURATION			5
// // #define		MAX_DDNS_UPDATE_DURATION			60

// // typedef enum
// // {
// //     DDNS_DISABLED=0,
// //     DDNS_ENABLED,
// //     MAX_NW_DDNS_STATE
// // }E_NW_DDNS_STATE;

// // typedef enum
// // {
// //     NO_UPDATE=0,
// //     CLIENT_UPDATES,
// //     SERVER_UPDATES
// // }E_NW_DDNS_TYPE;

// // #pragma pack(push, 1)
// // typedef struct
// // {
// //     E_NW_DDNS_STATE 	m_e_enable;						// control to enable/disable DDNS service
// //     INT8 				m_ai8_username[MAX_DDNS_USERNAME_LEN+1];		// username for authentication at DNS server
// //     INT8 				m_ai8_password[MAX_DDNS_PASSWORD_LEN+1];		// password for authentication at DNS server
// //     INT8 				m_ai8_hostname[MAX_DDNS_HOSTNAME_LEN+1];		// hostname to be updated at DNS server
// //     UINT8 				m_ui8_updateDuration;					// update duration for DNS, in minutes
// //     INT8				m_ai8_server[MAX_DDNS_SERVER_LEN+1];			// DDNS server
// // }T_DDNS_SETTING;

// // typedef struct
// // {
// //     E_NW_DDNS_STATE 	m_e_enable;
// //     E_NW_DDNS_TYPE      m_e_type;
// //     INT8				m_i8_currProvider[MAX_DDNS_SERVER_LEN+1];
// //     CHAR 				m_ac_supportedServer[MAX_DDNS_SERVER_LIST_LEN+1];	// List of supported provider
// // }T_DDNS_MISC;

// // typedef struct
// // {
// //     T_DDNS_MISC						m_x_ddnsMisc;
// //     UINT8 							m_ui8_noOfProvider;
// //     std::vector<T_DDNS_SETTING>		m_v_providerList;

// // }T_DDNS_ACCOUNTS;

// // #pragma pack(pop)
// // /****************************************** DDNS Setting end ****************************************/


// typedef struct
// {
// 	T_NETWORK_IPV4_LANSETTING		m_x_ipv4Lanset;
// 	// T_UPNP_SETTING				    m_x_upnpSet;
// 	// T_DDNS_MISC					    m_x_ddnsSet;
// 	// T_DSCP_SETTING					m_x_dscpSet;
// 	// T_COS_SETTING					m_x_cosSet;
// 	// T_HTTPS_SETTING					m_x_httpsSet;
// 	// T_SNMP_SETTING					m_x_snmpSet;
// 	// T_NETWORK_IPV6_SETTING			m_x_ipv6Lanset;
// 	// T_STREAM_HTTP					m_x_streamHttp;
// 	// T_STREAM_RTSP					m_x_streamRtsp;
// 	// T_WINS_SERVER_SETTING			m_x_winsServerSet;
// 	T_NETWORK_INTERFACES			m_x_network_interfaces;
//     // T_CYBER_SECURITY_SETTING        m_x_cyberSecSettings;

// }T_NETWORK_SETTING;

int onvif_fault(struct soap *soap, int flag, CCHARPTR value1, CCHARPTR value2, CCHARPTR reason);
void onvif_fault_probe(struct soap *soap, char* value1, char* reason);
int dom_att(struct soap_dom_attribute *dom, int num_of_attributes, char * name[], char * data[], char * nstr, wchar_t *wide, struct soap *soap);
void hexToRGB(unsigned int hexValue, int *r, int *g, int *b);
void RGBtoYCbCr(int r, int g, int b, double *Y, double *Cb, double *Cr);
void inverseRescaleCoordinates(int position_x, int position_y, float *x, float *y);
void rescaleCoordinates(float x, float y, int *rescaledX, int *rescaledY);
char* ycbcrToHex(float y, float cb, float cr);
void hexToYCbCr(const char *hexColor, float *Y, float *Cb, float *Cr);
void omitFirstFourChars(const char *input, char *output);

#endif
