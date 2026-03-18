#ifndef _IVE_DRV_CTL_H_
#define _IVE_DRV_CTL_H_

#include <kwrap/nvt_type.h>
#include <kwrap/list.h>
#include <kwrap/util.h>
#include <kwrap/spinlock.h>
#include "ive_ioctl.h"

struct ive_drv_job_cfg {

    /******* add configs at here *******/
	struct ive_drv_in_info in;
	struct ive_drv_out_info out;

	enum ive_drv_op_mode op;

	union {
		struct ive_drv_canny_edge_info canny_edge;
		//struct ive_drv_ncc_info ncc;
		//struct ive_drv_csc_info csc;
		struct ive_drv_histo_info histo;
		//struct ive_drv_ccl_info ccl;
        struct ive_drv_thres_lut_info lut_thres;
		struct ive_drv_img_op_info img_op;
		struct ive_drv_16_to_8_info bit16to8;
		struct ive_drv_copy_info copy;
		struct ive_drv_st_info st;
		struct ive_drv_lbp_info lbp;
		struct ive_drv_va_info va;
	};

    /******* add configs at here *******/
	int apply_next;   //1: get result and set to next job
    enum ive_drv_sts sts; //job ststus
    struct vos_list_head list; //job link list
};

struct ive_drv_job_head {

	UINT32 chip;
	UINT32 eng;
	UINT32 id;

    UINT16 num; //total job number

	UINT8 blk_mode; //0:non-blocking mode, 1:blocking mode

    struct vos_list_head job_list_root; //job list

    void (*callback)(struct ive_drv_job_head *head); //job callback to notify job finish
};

extern enum ive_drv_sts ive_drv_ctl_put_job(struct ive_drv_job_head *head);
/******************************************************************************************/
/*******                                kdrv interface                              *******/
/******************************************************************************************/
typedef VOID (*ive_drv_cb)(UINT32 trig_id, UINT32 p0, UINT32 p1);

#define KDRV_IVE_PARAM_BASE 0x12340000
enum kdrv_ive_param_id {
	KDRV_IVE_PARAM_QUEUE_MAX_NUM = KDRV_IVE_PARAM_BASE,		//[   /get] data type UINT32
	KDRV_IVE_PARAM_DMA_ABORT,								//[set/   ] no parameter
	KDRV_IVE_PARAM_BLK_MODE,								//[set/get] data type UINT32(0:disable(def), 1:enable)
	KDRV_IVE_PARAM_PRI,										//[set/get] data type UINT32(0(high) ~ 2(low), 1(def))
	KDRV_IVE_PARAM_MAX
};
#define KDRV_IVE_PARAM_ID_MAX (KDRV_IVE_PARAM_MAX - KDRV_IVE_PARAM_BASE)

/************************* API *************************/
/**
     ive drv init

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_ive_rtos_init(void);

/**
     ive drv uninit

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_ive_rtos_uninit(void);
/**
     allocate ive hw resource

     @param[in] chip designed chip index
     @param[in] eng  designed eng index

     @return Description of data returned.
         - @b >= 0: allocte resouce success, return hw resouce handle
         - @b <  0: allocate fail
*/
ULONG kdrv_ive_open(UINT32 chip, UINT32 eng);

/**
     free ive hw resource

     @param[in] hw resouce handle

     @return Description of data returned.
         - @b >= 0: free success
         - @b <  0: free fail
*/
INT32 kdrv_ive_close(ULONG handle);

/**
     get information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_ive_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_ive_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_ive_get(ULONG handle, enum kdrv_ive_param_id param_id, void *p_param);

/**
     set information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_ive_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_ive_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_ive_set(ULONG handle, enum kdrv_ive_param_id param_id, void *p_param);

/**
     trigger hw engine

     @param[in] id hw resouce handle
     @param[in] job information

     @return Description of data returned.
         - @b = IVE_DRV_STS_OK: success
*/
enum ive_drv_sts kdrv_ive_trigger(ULONG handle, struct ive_drv_job_head *head);
#endif  /* _IVE_DRV_CTL_H_ */

