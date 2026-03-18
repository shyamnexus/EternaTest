#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/synclink.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/delay.h>

#include <linux/soc/nvt/nvtmem.h>
#include <linux/soc/nvt/fmem.h>
// #include <mach/fmem.h>

#include <kwrap/cpu.h>
#include "kwrap/util.h"
#include "kwrap/mem.h"
#include <kwrap/perf.h> //VOS_TICK

#include "debug.h"
//#include "kdrv_videoenc/kdrv_videoenc.h"
#include "kdrv_videojpeg/kdrv_videoenc_jpeg.h"
#include "jpeg.h"

#define TEST_CHN		4
#define TEST_OSG_MASK	1
#define JE_SLICE_NUM	3

#define JPEG_ENC_SAVE_BS	1

#define JE_BURST_NUM		8
#define JE_MEASURE_PERIOD	5

#define JPEG_RC_FPS		10

#define ALIGN_4(a)		((((a)+3)>>2)<<2)
#define ALIGN_16(a)		((((a)+15)>>4)<<4)
#define ALIGN_64(a)		((((a)+63)>>6)<<6)

#define JPEG_JOB_ID(chn, frm_id)	(((chn)<<8)|((frm_id)&0xFF))
#define JPEG_JOB_CHN(job_id)		((job_id)>>8)
#define JPEG_JOB_FRM_ID(job_id)		((job_id)&0xFF)

#define RING_BUF_TEST		0

// #define DBG_DUMP(fmt, args...)	printk(fmt, ## args)
// #define DBG_ERR(fmt, args...)	printk(fmt, ## args)

typedef struct {
	uintptr_t ori_addr;
	uintptr_t addr_va;
	uintptr_t addr_pa;
	UINT32 size;
	VOS_MEM_CMA_HDL cma_hdl;
} JPEG_BUFFER;

/******************** encode ********************/
typedef struct {
	unsigned int kdrv_id;
	struct file *enc_bs_fp;
	JPEG_BUFFER enc_bs_buf;
	unsigned int enc_bs_len;
	unsigned int first_bs_len;
	unsigned int enc_bs_offset;
	BOOL b_jpeg_enc_cb;
	BOOL b_jpeg_enc_fail;
	unsigned int byte_sum;
	unsigned int frm_num;
	unsigned int slice_idx;
	unsigned int tri_cnt;
	unsigned int cb_cnt;
} JPEG_ENC_INFO;

typedef struct {
	char yuv_fn[0x40];
	UINT32 width;
	UINT32 height;
	UINT32 max_frm_num;
} JPEG_YUV_INFO;

typedef struct {
	char filename[0x40];
	UINT32 width;
	UINT32 height;
	BOOL valid;
} JPEG_OSG_INFO;

typedef struct {
	int pat_idx;
	int sce_en;
	int rot_type;
	int restart;
	// int rc_enable;
} JPEG_SIM_PARAM;

static struct file *je_yuv_fp = NULL;
static JPEG_BUFFER je_yuv_buf = {0};
static JPEG_BUFFER je_user_buf = {0};
static JPEG_ENC_INFO jpg_enc_info[TEST_CHN] = {0};
#define YUV_PAT_NUM	6
static JPEG_YUV_INFO je_yuv_info[YUV_PAT_NUM] = {
	{"/mnt/sd/cif.yuv", 352, 288, 100},
	{"/mnt/sd/cif2.yuv", 352, 288, 120},
	{"/mnt/sd/99_uv_pack.yuv", 1920, 1088, 30},
	{"/mnt/sd/1920x1080_yuv420_compress.yuv", 1920, 1080, 1},
	{"/mnt/sd/2160p.yuv", 3840, 2160, 1},
	{"/mnt/sd/cif_422sp.yuv", 352, 288, 1},
};
#if TEST_OSG_MASK
#define OSG_PAT_NUM	2
static JPEG_BUFFER je_osg_buf[OSG_PAT_NUM] = {0};

static JPEG_OSG_INFO jpeg_osg_pat[OSG_PAT_NUM] = {
	{"16x72_argb1555.bin", 	16, 	72, 0},
	{"224x32_argb1555.bin",	224,	32, 0}
};
#endif
static unsigned int je_frame_cnt = 0;
#if RING_BUF_TEST
static int first_bs_seg_size = 0x1000;
#endif

static int evaluate_bitrate(JPEG_ENC_INFO *p_info, unsigned int bs_size)
{
	p_info->byte_sum += bs_size;
	p_info->frm_num++;
	if (p_info->frm_num >= JPEG_RC_FPS) {
		printk("## bitrate %d (%d) ##\n", (p_info->byte_sum*8*JPEG_RC_FPS)/p_info->frm_num , p_info->frm_num);
		p_info->byte_sum = 0;
		p_info->frm_num = 0;
	}
	return 0;
}

INT32 jpeg_sim_callback_enc(VOID *callback_info, VOID *user_data)
{
	KDRV_VDOJPGE_RESULT *jpeg_result = (KDRV_VDOJPGE_RESULT *)callback_info;
	int chn;

	//printk("jpeg_sim_callback_enc: job 0x%x, status %d, bs len %d (bs offset %d)\n", jpeg_result->job_id, jpeg_result->status, jpeg_result->bs_len, jpeg_result->bs_offset);
	chn = JPEG_JOB_CHN(jpeg_result->job_id);
	if (KDRV_VDOJPGE_OK == jpeg_result->status) {
		jpg_enc_info[chn].enc_bs_len = jpeg_result->bs_len;
		jpg_enc_info[chn].first_bs_len = jpeg_result->first_bs_len;
		jpg_enc_info[chn].enc_bs_offset = jpeg_result->bs_offset;
		jpg_enc_info[chn].b_jpeg_enc_fail = 0;
	}
	else {
		jpg_enc_info[chn].b_jpeg_enc_fail = 1;
	}
	jpg_enc_info[chn].b_jpeg_enc_cb = TRUE;
	jpg_enc_info[chn].cb_cnt++;
	je_frame_cnt++;
	return 0;
}
/******************** encode ********************/

// allocate
#define ALLOCATE_BUFFER_TYPE		1	// 0: cma buffer, 1: kmalloc
static int jpeg_emu_allocate_buffer(JPEG_BUFFER *buf, UINT32 size)
{
	int ret = 0;
#if (1 == ALLOCATE_BUFFER_TYPE)
	buf->addr_va = (uintptr_t)kmalloc(size, GFP_KERNEL);
	if (0 == buf->addr_va)
		return -1;
	buf->addr_pa = (uintptr_t)vos_cpu_get_phy_addr(buf->addr_va);
	buf->size = size;
#else
	struct vos_mem_cma_info_t mem_info;
	ret = vos_mem_init_cma_info(&mem_info, VOS_MEM_CMA_TYPE_CACHE, size);
	buf->cma_hdl = vos_mem_alloc_from_cma(&mem_info);
	if (ret || NULL == buf->cma_hdl || 0 == mem_info.vaddr) {
		DBG_DUMP("%s: allocate buffer from cma error\n", __FUNCTION__);
		return -1;
	}
	buf->addr_va = (uintptr_t)mem_info.vaddr;
	buf->addr_pa = (uintptr_t)mem_info.paddr;
	buf->size = size;
	printk("vos allocate buffer 0x%lx, size %d\n", (unsigned long)buf->addr_va, buf->size);
#endif
	DBG_DUMP("allocate buffer pa 0x%08lx, va 0x%lx, size %d\r\n", (unsigned long)buf->addr_pa, (unsigned long)buf->addr_va, buf->size);
	// fill buffer
	{
		int i;
		unsigned char pat[20] = "NOVATEKIVOTCODEC";
		for (i = 0; i < 10; i++) {
			memcpy((void *)(buf->addr_va+i*16), (void *)pat, 16);
		}
		vos_cpu_dcache_sync(buf->addr_va, 16*16, VOS_DMA_BIDIRECTIONAL);
	}
	return ret;
}

static void jpeg_emu_free_buffer(JPEG_BUFFER *buf)
{
#if (1 == ALLOCATE_BUFFER_TYPE)
	if (buf->addr_va)
		kfree((void *)buf->addr_va);
#else
	vos_mem_release_from_cma(buf->cma_hdl);
#endif
}

static int je_open_file(char *src_fn, char *out_name)
{
	int ret = 0;
#if JPEG_ENC_SAVE_BS
	int chn;
	char fn[0x80];
#endif

	je_yuv_fp = filp_open(src_fn, O_RDONLY, 0777);
	if (IS_ERR(je_yuv_fp)) {
		je_yuv_fp = NULL;
		DBG_ERR("Error to open %s\n", src_fn);
		ret = -1;
		goto exit_open;
	}
	DBG_DUMP("open yuv file %s done\r\n", src_fn);

#if JPEG_ENC_SAVE_BS
	for (chn = 0; chn < TEST_CHN; chn++) {
		sprintf(fn, "/mnt/sd/chn%d_bs_%s.jpg", chn, out_name);
		jpg_enc_info[chn].enc_bs_fp = filp_open(fn, O_WRONLY | O_CREAT, 0777);
		if (IS_ERR(jpg_enc_info[chn].enc_bs_fp)) {
			jpg_enc_info[chn].enc_bs_fp = NULL;
			DBG_ERR("Error to open %s\n", fn);
			ret = -1;
			goto exit_open;
		}
		DBG_DUMP("open bs file %s done\r\n", fn);
	}
#endif

exit_open:
	return ret;
}

static void je_emu_close_file(void)
{
#if JPEG_ENC_SAVE_BS
	int chn;
#endif
	if (je_yuv_fp)
		filp_close(je_yuv_fp, NULL);
#if JPEG_ENC_SAVE_BS
	for (chn = 0; chn < TEST_CHN; chn++) {
		if (jpg_enc_info[chn].enc_bs_fp)
			filp_close(jpg_enc_info[chn].enc_bs_fp, NULL);
	}
#endif
}

static int je_read_src_yuv(uintptr_t addr, int frame_size)
{
	int ret = 0;
	if (je_yuv_fp) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
		ret = vfs_read(je_yuv_fp, (void *)addr, frame_size, &je_yuv_fp->f_pos);
#else
		ret = kernel_read(je_yuv_fp, (void *)addr, (size_t)frame_size, &je_yuv_fp->f_pos);
#endif
	}
	else {
		DBG_ERR("source yuv file no open\n");
		ret = -1;
	}
	vos_cpu_dcache_sync(addr, frame_size, VOS_DMA_BIDIRECTIONAL);
	DBG_IND("read src yuv 0x%lx size %d done\r\n", (unsigned long)addr, frame_size);
	return ret;
}

#if JPEG_ENC_SAVE_BS
static int je_write_bs(int chn, uintptr_t addr, int len)
{
	int ret = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	ret = vfs_write(jpg_enc_info[chn].enc_bs_fp, (unsigned char *)addr, len, &jpg_enc_info[chn].enc_bs_fp->f_pos);
#else
	ret = kernel_write(jpg_enc_info[chn].enc_bs_fp, (unsigned char *)addr, len, &jpg_enc_info[chn].enc_bs_fp->f_pos);
#endif
/*
	{
		unsigned char *ptr = (unsigned char *)addr;
		DBG_IND("jpeg size %d: start %02x %02x; last %02x %02x\n", len, ptr[0], ptr[1], ptr[len-2], ptr[len-1]); 
	}
*/
	//DBG_DUMP("write bs 0x%lx size %d done\r\n", (unsigned long)addr, len);
	return ret;
}
#endif

#if TEST_OSG_MASK
static int je_read_osg(uintptr_t addr, char *osg_fn, UINT32 width, UINT32 height, int buf_size)
{
	int ret = 0;
	struct file *osg_fp = NULL;
	int osg_size;

	osg_size = width*height*2;

	// read osg
	if (osg_size > buf_size) {
		DBG_DUMP("osg size(%d) is larger than buffer size(%d)\n", osg_size, buf_size);
		ret = -1;
		goto exit_osg;
	}

	osg_fp = filp_open(osg_fn, O_RDONLY, 0777);
	if (IS_ERR(osg_fp)) {
		DBG_ERR("Error to open %s\n", osg_fn);
		ret = -1;
		goto exit_osg;
	}
	DBG_DUMP("open osg file %s done\r\n", osg_fn);

	if (osg_fp) {
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
		ret = vfs_read(osg_fp, (void *)addr, osg_size, &osg_fp->f_pos);
	#else
		ret = kernel_read(osg_fp, (void *)addr, (size_t)osg_size, &osg_fp->f_pos);
	#endif
	}
	vos_cpu_dcache_sync(addr, osg_size, VOS_DMA_BIDIRECTIONAL);
	DBG_DUMP("read osg 0x%lx size %d done\r\n", (unsigned long)addr, osg_size);

	if (osg_fp)
		filp_close(osg_fp, NULL);
exit_osg:
	return ret;
}

static int jpeg_emu_prepare_osg_pattern(JPEG_BUFFER *buf, JPEG_OSG_INFO *pat)
{
	int ret = 0;
	char osg_fn[0x100];
	
	// allocate osg
	if (jpeg_emu_allocate_buffer(buf, 352 * 288 * 2) < 0) {
		DBG_DUMP("allocate osg buffer error\r\n");
		ret = -1;
		goto exit_osg;
	}

	sprintf(osg_fn, "/mnt/sd/%s", pat->filename);
	// read osg data
	if (je_read_osg(buf->addr_va, osg_fn, pat->width, pat->height, buf->size) < 0) {
		ret = -1;
		goto exit_osg;
	}

	pat->valid = 1;
exit_osg:
	return ret;
}

// 0xff0000, 0x00ff00, 0x0000ff
// 0xffff00, 0x00ffff, 0xff00ff
// 0xff5500, 0x0055ff, 0xff55ff
// 0xffaa00, 0x00aaff, 0xffaaff
// 0x005500, 0x00aa00, 
// 0xffffff, 0x000000
static UINT8 MASK_PALETTE[3][16] = {
	{76,  149,  29, 255, 178, 105, 126,  78, 155, 176, 128, 205,  49,  99, 255,   0},
	{85,  43,  255,	   0, 171, 212,  56, 227, 184,  28, 199, 156,  99,  71, 128, 128},
	{255, 21,  107, 148,   0, 234, 219,  71, 199, 184,  36, 163,  92,  56, 128, 128},
};

static int je_set_osg_param(UINT32 kdrv_id, UINT32 width, UINT32 height, UINT32 osg_type)
{
	int ret = 0;
	KDRV_VDOJPGE_OSG_WIN osg_param;
	KDRV_VDOJPGE_OSG_GLOBAL osg_global_param;
	UINT32 osg_pat_id = 0;
	int i;

	// set osg param
	memset(&osg_param, 0, sizeof(KDRV_VDOJPGE_OSG_WIN));
	memset(&osg_global_param, 0, sizeof(KDRV_VDOJPGE_OSG_GLOBAL));

	osg_param.enable = 1;
	osg_param.roi_invalid = 1;
	osg_param.st_grap.type = 0;
	osg_param.st_grap.width = jpeg_osg_pat[osg_pat_id].width;
	osg_param.st_grap.height = jpeg_osg_pat[osg_pat_id].height;
	osg_param.st_grap.line_offset = jpeg_osg_pat[osg_pat_id].width * 2; //unit: word (driver handle)
	osg_param.st_grap.addr = je_osg_buf[osg_pat_id].addr_va;
	osg_param.st_grap.addr_pa = je_osg_buf[osg_pat_id].addr_pa;

	osg_param.st_disp.mode = (2 == osg_type ? 1: 0);
	osg_param.st_disp.x_str = 0;
	osg_param.st_disp.y_str = 0;
	osg_param.st_disp.bg_alpha = 0;
	osg_param.st_disp.fg_alpha = 255;
	osg_param.st_disp.mask_type = 0;
	osg_param.st_disp.mask_bd_size = 0;
	osg_param.st_disp.mask_y[0] = MASK_PALETTE[0][0];
	osg_param.st_disp.mask_y[1] = MASK_PALETTE[0][0];
	osg_param.st_disp.mask_cb = MASK_PALETTE[1][0];
	osg_param.st_disp.mask_cr = MASK_PALETTE[2][0];

	if (osg_type > 2) {
		osg_global_param.overlap_type = 1;
		if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_OSG_GLOBAL, &osg_global_param) < 0)
			printk("set osg global fail !! \n");
	}

	if (osg_type <= 2) {
		for (i = 0; i < MAX_JPEG_OSG_NUM; i++) {
			osg_param.layer_idx = 0;
			osg_param.win_idx = i;
			osg_param.st_disp.x_str = i*20;
			osg_param.st_disp.y_str = i*8;
			osg_param.st_disp.mask_y[0] = MASK_PALETTE[0][i];
			osg_param.st_disp.mask_y[1] = MASK_PALETTE[0][i];
			osg_param.st_disp.mask_cb = MASK_PALETTE[1][i];
			osg_param.st_disp.mask_cr = MASK_PALETTE[2][i];
			if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_OSG_WIN, &osg_param) < 0)
				printk("set osg win%d-%d fail\n", osg_param.layer_idx, osg_param.win_idx);
			//printk("set osg win%d done\n", osg_param.win_idx);
		}
	}

	// set OSG
	osg_pat_id = 1;
	for (i = 0; i < MAX_JPEG_OSG_NUM; i++) {
		if (osg_type > 2)
			osg_param.layer_idx = 0;
		osg_param.win_idx = i;
		switch (osg_type) {
		case 3:
			osg_param.st_disp.mode = 0;
			break;
		case 4:
			osg_param.st_disp.mode = 1;
			break;
		default:
			osg_param.st_disp.mode = 2;
			break;
		}
		osg_param.st_disp.mosaic_blk_sz = osg_type % 3;
		osg_param.st_disp.x_str = 0x80 - i*6;//(0x80 < i*6 ? 0 : 0x80 - i*6);
		osg_param.st_disp.y_str = i*0x10;
		osg_param.st_grap.width = jpeg_osg_pat[osg_pat_id].width;
		osg_param.st_grap.height = jpeg_osg_pat[osg_pat_id].height;
		osg_param.st_grap.line_offset = jpeg_osg_pat[osg_pat_id].width * 2; //unit: word (driver handle)
		osg_param.st_grap.addr = je_osg_buf[osg_pat_id].addr_va;
		osg_param.st_grap.addr_pa = je_osg_buf[osg_pat_id].addr_pa;

		osg_param.st_disp.mask_y[0] = MASK_PALETTE[0][i];
		osg_param.st_disp.mask_y[1] = MASK_PALETTE[0][i];
		osg_param.st_disp.mask_cb = MASK_PALETTE[1][i];
		osg_param.st_disp.mask_cr = MASK_PALETTE[2][i];
		if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_OSG_WIN, &osg_param) < 0)
			printk("set osg win%d-%d fail\n", osg_param.layer_idx, osg_param.win_idx);
		//printk("set osg win%d done\n", osg_param.win_idx);
	}

	return ret;
}

static void je_clear_osg_param(UINT32 kdrv_id)
{
	int i, l;
	KDRV_VDOJPGE_OSG_WIN osg_param = {0};
	for (l = 0; l < MAX_JPEG_OSG_LAYER; l++) {
		for (i = 0; i < MAX_JPEG_OSG_NUM; i++) {
			osg_param.enable = 0;
			osg_param.roi_invalid = 0;
			osg_param.layer_idx = l;
			osg_param.win_idx = i;
			if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_OSG_WIN, &osg_param) < 0)
				printk("clear osg win%d-%d fail\n", osg_param.layer_idx, osg_param.win_idx);
		}
	}
}

static int je_set_mask_param(UINT32 kdrv_id, UINT32 width, UINT32 height, UINT32 mask_type)
{
	KDRV_VDOJPGE_MASK_INIT mask_init;
	KDRV_VDOJPGE_MASK_WIN mask_win;
	int x, y;
	//int base_x = 0, base_y = 0;
	int idx;

	mask_init.mosaic_blk_h = 4;
	mask_init.mosaic_blk_w = 4;
	memcpy(mask_init.pal_y, MASK_PALETTE[0], sizeof(mask_init.pal_y));
	memcpy(mask_init.pal_cb, MASK_PALETTE[1], sizeof(mask_init.pal_cb));
	memcpy(mask_init.pal_cr, MASK_PALETTE[2], sizeof(mask_init.pal_cr));

	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_MASK_INIT, &mask_init) < 0)
		DBG_ERR("set mask init fail\n");

	mask_win.enable = 1;
	mask_win.mosaic_en = 0;			///< [r/w] mask window de-identified method. range: 0~1 (0: orignial, 1: mosaic)
	mask_win.line_hit_opt = 0;		///< [r/w] mask window line hit operation. 0: and(inside), 1: or(outside), 2: border, 3: single line or concave border, 4: full concave mask
	mask_win.alpha = 0;				///< [r/w] mask window blending alpha. range : 0 ~ 256
	mask_win.alpha_type = 0;		///< [r/w] mosaic: 0: alpha blending with source and mosaic, 1: alpha blending with source after mosiac and mask color
	mask_win.line_chk_th = 0;		//driver set
	mask_win.hit_width[0] = mask_win.hit_width[1] = mask_win.hit_width[2] = mask_win.hit_width[3] = 2; //hit_width default is 2, range: 0, 2, 4, 8
	mask_win.mask_width = 64;
	mask_win.mask_height = 64;
	for (idx = 0; idx < 16; idx++) {
		if (2 == mask_type) {
			if (idx > 3) {
				mask_win.mosaic_en = 1;
				mask_win.mask_hollow_type = 0; //0: solid, 1: hollow
			} else {
				mask_win.mosaic_en = 1;
				mask_win.line_hit_opt = 0;
			}
		}
		else if (3 == mask_type) {
			if (idx > 3) {
				mask_win.mask_hollow_type = 1; //0: solid, 1: hollow
				mask_win.mask_hollow_size = 4;
			} else {
				mask_win.line_hit_opt = 3;
			}
		}
		else {
			if (idx > 3)
				mask_win.mask_hollow_type = 0; //0: solid, 1: hollow
			else
				mask_win.line_hit_opt = 0;
		}
		mask_win.mask_idx = idx;
		mask_win.pal_sel = idx;
		mask_win.alpha = 128 - (idx % 5) * 30;
		x = (idx % 4) * 72;
		y = (idx / 4) * 72;
		mask_win.st_pos[0].pos_x = x;
		mask_win.st_pos[0].pos_y = y;
		mask_win.st_pos[1].pos_x = x + mask_win.mask_width - 2;
		mask_win.st_pos[1].pos_y = y;
		mask_win.st_pos[2].pos_x = x + mask_win.mask_width - 2;
		mask_win.st_pos[2].pos_y = y + mask_win.mask_height - 2;
		mask_win.st_pos[3].pos_x = x;
		mask_win.st_pos[3].pos_y = y + mask_win.mask_height - 2;
		mask_win.clockwise = 1;

		// if (idx == 0) {
		// 	mask_win.st_pos[0].pos_x = 510;
		// 	mask_win.st_pos[0].pos_y = 286;
		// 	mask_win.st_pos[1].pos_x = 930;
		// 	mask_win.st_pos[1].pos_y = 337;
		// 	mask_win.st_pos[2].pos_x = 960;
		// 	mask_win.st_pos[2].pos_y = 843;
		// 	mask_win.st_pos[3].pos_x = 540;
		// 	mask_win.st_pos[3].pos_y = 843;
		// 	mask_win.clockwise = 1;
		// 	mask_win.line_hit_opt = 3;
		// 	mask_win.alpha = 0;
		// }
		// else if (idx == 1) {
		// 	mask_win.st_pos[0].pos_x = 200;
		// 	mask_win.st_pos[0].pos_y = 50;
		// 	mask_win.st_pos[1].pos_x = 200;
		// 	mask_win.st_pos[1].pos_y = 150;
		// 	mask_win.st_pos[2].pos_x = 280;
		// 	mask_win.st_pos[2].pos_y = 150;
		// 	mask_win.st_pos[3].pos_x = 240;
		// 	mask_win.st_pos[3].pos_y = 130;
		// 	mask_win.clockwise = 0;
		// 	mask_win.line_hit_opt = 3;
		// 	mask_win.alpha = 0;
		// }
		// else if (idx == 2) {
		// 	mask_win.st_pos[0].pos_x = 20;
		// 	mask_win.st_pos[0].pos_y = 20;
		// 	mask_win.st_pos[1].pos_x = 120;
		// 	mask_win.st_pos[1].pos_y = 25;
		// 	mask_win.st_pos[2].pos_x = 240;
		// 	mask_win.st_pos[2].pos_y = 220;
		// 	mask_win.st_pos[3].pos_x = 40;
		// 	mask_win.st_pos[3].pos_y = 230;
		// 	mask_win.clockwise = 1;
		// 	mask_win.line_hit_opt = 3;
		// 	mask_win.alpha = 0;
		// }
		// else if (idx == 3) {
		// 	mask_win.line_hit_opt = 3;
		// 	mask_win.alpha = 0;
		// }
		// else if (idx == 4) {
		// 	mask_win.line_hit_opt = 3;
		// 	mask_win.alpha = 0;
		// }
		// else {
		// 	continue;
		// }

		DBG_IND("jpeg set mask[%d] (%d,%d) (%d,%d) (%d,%d) (%d,%d) \n", mask_win.mask_idx, 
					mask_win.st_pos[0].pos_x, mask_win.st_pos[0].pos_y,
					mask_win.st_pos[1].pos_x, mask_win.st_pos[1].pos_y,
					mask_win.st_pos[2].pos_x, mask_win.st_pos[2].pos_y,
					mask_win.st_pos[3].pos_x, mask_win.st_pos[3].pos_y);

		if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_MASK_WIN, &mask_win) < 0)
			printk("set mask%d fail\n", mask_win.mask_idx);
	}
	return 0;
}

static void je_clear_mask_param(UINT32 kdrv_id)
{
	KDRV_VDOJPGE_MASK_WIN mask_win = {0};
	int i;
	for (i = 0; i < 16; i++) {
		mask_win.enable = 0;
		mask_win.mask_idx = i;
		if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_MASK_WIN, &mask_win) < 0)
			printk("clear mask%d fail\n", mask_win.mask_idx);
	}
}
#endif //TEST_OSG_MASK

static void je_set_gray_param(UINT32 kdrv_id, UINT32 gray_type)
{
	KDRV_VDOJPGE_GRAY gray_cfg = {0};
	if (gray_type) {
		gray_cfg.enable = 1;
		if (gray_type & 0x01)
			gray_cfg.color_to_gray = 1;
		if (gray_type & 0x02)
			gray_cfg.src_color_to_gray = 1;
	}
	else {
		gray_cfg.enable = 0;
	}
	//printk("set gray param: en %d, %d/%d\n", gray_cfg.enable, gray_cfg.color_to_gray, gray_cfg.src_color_to_gray);
	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_GRAY, (void *)&gray_cfg) < 0)
		printk("set color to gray fail\n");
}

static void je_clear_gray_param(UINT32 kdrv_id)
{
	KDRV_VDOJPGE_GRAY gray_cfg = {0};
	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_GRAY, (void *)&gray_cfg) < 0)
		printk("clear color to gray fail\n");;
}

static void je_set_user_data_param(UINT32 kdrv_id, UINT32 user_type)
{
	KDRV_VDOJPGE_USER_DATA user_data_cfg = {0};
	UINT8 *ptr;

	if (jpeg_emu_allocate_buffer(&je_user_buf, 1024) < 0) {
		DBG_ERR("allocate user data buffer error\r\n");
		return;
	}
	ptr = (UINT8 *)je_user_buf.addr_va;
	*(ptr+0) = 0xAB;
	*(ptr+1) = 0xCD;
	*(ptr+2) = 0xEE;
	*(ptr+3) = 0xFF;
	user_data_cfg.data_addr = je_user_buf.addr_va;

	if (user_type & 0x1) {
		user_data_cfg.enable = 1;
		user_data_cfg.data_length = 0x0;
	}
	else if (user_type & 0x2) {
		user_data_cfg.enable = 1;
		user_data_cfg.data_length = 0xF;
	}
	else if (user_type & 0x4) {
		user_data_cfg.enable = 1;
		user_data_cfg.data_length = 0xF0;
	}
	else if (user_type & 0x8) {
		user_data_cfg.enable = 1;
		user_data_cfg.data_length = 0xFF;
	}
	else if (user_type & 0x10) {
		user_data_cfg.enable = 0;
		user_data_cfg.data_length = 0xFF;
	}
	else if (user_type & 0x20) {
		user_data_cfg.enable = 1;
		user_data_cfg.data_length = 0x100;
	}
	else if (user_type & 0x40) {
		user_data_cfg.enable = 0;
		user_data_cfg.data_length = 0x100;
	}
	else {
		user_data_cfg.enable = 1;
		user_data_cfg.data_length = 0x400;
	}

	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_USER_DATA, (void *)&user_data_cfg) < 0) {
		DBG_ERR("set user data fail\n");
	}
}

static void je_clear_user_data_param(UINT32 kdrv_id)
{
	KDRV_VDOJPGE_USER_DATA user_data_cfg = {0};
	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_USER_DATA, (void *)&user_data_cfg) < 0)
		printk("clear user data fail\n");;
}

//roi
static void je_set_roi_param(UINT32 kdrv_id, UINT32 roi_type)
{
	KDRV_VDOJPGE_ROI roi_cfg = {0};
	int roi_idx;
	
	for (roi_idx = 0; roi_idx < MAX_JPEG_ROI_NUM; roi_idx++) {
		if (roi_type & 0x1) {
			//cif
			roi_cfg.stRoiParam[roi_idx].roi_pos_x = (roi_idx % 4) * 6 * 16;
			roi_cfg.stRoiParam[roi_idx].roi_pos_y = (roi_idx / 4) * 5 * 16;
			roi_cfg.stRoiParam[roi_idx].roi_width = 3 * 16;
			roi_cfg.stRoiParam[roi_idx].roi_height = 2 * 16;
			roi_cfg.stRoiParam[roi_idx].roi_enable = 1;
			roi_cfg.stRoiParam[roi_idx].roi_threshold = 15 - (roi_idx % 8);
		}
		else if (roi_type & 0x2) {
			//1080p
			roi_cfg.stRoiParam[roi_idx].roi_pos_x = (2 + (roi_idx % 4) * 30) * 16;
			roi_cfg.stRoiParam[roi_idx].roi_pos_y = (2 + (roi_idx / 4) * 25) * 16;
			roi_cfg.stRoiParam[roi_idx].roi_width = 15 * 16;
			roi_cfg.stRoiParam[roi_idx].roi_height = 10 * 16;
			roi_cfg.stRoiParam[roi_idx].roi_enable = 1;
			roi_cfg.stRoiParam[roi_idx].roi_threshold = 15;
			//printk("ROI param : [%d %d %d %d] \n",roi_cfg.stRoiParam[roi_idx].roi_pos_x,roi_cfg.stRoiParam[roi_idx].roi_pos_y,roi_cfg.stRoiParam[roi_idx].roi_width,roi_cfg.stRoiParam[roi_idx].roi_height);
			//printk("threshold = [%02d %02d %d] \n",roi_cfg.stRoiParam[roi_idx].roi_threshold,roi_cfg.stRoiParam[roi_idx].roi_threshold_ac1_ac2,roi_cfg.stRoiParam[roi_idx].roi_sub_ac1_en);
		}
		else {
			roi_cfg.stRoiParam[roi_idx].roi_enable = 0;
		}
	}
	
	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_ROI, (void *)&roi_cfg) < 0)
		printk("set roi fail\n");
}
static void je_clear_roi_param(UINT32 kdrv_id)
{
	KDRV_VDOJPGE_ROI roi_cfg = {0};
	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_ROI, (void *)&roi_cfg) < 0)
		printk("clear roi fail\n");;
}

static void je_set_rotation_param(UINT32 kdrv_id, UINT32 rotation_type)
{
	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_ROTATION, (void *)&rotation_type) < 0)
		printk("set rotation fail\n");
}

static void je_set_rate_control_param(UINT32 kdrv_id, UINT32 user_type)
{
	KDRV_VDOJPGE_RC rate_control_cfg = {0};

	rate_control_cfg.enable = 1;
	rate_control_cfg.base_qp = 50;
	rate_control_cfg.vbr_mode = 0;
	rate_control_cfg.min_quality = 10;
	rate_control_cfg.max_quality = 100;
	rate_control_cfg.target_rate = 800000;
	rate_control_cfg.frame_rate_base = 10;
	rate_control_cfg.frame_rate_incr = 1;

	if (kdrv_videoenc_jpeg_set(kdrv_id, VDOJPGE_SET_JPEG_RC, (void *)&rate_control_cfg) < 0) {
		DBG_ERR("set rate control fail !! \n");
	}
}

static int jpeg_enc_set_sim_param(UINT32 job_id, int chn, KDRV_VDOJPGE_PARAM *param, JPEG_BUFFER *yuv_buf, JPEG_BUFFER *bs_buf, int width, int height, JPEG_SIM_PARAM *sim_param)
{
	//uintptr_t bs_offset = 3;
	memset(param, 0, sizeof(KDRV_VDOJPGE_PARAM));
	param->job_id = job_id;
	DBG_IND("param->job_id = 0x%x \n", param->job_id);
	//param->id = 0;
	param->y_addr = yuv_buf->addr_va;
	param->y_addr_pa = yuv_buf->addr_pa;
	if (sim_param->sce_en) {
		param->c_addr = yuv_buf->addr_va + ALIGN_16(width * 3 / 4) * height;
		param->uv_addr_pa = yuv_buf->addr_pa + ALIGN_16(width * 3 / 4) * height; //auto set in driver?
		param->y_line_offset = ALIGN_16(width * 3 / 4);
		param->c_line_offset = ALIGN_16(width * 3 / 4);
	} else {
		param->c_addr = yuv_buf->addr_va + width * height;
		param->uv_addr_pa = yuv_buf->addr_pa + width * height; //auto set in driver?
		param->y_line_offset = width;
		param->c_line_offset = width;
	}
	//param->src_ddr_id = 0;

#if RING_BUF_TEST
	param->bs_addr_pa[0] = bs_buf->addr_pa + bs_buf->size - first_bs_seg_size;
	param->bs_addr_va[0] = bs_buf->addr_va + bs_buf->size - first_bs_seg_size;
	param->bs_buf_size[0] = first_bs_seg_size;
	param->bs_addr_pa[1] = bs_buf->addr_pa;
	param->bs_addr_va[1] = bs_buf->addr_va;
	param->bs_buf_size[1] = bs_buf->size - first_bs_seg_size;
	DBG_DUMP("bs_addr_pa[0]=0x%lx, bs_buf_size[0]=0x%x, bs_addr_pa[1]=0x%lx, bs_buf_size[1]=0x%x \n", 
				(unsigned long)param->bs_addr_pa[0], param->bs_buf_size[0], (unsigned long)param->bs_addr_pa[1], param->bs_buf_size[1]);
#else
	param->bs_addr_pa[0] = bs_buf->addr_pa;
	param->bs_addr_va[0] = bs_buf->addr_va;// + bs_offset;
	param->bs_buf_size[0] = bs_buf->size;// - bs_offset;
	DBG_IND("bs addr 0x%lx, source y 0x%lx, c 0x%lx\n", (unsigned long)param->bs_addr_va[0], (unsigned long)param->y_addr, (unsigned long)param->c_addr);
#endif

	//DBG_DUMP("bs addr 0x%p, size %u\r\n", (void *)param->bs_addr_va[0], (unsigned int)param->bs_buf_size[0]);
	//param->bs_hdr_size; 							   ///< [==== RESERVED ===== ][r] header size
	//param->bs_ddr_id = 0;

	if (sim_param->sce_en)
		param->st_src_decompression.enable = 1;
	else
		param->st_src_decompression.enable = 0;

	param->quality = 40 + (JPEG_JOB_CHN(param->job_id) % 4) * 10; //40 ~ 70
	DBG_IND("param->quality = %d, JPEG_JOB_CHN(param->job_id) = %d \n", param->quality, JPEG_JOB_CHN(param->job_id));
	param->retstart_interval = sim_param->restart;
	//param->encode_time; 								///<  [==== RESERVED ===== ]HW encode time(unit: us)
	param->encode_width = width;
	param->encode_height = height;

	if (sim_param->pat_idx == 5)
		param->in_fmt = KDRV_JPEGYUV_FORMAT_422;
	else
		param->in_fmt = KDRV_JPEGYUV_FORMAT_420;

	//param->rotation_type = rot_type;
	//param->user_data;

#if 0 //JPEG_KDRV_SET_RC
	// RC
	if (sim_param->rc_enable) {
		param->quality = 50;
		param->vbr_mode = 0;
		param->min_quality = 10;
		param->max_quality = 100;
		param->target_rate = 800000;
		param->frame_rate = JPEG_RC_FPS*1000;
	}
#endif

	if (sim_param->rot_type & 0x10) {
		param->fmt_trans_en = 1;
	}
	else {
		param->fmt_trans_en = 0;
	}
	if (sim_param->restart & 0x10000) {
		param->enc_slice_en = 1;
		param->sec_Width = width;
		param->sec_Height = ((height/16 + JE_SLICE_NUM - 1)/JE_SLICE_NUM)*16;
		param->retstart_interval = param->sec_Width/16 * param->sec_Height/16;
		param->quality = 50;
		param->slice_cnt = jpg_enc_info[chn].slice_idx;
		if (param->slice_cnt == JE_SLICE_NUM - 1) {
			param->sec_Height = height - param->sec_Height*(JE_SLICE_NUM-1);
			param->slice_cnt |= 0x80000000;
		}
		// printk("slice[%d] %d x %d, restart = %d, cnt 0x%x\n", jpg_enc_info[chn].slice_idx, param->sec_Width, param->sec_Height, param->retstart_interval, param->slice_cnt);
		jpg_enc_info[chn].slice_idx++;
		if (jpg_enc_info[chn].slice_idx >= JE_SLICE_NUM)
			jpg_enc_info[chn].slice_idx = 0;
	}
	vos_cpu_dcache_sync(bs_buf->addr_va, bs_buf->size, VOS_DMA_BIDIRECTIONAL);
	DBG_IND("set param done\r\n");
	return 0;
}

//int jpeg_enc_main(char *yuv_fn, int width, int height, int frame_num, int sce_en, int rot_type, int osg_mask_en, int restart, int rc_enable, int block_mode_en)
int jpeg_enc_main(int pat_idx, char *out_name, int frame_num, int sce_en, int rot_type, int osg_mask_en, int restart, int rc_enable, int block_mode_en)
{
	KDRV_VDOJPGE_PARAM je_param;
	KDRV_CALLBACK_FUNC jpeg_cb_func = {0};
	KDRV_CALLBACK_FUNC *p_cb_func;
	JPEG_SIM_PARAM sim_param;

	//UINT32 job_id = 0;
	UINT32 osg_type = 0, mask_type = 0, gray_type = 0, user_type = 0, roi_type = 0;
	UINT32 status = KDRV_VDOJPGE_OK;
	int width, height;
	int frame_size, bs_size;
	int chn, i, cnt;
	unsigned int sim_start = 0, sim_end = 0;

	if (pat_idx >= YUV_PAT_NUM) {
		printk("yuv pattern index (%d) over max pattern number (%d)\n", pat_idx, YUV_PAT_NUM);
		goto pat_fail;
	}
	width = je_yuv_info[pat_idx].width;
	height = je_yuv_info[pat_idx].height;
	//if (frame_num > je_yuv_info[pat_idx].max_frm_num)
	//	frame_num = je_yuv_info[pat_idx].max_frm_num;
	if (pat_idx == 5)
		frame_size = width * height * 2; //yuv422
	else
		frame_size = width * height * 3 / 2; //yuv420
	
	bs_size = width * height / 2;

	if (block_mode_en > 0) {
		p_cb_func = NULL;
	}
	else {
		jpeg_cb_func.callback = jpeg_sim_callback_enc;
		p_cb_func = &jpeg_cb_func;

		//ipc follow xvr
		if (kdrv_videoenc_jpeg_set(0, VDOJPGE_SET_JPEG_CB_FUNC, (void *)p_cb_func) < 0)
			DBG_ERR("set VDOJPGE_SET_JPEG_CB_FUNC fail !! \n");
	}

	// 0. init: open file, init buffer
	if (je_open_file(je_yuv_info[pat_idx].yuv_fn, out_name) < 0)
		goto exit_main;

	if (jpeg_emu_allocate_buffer(&je_yuv_buf, frame_size) < 0) {
		DBG_ERR("allocate yuv buffer error\r\n");
		goto exit_main;
	}

	for (chn = 0; chn < TEST_CHN; chn++) {
		if (jpeg_emu_allocate_buffer(&jpg_enc_info[chn].enc_bs_buf, bs_size) < 0) {
			DBG_ERR("{chn%d} allocate bs buffer error\r\n", chn);
			goto exit_main;
		}
	}

	// osg
	#if TEST_OSG_MASK
	osg_type = osg_mask_en & 0xF;
	mask_type = (osg_mask_en >> 4) & 0xF;
	gray_type = (osg_mask_en >> 8) & 0xF;
	user_type = (osg_mask_en >> 0xC) & 0xFF;
	roi_type = (osg_mask_en >> 0x14) & 0xF;
	
	if (osg_type) {
		int idx;
		for (idx = 0; idx < OSG_PAT_NUM; idx++) {
			jpeg_emu_prepare_osg_pattern(&je_osg_buf[idx], &jpeg_osg_pat[idx]);
		}
	}
	#endif

	for (chn = 0; chn < TEST_CHN; chn++) {
		jpg_enc_info[chn].kdrv_id = KDRV_DEV_ID(0, KDRV_VIDEOCDC_ENGINE_JPEG, /*KDRV_VDOENC_ID_1 +*/ chn);
		DBG_DUMP("jpg_enc_info[%d].kdrv_id = 0x%x \n", chn, jpg_enc_info[chn].kdrv_id);

		#if TEST_OSG_MASK
		// set OSG
		if (osg_type)
			je_set_osg_param(jpg_enc_info[chn].kdrv_id, width, height, osg_type);
		if (mask_type)
			je_set_mask_param(jpg_enc_info[chn].kdrv_id, width, height, mask_type);
		#endif
		// color to gray
		if (gray_type) {
			je_set_gray_param(jpg_enc_info[chn].kdrv_id, gray_type);
		}
		// user data
		if (user_type) {
			je_set_user_data_param(jpg_enc_info[chn].kdrv_id, user_type);
		}
		// roi
		if (roi_type) {
			je_set_roi_param(jpg_enc_info[chn].kdrv_id, roi_type);
		}
		je_set_rotation_param(jpg_enc_info[chn].kdrv_id, (UINT32)(rot_type & 0xF));
		jpg_enc_info[chn].byte_sum = 0;
		jpg_enc_info[chn].frm_num = 0;
		// rate control
		if (rc_enable) {
			je_set_rate_control_param(jpg_enc_info[chn].kdrv_id, user_type);
		}
		/*
		if (frame_num >= 30)
			jpeg_sim_set_rc_param(jpg_enc_info[chn].kdrv_id, 1, 1);
		else
			jpeg_sim_set_rc_param(jpg_enc_info[chn].kdrv_id, 0, 0);
		*/
		jpg_enc_info[chn].slice_idx = 0;
	}

	memset(&sim_param, 0, sizeof(JPEG_SIM_PARAM));
	sim_param.pat_idx = pat_idx;
	sim_param.sce_en = sce_en;
	sim_param.rot_type = rot_type;
	sim_param.restart = restart;
	// sim_param.rc_enable = rc_enable;

	memset(&je_param, 0, sizeof(KDRV_VDOJPGE_PARAM));
	for (i = 0; i < frame_num; i++) {
		printk("== encode frame %d ==\r\n", i);
		// 1. read YUV
		if (i < je_yuv_info[pat_idx].max_frm_num)
			je_read_src_yuv(je_yuv_buf.addr_va, frame_size);

		// 2. set JPEG kdrv
		for (chn = 0; chn < TEST_CHN; chn++) {
			//if (1 == chn)
			//	jpeg_enc_set_sim_param((chn << 8) + job_id, &je_param, &je_yuv_buf, &jpg_enc_info[chn].enc_bs_buf, width / 2, height / 2, sce_en, rot_type, restart);
			//else
			jpg_enc_info[chn].b_jpeg_enc_cb = FALSE;
			jpeg_enc_set_sim_param(JPEG_JOB_ID(chn, i), chn, &je_param, &je_yuv_buf, &jpg_enc_info[chn].enc_bs_buf, width, height, &sim_param);
			if (block_mode_en != 0)
				je_param.engine_idx = block_mode_en - 1; //mapping
			vos_perf_mark((VOS_TICK *)(&sim_start));
			if (kdrv_videoenc_jpeg_trigger(jpg_enc_info[chn].kdrv_id, &je_param, p_cb_func, NULL) < 0) {
				DBG_ERR("{chn%d} trigger error !!! \n\n", chn);
			}

			//stop_job
			// if (kdrv_videoenc_jpeg_set(jpg_enc_info[chn].kdrv_id, VDOJPGE_SET_JPEG_STOP_JOB, NULL) < 0)
			// 	printk("set kdrv_videoenc_jpeg_set error !! ");

			vos_perf_mark((VOS_TICK *)(&sim_end));
			DBG_IND("vos_perf_duration[%d] = %d, encode_time = (%u T) \n", i, (unsigned int)vos_perf_duration(sim_start, sim_end), (unsigned int)je_param.encode_time);
			if (p_cb_func == NULL) {
				//block mode set callback ok
				jpg_enc_info[chn].b_jpeg_enc_cb = TRUE;
			}
		}
		//printk("encode trigger done\r\n");
		
		// 3. wait frame done
		cnt = 0;
		for (chn = 0; chn < TEST_CHN; chn++) {
			while (jpg_enc_info[chn].b_jpeg_enc_cb == FALSE) {
				msleep(10);
				cnt++;
				if (cnt > 500) {
					status = KDRV_VDOJPGE_FAIL;
					DBG_ERR("encode timeout!!\n");
					goto exit_main;
				}
			}
		}
		//printk("wait cb done\n");

		// 4. save bitstream
		for (chn = 0; chn < TEST_CHN; chn++) {
		#if JPEG_ENC_SAVE_BS
			if (p_cb_func == NULL) {
				//printk("bs addr 0x%lx, bs_buf_size[0] %d, bs_offset = %d\n", (unsigned long)jpg_enc_info[chn].enc_bs_buf.addr_va, (int)je_param.bs_buf_size[0], (int)je_param.bs_offset);
				vos_cpu_dcache_sync(jpg_enc_info[chn].enc_bs_buf.addr_va, je_param.bs_buf_size[0], VOS_DMA_FROM_DEVICE);
				je_write_bs(chn, jpg_enc_info[chn].enc_bs_buf.addr_va + je_param.bs_offset, je_param.bs_buf_size[0]);
				if (rc_enable)
					evaluate_bitrate(&jpg_enc_info[chn], je_param.bs_buf_size[0]);
			}
			else {
				if (jpg_enc_info[chn].b_jpeg_enc_fail == 1) {
					DBG_WRN("enc callback fail !! \n");
					continue;
				}
#if RING_BUF_TEST
				vos_cpu_dcache_sync(jpg_enc_info[chn].enc_bs_buf.addr_va + jpg_enc_info[chn].enc_bs_offset + jpg_enc_info[chn].enc_bs_buf.size - first_bs_seg_size, jpg_enc_info[chn].first_bs_len, VOS_DMA_FROM_DEVICE);
				je_write_bs(chn, jpg_enc_info[chn].enc_bs_buf.addr_va + jpg_enc_info[chn].enc_bs_offset + jpg_enc_info[chn].enc_bs_buf.size - first_bs_seg_size, jpg_enc_info[chn].first_bs_len);
				vos_cpu_dcache_sync(jpg_enc_info[chn].enc_bs_buf.addr_va, jpg_enc_info[chn].enc_bs_len - jpg_enc_info[chn].first_bs_len, VOS_DMA_FROM_DEVICE);
				je_write_bs(chn, jpg_enc_info[chn].enc_bs_buf.addr_va, jpg_enc_info[chn].enc_bs_len - jpg_enc_info[chn].first_bs_len);
#else
				//printk("bs addr 0x%lx, enc_bs_len %d, bs_offset = %d\n", (unsigned long)jpg_enc_info[chn].enc_bs_buf.addr_va, (int)jpg_enc_info[chn].enc_bs_len, (int)jpg_enc_info[chn].enc_bs_offset);
				vos_cpu_dcache_sync(jpg_enc_info[chn].enc_bs_buf.addr_va, jpg_enc_info[chn].enc_bs_len, VOS_DMA_FROM_DEVICE);
				je_write_bs(chn, jpg_enc_info[chn].enc_bs_buf.addr_va + jpg_enc_info[chn].enc_bs_offset, jpg_enc_info[chn].enc_bs_len);
#endif
				if (rc_enable)
					evaluate_bitrate(&jpg_enc_info[chn], jpg_enc_info[chn].enc_bs_len);
			}
		#endif
		}
		//job_id++;
	}

exit_main:
	// 5. clear
	printk("exit main \n\n");
	je_emu_close_file();
	jpeg_emu_free_buffer(&je_yuv_buf);
	#if TEST_OSG_MASK
	if (osg_type) {
		int idx;
		for (idx = 0; idx < OSG_PAT_NUM; idx++)
			jpeg_emu_free_buffer(&je_osg_buf[idx]);
	}
	if (user_type) {
		jpeg_emu_free_buffer(&je_user_buf);
	}
	for (chn = 0; chn < TEST_CHN; chn++) {
		if (osg_type)
			je_clear_osg_param(jpg_enc_info[chn].kdrv_id);
		if (mask_type)
			je_clear_mask_param(jpg_enc_info[chn].kdrv_id);
		if (gray_type)
			je_clear_gray_param(jpg_enc_info[chn].kdrv_id);
		if (user_type)
			je_clear_user_data_param(jpg_enc_info[chn].kdrv_id);
		if (roi_type)
			je_clear_roi_param(jpg_enc_info[chn].kdrv_id);
	}
	#endif
	for (chn = 0; chn < TEST_CHN; chn++) {
		jpeg_emu_free_buffer(&jpg_enc_info[chn].enc_bs_buf);
		//kdrv_videoenc_jpeg_close(jpg_enc_info[chn].kdrv_id);
	}
pat_fail:
	return 0;
}

int jpeg_enc_main_perf(int pat_idx, char *out_name, int frame_num, int sce_en, int rot_type, int osg_mask_en, int restart, int rc_enable, int block_mode_en)
{
	KDRV_VDOJPGE_PARAM je_param;
	KDRV_CALLBACK_FUNC jpeg_cb_func = {0};
	KDRV_CALLBACK_FUNC *p_cb_func;
	JPEG_SIM_PARAM sim_param;

	//UINT32 job_id = 0;
	UINT32 osg_type = 0, mask_type = 0, gray_type = 0;
	UINT32 status = KDRV_VDOJPGE_OK;
	int width, height;
	int frame_size, bs_size;
	int chn, i, j, cnt;
	unsigned int sim_start = 0, sim_end = 0;

	if (pat_idx >= YUV_PAT_NUM) {
		printk("yuv pattern index (%d) over max pattern number (%d)\n", pat_idx, YUV_PAT_NUM);
		goto pat_fail;
	}
	width = je_yuv_info[pat_idx].width;
	height = je_yuv_info[pat_idx].height;
	//if (frame_num > je_yuv_info[pat_idx].max_frm_num)
	//	frame_num = je_yuv_info[pat_idx].max_frm_num;
	frame_size = width * height * 3 / 2;
	bs_size = width * height / 2;

	if (block_mode_en > 0) {
		p_cb_func = NULL;
	}
	else {
		jpeg_cb_func.callback = jpeg_sim_callback_enc;
		p_cb_func = &jpeg_cb_func;

		//ipc follow xvr
		if (kdrv_videoenc_jpeg_set(0, VDOJPGE_SET_JPEG_CB_FUNC, (void *)p_cb_func) < 0)
			DBG_ERR("set VDOJPGE_SET_JPEG_CB_FUNC fail !! \n");
	}

	// 0. init: open file, init buffer
	if (je_open_file(je_yuv_info[pat_idx].yuv_fn, out_name) < 0)
		goto exit_main;

	if (jpeg_emu_allocate_buffer(&je_yuv_buf, frame_size) < 0) {
		DBG_ERR("allocate yuv buffer error\r\n");
		goto exit_main;
	}

	for (chn = 0; chn < TEST_CHN; chn++) {
		if (jpeg_emu_allocate_buffer(&jpg_enc_info[chn].enc_bs_buf, bs_size) < 0) {
			DBG_ERR("{chn%d} allocate bs buffer error\r\n", chn);
			goto exit_main;
		}
	}

	// osg
	#if TEST_OSG_MASK
	osg_type = osg_mask_en&0xF;
	mask_type = (osg_mask_en>>4)&0xF;
	gray_type = (osg_mask_en>>8)&0xF;
	if (osg_type) {
		int idx;
		for (idx = 0; idx < OSG_PAT_NUM; idx++) {
			jpeg_emu_prepare_osg_pattern(&je_osg_buf[idx], &jpeg_osg_pat[idx]);
		}
	}
	#endif

	for (chn = 0; chn < TEST_CHN; chn++) {
		jpg_enc_info[chn].kdrv_id = KDRV_DEV_ID(0, KDRV_VIDEOCDC_ENGINE_JPEG, /*KDRV_VDOENC_ID_1 +*/ chn);
		DBG_DUMP("jpg_enc_info[%d].kdrv_id = 0x%x \n", chn, jpg_enc_info[chn].kdrv_id);

		#if TEST_OSG_MASK
		// set OSG
		if (osg_type)
			je_set_osg_param(jpg_enc_info[chn].kdrv_id, width, height, osg_type);
		if (mask_type)
			je_set_mask_param(jpg_enc_info[chn].kdrv_id, width, height, mask_type);
		#endif
		// color to gray
		if (gray_type) {
			je_set_gray_param(jpg_enc_info[chn].kdrv_id, gray_type);
		}
		je_set_rotation_param(jpg_enc_info[chn].kdrv_id, (UINT32)(rot_type & 0xF));
		jpg_enc_info[chn].byte_sum = 0;
		jpg_enc_info[chn].frm_num = 0;
		jpg_enc_info[chn].tri_cnt = 0;
		jpg_enc_info[chn].cb_cnt = 0;
		jpg_enc_info[chn].slice_idx = 0;
		/*
		if (frame_num >= 30)
			jpeg_sim_set_rc_param(jpg_enc_info[chn].kdrv_id, 1, 1);
		else
			jpeg_sim_set_rc_param(jpg_enc_info[chn].kdrv_id, 0, 0);
		*/
	}

	// 1. read YUV
	je_read_src_yuv(je_yuv_buf.addr_va, frame_size);

	memset(&sim_param, 0, sizeof(JPEG_SIM_PARAM));
	sim_param.pat_idx = pat_idx;
	sim_param.sce_en = sce_en;
	sim_param.rot_type = rot_type;
	sim_param.restart = restart;
	// sim_param.rc_enable = rc_enable;

	memset(&je_param, 0, sizeof(KDRV_VDOJPGE_PARAM));
	je_frame_cnt = 0;
	for (i = 0; i < frame_num; i++) {
		if (0 == sim_start) {
			vos_perf_mark((VOS_TICK *)(&sim_start));
		}
		else {
			vos_perf_mark((VOS_TICK *)(&sim_end));
			if (sim_start < sim_end) {
				if (vos_perf_duration(sim_start, sim_end) >= JE_MEASURE_PERIOD*1000000) {
					printk("performance = %d fps\n", je_frame_cnt/JE_MEASURE_PERIOD);
					je_frame_cnt = 0;
					vos_perf_mark((VOS_TICK *)(&sim_start));
				}
			}
			else {
				vos_perf_mark((VOS_TICK *)(&sim_start));
			}
		}
		// 2. set JPEG kdrv
		for (j = 0; j < JE_BURST_NUM; j++) {
			for (chn = 0; chn < TEST_CHN; chn++) {
				//if (1 == chn)
				//	jpeg_enc_set_sim_param((chn << 8) + job_id, &je_param, &je_yuv_buf, &jpg_enc_info[chn].enc_bs_buf, width / 2, height / 2, sce_en, rot_type, restart);
				//else
				jpg_enc_info[chn].b_jpeg_enc_cb = FALSE;
				jpeg_enc_set_sim_param(JPEG_JOB_ID(chn, i*JE_BURST_NUM+j), chn, &je_param, &je_yuv_buf, &jpg_enc_info[chn].enc_bs_buf, width, height, &sim_param);
				je_param.engine_idx = block_mode_en - 1; //mapping
				if (kdrv_videoenc_jpeg_trigger(jpg_enc_info[chn].kdrv_id, &je_param, p_cb_func, NULL) < 0) {
					DBG_ERR("{chn%d} trigger error !!! \n\n", chn);
				}
				jpg_enc_info[chn].tri_cnt++;
				if (p_cb_func == NULL)
					jpg_enc_info[chn].b_jpeg_enc_cb = TRUE;
			}
		}
		//printk("encode trigger done\r\n");
		
		// 3. wait frame done
		cnt = 0;
		for (chn = 0; chn < TEST_CHN; chn++) {
			while (jpg_enc_info[chn].cb_cnt < jpg_enc_info[chn].tri_cnt-2) {
				msleep(10);
				cnt++;
				if (cnt > 500) {
					status = KDRV_VDOJPGE_FAIL;
					DBG_ERR("encode timeout!!\n");
					goto exit_main;
				}
			}
		}
	}

	for (chn = 0; chn < TEST_CHN; chn++) {
		while (jpg_enc_info[chn].cb_cnt < jpg_enc_info[chn].tri_cnt) {
			msleep(10);
			cnt++;
			if (cnt > 500) {
				status = KDRV_VDOJPGE_FAIL;
				DBG_ERR("encode timeout!!\n");
				goto exit_main;
			}
		}
	}

	printk("cb cnt = %d, tri cnt = %d\n", jpg_enc_info[0].cb_cnt, jpg_enc_info[0].tri_cnt);

exit_main:
	// 5. clear
	printk("exit main\n");
	je_emu_close_file();
	jpeg_emu_free_buffer(&je_yuv_buf);
	printk("free yuv done\n");
	#if TEST_OSG_MASK
	if (osg_type) {
		int idx;
		for (idx = 0; idx < OSG_PAT_NUM; idx++)
			jpeg_emu_free_buffer(&je_osg_buf[idx]);
	}
	for (chn = 0; chn < TEST_CHN; chn++) {
		if (osg_type)
			je_clear_osg_param(jpg_enc_info[chn].kdrv_id);
		if (mask_type)
			je_clear_mask_param(jpg_enc_info[chn].kdrv_id);
		if (gray_type)
			je_clear_gray_param(jpg_enc_info[chn].kdrv_id);
	}
	#endif
	for (chn = 0; chn < TEST_CHN; chn++) {
		jpeg_emu_free_buffer(&jpg_enc_info[chn].enc_bs_buf);
		//kdrv_videoenc_jpeg_close(jpg_enc_info[chn].kdrv_id);
	}
pat_fail:
	return 0;
}

#endif /* __KERNEL__ */

