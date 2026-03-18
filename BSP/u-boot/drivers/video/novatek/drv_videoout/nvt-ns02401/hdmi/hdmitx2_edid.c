/*
    HDMI TX module EDID Parser

    HDMI TX module EDID Parser.

    @file       hdmitx2_edid.c
    @ingroup    mIDrvDisp_HDMITx
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#include "hdmitx_int.h"
#include "hdmitx_reg.h"

#if defined(__FREERTOS)
#include "rcw_macro.h"
#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include <kwrap/debug.h>
#endif
#include <dm.h>
#include <i2c.h>


//#include <kwrap/verinfo.h>




// The last one is used to store the valid VID number
static UINT8        g_svd_vid[HDMITX_EDIDNUM + 1];
static UINT8        g_svd_hdmivic[8];//1.4b extended 4K format
static UINT8        g_svd_3d_cap[HDMITX_EDIDNUM + 1];
static UINT32       gui_vsdb_h20cap;//store the HDMI2.0 VSDB byte 4-7
static UINT32       gui_hdmi_edid_info   = 0x00;
static UINT32       gui_native_edid_info = 0x00;
static UINT32       hdmi_ext_blk = 1;
static BOOL			hdmi_edid_error = 0;

static BOOL         gb_valid_resolution = FALSE;

extern I2C_SESSION  hdmi2_i2c_ses;
extern BOOL         hdmi2_i2c_select;
#ifdef __KERNEL__
extern HDMI_I2C_OBJ hdmi2_i2c_obj[1];
#else
extern HDMI_I2C_OBJ hdmi2_i2c_obj[2];
#endif
extern UINT32       hdmi2_manufacturer_id;
extern BOOL         b_hdmi2_i2c_opened;

static  ER      hdmitx2_ddc_read(UINT32 addr, UINT32 segm, UINT32 offset, UINT32 cnt);
static  UINT32  hdmitx2_parse_edid_header(void);
static  UINT32  hdmitx2_parse_edid_version(void);
static  UINT32  hdmitx2_verify_edid_checksum(UINT32 blk_num);
static  void    hdmitx2_parse_dtd(UINT32 ui_addr);
static  void    hdmitx2_parse_dtd_in_block_one(void);
static  void    hdmitx2_parse_dtd_in_ext_block(UINT32 ofs);
static  void    hdmitx2_parse_native_format(UINT32 blk_num);
static  UINT32  hdmitx2_parse_ext_block(UINT32 blk_num);
static  void    hdmitx2_parse_audio_std(UINT32 stdaddr, UINT32 stdreg);
static  void    hdmitx2_parse_video_std(UINT32 stdaddr, UINT32 stdreg);
static  void    hdmitx2_parse_std(UINT32 ofs);

static UINT8    hdmitx2_ddc_read_byte(UINT8 segm, UINT8 offset);
static ER       hdmitx2_ddc_read_byte2(UINT32 segm, UINT32 offset, UINT32 *pui_data);

#if 1
/*
    We check if the EDID header is correct sequence: {0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00}.
    The EDID parser would be stop if the header sequence check fail.
*/
static UINT32 hdmitx2_parse_edid_header(void)
{
	UINT8   i;

	// The EDID header: {0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00}
	if ((hdmitx2_ddc_read_byte(0x00, 0x00) == 0x00) && (hdmitx2_ddc_read_byte(0x00, 0x07) == 0x00)) {
		for (i = 1; i < 7; i++) {
			if (hdmitx2_ddc_read_byte(0x00, i) != 0xff) {
				hdmitx_edidmsg(("hdmitx2_Parse_EDIDHeader: Non-Legal EDID Header! FAIL!\r\n"));
				return 1;
			}
		}
	} else {
		hdmitx_edidmsg(("hdmitx2_Parse_EDIDHeader: Non-Legal EDID Header! FAIL!\r\n"));
		return 1;
	}

	// Get the Manufacturer-ID from the EDID header. This information can be used also in the CEC application.
	hdmi2_manufacturer_id = (hdmitx2_ddc_read_byte(0x00, 0x08) << 8) + hdmitx2_ddc_read_byte(0x00, 0x09);

	hdmitx_edidmsg(("hdmitx2_Parse_EDIDHeader: Legal EDID Header! ManID=0x%04X\r\n", hdmi2_manufacturer_id));
	return 0;

}

static UINT32 hdmitx2_parse_edid_version(void)
{
	// Check if the EDID version code is larger than V1.3. (hdmi compatible)
	// Assert fail if the version code is old.
	if ((hdmitx2_ddc_read_byte(0x00, 0x12) == 1) && (hdmitx2_ddc_read_byte(0x00, 0x13) >= 3)) {
		return 0;
	} else {
		return 1;
	}
}

/*
    Check the CheckSum value is correct for the certain EDID Block.
*/
static UINT32 hdmitx2_verify_edid_checksum(UINT32 blk_num)
{
	UINT32 ofs, i;
	UINT32 CRC = 0;

	ofs = 0x80 * (blk_num & 0x01);

	// Sum the first 127 Bytes of the specified EDID block.
	for (i = 0; i < 127; i++) {
		CRC += hdmitx2_ddc_read_byte((blk_num >> 1), i + ofs);
	}

	CRC &= 0xFF;
	CRC = 256 - CRC;
	CRC &= 0xFF;

	hdmitx_edidmsg(("CRC = 0x%02X\r\n", CRC));

	// Check if the CheckSum is correct with the 128th Byte.
	if (hdmitx2_ddc_read_byte((blk_num >> 1), ofs + 0x7F) != CRC) {
		return 1;
	} else {
		return 0;
	}

}

// Parsing Monitor Range Limits Packet
// Please refer to the spec of the CEA-861 for details.
static void hdmitx2_parse_mrl(UINT32 mrl)
{
	UINT32 max_v_rate, max_h_rate, max_pix_clk;

	max_v_rate = hdmitx2_ddc_read_byte(0x00, mrl + 6); //mrl[6];
	max_h_rate = hdmitx2_ddc_read_byte(0x00, mrl + 8); //mrl[8];
	max_pix_clk = hdmitx2_ddc_read_byte(0x00, mrl + 9); //mrl[9];

	hdmitx_edidmsg(("Monitor Max-V-rate = %d hz\r\n", max_v_rate));
	hdmitx_edidmsg(("Monitor Max-H-rate = %d kHz\r\n", max_h_rate));
	hdmitx_edidmsg(("Monitor Max-PixClock = %d MHz\r\n", (max_pix_clk * 10)));

	// These information is checked for DVI device.
	// If the display pixel-clock/Max-H-rate/Max-V-Rate abilities are larger than 720P60/1080I60/1080P60.
	// Then we think the display can afford these input resolutions.
	if (gb_valid_resolution && (max_v_rate >= 60) && (max_h_rate >= 33) && ((max_pix_clk * 10) >= 80)) {
		gui_hdmi_edid_info |= (HDMITX_DTD640X480P60 | HDMITX_DTD720X480P60 | HDMITX_DTD1080I60 | HDMITX_DTD720X480I60);

		if (max_h_rate >= 45) {
			gui_hdmi_edid_info |= HDMITX_DTD720P60;
		}

		if (((max_pix_clk * 10) >= 140) && (max_h_rate >= 67)) {
			gui_hdmi_edid_info |= HDMITX_DTD1080P60;
		}
	}

}

/*
    Parse the EDID Detailed-Timing-Descriptor (DTD).
    The EDID may contain several DTDs in the EDID,
    these information can help the determine the display abilities and timings.
    The detailed information of the DTD data struture is listed in the spec CEA-861.
*/
static void hdmitx2_parse_dtd(UINT32 ui_addr)
{
	UINT32  temp;
	UINT32  temp1, temp2;
	UINT32  h_resolution, v_resolution, h_blanking, v_blanking;
	UINT32  pixel_clk, v_total;
	UINT8   hz;


	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 1);            //DTD[1];
	temp = temp << 8;
	pixel_clk = temp + hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 0); //DTD[0];
	hdmitx_edidmsg(("Pixel clock is  %u hz\n\r",  pixel_clk * 10000));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 4);            //DTD[4];
	temp = (temp << 4) & 0x0f00;
	h_resolution = temp + hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 2); //DTD[2];
	hdmitx_edidmsg(("Horizontal Active Width is  %u pixels\n\r", h_resolution));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 4);            //DTD[4];
	temp = (temp << 8) & 0x0f00;
	h_blanking = temp + hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 3); //DTD[3];
	hdmitx_edidmsg(("Horizontal Blanking is  %u pixels\n\r",  h_blanking));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 7);            //DTD[7];
	temp = (temp << 4) & 0x0f00;
	v_resolution = temp + hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 5); //DTD[5];
	hdmitx_edidmsg(("Vertical Active is  %u lines\n\r",  v_resolution));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 7);            //DTD[7];
	temp = (temp << 8) & 0x0f00;
	v_blanking = temp + hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 6); //DTD[6];
	hdmitx_edidmsg(("Vertical Blanking is  %u lines\n\r",  v_blanking));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 11);           //DTD[11];
	temp = (temp << 2) & 0x0300;
	temp = temp + hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 8);     //DTD[8];
	hdmitx_edidmsg(("Horizontal Sync Offset is  %u pixels\n\r",  temp));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 11);           //DTD[11];
	temp = (temp << 4) & 0x0300;
	temp = temp + hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 9);     //DTD[9];
	hdmitx_edidmsg(("Horizontal Sync Pulse is  %u pixels\n\r",  temp));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 11);           //DTD[11];
	temp = (temp << 2) & 0x0030;
	temp = temp + (hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 10) >> 4); //DTD[10]
	hdmitx_edidmsg(("Vertical Sync Offset is  %u lines\n\r",  temp));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 11);           //DTD[11];
	temp = (temp << 4) & 0x0030;
	temp = temp + (hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 8) & 0x0f); //DTD[8]
	hdmitx_edidmsg(("Vertical Sync Pulse is  %u lines\n\r",  temp));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 14);           //DTD[14];
	temp = (temp << 4) & 0x0f00;
	//H_image_size = temp + hdmitx2_ddc_read_byte(ui_addr>>8, (ui_addr&0xFF)+12);  //DTD[12];
	hdmitx_edidmsg(("Horizontal Image size is  %u mm\n\r",  temp));

	temp = hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 14);           //DTD[14];
	temp = (temp << 8) & 0x0f00;
	//V_image_size = temp + hdmitx2_ddc_read_byte(ui_addr>>8, (ui_addr&0xFF)+13);  //DTD[13];
	hdmitx_edidmsg(("Vertical Image size is  %u mm\n\r",  temp));

	hdmitx_edidmsg(("Horizontal Border is  %u\n\r",  hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 15))); //DTD[15]
	hdmitx_edidmsg(("Vertical Border is  %u\n\r",  hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 16))); //DTD[16]

	if (hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 17) & 0x80) {
		//DTD[17]
		hdmitx_edidmsg(("Interlaced Mode\n\r"));
	} else {
		hdmitx_edidmsg(("Non-interlaced Mode\n\r"));
	}

	/*
	if(DTD[17] & 0x60)
	    hdmitx_edidmsg(("Table 3.17 for defenition\r\n "));
	else
	    hdmitx_edidmsg(("Normal Display\n\r "));
	*/

	if (hdmitx2_ddc_read_byte(ui_addr >> 8, (ui_addr & 0xFF) + 17) & 0x10) {
		//DTD[17]
		hdmitx_edidmsg(("Digital TV\n\r"));
	} else {
		hdmitx_edidmsg(("Analog TV\n\r"));
	}

	temp1 = h_resolution + h_blanking;
	v_total = v_resolution + v_blanking;
	temp1 = temp1 * v_total;
	temp2 = pixel_clk;
	temp2 = temp2 * 10000;

	//hz = Pixel-Clock / (H-total*V-total);
	if (temp1 == 0) {
		hz = 0;
	} else {
		hz = temp2 / temp1;
	}

	//Let 720P can be outputed during DVI connection, we check 1280X720P60/1920x1080I60/1920x1080P60.
	if ((v_resolution >= 720) && (h_resolution >= 1280)) {
		gb_valid_resolution = TRUE;
	}

	if ((hz == 59) || (hz == 60)) {
		#if HDMITX_DEBUG
		hz = 60;
		#endif

		if (v_resolution == 540) {
			gui_hdmi_edid_info |= HDMITX_DTD1080I60;
			hdmitx_edidmsg(("Support 1080I60\n\r"));
		} else if (v_resolution == 1080) {
			gui_hdmi_edid_info |= HDMITX_DTD1080P60;
			hdmitx_edidmsg(("Support 1080P60\n\r"));
		} else if (v_resolution == 720) {
			gui_hdmi_edid_info |= HDMITX_DTD720P60;
			hdmitx_edidmsg(("Support 720P60\n\r"));
		} else if ((h_resolution == 640) && (v_resolution == 480)) {
			gui_hdmi_edid_info |= HDMITX_DTD640X480P60;
			hdmitx_edidmsg(("Support 640x480P60\n\r"));
		} else if ((h_resolution == 720) && (v_resolution == 480)) {
			gui_hdmi_edid_info |= HDMITX_DTD720X480P60;
			hdmitx_edidmsg(("Support 720x480P60\n\r"));
		} else if ((h_resolution == 720) && (v_resolution == 240)) {
			gui_hdmi_edid_info |= HDMITX_DTD720X480I60;
			hdmitx_edidmsg(("Support 720x480I60\n\r"));
		} else if ((v_resolution == 2160) && (pixel_clk >= 29700) && (h_resolution >= 4096)) {
			gui_hdmi_edid_info |= HDMITX_DTD4096X2160P30;
			hdmitx_edidmsg(("Support 4096x2160P30\n\r"));
		} else if ((v_resolution == 2160) && (pixel_clk >= 29700) && (h_resolution >= 3840)) {
			gui_hdmi_edid_info |= HDMITX_DTD3840X2160P30;
			hdmitx_edidmsg(("Support 3840x2160P30\n\r"));
		} else {
			hdmitx_edidmsg(("Non-Supported Format: frame rate=(%d), H-res=(%d), V-res=(%d) \r\n", hz, h_resolution, v_resolution));
		}

	} else if (hz == 50) {
		if (v_resolution == 540) {
			gui_hdmi_edid_info |= HDMITX_DTD1080I50;
			hdmitx_edidmsg(("Support 1080I50\n\r"));
		} else if (v_resolution == 1080) {
			gui_hdmi_edid_info |= HDMITX_DTD1080P50;
			hdmitx_edidmsg(("Support 1080P50\n\r"));
		} else if (v_resolution == 720) {
			gui_hdmi_edid_info |= HDMITX_DTD720P50;
			hdmitx_edidmsg(("Support 720P50\n\r"));
		} else if (v_resolution == 576) {
			gui_hdmi_edid_info |= HDMITX_DTD576P50;
			hdmitx_edidmsg(("Support 576P50\n\r"));
		} else if (v_resolution == 288) {
			gui_hdmi_edid_info |= HDMITX_DTD576I50;
			hdmitx_edidmsg(("Support 576I50\n\r"));
		} else if ((v_resolution == 2160) && (pixel_clk >= 29700) && (h_resolution >= 4096)) {
			gui_hdmi_edid_info |= HDMITX_DTD4096X2160P30;
			hdmitx_edidmsg(("Support 4096x2160P30\n\r"));
		} else if ((v_resolution == 2160) && (pixel_clk >= 29700) && (h_resolution >= 3840)) {
			gui_hdmi_edid_info |= HDMITX_DTD3840X2160P30;
			hdmitx_edidmsg(("Support 3840x2160P30\n\r"));
		} else {
			hdmitx_edidmsg(("Non-Supported Format: frame rate=(%d), H-res=(%d), V-res=(%d) \r\n", hz, h_resolution, v_resolution));
		}
	} else if (hz == 30) {
		if ((v_resolution == 2160) && (pixel_clk >= 29700) && (h_resolution >= 4096)) {
			gui_hdmi_edid_info |= HDMITX_DTD4096X2160P30;
			hdmitx_edidmsg(("Support 4096x2160P30\n\r"));
		} else if ((v_resolution == 2160) && (pixel_clk >= 29700) && (h_resolution >= 3840)) {
			gui_hdmi_edid_info |= HDMITX_DTD3840X2160P30;
			hdmitx_edidmsg(("Support 3840x2160P30\n\r"));
		} else {
			hdmitx_edidmsg(("Non-Supported Format: frame rate=(%d), H-res=(%d), V-res=(%d) \r\n", hz, h_resolution, v_resolution));
		}
	} else {
		DBG_WRN("Non-Supported Frame Rate (%d)hz \r\n", hz);
	}


}

/*
    Parse the DTD information the EDID first Block.
*/
static void hdmitx2_parse_dtd_in_block_one(void)
{

	hdmitx_edidmsg(("\n\rParse the first DTD in Block one:\n\r"));

	// The first DTD is fixed at offset 0x36.
	hdmitx2_parse_dtd(0x36);

	// Check if the second DTD is existed or not.
	hdmitx_edidmsg(("\n\rParse the Second DTD in Block one:\n\r"));
	if ((hdmitx2_ddc_read_byte(0x00, 0x48) == 0) && (hdmitx2_ddc_read_byte(0x00, 0x49) == 0) && (hdmitx2_ddc_read_byte(0x00, 0x4A) == 0)) {
		if (hdmitx2_ddc_read_byte(0x00, 0x4B) == 0xFD) {
			hdmitx_edidmsg(("Paring Monitor Range Limits Packets! -->\n\r"));
			hdmitx2_parse_mrl(0x48);
		} else {
			hdmitx_edidmsg(("The second DTD in Block one is not used to descript video timing.\n\r"));
		}
	} else {
		// Parse the second DTD in the first EDID Block.
		hdmitx2_parse_dtd(0x48);
	}

	// Check if the third DTD is existed or not.
	hdmitx_edidmsg(("\n\rParse the Third DTD in Block one:\n\r"));
	if ((hdmitx2_ddc_read_byte(0x00, 0x5A) == 0) && (hdmitx2_ddc_read_byte(0x00, 0x5B) == 0) && (hdmitx2_ddc_read_byte(0x00, 0x5C) == 0)) {
		if (hdmitx2_ddc_read_byte(0x00, 0x5D) == 0xFD) {
			hdmitx_edidmsg(("Paring Monitor Range Limits Packets! -->\n\r"));
			hdmitx2_parse_mrl(0x5A);
		} else {
			hdmitx_edidmsg(("The third DTD in Block one is not used to descript video timing.\n\r"));
		}
	} else {
		// Parse the third DTD in the first EDID Block.
		hdmitx2_parse_dtd(0x5A);
	}

	hdmitx_edidmsg(("\n\rParse the Fourth DTD in Block one:\n\r"));
	if ((hdmitx2_ddc_read_byte(0x00, 0x6C) == 0) && (hdmitx2_ddc_read_byte(0x00, 0x6D) == 0) && (hdmitx2_ddc_read_byte(0x00, 0x6E) == 0)) {
		if (hdmitx2_ddc_read_byte(0x00, 0x6F) == 0xFD) {
			hdmitx_edidmsg(("Paring Monitor Range Limits Packets! -->\n\r"));
			hdmitx2_parse_mrl(0x6C);
		} else {
			hdmitx_edidmsg(("The fourth DTD in Block one is not used to descript video timing.\n\r"));
		}
	} else {
		hdmitx2_parse_dtd(0x6C);
	}

}


static void hdmitx2_parse_dtd_in_ext_block(UINT32 ofs)
{
	UINT32 dtd_begin_addr;

	dtd_begin_addr = hdmitx2_ddc_read_byte((ofs >> 8), 0x02 + (ofs & 0xFF)) + ofs;

	while (dtd_begin_addr < (0x6c + ofs)) {
		if ((hdmitx2_ddc_read_byte((dtd_begin_addr >> 8), (dtd_begin_addr) & 0xFF) == 0) && (hdmitx2_ddc_read_byte((dtd_begin_addr >> 8), (dtd_begin_addr + 1) & 0xFF) == 0) && (hdmitx2_ddc_read_byte((dtd_begin_addr >> 8), (dtd_begin_addr + 2) & 0xFF) == 0)) {
			hdmitx_edidmsg(("This DTD in Extension Block is not used to descript video timing.(%d)\n\r", dtd_begin_addr));
		} else {
			hdmitx_edidmsg(("\n\rParse the DTD in Extension Block :\n\r"));
			hdmitx2_parse_dtd(dtd_begin_addr);
		}

		dtd_begin_addr = dtd_begin_addr + 18;
	}
}


static void hdmitx2_parse_native_format(UINT32 blk_num)
{
	UINT8   temp;
	UINT32  ofs;

	ofs = 0x80 * (blk_num & 0x01);

	temp = hdmitx2_ddc_read_byte((blk_num >> 1), 0x03 + ofs) & 0xf0;

	if (temp & 0x80) {
		gui_hdmi_edid_info |= HDMITX_TV_UNDERSCAN;
		hdmitx_edidmsg(("DTV supports underscan.\n\r"));
	}
	if (temp & 0x40) {
		hdmitx_edidmsg(("DTV supports BasicAudio.\n\r"));
	}

	if (temp & 0x20) {
#if HDMITX_DEBUG
	#if !HDMITX_DEBUG_FORCE_YUV422
		gui_hdmi_edid_info |= HDMITX_TV_YCBCR444;
	#endif
#else
		gui_hdmi_edid_info |= HDMITX_TV_YCBCR444;
#endif
		hdmitx_edidmsg(("DTV supports YCbCr 4:4:4.\r\n"));
	}
	if (temp & 0x10) {
		gui_hdmi_edid_info |= HDMITX_TV_YCBCR422;
		hdmitx_edidmsg(("DTV supports YCbCr 4:2:2.\r\n"));
	}

}


static UINT32 hdmitx2_parse_ext_block(UINT32 blk_num)
{
	UINT8   c;
	UINT32  ofs;

	ofs = 0x80 * blk_num;

	{
		c = hdmitx2_ddc_read_byte((ofs >> 8), ofs & 0xFF);
		if (c == 0x02) {
			hdmitx2_parse_dtd_in_ext_block(ofs);
			hdmitx2_parse_std(ofs);
		} else {
			DBG_ERR("Extension block tag code error!\r\n");
			return HDMITX_ER_TAGCODE;
		}
	}

	return E_OK;
}

static void hdmitx2_parse_audio_std(UINT32 stdaddr, UINT32 stdreg)
{
	UINT32 i, audio_format, std_reg_tmp, std_addr_tmp;
	UINT8 temp;

	std_reg_tmp  = stdreg & 0x1f;
	std_addr_tmp = stdaddr + 1;
	i           = 0;

	while (i < std_reg_tmp) {
		audio_format = (hdmitx2_ddc_read_byte(std_addr_tmp >> 8, std_addr_tmp & 0xFF) & 0x78) >> 3;

		temp = hdmitx2_ddc_read_byte((std_addr_tmp + 1) >> 8, (std_addr_tmp + 1) & 0xFF);

		if (temp & 0x01) {
			hdmitx_edidmsg(("TV Audio Support 32Khz sampling rate! \r\n"));
			gui_hdmi_edid_info |= HDMITX_ADO_32KHZ;
		}
		if (temp & 0x02) {
			hdmitx_edidmsg(("TV Audio Support 44.1Khz sampling rate! \r\n"));
			gui_hdmi_edid_info |= HDMITX_ADO_44KHZ;
		}
		if (temp & 0x04) {
			hdmitx_edidmsg(("TV Audio Support 48Khz sampling rate! \r\n"));
			gui_hdmi_edid_info |= HDMITX_ADO_48KHZ;
		}
		if (temp & 0x78) {
			hdmitx_edidmsg(("TV Audio Support sampling rate over 48Khz! \r\n"));
			gui_hdmi_edid_info |= HDMITX_ADO_MORE;
		}

		if (audio_format == 1) {

			temp = hdmitx2_ddc_read_byte((std_addr_tmp + 2) >> 8, (std_addr_tmp + 2) & 0xFF);

			if (temp & 0x01) {
				hdmitx_edidmsg(("TV Audio support 16bits \r\n"));
			}
			if (temp & 0x02) {
				hdmitx_edidmsg(("TV Audio support 20bits \r\n"));
			}
			if (temp & 0x04) {
				hdmitx_edidmsg(("TV Audio support 24bits \r\n"));
			}
		} else {
			hdmitx_edidmsg(("Un-Known TV Audio Format Code! \r\n"));
		}

		i = i + 3;
		std_addr_tmp = std_addr_tmp + 3;
	}
}

static void hdmitx2_parse_video_std(UINT32 stdaddr, UINT32 stdreg)
{
	UINT32 i, std_reg_tmp, std_addr_tmp;


	std_reg_tmp  = stdreg & 0x1f;
	std_addr_tmp = stdaddr + 1;
	i           = 0;

	while (i < std_reg_tmp) {
		g_svd_vid[i] = hdmitx2_ddc_read_byte(std_addr_tmp >> 8, std_addr_tmp & 0xFF) & 0x7F;
		hdmitx_edidmsg(("Supported Video ID:  (%d)\r\n", g_svd_vid[i]));
		i = i + 1;
		std_addr_tmp = std_addr_tmp + 1;
	}

	g_svd_vid[HDMITX_EDIDNUM] = std_reg_tmp;
}

static void hdmitx2_parse_vendor_std(UINT32 stdaddr, UINT32 stdreg)
{
	UINT32  std_reg_tmp, std_addr_tmp, HDMIVICLEN_LOC;	
	UINT8   temp, temp2;
	UINT32  ieee_id;
	UINT32  HDMI_VIC_LEN;
#ifdef HDMI_CEC_EN
	UINT32  phy_addr = 0x00;
#endif

	std_reg_tmp  = stdreg & 0x1f;
	std_addr_tmp = stdaddr;


	ieee_id = (UINT32)hdmitx2_ddc_read_byte((std_addr_tmp + 1) >> 8, (std_addr_tmp + 1) & 0xFF) + ((UINT32)hdmitx2_ddc_read_byte((std_addr_tmp + 2) >> 8, (std_addr_tmp + 2) & 0xFF) << 8) + ((UINT32)hdmitx2_ddc_read_byte((std_addr_tmp + 3) >> 8, (std_addr_tmp + 3) & 0xFF) << 16);

	if ((ieee_id == 0x000C03) || (ieee_id == 0xC45DD8)) {
		hdmitx_edidmsg(("TV is HDMI Device!! \r\n"));
		gui_hdmi_edid_info |= HDMITX_TV_HDMI;
	}

	if (ieee_id == 0x000C03) {
#ifdef HDMI_CEC_EN
		phy_addr  =  hdmitx2_ddc_read_byte((std_addr_tmp + 5) >> 8, (std_addr_tmp + 5) & 0xFF);
		phy_addr |= (hdmitx2_ddc_read_byte((std_addr_tmp + 4) >> 8, (std_addr_tmp + 4) & 0xFF) << 8);

		// Set CEC Physical Address from VSDB
		hdmicec_set_config(CEC_CONFIG_ID_PHY_ADDR, phy_addr);
#endif

#if HDMITX_DEBUG
		if (std_reg_tmp >= 6) {
			temp = hdmitx2_ddc_read_byte((std_addr_tmp + 6) >> 8, (std_addr_tmp + 6) & 0xFF);

			if (temp & 0x78) {
				hdmitx_edidmsg(("HDMI Device supports Deep Color!!! flag= 0x%X \r\n", (hdmitx2_ddc_read_byte((std_addr_tmp + 6) >> 8, (std_addr_tmp + 6) & 0xFF) & 0x78)));
			}

			if (temp & 0x80) {
				hdmitx_edidmsg(("HDMI Support_AI bit is SET!!!  \r\n"));
			}
		}
#endif

		if (std_reg_tmp >= 8) {
			UINT32  video_start_ofs, video_start_base;
			UINT8   HDMI_3D_LEN, k;

			video_start_base = (std_addr_tmp + 8);
			temp = hdmitx2_ddc_read_byte(video_start_base >> 8, video_start_base & 0xFF);

			//HDMI_Video_present
			if (temp & 0x20) {
				if (temp & 0x80) {
					if (temp & 0x40) {
						video_start_ofs = 5;
						HDMIVICLEN_LOC = 14;
					} else {
						video_start_ofs = 3;
						HDMIVICLEN_LOC = 12;
					}
				} else {
					video_start_ofs  = 1;
					HDMIVICLEN_LOC = 10;
				}

				temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);
				//3D_present
				if (temp & 0x80) {
					g_svd_3d_cap[HDMITX_EDIDNUM] = 1;
				} else {
					g_svd_3d_cap[HDMITX_EDIDNUM] = 0;
				}

				switch (temp & 0x60) {
				// 3D_Multi_present = 01
				case 0x20: {
				video_start_ofs += 1;
				temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);

				video_start_ofs += ((temp >> 5) + 1);
				HDMI_3D_LEN = (temp & 0x1F); //2
				// 2 is 3D_Struc_All
				if (HDMI_3D_LEN >= 2) {
					HDMI_3D_LEN -= 2;
				}

				temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);
				if (temp & 0x1) {
					for (k = 0; k < 16; k++) {
						g_svd_3d_cap[k] |= HDMI3D_FRAMEPACKING;
					}
				}

				if (temp & 0x40) {
					for (k = 0; k < 16; k++) {
						g_svd_3d_cap[k] |= HDMI3D_TOPANDBOTTOM;
					}
				}

				video_start_ofs += 1;
				temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);
				if (temp & 0x01) {
					for (k = 0; k < 16; k++) {
						g_svd_3d_cap[k] |= HDMI3D_SIDEBYSIDE_HALF;
					}
				}

				video_start_ofs += 1;
				}
				break;

				case 0x40: { // 3D_Multi_present = 10
				UINT16 MASK_3D = 0;

				video_start_ofs += 1;
				temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);


				video_start_ofs += ((temp >> 5) + 1);
				HDMI_3D_LEN = (temp & 0x1F); //2
				// 4 is 3D_Struc_All & 3D_Mask
				if (HDMI_3D_LEN >= 4) {
					HDMI_3D_LEN -= 4;
				}


				MASK_3D = (hdmitx2_ddc_read_byte((video_start_base + video_start_ofs + 2) >> 8, (video_start_base + video_start_ofs + 2) & 0xFF) << 8);
				MASK_3D += hdmitx2_ddc_read_byte((video_start_base + video_start_ofs + 3) >> 8, (video_start_base + video_start_ofs + 3) & 0xFF);


				temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);
				if (temp & 0x1) {
					for (k = 0; k < 16; k++) {
					if (MASK_3D & (0x1 << k)) {
						g_svd_3d_cap[k] |= HDMI3D_FRAMEPACKING;
					}
					}
				}

				if (temp & 0x40) {
					for (k = 0; k < 16; k++) {
					if (MASK_3D & (0x1 << k)) {
						g_svd_3d_cap[k] |= HDMI3D_TOPANDBOTTOM;
					}
					}
				}

				video_start_ofs += 1;
				temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);
				if (temp & 0x01) {
					for (k = 0; k < 16; k++) {
					if (MASK_3D & (0x1 << k)) {
						g_svd_3d_cap[k] |= HDMI3D_SIDEBYSIDE_HALF;
					}
					}
				}
				video_start_ofs += 3;
			}
			break;

			//case 0x00:
			//case 0x60:
			default : {
			video_start_ofs += 1;
			temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);

			video_start_ofs += ((temp >> 5) + 1);
			HDMI_3D_LEN = (temp & 0x1F); //2
			}
			break;

			}

			hdmitx_edidmsg(("HDMI_3D_LEN = %d   video_start_ofs= %d\r\n", HDMI_3D_LEN, video_start_ofs));

			temp2 = video_start_ofs;
			while (video_start_ofs < (HDMI_3D_LEN + temp2)) {
				temp = hdmitx2_ddc_read_byte((video_start_base + video_start_ofs) >> 8, (video_start_base + video_start_ofs) & 0xFF);

				if (((temp & 0xF) == 0x0) || ((temp & 0xF) == 0x6) || ((temp & 0xF) == 0x8)) {
					// Frame Packing
					if ((temp & 0xF) == 0x0) {
						g_svd_3d_cap[(temp >> 4)] |= HDMI3D_FRAMEPACKING;
						video_start_ofs++;
					}

					// Top and Bottom
					if ((temp & 0xF) == 0x6) {
						g_svd_3d_cap[(temp >> 4)] |= HDMI3D_TOPANDBOTTOM;
						video_start_ofs++;
					}

					// Side by Side Half
					if ((temp & 0xF) == 0x8) {
						g_svd_3d_cap[(temp >> 4)] |= HDMI3D_SIDEBYSIDE_HALF;
						video_start_ofs += 2;
					}
				} else {
					hdmitx_edidmsg(("HDMI illegal 3D Parser Type = 0x%02X\r\n", (temp & 0xF)));
					video_start_ofs++;
					//return;
				}
			}

			if (std_reg_tmp >= HDMIVICLEN_LOC) {
				temp = hdmitx2_ddc_read_byte((std_addr_tmp + HDMIVICLEN_LOC) >> 8, (std_addr_tmp + HDMIVICLEN_LOC) & 0xFF);
				HDMI_VIC_LEN = (temp>>5)&0x7;
				hdmitx_edidmsg(("GOT HDMI14B HDMI_VIC LEN = %d\r\n", HDMI_VIC_LEN));
				for (k = 0; k < HDMI_VIC_LEN; k++) {
					g_svd_hdmivic[k] = hdmitx2_ddc_read_byte(((std_addr_tmp + HDMIVICLEN_LOC)+k+1) >> 8, ((std_addr_tmp + HDMIVICLEN_LOC)+k+1) & 0xFF);;
					hdmitx_edidmsg(("HDMI14B HDMI_VIC = %d\r\n", g_svd_hdmivic[k]));
				}
			}

			}


		}
	} else if (ieee_id == 0xC45DD8) {
		gui_vsdb_h20cap  = (UINT32)hdmitx2_ddc_read_byte((std_addr_tmp + 4) >> 8, (std_addr_tmp + 4) & 0xFF);
		gui_vsdb_h20cap += (UINT32)hdmitx2_ddc_read_byte((std_addr_tmp + 5) >> 8, (std_addr_tmp + 5) & 0xFF) << 8;
		gui_vsdb_h20cap += (UINT32)hdmitx2_ddc_read_byte((std_addr_tmp + 6) >> 8, (std_addr_tmp + 6) & 0xFF) << 16;
		gui_vsdb_h20cap += (UINT32)hdmitx2_ddc_read_byte((std_addr_tmp + 7) >> 8, (std_addr_tmp + 7) & 0xFF) << 24;
		hdmitx_edidmsg(("HDMI 2.0 VSDB Cap=0x%08X\r\n", gui_vsdb_h20cap));
	}

}


static void hdmitx2_parse_std(UINT32 ofs)
{
	UINT32  dtd_begin_addr;
	UINT32  stdaddr, stdreg;

	stdaddr = 0x04 + ofs;

	dtd_begin_addr = hdmitx2_ddc_read_byte((ofs >> 8), 0x02 + (ofs & 0xFF)) + ofs;

	while (stdaddr < dtd_begin_addr) {
		stdreg = hdmitx2_ddc_read_byte(stdaddr >> 8, stdaddr & 0xFF);

		switch (stdreg & 0xE0) {
		case 0x20:
			hdmitx2_parse_audio_std(stdaddr, stdreg);
			break;
		case 0x40:
			hdmitx2_parse_video_std(stdaddr, stdreg);
			break;
		//case 0x80:
		//    ANX7150_Parse_SpeakerSTD();
		//   break;
		case 0x60:
			//HDMI VSDB
			hdmitx2_parse_vendor_std(stdaddr, stdreg);
			break;
		default:
			break;
		}

		stdaddr = stdaddr + (stdreg & 0x1f) + 0x01;
	}
}


static void hdmitx2_ddc_init(void)
{
/*new arch config on dtsi*/
#ifndef __KERNEL__
	// Configure the I2C/DDC releated configurations such as bus-clock and NACK handle.
	hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_BUSCLOCK,     HDMITX_HWDDC_SPEED);
	hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_HANDLE_NACK,  TRUE);

	// Bug Fix: For QD980B, use driver default TSR=1 would be nacked. So we use 2.5us here. "(2500/20.833)-4"
	hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_TSR,          116);
#endif
}

static ER hdmitx2_ddc_read(UINT32 addr, UINT32 segm, UINT32 offset, UINT32 cnt)
{
#if HDMI_UBOOT_I2C
	uchar t_val,*t_val2;
	uint t_addr, d_addr;
	int ret,len;
	struct udevice *dev;

	if(hdmitx2_check_hotplug() != 1){
		return -1;
	}	
		
	if (segm != 0) {
		d_addr	= 0x60>>1;
		t_addr	= segm;
		len    = 0;
		
		//printf("[debug] your dev address is 0x%x, register address is 0x%x\n",d_addr, t_addr);
	
		ret = i2c_get_chip_for_busnum(1, d_addr, 1, &dev);
		if (ret < 0) {
			printf("[debug] hdmitx2_ddc_read no device get\n");
		 	return -1;
		}			
		ret = dm_i2c_write(dev, t_addr, &t_val, len);
	}
	
	
	d_addr	= HDMI_DDCSLAVE_ADDR>>1;
	t_addr	= offset;
	len    = cnt;
	
	//printf("[debug] your dev address is 0x%x, register address is 0x%x\n",d_addr, t_addr);
		
	ret = i2c_get_chip_for_busnum(1, d_addr, 1, &dev);
	if(ret < 0) {
		printf("[debug] hdmitx2_ddc_read no device get\n");
	 	return -1;
	}
	
	ret = dm_i2c_read(dev, t_addr, t_val2, len);

	if(ret){
		hdmi_edid_error = 1;
		printf("hdmi_edid_error\r\n");
	}
	
	if(hdmi_edid_error){
		return -1;
	} else {
		return E_OK;
	}
#endif
}

//#include <plat/i2c_reg.h>
#define HDMI_RST_I2C ENABLE

static ER hdmitx2_ddc_read_byte2(UINT32 segm, UINT32 offset, UINT32 *pui_data)
{
#if HDMI_UBOOT_I2C
	uchar t_val;
	uint t_addr, d_addr;
	int ret,len;
	struct udevice *dev;

	if(hdmitx2_check_hotplug() != 1){
		return -1;
	}
	
	if (segm != 0) {
		d_addr	= 0x60>>1;
		t_addr	= segm;
		len    = 0;
		
		//printf("[debug] your dev address is 0x%x, register address is 0x%x\n",d_addr, t_addr);
	
		ret = i2c_get_chip_for_busnum(1, d_addr, 1, &dev);
		if (ret < 0) {
			printf("[debug] hdmitx2_ddc_read_byte2 no device get\n");
		 	return -1;
		}		
		ret = dm_i2c_write(dev, t_addr, &t_val, len);
	}
	
	
	d_addr	= HDMI_DDCSLAVE_ADDR>>1;
	t_addr	= offset;
	len    = 1;
	
	//printf("[debug] your dev address is 0x%x, register address is 0x%x\n",d_addr, t_addr);
	
	ret = i2c_get_chip_for_busnum(1, d_addr, 1, &dev);

	if (ret < 0) {
		printf("[debug] hdmitx2_ddc_read_byte2 no device get\n");
	 	return -1;
	}
	ret = dm_i2c_read(dev, t_addr, &t_val, len);

	if(ret < 0 ){
		printf("[debug] hdmitx2_ddc_read_byte2 read fail\n");
	}
	
	
	*pui_data = (UINT32)t_val;
#endif
	return E_OK;
}


static UINT8 hdmitx2_ddc_read_byte(UINT8 segm, UINT8 offset)
{
#if HDMI_UBOOT_I2C
	uchar t_val;
	uint t_addr, d_addr;
	int ret,len;
	struct udevice *dev;
	
	if(hdmitx2_check_hotplug() != 1){
		return -1;
	}

	
	if (segm != 0) {
		d_addr	= 0x60>>1;
		t_addr	= segm;
		len    = 0;
		
		//printf("[debug] your dev address is 0x%x, register address is 0x%x\n",d_addr, t_addr);
	
		ret = i2c_get_chip_for_busnum(1, d_addr, 1, &dev);
		if (ret < 0) {
			 printf("[debug] hdmitx2_ddc_read_byte no device get\n");
		 return -1;
		}			
		ret = dm_i2c_write(dev, t_addr, &t_val, len);
	}
	
	
	d_addr	= HDMI_DDCSLAVE_ADDR>>1;
	t_addr	= offset;
	len    = 1;

	//printf("[debug] your dev address is 0x%x, register address is 0x%x\n",d_addr, t_addr);
		
	ret = i2c_get_chip_for_busnum(1, d_addr, 1, &dev);
	if (ret < 0) {
		printf("[debug] hdmitx2_ddc_read_byte no device get\n");
	 	return -1;
	}
	
	ret = dm_i2c_read(dev, t_addr, &t_val, len);
	
	if (ret < 0) {
		printf("[debug] hdmitx2_ddc_read_byte error\n");
	 	return -1;
	}	
	
	return t_val;
#endif
}


static ER hdmitx2_parse_edid(UINT32 ext_blk)
{
	UINT32 i;

	//
	// Parsing Block 1
	//
	// Phase-1: Parsing EDID Header
	if (hdmitx2_parse_edid_header()) {
		DBG_ERR("BAD EDID Header, Stop parsing \n\r");
		return HDMITX_ER_BADHEADER;
	}

	// Phase-2: Parsing EDID Version
	if (hdmitx2_parse_edid_version()) {
		gui_hdmi_edid_info |= HDMITX_TV_UNDERSCAN; // go to DVI path
		DBG_ERR("EDID does not support 861B, Stop parsing\n\r");
		return HDMITX_ER_NOSUPPORT_VERSION;
	}

	// Phase-3: Verify EDID CheckSum <Block 1>
	if (hdmitx2_verify_edid_checksum(0)) {
		DBG_ERR("EDID Block-1 check sum error, Stop parsing\n");
		//return HDMITX_ER_CRC;
	}

	//Phase-4: Parsing DTD
	hdmitx2_parse_dtd_in_block_one();

	// Check if Extension block exist
	if (hdmitx2_ddc_read_byte(0x00, 0x7E) == 0) {
		DBG_ERR("No EDID extension blocks.\n");
		ext_blk = 0;
	}

	//
	// Parsing Extention Blocks
	//
	for (i = 1; i <= ext_blk; i++) {
		hdmitx_edidmsg(("===========================\r\n"));
		hdmitx_edidmsg(("=Start Parsing Ext-Block %d=\r\n", i));
		hdmitx_edidmsg(("===========================\r\n"));


		// Verify EDID CheckSum
		if (hdmitx2_verify_edid_checksum(i)) {
			DBG_ERR("EDID Ext-Block-%d check sum error, Skip this block.\n\r", i);
			//continue;
		}

		// Parsing Data Collection Block
		if (hdmitx2_parse_ext_block(i) != E_OK) {
			continue;
		}

		// Parsing Native format
		hdmitx2_parse_native_format(i);

	}

	gui_native_edid_info = gui_hdmi_edid_info;
	DBG_IND("HDMI EDID Parsing Results Flag: 0X%08X\r\n", gui_hdmi_edid_info);


	if ((gui_hdmi_edid_info & HDMITX_TV_HDMI) == 0x00) {
		// DVI device can support RGB only.
		gui_hdmi_edid_info &= ~(HDMITX_TV_YCBCR444 | HDMITX_TV_YCBCR422);
		// Force Underscan at DVI mode.
		gui_hdmi_edid_info |= HDMITX_TV_UNDERSCAN;
	}

	return E_OK;
}

#endif


ER hdmitx2_scdc_write(UINT32 offset, UINT32 data)
{
#if HDMI_UBOOT_I2C
	uchar t_val;
	uint t_addr, d_addr;
	int ret,len;
	struct udevice *dev;

	if(hdmitx2_check_hotplug() != 1){
		return -1;
	}


	d_addr = HDMI2_SCDC_SLAVEADDR>>1;
	t_addr = offset;
	t_val  = (uchar)data;
	len	   = 1;

	//printf("[debug] your dev address is 0x%x, register address is 0x%x\n",d_addr, t_addr);

	ret = i2c_get_chip_for_busnum(1, d_addr, 1, &dev);
	if (ret < 0) {
		 printf("[debug] hdmitx2_scdc_write no device get\n");
	 return -1;
	}
	
	ret = dm_i2c_write(dev, t_addr, &t_val, len);

	if(ret){
		printf("[debug] dm_i2c_write error\n");
		 return -1;
	}
	
#endif
	return E_OK;
}


ER hdmitx2_scdc_read(UINT32 offset, UINT32 *pdata)
{
#if HDMI_UBOOT_I2C
	uchar t_val;
	uint t_addr, d_addr;
	int ret,len;
	struct udevice *dev;

	if(hdmitx2_check_hotplug() != 1){
		return -1;
	}


	d_addr = HDMI2_SCDC_SLAVEADDR>>1;
	t_addr = offset;
	len	   = 1;

	//printf("[debug] your dev address is 0x%x, register address is 0x%x\n",d_addr, t_addr);

	ret = i2c_get_chip_for_busnum(1, d_addr, 1, &dev);
	if (ret < 0) {
		 printf("[debug] hdmitx2_scdc_read no device get\n");
	 	return -1;
	}
	
	ret = dm_i2c_read(dev, t_addr, &t_val, len);

	if(ret){
		 printf("[debug] hdmitx2_scdc_read error\n");
		  return -1;
	}

	*pdata = (UINT32)t_val;
#endif
	return E_OK;
}

/*
    Initialize DDC to get EDID information

    Initialize DDC to get EDID information. The input parameter should specify the DDC used I2C channel is available or not.
    If the I2C channel is not available, the DDC read and EDID parser process would be skipped and output the warning message.

    @param[in] b_ddc_en   DDC used I2C channel is available or not.

    @return void
*/
void hdmitx2_init_ddc(BOOL b_ddc_en)
{
	UINT32 i, ext_blk = 0;
	int temp = 0;

	b_hdmi_edid_msg |= print_edid;

	gui_hdmi_edid_info     = 0x0;
	gui_native_edid_info   = 0x0;
	hdmi2_manufacturer_id   = 0x0;
	gb_valid_resolution   = FALSE;
	hdmi_edid_error       = 0;

	memset(g_svd_vid,    0x00, (HDMITX_EDIDNUM + 1));
	memset(g_svd_hdmivic, 0x00, 8);
	memset(g_svd_3d_cap, 0x00, (HDMITX_EDIDNUM + 1));
	gui_vsdb_h20cap = 0;
	
	if (b_ddc_en && (!skip_edid)) {

		/* sony monitor needs a little delay for i2c ready */		
		if (hdmi_ddc_init_delay != 0) {			
			DELAY_MS(hdmi_ddc_init_delay);		
		}
		
		hdmitx2_ddc_init();

#if HDMITX_SCDC_RRTEST
		hdmitx2_scdc_write(0x30, 0x00);
#endif

		temp = hdmitx2_ddc_read_byte2(0x00, 0x7E, &ext_blk); 
		if (temp != E_OK) {
			DBG_ERR("DDC Abort!0 %d\r\n", temp);
			return;
		}


		if (hdmi_cts_test == 0) {
			if (ext_blk > 2) {
				DBG_ERR("org ExtBlk:%d, set to 1 \n", ext_blk);
				ext_blk = 1;
				//vos_util_delay_ms(100); /* delay for a while to have the sink ready. */
			}
		}


		
		if (ext_blk == 0x0) {
			// DVI Device
			if (hdmitx2_ddc_read(HDMI_DDCSLAVE_ADDR, 0x00, 0x00, HDMITX_EDID_BLKSIZE) != E_OK) {
				DBG_ERR("DDC Abort!1\r\n");
				return;
			}
			hdmitx_edidmsg(("==ext_blk== %d %d\r\n", ext_blk, hdmi_ext_blk));
			ext_blk = hdmi_ext_blk;
		} else {
			// HDMI Device
			// Read EDID first 2 blocks
			if (hdmitx2_ddc_read(HDMI_DDCSLAVE_ADDR, 0x00, 0x00, HDMITX_EDID_BLKSIZE * 2) != E_OK) {
				DBG_ERR("DDC Abort!2\r\n");
				return;
			}
		}


		if (ext_blk > 0x1) {
			UINT32 ofs;

			ofs = (ext_blk - 1) >> 1;
			for (i = 0; i < ofs; i++) {
				if (hdmitx2_ddc_read(HDMI_DDCSLAVE_ADDR, i + 1, 0x00, HDMITX_EDID_BLKSIZE * 2) != E_OK) {
					DBG_ERR("DDC Abort!3\r\n");
					return;
				}

			}
			// Last One Block
			if ((ext_blk - 1) & 0x1) {
				if (hdmitx2_ddc_read(HDMI_DDCSLAVE_ADDR, ofs + 1, 0x00, HDMITX_EDID_BLKSIZE) != E_OK) {
					DBG_ERR("DDC Abort!4\r\n");
					return;
				}
			}
		}
		
		hdmitx_edidmsg(("==== ext_blk total num:[%d] ====\r\n", ext_blk));
		// Parsing all the Ext-EDID
		hdmitx2_parse_edid(ext_blk);


#if HDMITX_DEBUG
#if HDMITX_DEBUG_FORCE_RGB
		gui_hdmi_edid_info &= ~HDMITX_TV_YCBCR444;
		gui_hdmi_edid_info &= ~HDMITX_TV_YCBCR422;
#endif

#if HDMITX_DEBUG_FORCE_YUV
		gui_hdmi_edid_info |= (HDMITX_TV_YCBCR444 | HDMITX_TV_YCBCR422);
		gui_hdmi_edid_info |= HDMITX_TV_HDMI;
#endif

#if HDMITX_DEBUG_FORCE_OVSCAN
		gui_hdmi_edid_info &= ~HDMITX_TV_UNDERSCAN;
		gui_hdmi_edid_info |= HDMITX_ADO_32KHZ;//Prevent Edid= 0x0
#endif
#endif

	} else {
		DBG_WRN("Skip InitDDC\r\n");
	}

}


/*
    Get HDMI 2.0 VSDB Capability information

    Get HDMI 2.0 VSDB Capability information

    @return The parsing results bit-wise Flags.
*/
UINT32 hdmitx2_get_hdmi_2_vsdb_cap(void)
{
	return gui_vsdb_h20cap;
}

#if 1

/**
    Dump HDMI Information

    Dump HDMI debug information. The information includes EDID native and result flags
    and also the video format supported by the TV/Display from EDID descriptions.

    @return void
*/
void hdmitx2_dump_info(void)
{
	UINT32 i;

	if (hdmitx2_is_opened() == FALSE) {
		return;
	}

	//
	//Dump Information
	//
	DBG_DUMP("\r\n");
	DBG_DUMP("HDMI Configuration EDID Flag: 0X%08X\r\n", gui_hdmi_edid_info);
	DBG_DUMP("HDMI Native EDID Parsing Results Flag: 0X%08X\r\n", gui_native_edid_info);
	DBG_DUMP("\r\n");

	DBG_DUMP("HDMI Supported Short Video Descriptor Format Code\r\n");
	for (i = 0; i < g_svd_vid[HDMITX_EDIDNUM]; i++) {
		if (i <= HDMITX_EDIDNUM) {
			DBG_DUMP("%d  ", g_svd_vid[i]);
		}

		if ((i == 16) || (i == 32) || (i == 48)) {
			DBG_DUMP("\r\n");
		}
	}
}


/**
    Get HDMI EDID information

    Get HDMI EDID Parsing results flags.
    This information is used in the HDMI display device object to control the hdmi functionality.

    @return The parsing results bit-wise Flags.
*/
UINT32 hdmitx2_get_edid_info(void)
{
	if(hdmi_edid_error)
		return 0;

	return gui_hdmi_edid_info;
}


/**
    Verify HDMI Video ID Validation

    This api would verify the user desired video format is supported by TV/Display or not.
    The supported list is from the TV/Display's EDID's Short Timing Descriptor(STD).
    If the desired VID is not supported, the basic format would be outputed.(720x480P60 / 640x480P60)

    @param[in,out] vid_code    Video-ID defined in CEA-861D, please refer to hdmitx.h for details.

    @return void
*/
void hdmitx2_verify_video_id(HDMI_VIDEOID *vid_code)
{
	UINT32 valid, i;

	valid = 0;

	for (i = 0; i < g_svd_vid[HDMITX_EDIDNUM]; i++) {
		if (g_svd_vid[i] == *vid_code) {
			valid = 1;
		}
	}

	if (valid == 0) {
		if ((*vid_code == HDMI_1280X720P60) || (*vid_code == HDMI_1280X720P50)) {
			if (gui_hdmi_edid_info & (HDMITX_DTD720P60 | HDMITX_DTD720P50)) {
				valid = 1;
			}
		} else if ((*vid_code == HDMI_1920X1080I60) || (*vid_code == HDMI_1920X1080I50)) {
			if (gui_hdmi_edid_info & (HDMITX_DTD1080I60 | HDMITX_DTD1080I50)) {
				valid = 1;
			}
		} else if ((*vid_code == HDMI_1920X1080P60) || (*vid_code == HDMI_1920X1080P50)) {
			if (gui_hdmi_edid_info & (HDMITX_DTD1080P60 | HDMITX_DTD1080P50)) {
				valid = 1;
			}
		}
	}

	if (valid == 0) {
		// Basic Format
		for (i = 0; i < HDMITX_EDIDNUM; i++) {
			if ((valid == 0) && ((g_svd_vid[i] == HDMI_720X480P60) || (g_svd_vid[i] == HDMI_720X480P60_16X9) || (g_svd_vid[i] == HDMI_640X480P60))) {
				valid = 1;
				if (g_svd_vid[i] == HDMI_720X480P60) {
					*vid_code = HDMI_720X480P60;
				} else if (g_svd_vid[i] == HDMI_720X480P60_16X9) {
					*vid_code = HDMI_720X480P60_16X9;
				} else if (g_svd_vid[i] == HDMI_640X480P60) {
					*vid_code = HDMI_640X480P60;
				}
			}
		}
		if (valid == 0) {
			*vid_code = HDMI_640X480P60;
		}

		DBG_ERR("\r\nDesired Video Format Sink not support!! Output HDMI Basic format! (%d)\r\n\n", *vid_code);
	}

}

/**
    Get the HDMI Video Format Support Ability

    This api is used to get the HDMI Video Format Support Ability inlcudes the 2D/3D format supporting capability.
    The user provided "vdo_ability" buffer should contains at least 30 entries in safety.

    @param[out] p_len            Number of video formats supported by the TV/Display.
    @param[out] vdo_ability     The structure of the supported video format and its 3D ability.

    @return void
*/
void hdmitx2_get_video_ability(UINT32 *p_len, PHDMI_VDOABI vdo_ability)
{
	UINT32 i, k = 0;
	UINT32 size;

#if !(HDMITX_DEBUG && HDMITX_DEBUG_SKIP_EDID)
	if ((b_hdmi2_i2c_opened == FALSE) && (hdmitx2_check_hotplug())) {
		if (hdmi2_i2c_obj[hdmi2_i2c_select].open(&hdmi2_i2c_ses) == E_OK) {
			b_hdmi2_i2c_opened = TRUE;

			hdmitx2_init_ddc(b_hdmi2_i2c_opened);

			if (hdmi2_i2c_obj[hdmi2_i2c_select].close(hdmi2_i2c_ses) == E_OK) {
				b_hdmi2_i2c_opened = FALSE;
			}
		} else {
			b_hdmi2_i2c_opened = FALSE;
		}
	}
#endif

	if(hdmi_edid_error) {
		*p_len = 0;
		return;
	}

	size = *p_len;
	*p_len = (UINT32) g_svd_vid[HDMITX_EDIDNUM];

	for (i = 0; i < 8; i++) {
		if (g_svd_hdmivic[i] != 0)
			(*p_len)++;
	}
	if (*p_len > size)
		*p_len = size;
	if (*p_len > 60)
		*p_len = 60;

	hdmitx_edidmsg(("hdmitx2_get_video_ability: Length=%d  :\r\n", *p_len));
	for (i = 0; i < *p_len; i++) {
		vdo_ability[i].video_id      = g_svd_vid[i];
		vdo_ability[i].vdo_3d_ability = g_svd_3d_cap[i];

		//3D_present
		if (g_svd_3d_cap[HDMITX_EDIDNUM]) {
			if ((vdo_ability[i].video_id == HDMI_1920X1080P24) || (vdo_ability[i].video_id == HDMI_1280X720P60) || (vdo_ability[i].video_id == HDMI_1280X720P50)) {
				vdo_ability[i].vdo_3d_ability |= (HDMI3D_FRAMEPACKING | HDMI3D_TOPANDBOTTOM);
			}

			if ((vdo_ability[i].video_id == HDMI_1920X1080I60) || (vdo_ability[i].video_id == HDMI_1920X1080I50)) {
				vdo_ability[i].vdo_3d_ability |= HDMI3D_SIDEBYSIDE_HALF;
			}
		}

		if ((vdo_ability[i].video_id == 0) && (k < 8) && (g_svd_hdmivic[k] == 1)) {
			vdo_ability[i].video_id = HDMI14B_3840X2160P30;
			k++;
		}
		if ((vdo_ability[i].video_id == 0) && (k < 8) && (g_svd_hdmivic[k] == 2)) {
			vdo_ability[i].video_id = HDMI14B_3840X2160P25;
			k++;
		}
		if ((vdo_ability[i].video_id == 0) && (k < 8) && (g_svd_hdmivic[k] == 3)) {
			vdo_ability[i].video_id = HDMI14B_3840X2160P24;
			k++;
		}
		if ((vdo_ability[i].video_id == 0) && (k < 8) && (g_svd_hdmivic[k] == 4)) {
			vdo_ability[i].video_id = HDMI14B_4096X2160P24;
			k++;
		}

		hdmitx_edidmsg(("  VID=(%d)   3D_Cap=0x%X \r\n", vdo_ability[i].video_id, vdo_ability[i].vdo_3d_ability));
	}

	if ((*p_len == 0) && (!(gui_hdmi_edid_info & HDMITX_TV_HDMI)) &&
		(gui_hdmi_edid_info & 0x3000373F)) {

		i = 0;
		if (gui_hdmi_edid_info & HDMITX_DTD4096X2160P30) {
			vdo_ability[i++].video_id = HDMI_4096X2160P30;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD3840X2160P30) {
			vdo_ability[i++].video_id = HDMI_3840X2160P30;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD1080P60) {
			vdo_ability[i++].video_id = HDMI_1920X1080P60;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD1080P50) {
			vdo_ability[i++].video_id = HDMI_1920X1080P50;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD1080I60) {
			vdo_ability[i++].video_id = HDMI_1920X1080I60;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD720P60) {
			vdo_ability[i++].video_id = HDMI_1280X720P60;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD720P50) {
			vdo_ability[i++].video_id = HDMI_1280X720P50;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD1080I50) {
			vdo_ability[i++].video_id = HDMI_1920X1080I50;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD720X480P60) {
			vdo_ability[i++].video_id = HDMI_720X480P60;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD576P50) {
			vdo_ability[i++].video_id = HDMI_720X576P50;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD640X480P60) {
			vdo_ability[i++].video_id = HDMI_640X480P60;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD720X480I60) {
			vdo_ability[i++].video_id = HDMI_720X480I60;
			(*p_len)++;
		}
		if (gui_hdmi_edid_info & HDMITX_DTD576I50) {
			vdo_ability[i++].video_id = HDMI_720X576I50;
			(*p_len)++;
		}
	}


}

void hdmitx2_get_edid_raw_data(UINT32 *len, UINT8 *data)
{
	int i;

	if(data == NULL){
		DBG_ERR("EDID parsing Buffer NULL Error!\r\n");
		return;
	}
	
	if(*len > HDMITX_EDID_BLKSIZE*(hdmi_ext_blk+1)){
		*len = HDMITX_EDID_BLKSIZE*(hdmi_ext_blk+1);
	}

	for(i=0; i<(int)*len; i++){
		*(data+i) = hdmitx2_ddc_read_byte(0x00,0x00+i);
	}
}

/**
    Check if the HDMI RX's DDC Channel is available or not.

    This api is used to check if the HDMI Rx is available or not.

    @return
     - @b TRUE:  The HDMI RX's DDC Channel is available.
     - @b FALSE: The HDMI RX's DDC Channel is NOT available.
*/
BOOL hdmitx2_check_ddc(void)
{
	UINT32 ext_blk;

	if (b_hdmi2_i2c_opened == FALSE) {
		if (hdmi2_i2c_obj[hdmi2_i2c_select].open(&hdmi2_i2c_ses) != E_OK) {
			DBG_ERR("I2C Open Error!\r\n");
			return FALSE;
		}

		b_hdmi2_i2c_opened = TRUE;
	}

	hdmitx2_ddc_init();

	if (hdmitx2_ddc_read_byte2(0x00, 0x7E, &ext_blk) != E_OK) {
		DBG_ERR("DDC NACK\r\n");
		return FALSE;
	}

	return TRUE;
}

#endif

#if 0//def __KERNEL__
EXPORT_SYMBOL(hdmitx2_get_edid_info);
EXPORT_SYMBOL(hdmitx2_verify_video_id);
EXPORT_SYMBOL(hdmitx2_dump_info);
EXPORT_SYMBOL(hdmitx2_get_video_ability);
EXPORT_SYMBOL(hdmitx2_get_edid_raw_data);
EXPORT_SYMBOL(hdmitx2_check_ddc);
#endif

