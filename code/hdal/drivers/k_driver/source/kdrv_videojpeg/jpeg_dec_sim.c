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

#include "jpeg_dbg.h"
#include "kdrv_videojpeg/kdrv_videodec_jpeg.h"

#define TEST_CHN		4

#define JD_BURST_NUM		8
#define JD_MEASURE_PERIOD	5

#define ALIGN_4(a)		((((a)+3)>>2)<<2)
#define ALIGN_16(a)		((((a)+15)>>4)<<4)
#define ALIGN_64(a)		((((a)+63)>>6)<<6)

#define JPEG_JOB_ID(chn, frm_id)	(((chn)<<8)|((frm_id)&0xFF))
#define JPEG_JOB_CHN(job_id)		((job_id)>>8)
#define JPEG_JOB_FRM_ID(job_id)		((job_id)&0xFF)

// #define DBG_DUMP(fmt, args...)	printk(fmt, ## args)
// #define DBG_ERR(fmt, args...)	printk(fmt, ## args)

typedef struct {
	uintptr_t ori_addr;
	uintptr_t addr_va;
	uintptr_t addr_pa;
	UINT32 size;
	VOS_MEM_CMA_HDL cma_hdl;
} JPEG_BUFFER;

typedef struct {
	char bs_fn[0x40];
	UINT32 width;
	UINT32 height;
	UINT32 max_frm_num;
} JPEG_BS_INFO;

#define BS_PAT_NUM	8

static JPEG_BS_INFO jd_bs_info[BS_PAT_NUM] = {
	{"/mnt/sd/dec.mjpg", 352, 288, 10},
	{"/mnt/sd/99_uv_pack.jpg", 1920, 1088, 10},
	{"/mnt/sd/progressive.jpg", 800, 600, 1},
	{"/mnt/sd/IVOT_N12081_CO-146.jpg", 1280, 720, 1}, //fmt422 to 420 (sw dec)
	{"/mnt/sd/logo_720x576.jpg", 720, 576, 1}, //64x alignment
	{"/mnt/sd/D1.mjpg", 720, 480, 10},
	{"/mnt/sd/video_bs_1920_1080_jpeg.dat", 1920, 1080, 1},
	{"/mnt/sd/image4.jpg", 704, 540, 1},
};

/******************** decode ********************/
static int bs_len0[10] = {17163, 17028, 17157, 17323, 17380, 17388, 17436, 17406, 17366, 17418};
static int bs_len1[10] = {84569, 86344, 87380, 88069, 88561, 89440, 90489, 91738, 92343, 93603};
static int bs_len2[2] = {82469, 82469};
static int bs_len3[1] = {156930};
static int bs_len4[1] = {50126};
static int bs_len5[10] = {38272, 38175, 38163, 38262, 38245, 38338, 38261, 38339, 38264, 38258};
static int bs_len6[1] = {240559};
static int bs_len7[1] = {19144};

typedef struct {
	unsigned int kdrv_id;
	struct file *dec_yuv_fp;
	JPEG_BUFFER dec_yuv_buf;
	unsigned int dec_img_width;
	unsigned int dec_img_height;
	unsigned int dec_bg_width;
	unsigned int dec_bg_height;
	BOOL b_jpeg_dec_cb;
	BOOL b_jpeg_dec_fail;
	unsigned int tri_cnt;
	unsigned int cb_cnt;
} JPEG_DEC_INFO;

static JPEG_DEC_INFO jpg_dec_info[TEST_CHN] = {0};
static struct file *jd_bs_fp = NULL;
static JPEG_BUFFER jd_bs_buf = {0};
static unsigned int jd_frame_cnt = 0;

//extern INT32 kdrv_videodec_jpeg_trigger(UINT32 id, KDRV_VDOJPGD_PARAM *p_dec_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);

INT32 jpeg_sim_callback_dec(VOID *callback_info, VOID *user_data)
{
	KDRV_VDOJPGD_RESULT *jpeg_result = (KDRV_VDOJPGD_RESULT *)callback_info;
	int chn;

	DBG_IND("jpeg_sim_callback_dec: job 0x%x, status %d \n", jpeg_result->job_id, jpeg_result->status);
	chn = JPEG_JOB_CHN(jpeg_result->job_id);
	if (KDRV_VDOJPGD_OK == jpeg_result->status) {
		jpg_dec_info[chn].dec_img_width = jpeg_result->img_width;
		jpg_dec_info[chn].dec_img_height = jpeg_result->img_height;
		jpg_dec_info[chn].dec_bg_width = jpeg_result->bg_width;
		jpg_dec_info[chn].dec_bg_height = jpeg_result->bg_height;
		jpg_dec_info[chn].b_jpeg_dec_fail = 0;
		DBG_IND("jpeg_sim_callback_dec: chn = %d, size = %ux%u (%ux%u) \n", chn, jpg_dec_info[chn].dec_img_width, jpg_dec_info[chn].dec_img_height, jpg_dec_info[chn].dec_bg_width, jpg_dec_info[chn].dec_bg_height);
	}
	else {
		jpg_dec_info[chn].b_jpeg_dec_fail = 1;
	}
	jpg_dec_info[chn].b_jpeg_dec_cb = TRUE;
	jpg_dec_info[chn].cb_cnt++;
	jd_frame_cnt++;
	return 0;
}
/******************** decode ********************/

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
#endif
	DBG_IND("allocate buffer pa 0x%lx, va 0x%lx, size %d\r\n", (unsigned long)buf->addr_pa, (unsigned long)buf->addr_va, buf->size);
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

static int jd_emu_open_file(char *bs_fn, char *out_name)
{
	int ret = 0;
	int chn;
	char fn[0x80];

	for (chn = 0; chn < TEST_CHN; chn++) {
		sprintf(fn, "/mnt/sd/chn%d_dec_%s.yuv", chn, out_name);
		jpg_dec_info[chn].dec_yuv_fp = filp_open(fn, O_WRONLY | O_CREAT, 0777);
		if (IS_ERR(jpg_dec_info[chn].dec_yuv_fp)) {
			jpg_dec_info[chn].dec_yuv_fp = NULL;
			DBG_ERR("Error to open %s\n", fn);
			ret = -1;
			goto exit_open;
		}
		DBG_DUMP("open yuv file %s done\r\n", fn);
	}

	jd_bs_fp = filp_open(bs_fn, O_RDONLY, 0777);
	if (IS_ERR(jd_bs_fp)) {
		jd_bs_fp = NULL;
		DBG_ERR("Error to open %s\n", bs_fn);
		ret = -1;
		goto exit_open;
	}
	DBG_DUMP("open bs file %s done\r\n", bs_fn);

exit_open:
	return ret;
}

static void jd_emu_close_file(void)
{
	int chn;
	for (chn = 0; chn < TEST_CHN; chn++) {
		if (jpg_dec_info[chn].dec_yuv_fp)
			filp_close(jpg_dec_info[chn].dec_yuv_fp, NULL);
	}
	if (jd_bs_fp)
		filp_close(jd_bs_fp, NULL);
}

static int jd_read_bs(uintptr_t addr, int len)
{
	int ret = 0;
	if (jd_bs_fp) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
		ret = vfs_read(jd_bs_fp, (void *)addr, len, &jd_bs_fp->f_pos);
#else
		ret = kernel_read(jd_bs_fp, (void *)addr, (size_t)len, &jd_bs_fp->f_pos);
#endif
	}
	vos_cpu_dcache_sync(addr, len, VOS_DMA_BIDIRECTIONAL);
	DBG_IND("read bs 0x%lx size %d done\r\n", (unsigned long)addr, len);
	//dump_buffer(addr, 0x40);
	//dump_buffer(addr+0x240, 0x40);
	return ret;
}

static int jd_write_yuv(int chn, uintptr_t addr, int frame_size)
{
	int ret = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	ret = vfs_write(jpg_dec_info[chn].dec_yuv_fp, (unsigned char *)addr, frame_size, &jpg_dec_info[chn].dec_yuv_fp->f_pos);
#else
	ret = kernel_write(jpg_dec_info[chn].dec_yuv_fp, (unsigned char *)addr, frame_size, &jpg_dec_info[chn].dec_yuv_fp->f_pos);
#endif
	DBG_IND("write yuv 0x%lx size %d done\r\n", (unsigned long)addr, frame_size);
	return ret;
}

static int jpeg_dec_set_param(UINT32 job_id, KDRV_VDOJPGD_PARAM *param, JPEG_BUFFER *yuv_buf, JPEG_BUFFER *bs_buf, int bs_len, int y_size)
{
	//param->id = 0;
	param->jpeg_hdr_addr = bs_buf->addr_va;
	param->bs_addr_va[0] = bs_buf->addr_va;
	param->bs_addr_pa[0] = bs_buf->addr_pa;
	param->bs_buf_size[0] = bs_len;

	param->y_addr_va = yuv_buf->addr_va; //for sw decode
	param->uv_addr_va = yuv_buf->addr_va + y_size;
	param->y_addr_pa = yuv_buf->addr_pa;
	param->uv_addr_pa = param->y_addr_pa + y_size; //auto set in driver?
	
	param->job_id = job_id;
	param->raw_size = yuv_buf->size;
	
	return 0;
}

//int jpeg_dec_main(char *bs_fn, int pat_idx, int width, int height, int frame_num, int block_mode_en)
int jpeg_dec_main(int pat_idx, char *out_name, int frame_num, int block_mode_en)
{
	KDRV_VDOJPGD_PARAM jd_param;
	KDRV_CALLBACK_FUNC jpeg_cb_func = {0};
	KDRV_CALLBACK_FUNC *p_cb_func;
	int width, height;
	int bs_size;
	//UINT32 kdrv_id;
	UINT32 status = KDRV_VDOJPGD_OK;
	int frame_size;
	int len = 0, ret = 0;
	int chn, i, cnt;

	if (pat_idx >= BS_PAT_NUM) {
		printk("bs pattern index (%d) over max pattern number (%d)\n", pat_idx, BS_PAT_NUM);
		ret = -1;
		goto pat_fail;
	}
	width = jd_bs_info[pat_idx].width;
	height = jd_bs_info[pat_idx].height;
	if (frame_num > jd_bs_info[pat_idx].max_frm_num)
		frame_num = jd_bs_info[pat_idx].max_frm_num;
	bs_size = width * height * 2;
	if ((3 == pat_idx) || (6 == pat_idx))
		frame_size = ALIGN_64(width) * ALIGN_16(height) * 2;
	else
		frame_size = ALIGN_64(width) * ALIGN_16(height) * 3 / 2;

	jpeg_cb_func.callback = jpeg_sim_callback_dec;
	if (block_mode_en > 0)
		p_cb_func = NULL;
	else
		p_cb_func = &jpeg_cb_func;

	//ipc follow xvr
	kdrv_videodec_jpeg_set(0, VDODEC_SET_JPEG_CB_FUNC, (void*)p_cb_func);

	// 1. open file
	if (jd_emu_open_file(jd_bs_info[pat_idx].bs_fn, out_name) < 0) {
		DBG_ERR("open fail error\r\n");
		goto exit_main;
	}

	// 2. allocate buffer
	if (jpeg_emu_allocate_buffer(&jd_bs_buf, bs_size) < 0) {
		DBG_ERR("allocate bs buffer error\r\n");
		goto exit_main;
	}

	//cb_fun.callback = kdrv_jpeg_dec_cb;
	//kdrv_videodec_jpeg_set(0, VDODEC_SET_CB_FUNC, (void *)(&cb_fun));

	memset(&jd_param, 0, sizeof(KDRV_VDOJPGD_PARAM));

	for (chn = 0; chn < TEST_CHN; chn++) {
		jpg_dec_info[chn].kdrv_id = KDRV_DEV_ID(0, KDRV_VIDEOCDC_ENGINE_JPEG, chn);
		if (jpeg_emu_allocate_buffer(&jpg_dec_info[chn].dec_yuv_buf, frame_size) < 0) {
			DBG_ERR("allocate yuv buffer error\r\n");
			goto exit_main;
		}
	}

	// 3. trigger job
	for (i = 0; i < frame_num; i++) {
		printk("== decode frame %d ==\r\n", i);
		if (0 == pat_idx)
			len = bs_len0[i];
		else if (1 == pat_idx)
			len = bs_len1[i];
		else if (2 == pat_idx)
			len = bs_len2[i];
		else if (3 == pat_idx)
			len = bs_len3[i];
		else if (4 == pat_idx)
			len = bs_len4[i];
		else if (5 == pat_idx)
			len = bs_len5[i];
		else if (6 == pat_idx)
			len = bs_len6[i];
		else if (7 == pat_idx)
			len = bs_len7[i];
		//else
		//	len = bs_len_single[pat_idx - 2];

		// (1) read BS
		jd_read_bs(jd_bs_buf.addr_va, len);

		// (2) set JPEG kdrv
		for (chn = 0; chn < TEST_CHN; chn++) {
			jpg_dec_info[chn].b_jpeg_dec_cb = FALSE;
			jpeg_dec_set_param(JPEG_JOB_ID(chn, i), &jd_param, &jpg_dec_info[chn].dec_yuv_buf, &jd_bs_buf, len, ALIGN_64(width) * ALIGN_16(height));
			if (block_mode_en != 0)
				jd_param.engine_idx = block_mode_en - 1;
			if (kdrv_videodec_jpeg_trigger(jpg_dec_info[chn].kdrv_id, &jd_param, NULL, NULL) < 0) {
				DBG_ERR("{chn%d} decode trigger error !!! \n\n", chn);
			}
			// kdrv_videodec_jpeg_set(jpg_dec_info[chn].kdrv_id, VDODEC_SET_JPEG_STOP, NULL);
			if (p_cb_func == NULL) {
				//block mode set callback ok
				jpg_dec_info[chn].b_jpeg_dec_cb = TRUE;
			}
		}
		// printk("decode trigger done\r\n");

		// (3) wait frame done
		if (p_cb_func != NULL) {
			cnt = 0;
			for (chn = 0; chn < TEST_CHN; chn++) {
				while (jpg_dec_info[chn].b_jpeg_dec_cb == FALSE) {
					msleep(10);
					cnt++;
					if (cnt > 500) {
						status = KDRV_VDOJPGD_FAIL;
						DBG_ERR("decode timeout!!\n");
						goto exit_main;
					}
				}
			}
			// printk("wait cb done\n");
		}

		// (4) save output
		for (chn = 0; chn < TEST_CHN; chn++) {
			if (p_cb_func != NULL) {
				if (jpg_dec_info[chn].b_jpeg_dec_fail == 1) {
					DBG_WRN("dec callback fail !! \n");
					continue;
				}
				// if ((3 == pat_idx) || (6 == pat_idx))
				// 	jd_write_yuv(chn, jpg_dec_info[chn].dec_yuv_buf.addr_va, jpg_dec_info[chn].dec_bg_width * jpg_dec_info[chn].dec_bg_height * 2);
				// else
					jd_write_yuv(chn, jpg_dec_info[chn].dec_yuv_buf.addr_va, jpg_dec_info[chn].dec_bg_width * jpg_dec_info[chn].dec_bg_height * 3 / 2);
			}
			else
				jd_write_yuv(chn, jpg_dec_info[chn].dec_yuv_buf.addr_va, ALIGN_64(jd_param.uiWidth) * ALIGN_16(jd_param.uiHeight) * 3 / 2);
		}
		//dec_bs_offset += 4;
	}

	// 4. clear all
exit_main:
	jd_emu_close_file();
	jpeg_emu_free_buffer(&jd_bs_buf);
	for (chn = 0; chn < TEST_CHN; chn++) {
		jpeg_emu_free_buffer(&jpg_dec_info[chn].dec_yuv_buf);
		//kdrv_videodec_jpeg_close(jpg_dec_info[chn].kdrv_id);
	}
pat_fail:
	return ret;
}

int jpeg_dec_main_perf(int pat_idx, char *out_name, int frame_num, int block_mode_en)
{
	KDRV_VDOJPGD_PARAM jd_param;
	KDRV_CALLBACK_FUNC jpeg_cb_func = {0};
	KDRV_CALLBACK_FUNC *p_cb_func;
	int width, height;
	int bs_size;
	//UINT32 kdrv_id;
	UINT32 status = KDRV_VDOJPGD_OK;
	int frame_size;
	int len = 0, ret = 0;
	int chn, i, j, cnt;
	unsigned int sim_start = 0, sim_end = 0;

	if (pat_idx >= BS_PAT_NUM) {
		printk("bs pattern index (%d) over max pattern number (%d)\n", pat_idx, BS_PAT_NUM);
		ret = -1;
		goto pat_fail;
	}
	width = jd_bs_info[pat_idx].width;
	height = jd_bs_info[pat_idx].height;
	//if (frame_num > jd_bs_info[pat_idx].max_frm_num)
	//	frame_num = jd_bs_info[pat_idx].max_frm_num;
	bs_size = width * height * 2;
	frame_size = ALIGN_64(width) * ALIGN_16(height) * 3 / 2;

	jpeg_cb_func.callback = jpeg_sim_callback_dec;
	if (block_mode_en > 0)
		p_cb_func = NULL;
	else
		p_cb_func = &jpeg_cb_func;

	//ipc follow xvr
	kdrv_videodec_jpeg_set(0, VDODEC_SET_JPEG_CB_FUNC, (void*)p_cb_func);

	// 1. open file
	if (jd_emu_open_file(jd_bs_info[pat_idx].bs_fn, out_name) < 0) {
		DBG_ERR("open fail error\r\n");
		goto exit_main;
	}

	// 2. allocate buffer
	if (jpeg_emu_allocate_buffer(&jd_bs_buf, bs_size) < 0) {
		DBG_ERR("allocate bs buffer error\r\n");
		goto exit_main;
	}

	//cb_fun.callback = kdrv_jpeg_dec_cb;
	//kdrv_videodec_jpeg_set(0, VDODEC_SET_CB_FUNC, (void *)(&cb_fun));

	memset(&jd_param, 0, sizeof(KDRV_VDOJPGD_PARAM));

	for (chn = 0; chn < TEST_CHN; chn++) {
		jpg_dec_info[chn].kdrv_id = KDRV_DEV_ID(0, KDRV_VIDEOCDC_ENGINE_JPEG, chn);
		if (jpeg_emu_allocate_buffer(&jpg_dec_info[chn].dec_yuv_buf, frame_size) < 0) {
			DBG_ERR("allocate yuv buffer error\r\n");
			goto exit_main;
		}
		jpg_dec_info[chn].tri_cnt = 0;
		jpg_dec_info[chn].cb_cnt = 0;
	}

	if (0 == pat_idx)
		len = bs_len0[i];
	else if (1 == pat_idx)
		len = bs_len1[i];
	else if (2 == pat_idx)
		len = bs_len2[i];
	else
		len = bs_len3[i];

	// (1) read BS
	jd_read_bs(jd_bs_buf.addr_va, len);

	// 3. trigger job
	jd_frame_cnt = 0;
	for (i = 0; i < frame_num; i++) {
		if (0 == sim_start) {
			vos_perf_mark((VOS_TICK *)(&sim_start));
		}
		else {
			vos_perf_mark((VOS_TICK *)(&sim_end));
			if (sim_start < sim_end) {
				if (vos_perf_duration(sim_start, sim_end) >= JD_MEASURE_PERIOD*1000000) {
					printk("performance = %d fps\n", jd_frame_cnt/JD_MEASURE_PERIOD);
					jd_frame_cnt = 0;
					vos_perf_mark((VOS_TICK *)(&sim_start));
				}
			}
			else {
				vos_perf_mark((VOS_TICK *)(&sim_start));
			}
		}
		//printk("== decode frame %d ==\r\n", i);
		for (j = 0; j < JD_BURST_NUM; j++) {
			// (2) set JPEG kdrv
			for (chn = 0; chn < TEST_CHN; chn++) {
				jpg_dec_info[chn].b_jpeg_dec_cb = FALSE;
				jpeg_dec_set_param(JPEG_JOB_ID(chn, i*JD_BURST_NUM+j), &jd_param, &jpg_dec_info[chn].dec_yuv_buf, &jd_bs_buf, len, ALIGN_64(width)*ALIGN_16(height));
				if (kdrv_videodec_jpeg_trigger(jpg_dec_info[chn].kdrv_id, &jd_param, NULL, NULL) < 0)
					DBG_ERR("{chn%d} decode trigger error !!! \n\n", chn);
				jpg_dec_info[chn].tri_cnt++;
			}
		}
		// printk("decode trigger done\r\n");

		// (3) wait frame done
		cnt = 0;
		for (chn = 0; chn < TEST_CHN; chn++) {
			while (jpg_dec_info[chn].cb_cnt < jpg_dec_info[chn].tri_cnt-2) {
				msleep(10);
				cnt++;
				if (cnt > 500) {
					status = KDRV_VDOJPGD_FAIL;
					DBG_ERR("decode timeout!!\n");
					goto exit_main;
				}
			}
		}
	}

	for (chn = 0; chn < TEST_CHN; chn++) {
		while (jpg_dec_info[chn].cb_cnt < jpg_dec_info[chn].tri_cnt) {
			msleep(10);
			cnt++;
			if (cnt > 500) {
				status = KDRV_VDOJPGD_FAIL;
				DBG_ERR("decode timeout!!\n");
				goto exit_main;
			}
		}
	}

	printk("cb cnt = %d, tri cnt = %d\n", jpg_dec_info[0].cb_cnt, jpg_dec_info[0].tri_cnt);

	// 4. clear all
exit_main:
	jd_emu_close_file();
	jpeg_emu_free_buffer(&jd_bs_buf);
	for (chn = 0; chn < TEST_CHN; chn++) {
		jpeg_emu_free_buffer(&jpg_dec_info[chn].dec_yuv_buf);
		//kdrv_videodec_jpeg_close(jpg_dec_info[chn].kdrv_id);
	}
pat_fail:
	return ret;
}


#endif /* __KERNEL__ */

