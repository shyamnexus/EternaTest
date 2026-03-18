#ifndef _ISP_H_
#define _ISP_H_

//=============================================================================
// struct & enum definition
//=============================================================================
#define AIISP_LOAD_JSON                 1
#define AIISP_PARAM_SOCKET              1
#define AIISP_PARAM_SYNC                1
#define ISP_FPN_ENABLE                  1

#if (AIISP_PARAM_SOCKET)
// ================= Socket define ==========================
#define READ_AIISP_PQ_PARAM 11
#define WRITE_AIISP_PQ_PARAM 12

#define CBW_SIGNATURE     0x43425355 //CBSU
#define CSW_SIGNATURE     0x53425355 //SBSU

#define CBW_TYPE_READ 0x80
#define CBW_TYPE_READ_ARG 0x40
#define CBW_TYPE_WRITE 0x00

#define CBW_TYPE_DEMON_READ      (CBW_TYPE_READ|0x1)
#define CBW_TYPE_DEMON_WRITE     (CBW_TYPE_WRITE|0x1)
#define CBW_TYPE_DEMON_READ_ARG (CBW_TYPE_READ | CBW_TYPE_READ_ARG|0x1)
#define CBW_TYPE_IO_READ         (CBW_TYPE_READ|0x2)
#define CBW_TYPE_IO_WRITE        (CBW_TYPE_WRITE|0x2)
#define CBW_TYPE_IO_READ_ARG (CBW_TYPE_READ | CBW_TYPE_READ_ARG|0x2)
#define CBW_TYPE_VENDOR_READ     (CBW_TYPE_READ|0x3)
#define CBW_TYPE_VENDOR_WRITE    (CBW_TYPE_WRITE|0x3)
#define CBW_TYPE_VENDOR_READ_ARG (CBW_TYPE_READ | CBW_TYPE_READ_ARG|0x3)
#define CBW_TYPE_COMBO_CMD    (CBW_TYPE_READ | CBW_TYPE_READ_ARG|0x7)

#define CSW_NOVA_TAG_ERROR 0x6001
#define CSW_SIGNATURE_TAG_ERROR 0x6002
#define CSW_STATUS_ERROR 0x6003
#define CSW_ISP_DRIVER_ERROR 0x6004

#define WRITE_SOCKET_ERROR 0x6101
#define READ_SOCKET_ERROR 0x6102

#define ISPD_CMD_PASS   0
#define ISPD_CMD_FAIL   1
#define ISPD_CMD_NONE   2

typedef struct _ISPD_CBW
{
	unsigned int dSignature;
	unsigned int dTag;
	unsigned int dDataTransferLength;
	unsigned char bmFlags;
	unsigned char bLUN;
	unsigned char bCBLength;
	unsigned char CBWCB[16];
} ISPD_CBW;

typedef struct _ISPD_CSW
{
	unsigned int   dCSWSignature;
	unsigned int   dCSWTag;
	unsigned int   dCSWDataResidue;
	unsigned char  bCSWStatus;
} ISPD_CSW;
#endif

#if (ISP_FPN_ENABLE)
typedef struct _ISP_DDR_INFO {
	void *va;
	UINTPTR pa;
	UINT32 size;
} ISP_DDR_INFO;
#endif

//=============================================================================
// extern functions
//=============================================================================
extern void isp_set_total_gain(UINT32 gain);
extern BOOL isp_get_msg_print(void);
extern void isp_set_msg_print(BOOL enable);
extern HD_RESULT isp_enable_socket(void);
extern HD_RESULT isp_disable_socket(void);
extern HD_RESULT isp_get_json_name(CHAR *json_name);
extern HD_RESULT isp_set_json_name(CHAR *json_name);
extern HD_RESULT isp_load_json(void);
extern HD_RESULT isp_get_fpn_name(CHAR *fpn_name);
extern HD_RESULT isp_set_fpn_name(CHAR *fpn_name);
extern HD_RESULT isp_enable_fpn(UINT32 isp_id);
extern HD_RESULT isp_disable_fpn(void);
#endif

