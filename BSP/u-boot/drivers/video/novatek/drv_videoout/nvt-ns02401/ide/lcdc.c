/*
    Library for ide Video1/2 regiseter control

    This is low level control library for ide Video1/2.

    @file       ide2_video.c
    @ingroup    mIDrvDisp_IDE
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/
#include "./include/ide_reg.h"
#include "./include/ide2_int.h"
#include "../include/ide_protected.h"

#ifndef CHKPNT
#define CHKPNT    printf("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif
	
#ifndef DBGD
#define DBGD(x)   printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif
	
#ifndef DBGH
#define DBGH(x)   printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif
	
#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
#endif
	
#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printf("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif
	
#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  printf("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif
	
#if 0
#define DBG_IND(fmtstr, args...) printf("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif

/**
    @name ide Video Control
*/
//@{

/**
    Enable/Disable Video1.

    Enable/Disable Video1.

    @param[in] id   ide ID
    @param[in] b_en     enable/disable for the specific window
      - @b TRUE:    Enable
      - @b FALSE:   Disable.

    @return void
*/
void idec_set_ch_yaddr(IDE_ID id, CH_ID chid, IDEDATA_TYPE ui_y_addr)
{
	T_LCD_CH0_SRC_PARM00_REG reg;
	T_LCD_CH0_SRC_HADDR_REG reg2;
	UINT32 yoffset, h_yofs;


	if(chid == LCD_CH_0){
		yoffset = LCD_CH0_SRC_PARM00_REG_OFS;
		h_yofs = LCD_CH0_SRC_HADDR_REG_OFS;
	}else if(chid == LCD_CH_1){
		yoffset = LCD_CH1_SRC_PARM10_REG_OFS;
		h_yofs = LCD_CH1_SRC_HADDR_REG_OFS;
	}else if(chid == LCD_CH_2){
		yoffset = LCD_CH2_SRC_PARM20_REG_OFS;
		h_yofs = LCD_CH2_SRC_HADDR_REG_OFS;
	}else{
		return;
	}
	reg.reg = idec_get_reg(id, yoffset);
	reg.bit.y_ba = (UINT32)ide_platform_va2pa(id, ui_y_addr);
	idec_set_reg(id, yoffset, reg.reg);
	reg2.reg = idec_get_reg(id, h_yofs);
	reg2.bit.y_ba_high = (UINT32)(ide_platform_va2pa(id, ui_y_addr)>>32);
	idec_set_reg(id, h_yofs, reg2.reg);

}

void idec_set_ch_uvaddr(IDE_ID id, CH_ID chid, IDEDATA_TYPE ui_cb_addr)
{
	T_LCD_CH0_SRC_PARM01_REG reg;
	T_LCD_CH0_SRC_HADDR1_REG reg2;
	UINT32 uvoffset,h_uvofs;


	if(chid == LCD_CH_0){
		uvoffset = LCD_CH0_SRC_PARM01_REG_OFS;
		h_uvofs = LCD_CH0_SRC_HADDR1_REG_OFS;
	}else if(chid == LCD_CH_1){
		uvoffset = LCD_CH1_SRC_PARM11_REG_OFS;
		h_uvofs = LCD_CH1_SRC_HADDR1_REG_OFS;
	}else if(chid == LCD_CH_2){
		uvoffset = LCD_CH2_SRC_PARM21_REG_OFS;
		h_uvofs = LCD_CH2_SRC_HADDR1_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, uvoffset);
	reg.bit.cbr_ba = (UINT32)ide_platform_va2pa(id, ui_cb_addr);
	idec_set_reg(id, uvoffset, reg.reg);
	reg2.reg = idec_get_reg(id, h_uvofs);
	reg2.bit.cbr_ba_high = (UINT32)(ide_platform_va2pa(id, ui_cb_addr)>>32);
	idec_set_reg(id, h_uvofs, reg2.reg);

}


/**
    Enable/Disable Video2.

    Enable/Disable Video2.

    @param[in] id   ide ID
    @param[in] b_en enable/disable for the specific window.
      - @b TRUE:    Enable
      - @b FALSE:   Disable.

    @return void
*/
void idec_set_ch_dim(IDE_ID id, CH_ID chid, UINT32 ui_bw, UINT32 ui_bh, UINT32 ui_y_lofs, UINT32 ui_uv_lofs)
{
	T_LCD_CH0_SRC_PARM02_REG reg_dim;
	T_LCD_CH0_SRC_PARM05_REG reg_lofs;
	T_LCD_CH0_SRC_PARM06_REG reg;
	UINT32 offset;
	UINT32 y_lofs_ofs, uv_lofs_ofs;


	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM02_REG_OFS;
		y_lofs_ofs = LCD_CH0_SRC_PARM05_REG_OFS;
		uv_lofs_ofs = LCD_CH0_SRC_PARM06_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM12_REG_OFS;
		y_lofs_ofs = LCD_CH1_SRC_PARM15_REG_OFS;
		uv_lofs_ofs = LCD_CH1_SRC_PARM16_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM22_REG_OFS;
		y_lofs_ofs = LCD_CH2_SRC_PARM25_REG_OFS;
		uv_lofs_ofs = LCD_CH2_SRC_PARM26_REG_OFS;
	}else{
		return;
	}

	reg_dim.reg = idec_get_reg(id, offset);
	reg_dim.bit.hs = ui_bw;
	reg_dim.bit.vs = ui_bh;
	idec_set_reg(id, offset, reg_dim.reg);

	reg_lofs.reg = idec_get_reg(id, y_lofs_ofs);
	reg_lofs.bit.y_line_offset = ui_y_lofs;
	idec_set_reg(id, y_lofs_ofs, reg_lofs.reg);

	reg.reg = idec_get_reg(id, uv_lofs_ofs);
	reg.bit.cbr_line_offset = ui_uv_lofs;
	idec_set_reg(id, uv_lofs_ofs, reg.reg);

}

/**
    Video1 window status.

    Video1 window status.

    @param[in] id   ide ID
    @return
      - @b TRUE:    Enable
      - @b FALSE:   Disable.
*/
void idec_set_ch_xy(IDE_ID id, CH_ID chid, UINT32 ui_y_xy, UINT32 ui_uv_xy)
{
	T_LCD_CH0_SRC_PARM03_REG reg_y_xy;
	T_LCD_CH0_SRC_PARM04_REG reg_uv_xy;
	UINT32 y_xy_ofs, uv_xy_ofs;

	if(chid == LCD_CH_0){
		y_xy_ofs = LCD_CH0_SRC_PARM03_REG_OFS;
		uv_xy_ofs = LCD_CH0_SRC_PARM04_REG_OFS;
	}else if(chid == LCD_CH_1){
		y_xy_ofs = LCD_CH1_SRC_PARM13_REG_OFS;
		uv_xy_ofs = LCD_CH1_SRC_PARM14_REG_OFS;
	}else if(chid == LCD_CH_2){
		y_xy_ofs = LCD_CH2_SRC_PARM23_REG_OFS;
		uv_xy_ofs = LCD_CH2_SRC_PARM24_REG_OFS;
	}else{
		return;
	}

	reg_y_xy.reg = idec_get_reg(id, y_xy_ofs);
	reg_y_xy.bit.y_xy = ui_y_xy;
	idec_set_reg(id, y_xy_ofs, reg_y_xy.reg);

	reg_uv_xy.reg = idec_get_reg(id, uv_xy_ofs);
	reg_uv_xy.bit.cbr_xy = ui_uv_xy;
	idec_set_reg(id, uv_xy_ofs, reg_uv_xy.reg);

}


/**
    Video2 window status.

    Video2 window status.

    @param[in] id   ide ID
    @return
      - @b TRUE:    Enable
      - @b FALSE:   Disable.
*/
void idec_set_ch_alpha(IDE_ID id, CH_ID chid, UINT8 ui_galpha, UINT8 ui_alpha0, UINT8 ui_alpha1)
{
	T_LCD_CH0_SRC_PARM08_REG reg_alpha;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM08_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM18_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM28_REG_OFS;
	}else{
		return;
	}

	reg_alpha.reg = idec_get_reg(id, offset);
	reg_alpha.bit.galpha = ui_galpha;
	reg_alpha.bit.alpha0 = ui_alpha0;
	reg_alpha.bit.alpha1 = ui_alpha1;
	idec_set_reg(id, offset, reg_alpha.reg);

}

void idec_set_ch_en(IDE_ID id, CH_ID chid, BOOL b_en)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	T_LCD_CH0_SRC_PARM02_REG reg_dim;
	T_LCD_CH0_SRC_PARM05_REG reg_lofs;
	T_LCD_CH0_SRC_PARM06_REG reg_uv_lofs;
	UINT32 offset;
	UINT32 y_lofs_ofs, uv_lofs_ofs, dim_ofs;

	if(chid == LCD_CH_0){
		offset 		= LCD_CH0_SRC_PARM09_REG_OFS;
		dim_ofs 	= LCD_CH0_SRC_PARM02_REG_OFS;
		y_lofs_ofs 	= LCD_CH0_SRC_PARM05_REG_OFS;
		uv_lofs_ofs = LCD_CH0_SRC_PARM06_REG_OFS;		
	}else if(chid == LCD_CH_1){
		offset 		= LCD_CH1_SRC_PARM19_REG_OFS;
		dim_ofs 	= LCD_CH1_SRC_PARM12_REG_OFS;
		y_lofs_ofs 	= LCD_CH1_SRC_PARM15_REG_OFS;
		uv_lofs_ofs = LCD_CH1_SRC_PARM16_REG_OFS;		
	}else if(chid == LCD_CH_2){
		offset 		= LCD_CH2_SRC_PARM29_REG_OFS;
		dim_ofs 	= LCD_CH2_SRC_PARM22_REG_OFS;
		y_lofs_ofs 	= LCD_CH2_SRC_PARM25_REG_OFS;
		uv_lofs_ofs = LCD_CH2_SRC_PARM26_REG_OFS;		
	}else{
		return;
	}

	reg_dim.reg = idec_get_reg(id, dim_ofs);
	reg_lofs.reg = idec_get_reg(id, y_lofs_ofs);
	reg_uv_lofs.reg = idec_get_reg(id, uv_lofs_ofs);

	/*chk ide setting before enable ch*/
	if(!(reg_dim.bit.hs && reg_dim.bit.vs && reg_lofs.bit.y_line_offset && reg_uv_lofs.bit.cbr_line_offset)){
		DBG_ERR("Err, chanel enable with bad config!\r\n");
		//return;
	}

	reg.reg = idec_get_reg(id, offset);

	if (b_en) {
		reg.bit.ch_en = 1;
	} else {
		reg.bit.ch_en = 0;
	}

	idec_set_reg(id, offset, reg.reg);
}

/**
    Video2 window status.

    Video2 window status.

    @param[in] id   ide ID
    @return
      - @b TRUE:    Enable
      - @b FALSE:   Disable.
*/
BOOL idec_get_ch_en(IDE_ID id, CH_ID chid)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return 0;
	}

	reg.reg = idec_get_reg(id, offset);

	return reg.bit.ch_en;
}

/**
    Set ide Video1 buffer read order.

    Set ide Video1 buffer read order.

    @param[in] id   ide ID
    @param[in] b_l2r: Read order
		- IDE_BUFFER_READ_L2R: read from left to right
		- IDE_BUFFER_READ_R2L: read from right to left
    @param[in] b_t2b: Read order
		- IDE_BUFFER_READ_T2B: read from top to bottom
		- IDE_BUFFER_READ_B2T: read from bottom to top

    @return void
*/
void idec_set_ch_flip(IDE_ID id, CH_ID chid, IDE_HOR_READ b_l2r, IDE_VER_READ b_t2b)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	if (b_l2r) {
		reg.bit.l2r = 1;
	} else {
		reg.bit.l2r = 0;
	}

	if (b_t2b) {
		reg.bit.t2b = 1;
	} else {
		reg.bit.t2b = 0;
	}

	idec_set_reg(id, offset, reg.reg);
}

void idec_set_ch_swap(IDE_ID id, CH_ID chid, BOOL b_swap)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	if (b_swap) {
		reg.bit.ch_swap = 1;
	} else {
		reg.bit.ch_swap = 0;
	}

	idec_set_reg(id, offset, reg.reg);
}

void idec_set_ch_trans444_mode(IDE_ID id, CH_ID chid, BOOL b_t444mode)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	if (b_t444mode) {
		reg.bit.trans444_mode = 1;
	} else {
		reg.bit.trans444_mode = 0;
	}

	idec_set_reg(id, offset, reg.reg);
}


void idec_set_ch_mode(IDE_ID id, CH_ID chid, CH_MODE ui_mode)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	reg.bit.mode = ui_mode;

	idec_set_reg(id, offset, reg.reg);
}

void idec_set_ch_rld_ctrl(IDE_ID id, CH_ID chid, IDE_RLD_COUNT ui_cnt, IDE_RLD_LENGTH ui_len)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	reg.bit.rld_cnt = ui_cnt;
	reg.bit.rld_len = ui_len;

	idec_set_reg(id, offset, reg.reg);
}

void idec_set_ch_mg_ctrl(IDE_ID id, CH_ID chid, UINT8 b_en, UINT8 ui_fmt)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid != LCD_CH_0) return;

	offset = LCD_CH0_SRC_PARM09_REG_OFS;
	reg.reg = idec_get_reg(id, offset);

	if(b_en)
		reg.bit.mg_en = 1;
	else
		reg.bit.mg_en = 0;
	reg.bit.mg_fmt = ui_fmt;

	idec_set_reg(id, offset, reg.reg);
}

void idec_set_mg_pad_color(IDE_ID id, CH_ID chid, UINT32 ui_y, UINT32 ui_cb, UINT32 ui_cr)
{
	T_LCD_MG_PADDING_COLOR_REG reg;
	UINT32 offset;

	offset = LCD_MG_PADDING_COLOR_REG_OFS;
	reg.reg = idec_get_reg(id, offset);

	reg.bit.mg_color_y = ui_y;
	reg.bit.mg_color_cb = ui_cb;
	reg.bit.mg_color_cr = ui_cr;

	idec_set_reg(id, offset, reg.reg);
}

void idec_set_ch_sc_dim(IDE_ID id, CH_ID chid, UINT32 ui_width, UINT32 ui_height)
{
	T_LCD_CH0_SRC_PARM07_REG reg;
	UINT32 offset;

	switch(chid){
	case LCD_CH_0:
		offset = LCD_CH0_SRC_PARM07_REG_OFS;
		break;
	case LCD_CH_1:
		offset = LCD_CH1_SRC_PARM17_REG_OFS;
		break;
	case LCD_CH_2:
		offset = LCD_CH2_SRC_PARM27_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	reg.bit.sc_hs = ui_width;
	reg.bit.sc_vs = ui_height;

	idec_set_reg(id, offset, reg.reg);
}


void idec_set_csc_in_offset(IDE_ID id, CSC_ID cscid, UINT16 ui_in_ofs0, UINT16 ui_in_ofs1, UINT16 ui_in_ofs2)
{
	T_LCD_CSC_PARM00_REG reg;
	T_LCD_CSC_PARM01_REG reg2;
	UINT32 offset1, offset2;
	UINT8 csc_id = cscid;
	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM00_REG_OFS;
		offset2 = LCD_CH0_CSC_PARM01_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM10_REG_OFS;
		offset2 = LCD_CH1_CSC_PARM11_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM20_REG_OFS;
		offset2 = LCD_CH2_CSC_PARM21_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM0_REG_OFS;
		offset2 = LCD_CURSOR_CSC_PARM1_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM0_REG_OFS;
		offset2 = LCD_CSC1_PARM1_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM0_REG_OFS;
		offset2 = LCD_CSC2_PARM1_REG_OFS;
		break;
	case LCD_CSC_CSC3:
		offset1 = LCD_CSC3_PARM0_REG_OFS;
		offset2 = LCD_CSC3_PARM1_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.csc_in_offset0 = ui_in_ofs0&0xFFF;
	reg.bit.csc_in_offset1 = ui_in_ofs1&0xFFF;
	idec_set_reg(id, offset1, reg.reg);

	reg2.reg = idec_get_reg(id, offset2);	
	reg2.bit.csc_in_offset2 = ui_in_ofs2&0xFFF;
	idec_set_reg(id, offset2, reg2.reg);
	
}

void idec_set_csc_out_offset(IDE_ID id, CSC_ID cscid, UINT16 ui_out_ofs0, UINT16 ui_out_ofs1, UINT16 ui_out_ofs2)
{
	T_LCD_CSC_PARM02_REG reg;
	T_LCD_CSC_PARM03_REG reg2;
	UINT32 offset1, offset2;
	UINT8 csc_id = cscid;
	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM02_REG_OFS;
		offset2 = LCD_CH0_CSC_PARM03_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM12_REG_OFS;
		offset2 = LCD_CH1_CSC_PARM13_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM22_REG_OFS;
		offset2 = LCD_CH2_CSC_PARM23_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM2_REG_OFS;
		offset2 = LCD_CURSOR_CSC_PARM3_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM2_REG_OFS;
		offset2 = LCD_CSC1_PARM3_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM2_REG_OFS;
		offset2 = LCD_CSC2_PARM3_REG_OFS;
		break;
	case LCD_CSC_CSC3:
		offset1 = LCD_CSC3_PARM2_REG_OFS;
		offset2 = LCD_CSC3_PARM3_REG_OFS;
		break;

	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.csc_out_offset0 = ui_out_ofs0&0xFFF;
	reg.bit.csc_out_offset1 = ui_out_ofs1&0xFFF;
	idec_set_reg(id, offset1, reg.reg);

	reg2.reg = idec_get_reg(id, offset2);	
	reg2.bit.csc_out_offset2 = ui_out_ofs2&0xFFF;
	idec_set_reg(id, offset2, reg2.reg);
	
}

void idec_set_csc_coef_ctrl(IDE_ID id, CSC_ID cscid, UINT16 ui_coef0, UINT16 ui_coef1, UINT16 ui_coef2, UINT16 ui_coef3)
{
	T_LCD_CSC_PARM04_REG reg;
	T_LCD_CSC_PARM05_REG reg2;
	UINT32 offset1, offset2;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM04_REG_OFS;
		offset2 = LCD_CH0_CSC_PARM05_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM14_REG_OFS;
		offset2 = LCD_CH1_CSC_PARM15_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM24_REG_OFS;
		offset2 = LCD_CH2_CSC_PARM25_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM4_REG_OFS;
		offset2 = LCD_CURSOR_CSC_PARM5_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM4_REG_OFS;
		offset2 = LCD_CSC1_PARM5_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM4_REG_OFS;
		offset2 = LCD_CSC2_PARM5_REG_OFS;
		break;
	case LCD_CSC_CSC3:
		offset1 = LCD_CSC3_PARM4_REG_OFS;
		offset2 = LCD_CSC3_PARM5_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.csc_coef0 = ui_coef0&0xFFF;
	reg.bit.csc_coef1 = ui_coef1&0xFFF;
	idec_set_reg(id, offset1, reg.reg);

	reg2.reg = idec_get_reg(id, offset2);
	reg2.bit.csc_coef2 = ui_coef2&0xFFF;
	reg2.bit.csc_coef3 = ui_coef3&0xFFF;
	idec_set_reg(id, offset2, reg2.reg);

}

void idec_set_csc_coef_ctrl2(IDE_ID id, CSC_ID cscid, UINT16 ui_coef4, UINT16 ui_coef5, UINT16 ui_coef6, UINT16 ui_coef7)
{
	T_LCD_CSC_PARM06_REG reg;
	T_LCD_CSC_PARM07_REG reg2;
	UINT32 offset1, offset2;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM06_REG_OFS;
		offset2 = LCD_CH0_CSC_PARM07_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM16_REG_OFS;
		offset2 = LCD_CH1_CSC_PARM17_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM26_REG_OFS;
		offset2 = LCD_CH2_CSC_PARM27_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM6_REG_OFS;
		offset2 = LCD_CURSOR_CSC_PARM7_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM6_REG_OFS;
		offset2 = LCD_CSC1_PARM7_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM6_REG_OFS;
		offset2 = LCD_CSC2_PARM7_REG_OFS;
		break;
	case LCD_CSC_CSC3:
		offset1 = LCD_CSC3_PARM6_REG_OFS;
		offset2 = LCD_CSC3_PARM7_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.csc_coef4 = ui_coef4&0xFFF;
	reg.bit.csc_coef5 = ui_coef5&0xFFF;
	idec_set_reg(id, offset1, reg.reg);

	reg2.reg = idec_get_reg(id, offset2);	
	reg2.bit.csc_coef6 = ui_coef6&0xFFF;
	reg2.bit.csc_coef7 = ui_coef7&0xFFF;
	idec_set_reg(id, offset2, reg2.reg);
	
}

void idec_set_csc_coef_ctrl3(IDE_ID id, CSC_ID cscid, UINT16 ui_coef8)
{
	T_LCD_CSC_PARM08_REG reg;
	UINT32 offset1=0;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM08_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM18_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM28_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM8_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM8_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM8_REG_OFS;
		break;
	case LCD_CSC_CSC3:
		offset1 = LCD_CSC3_PARM8_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.csc_coef8 = ui_coef8&0xFFF;
	idec_set_reg(id, offset1, reg.reg);
	
}

void idec_set_csc_clip(IDE_ID id, CSC_ID cscid, UINT32 ui_in_clip, UINT32 ui_out_clip)
{
	T_LCD_CSC_PARM09_REG reg;
	UINT32 offset1=0;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM09_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM19_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM29_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM9_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM9_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM9_REG_OFS;
		break;
	case LCD_CSC_CSC3:
		offset1 = LCD_CSC3_PARM9_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.csc_input_clipping = ui_in_clip;
	reg.bit.csc_output_clipping = ui_out_clip;
	idec_set_reg(id, offset1, reg.reg);
	
}


/**
    Get ide Video1 buffer read order.

    Get ide Video1 buffer read order.

    @param[in] id   ide ID
    @param[out] pb_l2r Read order
    @param[out] pb_t2b Read order

    @return void
*/
void idec_get_ch_flip(IDE_ID id, CH_ID chid, IDE_HOR_READ *pb_l2r, IDE_VER_READ *pb_t2b)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}
	reg.reg = idec_get_reg(id, offset);
	*pb_l2r = reg.bit.l2r;
	*pb_t2b = reg.bit.t2b;
}

BOOL idec_get_ch_swap(IDE_ID id, CH_ID chid)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return 0;
	}
	reg.reg = idec_get_reg(id, offset);
	return reg.bit.ch_swap;
}

BOOL idec_get_ch_trans444_mode(IDE_ID id, CH_ID chid)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return 0;
	}
	reg.reg = idec_get_reg(id, offset);
	return reg.bit.trans444_mode;
}

void idec_get_ch_mode(IDE_ID id, CH_ID chid, CH_MODE* ui_mode)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}
	reg.reg = idec_get_reg(id, offset);
	*ui_mode = reg.bit.mode;
}

void idec_get_ch_rld_ctrl(IDE_ID id, CH_ID chid, IDE_RLD_COUNT *ui_cnt, IDE_RLD_LENGTH *ui_len)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	*ui_cnt = reg.bit.rld_cnt;
	*ui_len = reg.bit.rld_len;
}

void idec_get_ch_mg_ctrl(IDE_ID id, CH_ID chid, BOOL *ui_en, UINT8 *ui_fmt)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid != LCD_CH_0) return;

	offset = LCD_CH0_SRC_PARM09_REG_OFS;
	reg.reg = idec_get_reg(id, offset);

	*ui_fmt = reg.bit.mg_fmt;
	*ui_en = reg.bit.mg_en;
}

void idec_get_csc_in_offset(IDE_ID id, CSC_ID cscid, UINT32 *ui_in_ofs0, UINT32 *ui_in_ofs1, UINT32 *ui_in_ofs2)
{
	T_LCD_CSC_PARM00_REG reg;
	T_LCD_CSC_PARM01_REG reg2;
	UINT32 offset1, offset2;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM00_REG_OFS;
		offset2 = LCD_CH0_CSC_PARM01_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM10_REG_OFS;
		offset2 = LCD_CH1_CSC_PARM11_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM20_REG_OFS;
		offset2 = LCD_CH2_CSC_PARM21_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM0_REG_OFS;
		offset2 = LCD_CURSOR_CSC_PARM1_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM0_REG_OFS;
		offset2 = LCD_CSC1_PARM1_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM0_REG_OFS;
		offset2 = LCD_CSC2_PARM1_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	*ui_in_ofs0 = reg.bit.csc_in_offset0;
	*ui_in_ofs1 = reg.bit.csc_in_offset1;

	reg2.reg = idec_get_reg(id, offset2);	
	*ui_in_ofs2 = reg2.bit.csc_in_offset2;
	
}

void idec_get_csc_out_offset(IDE_ID id, CSC_ID cscid, UINT32 *ui_out_ofs0, UINT32 *ui_out_ofs1, UINT32 *ui_out_ofs2)
{
	T_LCD_CSC_PARM02_REG reg;
	T_LCD_CSC_PARM03_REG reg2;
	UINT32 offset1, offset2;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM02_REG_OFS;
		offset2 = LCD_CH0_CSC_PARM03_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM12_REG_OFS;
		offset2 = LCD_CH1_CSC_PARM13_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM22_REG_OFS;
		offset2 = LCD_CH2_CSC_PARM23_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM2_REG_OFS;
		offset2 = LCD_CURSOR_CSC_PARM3_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM2_REG_OFS;
		offset2 = LCD_CSC1_PARM3_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM2_REG_OFS;
		offset2 = LCD_CSC2_PARM3_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	*ui_out_ofs0 = reg.bit.csc_out_offset0;
	*ui_out_ofs1 = reg.bit.csc_out_offset1;

	reg2.reg = idec_get_reg(id, offset2);	
	*ui_out_ofs2 = reg2.bit.csc_out_offset2;
	
}

void idec_get_csc_coef_ctrl(IDE_ID id, CSC_ID cscid, UINT32 *ui_coef0, UINT32 *ui_coef1, UINT32 *ui_coef2, UINT32 *ui_coef3)
{
	T_LCD_CSC_PARM04_REG reg;
	T_LCD_CSC_PARM05_REG reg2;
	UINT32 offset1, offset2;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM04_REG_OFS;
		offset2 = LCD_CH0_CSC_PARM05_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM14_REG_OFS;
		offset2 = LCD_CH1_CSC_PARM15_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM24_REG_OFS;
		offset2 = LCD_CH2_CSC_PARM25_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM4_REG_OFS;
		offset2 = LCD_CURSOR_CSC_PARM5_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM4_REG_OFS;
		offset2 = LCD_CSC1_PARM5_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM4_REG_OFS;
		offset2 = LCD_CSC2_PARM5_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	*ui_coef0 = reg.bit.csc_coef0;
	*ui_coef1 = reg.bit.csc_coef1;

	reg2.reg = idec_get_reg(id, offset2);	
	*ui_coef2 = reg2.bit.csc_coef2;
	*ui_coef3 = reg2.bit.csc_coef3;
	
}

void idec_get_csc_coef_ctrl2(IDE_ID id, CSC_ID cscid, UINT32 *ui_coef4, UINT32 *ui_coef5, UINT32 *ui_coef6, UINT32 *ui_coef7)
{
	T_LCD_CSC_PARM06_REG reg;
	T_LCD_CSC_PARM07_REG reg2;
	UINT32 offset1, offset2;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM06_REG_OFS;
		offset2 = LCD_CH0_CSC_PARM07_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM16_REG_OFS;
		offset2 = LCD_CH1_CSC_PARM17_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM26_REG_OFS;
		offset2 = LCD_CH2_CSC_PARM27_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM6_REG_OFS;
		offset2 = LCD_CURSOR_CSC_PARM7_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM6_REG_OFS;
		offset2 = LCD_CSC1_PARM7_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM6_REG_OFS;
		offset2 = LCD_CSC2_PARM7_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	*ui_coef4 = reg.bit.csc_coef4;
	*ui_coef5 = reg.bit.csc_coef5;

	reg2.reg = idec_get_reg(id, offset2);	
	*ui_coef6 = reg2.bit.csc_coef6;
	*ui_coef7 = reg2.bit.csc_coef7;
	
}

void idec_get_csc_coef_ctrl3(IDE_ID id, CSC_ID cscid, UINT32 *ui_coef8)
{
	T_LCD_CSC_PARM08_REG reg;
	UINT32 offset1=0;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM08_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM18_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM28_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM8_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM8_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM8_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	*ui_coef8 = reg.bit.csc_coef8;
	
}

void idec_get_csc_clip(IDE_ID id, CSC_ID cscid, UINT32 *ui_in_clip, UINT32 *ui_out_clip)
{
	T_LCD_CSC_PARM09_REG reg;
	UINT32 offset1=0;
	UINT8 csc_id = cscid;

	switch(csc_id){
	case LCD_CSC_CH0:
		offset1 = LCD_CH0_CSC_PARM09_REG_OFS;
		break;
	case LCD_CSC_CH1:
		offset1 = LCD_CH1_CSC_PARM19_REG_OFS;
		break;
	case LCD_CSC_CH2:
		offset1 = LCD_CH2_CSC_PARM29_REG_OFS;
		break;
	case LCD_CSC_CURSOR:
		offset1 = LCD_CURSOR_CSC_PARM9_REG_OFS;
		break;
	case LCD_CSC_CSC1:
		offset1 = LCD_CSC1_PARM9_REG_OFS;
		break;
	case LCD_CSC_CSC2:
		offset1 = LCD_CSC2_PARM9_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	*ui_in_clip = reg.bit.csc_input_clipping;
	*ui_out_clip = reg.bit.csc_output_clipping;
	
}

void idec_get_ch_sc_dim(IDE_ID id, CH_ID chid, UINT32 *ui_width, UINT32 *ui_height)
{
	T_LCD_CH0_SRC_PARM07_REG reg;
	UINT32 offset;

	switch(chid){
	case LCD_CH_0:
		offset = LCD_CH0_SRC_PARM07_REG_OFS;
		break;
	case LCD_CH_1:
		offset = LCD_CH1_SRC_PARM17_REG_OFS;
		break;
	case LCD_CH_2:
		offset = LCD_CH2_SRC_PARM27_REG_OFS;
		break;
	default:
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	*ui_width = reg.bit.sc_hs;
	*ui_height = reg.bit.sc_vs;
}


/**
    Set ide Video1 window format.

    Set ide Video1 window format.

    @param[in] id   ide ID
    @param[in] ui_fmt: window data format
		- COLOR_YCBCR444: YCbCr 444
		- COLOR_YCBCR422: YCbCr 422
		- COLOR_YCBCR420: YCbCr 420
		- COLOR_YCC422P : YCbCr 422P
		- COLOR_YCC420P : YCbCr 420P

    @return void
*/
void idec_set_ch_fmt(IDE_ID id, CH_ID chid, IDE_COLOR_FORMAT ui_fmt)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	reg.bit.fmt = ui_fmt;

	idec_set_reg(id, offset, reg.reg);
}


/**
    Get ide Video1 window format.

    Get ide Video1 window format.

    @param[in] id   ide ID
    @param[out]  ui_fmt  window data format

    @return void
*/
void idec_get_ch_fmt(IDE_ID id, CH_ID chid, IDE_COLOR_FORMAT *ui_fmt)
{
	T_LCD_CH0_SRC_PARM09_REG reg;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM09_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM19_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM29_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset);

	*ui_fmt = reg.bit.fmt;
}

//@}

/**
@name ide Video color Key and Alpha Blending
*/
//@{

/**
    Set ide Video color key

    Set ide Video color key

    @param[in] id   ide ID
    @param[in] ui_ck_y color key Y.
    @param[in] ui_ck_cb color key CB.
    @param[in] ui_ck_cr color key CR.

    @return void
*/
void idec_set_ch_colorkey(IDE_ID id, CH_ID chid, UINT32 ui_ck_low, UINT32 ui_ck_high)
{
	T_LCD_CKEY_V0_REG reg;
	T_LCD_CKEY_V1_REG reg2;
	UINT32 offset1,offset2;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CKEY_V0_REG_OFS;
		offset2 = LCD_CH0_CKEY_V1_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_CKEY_V0_REG_OFS;
		offset2 = LCD_CH1_CKEY_V1_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CKEY_V0_REG_OFS;
		offset2 = LCD_CH2_CKEY_V1_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.low_ckey0 = (ui_ck_low & 0xFF);// Y
	reg.bit.low_ckey1 = ((ui_ck_low>>8) & 0xFF);//U
	reg.bit.low_ckey2 = ((ui_ck_low>>16) & 0xFF);//V
	reg.reg = ui_ck_low;
	idec_set_reg(id, offset1, reg.reg);
	reg2.reg = idec_get_reg(id, offset2);
	reg2.bit.high_ckey0 = (ui_ck_high & 0xFF);// Y
	reg2.bit.high_ckey1 = ((ui_ck_high>>8) & 0xFF);//U
	reg2.bit.high_ckey2 = ((ui_ck_high>>16) & 0xFF);//V
	reg.reg = ui_ck_high;
	idec_set_reg(id, offset2, reg2.reg);

}

void idec_get_ch_colorkey(IDE_ID id, CH_ID chid, UINT32 *ui_ck_low, UINT32 *ui_ck_high)
{
	T_LCD_CKEY_V0_REG reg;
	T_LCD_CKEY_V1_REG reg2;
	UINT32 offset1,offset2;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CKEY_V0_REG_OFS;
		offset2 = LCD_CH0_CKEY_V1_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_CKEY_V0_REG_OFS;
		offset2 = LCD_CH1_CKEY_V1_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CKEY_V0_REG_OFS;
		offset2 = LCD_CH2_CKEY_V1_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset1);
	*ui_ck_low = reg.bit.low_ckey0+(reg.bit.low_ckey1 << 8)+(reg.bit.low_ckey2 << 16);

	reg2.reg = idec_get_reg(id, offset2);
	*ui_ck_high = reg2.bit.high_ckey0+(reg2.bit.high_ckey1 << 8)+(reg2.bit.high_ckey2 << 16);


}


/**
    Set ide video1 and video2 color key selection.

    Set ide video1 and video2 color key selection.

    @param[in] id   ide ID
    @param[in] b_sel IDE_VIDEO_COLORKEY_COMPAREVIDEO2 or IDE_VIDEO_COLORKEY_COMPAREVIDEO1

    @return void
*/
void idec_set_ch_colorkey_en(IDE_ID id, CH_ID chid, BOOL b_en)
{
	T_LCD_CKEY_V1_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CKEY_V1_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_CKEY_V1_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CKEY_V1_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset1);

	if (b_en) {
		reg.bit.ckey_en = 1;
	} else {
		reg.bit.ckey_en = 0;
	}

	idec_set_reg(id, offset1, reg.reg);
}

BOOL idec_get_ch_colorkey_en(IDE_ID id, CH_ID chid)
{
	T_LCD_CKEY_V1_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CKEY_V1_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_CKEY_V1_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CKEY_V1_REG_OFS;
	}else{
		return 0;
	}

	reg.reg = idec_get_reg(id, offset1);
	return reg.bit.ckey_en;
}


/**
    Set ide video1 and video2 color key operation.

    Set ide video1 and video2 color key operation.

    @param[in] id   ide ID
    @param[in] ui_ck_op the colorkey operation for video.

    @return void
*/
void idec_set_ch_colorkey_mask(IDE_ID id, CH_ID chid, UINT32 ui_mask)
{

}

void idec_get_ch_colorkey_mask(IDE_ID id, CH_ID chid, UINT32 *ui_mask)
{

}

void idec_set_ch_rld_readsize(IDE_ID id, CH_ID chid, UINT32 ui_size)
{
	T_LCD_RLD_READ_SIZE_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_RLD_READ_SIZE_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_RLD_READ_SIZE_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_RLD_READ_SIZE_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.rld_read_size = ui_size;

	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_ch_rld_alpha_readsize(IDE_ID id, CH_ID chid, UINT32 ui_size)
{
	T_LCD_RLD_A_READ_SIZE_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_RLD_A_READ_SIZE_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_RLD_A_READ_SIZE_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_RLD_A_READ_SIZE_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.rld_a_read_size = ui_size;

	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_mixer_ctrl(IDE_ID id, UINT32 ui_plane)
{
	T_LCD_MIXER_CTRL0_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_MIXER_CTRL0_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.layer0_plane = ui_plane & 0x3;
	reg.bit.layer1_plane = (ui_plane>>2) & 0x3;
	reg.bit.layer2_plane = (ui_plane>>4) & 0x3;

	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_mixer_ctrl1(IDE_ID id, UINT32 layerid, UINT32 ui_layer_x, UINT32 ui_layer_y )
{
	T_LCD_MIXER_CTRL_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_MIXER_CTRL1_REG_OFS+layerid*4;

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.layer_xi = ui_layer_x;
	reg.bit.layer_yi = ui_layer_y;
	idec_set_reg(id, offset1, reg.reg);
}



void idec_get_mixer_ctrl(IDE_ID id, UINT32* ui_plane)
{
	T_LCD_MIXER_CTRL0_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_MIXER_CTRL0_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);

	*ui_plane = reg.bit.layer0_plane |(reg.bit.layer1_plane<<2)|(reg.bit.layer2_plane<<4);

}

void idec_get_mixer_ctrl1(IDE_ID id, UINT32 layerid, UINT32 *ui_layer_x, UINT32 *ui_layer_y )
{
	T_LCD_MIXER_CTRL_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_MIXER_CTRL1_REG_OFS+layerid*4;

	reg.reg = idec_get_reg(id, offset1);

	*ui_layer_x = reg.bit.layer_xi;
	*ui_layer_y = reg.bit.layer_yi;
}

void idec_set_color_bar(IDE_ID id, UINT32 cb_id, UINT32 ui_r, UINT32 ui_g, UINT32 ui_b )
{
	T_LCD_COLOR_BAR_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_COLOR_BAR0_REG_OFS+cb_id*4;

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.colbar_r = ui_r;
	reg.bit.colbar_g = ui_g;
	reg.bit.colbar_b = ui_b;
	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_pattern_gen_ctrl(IDE_ID id, UINT8 ch0_patgen, UINT8 ch1_patgen, UINT8 ch2_patgen)
{
	T_LCD_PATTERN_GEN_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_PATTERN_GEN_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.ch0_patgen = ch0_patgen;
	reg.bit.ch1_patgen = ch1_patgen;
	reg.bit.ch2_patgen = ch2_patgen;
	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_pattern_gen_ctrl1(IDE_ID id, UINT8 ui_ch, BOOL b_en)
{
	T_LCD_PATTERN_GEN_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_PATTERN_GEN_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.patgen_ch = ui_ch;
	if(b_en)
		reg.bit.patgen_en = 1;
	else
		reg.bit.patgen_en = 0;
	idec_set_reg(id, offset1, reg.reg);
}

void idec_get_pattern_gen_ctrl(IDE_ID id, UINT8 *ch0_patgen, UINT8 *ch1_patgen, UINT8 *ch2_patgen)
{
	T_LCD_PATTERN_GEN_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_PATTERN_GEN_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);

	*ch0_patgen = reg.bit.ch0_patgen;
	*ch1_patgen = reg.bit.ch1_patgen;
	*ch2_patgen = reg.bit.ch2_patgen;
}

void idec_get_pattern_gen_ctrl1(IDE_ID id, UINT8 *ui_ch, UINT8 *b_en)
{
	T_LCD_PATTERN_GEN_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_PATTERN_GEN_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);

	*ui_ch = reg.bit.patgen_ch;
	*b_en = reg.bit.patgen_en;
	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_ch_ce_blk_dim(IDE_ID id, CH_ID chid, UINT8 blkid, UINT32 ui_width, UINT32 ui_height)
{
	T_LCD_CE_PARM01_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CE_PARM01_REG_OFS+blkid*4;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CE_PARM01_REG_OFS+blkid*4;
	}

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.blk_width = ui_width;
	reg.bit.blk_height = ui_height;

	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_ch_ce_recip(IDE_ID id, CH_ID chid, UINT8 blkid, UINT32 ui_recip)
{
	T_LCD_CE_PARM03_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CE_PARM03_REG_OFS+blkid*4;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CE_PARM03_REG_OFS+blkid*4;
	}

	reg.reg = idec_get_reg(id, offset1);

	reg.bit.recip0 = ui_recip;

	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_ch_ce_ctrl(IDE_ID id, CH_ID chid, UINT32 ui_blnrat, UINT32 ui_th, UINT32 ui_strth)
{
	T_LCD_CE_PARM05_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CE_PARM05_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CE_PARM05_REG_OFS;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.blnrat = ui_blnrat;
	reg.bit.ceth = ui_th;
	reg.bit.strth = ui_strth;
	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_ch_ce_en(IDE_ID id, CH_ID chid, BOOL b_en)
{
	T_LCD_CE_PARM05_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CE_PARM05_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CE_PARM05_REG_OFS;
	}

	reg.reg = idec_get_reg(id, offset1);
	if(b_en)
		reg.bit.ceen = 1;
	else
		reg.bit.ceen = 0;
	idec_set_reg(id, offset1, reg.reg);
}

void idec_set_ch_ce_pos(IDE_ID id, CH_ID chid, UINT32 ui_sx, UINT32 ui_ex, UINT32 ui_sy, UINT32 ui_ey)
{
	T_LCD_CE_PARM06_REG reg;
	T_LCD_CE_PARM06_REG reg2;
	UINT32 offset1=0, offset2=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CE_PARM06_REG_OFS;
		offset2 = LCD_CH0_CE_PARM07_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CE_PARM06_REG_OFS;
		offset2 = LCD_CH0_CE_PARM07_REG_OFS;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.ce_sx = ui_sx;
	reg.bit.ce_sy = ui_sy;
	idec_set_reg(id, offset1, reg.reg);

	reg2.reg = idec_get_reg(id, offset2);
	reg2.bit.ce_sx = ui_ex;
	reg2.bit.ce_sy = ui_ey;
	idec_set_reg(id, offset2, reg2.reg);

}

BOOL idec_get_ch_ce_en(IDE_ID id, CH_ID chid)
{
	T_LCD_CE_PARM05_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_CE_PARM05_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_CE_PARM05_REG_OFS;
	}

	reg.reg = idec_get_reg(id, offset1);
	return reg.bit.ceen;
}


void idec_set_ch_brightness(IDE_ID id, CH_ID chid, BOOL b_en, UINT8 ui_bri)
{
	T_LCD_YUV_ADJ_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_YUV_ADJ_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_YUV_ADJ_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_YUV_ADJ_REG_OFS;
	}
	
	reg.reg = idec_get_reg(id, offset1);
	reg.bit.bright_en = b_en;
	reg.bit.brightness = ui_bri;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_contrast(IDE_ID id, CH_ID chid, BOOL b_en, UINT8 ui_cont, UINT8 ui_mode)
{
	T_LCD_YUV_ADJ_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_YUV_ADJ_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_YUV_ADJ_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_YUV_ADJ_REG_OFS;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.contrs_en = b_en;
	reg.bit.contrast = ui_cont;
	reg.bit.contrs_mode = ui_mode;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_hue_sat_en(IDE_ID id, CH_ID chid, BOOL b_en)
{
	T_LCD_YUV_ADJ_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_YUV_ADJ_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_YUV_ADJ_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_YUV_ADJ_REG_OFS;
	}

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.hs_en = b_en;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_hue(IDE_ID id, CH_ID chid, UINT8 *ui_hue)
{
	T_LCD_HUE_PARM0_REG reg;
	T_LCD_HUE_PARM1_REG reg2;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_HUE_PARM0_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_HUE_PARM0_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_HUE_PARM0_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	reg.bit.hue0 = ui_hue[0];
	reg.bit.hue1 = ui_hue[1];
	reg.bit.hue2 = ui_hue[2];
	reg.bit.hue3 = ui_hue[3];
	idec_set_reg(id, offset1, reg.reg);
	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_HUE_PARM1_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_HUE_PARM1_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_HUE_PARM1_REG_OFS;
	}
	reg2.reg = idec_get_reg(id, offset1);
	reg2.bit.hue4 = ui_hue[4];
	reg2.bit.hue5 = ui_hue[5];

	idec_set_reg(id, offset1, reg2.reg);

}

void idec_set_ch_sat(IDE_ID id, CH_ID chid, UINT8 *ui_sat)
{
	T_LCD_SAT_PARM0_REG reg;
	T_LCD_SAT_PARM1_REG reg2;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_SAT_PARM0_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_SAT_PARM0_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_SAT_PARM0_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sat0 = ui_sat[0];
	reg.bit.sat1 = ui_sat[1];
	reg.bit.sat2 = ui_sat[2];
	reg.bit.sat3 = ui_sat[3];
	idec_set_reg(id, offset1, reg.reg);

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_SAT_PARM1_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_SAT_PARM1_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_SAT_PARM1_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	reg2.bit.sat4 = ui_sat[4];
	reg2.bit.sat5 = ui_sat[5];
	idec_set_reg(id, offset1, reg2.reg);

}

void idec_set_ch_sdi(IDE_ID id, CH_ID chid, UINT8 ui_bypass, UINT8 ui_cropen, UINT8 ui_mapsel)
{
	T_LCD_SDI_REG reg;
	UINT32 offset1=0;
	UINT32 shift=0;

	offset1 = LCD_SDI_REG_OFS;
	
	
	if(chid == LCD_CH_0){
		shift = 0;
	}else if(chid == LCD_CH_1){
		shift = 1;
	}
	reg.reg = idec_get_reg(id, offset1);	
	reg.bit.bypass &= ~(1<<shift);
	reg.bit.crop_en &= ~(1<<shift);
	reg.bit.map_sel &= ~(1<<shift);
	reg.bit.bypass |= ui_bypass ? (1<<shift): 0;
	reg.bit.crop_en |= ui_cropen ? (1<<shift): 0;
	reg.bit.map_sel |= ui_mapsel ? (1<<shift): 0;

	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_sdi_alg(IDE_ID id, CH_ID chid, UINT8 ui_luma, UINT8 ui_chroma)
{
	T_LCD_SDI1_ALG_SEL_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDI1_ALG_SEL_REG_OFS;
	
	reg.reg = idec_get_reg(id, offset1);
	if(chid == LCD_CH_0){
		reg.bit.sdi0_y_luma_smo_en = ui_luma & 0x03;
		reg.bit.sdi0_y_luma_wadi_en = (ui_luma>>2) & 0x03;
		reg.bit.sdi0_y_luma_mean_en = (ui_luma>>4) & 0x03;
		reg.bit.sdi0_x_luma_algo_en = (ui_luma>>6) & 0x03;
		reg.bit.sdi0_y_chroma_algo_en = ui_chroma & 0x03;
		reg.bit.sdi0_x_chroma_algo_en = (ui_chroma>>2) & 0x03;
	}else if(chid == LCD_CH_1){
		reg.bit.sdi1_y_luma_smo_en = ui_luma & 0x03;
		reg.bit.sdi1_y_luma_wadi_en = (ui_luma>>2) & 0x03;
		reg.bit.sdi1_y_luma_mean_en = (ui_luma>>4) & 0x03;
		reg.bit.sdi1_x_luma_algo_en = (ui_luma>>6) & 0x03;
		reg.bit.sdi1_y_chroma_algo_en = ui_chroma & 0x03;
		reg.bit.sdi1_x_chroma_algo_en = (ui_chroma>>2) & 0x03;

	}
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_sdi_sc_dim0(IDE_ID id, CH_ID chid, UINT16 ui_src_width, UINT16 ui_src_height)
{
	T_LCD_SDI0_SRC_REG reg;
	UINT32 offset1=0;	

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_SRC_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_SRC_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdi0_src_width = ui_src_width;
	reg.bit.sdi0_src_height = ui_src_height;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_sdi_sc_dim1(IDE_ID id, CH_ID chid, UINT16 ui_dst_width, UINT16 ui_dst_height)
{
	T_LCD_SDI0_DST_REG reg;
	UINT32 offset1=0;	

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_DST_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_DST_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdi0_dst_width = ui_dst_width;
	reg.bit.sdi0_dst_height = ui_dst_height;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_sdi_step(IDE_ID id, CH_ID chid, UINT16 ui_hstep, UINT16 ui_vstep)
{
	T_LCD_SDI_STEP_REG reg;
	UINT32 offset1=0;	

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_STEP_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_STEP_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdi_hstep = ui_hstep;
	reg.bit.sdi_vstep = ui_vstep;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_sdi_div(IDE_ID id, CH_ID chid, UINT16 ui_hdiv, UINT16 ui_vdiv)
{
	T_LCD_SDI_DIV_REG reg;
	UINT32 offset1=0;	

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_DIV_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_DIV_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdi_hsca_divisor = ui_hdiv;
	reg.bit.sdi_vsca_divisor = ui_vdiv;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_sdi_mask(IDE_ID id, CH_ID chid, UINT8 ui_hmsk, UINT8 ui_vmsk, UINT16 ui_kval)
{
	T_LCD_SDI_MASK_REG reg;
	UINT32 offset1=0;	

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_MASK_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_MASK_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdi1_havg_pxl_msk = ui_hmsk;
	reg.bit.sdi1_vavg_pxl_msk = ui_vmsk;
	reg.bit.sdi1_kval = ui_kval;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_ch_sdi_crop_width(IDE_ID id, CH_ID chid, UINT16 ui_crop_xst, UINT16 ui_crop_width)
{
	T_LCD_SDI_SRC_CROP_WIDTH_REG reg;
	UINT32 offset=0;

	if(chid == LCD_CH_0){
		offset = LCD_SDI0_SRC_CROP_WIDTH_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_SDI1_SRC_CROP_WIDTH_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset);
	reg.bit.sdi_crop_x_st = ui_crop_xst;
	reg.bit.sdi_crop_width = ui_crop_width;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_ch_sdi_crop_height(IDE_ID id, CH_ID chid, UINT16 ui_crop_yst, UINT16 ui_crop_height)
{
	T_LCD_SDI_SRC_CROP_HEIGHT_REG reg;
	UINT32 offset=0;

	if(chid == LCD_CH_0){
		offset = LCD_SDI0_SRC_CROP_HEIGHT_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_SDI1_SRC_CROP_HEIGHT_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset);
	reg.bit.sdi_crop_y_st = ui_crop_yst;
	reg.bit.sdi_crop_height = ui_crop_height;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_ch_sdi_alpha_wet(IDE_ID id, CH_ID chid, UINT8 ui_x_alpha_wet, UINT8 ui_y_alpha_wet)
{
	T_LCD_SDI_ALPHA_X_REG reg;
	T_LCD_SDI_ALPHA_Y_REG reg2;
	UINT32 offset=0, offset2=0;

	offset = LCD_SDI_ALPHA_X_REG_OFS;
	offset2 = LCD_SDI_ALPHA_Y_REG_OFS;
	reg.reg = idec_get_reg(id, offset);
	reg2.reg = idec_get_reg(id, offset2);
	if(chid == LCD_CH_0){
		reg.bit.sd0_x_alpha_wet = ui_x_alpha_wet;
		reg2.bit.sd0_y_alpha_wet = ui_y_alpha_wet;

	}else if(chid == LCD_CH_1){
		reg.bit.sd1_x_alpha_wet = ui_x_alpha_wet;
		reg2.bit.sd1_y_alpha_wet = ui_y_alpha_wet;
	}
	idec_set_reg(id, offset, reg.reg);
	idec_set_reg(id, offset2, reg2.reg);

}

void idec_set_ch_sdi_hstep_ofs(IDE_ID id, CH_ID chid, UINT32 ui_hstep_ofs)
{
	T_LCD_SD_HSTEP_OFS_REG reg;
	UINT32 offset=0;

	if(chid == LCD_CH_0){
		offset = LCD_SD0_HSTEP_OFS_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_SD1_HSTEP_OFS_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset);
	reg.bit.sd_hstep_ofs = ui_hstep_ofs;
	idec_set_reg(id, offset, reg.reg);

}

void idec_get_ch_sdi(IDE_ID id, CH_ID chid, UINT8 *ui_bypass, UINT8 *ui_cropen, UINT8 *ui_mapsel)
{
	T_LCD_SDI_REG reg;
	UINT32 offset1=0;
	UINT32 shift=0;

	offset1 = LCD_SDI_REG_OFS;
	if(chid == LCD_CH_0){
		shift = 0;
	}else if(chid == LCD_CH_1){
		shift = 1;
	}
	reg.reg = idec_get_reg(id, offset1);
	*ui_bypass = (reg.bit.bypass &(1<<shift)) ? 1 : 0;
	*ui_cropen = (reg.bit.crop_en &(1<<shift)) ? 1 : 0;
	*ui_mapsel = (reg.bit.map_sel &(1<<shift)) ? 1 : 0;

}

void idec_get_ch_sdi_alg(IDE_ID id, CH_ID chid, UINT8 *ui_luma, UINT8 *ui_chroma)
{
	T_LCD_SDI1_ALG_SEL_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDI1_ALG_SEL_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	if(chid == LCD_CH_0){
		*ui_luma = reg.bit.sdi0_y_luma_smo_en |
				(reg.bit.sdi0_y_luma_wadi_en << 2) |
				(reg.bit.sdi0_y_luma_mean_en << 4) |
				(reg.bit.sdi0_x_luma_algo_en << 6);
		*ui_chroma = reg.bit.sdi0_y_chroma_algo_en |
				(reg.bit.sdi0_x_chroma_algo_en << 2);
	}else if(chid == LCD_CH_1){
		*ui_luma = reg.bit.sdi1_y_luma_smo_en |
				(reg.bit.sdi1_y_luma_wadi_en << 2)|
				(reg.bit.sdi1_y_luma_mean_en << 4) |
				(reg.bit.sdi1_x_luma_algo_en << 6);
		*ui_chroma = reg.bit.sdi1_y_chroma_algo_en |
				(reg.bit.sdi1_x_chroma_algo_en << 2);
	}

}

void idec_get_ch_sdi_sc_dim0(IDE_ID id, CH_ID chid, UINT16 *ui_src_width, UINT16 *ui_src_height)
{
	T_LCD_SDI0_SRC_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_SRC_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_SRC_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	*ui_src_width = reg.bit.sdi0_src_width;
	*ui_src_height = reg.bit.sdi0_src_height;

}

void idec_get_ch_sdi_sc_dim1(IDE_ID id, CH_ID chid, UINT16 *ui_dst_width, UINT16 *ui_dst_height)
{
	T_LCD_SDI0_DST_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_DST_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_DST_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	*ui_dst_width = reg.bit.sdi0_dst_width;
	*ui_dst_height = reg.bit.sdi0_dst_height;

}

void idec_get_ch_sdi_step(IDE_ID id, CH_ID chid, UINT16 *ui_hstep, UINT16 *ui_vstep)
{
	T_LCD_SDI_STEP_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_STEP_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_STEP_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	*ui_hstep = reg.bit.sdi_hstep;
	*ui_vstep = reg.bit.sdi_vstep;

}

void idec_get_ch_sdi_div(IDE_ID id, CH_ID chid, UINT16 *ui_hdiv, UINT16 *ui_vdiv)
{
	T_LCD_SDI_DIV_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_DIV_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_DIV_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	*ui_hdiv = reg.bit.sdi_hsca_divisor;
	*ui_vdiv = reg.bit.sdi_vsca_divisor;

}

void idec_get_ch_sdi_mask(IDE_ID id, CH_ID chid, UINT8 *ui_hmsk, UINT8 *ui_vmsk, UINT16 *ui_kval)
{
	T_LCD_SDI_MASK_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_SDI0_MASK_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_SDI1_MASK_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset1);
	*ui_hmsk = reg.bit.sdi1_havg_pxl_msk;
	*ui_vmsk = reg.bit.sdi1_vavg_pxl_msk;
	*ui_kval = reg.bit.sdi1_kval;

}

void idec_get_ch_sdi_crop_width(IDE_ID id, CH_ID chid, UINT16 *ui_crop_xst, UINT16 *ui_crop_width)
{
	T_LCD_SDI_SRC_CROP_WIDTH_REG reg;
	UINT32 offset=0;

	if(chid == LCD_CH_0){
		offset = LCD_SDI0_SRC_CROP_WIDTH_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_SDI1_SRC_CROP_WIDTH_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset);
	*ui_crop_xst = reg.bit.sdi_crop_x_st;
	*ui_crop_width = reg.bit.sdi_crop_width;

}

void idec_get_ch_sdi_crop_height(IDE_ID id, CH_ID chid, UINT16 *ui_crop_yst, UINT16 *ui_crop_height)
{
	T_LCD_SDI_SRC_CROP_HEIGHT_REG reg;
	UINT32 offset=0;

	if(chid == LCD_CH_0){
		offset = LCD_SDI0_SRC_CROP_HEIGHT_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_SDI1_SRC_CROP_HEIGHT_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset);
	*ui_crop_yst = reg.bit.sdi_crop_y_st;
	*ui_crop_height = reg.bit.sdi_crop_height;

}

void idec_get_ch_sdi_alpha_wet(IDE_ID id, CH_ID chid, UINT8 *ui_x_alpha_wet, UINT8 *ui_y_alpha_wet)
{
	T_LCD_SDI_ALPHA_X_REG reg;
	T_LCD_SDI_ALPHA_Y_REG reg2;
	UINT32 offset=0, offset2=0;

	offset = LCD_SDI_ALPHA_X_REG_OFS;
	offset2 = LCD_SDI_ALPHA_Y_REG_OFS;
	reg.reg = idec_get_reg(id, offset);
	reg2.reg = idec_get_reg(id, offset2);

	if(chid == LCD_CH_0){
		*ui_x_alpha_wet = reg.bit.sd0_x_alpha_wet;
		*ui_y_alpha_wet = reg2.bit.sd0_y_alpha_wet;
	}else if(chid == LCD_CH_1){
		*ui_x_alpha_wet = reg.bit.sd1_x_alpha_wet;
		*ui_y_alpha_wet = reg2.bit.sd1_y_alpha_wet;

	}
}

void idec_get_ch_sdi_hstep_ofs(IDE_ID id, CH_ID chid, UINT32 *ui_hstep_ofs)
{
	T_LCD_SD_HSTEP_OFS_REG reg;
	UINT32 offset=0;

	if(chid == LCD_CH_0){
		offset = LCD_SD0_HSTEP_OFS_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_SD1_HSTEP_OFS_REG_OFS;
	}
	reg.reg = idec_get_reg(id, offset);
	*ui_hstep_ofs = reg.bit.sd_hstep_ofs;

}


void idec_set_sdo(IDE_ID id, UINT8 ui_bypass, UINT8 ui_cropen, UINT8 ui_mapsel, UINT8 ui_en)
{
	T_LCD_SDO_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdo_bypass = ui_bypass;
	reg.bit.sdo_crop_en = ui_cropen;
	reg.bit.sdo_en = ui_en;
	reg.bit.sdo_sca_map_sel = ui_mapsel;

	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_sdo_alg(IDE_ID id, UINT8 ui_luma, UINT8 ui_chroma)
{
	T_LCD_SDO1_ALG_SEL_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO1_ALG_SEL_REG_OFS;
	
	reg.reg = idec_get_reg(id, offset1);

	reg.bit.sdo_res0_y_luma_smo_en = ui_luma & 0x03;
	reg.bit.sdo_res0_y_luma_wadi_en = (ui_luma>>2) & 0x03;
	reg.bit.sdo_res0_y_luma_mean_en = (ui_luma>>4) & 0x03;
	reg.bit.sdo_res0_x_luma_algo_en = (ui_luma>>6) & 0x03;
	reg.bit.sdo_res0_y_chroma_algo_en = ui_chroma & 0x03;
	reg.bit.sdo_res0_x_chroma_algo_en = (ui_chroma>>2) & 0x03;

	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_sdo_sc_dim0(IDE_ID id, UINT16 ui_src_width, UINT16 ui_src_height)
{
	T_LCD_SDO_SRC_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_SRC_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdo_src_width = ui_src_width;
	reg.bit.sdo_src_height = ui_src_height;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_sdo_sc_dim1(IDE_ID id, UINT16 ui_dst_width, UINT16 ui_dst_height)
{
	T_LCD_SDO_RES0_DST_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_RES0_DST_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdo_res_dst_width = ui_dst_width;
	reg.bit.sdo_res_dst_height = ui_dst_height;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_sdo_step(IDE_ID id, UINT16 ui_hstep, UINT16 ui_vstep)
{
	T_LCD_SDO_RES_STEP_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_RES0_STEP_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdo_res_hvstep = ui_hstep;
	reg.bit.sdo_res_vstep = ui_vstep;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_sdo_div(IDE_ID id, UINT16 ui_hdiv, UINT16 ui_vdiv)
{
	T_LCD_SDO_DIV_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_DIV_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdo_res_hsca_divisor = ui_hdiv;
	reg.bit.sdo_res_vsca_divisor = ui_vdiv;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_sdo_mask(IDE_ID id, UINT8 ui_hmsk, UINT8 ui_vmsk, UINT16 ui_kval)
{
	T_LCD_SDO_RES_MASK_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_RES0_MASK_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	reg.bit.sdo_res_havg_pxl_msk = ui_hmsk;
	reg.bit.sdo_res_vavg_pxl_msk = ui_vmsk;
	reg.bit.sdo_res_kval = ui_kval;
	idec_set_reg(id, offset1, reg.reg);

}

void idec_set_sdo_crop_width(IDE_ID id, UINT16 ui_crop_xst, UINT16 ui_crop_width)
{
	T_LCD_SDO_RES0_CROP_WIDTH_REG reg;
	UINT32 offset;

	offset = LCD_SDO_RES0_CROP_WIDTH_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.sdo_res0_crop_x_st = ui_crop_xst;
	reg.bit.sdo_res0_crop_width = ui_crop_width;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_sdo_crop_height(IDE_ID id, UINT16 ui_crop_yst, UINT16 ui_crop_height)
{
	T_LCD_SDO_RES_CROP_HEIGHT_REG reg;
	UINT32 offset;

	offset = LCD_SDO_RES0_CROP_HEIGHT_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.sdo_res0_crop_y_st = ui_crop_yst;
	reg.bit.sdo_res0_crop_height = ui_crop_height;
	idec_set_reg(id, offset, reg.reg);

}

void idec_get_sdo(IDE_ID id, UINT8 *ui_bypass, UINT8 *ui_cropen, UINT8 *ui_mapsel, UINT8 *ui_en)
{
	T_LCD_SDO_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	*ui_bypass = reg.bit.sdo_bypass;
	*ui_cropen = reg.bit.sdo_crop_en;
	*ui_en = reg.bit.sdo_en;
	*ui_mapsel = reg.bit.sdo_sca_map_sel;

}

void idec_get_sdo_alg(IDE_ID id, UINT8 *ui_luma, UINT8 *ui_chroma)
{
	T_LCD_SDO1_ALG_SEL_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO1_ALG_SEL_REG_OFS;
	
	reg.reg = idec_get_reg(id, offset1);

	*ui_luma = reg.bit.sdo_res0_y_luma_smo_en|
			(reg.bit.sdo_res0_y_luma_wadi_en << 2)|
			(reg.bit.sdo_res0_y_luma_mean_en << 4)|
			(reg.bit.sdo_res0_x_luma_algo_en << 6);
	*ui_chroma = reg.bit.sdo_res0_y_chroma_algo_en|
			(reg.bit.sdo_res0_x_chroma_algo_en<<2);

}

void idec_get_sdo_sc_dim0(IDE_ID id, UINT16 *ui_src_width, UINT16 *ui_src_height)
{
	T_LCD_SDO_SRC_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_SRC_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	*ui_src_width = reg.bit.sdo_src_width;
	*ui_src_height = reg.bit.sdo_src_height;

}

void idec_get_sdo_sc_dim1(IDE_ID id, UINT16 *ui_dst_width, UINT16 *ui_dst_height)
{
	T_LCD_SDO_RES0_DST_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_RES0_DST_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	*ui_dst_width = reg.bit.sdo_res_dst_width;
	*ui_dst_height = reg.bit.sdo_res_dst_height;

}

void idec_get_sdo_step(IDE_ID id, UINT16 *ui_hstep, UINT16 *ui_vstep)
{
	T_LCD_SDO_RES_STEP_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_RES0_STEP_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	*ui_hstep = reg.bit.sdo_res_hvstep;
	*ui_vstep = reg.bit.sdo_res_vstep;

}

void idec_get_sdo_div(IDE_ID id, UINT16 *ui_hdiv, UINT16 *ui_vdiv)
{
	T_LCD_SDO_DIV_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_DIV_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	*ui_hdiv = reg.bit.sdo_res_hsca_divisor;
	*ui_vdiv = reg.bit.sdo_res_vsca_divisor;

}

void idec_get_sdo_mask(IDE_ID id, UINT8 *ui_hmsk, UINT8 *ui_vmsk, UINT16 *ui_kval)
{
	T_LCD_SDO_RES_MASK_REG reg;
	UINT32 offset1=0;

	offset1 = LCD_SDO_RES0_MASK_REG_OFS;

	reg.reg = idec_get_reg(id, offset1);
	*ui_hmsk = reg.bit.sdo_res_havg_pxl_msk;
	*ui_vmsk = reg.bit.sdo_res_vavg_pxl_msk;
	*ui_kval = reg.bit.sdo_res_kval;

}

void idec_get_sdo_crop_width(IDE_ID id, UINT16 *ui_crop_xst, UINT16 *ui_crop_width)
{
	T_LCD_SDO_RES0_CROP_WIDTH_REG reg;
	UINT32 offset;

	offset = LCD_SDO_RES0_CROP_WIDTH_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*ui_crop_xst = reg.bit.sdo_res0_crop_x_st;
	*ui_crop_width = reg.bit.sdo_res0_crop_width;

}

void idec_get_sdo_crop_height(IDE_ID id, UINT16 *ui_crop_yst, UINT16 *ui_crop_height)
{
	T_LCD_SDO_RES_CROP_HEIGHT_REG reg;
	UINT32 offset;

	offset = LCD_SDO_RES0_CROP_HEIGHT_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*ui_crop_yst = reg.bit.sdo_res0_crop_y_st;
	*ui_crop_height = reg.bit.sdo_res0_crop_height;

}


void idec_set_wb_ctrl(IDE_ID id, UINT8 ui_mode, UINT8 ui_uvswap)
{
	T_LCD_SDO_WB_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_mode = ui_mode;
	reg.bit.wb_uvswap = ui_uvswap;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_wb_ctrl1(IDE_ID id, UINT8 ui_shot, UINT8 ui_oneshot, UINT8 ui_sel)
{
	T_LCD_SDO_WB_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_shot = ui_shot;
	reg.bit.wb_oneshot = ui_oneshot;
	reg.bit.wb_sel = ui_sel;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_wb_ctrl_2(IDE_ID id, UINT8 ui_bit6, UINT8 ui_bit7)
{
	T_LCD_SDO_WB_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_6 = ui_bit6;
	reg.bit.wb_7 = ui_bit7;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_wb_en(IDE_ID id, BOOL b_en)
{
	T_LCD_SDO_WB_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_en = b_en;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_wb_pos(IDE_ID id, UINT32 ui_x, UINT32 ui_y)
{
	T_LCD_SDO_WB_POS_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_POS_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_x = ui_x;
	reg.bit.wb_y = ui_y;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_wb_dim(IDE_ID id, UINT32 ui_width, UINT32 ui_height)
{
	T_LCD_SDO_WB_DIM_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_DIM_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_w = ui_width;
	reg.bit.wb_h = ui_height;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_wb_yaddr(IDE_ID id, IDEDATA_TYPE addr)
{
	T_LCD_WB_ADDR_REG reg;
	T_LCD_WB_HADDR_REG reg2;
	UINT32 offset;

	offset = LCD_WB_ADDR_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_ba = (UINT32)ide_platform_va2pa(id,addr);
	idec_set_reg(id, offset, reg.reg);

	offset = LCD_WB_HADDR_REG_OFS;

	reg2.reg = idec_get_reg(id, offset);
	reg2.bit.wb_hba = (UINT32)(ide_platform_va2pa(id,addr)>>32);
	idec_set_reg(id, offset, reg2.reg);

}


void idec_set_wb_uvaddr(IDE_ID id, IDEDATA_TYPE addr)
{
	T_LCD_WB_CBR_ADDR_REG reg;
	T_LCD_WB_CBR_ADDR_H_REG reg2;
	UINT32 offset;

	offset = LCD_WB_CBR_ADDR_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_cbr_ba = (UINT32)ide_platform_va2pa(id,addr);
	idec_set_reg(id, offset, reg.reg);

	offset = LCD_WB_CBR_ADDR_H_REG_OFS;

	reg2.reg = idec_get_reg(id, offset);
	reg2.bit.wb_cbr_hba = (UINT32)(ide_platform_va2pa(id,addr)>>32);
	idec_set_reg(id, offset, reg2.reg);

}


void idec_set_wb_fmt(IDE_ID id, UINT8 ui_fmt)
{
	T_LCD_WB_CRL_REG reg;
	UINT32 offset;

	offset = LCD_WB_CRL_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_fmt = ui_fmt;
	idec_set_reg(id, offset, reg.reg);

}

void idec_get_wb_ctrl(IDE_ID id, UINT8 *ui_mode, UINT8 *ui_uvswap)
{
	T_LCD_SDO_WB_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*ui_mode = reg.bit.wb_mode;
	*ui_uvswap = reg.bit.wb_uvswap;

}

void idec_get_wb_ctrl1(IDE_ID id, UINT8 *ui_shot, UINT8 *ui_oneshot, UINT8 *ui_sel)
{
	T_LCD_SDO_WB_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*ui_shot = reg.bit.wb_shot;
	*ui_oneshot = reg.bit.wb_oneshot;
	*ui_sel = reg.bit.wb_sel;

}

BOOL idec_get_wb_en(IDE_ID id)
{
	T_LCD_SDO_WB_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	return reg.bit.wb_en;

}

void idec_get_wb_pos(IDE_ID id, UINT32 *ui_x, UINT32 *ui_y)
{
	T_LCD_SDO_WB_POS_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_POS_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*ui_x = reg.bit.wb_x;
	*ui_y = reg.bit.wb_y;

}

void idec_get_wb_dim(IDE_ID id, UINT32 *ui_width, UINT32 *ui_height)
{
	T_LCD_SDO_WB_DIM_REG reg;
	UINT32 offset;

	offset = LCD_SDO_WB_DIM_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*ui_width = reg.bit.wb_w;
	*ui_height = reg.bit.wb_h;

}

void idec_get_wb_yaddr(IDE_ID id, IDEDATA_TYPE *addr)
{
	T_LCD_WB_ADDR_REG reg;
	UINT32 offset;

	offset = LCD_WB_ADDR_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*addr = reg.bit.wb_ba;

}

void idec_get_wb_uvaddr(IDE_ID id, IDEDATA_TYPE *addr)
{
	T_LCD_WB_CBR_ADDR_REG reg;
	UINT32 offset;

	offset = LCD_WB_CBR_ADDR_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*addr = reg.bit.wb_cbr_ba;

}

void idec_get_wb_fmt(IDE_ID id, UINT8 *ui_fmt)
{
	T_LCD_WB_CRL_REG reg;
	UINT32 offset;

	offset = LCD_WB_CRL_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*ui_fmt = reg.bit.wb_fmt;

}


void idec_set_ch_hstep_ofs(IDE_ID id, UINT32 ui_hstep_ofs)
{
	T_LCD_SDO_RES0_HSTEP_OFS_REG reg;
	UINT32 offset;

	offset = LCD_SDO_RES0_HSTEP_OFS_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.sdo_res0_hstep_ofs = ui_hstep_ofs;
	idec_set_reg(id, offset, reg.reg);

}

void idec_set_wb_lofs(IDE_ID id, UINT32 ui_y_lofs, UINT32 ui_uv_lofs)
{
	T_LCD_SDO_WB_LINE_OFS_REG reg;
	T_LCD_SDO_WB_CBR_LINE_OFS_REG reg2;
	UINT32 offset;

	offset = LCD_SDO_WB_LINE_OFS_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.wb_ofs = ui_y_lofs;
	idec_set_reg(id, offset, reg.reg);

	offset = LCD_SDO_WB_CBR_LINE_OFS_REG_OFS;
	reg2.reg = idec_get_reg(id, offset);
	reg2.bit.wb_cbr_ofs = ui_uv_lofs;
	idec_set_reg(id, offset, reg2.reg);


}

void idec_set_ycc_qtable_0(IDE_ID id, UINT8 *ui_table)
{
	T_LCD_YCC_QTBL_IDX_REG reg;
	UINT32 reg_offset;
	int i=0;


	for(i=0;i<16;i+=4){
		reg_offset = LCD_YCC_QTBL_IDX0_REG_OFS+i;
		reg.reg = idec_get_reg(id, reg_offset);
		reg.bit.qtbl0 = (ui_table[i]&0x1f);
		reg.bit.qtbl1 = (ui_table[i+1]&0x1f);
		reg.bit.qtbl2 = (ui_table[i+2]&0x1f);
		reg.bit.qtbl3 = (ui_table[i+3]&0x1f);
		idec_set_reg(id, reg_offset, reg.reg);
	}
	
}

void idec_get_ycc_qtable_0(IDE_ID id, UINT8 *ui_table)
{
	T_LCD_YCC_QTBL_IDX_REG reg;
	UINT32 reg_offset;
	int i=0;

	for(i=0;i<16;i+=4){
		reg_offset = LCD_YCC_QTBL_IDX0_REG_OFS+i;
		reg.reg = idec_get_reg(id, reg_offset);

		ui_table[i] = reg.bit.qtbl0;
		ui_table[i+1] = reg.bit.qtbl1;
		ui_table[i+2] = reg.bit.qtbl2;
		ui_table[i+3] = reg.bit.qtbl3;
	}

}

void idec_set_ycc_th_0(IDE_ID id, UINT8 *ui_th)
{
	T_LCD_YCC_QUALITY_TH0_REG reg;
	UINT32 reg_offset;
	
	reg.bit.dct_lvl_th0 = ui_th[0];
	reg.bit.dct_lvl_th1 = ui_th[1];
	reg.bit.dct_lvl_th2 = ui_th[2];
	reg.bit.dct_lvl_th3 = ui_th[3];
	reg_offset = LCD_YCC_QUALITY_TH0_REG_OFS;
	idec_set_reg(id, reg_offset, reg.reg);
	reg.bit.dct_lvl_th0 = ui_th[4];
	reg.bit.dct_lvl_th1 = ui_th[5];
	reg.bit.dct_lvl_th2 = ui_th[6];
	reg.bit.dct_lvl_th3 = ui_th[7];
	reg_offset = LCD_YCC_QUALITY_TH1_REG_OFS;
	idec_set_reg(id, reg_offset, reg.reg);

}

void idec_get_ycc_th_0(IDE_ID id, UINT8 *ui_th)
{
	T_LCD_YCC_QUALITY_TH0_REG reg;
	UINT32 reg_offset;

	reg_offset = LCD_YCC_QUALITY_TH0_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);

	ui_th[0] = reg.bit.dct_lvl_th0;
	ui_th[1] = reg.bit.dct_lvl_th1;
	ui_th[2] = reg.bit.dct_lvl_th2;
	ui_th[3] = reg.bit.dct_lvl_th3;

	reg_offset = LCD_YCC_QUALITY_TH1_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	ui_th[4] = reg.bit.dct_lvl_th0;
	ui_th[5] = reg.bit.dct_lvl_th1;
	ui_th[6] = reg.bit.dct_lvl_th2;
	ui_th[7] = reg.bit.dct_lvl_th3;	
}


void idec_set_ygamma_idx(IDE_ID id, CH_ID chid, UINT8 *blk_idx)
{
	T_LCD_GAMMA_PARM0_REG reg;
	UINT32 reg_offset=0;
	UINT32 i=0;

	for(i=0;i<33;i+=4){
		if(chid == LCD_CH_0){
			reg_offset = LCD_CH0_GAMMA_PARM0_REG_OFS+i;
		}else if(chid == LCD_CH_1){
			reg_offset = LCD_CH1_GAMMA_PARM0_REG_OFS+i;
		}else if(chid == LCD_CH_2){
			reg_offset = LCD_CH2_GAMMA_PARM0_REG_OFS+i;
		}else{
			return;
		}
		reg.reg = idec_get_reg(id, reg_offset);

		reg.bit.blk0_idx = blk_idx[i] & 0x3F;
		reg.bit.blk1_idx = blk_idx[i+1] & 0x3F;
		reg.bit.blk2_idx = blk_idx[i+2] & 0x3F;
		reg.bit.blk3_idx = blk_idx[i+3] & 0x3F;

		idec_set_reg(id, reg_offset, reg.reg);
	}
}

void idec_set_ygamma_split(IDE_ID id, CH_ID chid, UINT32 blk_split)
{
	T_LCD_GAMMA_PARM4_REG reg;
	UINT32 reg_offset;

	if(chid == LCD_CH_0){
		reg_offset = LCD_CH0_GAMMA_PARM4_REG_OFS;
	}else if(chid == LCD_CH_1){
		reg_offset = LCD_CH1_GAMMA_PARM4_REG_OFS;
	}else if(chid == LCD_CH_2){
		reg_offset = LCD_CH2_GAMMA_PARM4_REG_OFS;
	}else{
		return;
	}
	reg.reg = idec_get_reg(id, reg_offset);

	reg.bit.blk0_split = blk_split & 0x3;
	reg.bit.blk1_split = (blk_split >> 2) & 0x3;
	reg.bit.blk2_split = (blk_split >> 4) & 0x3;
	reg.bit.blk3_split = (blk_split >> 6) & 0x3;
	reg.bit.blk4_split = (blk_split >> 8) & 0x3;
	reg.bit.blk5_split = (blk_split >> 10) & 0x3;
	reg.bit.blk6_split = (blk_split >> 12) & 0x3;
	reg.bit.blk7_split = (blk_split >> 14) & 0x3;
	reg.bit.blk8_split = (blk_split >> 16) & 0x3;
	reg.bit.blk9_split = (blk_split >> 18) & 0x3;
	reg.bit.blk10_split = (blk_split >> 20) & 0x3;
	reg.bit.blk11_split = (blk_split >> 22) & 0x3;
	reg.bit.blk12_split = (blk_split >> 24) & 0x3;
	reg.bit.blk13_split = (blk_split >> 26) & 0x3;
	reg.bit.blk14_split = (blk_split >> 28) & 0x3;
	reg.bit.blk15_split = (blk_split >> 30) & 0x3;


	idec_set_reg(id, reg_offset, reg.reg);
}

void idec_set_ygamma_en(IDE_ID id, CH_ID chid, BOOL b_en)
{
	T_LCD_GAMMA_PARM10_REG reg;
	UINT32 reg_offset;

	reg_offset = LCD_GAMMA_PARM10_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	if(chid == LCD_CH_0){
		reg.bit.ch0_gamma_en = b_en;
	}else if(chid == LCD_CH_1){
		reg.bit.ch1_gamma_en = b_en;
	}else if(chid == LCD_CH_2){
		reg.bit.ch2_gamma_en = b_en;
	}else{
		return;
	}

	idec_set_reg(id, reg_offset, reg.reg);
}

void idec_set_ygamma_lut(IDE_ID id, CH_ID chid, UINT32 lutidx, UINT32 lut0,  UINT32 lut1)
{
	T_LCD_GAMMA_LUT_REG reg;
	UINT32 reg_offset;

	if(chid == LCD_CH_0){
		reg_offset = lutidx*4+LCD_CH0_GAMMA_LUT_REG_OFS;
	}else if(chid == LCD_CH_1){
		reg_offset = lutidx*4+LCD_CH1_GAMMA_LUT_REG_OFS;
	}else if(chid == LCD_CH_2){
		reg_offset = lutidx*4+LCD_CH2_GAMMA_LUT_REG_OFS;
	}else{
		return;
	}
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.gamma_lut0 = lut0 & 0xFFF;
	reg.bit.gamma_lut1 = lut1 & 0xFFF;
	idec_set_reg(id, reg_offset, reg.reg);
}

void idec_set_afbc_channel(IDE_ID id, UINT8 ui_channel)
{
	UINT32 reg_offset;
	T_LCD_AFBC_CTRL_REG reg;

	reg_offset = LCD_AFBC_CTRL_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.chn = ui_channel&3;
	idec_set_reg(id, reg_offset, reg.reg);
}

void idec_set_afbc_block_split(IDE_ID id, BOOL block_split)
{
	UINT32 reg_offset;
	T_LCD_AFBC_CTRL_REG reg;

	reg_offset = LCD_AFBC_CTRL_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.block_split = block_split;
	idec_set_reg(id, reg_offset, reg.reg);
}

void idec_set_afbc_hdr_addr(IDE_ID id, IDEDATA_TYPE ui_addr)
{
	UINT32 reg_offset;
	T_LCD_AFBC_HDR_ADDR_REG reg;
	T_LCD_AFBC_HDR_HADDR_REG reg2;

	reg_offset = LCD_AFBC_HDR_ADDR_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.hdr_ba = (UINT32)ide_platform_va2pa(id,ui_addr);
	idec_set_reg(id, reg_offset, reg.reg);

	reg_offset = LCD_AFBC_HDR_HADDR_REG_OFS;
	reg2.reg = idec_get_reg(id, reg_offset);
	reg2.bit.hdr_hba = (UINT32)(ide_platform_va2pa(id,ui_addr)>>32);
	idec_set_reg(id, reg_offset, reg2.reg);

}

void idec_get_afbc_hdr_addr(IDE_ID id, IDEDATA_TYPE *ui_addr)
{
	UINT32 reg_offset;
	T_LCD_AFBC_HDR_ADDR_REG reg;

	reg_offset = LCD_AFBC_HDR_ADDR_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	*ui_addr = reg.bit.hdr_ba;
}

void idec_get_afbc_checksum(IDE_ID id, UINT8 ui_type, UINT32 *ui_checksum)
{
	UINT32 reg_offset;
	T_LCD_AFBC_CKSUM_REG reg;

	reg_offset = LCD_AFBC_HDR_CKSUM_REG_OFS+ui_type*4;

	reg.reg = idec_get_reg(id, reg_offset);
	*ui_checksum = reg.bit.checksum;
}

void idec_set_afbc_crop_dim(IDE_ID id, UINT8 ui_left_crop, UINT8 ui_top_crop)
{
	UINT32 reg_offset;
	T_LCD_AFBC_CROP_REG reg;

	reg_offset = LCD_AFBC_CROP_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.left_crop = ui_left_crop;
	reg.bit.top_crop = ui_top_crop;
	idec_set_reg(id, reg_offset, reg.reg);

}

void idec_get_afbc_channel(IDE_ID id, UINT8 *ui_channel)
{
	UINT32 reg_offset;
	T_LCD_AFBC_CTRL_REG reg;

	reg_offset = LCD_AFBC_CTRL_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	*ui_channel = reg.bit.chn;
}


void idec_set_mg_en(IDE_ID id, UINT32 ui_idx,BOOL b_en)
{
	UINT32 reg_offset;
	T_LCD_MG_CTRL0_REG reg;

	reg_offset = LCD_MG0_CTRL0_REG_OFS + 0x20*ui_idx;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.mg_en = b_en;
	idec_set_reg(id, reg_offset, reg.reg);

}

void idec_set_mg_ctrl(IDE_ID id, UINT32 ui_idx,UINT8 ui_stx)
{
	UINT32 reg_offset;
	T_LCD_MG_CTRL0_REG reg;

	reg_offset = LCD_MG0_CTRL0_REG_OFS + 0x20*ui_idx;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.mg_stx = ui_stx;
	idec_set_reg(id, reg_offset, reg.reg);

}

void idec_set_mg_dim(IDE_ID id, UINT32 ui_idx,UINT16 ui_w, UINT16 ui_h, UINT16 ui_lofs)
{
	UINT32 reg_offset;
	T_LCD_MG_CTRL1_REG reg1;
	T_LCD_MG_CTRL0_REG reg;

	reg_offset = LCD_MG0_CTRL0_REG_OFS + 0x20*ui_idx;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.mg_lofs = ui_lofs;
	idec_set_reg(id, reg_offset, reg.reg);


	reg_offset = LCD_MG0_CTRL1_REG_OFS + 0x20*ui_idx;
	reg1.reg = idec_get_reg(id, reg_offset);
	reg1.bit.mg_w = ui_w;
	reg1.bit.mg_h = ui_h;
	idec_set_reg(id, reg_offset, reg1.reg);

}

void idec_set_mg_pos(IDE_ID id, UINT32 ui_idx,UINT16 ui_sx, UINT16 ui_sy, UINT16 ui_ex, UINT16 ui_ey)
{
	UINT32 reg_offset;
	T_LCD_MG_CTRL2_REG reg2;
	T_LCD_MG_CTRL3_REG reg3;

	reg_offset = LCD_MG0_CTRL2_REG_OFS + 0x20*ui_idx;
	reg2.reg = idec_get_reg(id, reg_offset);
	reg2.bit.mg_sx = ui_sx;
	reg2.bit.mg_sy = ui_sy;
	idec_set_reg(id, reg_offset, reg2.reg);

	reg_offset = LCD_MG0_CTRL3_REG_OFS + 0x20*ui_idx;
	reg3.reg = idec_get_reg(id, reg_offset);
	reg3.bit.mg_ex = ui_ex;
	reg3.bit.mg_ey = ui_ey;
	idec_set_reg(id, reg_offset, reg3.reg);

}

void idec_set_mg_buf_addr(IDE_ID id,UINT32 ui_idx, IDEDATA_TYPE ui_yaddr, IDEDATA_TYPE ui_uvaddr)
{
	UINT32 reg_offset;
	T_LCD_MG_Y_ADDR_REG reg;
	T_LCD_MG_UV_ADDR_REG reg1;
	T_LCD_MG_Y_HADDR_REG reg2;
	T_LCD_MG_UV_HADDR_REG reg3;


	reg_offset = LCD_MG0_Y_ADDR_REG_OFS + 0x20*ui_idx;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.mg_y_ba = (UINT32)ide_platform_va2pa(id,ui_yaddr);
	idec_set_reg(id, reg_offset, reg.reg);

	reg_offset = LCD_MG0_UV_ADDR_REG_OFS + 0x20*ui_idx;
	reg1.reg = idec_get_reg(id, reg_offset);
	reg1.bit.mg_uv_ba = (UINT32)ide_platform_va2pa(id,ui_uvaddr);
	idec_set_reg(id, reg_offset, reg1.reg);

	reg_offset = LCD_MG0_Y_HADDR_REG_OFS + 0x20*ui_idx;
	reg2.reg = idec_get_reg(id, reg_offset);
	reg2.bit.mg_y_hba = (UINT32)(ide_platform_va2pa(id,ui_yaddr)>>32);
	idec_set_reg(id, reg_offset, reg2.reg);

	reg_offset = LCD_MG0_UV_HADDR_REG_OFS + 0x20*ui_idx;
	reg3.reg = idec_get_reg(id, reg_offset);
	reg3.bit.mg_uv_hba = (UINT32)(ide_platform_va2pa(id,ui_uvaddr)>>32);
	idec_set_reg(id, reg_offset, reg3.reg);


}

BOOL idec_get_mg_en(IDE_ID id, UINT32 ui_idx)
{
	UINT32 reg_offset;
	T_LCD_MG_CTRL0_REG reg;

	reg_offset = LCD_MG0_CTRL0_REG_OFS + 0x20*ui_idx;
	reg.reg = idec_get_reg(id, reg_offset);
	return reg.bit.mg_en;

}

void idec_get_mg_ctrl(IDE_ID id, UINT32 ui_idx, UINT8 *ui_stx)
{
	UINT32 reg_offset;
	T_LCD_MG_CTRL0_REG reg;

	reg_offset = LCD_MG0_CTRL0_REG_OFS + 0x20*ui_idx;
	reg.reg = idec_get_reg(id, reg_offset);
	*ui_stx = reg.bit.mg_stx;

}

void idec_get_mg_dim(IDE_ID id, UINT32 ui_idx,UINT16 *ui_w, UINT16 *ui_h, UINT16 *ui_lofs)
{
	UINT32 reg_offset;
	T_LCD_MG_CTRL1_REG reg1;
	T_LCD_MG_CTRL0_REG reg;

	reg_offset = LCD_MG0_CTRL0_REG_OFS + 0x20*ui_idx;
	reg.reg = idec_get_reg(id, reg_offset);
	*ui_lofs = reg.bit.mg_lofs;

	reg_offset = LCD_MG0_CTRL1_REG_OFS + 0x20*ui_idx;
	reg1.reg = idec_get_reg(id, reg_offset);
	*ui_w = reg1.bit.mg_w;
	*ui_h = reg1.bit.mg_h;

}

void idec_get_mg_pos(IDE_ID id, UINT32 ui_idx,UINT16 *ui_sx, UINT16 *ui_sy, UINT16 *ui_ex, UINT16 *ui_ey)
{
	UINT32 reg_offset;
	T_LCD_MG_CTRL2_REG reg2;
	T_LCD_MG_CTRL3_REG reg3;

	reg_offset = LCD_MG0_CTRL2_REG_OFS + 0x20*ui_idx;
	reg2.reg = idec_get_reg(id, reg_offset);
	*ui_sx = reg2.bit.mg_sx;
	*ui_sy = reg2.bit.mg_sy;

	reg_offset = LCD_MG0_CTRL3_REG_OFS + 0x20*ui_idx;
	reg3.reg = idec_get_reg(id, reg_offset);
	*ui_ex = reg3.bit.mg_ex;
	*ui_ey = reg3.bit.mg_ey;

}

void idec_get_mg_buf_addr(IDE_ID id, UINT32 ui_idx,IDEDATA_TYPE *ui_yaddr, IDEDATA_TYPE *ui_uvaddr)
{
	UINT32 reg_offset;
	T_LCD_MG_Y_ADDR_REG reg;
	T_LCD_MG_UV_ADDR_REG reg1;

	reg_offset = LCD_MG0_Y_ADDR_REG_OFS + 0x20*ui_idx;
	reg.reg = idec_get_reg(id, reg_offset);
	*ui_yaddr = reg.bit.mg_y_ba;

	reg_offset = LCD_MG0_UV_ADDR_REG_OFS + 0x20*ui_idx;
	reg1.reg = idec_get_reg(id, reg_offset);
	*ui_uvaddr = reg1.bit.mg_uv_ba;

}


void idec_set_sw_reset(IDE_ID id, BOOL b_en)
{
	UINT32 reg_offset;
	T_LCD_SW_RESET_REG reg;

	reg_offset = LCD_SW_RESET_REG_OFS;
	reg.reg = idec_get_reg(id, reg_offset);
	reg.bit.sw_rst = b_en;
	idec_set_reg(id, reg_offset, reg.reg);

}



void idec_get_ch_yaddr(IDE_ID id, CH_ID chid, IDEDATA_TYPE *ui_y_addr)
{
	T_LCD_CH0_SRC_PARM00_REG reg;
	UINT32 yoffset;


	if(chid == LCD_CH_0){
		yoffset = LCD_CH0_SRC_PARM00_REG_OFS;
	}else if(chid == LCD_CH_1){
		yoffset = LCD_CH1_SRC_PARM10_REG_OFS;
	}else if(chid == LCD_CH_2){
		yoffset = LCD_CH2_SRC_PARM20_REG_OFS;
	}else{
		return;
	}
	reg.reg = idec_get_reg(id, yoffset);
	*ui_y_addr = reg.bit.y_ba;
}

void idec_get_ch_uvaddr(IDE_ID id, CH_ID chid, IDEDATA_TYPE *ui_cb_addr)
{
	T_LCD_CH0_SRC_PARM01_REG reg2;
	UINT32 uvoffset;


	if(chid == LCD_CH_0){
		uvoffset = LCD_CH0_SRC_PARM01_REG_OFS;
	}else if(chid == LCD_CH_1){
		uvoffset = LCD_CH1_SRC_PARM11_REG_OFS;
	}else if(chid == LCD_CH_2){
		uvoffset = LCD_CH2_SRC_PARM21_REG_OFS;
	}else{
		return;
	}
	
	reg2.reg = idec_get_reg(id, uvoffset);	
	*ui_cb_addr = reg2.bit.cbr_ba;	
}

void idec_get_ch_dim(IDE_ID id, CH_ID chid, UINT32 *ui_bw, UINT32 *ui_bh, UINT32 *ui_y_lofs)
{
	T_LCD_CH0_SRC_PARM02_REG reg_dim;
	T_LCD_CH0_SRC_PARM05_REG reg_lofs;
	//T_LCD_CH0_SRC_PARM06_REG reg;
	UINT32 offset;
	UINT32 y_lofs_ofs;//, uv_lofs_ofs;


	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM02_REG_OFS;
		y_lofs_ofs = LCD_CH0_SRC_PARM05_REG_OFS;
		//uv_lofs_ofs = LCD_CH0_SRC_PARM06_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM12_REG_OFS;
		y_lofs_ofs = LCD_CH1_SRC_PARM15_REG_OFS;
		//uv_lofs_ofs = LCD_CH1_SRC_PARM16_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM22_REG_OFS;
		y_lofs_ofs = LCD_CH2_SRC_PARM25_REG_OFS;
		//uv_lofs_ofs = LCD_CH2_SRC_PARM26_REG_OFS;
	}else{
		return;
	}

	reg_dim.reg = idec_get_reg(id, offset);
	*ui_bw = reg_dim.bit.hs;
	*ui_bh = reg_dim.bit.vs;

	reg_lofs.reg = idec_get_reg(id, y_lofs_ofs);
	*ui_y_lofs = reg_lofs.bit.y_line_offset;

	//reg.reg = idec_get_reg(id, uv_lofs_ofs);
	//*ui_uv_lofs = reg.bit.cbr_line_offset;

}

void idec_get_ch_rld_readsize(IDE_ID id, CH_ID chid, UINT32 *ui_size)
{
	T_LCD_RLD_READ_SIZE_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_RLD_READ_SIZE_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_RLD_READ_SIZE_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_RLD_READ_SIZE_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset1);

	*ui_size = reg.bit.rld_read_size;
}

void idec_get_ch_rld_alpha_readsize(IDE_ID id, CH_ID chid, UINT32 *ui_size)
{
	T_LCD_RLD_A_READ_SIZE_REG reg;
	UINT32 offset1=0;

	if(chid == LCD_CH_0){
		offset1 = LCD_CH0_RLD_A_READ_SIZE_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset1 = LCD_CH1_RLD_A_READ_SIZE_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset1 = LCD_CH2_RLD_A_READ_SIZE_REG_OFS;
	}else{
		return;
	}

	reg.reg = idec_get_reg(id, offset1);

	*ui_size = reg.bit.rld_a_read_size;
}

void idec_get_ch_xy(IDE_ID id, CH_ID chid, UINT32 *ui_y_xy, UINT32 *ui_uv_xy)
{
	T_LCD_CH0_SRC_PARM03_REG reg_y_xy;
	T_LCD_CH0_SRC_PARM04_REG reg_uv_xy;
	UINT32 y_xy_ofs, uv_xy_ofs;

	if(chid == LCD_CH_0){
		y_xy_ofs = LCD_CH0_SRC_PARM03_REG_OFS;
		uv_xy_ofs = LCD_CH0_SRC_PARM04_REG_OFS;
	}else if(chid == LCD_CH_1){
		y_xy_ofs = LCD_CH1_SRC_PARM13_REG_OFS;
		uv_xy_ofs = LCD_CH1_SRC_PARM14_REG_OFS;
	}else if(chid == LCD_CH_2){
		y_xy_ofs = LCD_CH2_SRC_PARM23_REG_OFS;
		uv_xy_ofs = LCD_CH2_SRC_PARM24_REG_OFS;
	}else{
		return;
	}

	reg_y_xy.reg = idec_get_reg(id, y_xy_ofs);
	*ui_y_xy = reg_y_xy.bit.y_xy;

	reg_uv_xy.reg = idec_get_reg(id, uv_xy_ofs);
	*ui_uv_xy = reg_uv_xy.bit.cbr_xy;

}


void idec_get_ch_alpha(IDE_ID id, CH_ID chid, UINT8 *ui_galpha, UINT8 *ui_alpha0, UINT8 *ui_alpha1)
{
	T_LCD_CH0_SRC_PARM08_REG reg_alpha;
	UINT32 offset;

	if(chid == LCD_CH_0){
		offset = LCD_CH0_SRC_PARM08_REG_OFS;
	}else if(chid == LCD_CH_1){
		offset = LCD_CH1_SRC_PARM18_REG_OFS;
	}else if(chid == LCD_CH_2){
		offset = LCD_CH2_SRC_PARM28_REG_OFS;
	}else{
		return;
	}

	reg_alpha.reg = idec_get_reg(id, offset);
	*ui_galpha = reg_alpha.bit.galpha;
	*ui_alpha0 = reg_alpha.bit.alpha0;
	*ui_alpha1 = reg_alpha.bit.alpha1;

}

void idec_set_axi_ctrl(IDE_ID id, UINT16 channel_en, UINT16 lock_dis)
{
	T_LCD_AXI_CTRL0_REG reg;
	UINT32 offset;

	offset = LCD_AXI_CTRL0_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.channel_en = channel_en;
	reg.bit.lock_dis = lock_dis;
	idec_set_reg(id, offset, reg.reg);
}

void idec_set_axi_ctrl1(IDE_ID id, BOOL axi_dis, UINT8 r_ostd_num, UINT8 w_ostd_num)
{
	T_LCD_AXI_CTRL1_REG reg;
	UINT32 offset;

	offset = LCD_AXI_CTRL1_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.axi_dis = axi_dis;
	reg.bit.r_ostd_num = r_ostd_num;
	reg.bit.w_ostd_num = w_ostd_num;
	idec_set_reg(id, offset, reg.reg);
}

void idec_set_hdmi_fmt(IDE_ID id, UINT8 hdmi_fmt)
{
	T_LCD_TIMING_CTRL_PRM0_REG reg;
	UINT32 offset;

	offset = LCD_TIMING_CTRL_PRM0_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	reg.bit.dev_hdmi = hdmi_fmt;
	idec_set_reg(id, offset, reg.reg);
}

void idec_get_hdmi_fmt(IDE_ID id, UINT8 *hdmi_fmt)
{
	T_LCD_TIMING_CTRL_PRM0_REG reg;
	UINT32 offset;

	offset = LCD_TIMING_CTRL_PRM0_REG_OFS;

	reg.reg = idec_get_reg(id, offset);
	*hdmi_fmt = reg.bit.dev_hdmi;
}

void idec_get_axi_rwcnt(IDE_ID id, UINT8 *r_cnt, UINT8 *w_cnt)
{
	UINT32 data=0;
	data = idec_get_reg(id, 0x83c);
	*r_cnt = data & 0xFF;
	*w_cnt = (data >> 16) & 0xFF;
}

