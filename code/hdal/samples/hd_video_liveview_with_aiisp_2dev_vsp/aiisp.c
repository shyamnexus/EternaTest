/**
	@brief Source file of liveview with ai net sample code.

	@file video_liveview_with_aiisp.c

	@ingroup ai_net_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2020.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/file.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_isp.h"
#include "vendor_common.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_videoprocess.h"
#include "vendor_videoenc.h"
#include "video_record.h"

// platform dependent
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai_video_liveview_with_net, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define AI_DDRID       DDR_ID0

typedef struct _AIISP_PQ_FINAL_PARAM {
	UINT32 src_blend_str;
	UINT8  _3dnr_str;
	UINT8  _2dnr_motion_str;
	UINT8  _2dnr_still_str;
	UINT8  _2dnr_still_texture_str;
	UINT32 coef_a;
	UINT32 coef_b;
} AIISP_PQ_FINAL_PARAM;

///////////////////////////////////////////////////////////////////////////////

#define DUMP_POSTPROC_INFO 0
#define DBG_OUT_DUMP                0  // debug mode, dump output iobuf

#define LABEL_LEN           256     ///< maximal length of class label
#define MAX_CLASS_NUMBER    1000
#define TOP_N                5


///////////////////////////////////////////////////////////////////////////////


#define VENDOR_AI_CFG  				0x000f0000  //vendor ai config

#define AI_RGB_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))

#if AI_ENABLE
static AIISP_PQ_FINAL_PARAM aiisp_pq_final_param = {
	.src_blend_str = 255,
	._3dnr_str = 128,
	._2dnr_motion_str = 128,
	._2dnr_still_str = 128,
	._2dnr_still_texture_str = 128,
	.coef_a = 128,
	.coef_b = 128
};
#endif
///////////////////////////////////////////////////////////////////////////////


/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/

#if AI_ENABLE
static HD_RESULT mem_alloc(MEM_PARM *mem_parm, CHAR* name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	UINTPTR pa   = 0;
	void  *va   = NULL;

	//alloc private pool
	ret = hd_common_mem_alloc(name, &pa, (void**)&va, size, AI_DDRID);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa   = pa;
	mem_parm->va   = (UINTPTR)va;
	mem_parm->size = size;
	mem_parm->blk  = (UINT32)-1;

	return HD_OK;
}

static HD_RESULT mem_free(MEM_PARM *mem_parm)
{
	HD_RESULT ret = HD_OK;

	//free private pool
	ret =  hd_common_mem_free(mem_parm->pa, (void *)mem_parm->va);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa = 0;
	mem_parm->va = 0;
	mem_parm->size = 0;
	mem_parm->blk = (UINT32)-1;

	return HD_OK;
}
#endif
/*-----------------------------------------------------------------------------*/
/* Input Functions                                                             */
/*-----------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Network Functions                                                             */
/*-----------------------------------------------------------------------------*/

typedef struct _NOISE_PROFILE  { //input blob
	UINT16 noise_base;
	UINT16 noise_slope;
} NOISE_PROFILE;

typedef struct _SNR_STRENGTH  { //input blob
	UINT16 min_motion;
	UINT16 max_motion;
	UINT16 min_detail;
	UINT16 max_detail;
} SNR_STRENGTH;

typedef struct _USE_REFERENCE  { //input blob
	UINT16 blend;
} USE_REFERENCE;

typedef enum {
	NET_IN_INPUT_COEFFA,
	NET_IN_INPUT_COEFFB,
	NET_IN_INPUT_BLEND,
	NET_IN_INPUT_MAX,
	ENUM_DUMMY4WORD(NET_IN_INPUT)
} NET_IN_INPUT;



typedef struct _NET_IN {

	NET_IN_CONFIG in_cfg[NET_IN_INPUT_MAX];
	MEM_PARM input_mem[NET_IN_INPUT_MAX];
	UINT32 in_id;
	VENDOR_AI3_BUF src_img[NET_IN_INPUT_MAX];
} NET_IN;

typedef struct _NET_PROC {

	NET_PROC_CONFIG net_cfg;
	MEM_PARM proc_mem;
	UINT32 proc_id;

	CHAR out_class_labels[MAX_CLASS_NUMBER * LABEL_LEN];
	MEM_PARM rslt_mem;
	MEM_PARM io_mem;
	MEM_PARM intl_mem;
	VENDOR_AI3_NET_INFO net_info;

	MEM_PARM input_mem;

	NOISE_PROFILE noise_profile[3];
	SNR_STRENGTH snr_strength[3];
	USE_REFERENCE use_reference[3];
} NET_PROC;



#if AI_ENABLE
static NET_PROC g_net[16] = {0};
static NET_IN g_in[16] = {0};

HD_RESULT input_init(void)
{
	HD_RESULT ret = HD_OK;
	int  i;

	for (i = 0; i < 16; i++) {
		NET_IN* p_net = g_in + i;
		p_net->in_id = i;
	}
	return ret;
}

HD_RESULT input_uninit(void)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static INT32 _getsize_model(char* filename)
{
	FILE *bin_fd;
	UINT32 bin_size = 0;

	bin_fd = fopen(filename, "rb");
	if (!bin_fd) {
		printf("get bin(%s) size fail\n", filename);
		return (-1);
	}

	fseek(bin_fd, 0, SEEK_END);
	bin_size = ftell(bin_fd);
	fseek(bin_fd, 0, SEEK_SET);
	fclose(bin_fd);

	return bin_size;
}

static UINT32 _load_model(CHAR *filename, UINTPTR va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const UINTPTR model_addr = va;
	//DBG_DUMP("model addr = %08x\r\n", (int)model_addr);

	fd = fopen(filename, "rb");
	if (!fd) {
		printf("load model(%s) fail\r\n", filename);
		return 0;
	}

	fseek ( fd, 0, SEEK_END );
	file_size = ALIGN_CEIL_4( ftell(fd) );
	fseek ( fd, 0, SEEK_SET );

	read_size = fread ((void *)model_addr, 1, file_size, fd);
	if (read_size != file_size) {
		printf("size mismatch, real = %d, idea = %d\r\n", (int)read_size, (int)file_size);
	}
	fclose(fd);

	printf("load model(%s) ok\r\n", filename);
	return read_size;
}

static HD_RESULT network_alloc_io_buf(NET_PATH_ID net_path, UINT32 req_size)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "ai_io_buf %u", net_path);

	ret = mem_alloc(&p_net->io_mem, mem_name, req_size);
	if (ret != HD_OK) {
		printf("net_path(%lu) alloc ai_io_buf fail\r\n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_io_buf: work buf, pa = %#lx, va = %#lx, size = %lu\r\n", p_net->io_mem.pa, p_net->io_mem.va, p_net->io_mem.size);

	return ret;
}

static HD_RESULT network_free_io_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	if (p_net->io_mem.pa && p_net->io_mem.va) {
		mem_free(&p_net->io_mem);
	}
	return ret;
}

static HD_RESULT network_alloc_intl_buf(NET_PATH_ID net_path, UINT32 req_size)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "ai_ronly_buf %u", net_path);

	ret = mem_alloc(&p_net->intl_mem, mem_name, req_size);
	if (ret != HD_OK) {
		printf("net_path(%lu) alloc ai_ronly_buf fail\r\n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_intl_buf: internal buf, pa = %#lx, va = %#lx, size = %lu\r\n", p_net->intl_mem.pa, p_net->intl_mem.va, p_net->intl_mem.size);

	return ret;
}

static HD_RESULT network_free_intl_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	if (p_net->intl_mem.pa && p_net->intl_mem.va) {
		mem_free(&p_net->intl_mem);
	}
	return ret;
}

static HD_RESULT network_alloc_input_buf(NET_PATH_ID net_path, UINT32 req_size)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "ai_io_buf %u", net_path);

	ret = mem_alloc(&p_net->input_mem, mem_name, req_size);
	if (ret != HD_OK) {
		printf("net_path(%lu) alloc ai_io_buf fail\r\n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_input_buf: work buf, pa = %#lx, va = %#lx, size = %lu\r\n", p_net->input_mem.pa, p_net->input_mem.va, p_net->input_mem.size);

	return ret;
}

static HD_RESULT network_free_input_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	if (p_net->input_mem.pa && p_net->input_mem.va) {
		mem_free(&p_net->input_mem);
	}
	return ret;
}

HD_RESULT network_init(void)
{
	HD_RESULT ret = HD_OK;

	// call init
	{
		VENDOR_AI3_DEV_CFG dev_cfg = {0};

		ret = vendor_ai3_dev_init(&dev_cfg);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_init fail=%d\n", ret);
			return ret;
		}
	}
	// dump AI3 version
	{
		VENDOR_AI3_VER ai3_ver = {0};
		ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_VER, &ai3_ver);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_get(CFG_VER) fail=%d\n", ret);
			return ret;
		}
		printf("vendor_ai version = %s\r\n", ai3_ver.vendor_ai_impl_version);
		printf("kflow_ai  version = %s\r\n", ai3_ver.kflow_ai_impl_version);
		printf("kdrv_ai   version = %s\r\n", ai3_ver.kdrv_ai_impl_version);
	}
	return ret;
}

HD_RESULT network_uninit(void)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		printf("vendor_ai3_dev_uninit fail=%d\n", ret);
	}

	return ret;
}

INT32 network_mem_config(NET_PATH_ID net_path, void* p_cfg)
{
	NET_PROC* p_net = g_net + net_path;
	NET_PROC_CONFIG* p_proc_cfg = (NET_PROC_CONFIG*)p_cfg;

	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));
	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("net_path(%u) input model is null\r\n", net_path);
		return HD_ERR_NG;
	}

	p_net->net_cfg.binsize = _getsize_model(p_net->net_cfg.model_filename);
	if (p_net->net_cfg.binsize <= 0) {
		printf("net_path(%u) input model is not exist?\r\n", net_path);
		return HD_ERR_NG;
	}

	printf("net_path(%u) set net_mem_cfg: model-file(%s), binsize=%d\r\n",
		net_path,
		p_net->net_cfg.model_filename,
		p_net->net_cfg.binsize);

	return HD_OK;
}

HD_RESULT network_open(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 loadsize = 0;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "model.bin %u", net_path);

	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("net_path(%u) input model is null\r\n", net_path);
		return 0;
	}
	ret  =  mem_alloc(&p_net->proc_mem, mem_name,  p_net->net_cfg.binsize);
	if (ret != HD_OK) {
		printf("net_path(%u) mem_alloc model.bin fail=%d\n", net_path, ret);
		return HD_ERR_FAIL;
	}
	//load file
	loadsize = _load_model(p_net->net_cfg.model_filename, p_net->proc_mem.va);

	if (loadsize <= 0) {
		printf("net_path(%u) input model load fail: %s\r\n", net_path, p_net->net_cfg.model_filename);
		return 0;
	}

	// query model info for WORKBUF/RONLYBUF size , then alloc WORKBUF/RONLYBUF
	{
		VENDOR_AI3_MODEL_INFO model_info = {0};

		model_info.model_buf.pa   = p_net->proc_mem.pa;
		model_info.model_buf.va   = p_net->proc_mem.va;
		model_info.model_buf.size = p_net->proc_mem.size;
#if DBG_OUT_DUMP
		model_info.ctrl           = CTRL_BUF_DEBUG | CTRL_JOB_DEBUG | CTRL_JOB_DUMPOUT;
#endif
		ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_MODEL_INFO, &model_info);
		if (ret != HD_OK) {
			printf("net_path(%u) vendor_ai3_dev_get(MODEL_INFO) fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		}

		printf("model_info get => workbuf size = %d, ronlybuf size = %d\r\n", model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size, model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);

		// alloc WORKBUF/RONLYBUF
		ret = network_alloc_intl_buf(net_path, model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);
		if (ret != HD_OK) {
			printf("net_path(%u) alloc ronlybuf fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		}

		ret = network_alloc_io_buf(net_path, model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size);
		if (ret != HD_OK) {
			printf("net_path(%u) alloc workbuf fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		}

		ret = network_alloc_input_buf(net_path, 0x10000);
		if (ret != HD_OK) {
			printf("net_path(%u) alloc workbuf fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		}

		{
			//default
			p_net->noise_profile[0].noise_base = 378;
			p_net->noise_profile[0].noise_slope = 9762;
			p_net->snr_strength[0].min_motion = 3276;
			p_net->snr_strength[0].max_motion = 4095;
			p_net->snr_strength[0].min_detail = 410;
			p_net->snr_strength[0].max_detail = 410;
			p_net->use_reference[0].blend = 1;

			//disable 3D
			p_net->noise_profile[1].noise_base = 378;
			p_net->noise_profile[1].noise_slope = 9762;
			p_net->snr_strength[1].min_motion = 3276;
			p_net->snr_strength[1].max_motion = 4095;
			p_net->snr_strength[1].min_detail = 410;
			p_net->snr_strength[1].max_detail = 410;
			p_net->use_reference[1].blend = 0;

			//small noise profile
			p_net->noise_profile[2].noise_base = 141;
			p_net->noise_profile[2].noise_slope = 4994;
			p_net->snr_strength[2].min_motion = 3276;
			p_net->snr_strength[2].max_motion = 4095;
			p_net->snr_strength[2].min_detail = 410;
			p_net->snr_strength[2].max_detail = 410;
			p_net->use_reference[2].blend = 1;
		}
	}

	// call open()
	{
		VENDOR_AI3_PROC_CFG proc_cfg = {0};

		proc_cfg.model_buf.pa   = p_net->proc_mem.pa;
		proc_cfg.model_buf.va   = p_net->proc_mem.va;
		proc_cfg.model_buf.size = p_net->proc_mem.size;

		proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].pa   = p_net->intl_mem.pa;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].va   = p_net->intl_mem.va;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size = p_net->intl_mem.size;

		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa   = p_net->io_mem.pa;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].va   = p_net->io_mem.va;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size = p_net->io_mem.size;

		proc_cfg.plugin[AI3_PLUGIN_CPU] = vendor_ai_cpu1_get_engine();
		proc_cfg.config[AI3_PROC_CFG_ISP_POOL_ID] = ISP_POOL_ID(net_path) ; 
#if DBG_OUT_DUMP
		proc_cfg.ctrl           = CTRL_BUF_DEBUG | CTRL_JOB_DEBUG | CTRL_JOB_DUMPOUT;
#endif
		ret = vendor_ai3_net_open(&p_net->proc_id, &proc_cfg, &p_net->net_info);
		if (ret != HD_OK) {
			printf("net_path(%u) vendor_ai3_net_open() fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		} else {
			printf("net_path(%u) open success => get proc_id(%u), in_buf_cnt(%u), out_buf_cnt(%u)\r\n", net_path, p_net->proc_id, p_net->net_info.in_buf_cnt, p_net->net_info.out_buf_cnt);
		}


	}
	return ret;
}

HD_RESULT network_close(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;

// close
	ret = vendor_ai3_net_close(proc_id);
	if (ret != HD_OK) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_close fail=%d\n", net_path, proc_id, ret);
		return HD_ERR_FAIL;
	}

	if ((ret = network_free_intl_buf(net_path)) != HD_OK)
		return ret;

	if ((ret = network_free_io_buf(net_path)) != HD_OK)
		return ret;

	if ((ret = network_free_input_buf(net_path)) != HD_OK)
		return ret;

	mem_free(&p_net->proc_mem);
	memset(&p_net->net_info, 0, sizeof(VENDOR_AI3_NET_INFO));

	return ret;
}

HD_RESULT network_bind_cb(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_AI_CB, &p_stream->ai_cb);
	if (ret != HD_OK) {
		printf("get ai cb, ret = %d\r\n", ret);
		return ret;
	}

	ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_AI_CB, &p_stream->ai_cb);
	if (ret != HD_OK) {
		printf("set ai cb, ret = %d\r\n", ret);
		return ret;
	}

	return ret;
}

HD_RESULT network_bind_isp_cb(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_videoproc_get(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_CB, &p_stream->isp_cb);
	if (ret != HD_OK) {
		printf("get isp cb, ret = %d\r\n", ret);
		return ret;
	}

	ret = vendor_ai3_dev_set(VENDOR_AI3_CFG_ISP_CB, &p_stream->isp_cb);
	if (ret != HD_OK) {
		printf("set isp cb, ret = %d\r\n", ret);
		return ret;
	}

	return ret;
}

HD_RESULT network_set_isp_ai_cfg(VIDEO_RECORD *p_stream, UINT32 path_id, BOOL enable)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + p_stream->net_path;

	//for dummy model
	VENDOR_VIDEOPROC_ISP_AI isp_ai = {0};

	isp_ai.path_id = path_id;
	isp_ai.proc_id = p_net->proc_id;

	if (enable) {
		ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_AI_START, &isp_ai);
	} else {
		ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_ISP_AI_STOP, &isp_ai);
	}

	return ret;
}

HD_RESULT network_set_buf_by_in_path_list(VIDEO_RECORD *p_stream, UINT32 para_id)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;
	UINT32 i;
	VENDOR_AI3_BUF ai_buf = {0};

	uintptr_t va, pa;

	ai_buf.sign = MAKEFOURCC('A','B','U','F');
	pa = p_net->input_mem.pa;
	va = p_net->input_mem.va;

	for (i = 0; i < p_net->net_info.in_buf_cnt; i++) {
		// get out buf (by out path list)
		ret = vendor_ai3_net_get(proc_id, p_net->net_info.in_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("net_path(%u), proc_id(%u) get in buf fail, i(%d), in_path(0x%lx)\n", p_stream->net_path, proc_id, i,  p_net->net_info.in_path_list[i]);
			goto exit;
		}

		ai_buf.va = va;
		ai_buf.pa = pa;

		if (i == 0) {
			//coeff_a
			*(UINT16*)ai_buf.va = (UINT16)aiisp_pq_final_param.coef_a;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT16));
			ai_buf.size = sizeof(UINT16);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT16;

			//printf("set coef_a %d\r\n", *(UINT16*)ai_buf.va);
		} else if (i == 1) {
			//coeff_b
			*(UINT16*)ai_buf.va = (UINT16)aiisp_pq_final_param.coef_b;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT16));
			ai_buf.size = sizeof(UINT16);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT16;

			//printf("set coef_b %d\r\n", *(UINT16*)ai_buf.va);
		} else if (i == 2) {
			//temp_3d_str
			*(UINT8*)ai_buf.va = aiisp_pq_final_param._3dnr_str;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			//printf("set _3dnr_str %d\r\n", *(UINT8*)ai_buf.va);
		} else if (i == 3) {
			//motion_2d_str
			*(UINT8*)ai_buf.va = aiisp_pq_final_param._2dnr_motion_str;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			//printf("set _2dnr_motion_str %d\r\n", *(UINT8*)ai_buf.va);
		} else if (i == 4) {
			//still_2d_str
			*(UINT8*)ai_buf.va = aiisp_pq_final_param._2dnr_still_str;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			//printf("set _2dnr_still_str %d\r\n", *(UINT8*)ai_buf.va);
		} else if (i == 5) {
			//still_texture_2d_str
			*(UINT8*)ai_buf.va = aiisp_pq_final_param._2dnr_still_texture_str;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			//printf("set _2dnr_still_texture_str %d\r\n", *(UINT8*)ai_buf.va);
		} else {
			//src_blending
			*(UINT8*)ai_buf.va = (UINT8)aiisp_pq_final_param.src_blend_str;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;
			//printf("set src_blend_str %d\r\n", *(UINT8*)ai_buf.va);
		}

		if (HD_OK != ret) {
			printf("in_path(%u) pull input fail !!\n", (p_stream->in_path + i));
			goto exit;
		}
		ret = vendor_ai3_net_set(proc_id, p_net->net_info.in_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u)push input fail !! i(%lu)\n", proc_id, i);
			goto exit;
		}

		pa += ALIGN_CEIL_64(ai_buf.size);
		va += ALIGN_CEIL_64(ai_buf.size);
	}

exit:
	return ret;
}

#endif


///////////////////////////////////////////////////////////////////////////////

HD_RESULT network_start(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret = HD_OK;

	#if AI_ENABLE
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;
	#endif

	#if AI_ENABLE
	ret = vendor_ai3_net_start(proc_id);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_start fail !!\n", p_stream->net_path, proc_id);
	}

	ret = network_set_buf_by_in_path_list(p_stream, 0);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u), para_id(%u) network_set_buf_by_in_path_list fail !!\n", p_stream->net_path, proc_id, 0);
	}
	#endif

	return ret;
}

HD_RESULT network_stop(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret = HD_OK;

	#if AI_ENABLE
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;
	#endif

	#if AI_ENABLE
	//stop: should be call after last time proc
	ret = vendor_ai3_net_stop(proc_id);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_stop fail !!\n", p_stream->net_path, proc_id);
	}
	#endif

	return ret;
}

