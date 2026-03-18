/**
    Public header file for kdrv_dce

    This file is the header file that define the API and data type for kdrv_csi_tx.

    @file       csi_tx_kdrv.h
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2021.    All rights reserved.
*/
#ifndef _KDRV_CSI_TX_H_
#define _KDRV_CSI_TX_H_

/**
    CSI_TX PKTS CONFIG structure - parameters
*/
typedef struct {
	UINT32 PKT_FMT;                              ///< 
	UINT32 DESKEW_PKT_EN;                              ///< 
	UINT32 BLANK_CTRL;                              ///< 
	UINT32 CLK_LP_CTRL;                          ///< 
	UINT32 LINE_SP_VACT_EN;                              ///< 
	UINT32 LINE_SP_VBLK_EN;                          ///< 
	UINT32 LINE_SP_WC_CTRL;                              ///< 
	UINT32 FRAME_SP_WC_CTRL;                          ///< 
	UINT32 U_V_SWAP;                           ///< 
	UINT32 Y_UV_SWAP;                           ///< 
	UINT32 BUFFER_ID;                           ///< 
	UINT32 PIXPKT_PH_DT;                           ///< 
	UINT32 PIXPKT_PH_VC;                           ///< 
	UINT32 BLANK_DATA;                           ///< 
	UINT32 LINE_SP_WC_FIXED;                           ///< 
	UINT32 FRAME_SP_WC_FIXED;                           ///< 
	UINT32 INITIAL_DESKEW_PKT_LEN;                           ///< 
	UINT32 PERIODIC_DESKEW_PK_LEN;                           ///< 
} KDRV_CSI_TX_PKTS_CONFIG_PARAM;

/**
    CSI_TX CMD CONFIG structure - parameters
*/
typedef struct {
	UINT32 ACMD_EN;                              ///< 
	UINT32 ACMD_NUM;                          ///< 
	UINT32 CMD_DT;                              ///< 
	UINT32 CMD_VC;                          ///< 
	UINT32 CMD_WC;                              ///< 
	UINT32 CMD_PT;                          ///< 
	UINT32 CMD_ID;                           ///< 
} KDRV_CSI_TX_CMD_CONFIG_PARAM;
	
/**
    CSI_TX TIMEING CONFIG structure - parameters
*/
typedef struct {
	UINT32 VTOTAL;                              ///< 
	UINT32 VVALID_START;                          ///< 
	UINT32 VVALID_END;                              ///< 
	UINT32 BLLP;                          ///< 
	UINT32 HBP;                              ///< 
	UINT32 HFP;                          ///< 
	UINT32 HACT;                           ///< 
	UINT32 DESKEW_DLY;                           ///<
} KDRV_CSI_TX_TIMEING_CONFIG_PARAM;

/**
    CSI_TX BUS TIMEING CONFIG structure - parameters
*/
typedef struct {
	UINT32 TLPX;                              ///< 
	UINT32 THS_PREPARE;                          ///< 
	UINT32 THS_ZERO;                              ///< 
	UINT32 THS_TRAIL;                          ///< 
	UINT32 THS_EXIT;                              ///< 
	UINT32 TWAKEUP;                          ///< 
	UINT32 TCLK_PREPARE;                           ///< 
	UINT32 TCLK_ZERO;                           ///<
	UINT32 TCLK_POST;                           ///<
	UINT32 TCLK_PRE;                           ///<
	UINT32 TCLK_TRAIL;                           ///<
} KDRV_CSI_TX_BUS_TIMEING_CONFIG_PARAM;

/**
    CSI_TX BUFFER CONFIG structure - parameters
*/
typedef struct {
	ULONG BUFFER_MODE;                              ///< 
	ULONG BUF0_0_ADDR;                              ///< 
	ULONG BUF0_1_ADDR;                          ///< 
	ULONG BUF1_0_ADDR;                              ///< 
	ULONG BUF1_1_ADDR;                          ///< 
	ULONG BUF2_0_ADDR;                              ///< 
	ULONG BUF2_1_ADDR;                          ///< 
	UINT32 FRAME_WIDTH;                           ///< 
	UINT32 FRAME_HEIGHT;                           ///<
	UINT32 Y_LINE_OFFSET;                           ///<
	UINT32 UV_LINE_OFFSET;                           ///<
} KDRV_CSI_TX_BUFF_CONFIG_PARAM;

/**
    KDRV CSI_TX parameter select id

    This is used in kdrv_csi_tx_set and kdrv_csi_tx_get to set and get the CSI_TX configuration parameter.
*/
typedef enum {
	CSI_TX_CTRL_BASE = 0x00000000,
	CSI_TX_WAIT_FRM_END,                  ///< [set] Wait csi_tx engine's frame end.
	CSI_TX_ENABLE,                        ///< [set/get] Set csi_tx engine's global enable/disable.
	CSI_TX_PKTS_CONFIG,
	CSI_TX_CMD_CONFIG,
	CSI_TX_TIMEING_CONFIG,
	CSI_TX_BUS_TIMEING_CONFIG,
	CSI_TX_BUFF_CONFIG,	
	
	ENUM_DUMMY4WORD(KDRV_CSI_TX_PARAM_ID)
} KDRV_CSI_TX_PARAM_ID;

/*!
 * @fn INT32 kdrv_csi_tx_open(void)
 * @brief open hardware engine
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_csi_tx_open(void);

/*!
 * @fn INT32 kdrv_csi_tx_close(void)
 * @brief close hardware engine
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_csi_tx_close(void);

/*!
 * @brief trigger hardware engine to load new config
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_csi_tx_trigger(KDRV_CALLBACK_FUNC *p_cb_func);

/*!
 * @fn INT32 kdrv_csi_tx_set(UINT32 handler, CSI_TX_ID id, VOID *p_param)
 * @brief set parameters to hardware engine
 * @param id             the id of parameters
 * @param param          the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_csi_tx_set(KDRV_CSI_TX_PARAM_ID id, VOID *p_param);

/*!
 * @fn INT32 kdrv_csi_tx_get(UINT32 handler, CSI_TX_ID id, VOID *p_param)
 * @brief set parameters to hardware engine
 * @param id             the id of parameters
 * @param param          the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_csi_tx_get(KDRV_CSI_TX_PARAM_ID id, VOID *p_param);


#endif //_KDRV_CSI_TX_H_
