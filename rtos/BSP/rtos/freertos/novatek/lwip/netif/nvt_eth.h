#include "lwip/arch.h"
#include "lwip/netif.h"
/*
    Ethernet Configuration ID

*/
typedef enum {
	ETH_CONFIG_ID_SET_MEM_REGION,   //< Install buffer address.
	//< Context should be DRAM address
	ETH_CONFIG_ID_MAC_ADDR,         //< local MAC address
	//< Context should be a unsigned char array
	//< length should be 6 bytes
	ETH_CONFIG_ID_LOOPBACK,         //< loopback option
	//< Context can be:
	//< - @b ETH_LOOPBACK_MODE_DISABLE
	//< - @b ETH_LOOPBACK_MODE_MAC
	//< - @b ETH_LOOPBACK_MODE_PHY
	ETH_CONFIG_ID_WRITE_DETECT_TEST,//< Write detect test option
	//< Context can be: TRUE or FALSE
	ETH_CONFIG_ID_SPEED,            //< Force MAC/PHY speed
	//< Context can be:
	//< - @b 10
	//< - @b 100
	//< - @b 1000
	ETH_CONFIG_ID_DUPLEX,           //< Force MAC/PHY duplex
	//< Context can be:
	//< - @b ETH_DUPLEX_HALF
	//< - @b ETH_DUPLEX_FULL


	ETH_CONFIG_ID_COUNT,            //< MAC address count

	ETH_CONFIG_SET_LPI,             //< Enable/Disable LPI
	ETH_CONFIG_SET_PMT,             //< Enable/Disable PMT
	ETH_CONFIG_SET_MSS,             //< Set MSS
	ETH_PHY_RST_BYPASS,             //< Set phy rst bypass

	//NUM_DUMMY4WORD(ETH_CONFIG_ID)
} ETH_CONFIG_ID;

typedef enum {
	ETH_PAYLOAD_RAW,                //< Raw data
	ETH_PAYLOAD_IP,                 //< IP(v4) packet
	ETH_PAYLOAD_ARP,                //< ARP packet
	ETH_PAYLOAD_TCP,                //< TCP packet (IP/TCP)
	ETH_PAYLOAD_UDP,                //< UDP packet (IP/UDP)

	//ENUM_DUMMY4WORD(ETH_PAYLOAD_ENUM)
} ETH_PAYLOAD_ENUM;

/*
    Ethernet loopback mode

*/
typedef enum {
	ETH_LOOPBACK_MODE_DISABLE,      //< Normal (disable loopback)
	ETH_LOOPBACK_MODE_MAC,          //< Loopback at MAC layer
	ETH_LOOPBACK_MODE_PHY,          //< Loopback at PHY layer
	ETH_LOOPBACK_MODE_EXTERNAL,     //< Loopback at External layer

	//ENUM_DUMMY4WORD(ETH_LOOPBACK_MODE_ENUM)
} ETH_LOOPBACK_MODE_ENUM;

typedef struct  {
	int (*nvt_eth_open)(void);
	int (*nvt_eth_send)(u8_t *pDstAddr, ETH_PAYLOAD_ENUM type, u32_t uiLen, u8_t *pData);
	int (*nvt_eth_rcv)(u32_t uiBufSize, u32_t *puiLen, u8_t *pData);
	int (*nvt_eth_set_config)(ETH_CONFIG_ID configID, u32_t uiConfig);
} eth_callback;

#define MAC_LOOP 0
#define PHY_LOOP 0
#define EXT_LOOP 0

extern void eth_set_callback(eth_callback *callback);
extern void eth_lwip_init(void);
extern void ethernetif_input(void * pvParameters);
extern void eth_set_local_netif(struct netif *netif);
extern void eth_set_top(void);
extern void nvt_eth_init(void);
