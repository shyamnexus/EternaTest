#ifndef __MODULE_IOCTL_CMD_H_
#define __MODULE_IOCTL_CMD_H_

#include <linux/ioctl.h>
#include "kwrap/type.h"
#include "comm/sif_api.h"

typedef struct config_info {
    SIF_CH Ch;
    SIF_CONFIG_ID ConfigID;
    UINT32 uiConfig;
} CONFIG_INFO;

typedef struct tx_info {
    SIF_CH Ch;
    UINT32 uiData0;
    UINT32 uiData1;
    UINT32 uiData2;
} TX_INFO;

#define MODULE_REG_LIST_NUM     10

typedef struct reg_info {
    unsigned long uiAddr;
    unsigned int uiValue;
} REG_INFO;

typedef struct reg_info_list {
    unsigned int uiCount;
    REG_INFO RegList[MODULE_REG_LIST_NUM];
} REG_INFO_LIST;

//============================================================================
// IOCTL command
//============================================================================
#define SIF_IOC_COMMON_TYPE 'M'
#define SIF_IOC_START                   _IO(SIF_IOC_COMMON_TYPE, 1)
#define SIF_IOC_STOP                    _IO(SIF_IOC_COMMON_TYPE, 2)

#define SIF_IOC_READ_REG                _IOWR(SIF_IOC_COMMON_TYPE, 3, void*)
#define SIF_IOC_WRITE_REG               _IOWR(SIF_IOC_COMMON_TYPE, 4, void*)
#define SIF_IOC_READ_REG_LIST           _IOWR(SIF_IOC_COMMON_TYPE, 5, void*)
#define SIF_IOC_WRITE_REG_LIST          _IOWR(SIF_IOC_COMMON_TYPE, 6, void*)


#define SIF_IOC_OPEN                    _IOWR(SIF_IOC_COMMON_TYPE, 7,  void*)
#define SIF_IOC_CLOSE                   _IOWR(SIF_IOC_COMMON_TYPE, 8,  void*)
#define SIF_IOC_ISOPENED                _IOWR(SIF_IOC_COMMON_TYPE, 9,  void*)
#define SIF_IOC_SET_CONFIG              _IOWR(SIF_IOC_COMMON_TYPE, 10, void*)
#define SIF_IOC_GET_CONFIG              _IOWR(SIF_IOC_COMMON_TYPE, 11, void*)
#define SIF_IOC_SEND                    _IOWR(SIF_IOC_COMMON_TYPE, 12, void*)
#define SIF_IOC_WAIT_END                _IOWR(SIF_IOC_COMMON_TYPE, 13, void*)


/* Add other command ID here*/


#endif
