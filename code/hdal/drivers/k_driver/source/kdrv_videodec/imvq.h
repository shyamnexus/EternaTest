#ifndef _IMVQ_H_
#define _IMVQ_H_

//#include <linux/ratelimit.h>
#include "platform_port.h"
#include "portab.h"
#include "h26xdec_ll.h"
#include "debug.h"

#define NOT_PARSING_SEI				1
#define H26XD_SDK_MODE				1
#define FILL_REF_LIST				1

#define	LINK_LIST_NUM_JOBS			1

#define MAX_IMVQ_WRAP_NUM       (3)

/* H265 */
#define IMVQ_VDEC1_OFFSET		(0x0000)
#define IMVQ_AD0_OFFSET			(0x2000)
#define IMVQ_BIN_OFFSET			(0x2800)

/* H264*/
#define IMVQ_VDEC0_OFFSET		(0x1800)
#define IMVQ_H264_BIN_OFFSET    (0x1000)

/* link list */
#define IMVQ_LL_OFFSET			(0x3000)

#define HW_RESET_COUNT		(0x100)

typedef enum IVMQ_VDEC_COMPONENT_t
{
	IMVQ_VDEC_H264_EN       = 0x00000002,
    IMVQ_VDEC_HEVCBIN_EN    = 0x00100000,
    IMVQ_VDEC_HEVCAD0_EN    = 0x00200000,
} IMVQ_VDEC_COMPONENT_t;

typedef enum _IMVQ_VDEC_OPTION_t
{
	VDEC0_OPT_WRITE_MERGE_EN      = 0x01, //Enable wrapper reconstruction write merge
	VDEC0_OPT_MC_BURST_MODE_EN    = 0x02, //Enable wrapper MC read burst mode (always set to "1")
	VDEC0_OPT_MC_CACHE_EN         = 0x04, //Enable wrapper MC cache (16B alignment)
	VDEC0_OPT_LOCAL_SRAM_EN       = 0x08, //Enable wrapper local sram
	VDEC0_OPT_MC_CACHE_16B_EN     = 0x10, //Enable wrapper MC cache 16B alignment (non-used)
	VDEC0_OPT_VC1_CHECK_WRESP_EN  = 0x20, //Select write merge for VC1 reconstruction
	VDEC0_OPT_CHK_WRES_EN         = 0x40, //Enable wrapper wait last write response
	VDEC0_OPT_WAIT_WREADY_EN      = 0x80, //Enable wrapper wati wready
	VDEC0_OPT_WRITE_MERGE_64B_EN  = 0x100,//Enable wrapper reconstruction write merge 64Byte alignment //hk
	VDEC0_OPT_CACHE_IDX_UPDATE_EN = 0x1000,

    VDEC1_OPT_WRITE_MERGE_EN      = 0x01, //Enable wrapper reconstruction write merge
    VDEC1_OPT_MC_BURST_MODE_EN    = 0x02, //Enable wrapper MC read burst mode (always set to "1")
    VDEC1_OPT_MC_CACHE_EN         = 0x04, //Enable wrapper MC cache (16B alignment)
    VDEC1_OPT_LOCAL_SRAM_EN       = 0x08, //Enable wrapper local sram
    VDEC1_OPT_MC_CACHE_64B_N      = 0x10,
    VDEC1_OPT_CHK_WRES_EN         = 0x20, //Enable wrapper wait last write response
    VDEC1_OPT_HEVC_10B_EN         = 0x100,
    VDEC1_OPT_DATA_COMPARESS_EN   = 0x200,
    VDEC1_OPT_CACHE_IDX_UPDATE_EN = 0x1000,
    VDEC1_OPT_MERGE_PINGPONG_EN   = 0x2000,//Write merge 64
    VDEC1_OPT_MERGE_128B_EN       = 0x4000,//Write merge 128
    VDEC1_OPT_MC_CACHE_SP1W_X_EN  = 0x10000,
    VDEC1_OPT_MC_CACHE_SP1W_Y_EN  = 0x20000,
    VDEC1_OPT_MC_INFO_CACHE_EN    = 0x40000,//32byte
    VDEC1_OPT_MC_INFO_CACHE_64B_EN= 0x80000,//64byte
    VDEC1_OPT_MC_INFO_CACHE_IDX_UP= 0x100000,
    VDEC1_OPT_MC_INFO_CACHE_SP1W_Y= 0x200000,
    VDEC1_OPT_AXI_REC_W_SEP_EN    = 0x400000,
} IMVQ_VDEC_OPTION_t;

typedef enum _IMVQ_VDEC_INTERRUPT_t
{
    INTERRUPT_STATUS_DEC    = 0x00000001,
    INTERRUPT_STATUS_AD0    = 0x00000002,
	INTERRUPT_STATUS_HW_TIMEOUT_DEC = 0x00000004,
	INTERRUPT_STATUS_HW_TIMEOUT_AD0 = 0x00000008,
	INTERRUPT_STATUS_BSDMA_END = 0x00000010,
} IMVQ_VDEC_INTERRUPT_t;

/*----------------------------------------------------------------------------------------------------*/
/* VDEC register(Wrapper) */
typedef struct
{
    volatile unsigned int IMVQ_EN;                //[0x00]        VDEC_EN
                                            //[    0]       [R/W]enable JPG
                                            //[    1]       [R/W]enable H264Dec
                                            //[    2]       [R/W]enable VC1
                                            //[    3]       [R/W]enable RV34
                                            //[    4]       [R/W]enable MPEG4
                                            //[    5]       [R/W]enable MPEG2
                                            //[    6]       [R/W]enable AVS
                                            //[    7]       [R/W]enable VP8

    volatile unsigned int IMVQ_WRAP_0_AXI;        //[0x04]       VDEC_AXI_ID for id = 0 and id = 1
                                            //[ 4: 0]       [R/W]DMA wrapper 0 AXI Write ID
                                            //[12: 8]       [R/W]DMA wrapper 0 AXI Read ID for MC
                                            //[20:16]       [R/W]DMA wrapper 0 AXI read ID 1
                                            //[   24]       [R/W]DMA wrapper 0 AXI read ID for MC dma channel (always be 0)
                                            //[   25]       [R/W]DMA wrapper 0 AXI read ID for non-MC dma channel (always be 1)

    volatile unsigned int IMVQ_WRAP_0_OPTION;     //[0x08]        VDEC_OPTION
                                            //[    0]       [R/W]Enable wrapper[0] reconstruction write merge
                                            //[    1]       [R/W]Enable wrapper[0] MC read burst mode (always set to "1")
                                            //[    2]       [R/W]Enable wrapper[0] MC cache
                                            //[    3]       [R/W]Enable wrapper[0] local sram.
                                            //[    4]       [R/W]Enable wrapper[0] MC cache (16B alignment)
                                            //[    5]       [R/W]Select write merge for VC1 reconstruction, 0: always merge reconstruction write, 1: merge range map write
                                            //[    6]       [R/W]Enable wrapper[0] wait last write response
                                            //[    7]       [R/W]Enable wrapper[0] wait wready(for RW inorder issue)
											//[	   8]		[R/W]Enable wrapper[0]
                                            //[   12]       [R/W]Enable wrapper[0] cache index update

    volatile unsigned int IMVQ_WRAP_0_MC_CACHE_IDX;    //[0x0C]    VDEC_MC_CACHE_IDX
                                                //[ 3: 0]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 0 for B frame
                                                //[ 7: 4]       [R/W]Set wrapper[0] cache index by forward reference list 1 index 0 for B frame
                                                //[11: 8]       [R/W]Set wrapper[0] cache index by backward reference list 0 index 0 for P frame
                                                //[15:12]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 1 for P frame
                                                //[17:16]       [R/W]Set wrapper[0] current decoding picture LCU size[0:16x16, 1:32x32, 2:64x64]

    volatile unsigned int IMVQ_WRAP_0_MC_CACHE_CMD_SIZE;//[0x10]  VDEC_MC_CACHE_CMD_SIZE
                                            //[ 8:0]        [R/W]Wrapper[0] Max of out-standing DMA MC cache command, default[391]
                                            //[  10]        [R/W]Wrapper[0] Enable MC cache cmd overflow interrupt


    volatile unsigned int IMVQ_AXI_WRAP_0_CTRL;     //[0x14]        AXI_WRAP_CTRL
                                            //[    0]       [R/W]DMA wrapper 0 soft reset trigger [W:reset wrapper0  / R: wrapper0 ready flag]
                                            //[    1]       [R/W]DMA wrapper 0 enable [0:disable 1: enable ]

    //Should be removed.
    volatile unsigned int IMVQ_WRAP_0_MC_BUF_CHECK;//[0x18]       VDEC_MC_CACHE_CMD_SIZE
                                            //[ 9: 0]       [R/W]Wrapper[0] mc input buffer size (0~255)
                                            //[21:12]       [R/W]Wrapper[0] mc input buffer check buffer before read command request (0~255)
                                            //[   24]       [R/W]Wrapper[0] mc input buffer check before read command request
                                            //[   25]       [R/W]Wrapper[0] Enable MC cache command fifo picture initial


    volatile unsigned int IMVQ_DEC_SW_BOUNDING;   //[0x1C]
											//[    8]       [R/W]Efuse SW bounding setting for H264 DEC , 0:enable 1:disable

    volatile unsigned int IMVQ_DEC_BOUNDING_STATUS;//[0x20]    IMVQ_DEC_BOUNDING_STATUS
                                            //[    8]       [R]Efuse SW bounding setting for H264 DEC , 0:enable 1:disable

    volatile unsigned int IMVQ_HW_BUSY_FLAG;      //[0x24]    IMVQ_HW_BUSY_FLAG
                                            //[    0]       [R]DEC is busy

    volatile unsigned int IMVQ_RESERVED_28;       //[0x28]

    volatile unsigned int IMVQ_WRAP_1_AXI;        //[0x2C]       VDEC_AXI_ID for id = 0 and id = 1
                                            //[ 4: 0]       [R/W]DMA wrapper 0 AXI Write ID
                                            //[12: 8]       [R/W]DMA wrapper 0 AXI Read ID

    volatile unsigned int IMVQ_WRAP_1_OPTION;     //[0x30]        VDEC_OPTION
                                            //[    0]       [R/W]Enable wrapper[0] reconstruction write merge
                                            //[    1]       [R/W]Enable wrapper[0] MC read burst mode (always set to "1")
                                            //[    2]       [R/W]Enable wrapper[0] MC cache
                                            //[    3]       [R/W]reserved
                                            //[    4]       [R/W]Enable wrapper[0] MC cache (16B alignment)
                                            //[    5]       [R/W]Select write merge for VC1 reconstruction, 0: always merge reconstruction write, 1: merge range map write
                                            //[    6]       [R/W]Enable wrapper[0] wait last write response
                                            //[    7]       [R/W]Enable wrapper[0] wait wready(for RW inorder issue)

    volatile unsigned int IMVQ_WRAP_1_MC_CACHE_IDX;    //[0x34]    VDEC_MC_CACHE_IDX
                                                //[ 3: 0]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 0 for B frame
                                                //[ 7: 4]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 0 for B frame
                                                //[11: 8]       [R/W]Set wrapper[0] cache index by backward reference list 1 index 0 for P frame
                                                //[15:12]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 1 for P frame
                                                //[17:16]       [R/W]Set wrapper[0] current decoding picture LCU size[0:16x16, 1:32x32, 2:64x64]

    volatile unsigned int IMVQ_WRAP_1_MC_CACHE_CMD_SIZE;//[0x38]  VDEC_MC_CACHE_CMD_SIZE
                                            //[ 8:0]        [R/W]Wrapper[0] Max of out-standing DMA MC cache command, default[391]


    volatile unsigned int IMVQ_AXI_WRAP_1_CTRL;     //[0x3C]        AXI_WRAP_CTRL
                                            //[    0]       [R/W]DMA wrapper 0 soft reset trigger [W:reset wrapper0  / R: wrapper0 ready flag]
                                            //[    1]       [R/W]DMA wrapper 0 enable [0:disable 1: enable ]
                                            //[    4]       [R/W]DMA wrapper 1 soft reset trigger [W:reset wrapper1  / R: wrapper1 ready flag]
                                            //[    5]       [R/W]DMA wrapper 1 enable [0:disable 1: enable ]
                                            //[    8]       [R/W]DMA wrapper 2 soft reset trigger [W:reset wrapper2  / R: wrapper2 ready flag]
                                            //[    9]       [R/W]DMA wrapper 2 enable [0:disable 1: enable ]

    volatile unsigned int IMVQ_WRAP_1_MC_BUF_CHECK;//[0x40]       VDEC_MC_CACHE_CMD_SIZE
                                                            //[ 9: 0]       [R/W]Wrapper[0] mc input buffer size (0~255)
                                                            //[21:12]       [R/W]Wrapper[0] mc input buffer check buffer before read command request (0~255)
                                                            //[   24]       [R/W]Wrapper[0] mc input buffer check before read command request

    volatile unsigned int IMVQ_RESERVED_44;       //[0x44]

    volatile unsigned int IMVQ_INTERRUPT_MASK;    //[0x48]    IMVQ_INTERRUPT_MASK for ARM & leon2
                                            //[    0]       [R/W]mask for H26x BIN decoder/others decoder interrupt status

    volatile unsigned int IMVQ_INTERRUPT_STATUS;  //[0x4C]    IMVQ_INTERRUPT_STATUS
                                            //[    0]       [R]H26x BIN decoder/others decoder interrupt status

    volatile unsigned int IMVQ_WRAP0_MC_CACHE_IN_BW;    //[0x50]
    volatile unsigned int IMVQ_WRAP0_MC_CACHE_IN_BW_16B;//[0x54]
    volatile unsigned int IMVQ_WRAP0_MC_CACHE_OUT_BW;   //[0x58]
    volatile unsigned int IMVQ_WRAP1_MC_CACHE_IN_BW;    //[0x5C]
    volatile unsigned int IMVQ_WRAP1_MC_CACHE_IN_BW_16B;//[0x60]
    volatile unsigned int IMVQ_WRAP1_MC_CACHE_OUT_BW;   //[0x64]
    volatile unsigned int IMVQ_RQTABLE_GATE_EN;         //[0x68]
    volatile unsigned int CLK_EN;                       //[0x6C]
    volatile unsigned int REG070;
    volatile unsigned int REG074;
    volatile unsigned int REG078;
    volatile unsigned int REG07C;
    volatile unsigned int REG080;
	volatile unsigned int REG084;
	volatile unsigned int REG088;					//Store the start_address test of extra-write
	volatile unsigned int REG08C;
	volatile unsigned int REG090;
	volatile unsigned int EXTRA_WR_CFG;     		// 0x94
	volatile unsigned int EXTRA_WR_Y_ADDR;		// 0x98
	volatile unsigned int EXTRA_WR_C_ADDR;		// 0x9c
	volatile unsigned int EXTRA_WR_LINE_OFS;		// 0xa0
	volatile unsigned int EXTRA_WR_CHKSUM;		// 0xa4

} IMVQ_VDEC0_REG, *P_IMVQ_VDEC0_REG;

typedef struct
{
    volatile unsigned int IMVQ_EN;                //[0x00]        VDEC_EN
                                            //[   20]       [R/W]enable HEVC BIN
                                            //[   21]       [R/W]enable HEVC AD0
                                            //[   22]       [R/W]enable HEVC AD1
                                            //[   23]       [R/W]enable VP9  BIN
                                            //[   24]       [R/W]enable VP9  AD

    volatile unsigned int IMVQ_AXI_WRAP_CTRL;     //[0x04]        AXI_WRAP_CTRL
                                            //[    0]       [R/W]DMA wrapper 0 soft reset trigger [W:reset wrapper0  / R: wrapper0 ready flag]
                                            //[    1]       [R/W]DMA wrapper 0 enable [0:disable 1: enable ]
                                            //[    4]       [R/W]DMA wrapper 1 soft reset trigger [W:reset wrapper1  / R: wrapper1 ready flag]
                                            //[    5]       [R/W]DMA wrapper 1 enable [0:disable 1: enable ]
                                            //[    8]       [R/W]DMA wrapper 2 soft reset trigger [W:reset wrapper2  / R: wrapper2 ready flag]
                                            //[    9]       [R/W]DMA wrapper 2 enable [0:disable 1: enable ]
                                            //[   16]       [R/W]AXI wrapper 0 soft reset trigger
                                            //[   17]       [R/W]AXI wrapper 0 enable [0:disable 1: enable ]
                                            //[   20]       [R/W]AXI warpper 3 soft reset trigger
                                            //[   21]       [R/W]AXI wrapper 3 enable (Luma mc info)

    volatile unsigned int IMVQ_WRAP_01_AXI;       //[0x08]       VDEC_AXI_ID for id = 0 and id = 1
                                            //[ 4: 0]       [R/W]DMA wrapper 0 AXI Write ID
                                            //[12: 8]       [R/W]DMA wrapper 0 AXI Read ID
                                            //[20:16]       [R/W]DMA wrapper 1 AXI Write ID
                                            //[28:24]       [R/W]DMA wrapper 1 AXI Read ID

    volatile unsigned int IMVQ_WRAP_02_AXI;        //[0x0C]        VDEC_AXI_ID for id = 2
                                            //[ 4: 0]       [R/W]DMA wrapper 2 AXI Write ID
                                            //[12: 8]       [R/W]DMA wrapper 2 AXI Read ID
                                            //[20:16]       [R/W]DMA wrapper 0 AXI MC Read ID
                                            //[28:24]       [R/W]DMA wrapper 1 AXI MC Read ID


    volatile unsigned int IMVQ_WRAP_0_OPTION;     //[0x10]        VDEC_OPTION
                                            //[    0]       [R/W]Enable wrapper[0] reconstruction write merge
                                            //[    1]       [R/W]Enable wrapper[0] MC read burst mode (always set to "1")
                                            //[    2]       [R/W]Enable wrapper[0] MC cache (16B alignment)
                                            //[    3]       [R/W]Enable wrapper[0] local sram
                                            //[    4]
                                            //[    5]       [R/W]Enable wrapper[0] wait last write response,
                                            //[    6]
                                            //[    8]       [R/W]HEVC 10BIT Stream, 0:8b, 1:10b
                                            //[    9]       [R/W]Enable frame data compression.
                                            //[   12]       [R/W]Enable cache index update (must set to 0 for vp9)
                                            //[   13]       [R/W]Enable write merge ping-pong(always be 1)
                                            //[   16]       [R/W]Enable wrapper[0] mc cache spatial 1-way in x direction.
                                            //[   17]       [R/W]Enable wrapper[0] mc cache spatial 1-way in y direction.

    volatile unsigned int IMVQ_WRAP_1_OPTION;     //[0x14]        VDEC_OPTION
                                            //[    0]       [R/W]Enable wrapper[1] reconstruction write merge
                                            //[    1]       [R/W]Enable wrapper[1] MC read burst mode (always set to "1")
                                            //[    2]       [R/W]Enable wrapper[1] MC cache (16B alignment)
                                            //[    3]
                                            //[    4]
                                            //[    5]       [R/W]Enable wrapper[0] wait last write response,
                                            //[    6]
                                            //[    8]       [R/W]HEVC 10BIT Stream, 0:8b, 1:10b
                                            //[    9]       [R/W]Enable frame data compression.
                                            //[   12]       [R/W]Enable cache index update (must set to 0 for vp9)
                                            //[   13]
                                            //[   16]       [R/W]Enable wrapper 0 mc cache spatial 1-way in x direction.
                                            //[   17]       [R/W]Enable wrapper 0 mc cache spatial 1-way in y direction.
                                            //[   18]       [R/W]Enable Info cache (16B alignment)
                                            //[   19]       [R/W]Enable Info cache (32B alignment)
                                            //[   20]       [R/W]Enable Info cache index update (for HEVC only)
                                            //[   21]       [R/W]Enable wrapper 1 info cache spatial 1-way in y direction (Chroma)

    volatile unsigned int IMVQ_WRAP_2_OPTION;     //[0x18]    VDEC_OPTION
                                            //[    0]       Reserved
                                            //[    1]       [R/W]Enable wrapper[2] MC read burst mode (always set to "1")
                                            //[    3]       [R/W]Enable wrapper[2] local sram
                                            //[    5]       [R/W]Enable wrapper[2] wait last write response,
                                            //[   13]       [R/W]Enable write merge ping-pong (non-used)

    volatile unsigned int IMVQ_WRAP_0_MC_CACHE_IDX;    //[0x1C]    VDEC_MC_CACHE_IDX
                                            //[ 3: 0]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 1 for P frame
                                            //[ 7: 4]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 0 for P frame
                                            //[11: 8]       [R/W]Set wrapper[0] cache index by backward reference list 1 index 0 for B frame
                                            //[15:12]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 0 for B frame
                                            //[17:16]       [R/W]Set wrapper[0] current decoding picture LCU size[0:16x16, 1:32x32, 2:64x64]
                                            //[23:20]       [R/W]Set wrapper[0] cache index by additional different dpb index
                                            //[27:24]       [R/W]Set wrapper[0] cache index by forward reference list 0 index 2 for P frame

    volatile unsigned int IMVQ_WRAP_1_MC_CACHE_IDX;    //[0x20]    VDEC_MC_CACHE_IDX
                                            //[ 3: 0]       [R/W]Set wrapper[1] cache index by forward reference list 0 index 1 for P frame
                                            //[ 7: 4]       [R/W]Set wrapper[1] cache index by forward reference list 0 index 0 for P frame
                                            //[11: 8]       [R/W]Set wrapper[1] cache index by backward reference list 1 index 0 for B frame
                                            //[15:12]       [R/W]Set wrapper[1] cache index by forward reference list 0 index 0 for B frame
                                            //[17:16]       [R/W]Set wrapper[1] current decoding picture LCU size[0:16x16, 1:32x32, 2:64x64]
                                            //[23:20]       [R/W]Set wrapper[1] cache index by additional different dpb index
                                            //[27:24]       [R/W]Set wrapper[1] cache index by forward reference list 0 index 2 for P frame

    volatile unsigned int IMVQ_WRAP_01_MC_CACHE_CMD_SIZE;//[0x24]  VDEC_MC_CACHE_CMD_SIZE
                                            //[ 8:0]        [R/W]Wrapper[0] Max of out-standing DMA MC cache command, default[391]
                                            //[20:12]       [R/W]Wrapper[1] Max of out-standing DMA MC cache command, default[391]


    volatile unsigned int IMVQ_WRAP_0_MC_BUF_CHECK;    //[0x28]    VDEC_MC_CACHE_CMD_SIZE
                                            //[ 9: 0]       [R/W]Wrapper[0] mc input buffer size (0~255)
                                            //[21:12]       [R/W]Wrapper[0] mc input buffer check buffer before read command request (0~255)
                                            //[   24]       [R/W]Wrapper[0] mc input buffer check before read command request

    volatile unsigned int IMVQ_WRAP_1_MC_BUF_CHECK;    //[0x2C]    VDEC_MC_CACHE_CMD_SIZE
                                            //[ 9: 0]       [R/W]Wrapper[1] mc input buffer size (0~255)
                                            //[21:12]       [R/W]Wrapper[1] mc input buffer check buffer before read command request (0~255)
                                            //[   24]       [R/W]Wrapper[1] mc input buffer check before read command request

    volatile unsigned int IMVQ_WRAP_1_CACHE_IDX_EXT1;  //[0x30]    WRAP1_CACHE_IDX
                                            //[4:0]   B2
                                            //[12:8]  P3
                                            //[20:16] B3
                                            //[28:24] P4



    volatile unsigned int IMVQ_INTERRUPT_STATUS;  //[0x34]    IMVQ_INTERRUPT_STATUS
                                            //[    0]       [R]H26x BIN decoder/others decoder interrupt status
                                            //[    1]       [R]H26x AD0 decoder interrupt status
                                            //[    2]       [R]H26x AD1 decoder interrupt status

    volatile unsigned int IMVQ_INTERRUPT_STATUS_FOR_LEON2;  //[0x38]    IMVQ_INTERRUPT_STATUS
                                            //[    0]       [R]H26x BIN decoder/others decoder interrupt status for leon2
                                            //[    1]       [R]H26x AD0 decoder interrupt status for leon2
                                            //[    2]       [R]H26x AD1 decoder interrupt status for leon2

    volatile unsigned int IMVQ_INTERRUPT_MASK;    //[0x3C]    IMVQ_INTERRUPT_MASK for ARM & leon2
                                            //[    0]       [R/W]mask for H26x BIN decoder/others decoder interrupt status
                                            //[    1]       [R/W]mask for H26x AD0 decoder interrupt status
                                            //[    2]       [R/W]mask for H26x AD1 decoder interrupt status
                                            //[    8]       [R/W]mask for H26x BIN decoder/others decoder interrupt status for leon2
                                            //[    9]       [R/W]mask for H26x AD0 decoder interrupt status for leon2
                                            //[   10]       [R/W]mask for H26x AD1 decoder interrupt status for leon2

    volatile unsigned int IMVQ_WRAP0_MC_CACHE_IN_BW;   //[0x40]    WRP0_MC_CACHE_IN_BW
                                            //[31: 0]       [R]Wrapper[0] total words for 16 byte alignment MC bandwidth

    volatile unsigned int IMVQ_WRAP0_MC_CACHE_IN_BW_32B;//[0x44]   WRP0_MC_CACHE_IN_BW_32B
                                            //[31: 0]       [R]Wrapper[0] total words for 32 byte alignment MC bandwidth

    volatile unsigned int IMVQ_WRAP0_MC_CACHE_OUT_BW;  //[0x48]    WRP0_MC_CACHE_OUT_BW
                                            //[31: 0]       [R]Wrapper[0] total words for real MC bandwidth

    volatile unsigned int IMVQ_WRAP1_MC_CACHE_IN_BW;   //[0x4C]    WRP1_MC_CACHE_IN_BW
                                            //[31: 0]       [R]Wrapper[1] total words for 16 byte alignment MC bandwidth

    volatile unsigned int IMVQ_WRAP1_MC_CACHE_IN_BW_32B;//[0x50]   WRP1_MC_CACHE_IN_BW_32B
                                            //[31: 0]       [R]Wrapper[1] total words for 32 byte alignment MC bandwidth

    volatile unsigned int IMVQ_WRAP1_MC_CACHE_OUT_BW;  //[0x54]    WRP1_MC_CACHE_OUT_BW
                                            //[31: 0]       [R]Wrapper[1] total words for real MC bandwidth

    volatile unsigned int IMVQ_HW_BUSY_FLAG;      //[0x58]    IMVQ_HW_BUSY_FLAG
                                            //[    0]       [R]DEC (BIN) is busy
                                            //[    1]       [R]H26x AD0 is busy
                                            //[    2]       [R]H26x AD1 is busy

    volatile unsigned int IMVQ_WRAP0_WRITE_MERGE_SWRST;     //[0x5C]

    volatile unsigned int IMVQ_DEC_SW_BOUNDING;      //[0x60]    IMVQ_DEC_BOUNDING
                                            //[    9]       [R/W]Efuse SW bounding setting for HEVC DEC , 0:enable 1:disable
                                            //[   11]       [R/W]Efuse SW bounding setting for VP9 DEC  , 0:enable 1:disable

    volatile unsigned int IMVQ_DEC_BOUNDING_STATUS;//[0x64]    IMVQ_DEC_BOUNDING_STATUS
                                            //[    9]       [R]Efuse bounding setting for HEVC DEC , 0:enable 1:disable
                                            //[   11]       [R]Efuse bounding setting for VP9 DEC  , 0:enable 1:disable
    volatile unsigned int Reversed68;
    volatile unsigned int Reversed6C;
    volatile unsigned int Reversed70;
    volatile unsigned int Reversed74;
    volatile unsigned int Reversed78;
    volatile unsigned int Reversed7C;
    volatile unsigned int CLK_EN;                //[0x80]        VDEC_EN
                                            //[   20]       [R/W]enable HEVC BIN clock
                                            //[   21]       [R/W]enable HEVC AD0 clock
                                            //[   22]       [R/W]enable HEVC AD1 clock
                                            //[   23]       [R/W]enable HEVC AD1 clock
                                            //[   24]       [R/W]enable HEVC AD1 clock
    volatile unsigned int Reserved84;
    volatile unsigned int Reserved88;
    volatile unsigned int Reserved8C;
    volatile unsigned int Reserved90;
    volatile unsigned int Reserved94;
    volatile unsigned int Reserved98;
    volatile unsigned int Reserved9C;
    volatile unsigned int ReservedA0;
    volatile unsigned int ReservedA4;
    volatile unsigned int ReservedA8;
    volatile unsigned int ReservedAC;
    volatile unsigned int ReservedB0;
    volatile unsigned int ReservedB4;
    volatile unsigned int ReservedB8;
    volatile unsigned int ReservedBC;
    volatile unsigned int ReservedC0;
    volatile unsigned int ReservedC4;
    volatile unsigned int ReservedC8;
    volatile unsigned int ReservedCC;
    volatile unsigned int ReservedD0;
    volatile unsigned int ReservedD4;
    volatile unsigned int ReservedD8;
    volatile unsigned int ReservedDC;
    volatile unsigned int IMVQ_MC_INFO_CACHE_CMD_SIZE;//[0xE0]
                                                    //[20:12] Wrapper0 Max of out-standing DMA MC cache command, default 255

    volatile unsigned int IMVQ_MC_INFO_INPUT_BUF_CHECK;//[0xE4]
                                                    //[ 9: 0] Wrapper0 mc input buffer size (0~255)
                                                    //[21:12] Wrapper0 mc input buffer check buffer before read command request(0~255)
                                                    //[   24] Wrapper0 mc input buffer check before read command request.

    volatile unsigned int IMVQ_AXI_WRP0_WID;           //[0xE8]
                                                    //[ 4: 0] AXI wrapper0 AXI Wrie ID

    volatile unsigned int IMVQ_AXI_WRAP_ARBITER;       //[0xEC]
                                                    //[    0] AXI_WRP0_R_ARB_PROI0 priority, 1: high, 0: low
                                                    //[    1] AXI WRP0_R_ARB_PROI1 priority, 1: high, 0: low
                                                    //[    2] AXI_WRP0_R_CHECK_GNT_CNT, 1:enable, 0: disable
                                                    //[    3] Reserved
                                                    //[    4] AXI_WRP0_W_ARB_PROI0 priority, 1: high, 0: low
                                                    //[    5] AXI_WRP0_W_ARB_PROI1 priority, 1: high, 0: low
                                                    //[    6] AXI_WRP0_W_CHECK_GNT_CNT, 1:enable, 0: disable
                                                    //[    7] Reserved
                                                    //[    8] AXI_WRP1_R0_ARB_PROI0 priority, 1: high, 0: low
                                                    //[    9] AXI_WRP1_R0_ARB_PROI1 priority, 1: high, 0: low
                                                    //[   10] AXI_WRP1_R0_CHECK_GNT_CNT, 1:enable, 0: disable
                                                    //[   11] Reserved
                                                    //[   12] AXI_WRP1_R1_ARB_PROI0 proirity, 1: high, 0: low
                                                    //[   13] AXI_WRP1_R1_ARB_PROI1 proirity, 1: high, 0: low
                                                    //[   14] AXI_WRP1_R1_CHECK_GNT_CNT, 1:enable, 0: disable

    volatile unsigned int IMVQ_WRP0_RW_ARB_GNT_CNT0;   //[0xF0]
                                                    //[ 4: 0] AXI_WRP0_R_ARB_GNT_CNT0, set contiunes grant count(1~31)
                                                    //[12: 8] AXI_WRP0_R_ARB_GNT_CNT0, set contiunes grant count(1~31)
                                                    //[20:16] AXI_WRP0_W_ARB_GNT_CNT0, set contiunes grant count(1~31)
                                                    //[28:24] AXI_WRP0_W_ARB_GNT_CNT0, set contiunes grant count(1~31)

    volatile unsigned int IMVQ_WRP1_R_ARB_GNT_CNT0;   //[0xF4]
                                                    //[ 4: 0] AXI_WRP0_R0_ARB_GNT_CNT0, set contiunes grant count(1~31)
                                                    //[12: 8] AXI_WRP0_R0_ARB_GNT_CNT0, set contiunes grant count(1~31)
                                                    //[20:16] AXI_WRP0_R1_ARB_GNT_CNT0, set contiunes grant count(1~31)
                                                    //[28:24] AXI_WRP0_R1_ARB_GNT_CNT0, set contiunes grant count(1~31)

    volatile unsigned int ReservedF8;
    volatile unsigned int ReservedFC;
    volatile unsigned int IMVQ_WRP1_CACHE_IDX;        //[0x100]
                                                    //[    0] WRP1_CACHE_IDX_B0_0[4], Set wrapper 1 cache index by forward reference list 0 index 0 for B frame (MSB bit)
                                                    //[    4] WRP1_CACHE_IDX_B1_0[4], Set wrapper 1 cache index by backward reference list 1 index 0 for B frame (MSB bit)
                                                    //[    8] WRP1_CACHE_IDX_P0_0[4], Set wrapper 1 cache index by forward reference list 0 index 0 for P frame (MSB bit)
                                                    //[   12] WRP1_CACHE_IDX_P0_1[4], Set wrapper 1 cache index by forward reference list 0 index 1 for P frame (MSB bit)
                                                    //[   16] WRP1_CACHE_IDX_B_1[4], Set wrapper 1 cache index by additional differeent dpb index (MSB bit)
                                                    //[   20] WRP1_CACHE_IDX_P0_2[4], Set wrapper 1 cache index by forward reference list 0 index 2 for P frame (MSB bit)

    volatile unsigned int IMVQ_WRP0_CACHE_IDX;        //[0x104]
                                                    //[    0] WRP0_CACHE_IDX_B0_0[4], Set wrapper 0 cache index by forward reference list 0 index 0 for B frame (MSB bit)
                                                    //[    4] WRP0_CACHE_IDX_B1_0[4], Set wrapper 0 cache index by backward reference list 1 index 0 for B frame (MSB bit)
                                                    //[    8] WRP0_CACHE_IDX_P0_0[4], Set wrapper 0 cache index by forward reference list 0 index 0 for P frame (MSB bit)
                                                    //[   12] WRP0_CACHE_IDX_P0_1[4], Set wrapper 0 cache index by forward reference list 0 index 1 for P frame (MSB bit)
                                                    //[   16] WRP0_CACHE_IDX_B_1[4], Set wrapper 0 cache index by additional differeent dpb index (MSB bit)
                                                    //[   20] WRP0_CACHE_IDX_P0_2[4], Set wrapper 0 cache index by forward reference list 0 index 2 for P frame (MSB bit)
	volatile unsigned int Reversed108;
	volatile unsigned int Reversed10c;
	volatile unsigned int BOUNDING_PASSWORD;             //[0x110]
    volatile unsigned int LUMA_MC_INFO_CACHE_IN_BW;      //[0x114]
    volatile unsigned int LUMA_MC_INFO_CACHE_IN_BW_32B;  //[0x118]
    volatile unsigned int LUMA_MC_INFO_CACHE_OUT_BW;     //[0x11C]
    volatile unsigned int CHROMA_MC_INFO_CACHE_IN_BW;    //[0x120]
    volatile unsigned int CHROMA_MC_INFO_CACHE_IN_BW_32B;//[0x124]
    volatile unsigned int CHROMA_MC_INFO_CACHE_OUT_BW;   //[0x128]
    volatile unsigned int WRP0_REC_BW_32B;               //[0x12C]
    volatile unsigned int WRP0_REC_BW_64B;               //[0x130]
    volatile unsigned int WRP2_MISC_R_BW_32B;            //[0x134]
    volatile unsigned int WRP2_MISC_R_BW_64B;            //[0x138]
    volatile unsigned int WRP2_MISC_W_BW_32B;            //[0x13C]
    volatile unsigned int WRP2_MISC_W_BW_64B;            //[0x140]
    volatile unsigned int WRP1_REC_BW_32B;               //[0x144]
    volatile unsigned int WRP1_REC_BW_64B;               //[0x148]
    volatile unsigned int Reserved14C;
    volatile unsigned int Reserved150;
    volatile unsigned int Reserved154;
    volatile unsigned int Reserved158;                       //[0x158]
                                                      //[ 2: 0] apl_shf_0
                                                      //[ 7: 4] apl_shf_1
                                                      //[17: 8] apl_shf_div

    volatile unsigned int Reserved15C;                     //[0x15C]
                                                      //[    0] hist_en
                                                      //[    1] hist_acc_en
                                                      //[    2] hist_dma_mode_en
                                                      //[    3] hist_clr

    volatile unsigned int Reserved160;                  //[0x160]
    volatile unsigned int Reserved164;                  //[0x164]
    volatile unsigned int Reserved168;                 //[0x168]
    volatile unsigned int Reserved16C;                  //[0x16C]
    volatile unsigned int Reserved170;                   //[0x170] [R/W] 0 ~ 31
    volatile unsigned int Reserved174;                      //[0x174] [R] according to hist_acc_sel, output hist_acc

    volatile unsigned int IMVQ_WRAP_0_CACHE_IDX_EXT1;    //[0x178]
    volatile unsigned int IMVQ_WRAP_0_CACHE_IDX_EXT2;    //[0x17C]
    volatile unsigned int IMVQ_WRAP_1_CACHE_IDX_EXT2;    //[0x180]

} IMVQ_VDEC1_REG, *P_IMVQ_VDEC1_REG;

/*----------------------------------------------------------------------------------------------------*/
/* level of log message */
#define LOG_ERROR		0
#define LOG_WARNING	    1
#define LOG_INFO		2

/* position of log message */
#define POS_VPS             BIT0
#define POS_NAL             BIT1
#define POS_ENTITY          BIT2
#define POS_TOOL            BIT3
#define POS_RESMGR          BIT4
#define POS_HW              BIT5
#define POS_H264_VPS        BIT6
#define POS_H264_NAL        BIT7
#define POS_H264_ENTITY     BIT8
#define POS_H264_PICBUF     BIT9
#define POS_H264_DRV        BIT10
#define POS_H264_VLD        BIT11
#define POS_H264_HW         BIT12
#define POS_ALL             (0xffff)

/* print debug log */
#define PRINT_COND(level) (1)

extern int h26xd_dbglevel;
extern int h26xd_dbg_mode;
extern int h26xd_dbgpos;

//#include "log.h"
#define LOG_M_PFX(pos, level, prefix, fmt, args...) do {		\
		if(PRINT_COND(level)) {							\
			const char *fmt_str = fmt;						\
			char *pfx_str = prefix;						\
			if (0 == level || ((h26xd_dbg_mode == 1) && (h26xd_dbglevel >= level) && (h26xd_dbgpos & pos))) { \
				printm(pfx_str, fmt_str, ## args);			\
			}											\
		}												\
	} while (0);

#define LOG_PRINT(pos, level, fmt, args...)       LOG_M_PFX(pos, level, "DE", fmt, ## args)
/*----------------------------------------------------------------------------------------------------*/

void imvq_init(uintptr_t imvq_addr, int chip_idx);
void imvq_clk_enable(int partG, int chip_idx);
void imvq_clk_disable(int partG, int chip_idx);

void IMVQ_VDEC_Enable(int partG, IMVQ_VDEC_COMPONENT_t enCodec, int chip_idx);
void IMVQ_VDEC_Disable(int partG, IMVQ_VDEC_COMPONENT_t enCodec, int chip_idx);
void IMVQ_VDEC_EnableWrap(int partG, int chip_idx);
void IMVQ_VDEC_DisableWrap(int partG, int chip_idx);
void IMVQ_VDEC_SetupRegisterBaseAddress(uintptr_t uiAddr, int chip_idx);
void IMVQ_VDEC_SetInterruptMask(int partG, unsigned int mask, int chip_idx);
unsigned int IMVQ_VDEC_GetInterruptStatus(int partG, int chip_idx);
unsigned int IMVQ_VDEC_GetHWBusyFlag(int partG, int chip_idx);
void IMVQ_VDEC_CheckH264Enable(int chip_idx);

void VPU_AcquireHW(int partG, int chip_idx);
void VPU_ReleaseHW(int partG, int chip_idx);
void VPU_ResetHW(int partG, int chip_idx, int fail_case_flag);
void VPU_ClearInterrupt(int partG, int chip_idx);

void IMVQ_VDEC_OpenHW(uintptr_t imvq_addr, int chip_idx);
void IMVQ_VDEC_CloseHW(int chip_idx);

void IMVQ_VDEC_SetWrapOption(int partG, unsigned int u32DevId, unsigned int opt, int chip_idx);
unsigned int IMVQ_VDEC_GetWrapOption(int partG, unsigned int u32DevId, int chip_idx);

void IMVQ_VDEC_SetLLstartAddr(uintptr_t addr, int chip_idx);
void IMVQ_VDEC_TriggerLLfire(int partG, DecLLJob *job, int chip_idx);
void IMVQ_VDEC_ConnectLLJob(DecLLJob *cur_job, int job_flag);
void IMVQ_VDEC_LLReset(int chip_idx);
unsigned int IMVQ_VDEC_LLGetCurJobId(int chip_idx, unsigned int *next_job_addr);
unsigned int IMVQ_VDEC_LLCheckFinalJob(int chip_idx);
void IMVQ_VDEC_JumpLLJob(int chip_idx, int job_id);

#endif

