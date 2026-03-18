/**
	@brief Source file of IO control of vendor net isp sample.

	@file net_isp_sample_ioctl.c

	@ingroup net_isp_sample

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

#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/fdtable.h>
#include <linux/fs.h> 
#include <linux/fs_struct.h>
#include <linux/dcache.h>
#include <linux/slab.h>
//#include "frammap/frammap_if.h"
#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h>
#else
#include <mach/fmem.h>
#endif

#include "kflow_ai_net/kflow_ai_net_platform.h"
//=============================================================
#define __CLASS__ 				"[ai][kflow_isp][ictl]"
#include "kflow_ai_isp_debug.h"
//=============================================================


#include "kflow_isp/kflow_isp.h"
#include "kflow_isp/kflow_ai_isp_net.h"
#include "kflow_ai_isp_version.h"
#include "kflow_ai_isp_proc.h"

unsigned int kflow_ai_isp_debug_level = NVT_DBG_ERR;
#if defined (__LINUX)
module_param_named(kflow_ai_isp_debug_level, kflow_ai_isp_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(kflow_ai_isp_debug_level, "kflow_ai_isp_debug_level");
#endif




extern UINT32 kflow_isp_init;
extern UINT32 kflow_isp_init_count;
BOOL *kflow_isp_net_path_open;
BOOL *kflow_isp_path_need_reset;

SEM_HANDLE g_kflow_ai_isp_sem_id;
SEM_HANDLE g_kflow_ai_isp_init_uninit_sem_id;
/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
BOOL kflow_isp_is_path_need_reset(VOID) ; 
/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
#define MODULE_NAME VENDOR_AIS_ISP_DEV_NAME
typedef struct _KFLOW_AI_NET_DEV {
	struct class *pmodule_class;
	struct device **pdevice;
	//struct resource* presource[MODULE_REG_NUM];
	struct cdev cdev;
	dev_t dev_id;
	UINT32 minor_base;
} KFLOW_AI_NET_DEV, *PKFLOW_AI_NET_DEV;
static KFLOW_AI_NET_DEV kflow_ai_net_dev;

/*-----------------------------------------------------------------------------*/
/* external Functions                                                          */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Functions                                                             */
/*-----------------------------------------------------------------------------*/
static int vendor_ais_isp_miscdev_open(struct inode *inode, struct file *file)
{
	KFLOW_AI_NET_DEV *pdrv_info = &kflow_ai_net_dev;
	file->private_data = (void *)pdrv_info;
	return 0;
}



static long vendor_ais_isp_miscdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	UINT32 net_id = 0 ; 
	VENDOR_AIS_ISP_UBUF_INFO ubuf_info = {0};

	if (_IOC_TYPE(cmd) != VENDOR_AIS_ISP_IOC_MAGIC) {
		ret = -ENOIOCTLCMD;
		goto exit;
	}

switch (cmd) {
	
   case KFLOW_AI_ISP_IOC_INIT_ISP_NET:
      {
		SEM_WAIT(g_kflow_ai_isp_sem_id);
		SEM_WAIT(g_kflow_ai_isp_init_uninit_sem_id);
        ret = kflow_isp_init_jmisp_net() ;
		SEM_SIGNAL(g_kflow_ai_isp_init_uninit_sem_id);
		SEM_SIGNAL(g_kflow_ai_isp_sem_id);

      }
      break ; 
	  
   case KFLOW_AI_ISP_IOC_UNINIT_ISP_NET:
      {
		SEM_WAIT(g_kflow_ai_isp_sem_id);
		SEM_WAIT(g_kflow_ai_isp_init_uninit_sem_id);
        ret = kflow_isp_uninit_jmisp_net() ;
		SEM_SIGNAL(g_kflow_ai_isp_init_uninit_sem_id);
		SEM_SIGNAL(g_kflow_ai_isp_sem_id);
      }
      break ; 
	case KFLOW_AI_ISP_IOC_RESET_ISP_NET:
      {
		SEM_WAIT(g_kflow_ai_isp_sem_id);
		SEM_WAIT(g_kflow_ai_isp_init_uninit_sem_id);
        ret = kflow_isp_reset_jmisp_net() ;
		SEM_SIGNAL(g_kflow_ai_isp_init_uninit_sem_id);
		SEM_SIGNAL(g_kflow_ai_isp_sem_id);
      }
      break ;   
   case KFLOW_AI_ISP_IOC_PARS_ISP_NET:
      {
         VENDOR_AIS_ISP_PARM isp_parm = {0} ;  
         if (copy_from_user(&isp_parm, (void __user *)arg, sizeof(VENDOR_AIS_ISP_PARM))) {
            ret = -EFAULT;
            goto exit;
         }
		 if (isp_parm.p_info.net_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",isp_parm.p_info.net_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
		 net_id = isp_parm.p_info.net_id  ;
		 if (kflow_isp_path_need_reset[net_id]){
			kflow_isp_close_jmisp_net(net_id) ;
			kflow_isp_path_need_reset[net_id] = 0 ; 
		 }
         ret = kflow_isp_pars_jmisp_net(&isp_parm);
		 kflow_isp_net_path_open[net_id] = 1 ;

      }
      break ; 
   case KFLOW_AI_ISP_IOC_CLOSE_ISP_NET:
      {
         if (copy_from_user(&net_id, (void __user *)arg, sizeof(UINT32))) {
         ret = -EFAULT;
         goto exit;
         }
		if (net_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",net_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
         ret = kflow_isp_close_jmisp_net(net_id);
		 kflow_isp_net_path_open[net_id] = 0 ;

      }
      break ; 

   case KFLOW_AI_ISP_IOC_SET_ISP_CB:
      {
         uintptr_t isp_cb = {0};

         if (copy_from_user((void *)&isp_cb, (void *)arg, sizeof(uintptr_t))) {
            DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &isp_cb, arg, sizeof(uintptr_t));
            return -EFAULT;
         }
         ret = kflow_isp_set_cb(isp_cb);
      }
      break;

   case KFLOW_AI_ISP_IOC_GET_ISP_CB:
      {
         uintptr_t isp_cb = {0};

         if (copy_from_user((void *)&isp_cb, (void *)arg, sizeof(uintptr_t))) {
            DBG_ERR("copy_from_user fail(%p, %#lx, %lu)\r\n", &isp_cb, arg, sizeof(uintptr_t));
            return -EFAULT;
         }
         isp_cb = (uintptr_t) &kflow_isp_cb;
         // DBG_DUMP("isp_cb = %lx \r\n", (ULONG)isp_cb) ; 
         
         if (copy_to_user((void *)arg, &isp_cb, sizeof(uintptr_t))) {
            DBG_ERR("copy_to_user fail(%p, %#lx, %lu)\r\n", &isp_cb, arg, sizeof(uintptr_t));
            return -EFAULT;
         }
      }
      break;   

   case KFLOW_AI_ISP_IOC_SET_ISP_INPUT:
      {
         VENDOR_AIS_ISP_INPUT_INFO isp_input_info = {0} ; 
         if (copy_from_user(&isp_input_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_INPUT_INFO))) {
            ret = -EFAULT;
            goto exit;
         }
		 if (isp_input_info.net_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",isp_input_info.net_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
         ret = kflow_isp_set_input(&isp_input_info);
      
         break;
      }
      break;   

	case KFLOW_AI_ISP_IOC_SET_ISP_INPUT_INFO:  // 539A flow (1) keep input info when start (2) used when set input from IPP callback
	{
		VENDOR_AIS_ISP_INPUT_INFO isp_input_info = {0};
		if (copy_from_user(&isp_input_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_INPUT_INFO))) {
			ret = -EFAULT;
			goto exit;
		}
		if (isp_input_info.net_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",isp_input_info.net_id );
			ret = -EINVAL;
			goto exit;
		}
		ret = kflow_isp_set_input_info(&isp_input_info);
	}
	break;

   case KFLOW_AI_ISP_IOC_WEIGHT_LOC:
      {
         VENDOR_AIS_ISP_WEIGHT_LOC loc = {0} ;  
         if (copy_from_user(&loc, (void __user *)arg, sizeof(VENDOR_AIS_ISP_WEIGHT_LOC))) {
            ret = -EFAULT;
            goto exit;
         }
		 if (loc.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",loc.proc_id  ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
         ret = kflow_isp_set_weight_loc(&loc);

      }
      break ; 
   case KFLOW_AI_ISP_IOC_GET_VER:
	  {
		CHAR version_info[32] = KFLOW_AI_ISP_IMPL_VERSION;
		ret = copy_to_user((void __user *)arg, version_info, sizeof(KFLOW_AI_ISP_IMPL_VERSION));
	  }
	  break;   
   case KFLOW_AI_ISP_IOC_GET_UBUF_ISP:
      {
         if (copy_from_user(&ubuf_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_UBUF_INFO))) {
            ret = -EFAULT;
            goto exit;
         }
		if (ubuf_info.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",ubuf_info.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
         ret = kflow_isp_get_ubuf(ubuf_info.proc_id, &ubuf_info.idx, ubuf_info.size_in_mb);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }

         if (unlikely(copy_to_user((void __user *)arg, &ubuf_info, sizeof(VENDOR_AIS_ISP_UBUF_INFO)))) {
            //DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
            ret = -EFAULT;
            goto exit;
         }
      }
      break;

   case KFLOW_AI_ISP_IOC_FREE_UBUF_ISP:
      {
         if (copy_from_user(&ubuf_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_UBUF_INFO))) {
            ret = -EFAULT;
            goto exit;
         }
		 if (ubuf_info.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",ubuf_info.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
         ret = kflow_isp_free_ubuf(ubuf_info.proc_id, ubuf_info.idx, ubuf_info.size_in_mb);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;
   case KFLOW_AI_ISP_IOC_SET_POOL_INFO:
      {
		 VENDOR_AIS_ISP_POOL_INFO pool_info = {0} ;
         if (copy_from_user(&pool_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_POOL_INFO))) {
            ret = -EFAULT;
            goto exit;
         }
		 if (pool_info.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",pool_info.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
		 if (kflow_isp_path_need_reset[pool_info.proc_id]){
			kflow_isp_close_jmisp_net(pool_info.proc_id) ;
			kflow_isp_path_need_reset[pool_info.proc_id] = 0 ; 
		 }

		 if(pool_info.pool_id == 0xff)
				ret = kflow_isp_set_default_pool_info(&pool_info);
		 else 
         		ret = kflow_isp_set_pool_info(&pool_info);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;
   case KFLOW_AI_ISP_IOC_GET_PROC_ID_BY_POOL:
      {
		 VENDOR_AIS_ISP_POOL_INFO pool_info = {0} ;
         if (copy_from_user(&pool_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_POOL_INFO))) {
            ret = -EFAULT;
            goto exit;
         }

         ret = kflow_isp_get_proc_id_by_pool(&pool_info);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }
		 if (unlikely(copy_to_user((void __user *)arg, &pool_info, sizeof(VENDOR_AIS_ISP_POOL_INFO)))) {
				ret = -EFAULT;
				goto exit;
			}
      }
      break;
  case KFLOW_AI_ISP_IOC_DBG_CAL_SLICE:
      {
		NN_ISP_FRAME isp_input_frame = {0} ; 
		if (copy_from_user(&isp_input_frame, (void __user *)arg, sizeof(NN_ISP_FRAME))) {
			ret = -EFAULT;
			goto exit;
		}
		if (isp_input_frame.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",isp_input_frame.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		}
		ret = kflow_ai_isp_dbg_cal_slice(&isp_input_frame) ; 
		if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;
  case KFLOW_AI_ISP_IOC_DBG_PUSH_FRAME:
      {
		 VENDOR_AIS_ISP_INPUT_INFO isp_input_info = {0} ; 
         if (copy_from_user(&isp_input_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_INPUT_INFO))) {
            ret = -EFAULT;
            goto exit;
         }
		 if (isp_input_info.net_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",isp_input_info.net_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
		ret = kflow_ai_isp_dbg_push_frame(&isp_input_info) ; 
		if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;
   case KFLOW_AI_ISP_IOC_GET_JOBM_CORE:
      {
         if (copy_from_user(&ubuf_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_UBUF_INFO))) {
            ret = -EFAULT;
            goto exit;
         }
		if (ubuf_info.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",ubuf_info.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
		 ret = kflow_isp_get_jobm_core(ubuf_info.proc_id, &ubuf_info.idx);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }

         if (unlikely(copy_to_user((void __user *)arg, &ubuf_info, sizeof(VENDOR_AIS_ISP_UBUF_INFO)))) {
            //DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
            ret = -EFAULT;
            goto exit;
         }
      }
      break;   
  case KFLOW_AI_ISP_IOC_DBG_SET_REF_FRAME:
      {
        VENDOR_AIS_ISP_INPUT_INFO isp_input_info = {0} ;
        if (copy_from_user(&isp_input_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_INPUT_INFO))) {
           ret = -EFAULT;
           goto exit;
        }
        if (isp_input_info.net_id >= kflow_ai_get_net_supported_num()){
           DBG_ERR(" proc_id[%u] is out of range ! \r\n",isp_input_info.net_id ) ;
           ret = -EINVAL ;
           goto exit;
        }
        ret = kflow_ai_isp_dbg_set_ref_frame(&isp_input_info) ;
        if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;
 case KFLOW_AI_ISP_IOC_GET_SET_CORE_MASK:
      {
		 VENDOR_AIS_ISP_MASK mask_info = {0} ; 
         if (copy_from_user(&mask_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_MASK))) {
            ret = -EFAULT;
            goto exit;
         }
		if (mask_info.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",mask_info.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
		 ret = kflow_isp_set_core_mask(&mask_info);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;   
  case KFLOW_AI_ISP_IOC_GET_SET_UB_MASK:
      {
		 VENDOR_AIS_ISP_MASK mask_info = {0} ; 
         if (copy_from_user(&mask_info, (void __user *)arg, sizeof(VENDOR_AIS_ISP_MASK))) {
            ret = -EFAULT;
            goto exit;
         }
		if (mask_info.proc_id >= kflow_ai_get_net_supported_num()){
			DBG_ERR(" proc_id[%u] is out of range ! \r\n",mask_info.proc_id ) ; 
			ret = -EINVAL ; 
			goto exit;
		 }
		 ret = kflow_isp_set_ub_mask(&mask_info);
         if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;   
  case KFLOW_AI_ISP_IOC_DBG_SET_ISP_PARAM:
      {
        NN_ISP_ISP_PARAM isp_param = {0} ;
        if (copy_from_user(&isp_param, (void __user *)arg, sizeof(NN_ISP_ISP_PARAM))) {
           ret = -EFAULT;
           goto exit;
        }
        if (isp_param.proc_id >= kflow_ai_get_net_supported_num()){
           DBG_ERR(" proc_id[%u] is out of range ! \r\n",isp_param.proc_id ) ;
           ret = -EINVAL ;
           goto exit;
        }
        ret = kflow_ai_isp_dbg_set_isp_param(&isp_param) ;
        if (ret != E_OK) { ret = -EFAULT ; goto exit; }
      }
      break;
	  
  case KFLOW_AI_ISP_IOC_GET_JMISP_INFO_BUF:
      {
        VENDOR_AIS_ISP_JMISP_INFO_PARM jmisp_info_param = {0} ;
        if (copy_from_user(&jmisp_info_param, (void __user *)arg, sizeof(VENDOR_AIS_ISP_JMISP_INFO_PARM))) {
           ret = -EFAULT;
           goto exit;
		}

		jmisp_info_param.jmisp_info.va = (uintptr_t)nvt_ai_pa2va_remap(jmisp_info_param.jmisp_info.pa, jmisp_info_param.jmisp_info.size);
        ret = kflow_ai_isp_get_workbuf_size_from_jmisp_info(&jmisp_info_param) ;
		nvt_ai_pa2va_unmap(jmisp_info_param.jmisp_info.va, jmisp_info_param.jmisp_info.pa);
        if (ret != E_OK) { ret = -EFAULT ; goto exit; }
		if (unlikely(copy_to_user((void __user *)arg, &jmisp_info_param, sizeof(VENDOR_AIS_ISP_JMISP_INFO_PARM)))) {
            //DBG_ERR("copy_to_user(0x%X, 0x%X, 0x%X)\r\n", (UINT32)argp, (UINT32)p_cmd, sizeof(VENDOR_AIS_FLOW_VERS));
            ret = -EFAULT;
            goto exit;
        }
      }

	default :
		break;
	}

exit:

	return ret;
}

static int vendor_ais_isp_miscdev_release(struct inode *inode, struct file *file)
{
	KFLOW_AI_NET_DEV *pdrv_info = &kflow_ai_net_dev;
	UINT32 minor = (MINOR(inode->i_rdev) - pdrv_info->minor_base);
    UINT32 net_max_num = kflow_ai_get_net_supported_num(); // net_max_num should be got from kdriver
    if (minor > 0 && minor <= net_max_num) {

        SEM_WAIT(g_kflow_ai_isp_sem_id);

        if(kflow_isp_net_path_open[minor-1] == 1) {
            kflow_isp_path_need_reset[minor-1] = 1;
        }

        SEM_SIGNAL(g_kflow_ai_isp_sem_id);
    }
    else if (minor == 0) {
		
		SEM_WAIT(g_kflow_ai_isp_sem_id);
		if(kflow_isp_init_count > 0 ) 
        	kflow_isp_init_count -= 1;

		if(kflow_isp_init_count == 0 && kflow_isp_init == 1 && kflow_isp_is_path_need_reset() == 0) {
            kflow_isp_uninit_jmisp_net();
		}
   

        SEM_SIGNAL(g_kflow_ai_isp_sem_id);
    }	
	return 0;
}



static struct file_operations vendor_ais_isp_miscdev_fops = {
	.owner          = THIS_MODULE,
	.open           = vendor_ais_isp_miscdev_open,
	.release        = vendor_ais_isp_miscdev_release,
	.unlocked_ioctl = vendor_ais_isp_miscdev_ioctl,
//	.write          = vendor_ais_isp_miscdev_write,
//	.read           = vendor_ais_isp_miscdev_read,
};

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
BOOL kflow_isp_is_path_need_reset(VOID)
{
	int i;
	int g_ai_support_net_max = kflow_ai_get_net_supported_num();
	for(i = 0;i < g_ai_support_net_max; i++) {
		if(kflow_isp_path_need_reset[i] == 1)
			return 1;
	}
	return 0;
}
ER kflow_isp_global_init(void)
{
	int g_ai_support_net_max = kflow_ai_get_net_supported_num();
	if (g_ai_support_net_max < 1) {
		DBG_ERR("g_ai_support_net_max < 1\r\n");
		return E_SYS;
	}	
	

	kflow_isp_net_path_open = (BOOL *)nvt_ai_mem_alloc(sizeof(BOOL) * g_ai_support_net_max);
	if (kflow_isp_net_path_open == NULL) {
		DBG_ERR("kflow_isp_net_path_open alloc fail\r\n");
		return E_NOMEM;
	}
	memset(kflow_isp_net_path_open, 0x0, sizeof(BOOL) * g_ai_support_net_max);

	kflow_isp_path_need_reset = (BOOL *)nvt_ai_mem_alloc(sizeof(BOOL) * g_ai_support_net_max);
	if (kflow_isp_path_need_reset == NULL) {
		DBG_ERR("kflow_isp_path_need_reset alloc fail\r\n");
		return E_NOMEM;
	}
	memset(kflow_isp_path_need_reset, 0x0, sizeof(BOOL) * g_ai_support_net_max);

	return E_OK;
}

ER kflow_isp_global_uninit(void)
{
	

	if (kflow_isp_net_path_open) {
		nvt_ai_mem_free(kflow_isp_net_path_open);
		kflow_isp_net_path_open = 0;
	}

	if (kflow_isp_path_need_reset) {
		nvt_ai_mem_free(kflow_isp_path_need_reset);
		kflow_isp_path_need_reset = 0;
	}
	
	return E_OK;
}

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

	current_files = current->files;
	files_table = files_fdtable(current_files);
	while(files_table->fd[i] != NULL) { 
		files_path = files_table->fd[i]->f_path;
		cwd = d_path(&files_path,buf,100*sizeof(char));
		printk(KERN_ALERT "Open file with fd %d  %s", i,cwd);
		i++;
	}
}
#endif

int vendor_ais_isp_miscdev_init(void)
{
	int ret = 0;
	unsigned char ucloop;
	// module_count should be got from kflow_ai 
	int module_count = kflow_ai_get_net_supported_num() + 2  ; 
	KFLOW_AI_NET_DEV *pdrv_info = &kflow_ai_net_dev;
#if 0
	dump_fd();
#endif
	
	pdrv_info->pdevice = nvt_ai_mem_alloc(module_count * sizeof(struct device *));
	if (pdrv_info->pdevice == 0) {
		pr_err("Can't alloc device buffer\n");
		return -ENODEV;
	}
	
	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, module_count, MODULE_NAME)) {
		pr_err("Can't get device ID\n");
		return -ENODEV;
	}

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &vendor_ais_isp_miscdev_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, module_count)) {
		pr_err("Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	pdrv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
#else
	pdrv_info->pmodule_class = class_create(MODULE_NAME);
#endif
	if(IS_ERR(pdrv_info->pmodule_class)) {
		pr_err("failed in creating class.\n");
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

	kflow_ai_isp_proc_create();
	SEM_CREATE(g_kflow_ai_isp_sem_id, 1);
	SEM_CREATE(g_kflow_ai_isp_init_uninit_sem_id, 1);
	kflow_isp_global_init() ;
	return ret;

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, module_count);

	return ret;
}

void vendor_ais_isp_miscdev_exit(void)
{
	unsigned char ucloop;
	int module_count = kflow_ai_get_net_supported_num() + 2; // module_count should be got from kdriver
	KFLOW_AI_NET_DEV *pdrv_info = &kflow_ai_net_dev;

	//--- clear context ---

	kflow_isp_reset_jmisp_net() ;
	SEM_DESTROY(g_kflow_ai_isp_sem_id);
	SEM_DESTROY(g_kflow_ai_isp_init_uninit_sem_id);
	kflow_ai_isp_proc_remove();
	kflow_isp_global_uninit() ;
	for (ucloop = 0 ; ucloop < module_count ; ucloop++) {
		device_unregister(pdrv_info->pdevice[ucloop]);
	}
	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, module_count);
	
	if (pdrv_info->pdevice != 0) {
		nvt_ai_mem_free(pdrv_info->pdevice);
		pdrv_info->pdevice = 0;
	}
	
}

module_init(vendor_ais_isp_miscdev_init);
module_exit(vendor_ais_isp_miscdev_exit);

MODULE_AUTHOR("Novatek Microelectronics Corp.");
//MODULE_LICENSE("NVT");
MODULE_LICENSE("GPL");
MODULE_VERSION(KFLOW_AI_ISP_IMPL_VERSION);
