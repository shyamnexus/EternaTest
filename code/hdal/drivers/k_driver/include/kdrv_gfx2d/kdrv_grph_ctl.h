#ifndef _GRAPH_DRV_CTL_H_
#define _GRAPH_DRV_CTL_H_

#include <kwrap/nvt_type.h>
#include <kwrap/list.h>
#include <kwrap/util.h>
#include <kwrap/spinlock.h>

#include "kdrv_grph.h"

/* job status */
enum graph_drv_sts {
    GRAPH_DRV_STS_IDLE = 0,
    GRAPH_DRV_STS_QUEUE,     	// not process yet, in job list
    GRAPH_DRV_STS_PROC,			// ready to process
    GRAPH_DRV_STS_PROC_END,		// process done
    GRAPH_DRV_STS_DONE,			// done by hw process
    GRAPH_DRV_STS_FLUSH,     	// stop by driver(not done)
    GRAPH_DRV_STS_ERROR,		// error by driver(not done)
    GRAPH_DRV_STS_OK,
    GRAPH_DRV_STS_QFULL,		// queue full
    GRAPH_DRV_STS_MAX
};

struct graph_drv_job_cfg {

    /******* add configs at here *******/
	KDRV_GRPH_TRIGGER_PARAM req;
    /******* add configs at here *******/

    enum graph_drv_sts sts; //job ststus
    struct vos_list_head list; //job link list
};

struct graph_drv_job_head {

	UINT32 chip; //0~n
	UINT32 eng;  //0:graph0, 1:graph1, 2:graph2
	UINT32 id;   //job_id:unique code for debug used

    UINT16 num; //total job number

    struct vos_list_head job_list_root; //job list

    void (*callback)(struct graph_drv_job_head *head); //job callback to notify job finish
};

extern enum graph_drv_sts graph_drv_ctl_put_job(struct graph_drv_job_head *head);
extern int graph_drv_eng_get_chip_num(void);
extern int graph_drv_eng_get_eng_num(void);
/******************************************************************************************/
/*******                                kdrv interface                              *******/
/******************************************************************************************/
typedef VOID (*graph_drv_cb)(UINT32 trig_id, UINT32 p0, UINT32 p1);

#define KDRV_GRAPH_PARAM_BASE 0x12340000
enum kdrv_graph_param_id {
	KDRV_GRAPH_PARAM_QUEUE_MAX_NUM = KDRV_GRAPH_PARAM_BASE,		//[   /get] data type UINT32
	KDRV_GRAPH_PARAM_DMA_ABORT,								//[set/   ] no parameter
	KDRV_GRAPH_PARAM_BLK_MODE,								//[set/get] data type UINT32(0:disable(def), 1:enable)
	KDRV_GRAPH_PARAM_MAX
};
#define KDRV_GRAPH_PARAM_ID_MAX (KDRV_GRAPH_PARAM_MAX - KDRV_GRAPH_PARAM_BASE)

/************************* API *************************/
/**
     graph drv init

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_graph_rtos_init(void);

/**
     graph drv uninit

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_graph_rtos_uninit(void);
/**
     allocate graph hw resource

     @param[in] chip designed chip index
     @param[in] eng  designed eng index

     @return Description of data returned.
         - @b >= 0: allocte resouce success, return hw resouce handle
         - @b <  0: allocate fail
*/
ULONG kdrv_graph_open(UINT32 chip, UINT32 eng);

/**
     free graph hw resource

     @param[in] hw resouce handle

     @return Description of data returned.
         - @b >= 0: free success
         - @b <  0: free fail
*/
INT32 kdrv_graph_close(ULONG handle);

/**
     get information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_graph_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_graph_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_graph_get(ULONG handle, enum kdrv_graph_param_id param_id, void *p_param);

/**
     set information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_graph_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_graph_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_graph_set(ULONG handle, enum kdrv_graph_param_id param_id, void *p_param);

/**
     trigger hw engine

     @param[in] id hw resouce handle
     @param[in] job information

     @return Description of data returned.
         - @b = GRAPH_DRV_STS_OK: success
*/
enum graph_drv_sts kdrv_graph_trigger(ULONG handle, struct graph_drv_job_head *head);
#endif  /* _GRAPH_DRV_CTL_H_ */

