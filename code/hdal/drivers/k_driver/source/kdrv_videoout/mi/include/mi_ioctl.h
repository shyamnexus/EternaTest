#ifndef __MI_IOCTL_CMD_H_
#define __MI_IOCTL_CMD_H_

#include <linux/ioctl.h>

#define MODULE_REG_LIST_NUM     10

typedef struct reg_info {
	uintptr_t addr;
	unsigned int value;
} REG_INFO;

typedef struct reg_info_list {
	unsigned int count;
	REG_INFO reg_list[MODULE_REG_LIST_NUM];
} REG_INFO_LIST;

//============================================================================
// IOCTL command
//============================================================================
#define MI_IOC_COMMON_TYPE 'M'
#define MI_IOC_START                   _IO(MI_IOC_COMMON_TYPE, 1)
#define MI_IOC_STOP                    _IO(MI_IOC_COMMON_TYPE, 2)

#define MI_IOC_READ_REG                _IOWR(MI_IOC_COMMON_TYPE, 3, void*)
#define MI_IOC_WRITE_REG               _IOWR(MI_IOC_COMMON_TYPE, 4, void*)
#define MI_IOC_READ_REG_LIST           _IOWR(MI_IOC_COMMON_TYPE, 5, void*)
#define MI_IOC_WRITE_REG_LIST          _IOWR(MI_IOC_COMMON_TYPE, 6, void*)




/* Add other command ID here*/


#endif
