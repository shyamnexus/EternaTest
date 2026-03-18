#ifndef _TRKE_DRV_CTL_H_
#define _TRKE_DRV_CTL_H_

#include <kwrap/nvt_type.h>
#include <kwrap/list.h>
#include <kwrap/util.h>
#include <kwrap/spinlock.h>
#include "trke_ioctl.h"

#define TRKE_DRV_PT_MAX_NUM 16

struct trke_drv_job_cfg {

    /******* add configs at here *******/
	unsigned int layer_num;
	struct trke_layer_info layer;
    unsigned char init_sts; // 0: do not set the pt_sts, 1: set the pt_sts to trke
	unsigned int pt_sts[TRKE_DRV_PT_MAX_NUM];
    UINT32* va_status_rlt;
    /******* add configs at here *******/
	//int apply_next;   //1: get result and set to next job
    enum trke_drv_sts sts; //job ststus
    struct vos_list_head list; //job link list
};

struct trke_drv_job_head {

	UINT32 chip;
	UINT32 eng;
	UINT32 id;

    UINT16 num; //total job number

	UINT8 blk_mode; //0:non-blocking mode, 1:blocking mode

    struct vos_list_head job_list_root; //job list

    void (*callback)(struct trke_drv_job_head *head); //job callback to notify job finish
};

struct trke_drv_sts_buf {
	unsigned int hdl_id;
	UINT32 status[TRKE_DRV_PT_MAX_NUM];
    struct vos_list_head list; //job link list
};

extern enum trke_drv_sts trke_drv_ctl_put_job(struct trke_drv_job_head *head);
/******************************************************************************************/
/*******                                kdrv interface                              *******/
/******************************************************************************************/
typedef VOID (*trke_drv_cb)(UINT32 trig_id, UINT32 p0, UINT32 p1);

#define KDRV_TRKE_PARAM_BASE 0x12340000
enum kdrv_trke_param_id {
	KDRV_TRKE_PARAM_QUEUE_MAX_NUM = KDRV_TRKE_PARAM_BASE,		//[   /get] data type UINT32
	KDRV_TRKE_PARAM_DMA_ABORT,								//[set/   ] no parameter
	KDRV_TRKE_PARAM_BLK_MODE,								//[set/get] data type UINT32(0:disable(def), 1:enable)
	KDRV_TRKE_PARAM_PRI,										//[set/get] data type UINT32(0(high) ~ 2(low), 1(def))
	KDRV_TRKE_PARAM_MAX
};
#define KDRV_TRKE_PARAM_ID_MAX (KDRV_TRKE_PARAM_MAX - KDRV_TRKE_PARAM_BASE)

/************************* API *************************/
/**
     trke drv init

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_trke_rtos_init(void);

/**
     trke drv uninit

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_trke_rtos_uninit(void);
/**
     allocate trke hw resource

     @param[in] chip designed chip index
     @param[in] eng  designed eng index

     @return Description of data returned.
         - @b >= 0: allocte resouce success, return hw resouce handle
         - @b <  0: allocate fail
*/
ULONG kdrv_trke_open(UINT32 chip, UINT32 eng);

/**
     free trke hw resource

     @param[in] hw resouce handle

     @return Description of data returned.
         - @b >= 0: free success
         - @b <  0: free fail
*/
INT32 kdrv_trke_close(ULONG handle);

/**
     get information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_trke_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_trke_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_trke_get(ULONG handle, enum kdrv_trke_param_id param_id, void *p_param);

/**
     set information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_trke_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_trke_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_trke_set(ULONG handle, enum kdrv_trke_param_id param_id, void *p_param);

/**
     trigger hw engine

     @param[in] id hw resouce handle
     @param[in] job information

     @return Description of data returned.
         - @b = TRKE_DRV_STS_OK: success
*/
enum trke_drv_sts kdrv_trke_trigger(ULONG handle, struct trke_drv_job_head *head);
#endif  /* _TRKE_DRV_CTL_H_ */

