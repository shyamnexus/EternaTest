/*
    Public header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _SIE_ENG_H_
#define _SIE_ENG_H_

#include "kwrap/type.h"
#include "sie_eng_handle.h"

/*************************************************************************************
 *  SIE Driver Version Definition
 *************************************************************************************/
#define SIE_ENG_DRV_VERSION         "0.02.05"

/*************************************************************************************
 *  SIE Register Number Definition
 *************************************************************************************/
#define SIE_ENG_REG_NUM             (0xC00>>2)

/*************************************************************************************
 *  SIE Engine Index Definition
 *************************************************************************************/
typedef enum {
    SIE_ENG_ID_1  = 0,              ///< SIE1
    SIE_ENG_ID_2,                   ///< SIE2
    SIE_ENG_ID_3,                   ///< SIE3
    SIE_ENG_ID_4,                   ///< SIE4
    SIE_ENG_ID_5,                   ///< SIE5
    SIE_ENG_ID_MAX
} SIE_ENG_ID_T;

/*************************************************************************************
 *  SIE Engine Clock Source Definition (SIEx_CLK)
 *************************************************************************************/
typedef enum {
    SIE_ENG_CLKSRC_CURR = 0,        ///< clock source as current setting(no setting, no changing)
    SIE_ENG_CLKSRC_480,
    SIE_ENG_CLKSRC_PLL17,           ///< only 539A, SIE1/2 supported
    SIE_ENG_CLKSRC_PLL5,
    SIE_ENG_CLKSRC_PLL13,
    SIE_ENG_CLKSRC_PLL12,
    SIE_ENG_CLKSRC_320,
    SIE_ENG_CLKSRC_192,
    SIE_ENG_CLKSRC_PLL10,
    SIE_ENG_CLKSRC_MAX
} SIE_ENG_CLKSRC;

#define SIE_ENG_CLKSRC_RSVD1        SIE_ENG_CLKSRC_PLL17

/*************************************************************************************
 *  SIE Pixel Clock Source Definition (SIEx_PXCLK), not supported
 *************************************************************************************/
typedef enum {
    SIE_ENG_PXCLKSRC_OFF = 0,       ///< SIE pixel-clock disabled
    SIE_ENG_PXCLKSRC_PAD,           ///< SIE pixel-clock enabled, source as Parallel-Pad (external), only SIE1/3/5 support
    SIE_ENG_PXCLKSRC_MCLK,          ///< SIE pixel-clock enabled, source as SENSOR_MCLK  (internal), not supported
    SIE_ENG_PXCLKSRC_MAX
} SIE_ENG_PXCLKSRC;

/*************************************************************************************
 *  SIE BCC Clock Source Definition, not supported
 *************************************************************************************/

typedef enum {
    SIE_ENG_BCCCLKSRC_CURR = 0,     ///< clock source as current setting(no setting, no changing)
    SIE_ENG_BCCCLKSRC_480,
    SIE_ENG_BCCCLKSRC_MAX
} SIE_ENG_BCCCLKSRC;

/*************************************************************************************
 *  SIE Internal Clock Source Definition (SIEx_INTCLK)
 *************************************************************************************/
typedef enum {
    SIE_ENG_INTCLKSRC_CURR = 0,     ///< clock source as current setting(no setting, no changing)
    SIE_ENG_INTCLKSRC_SIE1,         ///< engine clock from SIE1_CLK, available for SIE2
    SIE_ENG_INTCLKSRC_SIE2,
    SIE_ENG_INTCLKSRC_SIE3,         ///< engine clock from SIE3_CLK, available for SIE4
    SIE_ENG_INTCLKSRC_SIE4,
    SIE_ENG_INTCLKSRC_SIE5,
    SIE_ENG_INTCLKSRC_MAX
} SIE_ENG_INTCLKSRC;

/*************************************************************************************
 *  SIE SENSOR MClock Source Definition (SENSOR_MCLKx)
 *************************************************************************************/
typedef enum {
    SIE_ENG_MCLK_ID_1  = 0,         ///< SENSOR_MCLK1
    SIE_ENG_MCLK_ID_2,              ///< SENSOR_MCLK2
    SIE_ENG_MCLK_ID_3,              ///< SENSOR_MCLK3
    SIE_ENG_MCLK_ID_4,              ///< SENSOR_MCLK4
    SIE_ENG_MCLK_ID_MAX
} SIE_ENG_MCLK_ID_T;

typedef enum {
    SIE_ENG_MCLKSRC_CURR = 0,       ///< SENSOR_MCLK source as current setting(no setting, no changing)
    SIE_ENG_MCLKSRC_480,
    SIE_ENG_MCLKSRC_PLL5,
    SIE_ENG_MCLKSRC_PLL6,
    SIE_ENG_MCLKSRC_PLL12,
    SIE_ENG_MCLKSRC_MAX
} SIE_ENG_MCLKSRC;

/*************************************************************************************
 *  SIE T-SENSOR RX Clock Source Definition (only SIE5)
 *************************************************************************************/
typedef enum {
    SIE_ENG_TSEN_RXCLKSRC_CURR = 0,
    SIE_ENG_TSEN_RXCLKSRC_PXCLK,            ///< from sensor pixel clock
    SIE_ENG_TSEN_RXCLKSRC_SN_MCLK3,         ///< SENSOR_MCLK3, loopback from sensor mclk
    SIE_ENG_TSEN_RXCLKSRC_MAX
} SIE_ENG_TSEN_RXCLKSRC;

/*************************************************************************************
 *  SIE Shared SRAM Clock Source Definition (only SIE5)
 *************************************************************************************/
typedef enum {
    SIE_ENG_SHARED_SRAM_CLKSRC_CURR = 0,    ///< no setting, no changing
    SIE_ENG_SHARED_SRAM_CLKSRC_SIE,
    SIE_ENG_SHARED_SRAM_CLKSRC_VIE,
    SIE_ENG_SHARED_SRAM_CLKSRC_MAX
} SIE_ENG_SHARED_SRAM_CLKSRC;

/*************************************************************************************
 *  SIE Interrupt & Event Definition
 *************************************************************************************/
#define SIE_INT_VD                          0x00000001
#define SIE_INT_BP1                         0x00000002
#define SIE_INT_BP2                         0x00000004
#define SIE_INT_BP3                         0x00000008
#define SIE_INT_ACTST                       0x00000010
#define SIE_INT_CRPST                       0x00000020
#define SIE_INT_DRAM_IN_OUT_ERR             0x00000040
#define SIE_INT_DRAM_OUT0_END               0x00000080
#define SIE_INT_DRAM_OUT1_END               0x00000100
#define SIE_INT_DRAM_OUT2_END               0x00000200
#define SIE_INT_DRAM_DBGOUT_END             0x00000800
#define SIE_INT_DPCF                        0x00002000
#define SIE_INT_SIECLK_ERR                  0x00004000
#define SIE_INT_RAWENC_OVFL                 0x00008000
#define SIE_INT_ACTEND                      0x00010000
#define SIE_INT_CROPEND                     0x00020000
#define SIE_INT_BEHAVIOR_ERR                0x00040000
#define SIE_INT_SRC_BUF_OVFL                0x00080000     ///< not supported
#define SIE_INT_HISTO_END                   0x00100000
#define SIE_INT_ROI_ACC_END                 0x00200000     ///< not supported
#define SIE_INT_RAWENC_FIFO_OVFL            0x00400000     ///< not supported
#define SIE_INT_RAWENC_DATA_OVLAP           0x00800000     ///< BCC share and process data overlap
#define SIE_INT_RAWENC_DATA_BEHAVIOR_ERR    0x01000000     ///< BCC input data behavior does not meet expectations
#define SIE_INT_TSEN_OOC_VD                 0x02000000     ///< Thermal sensor VD of OOC data, only SIE5
#define SIE_INT_TSEN_END                    0x04000000     ///< Thermal sensor tx cfg or ooc end in single mode, tx ooc end in combine mode, only SIE5
#define SIE_INT_TSEN_DRAM_IN3_UDFL          0x08000000     ///< Thermal sensor ooc data dram in3 underflow, only SIE5
#define SIE_INT_TSEN_FIFO_OVFL              0x10000000     ///< Thermal sensor module clock < rx clock,     only SIE5
#define SIE_INT_TSEN_CONFIG_END             0x20000000     ///< Thermal sensor tx cfg end in combine mode,  only SIE5, 539A
#define SIE_INT_ALL                         0x3f97ebff

typedef enum  {
    SIE_ENG_EVENT_VD                        = 0x00000001,  ///< enable interrupt: vd
    SIE_ENG_EVENT_BP1                       = 0x00000002,  ///< enable interrupt: bp1
    SIE_ENG_EVENT_BP2                       = 0x00000004,  ///< enable interrupt: bp2
    SIE_ENG_EVENT_BP3                       = 0x00000008,  ///< enable interrupt: bp3
    SIE_ENG_EVENT_ACTST                     = 0x00000010,  ///< enable interrupt: active start
    SIE_ENG_EVENT_CRPST                     = 0x00000020,  ///< enable interrupt: crop start
    SIE_ENG_EVENT_DRAM_IN_OUT_ERR           = 0x00000040,  ///< enable interrupt: dram in/out error
    SIE_ENG_EVENT_DRAM_OUT0_END             = 0x00000080,  ///< enable interrupt: out0 dram end
    SIE_ENG_EVENT_DRAM_OUT1_END             = 0x00000100,  ///< enable interrupt: out1 dram end
    SIE_ENG_EVENT_DRAM_OUT2_END             = 0x00000200,  ///< enable interrupt: out2 dram end
    SIE_ENG_EVENT_DRAM_DBGOUT_END           = 0x00000800,  ///< enable interrupt: debug out dram end
    SIE_ENG_EVENT_DPCF                      = 0x00002000,  ///< enable interrupt: dpc fail
    SIE_ENG_EVENT_SIECLK_ERR                = 0x00004000,  ///< enable interrupt: sie module clock error
    SIE_ENG_EVENT_RAWENC_OUTOVFL            = 0x00008000,  ///< enable interrupt: bayer compression overflow
    SIE_ENG_EVENT_ACTEND                    = 0x00010000,  ///< enable interrupt: active end
    SIE_ENG_EVENT_CRPEND                    = 0x00020000,  ///< enable interrupt: crop end
    SIE_ENG_EVENT_BEHAVIOR_ERR              = 0x00040000,  ///< enable interrupt: behavior error (should be VD -> DRAM END -> VD...)
    SIE_ENG_EVENT_SRC_BUF_OVFL              = 0x00080000,  ///< enable interrupt: source input fifo overflow
    SIE_ENG_EVENT_HISTO_END                 = 0x00100000,  ///< enable interrupt: histogram Y result done
    SIE_ENG_EVENT_ROI_ACC_END               = 0x00200000,  ///< enable interrupt: ROI accumulation result done
    SIE_ENG_EVENT_RAWENC_FIFO_OVFL          = 0x00400000,  ///< enable interrupt: bayer compression fifo overflow
    SIE_ENG_EVENT_RAWENC_DATA_OVLAP         = 0x00800000,  ///< enable interrupt: bayer compression data overlap
    SIE_ENG_EVENT_RAWENC_DATA_BEHAVIOR_ERR  = 0x01000000,  ///< enable interrupt: bayer compression input data behavior does not meet expectations
    SIE_ENG_EVENT_TSEN_OOC_VD               = 0x02000000,  ///< enable interrupt: thermal sensor VD of OOC data, only SIE5
    SIE_ENG_EVENT_TSEN_END                  = 0x04000000,  ///< enable interrupt: thermal sensor tx cfg or ooc end in single mode, tx ooc end in combine mode, only SIE5
    SIE_ENG_EVENT_TSEN_DRAM_IN3_UDFL        = 0x08000000,  ///< enable interrupt: thermal sensor ooc data dram in3 underflow, only SIE5
    SIE_ENG_EVENT_TSEN_FIFO_OVFL            = 0x10000000,  ///< enable interrupt: thermal sensor module clock < rx clock,     only SIE5
    SIE_ENG_EVENT_TSEN_CONFIG_END           = 0x20000000,  ///< enable interrupt: thermal sensor tx cfg end in combine mode,  only SIE5, 539A
    SIE_ENG_EVENT_ALL                       = 0x3f97ebff,  ///< enable interrupt: all
} SIE_ENG_INT_EVENT;

typedef struct {
    BOOL    act_en;
    BOOL    act2_en;                        ///< not supported
} SIE_START_CFG;

typedef struct {
    BOOL    dram_in1_udfl;
    BOOL    dram_in2_udfl;
    BOOL    dram_out0_lnbufovfl;
    BOOL    dram_out1_lnbufovfl;
    BOOL    dram_out2_lnbufovfl;
    BOOL    dram_out3_lnbufovfl;            ///< not supported
    BOOL    dram_out0_lnbufsizerr;
    BOOL    dram_out3_lnbufsizerr;          ///< not supported
} SIE_ENGINE_STATUS_INFO;

typedef struct {
    UINT32                  chip_id;        ///< engine chip index
    UINT32                  eng_id;         ///< engine index of chip, reference to SIE_ENG_ID_T
    SIE_ENGINE_STATUS_INFO  status;
} SIE_ENGINE_STATUS_CB;

/*************************************************************************************
 *  SIE Engine Initial Parameter Definition
 *************************************************************************************/
typedef enum {
    SIE_REG_RW_MODE_BUFFERED = 0,           ///< register r/w in dram base
    SIE_REG_RW_MODE_DIRECT,                 ///< register r/w in i/o  base
    SIE_REG_RW_MODE_MAX
} SIE_REG_RW_MODE;

typedef void (*SIE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
    UINT32          chip_id;                ///< engine chip index, start from 0
    UINT32          eng_id;                 ///< engine index of chip, reference to SIE_ENG_ID_T
    CHAR            name[16];               ///< engine name

    UINT32          clock_rate;             ///< engine master clock rate, Hz
    SIE_ENG_CLKSRC  clock_src;              ///< engine master clock source, reference to SIE_ENG_CLKSRC

    void            *sie_clk;               ///< engine clock node for master   clock source and gating and rate
    void            *sie_bccclk;            ///< engine clock node for bcc      clock source, set to NULL if not used
    void            *sie_intclk;            ///< engine clock node for internal clock source, set to NULL if not used
    void            *sie_pxclk;             ///< engine clock node for pixel    clock gating, set to NULL if not used
    void            *sie_pxclkpad;          ///< engine clock node for pixel    clock source, from PAD or SENSOR_MCLK(SIE_MCLK)
    void            *sie_mclk;              ///< engine clock node for patgen   clock source and gating and rate, from SENSOR_MCLK#1~4(SIE_MCLK#1~4), set to NULL if not used
    void            *sie_tsen_rxclk;        ///< engine clock node for SIE t-sensor clock gating and source select(only SIE5), set to NULL if not used

    SIE_REG_RW_MODE reg_rw_mode;            ///< engine register r/w operation mode
    uintptr_t       reg_io_base;            ///< engine register i/o  base, freertos => physical address, linux => virtual address from ioremap
    uintptr_t       p_sie_reg_st;           ///< engine register dram base, for record register setting to dram, set to 0 if SIE_REG_RW_MODE_DIRECT
    UINT8           *p_sie_reg_chg_flag;    ///< engine dram resgiter flag, for record register change or not, set to NULL if SIE_REG_RW_MODE_DIRECT
    UINT32          irq_id;                 ///< engine interrupt number

    SIE_ISR_CB      isr_cb;                 ///< engine interrupt callback function
} SIE_ENG_INIT_PARAM;

/*************************************************************************************
 *  Public Function Prototype
 *************************************************************************************/
void sie_eng_set_dbg_level(int level);      ///< for driver global debug level, 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
int  sie_eng_get_dbg_level(void);
void sie_eng_set_isr_dbg_level(int level);  ///< for dirver isr    debug level, 0=disable 1=enable
int  sie_eng_get_isr_dbg_level(void);

int  sie_eng_init(UINT32 chip_num, UINT32 eng_num);
void sie_eng_release(void);

int  sie_eng_init_resource(SIE_ENG_INIT_PARAM *param);
void sie_eng_release_resource(UINT32 chip_id, UINT32 eng_id);
int  sie_eng_reg_isr_callback(SIE_ENG_HANDLE *p_eng, SIE_ISR_CB cb);
void sie_eng_isr_hw_reg(SIE_ENG_HANDLE *p_eng);

int  sie_eng_open(SIE_ENG_HANDLE *p_eng);
int  sie_eng_close(SIE_ENG_HANDLE *p_eng);
int  sie_eng_trig_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_START_CFG *trig_cfg);
int  sie_eng_stop_hw_reg(SIE_ENG_HANDLE *p_eng);

int  sie_eng_fastboot_open(SIE_ENG_HANDLE *p_eng);

int  sie_eng_set_clock(SIE_ENG_HANDLE *p_eng, SIE_ENG_CLKSRC clk_src, UINT32 clk_rate);
int  sie_eng_set_bccclock(SIE_ENG_HANDLE *p_eng, SIE_ENG_BCCCLKSRC bccclk_src);
int  sie_eng_set_pxclock(SIE_ENG_HANDLE *p_eng, SIE_ENG_PXCLKSRC pxclk_src);
int  sie_eng_set_intclock(SIE_ENG_HANDLE *p_eng, SIE_ENG_INTCLKSRC intclk_src);
int  sie_eng_set_mclock(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id, SIE_ENG_MCLKSRC mclk_src, UINT32 clk_rate, BOOL mclk_en);
int  sie_eng_set_tsen_rxclock(SIE_ENG_HANDLE *p_eng, SIE_ENG_TSEN_RXCLKSRC clk_src, BOOL clk_en);

int  sie_eng_set_clock_auto_gating(SIE_ENG_HANDLE *p_eng, BOOL enb);
int  sie_eng_get_clock_auto_gating(SIE_ENG_HANDLE *p_eng);              ///< 0:disable 1:enable, < 0:error

int  sie_eng_wait_event(SIE_ENG_HANDLE *p_eng, SIE_ENG_INT_EVENT WaitEvent, BOOL bClrFlag);
int  sie_eng_wait_event_timeout(SIE_ENG_HANDLE *p_eng, SIE_ENG_INT_EVENT WaitEvent, BOOL bClrFlag, int timeout_ms);

int    sie_eng_write_hw_reg(SIE_ENG_HANDLE *p_eng, UINT32 reg_ofs, UINT32 val);
UINT32 sie_eng_read_hw_reg(SIE_ENG_HANDLE *p_eng, UINT32 reg_ofs);

UINT32 sie_eng_get_max_clock_rate(UINT32 chip_id, UINT32 eng_id);

#endif  /* _SIE_ENG_H_ */
