#ifndef __jpeg_api_h_
#define __jpeg_api_h_
//#include "jpeg_drv.h"
/*
int nvt_jpg_api_write_reg(PJPG_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_jpg_api_write_pattern(PJPG_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_jpg_api_read_reg(PJPG_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
*/

//VBR control
typedef enum {
	VBR_NORMAL = 0,
	VBR_TO_CBR = 1,
	//ENUM_DUMMY4WORD(JPG_RC_VBR_STATE),
} JPG_RC_VBR_STATE;

typedef enum {
	JPG_RC_NONE,
	JPG_RC_FIX,
	JPG_RC_CBR,
	JPG_RC_VBR,
} JPG_RC_MODE;

typedef struct _JPEG_RC_INFO_
{
	unsigned int mode;
	unsigned int bitrate;
	unsigned int frame_rate_base;
	unsigned int frame_rate_incr;
	unsigned int init_quality;
	unsigned int max_quality;
	unsigned int min_quality;
	unsigned int quality;
	unsigned int priority;
	unsigned int cur_fps;
	unsigned int min_fps;
} JPEG_RC_INFO;

typedef struct _JPEG_ENG_INFO_
{
	uintptr_t engine_base_pa;
	uintptr_t engine_base_va;
	int active;
	int engine_busy;
	int active_type;
	int clk_flag;
} JPEG_ENG_INFO;

typedef struct JpgSWDec_s
{
	unsigned int img_width;
	unsigned int img_height;
	unsigned int bg_width;
	unsigned int bg_height;
	unsigned int uv_swap;
	unsigned char *jpeg_hdr_addr;
	unsigned int bs_size;
	unsigned char *yuv_addr_va;
	unsigned int raw_size;
	int error_type;
} JpgSWDec;

typedef struct {
	int (*jpg_sw_dec)(JpgSWDec *dec_param);
} JPEG_SW_DEC_FUNC;

extern unsigned int nvt_jpg_get_dbg_level(void);
extern void nvt_jpg_set_dbg_level(unsigned int level);
extern unsigned int nvt_jpg_get_util_record(void);
extern void nvt_jpg_set_util_record(unsigned int util_record);
extern unsigned int nvt_jpeg_get_engine_util(unsigned int chip, unsigned int engine);
extern unsigned int nvt_jpeg_clear_engine_util(unsigned int chip, unsigned int engine);
extern int nvt_jpeg_get_engine_info(unsigned int chip, unsigned int engine, JPEG_ENG_INFO *pEngInfo);

extern int jpeg_get_quality_update_mode(void);
extern int jpeg_set_quality_update_mode(int mode);
extern int jpeg_get_quality_fix_ratio(void);
extern int jpeg_set_quality_fix_ratio(int ratio);
extern int jpeg_get_max_quality_step(void);
extern int jpeg_set_max_quality_step(int step);
extern int jpeg_get_header_checksum_en(void);
extern int jpeg_set_header_checksum_en(int enable);
extern int jpeg_get_SWTimeoutPeriod(void);
extern int jpeg_set_SWTimeoutPeriod(int timeout_period);
extern int jpeg_get_choose_engine_use(void);
extern int jpeg_set_choose_engine_use(int engine);

extern int jpeg_get_update_period(void);
extern int jpeg_set_update_period(int period);
extern int jpeg_get_upper_bound(void);
extern int jpeg_set_upper_bound(int bound);
extern int jpeg_get_lower_bound(void);
extern int jpeg_set_lower_bound(int bound);
extern int jpeg_get_check_dma_idle(void);
extern int jpeg_set_check_dma_idle(int check_en);

extern int jpeg_get_dump_br(void);
extern int jpeg_set_dump_br(int chn);

extern unsigned int jpeg_get_enc_max_chn(void);

// rc api //
extern int jpeg_get_rc_info(unsigned int chip, unsigned int chn, JPEG_RC_INFO *p_rc_param);

extern char *nvt_jpeg_get_version(void);

#endif
