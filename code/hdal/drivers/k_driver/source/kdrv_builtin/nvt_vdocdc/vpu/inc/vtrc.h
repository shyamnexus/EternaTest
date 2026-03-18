#ifndef _VTRC_H_
#define _VTRC_H_

#define VTRC_1S0_BSDMA_VALID_BIT	0
#define VTRC_1S1_BSDMA_VALID_BIT	1
#define VTRC_2S0_BSDMA_VALID_BIT	8
#define VTRC_1S0_BSOUT_VALID_BIT	16
#define VTRC_1S1_BSOUT_VALID_BIT	17
#define VTRC_2S0_BSOUT_VALID_BIT	24

#define VTRC_1S1_BSDMA_END_INT			0x00000002
#define VTRC_2S_BSDMA_END_INT			0x00000100
#define VTRC_1S0_BSOUT_INT		    	0x00010000
#define VTRC_1S1_BSOUT_INT		    	0x00020000
#define VTRC_2S_BSOUT_INT		    	0x01000000
#define VTRC_TRANSCODER_INT		    	0x02000000
#define VTRC_ERROR_INT		    	    0x04000000
#define VTRC_TIME_OUT_INT	    	    0x08000000
#define VTRC_FRAME_TIME_OUT_INT	    	0x10000000

#define VTRC_1S0_BSDMA_END_INT			0x00000001
#define VTRC_1S1_BSDMA_END_INT			0x00000002
#define VTRC_2S_BSDMA_END_INT			0x00000100
#define VTRC_1S0_BSOUT_INT		    	0x00010000
#define VTRC_1S1_BSOUT_INT		    	0x00020000
#define VTRC_2S_BSOUT_INT		    	0x01000000
#define VTRC_TRANSCODER_INT		    	0x02000000
#define VTRC_ERROR_INT		    	    0x04000000
#define VTRC_TIME_OUT_INT	    	    0x08000000
#define VTRC_FRAME_TIME_OUT_INT	    	0x10000000

#define VTRC_INIT_INT_STATUS			(0xFFFFFFFF)

// euum of AVC_TILE_APB need to modify at nt98690 //
typedef enum _AVC_TILE_APB_{
	AVC_TILE_CTRL,				         // 0x000
    AVC_TILE_VALID,                      // 0x004
    AVC_TILE_INT_FLAG,                   // 0x008
    AVC_TILE_INT_EN,			         // 0x00C
	AVC_TILE_SRAM,			             // 0x010
	AVC_TILE_FRAME_TIMEOUT,  	         // 0x014
	AVC_TILE_MB_TIMEOUT,			     // 0x018
    AVC_TILE_S0_BSDMA_CMD_BUF_ADDR,      // 0x01C
	AVC_TILE_S1_BSDMA_CMD_BUF_ADDR,	 	 // 0x020
	AVC_TILE_S2_BSDMA_CMD_BUF_ADDR,	     // 0x024
	AVC_TILE_S0_BSOUT_BUF_ADDR,			 // 0x028
	AVC_TILE_S1_BSOUT_BUF_ADDR, 		 // 0x02C
	AVC_TILE_S2_BSOUT_BUF_ADDR,			 // 0x030
	AVC_TILE_NAL_LEN_DUMP_ADDR,          // 0x034
	AVC_TILE_S0_BSOUT_BUF_SIZE,			 // 0x038
	AVC_TILE_S1_BSOUT_BUF_SIZE,			 // 0x03C
	AVC_TILE_S2_BSOUT_BUF_SIZE,			 // 0x040
	AVC_TILE_S0_NAL_HEADER_TOTAL_LEN,	 // 0x044
	AVC_TILE_S1_NAL_HEADER_TOTAL_LEN,	 // 0x048
	AVC_TILE_S2_NAL_HEADER_TOTAL_LEN,	 // 0x04C
	AVC_TILE_PIC,				         // 0x050
	AVC_TILE_SLICE,			        	 // 0x054
	AVC_TILE_FRM_WIDTH,			         // 0x058
	AVC_TILE_TILE_WIDTH,			     // 0x05C
	AVC_TILE_AEAD,		                 // 0x060
    AVC_TILE_SHA_INIT_0,                 // 0x064
    AVC_TILE_SHA_INIT_1,                 // 0x068
    AVC_TILE_SHA_INIT_2,                 // 0x06C
    AVC_TILE_SHA_INIT_3,                 // 0x070
    AVC_TILE_SHA_INIT_4,                 // 0x074
    AVC_TILE_SHA_INIT_5,                 // 0x078
    AVC_TILE_SHA_INIT_6,                 // 0x07C
    AVC_TILE_SHA_INIT_7,                 // 0x080
    AVC_TILE_SHA_REPORT_0,               // 0x084
    AVC_TILE_SHA_REPORT_1,               // 0x088
    AVC_TILE_SHA_REPORT_2,               // 0x08C
    AVC_TILE_SHA_REPORT_3,               // 0x090
    AVC_TILE_SHA_REPORT_4,               // 0x094
    AVC_TILE_SHA_REPORT_5,               // 0x098
    AVC_TILE_SHA_REPORT_6,               // 0x09C
    AVC_TILE_SHA_REPORT_7,               // 0x0A0
	AVC_TILE_CYCLE,      				 // 0x0A4
	AVC_TILE_CHECK_NAL_0,      		     // 0x0A8
	AVC_TILE_CHECK_NAL_1,      		     // 0x0AC
	AVC_TILE_PASS2_ENC_BS_LEN,			 // 0x0B0
	AVC_TILE_PASS2_ENC_BS_CHKSUM,		 // 0x0B4
	AVC_TILE_DBG_0,			             // 0x0B8
	AVC_TILE_DBG_1,			             // 0x0BC
}AVC_TILE_APB;

typedef struct _VTRCRegSet{
	UINT32 CTRL;				        // 0x000
    UINT32 VALID;                      // 0x004
    UINT32 INT_FLAG;                   // 0x008
    UINT32 INT_EN;			        	// 0x00C
	UINT32 SRAM;			            // 0x010
	UINT32 FRAME_TIMEOUT;  	        // 0x014
	UINT32 MB_TIMEOUT;			        // 0x018
    UINT32 S0_BSDMA_CMD_BUF_ADDR;      // 0x01C
	UINT32 S1_BSDMA_CMD_BUF_ADDR;	 	// 0x020
	UINT32 S2_BSDMA_CMD_BUF_ADDR;	    // 0x024
	UINT32 S0_BSOUT_BUF_ADDR;			// 0x028
	UINT32 S1_BSOUT_BUF_ADDR; 		    // 0x02C
	UINT32 S2_BSOUT_BUF_ADDR;			// 0x030
	UINT32 NAL_LEN_DUMP_ADDR;          // 0x034
	UINT32 S0_BSOUT_BUF_SIZE;			// 0x038
	UINT32 S1_BSOUT_BUF_SIZE;			// 0x03C
	UINT32 S2_BSOUT_BUF_SIZE;			// 0x040
	UINT32 S0_NAL_HEADER_TOTAL_LEN;	// 0x044
	UINT32 S1_NAL_HEADER_TOTAL_LEN;	// 0x048
	UINT32 S2_NAL_HEADER_TOTAL_LEN;	// 0x04C
	UINT32 PIC;				        // 0x050
	UINT32 SLICE;			        	// 0x054
	UINT32 FRM_WIDTH;			        // 0x058
	UINT32 TILE_WIDTH;			        // 0x05C
	UINT32 AEAD;		                // 0x060
	UINT32 SHA_INIT[8]; 		    	// 0x064 ~ 0x080
	UINT32 SHA_REPORT[8]; 			    // 0x084 ~ 0x0A0
	UINT32 CYCLE;      				// 0x0A4
	UINT32 CHECK_NAL_0;      		    // 0x0A8
	UINT32 CHECK_NAL_1;      		    // 0x0AC
	UINT32 PASS2_ENC_BS_LEN;			// 0x0B0
	UINT32 PASS2_ENC_BS_CHKSUM;		// 0x0B4
	UINT32 AVC_TILE_DBG_0;			    // 0x0B8
	UINT32 AVC_TILE_DBG_1;			    // 0x0BC
}VTRCRegSet;

extern void vtrc_disableClk(UINT32 chip_idx);
extern void vtrc_powerOn(void);
extern void vtrc_powerOff(void);
extern void vtrc_resetINT(UINT32 chip_idx);
extern UINT32 vtrc_waitINT(UINT32 chip_idx);
extern void vtrc_resetHW(UINT32 chip_idx);
extern void vtrc_module_reset(UINT32 chip_idx);
extern void vtrc_isr(UINT32 chip_idx);
extern UINT32 vtrc_exit_isr(UINT32 chip_idx);
extern void  vtrc_request_irq(void);
extern void vtrc_resetIntStatus(void);
extern void vtrc_reset(UINT32 chip_idx);
extern void vtrc_start(UINT32 chip_idx);
extern void vtrc_setIntEn(UINT32 chip_idx, UINT32 uiVal);
extern void vtrc_setHWTimeoutCnt(UINT32 chip_idx, UINT32 uiVal);
extern void vtrc_setBsDmaEn(UINT32 chip_idx, UINT32 idx);
extern void vtrc_setBsOutEn(UINT32 chip_idx, UINT32 idx);
extern void vtrc_setCodecClock(UINT32 chip_idx, BOOL enable);
extern void vtrc_setCodecPClock(UINT32 chip_idx, BOOL enable);
extern void vtrc_setNextBsDmaBuf(UINT32 chip_idx, UINT32 uiAddr, UINT32 idx);
extern void vtrc_setBsLen(UINT32 chip_idx, UINT32 uiSize, UINT32 idx);
extern void vtrc_setLock(UINT32 chip_idx);
extern void vtrc_setUnLock(UINT32 chip_idx);
extern void vtrc_clearIntStatus(UINT32 chip_idx, UINT32 uiVal);
extern UINT32 vtrc_getIsrStatus(void);
extern UINT32 vtrc_getIntEn(UINT32 chip_idx);
extern UINT32 vtrc_getIntStatus(UINT32 chip_idx);
extern UINT32 vtrc_getHwRegSize(void);
extern void vtrc_setRegSet(UINT32 chip_idx, volatile uintptr_t uiAPBAddr);
extern void vtrc_setEncDirectRegSet(UINT32 chip_idx, uintptr_t uiAPBAddr);
extern void vtrc_getSHAReport(UINT32 chip_idx, UINT32 uiShaReport[8]);
extern void vtrc_prtReg(UINT32 chip_idx);
extern void vtrc_setBsOutAddr(UINT32 chip_idx, UINT32 uiAddr, UINT32 uiSize, UINT32 idx);
extern UINT32 vtrc_getBsOutAddr(UINT32 chip_idx, UINT32 idx);
extern UINT32 vtrc_getBsOutSize(UINT32 chip_idx, UINT32 idx);
extern UINT32 vtrc_getBsLen(UINT32 chip_idx);
extern UINT32 vtrc_getBsChecksum(UINT32 chip_idx);
extern void vtrc_create_resource(UINT32 chip_idx);
extern void vtrc_release_resource(UINT32 chip_idx);
extern void vtrc_setShaEnable(UINT32 chip_idx, BOOL enable);
extern UINT32 vtrc_getNalLen(UINT32 chip_idx, UINT32 uiSel);
extern void vtrc_getDebug(UINT32 chip_idx);
extern UINT32 vtrc_getDbg1(UINT32 chip_idx, UINT32 uiSel);
extern void vtrc_setNextBsBuf(UINT32 chip_idx, UINT32 uiAddr, UINT32 uiSize);
extern void vtrc_doAgain(UINT32 chip_idx);
extern BOOL vtrc_getCodecClock(UINT32 chip_idx);
extern BOOL vtrc_getCodecPClock(UINT32 chip_idx);
extern void vtrc_getGatingInfo(void);
extern void vtrc_setClk(UINT32 chip_idx, UINT32 clock);
extern UINT32 vtrc_updateIntStatus(UINT32 chip_idx);
extern UINT32 vtrc_updateIntStatus2(UINT32 chip_idx);
#ifdef VDOCDC_EMU
extern void vtrc_reset_emu(UINT32 chip_idx);
#endif
extern H26xStatus vtrc_open(UINT32 chip_idx, UINT32 clock, uintptr_t reg_va, uintptr_t rst_va, void *vtrc_clk);
extern H26xStatus vtrc_close(UINT32 chip_idx);
extern H26xStatus vtrc_unlock(UINT32 chip_idx);
extern void vtrc_start_disdma(UINT32 chip_idx);
extern UINT32 vtrc_getCtrl(UINT32 chip_idx);
extern void vtrc_disasbleBvalid(UINT32 chip_idx);
extern UINT32 vtrc_getVtrcCycle(UINT32 chip_idx);
extern void vtrc_enableClk(UINT32 chip_idx);
extern void vtrc_disableClk(UINT32 chip_idx);
#endif

