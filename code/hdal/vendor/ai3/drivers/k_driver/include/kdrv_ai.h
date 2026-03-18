#ifndef _KDRV_AI_H_
#define _KDRV_AI_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "kwrap/platform.h"
#include "kdrv_type.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define KDRV_AI_JMISP_MAX_FRAME_NUM    6
#define KDRV_AI_JMISP_MAX_PINGPONG_NUM 3
#define KDRV_AI_JMISP_MAX_PATH_NUM     4
#define KDRV_AI_JMISP_MAX_CUST_NUM     2
#define KDRV_AI_JMISP_MAX_PIPEBUFF_NUM 2

/********************************************************************
	TYPE DEFINITION
********************************************************************/

typedef INT32(*KDRV_AI_ISRCB)(UINT32, UINT32, UINT32, UINT32, UINT32, void *);

/**
    AI engine
*/
typedef enum {
	KDRV_AI_ENG_UNKNOWN          = -1,
	KDRV_AI_ENG_CONV1            = 0,
	KDRV_AI_ENG_CONV2            = 1,
	KDRV_AI_ENG_CONV1_LL1        = 1,
	KDRV_AI_ENG_CONV3            = 2,
	KDRV_AI_ENG_CONV4            = 3,
	KDRV_AI_ENG_LSU				 = 4,
	KDRV_AI_ENG_UTIL             = 5,
	KDRV_AI_ENG_ROU              = 6,
	KDRV_AI_ENG_CAL              = 7,
	KDRV_AI_ENG_NUE2             = 8,
	KDRV_AI_ENG_PPU              = 9,
	KDRV_AI_ENG_JOBM_JLA         = 10,
	KDRV_AI_ENG_JOBM_JLB         = 11,
	KDRV_AI_ENG_JOBM_JLC         = 12,
	KDRV_AI_ENG_JOBM_JLD         = 13,
	KDRV_AI_ENG_JOBM_JLE         = 14,
	KDRV_AI_ENG_JMISP_PLA        = 15,
	KDRV_AI_ENG_JMISP_PLB        = 16,
	KDRV_AI_ENG_JMISP_PLC        = 17,
	KDRV_AI_ENG_JMISP_PLD        = 18,
	KDRV_AI_ENG_JOBM_JLA_DBG     = 19,
	KDRV_AI_ENG_JOBM_JLB_DBG     = 20,
	KDRV_AI_ENG_JOBM_JLC_DBG     = 21,
	KDRV_AI_ENG_JOBM_JLD_DBG     = 22,
	KDRV_AI_ENG_JOBM_JLE_DBG     = 23,
	KDRV_AI_ENG_UTIL2            = 24,
	KDRV_AI_ENG_UTIL_LL1         = 24,
	KDRV_AI_ENG_ROU2             = 25,
	KDRV_AI_ENG_ROU_LL1          = 25,
	KDRV_AI_ENG_CAL2             = 26,
	KDRV_AI_ENG_CAL_LL1          = 26,
	KDRV_AI_ENG_POU              = 27, // NT98539a
	KDRV_AI_ENG_ROU3             = 28,
	KDRV_AI_ENG_ROU4             = 29,
	KDRV_AI_ENG_ROU2_LL1         = 29,
	KDRV_AI_ENG_LSU2			 = 30,
	KDRV_AI_ENG_TOTAL,
	ENUM_DUMMY4WORD(KDRV_AI_ENG)
} KDRV_AI_ENG;

typedef enum {
	KDRV_AI_PARAM_UNKNOWN             = -1,
	KDRV_AI_PARAM_JOBM_JLA_PARAM      = 0,
	KDRV_AI_PARAM_JOBM_JLB_PARAM      = 1,
	KDRV_AI_PARAM_JOBM_JLC_PARAM      = 2,
	KDRV_AI_PARAM_JOBM_JLD_PARAM      = 3,
	KDRV_AI_PARAM_JOBM_JLE_PARAM      = 4,	
	KDRV_AI_PARAM_CBFUNC              = 5,	
	KDRV_AI_PARAM_BASE_ADDR           = 6,
	KDRV_AI_PARAM_JOBM_DBG            = 7,
	KDRV_AI_PARAM_JMISP_PARAM         = 8,
	KDRV_AI_PARAM_JMISP_DBG_RUN_SLICE = 9,
	KDRV_AI_PARAM_JMISP_DBG           = 10,
	KDRV_AI_PARAM_JMISP_ADDR_PARAM    = 11,
	KDRV_AI_PARAM_JOBM_BUSY_CNT       = 12,
	KDRV_AI_PARAM_JMISP_INT           = 13,
	KDRV_AI_PARAM_JOBM_ENG_BUSY_CNT   = 14,
	KDRV_AI_PARAM_JOBM_ARB_STATUS     = 15,
	KDRV_AI_PARAM_JOBM_ARB_INFO       = 16,
	KDRV_AI_PARAM_LL_SETTING          = 17,
	KDRV_AI_PARAM_JMISP_FRM_UPD_PARAM = 18,
	KDRV_AI_PARAM_GATING 			  = 19,
	KDRV_AI_PARAM_JMISP_ARB_STATUS    = 20,
	KDRV_AI_PARAM_JMISP_ARB_INFO      = 21,
	KDRV_AI_PARAM_JOBM_ARB2_STATUS    = 22,
	KDRV_AI_PARAM_JOBM_ARB2_INFO      = 23,
	KDRV_AI_PARAM_JOBM_ENG_BUSY_CNT_ARB2 = 24,
	KDRV_AI_PARAM_JOBM_ARB_DBG_CYCLE  = 25,
	KDRV_AI_PARAM_CLK_INFO            = 26,
	KDRV_AI_PARAM_ID_TOTAL,
	ENUM_DUMMY4WORD(KDRV_AI_PARAM_ID)
} KDRV_AI_PARAM_ID;

typedef enum {
	KDRV_AI_JL_UNKNOWN          = -1,
	KDRV_AI_JL_A                = 0,
	KDRV_AI_JL_B                = 1,
	KDRV_AI_JL_C                = 2,
	KDRV_AI_JL_D                = 3,
	KDRV_AI_JL_E                = 4,
	KDRV_AI_JL_TOTAL,
	ENUM_DUMMY4WORD(KDRV_AI_JL_ID)
} KDRV_AI_JL_ID;

typedef enum {
	KDRV_AI_PL_UNKNOWN          = -1,
	KDRV_AI_PL_A                = 0,
	KDRV_AI_PL_B                = 1,
	KDRV_AI_PL_C                = 2,
	KDRV_AI_PL_D                = 3,
	KDRV_AI_PL_TOTAL,
	ENUM_DUMMY4WORD(KDRV_AI_PL_ID)
} KDRV_AI_PL_ID;

typedef enum {
	KDRV_AI_JM_DBG_MODE_UNKNOWN    = -1,
	KDRV_AI_JM_DBG_MODE_NONE       = 0,
	KDRV_AI_JM_DBG_MODE_SINGLE     = 1,
	KDRV_AI_JM_DBG_MODE_ARB        = 2,
	KDRV_AI_JM_DBG_MODE_ARB2       = 3,
	KDRV_AI_JM_DBG_MODE_TOTAL,
	ENUM_DUMMY4WORD(KDRV_AI_JM_DBG_MODE)
} KDRV_AI_JM_DBG_MODE;

typedef enum {
	KDRV_AI_JMISP_DBG_MODE_UNKNOWN    = -1,
	KDRV_AI_JMISP_DBG_MODE_NONE       = 0,
	KDRV_AI_JMISP_DBG_MODE_SINGLE     = 1,
	KDRV_AI_JMISP_DBG_MODE_ARB        = 2,
	KDRV_AI_JMISP_DBG_MODE_TOTAL,
	ENUM_DUMMY4WORD(KDRV_AI_JMISP_DBG_MODE)
} KDRV_AI_JMISP_DBG_MODE;

typedef enum {
	KDRV_AI_JM_DBG_ENG_UNKNOWN    = -1,
	KDRV_AI_JM_DBG_ENG_CONV0      = 0x01,
	KDRV_AI_JM_DBG_ENG_CONV1      = 0x02,
	KDRV_AI_JM_DBG_ENG_CONV2      = 0x04,
	KDRV_AI_JM_DBG_ENG_CONV3      = 0x08,
	KDRV_AI_JM_DBG_ENG_LSU        = 0x10,
	KDRV_AI_JM_DBG_ENG_UTIL       = 0x20,
	KDRV_AI_JM_DBG_ENG_ROU        = 0x40,
	KDRV_AI_JM_DBG_ENG_CAL        = 0x80,
	KDRV_AI_JM_DBG_ENG_ALL        = 0xFF,
	ENUM_DUMMY4WORD(KDRV_AI_JM_DBG_ENG)
} KDRV_AI_JM_DBG_ENG;

typedef enum {
	KDRV_AI_JMISP_DBG_ENG_UNKNOWN    = -1,
	KDRV_AI_JMISP_DBG_ENG_PPU        = 0x01,
	KDRV_AI_JMISP_DBG_ENG_POU        = 0x02,
	KDRV_AI_JMISP_DBG_ENG_ALL        = 0x03,
	ENUM_DUMMY4WORD(KDRV_AI_JMISP_DBG_ENG)
} KDRV_AI_JMISP_DBG_ENG;

typedef enum {
	KDRV_AI_JMISP_HANDSHAKE_CPU        = 0,
	KDRV_AI_JMISP_HANDSHAKE_IPP0_PATH0 = 1,
	KDRV_AI_JMISP_HANDSHAKE_IPP0_PATH1 = 2,
	KDRV_AI_JMISP_HANDSHAKE_IPP0_PATH2 = 3,
	KDRV_AI_JMISP_HANDSHAKE_IPP0_PATH3 = 4,
	KDRV_AI_JMISP_HANDSHAKE_IPP1_PATH0 = 5,
	KDRV_AI_JMISP_HANDSHAKE_IPP1_PATH1 = 6,
	KDRV_AI_JMISP_HANDSHAKE_IPP1_PATH2 = 7,
	KDRV_AI_JMISP_HANDSHAKE_IPP1_PATH3 = 8,
	ENUM_DUMMY4WORD(KDRV_AI_JMISP_HANDSHAKE_TYPE)
} KDRV_AI_JMISP_HANDSHAKE_TYPE;

typedef struct _KDRV_AI_JOBM_PARAM {
	UINT32 jl_no_dispatch_cnt;
	UINT32 jl_priority;
	UINT32 net_id;
} KDRV_AI_JOBM_PARAM;

typedef struct _KDRV_AI_BASE_ADDR_PARAM {
	KDRV_AI_ENG eng;
	UINT32 base_id;
	uintptr_t pa;
} KDRV_AI_BASE_ADDR_PARAM;

typedef struct _KDRV_AI_LL_SETTING_PARAM {
	KDRV_AI_ENG eng;
	UINT32 unlock_cycle;
	UINT32 priority_mode;
	UINT32 qos_tot_time;
	UINT32 qos_ocpy_time;
	UINT32 sta_period_time;
} KDRV_AI_LL_SETTING_PARAM;

typedef struct _KDRV_AI_CBFUNC_PARAM {
	KDRV_AI_ENG   eng;
	KDRV_AI_ISRCB isrcb_fp;
} KDRV_AI_CBFUNC_PARAM;

typedef struct _KDRV_AI_JOBM_DBG_PARAM {
	KDRV_AI_JM_DBG_MODE dbg_mode;
	KDRV_AI_JL_ID       single_jl_id; // only used in single mode
	KDRV_AI_JM_DBG_ENG  arb_eng;      // only used in arb mode
} KDRV_AI_JOBM_DBG_PARAM;

typedef struct _KDRV_AI_JMISP_DBG_PARAM {
	KDRV_AI_JMISP_DBG_MODE dbg_mode;
	KDRV_AI_PL_ID       single_pl_id; // only used in single mode
	KDRV_AI_JMISP_DBG_ENG  arb_eng;      // only used in arb mode
} KDRV_AI_JMISP_DBG_PARAM;

typedef struct _KDRV_AI_JMISP_BLOCK_NUM_INFO {
	UINT32 blk_x_first_num;
	UINT32 blk_x_middle_num;
	UINT32 blk_x_last_num;
	UINT32 blk_y_first_num;
	UINT32 blk_y_middle_num;
	UINT32 blk_y_last_num;
} KDRV_AI_JMISP_BLOCK_NUM_INFO;

typedef struct _KDRV_AI_JMISP_BLOCK_OFS_INFO {
	UINT32 blk_x_first_ofs;  
	UINT32 blk_x_middle_ofs; 
	UINT32 blk_y_first_ofs;  
	UINT32 blk_y_middle_ofs; 
} KDRV_AI_JMISP_BLOCK_OFS_INFO;


typedef struct _KDRV_AI_JMISP_FRAME_INFO {
	UINT32 ring_en;
	UINT64 ring_start_addr;
	UINT64 ring_end_addr;
	UINT64 frame_addr;
	UINT32 first_stripe_x_ofs;  
	UINT32 middle_stripe_x_ofs; 	
	UINT32 first_slice_y_ofs;   
	UINT32 middle_slice_y_ofs;  	
	KDRV_AI_JMISP_BLOCK_OFS_INFO blk_ofs_info;
} KDRV_AI_JMISP_FRAME_INFO;

typedef struct _KDRV_AI_JMISP_PINGPONG_INFO {
	UINT64 pingpong_addr0;
	UINT64 pingpong_addr1;
	KDRV_AI_JMISP_BLOCK_OFS_INFO blk_ofs_info;
} KDRV_AI_JMISP_PINGPONG_INFO;

typedef struct _KDRV_AI_JMISP_PATH_INFO {
	KDRV_AI_JMISP_FRAME_INFO     frame[KDRV_AI_JMISP_MAX_FRAME_NUM];
	KDRV_AI_JMISP_PINGPONG_INFO  pingpong[KDRV_AI_JMISP_MAX_PINGPONG_NUM];
	KDRV_AI_JMISP_HANDSHAKE_TYPE handshake;
	UINT32 stripe_num;
	UINT32 slice_num;
	UINT32 signal_mode_en;
	KDRV_AI_JMISP_BLOCK_NUM_INFO blk_num_info;
	UINT32 path_id;
    UINT32 cust[KDRV_AI_JMISP_MAX_CUST_NUM];
	UINT32 ppu_pipebuf_lsbaddr[KDRV_AI_JMISP_MAX_PIPEBUFF_NUM];
	UINT32 blk_skip;
	UINT32 wait_en;
} KDRV_AI_JMISP_PATH_INFO;

typedef struct _KDRV_AI_JMISP_FRAME_ADDR_INFO {
	UINT64 frame_addr[KDRV_AI_JMISP_MAX_FRAME_NUM];
	UINT32 path_id;
} KDRV_AI_JMISP_FRAME_ADDR_INFO;

typedef struct _KDRV_AI_JOBM_BUSY_CNT_INFO {
	UINT32 busy_cnt[KDRV_AI_ENG_TOTAL];
} KDRV_AI_JOBM_BUSY_CNT_INFO;

typedef struct _KDRV_AI_JOBM_SINGLE_BUSY_CNT_INFO {
	KDRV_AI_JM_DBG_ENG eng;
	UINT32 busy_cnt;
} KDRV_AI_JOBM_SINGLE_BUSY_CNT_INFO;

typedef struct _KDRV_AI_JMISP_INT_INFO {
	UINT8 pl_disable[KDRV_AI_JMISP_MAX_PATH_NUM];
} KDRV_AI_JMISP_INT_INFO;

typedef struct _KDRV_AI_JMISP_FRM_UPD_INFO {
	UINT32 ring_en;
	UINT64 ring_start_addr;
	UINT64 ring_end_addr;
	UINT64 frame_addr;
	KDRV_AI_JMISP_BLOCK_OFS_INFO blk_ofs_info;
} KDRV_AI_JMISP_FRM_UPD_INFO;

typedef struct _KDRV_AI_JMISP_FRM_UPD_PATH_INFO {
    KDRV_AI_JMISP_FRM_UPD_INFO   frame[KDRV_AI_JMISP_MAX_FRAME_NUM];
    KDRV_AI_JMISP_PINGPONG_INFO  pingpong[KDRV_AI_JMISP_MAX_PINGPONG_NUM];
    UINT32 cust[KDRV_AI_JMISP_MAX_CUST_NUM];
    UINT32 ppu_pipebuf_lsbaddr[KDRV_AI_JMISP_MAX_PIPEBUFF_NUM];
	UINT32 path_id;
} KDRV_AI_JMISP_FRM_UPD_PATH_INFO;
// ----------------- FOR NNISP interface -----------------
#define KDRV_AI_JMISP_TEST 1
typedef enum{
  KDRV_AI_NNISP_FMT_Y8 = 0,
  KDRV_AI_NNISP_FMT_BAYER,
  KDRV_AI_NNISP_FMT_YUV,
  KDRV_AI_NNISP_FMT_UNKNOWN,
  ENUM_DUMMY4WORD(KDRV_AI_NNISP_FMT)
} KDRV_AI_NNISP_FMT; 


typedef struct _KDRV_AI_JMISP_FUNC_INFO {
	UINT64 frame_y_addr;
	UINT64 frame_uv_addr;
	UINT64 frame_texture_addr;
	UINT64 out_pingpong_y_addr;
	UINT64 out_pingpong_uv_addr;
	UINT64 out_pingpong_texture_addr;
	UINT64 in_ring_y_start_addr;
	UINT64 in_ring_y_end_addr;
	UINT64 in_ring_uv_start_addr;
	UINT64 in_ring_uv_end_addr;
	UINT32 max_out_lofs;
	UINT32 max_out_height;
	UINT32 isp_id;
	UINT32 stripe_num;
	UINT32 slice_num;
	UINT32 signal_mode_en;
	UINT32 path_id;
	KDRV_AI_NNISP_FMT img_fmt;
} KDRV_AI_JMISP_FUNC_INFO;

typedef struct _KDRV_AI_JMISP_JOB_INFO {
	KDRV_AI_JMISP_FUNC_INFO func_info[KDRV_AI_JMISP_MAX_PATH_NUM];
	UINT32 func_num;
} KDRV_AI_JMISP_JOB_INFO;

typedef struct _KDRV_AI_JOBM_ARB_STATUS {
	BOOL arb_status[KDRV_AI_ENG_TOTAL];
} KDRV_AI_JOBM_ARB_STATUS;

typedef struct _KDRV_AI_JOBM_ARB_INFO {
	KDRV_AI_ENG eng;
	UINT32 arb_jl_idx;
	UINT32 arb_dispatch_sel;
	UINT32 arb_net_id;
	UINT64 arb_job_addr;
} KDRV_AI_JOBM_ARB_INFO;

typedef struct _KDRV_AI_JOBM_ARB_CYCLE {
	KDRV_AI_ENG eng;
	UINT32 eng_cycle[8];
	UINT32 arb_id;
} KDRV_AI_JOBM_ARB_CYCLE;

typedef struct _KDRV_AI_GATING_INFO {
	UINT32 flag;
	UINT32 engine_index;
} KDRV_AI_GATING_INFO;

typedef struct _KDRV_AI_CLK_INFO {
	KDRV_AI_ENG eng;
	UINT32 clk_rate;
} KDRV_AI_CLK_INFO;
/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
/*!
 * @fn INT32 kdrv_ai_init(VOID)
 * @brief initial hardware engine
 * @return return 0 on success, others on error
 */
extern INT32 kdrv_ai_init(VOID);

/*!
 * @fn INT32 kdrv_ai_uninit(VOID)
 * @brief uninitial hardware engine
 * @return return 0 on success, others on error
 */
extern INT32 kdrv_ai_uninit(VOID);

/*!
 * @fn INT32 kdrv_ai_set(UINT32 id, KDRV_AI_PARAM_ID param_id, VOID* p_param)
 * @brief set parameters to kdrv ai
 * @param param_id  the id of parameters
 * @param p_param   the parameters
 * @return return 0 on success, others on error
 */
extern INT32 kdrv_ai_set(KDRV_AI_PARAM_ID param_id, VOID *p_param);

/*!
 * @fn INT32 kdrv_ai_get(UINT32 id, KDRV_AI_PARAM_ID param_id, VOID* p_param)
 * @brief get parameters from kdrv ai
 * @param param_id  the id of parameters
 * @param p_param   the parameters
 * @return return 0 on success, others on error
 */
extern INT32 kdrv_ai_get(KDRV_AI_PARAM_ID param_id, VOID *p_param);

/*!
 * @fn INT32 kdrv_ai_trigger(KDRV_AI_ENG eng, uintptr_t pa)
 * @brief trigger hardware engine
 * @param eng               the engine id
 * @param pa                the physical address of LL/JL/PL
 * @return return 0 on success, others on error
 */
extern INT32 kdrv_ai_trigger(KDRV_AI_ENG eng, uintptr_t pa);
//extern INT32 kdrv_ai_trigger_isr(KDRV_AI_ENG eng, uintptr_t pa);

/*!
 * @fn INT32 kdrv_ai_jm_dbg_run_next(KDRV_AI_ENG eng, uintptr_t pa)
 * @brief run next step in jobm debug mode
 * @param jm_dbg_param      jobm debug parameter
 * @return return 0 on success, others on error
 */
extern INT32 kdrv_ai_jm_dbg_run_next(KDRV_AI_JOBM_DBG_PARAM jm_dbg_param);
extern INT32 kdrv_ai_jmisp_dbg_run_next(KDRV_AI_JMISP_DBG_PARAM jmisp_dbg_param);
extern UINT32 kdrv_ai_get_eng_caps(KDRV_AI_ENG eng);
extern INT32 kdrv_ai_dma_abort(KDRV_AI_ENG eng);
extern INT32 kdrv_ai_engine_reset(KDRV_AI_ENG ai_eng);
extern const CHAR *kdrv_ai_get_version(VOID);
#if KDRV_AI_JMISP_TEST
extern INT32 set_jmisp_parm(VOID* param, uintptr_t workbuf_va, uintptr_t workbuf_pa);
extern UINT32 get_jmisp_workbuf_size(UINT32 max_lofs, UINT32 max_height);
#endif
#endif
