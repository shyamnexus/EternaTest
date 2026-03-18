/**
    MIPI-CSI_TX Controller global header

    MIPI-CSI_TX Controller global header

    @file       csi_tx.h
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/
#ifndef __CSI_TX_H__
#define __CSI_TX_H__

#ifdef __KERNEL__
#include "kwrap/type.h"
#endif

//#if defined(_NVT_FPGA_)
#define _TC18039_	0
//#endif

#if (defined __FREERTOS)
#define CSI_TX_DATA_TYPE		UINT32
#define CSI_TX_PHY_DATA_TYPE	UINT32
#else
#define CSI_TX_DATA_TYPE		UINT64
#define CSI_TX_PHY_DATA_TYPE	UINT64
#endif


#define gEMU_CSI_TX2 0

#define CSI_TX_FREQ_MULTIPLICATION              0
#define CSI_TX_FRAME_WIDTH_RESTRICTION          0 // 1 => 530 before eco

/**
    @addtogroup mIDrvDisp_CSI_TX
*/
//@{

/**
    @name   Content value on CSI_TX host layer

*/
//@{
#define CSI_TX_ACMD_SRAM_SIZE                   256         // bytes
#define CSI_TX_WORD_SIZE                        4

#define CSI_TX_WORD_ALIGN_MASK                  0x3
#define CSI_TX_AXI_128_BITS_ALIGN_MASK          0xF

#define CSI_TX_WAIT_ISR_TIMEOUT                 5000        // ms, 4096*2160 transfer time larger than 3 sec on FPGA
#define CSI_TX_PLL_DEFAULT_CLK                  480000000   // Hz
#define CSI_TX_PLL_CLK_MAX                      960000000   // Hz
#define CSI_TX_PLL_BAND_THD                     1100000000  // Hz
#define CSI_TX_HS_CLK_MAX                       2500000000  // Hz
#define CSI_TX_BLANK_MIN                        10
#define CSI_TX_DMA_BURST_TIME                   75          // cycle, with margin

#define CSI_TX_HINV_MAX                         0xFFFF		// max
#define CSI_TX_HBP_MAX                          0XFFF		// max
#define CSI_TX_HFP_MAX                          0XFFF		// max

// CSI_TX2 //
#define CSI_TX2_ACMD_SRAM_SIZE                   256         // bytes
#define CSI_TX2_WORD_SIZE                        4

#define CSI_TX2_WORD_ALIGN_MASK                  0x3
#define CSI_TX2_AXI_128_BITS_ALIGN_MASK          0xF

#define CSI_TX2_WAIT_ISR_TIMEOUT                 160000 // FPGA 12M     //5000 ms, 4096*2160 transfer time larger than 3 sec on FPGA
#define CSI_TX2_PLL_DEFAULT_CLK                  480000000   // Hz

#define CSI_TX2_PLL_CLK_MAX                      960000000   // Hz
#define CSI_TX2_PLL_BAND_THD                     1100000000  // Hz
#define CSI_TX2_HS_CLK_MAX                       2500000000  // Hz
#define CSI_TX2_BLANK_MIN                        3
#define CSI_TX2_DMA_BURST_TIME                   75          // cycle, with margin
#define CSI_TX2_HINV_MAX                         4095



//@}

/**
    @name   Host to peripheral packet data types

*/
//@{
#define DATA_TYPE_SHORT_FRAME_START           0x00
#define DATA_TYPE_SHORT_FRAME_END             0x01
#define DATA_TYPE_SHORT_LINE_START            0x02
#define DATA_TYPE_SHORT_LINE_END           	  0x03
#define DATA_TYPE_SHORT_GENERIC_1          	  0x08
#define DATA_TYPE_SHORT_GENERIC_2          	  0x09
#define DATA_TYPE_SHORT_GENERIC_3          	  0x0A
#define DATA_TYPE_SHORT_GENERIC_4          	  0x0B
#define DATA_TYPE_SHORT_GENERIC_5          	  0x0C
#define DATA_TYPE_SHORT_GENERIC_6          	  0x0D
#define DATA_TYPE_SHORT_GENERIC_7          	  0x0E
#define DATA_TYPE_SHORT_GENERIC_8          	  0x0F

#define DATA_TYPE_LONG_NULL              	  0x10
#define DATA_TYPE_LONG_BLANKING          	  0x11
#define DATA_TYPE_LONG_8BIT_DATA           	  0x12

#define DATA_TYPE_YUV420_8BIT           	  0x18
#define DATA_TYPE_YUV420_10BIT           	  0x19
#define DATA_TYPE_LEGACY_YUV420_8BIT       	  0x1A
#define DATA_TYPE_YUV420_8BIT_CHROMASHOFT	  0x1C
#define DATA_TYPE_YUV420_10BIT_CHROMASHOFT 	  0x1D
#define DATA_TYPE_YUV422_8BIT           	  0x1E
#define DATA_TYPE_YUV422_10BIT           	  0x1F

#define DATA_TYPE_RGB444					  0x20
#define DATA_TYPE_RGB555					  0x21
#define DATA_TYPE_RGB565					  0x22
#define DATA_TYPE_RGB666					  0x23
#define DATA_TYPE_RGB888					  0x24

#define DATA_TYPE_RAW6						  0x28
#define DATA_TYPE_RAW7						  0x29
#define DATA_TYPE_RAW8						  0x2A
#define DATA_TYPE_RAW10						  0x2B
#define DATA_TYPE_RAW12						  0x2C
#define DATA_TYPE_RAW14						  0x2D
#define DATA_TYPE_RAW32					  	  0x28

#define DATA_TYPE_USER_DEFINE_1           	  0x30
//@}

/**
    @name   Ultra low power state entry command pattern

    @note for csi_tx_set_escape_entry()
*/
//@{
#define ULPS_ENTRY_CMD_NOP                      0x00
#define ULPS_ENTRY_CMD_LPDT                     0x87    ///< Low power data transmission
#define ULPS_ENTRY_CMD_ULPS                     0x78    ///< Enter ultra Low power state
#define ULPS_ENTRY_CMD_RESET                    0x46    ///< reset trigger
//@}

/**
    @name   Macro on CSI_TX host layer

*/
//@{
#define CSI_TX_DIV_256_ROUND_UP(x)              ((x + 255) >> 8)
#define CSI_TX_BLANK_DEFAULT(frame_height)      ((frame_height / 20) + 1)
//@}

/* for csi_tx_cb_func */
// typedef void (*DRV_CB)(ULONG event);
 typedef int (*CSITX_CB)(uintptr_t  y_buf_0, uintptr_t uv_buf_1 );

/**
    CSI_TX mode select

    @note for csi_tx_set_config()
*/
typedef enum {
	CSI_TX_MODE_AUTO_MODE = 0,   ///< auto round robin around the triple buffer addresses.
	CSI_TX_MODE_MANUAL_MODE,     ///< Manual keep sending MANUAL_BUFFER_ID buffer.
	CSI_TX_MODE_ONESHOT_MODE,    ///< sending MANUAL_BUFFER_ID buffer. One time and auto clear CSITX_EN and also issue the DISABLE interrupt status

	CSI_TX_MODE_CNT,
    CSI_TX_MODE_SKIP = CSI_TX_MODE_CNT, // skip tx mode setting

	ENUM_DUMMY4WORD(CSI_TX_MODESEL)
} CSI_TX_MODESEL;

/**
    CSI_TX pixel format select

    @note for csi_tx_set_config()
*/
typedef enum {
	CSI_TX_PIXEL_RAW8 = 0,      ///< 8 bits / pixel
	CSI_TX_PIXEL_YUV422_8BIT,    ///
	CSI_TX_PIXEL_LEGACY_YUV420_8BIT,  ///
	CSI_TX_PIXEL_RAW10,          ///< 10 bits / pixel
	CSI_TX_PIXEL_RAW12,          ///< 12 bits / pixel
	CSI_TX_PIXEL_RAW14,          ///< 14 bits / pixel
	CSI_TX_PIXEL_RGB565,         ///< 16 bits / pixel
	CSI_TX_PIXEL_RGB888,         ///< 24 bits / pixel
	CSI_TX_PIXEL_RAW32,          ///< 32 bits / pixel

	CSI_TX_PIXEL_FMT_CNT,
    CSI_TX_PIXEL_SKIP = CSI_TX_PIXEL_FMT_CNT,   // skip pixel format setting
	ENUM_DUMMY4WORD(CSI_TX_PIXEL_FORMATSEL)
} CSI_TX_PIXEL_FORMATSEL;

/**
    CSI_TX Lane select

    @note for csi_tx_set_config()
*/
typedef enum {
	CSI_TX_DATA_LANE_0 = 0,        ///< Data lane 0
	CSI_TX_DATA_LANE_1,            ///< Data lane 1
	CSI_TX_DATA_LANE_2,            ///< Data lane 2
	CSI_TX_DATA_LANE_3,            ///< Data lane 3

	CSI_TX_DATA_LANE_CNT,

	ENUM_DUMMY4WORD(CSI_TX_LANESEL)
} CSI_TX_LANESEL;

/**
    CSI_TX Lane select

    @note for csi_tx_set_config()
*/
typedef enum {
	CSI_TX_DATA_1_LANE = 0,        ///< 1 data lane
	CSI_TX_DATA_2_LANE,            ///< 2 data lanes
	CSI_TX_DATA_4_LANE,            ///< 4 data lanes

	CSI_TX_DATA_LANE_NUM,
    CSI_TX_DATA_LANE_SKIP = CSI_TX_DATA_LANE_NUM,   // skip lane number setting

	ENUM_DUMMY4WORD(CSI_TX_LANENUM)
} CSI_TX_LANENUM;


/**
    CSI_TX CLK ULP select

    @note for csi_tx_set_config()
*/
typedef enum {
	CSI_TX_ULP_EXIT = 0,           ///< Clock lane exit ULPS
	CSI_TX_ULP_ENTER = 1,          ///< Clock lane enter ULPS
	CSI_TX_ULP_SEL_CNT,

	ENUM_DUMMY4WORD(CSI_TX_ULP_SEL)
} CSI_TX_ULP_SEL;


/**
    CSI_TX escape control operation select

    @note for csi_tx_set_config()
*/
typedef enum {
	CSI_TX_ESCAPE_TRIGGER = 0,      ///< Escape command trigger
	CSI_TX_ESCAPE_START,            ///< Escape command start procedure
	CSI_TX_ESCAPE_STOP,             ///< Escape command stop procedure
	ENUM_DUMMY4WORD(CSI_TX_ESC_OP)
} CSI_TX_ESC_OP;


/**
    CSI_TX escape control operation stop action selection

    @note for csi_tx_set_config()
*/
typedef enum {
	CSI_TX_SET_ESC_NOT_STOP = 0x0,         ///< Escape command not stop
	CSI_TX_SET_ESC_STOP_WITH_ESC_CMD,      ///< Escape command stop but not send exit cmd
	CSI_TX_SET_ESC_STOP_WITHOUT_ESC_CMD,   ///< Escape command stopp and send exit cmd

	ENUM_DUMMY4WORD(CSI_TX_CFG_ESCAPE_CMD_STOP_TYPE)
} CSI_TX_CFG_ESCAPE_CMD_STOP_TYPE;

/**
    CSI_TX Functional Buffer Address Configuration Selection

    This definition is used in csi_tx_set_buf_address()/csi_tx_get_buf_address() specify which of the csi_tx function is selected
    to assign new configurations.
*/
typedef enum {
	CSI_TX_CONFIG_BUF_ID_0_ADDR,      ///< Configure CSI_TX BUFFER0 ADDR.
	CSI_TX_CONFIG_BUF_ID_1_ADDR,      ///< Configure CSI_TX BUFFER1 ADDR.
	CSI_TX_CONFIG_BUF_ID_2_ADDR,      ///< Configure CSI_TX BUFFER2 ADDR.
	ENUM_DUMMY4WORD(CSI_TX_CONFIG_BUF_ID)
} CSI_TX_CONFIG_BUF_ID;

/**
    CSI_TX Functional Configuration Selection

    This definition is used in csi_tx_set_config()/csi_tx_get_config() specify which of the csi_tx function is selected
    to assign new configurations.
*/
typedef enum {
	CSI_TX_CONFIG_ID_LOAD,
	CSI_TX_CONFIG_ID_PIXEL_FMT,    ///< Configure CSI_TX pinxel format. Use RAW8/YUV422/YUV420...
	CSI_TX_CONFIG_ID_DATALANE_NO,  ///< Configure CSI_TX  data lane number.
	CSI_TX_CONFIG_ID_DESKEW_PKT_EN,  ///< Configure CSI_TX  data lane number.
	CSI_TX_CONFIG_ID_BLANK_CTRL,   ///< Configure CSI_TX  blank control
	CSI_TX_CONFIG_ID_CLK_LP_CTRL,  ///< Configure CSI_TX clock enter LP or not.
	CSI_TX_CONFIG_ID_LINE_SP_VACT_EN,      ///< Configure CSI_TX LINE SP VACT enable/disable.
	CSI_TX_CONFIG_ID_LINE_SP_VBLK_EN,      ///< Configure CSI_TX LINE SP VBLK enable/disable.
	CSI_TX_CONFIG_ID_LINE_SP_WC_CTRL,      ///< Configure CSI_TX LINE SP WC value control.
	CSI_TX_CONFIG_ID_FRAME_SP_WC_CTRL,      ///< Configure CSI_TX FRAME SP WC value control.
	CSI_TX_CONFIG_ID_U_V_SWAP,     ///< Configure CSI_TX U/V swap
	CSI_TX_CONFIG_ID_Y_UV_SWAP,     ///< Configure CSI_TX Y/UV swap

	CSI_TX_CONFIG_ID_MODE,         ///< Configure CSI_TX mode
	///< @note for CSI_TX_CFG_MODE_SEL
	///< Context can be any of:
	///< - @b CSI_TX_MODE_AUTO_MODE  :
	///< - @b CSI_TX_MODE_MANUAL_MODE   :
	///< - @b CSI_TX_MODE_ONESHOT_MODE   :
	CSI_TX_CONFIG_BUFFER_ID,         ///< Configure CSI_TX BUFFER ID
	CSI_TX_CONFIG_CURR_BUFFER_ID,         ///< Configure CSI_TX CURRENT BUFFER ID

	CSI_TX_CONFIG_ID_PIXPKT_PH_DT, ///< Configure CSI_TX pixel Packet header DataType.
	CSI_TX_CONFIG_ID_PIXPKT_PH_VC, ///< Configure CSI_TX pixel Packet header VirtualChannel.
	CSI_TX_CONFIG_ID_BLANK_DATA,

	CSI_TX_CONFIG_ID_LINE_SP_WC_FIXED,      ///< Configure CSI_TX LINE SP WC VALUE.
	CSI_TX_CONFIG_ID_FRAME_SP_WC_FIXED,      ///< Configure CSI_TX FRAME SP WC VALUE.

	CSI_TX_CONFIG_ID_INITIAL_DESKEW_PKT_LEN,      ///< Configure CSI_TX INITIAL DESKEW PACKET LENGTH.
	CSI_TX_CONFIG_ID_PERIODIC_DESKEW_PKT_LEN,      ///< Configure CSI_TX PERIODIC DESKEW PACKET LENGTH.

	CSI_TX_CONFIG_ID_VTOTAL,       ///< Configure CSI_TX vertical total timing.
	CSI_TX_CONFIG_ID_VVALID_START, ///< Configure CSI_TX vertical valid start timing.

	CSI_TX_CONFIG_ID_VVALID_END,   ///< Configure CSI_TX vertical valid end timing.
	CSI_TX_CONFIG_ID_BLLP,         ///< Configure CSI_TX BLLP period, this period can trasmit HS packets or entering LP11.

	CSI_TX_CONFIG_ID_HBP,          ///< Configure CSI_TX horizontal back porch period.
	CSI_TX_CONFIG_ID_HFP,          ///< Configure CSI_TX horizontal front  porch period.

	CSI_TX_CONFIG_ID_HACT,         ///< Configure CSI_TX horizontal active period.
	CSI_TX_CONFIG_ID_DESKEW_DLY,   ///< Configure CSI_TX DESKEW PACKET DELAY.

	CSI_TX_CONFIG_ID_HINV,   ///< Configure CSI_TXhorizontal invalid period.

	CSI_TX_CONFIG_ID_TLPX,         ///< Configure CSI_TX LTPX timing.
	CSI_TX_CONFIG_ID_THS_PREPARE,  ///< Configure CSI_TX THS_PREPARE timing.
	CSI_TX_CONFIG_ID_THS_ZERO,     ///< Configure CSI_TX THS_ZERO timing.
	CSI_TX_CONFIG_ID_THS_TRAIL,    ///< Configure CSI_TX THS_TRAIL timing.
	CSI_TX_CONFIG_ID_THS_EXIT,     ///< Configure CSI_TX THS_EXIT timing.

	CSI_TX_CONFIG_ID_TWAKEUP,      ///< Configure CSI_TX wakeup timing.
	CSI_TX_CONFIG_ID_TCLK_PREPARE, ///< Configure CSI_TX TCLK_PREPARE timing.
	CSI_TX_CONFIG_ID_TCLK_ZERO,    ///< Configure CSI_TX TCLK_ZERO timing.
	CSI_TX_CONFIG_ID_TCLK_POST,    ///< Configure CSI_TX TCLK_POST timing.

	CSI_TX_CONFIG_ID_TCLK_PRE,     ///< Configure CSI_TX TCLK_PRE timing.
	CSI_TX_CONFIG_ID_TCLK_TRAIL,   ///< Configure CSI_TX TCLK_TRAIL timing.


	CSI_TX_CONFIG_ID_BUF_DIM_WIDTH,      ///< Configure CSI_TX FRAME WIDTH.
	CSI_TX_CONFIG_ID_BUF_DIM_HEIGHT,      ///< Configure CSI_TX FRAME HEIGHT.

	CSI_TX_CONFIG_ID_Y_LINE_OFFSET,      ///< Configure CSI_TX Y PLAIN OFFSET.
	CSI_TX_CONFIG_ID_UV_LINE_OFFSET,      ///< Configure CSI_TX UV PLAIN OFFSET.

	CSI_TX_CONFIG_ID_AXI_WRAP1_EN,      ///< Configure CSI_TX AXI WRAPPER1 ENABLE.
	CSI_TX_CONFIG_ID_AXI_WRAP2_EN,      ///< Configure CSI_TX AXI WRAPPER2 ENABLE.
	CSI_TX_CONFIG_ID_AXI_WRAP1_LOCK_EN, ///< Configure CSI_TX AXI WRAPPER1 lock ENABLE.
	CSI_TX_CONFIG_ID_AXI_WRAP2_LOCK_EN, ///< Configure CSI_TX AXI WRAPPER2 lock ENABLE.
	CSI_TX_CONFIG_ID_AXI_CH_DIS,        ///< Configure CSI_TX AXI CHANNEL DISABLE.

	CSI_TX_CONFIG_ID_ACMD_EN,      ///< Configure CSI_TX CMD ISSUE enable/disable
	CSI_TX_CONFIG_ID_ACMD_NUM,      ///< Configure CSI_TX CMD NUMBER

	CSI_TX_CONFIG_ID_PHY_HS_DRVING,   ///< Configure CSI_TX PHY HS driving
    CSI_TX_CONFIG_ID_PHY_LP_DRVING,   ///< Configure CSI_TX PHY LP driving

	CSI_TX_CONFIG_ID_CLK_PHASE_OFS,   ///< Configure CSI_TX PHY clock phase offset.
	CSI_TX_CONFIG_ID_DAT0_PHASE_OFS,  ///< Configure CSI_TX PHY data0 phase offset.
	CSI_TX_CONFIG_ID_DAT1_PHASE_OFS,  ///< Configure CSI_TX PHY data1 phase offset.
	CSI_TX_CONFIG_ID_DAT2_PHASE_OFS,  ///< Configure CSI_TX PHY data2 phase offset.
	CSI_TX_CONFIG_ID_DAT3_PHASE_OFS,  ///< Configure CSI_TX PHY data3 phase offset.
	CSI_TX_CONFIG_ID_PHY_HS_CLK_INV,     ///< Configure CSI_TX PHY high speed mode clock inverse.
	CSI_TX_CONFIG_ID_PHASE_DELAY_ENABLE_OFS, ///< Configure CSI_TX PHY phase delay enable.

	//CSI_TX_CFG_ID_FREQ
	CSI_TX_CONFIG_ID_FREQ,         ///< CSI_TX module target clock (Unit: Hz)
	CSI_TX_CONFIG_ID_LPFREQ,       ///< CSI_TX module LP clock (Unit: Hz)

	CSI_TX_CONFIG_ID_LANSEL_D0,    ///< Configure CSI_TX DAT0 Lane mapping
	CSI_TX_CONFIG_ID_LANSEL_D1,    ///< Configure CSI_TX DAT1 Lane mapping
	CSI_TX_CONFIG_ID_LANSEL_D2,    ///< Configure CSI_TX DAT2 Lane mapping
	CSI_TX_CONFIG_ID_LANSEL_D3,    ///< Configure CSI_TX DAT3 Lane mapping

	CSI_TX_CONFIG_ID_PHY_LP_RX_DAT0,///< Configure CSI_TX LP RX DAT0 enable/disable
	CSI_TX_CONFIG_ID_PHY_LP_RX_DAT1,///< Configure CSI_TX LP RX DAT1 enable/disable
	CSI_TX_CONFIG_ID_PHY_LP_RX_DAT2,///< Configure CSI_TX LP RX DAT2 enable/disable
	CSI_TX_CONFIG_ID_PHY_LP_RX_DAT3,///< Configure CSI_TX LP RX DAT3 enable/disable
    CSI_TX_CONFIG_ID_PHY_EMCTRL_C,  ///< PHY_REG_EMCTRL_C[3:0] >
	CSI_TX_CONFIG_ID_CHIP_VER,     ///< Get CSI_TX chip version

	CSI_TX_CONFIG_ID_FS_CNT,
	CSI_TX_CONFIG_ID_FE_CNT,
	CSI_TX_CONFIG_ID_TD_CNT,
	CSI_TX_CONFIG_ID_FU_CNT,
	CSI_TX_CONFIG_ID_BUF0_CNT,
	CSI_TX_CONFIG_ID_BUF1_CNT,
	CSI_TX_CONFIG_ID_BUF2_CNT,
	CSI_TX_CONFIG_ID_BUF_STS,
	CSI_TX_CONFIG_ID_INT_EN,

	CSI_TX_CONFIG_ID_HWLS_CNT,
	CSI_TX_CONFIG_ID_HWLE_CNT,
	CSI_TX_CONFIG_ID_HWFS_CNT,
	CSI_TX_CONFIG_ID_HWFE_CNT,
	CSI_TX_CONFIG_ID_HWLS_CLEAR,
	CSI_TX_CONFIG_ID_HWLE_CLEAR,
	CSI_TX_CONFIG_ID_HWFS_CLEAR,
	CSI_TX_CONFIG_ID_HWFE_CLEAR,

    CSI_TX_CONFIG_ID_IO_TEST_EN,
    CSI_TX_CONFIG_ID_IO_TEST_MODE_SEL,
    CSI_TX_CONFIG_ID_IO_TEST_OUT_DATA_SEL,
    CSI_TX_CONFIG_ID_IO_TEST_OUT_CLK_SEL,
    CSI_TX_CONFIG_ID_IO_TEST_DAT0_OUT,
    CSI_TX_CONFIG_ID_IO_TEST_DAT1_OUT,
    CSI_TX_CONFIG_ID_IO_TEST_DAT2_OUT,
    CSI_TX_CONFIG_ID_IO_TEST_DAT3_OUT,

    CSI_TX_CONFIG_ID_ESC_DAT0_TRIG,
    CSI_TX_CONFIG_ID_ESC_DAT1_TRIG,
    CSI_TX_CONFIG_ID_ESC_DAT2_TRIG,
    CSI_TX_CONFIG_ID_ESC_DAT3_TRIG,
    CSI_TX_CONFIG_ID_ESC_DAT0_START,
    CSI_TX_CONFIG_ID_ESC_DAT1_START,
    CSI_TX_CONFIG_ID_ESC_DAT2_START,
    CSI_TX_CONFIG_ID_ESC_DAT3_START,
    CSI_TX_CONFIG_ID_ESC_DAT0_STOP,
    CSI_TX_CONFIG_ID_ESC_DAT1_STOP,
    CSI_TX_CONFIG_ID_ESC_DAT2_STOP,
    CSI_TX_CONFIG_ID_ESC_DAT3_STOP,
    CSI_TX_CONFIG_ID_ESC_DAT0_CMD,
    CSI_TX_CONFIG_ID_ESC_DAT1_CMD,
    CSI_TX_CONFIG_ID_ESC_DAT2_CMD,
    CSI_TX_CONFIG_ID_ESC_DAT3_CMD,

	ENUM_DUMMY4WORD(CSI_TX_CONFIG_ID)
} CSI_TX_CONFIG_ID;

// -----------------------------------------------------------------------------
// CSI_TX Command RW control (0x2C)
// -----------------------------------------------------------------------------
/**
    CSI_TX command RW control configuration

    @note for csi_tx_set_cmd_rw_ctrl()
*/
typedef enum {
	CSI_TX_SET_CMD_NUMBER = 0x0,           ///< The number of (CSI_TX_CMD_NUMBER+1) commands would be sent in CSI_TX_MODE= 0/3/4.
	///< Context is
	///< - @b UINT32 : 1 - 8
	CSI_TX_SET_SRAM_READ_OFS,              ///< Context is
	///< - @b UINT32 : Sram offset 0~255 (256 bytes total)

	CSI_TX_RW_CMD_CTRL_CNT,
	ENUM_DUMMY4WORD(CSI_TX_CFG_CMD_RW_CTRL)
} CSI_TX_CFG_CMD_RW_CTRL;


// -----------------------------------------------------------------------------
// CSI_TX Command Register 0,1
// -----------------------------------------------------------------------------
/**
    CSI_TX packet type

    @note for csi_tx_set_cmd_register()
*/
//@{
typedef enum {
	CSI_TX_SHORT_PACKET = 0x0,             ///< This CSI_TX short packet
	CSI_TX_LONG_PACKET,                    ///< This CSI_TX long packet

	CSI_TX_PT_CNT,
	ENUM_DUMMY4WORD(CSI_TX_PACKET_TYPE)
} CSI_TX_PACKET_TYPE;
//@}

/**
    CSI_TX command register set

    @note for csi_tx_set_cmd_register()
*/
typedef enum {
	CSI_TX_CMD_REG0 = 0x0,                 ///< Command register 0
	CSI_TX_CMD_REG1,                       ///< Command register 1
	CSI_TX_CMD_REG2,                       ///< Command register 2
	CSI_TX_CMD_REG3,                       ///< Command register 3
	CSI_TX_CMD_REG4,                       ///< Command register 4
	CSI_TX_CMD_REG5,                       ///< Command register 5
	CSI_TX_CMD_REG6,                       ///< Command register 6
	CSI_TX_CMD_REG7,                       ///< Command register 7

	CSI_TX_CMD_SET_CNT,
	ENUM_DUMMY4WORD(CSI_TX_CMD_REG_NUM)
} CSI_TX_CMD_REG_NUM;


/**
    CSI_TX command register configuration

    @note for csi_tx_set_cmd_register()
*/
typedef enum {
	CSI_TX_SET_CMD_DT = 0x0,                   // The command n Data Type in the Data Identification (DI) field of the CSI_TX packet header.
	CSI_TX_SET_CMD_VC,                         // The virtual channel ID for the command n
	CSI_TX_SET_CMD_WC,                         // If the command n is the Short Packet, this field is the command data for this short packet command.
	// If the command n is the Long Packet, this field is the length for this long packet command.
	// The long packet command data is stored in the SRAM in serially by commands. The command data n is from the SRAM address offset 0.

	CSI_TX_SET_CMD_DATA = CSI_TX_SET_CMD_WC,
	CSI_TX_SET_CMD_PT,                         // The Packet Type of the command n.
	// 0: Short Packet
	// 1: Long Packet
	ENUM_DUMMY4WORD(CSI_TX_CFG_CMD_REG)
} CSI_TX_CFG_CMD_REG;

/**
    CSI_TX Command control

    Set CSI_TX Command control
*/
typedef struct {
	UINT32  ui_data_type;                  ///< data type
	UINT32  ui_virtual_channel;            ///< virtual channel id

	UINT32  ui_packet_type;                ///< packet type
	//UINT32  ui_dcs_cmd;                    ///< dcs commnd
	UINT8   *ui_param;                     ///< point to parameter
	UINT32  ui_param_cnt;                  ///< parmeter count
} CSI_TX_CMD_CTRL_PARAM, *PCSI_TX_CMD_CTRL_PARAM;

/**
    CSI_TX Command R/W control

    Set CSI_TX Command R/W control
*/
typedef struct {
	UINT32                  ui_cmd_no;     ///< commnad number
	//BOOL                    b_bta_en;      ///< BTA enable/disable
	//BOOL                    b_bta_only;    ///< issue BTA only
	UINT32                  ui_sram_ofs;   ///< set the sram offset
	//BOOL                    b_eot_en;      ///< set the EOT enable or not
	PCSI_TX_CMD_CTRL_PARAM     p_csi_tx_cmd_ctx; ///< set commnd control
} CSI_TX_CMD_RW_CTRL_PARAM, *PCSI_TX_CMD_RW_CTRL_PARAM;

// csi_tx_set_general_config(&general_cfg);
typedef struct {
	UINT32 frequency;
    CSI_TX_MODESEL tx_mode;
    CSI_TX_LANENUM lane_num;
    CSI_TX_PIXEL_FORMATSEL pixel_fmt;
    UINT32 frame_width;
    UINT32 frame_height;
    UINT32 pixel_hdr_dt;
	UINT32 frame_rate;
} CSI_TX_GENERAL_CFG;


// CSI_TX_MODE_AUTO_MODE: buf_0 ~ buf_2 round robin
// CSI_TX_MODE_MANUAL_MODE: use one buffer which CSI_TX_CONFIG_BUFFER_ID indicate
// CSI_TX_MODE_ONESHOT_MODE: use one buffer which CSI_TX_CONFIG_BUFFER_ID indicate
typedef union {
    uintptr_t buf_addr[6];
    struct {
        uintptr_t y_buf_0;
        uintptr_t uv_buf_0;
        uintptr_t y_buf_1;
        uintptr_t uv_buf_1;
        uintptr_t y_buf_2;
        uintptr_t uv_buf_2;
    };
} CSI_TX_BUFFER_CFG;

/**
    CSI_TX D_PHY

    @note for CSI_TX D_PHY
*/

typedef enum {
	CSI_TX_PHY1 = 0X0,
    CSI_TX_PHY2 = 0x1,

	ENUM_DUMMY4WORD(CSI_TX_PHY_ID)
} CSI_TX_PHY_ID;

#ifdef __KERNEL__
extern void 	csi_tx_create_resource(void);
extern void 	csi_tx_release_resource(void);
extern void 	csi_tx_set_base_addr(CSI_TX_DATA_TYPE addr);
extern void 	csi_tx_phy_set_base_addr(CSI_TX_PHY_DATA_TYPE addr);
extern void		csi_tx_isr(void);
// tx2 
#if 0
extern void 	csi_tx2_create_resource(void);
extern void 	csi_tx2_release_resource(void);
extern void 	csi_tx2_set_base_addr(CSI_TX_DATA_TYPE addr);
extern void 	csi_tx2_phy_set_base_addr(CSI_TX_PHY_DATA_TYPE addr);
extern void		csi_tx2_isr(void);
#endif

#endif

extern ER       csi_tx_open(void);
extern ER       csi_tx_close(void);
extern BOOL     csi_tx_is_opened(void);
extern ER       csi_tx_set_tx_en(BOOL b_en, BOOL b_wait);
extern BOOL		csi_tx_get_tx_en(void);
extern ER       csi_tx_wait_tx_done(void);
extern ER 		csi_tx_wait_frame_end(UINT32 *csi_tx_buf_sts);
extern ER		csi_tx_set_cb_func(CSITX_CB p_cb_func);
extern ER       csi_tx_ulps_trigger(void);
extern ER       csi_tx_set_lps_clock_sel(CSI_TX_ULP_SEL ulp_sel);
extern ER       csi_tx_set_config(CSI_TX_CONFIG_ID cfg_id, UINT32 config_value);
extern UINT32   csi_tx_get_config(CSI_TX_CONFIG_ID cfg_id);
extern ER       csi_tx_set_lp_dcs_command(CSI_TX_LANESEL data_lane, UINT32 lp_cmd);
extern ER       csi_tx_set_escape_entry(CSI_TX_LANESEL data_lane, UINT32 entry_cmd, BOOL b_stop);
extern ER       csi_tx_set_escape_control(CSI_TX_LANESEL data_lane, CSI_TX_ESC_OP esc_op, BOOL b_en);
extern ER       csi_tx_set_cmd_register(CSI_TX_CMD_REG_NUM cmd_reg_no, CSI_TX_CFG_CMD_REG cmd_reg, UINT32 param);
extern ER       csi_tx_get_cmd_register(CSI_TX_CMD_REG_NUM ui_cmd_reg_no, CSI_TX_CFG_CMD_REG ui_cmd_reg, UINT32 *param);
extern ER       csi_tx_reset_acmd_sram(void);
extern ER       csi_tx_set_acmd_buf(UINT8 cmd_id, UINT32 *data_buf);
extern ER       csi_tx_lunch_acmd_buf(void);
extern ER       csi_tx_set_escape_transmission(CSI_TX_LANESEL data_lane, UINT32 cmd, CSI_TX_CFG_ESCAPE_CMD_STOP_TYPE exit);
extern UINT32   csi_tx_get_hact(CSI_TX_PIXEL_FORMATSEL pixel_fmt, UINT32 frame_width);
extern ER       csi_tx_set_general_config(CSI_TX_GENERAL_CFG *config);
extern ER 		csi_tx_set_buf_address(CSI_TX_CONFIG_BUF_ID buf_id, uintptr_t dma_y_buf_addr, uintptr_t dma_uv_buf_addr);
extern ER 		csi_tx_trigger_buf_address(int uv_en, uintptr_t dma_ybuf_addr,  uintptr_t dma_uvbuf_addr);
extern ER       csi_tx_set_buffer_config(CSI_TX_BUFFER_CFG *config);
extern void     csi_tx_dump_info(void);

// tx2 
#if 0
extern ER       csi_tx2_open(void);
extern ER       csi_tx2_close(void);
extern BOOL     csi_tx2_is_opened(void);
extern ER       csi_tx2_set_tx_en(BOOL b_en, BOOL b_wait);
extern BOOL		csi_tx2_get_tx_en(void);
extern ER       csi_tx2_wait_tx_done(void);
extern ER 		csi_tx2_wait_frame_end(UINT32 *csi_tx_buf_sts);
extern ER       csi_tx2_ulps_trigger(void);
extern ER       csi_tx2_set_lps_clock_sel(CSI_TX_ULP_SEL ulp_sel);
extern ER       csi_tx2_set_config(CSI_TX_CONFIG_ID cfg_id, UINT32 config_value);
extern UINT32   csi_tx2_get_config(CSI_TX_CONFIG_ID cfg_id);
extern ER       csi_tx2_set_lp_dcs_command(CSI_TX_LANESEL data_lane, UINT32 lp_cmd);
extern ER       csi_tx2_set_escape_entry(CSI_TX_LANESEL data_lane, UINT32 entry_cmd, BOOL b_stop);
extern ER       csi_tx2_set_escape_control(CSI_TX_LANESEL data_lane, CSI_TX_ESC_OP esc_op, BOOL b_en);
extern ER       csi_tx2_set_cmd_register(CSI_TX_CMD_REG_NUM cmd_reg_no, CSI_TX_CFG_CMD_REG cmd_reg, UINT32 param);
extern ER       csi_tx2_get_cmd_register(CSI_TX_CMD_REG_NUM ui_cmd_reg_no, CSI_TX_CFG_CMD_REG ui_cmd_reg, UINT32 *param);
extern ER       csi_tx2_reset_acmd_sram(void);
extern ER       csi_tx2_set_acmd_buf(UINT8 cmd_id, UINT32 *data_buf);
extern ER       csi_tx2_lunch_acmd_buf(void);
extern ER       csi_tx2_set_escape_transmission(CSI_TX_LANESEL data_lane, UINT32 cmd, CSI_TX_CFG_ESCAPE_CMD_STOP_TYPE exit);
extern UINT32   csi_tx2_get_hact(CSI_TX_PIXEL_FORMATSEL pixel_fmt, UINT32 frame_width);
extern ER       csi_tx2_set_general_config(CSI_TX_GENERAL_CFG *config);
extern ER 		csi_tx2_set_buf_address(CSI_TX_CONFIG_BUF_ID buf_id, uintptr_t dma_y_buf_addr, uintptr_t dma_uv_buf_addr);
extern ER 		csi_tx2_trigger_buf_address(int uv_en, uintptr_t dma_ybuf_addr,  uintptr_t dma_uvbuf_addr);
extern ER       csi_tx2_set_buffer_config(CSI_TX_BUFFER_CFG *config);
extern void     csi_tx2_dump_info(void);
#endif
/* csi_tx int*/
#if defined(__FREERTOS)
#if defined(_TC18039_) && defined(_NVT_FPGA_)
extern void csi_tx_phy_init(void);
#endif
extern ER csi_tx_write_phy_reg(UINT32 uiOffset, UINT32 uiValue);
extern ER csi_tx_read_phy_reg(UINT32 uiOffset, UINT32 *puiValue);
#endif


#ifdef __KERNEL__
extern void csi_tx_tasklet(int id);
//extern void csi_tx2_tasklet(int id);
#endif


//@}

#endif
