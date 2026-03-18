/**
	@brief Source file of IO control of vendor net flow sample.

	@file net_flow_sample_ioctl.c

	@ingroup net_flow_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "kwrap/type.h"
#include "kwrap/util.h"
#include <linux/cdev.h>
#include <kwrap/dev.h>
#include <kwrap/spinlock.h>
#include <plat/top.h>
//#include "frammap/frammap_if.h"
#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h>
#else
#include <mach/fmem.h>
#endif
#include "kflow_ai_net/kflow_ai_net.h"
#include "kflow_ai_net/kflow_ai_net_comm.h"
#include "kflow_ai_net/nn_net.h"
#include "kflow_ai_net/nn_verinfo.h"
#include "kflow_ai_net_ioctl.h"
#include "kflow_ai_net_parm.h"
#include "../kflow_ai_net_int.h"
//=============================================================
#define __CLASS__ 				"[ai][kflow][ictl]"
#include "kflow_ai_debug.h"
//=============================================================

#include "kflow_ai_net/kflow_ai_core.h"
#include "kflow_ai_net/kflow_ai_core_callback.h"
#include "kflow_ai_net/kflow_ai_net_platform.h"

#include "kflow_conv/kflow_conv.h"
#include "kflow_rou/kflow_rou.h"
#include "kflow_nue2/kflow_nue2.h"
#include "kflow_jobm/kflow_jobm.h"
#include "kflow_jmisp/kflow_jmisp.h"
#include "kflow_cpu/kflow_cpu.h"
#include "kflow_cpu/kflow_cpu_callback.h"
#include "kflow_dsp/kflow_dsp.h"
#include "kflow_dsp/kflow_dsp_callback.h"
#include "kflow_hte/kflow_hte.h"
#include "kflow_cal/kflow_cal.h"
#include "kflow_lsu/kflow_lsu.h"
#include "kflow_util/kflow_util.h"
#include "kflow_ppu/kflow_ppu.h"
#include "kflow_pou/kflow_pou.h"

#include "kflow_ai_net_proc.h"
#include "kflow_ai_version.h"
#include "kdrv_ai.h"

unsigned int kflow_ai_debug_level = NVT_DBG_ERR;
#if defined (__LINUX)
module_param_named(kflow_ai_debug_level, kflow_ai_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(kflow_ai_debug_level, "kflow_ai_debug_level");
#endif
unsigned int vendor_ai_debug_level = NVT_DBG_ERR;
#if defined (__LINUX)
module_param_named(vendor_ai_debug_level, vendor_ai_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(vendor_ai_debug_level, "vendor_ai_debug_level");
#endif

unsigned int ai_net_supported_num = AI_SUPPORT_PROC_DEF;
#if defined (__LINUX)
module_param_named(ai_net_supported_num, ai_net_supported_num, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ai_net_supported_num, "set ai_net_supported_number");
#endif

extern UINT32 kflow_init;
extern BOOL g_ai_net_init;
extern UINT32 kflow_init_count;
extern UINT32 g_ai_net_init_count;
extern BOOL *g_ai_net_path_open;
extern BOOL *g_ai_net_path_is_used;
extern BOOL *g_ai_net_path_need_reset;
extern SEM_HANDLE g_ai_path_sem_id;
extern UINT32 MAX_ISP;
extern BOOL nvt_ai_is_path_need_reset(VOID);
extern void kflow_ai_core_add_jobm_cycle(void);
extern KDRV_AI_JMISP_PATH_INFO **kflow_ai_net_gblk_drv_info;
extern KDRV_AI_JMISP_PATH_INFO **kflow_ai_net_postproc_drv_info;

SEM_HANDLE g_kflow_net_sem_id;
ID *g_kflow_release_flag_id;
/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
#define MODULE_NAME VENDOR_AIS_FLOW_DEV_NAME
typedef struct _KFLOW_AI_NET_DEV {
	struct class *pmodule_class;
	struct device **pdevice;
	//struct resource* presource[MODULE_REG_NUM];
	struct cdev cdev;
	dev_t dev_id;
	UINT32 minor_base;
} KFLOW_AI_NET_DEV, *PKFLOW_AI_NET_DEV;
static KFLOW_AI_NET_DEV kflow_ai_net_dev;

vk_spinlock_t ioctl_lock;     ///< device ioctl locker
extern VENDOR_AIS_FLOW_MAP_MEM_PARM g_ai_map_mem;
CHAR dbg_dump_outbuf_path[256] = {};

extern UINT32 debug_func;

//static void *vendor_ais_buf_va_addr = NULL;     //???!! from structure changed to void
/*-----------------------------------------------------------------------------*/
/* external Functions                                                          */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Functions                                                             */
/*-----------------------------------------------------------------------------*/

#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/fdtable.h>
#include <linux/fs.h> 
#include <linux/fs_struct.h>
#include <linux/dcache.h>
#include <linux/slab.h>

#if 0
void dump_fd(void)
{
	struct files_struct *current_files; 
	struct fdtable *files_table;
	//unsigned int *fds;
	int i=0;
	struct path files_path;
	char *cwd;
	char *buf = (char *)kmalloc(GFP_KERNEL,100*sizeof(char));

	printk("Dump file - begin\r\n");
	current_files = current->files;
	files_table = files_fdtable(current_files);
	while(files_table->fd[i] != NULL) { 
		files_path = files_table->fd[i]->f_path;
		cwd = d_path(&files_path,buf,100*sizeof(char));
		printk("Open file with fd[%d] = %s\r\n", i, cwd);
		i++;
	}
	printk("Dump file - end\r\n");

	kfree(buf);
}
#endif



static int vendor_ais_flow_miscdev_open(struct inode *inode, struct file *file)
{
	KFLOW_AI_NET_DEV *pdrv_info = &kflow_ai_net_dev;
    UINT32 net_max_num = kflow_ai_get_net_supported_num(); // net_max_num should be got from kdriver
    UINT32 minor = (MINOR(inode->i_rdev) - pdrv_info->minor_base);
    FLGPTN flag = 0, p_bit = 0;
    UINT32 p_id = 0;
	file->private_data = (void *)pdrv_info;

#if 0
	printk("<<open>> %s%d, pid =%d, process = %s\n", 
		MODULE_NAME, (MINOR(inode->i_rdev) - pdrv_info->minor_base),
		(int) task_pid_nr(current), current->comm);
	
	dump_fd();
#endif
    if (minor > 0 && minor <= net_max_num) {
        //default value is 1, blocking user after calling close fd until vendor_ais_flow_miscdev_release done
        p_id = (minor-1)/32;
        p_bit = FLGPTN_BIT((minor-1)%32);
        wai_flg(&flag, g_kflow_release_flag_id[p_id], p_bit, TWF_ORW | TWF_CLR);
    }
	return 0;
}


extern uintptr_t nvt_ai_user2kerl_va(uintptr_t addr, UINT32 net_id);
extern uintptr_t nvt_ai_kerl2user_va(uintptr_t addr, UINT32 net_id);
extern uintptr_t nvt_ai_user2user_va_in_kerl(uintptr_t addr, UINT32 net_id);

#if (FLOW_AI_JOB == 1)
static void kflow_cpu_exec(KFLOW_AI_JOB* p_job)
{
	if (p_job->state == 13) {   //ABORT
		//force stop (cancel last CPU job for reset after ctrl-c)
		kflow_ai_cpu_sig(p_job->proc_id, p_job);
		return;
	}

#if NN_DLI
	if (p_job->state == 0xff) {   //start
		//call user space to do CPU start
		kflow_ai_cpu_cb2(p_job->proc_id, p_job);
		p_job->state = 0;
		return;
	}
	
	if (p_job->state == 0xfe) {   //stop
		//call user space to do CPU stop
		kflow_ai_cpu_cb3(p_job->proc_id, p_job);
		p_job->state = 0;
		return;
	}
#endif
	
	//call user space to do CPU exec
	kflow_ai_cpu_cb(p_job->proc_id, p_job);
}

#if (AI_SUPPORT_DSP == 1)
static void kflow_dsp_exec(KFLOW_AI_JOB* p_job)
{
	if (p_job->state == 13) {
		//force stop (cancel last DSP job for reset after ctrl-c)
		kflow_ai_dsp_sig(p_job->proc_id, p_job);
		return;
	}

#if NN_DLI
	if (p_job->state == 0xff) {   //start
		//call user space to do DSP start
		kflow_ai_dsp_cb2(p_job->proc_id, p_job);
		p_job->state = 0;
		return;
	}
	
	if (p_job->state == 0xfe) {   //stop
		//call user space to do DSP stop
		kflow_ai_dsp_cb3(p_job->proc_id, p_job);
		p_job->state = 0;
		return;
	}
#endif
	
	//call user space to do DSP exec
	kflow_ai_dsp_cb(p_job->proc_id, p_job);
}
#endif
#endif

static long vendor_ais_flow_miscdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int __user *argp = (int __user *)arg;

#if (FLOW_AI_DEV == 1)
	VENDOR_AIS_MEM_INFO mem_info;
#endif
#if (FLOW_AI_NET == 1)
	VENDOR_AIS_FLOW_MAP_MEM_INFO map_mem_info;
#endif
#if (FLOW_AI_JOB == 1)
	VENDOR_AIS_FLOW_CORE_INFO core2_info;
#endif
#if (FLOW_AI_JOB == 1)
	VENDOR_AIS_FLOW_CORE_CFG core_cfg;
#endif
#if (FLOW_AI_DEV == 1)
	VENDOR_AIS_FLOW_ID id_info;
#endif
#if (FLOW_AI_JOB == 1)
	VENDOR_AIS_FLOW_JOBLIST_INFO joblist_info;
#endif
#if (FLOW_AI_JOB == 1)
	VENDOR_AIS_FLOW_JOB_INFO job_info;
#endif
#if (FLOW_AI_JOB == 1)
	KFLOW_AI_NET* p_net;
	KFLOW_AI_JOB* p_job;
	KFLOW_AI_JOB* p_next_job;
#endif


#if ((FLOW_AI_OP == 1) || \
	(FLOW_AI_PROCID == 1) || \
	(FLOW_AI_UBUFID == 1) || \
	(FLOW_AI_DEBUG == 1) || \
	(FLOW_AI_NET == 1))
	ER er = 0;
#endif 

	if (_IOC_TYPE(cmd) != VENDOR_AIS_FLOW_IOC_MAGIC) {
		ret = -ENOIOCTLCMD;
		goto exit;
	}

	switch (cmd) {
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_MEM_INFO:
		if (copy_from_user(&mem_info, (void __user *)arg, sizeof(VENDOR_AIS_MEM_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		nvt_ai_mem_info_update_1(&mem_info);
		ret = 0;
		break;
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_GET_NUM:
		if (copy_from_user(&id_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_ID))) {
			ret = -EFAULT;
			goto exit;
		}
		nvt_ai_get_max_net(&id_info);
		ret = (copy_to_user((void __user *)arg, &id_info, sizeof(VENDOR_AIS_FLOW_ID))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_NET_RESET:
		if (copy_from_user(&id_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_ID))) {
			ret = -EFAULT;
			goto exit;
		}
		SEM_WAIT(g_kflow_net_sem_id);
		nvt_ai_reset_net();
		SEM_SIGNAL(g_kflow_net_sem_id);
		ret = (copy_to_user((void __user *)arg, &id_info, sizeof(VENDOR_AIS_FLOW_ID))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_NET_INIT:
		if (copy_from_user(&id_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_ID))) {
			ret = -EFAULT;
			goto exit;
		}
		SEM_WAIT(g_kflow_net_sem_id);
		ret = nvt_ai_init_net(&id_info);
		if (E_OK != ret) {
			SEM_SIGNAL(g_kflow_net_sem_id);
			ret = -EFAULT;
			goto exit;
		}
		SEM_SIGNAL(g_kflow_net_sem_id);
		
		ret = (copy_to_user((void __user *)arg, &id_info, sizeof(VENDOR_AIS_FLOW_ID))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_NET_UNINIT:
		/*
		if (copy_from_user(&map_mem_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_ID))) {
			ret = -EFAULT;
			goto exit;
		}
		*/
		SEM_WAIT(g_kflow_net_sem_id);
		ret = nvt_ai_uninit_net();
		SEM_SIGNAL(g_kflow_net_sem_id);
		//ret = (copy_to_user((void __user *)arg, &map_mem_info, sizeof(VENDOR_AIS_FLOW_ID))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_NET == 1)
	case VENDOR_AIS_FLOW_IOC_NET_LOCK:
		if (copy_from_user(&id_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_ID))) {
			ret = -EFAULT;
			goto exit;
		}
		er = nvt_ai_lock_net(id_info.net_id);
		if (er == E_RLWAI) {
			ret = -ERESTARTSYS; //if lock_net() is NOT interruptted by a ctrl-c signal, this IOCTL will retry again.
			DBG_WRN("proc[%u] NET_LOCK get signal.\r\n", id_info.net_id);
		}
		//ret = (copy_to_user((void __user *)arg, &id_info, sizeof(VENDOR_AIS_FLOW_ID))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_NET == 1)
	case VENDOR_AIS_FLOW_IOC_NET_UNLOCK:
		if (copy_from_user(&id_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_ID))) {
			ret = -EFAULT;
			goto exit;
		}
		nvt_ai_unlock_net(id_info.net_id);
		//ret = (copy_to_user((void __user *)arg, &id_info, sizeof(VENDOR_AIS_FLOW_ID))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_COMMON_LOCK:
		 nvt_ai_comm_usr_lock();
		 break;
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_COMMON_UNLOCK:
		 nvt_ai_comm_usr_unlock();
		 break;	 	
#endif
#if (FLOW_AI_OP == 1)
	case VENDOR_AIS_FLOW_IOC_OP_LOCK:
		 nvt_ai_op_lock();
		 break;
#endif
#if (FLOW_AI_OP == 1)
	case VENDOR_AIS_FLOW_IOC_OP_UNLOCK:
		 nvt_ai_op_unlock();
		 break;	 		 
#endif
#if (FLOW_AI_NET == 1)
	case VENDOR_AIS_FLOW_IOC_REMAP_ADDR:
		if (copy_from_user(&map_mem_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_MAP_MEM_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		nvt_ai_open_net(&map_mem_info.parm, map_mem_info.net_id);
		ret = (copy_to_user((void __user *)arg, &map_mem_info, sizeof(VENDOR_AIS_FLOW_MAP_MEM_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_NET == 1)
	case VENDOR_AIS_FLOW_IOC_UNMAP_ADDR:
		if (copy_from_user(&map_mem_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_MAP_MEM_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		nvt_ai_close_net(map_mem_info.net_id);
		break;
#endif

#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_CORE_RESET:
		/*if (copy_from_user(&core2_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_CORE_INFO))) {
			ret = -EFAULT;
			goto exit;
		}*/
		{
            SEM_WAIT(g_kflow_net_sem_id);
            if(kflow_init_count == 0 && kflow_init == 1) 
            {
                kflow_ai_core_reset();
                kflow_ai_cpu_uninit_cb();
#if (AI_SUPPORT_DSP == 1)
                kflow_ai_dsp_uninit_cb();
#endif
            }
            SEM_SIGNAL(g_kflow_net_sem_id);
		}
		/*
		ret = (copy_to_user((void __user *)arg, &core2_info, sizeof(VENDOR_AIS_FLOW_CORE_INFO))) ? (-EFAULT) : 0;
		*/
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_CORE_INIT:
		if (copy_from_user(&core2_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_CORE_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
        SEM_WAIT(g_kflow_net_sem_id);
        kflow_init_count += 1;
        if(kflow_init == 0)
        {
            UINT32 max_nue2 = (core2_info.info >> 16) & 0xff;
            UINT32 max_cnn = (core2_info.info >> 8) & 0xff;
            UINT32 max_nue = (core2_info.info) & 0xff;
#if defined(_BSP_NS02302_)
            UINT32 real_chip_id = nvt_get_chip_id();
#endif
            ER rv = E_OK;

            kflow_ai_core_reset_engine();
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_CONV, kflow_conv_get_engine());
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_ROU, kflow_rou_get_engine());
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_NUE2, kflow_nue2_get_engine());
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_LSU, kflow_lsu_get_engine());
#if defined(_BSP_NS02201_) || defined(_BSP_NS02401_)
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_UTIL, kflow_util_get_engine());
#elif defined(_BSP_NS02302_)
            if(real_chip_id != CHIP_NS02402) {
                kflow_ai_core_add_engine(KFLOW_AI_ENGINE_UTIL, kflow_util_get_engine());
            }
#endif
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_CAL, kflow_cal_get_engine());
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_PPU, kflow_ppu_get_engine());
#if defined(_BSP_NS02302_)
            if(real_chip_id == CHIP_NS02402) {
                kflow_ai_core_add_engine(KFLOW_AI_ENGINE_POU, kflow_pou_get_engine());
            }
#endif
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_JOBM, kflow_jobm_get_engine());
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_JMISP, kflow_jmisp_get_engine());
            #if (USE_HTE == 1)
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_HTE, kflow_hte_get_engine());
            #else
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_CPU, kflow_cpu_get_engine()); // 13 = VENDOR_AI_ENGINE_CPU
#if (AI_SUPPORT_DSP == 1)
            kflow_ai_core_add_engine(KFLOW_AI_ENGINE_DSP, kflow_dsp_get_engine()); // 14 = VENDOR_AI_ENGINE_DSP
#endif
            #endif
            kflow_cpu_reg_exec_cb(kflow_cpu_exec);
#if (AI_SUPPORT_DSP == 1)
            kflow_dsp_reg_exec_cb(kflow_dsp_exec);
#endif
            {
                //KFLOW_AI_ENGINE_CTX* p_eng = kflow_cpu_get_engine();
                //p_eng->p_ch[0]->trigger = kflow_cpu_ch1_trig; //hook
            }

            //set sw-limit
            kflow_ai_core_get_engine(KFLOW_AI_ENGINE_CONV)->channel_max = max_cnn;
            kflow_ai_core_get_engine(KFLOW_AI_ENGINE_ROU)->channel_max = max_nue;
            kflow_ai_core_get_engine(KFLOW_AI_ENGINE_NUE2)->channel_max = max_nue2;

            rv = kflow_ai_cpu_init_cb();
            if (E_OK != rv) { DBG_ERR("cpu cb init failed ... !! ret = %d\r\n", rv);    ret = -EFAULT;    SEM_SIGNAL(g_kflow_net_sem_id);    goto exit;  }
#if (AI_SUPPORT_DSP == 1)
            rv = kflow_ai_dsp_init_cb();
            if (E_OK != rv) { DBG_ERR("dsp cb init failed ... !! ret = %d\r\n", rv);    ret = -EFAULT;    SEM_SIGNAL(g_kflow_net_sem_id);    goto exit;  }
#endif
            rv = kflow_ai_core_init();
            if (E_OK != rv) { DBG_ERR("ai core init failed ... !! ret = %d\r\n", rv);   ret = -EFAULT;    SEM_SIGNAL(g_kflow_net_sem_id);    goto exit;  }

            //get final count
            max_cnn = kflow_ai_core_get_engine(KFLOW_AI_ENGINE_CONV)->channel_count;
            max_nue = kflow_ai_core_get_engine(KFLOW_AI_ENGINE_ROU)->channel_count;
            max_nue2 = kflow_ai_core_get_engine(KFLOW_AI_ENGINE_NUE2)->channel_count;

            core2_info.info = (max_nue2 << 16) | (max_cnn << 8) | (max_nue);
        }
        SEM_SIGNAL(g_kflow_net_sem_id);
		ret = (copy_to_user((void __user *)arg, &core2_info, sizeof(VENDOR_AIS_FLOW_CORE_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_CORE_UNINIT:
		/*
		if (copy_from_user(&core2_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_CORE_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		*/
        SEM_WAIT(g_kflow_net_sem_id);
        //kflow_init_count -= 1;
        if(kflow_init_count == 1)
        {
            kflow_ai_core_uninit(0);
            kflow_ai_cpu_uninit_cb();
#if (AI_SUPPORT_DSP == 1)
            kflow_ai_dsp_uninit_cb();
#endif
        }
        SEM_SIGNAL(g_kflow_net_sem_id);
		/*
		ret = (copy_to_user((void __user *)arg, &core2_info, sizeof(VENDOR_AIS_FLOW_CORE_INFO))) ? (-EFAULT) : 0;
		*/
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_CORE_CFGSCHD:
		if (copy_from_user(&core_cfg, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_CORE_CFG))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			kflow_ai_core_cfgschd(core_cfg.schd);
		}
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_CORE_CFGCHK:
		if (copy_from_user(&core_cfg, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_CORE_CFG))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			kflow_ai_core_cfgchk(core_cfg.schd);
		}
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_NEW_JOBLIST:
		if (copy_from_user(&joblist_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		p_net = kflow_ai_core_net(joblist_info.proc_id);
		//DBG_DUMP("proc[%u] NEW_NET [max_job=%u, job=%u, bind=%u]\r\n", p_net->proc_id, joblist_info.max_job_cnt, joblist_info.job_cnt, joblist_info.bind_cnt);
		kflow_ai_net_create(p_net, joblist_info.max_job_cnt, joblist_info.job_cnt, joblist_info.bind_cnt, joblist_info.ddr_id);
        ret = (copy_to_user((void __user *)arg, &joblist_info, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_NEW_SUB_JOBLIST:
		if (copy_from_user(&joblist_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		p_net = kflow_ai_core_net(joblist_info.proc_id);
		//DBG_DUMP("proc[%u] NEW_NET [max_job=%u, job=%u, bind=%u]\r\n", p_net->proc_id, joblist_info.max_job_cnt, joblist_info.job_cnt, joblist_info.bind_cnt);
		kflow_ai_net_create_sub(p_net, joblist_info.job_cnt);
		ret = (copy_to_user((void __user *)arg, &joblist_info, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_DUMP_JOBLIST:
		if (copy_from_user(&joblist_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		p_net = kflow_ai_core_net(joblist_info.proc_id);
		kflow_ai_net_dump(p_net, joblist_info.job_cnt); //job_cnt = info
		ret = (copy_to_user((void __user *)arg, &joblist_info, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_DEL_JOBLIST:
		if (copy_from_user(&joblist_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		p_net = kflow_ai_core_net(joblist_info.proc_id);
		kflow_ai_net_destory(p_net);
		ret = (copy_to_user((void __user *)arg, &joblist_info, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_DEL_SUB_JOBLIST:
		if (copy_from_user(&joblist_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		p_net = kflow_ai_core_net(joblist_info.proc_id);
		kflow_ai_net_destory_sub(p_net);
		ret = (copy_to_user((void __user *)arg, &joblist_info, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_CLR_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] CLR_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			p_job = kflow_ai_net_job(p_net, job_info.job_id);
			if (p_job == NULL) {
				DBG_ERR("proc[%u] CLR_JOB, invalid job_id=%u?\r\n", p_net->proc_id, job_info.job_id);
				ret = 0;
				goto exit;
			}
			kflow_ai_core_clr_job(p_net, p_job);
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_SET_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			//job_info.info is a modectrl ptr!, convert user va to kernel va
#if (FLOW_AI_NET == 1)
            //uintptr_t info = nvt_ai_user2user_va_in_kerl(job_info.info, job_info.proc_id);
			uintptr_t info2 = nvt_ai_user2user_va_in_kerl(job_info.info2, job_info.proc_id);
#else
			//uintptr_t info = 0;
			uintptr_t info2 = 0;
#endif
			INT32 wait_ms = job_info.wait_ms;
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] SET_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			// DBG_DUMP("proc[%u] ADD_JOB [%u]\r\n", p_net->proc_id, job_info.job_id);
			p_job = kflow_ai_net_add_job(p_net, job_info.job_id);
			if (p_job == NULL) {
				DBG_ERR("proc[%u] SET_JOB, invalid job_id=%u?\r\n", p_net->proc_id, job_info.job_id);
				ret = 0;
				goto exit;
			}
			p_job->engine_id = job_info.engine_id;
			p_job->engine_op = job_info.engine_op;
			p_job->schd_parm = job_info.schd_parm;
			p_job->wait_ms = job_info.wait_ms;
			if(p_job->engine_id == KFLOW_AI_ENGINE_PPU || p_job->engine_id == KFLOW_AI_ENGINE_POU){
				// if engine is ppu/pou, skip job setting
				goto exit;
			}else if(p_job->engine_id == KFLOW_AI_ENGINE_JMISP){ // pathll mode (for gblktile)
                UINT32 core_id;
				p_job->p_eng = kflow_ai_core_get_engine(KFLOW_AI_ENGINE_JOBM); //running this job by JOBM engine
				if (p_job->p_eng == 0) {
					DBG_ERR("proc[%u] SET_JOB, job[%u], jobm eng is invalid?\r\n", p_net->proc_id, job_info.job_id);
					ret = 0;
					goto exit;
				}
				if (kflow_ai_net_gblk_drv_info == NULL) {
					DBG_ERR("proc[%u] SET_JOB, job[%u], kflow_ai_net_gblk_drv_info is null?\r\n", p_net->proc_id, job_info.job_id);
					ret = 0;
					goto exit;
				}
                core_id = job_info.job_id;      // use job_id as core_id
				info2 = (uintptr_t)&kflow_ai_net_gblk_drv_info[job_info.proc_id][core_id];
				p_job->engine_op |= 0x00008000 ;  //running this job by JOBM engine, and mark it with 0x00008000 for JMISP mode
			}else if(((p_job->engine_op)>>16) == 4) { //jobll mode
                p_job->p_eng = kflow_ai_core_get_engine(KFLOW_AI_ENGINE_JOBM);
                if (p_job->p_eng == 0) {
                    DBG_ERR("proc[%u] SET_JOB, job[%u], jobm eng is invalid?\r\n", p_net->proc_id, job_info.job_id);
                    ret = 0;
                    goto exit;
                }
            } else {
                p_job->p_eng = kflow_ai_core_get_engine(p_job->engine_id); //assign this job's engine!
                if (p_job->p_eng == 0) {
                    DBG_ERR("proc[%u] SET_JOB, job[%u], invalid engine_id=%u?\r\n", p_net->proc_id, job_info.job_id, p_job->engine_id);
                    ret = 0;
                    goto exit;
                }
            }
			
			//kflow_ai_core_set_job(p_net, p_job, job_info.engine_id, job_info.engine_op, job_info.schd_parm, (void*)info, (void*)info2, wait_ms);
			kflow_ai_core_set_job(p_net, p_job, (void*)&job_info.trig_info, (void*)info2, wait_ms);
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		break;
	case KFLOW_AI_IOC_GBLK_HAS_POOL:
		{
			BOOL has_pool ;
			if (copy_from_user(&has_pool, (void __user *)arg, sizeof(BOOL))) {
				ret = -EFAULT;
				goto exit;
			}
			ret = nvt_ai_has_pool(&has_pool);
			ret = (copy_to_user((void __user *)arg, &has_pool, sizeof(BOOL))) ? (-EFAULT) : 0;

		}
		break;
	case KFLOW_AI_IOC_GET_GBLK_DRV_INFO:
		{
			VENDOR_AIS_FLOW_GBLK_DRV_INFO drv_info = {0};
			if (copy_from_user(&drv_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_GBLK_DRV_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
			if (kflow_ai_net_gblk_drv_info == NULL) {
				DBG_ERR("kflow_ai_net_gblk_drv_info is NOT init yet ... !!\r\n");
				ret = 0;
				goto exit;
			}
			drv_info.drv_info_pa = (uintptr_t)nvt_ai_va2pa((uintptr_t)&kflow_ai_net_gblk_drv_info[drv_info.proc_id][0]); // start addr of (all core) for this proc_id

			if (unlikely(copy_to_user((void __user *)arg, &drv_info, sizeof(VENDOR_AIS_FLOW_GBLK_DRV_INFO)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
	case KFLOW_AI_IOC_SET_GBLK_POOL_INFO:
		{
			VENDOR_AIS_GBlkTile_POOL_INFO pool_info = {0};
			
			if (copy_from_user(&pool_info, (void __user *)arg, sizeof(VENDOR_AIS_GBlkTile_POOL_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
			if(pool_info.pool_id & 0xff)
				ret = kflow_ai_set_default_pool_info(&pool_info);
			else 
				ret = kflow_ai_set_pool_info(&pool_info);
		
		}
		break;
	case KFLOW_AI_IOC_GET_GBLK_POOL_UBUF:
		{
			VENDOR_AIS_UBUF_INFO ubuf_info = {0};

			if (copy_from_user(&ubuf_info, (void __user *)arg, sizeof(VENDOR_AIS_UBUF_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
			
			ret = kflow_ai_get_gblktile_ubuf(ubuf_info.proc_id, &ubuf_info.idx);
			if (ret != E_OK) { ret = -EFAULT ; goto exit; }

			if (unlikely(copy_to_user((void __user *)arg, &ubuf_info, sizeof(VENDOR_AIS_UBUF_INFO)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		
		}
		break;
	case KFLOW_AI_IOC_GET_GBLK_POOL_JOBM_CORE:
		{
			VENDOR_AIS_UBUF_INFO jobm_core_info = {0};

			if (copy_from_user(&jobm_core_info, (void __user *)arg, sizeof(VENDOR_AIS_UBUF_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
			
			ret = kflow_ai_get_gblktile_jobm_core(jobm_core_info.proc_id, &jobm_core_info.idx);
			if (ret != E_OK) { ret = -EFAULT ; goto exit; }

			if (unlikely(copy_to_user((void __user *)arg, &jobm_core_info, sizeof(VENDOR_AIS_UBUF_INFO)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		
		}
		break;
	case KFLOW_AI_IOC_SET_GBLK_CORE_MASK:
      {
		 VENDOR_AIS_GBlkTile_MASK mask_info = {0} ; 
         if (copy_from_user(&mask_info, (void __user *)arg, sizeof(VENDOR_AIS_GBlkTile_MASK))) {
            ret = -EFAULT;
            goto exit;
         }
		if (mask_info.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",mask_info.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
		 ret = kflow_gblktile_set_core_mask(&mask_info);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;   
  case KFLOW_AI_IOC_SET_GBLK_UB_MASK:
      {
		 VENDOR_AIS_GBlkTile_MASK mask_info = {0} ; 
         if (copy_from_user(&mask_info, (void __user *)arg, sizeof(VENDOR_AIS_GBlkTile_MASK))) {
            ret = -EFAULT;
            goto exit;
         }
		if (mask_info.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",mask_info.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
		 ret = kflow_gblktile_set_ub_mask(&mask_info);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;   
  case VENDOR_AIS_FLOW_IOC_GET_DTSI_INFO:
	{
		VENDOR_AIS_DTSI_INFO dtsi_info = {0};
		 if (copy_from_user(&dtsi_info, (void __user *)arg, sizeof(VENDOR_AIS_DTSI_INFO))) {
            ret = -EFAULT;
            goto exit;
         }

		ret = kflow_get_dtsi_info(&dtsi_info);

		ret = (copy_to_user((void __user *)arg, &dtsi_info, sizeof(VENDOR_AIS_DTSI_INFO))) ? (-EFAULT) : 0;

	}
	break;   
	case KFLOW_AI_IOC_GET_POSTPROC_DRV_INFO:
		{
			VENDOR_AIS_FLOW_POSTPROC_DRV_INFO drv_info = {0};
			if (copy_from_user(&drv_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_POSTPROC_DRV_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
			if (kflow_ai_net_postproc_drv_info == NULL) {
				DBG_ERR("kflow_ai_net_postproc_drv_info is NOT init yet ... !!\r\n");
				ret = 0;
				goto exit;
			}
			drv_info.drv_info_pa = (uintptr_t)nvt_ai_va2pa((uintptr_t)&kflow_ai_net_postproc_drv_info[drv_info.proc_id][0]); // start addr of (all core) for this proc_id

			if (unlikely(copy_to_user((void __user *)arg, &drv_info, sizeof(VENDOR_AIS_FLOW_POSTPROC_DRV_INFO)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_SET_SUB_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] SET_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			// DBG_DUMP("proc[%u] ADD_JOB [%u]\r\n", p_net->proc_id, job_info.job_id);
			p_job = kflow_ai_net_add_sub_job(p_net, job_info.job_id);
			if (p_job == NULL) {
				DBG_ERR("proc[%u] SET_SUB_JOB, invalid sub_job_id=%u\r\n", p_net->proc_id, job_info.job_id);
				ret = 0;
				goto exit;
			}
            //record llcmd addr for searching correct job index
            p_job->parm_addr = job_info.info;
			//kflow_ai_core_set_job(p_net, p_job, job_info.engine_id, job_info.engine_op, job_info.schd_parm, (void*)info, (void*)info2, wait_ms);
			//kflow_ai_core_set_job(p_net, p_job, (void*)&job_info.trig_info, (void*)info2, wait_ms);
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_OP == 1)
	case VENDOR_AIS_FLOW_IOC_SET_JOB2:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			INT32 wait_ms = job_info.wait_ms;
			//info = nvt_ai_user2user_va_in_kerl(job_info.info, job_info.proc_id);
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] SET_JOB2, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			p_job = kflow_ai_net_add_job(p_net, job_info.job_id);
			if (p_job == NULL) {
				DBG_ERR("proc[%u] SET_JOB2, invalid job_id=%u?\r\n", p_net->proc_id, job_info.job_id);
				ret = 0;
				goto exit;
			}
			p_job->engine_id = job_info.engine_id;
			p_job->engine_op = job_info.engine_op;
			p_job->schd_parm = job_info.schd_parm;
			p_job->wait_ms = job_info.wait_ms;
			if(p_job->engine_id == KFLOW_AI_ENGINE_PPU || p_job->engine_id == KFLOW_AI_ENGINE_POU){
				// if engine is ppu/pou, skip job setting
				goto exit;
			}else if(p_job->engine_id == KFLOW_AI_ENGINE_JMISP){ // pathll mode (for postproc)
                UINT32 core_id;
				uintptr_t info2 = 0;
				p_job->p_eng = kflow_ai_core_get_engine(KFLOW_AI_ENGINE_JOBM); //running this job by JOBM engine
				if (p_job->p_eng == 0) {
					DBG_ERR("proc[%u] SET_JOB, job[%u], jobm eng is invalid?\r\n", p_net->proc_id, job_info.job_id);
					ret = 0;
					goto exit;
				}
				if (kflow_ai_net_postproc_drv_info == NULL) {
					DBG_ERR("proc[%u] SET_JOB, job[%u], kflow_ai_net_postproc_drv_info is null?\r\n", p_net->proc_id, job_info.job_id);
					ret = 0;
					goto exit;
				}
                core_id = job_info.job_id;      // use job_id as core_id
				info2 = (uintptr_t)&kflow_ai_net_postproc_drv_info[job_info.proc_id][core_id];
				p_job->engine_op |= 0x00008000 ;  //running this job by JOBM engine, and mark it with 0x00008000 for JMISP mode
				kflow_ai_core_set_job(p_net, p_job, (void*)&job_info.trig_info, (void*)info2, wait_ms);
			}else if(((p_job->engine_op)>>16) == 4) { //jobll mode
                p_job->p_eng = kflow_ai_core_get_engine(KFLOW_AI_ENGINE_JOBM);
                if (p_job->p_eng == 0) {
                    DBG_ERR("proc[%u] SET_JOB, job[%u], jobm eng is invalid?\r\n", p_net->proc_id, job_info.job_id);
                    ret = 0;
                    goto exit;
                }
				kflow_ai_core_set_job(p_net, p_job, (void*)&job_info.trig_info, 0, wait_ms);
            } else {
                p_job->p_eng = kflow_ai_core_get_engine(p_job->engine_id); //assign this job's engine!
                if (p_job->p_eng == 0) {
                    DBG_ERR("proc[%u] SET_JOB, job[%u], invalid engine_id=%u?\r\n", p_net->proc_id, job_info.job_id, p_job->engine_id);
                    ret = 0;
                    goto exit;
                }
				kflow_ai_core_set_job(p_net, p_job, (void*)&job_info.trig_info, 0, wait_ms);
            }
			// kflow_ai_core_set_job(p_net, p_job, job_info.engine_id, job_info.engine_op, (void*)info, (void*)info2, wait_ms);
			// kflow_ai_core_set_job(p_net, p_job, (void*)&job_info.trig_info, 0, wait_ms);
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_BIND_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] BIND_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			//DBG_DUMP("proc[%u] BIND_JOB [%u~%u]\r\n", p_net->proc_id, job_info.job_id, job_info.info);
			p_job = kflow_ai_net_job(p_net, job_info.job_id);
			if (p_job == NULL) {
				DBG_ERR("proc[%u] BIND_JOB, invalid job_id=%u?\r\n", p_net->proc_id, job_info.job_id);
				ret = 0;
				goto exit;
			}
			p_next_job = kflow_ai_net_job(p_net, job_info.info);
			if (p_next_job == NULL) {
				DBG_ERR("proc[%u] BIND_JOB, set end job_id=%u\r\n", p_net->proc_id, job_info.job_id);
			}
			kflow_ai_core_bind_job(p_net, p_job, p_next_job);
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_UNBIND_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] UNBIND_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			kflow_ai_core_sum_job(p_net, &job_info.info, &job_info.info2);
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOBLIST_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_LOCK_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] UOCK_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			p_job = kflow_ai_net_job(p_net, job_info.job_id);
			if (p_job == 0) {
				job_info.rv = 0;
				job_info.wait_ms = -1;
			} else {
				job_info.rv = kflow_ai_core_lock_job(p_net, p_job);
				job_info.wait_ms = 0;
			}
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_UNLOCK_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] UNLOCK_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			p_job = kflow_ai_net_job(p_net, job_info.job_id);
			if (p_job == 0) {
				job_info.rv = 0;
				job_info.wait_ms = -1;
			} else {
				job_info.rv = kflow_ai_core_unlock_job(p_net, p_job);
				if (job_info.rv == -2) {
					DBG_ERR("proc[%u] UNLOCK_JOB abort.\r\n", p_net->proc_id);
				}
				job_info.wait_ms = 0;
			}
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_PUSH_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] PUSH_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			if (job_info.job_id >= 0xf0000000) {
				if (job_info.job_id == 0xf0000001) {
					//BEGIN
					kflow_ai_core_push_begin(p_net);
				} else if (job_info.job_id == 0xf0000002) {
					//END
					kflow_ai_core_push_end(p_net);
				}
			} else {
				p_job = kflow_ai_net_job(p_net, job_info.job_id);
				if (p_job == 0) {
					job_info.wait_ms = -1;
				} else {
					kflow_ai_core_push_job(p_net, p_job, job_info.schd_parm, job_info.job_priority);
					job_info.wait_ms = 0;
				}
			}
		}
		ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_PULL_JOB:
		if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		{
			p_net = kflow_ai_core_net(job_info.proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] PULL_JOB, invalid p_net?\r\n", job_info.proc_id);
				ret = 0;
				goto exit;
			}
			job_info.rv = 0;
			if (job_info.job_id >= 0xf0000000) {
				if (job_info.job_id == 0xf0000001) {
					//BEGIN
					kflow_ai_core_pull_begin(p_net);
				} else if (job_info.job_id == 0xf0000002) {
					//END
					kflow_ai_core_pull_end(p_net);
				} else if (job_info.job_id == 0xf0000003) {
					//ready
					p_job = 0;
					kflow_ai_core_pull_ready(p_net, p_job);
				} else if (job_info.job_id == 0xf0000004) {
					KFLOW_AI_JOB* pull_job;
					//ready
					pull_job = 0;
					job_info.rv = kflow_ai_core_pull_job(p_net, &pull_job);
					if (job_info.rv == 0) {
						job_info.wait_ms = 0;
						job_info.job_id = pull_job->job_id;
						job_info.info = 0; // 0 => HD_ERR_EOL
						//DBG_ERR("proc[%u] PULL_JOB ok.\r\n", p_net->proc_id);
					} else if (job_info.rv > 0) {
						job_info.wait_ms = 0;
						job_info.job_id = pull_job->job_id;
						job_info.info = 1;  // 1 => HD_OK
						//DBG_ERR("proc[%u] PULL_JOB ok-2.\r\n", p_net->proc_id);
					} else if (job_info.rv == -2) {
						job_info.wait_ms = 0;
						ret = -ERESTARTSYS; //if pull_job() is NOT interruptted by a ctrl-c signal, this IOCTL will retry again.
						DBG_WRN("proc[%u] PULL_JOB get signal.\r\n", p_net->proc_id);
					} else if (job_info.rv == -3) {
						job_info.wait_ms = 0;
						if (p_net->rv == -1) {
							job_info.wait_ms = 0;
							job_info.job_id = pull_job->job_id;
							job_info.info = (UINT32)-1;  // -1 => HD_ERR_FAIL
							DBG_ERR("proc[%u] PULL_JOB fail!\r\n", p_net->proc_id);
						}
						if (p_net->rv == -2) {
							job_info.wait_ms = 0;
							job_info.job_id = pull_job->job_id;
							job_info.info = (UINT32)-2;  // -2 => HD_ERR_TIMEDOUT					
							DBG_ERR("proc[%u] PULL_JOB timeout!\r\n", p_net->proc_id);
						}
					} else {    // if (job_info.rv < 0)
						//DBG_DUMP("->>> proc[%u] - IOCTL cancel\r\n", job_info.proc_id);
						job_info.wait_ms = -1;
						DBG_ERR("proc[%u] PULL_JOB error!!!\r\n", p_net->proc_id);
					}
				}
			}
		}
		//ret = (copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) ? (-EFAULT) : 0;
		if (unlikely(copy_to_user((void __user *)arg, &job_info, sizeof(VENDOR_AIS_FLOW_JOB_INFO)))) {
			return -EFAULT;
		}
		break;
#endif
	case VENDOR_AIS_FLOW_IOC_SET_USAGE_LIMIT:
		{
			UINT32 usage_limit = 0 ; 
			if (copy_from_user(&usage_limit, (void __user *)arg, sizeof(UINT32))) {
				ret = -EFAULT;
				goto exit;
			}
			kflow_ai_core_set_usage_limit(usage_limit) ; 
		}
		break;
#if (FLOW_AI_DEV == 1)
/*		
	case VENDOR_AIS_FLOW_IOC_WAI_JOB: {
			VENDOR_AIS_FLOW_JOB_WAI cmd = {0};
			VENDOR_AIS_FLOW_JOB_WAI *p_cmd  = 0;
			p_cmd  = &cmd;
			if (unlikely(copy_from_user(p_cmd, argp, sizeof(VENDOR_AIS_FLOW_JOB_WAI)))) {
				//DBG_ERR("copy_from_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)p_cmd, (UINT32)argp, sizeof(VENDOR_AIS_FLOW_JOB_WAI));
				return -EFAULT;
			}
			//DBG_IND("ioctl OUT_WAI: open\r\n");
			p_net = kflow_ai_core_net(p_cmd->proc_id);
			p_job = kflow_ai_core_cb_wait(p_cmd->proc_id);
			if (p_job == NULL) {
				p_cmd->job_id = 0xffffffff;
			} else {
				p_cmd->job_id = kflow_ai_net_job_id(p_net, p_job);
				if (p_cmd->job_id == 0xffffffff) {
					DBG_ERR("proc[%u] WAI_JOB, invalid job_id?\r\n", p_net->proc_id);
				}
			}
			if (unlikely(copy_to_user(argp, p_cmd, sizeof(VENDOR_AIS_FLOW_JOB_WAI)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_JOB_WAI));
				return -EFAULT;
			}
		}
		break;

	case VENDOR_AIS_FLOW_IOC_SIG_JOB: {
			VENDOR_AIS_FLOW_JOB_SIG cmd = {0};
			VENDOR_AIS_FLOW_JOB_SIG *p_cmd  = 0;
			p_cmd  = &cmd;
			if (unlikely(copy_from_user(p_cmd, argp, sizeof(VENDOR_AIS_FLOW_JOB_SIG)))) {
				//DBG_ERR("copy_from_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)p_cmd, (UINT32)argp, sizeof(VENDOR_AIS_FLOW_JOB_SIG));
				return -EFAULT;
			}
			p_net = kflow_ai_core_net(p_cmd->proc_id);
			//DBG_IND("ioctl OUT_LOG: close\r\n");
			p_job = kflow_ai_net_job(p_net, p_cmd->job_id);
			if (p_job == 0)
			    p_job = (KFLOW_AI_JOB*)0xffffffff;
			kflow_ai_core_cb_sig(p_cmd->proc_id, p_job);
		}
		break;
*/
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_WAI_CPU: {
			VENDOR_AIS_FLOW_CPU_WAI cmd = {0};
			VENDOR_AIS_FLOW_CPU_WAI *p_cmd  = 0;
#if NN_DLI
			UINT32 event = 0;
#endif
			p_cmd  = &cmd;
			if (unlikely(copy_from_user(p_cmd, argp, sizeof(VENDOR_AIS_FLOW_CPU_WAI)))) {
				//DBG_ERR("copy_from_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)p_cmd, (UINT32)argp, sizeof(VENDOR_AIS_FLOW_CPU_WAI));
				return -EFAULT;
			}
			//DBG_IND("ioctl OUT_WAI: open\r\n");
			p_net = kflow_ai_core_net(p_cmd->proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] WAI_CPU, invalid p_net?\r\n", p_cmd->proc_id);
				ret = 0;
				goto exit;
			}
#if NN_DLI
			p_job = kflow_ai_cpu_wait(p_cmd->proc_id, &event);
#else
			p_job = kflow_ai_cpu_wait(p_cmd->proc_id);
#endif

			if (p_job == (KFLOW_AI_JOB*)0x0C) { //interrupt by system (include ctrl-c or other signals)
				p_cmd->job_id = 0xffffffff;
				ret = -ERESTARTSYS; //if cpu_wait() is NOT interruptted by a ctrl-c signal, this IOCTL will retry again.
				//DBG_WRN("proc[%u] WAI_CPU get signal.\r\n", p_cmd->proc_id);
			} else if (p_job == NULL) {  //stopped by user (normal case)
				p_cmd->job_id = 0xffffffff;
			} else {
				p_cmd->job_id = kflow_ai_net_job_id(p_net, p_job);
				if (p_cmd->job_id == 0xffffffff) {
					DBG_ERR("proc[%u] WAI_CPU, invalid job_id?\r\n", p_net->proc_id);
				}
#if NN_DLI
				if (event == 0xff) {
					p_cmd->job_id |= 0xff000000; //start
				}
				if (event == 0xfe) {
					p_cmd->job_id |= 0xfe000000; //stop
				}
				//event == 0  //proc
#endif
			}
			if (unlikely(copy_to_user(argp, p_cmd, sizeof(VENDOR_AIS_FLOW_CPU_WAI)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_CPU_WAI));
				return -EFAULT;
			}
		}
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_SIG_CPU: {
			VENDOR_AIS_FLOW_CPU_SIG cmd = {0};
			VENDOR_AIS_FLOW_CPU_SIG *p_cmd  = 0;
			p_cmd  = &cmd;
			if (unlikely(copy_from_user(p_cmd, argp, sizeof(VENDOR_AIS_FLOW_CPU_SIG)))) {
				//DBG_ERR("copy_from_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)p_cmd, (UINT32)argp, sizeof(VENDOR_AIS_FLOW_CPU_SIG));
				return -EFAULT;
			}
			p_net = kflow_ai_core_net(p_cmd->proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] SIG_CPU, invalid p_net?\r\n", p_cmd->proc_id);
				ret = 0;
				goto exit;
			}
			//DBG_IND("ioctl OUT_LOG: close\r\n");
#if NN_DLI
            p_cmd->job_id &= ~0xff000000; //start
            p_cmd->job_id &= ~0xfe000000; //stop
#endif
			p_job = kflow_ai_net_job(p_net, p_cmd->job_id);
			if (p_job == NULL) {
				DBG_ERR("proc[%u] SIG_CPU, invalid job_id=%lu?\r\n", p_net->proc_id, p_cmd->job_id);
				ret = 0;
				goto exit;
			}
			kflow_ai_cpu_sig(p_cmd->proc_id, p_job);
		}
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_WAI_DSP: {
			VENDOR_AIS_FLOW_DSP_WAI cmd = {0};
			VENDOR_AIS_FLOW_DSP_WAI *p_cmd  = 0;
#if NN_DLI && (AI_SUPPORT_DSP == 1)
			UINT32 event = 0;
#endif
			p_cmd  = &cmd;
			if (unlikely(copy_from_user(p_cmd, argp, sizeof(VENDOR_AIS_FLOW_DSP_WAI)))) {
				//DBG_ERR("copy_from_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)p_cmd, (UINT32)argp, sizeof(VENDOR_AIS_FLOW_DSP_WAI));
				return -EFAULT;
			}
			//DBG_IND("ioctl OUT_WAI: open\r\n");
#if (AI_SUPPORT_DSP == 1)
			p_net = kflow_ai_core_net(p_cmd->proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] WAI_DSP, invalid p_net?\r\n", p_cmd->proc_id);
				ret = 0;
				goto exit;
			}
#if NN_DLI
			p_job = kflow_ai_dsp_wait(p_cmd->proc_id, &event);
#else
			p_job = kflow_ai_dsp_wait(p_cmd->proc_id);
#endif

			if (p_job == (KFLOW_AI_JOB*)0x0C) { //interrupt by system (include ctrl-c or other signals)
				p_cmd->job_id = 0xffffffff;
				ret = -ERESTARTSYS; //if dsp_wait() is NOT interruptted by a ctrl-c signal, this IOCTL will retry again.
				//DBG_WRN("proc[%u] WAI_DSP get signal.\r\n", p_cmd->proc_id);
			} else if (p_job == NULL) {  //stopped by user (normal case)
				p_cmd->job_id = 0xffffffff;
			} else {
				p_cmd->job_id = kflow_ai_net_job_id(p_net, p_job);
				if (p_cmd->job_id == 0xffffffff) {
					DBG_ERR("proc[%u] WAI_DSP, invalid job_id?\r\n", p_net->proc_id);
				}
#if NN_DLI
				if (event == 0xff) {
					p_cmd->job_id |= 0xff000000; //start
				}
				if (event == 0xfe) {
					p_cmd->job_id |= 0xfe000000; //stop
				}
				//event == 0  //proc
#endif
			}
#else
            //not support dsp flow
            p_cmd->job_id = 0xffffffff; 
#endif
			if (unlikely(copy_to_user(argp, p_cmd, sizeof(VENDOR_AIS_FLOW_DSP_WAI)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_CPU_WAI));
				return -EFAULT;
			}
		}
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case VENDOR_AIS_FLOW_IOC_SIG_DSP: {
			VENDOR_AIS_FLOW_DSP_SIG cmd = {0};
			VENDOR_AIS_FLOW_DSP_SIG *p_cmd  = 0;
			p_cmd  = &cmd;
			if (unlikely(copy_from_user(p_cmd, argp, sizeof(VENDOR_AIS_FLOW_DSP_SIG)))) {
				//DBG_ERR("copy_from_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)p_cmd, (UINT32)argp, sizeof(VENDOR_AIS_FLOW_DSP_SIG));
				return -EFAULT;
			}
#if (AI_SUPPORT_DSP == 1)
			p_net = kflow_ai_core_net(p_cmd->proc_id);
			if (p_net == NULL) {
				DBG_ERR("proc[%u] SIG_DSP, invalid p_net?\r\n", p_cmd->proc_id);
				ret = 0;
				goto exit;
			}
#if NN_DLI
            p_cmd->job_id &= ~0xff000000; //start
            p_cmd->job_id &= ~0xfe000000; //stop
#endif
			//DBG_IND("ioctl OUT_LOG: close\r\n");
			p_job = kflow_ai_net_job(p_net, p_cmd->job_id);
			if (p_job == NULL) {
				DBG_ERR("proc[%u] SIG_DSP, invalid job_id=%lu?\r\n", p_net->proc_id, p_cmd->job_id);
				ret = 0;
				goto exit;
			}
			kflow_ai_dsp_sig(p_cmd->proc_id, p_job);
#endif
		}
		break;
#endif

/*
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_VERS:
		{
			VENDOR_AIS_FLOW_VERS vers_info = {0};

			if (copy_from_user(&vers_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_VERS))) {
				ret = -EFAULT;
				goto exit;
			}

			vers_info.rv = nvt_ai_chk_vers(&vers_info);

			if (unlikely(copy_to_user((void __user *)arg, &vers_info, sizeof(VENDOR_AIS_FLOW_VERS)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
*/
#if (FLOW_AI_PROCID == 1)
	case VENDOR_AIS_FLOW_IOC_GET_ID:
		{
			UINT32 proc_id = 0;
			if (copy_from_user(&proc_id, (void __user *)arg, sizeof(UINT32))) {
				ret = -EFAULT;
				goto exit;
			}
			nvt_ai_get_id(&proc_id);
	
			if (unlikely(copy_to_user((void __user *)arg, &proc_id, sizeof(UINT32)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
#if (FLOW_AI_PROCID == 1)
	case VENDOR_AIS_FLOW_IOC_RELEASE_ID:
		{
			UINT32 proc_id = 0;
			if (copy_from_user(&proc_id, (void __user *)arg, sizeof(UINT32))) {
				ret = -EFAULT;
				goto exit;
			}

			nvt_ai_release_id(proc_id);

			if (unlikely(copy_to_user((void __user *)arg, &proc_id, sizeof(UINT32)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_OPEN_PATH:
		{
			UINT32 proc_id = 0;
			if (copy_from_user(&proc_id, (void __user *)arg, sizeof(UINT32))) {
				ret = -EFAULT;
				goto exit;
			}
			SEM_WAIT(g_kflow_net_sem_id);
#if (FLOW_AI_PROCID == 1)
			if(g_ai_net_path_need_reset[proc_id] == 1) {
				nvt_ai_reset_net_path(proc_id);
				kflow_ai_core_reset_path(proc_id);
				kflow_ai_cpu_uninit_cb_path(proc_id);
#if (AI_SUPPORT_DSP == 1)
				kflow_ai_dsp_uninit_cb_path(proc_id);
#endif
				g_ai_net_path_need_reset[proc_id] = 0;
			}
#endif
#if (FLOW_AI_PROCID == 1)
 			g_ai_net_path_open[proc_id] = 1;
#endif
			SEM_SIGNAL(g_kflow_net_sem_id);
			if (unlikely(copy_to_user((void __user *)arg, &proc_id, sizeof(UINT32)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_CLOSE_PATH:
		{
			UINT32 proc_id = 0;
			if (copy_from_user(&proc_id, (void __user *)arg, sizeof(UINT32))) {
				ret = -EFAULT;
				goto exit;
			}
			SEM_WAIT(g_kflow_net_sem_id);
#if (FLOW_AI_PROCID == 1)
			g_ai_net_path_open[proc_id] = 0;
#endif
			SEM_SIGNAL(g_kflow_net_sem_id);
			if (unlikely(copy_to_user((void __user *)arg, &proc_id, sizeof(UINT32)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
    #if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_MULTI_PROCESS:
		{
			BOOL is_multi_process = 0;
			if (copy_from_user(&is_multi_process, (void __user *)arg, sizeof(BOOL))) {
				ret = -EFAULT;
				goto exit;
			}
			if (unlikely(copy_to_user((void __user *)arg, &is_multi_process, sizeof(BOOL)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
#if (FLOW_AI_DYSCALE == 1)
	case VENDOR_AIS_FLOW_IOC_PROC_UPDATE_INFO:
		{
			VENDOR_AIS_FLOW_UPDATE_NET_INFO up_net_info = {0};
			//uintptr_t kerl_va = 0;

			if (copy_from_user(&up_net_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_UPDATE_NET_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
            kflow_ai_core_fix_job_parm_addr(up_net_info.net_id, up_net_info.job_id, up_net_info.parm_addr);

			break;
		}
    case VENDOR_AIS_FLOW_IOC_PROC_UPDATE_TRIG:
		{
			VENDOR_AIS_FLOW_JOB_INFO job_info = {0};
			//uintptr_t kerl_va = 0;

			if (copy_from_user(&job_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_JOB_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
            kflow_ai_core_fix_tot_trig_eng_times(job_info.proc_id, job_info.job_id, job_info.trig_info.tot_trig_eng_times);

			break;
		}
    case VENDOR_AIS_FLOW_IOC_PROC_UPDATE_OUT:
		{
			VENDOR_AIS_FLOW_REAL_JOB_OUT_INFO out_info = {0};
			//uintptr_t kerl_va = 0;

			if (copy_from_user(&out_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_REAL_JOB_OUT_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
            kflow_ai_core_fix_out_info(out_info.proc_id, out_info.job_id, out_info.real_sao);

			break;
		}
    case VENDOR_AIS_FLOW_IOC_PROC_UPDATE_SUB:
		{
			VENDOR_AIS_FLOW_UPDATE_NET_INFO up_net_info = {0};
			//uintptr_t kerl_va = 0;

			if (copy_from_user(&up_net_info, (void __user *)arg, sizeof(VENDOR_AIS_FLOW_UPDATE_NET_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
            kflow_ai_core_fix_sub_job_parm_addr(up_net_info.net_id, up_net_info.job_id, up_net_info.parm_addr);

			break;
		}
#endif
#if (FLOW_AI_DEV == 1)
	case KFLOW_AI_IOC_CMD_DMA_ABORT:
		{
			break;
		}
#endif
#if (FLOW_AI_DEBUG == 1)
	case KFLOW_AI_IOC_CMD_OUT_INIT: 
		{
            SEM_WAIT(g_kflow_net_sem_id);
			kflow_cmd_out_init();
			kflow_msg_out_init();
            SEM_SIGNAL(g_kflow_net_sem_id);
			break;
		}
#endif
#if (FLOW_AI_DEBUG == 1)
	case KFLOW_AI_IOC_CMD_OUT_UNINIT: 
		{
            SEM_WAIT(g_kflow_net_sem_id);
			kflow_cmd_out_uninit();
			kflow_msg_out_uninit();
            SEM_SIGNAL(g_kflow_net_sem_id);
		}
		break;
#endif
#if (FLOW_AI_DEBUG == 1)
	case KFLOW_AI_IOC_CMD_OUT_PROG_DEBUG:
		{
			kflow_cmd_out_prog_debug();
		}
		break;
#endif
#if (FLOW_AI_DEBUG == 1)
	case KFLOW_AI_IOC_CMD_OUT_RUN_DEBUG:
		{
			kflow_cmd_out_run_debug();
		}
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case KFLOW_AI_IOC_CMD_PERF_UT_BEGIN:
		{
			//DBG_DUMP("=> api: perf ut begin:\r\n");

			kflow_ai_core_add_jobm_cycle(); // this will reset kdrv jobm count = 0 , and will update (conv/rou/util/cal/lsu) lts_exec  (but we don't want those values ... it's OK , next line will reset it)
			kflow_ai_perf(3, 1);            // this will call kflow_ai_core_reset_perf_ut() to reset lts_exec = 0
		}
		break;
#endif
#if (FLOW_AI_JOB == 1)
	case KFLOW_AI_IOC_CMD_PERF_UT_END:
		{
			KFLOW_AI_PERF_UT p_cmd = {0};
			
			kflow_ai_perf(0, 1);

			kflow_ai_core_add_jobm_cycle(); // this will get kdrv jobm count to update (conv/rou/util/cal/lsu) lts_exec
			kflow_ai_core_get_perf_ut(&p_cmd);
			if (unlikely(copy_to_user((void __user *)arg, &p_cmd, sizeof(KFLOW_AI_PERF_UT)))) {
				return -EFAULT;
			}
			//DBG_DUMP("=> api: perf ut end:\r\n");
		}
		break;
#endif
	case KFLOW_AI_IOC_CMD_PERF_TIMELINE_BEGIN:
		{
			kflow_ai_core_perf_timeline_begin();
		}
		break;
	case KFLOW_AI_IOC_CMD_PERF_TIMELINE_END:
		{
			kflow_ai_core_perf_timeline_end();
		}
		break;
    case KFLOW_AI_IOC_CMD_SET_TIMELINE_JOB_INFO:
		{
			VENDOR_AIS_FLOW_TIMELINE_JOB_INFO p_info = {0};

			if (unlikely(copy_from_user((void *)&p_info, (void *)arg, sizeof(VENDOR_AIS_FLOW_TIMELINE_JOB_INFO)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_info, arg, sizeof(VENDOR_AIS_FLOW_TIMELINE_JOB_INFO));
				return -EFAULT;
			}
			kflow_ai_core_set_user_timeline_job(p_info.proc_id, p_info.class, p_info.param, p_info.ts_start, p_info.ts_end);
		}
		break;
    case KFLOW_AI_IOC_CMD_SET_TIMELINE_INFO:
		{
			VENDOR_AIS_FLOW_TIMELINE_INFO p_info = {0};
			if (unlikely(copy_from_user((void *)&p_info, (void *)arg, sizeof(VENDOR_AIS_FLOW_TIMELINE_INFO)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_info, arg, sizeof(VENDOR_AIS_FLOW_TIMELINE_INFO));
				return -EFAULT;
			}
            if(p_info.timeline_func) {
                if(p_info.timeline_func <= 0xf) {
                    kflow_ai_net_set_timeline_func_mask(p_info.timeline_func);
                } else {
                    DBG_ERR("timeline_func = 0x%x is not support...\r\n", p_info.timeline_func);
                    return -EFAULT;
                }
            } 
            if(p_info.timeline_buf_size) {
                kflow_ai_core_set_timeline_job_num(p_info.timeline_buf_size);
            }
    	}
		break;
#if (FLOW_AI_DEBUG == 1)
	case KFLOW_AI_IOC_CMD_OUT_WAI:
		{
			KFLOW_AI_IOC_CMD_OUT p_cmd = {0};

			if (unlikely(copy_from_user((void *)&p_cmd, (void *)arg, sizeof(KFLOW_AI_IOC_CMD_OUT)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CMD_OUT));
				return -EFAULT;
			}
			p_cmd.proc_id = kflow_ai_cmd_out_wait(&p_cmd);
			if (unlikely(copy_to_user((void *)arg, &p_cmd, sizeof(KFLOW_AI_IOC_CMD_OUT)))) {
				DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CMD_OUT));
				return -EFAULT;
			}
		}
		break;

	case KFLOW_AI_IOC_CMD_OUT_SIG:
		{
			KFLOW_AI_IOC_CMD_OUT p_cmd = {0};

			if (unlikely(copy_from_user((void *)&p_cmd, (void *)arg, sizeof(KFLOW_AI_IOC_CMD_OUT)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CMD_OUT));
				return -EFAULT;
			}
			kflow_ai_cmd_out_sig(p_cmd.proc_id, &p_cmd);
			if (unlikely(copy_to_user((void *)arg, &p_cmd, sizeof(KFLOW_AI_IOC_CMD_OUT)))) {
				DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CMD_OUT));
				return -EFAULT;
			}
		}
		break;

	case KFLOW_AI_IOC_MSG_OUT_WAI:
		{
			KFLOW_AI_IOC_CMD_OUT p_cmd = {0};

			if (unlikely(copy_from_user((void *)&p_cmd, (void *)arg, sizeof(KFLOW_AI_IOC_CMD_OUT)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CMD_OUT));
				return -EFAULT;
			}
			p_cmd.proc_id = kflow_ai_msg_out_wait(&p_cmd);
			if (unlikely(copy_to_user((void *)arg, &p_cmd, sizeof(KFLOW_AI_IOC_CMD_OUT)))) {
				DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CMD_OUT));
				return -EFAULT;
			}
		}
		break;

	case KFLOW_AI_IOC_MSG_OUT_SIG:
		{
			KFLOW_AI_IOC_CMD_OUT p_cmd = {0};

			if (unlikely(copy_from_user((void *)&p_cmd, (void *)arg, sizeof(KFLOW_AI_IOC_CMD_OUT)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CMD_OUT));
				return -EFAULT;
			}
			kflow_ai_msg_out_sig(p_cmd.proc_id, &p_cmd);
			if (unlikely(copy_to_user((void *)arg, &p_cmd, sizeof(KFLOW_AI_IOC_CMD_OUT)))) {
				DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CMD_OUT));
				return -EFAULT;
			}
		}
		break;

#endif
#if (FLOW_AI_DEBUG == 1)
	case KFLOW_AI_IOC_SET_VERSION:
		{
			KFLOW_AI_IOC_VERSION p_cmd = {0};

			if (unlikely(copy_from_user((void *)&p_cmd, (void *)arg, sizeof(KFLOW_AI_IOC_VERSION)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_VERSION));
				return -EFAULT;
			}
			ret = kflow_ai_set_lib_version(&p_cmd);
		}
		break;
    case KFLOW_AI_IOC_CMD_CHECK_IS_INIT: 
		{
            UINT32 is_init = 0;
            SEM_WAIT(g_kflow_net_sem_id);
            is_init = kflow_cmd_is_init();
            SEM_SIGNAL(g_kflow_net_sem_id);
			if (unlikely(copy_to_user((void *)arg, &is_init, sizeof(UINT32)))) {
				DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &is_init, arg, sizeof(UINT32));
				return -EFAULT;
			}
			break;
		}
#endif
#if (FLOW_AI_DEV == 1)
	case VENDOR_AIS_FLOW_IOC_GET_VER:
		{
			CHAR version_info[32] = KFLOW_AI_IMPL_VERSION;
			ret = copy_to_user((void __user *)arg, version_info, sizeof(KFLOW_AI_IMPL_VERSION));
		}
		break;
#endif
#if (FLOW_AI_DEV == 1)
	case KFLOW_AI_IOC_CACHE_CHECK:
		{
			uintptr_t p_cmd = {0};

			if (unlikely(copy_from_user((void *)&p_cmd, (void *)arg, sizeof(uintptr_t)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_VERSION));
				return -EFAULT;
			}
			ret = nvt_ai_user_va_cacheable(p_cmd);
		}
		break;
#endif

#if (FLOW_AI_DEBUG == 1)
	case KFLOW_AI_IOC_GET_DEBUG_LVL:
		{
			KFLOW_AI_IOC_DEBUG_LVL p_cmd = {0};
			ret = kflow_ai_get_debug_lvl(&p_cmd);
			if (unlikely(copy_to_user((void __user *)arg, &p_cmd, sizeof(KFLOW_AI_IOC_DEBUG_LVL)))) {
				ret = -EFAULT;
			}
		}
		break;
#endif
#if (FLOW_AI_DEBUG == 1)
	case KFLOW_AI_IOC_GET_DEBUG_INFO:
		{
			KFLOW_AI_IOC_DEBUG_INFO p_cmd = {0};
			ret = kflow_ai_get_debug_info(&p_cmd);
			if (unlikely(copy_to_user((void __user *)arg, &p_cmd, sizeof(KFLOW_AI_IOC_DEBUG_INFO)))) {
				ret = -EFAULT;
			}
		}
		break;
#endif
#if (FLOW_AI_DEBUG == 1)
	case VENDOR_AIS_FLOW_IOC_SET_DEBUG:
		{
			VENDOR_AIS_SET_DEBUG_INFO debug_info = {0};
			if (copy_from_user(&debug_info, (void __user *)arg, sizeof(VENDOR_AIS_SET_DEBUG_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
            SEM_WAIT(g_kflow_net_sem_id);
            if(debug_info.proc_id == 0xffffffff) {
                kflow_ai_net_set_debug_all(debug_info.debug_item, 0); // is_extend = 0, absolute value
            } else {
			    kflow_ai_net_set_debug(debug_info.proc_id, debug_info.debug_item, 0); // is_extend = 0, absolute value
            }
            SEM_SIGNAL(g_kflow_net_sem_id);
        }
		break;
#endif
#if (FLOW_AI_DEBUG == 1)
	case VENDOR_AIS_FLOW_IOC_GET_DEBUG:
		{
			VENDOR_AIS_SET_DEBUG_INFO debug_info = {0};

			// get debug_info.proc_id from user
			if (copy_from_user(&debug_info, (void __user *)arg, sizeof(VENDOR_AIS_SET_DEBUG_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
            SEM_WAIT(g_kflow_net_sem_id);
            if(debug_info.proc_id == 0xffffffff) {
                kflow_ai_net_get_debug_all(&debug_info.debug_item);
            } else {
                kflow_ai_net_get_debug(debug_info.proc_id, &debug_info.debug_item);
            }
            SEM_SIGNAL(g_kflow_net_sem_id);

			// return debug_info.debug_item to user
			if (unlikely(copy_to_user((void __user *)arg, &debug_info, sizeof(VENDOR_AIS_SET_DEBUG_INFO)))) {
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif

#if (FLOW_AI_UBUFID == 1)
	case VENDOR_AIS_FLOW_IOC_GET_UBUF_NORMAL:
		{
			VENDOR_AIS_UBUF_INFO ubuf_info = {0};

			if (copy_from_user(&ubuf_info, (void __user *)arg, sizeof(VENDOR_AIS_UBUF_INFO))) {
				ret = -EFAULT;
				goto exit;
			}

			ret = nvt_ai_normal_get_ubuf(ubuf_info.proc_id, &ubuf_info.idx, ubuf_info.size_in_mb);
			if (ret != E_OK) { ret = -EFAULT ; goto exit; }

			if (unlikely(copy_to_user((void __user *)arg, &ubuf_info, sizeof(VENDOR_AIS_UBUF_INFO)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
#if (FLOW_AI_UBUFID == 1)
	case VENDOR_AIS_FLOW_IOC_FREE_UBUF_NORMAL:
		{
			VENDOR_AIS_UBUF_INFO ubuf_info = {0};

			if (copy_from_user(&ubuf_info, (void __user *)arg, sizeof(VENDOR_AIS_UBUF_INFO))) {
				ret = -EFAULT;
				goto exit;
			}
			ret = nvt_ai_normal_free_ubuf(ubuf_info.proc_id, ubuf_info.idx, ubuf_info.size_in_mb);
			if (ret != E_OK) { ret = -EFAULT ; goto exit; }
		}
		break;
#endif
#if (FLOW_AI_DEBUG == 1)
	case VENDOR_AIS_GET_OBUF_PATH:
		{
			kflow_ai_get_obuf_path(dbg_dump_outbuf_path);
			if (unlikely(copy_to_user((void __user *)arg, &dbg_dump_outbuf_path, sizeof(dbg_dump_outbuf_path)))) {
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
#endif
	case VENDOR_AIS_SW_RESET:
		{
			SEM_WAIT(g_kflow_net_sem_id);
			kflow_ai_sw_reset();
			SEM_SIGNAL(g_kflow_net_sem_id);
		}
		break;
    case VENDOR_AIS_GET_MAX_ISP:
		{
			UINT32 max_isp = 0;
			if (copy_from_user(&max_isp, (void __user *)arg, sizeof(UINT32))) {
				ret = -EFAULT;
				goto exit;
			}
			max_isp = MAX_ISP;
	
			if (unlikely(copy_to_user((void __user *)arg, &max_isp, sizeof(UINT32)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
		
#if (FLOW_AI_DEBUG == 1)
    case KFLOW_AI_IOC_CAT_CHECK_IS_INIT: 
		{
            UINT32 is_init = 0;
            SEM_WAIT(g_kflow_net_sem_id);
			// DBG_DUMP("is_init: %u\n", is_init);
            is_init = kflow_cat_is_init();
			// DBG_DUMP("is_init: %u\n", is_init);
            SEM_SIGNAL(g_kflow_net_sem_id);
			if (unlikely(copy_to_user((void *)arg, &is_init, sizeof(UINT32)))) {
				DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &is_init, arg, sizeof(UINT32));
				return -EFAULT;
			}
			break;
		}

	case KFLOW_AI_IOC_CAT_OUT_INIT: 
		{
            SEM_WAIT(g_kflow_net_sem_id);
			kflow_cat_out_init();
            SEM_SIGNAL(g_kflow_net_sem_id);
			break;
		}

	case KFLOW_AI_IOC_CAT_OUT_UNINIT: 
		{
            SEM_WAIT(g_kflow_net_sem_id);
			kflow_cat_out_uninit();
            SEM_SIGNAL(g_kflow_net_sem_id);
		}
		break;

	case KFLOW_AI_IOC_CAT_OUT_WAI:
		{
			KFLOW_AI_IOC_CAT_OUT p_cmd = {0};

			if (unlikely(copy_from_user((void *)&p_cmd, (void *)arg, sizeof(KFLOW_AI_IOC_CAT_OUT)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CAT_OUT));
				return -EFAULT;
			}
			p_cmd.proc_id = kflow_ai_cat_out_wait(&p_cmd);
			if (unlikely(copy_to_user((void *)arg, &p_cmd, sizeof(KFLOW_AI_IOC_CAT_OUT)))) {
				DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CAT_OUT));
				return -EFAULT;
			}
		}
		break;

	case KFLOW_AI_IOC_CAT_OUT_SIG:
		{
			KFLOW_AI_IOC_CAT_OUT p_cmd = {0};

			if (unlikely(copy_from_user((void *)&p_cmd, (void *)arg, sizeof(KFLOW_AI_IOC_CAT_OUT)))) {
				DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CAT_OUT));
				return -EFAULT;
			}
			kflow_ai_cat_out_sig(p_cmd.proc_id, &p_cmd);
			if (unlikely(copy_to_user((void *)arg, &p_cmd, sizeof(KFLOW_AI_IOC_CAT_OUT)))) {
				DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &p_cmd, arg, sizeof(KFLOW_AI_IOC_CAT_OUT));
				return -EFAULT;
			}
		}
		break;

	case KFLOW_AIS_CAT_STR_OUT:
		{
			char cat_str_tmp[128] = {};
			if (copy_from_user(&cat_str_tmp, (void __user *)arg, sizeof(cat_str_tmp))) {
				ret = -EFAULT;
				goto exit;
			}
			// DBG_DUMP("this is %s \n", cat_str_tmp);
			debug_log_output(cat_str_tmp);
		}
		break;
#endif
    case KFLOW_AI_IOC_WAIT_MISCDEV_RELEASE_DONE:
		{
			UINT32 proc_id = 0, p_id = 0;
            FLGPTN flag = 0, p_bit = 0;

			if (copy_from_user(&proc_id, (void __user *)arg, sizeof(UINT32))) {
				ret = -EFAULT;
				goto exit;
			}
            p_id = proc_id/32;
            p_bit = FLGPTN_BIT(proc_id%32);
            //blocking here until vendor_ais_flow_miscdev_release is done
            wai_flg(&flag, g_kflow_release_flag_id[p_id], p_bit, TWF_ORW | TWF_CLR);
            //signal immediately so that the next vendor_ais_flow_miscdev_open can succeed
            set_flg(g_kflow_release_flag_id[p_id], p_bit);
        }
		break;
    case VENDOR_AIS_FLOW_IOC_GET_CHIP_ID:
		{
			UINT32 chip_id = 0;
            UINT32 real_chip_id = 0;
			if (copy_from_user(&chip_id, (void __user *)arg, sizeof(UINT32))) {
				ret = -EFAULT;
				goto exit;
			}
			real_chip_id = nvt_get_chip_id();
            switch(real_chip_id) {
#if defined(_BSP_NS02201_)
                case CHIP_NS02201:
                    chip_id = NN_CHIP_CNN30_A;
                    break;
#elif defined(_BSP_NS02302_)
                case CHIP_NS02302:
                    chip_id = NN_CHIP_CNN30_B;
                    break;
                case CHIP_NS02402:
                    chip_id = NN_CHIP_CNN30_C;
                    break;
#elif defined(_BSP_NS02401_)
				case CHIP_NS02401:
					chip_id = NN_CHIP_CNN30_D;
                    break;
#endif
                default:
                    DBG_ERR("real_chip_id(0x%x) is not support in AI3 SDK...\n", real_chip_id);
                    break;
            }
	
			if (unlikely(copy_to_user((void __user *)arg, &chip_id, sizeof(UINT32)))) {
				//DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
				ret = -EFAULT;
				goto exit;
			}
		}
		break;

	default :
		break;
	}

exit:

	return ret;
}

static int vendor_ais_flow_miscdev_release(struct inode *inode, struct file *file)
{
//	if (vendor_ais_buf_va_addr) {
//		if (fmem_release_from_cma(vendor_ais_buf_va_addr, 0) < 0) {
//			return -1;
//		}
//		vendor_ais_buf_va_addr = NULL;
//	}
	KFLOW_AI_NET_DEV *pdrv_info = &kflow_ai_net_dev;
	UINT32 minor = (MINOR(inode->i_rdev) - pdrv_info->minor_base);
    UINT32 net_max_num = kflow_ai_get_net_supported_num(); // net_max_num should be got from kdriver
    if (minor > 0 && minor <= net_max_num) {
        UINT32 p_id = 0;
        FLGPTN p_bit = 0;
#if (FLOW_AI_DEV == 1)
        SEM_WAIT(g_ai_path_sem_id);
        SEM_WAIT(g_kflow_net_sem_id);
#if (FLOW_AI_PROCID == 1)
        if(g_ai_net_path_open[minor-1] == 1) {
            //ctrl-c flow
            kflow_ai_reset_net_path(minor-1);
        }
        //notify user(KFLOW_AI_IOC_WAIT_MISCDEV_RELEASE_DONE) release is done
        p_id = (minor-1)/32;
        p_bit = FLGPTN_BIT((minor-1)%32);
        set_flg(g_kflow_release_flag_id[p_id], p_bit);

#endif
        SEM_SIGNAL(g_kflow_net_sem_id);
        SEM_SIGNAL(g_ai_path_sem_id);
#endif
    }
    else if (minor == 0) {

#if (FLOW_AI_DEV == 1)
        SEM_WAIT(g_kflow_net_sem_id);
		kflow_cmd_out_uninit(); // uinit cmd
		kflow_cat_out_uninit(); // uinit cat
		// avoid overflow  => ctrl + c happened between doing dla_reset(), dla_init() will cause overflow
		if(g_ai_net_init_count > 0 )
        	g_ai_net_init_count -= 1;
		if(kflow_init_count > 0 ) 
        	kflow_init_count -= 1;
		if(g_ai_net_init_count == 0) {
#if (FLOW_AI_NET == 1)
			if(g_ai_net_init == 1 && nvt_ai_is_path_need_reset() == 0) {
				nvt_ai_uninit_net();
			}
#endif
		}
#if (FLOW_AI_NET == 1)
        if(kflow_init_count == 0 && kflow_init == 1 && nvt_ai_is_path_need_reset() == 0) {
#if (FLOW_AI_JOB == 1)
            kflow_ai_core_uninit(1);
            kflow_ai_cpu_uninit_cb();
#if (AI_SUPPORT_DSP == 1)
            kflow_ai_dsp_uninit_cb();
#endif
#endif
        }
#endif
        SEM_SIGNAL(g_kflow_net_sem_id);
#endif
    }	
	return 0;
}

#if 0
static ssize_t vendor_ais_flow_miscdev_write(struct file *file, const char __user *buf, size_t count, loff_t *offp)
{
	int ret = 0;
	int bufsize = count;
	vendor_ais_ddr_t *mem;
	KFLOW_AI_NET_DEV *pdev = (KFLOW_AI_NET_DEV *)file->private_data;

	if (pdev->mem_type == 0) {
		mem = &vendor_ais_noncache_buf;
		if (bufsize >= (mem->bufsize - pdev->mem_offset - *offp)) {
			DBG_ERR("skip write file, write file size > noncache size\r\n");
			goto exit;
		}
	} else {
		mem = &vendor_ais_cache_buf;
		if (bufsize >= (mem->bufsize - pdev->mem_offset - *offp)) {
			DBG_ERR("skip write file, write file size > cache size\r\n");
			goto exit;
		}
	}
	if ((pdev->mem_limitsz != 0) && (bufsize > pdev->mem_limitsz)) {
		bufsize = pdev->mem_limitsz;
	}

	if (copy_from_user((void *)(mem->va + pdev->mem_offset + *offp), (void __user *)buf, bufsize)) {
		DBG_ERR("copy from user fail\r\n");
	} else {
		ret = bufsize;
	}

	if (mem->type == ALLOC_CACHEABLE) {
		fmem_dcache_sync((mem->va + pdev->mem_offset + *offp), bufsize, DMA_TO_DEVICE);
	}

exit:

	return ret;
}

static ssize_t vendor_ais_flow_miscdev_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	int ret = 0;
	int bufsize = count;
	vendor_ais_ddr_t *mem;
	KFLOW_AI_NET_DEV *pdev = (KFLOW_AI_NET_DEV *)file->private_data;

	if (pdev->mem_type == 0) {
		mem = &vendor_ais_noncache_buf;
		if (bufsize >= (mem->bufsize - pdev->mem_offset - *pos)) {
			DBG_ERR("skip read mem, read mem size > noncache size\r\n");
			goto exit;
		}
	} else {
		mem = &vendor_ais_cache_buf;
		if (bufsize >= (mem->bufsize - pdev->mem_offset - *pos)) {
			DBG_ERR("skip read mem, read mem size > cache size\r\n");
			goto exit;
		}
	}

	if ((pdev->mem_limitsz != 0) && (bufsize > pdev->mem_limitsz)) {
		bufsize = pdev->mem_limitsz;
	}

	if (mem->type == ALLOC_CACHEABLE) {
		fmem_dcache_sync((mem->va + pdev->mem_offset + *pos), bufsize, DMA_FROM_DEVICE);
	}

	if (copy_to_user((void __user *)buf, (mem->va + pdev->mem_offset + *pos), bufsize)) {
		DBG_ERR("copy to user fail\r\n");
	} else {
		ret = bufsize;
	}

exit:

	return ret;
}
#endif

static struct file_operations vendor_ais_flow_miscdev_fops = {
	.owner          = THIS_MODULE,
	.open           = vendor_ais_flow_miscdev_open,
	.release        = vendor_ais_flow_miscdev_release,
	.unlocked_ioctl = vendor_ais_flow_miscdev_ioctl,
//	.write          = vendor_ais_flow_miscdev_write,
//	.read           = vendor_ais_flow_miscdev_read,
};

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/

UINT32 kflow_ai_get_net_supported_num(VOID)
{
	UINT32 ai_max_net_num_now;

	ai_max_net_num_now = (UINT32) ai_net_supported_num;
	
	if (ai_max_net_num_now == 0) {
		DBG_ERR("User config net supported num = %u? force update to %u!\n",
						 ai_max_net_num_now, 1);
		ai_max_net_num_now = 1;
		ai_net_supported_num = 1;
	}
	if (ai_max_net_num_now > AI_SUPPORT_PROC_MAX) {
		DBG_ERR("User config net supported num = %u > %u, force update to %u!\n",
						 ai_max_net_num_now, AI_SUPPORT_PROC_MAX, AI_SUPPORT_PROC_MAX);
		ai_max_net_num_now = AI_SUPPORT_PROC_MAX;
		ai_net_supported_num = AI_SUPPORT_PROC_MAX;
	}

	return (UINT32) ai_max_net_num_now;
}
EXPORT_SYMBOL(kflow_ai_get_net_supported_num) ; 

int vendor_ais_flow_miscdev_init(void)
{
	int ret = 0;
	unsigned int ucloop;
	// module_count could be set by user via insmod. 
	// For example,  "insmod kflow_ai.ko ai_net_supported_num=16"
	int module_count = kflow_ai_get_net_supported_num() + 2 ; 
    int release_flag_count = 0;
	KFLOW_AI_NET_DEV *pdrv_info = &kflow_ai_net_dev;

#if 0
	dump_fd();
#endif
	pdrv_info->pdevice = nvt_ai_mem_alloc(module_count * sizeof(struct device *));
	if (pdrv_info->pdevice == 0) {
		DBG_ERR("Can't alloc device buffer\n");
		return -ENODEV;
	}

    if(kflow_ai_get_net_supported_num() % 32 == 0) {
        release_flag_count = kflow_ai_get_net_supported_num()/32;
    } else {
        release_flag_count = kflow_ai_get_net_supported_num()/32 + 1;
    }

    g_kflow_release_flag_id = nvt_ai_mem_alloc(release_flag_count * sizeof(ID));
	if (g_kflow_release_flag_id == 0) {
		DBG_ERR("Can't alloc g_kflow_release_flag_id\n");
		return -ENODEV;
	}
	
	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, module_count, MODULE_NAME)) {
		DBG_ERR("Can't get device ID\n");
		return -ENODEV;
	}

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &vendor_ais_flow_miscdev_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, module_count)) {
		DBG_ERR("Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	pdrv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
#else
	pdrv_info->pmodule_class = class_create(MODULE_NAME);
#endif
	if(IS_ERR(pdrv_info->pmodule_class)) {
		DBG_ERR("failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	pdrv_info->minor_base = MINOR(pdrv_info->dev_id);

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	for (ucloop = 0 ; ucloop < (module_count) ; ucloop++) {
		if (ucloop == 0) {
		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
									   , MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
									   , MODULE_NAME);
		}else if (ucloop == module_count -1 ){
		// reserve the last one 
		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
									   , MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
									   , MODULE_NAME"%d", kflow_ai_get_net_supported_num()+1);

		} else {
		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
									   , MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
									   , MODULE_NAME"%d", ucloop);
		}
		
		if (IS_ERR(pdrv_info->pdevice[ucloop])) {
			DBG_ERR("failed in creating device%d.\n", ucloop);

				for (;ucloop > 0; ucloop--) {
					device_unregister(pdrv_info->pdevice[ucloop - 1]);
				}
			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}
    if(kflow_ai_get_net_supported_num() % 32 == 0) {
        release_flag_count = kflow_ai_get_net_supported_num()/32;
    } else {
        release_flag_count = kflow_ai_get_net_supported_num()/32 + 1;
    }
    for (ucloop = 0 ; ucloop < (release_flag_count) ; ucloop++) {
        OS_CONFIG_FLAG(g_kflow_release_flag_id[ucloop]);
        set_flg(g_kflow_release_flag_id[ucloop], FLGPTN_BIT_ALL); //set all flag
    }
	vk_spin_lock_init(&ioctl_lock);
	kflow_ai_net_proc_create();
	SEM_CREATE(g_kflow_net_sem_id, 1);

#if (FLOW_AI_DEV == 1)
	nvt_ai_global_init();
#endif

	return ret;

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, module_count);

	return ret;
}

void vendor_ais_flow_miscdev_exit(void)
{
	unsigned int ucloop;
	int module_count = kflow_ai_get_net_supported_num() + 2 ; // module_count should be got from kdriver
	int release_flag_count = 0;
    KFLOW_AI_NET_DEV *pdrv_info = &kflow_ai_net_dev;

	//--- clear context ---
    kflow_ai_core_reset();
    kflow_ai_cpu_uninit_cb();
#if (AI_SUPPORT_DSP == 1)
    kflow_ai_dsp_uninit_cb();
#endif
#if (FLOW_AI_DEV == 1)
    nvt_ai_reset_net();
#endif
	SEM_DESTROY(g_kflow_net_sem_id);
	kflow_ai_net_proc_remove();

#if (FLOW_AI_DEV == 1)
	nvt_ai_global_uninit();
#endif

	for (ucloop = 0 ; ucloop < module_count ; ucloop++) {
		device_unregister(pdrv_info->pdevice[ucloop]);
	}

    if(kflow_ai_get_net_supported_num() % 32 == 0) {
        release_flag_count = kflow_ai_get_net_supported_num()/32;
    } else {
        release_flag_count = kflow_ai_get_net_supported_num()/32 + 1;
    }

    for (ucloop = 0 ; ucloop < release_flag_count ; ucloop++) {
        rel_flg(g_kflow_release_flag_id[ucloop]);
	}

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, module_count);

	if (pdrv_info->pdevice != 0) {
		nvt_ai_mem_free(pdrv_info->pdevice);
		pdrv_info->pdevice = 0;
	}

    if (g_kflow_release_flag_id != 0) {
		nvt_ai_mem_free(g_kflow_release_flag_id);
		g_kflow_release_flag_id = 0;
	}
	
}

module_init(vendor_ais_flow_miscdev_init);
module_exit(vendor_ais_flow_miscdev_exit);
MODULE_AUTHOR("Novatek Microelectronics Corp.");
//MODULE_LICENSE("NVT");
MODULE_LICENSE("GPL");
MODULE_VERSION(KFLOW_AI_IMPL_VERSION);
