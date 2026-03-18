#ifndef _ISE_DRV_CTL_H_
#define _ISE_DRV_CTL_H_

#include <kwrap/nvt_type.h>
#include <kwrap/list.h>
#include <kwrap/util.h>
#include <kwrap/spinlock.h>

/* job status */
enum ise_drv_sts {
    ISE_DRV_STS_IDLE = 0,
    ISE_DRV_STS_QUEUE,      // not process yet, in job list
    ISE_DRV_STS_PROC,		// ready to process
    ISE_DRV_STS_PROC_END,	// process done
    ISE_DRV_STS_DONE,		// done by hw process
    ISE_DRV_STS_FLUSH,      // stop by driver(not done)
    ISE_DRV_STS_ERROR,		// error by driver(not done)
    ISE_DRV_STS_OK,
    ISE_DRV_STS_QFULL,		// queue full
    ISE_DRV_STS_MAX
};

struct ise_drv_addr {
	uintptr_t va;
	uintptr_t pa;
};

struct ise_drv_roi {
	UINT32 w;
	UINT32 h;
	UINT32 lofs[1];
};

enum ise_drv_fmt {
	ISE_DRV_Y8_ONLY = 0,		//in scale method: bilinear/nearest/isd
	ISE_DRV_Y4_ONLY,			//in scale method: bilinear/nearest, scale down only
	ISE_DRV_Y1_ONLY,			//in scale method: nearest, scale down only
	ISE_DRV_UVP,				//in scale method: bilinear/nearest/isd, 1 pair of UV = 1 pixel
	ISE_DRV_RGB565,				//in scale method: bilinear/nearest
	ISE_DRV_ARGB8888,			//in scale method: bilinear/nearest
	ISE_DRV_ARGB1555,			//in scale method: bilinear/nearest
	ISE_DRV_ARGB4444,			//in scale method: bilinear/nearest
	ISE_DRV_YUVP,				//in scale method: bilinear/nearest/isd, 1 pair of YUV = 1 pixel
    ISE_DRV_FMT_MAX,
};

enum ise_drv_scale_method_sel {
	ISE_DRV_SCALE_METHOD_AUTO = 0,  //auto select bilinear or ISD
	ISE_DRV_SCALE_METHOD_BILINEAR,
	ISE_DRV_SCALE_METHOD_NEAREST,
	ISE_DRV_SCALE_METHOD_ISD,		//scale down only,
	ISE_DRV_SCALE_METHOD_MAX
};

struct ise_drv_iq_info {
	enum ise_drv_scale_method_sel scale_method;
	enum ise_drv_scale_method_sel alpha_scale_method; //only valid at ARGB format, scale method: bilinear/nearest
};

struct ise_drv_in_info {
	struct ise_drv_addr addr[1];
	UINT8 ddr_id;
	struct ise_drv_roi roi;
	enum ise_drv_fmt fmt;
	UINT8 flush_buf;						//0:not to do flush, 1: to do flush
};

struct ise_drv_out_info {
	struct ise_drv_addr addr[1];
	UINT8 ddr_id;
	struct ise_drv_roi roi;
	UINT8 alpha_trans_th;					//0~255, only valid at output = ARGB1555, alpha > th, A=1 else A=0
	UINT8 flush_buf;						//0:not to do flush, 1: to do flush
};

struct ise_drv_job_cfg {

    /******* add configs at here *******/
	struct ise_drv_in_info in;
	struct ise_drv_out_info out;
	struct ise_drv_iq_info iq;
    /******* add configs at here *******/

    enum ise_drv_sts sts; //job ststus
    struct vos_list_head list; //job link list
};

#define KDRV_ISE_MODE struct ise_drv_job_cfg

struct ise_drv_job_head {

	UINT32 chip;
	UINT32 eng;
	UINT32 id;

    UINT16 num; //total job number

    struct vos_list_head job_list_root; //job list

    void (*callback)(struct ise_drv_job_head *head); //job callback to notify job finish
};

extern enum ise_drv_sts ise_drv_ctl_put_job(struct ise_drv_job_head *head);
extern int ise_drv_eng_get_chip_num(void);
extern int ise_drv_eng_get_eng_num(void);

#define KDRV_ISE 0xA0
extern void ise_drv_ctl_log(unsigned char e0, unsigned char e1, unsigned char e2, unsigned char e3, unsigned int p0, unsigned int p1);
/******************************************************************************************/
/*******                                kdrv interface                              *******/
/******************************************************************************************/
typedef VOID (*ise_drv_cb)(UINT32 trig_id, UINT32 p0, UINT32 p1);

#define KDRV_ISE_PARAM_BASE 0x12340000
enum kdrv_ise_param_id {
	KDRV_ISE_PARAM_QUEUE_MAX_NUM = KDRV_ISE_PARAM_BASE,		//[   /get] data type UINT32
	KDRV_ISE_PARAM_DMA_ABORT,								//[set/   ] no parameter
	KDRV_ISE_PARAM_BLK_MODE,								//[set/get] data type UINT32(0:disable(def), 1:enable)
	KDRV_ISE_PARAM_PRI,										//[set/get] data type UINT32(0(high) ~ 2(low), 1(def))
	KDRV_ISE_PARAM_MAX
};
#define KDRV_ISE_PARAM_ID_MAX (KDRV_ISE_PARAM_MAX - KDRV_ISE_PARAM_BASE)

/************************* API *************************/
/**
     ise drv init

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_ise_rtos_init(void);

/**
     ise drv uninit

     @return Description of data returned.
         - @b >= 0: pass
         - @b <  0: fail
*/
INT32 kdrv_ise_rtos_uninit(void);
/**
     allocate ise hw resource

     @param[in] chip designed chip index
     @param[in] eng  designed eng index

     @return Description of data returned.
         - @b >= 0: allocte resouce success, return hw resouce handle
         - @b <  0: allocate fail
*/
ULONG kdrv_ise_open(UINT32 chip, UINT32 eng);

/**
     free ise hw resource

     @param[in] hw resouce handle

     @return Description of data returned.
         - @b >= 0: free success
         - @b <  0: free fail
*/
INT32 kdrv_ise_close(ULONG handle);

/**
     get information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_ise_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_ise_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_ise_get(ULONG handle, enum kdrv_ise_param_id param_id, void *p_param);

/**
     set information

     @param[in] handler hw resouce handle
     @param[in] param_id enum kdrv_ise_param_id
     @param[in,out] p_param parameter, data type ref. enum kdrv_ise_param_id

     @return Description of data returned.
         - @b >= 0: success
         - @b <  0: fail
*/
INT32 kdrv_ise_set(ULONG handle, enum kdrv_ise_param_id param_id, void *p_param);

/**
     trigger hw engine

     @param[in] id hw resouce handle
     @param[in] job information

     @return Description of data returned.
         - @b = ISE_DRV_STS_OK: success
*/
enum ise_drv_sts kdrv_ise_trigger(ULONG handle, struct ise_drv_job_head *head);
#endif  /* _ISE_DRV_CTL_H_ */

