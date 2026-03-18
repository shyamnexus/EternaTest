#ifndef __UVCP_IOCTL_CMD_H_
#define __UVCP_IOCTL_CMD_H_

#include <linux/ioctl.h>

#define MODULE_REG_LIST_NUM     10



typedef struct {
	UINT32 			in1;
	UINT32			in2;
	UINT32			in3;
	uintptr_t               in4;
	UINT32                  in5;
	uintptr_t               in6;
	UINT32			*out1;
} UVCP_IOCAPI_DATA;



//============================================================================
// IOCTL command
//============================================================================
#define UVCP_IOC_COMMON_TYPE 'V'

#define UVCP_IOC_OPEN                _IO(UVCP_IOC_COMMON_TYPE,		1)
#define UVCP_IOC_CLOSE               _IO(UVCP_IOC_COMMON_TYPE,		2)
#define UVCP_IOC_WAIT_COMPLETE       _IO(UVCP_IOC_COMMON_TYPE,		3)
#define UVCP_IOC_WAIT_BREAK          _IO(UVCP_IOC_COMMON_TYPE,		4)

#define UVCP_IOC_IS_OPENED           _IOWR(UVCP_IOC_COMMON_TYPE,	10, void*)
#define UVCP_IOC_TRIGGER             _IOWR(UVCP_IOC_COMMON_TYPE,	11, void*)
#define UVCP_IOC_SET_CONFIG          _IOWR(UVCP_IOC_COMMON_TYPE,	12, void*)
#define UVCP_IOC_GET_CONFIG          _IOWR(UVCP_IOC_COMMON_TYPE,	13, void*)
#define UVCP_IOC_CFG_DMA             _IOWR(UVCP_IOC_COMMON_TYPE,	14, void*)

//#ifdef  _NVT_UVCP2_SUPPORT
#define UVCP2_IOC_COMMON_TYPE 'V'

#define UVCP2_IOC_OPEN                _IO(UVCP2_IOC_COMMON_TYPE,          15)
#define UVCP2_IOC_CLOSE               _IO(UVCP2_IOC_COMMON_TYPE,          16)
#define UVCP2_IOC_WAIT_COMPLETE       _IO(UVCP2_IOC_COMMON_TYPE,          17)
#define UVCP2_IOC_WAIT_BREAK          _IO(UVCP2_IOC_COMMON_TYPE,          18)

#define UVCP2_IOC_IS_OPENED           _IOWR(UVCP2_IOC_COMMON_TYPE,        19, void*)
#define UVCP2_IOC_TRIGGER             _IOWR(UVCP2_IOC_COMMON_TYPE,        20, void*)
#define UVCP2_IOC_SET_CONFIG          _IOWR(UVCP2_IOC_COMMON_TYPE,        21, void*)
#define UVCP2_IOC_GET_CONFIG          _IOWR(UVCP2_IOC_COMMON_TYPE,        22, void*)
#define UVCP2_IOC_CFG_DMA             _IOWR(UVCP2_IOC_COMMON_TYPE,        23, void*)

//#endif


#endif
