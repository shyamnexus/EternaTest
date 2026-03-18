#include "platform_port.h"
#include "kwrap/cpu.h"
#include "kwrap/spinlock.h"
#include "kdrv_videodec.h"
#include "h26xdec_platform.h"
//#include "log.h"

#define H26X_DEV_NAME			"h26xdec"
#define MODULE_NAME				"DE"    /* two bytes character */
#define ENTITY_CHIPS			H26XD_CHIP      /* number of chips */
#define ENTITY_ENGINES			H26XD_NUM       /* number of hardware engines */
#define ENTITY_MINORS			H26XD_CHN       /* job's minor number must be within the rage: 0 ~ (ENTITY_MINORS-1) */
#define MAX_CHN_NUM         ENTITY_MINORS      /* the max number of channels that may co-exist at a time */
#define MAX_FRAME_BUFFER_SIZE          (32)
#define ENGINE_IDX(chip, engine)    ((chip*ENTITY_ENGINES) + engine)

#ifdef __KERNEL__
#define H26XD_HZ        1000
#define H26XD_JIFFIES   get_nvt_jiffies()
#else
#define H26XD_HZ        1000000
#define H26XD_JIFFIES   jiffies_to_usecs(jiffies)
#endif

#define H26XD_DEFINE_SPINLOCK(x)    vk_spinlock_t x = __VK_SPIN_LOCK_UNLOCKED(x)
#define h26xd_spin_lock_init(lock_ptr)                vk_spin_lock_init(lock_ptr)
#define h26xd_spin_lock_irqsave(lock_ptr, flags)       vk_spin_lock_irqsave(lock_ptr, flags)
#define h26xd_spin_unlock_irqrestore(lock_ptr, flags)   vk_spin_unlock_irqrestore(lock_ptr, flags)

#define h26xdec_dcache_sync(addr,size)      vos_cpu_dcache_sync(addr, size, VOS_DMA_BIDIRECTIONAL)
#define h26xdec_dcache_clean(addr,size)     vos_cpu_dcache_sync(addr, size, VOS_DMA_TO_DEVICE)
#define h26xdec_dcache_invalidate(addr,size)    vos_cpu_dcache_sync(addr, size, VOS_DMA_FROM_DEVICE)

/* global variables that controls the log function behavior */
extern int h26xd_dbglevel;
extern int h26xd_dbg_mode;
extern int h26xd_dbgpos;

/* level of log message in K_DRV flow */
#ifndef LOG_ERROR
/* copy from imvq.h */
#define LOG_ERROR		0
#define LOG_WARNING		1
#define LOG_INFO		2
#endif

extern void printm(char *module, const char *fmt, ...);

/*	print bitstream message when error happen
*	0: disable, 1: printm, 2: printk
*/
extern unsigned int dump_bs_msg[4];
#define DUMP_BS_MSG(fmt, args...) do { \
       const char *fmt_str = fmt;                        \
	   switch(dump_bs_msg[0]){							\
	   case 1:										\
		   printm(MODULE_NAME, fmt_str, ## args);       \
		   break;									\
	   case 2:										\
		   printk(fmt_str, ## args);    \
		   break;	\
	   default:	\
		   break;\
		}\
    }while(0)

int h26xd_kdrv_init(void);
void h26xd_kdrv_cleanup(int fail_flag);

int h26xdec_open(unsigned int chip, unsigned int codec_enigne);
int h26xdec_close(unsigned int id);
int h26xdec_setInitial(unsigned int handle, KDRV_VDODEC_H26X_DATA *p_dec_param);
int h26xdec_setCallbackFunc(void *param);
int h26xdec_prepareOnePicture(unsigned int handle, void *p_dec_param);
int h26xdec_getChipInfo(void *param);
int h26xdec_getCabacNeedBufferSize(void *param);
int h26xdec_freeCabacBuffer(void *param);
int h26xdec_setCabacBuffer(void *param);
