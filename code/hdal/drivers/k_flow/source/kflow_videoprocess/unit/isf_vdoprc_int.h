
#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/task.h"
#include "kwrap/spinlock.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/list.h"
#include "kwrap/util.h"
#include "comm/hwclock.h"
#include "plat/top.h"
#include "kdrv_builtin/nvtmpp_init.h"
#include "kflow_common/nvtmpp.h"
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_core.h"
#include "kflow_videoprocess/isf_vdoprc.h"
#include "kflow_videoprocess/ctl_ipp.h"
#include <kdrv_builtin/kdrv_builtin.h>
#include <kdrv_builtin/kdrv_ipp_builtin.h>

#define VPRC_FB_MSG_ENABLE (0)
#if VPRC_FB_MSG_ENABLE
#define FB_DUMP(fmt, args...) DBG_DUMP(fmt, ##args)
#else
#define FB_DUMP(fmt, args...)
#endif

#if defined (__FREERTOS)
#define SUPPORT_SMART_STITCH    DISABLE
#define USE_VPE   				ENABLE
#define USE_DRE   				ENABLE //to do
#else
#define SUPPORT_SMART_STITCH    ENABLE
#if defined(CONFIG_NVT_SMALL_HDAL)
#define USE_VPE   				DISABLE
#define USE_DRE   				DISABLE
#else
#if defined(_BSP_NA51055_)
#define USE_VPE   				ENABLE
#define USE_DRE   				DISABLE
#elif defined(_BSP_NA51102_)
#define USE_VPE   				ENABLE
#define USE_DRE   				ENABLE //to do
#elif defined(_BSP_NS02201_) || defined(_BSP_NS02302_)
#define USE_VPE   				ENABLE
#define USE_DRE   				ENABLE
#else
#define USE_VPE   				DISABLE
#define USE_DRE   				DISABLE
#endif
#endif
#endif
#define USE_ISE   				ENABLE
#define USE_EIS                 ENABLE

#if (USE_VPE == ENABLE)
#include "kflow_videoprocess/ctl_vpe.h"
#endif
#if (USE_ISE == ENABLE)
#include "kflow_videoprocess/ctl_ise.h"
#endif
#if (USE_DRE == ENABLE)
#include "kflow_videoprocess/ctl_dre.h"
#endif
#if (USE_EIS == ENABLE)
#include "kflow_videoprocess/ctl_vpe_isp.h"
#endif
#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
#include <malloc.h>
#define module_param_named(a, b, c, d)
#define MODULE_PARM_DESC(a, b)
#define EXPORT_SYMBOL(x)

#define debug_msg 			vk_printk

#include <string.h>
#define SLEEP(x)    		vos_util_delay_ms(1000*(x))
#define MSLEEP(x)    		vos_util_delay_ms(x)
#define USLEEP(x)   		vos_util_delay_us(x)
#define DELAY_M_SEC(x)		vos_util_delay_ms(x)
#define DELAY_U_SEC(x)		vos_util_delay_us(x)
#else
#include <linux/soc/nvt/nvtmem.h>
//#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>

#include <linux/printk.h>
#define debug_msg 			vk_printk

#define SLEEP(x)    		vos_util_delay_ms(1000*(x))
#define MSLEEP(x)    		vos_util_delay_ms(x)
#define USLEEP(x)   		vos_util_delay_us(x)
#define DELAY_M_SEC(x)		vos_util_delay_ms(x)
#define DELAY_U_SEC(x)		vos_util_delay_us(x)
#endif


#if defined (__FREERTOS)
#define USE_GFX				ENABLE  //external depend
#define USE_VDS				DISABLE  //external depend
#else
#define USE_GFX				ENABLE  //external depend
#define USE_VDS				ENABLE  //external depend
#endif

#define USE_NEW_SHDR			ENABLE
#define USE_IN_FRC				ENABLE
#define USE_OUT_FRC				ENABLE
#define USE_OUT_EXT  			ENABLE
#define USE_PULL	   			ENABLE

#define CUR_IN_FPS              ENABLE

#define USER_OUT_BUFFER_QUEUE_TIMEOUT 2000//ms

#define ISF_VDOPRC_VPE_OUT_NUM 	VDOPRC_MAX_VPE_OUT_NUM

#define USE_IN_ONEBUF	   		ENABLE
#define USE_IN_DIRECT	   		ENABLE
#define USE_OUT_ONEBUF	   		ENABLE
#define USE_OUT_LOWLATENCY	   	ENABLE
#define USE_OUT_DIS				ENABLE

#define ISF_VDOPRC_IN_NUM 		VDOPRC_MAX_IN_NUM
#define ISF_VDOPRC_OUT_NUM 		VDOPRC_MAX_OUT_NUM
#define ISF_VDOPRC_PHY_OUT_NUM 	VDOPRC_MAX_PHY_OUT_NUM
#define ISF_VDOPRC_PATH_NUM 	ISF_VDOPRC_OUT_NUM

#if defined(CONFIG_NVT_SMALL_HDAL)
#define ISF_VDOPRC_OUTQ_MAX 	1 //reduce for small version
#define ISF_VDOPRC_PULLQ_MAX 	5 //reduce for small version
#else
#define ISF_VDOPRC_OUTQ_MAX 	15
#define ISF_VDOPRC_PULLQ_MAX 	60
#endif

#define ISF_NEW_PULL			ENABLE
//#if defined(_BSP_NA51000_) || defined(_BSP_NA51055_)
//#define VDOPRC_OUT_DEPTH_MAX 2
//#endif
//#if defined(_BSP_NA51023_)
#define VDOPRC_OUT_DEPTH_MAX 	(4+ISF_VDOPRC_OUTQ_MAX)
//#endif

#if defined(CONFIG_NVT_SMALL_HDAL)
#define VDOPRC_IN_DEPTH_MAX 	4 //reduce for small version
#else
#define VDOPRC_IN_DEPTH_MAX 	15
#endif
#define VDOPRC_IN_DEPTH_DEF 	2

#define VDOPRC_DBG_TS_MAXNUM	30

#define PIPE_COMBINE_MASK        0x1000

#ifdef VDOPRC_FUNC_LCA
#define VDOPRC_FUNC_LCA_DEF    0x00
#define VDOPRC_FUNC_LCA_EN     0x01
#define VDOPRC_FUNC_LCA_DIS    0x02
#endif

// VSP
typedef enum _VDOPRC_VSP_STATE{
	VSP_STATE_UNINIT = 0x00,
	VSP_STATE_VPE    = 0x81,
	VSP_STATE_VPE2   = 0x82,
	VSP_STATE_DRE    = 0x83,
	VSP_STATE_VPE_LITE = 0x84,
	ENUM_DUMMY4WORD(VDOPRC_VSP_STATE)
} VDOPRC_VSP_STATE;

#define VSP_MAIN_FRAME_IDX    0
#define VSP_BLEND_FRAME_IDX   1

#define VSP_MAX_BLEND_FRAME   4

#define VSP_DEV_HANDLE_EX_VPE2       0
#define VSP_DEV_HANDLE_EX_DRE        1
#define VSP_DEV_HANDLE_EX_VPE_LITE   2
#define VSP_DEV_HANDLE_MAX_NUM       3

#define VSP_PIPE_NUM               5

#define VPE_PALETE_INDEX_BG_CLEAR  0
#define VPE_PALETE_INDEX_MASK      1

#define DRE_MAX_BLEND_FRAME		2
#define DRE_MAX_OUT_FRAME		1
#define DRE_MAX_LAYER_NUM		10

typedef enum {
	VPE_OUT0_SPLIT_PATH = 5,
	VPE_OUT1_SPLIT_PATH,
	VPE_OUT2_SPLIT_PATH,
	VPE_OUT3_SPLIT_PATH,
	VPE_OUT0_SPLIT_PATH2 = 9,
	VPE_OUT1_SPLIT_PATH2,
	VPE_OUT2_SPLIT_PATH2,
	VPE_OUT3_SPLIT_PATH2,
} VPE_SPLIT_ID;

#if (USE_OUT_EXT == ENABLE)
typedef struct _ISF_PIPE {
	UINT32 cmd_count;
	UINT32 cmd[64]; //this is call begin(), add_xxxx(), end() to fill action and parameters
} ISF_PIPE;

#define ISF_PIPE_CMD_BEGIN		0xFFFF0001
#define ISF_PIPE_CMD_SCALE		0xFFFF0002
#define ISF_PIPE_CMD_DIR		0xFFFF0003
//#define ISF_PIPE_CMD_PXLFMT	0xFFFF0004
#define ISF_PIPE_CMD_SCROP		0xFFFF0005
#define ISF_PIPE_CMD_DCROP		0xFFFF0006
#define ISF_PIPE_CMD_END		0xFFFF0000

extern void isf_pipe_begin(ISF_PIPE* p_pipe);
extern void isf_pipe_add_scrop(ISF_PIPE* p_pipe, INT32 crop_x, INT32 crop_y, INT32 crop_w, INT32 crop_h);
//extern void isf_pipe_add_dcrop(ISF_PIPE* p_pipe, INT32 crop_x, INT32 crop_y, INT32 crop_w, INT32 crop_h);
extern void isf_pipe_add_scale(ISF_PIPE* p_pipe, INT32 scale_w, INT32 scale_h, UINT32 pxlfmt, INT32 scale_h_align);
extern void isf_pipe_add_dir(ISF_PIPE* p_pipe, UINT32 dir, INT32 h_align);
extern void isf_pipe_end(ISF_PIPE* p_pipe);
extern ISF_RV isf_pipe_exec(ISF_UNIT* p_unit, UINT32 nport, ISF_PIPE* p_pipe, ISF_DATA* p_src_data, ISF_DATA* p_tmp_rotate);

extern BOOL _vdoprc_is_out_rotate(ISF_UNIT *p_thisunit, UINT32 pid);
extern void _vdoprc_config_out_ext(ISF_UNIT *p_thisunit, UINT32 pid, UINT32 en);
extern ISF_RV _isf_vdoprc_oport_do_dispatch_out_ext(struct _ISF_UNIT *p_thisunit, UINT32 oport, ISF_DATA *p_data, INT32 wait_ms, ISF_DATA* p_tmp_rotate);
extern UINT32 _ctlipp_id_map(ISF_UNIT *p_thisunit,UINT32 ctl_ipp_id);

#endif
#if (USE_EIS == ENABLE)
#define MAX_EIS_PATH_NUM 2

typedef enum {
	VDOPRC_EIS_FUNC_NONE = 0,    //eis is disabled
	VDOPRC_EIS_FUNC_NORMAL,      //eis is enabled with NVT gyro data
	VDOPRC_EIS_FUNC_CUSTOMIZED,  //eis is enabled with user's gyro data and others
	VDOPRC_EIS_FUNC_USER_DATA,   //eis is enabled with NVT gyro data and user data
} VDOPRC_EIS_FUNC;

//referring to VENDOR_EIS_GYRO_INFO
typedef struct _VDOCAP_EIS_GYRO_INFO
{
	UINT64 t_diff_crop;
	UINT64 t_diff_crp_end_to_vd;
	UINT32 angular_rate_x[MAX_GYRO_DATA_NUM];
	UINT32 angular_rate_y[MAX_GYRO_DATA_NUM];
	UINT32 angular_rate_z[MAX_GYRO_DATA_NUM];
	UINT32 acceleration_rate_x[MAX_GYRO_DATA_NUM];
	UINT32 acceleration_rate_y[MAX_GYRO_DATA_NUM];
	UINT32 acceleration_rate_z[MAX_GYRO_DATA_NUM];
	UINT32 gyro_timestamp[MAX_GYRO_DATA_NUM];
	UINT32 data_num;
} VDOCAP_EIS_GYRO_INFO;

typedef struct
{
	UINT32 offset;
	UINT32 size;
} VDOPRC_LUT2D_INFO;

typedef struct
{
	ULONG buf;
	UINT32 size;
	BOOL valid;
} VDOPRC_LAST_LUT2D_INFO;

extern UINT32 vprc_gyro_latency;
extern UINT32 vprc_gyro_data_num;
extern BOOL vprc_gyro_1st_frame;
extern UINT32 vprc_gyro_frame_time;
extern UINT32 prev_gyro_timestamp[MAX_GYRO_DATA_NUM];
extern UINT64 prev_frame_count;
extern UINT64 prev_crop_start;
extern UINT64 prev_crop_end;

#define VDOPRC_EIS_DBG_IN_GYRO      0x00000001
#define VDOPRC_EIS_DBG_OUT_FRM      0x00000002
#define VDOPRC_EIS_DBG_PERF         0x00000004
#define VDOPRC_EIS_DBG_FLOW         0x00000008

extern unsigned int isf_vdoprc_eis_dbg_flag;


extern VDOPRC_LUT2D_INFO lut2d_buf_info[];

extern VDOPRC_EIS_PLUGIN* _vprc_eis_plugin_;
extern void isf_vdoprc_put_gyro_frame_cnt(UINT64 frame_cnt);
extern BOOL isf_vdoprc_check_frame_with_gyro(UINT64 frame_cnt);
#endif

typedef void (*IPL_SYNC_CB)(UINT32 path, UINT64 end, UINT64 cycle);
#if (USE_IN_DIRECT == ENABLE)
typedef void (*SIE_UNLOCK_CB)(ISF_UNIT *p_thisunit, UINT32 oport, void *p_header_info);
#endif

typedef void (*SEN_USER3_CB)(ISF_UNIT *p_thisunit, UINT32 param);

typedef struct _VDOPRC_COLOR {
	CTL_IPP_OUT_COLOR_SPACE space;
} VDOPRC_COLOR;

typedef struct _VDOPRC_CTRL {
	UINT32 pipe;
	UINT32 in_cfg_func;
	UINT32 out_cfg_func[ISF_VDOPRC_OUT_NUM];  // keep user setting
	UINT32 iq_id;
	UINT32 _3dnr_refpath;
	UINT32 cur_3dnr_refpath;
	UINT32 _lowlatency_trig;
	CTL_IPP_FUNC func_max;
	CTL_IPP_FUNC func;
	CTL_IPP_FUNC cur_func;
#if _TODO
	UINT32 func2_max;
	UINT32 func2;
	UINT32 cur_func2;
#endif
#if (USE_NEW_SHDR == ENABLE)
	UINT32 shdr_cnt;
	UINT32 shdr_j;
	UINT32 shdr_i;
#endif
	CTL_IPP_SCL_METHOD_SEL scale;
	VDOPRC_COLOR color;
    UINT32 stripe_rule;
} VDOPRC_CTRL;

typedef struct _VDOPRC_PROCESS {
	UINT32 st_osdmask;
} VDOPRC_PROCESS;

typedef struct _VDOPRC_IN_PATH {
	USIZE max_size;
	VDO_PXLFMT max_pxlfmt;
	CTL_IPP_IN_CROP crop;
	CTL_IPP_FLIP_TYPE dir;
#if 0
	UINT32 user_count;
#endif
} VDOPRC_IN_PATH;

typedef struct _VDOPRC_IN_QUEUE {
	ISF_DATA input_pool;
	ISF_DATA* input_data;
	UINT32 input_used[VDOPRC_IN_DEPTH_MAX];
	UINT32 input_max;
	UINT32 input_cnt;
} VDOPRC_IN_QUEUE;

typedef struct _VDOPRC_OUT_QUEUE {
	UINT32 count_new_ok[ISF_VDOPRC_OUT_NUM];
	UINT32 count_new_fail[ISF_VDOPRC_OUT_NUM];
	UINT32 count_push_ok[ISF_VDOPRC_OUT_NUM];
	UINT32 count_push_fail[ISF_VDOPRC_OUT_NUM];
	UINT32 output_en[ISF_VDOPRC_OUT_NUM];
	UINT32* output_cur_en;
	UINT32 output_connecttype[ISF_VDOPRC_OUT_NUM];
#if (USE_OUT_ONEBUF == ENABLE)
	ULONG force_onebuffer[ISF_VDOPRC_OUT_NUM];
	UINT32 force_onesize[ISF_VDOPRC_OUT_NUM];
	UINT32 force_onej[ISF_VDOPRC_OUT_NUM];
	UINT32 force_one_reset[ISF_VDOPRC_OUT_NUM];
	ULONG force_refbuffer;
	UINT32 force_refsize;
	UINT32 force_refj;
	UINT32 force_onesize_max[ISF_VDOPRC_OUT_NUM];
#endif
#if _TODO
//for Notify
	UINT32 trigdata_id;
	UINT32* cur_trigdata_id;
#endif
#if _TODO
//sync queue
	//UINT32 ISF_VDOPRC_OUT_SEM_ID = {0};
	ISF_DATA sync_data[ISF_VDOPRC_OUT_NUM][VDOPRC_OUT_DEPTH_MAX];
	UINT32 sync_used[ISF_VDOPRC_OUT_NUM][VDOPRC_OUT_DEPTH_MAX];
	UINT32 sync_head[ISF_VDOPRC_OUT_NUM];
	UINT32 sync_tail[ISF_VDOPRC_OUT_NUM];
	UINT32 sync_cnt[ISF_VDOPRC_OUT_NUM];
#endif
//output queue
	ISF_DATA output_data[ISF_VDOPRC_OUT_NUM][VDOPRC_OUT_DEPTH_MAX];
	UINT32 output_used[ISF_VDOPRC_OUT_NUM][VDOPRC_OUT_DEPTH_MAX];
	UINT32 output_type[ISF_VDOPRC_OUT_NUM][VDOPRC_OUT_DEPTH_MAX];
	UINT32 output_cnt[ISF_VDOPRC_OUT_NUM];
	UINT32 output_max[ISF_VDOPRC_OUT_NUM];
#if (USE_OUT_DIS == ENABLE)
	UINT32 dis_start[ISF_VDOPRC_PHY_OUT_NUM];
	UINT32 dis_mode;
	UINT32 dis_en;
	UINT32 dis_buf_size;
	UINT32 dis_buf_addr;
	UINT32 dis_cfg_scaleratio;
	UINT32 dis_cfg_subsample;
	UINT32 dis_scaleratio;
	UINT32 dis_subsample;
	//tracking queue
	ISF_DATA* track_data[ISF_VDOPRC_PHY_OUT_NUM];
	UINT32 track_framecnt[ISF_VDOPRC_PHY_OUT_NUM];
	UINT32 track_j[ISF_VDOPRC_PHY_OUT_NUM];
#endif
} VDOPRC_OUT_QUEUE;

#if (ISF_NEW_PULL == ENABLE)
typedef struct _ISF_PULL_QUEUE {
	UINT32 		sign; ///< signature, equal to ISF_SIGN_QUEUE
	CHAR *		unit_name; ///< name string
	UINT32 		id; ///< id of this module
	UINT32		en;
	UINT32		head;	///< head pointer
	UINT32		tail;	///< tail pointer
	UINT32		is_full;	///< full flag
	UINT32		cnt; ///< current count
	UINT32		max;		///< max count
	ISF_DATA*	data;
	void*     	p_sem_q;
} ISF_PULL_QUEUE, *PISF_PULL_QUEUE;
#endif

typedef struct _VDOPRC_PULL_QUEUE {
	ISF_DATA 		data[ISF_VDOPRC_OUT_NUM][ISF_VDOPRC_PULLQ_MAX];
#if (ISF_NEW_PULL == ENABLE)
	ISF_PULL_QUEUE	output[ISF_VDOPRC_OUT_NUM];
#else
	ISF_DATA_QUEUE	output[ISF_VDOPRC_OUT_NUM];
#endif
	UINT32			num[ISF_VDOPRC_OUT_NUM];
	UINT32			fboot_buffing[ISF_VDOPRC_OUT_NUM]; //for buildin yuv number
} VDOPRC_PULL_QUEUE;

typedef struct _VDOPRC_OUT_DBG {
	UINT64 t[ISF_VDOPRC_OUT_NUM][VDOPRC_DBG_TS_MAXNUM];
} VDOPRC_OUT_DBG;

typedef struct _VDOPRC_PRE_ROTATE {
	ISF_DATA rotate_data;
	ISF_DATA* p_data;
} VDOPRC_PRE_ROTATE;

typedef struct _VDOPRC_EXT_CONTEXT {
	BOOL is_proc;
	UINT32 src_path;
	ISF_DATA in_data;
	ISF_DATA* p_data;
	ISF_PIPE pipe;
} VDOPRC_EXT_CONTEXT;

typedef struct _VDOPRC_COMMON_MEM {
	ISF_DATA    memblk;
	MEM_RANGE 	unit_buf;
	MEM_RANGE 	kflow_buf;
#if (USE_VPE == ENABLE)
	MEM_RANGE 	kflow_vpe_buf;
#endif
#if (USE_DRE == ENABLE)
	MEM_RANGE 	kflow_dre_buf;
#endif
#if (USE_ISE == ENABLE)
	MEM_RANGE 	kflow_ise_buf;
#endif
	MEM_RANGE 	total_buf;
} VDOPRC_COMMON_MEM;


typedef struct _VDOPRC_VSP_CFG {
	UINT32     blend_frm_num;
	UINT32     dewarp_out_x_ofs_max[VSP_MAX_BLEND_FRAME];
	UINT32     ovlp_width[VSP_MAX_BLEND_FRAME];
	USIZE      scale_out_size[VSP_MAX_BLEND_FRAME];
	UINT32     dre_proc_mode;
	UINT32 	   dre_quality_level;	//min: 0(minimum proc time), max: 6(highest quality)
	ULONG      dre_fusion_tbl_pa[VSP_MAX_BLEND_FRAME];		//layer 0 dre fusion weighting table, ignore this parameters when dre_fusion_tbl_pa_ext != 0
    ULONG      dre_fusion_tbl_pa_ext[VSP_MAX_BLEND_FRAME][DRE_MAX_LAYER_NUM];    ///< the physical address of dre fusion table of each layer
    UINT32 	   dre_fusion_tbl_lofs[VSP_MAX_BLEND_FRAME][DRE_MAX_LAYER_NUM];    ///< lineoffset of dre fusion table of each layer
	ULONG      dre_work_buf_pa;
	ULONG      dre_work_buf_size;
	ISIZE      vpe_out_size;
	UINT32     en_ptz;
	UINT32     dis_fusion;
	UINT32     smart_ver;           ///< smart stitch version
} VDOPRC_VSP_CFG;

typedef struct _VENDOR_VIDEOPROC_DRE_CFG {
	UINT32     dre_proc_mode;       ///< the dre fusion process mode, using enum VENDOR_VIDEOPROC_DRE_PROC_MODE
	UINT32     dre_quality_level;	///< the dre fusion quality , min: 0(minimum proc time, low quality), max: 6(maximum proc time, highest quality)
	ULONG      dre_fusion_tbl_pa;   ///< the physical address of dre fusion table
    ULONG      dre_fusion_tbl_pa_ext[DRE_MAX_LAYER_NUM];   ///< the physical address of dre fusion table of each layer
    UINT32 	   dre_fusion_tbl_lofs[DRE_MAX_LAYER_NUM];    ///< lineoffset of dre fusion table of each layer
	ULONG      dre_work_buf_pa;     ///< the physical address of dre working buffer
	ULONG      dre_work_buf_size;   ///< the size of dre working buffer
	URECT      in_crop[DRE_MAX_BLEND_FRAME];//crop for input image, if needed
} VDOPRC_DRE_CFG;

typedef struct _VDOPRC_VSP_BUF_INFO {
	UINT32     buf_id;
	ULONG      buf_addr;
	VDO_FRAME  keepframe;    // keep for vsp two temp frames
} VDOPRC_VSP_BUF_INFO;

typedef struct _VDOPRC_VSP_CTRL {
	VDOPRC_VSP_STATE     vsp_state;                          //
	UINT32               vpe_in_cancel_job_cnt;
	UINT32               vpe2_total_job_cnt;
	UINT32               vpe2_finish_job_cnt;
	UINT32               vpe2_trigger_job_cnt;
	VDOPRC_VSP_BUF_INFO  buf_info[2];                        // keep for vsp two temp frames
	VDOPRC_VSP_BUF_INFO  s_blend_buf[VSP_MAX_BLEND_FRAME*2]; // the samll blend buffer
	VDOPRC_VSP_BUF_INFO  vpe2_in_buf;
	VDOPRC_VSP_BUF_INFO  vpe2_out_buf;
	UINT32               blend_w[VSP_MAX_BLEND_FRAME];
	ULONG                s_lut2d_sz;
	ULONG                s_lut2d_pa[VSP_MAX_BLEND_FRAME*2];  // the 2dlut of samll blend buffer
	UINT64               timestamp;
	UINT32               alloc_out_buf_cnt;
	UINT32               dre_job_cnt;
} VDOPRC_VSP_CTRL;

typedef struct _VDOPRC_VSP_COMM_CTRL {
	BOOL                 is_processing_vpe2;
	VDOPRC_VSP_CTRL     *vpe2_owner_ctrl;
	vk_spinlock_t        vsp_lock;
	UINT32               prev_free_idx;
	ID                   vsp_sem;
	UINT32               is_start_vsp;
} VDOPRC_VSP_COMM_CTRL;

typedef struct _VDOPRC_VPE_CLEAR_WIN {
	UINT32  color;     ///< rectangle color
	URECT   rect;      ///< coordinate and dimension
} VDOPRC_VPE_CLEAR_WIN;

typedef struct _VDOPRC_IPP_THERMAL_INFO {
	BOOL is_set;     ///< is set or not
	BOOL enable;     ///< thermal mode enable
} VDOPRC_IPP_THERMAL_INFO;

typedef struct _VDOPRC_CONTEXT {
	SEM_HANDLE ISF_VDOPRC_OUTQ_SEM_ID[ISF_VDOPRC_OUT_NUM];
#if (ISF_NEW_PULL == ENABLE)
#else
	SEM_HANDLE ISF_VDOPRC_OUT_SEM_ID[ISF_VDOPRC_OUT_NUM];
#endif
	UINT32 dev;
	ULONG dev_handle; //handle of ctl_ipp
	ULONG dev_handle_ex[VSP_DEV_HANDLE_MAX_NUM]; //handle of extension module, ex: dre, vpe-lite
	UINT32 dev_trigger_open; //before open
	UINT32 dev_trigger_close; //before close
	UINT32 dev_ready; //enable after open, disable before close
/*
	SEM_HANDLE* inq_sem_id;
	SEM_HANDLE* outq_sem_id;
*/
	/*-------------------------------*/
	IPP_EVENT_FP on_input;
	IPP_EVENT_FP on_process;
	IPP_EVENT_FP on_output;
	IPP_EVENT_FP on_osd;
	IPP_EVENT_FP on_mask;
	IPL_SYNC_CB on_sync;
#if defined(_BSP_NA51023_)
	IPL_MD_CB md_cb;
#endif
    void *ai_cb;
    VDOPRC_ISP_AI isp_ai;
	UINT32 isp_ai_effect_is_set;
	UINT32 isp_ai_effect_need_update;
#if defined(_BSP_NA51023_)
	IPL_VIEWTRACKING_CB on_viewtrack;
#endif
	/*-------------------------------*/
	MEM_RANGE mem; //for private buf
	NVTMPP_VB_POOL mempool; //for private buf
	ISF_DATA memblk; //for private buf
	NVTMPP_DDR ddr; //for common buf (output)
	/*-------------------------------*/
	UINT32 cur_mode;
	UINT32 new_mode;
	UINT32 func_allow;
	UINT32 ifunc_allow;
	UINT32 cur_in_cfg_func;
	UINT32 new_in_cfg_func;
	UINT32 ofunc_allow[ISF_VDOPRC_OUT_NUM];

	UINT32 cur_out_cfg_func[ISF_VDOPRC_OUT_NUM];   //after apply.in setmode/runtime end would update from new
	UINT32 new_out_cfg_func[ISF_VDOPRC_OUT_NUM];   //before apply.in setmode/runtime begine would update from user
	UINT32 sleep;
	VDOPRC_CTRL ctrl;
	VDOPRC_IN_PATH in[ISF_VDOPRC_IN_NUM];
	CTL_IPP_OUT_PATH out[ISF_VDOPRC_OUT_NUM];
	UINT32 out_crop_mode[ISF_VDOPRC_OUT_NUM];
#if (USE_OUT_ONEBUF == ENABLE)
	CTL_IPP_OUT_PATH_BUFMODE bufmode[ISF_VDOPRC_PHY_OUT_NUM];
#endif
	VDOPRC_PRE_ROTATE out_rot[ISF_VDOPRC_PHY_OUT_NUM];
	VDOPRC_EXT_CONTEXT out_ext[ISF_VDOPRC_OUT_NUM - ISF_VDOPRC_PHY_OUT_NUM];
	VDOPRC_EXT_CONTEXT out_ext_bak[ISF_VDOPRC_OUT_NUM - ISF_VDOPRC_PHY_OUT_NUM]; // back up for input drop
	ISF_FRC infrc[ISF_VDOPRC_IN_NUM];
	ISF_FRC outfrc[ISF_VDOPRC_OUT_NUM];
	UINT32 nvxcodec; //for yuv-compress
	UINT32 codec[ISF_VDOPRC_OUT_NUM];
	VDOPRC_IN_QUEUE inq;
	VDOPRC_OUT_QUEUE outq;
	VDOPRC_PROCESS proc;
	VDOPRC_PULL_QUEUE pullq;
	VDOPRC_OUT_DBG out_dbg;
	void* p_sem_poll; //for poll_list (shared)
	UINT32 poll_mask; //for poll_list user query
	UINT32 phy_mask; //for poll_list phy-path sync mask
	UINT32 result_mask; //for poll_list result
	UINT32 start_mask; //for poll_list state
#if (USE_IN_DIRECT == ENABLE)
	SIE_UNLOCK_CB sie_unl_cb;
	struct _ISF_UNIT *p_srcunit;
	SEN_USER3_CB sen_user3_cb;
	UINT32 user_crop_trig;
	UINT32 user_crop_param;
	IPP_EVENT_FP sie_sts_cb;
	UINT32 sie_direct_map;
#endif
#if (USE_OUT_ONEBUF == ENABLE)
	UINT32 max_strp_num;
#endif
#if (USE_NEW_SHDR == ENABLE)
	UINT32 shdr_in;
#endif
#if (USE_VPE == ENABLE)
	UINT32 vpe_mode;
#endif
#if (USE_ISE == ENABLE)
	BOOL ise_mode;
#endif

#if (USE_VPE == ENABLE || USE_ISE == ENABLE)
	URECT pre_scl_crop[ISF_VDOPRC_VPE_OUT_NUM];
	URECT hole_region[ISF_VDOPRC_VPE_OUT_NUM];
	URECT clear_region[ISF_VDOPRC_VPE_OUT_NUM];
	UINT32 clear_color[ISF_VDOPRC_VPE_OUT_NUM];
	VDOPRC_SPLIT_INFO split_info[ISF_VDOPRC_OUT_NUM];
#endif
	ISF_VDO_WIN out_win[ISF_VDOPRC_OUT_NUM];
	ULONG user_out_blk[ISF_VDOPRC_OUT_NUM];
	UINT32 user_out_blk_size[ISF_VDOPRC_OUT_NUM];
	UINT32 out_h_align[ISF_VDOPRC_OUT_NUM];
	UINT32 err_cnt;
	UINT32 out_order[ISF_VDOPRC_OUT_NUM];
	UINT32 combine_mode;
	BOOL   slice_isSet;
	VDOPRC_SLICE_MODE_INFO slice_info;
	UINT32 out_loff_align[ISF_VDOPRC_OUT_NUM];
	UINT32 out_vndcfg_func[ISF_VDOPRC_OUT_NUM];
	VDOPRC_VSP_CFG  vsp_cfg;
	VDOPRC_VSP_CTRL vsp_ctrl[VSP_PIPE_NUM];
	VDOPRC_VSP_COMM_CTRL vsp_comm_ctrl;
#if (USE_EIS == ENABLE)
	UINT32 eis_func;
	VDOPRC_EIS_PROC_INFO *p_eis_info_ctx;
	VDOPRC_LAST_LUT2D_INFO last_2dlut;
#endif
	CTL_DRE_WT_CONFIG wt_cfg;
	VDOPRC_IPP_THERMAL_INFO thermal_info;
#ifdef VDOPRC_FUNC_LCA
	UINT32 lca_func;
#endif
#if (CUR_IN_FPS == ENABLE)
	UINT32 cur_in_fps[ISF_VDOPRC_IN_NUM];
#endif
	VDOPRC_ISP_BIND_INFO isp_bind_info;
} VDOPRC_CONTEXT;

typedef struct {
	ULONG ctl_ipp_handle;
	ISIZE img_size;
} _IPP_DS_CB_INPUT_INFO;

typedef struct {
	VDO_FRAME* p_vdoframe;
} _IPP_DS_CB_OUTPUT_INFO;

typedef struct {
	ULONG ctl_ipp_handle;
	ISIZE img_size;
} _IPP_PM_CB_INPUT_INFO;

typedef struct {
	VDO_FRAME* p_vdoframe;
} _IPP_PM_CB_OUTPUT_INFO;


/**
	AI information
*/
typedef struct{
  BOOL   en;
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 path_id;
} VDOPRC_NN_ISP_IN_PARAM;

typedef enum {
	VDOPRC_NN_ISP_OP_QUERY_AI_STATUS = 8, // query ai open      // call before open/start
} VDOPRC_NN_ISP_OPERATION;

typedef INT32 (*VDOPRC_AI_CB)(VDOPRC_NN_ISP_OPERATION op, void *p_in, void *p_out);

extern UINT32 g_vdoprc_max_count;
#define DEV_MAX_COUNT	g_vdoprc_max_count
extern ISF_UNIT *g_vdoprc_list[VDOPRC_MAX_NUM]; //list of all videoproc device
#define DEV_UNIT(did)	g_vdoprc_list[(did)]

#define VENDOR_AI_CFG_ENABLE_CNN	    0x00010000  //ai "1:enable CNN" (default 0)
#define HD_VIDEOPROC_CFG				0x000f0000	//vprc
#define HD_VIDEOPROC_CFG_STRIP_MASK		0x00000007  //vprc stripe rule mask: (default 0)
#define HD_VIDEOPROC_CFG_STRIP_LV1		0x00000000  //vprc "0: GDC,low latency balanced
#define HD_VIDEOPROC_CFG_STRIP_LV2		0x00010000  //vprc "1: low latency Best
#define HD_VIDEOPROC_CFG_STRIP_LV3		0x00020000  //vprc "2: 2D_LUT Best if 2D_LUT supported
#define HD_VIDEOPROC_CFG_STRIP_LV4		0x00030000  //vprc "3: GDC Best,low lantency don't case #define HD_VIDEOPROC_CFG_DISABLE_GDC	HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_STRIPN_MASK	0x00f00000  //vprc force stripe number
#define HD_VIDEOPROC_CFG_STRIP_NUM2		0x00200000  //vprc "0: force 2 stripe
#define HD_VIDEOPROC_CFG_STRIP_NUM3		0x00300000  //vprc "0: force 3 stripe
#define HD_VIDEOPROC_CFG_STRIP_NUM4		0x00400000  //vprc "0: force 4 stripe
#define HD_VIDEOPROC_CFG_STRIP_NUM5		0x00500000  //vprc "0: force 5 stripe
#define HD_VIDEOPROC_CFG_STRIP_NUM6		0x00600000  //vprc "0: force 6 stripe
#define HD_VIDEOPROC_CFG_STRIP_NUM7		0x00700000  //vprc "0: force 7 stripe
#define HD_VIDEOPROC_CFG_STRIP_NUM8		0x00800000  //vprc "0: force 8 stripe
#define HD_VIDEOPROC_CFG_STRIP_NUM9		0x00900000  //vprc "0: force 9 stripe
#define HD_VIDEOPROC_CFG_DISABLE_GDC    HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_LL_FAST		HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_2DLUT_BEST		HD_VIDEOPROC_CFG_STRIP_LV3
#define HD_VIDEOPROC_CFG_GDC_BEST		HD_VIDEOPROC_CFG_STRIP_LV4

extern UINT32 _isf_vdoprc_get_cfg(void);
extern BOOL _isf_vdoprc_is_init(void);

extern ISF_RV _isf_vdoprc_do_command(UINT32 cmd, UINT32 p0, UINT32 p1, UINT32 p2);

//control proc parameters and state
extern ISF_RV _isf_vdoprc_do_bindinput(ISF_UNIT* p_thisunit, UINT32 iport, ISF_UNIT* p_srcunit, UINT32 oport);
extern ISF_RV _isf_vdoprc_do_offsync(ISF_UNIT *p_thisunit, UINT32 oport);
extern ISF_RV _isf_vdoprc_do_setparam(ISF_UNIT *p_thisunit, UINT32 nport, UINT32 param, ULONG value);
extern ULONG _isf_vdoprc_do_getparam(ISF_UNIT *p_thisunit, UINT32 nport, UINT32 param);
extern ISF_RV _isf_vdoprc_do_setparamstruct(ISF_UNIT *p_thisUnit, UINT32 nport, UINT32 param, ULONG* p_struct, UINT32 size);
extern ISF_RV _isf_vdoprc_do_getparamstruct(ISF_UNIT *p_thisUnit, UINT32 nport, UINT32 param, ULONG* p_struct, UINT32 size);
extern ISF_RV _isf_vdoprc_do_updateport(ISF_UNIT *p_thisunit, UINT32 oport, ISF_PORT_CMD cmd);

//input data
extern void _isf_vprc_set_ifunc_allow(ISF_UNIT *p_thisunit);
extern ISF_RV _isf_vprc_check_ifunc(ISF_UNIT *p_thisunit);
extern ISF_RV _vdoprc_max_in(ISF_UNIT *p_thisunit, USIZE* p_size, VDO_PXLFMT* p_fmt);
extern ISF_RV _vdoprc_config_in_crop(ISF_UNIT *p_thisunit, UINT32 iport);
extern ISF_RV _vdoprc_update_in_crop(ISF_UNIT *p_thisunit, UINT32 iport);
extern ISF_RV _vdoprc_config_in_direct(ISF_UNIT *p_thisunit, UINT32 iport);
extern ISF_RV _vdoprc_update_in_direct(ISF_UNIT *p_thisunit, UINT32 iport);
extern void _vdoprc_iport_setqueuecount(ISF_UNIT *p_thisunit, UINT32 count);
extern ISF_RV _isf_vdoprc_iport_alloc(ISF_UNIT *p_thisunit);
extern ISF_RV _isf_vdoprc_iport_free(ISF_UNIT *p_thisunit);

extern ISF_RV _isf_vdoprc_iport_do_push(ISF_UNIT *p_thisunit, UINT32 iport, ISF_DATA *p_data, INT32 wait_ms);
extern void _isf_vdoprc_iport_do_push_fail(ISF_UNIT *p_thisunit, UINT32 iport, UINT32 buf_handle, UINT32 probe, UINT32 r);
extern void _isf_vdoprc_iport_do_proc_cb(ISF_UNIT *p_thisunit, UINT32 iport, UINT32 buf_handle, UINT32 event, INT32 err);
#if (USE_IN_DIRECT == ENABLE)
extern void _isf_vdoprc_iport_do_cap_cb(ISF_UNIT *p_thisunit, UINT32 iport, UINT32 event, void* evt);
#endif
extern void _vdoprc_iport_dropdata(ISF_UNIT *p_thisunit, UINT32 iport, UINT32 buf_handle, UINT32 probe, UINT32 r);
extern void _vdoprc_iport_releasedata(ISF_UNIT *p_thisunit, UINT32 iport, UINT32 buf_handle, UINT32 probe, UINT32 r);

//output data
extern void _isf_vprc_set_ofunc_allow(ISF_UNIT *p_thisunit);
extern ISF_RV _vdoprc_config_ofunc(ISF_UNIT *p_thisunit, BOOL en);
extern ISF_RV _vdoprc_config_out(ISF_UNIT *p_thisunit, UINT32 pid, UINT32 en);
extern ISF_RV _vdoprc_update_out(ISF_UNIT *p_thisunit, UINT32 pid, UINT32 en);
extern void _vdoprc_oport_initqueue(ISF_UNIT *p_thisunit);
extern void _vdoprc_oport_set_enable(ISF_UNIT *p_thisunit, UINT32 oport, UINT32 en);
extern BOOL _vdoprc_oport_is_enable(ISF_UNIT *p_thisunit, UINT32 oport);

#if (USE_VPE == ENABLE)
extern ISF_RV _vdoprc_config_out_vpe(ISF_UNIT *p_thisunit, UINT32 pid, UINT32 en, BOOL runtime_update);
extern void _isf_vdoprc_vpe_oport_do_out_cb(ISF_UNIT *p_thisunit, UINT32 event, void *p_in, void *p_out);
extern ISF_RV _vdoprc_config_vpe_in_crop(ISF_UNIT * p_thisunit, UINT32 iport);

extern void _isf_vdoprc_vpe_iport_do_proc_cb(ISF_UNIT *p_thisunit, UINT32 iport, UINT32 event, void *ctl_vpe_evt);
extern ISF_RV _vdoprc_config_out_dre(ISF_UNIT *p_thisunit, UINT32 pid, UINT32 en, BOOL runtime_update);
extern ISF_RV _vdoprc_config_out_vsp(ISF_UNIT *p_thisunit, UINT32 pid, UINT32 en, BOOL runtime_update);
extern void _isf_vdoprc_vsp_dump_all_info(VDOPRC_CONTEXT* p_ctx);
extern void _isf_vdoprc_vsp_clear_status(ISF_UNIT *p_thisunit, VDOPRC_VSP_CTRL *p_vsp_ctrl);
#endif
#if (USE_DRE == ENABLE)
extern ISF_RV _vdoprc_config_out_dre(ISF_UNIT *p_thisunit, UINT32 pid, UINT32 en, BOOL runtime_update);
extern void _isf_vdoprc_dre_oport_do_out_cb(ISF_UNIT *p_thisunit, UINT32 event, void *p_in, void *p_out);
extern ISF_RV _vdoprc_config_dre_in_crop(ISF_UNIT * p_thisunit, UINT32 iport);
extern void _isf_vdoprc_dre_iport_do_proc_cb(ISF_UNIT *p_thisunit, UINT32 iport, UINT32 buf_handle, UINT32 event, INT32 kr);
#endif
#if (USE_ISE == ENABLE)
extern ISF_RV _vdoprc_config_out_ise(ISF_UNIT *p_thisunit, UINT32 pid, UINT32 en, BOOL runtime_update);
extern void _isf_vdoprc_ise_oport_do_out_cb(ISF_UNIT *p_thisunit, UINT32 event, void *p_in, void *p_out);
extern ISF_RV _vdoprc_config_ise_in_crop(ISF_UNIT * p_thisunit, UINT32 iport);
extern void _isf_vdoprc_ise_iport_do_proc_cb(ISF_UNIT *p_thisunit, UINT32 iport, UINT32 buf_handle, UINT32 event, INT32 kr);
#endif
extern void _isf_vdoprc_push_dummy_queue_for_user_buf(ISF_UNIT *p_thisunit, UINT32 oport, ISF_DATA *p_data);

extern UINT32 _isf_vdoprc_oport_do_new(ISF_UNIT *p_thisunit, UINT32 oport, UINT32 buf_size, UINT32 ddr, ULONG* p_addr);
extern ISF_RV _isf_vdoprc_oport_do_push(ISF_UNIT *p_thisunit, UINT32 oport, UINT32 buf_handle, VDO_FRAME* p_srcvdoframe);
extern int _isf_vdoprc_oport_do_fboot_push(ISF_UNIT *p_thisunit, UINT32 oport, KDRV_IPP_BUILTIN_FRM *p_buildframe);
extern void _isf_vdoprc_oport_do_lock(ISF_UNIT *p_thisunit, UINT32 oport, UINT32 buf_handle);
extern void _isf_vdoprc_oport_do_unlock(ISF_UNIT *p_thisunit, UINT32 oport, UINT32 buf_handle, INT32 err);
extern void _isf_vdoprc_oport_do_proc_begin(ISF_UNIT *p_thisunit, UINT32 oport, UINT32 buf_handle);
extern void _isf_vdoprc_oport_do_proc_end(ISF_UNIT *p_thisunit, UINT32 oport, UINT32 buf_handle);
extern void _isf_vdoprc_oport_do_start(ISF_UNIT *p_thisunit, UINT32 oport);
extern void _isf_vdoprc_oport_do_stop(ISF_UNIT *p_thisunit, UINT32 oport);

//process data
extern void _isf_vprc_set_func_allow(ISF_UNIT *p_thisunit);
extern ISF_RV _isf_vprc_check_func(ISF_UNIT *p_thisunit);

extern void _vdoprc_max_func(ISF_UNIT *p_thisunit, CTL_IPP_FUNC* p_func);
extern ISF_RV _vdoprc_config_func(ISF_UNIT *p_thisunit, BOOL en);
extern ISF_RV _vdoprc_update_func(ISF_UNIT *p_thisunit);

extern void _isf_vdoprc_do_process(ISF_UNIT *p_thisunit, UINT32 event);

extern void _isf_vdoprc_do_input_osd(ISF_UNIT *p_thisunit, UINT32 iport, void *in, void *out);
extern void _isf_vdoprc_finish_input_osd(ISF_UNIT *p_thisunit, UINT32 iport);

extern void _isf_vdoprc_do_input_mask(ISF_UNIT *p_thisunit, UINT32 iport, void *in, void *out);
extern void _isf_vdoprc_finish_input_mask(ISF_UNIT *p_thisunit, UINT32 iport);

extern void _isf_vdoprc_vpe_do_input_mask(ISF_UNIT *p_thisunit, UINT32 iport);

extern void _isf_vdoprc_do_output_osd(ISF_UNIT *p_thisunit, UINT32 oport, void *in, void *out);
extern void _isf_vdoprc_do_output_mask(ISF_UNIT *p_thisunit, UINT32 oport, void *in, void *out);

extern void _vdoprc_set_slice_mode(ISF_UNIT *p_thisunit);
//pull data
extern void _isf_vdoprc_oqueue_do_open(ISF_UNIT *p_thisunit, UINT32 oport);
extern void _isf_vdoprc_oqueue_pre_start(ISF_UNIT *p_thisunit, UINT32 oport);
extern void _isf_vdoprc_oqueue_do_start(ISF_UNIT *p_thisunit, UINT32 oport);
extern void _isf_vdoprc_oqueue_force_stop(ISF_UNIT *p_thisunit, UINT32 oport);
extern void _isf_vdoprc_oqueue_do_stop(ISF_UNIT *p_thisunit, UINT32 oport);
extern void _isf_vdoprc_oqueue_do_close(ISF_UNIT *p_thisunit, UINT32 oport);
extern ISF_RV _isf_vdoprc_oqueue_wait_for_push_in(ISF_UNIT *p_thisunit, UINT32 oport, INT32 wait_ms);
extern ISF_RV _isf_vdoprc_oqueue_do_push(ISF_UNIT *p_thisunit, UINT32 oport, ISF_DATA *p_data);
extern ISF_RV _isf_vdoprc_oqueue_do_pull(ISF_UNIT *p_thisunit, UINT32 oport, ISF_DATA *p_data, INT32 wait_ms);
extern ISF_RV _isf_vdoprc_oqueue_do_push_with_clean(ISF_UNIT *p_thisunit, UINT32 oport, ISF_DATA *p_data, INT32 keep_this);
extern ISF_RV _isf_vdoprc_oqueue_do_push_wait(ISF_UNIT *p_thisunit, UINT32 oport, ISF_DATA *p_data, UINT32 wait_ms);
extern ISF_RV _isf_vdoprc_oqueue_do_poll_list(ISF_UNIT *p_thisunit, VDOPRC_POLL_LIST *p_poll_info);
extern ISF_RV _isf_vdoprc_oqueue_get_poll_mask(ISF_UNIT *p_thisunit, VDOPRC_POLL_LIST *p_poll_info);
extern void _isf_vdoprc_oqueue_cancel_poll(ISF_UNIT *p_thisunit);
extern UINT32 _isf_vdoprc_meta_get_size(VDO_FRAME *frame);
extern UINT32 _isf_vdoprc_meta_copy(VDO_FRAME *pframe_new,VDO_FRAME *pframe_old,ULONG addr,UINT32 dir,IRECT* p_crop);
extern VDO_FRAME *_isf_vdoprc_meta_get_frame(ISF_UNIT *p_thisunit, UINT64 fcnt);
extern void _vdoprc_iport_dumpqueue(ISF_UNIT *p_thisunit, BOOL force_release,UINT32 log_type);

//debug
extern void isf_vdoprc_dump_status(int (*dump)(const char *fmt, ...), ISF_UNIT *p_thisunit);

//extend process
typedef void (*_ISF_EXT_PROCESS_CB)(void);

extern ISF_RV _isf_vdoprc_ext_tsk_open(void);
extern ISF_RV _isf_vdoprc_ext_tsk_close(ISF_UNIT *p_thisunit);
extern ISF_RV _isf_vdoprc_ext_tsk_trigger_proc(ISF_UNIT *p_thisunit, UINT32 ext_pid, UINT32 src_path, ISF_DATA* p_data);


///////////////////////////////////////////////////////////////////////////////
extern void isf_vdoprc_install_id(void) _SECTION(".kercfg_text");
extern void isf_vdoprc_uninstall_id(void) _SECTION(".kercfg_text");

#define ISF_VDOPRC_EXT_TSK_PRI       3
#define ISF_VDOPRC_EXT_TSK_STKSIZE   4096
extern THREAD_HANDLE _SECTION(".kercfg_data") ISF_VDOPRC_EXT_TSK_ID;
extern THREAD_DECLARE(isf_vdoprc_ext_tsk, arglist);

extern UINT32 g_vdoprc_init[ISF_FLOW_MAX];

//extern SEM_HANDLE _SECTION(".kercfg_data") ISF_VDOPRC_IN_SEM_ID[];
extern SEM_HANDLE _SECTION(".kercfg_data") ISF_VDOPRC_OUTP_SEM_ID;
extern SEM_HANDLE _SECTION(".kercfg_data") ISF_VDOPRC_OUTQ_SEM_ID[];
#if (ISF_NEW_PULL == ENABLE)
#else
extern SEM_HANDLE _SECTION(".kercfg_data") ISF_VDOPRC_OUT_SEM_ID[];
#endif
extern SEM_HANDLE _SECTION(".kercfg_data") ISF_VDOPRC_PROC_SEM_ID;

extern ID _SECTION(".kercfg_data") FLG_ID_VDOPRC_EXT;


