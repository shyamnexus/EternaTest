#ifndef __AI_IOCTL_CMD_H_
#define __AI_IOCTL_CMD_H_

#include "kwrap/ioctl.h"

#include "kdrv_ai.h"

#define MODULE_REG_LIST_NUM     10

typedef struct reg_info {
	uintptr_t reg_addr;
	unsigned int reg_value;
} REG_INFO;

typedef struct reg_info_list {
	unsigned int reg_cnt;
	REG_INFO reg_list[MODULE_REG_LIST_NUM];
} REG_INFO_LIST;

typedef struct AI_PAT_PARM_ {
	UINT32 golden_chksum;
	uintptr_t cmd_pa;
	UINT32 round;
	UINT32 log_cnt;
} AI_PAT_PARM;
//============================================================================
// IOCTL command
//============================================================================
#define AI_IOC_COMMON_TYPE 'M'
#define AI_IOC_START                    _VOS_IO(AI_IOC_COMMON_TYPE, 1)
#define AI_IOC_STOP                     _VOS_IO(AI_IOC_COMMON_TYPE, 2)

#define AI_IOC_READ_REG                 _VOS_IOWR(AI_IOC_COMMON_TYPE, 3, void*)
#define AI_IOC_WRITE_REG                _VOS_IOWR(AI_IOC_COMMON_TYPE, 4, void*)
#define AI_IOC_READ_REG_LIST            _VOS_IOWR(AI_IOC_COMMON_TYPE, 5, void*)
#define AI_IOC_WRITE_REG_LIST           _VOS_IOWR(AI_IOC_COMMON_TYPE, 6, void*)

#define AI_IOC_GET_VER			        _VOS_IOWR(AI_IOC_COMMON_TYPE, 7, void*)
#define AI_IOC_PAT_TEST			        _VOS_IOWR(AI_IOC_COMMON_TYPE, 8, void*)

/* Add other command ID here*/


#if defined(__FREERTOS)
int nvt_ai_ioctl(int fd, unsigned int uiCmd, void *p_arg);
void nvt_ai_drv_init_rtos(void);
void nvt_ai_drv_uninit_rtos(void);
UINT8 * kdrv_ai_vmalloc(UINT32 v_size);
VOID kdrv_ai_vfree(UINT8 *v_buff);
#endif


#endif
