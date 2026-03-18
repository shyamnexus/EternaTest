/**
	@brief Function of video Playback with vsp.\n

	@file vsp_4dec_func.c

	@author Jack CC Liu

	@ingroup mhdal

	@note This file is modified from video_playback.c.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videoprocess.h"
#include "vsp_dec_func.h"

/////////////////////////////////////////////////

//header
#define DBGINFO_BUFSIZE()	(0x200)

//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NRX: RAW compress: Only support 12bit mode
#define RAW_COMPRESS_RATIO 41
#define VDO_NRX_BUFSIZE(w, h)           (ALIGN_CEIL_4(ALIGN_CEIL_64(w) / 64 * ((24*RAW_COMPRESS_RATIO+99)/100) * 4 * (h)))
//CA for AWB
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
//LA for AE
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)

//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NVX: YUV compress
#define YUV_COMPRESS_RATIO 75
#define VDO_NVX_BUFSIZE(w, h, pxlfmt)	(VDO_YUV_BUFSIZE(w, h, pxlfmt) * YUV_COMPRESS_RATIO / 100)

// MD
#define MD_HEAD_BUFSIZE()	            (0x40)

// Note , the md info w, h is vprc input w, h not out w, h
#define MD_INFO_BUFSIZE(w, h)           (ALIGN_CEIL_64((((((w + 127 )>> 7) + 3) >> 2) << 2) * ((h + 15) >> 4)))

// MB Info
#define VDO_H264_MBINFO_BUFSIZE(w, h)        (w * h * 5 / 16)
#define VDO_H265_MBINFO_BUFSIZE(w, h)        (w * h / 16)

// Side Info
/* SideInfo line-offset */
#define VDO_SIDEINFO_STRIDE_TMP(x)        ((ALIGN_CEIL_64((x))/64) * 16)
#define VDO_SIDEINFO_LINE_OFFSET(x)        ALIGN_CEIL(VDO_SIDEINFO_STRIDE_TMP(x), 128)/* Stride128 */
/* SideInfo size */
#define VDO_SIDEINFO_BUF_H(y)             (ALIGN_CEIL_64((y))/8)
#define VDO_SIDEINFO_SIZE(x,y)            (VDO_SIDEINFO_LINE_OFFSET((x)) * VDO_SIDEINFO_BUF_H((y)))

/////////////////////////////////////////////////

#define VDEC_DDR_ID		DDR_ID0
#define VDO_SIZE_W      3840
#define VDO_SIZE_H      2160
#define BS_BLK_SIZE     0x500000

#define FILE_NAME_1		"vsp_sensor1"	//.dat & .len
#define FILE_NAME_2		"vsp_sensor2"
#define FILE_NAME_3		"vsp_sensor3"
#define FILE_NAME_4		"vsp_sensor4"

////////////////////////////////////////////////

static UINT32 g_frame_interval_us = 0;
static UINT32 g_sleep_time_us = 0;
static unsigned long g_frame_idx = -1;
static char file_folder[32];

///////////////////////////////////////////////

HD_RESULT set_dec_cfg(HD_PATH_ID video_dec_path, HD_DIM *p_max_dim, UINT32 dec_type)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEODEC_PATH_CONFIG video_path_cfg = {0};

	if (p_max_dim != NULL) {
		// set videodec path config
		video_path_cfg.max_mem.codec_type = dec_type;
		video_path_cfg.max_mem.dim.w = p_max_dim->w;
		video_path_cfg.max_mem.dim.h = p_max_dim->h;
		video_path_cfg.max_mem.max_ref_num = 1;
		video_path_cfg.max_mem.ddr_id = VDEC_DDR_ID;
		ret = hd_videodec_set(video_dec_path, HD_VIDEODEC_PARAM_PATH_CONFIG, &video_path_cfg);
	} else {
		ret = HD_ERR_NG;
	}

	return ret;
}

HD_RESULT set_dec_param(HD_PATH_ID video_dec_path, UINT32 dec_type)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEODEC_IN video_in_param = {0};

	//--- HD_VIDEODEC_PARAM_IN ---
	video_in_param.codec_type = dec_type;
	ret = hd_videodec_set(video_dec_path, HD_VIDEODEC_PARAM_IN, &video_in_param);

	return ret;
}

/////////////////////////////////////////////////

HD_RESULT open_module_dec1(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	#if 0
	HD_RESULT ret;

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_0_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videodec_open(HD_VIDEODEC_0_IN_0, HD_VIDEODEC_0_OUT_0, &p_stream->dec_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
		return ret;
	#endif
	return HD_OK;
}

HD_RESULT open_module_dec2(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	#if 0
	HD_RESULT ret;

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_1_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg2 fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if((ret = hd_videodec_open(HD_VIDEODEC_0_IN_1, HD_VIDEODEC_0_OUT_1, &p_stream->dec_path)) != HD_OK)
        return ret;
    if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_OUT_1, &p_stream->enc_path)) != HD_OK)
		return ret;
	#endif
	return HD_OK;
}

HD_RESULT open_module_dec3(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	#if 0
	HD_RESULT ret;

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_2_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg3 fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if((ret = hd_videodec_open(HD_VIDEODEC_0_IN_2, HD_VIDEODEC_0_OUT_2, &p_stream->dec_path)) != HD_OK)
        return ret;
    if ((ret = hd_videoproc_open(HD_VIDEOPROC_2_IN_0, HD_VIDEOPROC_2_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_2, HD_VIDEOENC_0_OUT_2, &p_stream->enc_path)) != HD_OK)
		return ret;
	#endif
	return HD_OK;
}

HD_RESULT open_module_dec4(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	#if 0
	HD_RESULT ret;

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_3_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg4 fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if((ret = hd_videodec_open(HD_VIDEODEC_0_IN_3, HD_VIDEODEC_0_OUT_3, &p_stream->dec_path)) != HD_OK)
        return ret;
    if ((ret = hd_videoproc_open(HD_VIDEOPROC_3_IN_0, HD_VIDEOPROC_3_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_3, HD_VIDEOENC_0_OUT_3, &p_stream->enc_path)) != HD_OK)
		return ret;
	#endif
	return HD_OK;
}

//////////////////////////////////////////////////////////////

BOOL check_test_pattern(UINT32 dec_type)
{
	FILE *f_in;
	char codec_name[8], filename[64], filepath[128];

	// assign video codec
	switch (dec_type) {
		case HD_CODEC_TYPE_JPEG:	snprintf(codec_name, sizeof(codec_name), "jpeg");		break;
		case HD_CODEC_TYPE_H264:	snprintf(codec_name, sizeof(codec_name), "h264");		break;
		case HD_CODEC_TYPE_H265:	snprintf(codec_name, sizeof(codec_name), "h265");		break;
		default:
			printf("invalid video codec(%d)\n", dec_type);
			return FALSE;
	}

	sprintf(file_folder, "/mnt/sd/vsp_scenes_%d", g_folder_num);

	// check .dat file_1
	sprintf(filename, "%s.dat", FILE_NAME_1);
	sprintf(filepath, "%s/%s", file_folder, filename);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		return FALSE;
	}
	fclose(f_in);
	// check .dat file_2
	sprintf(filename, "%s.dat", FILE_NAME_2);
	sprintf(filepath, "%s/%s", file_folder, filename);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		return FALSE;
	}
	fclose(f_in);
	// check .dat file_3
	sprintf(filename, "%s.dat", FILE_NAME_3);
	sprintf(filepath, "%s/%s", file_folder, filename);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		return FALSE;
	}
	fclose(f_in);
	// check .dat file_4
	sprintf(filename, "%s.dat", FILE_NAME_4);
	sprintf(filepath, "%s/%s", file_folder, filename);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		return FALSE;
	}
	fclose(f_in);

	// check .len file_1
	sprintf(filename, "%s.len", FILE_NAME_1);
	sprintf(filepath, "%s/%s", file_folder, filename);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		return FALSE;
	}
	fclose(f_in);
	// check .len file_2
	sprintf(filename, "%s.len", FILE_NAME_2);
	sprintf(filepath, "%s/%s", file_folder, filename);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		return FALSE;
	}
	fclose(f_in);
	// check .len file_3
	sprintf(filename, "%s.len", FILE_NAME_3);
	sprintf(filepath, "%s/%s", file_folder, filename);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		return FALSE;
	}
	fclose(f_in);
	// check .len file_4
	sprintf(filename, "%s.len", FILE_NAME_4);
	sprintf(filepath, "%s/%s", file_folder, filename);

	if ((f_in = fopen(filepath, "rb")) == NULL) {
		printf("fail to open %s\n", filepath);
		return FALSE;
	}
	fclose(f_in);

	return TRUE;
}

HD_RESULT get_video_frame_buf(HD_VIDEO_FRAME *p_video_frame, UINT32 dec_type)
{
	HD_COMMON_MEM_VB_BLK blk;
	HD_COMMON_MEM_DDR_ID ddr_id = 0;
	UINT32 blk_size = 0, pa = 0;
	UINT32 width = 0, height = 0;
	HD_VIDEO_PXLFMT pxlfmt = 0;
	UINT32 mbinfo_bufsize = 0;


	if (p_video_frame == NULL) {
		printf("config_vdo_frm: p_video_frame is null\n");
		return HD_ERR_SYS;
	}

	// config yuv info
	ddr_id = VDEC_DDR_ID;
	width = VDO_SIZE_W;
	height = VDO_SIZE_H;
	pxlfmt = HD_VIDEO_PXLFMT_YUV420;

	if (dec_type == HD_CODEC_TYPE_H265) {
		mbinfo_bufsize = VDO_H265_MBINFO_BUFSIZE(ALIGN_CEIL_64(VDO_SIZE_W), ALIGN_CEIL_64(VDO_SIZE_H));
	} else if (dec_type == HD_CODEC_TYPE_H264) {
		mbinfo_bufsize = VDO_H264_MBINFO_BUFSIZE(ALIGN_CEIL_64(VDO_SIZE_W), ALIGN_CEIL_64(VDO_SIZE_H));
	} else {
		mbinfo_bufsize = 0;
	}

	blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_64(width), ALIGN_CEIL_64(height), pxlfmt)
								+ mbinfo_bufsize
								+ ALIGN_CEIL(VDO_SIDEINFO_SIZE(ALIGN_CEIL_64(width), ALIGN_CEIL_64(height))*3/2, 4096);;  // align to 64 for h265 raw buffer

	// get memory
	blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("config_vdo_frm: get blk fail, blk(0x%x)\n", blk);
		return HD_ERR_NOMEM;
	}

	pa = hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0) {
		printf("config_vdo_frm: blk2pa fail, blk(0x%x)\n", blk);
		return HD_ERR_SYS;
	}

	p_video_frame->sign = MAKEFOURCC('V', 'F', 'R', 'M');
	p_video_frame->ddr_id = ddr_id;
	p_video_frame->pxlfmt = pxlfmt;
	p_video_frame->dim.w = width;
	p_video_frame->dim.h = height;
	p_video_frame->pw[0] = ALIGN_CEIL_64(width);
	p_video_frame->ph[0] = ALIGN_CEIL_64(height);
	p_video_frame->phy_addr[0] = pa;
	p_video_frame->blk = blk;
	p_video_frame->reserved[0]= (ULONG)blk_size;

	return HD_OK;
}

void *feed_bs_thread_1(void *arg)
{
	VIDEO_RECORD *p_stream0 = (VIDEO_RECORD *)arg;
	char codec_name[8], file_name[128];
	FILE *bs_fd = 0, *bslen_fd = 0;
	HD_RESULT ret = HD_OK;
	HD_COMMON_MEM_VB_BLK blk;
	HD_COMMON_MEM_DDR_ID ddr_id = VDEC_DDR_ID;
	UINT32 blk_size = BS_BLK_SIZE;
	UINTPTR pa = 0, va = 0, bs_buf_start = 0, bs_buf_curr = 0, bs_buf_end = 0;
	INT bs_size = 0, read_len = 0;
	BOOL is_desc = FALSE;
	struct timeval start_time, curr_time;

	// wait flow_start
	while (p_stream0->flow_start == 0) sleep(1);

	// assign video codec
	switch (p_stream0->dec_type) {
		case HD_CODEC_TYPE_JPEG:
			snprintf(codec_name, sizeof(codec_name), "jpeg");
			break;
		case HD_CODEC_TYPE_H264:
			snprintf(codec_name, sizeof(codec_name), "h264");
			break;
		case HD_CODEC_TYPE_H265:
			snprintf(codec_name, sizeof(codec_name), "h265");
			break;
		default:
			printf("invalid video codec(%d)\n", p_stream0->dec_type);
			break;
	}

	// open input files
	sprintf(file_name, "%s/%s.dat", file_folder, FILE_NAME_1);

	if ((bs_fd = fopen(file_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", file_name);
		return 0;
	}
	printf("bs file: [%s]\n", file_name);

	snprintf(file_name, sizeof(file_name), "%s/%s.len", file_folder, FILE_NAME_1);
	if ((bslen_fd = fopen(file_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", file_name);
		goto quit_rel_fd;
	}
	printf("bslen file: [%s]\n", file_name);

	// get memory
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail, blk = 0x%x\n", blk);
		goto rel_blk;
	}

	pa = hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0) {
		printf("blk2pa fail, blk(0x%x)\n", blk);
		goto rel_blk;
	}
	if (pa > 0) {
		va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size); // Get virtual addr
		if (va == 0) {
			printf("get va fail, va(0x%x)\n", blk);
			goto rel_blk;
		}
		// allocate bs buf
		bs_buf_start = va;
		bs_buf_curr = bs_buf_start;
		bs_buf_end = bs_buf_start + blk_size;
	}

	g_frame_interval_us = (1000000/g_fps);
	// feed bs
	while (p_stream0->dec_exit == 0) {
		g_frame_idx++;
		// get bs size
		if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
			// reach EOF, read from the beginning
			fseek(bs_fd, 0, SEEK_SET);
			fseek(bslen_fd, 0, SEEK_SET);
			if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
				printf("[ERROR] fscanf error\n");
				continue;
			}
		}
		if (bs_size == 0) {
			printf("Invalid bs_size(%d)\n", bs_size);
			continue;
		}

		// check bs buf rollback
		if ((bs_buf_curr + bs_size) > bs_buf_end) {
			bs_buf_curr = bs_buf_start;
		}

		// read bs from file
		read_len = fread((void *)bs_buf_curr, 1, bs_size, bs_fd);
		if (read_len != bs_size) {
			printf("reading error\n");
			continue;
		}
		hd_common_mem_flush_cache((void *)bs_buf_curr, bs_size);

		// push in
		if (!is_desc) { // only push I or P
			HD_VIDEODEC_BS video_bitstream = {0};
			HD_VIDEO_FRAME video_frame = {0};

			// config video bs
			video_bitstream.sign          = MAKEFOURCC('V','S','T','M');
			video_bitstream.p_next        = NULL;
			video_bitstream.ddr_id        = ddr_id;
			video_bitstream.vcodec_format = p_stream0->dec_type;
			video_bitstream.timestamp     = hd_gettime_us();
			video_bitstream.blk           = blk;
			video_bitstream.count         = 0;
			video_bitstream.phy_addr      = pa + (bs_buf_curr - bs_buf_start);
			video_bitstream.size          = bs_size;

			// get video frame
			ret = get_video_frame_buf(&video_frame, p_stream0->dec_type);
			if (ret != HD_OK) {
				printf("get video frame error(%d) !!\r\n", ret);
				continue;
			}

			ret = hd_videodec_push_in_buf(p_stream0->dec_path, &video_bitstream, &video_frame, 0); // always blocking mode
			if (ret != HD_OK) {
				printf("push_in error(%d) !!\r\n", ret);
				// release video frame buf
				ret = hd_videodec_release_out_buf(p_stream0->dec_path, &video_frame);
				if (ret != HD_OK) {
					printf("release video frame error(%d) !!\r\n\r\n", ret);
				}
				continue;
			}
			// release video frame buf
			ret = hd_videodec_release_out_buf(p_stream0->dec_path, &video_frame);
			if (ret != HD_OK) {
				printf("release video frame error(%d) !!\r\n\r\n", ret);
			}
		}

		bs_buf_curr += ALIGN_CEIL_64(bs_size); // shift to next

		if(g_frame_idx == 0) {
			gettimeofday(&start_time, NULL);
		} else {
			gettimeofday(&curr_time, NULL);
			g_sleep_time_us = ((g_frame_idx * g_frame_interval_us) - ((curr_time.tv_sec-start_time.tv_sec)*1000000 + (curr_time.tv_usec-start_time.tv_usec)));
			if (g_sleep_time_us > 50000) {
				g_sleep_time_us = g_frame_interval_us;
				g_frame_idx ++;
				printf("g_sleep_time_us(%d) !!\r\n\r\n", g_sleep_time_us);
			}
			if (g_sleep_time_us > 0) usleep(g_sleep_time_us);
		}
	}

	// mummap
	ret = hd_common_mem_munmap((void*)va, blk_size);
	if (ret != HD_OK) {
		printf("mnumap error(%d) !!\r\n\r\n", ret);
	}

rel_blk:
	// release blk
	ret = hd_common_mem_release_block(blk);
	if (ret != HD_OK) {
		printf("release error(%d) !!\n", ret);
	}

quit_rel_fd:
	if (bs_fd) fclose(bs_fd);
	if (bslen_fd) fclose(bslen_fd);

	return 0;
}

void *feed_bs_thread_2(void *arg)
{
	VIDEO_RECORD *p_stream0 = (VIDEO_RECORD *)arg;
	char codec_name[8], file_name[128];
	FILE *bs_fd = 0, *bslen_fd = 0;
	HD_RESULT ret = HD_OK;
	HD_COMMON_MEM_VB_BLK blk;
	HD_COMMON_MEM_DDR_ID ddr_id = VDEC_DDR_ID;
	UINT32 blk_size = BS_BLK_SIZE;
	UINTPTR pa = 0, va = 0, bs_buf_start = 0, bs_buf_curr = 0, bs_buf_end = 0;
	INT bs_size = 0, read_len = 0;
	BOOL is_desc = FALSE;
	struct timeval start_time, curr_time;

	// wait flow_start
	while (p_stream0->flow_start == 0) sleep(1);

	// assign video codec
	switch (p_stream0->dec_type) {
		case HD_CODEC_TYPE_JPEG:
			snprintf(codec_name, sizeof(codec_name), "jpeg");
			break;
		case HD_CODEC_TYPE_H264:
			snprintf(codec_name, sizeof(codec_name), "h264");
			break;
		case HD_CODEC_TYPE_H265:
			snprintf(codec_name, sizeof(codec_name), "h265");
			break;
		default:
			printf("invalid video codec(%d)\n", p_stream0->dec_type);
			break;
	}

	// open input files
	sprintf(file_name, "%s/%s.dat", file_folder, FILE_NAME_2);

	if ((bs_fd = fopen(file_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", file_name);
		return 0;
	}
	printf("bs file: [%s]\n", file_name);

	snprintf(file_name, sizeof(file_name), "%s/%s.len", file_folder, FILE_NAME_2);
	if ((bslen_fd = fopen(file_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", file_name);
		goto quit_rel_fd;
	}
	printf("bslen file: [%s]\n", file_name);

	// get memory
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail, blk = 0x%x\n", blk);
		goto rel_blk;
	}

	pa = hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0) {
		printf("blk2pa fail, blk(0x%x)\n", blk);
		goto rel_blk;
	}
	if (pa > 0) {
		va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size); // Get virtual addr
		if (va == 0) {
			printf("get va fail, va(0x%x)\n", blk);
			goto rel_blk;
		}
		// allocate bs buf
		bs_buf_start = va;
		bs_buf_curr = bs_buf_start;
		bs_buf_end = bs_buf_start + blk_size;
	}

	// feed bs
	while (p_stream0->dec_exit == 0) {
		// get bs size
		if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
			// reach EOF, read from the beginning
			fseek(bs_fd, 0, SEEK_SET);
			fseek(bslen_fd, 0, SEEK_SET);
			if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
				printf("[ERROR] fscanf error\n");
				continue;
			}
		}
		if (bs_size == 0) {
			printf("Invalid bs_size(%d)\n", bs_size);
			continue;
		}

		// check bs buf rollback
		if ((bs_buf_curr + bs_size) > bs_buf_end) {
			bs_buf_curr = bs_buf_start;
		}

		// read bs from file
		read_len = fread((void *)bs_buf_curr, 1, bs_size, bs_fd);
		if (read_len != bs_size) {
			printf("reading error\n");
			continue;
		}
		hd_common_mem_flush_cache((void *)bs_buf_curr, bs_size);

		// push in
		if (!is_desc) { // only push I or P
			HD_VIDEODEC_BS video_bitstream = {0};
			HD_VIDEO_FRAME video_frame = {0};

			// config video bs
			video_bitstream.sign          = MAKEFOURCC('V','S','T','M');
			video_bitstream.p_next        = NULL;
			video_bitstream.ddr_id        = ddr_id;
			video_bitstream.vcodec_format = p_stream0->dec_type;
			video_bitstream.timestamp     = hd_gettime_us();
			video_bitstream.blk           = blk;
			video_bitstream.count         = 0;
			video_bitstream.phy_addr      = pa + (bs_buf_curr - bs_buf_start);
			video_bitstream.size          = bs_size;

			// get video frame
			ret = get_video_frame_buf(&video_frame, p_stream0->dec_type);
			if (ret != HD_OK) {
				printf("get video frame error(%d) !!\r\n", ret);
				continue;
			}

			ret = hd_videodec_push_in_buf(p_stream0->dec_path, &video_bitstream, &video_frame, 0); // always blocking mode
			if (ret != HD_OK) {
				printf("push_in error(%d) !!\r\n", ret);
				// release video frame buf
				ret = hd_videodec_release_out_buf(p_stream0->dec_path, &video_frame);
				if (ret != HD_OK) {
					printf("release video frame error(%d) !!\r\n\r\n", ret);
				}
				continue;
			}
			// release video frame buf
			ret = hd_videodec_release_out_buf(p_stream0->dec_path, &video_frame);
			if (ret != HD_OK) {
				printf("release video frame error(%d) !!\r\n\r\n", ret);
			}
		}

		bs_buf_curr += ALIGN_CEIL_64(bs_size); // shift to next

		usleep(g_sleep_time_us);
	}

	// mummap
	ret = hd_common_mem_munmap((void*)va, blk_size);
	if (ret != HD_OK) {
		printf("mnumap error(%d) !!\r\n\r\n", ret);
	}

rel_blk:
	// release blk
	ret = hd_common_mem_release_block(blk);
	if (ret != HD_OK) {
		printf("release error(%d) !!\n", ret);
	}

quit_rel_fd:
	if (bs_fd) fclose(bs_fd);
	if (bslen_fd) fclose(bslen_fd);

	return 0;
}

void *feed_bs_thread_3(void *arg)
{
	VIDEO_RECORD *p_stream0 = (VIDEO_RECORD *)arg;
	char codec_name[8], file_name[128];
	FILE *bs_fd = 0, *bslen_fd = 0;
	HD_RESULT ret = HD_OK;
	HD_COMMON_MEM_VB_BLK blk;
	HD_COMMON_MEM_DDR_ID ddr_id = VDEC_DDR_ID;
	UINT32 blk_size = BS_BLK_SIZE;
	UINTPTR pa = 0, va = 0, bs_buf_start = 0, bs_buf_curr = 0, bs_buf_end = 0;
	INT bs_size = 0, read_len = 0;
	BOOL is_desc = FALSE;
	struct timeval start_time, curr_time;

	// wait flow_start
	while (p_stream0->flow_start == 0) sleep(1);

	// assign video codec
	switch (p_stream0->dec_type) {
		case HD_CODEC_TYPE_JPEG:
			snprintf(codec_name, sizeof(codec_name), "jpeg");
			break;
		case HD_CODEC_TYPE_H264:
			snprintf(codec_name, sizeof(codec_name), "h264");
			break;
		case HD_CODEC_TYPE_H265:
			snprintf(codec_name, sizeof(codec_name), "h265");
			break;
		default:
			printf("invalid video codec(%d)\n", p_stream0->dec_type);
			break;
	}

	// open input files
	sprintf(file_name, "%s/%s.dat", file_folder, FILE_NAME_3);

	if ((bs_fd = fopen(file_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", file_name);
		return 0;
	}
	printf("bs file: [%s]\n", file_name);

	snprintf(file_name, sizeof(file_name), "%s/%s.len", file_folder, FILE_NAME_3);
	if ((bslen_fd = fopen(file_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", file_name);
		goto quit_rel_fd;
	}
	printf("bslen file: [%s]\n", file_name);

	// get memory
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail, blk = 0x%x\n", blk);
		goto rel_blk;
	}

	pa = hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0) {
		printf("blk2pa fail, blk(0x%x)\n", blk);
		goto rel_blk;
	}
	if (pa > 0) {
		va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size); // Get virtual addr
		if (va == 0) {
			printf("get va fail, va(0x%x)\n", blk);
			goto rel_blk;
		}
		// allocate bs buf
		bs_buf_start = va;
		bs_buf_curr = bs_buf_start;
		bs_buf_end = bs_buf_start + blk_size;
	}

	// feed bs
	while (p_stream0->dec_exit == 0) {
		// get bs size
		if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
			// reach EOF, read from the beginning
			fseek(bs_fd, 0, SEEK_SET);
			fseek(bslen_fd, 0, SEEK_SET);
			if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
				printf("[ERROR] fscanf error\n");
				continue;
			}
		}
		if (bs_size == 0) {
			printf("Invalid bs_size(%d)\n", bs_size);
			continue;
		}

		// check bs buf rollback
		if ((bs_buf_curr + bs_size) > bs_buf_end) {
			bs_buf_curr = bs_buf_start;
		}

		// read bs from file
		read_len = fread((void *)bs_buf_curr, 1, bs_size, bs_fd);
		if (read_len != bs_size) {
			printf("reading error\n");
			continue;
		}
		hd_common_mem_flush_cache((void *)bs_buf_curr, bs_size);

		// push in
		if (!is_desc) { // only push I or P
			HD_VIDEODEC_BS video_bitstream = {0};
			HD_VIDEO_FRAME video_frame = {0};

			// config video bs
			video_bitstream.sign          = MAKEFOURCC('V','S','T','M');
			video_bitstream.p_next        = NULL;
			video_bitstream.ddr_id        = ddr_id;
			video_bitstream.vcodec_format = p_stream0->dec_type;
			video_bitstream.timestamp     = hd_gettime_us();
			video_bitstream.blk           = blk;
			video_bitstream.count         = 0;
			video_bitstream.phy_addr      = pa + (bs_buf_curr - bs_buf_start);
			video_bitstream.size          = bs_size;

			// get video frame
			ret = get_video_frame_buf(&video_frame, p_stream0->dec_type);
			if (ret != HD_OK) {
				printf("get video frame error(%d) !!\r\n", ret);
				continue;
			}

			ret = hd_videodec_push_in_buf(p_stream0->dec_path, &video_bitstream, &video_frame, 0); // always blocking mode
			if (ret != HD_OK) {
				printf("push_in error(%d) !!\r\n", ret);
				// release video frame buf
				ret = hd_videodec_release_out_buf(p_stream0->dec_path, &video_frame);
				if (ret != HD_OK) {
					printf("release video frame error(%d) !!\r\n\r\n", ret);
				}
				continue;
			}
			// release video frame buf
			ret = hd_videodec_release_out_buf(p_stream0->dec_path, &video_frame);
			if (ret != HD_OK) {
				printf("release video frame error(%d) !!\r\n\r\n", ret);
			}
		}

		bs_buf_curr += ALIGN_CEIL_64(bs_size); // shift to next

		usleep(g_sleep_time_us);
	}

	// mummap
	ret = hd_common_mem_munmap((void*)va, blk_size);
	if (ret != HD_OK) {
		printf("mnumap error(%d) !!\r\n\r\n", ret);
	}

rel_blk:
	// release blk
	ret = hd_common_mem_release_block(blk);
	if (ret != HD_OK) {
		printf("release error(%d) !!\n", ret);
	}

quit_rel_fd:
	if (bs_fd) fclose(bs_fd);
	if (bslen_fd) fclose(bslen_fd);

	return 0;
}

void *feed_bs_thread_4(void *arg)
{
	VIDEO_RECORD *p_stream0 = (VIDEO_RECORD *)arg;
	char codec_name[8], file_name[128];
	FILE *bs_fd = 0, *bslen_fd = 0;
	HD_RESULT ret = HD_OK;
	HD_COMMON_MEM_VB_BLK blk;
	HD_COMMON_MEM_DDR_ID ddr_id = VDEC_DDR_ID;
	UINT32 blk_size = BS_BLK_SIZE;
	UINTPTR pa = 0, va = 0, bs_buf_start = 0, bs_buf_curr = 0, bs_buf_end = 0;
	INT bs_size = 0, read_len = 0;
	BOOL is_desc = FALSE;
	struct timeval start_time, curr_time;

	// wait flow_start
	while (p_stream0->flow_start == 0) sleep(1);

	// assign video codec
	switch (p_stream0->dec_type) {
		case HD_CODEC_TYPE_JPEG:
			snprintf(codec_name, sizeof(codec_name), "jpeg");
			break;
		case HD_CODEC_TYPE_H264:
			snprintf(codec_name, sizeof(codec_name), "h264");
			break;
		case HD_CODEC_TYPE_H265:
			snprintf(codec_name, sizeof(codec_name), "h265");
			break;
		default:
			printf("invalid video codec(%d)\n", p_stream0->dec_type);
			break;
	}

	// open input files
	sprintf(file_name, "%s/%s.dat", file_folder, FILE_NAME_4);

	if ((bs_fd = fopen(file_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", file_name);
		return 0;
	}
	printf("bs file: [%s]\n", file_name);

	snprintf(file_name, sizeof(file_name), "%s/%s.len", file_folder, FILE_NAME_4);
	if ((bslen_fd = fopen(file_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", file_name);
		goto quit_rel_fd;
	}
	printf("bslen file: [%s]\n", file_name);

	// get memory
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail, blk = 0x%x\n", blk);
		goto rel_blk;
	}

	pa = hd_common_mem_blk2pa(blk); // get physical addr
	if (pa == 0) {
		printf("blk2pa fail, blk(0x%x)\n", blk);
		goto rel_blk;
	}
	if (pa > 0) {
		va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size); // Get virtual addr
		if (va == 0) {
			printf("get va fail, va(0x%x)\n", blk);
			goto rel_blk;
		}
		// allocate bs buf
		bs_buf_start = va;
		bs_buf_curr = bs_buf_start;
		bs_buf_end = bs_buf_start + blk_size;
	}

	// feed bs
	while (p_stream0->dec_exit == 0) {
		// get bs size
		if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
			// reach EOF, read from the beginning
			fseek(bs_fd, 0, SEEK_SET);
			fseek(bslen_fd, 0, SEEK_SET);
			if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
				printf("[ERROR] fscanf error\n");
				continue;
			}
		}
		if (bs_size == 0) {
			printf("Invalid bs_size(%d)\n", bs_size);
			continue;
		}

		// check bs buf rollback
		if ((bs_buf_curr + bs_size) > bs_buf_end) {
			bs_buf_curr = bs_buf_start;
		}

		// read bs from file
		read_len = fread((void *)bs_buf_curr, 1, bs_size, bs_fd);
		if (read_len != bs_size) {
			printf("reading error\n");
			continue;
		}
		hd_common_mem_flush_cache((void *)bs_buf_curr, bs_size);

		// push in
		if (!is_desc) { // only push I or P
			HD_VIDEODEC_BS video_bitstream = {0};
			HD_VIDEO_FRAME video_frame = {0};

			// config video bs
			video_bitstream.sign          = MAKEFOURCC('V','S','T','M');
			video_bitstream.p_next        = NULL;
			video_bitstream.ddr_id        = ddr_id;
			video_bitstream.vcodec_format = p_stream0->dec_type;
			video_bitstream.timestamp     = hd_gettime_us();
			video_bitstream.blk           = blk;
			video_bitstream.count         = 0;
			video_bitstream.phy_addr      = pa + (bs_buf_curr - bs_buf_start);
			video_bitstream.size          = bs_size;

			// get video frame
			ret = get_video_frame_buf(&video_frame, p_stream0->dec_type);
			if (ret != HD_OK) {
				printf("get video frame error(%d) !!\r\n", ret);
				continue;
			}

			ret = hd_videodec_push_in_buf(p_stream0->dec_path, &video_bitstream, &video_frame, 0); // always blocking mode
			if (ret != HD_OK) {
				printf("push_in error(%d) !!\r\n", ret);
				// release video frame buf
				ret = hd_videodec_release_out_buf(p_stream0->dec_path, &video_frame);
				if (ret != HD_OK) {
					printf("release video frame error(%d) !!\r\n\r\n", ret);
				}
				continue;
			}
			// release video frame buf
			ret = hd_videodec_release_out_buf(p_stream0->dec_path, &video_frame);
			if (ret != HD_OK) {
				printf("release video frame error(%d) !!\r\n\r\n", ret);
			}
		}

		bs_buf_curr += ALIGN_CEIL_64(bs_size); // shift to next

		usleep(g_sleep_time_us);
	}

	// mummap
	ret = hd_common_mem_munmap((void*)va, blk_size);
	if (ret != HD_OK) {
		printf("mnumap error(%d) !!\r\n\r\n", ret);
	}

rel_blk:
	// release blk
	ret = hd_common_mem_release_block(blk);
	if (ret != HD_OK) {
		printf("release error(%d) !!\n", ret);
	}

quit_rel_fd:
	if (bs_fd) fclose(bs_fd);
	if (bslen_fd) fclose(bslen_fd);

	return 0;
}
