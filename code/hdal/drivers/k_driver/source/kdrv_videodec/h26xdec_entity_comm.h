#ifndef _H26X_DEC_ENTITY_COMMON_H_
#define _H26X_DEC_ENTITY_COMMON_H_

#include "kdrv_videodec.h"

#include "vpu/h265/VpuCode.h"
#include "vps/h265/VpsCode_hevc.h"

#include "vpu/h264/VpuCode.h"
#include "vps/h264/VpsCode.h"

#include "kwrap/list.h"

#define DCACHE_LINE_SIZE (64)

/* function pointers passing to low level driver */
typedef void *(*MALLOC_PTR_dec)(unsigned int size);
typedef void(*FREE_PTR_dec)(void * virt_ptr);

enum
{
	HDR_MASTERING_DISPLAY = 0,
	HDR_BT2020,
};

enum err_position {
	ERR_NONE = 0,
	ERR_TRIG = 1,
	ERR_ISR = 2,
};

enum h26xd_err_type{
	H26XD_OK = 0,					// Operation succeed
	H26XD_SCAN_HEADER_ERR = 1,
	H26XD_SET_REC_BUF_ERR = 2,
	H26XD_ERR_TRIGGER_ERR = 3,
	H26XD_ERR_HW_TIMEOUT = 4,
	H26XD_ERR_AD_ISR_ERR = 5,
	H26XD_ERR_BIN_ISR_ERR = 6,
};

enum {
	YUV_RANGE_BYPASS = 0,
	YUV_RANGE_TV = 1,		/// TV level (Y:16~235, C:16~240)
	YUV_RANGE_PC = 2,		/// PC level (Y:0~255, C:0~255)
	YUV_RANGE_MAXTYPE = 3,
};

/* for calculating engine utilization */
struct utilization_record_t {
	unsigned int utilization_start;  // the start time of an engine utilization measurment (at engine start)
	unsigned int utilization_record; // the result of the latest utilization measurment
	unsigned int engine_start;       // the engine starting time
	unsigned int engine_end;         // the engine ending time
	unsigned int engine_time;        // the last engine running time (end time - star time)
};

struct h26xd_job_item_t {
	KDRV_VDODEC_H26X_CB_DATA *callback_data;
	unsigned int		job_id;  // record job->id
	unsigned int		fd;		// record kflow fd
	unsigned int		codec_type;	// record codec type
	short				chip;	// indicate which chip of hw
	short				engine;  // indicate which engine of hw
	short				chn;     // indicate which channel of hw

	/* list */
	struct vos_list_head	standby_list;
	struct vos_list_head    chan_list;		// belong to which channel
	int                 buf_idx;       // buf_idx associated to this job
	unsigned char       status;		// job_item status
	struct vos_list_head    job_list;   // use to add h264_job_head or h265_job_head

	/* flags for deciding whether a job_item can be freed */
	unsigned char       callback_flg;  /* indicates the job has been callback and removed from all lists */
	unsigned char       in_property_parsed_flg;
	unsigned char       addr_chk_flg;  /* indicates whether the address has been checked */

	unsigned char       res_flg;       /* indicates whether video_reserve_buffer() is called for output buffer */
	void               *res_out_buf;	// info of reserved output buffer

	unsigned int        puttime;    // putjob time
	unsigned int        starttime;  // start engine time
	unsigned int        finishtime; // finish engine time

	int                	err_num;    /* error number during the decoding (for printing error message in callback thread)*/

	/* info for showing error message */
	enum err_position   err_pos;       /* where the error is detected (TRIG or ISR) */

	/* input bitstream address/size of the frame to be decoded */
	uintptr_t        	bs_addr_va;
	uintptr_t        	bs_addr_pa;
	uintptr_t        	org_bs_addr_pa;	/* keep the bs_addr_pa from kflow due to bs_addr_pa may be changed due to pcie translation! */
	unsigned int        bs_size;
	unsigned int        in_buf_size;
	unsigned int        bs_ddr_id;

	/* input reconstruct address/size of the frame to be decoded */
    uintptr_t        	org_rec_addr; /* keep the rec_addr_pa from kflow due to rec_addr_pa may be changed due to pcie translation! */
	uintptr_t        	rec_addr;
	uintptr_t        	sub_rec_addr;
	uintptr_t        	mbinfo_addr;
	unsigned int        rec_size;
	unsigned int        sub_rec_size;
	unsigned int        mbinfo_size;
	unsigned int        rec_ddr_id;
	unsigned int        sub_rec_ddr_id;
	unsigned int        mbinfo_ddr_id;

	/* input property */
	unsigned int  dst_fmt;
	unsigned int  dst_bg_dim;
	unsigned int  yuv_width_thrd;	//TBD, should be removed
	unsigned char sub_yuv_ratio;	//TBD, should be removed

	/* derive from input property */
	unsigned char sub_yuv_en;      /* sub YUV enable */

	//new
	unsigned int fw_disable;
	unsigned int fw_uv_swap;
	unsigned int extw_uv_swap;
	unsigned int extw_sce_enabled;

	/* output property */
	int         	poc;
	int				yuv_range;

	unsigned int   	job_item_id; /* cotrol buffer of job_item in  __alloc_job_item / __free_job_item */

	unsigned int	hdal_max_ref_num;
	void 			*log_item;	/* job_log_item_t */
}; /* h26xd_job_item_t */

typedef struct dec_init_param
{
	MALLOC_PTR_dec	pfnMalloc;
	FREE_PTR_dec	pfnFree;
	unsigned short	u16MaxWidth;
	unsigned short	u16MaxHeight;
} H26XD_DEC_INIT_PARAM;

typedef struct decoder_engine_info_t
{
	unsigned short  chip_idx;			/* chip idx */
	unsigned short  engine_idx;			/* engine index */
	short   chn_idx;					/* bound channel index */
	short   irq_num;

	uintptr_t 	*pu32BaseAddr;          /* register virtual base address */
	void *bs_addr_va;					/* linear buffer from kmalloc() with PAGE_SIZE */

	/*new add*/
    int ll_buf_idx;
    struct device   *dev;
} DecoderEngInfo;

typedef struct decoder_parameter_t
{
	MALLOC_PTR_dec pfnMalloc;
	FREE_PTR_dec   pfnFree;
    uintptr_t 	*pu32ChnStAddr;  /* channel struct virtual base address */
    unsigned int u32ChnStSize;  /* channel struct size */

	DecoderEngInfo *eng_info;
	ST_H265_VPS_CONTEXT		stVpsCtx_H265;
	ST_H265_VPU_CODEC_CONTEXT	stVpuCtx_H265;

	ST_H264_VPS_CONTEXT		stVpsCtx_H264;
	ST_H264_VPU_CODEC_CONTEXT	stVpuCtx_H264;

	int		chn_idx;
	int		chip_idx;
	unsigned short	u16MaxWidth;
	unsigned short	u16MaxHeight;
	unsigned short	u16FrameWidth;     /* actual frame width (un-crpped) */
	unsigned short	u16FrameHeight;    /* actual frame height (un-crpped) */
	unsigned short	u16SubBgWidth;     /* sub background width */
	unsigned short	u16SubBgHeight;    /* sub background height */

	/* cropping info */
	unsigned short u16CroppingLeft;
	unsigned short u16CroppingRight;
	unsigned short u16CroppingTop;
	unsigned short u16CroppingBottom;
	unsigned short u16FrameWidthCropped;  /* actual frame width - crop_left - crop_right */
	unsigned short u16FrameHeightCropped; /* actual frame height - crop_top - crop_bottom */

	unsigned int	u32Profile;			/* Get the profile information from video decoder */
	unsigned int u32ProgSeq;			/* Source sequence's scan type is Progressive or Interlaced */
	unsigned int u32RefFrameNum;       /* Number of reference frame */

	unsigned short u16ScaleYuvWidthThrd;	//tbd, should be removed
	unsigned char bHasScaledFrame;

	int	max_num_ref_frames;	//Harry
} DecoderParams;

typedef struct dec_buffer_t
{
	unsigned char *dec_yuv_buf_phy;
	unsigned int dec_yuv_buf_size;
	unsigned char *dec_mbinfo_buf_phy;
	unsigned int dec_mbinfo_size;
	unsigned char *dec_scale_buf_phy;
	unsigned int dec_scale_buf_size;
	int buffer_index;
} H26X_DEC_BUFFER;

typedef enum {
	DEC_FRAME_UKWN = 0,
	DEC_FRAME_I,
	DEC_FRAME_P,
	DEC_FRAME_B,
	DEC_FRAME_OTHERS,
} dec_frame_slice_t;

typedef struct
{
	H26X_DEC_BUFFER *ptReconstBuf;  /* input to low level driver */

	unsigned char u8ReleaseBufferNum;		/* number of buffers in the array (u8ReleaseBuffer) */
	unsigned char u8ReleaseBuffer[64];	/* buffers that are not used by decoder/driver. It contains the index of the buffer of decp->dec_buf arrays */

    unsigned int u32BgDim;
	int	i32POC;
	int i32YuvRange;
	int	i32BufferIdx;
	dec_frame_slice_t	slice_type;	/* new. DEC_FRAME_I/P/B. For debug only */

	//new
	unsigned int fw_disable;
	unsigned int fw_uv_swap;
	unsigned int extw_uv_swap;
	unsigned int extw_sce_enabled;
	unsigned int chn_idx;
	unsigned int extw_sub_yuv_en;
	unsigned int extw_sub_yuv_ratio;
	unsigned int yuv_width_thrd;
} H26X_DEC_FRAME_IOCTL;

struct buffer_info_t
{
	uintptr_t 	addr_pa;		/* maybe in rc or ep */
	uintptr_t 	addr_va;
	unsigned int size;
	unsigned int chip_id;
	unsigned int ddr_no;
};

struct bindch_info_t {
	atomic_t is_used;
	atomic_t is_closed;
	atomic_t is_ready;
	unsigned int chn_id;
};

typedef struct {
	unsigned char is_used;      /* whether it is allocated(pushed into dec_buf) */
	unsigned char is_released;  /* whether it is mark as released (during processing release list) */
	unsigned char is_outputed;  /* whether it is mark as outputed (during processing output list). It only indicates the buffers already been processed. */
	uintptr_t     start_pa;
	unsigned int  yuv_buf_size;
	uintptr_t  	  mbinfo_pa;
	unsigned int  mbinfo_size;
	uintptr_t  	  scale_pa;
	unsigned int  scale_buf_size;
	struct h26xd_job_item_t *job_item;
} BufferAddr;

typedef struct {
	u32 trig_fail;
	u32 slice_header_fail;
	u32	state_machine_err;
	u32	no_bufidx;
	u32	internal_err;
    u32	dec_isr_err;
    u32	dec_hwtimeout;
    u32 set_rec_buf_err;
	u32	putjob_cnt;
	u32	callback_cnt;
	u32	record_hw_timeout_cnt;
	u32	record_isr_err_cnt;
    u32 fd;
    u32 resolution;
    u32 last_hwto_val;
    u32	address_err;
    u8 codec_type;
} h26xd_countr_t;

struct h26xd_data_t {
	int chip;   /* new addition */
	int engine; /* indicate which engine this channel is bound to. -1 means no engine is bound */
	int chn;

	DecoderParams           *dec_handle;      // pointer to private data of lowwer level decoder driver
	H26X_DEC_FRAME_IOCTL     dec_frame;       // for storing info of each decoding frame
	H26X_DEC_BUFFER          recon_buf;       // for reconstruct buffer
	BufferAddr               dec_buf[32];     // buffers of a channel, can' exceed MAX_FRAME_BUFFER_SIZE. B-frame L0/L1
	unsigned int             codec_type;      // record codec type
	int                      used_buffer_num; // number of used buffers in dec_buf array
	unsigned int            last_callback_time; // jiffies at last calling callback
	struct vos_list_head    	standby_list;		/* only stores job's state = JOB_STATUS_STANDBY */
	struct vos_list_head		chan_list;			/* stores jobs belong to this channel */
												/* input property */
	unsigned int  dst_bg_dim;	//upper16: width, lower16: height

	//HARRY, TBD. the following both should be removed.
	unsigned int yuv_width_thrd;
	unsigned char  sub_yuv_ratio;


	unsigned char  sub_yuv_en;      // sub YUV enable

	//new
	unsigned int fw_disable;
	unsigned int fw_uv_swap;
	unsigned int extw_uv_swap;
	unsigned int extw_sce_enabled;

	/* input/output property */
	unsigned int  dst_fmt;

	/* output property */
	unsigned char  frame_interlace;	//src_interlace;
	int	max_num_ref_frames;	//Harry
	struct h26xd_job_item_t *curr_job;
}; /* h26xd_data_t */

#define MAX_JOB_LOG_ITEM	512

typedef struct job_log_item {
	unsigned int	bs_len;
	unsigned int	fd;
	unsigned short	width;
	unsigned short	height;
	unsigned int	job_id;
	unsigned short	process_time;	/* hw process time */
	unsigned char	chn;
	char          	err_num;
	unsigned int	hw_timeout;		/* cycle count */
	unsigned char 	codec_type;		/* 0: h264, 1: h265 */
	unsigned char 	slice_type;		/* dec_frame_slice_t */
} job_log_item_t;

typedef struct h26xd_job_log {
	unsigned int  head;
	unsigned int  tail;
	job_log_item_t log_item[MAX_JOB_LOG_ITEM];
} h26xd_job_log_t;

void *H26XDec_Handler_Create(const H26XD_DEC_INIT_PARAM *ptParam, unsigned int chn_st_size, int chip, int ndev);
void H26XDec_Handler_Release(DecoderParams *p_Dec);
int H26XDec_BindEngine(DecoderEngInfo *eng_info, int real_chn_idx, int ll_buf_idx);
int H26XDec_UnbindEngine(DecoderEngInfo *eng_info);

void mark_engine_start(int chip, int engine, unsigned int utilization_period);
void mark_engine_finish(int chip, int engine, unsigned int utilization_period);
int test_engine_idle(int chip, int engine);
int test_and_set_engine_busy(int chip, int engine);
void set_engine_idle(int chip, int engine);
void set_engine_busy(int chip, int engine);

#endif // _H26X_DEC_ENTITY_COMMON_H_
