/**
    @file       ctl_sen_int.h

    @brief      Define sensor control function

    @note      Sensor Driver APIs

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/
#ifndef _CTL_SEN_INT_H_
#define _CTL_SEN_INT_H_

#include "plat/top.h"
#include "ctl_sen.h"
#include "kdrv_videocapture/kdrv_ssenif.h"
#include "kdrv_videocapture/kdrv_ssenif_lmt.h"
#include "kdrv_videocapture/kdrv_sie.h"
#include "kdrv_videocapture/kdrv_vie.h"
#include "kdrv_videocapture/kdrv_tge.h"
#include "kwrap/mem.h"
#include "kwrap/error_no.h"
#include "kwrap/flag.h"
#include "kwrap/task.h"
#include "kwrap/util.h"
#include <kwrap/spinlock.h>
#include "kwrap/semaphore.h"
#include "ctl_sen_dbg.h"
#include "comm/sif_api.h"
#include "comm/hwclock.h"
#include <nvt_api_ver.h>


#define _INLINE static inline
#if defined(__FREERTOS) || defined(__ECOS) || defined(__UITRON)
#include <malloc.h>
#include <string.h>
#else
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/slab.h>
#endif
#define CTL_SEN_MAX_OUTPUT_SIE_IDX KDRV_SIE_MAX_ENG
#define CTL_SEN_MAX_OUTPUT_VIE_IDX (KDRV_VIE_MAX_ENG * KDRV_VIE_MAX_VDO_CH)
#define CTL_SEN_TGE_MAX_CH 8
#define SEN_DIV_U64(x, y) sen_uint64_dividend(x, y)
#define SEN_240MHZ 240000000
#define SEN_480MHZ 480000000
#define CTL_SEN_PLL5_DFT_FREQ PLL5_MAX_FREQ
/* for sen ctrl if */
#define CTL_SEN_SKIP_SET_KDRV 0xff  /* kdrv not support feature */
/* for pinmux drv*/
#define CTL_SEN_PINMUX_SENSOR_MAX 2 /* PIN_SENSOR_CFG ~ PIN_SENSORx_CFG */
/* for kdrv */
#define KDRV_CHIP_SSENIF KDRV_CHIP0
#define KDRV_CHIP_TGE KDRV_CHIP0
#define KDRV_CHIP_SIE KDRV_CHIP0
#define KDRV_SSENIF_GLB_HDL 0


/*
    buffer control
*/
/*
--> buf_addr -----------------------
            |kflow (idx 1)          | (size:context_size)
             -----------------------
            |kflow (idx 2)          | (size:context_size)
             -----------------------
            .
            .
            .
             -----------------------
            |kflow (idx context_num)| (size:context_size)
             -----------------------
            |kdrv                   | (size:kdrv_size)
             -----------------------

*/
typedef struct {
	BOOL b_init;
	UINT32 context_num;     // hdal query device number
	UINT32 context_used;    // used device number
	UINT32 context_size;;   // buffer size per device
	ULONG buf_addr;        // total buffer start address
	UINT32 buf_size;        // total buffer size
	UINT32 kflow_size;      // base on context_num, kflow total request buffer size
	UINT32 kdrv_size;       // base on context_num, kdrv total request buffer size
	BOOL local_alloc;       // 0: ctl_sen_init buffer from HDAL, 1: kflow allocate buffer for ctl_sen_init
} CTL_SEN_HDL_CONTEXT;

//#define CTL_SEN_CONTEXT_SIZE CTL_SEN_ALIGN_ROUNDUP(sizeof(CTL_SEN_INIT_OBJ) + sizeof(CTL_SEN_CTRL_OBJ), 4) // TODO
#define CTL_SEN_KDRV_BUF(num) 0 // if kdrv need buffer, add here

/**
    sensor ext object (by project & HW)
*/
typedef struct {
	UINT32 timeout_ms;   ///< time out ms
	UINT32 tge_sync;   ///< tge sync bit
	UINT32 force_dis;	///< force disable when stop
	UINT32 pinmux_cond; ///< clean/keep pinmux cfg if off
} CTL_SEN_EXT_IF;

typedef struct {
	CTL_SEN_EXT_IF if_info;
} CTL_SEN_INIT_EXT_OBJ;

typedef struct {
	CTL_SEN_PWR_CTRL pwr_ctrl;
	CTL_SEN_INIT_CFG_OBJ init_cfg_obj;
	CTL_SEN_INIT_EXT_OBJ init_ext_obj;
	CTL_SEN_PLUG_IN det_plug_in;
	CTL_SEN_DRV_TAB *drv_tab;
} CTL_SEN_INIT_OBJ, *PCTL_SEN_INIT_OBJ;

/* time record */
#if 0
typedef enum {
	CTL_SEN_PROC_TIME_ITEM_ENTER,
	CTL_SEN_PROC_TIME_ITEM_EXIT,
	CTL_SEN_PROC_TIME_ITEM_MAX,
	ENUM_DUMMY4WORD(CTL_SEN_PROC_TIME_ITEM),
} CTL_SEN_PROC_TIME_ITEM;
#endif



typedef struct {
	struct vos_mem_cma_info_t cma_info;
	VOS_MEM_CMA_HDL cma_hdl;
} CTL_SEN_VOS_MEM_INFO;

/* common covert api */
extern UINT32 sen_uint64_dividend(UINT64 dividend, UINT32 divisor);

#if 0 // TODO if 0
/* mem info */
extern INT32 ctl_sen_util_os_malloc(CTL_SEN_VOS_MEM_INFO *vod_mem_info, UINT32 req_size);
extern INT32 ctl_sen_util_os_mfree(CTL_SEN_VOS_MEM_INFO *vod_mem_info);
extern void *ctl_sen_util_os_malloc_wrap(UINT32 want_size);
extern void ctl_sen_util_os_mfree_wrap(void *p_buf);
#endif


/**
    atomic operations
*/
#if defined(__LINUX)
/* use linux kernel atomic api */
#else
/* use gcc api */
typedef INT32 atomic_t;

#define ATOMIC_INIT(i) (i)
#define atomic_inc(x) __sync_add_and_fetch(x, 1)
#define atomic_dec(x) __sync_sub_and_fetch(x, 1)
#define atomic_read(x) __sync_add_and_fetch(x, 0)
#define atomic_set(x, v) __sync_bool_compare_and_swap(x, *x, v)
#define atomic_add(i, x) __sync_add_and_fetch(x, i)
#define atomic_xchg(x, v) __sync_val_compare_and_swap(x, *x, v)

#endif


/* Flag id */
typedef enum {
	CTL_SEN_FLAG_GLB,
	CTL_SEN_FLAG_MAP,
	CTL_SEN_FLAG_CHIP,

	CTL_SEN_FLAG_MAX,
	ENUM_DUMMY4WORD(CTL_SEN_FLAG)
} CTL_SEN_FLAG;

/* CTL_SEN_FLAG_GLB */
#define CTL_SEN_FLAG_GLB_OFS_INIT_FLOW 		1	// kflow_ctl_sen_init & kflow_ctl_sen_uninit
#define CTL_SEN_FLAG_GLB_OFS_INIT_FLOW_DONE	2	// kflow_ctl_sen_init & kflow_ctl_sen_uninit
#define CTL_SEN_FLAG_GLB_OFS_INIT_BUF 		3	// ctl_sen_init & ctl_sen_uninit
#define CTL_SEN_FLAG_GLB_OFS_INIT_BUF_DONE 	4	// ctl_sen_init & ctl_sen_uninit
#define CTL_SEN_FLAG_GLB_OFS_INIT_MAP 		5
#define CTL_SEN_FLAG_GLB_INIT (FLGPTN_BIT(CTL_SEN_FLAG_GLB_OFS_INIT_FLOW) | FLGPTN_BIT(CTL_SEN_FLAG_GLB_OFS_INIT_BUF) | FLGPTN_BIT(CTL_SEN_FLAG_GLB_OFS_INIT_MAP))

/* CTL_SEN_FLAG_MAP */
#define CTL_SEN_FLAG_MAP_OFS_IDX	1
#define CTL_SEN_FLAG_MAP_INIT (FLGPTN_BIT(CTL_SEN_FLAG_MAP_OFS_IDX))

/* CTL_SEN_FLAG_CHIP */
#define CTL_SEN_FLAG_CHIP_OFS_IDX	1
#define CTL_SEN_FLAG_CHIP_OFS_GET	2
#define CTL_SEN_FLAG_CHIP_INIT (FLGPTN_BIT(CTL_SEN_FLAG_CHIP_OFS_IDX))

/* config api */
extern ID ctl_sen_flag_glb;
extern ID ctl_sen_flag_map[CTL_SEN_NUM_SEN_ID];
extern ID ctl_sen_flag_chip[CTL_SEN_NUM_SEN_ID];
extern ID ctl_sen_get_flag_id(CTL_SEN_FLAG type, UINT32 idx);
extern SEM_HANDLE ctl_sen_sem_proc;
extern SEM_HANDLE ctl_sen_sem_mclken;

#define CTL_SEN_MAP_NULL CTL_SEN_IGNORE

/*
    chip_hdl information
    1 chip_hdl <-> multiple map_hdl
*/

typedef INT32(*IF_OPEN_FP)(void *);
typedef INT32(*IF_CLOSE_FP)(void *);
typedef INT32(*IF_START_FP)(void *);
typedef INT32(*IF_STOP_FP)(void *);
typedef INT32(*IF_SETMODE_FP)(void *);
typedef INT32(*IF_SET_FP)(void *, UINT32, void *);
typedef INT32(*IF_GET_FP)(void *, UINT32, void *);
typedef CTL_SEN_INTE(*if_WAITINTRPT_FP)(void *, CTL_SEN_INTE);

typedef struct {
	/* chip_id info */
	UINT32					chip_idx;		// set first init_map sen_id as chip_idx

	CTL_SEN_REG_OBJ        *sendrv;
	UINT32                  sen_mode;
	UINT32                  chgmode_fps;

	CTL_SEN_CLKSRC_SEL		mclk_src_sel;
	UINT32					mclk_freq;
	UINT32                  mclk_sen_mode;	// current mclk_freq get from which sen_mode

	CTL_SEN_INIT_CFG_OBJ    init_cfg_obj;
	CTL_SEN_INIT_EXT_OBJ    init_ext_obj;

	/* internal */
	CTL_SEN_PINMUX          *pinmux_box;
	UINT32					data_lane_num;	// current data lane num get from init_cfg_obj->pin_cfg->sen_2_serial_pin_map
	UINT32					is_ad;			// check if is ad sensor
	CTL_SEN_LVDS_SYNC_MANUAL_INFO lvds_sync_info;	// lvds manual sync info


	/* chip_id ctrl */
	atomic_t                open_cnt;       // open++,close--
	atomic_t                pwron_cnt;      // pwron++,pwroff--
	atomic_t                chgmode_cnt;    // chgmode++,all sen_id(used this chip_hdl) del_map set0
	atomic_t                sleep_cnt;    	// sleep++,wakeup--
	atomic_t                get_cnt;       	// get_begin++,get_end--,chgmode_begin check 0

#if 0 // cannot dump after uninit map
	/* error cnt */
	atomic_t 				open_err_cnt;
	atomic_t 				close_err_cnt;
	atomic_t 				pwron_err_cnt;
	atomic_t 				pwrof_err_cnt;
	atomic_t 				chgmode_err_cnt;
	atomic_t 				sleep_err_cnt;
	atomic_t 				wakup_err_cnt;
#endif

	/* ssenif ctrl */
	IF_OPEN_FP              if_open_fp;
	IF_CLOSE_FP             if_close_fp;
	IF_START_FP             if_start_fp;
	IF_STOP_FP              if_stop_fp;
	IF_SETMODE_FP           if_setmode_fp;
	IF_SET_FP           	if_set_fp;
	IF_GET_FP           	if_get_fp;
	if_WAITINTRPT_FP    	if_waitintrpt_fp;
	UINT32                  if_hdl;
	UINT32					if_en_user;	// 1 : start & stop by sendrv

	/* tge ctrl */
	IF_OPEN_FP              if2_open_fp;
	IF_CLOSE_FP             if2_close_fp;
	IF_START_FP             if2_start_fp;
	IF_STOP_FP              if2_stop_fp;
	IF_SETMODE_FP           if2_setmode_fp;
	IF_SET_FP           	if2_set_fp;
	IF_GET_FP           	if2_get_fp;
	if_WAITINTRPT_FP    	if2_waitintrpt_fp;
	UINT32					if2_en_user;// 1 : start & stop by sendrv

	atomic_t                if2_open_cnt;       // open++,close--

#define if2_ch_param_cnt 2
#define tge_param_vdhd 0			// KDRV_TGE_VDHD_CH
#define tge_param_wait_event_vd 1	// TGE_WAIT_EVENT_SEL(VD)

	UINT32 					if2_ch_param[if2_ch_param_cnt];

} CTL_SEN_CHIP_HDL;

/*
    map_hdl informatin
    1 sen_id -> 1 map_hdl  -> 1 chip_hdl -> 1 frm_idx
*/
typedef struct {
	/* sen_id info */
	UINT32                  sen_id;	// range : CTL_SEN_MIN_SEN_ID ~ CTL_SEN_MAX_SEN_ID

	UINT32                  chip_id;
	CHAR                    chip_name[CTL_SEN_NAME_LEN];
	CTL_SEN_CHIP_HDL       *chip_hdl;

	UINT32                  frm_idx;
	UINT32                  output_dest;    // ctl_sen_output_sie_x or ctl_sen_output_vie_x, ex ctl_sen_output_sie_0
	UINT32                  sen_id_status;  // CTL_SEN_STATUS_XXX (ex : CTL_SEN_STATUS_IDLE)
	UINT32					chgmode_cond;	// CTL_SEN_CHGMODE_COND_XXX (ex : CTL_SEN_CHGMODE_COND_FORCE_SENDRV)
	CTL_SEN_DRVDEV			drvdev;			// for combo mode, which every MAP_HDL have unique controller
	UINT32                  if_hdl;			// for combo mode, which every MAP_HDL have unique controller

	/* list */
	struct vos_list_head    list;
} CTL_SEN_MAP_HDL;


typedef struct {
	/* info */
	CTL_SEN_REG_OBJ  sendrv;
	CHAR             chip_name[CTL_SEN_NAME_LEN];

	/* list */
	struct vos_list_head    list;
} CTL_SEN_SENDRV_HDL;


typedef struct {
	BOOL					init_flag;

	/* map_hdl list ctrl */
	vk_spinlock_t           map_list_lock;
	unsigned long           map_list_flag;
	struct vos_list_head    map_list_root;  // link CTL_SEN_MAP_HDL

	/* sendrv_hdl list ctrl */
	vk_spinlock_t           sendrv_list_lock;
	unsigned long           sendrv_list_flag;
	struct vos_list_head    sendrv_list_root;  // link CTL_SEN_SENDRV_HDL

} CTL_SEN_HDL;

typedef INT32(*CTL_SEN_SET_FP)(CTL_SEN_MAP_HDL *, void *);
typedef INT32(*CTL_SEN_GET_FP)(CTL_SEN_MAP_HDL *, void *);


extern CTL_SEN_HDL *ctl_sen_hdl;
extern void(*ctl_sen_cert_func)(CTL_SEN_MAP_HDL *, CTL_SEN_INIT_MAP *, UINT32);

#endif // _CTL_SEN_INT_H_

