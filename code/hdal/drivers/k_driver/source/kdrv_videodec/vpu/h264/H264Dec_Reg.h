
#ifndef _H264DEC_REG_H_
#define _H264DEC_REG_H_

#include "../../imvq.h"
#include "../vpu_comm.h"

//----------------------------------------------------------------------------------------------------

//H264_CTRL
#define H264_START                      	0x02
#define H264_SWRST                      	0x01

#define H264BIN_BSDMA_CMD_BUF_ADDR_VALID  0x00000001	/* 0x68 of BIN */

// 2ND SRAM
#define H264BIN_SRAM_DB_EN          	0x1
#define H264BIN_SRAM_MISC_EN       		0x2
#define H264BIN_SRAM_MASK            	(H264BIN_SRAM_DB_EN | H264BIN_SRAM_MISC_EN)

typedef enum _H264_ERROR_ERSILIENCE_OPTION_t
{
    H264_ERR_IRA_PRED                           = 0x80000000,   //(MB)intra-predction out of bound error
    H264_ERR_SLICE_BOUND_CHK                    = 0x40000000,   //(SH/MB)illegal slice end check
    H264_ERR_MVRANGE                            = 0x20000000,   //(MB)motion vector range out of range check
    H264_ERR_PPS_ID_OOR                         = 0x10000000,   //(SH)pps_id out of range error, pps_id range (0-255)
    H264_ERR_SH_PQ_DELTA                        = 0x08000000,   //(SH)sh_qp_delta decode error
    H264_ERR_SH_DDFI                            = 0x04000000,   //(SH)disable_deblocking_filter_idc out of range(0-2)
    H264_ERR_UNARY                              = 0x02000000,   //(MB)unary code parsing error in CABAC
    H264_ERR_INIT_OFFSET                        = 0x01000000,   //(MB)CABAC init offset incorrect
    H264_ERR_APU                                = 0x00800000,   //(MB)ref_idx target to an non-exist frame
    H264_ERR_RPLR                               = 0x00400000,   //(MB)Reorder_reference_list error
    H264_ERR_FINAL_MB                           = 0x00200000,   //(MB)FINAL_MB mb_eros = 0 error
    H264_ERR_OVER_MAX_LEN                       = 0x00100000,   //(MB)Total decode liength over max_dec_len
    H264_ERR_CAVLD                              = 0x00080000,   //(MB)All syntax decode error in cavld
    H264_ERR_FMB_INVALID                        = 0x00040000,   //(SH)Slice start MB addr incorrect
    H264_ERR_FRAME_NUM_CH                       = 0x00020000,   //(SH)The frame number coded in slice difference differernt with the frame buber coded in 1st slice of the pic
    H264_ERR_PPS_ID_CHANGE                      = 0x00010000,   //(SH)The pps_id coded in slices different with th epps_id coded in 1st slice of the pic
	H264_USE_INTRA_CONCEAL_EN					= 0x00000004,
    H264_INTRA_UPPER_LEFT_PLANE_MODE_ERROR_EN   = 0x00000001,
} H264_ERROR_ERSILIENCE_OPTION_t;

#define H264_ERROR_RESILIENCE_OPT   (	H264_ERR_PPS_ID_CHANGE                      |\
                                        H264_ERR_FRAME_NUM_CH                       |\
                                        H264_ERR_FMB_INVALID                        |\
                                        H264_ERR_CAVLD                              |\
                                        H264_ERR_OVER_MAX_LEN                       |\
                                        H264_ERR_FINAL_MB                           |\
                                        H264_ERR_RPLR                               |\
                                        H264_ERR_APU                                |\
                                        H264_ERR_INIT_OFFSET                        |\
                                        H264_ERR_UNARY                              |\
                                        H264_ERR_SH_DDFI                            |\
                                        H264_ERR_SH_PQ_DELTA                        |\
                                        H264_ERR_PPS_ID_OOR                         |\
                                        H264_ERR_MVRANGE                            |\
                                        H264_ERR_SLICE_BOUND_CHK                    |\
                                        H264_ERR_IRA_PRED                           |\
                                        H264_USE_INTRA_CONCEAL_EN                   |\
                                        H264_INTRA_UPPER_LEFT_PLANE_MODE_ERROR_EN  )


//H264 BIN register
typedef struct _H264DEC_BIN_REG
{
    volatile UINT32 CTRL;            //[01]    0x00     H264_CTRL - H264 Control Register
                                     //         0       H264_SWRST
                                     //         1       H264_START


    volatile UINT32 PIC_SIZE;        //[02]    0x04     H264_PIC_SIZE - H264 Picture Size Register
    volatile UINT32 REC_LINE_OFFSET; //[03]    0x08     H264_REC_LINE_OFFSET - H264 Reconstruction Line Offset Register, 32 byte alignment
    volatile UINT32 SEQ_CFG;         //[04]    0x0C     H264_SEQ_CFG - H264 Sequence Configuration Register
    volatile UINT32 PIC_CFG_I;       //[05]    0x10     H264_PIC_CFG_I - H264 Picture Configuration I Register

    volatile UINT32 PIC_CFG_II;      //[06]   	0x14      H264_PIC_CFG_II - H264 Picture Configuration II Register


    volatile UINT32 SCALING_MATRIX_ADDR;    //[07]0x18       H264_ SCALING_MATRIX_RD_ADDR

    volatile UINT32 PIC_TOP_POC;     //[08]   0x1C      H264_PIC_TOP_POC

    volatile UINT32 PIC_BOTTOM_POC;  //[09]   0x20      H264_PIC_BOTTOM_POC

    volatile UINT32 RPIC_DPB_ADDR;   //[10]   0x24      H264_RPIC_DPB_FRAME_ADDR

    volatile UINT32 RPIC_LIST_ADDR;  //[11]   0x28      H264_RPIC_REF_LIST_ADDR


    volatile UINT32 BSDMA_CMD_ADDR;  //[12]   0x2c      H264_BSDMA_CMD_BUF_ADDR

    volatile UINT32 DEC_MAX_LEN;     //[13]   0x30      H264_DEC_MAX_LEN - H264 Decoded Maximum Length Register
                                     //      25..0      H264_DEC_MAX_LEN
                                     //

    volatile UINT32 DEC_BS_LEN;      //[14]   0x34      H264_BS_LEN - H264 Decoding Bitstream Length Register
                                     //      25..0      H264_BS_LEN

    volatile UINT32 INT_EN;          //[15]   0x38      H264_INT_EN - H264 Interrupt Enable Register

    volatile UINT32 INT_FLG;         //[16]   0x3c      H264_INT_FLG - H264 Interrupt Flag Register

    volatile UINT32 REC_Y_ADDR;      //[17]   0x40      H264 Interpolation Y Starting Address
                                     //       28..0     H264_REC_Y_START_ADDR
                                     //

    volatile UINT32 REC_UV_ADDR;     //[18]   0x44      H264 Interpolation Packed UV Starting Address
                                     //       28..0     H264_REC_UV_START_ADDR
                                     //

    volatile UINT32 REC_COL_ADDR;    //[19]   0x48      H264 Interpolation col info Starting Address
                                     //       28..0     H264_COL_INFO_WT_START_ADDR
                                     //

    volatile UINT32 H264_MISC_URI_ADDR;	//[20] 0x4c      H264_MISC_URI_START_ADDR - H264 Upper Row Information Data Starting Address Register
                                     	//       28..0     H264_MISC_URI_START_ADDR

    volatile UINT32 H264_DB_URI_ADDR;	//[21]   0x50      H264_REC_DB_URI_START_ADDR - H264 Deblocking Reconstruction Upper Row Starting Address Register
                                     	//       28..0     H264_REC_DB_URI_START_ADDR

    volatile UINT32 H264_2ND_SRAM;   	//[22]   0x54      2ND_SRAM enable
                                     	//       1:0       {MISC_URI, DB_URI}

    volatile UINT32 RVD058;
    volatile UINT32 RVD05c;
    volatile UINT32 RVD060;
    volatile UINT32 RVD064;
    volatile UINT32 H264_BSDMA_CMD_BUF_ADDR_VALID;  // 0x68 H264_BSDMA_CMD_BUF_ADDR_VALID
    volatile UINT32 RVD06c;
    volatile UINT32 RVD070;
    volatile UINT32 RVD074;
    volatile UINT32 RVD078;
    volatile UINT32 RVD07c;

    volatile UINT32 CHECK_SUM;       //[33]   0x80      H264_CHECK_SUM
                                     //       31..0

    volatile UINT32 H264_VERSION;    //[34]   0x84      H264_VERSION - should be 4096. bit31: busy bit

    volatile UINT32 RVD088;			//[35]	0x88		Use for Debug.

    volatile UINT32 SLICE_CFG_I;     //[36]   0x8c      H264_SLICE_CFG_I - H264 Slice Configuration I Register


    volatile UINT32 SLICE_CFG_II;    //[37]   0x90      H264_SLICE_CFG_II - H264 Slice Configuration II Register
                                     //      31..0      H264_SLICE_PIC_ORDER_CNT_LSB or H264_SLICE_DELTA_PIC_ORDER_CNT[0]

    volatile UINT32 SLICE_CFG_III;   //[38]   0x94      H264_SLICE_CFG_II - H264 Slice Configuration II Register
                                     //      31..0      H264_SLICE_DELTA_PIC_ORDER_BOTTOM or H264_SLICE_DELTA_PIC_ORDER_CNT[1]

    volatile UINT32 ERR_STATUS;             // 0x98
                                            //      0   H264_ERR_STATUS_PPS_ID_CHANGE
                                            //      1   H264_ERR_STATUS_FRM_NUM_CHANGE
                                            //      2   H264_ ERR_STATUS_FIRST_MB_INVALID
                                            //      3   H264_ ERR_STATUS_VLD_ERROR
                                            //      4   H264_ERR_STATUS_VLD_OVER_DEC_MAX_LEN
                                            //      5   H264_ERR_STATUS_FINAL_MB_ERROR
                                            //      6   H264_ERR_STATUS_REORDERING_ERROR
                                            //      7   H264_ERR_STATUS_APU_ERROR
                                            //      8   H264_ ERR_STATUS_CABAD_ERROR_AD_OFFSET
                                            //      9   H264_ ERR_STATUS_CABAD_ERROR_AD_BIN
    volatile UINT32 WANRING_STATUS;         // 0x9c
                                            //      0   H264_WARN_SPS_NAL_FOUND
                                            //      1   H264_WARN_PPS_NAL_FOUND
                                            //      2   H264_WARN_SEI_NAL_FOUND
                                            //      3   H264_WARN_OTHER_NAL_FOUND

    volatile UINT32 H264_CYCLE;             // 0xa0 31:0
    volatile UINT32 H264_ED_CYCLE;          // 0xa4 31:0
    volatile UINT32 H264_MISC_MB_CYCLE;     // 0xa8 31:0
    volatile UINT32 H264_MISC_SLICE_CYCLE;  // 0xac 31:0
    volatile UINT32 H264_BSCAL_CYCLE;       // 0xb0 31:0
    volatile UINT32 H264_ITQ_CYCLE;         // 0xb4 31:0
    volatile UINT32 H264_INTRA_CYCLE;       // 0xb8 31:0
    volatile UINT32 H264_DB_CYCLE;          // 0xbc 31:0

    volatile UINT32 H264_CONCEAL_EN;        //  0xc0    H264 _CONCEAL_EN

    volatile UINT32 NUM_CONCEALED_MB;       //  0xC4    H264_NUM_CONCEALED_MB
                                            //  15..0   Indicate how many MBs are concealed

    volatile UINT32 ERR_CONCEAL_MVY_RANGE;  //  0xC8    ERROR_CONCEAL_MVY_RANGE
                                            //  15..0   Range in qpel (default: 0x200, i.e. -128 ~ +128)
    volatile UINT32 RVD0CC;					//  0xCC

    volatile UINT32 ERR_CONCEALMENT_OPT;    //  0xD0    ERROR_CONCEALMENT_OPT - error concealment options
											//		31: itq_dequant_error
											//		30: ira_pred_mode_err
											//		29: err_mv_range
											//		28: err_pps_id
											//		27: err_sh_qp_delta
											//		26: err_sh_ddfi
											//		25: err_unary
											//		24: err_init_offset
											//		23: err_apu
											//		22: error_rplr
											//		21: error_final mb
											//		20: over max length
											//		19: err_cavld
											//		18: first_mb_invalid
											//		17: err_frame_num_change
											//		16: err_pps_id_change
                                            //  15:  3	Reserved
                                            //		 2	H264_USE_INTRA_CONCEAL_EN
                                            //   	 1	Reserved
                                            //   	 0	H264_INTRA_UPPER_LEFT_PLANE_MODE_ERROR_EN
    volatile UINT32 RVD0D4;     			// 0xD4	Reserved.
    volatile UINT32 H264_EOS_ERR_MODE;     	// 0xD8	H264 EOS Error mode (0: enable, 1: disable)
    volatile UINT32 H264_EOS_INFO_ADDR;		// 0xDC	This register is starting address of eos information buffer, minimum size : 1 words/Slice
	volatile UINT32 RVD0E0[8];				//0xE0 ~ 0xFC Reserved.
	volatile UINT32 WD_LOAD;				//0x100 H/W timer initial value
	volatile UINT32 FIRST_WR;				//0x104 H/W uvswp/disable/auto_sw_resetn

} H264DEC_BIN_REG, *PH264DEC_BIN_REG;


//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
#define CSL_H264_PIC_FSH_INT						0x00000001
#define CSL_H264_BSDMA_END_INT				    	0x00000002
#define CSL_H264_ERROR_INT				    		0x00000004
#define CSL_H264_WARNING_INT				    	0x00000008
#define CSL_H264_HW_TIMEOUT_INT				    	0x00000010

void CSL_H264DEC_SetupRegisterBaseAddress(uintptr_t ulAddr, int chip_idx);
void CSL_ShowErrRegisters(int chip_idx);
void CSL_H264DEC_Close_INT_EN(int chip_idx);
void CSL_H264DEC_Clr_INT_FLG(unsigned int uiIntFlag, int chip_idx);
unsigned int CSL_H264DEC_Get_INT_FLG(int chip_idx);
unsigned int CSL_H264DEC_Get_Cycle_Cnt(int chip_idx);
unsigned int CSL_H264DEC_Get_ConcealedMbNum(int chip_idx);
unsigned int CSL_H264DEC_Get_CheckSum(int chip_idx);
/* 0x4C */
void CSL_H264DEC_Set_MISC_URI_ADDR(uintptr_t Misc_Addr, int chip_idx);
/* H264_LOCAL_SRAM_MISC_URI_ENABLE / H264_LOCAL_SRAM_DB_URI_ENABLE */
void CSL_H264DEC_Set_2nd_SRAM(unsigned int uiVal, int chip_idx);
/* 0x50, only BIN */
void CSL_H264DEC_Set_DB_URI_ADDR(uintptr_t Db_Addr, int chip_idx);
//----------------------------------------------------------------------------------------------------
#endif
