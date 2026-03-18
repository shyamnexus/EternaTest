/**
	@brief Function of video Playback with vsp.\n

	@file vsp_dec_func.c

	@author Jack CC Liu

	@ingroup mhdal

	@note This file is modified from video_playback.c.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#if defined(__LINUX)
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <signal.h>
#include <pthread.h>
#endif
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
#define BS_BLK_SIZE     0x500000
#define H26X_NAL_MAXSIZE    512          // H.265 NAL maximum length
#define BS_BLK_DESC_SIZE    0x200     // bitstream buffer size

#define FILE_NAME		"vsp_sensor"	//.dat & .len

////////////////////////////////////////////////

static UINT32 g_frame_interval_us = 0;
static UINT32 g_sleep_time_us = 0;
static char file_folder[32];

///////////////////////////////////////////////

static HD_RESULT parse_h26x_desc(UINT32 codec, UINTPTR src_addr, UINT32 size, BOOL *is_desc)
{
	HD_RESULT r = HD_OK;
	UINT32 start_code = 0, count = 0;
	UINT8 *ptr8 = NULL;

	if (src_addr == 0) {
		printf("buf_addr is 0\r\n");
		return HD_ERR_NG;
	}
	if (size == 0) {
		printf("size is 0\r\n");
		return HD_ERR_NG;
	}
	if (!is_desc) {
		printf("is_desc is null\r\n");
		return HD_ERR_NG;
	}

	ptr8 = (UINT8 *)src_addr;
	count = size;

	if (codec == HD_CODEC_TYPE_H264) {
		while (count--) {
			// search start code to skip (sps, pps)
			if ((*ptr8 == 0x00) && (*(ptr8 + 1) == 0x00) && (*(ptr8 + 2) == 0x00) && (*(ptr8 + 3) == 0x01) && (*(ptr8 + 4) == 0x67 || *(ptr8 + 4) == 0x68)) {
	            start_code++;
			}
			if (start_code == 2) {
				*is_desc = TRUE;
				return HD_OK;
			}
			ptr8++;
		}
	} else if (codec == HD_CODEC_TYPE_H265) {
		while (count--) {
			// search start code to skip (vps, sps, pps)
			if ((*ptr8 == 0x00) && (*(ptr8 + 1) == 0x00) && (*(ptr8 + 2) == 0x00) && (*(ptr8 + 3) == 0x01) && ((*(ptr8 + 4) == 0x40) || (*(ptr8 + 4) == 0x42) || (*(ptr8 + 4) == 0x44))) {
	            start_code++;
			}
			if (start_code == 3) {
				*is_desc = TRUE;
				return HD_OK;
			}
			ptr8++;
		}
	} else {
		printf("unknown codec (%d)\r\n", codec);
		return HD_ERR_NG;
	}

	*is_desc = FALSE;
	return r;
}

static HD_RESULT set_dec_cfg(HD_PATH_ID video_dec_path, HD_DIM *p_max_dim, UINT32 dec_type)
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

static HD_RESULT set_dec_param(VIDEO_RECORD p_stream)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEODEC_IN video_in_param = {0};

	//--- HD_VIDEODEC_PARAM_IN ---
	video_in_param.codec_type = p_stream.dec_type;
	ret = hd_videodec_set(p_stream.dec_path, HD_VIDEODEC_PARAM_IN, &video_in_param);

	//--- HD_VIDEODEC_PARAM_IN_DESC ---
	if (p_stream.dec_type == HD_CODEC_TYPE_H264 || p_stream.dec_type == HD_CODEC_TYPE_H265) {
		//UINT32 desc_len = 0;
		BOOL is_desc = FALSE;
		INT bs_size = 0, read_len = 0;
		char codec_name[8], file_name[128];
		UINT8 desc_buf[H26X_NAL_MAXSIZE+1] = {0};
		FILE *bs_fd = 0, *bslen_fd = 0;
		HD_COMMON_MEM_VB_BLK blk;
		HD_COMMON_MEM_DDR_ID ddr_id = VDEC_DDR_ID;
		UINT32 blk_size = BS_BLK_DESC_SIZE;
		UINTPTR pa = 0, va = 0, bs_buf_start = 0, bs_buf_curr = 0;

		// assign video codec
		if (p_stream.dec_type == HD_CODEC_TYPE_H264) {
			snprintf(codec_name, sizeof(codec_name), "h264");
		} else if (p_stream.dec_type == HD_CODEC_TYPE_H265) {
			snprintf(codec_name, sizeof(codec_name), "h265");
		}

		// open input files
		if ((bs_fd = fopen(p_stream.bs_name, "rb")) == NULL) {
			printf("open file (%s) fail !!\r\n", file_name);
			return HD_ERR_SYS;
		}

		if ((bslen_fd = fopen(p_stream.bs_len, "rb")) == NULL) {
			printf("open file (%s) fail !!\r\n", file_name);
			return HD_ERR_SYS;
		}

		// get memory
		blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_POOL_BEGIN, blk_size, ddr_id); // Get block from mem pool
		if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("get block fail, blk = 0x%x\n", blk);
			return HD_ERR_SYS;
		}

		pa = hd_common_mem_blk2pa(blk); // get physical addr
		if (pa == 0) {
			printf("blk2pa fail, blk(0x%x)\n", blk);
			return HD_ERR_SYS;
		}
		if (pa > 0) {
			va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size); // Get virtual addr
			if (va == 0) {
				printf("get va fail, va(0x%x)\n", blk);
				return HD_ERR_SYS;
			}
			// allocate bs buf
			bs_buf_start = va;
			bs_buf_curr = bs_buf_start;
		}

		// get bs size
		if (fscanf(bslen_fd, "%d\n", &bs_size) == EOF) {
			printf("[ERROR] fscanf error\n");
			return HD_ERR_SYS;
		}

		if (bs_size == 0 || bs_size > H26X_NAL_MAXSIZE) {
			printf("[ERROR] DESC bs_size is 0 !!!\n");
			return HD_ERR_SYS;
		}

		// read bs from file
		read_len = fread((void *)&desc_buf, 1, bs_size, bs_fd);
		desc_buf[read_len] = '\0';
		if (read_len != bs_size) {
			printf("set_desc reading error (read_len=%d, bs_size=%d)\n", read_len, bs_size);
			return HD_ERR_SYS;
		}

		// parse and get h.26x desc
		parse_h26x_desc(p_stream.dec_type, (UINTPTR)&desc_buf, bs_size, &is_desc);
		if (is_desc) {
			HD_H26XDEC_DESC desc_info = {0};
			memcpy((UINT8 *)bs_buf_curr, desc_buf, read_len);
			desc_info.addr = bs_buf_curr;//(UINT32)&desc_buf;
			desc_info.len = read_len;
			ret = hd_videodec_set(p_stream.dec_path, HD_VIDEODEC_PARAM_IN_DESC, &desc_info);
			if (ret != HD_OK) {
				printf("set desc error(%d) !!\r\n\r\n", ret);
				return HD_ERR_SYS;
			}
		} else {
			printf("invalid desc_addr = 0x%lx, len = 0x%x\n", (ULONG)&desc_buf, read_len);
		}
	}

	return ret;
}

HD_RESULT set_all_dec_cfg(VIDEO_RECORD *p_stream, UINT32 dec_dim_w, UINT32 dec_dim_h, UINT32 vdec_num)
{
	HD_RESULT ret = HD_OK;
	UINT32 i = 0;

	for (i = 0;i < vdec_num; i++) {
		// set videodec config
		p_stream[i].dec_max_dim.w = dec_dim_w;
		p_stream[i].dec_max_dim.h = dec_dim_h;
		ret = set_dec_cfg(p_stream[i].dec_path, &p_stream[i].dec_max_dim, p_stream[i].dec_type);
		if (ret != HD_OK) {
			printf("set dec-cfg fail=%d\n", ret);
			return HD_ERR_NG;
		}

		// set videodec parameter
		ret = set_dec_param(p_stream[i]);
		if (ret != HD_OK) {
			printf("set dec fail=%d\n", ret);
			return HD_ERR_NG;
		}
	}

	return HD_OK;
}

/////////////////////////////////////////////////

static HD_RESULT open_module_dec(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim, UINT32 vdec_idx)
{
	HD_RESULT ret = HD_OK;

	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_CTRL(vdec_idx), HD_ISP_DONT_CARE, HD_VIDEOPROC_OUT(vdec_idx, 0), HD_VIDEO_PXLFMT_YUV420);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videodec_open(HD_VIDEODEC_IN(0, vdec_idx), HD_VIDEODEC_OUT(0, vdec_idx), &p_stream->dec_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(vdec_idx, 0), HD_VIDEOPROC_OUT(vdec_idx, 0), &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_IN(0, vdec_idx), HD_VIDEOENC_OUT(0, vdec_idx), &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

HD_RESULT open_module_all_dec(VIDEO_RECORD *p_stream, UINT32 proc_max_dim_w, UINT32 proc_max_dim_h, UINT32 vdec_num)
{
	HD_RESULT ret = HD_OK;
	UINT32 i = 0;

	for (i = 0;i < vdec_num; i++) {
		p_stream[i].proc_max_dim.w = proc_max_dim_w;
		p_stream[i].proc_max_dim.h = proc_max_dim_h;
		p_stream[i].wait_ms = -1; //blocking mode
		ret = open_module_dec(&p_stream[i], &p_stream[i].proc_max_dim, i);
		if (ret != HD_OK) {
			printf("open_dec%d fail=%d\n", i, ret);
			return HD_ERR_NG;
		}
	}

	return HD_OK;
}

HD_RESULT close_module_all_dec(VIDEO_RECORD *p_stream, UINT32 vdec_num)
{
	HD_RESULT ret = HD_OK;
	UINT32 i = 0;

	for (i = 0;i < vdec_num; i++) {
		ret = hd_videoenc_close(p_stream[i].enc_path);
		ret = hd_videoproc_close(p_stream[i].proc_path);
		ret = hd_videodec_close(p_stream[i].dec_path);
		if (ret != HD_OK) {
			printf("close decode stream %d fail=%d\n", i, ret);
			return ret;
		}
	}

	return HD_OK;
}

//////////////////////////////////////////////////////////////

BOOL check_test_pattern(VIDEO_RECORD *p_stream, UINT32 vdec_num)
{
	FILE *f_in;
	char filename[64], filepath[128];
	UINT32 i = 0;

	sprintf(file_folder, "/mnt/sd/vsp/dump_bs/vsp_scenes_%d", g_folder_num);

	for (i = 0;i < vdec_num; i++) {
		// check .dat file
		snprintf(filename, 64, "%s%d.dat", FILE_NAME, i);
		snprintf(filepath, 128, "%s/%s", file_folder, filename);

		if ((f_in = fopen(filepath, "rb")) == NULL) {
			printf("fail to open %s\n", filepath);
			return FALSE;
		}
		fclose(f_in);
		snprintf(p_stream[i].bs_name, 128, filepath);

		// check .len file
		snprintf(filename, 64, "%s%d.len", FILE_NAME, i);
		snprintf(filepath, 128, "%s/%s", file_folder, filename);

		if ((f_in = fopen(filepath, "rb")) == NULL) {
			printf("fail to open %s\n", filepath);
			return FALSE;
		}
		fclose(f_in);
		snprintf(p_stream[i].bs_len, 128, filepath);
	}
	return TRUE;
}

HD_RESULT get_video_frame_buf(HD_VIDEO_FRAME *p_video_frame, HD_DIM video_size, UINT32 dec_type)
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
	width = video_size.w;
	height = video_size.h;
	pxlfmt = HD_VIDEO_PXLFMT_YUV420;

	if (dec_type == HD_CODEC_TYPE_H265) {
		mbinfo_bufsize = VDO_H265_MBINFO_BUFSIZE(ALIGN_CEIL_64(video_size.w), ALIGN_CEIL_64(video_size.h));
	} else if (dec_type == HD_CODEC_TYPE_H264) {
		mbinfo_bufsize = VDO_H264_MBINFO_BUFSIZE(ALIGN_CEIL_64(video_size.w), ALIGN_CEIL_64(video_size.h));
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
	unsigned long frame_idx = 0;
	if (p_stream0->dec_type == HD_CODEC_TYPE_JPEG) frame_idx = -1;

	// wait flow_start
	while (p_stream0->flow_start == 0) sleep(1);

	// open input files
	if ((bs_fd = fopen(p_stream0->bs_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", p_stream0->bs_name);
		return 0;
	}
	printf("bs file: [%s]\n", p_stream0->bs_name);

	if ((bslen_fd = fopen(p_stream0->bs_len, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", p_stream0->bs_len);
		goto quit_rel_fd;
	}
	printf("bslen file: [%s]\n", p_stream0->bs_len);

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

		if ((p_stream0->dec_type == HD_CODEC_TYPE_H264 || p_stream0->dec_type == HD_CODEC_TYPE_H265)) {
			is_desc = FALSE;
			parse_h26x_desc(p_stream0->dec_type, bs_buf_curr, bs_size, &is_desc);
		} else is_desc = FALSE;

		// push in
		if (!is_desc) { // only push I or P
			frame_idx++;
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
			ret = get_video_frame_buf(&video_frame, p_stream0->dec_max_dim, p_stream0->dec_type);
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
		if(frame_idx == 0) {
			gettimeofday(&start_time, NULL);
		} else {
			gettimeofday(&curr_time, NULL);
			g_sleep_time_us = ((frame_idx * g_frame_interval_us) - ((curr_time.tv_sec-start_time.tv_sec)*1000000 + (curr_time.tv_usec-start_time.tv_usec)));
//			printf("g_sleep_time_us(%d) !!\r\n\r\n", g_sleep_time_us);
			if (g_sleep_time_us > g_frame_interval_us || g_sleep_time_us < 0) {
				g_sleep_time_us = g_frame_interval_us;
				frame_idx ++;
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

	// open input files
	if ((bs_fd = fopen(p_stream0->bs_name, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", p_stream0->bs_name);
		return 0;
	}
	printf("bs file: [%s]\n", p_stream0->bs_name);

	if ((bslen_fd = fopen(p_stream0->bs_len, "rb")) == NULL) {
		printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", p_stream0->bs_len);
		goto quit_rel_fd;
	}
	printf("bslen file: [%s]\n", p_stream0->bs_len);

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

		if ((p_stream0->dec_type == HD_CODEC_TYPE_H264 || p_stream0->dec_type == HD_CODEC_TYPE_H265)) {
			is_desc = FALSE;
			parse_h26x_desc(p_stream0->dec_type, bs_buf_curr, bs_size, &is_desc);
		} else is_desc = FALSE;

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
			ret = get_video_frame_buf(&video_frame, p_stream0->dec_max_dim, p_stream0->dec_type);
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

HD_RESULT create_all_dec_thread(VIDEO_RECORD *p_stream, UINT32 vdec_num)
{
	HD_RESULT ret = HD_OK;
	UINT32 i = 0;

	// create decode_thread (push_in bitstream 1st)
	ret = pthread_create(&p_stream[i].feed_thread_id, NULL, feed_bs_thread_1, (void *)&p_stream[i]);
	if (ret < 0) {
		printf("create playback thread num_1 failed");
		return HD_ERR_NG;
	}

	for (i = 1;i < vdec_num; i++) {
		// create decode_thread (push_in bitstream 1st)
		ret = pthread_create(&p_stream[i].feed_thread_id, NULL, feed_bs_thread_2, (void *)&p_stream[i]);
		if (ret < 0) {
			printf("create playback thread num_%d failed", vdec_num);
			return HD_ERR_NG;
		}
	}

	// start all decode thread
	for (i = 0;i < vdec_num; i++) {
		p_stream[i].flow_start = 1;
	}

	return HD_OK;
}
