/*
    Internal header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _SIE_ENG_INT_H_
#define _SIE_ENG_INT_H_

#include "kwrap/type.h"
#include "kwrap/flag.h"
#include "kwrap/spinlock.h"
#include "kwrap/util.h"

#include "sie_eng.h"
#include "sie_eng_base.h"

/*************************************************************************************
 *  SIE Event Flag Definition
 *************************************************************************************/
#define FLGPTN_SIE_VD                       FLGPTN_BIT(0)
#define FLGPTN_SIE_BP1                      FLGPTN_BIT(1)
#define FLGPTN_SIE_BP2                      FLGPTN_BIT(2)
#define FLGPTN_SIE_BP3                      FLGPTN_BIT(3)
#define FLGPTN_SIE_ACTST                    FLGPTN_BIT(4)
#define FLGPTN_SIE_CRPST                    FLGPTN_BIT(5)
#define FLGPTN_SIE_IN_OUT_ERR               FLGPTN_BIT(6)
#define FLGPTN_SIE_OUT0_END                 FLGPTN_BIT(7)
#define FLGPTN_SIE_OUT1_END                 FLGPTN_BIT(8)
#define FLGPTN_SIE_OUT2_END                 FLGPTN_BIT(9)
#define FLGPTN_SIE_DBGOUT_END               FLGPTN_BIT(11)
#define FLGPTN_SIE_DPCF                     FLGPTN_BIT(13)
#define FLGPTN_SIE_SIECLK_ERR               FLGPTN_BIT(14)
#define FLGPTN_SIE_RAWENC_OVFL              FLGPTN_BIT(15)
#define FLGPTN_SIE_ACTEND                   FLGPTN_BIT(16)
#define FLGPTN_SIE_CRPEND                   FLGPTN_BIT(17)
#define FLGPTN_SIE_BEHAVIOR_ERR             FLGPTN_BIT(18)
#define FLGPTN_SIE_SRC_BUF_OVFL             FLGPTN_BIT(19)  ///< not supported
#define FLGPTN_SIE_HISTO_END                FLGPTN_BIT(20)
#define FLGPTN_SIE_ROI_ACC_END              FLGPTN_BIT(21)  ///< not supported
#define FLGPTN_SIE_RAWENC_FIFO_OVFL         FLGPTN_BIT(22)  ///< not supported
#define FLGPTN_SIE_RAWENC_DATA_OVLAP        FLGPTN_BIT(23)
#define FLGPTN_SIE_RAWENC_DATA_BEHAVIOR_ERR FLGPTN_BIT(24)
#define FLGPTN_SIE_TSEN_OOC_VD              FLGPTN_BIT(25)  ///< only SIE5
#define FLGPTN_SIE_TSEN_END                 FLGPTN_BIT(26)  ///< only SIE5
#define FLGPTN_SIE_TSEN_DRAM_IN3_UDFL       FLGPTN_BIT(27)  ///< only SIE5
#define FLGPTN_SIE_TSEN_FIFO_OVFL           FLGPTN_BIT(28)  ///< only SIE5
#define FLGPTN_SIE_TSEN_CONFIG_END          FLGPTN_BIT(29)  ///< only SIE5, 539A

/*************************************************************************************
 *  SIE Internal Definition
 *************************************************************************************/
#define SIE_CLAMP(_x, _min, _max)   (((_x) >= (_max)) ? (_max) : (((_x) <= (_min)) ? (_min) : (_x)))
#define SIE_DMA_ADDR_LOWORD(x)      ((UINT32)((UINT64)(x)))
#define SIE_DMA_ADDR_HIWORD(x)      ((UINT32)(((UINT64)(x)) >> 32))

typedef struct {
    UINT32  ecs_map_size;
    UINT32  act_sz_x;
    UINT32  act_sz_y;
} SIE_ECS_SCALE_CAL_PARAM;

typedef struct {
    UINT32  in_sz;              ///< input pixel number
    UINT32  out_sz;             ///< output pixel number
    UINT32  lpf;                ///< low-pass valumn, 0~100
    UINT32  binPwr;             ///< binning, 0~2
    BOOL    adaptivelpf_en;     ///< 0: LPF is absolute value; 1: LPF is adaptive value (suggestion: 1)
} SIE_BS_H_ADJ_PARAM;

typedef struct {
    UINT32  in_sz;              ///< input pixel number
    UINT32  out_sz;             ///< output pixel number
    UINT32  lpf;                ///< low-pass valumn, 0~100
    UINT32  binPwr;             ///< binning, 0~2
    BOOL    adaptivelpf_en;     ///< 0: LPF is absolute value; 1: LPF is adaptive value (suggestion: 1)
} SIE_BS_V_ADJ_PARAM;

typedef struct {
    UINT32  irsub_r;
    UINT32  irsub_g;
    UINT32  irsub_b;
} SIE_CA_CALC_DATA;

typedef struct {
    UINT32  satgen_g_last;
    UINT32  satgen_g_cur;
    UINT32  rgb_ir_level_rst;
    UINT32  ir_level_tk_last;
    UINT32  ir_leve_lr_dg_last;
} SIE_IR_CALC_DATA;

/*************************************************************************************
 *  SIE Engine Handle Structure
 *************************************************************************************/
#define SIE_MAKE_HDL_IDX(_pinfo, _chip_id, _eng_id)     (((_chip_id)*(_pinfo->eng_num)) + (_eng_id))

struct sie_eng_handle_t {
    UINT32           index;              ///< engine handle index, create from "SIE_MAKE_HDL_IDX"
    UINT32           chip_id;            ///< engine chip index
    UINT32           eng_id;             ///< engine index of chip
    CHAR             name[16];           ///< engine name
    CHAR             alias_name[16];     ///< engine alias name from kdrv
    vk_spinlock_t    lock;               ///< engine spinlock
    ID               flg_id;             ///< engine event flag
    int              opened;             ///< engine open  flag

    SIE_REG_RW_MODE  reg_rw_mode;        ///< engine register r/w operation mode
    uintptr_t        reg_io_base;        ///< engine register i/o  base, freertos => physical address, linux => virtual address from ioremap
    uintptr_t        reg_buf_base;       ///< engine register dram base, for record register setting to dram
    UINT8            *reg_buf_flag;      ///< engine dram resgiter flag, for record register change or not
    UINT32           irq_id;             ///< engine interrupt number
    void             *private;           ///< engine private data, pointer to "SIE_ENG_DRV_INFO"

    UINT32           clk_rate;           ///< engine operation clock rate, Hz
    SIE_ENG_CLKSRC   clk_src;            ///< engine operation clock source

    void             *sie_clk;           ///< engine clock node for SIE master   clock gating and rate
    void             *sie_bccclk;        ///< engine clock node for SIE bcc      clock source select
    void             *sie_intclk;        ///< engine clock node for SIE internal clock source select
    void             *sie_pxclk;         ///< engine clock node for SIE pixel    clock gating
    void             *sie_pxclkpad;      ///< engine clock node for SIE pixel    clock source select, from pad or SENSOR_MCLK(SIE_MCLK)
    void             *sie_mclk;          ///< engine clock node for SIE patgen   clock gating and rate, from SENSOR_MCLK(SIE_MCLK)
    void             *sie_tsen_rxclk;    ///< engine clock node for SIE t-sensor clock gating and source select(only SIE5)

    SIE_ISR_CB       isr_cb;             ///< engine interrupt callback function

    SIE_CA_CALC_DATA ca_data;
    SIE_IR_CALC_DATA ir_data;

    UINT32           plat_id;            ///< engine platform id,      for function support check
    UINT32           plat_ver;           ///< engine platform version, for function support check
    UINT32           max_clk_rate;       ///< engine platform max clock rate support
};

/*************************************************************************************
 *  SIE Engine Driver Information Structure
 *************************************************************************************/
typedef struct {
    UINT32          chip_num;           ///< maximun chip   count for driver control
    UINT32          eng_num;            ///< maximun engine count for driver control
    UINT32          total_eng;          ///< total engines for driver control
    void            **eng_hdl;          ///< pointer to engine handler
} SIE_ENG_DRV_INFO;

/*************************************************************************************
 *  Public Internal Function Prototype
 *************************************************************************************/
int sie_eng_calc_ecs_scl(SIE_ENG_HANDLE *p_eng, SIE_ECS_SCAL_PARAM *p_scale_map, SIE_ECS_SCALE_CAL_PARAM *p_ref_param);
int sie_eng_calc_bsh_scl(SIE_ENG_HANDLE *p_eng, SIE_BS_H_PARAM *pBshParam, SIE_BS_H_ADJ_PARAM *pBshAdjParam);
int sie_eng_calc_bsv_scl(SIE_ENG_HANDLE *p_eng, SIE_BS_V_PARAM *pBsvParam, SIE_BS_V_ADJ_PARAM *pBsvAdjParam);
int sie_eng_init_rawenc_param(SIE_ENG_HANDLE *p_eng, SIE_RAWENC_PARAM *p_rawenc_param);

#endif  /* _SIE_ENG_INT_H_ */
