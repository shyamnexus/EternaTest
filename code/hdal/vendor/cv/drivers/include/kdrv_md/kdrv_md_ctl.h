#ifndef _MD_DRV_CTL_H_
#define _MD_DRV_CTL_H_

#include <kwrap/nvt_type.h>
#include <kwrap/list.h>
#include <kwrap/util.h>
#include <kwrap/spinlock.h>
#include "md_ioctl.h"

struct md_drv_job_cfg {

    /******* add configs at here *******/
	MDBC_MODE                   mode;          ///< MDBC operation mode
	md_drv_ctrl_en              ctrl_en;     ///< MDBC control enable
	UINT32                      intrpt_en;       ///< MDBC Interrupt enable
    md_drv_input_info           in_info;        ///< MDBC input address & lofs
    md_drv_output_info          out_info;       ///< MDBC output address
	md_drv_in_size              size;          ///< MDBC size
    md_drv_mdmatch_param        mdmatch_para;   ///< MDBC Model Match
    md_drv_mor_param            mor_para;       ///< MDBC Morphological process
    md_drv_upd_param            upd_para;       ///< MDBC Update
    md_drv_roi_param            roi_para[MD_ROI_MAX];      ///< MDBC ROI0 ~ 7

	//rlt infor
	struct md_drv_rlt_info 		rlt_info;

    /******* add configs at here *******/
	int apply_next;   //1: get result and set to next job
    enum md_drv_sts sts; //job ststus
    struct vos_list_head list; //job link list
};

struct md_drv_job_head {

	UINT32 chip;
	UINT32 eng;
	UINT32 id;

    UINT16 num; //total job number

	UINT8 blk_mode; //0:non-blocking mode, 1:blocking mode

    struct vos_list_head job_list_root; //job list

    void (*callback)(struct md_drv_job_head *head); //job callback to notify job finish
};

extern enum md_drv_sts md_drv_ctl_put_job(struct md_drv_job_head *head);
/******************************************************************************************/
/*******                                kdrv interface                              *******/
/******************************************************************************************/
typedef VOID (*md_drv_cb)(UINT32 trig_id, UINT32 p0, UINT32 p1);

#define KDRV_MD_PARAM_BASE 0x12340000
enum kdrv_md_param_id {
	KDRV_MD_PARAM_QUEUE_MAX_NUM = KDRV_MD_PARAM_BASE,		//[   /get] data type UINT32
	KDRV_MD_PARAM_DMA_ABORT,								//[set/   ] no parameter
	KDRV_MD_PARAM_BLK_MODE,								//[set/get] data type UINT32(0:disable(def), 1:enable)
	KDRV_MD_PARAM_PRI,										//[set/get] data type UINT32(0(high) ~ 2(low), 1(def))
	KDRV_MD_PARAM_MAX
};
#define KDRV_MD_PARAM_ID_MAX (KDRV_MD_PARAM_MAX - KDRV_MD_PARAM_BASE)

/************************* API *************************/
/**
     md drv init

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_md_rtos_init(void);

/**
     md drv uninit

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_md_rtos_uninit(void);
/**
     allocate md hw resource

     @param[in] chip designed chip index
     @param[in] eng  designed eng index

     @return Description of data returned.
         - @b >= 0: allocte resouce success, return hw resouce handle
         - @b <  0: allocate fail
*/
ULONG kdrv_md_open(UINT32 chip, UINT32 eng);

/**
     free md hw resource

     @param[in] hw resouce handle

     @return Description of data returned.
         - @b >= 0: free success
         - @b <  0: free fail
*/
INT32 kdrv_md_close(ULONG handle);

/**
     get information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_md_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_md_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_md_get(ULONG handle, enum kdrv_md_param_id param_id, void *p_param);

/**
     set information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_md_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_md_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_md_set(ULONG handle, enum kdrv_md_param_id param_id, void *p_param);

/**
     trigger hw engine

     @param[in] id hw resouce handle
     @param[in] job information

     @return Description of data returned.
         - @b = MD_DRV_STS_OK: success
*/
enum md_drv_sts kdrv_md_trigger(ULONG handle, struct md_drv_job_head *head);
#endif  /* _MD_DRV_CTL_H_ */

