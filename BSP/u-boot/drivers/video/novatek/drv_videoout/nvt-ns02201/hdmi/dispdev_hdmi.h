/*
    TV HDMI TX module internal header file

    TV HDMI TX module internal header file

    @file       dispdev_hdmi.h
    @ingroup    mIDISPHDMITx
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _DISPDEV_HDMI_H
#define _DISPDEV_HDMI_H

#include "hdmitx_int.h"



#define DISPHDMI_DEBUG              DISABLE
#if DISPHDMI_DEBUG
#define disphdmi_debug(msg)         DBG_WRN msg
#else
#define disphdmi_debug(msg)
#endif


#define ACT_HIGH                    0   //Signal change level at PIXCLK positive edge
#define ACT_LOW                     1   //Signal change level at PIXCLK negative edge


typedef struct {
	HDMI_VIDEOID    VID;                ///< Video ID

	BOOL            HS;                 ///< H-Sync Active edge
	BOOL            VS;                 ///< V-Sync Active edge
	BOOL            HVLD;               ///< H-Valid Active edge
	BOOL            VVLD;               ///< V-Valid Active edge
	BOOL            CLK;                ///< Pixel-Clock Active edge
	BOOL            DE;                 ///< Data Enable Active edge
	BOOL            PROGRESSIVE;        ///< Progressive/Interlaced Selection

	UINT32          H_SYNCTIME;         ///< H-Sync Time
	UINT32          H_TOTAL;            ///< H-Total Time
	UINT32          H_STARTTIME;        ///< H-Start Time
	UINT32          H_ENDTIME;          ///< H-End Time

	UINT32          V_SYNCTIME;         ///< V-Sync Time
	UINT32          V_TOTAL;            ///< V-Total Time
	UINT32          V_ODD_STARTTIME;    ///< V-Odd field Start Time
	UINT32          V_ODD_ENDTIME;      ///< V-Odd field End Time
	UINT32          V_EVEN_STARTTIME;   ///< V-Even field Start Time
	UINT32          V_EVEN_ENDTIME;     ///< V-Even field End Time

	UINT32          WINX_OFS;           ///< IDE Window Offset X when TV overscan
	UINT32          WINY_OFS;           ///< IDE Window Offset Y when TV overscan
} DISPDEV_HDMI_TIMING, *PDISPDEV_HDMI_TIMING;


typedef enum {
	HDMI_MODE_RGB444 = 0, ///<    HDMI Output format is RGB-444
	HDMI_MODE_YCBCR422,  ///<    HDMI Output format is YCbCr-422
	HDMI_MODE_YCBCR444,  ///<    HDMI Output format is YCbCr-444
	HDMI_MODE_YCBCR420,  ///<    HDMI Output format is YCbCr-444

	ENUM_DUMMY4WORD(HDMI_MODE)
} HDMI_MODE;

typedef enum {
	HDMI_ID_1 = 0,  ///<    HDMI engine 1
	HDMI_ID_2, 	    ///<    HDMI engine 2
	HDMI_ID_MAX,

	ENUM_DUMMY4WORD(HDMI_ID)
} HDMI_ID;




#define HDMI_PLL216     0x240000 // 216*131072/12 For 13.5/27/54/108MHz
#define HDMI_PLL252     0x2A0000 // 252*131072/12 For 25.2MHz
#define HDMI_PLL297     0x318000 // 297*131072/12 For 74.25MHz
#define HDMI_PLL324     0x360000
#define HDMI_PLL260     0x2B5555
#define HDMI_PLL241     0x284000
#define HDMI_PLL594     0x630000

#define HDMI_PLL215P78  0x23F6C8
#define HDMI_PLL251P17	0x29F53F
#define HDMI_PLL259P74  0x2B4A3D
#define HDMI_PLL323P67  0x35F22D
#define HDMI_PLL241P25  0x284000
#define HDMI_PLL593P4	0x62E6A7

#define HDMI_PLL_DIV(x,y) x/y*1000


#if _FPGA_EMULATION_
// APIs for FPGA Verification Usage Only.
extern void hdmitx_set_pg_enable(BOOL b_en);
extern void hdmitx_set_pg_pattern(UINT32 ui_pat_id, UINT32 pxl_rpt_cnt);
extern void hdmitx_set_pg_1(UINT32 gray, UINT32 HTOT);
extern void hdmitx_set_pg_2(UINT32 HSW, UINT32 HAB);
extern void hdmitx_set_pg_3(UINT32 HAW, UINT32 VTOT);
extern void hdmitx_set_pg_4(UINT32 VSW, UINT32 VAB);
extern void hdmitx_set_pg_5(UINT32 VAW);
extern void hdmitx_reset_sil9002(void);
#endif

/*
	HDMI function object
*/
typedef struct {
	void 	(*hdmitx_set_config)(HDMI_CONFIG_ID config_id, UINT32 ui_config);
	UINT32  (*hdmitx_get_config)(HDMI_CONFIG_ID config_id);
	UINT32  (*hdmitx_get_edid_info)(void);
}HDMI_OBJ, *PHDMI_OBJ;



#endif
