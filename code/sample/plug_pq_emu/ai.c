
#include <string.h>
#include <pd_shm.h>
#include <sys/shm.h>

#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_videoprocess.h"
#include "aiisp_pq.h"

#include "ai.h"

///////////////////////////////////////////////////////////////////////////////
#define AIISP_MODEL_NAME_1 "nvt_model_1_2_5_1_0"
#define AIISP_MODEL_NAME_2 "nvt_model_1_2_5_1_0_low"

#define AIISP_PRINT_MODEL_INFO 1

///////////////////////////////////////////////////////////////////////////////
#define AI_DDRID       DDR_ID0
#define VIDEO_DDRID    DDR_ID0

#define NET_PATH_ID UINT32
#define VENDOR_AI_CFG 0x000f0000  //vendor ai config
#define AI_RGB_BUFSIZE(w, h) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))
#define POOL_SIZE_USER_DEFINIED  0x1000000

#define DBG_OUT_DUMP                0  // debug mode, dump output iobuf

#define LABEL_LEN           256     ///< maximal length of class label
#define MAX_CLASS_NUMBER    1000
#define TOP_N                5

///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/
typedef struct _MEM_PARM {
	UINTPTR pa;
	UINTPTR va;
	UINT32 size;
	UINT32 blk;
} MEM_PARM;

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

typedef struct _NET_IN_CONFIG {
	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
	UINT32 type;
	UINT32 batch;
	UINT32 time;
} NET_IN_CONFIG;

typedef struct _NET_IN {

	NET_IN_CONFIG in_cfg[NET_IN_INPUT_MAX];
	MEM_PARM input_mem[NET_IN_INPUT_MAX];
	UINT32 in_id;
	VENDOR_AI3_BUF src_img[NET_IN_INPUT_MAX];
} NET_IN;

typedef struct _NET_PROC_CONFIG {

	CHAR model_filename[256];
	INT32 binsize;
	CHAR label_filename[256];

} NET_PROC_CONFIG;

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

typedef struct _VIDEO_AI {
	HD_PATH_ID proc_ctrl;

	NET_PATH_ID net_path;
	NET_PATH_ID in_path;

	uintptr_t ai_cb;  //isp call to ai
	uintptr_t isp_cb;  //ai call to isp
} VIDEO_AI;

static VIDEO_AI stream[3] = {0}; //0: main stream //1: sub stream (vout) //2: second AI

///////////////////////////////////////////////////////////////////////////////
static AIISP_PQ_FINAL_PARAM aiisp_pq_final_param = {
	.reserved_common = 255,
	._2dnr_still_str = 128,
	.reserved_1 = 128,
	.reserved_2 = 128,
	.reserved_3 = 128,
	.coef_a = 128,
	.coef_b = 128
};

static AI_MODE_SEL last_sel = AI_MODE_SEL_DISABLE;
static NET_PROC g_net[16] = {0};
static NET_IN g_in[16] = {0};
CHAR ai_model_name[2][64] = {AIISP_MODEL_NAME_1, AIISP_MODEL_NAME_2};

#if (AIISP_PRINT_MODEL_INFO)
typedef enum _AI_MODEL_FUNCTION {
	BAYER_AI_3DNR = 1,
	BAYER_AI_2DNR,
	AI_MODEL_TOTAL,
	ENUM_DUMMY4WORD(AI_MODEL_FUNCTION)
} AI_MODEL_FUNCTION;

typedef enum _AI_3DNR_MODEL_VERSION {
	FULL_VERSION = 0,
	LITE_VERSION,
	FUSION_VERSION,
	FULL_VERSION_HDR,
	FUSION_VERSION_HDR,
	ENUM_DUMMY4WORD(AI_3DNR_MODEL_VERSION)
} AI_3DNR_MODEL_VERSION;

typedef struct _AI_MODEL_TAG {
	//Word 0
	UINT8   function;               //Byte 0
	UINT8   model_version;          //Byte 1
	UINT8   compu_utility_version;  //Byte 2
	UINT8   optimize_info;          //Byte 3

	//Word 1
	UINT8   minor_version_num;      //Byte 0
	UINT8   reserved0;              //Byte 1
	UINT8   reserved1;              //Byte 2
	UINT8   reserved2;              //Byte 3

	//Word 2
	UINT8   reserved3;              //Byte 0
	UINT8   reserved4;              //Byte 1
	UINT8   reserved5;              //Byte 2
	UINT8   reserved6;              //Byte 3

	//Word 3
	UINT16  min_coefa;              //U16, Byte 1..0
	UINT16  max_coefa;              //U16, Byte 3..2

	//Word 4
	UINT16  min_coefb;              //U16, Byte 1..0
	UINT16  max_coefb;              //U16, Byte 3..2

	//Word 5
	UINT8   iso_low;                // 0:ISO100, 1:ISO200,... , byte 0
	UINT8   iso_high;               // 0:ISO100, 1:ISO200,... , byte 1
	UINT8   dummy0;                 //byte 2
	UINT8   dummy1;                 //byte 3
} AI_MODEL_TAG;
#endif

//=============================================================================
// internal functions
//=============================================================================
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

static HD_RESULT input_init(void)
{
	HD_RESULT ret = HD_OK;
	int  i;

	for (i = 0; i < 16; i++) {
		NET_IN* p_net = g_in + i;
		p_net->in_id = i;
	}
	return ret;
}

static HD_RESULT input_uninit(void)
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
		printf("get bin(%s) size fail \n", filename);
		return (-1);
	}

	fseek(bin_fd, 0, SEEK_END);
	bin_size = ftell(bin_fd);
	fseek(bin_fd, 0, SEEK_SET);
	fclose(bin_fd);

	return bin_size;
}

#if (AIISP_PRINT_MODEL_INFO)
static void netword_print_model_info(UINTPTR model_addr)
{
	NN_GEN_MODEL_HEAD *p_head = (NN_GEN_MODEL_HEAD *)model_addr;
	UINT32 ex_total_size = 0;
	UINT64 p_ex_head = model_addr + sizeof(NN_GEN_MODEL_HEAD);
	UINT32 aiisp_info_tag = (UINT32)((UINT32)('I') | ((UINT32)('S')<<8) | ((UINT32)('I')<<16) | ((UINT32)('F')<<24));
	UINT64 tmp_size = 0, total_size;
	CHAR tag[24];
	BOOL is_print_tag = FALSE;
	AI_MODEL_TAG *ai_model_tag = NULL;

	if (sizeof(tag) == sizeof(AI_MODEL_TAG)) {
		ex_total_size = p_head->external_size;
		total_size = p_head->user_parm_non_align - sizeof(NN_GEN_MODEL_HEAD);
		if (ex_total_size == 0) {
			printf("AI model tag doesn't have external info block. \n");
		} else {
			while (tmp_size < total_size) {
				UINT32* p_tmp_head = (UINT32*)((p_ex_head) + tmp_size);
				if (p_tmp_head[1] == aiisp_info_tag) {
					UINT32* bin_size = (UINT32* )(((UINT64)p_tmp_head) + BLKTAGSIZE * sizeof(UINT32));
					uintptr_t* bin_addr = (uintptr_t* )(((UINT64)p_tmp_head) + BLKTAGSIZE * sizeof(UINT32) + sizeof(UINT32));
					if (*bin_size <= sizeof(tag)) {
						memcpy(&tag, bin_addr, *bin_size);
						is_print_tag = TRUE;
					} else {
						printf("write AI model tag fail (%d) (%d) \n", *bin_size, sizeof(tag));
					}
				}
				tmp_size = tmp_size + p_tmp_head[0];
			}
		}

		if (is_print_tag) {
			ai_model_tag = (AI_MODEL_TAG *)(&tag);
			printf("AI model tag: version %d.%d.%d.%d.%d, iso %d - %d, coefa %d - %d, coefb %d - %d \n", ai_model_tag->function, ai_model_tag->model_version, ai_model_tag->minor_version_num, ai_model_tag->compu_utility_version, ai_model_tag->optimize_info,
																							ai_model_tag->iso_low, ai_model_tag->iso_high, ai_model_tag->min_coefa, ai_model_tag->max_coefa, ai_model_tag->min_coefb, ai_model_tag->max_coefb, ai_model_tag->iso_low);
		} else {
			printf("AI model tag is not exist \n");
		}
	} else {
		printf("AI model tag size not match (%d) (%d) \n", sizeof(tag), sizeof(AI_MODEL_TAG));
	}
};
#endif

static UINT32 network_load_model(CHAR *filename, UINTPTR va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const UINTPTR model_addr = va;
	//DBG_DUMP("model addr = %08x\r\n", (int)model_addr);

	fd = fopen(filename, "rb");
	if (!fd) {
		printf("load model(%s) fail \n", filename);
		return 0;
	}

	fseek ( fd, 0, SEEK_END );
	file_size = ALIGN_CEIL_4( ftell(fd) );
	fseek ( fd, 0, SEEK_SET );

	read_size = fread ((void *)model_addr, 1, file_size, fd);
	if (read_size != file_size) {
		printf("size mismatch, real = %d, idea = %d \n", (int)read_size, (int)file_size);
	}
	fclose(fd);

	printf("load model(%s) ok \n", filename);

	#if (AIISP_PRINT_MODEL_INFO)
	netword_print_model_info(model_addr);
	#endif

	return read_size;
}

static HD_RESULT network_init(void)
{
	HD_RESULT ret = HD_OK;

	// call init
	{
		VENDOR_AI3_DEV_CFG dev_cfg = {0};

		ret = vendor_ai3_dev_init(&dev_cfg);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_init fail = %d \n", ret);
			return ret;
		}
	}
	// dump AI3 version
	{
		VENDOR_AI3_VER2 ai3_ver = {0};
		ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_VER2, &ai3_ver);
		if (ret != HD_OK) {
			printf("vendor_ai3_dev_get(CFG_VER) fail = %d \n", ret);
			return ret;
		}
		printf("vendor_ai version = %s\r\n", ai3_ver.vendor_ai_impl_version);
		printf("kflow_ai version  = %s\r\n", ai3_ver.kflow_ai_impl_version);
		printf("kflow_isp version = %s\r\n", ai3_ver.kflow_ai_isp_impl_version);
		printf("kdrv_ai version   = %s\r\n", ai3_ver.kdrv_ai_impl_version);
	}
	return ret;
}

static HD_RESULT network_uninit(void)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		printf("vendor_ai3_dev_uninit fail = %d \n", ret);
	}

	return ret;
}

static INT32 network_mem_config(NET_PATH_ID net_path, void* p_cfg)
{
	NET_PROC* p_net = g_net + net_path;
	NET_PROC_CONFIG* p_proc_cfg = (NET_PROC_CONFIG*)p_cfg;

	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));
	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("net_path(%u) input model is null \n", net_path);
		return HD_ERR_NG;
	}

	p_net->net_cfg.binsize = _getsize_model(p_net->net_cfg.model_filename);
	if (p_net->net_cfg.binsize <= 0) {
		printf("net_path(%u) input model is not exist? \n", net_path);
		return HD_ERR_NG;
	}

	printf("net_path(%u) set net_mem_cfg: model-file(%s), binsize=%d \n",
		net_path,
		p_net->net_cfg.model_filename,
		p_net->net_cfg.binsize);

	return HD_OK;
}

static HD_RESULT network_alloc_io_buf(NET_PATH_ID net_path, UINT32 req_size)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "ai_io_buf %u", net_path);

	ret = mem_alloc(&p_net->io_mem, mem_name, req_size);
	if (ret != HD_OK) {
		printf("net_path(%lu) alloc ai_io_buf fail \n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_io_buf: work buf, pa = %#lx, va = %#lx, size = %lu \n", p_net->io_mem.pa, p_net->io_mem.va, p_net->io_mem.size);

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
		printf("net_path(%lu) alloc ai_ronly_buf fail \n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_intl_buf: internal buf, pa = %#lx, va = %#lx, size = %lu \n", p_net->intl_mem.pa, p_net->intl_mem.va, p_net->intl_mem.size);

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
		printf("net_path(%lu) alloc ai_io_buf fail \n", net_path);
		return HD_ERR_FAIL;
	}

	printf("alloc_input_buf: work buf, pa = %#lx, va = %#lx, size = %lu \n", p_net->input_mem.pa, p_net->input_mem.va, p_net->input_mem.size);

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

static HD_RESULT network_open(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 loadsize = 0;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "model.bin %u", net_path);

	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("net_path(%u) input model is null \n", net_path);
		return 0;
	}
	ret  =  mem_alloc(&p_net->proc_mem, mem_name,  p_net->net_cfg.binsize);
	if (ret != HD_OK) {
		printf("net_path(%u) mem_alloc model.bin fail = %d \n", net_path, ret);
		return HD_ERR_FAIL;
	}
	//load file
	loadsize = network_load_model(p_net->net_cfg.model_filename, p_net->proc_mem.va);

	if (loadsize <= 0) {
		printf("net_path(%u) input model load fail: %s \n", net_path, p_net->net_cfg.model_filename);
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
			printf("net_path(%u) vendor_ai3_dev_get(MODEL_INFO) fail=%d \n", net_path, ret);
			return HD_ERR_FAIL;
		}

		printf("model_info get => workbuf size = %d, ronlybuf size = %d \n", model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size, model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);

		// alloc WORKBUF/RONLYBUF
		ret = network_alloc_intl_buf(net_path, model_info.proc_mem.buf[AI3_PROC_BUF_RONLYBUF].size);
		if (ret != HD_OK) {
			printf("net_path(%u) alloc ronlybuf fail = %d \n", net_path, ret);
			return HD_ERR_FAIL;
		}

		// aiisp mode could share workbuf in multi-iso => only first model needs to alloc work buf
		if(net_path == 0 ) {
			ret = network_alloc_io_buf(net_path, model_info.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size);
			if (ret != HD_OK) {
				printf("net_path(%u) alloc workbuf fail=%d\n", net_path, ret);
				return HD_ERR_FAIL;
			}
		}

		ret = network_alloc_input_buf(net_path, 0x10000);
		if (ret != HD_OK) {
			printf("net_path(%u) alloc workbuf fail = %d \n", net_path, ret);
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

		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].pa   = g_net->io_mem.pa;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].va   = g_net->io_mem.va;
		proc_cfg.proc_mem.buf[AI3_PROC_BUF_WORKBUF].size = g_net->io_mem.size;

		proc_cfg.plugin[AI3_PLUGIN_CPU] = vendor_ai_cpu1_get_engine();
		proc_cfg.config[AI3_PROC_CFG_ISP_POOL_ID] = ISP_POOL_ID(0);
		proc_cfg.config[AI3_PROC_CFG_ISP_SHARE] = TRUE;
		#if DBG_OUT_DUMP
		proc_cfg.ctrl           = CTRL_BUF_DEBUG | CTRL_JOB_DEBUG | CTRL_JOB_DUMPOUT;
		#endif
		ret = vendor_ai3_net_open(&p_net->proc_id, &proc_cfg, &p_net->net_info);
		if (ret != HD_OK) {
			printf("net_path(%u) vendor_ai3_net_open() fail = %d \n", net_path, ret);
			return HD_ERR_FAIL;
		} else {
			printf("net_path(%u) open success => get proc_id(%u), in_buf_cnt(%u), out_buf_cnt(%u) \n", net_path, p_net->proc_id, p_net->net_info.in_buf_cnt, p_net->net_info.out_buf_cnt);
		}
	}
	return ret;
}

static HD_RESULT network_close(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;

// close
	ret = vendor_ai3_net_close(proc_id);
	if (ret != HD_OK) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_close fail = %d \n", net_path, proc_id, ret);
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

static HD_RESULT network_bind_cb(VIDEO_AI *p_stream)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai3_dev_get(VENDOR_AI3_CFG_AI_CB, &p_stream->ai_cb);
	if (ret != HD_OK) {
		printf("get ai cb, ret = %d \n", ret);
		return ret;
	}

	ret = vendor_videoproc_set(p_stream->proc_ctrl, VENDOR_VIDEOPROC_PARAM_AI_CB, &p_stream->ai_cb);
	if (ret != HD_OK) {
		printf("set ai cb, ret = %d \n", ret);
		return ret;
	}

	return ret;
}

static HD_RESULT network_bind_isp_cb(VIDEO_AI *p_stream)
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

static HD_RESULT network_set_isp_ai_cfg(VIDEO_AI *p_stream, UINT32 path_id, BOOL enable)
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

static HD_RESULT network_set_buf_by_in_path_list(VIDEO_AI *p_stream, UINT32 para_id)
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
			printf("net_path(%u), proc_id(%u) get in buf fail, i(%d), in_path(0x%lx) \n", p_stream->net_path, proc_id, i,  p_net->net_info.in_path_list[i]);
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
			*(UINT8*)ai_buf.va = aiisp_pq_final_param._2dnr_still_str;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			//printf("set _2dnr_still_str %d\r\n", *(UINT8*)ai_buf.va);
		} else if (i == 3) {
			//motion_2d_str
			*(UINT8*)ai_buf.va = aiisp_pq_final_param.reserved_1;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			//printf("set reserved_1 %d\r\n", *(UINT8*)ai_buf.va);
		} else if (i == 4) {
			//still_2d_str
			*(UINT8*)ai_buf.va = aiisp_pq_final_param.reserved_2;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			//printf("set reserved_2 %d\r\n", *(UINT8*)ai_buf.va);
		} else if (i == 5) {
			//still_texture_2d_str
			*(UINT8*)ai_buf.va = aiisp_pq_final_param.reserved_3;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;

			//printf("set reserved_3 %d\r\n", *(UINT8*)ai_buf.va);
		} else {
			//src_blending
			*(UINT8*)ai_buf.va = (UINT8)aiisp_pq_final_param.reserved_common;
			hd_common_mem_flush_cache((void *)ai_buf.va, sizeof(UINT8));
			ai_buf.size = sizeof(UINT8);
			ai_buf.fmt = HD_VIDEO_PXLFMT_AI_UINT8;
			//printf("set reserved_common %d\r\n", *(UINT8*)ai_buf.va);
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

static HD_RESULT network_start(VIDEO_AI *p_stream)
{
	HD_RESULT ret = HD_OK;

	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;
	ret = vendor_ai3_net_start(proc_id);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_start fail \n", p_stream->net_path, proc_id);
	}

	ret = network_set_buf_by_in_path_list(p_stream, 0);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u), para_id(%u) network_set_buf_by_in_path_list fail \n", p_stream->net_path, proc_id, 0);
	}

	return ret;
}

static HD_RESULT network_stop(VIDEO_AI *p_stream)
{
	HD_RESULT ret = HD_OK;

	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;

	//stop: should be call after last time proc
	ret = vendor_ai3_net_stop(proc_id);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_stop fail \n", p_stream->net_path, proc_id);
	}

	return ret;
}

static HD_RESULT init_module(void)
{
	HD_RESULT ret;

	if ((ret = input_init()) != HD_OK)
		return ret;
	if ((ret = network_init()) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module(VIDEO_AI *p_stream, VIDEO_AI *p_stream2)
{
	HD_RESULT ret;

	if ((ret = network_bind_isp_cb(p_stream)) != HD_OK)
		return ret;

	if ((ret = network_open(p_stream->net_path)) != HD_OK)
		return ret;

	if ((ret = network_open(p_stream2->net_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_AI *p_stream)
{
	HD_RESULT ret;

	if ((ret = network_close(p_stream->net_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;

	if ((ret = input_uninit()) != HD_OK)
		return ret;
	if ((ret = network_uninit()) != HD_OK)
		return ret;

	return HD_OK;
}

//=============================================================================
// external functions
//=============================================================================
HD_RESULT ai_mem_config(void)
{
	HD_RESULT ret = HD_OK;

	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine

	NET_PROC_CONFIG net_cfg = {
		.label_filename = "/mnt/sd/accuracy/labels.txt"
	};

	NET_PROC_CONFIG net_cfg2 = {
		.label_filename = "/mnt/sd/accuracy/labels2.txt"
	};

	snprintf(net_cfg.model_filename, 256, "/mnt/sd/para/%s.bin", ai_model_name[0]);
	snprintf(net_cfg2.model_filename, 256, "/mnt/sd/para/%s.bin", ai_model_name[1]);

	stream[0].net_path = 0;
	stream[0].in_path = 0;
	ret = network_mem_config(stream[0].net_path, &net_cfg);

	stream[2].net_path = 1;
	stream[2].in_path = 1;
	ret = network_mem_config(stream[2].net_path, &net_cfg2);

	return ret;
}

HD_RESULT ai_init(void)
{
	HD_RESULT ret = HD_OK;

	ret = init_module();
	if (ret != HD_OK) {
		printf("ai init fail = %d \n", ret);
	}

	return ret;
}

HD_RESULT ai_open(HD_PATH_ID proc_ctrl)
{
	HD_RESULT ret = HD_OK;

	stream[0].proc_ctrl = proc_ctrl;
	stream[2].proc_ctrl = proc_ctrl;

	ret = open_module(&stream[0], &stream[2]);
	if (ret != HD_OK) {
		printf("ai open fail = %d \n", ret);
	}

	return ret;
}

HD_RESULT ai_start(void)
{
	HD_RESULT ret = HD_OK;

	network_start(&stream[0]);

	network_start(&stream[2]);

	return ret;
}

HD_RESULT ai_set_isp(void)
{
	HD_RESULT ret = HD_OK;

	ret = network_bind_cb(&stream[0]);
	if (ret != HD_OK) {
		printf("network_bind_cb fail = %d \n", ret);
	}

	#if 0  // NOTE: Remove
	ret = network_set_isp_ai_cfg(&stream[0], 0, TRUE);
	if (ret != HD_OK) {
		printf("network_set_isp_ai_cfg fail = %d \n", ret);
	}
	#endif

	return ret;
}

HD_RESULT ai_stop(void)
{
	HD_RESULT ret = HD_OK;

	ret = network_stop(&stream[0]);
	if (ret != HD_OK) {
		printf("ai stop 0 fail = %d \n", ret);
	}

	ret = network_stop(&stream[2]);
	if (ret != HD_OK) {
		printf("ai stop 1 fail = %d \n", ret);
	}

	return ret;
}

HD_RESULT ai_close(void)
{
	HD_RESULT ret = HD_OK;

	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("ai close 0 fail = %d \n", ret);
	}

	ret = close_module(&stream[2]);
	if (ret != HD_OK) {
		printf("ai close 1 fail = %d \n", ret);
	}

	return ret;
}

HD_RESULT ai_exit(void)
{
	HD_RESULT ret = HD_OK;

	ret = exit_module();
	if (ret != HD_OK) {
		printf("ai exit fail = %d \n", ret);
	}

	return ret;
}

HD_RESULT ai_get_model_name1(CHAR *ai_name1)
{
	HD_RESULT ret = HD_OK;

	memcpy(ai_name1, ai_model_name[0], sizeof(ai_model_name) / 2);

	return ret;
}

HD_RESULT ai_set_model_name1(CHAR *ai_name1)
{
	HD_RESULT ret = HD_OK;

	memcpy(ai_model_name[0], ai_name1, sizeof(ai_model_name) / 2);

	return ret;
}

HD_RESULT ai_get_model_name2(CHAR *ai_name2)
{
	HD_RESULT ret = HD_OK;

	memcpy(ai_name2, ai_model_name[1], sizeof(ai_model_name) / 2);

	return ret;
}

HD_RESULT ai_set_model_name2(CHAR *ai_name2)
{
	HD_RESULT ret = HD_OK;

	memcpy(ai_model_name[1], ai_name2, sizeof(ai_model_name) / 2);

	return ret;
}

HD_RESULT ai_model_sel(AI_MODE_SEL sel)
{
	static UINT32 curr_ai_path = 0;
	VIDEO_AI *p_stream;
	HD_RESULT ret = HD_OK;

	last_sel = sel;

	if (curr_ai_path == 0) {
		network_set_buf_by_in_path_list(&stream[0], 0);
	} else {
		network_set_buf_by_in_path_list(&stream[2], 0);
	}

	switch(sel) {
		case AI_MODE_SEL_DISABLE:
			if (curr_ai_path == 0) {
				p_stream = &stream[0];
			} else {
				p_stream = &stream[2];
			}

			ret = network_set_isp_ai_cfg(p_stream, 0, FALSE);
			if (ret != HD_OK) {
				printf("set isp_ai disable fail = %d \n", ret);
				return HD_ERR_NG;
			}
		break;

		case AI_MODE_SEL_0:
		default:
			p_stream = &stream[0];
			curr_ai_path = 0;

			ret = network_set_isp_ai_cfg(p_stream, 0, TRUE);
			if (ret != HD_OK) {
				printf("set isp_ai fail=%d\n", ret);
				return HD_ERR_NG;
			}
		break;

		case AI_MODE_SEL_1:
			p_stream = &stream[2];
			curr_ai_path = 1;
			
			ret = network_set_isp_ai_cfg(p_stream, 0, TRUE);
			if (ret != HD_OK) {
				printf("set isp_ai fail=%d\n", ret);
				return HD_ERR_NG;
			}
		break;
	}

	return ret;
}

HD_RESULT ai_set_param(AIISP_PQ_FINAL_PARAM *param)
{
	HD_RESULT ret = HD_OK;

	memcpy(&aiisp_pq_final_param, param, sizeof(AIISP_PQ_FINAL_PARAM));

	return ret;
}

HD_RESULT ai_model_trig(void)
{
	HD_RESULT ret = HD_OK;

	if (last_sel != AI_MODE_SEL_DISABLE) {
		ai_model_sel(last_sel);
	}

	return ret;
}

