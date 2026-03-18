#ifndef _JPEG_KDRV_H_
#define _JPEG_KDRV_H_

#include <kwrap/list.h>
#include "jpeg.h"

#define JPEGENC_IDX(chip, chn)				((chip*jpeg_enc_max_chn) + (chn))
#define JPEGENC_IDX_2(chip, engine, chn)	((chip*jpeg_enc_max_chn*2) + (engine*2) + (chn))
#define JPEGDEC_IDX(chip, chn)				((chip*jpeg_dec_max_chn) + (chn))

//////// const variable ////////
#define DRIVER_STATUS_STANDBY				1
#define DRIVER_STATUS_ONGOING				2
#define DRIVER_STATUS_FINISH				3
#define DRIVER_STATUS_FAIL					4
#define DRIVER_STATUS_RETRIGGER				5
#define DRIVER_STATUS_STOP					6
#define DRIVER_STATUS_RELEASE				7

#define JPEG_DEC_TYPE		0
#define JPEG_ENC_TYPE		1

struct job_item_t {
	unsigned int 	job_id;
	unsigned int 	chip;
	unsigned int 	engine;
	unsigned int 	chn;
	struct vos_list_head	engine_list;
	struct vos_list_head	minor_list;
    int                 status;
    unsigned long       puttime;    //putjob time
    unsigned int        starttime;  //start engine time
    unsigned int        finishtime; //finish engine time
    unsigned int        callbacktime;
	// encode
    unsigned int        bs_length;
    unsigned int        first_bs_length;
	unsigned int        bs_offset;

	unsigned int        bs_slice_size;
	//unsigned int        bs_header_length;
	//unsigned int        bs_data_length;
	// deocde
	unsigned int        img_width;
	unsigned int        img_height;
	unsigned int        bg_width;
	unsigned int        bg_height;
	unsigned int        jpeg_type;	// JPEG_DEC_TYPE, JPEG_ENC_TYPE

	//void *param;
	union {
		KDRV_VDOJPGE_PARAM enc_param;
		KDRV_VDOJPGD_PARAM dec_param;
	};

	void *user_data;
	//int	blocking_mode;	/* if callback function is NULL, then use blocking mode */
	void *p_cb_func;
	// callback
	unsigned int 		is_used;	// for job array
	unsigned int        trans422;
	
	/* for re-trigger job. */
	union {
		KDRV_VDOJPGE_PARAM enc_param_backup;
		KDRV_VDOJPGD_PARAM dec_param_backup;
	};
	int					retrigger_cnt;	//if timeout, we have N chances to re-trigger this job again.
};

struct job_item_pool_t
{
    struct job_item_t *job_item_array;
    unsigned int total_num;
    unsigned int cur_idx;
};

struct jpeg_enc_data_t
{
	unsigned int chip;
	unsigned int chn;
	int active;
	JPEG_PROCESS_CFG enc_cfg;
	int new_feature;	// rotation/OSG/MASK/c2g
};

struct jpeg_dec_data_t
{
	unsigned int chip;
	unsigned int chn;
	int active;
	JPEG_DEC_PROCESS_CFG dec_cfg;
};

extern void jpeg_kdrv_close(void);
extern int jpeg_kdrv_init(void);

int jpeg_enc_get_unused_channel(UINT32 chip, UINT32 engine);
int jpeg_dec_get_unused_channel(UINT32 chip, UINT32 engine);
extern int jpeg_enc_release_channel(unsigned int kdrv_id);
extern int jpeg_dec_release_channel(unsigned int kdrv_id);

#endif