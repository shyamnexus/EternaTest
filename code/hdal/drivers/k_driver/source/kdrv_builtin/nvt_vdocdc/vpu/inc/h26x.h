#ifndef _H26x_H_
#define _H26x_H_

#include "kwrap/type.h"
#include "kwrap/perf.h"
#include "h26x_def.h"

#ifdef __FREERTOS
#include "plat/dma.h"
#include "dma_protected.h"
#endif

#define H26X_FINISH_INT			0x00000001  /* Picture Finish Interrupt. */
#define H26X_BSDMA_INT			0x00000010	/* BSDAM empty interrupt */
#define H26X_ERR_INT			0x00000100	/* Error interrupt */
#define H26X_TIME_OUT_INT		0x00000200	/* Timeout interrupt */
#define H26X_FRAME_TIME_OUT_INT	0x00000400	/* Frame Timeout interrupt */
#define H26X_BSOUT_INT			0x00001000	/* Bitstream buffer full interrupt */
#define H26X_FBC_ERR_INT		0x00100000	/* FBC decompression error interrupt */
#define H26X_SRC_DECMP_ERR_INT  0x01000000  /* source decompression error interrupt */
#define H26X_SRC_D2D_OV_INT  	0x02000000  /* source low latency overwrite interrupt */
#define H26X_SWRST_FINISH_INT	0x10000000	/* swrest finish interrupt */
#define H26X_MB_DBUG_INT		0x20000000	/* MB debug interrupt */
#define H26X_SLICE_DONE_INT		0x40000000	/* slice finish interrupt */
#define H26X_DMACH_DIS_INT		0x80000000	/* dma channel disable finish interrupt */
#define H26X_POLLING_MODE_ERR_INT			0x00002000	/* sw hack interrupt */


// APB set //
#define H26X_REG_BASIC_START_OFFSET			0x20
#define H26X_REG_BASIC_FINISH_OFFSET		0x3D0
#define H26X_REG_BASIC_2_START_OFFSET		0x3D4
#define H26X_REG_BASIC_2_FINISH_OFFSET		0x7A4
#define H26X_REG_BASIC_3_START_OFFSET		0x7A8
#define H26X_REG_BASIC_3_FINISH_OFFSET		0xBA0
#define H26X_REG_BASIC_4_START_OFFSET		0xBA4
#define H26X_REG_BASIC_4_FINISH_OFFSET		0xCE4
#define H26X_REG_REPORT_START_OFFSET		0xD14 //0x250
#define H26X_REG_REPORT_FINISH_OFFSET		0xEA8 // include SHA_resturn_state //
#define H26X_REG_REPORT2_START_OFFSET		0x3B4 // MD_STAT //
#define H26X_REG_REPORT2_FINISH_OFFSET		0x3D0

#define H26X_REG_STABLE_LEN_START_OFFSET	0xE58	// stable length, shall not using in link-list //
#define H26X_REG_STABLE_LEN_FINISH_OFFSET	0xE5C

// APB size //
#define H26X_REG_BASIC_SIZE					(((H26X_REG_BASIC_FINISH_OFFSET - H26X_REG_BASIC_START_OFFSET)>>2) + 1)
#define H26X_REG_BASIC_2_SIZE				(((H26X_REG_BASIC_2_FINISH_OFFSET - H26X_REG_BASIC_2_START_OFFSET)>>2) + 1)
#define H26X_REG_BASIC_3_SIZE				(((H26X_REG_BASIC_3_FINISH_OFFSET - H26X_REG_BASIC_3_START_OFFSET)>>2) + 1)
#define H26X_REG_BASIC_4_SIZE				(((H26X_REG_BASIC_4_FINISH_OFFSET - H26X_REG_BASIC_4_START_OFFSET)>>2) + 1)
#define H26X_REG_REPORT_SIZE				(((H26X_REG_REPORT_FINISH_OFFSET - H26X_REG_REPORT_START_OFFSET)>>2) + 1)
#define H26X_REG_REPORT2_SIZE				(((H26X_REG_REPORT2_FINISH_OFFSET - H26X_REG_REPORT2_START_OFFSET)>>2) + 1)

// link-list mode //
#define H26X_MAX_DUMMY_WRITE				(0x10)
#define H26X_REPORT_BUF_SIZE				(SIZE_256X(H26X_REG_REPORT_SIZE<<2))
#define H26X_REPORT2_BUF_SIZE				(SIZE_256X(H26X_REG_REPORT2_SIZE<<2))

typedef enum _H26xStatus{
    H26X_STATUS_ERROR_SRC_BUF_IS_NOT_USED = -12,  /*!< Current source buffer is not used. */
    H26X_STATUS_ERROR_YADDR_INVALID       = -11,  /*!< Luma address of current picture is invalid. */
    H26X_STATUS_ERROR_DISPLAY_FULL        = -10,  /*!< All display-buffer is full. */
    H26X_STATUS_ERROR_DISPLAY_EMPTY       = -9,   /*!< There is no extra buffer to be displayed. */
    H26X_STATUS_ERROR_REC_BUF_FULL        = -8,   /*!< All recontruction buffer is full. */
    H26X_STATUS_ERROR_REGOBJ_EMPTY        = -7,   /*!< There is no register object for h26x to process. */
    H26X_STATUS_ERROR_REGOBJ_FULL         = -6,
    H26X_STATUS_ERROR_UNLOCK              = -5,   /*!< h26x module unlock error. */
    H26X_STATUS_ERROR_LOCK                = -4,   /*!< h26x module lock error. */
    H26X_STATUS_ERROR_CLOSE               = -3,   /*!< h26x module close error. */
    H26X_STATUS_ERROR_OPEN                = -2,   /*!< h26x module open error. */
    H26X_STATUS_ERROR                     = -1,   /*!< Error. */
    H26X_STATUS_OK                        = 0,    /*!< OK. */
    ENUM_DUMMY4WORD(H26xStatus),
}H26xStatus;

typedef enum _H26XREPORT_{
	H26X_CYCLE_CNT = 0,
	H26X_REC_CHKSUM,
	H26X_REPORT_RES_D1C,
	H26X_REPORT_RES_D20,
	H26X_TNR_OUT_Y_CHKSUM,
	H26X_SCD_REPORT,
	H26X_BS_LEN,
	H26X_BS_CHKSUM,
	H26X_QP_CHKSUM,
	H26X_ILF_DIS_CTB,
	H26X_RRC_BIT_LEN,
	H26X_RRC_RDOPT_COST_LSB,
	H26X_RRC_RDOPT_COST_MSB,
	H26X_RRC_SIZE,
	H26X_RRC_FRM_COST_LSB,
	H26X_RRC_FRM_COST_MSB,
	H26X_RRC_FRM_COMPLEXITY_LSB,
	H26X_RRC_FRM_COMPLEXITY_MSB,
	H26X_RRC_SSE_DIST_LSB,
	H26X_RRC_SSE_DIST_MSB,
	H26X_RRC_QP_SUM,
	H26X_RRC_COEFF,
	H26X_MD_MOT_CNT,
	H26X_SRAQ_ISUM_ACT_LOG,
	H26X_PSNR_FRM_Y_LSB,
	H26X_PSNR_FRM_Y_MSB,
	H26X_PSNR_FRM_U_LSB,
	H26X_PSNR_FRM_U_MSB,
	H26X_PSNR_FRM_V_LSB,
	H26X_PSNR_FRM_V_MSB,
	H26X_PSNR_ROI_Y_LSB,
	H26X_PSNR_ROI_Y_MSB,
	H26X_PSNR_ROI_U_LSB,
	H26X_PSNR_ROI_U_MSB,
	H26X_PSNR_ROI_V_LSB,
	H26X_PSNR_ROI_V_MSB,
	H26X_PSNR_MOT_Y_LSB,
	H26X_PSNR_MOT_Y_MSB,
	H26X_PSNR_MOT_U_LSB,
	H26X_PSNR_MOT_U_MSB,
	H26X_PSNR_MOT_V_LSB,
	H26X_PSNR_MOT_V_MSB,
	H26X_REPORT_RES_DBC,
	H26X_REPORT_RES_DC0,
	H26X_REPORT_RES_DC4,
	H26X_REPORT_RES_DC8,
	H26X_REPORT_RES_DCC,
	H26X_REPORT_RES_DD0,
	H26X_IME_CHKSUM_LSB,
	H26X_IME_CHKSUM_MSB,
	H26X_EC_CHKSUM,
	H26X_SIDE_INFO_CHKSUM,
	H26X_ROI_CNT,
    H26X_CRC_Y_HIT_SUM,
    H26X_CRC_C_HIT_SUM,
    H26X_EC_REAL_LEN_SUM,
    H26X_EC_SKIP_LEN_SUM,
    H26X_MOTION_NUM,
    H26X_BGR_NUM,
	H26X_STATS_INTER_CNT,
	H26X_STATS_SKIP_CNT,
	H26X_STATS_MERGE_CNT,
	H26X_STATS_IRA4_CNT,
	H26X_STATS_IRA8_CNT,
	H26X_STATS_IRA16_CNT,
	H26X_STATS_IRA32_CNT,
	H26X_STATS_CU64_CNT,
	H26X_STATS_CU32_CNT,
	H26X_STATS_CU16_CNT,
	H26X_STATS_SCD_INTER_CNT,
	H26X_STATS_SCD_IRANG_CNT,
	H26X_SRC_Y_CHKSUM,
	H26X_SRC_C_CHKSUM,
	H26X_SPN_Y_CHKSUM,
	H26X_SPN_C_CHKSUM,
    H26X_JND_Y_CHKSUM,
	H26X_JND_C_CHKSUM,
    H26X_MASK_Y_CHKSUM,
	H26X_MASK_C_CHKSUM,
    H26X_OSG_Y_CHKSUM,
	H26X_OSG_C_CHKSUM,
    H26X_STABLE_BS_LEN,
	H26X_STABLE_SLICE_NUM,
    H26X_CHK_SLICE_NO,
	H26X_CHK_SLICE_LEN,
	H26X_JND_GRAD_LSB,
	H26X_JND_GRAD_CNT,
	H26X_JND_GRAD_MSB,
    H26X_MD_STAT_0,		// H26X_CHK_TURBO_SKIP_TH
	H26X_MD_STAT_1,		// H26X_TURBO_NZ_MV_CNT
	H26X_MD_STAT_2,		// H26X_RES_E7C
	H26X_MD_STAT_3,		// H26X_RES_E80
	H26X_SLICE_BS_LEN_WO_DUMMY,
	H26X_SLICE_BS_LEN_W_DUMMY,
	H26X_RET_MAX
}H26XREPORT;

typedef struct _H26XRegSet{
	UINT32 INT_EN;					// 0x020
	UINT32 SRC_Y_ADDR;				// 0x024
	UINT32 SRC_C_ADDR;				// 0x028
	UINT32 REC_Y_ADDR;				// 0x02C
	UINT32 REC_C_ADDR;				// 0x030
	UINT32 REF_Y_ADDR;				// 0x034
	UINT32 REF_C_ADDR;				// 0x038
	UINT32 TNR_OUT_Y_ADDR;			// 0x03C
	UINT32 TNR_OUT_C_ADDR;			// 0x040
	UINT32 COL_MVS_WR_ADDR;			// 0x044
	UINT32 COL_MVS_RD_ADDR;			// 0x048
	UINT32 SIDE_INFO_WR_ADDR_0;		// 0x04C
	UINT32 SIDE_INFO_RD_ADDR_0;		// 0x050
	UINT32 BSOUT_BUF_ADDR_1;		// 0x054
	UINT32 BSOUT_BUF_SIZE_1;		// 0x058
	UINT32 RRC_WR_ADDR;				// 0x05C
	UINT32 RRC_RD_ADDR;				// 0x060
	UINT32 QP_MAP_ADDR;				// 0x064
	UINT32 NAL_LEN_OUT_ADDR;		// 0x068
	UINT32 BSDMA_CMD_BUF_ADDR;		// 0x06C
	UINT32 BSOUT_BUF_ADDR_0;		// 0x070
	UINT32 DEC_CFG[2];				// 0x074 ~ 0x078
	UINT32 BSOUT_BUF_SIZE_0;		// 0x07C
	UINT32 NAL_HDR_TOTAL_LEN;		// 0x080
	UINT32 SRC_LINE_OFFSET;			// 0x084
	UINT32 REC_LINE_OFFSET;			// 0x088
	UINT32 TNR_OUT_LINE_OFFSET;		// 0x08C
	UINT32 SIDE_INFO_LINE_OFFSET;	// 0x090
	UINT32 QP_MAP_LINE_OFFSET;		// 0x094
	UINT32 SLICE_SET_CFG;			// 0x098
	UINT32 TIMEOUT_CNT_MAX;			// 0x09C
	UINT32 FUNC_CFG[2];				// 0x0A0 ~ 0x0A4
	UINT32 SEQ_CFG[2];				// 0x0A8 ~ 0x0AC
	UINT32 RMD_CFG_1;				// 0x0B0
	UINT32 PIC_CFG;					// 0x0B4
	UINT32 QP_CFG[2];				// 0x0B8 ~ 0x0BC
	UINT32 ILF_CFG[2];				// 0x0C0 ~ 0x0C4
	UINT32 AEAD_CFG;				// 0x0C8
	UINT32 DSF_CFG;					// 0x0CC
	UINT32 GDR_CFG[2];				// 0x0D0 ~ 0x0D4
	UINT32 FRO_CFG[21];				// 0x0D8 ~ 0x128
	UINT32 RRC_CFG[18];				// 0x12C ~ 0x170
	UINT32 SRAQ_CFG[18];			// 0x174 ~ 0x1B8
	UINT32 LPM_CFG;					// 0x1BC
	UINT32 RMD_CFG;					// 0x1C0
	UINT32 RND_CFG[2];				// 0x1C4 ~ 0x1C8
	UINT32 VAR_CFG[2];				// 0x1CC ~ 0x1D0
	UINT32 IME_CFG;					// 0x1D4
	UINT32 SCD_CFG;					// 0x1D8
	UINT32 RDO_CFG_5;				// 0x1DC
	UINT32 ROI_CFG[21];				// 0x1E0 ~ 0x230
	UINT32 RDO_CFG[5];				// 0x234 ~ 0x244
	UINT32 MAQ_CFG;				    // 0x248
	UINT32 PAR_FRM_SKIP;			// 0x24C
	UINT32 JND_CFG[4];				// 0x250 ~ 0x25C
	UINT32 SLICE_CFG[2];			// 0x260 ~ 0x264
	UINT32 ESKIP_CFG[2];			// 0x268 ~ 0x26C
	UINT32 RES_SUP_CFG[3];			// 0x270 ~ 0x278
	UINT32 GMV_CFG[8];				// 0x27C ~ 0x298
	UINT32 MOTION_BIT_ADDR[3]; 		// 0x29C ~ 0x2A4
	UINT32 MOTION_BIT_WR_ADDR;		// 0x2A8
	UINT32 MOTION_DET_CFG;			// 0x2AC
	UINT32 SHA_INIT[8]; 		    // 0x2B0 ~ 0x2CC
	UINT32 MF_CFG;       		    // 0x2D0
	UINT32 MOTION_BIT_RD_ADDR;		// 0x2D4
	UINT32 TURBO_CFG[3];			// 0x2D8 ~ 0x2E0
	UINT32 RES_2E4_2EC[3]; 		    // 0x2E0 ~ 0x2EC
	UINT32 TILE_CFG;				// 0x2F0
	UINT32 TILE_WIDTH_CFG;			// 0x2F4
	UINT32 TILE_SI_OFFSET;			// 0x2F8
	UINT32 TILE_SI_WR_ADDR;			// 0x2FC
	UINT32 TILE_SI_RD_ADDR;			// 0x300
	UINT32 TILE_EXT_WR_Y_ADDR[2];	// 0x304 ~ 0x308
	UINT32 TILE_EXT_WR_C_ADDR[2];	// 0x30C ~ 0x310
	UINT32 TILE_EXT_RD_Y_ADDR[2];	// 0x314 ~ 0x318
	UINT32 TILE_EXT_RD_C_ADDR[2];	// 0x31C ~ 0x320
	UINT32 LMTBL_CFG[26];			// 0x324 ~ 0x388
	UINT32 SLMTBL_CFG[18];			// 0x38C ~ 0x3D0
	UINT32 GMV_REPORT[40];			// 0x3D4 ~ 0x470
	UINT32 RES_474_47C[3];			// 0x474 ~ 0x47C
	UINT32 SDE_CFG[49]; 		    // 0x480 ~ 0x540
	UINT32 SPN_CFG[16];				// 0x544 ~ 0x580
	UINT32 MASK_CFG[137];			// 0x584 ~ 0x7A4
	UINT32 OSG_CFG[314];			// 0x7A8 ~ 0xC8C
	UINT32 WTMK_CFG[17];			// 0xC90 ~ 0xCD0
	UINT32 MSB_CFG[5];				// 0xCD4 ~ 0xCE4
	UINT32 RES_CE8_CFC[6];			// 0xCE8 ~ 0xCFC
	UINT32 DBG_CFG;					// 0xD00
	UINT32 DBG_REPORT[3];			// 0xD04 ~ 0xD0C
	UINT32 DEB_CFG;				    // 0xD10
	UINT32 CHK_REPORT[H26X_RET_MAX];// 0xD14 ~ 0xE88
	UINT32 SHA_RET[8]; 		    	// 0xE8C ~ 0xEA8
#ifdef VDOCDC_EMU
	UINT32 DUMMY_RES[405];			// 0xEAC ~ 0x14FC
	UINT32 ISP_CFG[8];				// 0x1500 ~ 0x151C
#endif
}H26XRegSet;

// attach system function //
extern void h26x_powerOn(void);
extern void h26x_powerOff(void);
extern H26xStatus h26x_open(UINT32 chip_idx, UINT32 clock, uintptr_t reg_va, uintptr_t rst_va, void *venc_clk, void *ae_clk);
extern H26xStatus h26x_close(UINT32 chip_idx);
extern void h26x_resetINT(UINT32 chip_idx);
extern UINT32 h26x_waitINT(UINT32 chip_idx);
extern UINT32 h26x_checkINT(UINT32 chip_idx);
extern void h26x_resetHW(UINT32 chip_idx);
extern void h26x_polling_reset(UINT32 chip_idx);
extern void h26x_module_reset(UINT32 chip_idx);
extern void h26x_isr(UINT32 chip_idx);
extern UINT32 h26x_exit_isr(UINT32 chip_idx);
extern void h26x_resetIntStatus(void);
extern void h26x_setWakeUpSRAM(UINT32 chip_idx);
extern H26xStatus h26x_lock(UINT32 chip_idx);
extern H26xStatus h26x_unlock(UINT32 chip_idx);

// set hw function //
extern void h26x_setClk(UINT32 chip_idx, UINT32 h26x_clk);
extern BOOL h26x_reset(UINT32 chip_idx);
extern void h26x_reset2(UINT32 chip_idx, UINT32 mask_en);
extern void h26x_polling_reset(UINT32 chip_idx);
extern void h26x_start(UINT32 chip_idx);
extern void h26x_start_disdma(UINT32 chip_idx);
extern void h26x_setDramBurstLen(UINT32 chip_idx, UINT32 uiSel);
extern void h26x_setBsDmaEn(UINT32 chip_idx);
extern void h26x_setBsOutEn(UINT32 chip_idx);
extern UINT32 h26x_setDmaChDis(UINT32 chip_idx, BOOL enable, int wait_int);
extern void h26x_setChkSumEn(UINT32 chip_idx, BOOL enable);
extern void h26x_setNextBsBuf(UINT32 chip_idx, uintptr_t uiAddr, UINT32 uiSize, uintptr_t uiAddr_2, UINT32 uiSize_2);
extern void h26x_setNextBsDmaBuf(UINT32 chip_idx, uintptr_t uiAddr);
extern void h26x_setBsLen(UINT32 chip_idx, UINT32 uiSize);
extern void h26x_setUnLock(UINT32 chip_idx);
extern void h26x_setLock(UINT32 chip_idx);
extern void h26x_clearIntStatus(UINT32 chip_idx, UINT32 uiVal);
extern void h26x_setSRAMMode(UINT32 chip_idx, UINT32 uiCycle, UINT32 uiClkRdyChk, UINT32 uiLightSleepEn, UINT32 uiSleepDownEn);

// get hw function //
extern UINT32 h26x_getClk(UINT32 chip_idx);
extern UINT32 h26x_getDramBurstLen(UINT32 chip_idx);
extern UINT32 h26x_getIntEn(UINT32 chip_idx);
extern UINT32 h26x_getIntStatus(UINT32 chip_idx);
extern UINT32 h26x_getHwRegSize(void);
extern UINT32 h26x_getDummyWTSize(void);

// prepare trigger hw encode register //
extern void h26x_setIntEn(UINT32 chip_idx, UINT32 uiVal);
extern void h26x_setRegSet(UINT32 chip_idx, uintptr_t uiAPBAddr);
extern void h26x_setEncDirectRegSet(UINT32 chip_idx, uintptr_t uiAPBAddr);
extern void h26x_setEncLLRegSet(UINT32 chip_idx, UINT32 uiJobNum, uintptr_t uiJob0APBAddr);
extern void h26x_setLLDummyWriteNum(UINT32 chip_idx, UINT32 uiVal);
extern void h26x_setHWTimeoutCnt(UINT32 uiVal);
// get hw status //
extern BOOL h26x_getBusyStatus(UINT32 chip_idx);
extern BOOL h26x_getBusyStatus2(UINT32 chip_idx);
extern void h26x_getEncReport(UINT32 chip_idx, UINT32 uiEncReport[H26X_RET_MAX]);
extern void h26x_getSHAReport(UINT32 chip_idx, UINT32 uiShaReport[8]);
extern void h26x_getMDStat(UINT32 chip_idx, UINT32 uiMDStat[8]);
extern UINT32 h26x_getCurJobNum(UINT32 chip_idx);

// debug tools //
extern void h26x_prtMem(uintptr_t uiMemAddr,UINT32 uiMemLen);
extern void h26x_prtReg(UINT32 chip_idx);
extern void h26x_prtReg2(UINT32 chip_idx, UINT32 offset, UINT32 size);
//extern void h26x_prtDebug(void); // debug 370 need to modify //
extern UINT32 h26x_getDbg1(UINT32 chip_idx, UINT32 uiSel);
extern UINT32 h26x_getDbg2(UINT32 chip_idx, UINT32 uiSel);
extern UINT32 h26x_getDbg3(UINT32 chip_idx, UINT32 uiSel);
extern void h26x_getDebug(UINT32 chip_idx);

// get physical address //
extern uintptr_t h26x_getPhyAddr(uintptr_t uiAddr);
extern uintptr_t h26x_getVirAddr(uintptr_t uiAddr);
extern void h26x_flushCache(uintptr_t uiAddr, UINT32 uiSize);
extern void h26x_cache_clean(uintptr_t uiAddr, UINT32 uiSize);
extern void h26x_cache_invalidate(uintptr_t uiAddr, UINT32 uiSize);
extern void h26x_setBSDMA(uintptr_t uiBSDMAAddr, UINT32 uiHwHeaderNum, uintptr_t uiHwHeaderAddr, UINT32 *uiHwHeaderSize);
extern void h26x_setBSDMA2(uintptr_t uiBSDMAAddr, UINT32 uiHwHeaderNum, uintptr_t uiHwHeaderAddr, UINT32 TotalSize);
extern void h26x_setBsOutAddr(UINT32 chip_idx, uintptr_t uiAddr, UINT32 uiSize, uintptr_t uiAddr_2, UINT32 uiSize_2);
extern UINT32 h26x_getBsOutAddr(UINT32 chip_idx);
extern UINT32 h26x_getTmnrSumY(UINT32 chip_idx);
extern UINT32 h26x_getTmnrSumC(UINT32 chip_idx);
extern UINT32 h26x_getMaskSumY(UINT32 chip_idx);
extern UINT32 h26x_getMaskSumC(UINT32 chip_idx);
extern UINT32 h26x_getOsgSumY(UINT32 chip_idx);
extern UINT32 h26x_getOsgSumC(UINT32 chip_idx);
extern void h26x_setDebugSel(UINT32 chip_idx, UINT32 uiVal370,UINT32 uiVal374,UINT32 uiVal378);
extern UINT32 h26x_getCheckSumSelResult(UINT32 chip_idx);
extern void h26x_setUnitChecksum(UINT32 chip_idx, UINT32 uiVal);
extern UINT32 h26x_getUnitChecksum(UINT32 chip_idx);
extern void h26x_setCodecClock(UINT32 chip_idx, BOOL enable);
extern void h26x_setCodecPClock(UINT32 chip_idx, BOOL enable);
extern UINT32 h26x_getBslen(UINT32 chip_idx);
//extern UINT32 h26x_getChipId(void);
extern void h26x_tick_open(void);
extern void h26x_tick_close(void);
extern VOS_TICK h26x_tick_result(void);
extern UINT32 h26x_getNalLen(UINT32 chip_idx, UINT32 uiSel);
extern UINT32 h26x_getQpSum(UINT32 chip_idx);
extern UINT32 h26x_getRecSum(UINT32 chip_idx);

// for kdrv //
extern void h26x_create_resource(UINT32 chip_idx);
extern void h26x_release_resource(UINT32 chip_idx);

// for efuse check //
BOOL h26x_efuse_check(UINT32 uiWidth, UINT32 uiHeight);

extern UINT32 h26x_getIsrStatus(void);
#if defined (__FREERTOS)
extern void h26x_request_irq(void);
#endif

extern UINT32 h26x_getStableSliceNum(UINT32 chip_idx);
extern UINT32 h26x_getStableLen(UINT32 chip_idx);

extern UINT32 h26x_getCtrl(UINT32 chip_idx);
extern UINT32 get_h26x_finish(void);
extern void h26x_get_utilization(UINT32 chip_idx, UINT32 *util_rate, UINT32 *fps);
#ifdef VDOCDC_EMU
extern void h26x_reset_emu(UINT32 chip_idx);
extern void h26x_dmach_dis_emu(UINT32 chip_idx);
extern void h26x_reset_emu2(UINT32 chip_idx);
extern void h26x_reset_emu2_ll(UINT32 chip_idx);
extern void h26x_polling_reset_emu_ll(UINT32 chip_idx);
extern void h26x_setLLCRegSet(UINT32 chip_idx, UINT32 uiJobNum, uintptr_t uiJob0APBAddr);
#endif

extern void h26x_enableClk(UINT32 chip_idx);
extern UINT32 h26x_getSliceBsLenwoDummy(UINT32 chip_idx);
extern UINT32 h26x_getSliceBsLenwiDummy(UINT32 chip_idx);
extern void h26x_setSliceSetEn(UINT32 chip_idx, UINT32 idx);
extern void h26x_setSliceSetDummyWT(UINT32 chip_idx, UINT32 uiNum);
extern void h26x_setSliceIntEn(UINT32 chip_idx, BOOL enable);
extern void h26x_setAxiBrstMax(UINT32 chip_idx, UINT32 uiSRC, UINT32 uiREF, UINT32 uiREC, UINT32 uiOSG);

extern void h26x_prtRegforCmp(UINT32 chip_idx);
extern BOOL h26x_chkCycleCnt(UINT32 chip_idx);
extern void h26x_setRecOutEn(UINT32 chip_idx, UINT32 enable);

extern BOOL h26x_ChkBusy2(UINT32 chip_idx);
extern void h26x_setSliceSetCtrlEn(UINT32 chip_idx);
extern void h26x_init_wrapper(UINT32 chip_idx);
extern void h26x_setPatchCount(UINT32 chip_idx, UINT32 cnt);
extern UINT32 h26x_getPatchCount(UINT32 chip_idx);

#endif	//_H26x_H_

