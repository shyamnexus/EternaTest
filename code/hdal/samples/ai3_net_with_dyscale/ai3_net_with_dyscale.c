/**
	@brief Source file of vendor ai net sample code.

	@file ai3_net_with_dyscale.c

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
#include "hdal.h"
#include "hd_type.h"
#include "hd_debug.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#if defined(_NVT_NVR_SDK_)
#include <comm/nvtmem_if.h>
#include <sys/ioctl.h>
#endif
#if defined(_BSP_NA51068_) || defined(_BSP_NA51090_)
#include "vendor_common.h"
#endif

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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai3_net_with_dyscale, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DUMP_POSTPROC_INFO			0
#define DBG_OUT_DUMP                0 // debug mode, dump output iobuf
#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
///////////////////////////////////////////////////////////////////////////////

#define NET_PATH_ID		UINT32

#define VENDOR_AI_CFG  				0x000f0000  //vendor ai config

#define AI_RGB_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))
#define MULTI_BLOB_IN    0

#define VENDOR_AIS_LBL_LEN      256     ///< maximal length of class label
#define MAX_CLASS_NUM           1000
#define SET_DUMP_DIR_MODE       1
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

typedef struct _MEM_PARM {
	UINTPTR pa;
	UINTPTR va;
	UINT32 size;
	UINTPTR blk;
} MEM_PARM;


/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/

static char dump_dir[256];
static int dump_idx = 0;
extern HD_RESULT _vendor_ai_net_debug_layer(UINT32 proc_id, UINT32 layer_opt, CHAR *filedir);
static int ai_test_debug_dumpout(void)
{
#if SET_DUMP_DIR_MODE
#else
	snprintf(dump_dir, 256 - 1, "/mnt/sd/output_idx%d", dump_idx);
#endif
	_vendor_ai_net_debug_layer(0, 1, dump_dir);
	printf("!!## dump_layer done ##!!\n");
	dump_idx++;
	return 1;
}

static HD_RESULT mem_alloc(MEM_PARM *mem_parm, CHAR* name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	UINTPTR pa   = 0;
	void  *va   = NULL;

	//alloc private pool
	ret = hd_common_mem_alloc(name, &pa, (void**)&va, size, DDR_ID0);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa   = pa;
	mem_parm->va   = (UINTPTR)va;
	mem_parm->size = size;
	mem_parm->blk  = (UINTPTR)-1;

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


static INT32 mem_load(MEM_PARM *mem_parm, const CHAR *filename)
{
	FILE *fd;
	INT32 size = 0;

	fd = fopen(filename, "rb");

	if (!fd) {
		printf("cannot read %s\r\n", filename);
		size = -1;
		goto exit;
	}

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	// check "ai_in_buf" enough or not
	if (mem_parm->size < (UINT32)size) {
		printf("ERROR: ai_in_buf(%u) is not enough, input file(%u)\r\n", mem_parm->size, (UINT32)size);
		size = -1;
		goto exit;
	}
	
	if (size < 0) {
		printf("getting %s size failed\r\n", filename);
		goto exit;
	} else if ((INT32)fread((VOID *)mem_parm->va, 1, size, fd) != size) {
		printf("read size < %ld\r\n", size);
		size = -1;
		goto exit;
	}
	mem_parm->size = size;

	// we use cpu to read memory, which needs to deal cache flush.
	if(hd_common_mem_flush_cache((VOID *)mem_parm->va, mem_parm->size) != HD_OK) {
        printf("flush cache failed.\r\n");
    }

exit:
	if (fd) {
		fclose(fd);
	}

	return size;
}

/*-----------------------------------------------------------------------------*/
/* Input Functions                                                             */
/*-----------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////

typedef struct _NET_IN_CONFIG {

	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 b;
	UINT32 bitdepth;
	UINT32 loff;
	UINT32 fmt;
	UINT32 is_comb_img;  // 1: image image (or feature-in) is a combination image (or feature-in).
	
} NET_IN_CONFIG;

typedef struct _NET_IN {

	NET_IN_CONFIG in_cfg;
	MEM_PARM input_mem;
	UINT32 in_id;
	VENDOR_AI3_BUF src_img;
		
} NET_IN;

static NET_IN *g_in = NULL;

static UINT32 _calc_ai_buf_size(UINT32 loff, UINT32 h, UINT32 c, UINT32 b, UINT32 bitdepth, UINT32 fmt)
{
	UINT size = 0;

	switch (fmt) {
	case HD_VIDEO_PXLFMT_YUV420:
		{
			size = loff * h * 3 / 2;
		}
		break;
	case HD_VIDEO_PXLFMT_RGB888_PLANAR:
		{
			size = AI_RGB_BUFSIZE(loff, h);
		}
		break;
	case HD_VIDEO_PXLFMT_BGR888_PLANAR:
		{
			size = AI_RGB_BUFSIZE(loff, h);
		}
		break;
	default: // feature-in
		{
			size = loff * h * c;
		}
		break;
	}

	if (!size) {
		printf("ERROR!! ai_buf size = 0\n");
	}
	return size;
}

static HD_RESULT _load_buf(MEM_PARM *mem_parm, CHAR *filename, VENDOR_AI3_BUF *p_buf, UINT32 w, UINT32 h, UINT32 c, UINT32 b, UINT32 bitdepth, UINT32 loff, UINT32 fmt)
{
	INT32 file_len;

	file_len = mem_load(mem_parm, filename);
	if (file_len < 0) {
		printf("load buf(%s) fail\r\n", filename);
		return HD_ERR_NG;
	}
	printf("load buf(%s) ok\r\n", filename);
	p_buf->width = w;
	p_buf->height = h;
	p_buf->channel = c;
	p_buf->batch_num = b;
	p_buf->line_ofs = loff;
	p_buf->fmt = fmt;
	p_buf->pa   = mem_parm->pa;
	p_buf->va   = mem_parm->va;
	p_buf->sign = MAKEFOURCC('A','B','U','F');
	p_buf->size = _calc_ai_buf_size(loff, h, c, b, bitdepth, fmt);
	if (p_buf->size == 0) {
		printf("load buf(%s) fail, p_buf->size = 0\r\n", filename);
		return HD_ERR_NG;
	}

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


static HD_RESULT input_set_config(NET_PATH_ID in_path, NET_IN_CONFIG* p_in_cfg)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + in_path;
	UINT32 in_id = p_net->in_id;
	
	memcpy((void*)&p_net->in_cfg, (void*)p_in_cfg, sizeof(NET_IN_CONFIG));
	printf("in_path(%u) in_id(%u) set in_cfg: file(%s), buf=(%u,%u,%u,%u,%u,%u,%08x)\r\n", 
		in_path,
		in_id,
		p_net->in_cfg.input_filename,
		p_net->in_cfg.w,
		p_net->in_cfg.h,
		p_net->in_cfg.c,
		p_net->in_cfg.b,
		p_net->in_cfg.bitdepth,
		p_net->in_cfg.loff,
		p_net->in_cfg.fmt);
	
	return ret;
}

static INT32 get_file_size(const CHAR *filename)
{
	FILE *fd;
	INT32 size = 0;
	fd = fopen(filename, "rb");
	if (!fd) {
		printf("cannot read %s\r\n", filename);
		size = -1;
		return 0;
	}
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	return size;
}

static HD_RESULT input_open(NET_PATH_ID in_path)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + in_path;
	UINT32 in_id = p_net->in_id;
	UINT32 buf_size = 0, file_size = 0;
	CHAR mem_name[23] ;
	snprintf(mem_name, 23, "ai_in_buf %u", in_id); 

	// calculate in buf size
	buf_size = _calc_ai_buf_size(p_net->in_cfg.loff, p_net->in_cfg.h, p_net->in_cfg.c, p_net->in_cfg.b, p_net->in_cfg.bitdepth, p_net->in_cfg.fmt);
	if (buf_size == 0) {
		printf("in_path(%lu) calc buf size 0.\r\n", in_path);
		return HD_ERR_FAIL;
	}
	file_size = get_file_size(p_net->in_cfg.input_filename);
	if (file_size == 0) {
		printf("in_path(%lu) in_id(%u) size == 0 \r\n", in_path, in_id);
		return HD_ERR_FAIL;
	}
	if (buf_size < file_size) {
		buf_size = file_size;
	}
	ret = mem_alloc(&p_net->input_mem, mem_name, buf_size * p_net->in_cfg.b);
	if (ret != HD_OK) {
		printf("in_path(%lu) in_id(%u) alloc ai_in_buf fail\r\n", in_path, in_id);
		return HD_ERR_FAIL;
	}
	printf("alloc_in_buf: pa = 0x%lx, va = 0x%lx, size = %u\n", p_net->input_mem.pa, p_net->input_mem.va, p_net->input_mem.size);

	ret = _load_buf(&p_net->input_mem, 
		p_net->in_cfg.input_filename, 
		&p_net->src_img,
		p_net->in_cfg.w,
		p_net->in_cfg.h,
		p_net->in_cfg.c,
		p_net->in_cfg.b,
		p_net->in_cfg.bitdepth,
		p_net->in_cfg.loff,
		p_net->in_cfg.fmt);
	if (ret != HD_OK) {
		printf("in_path(%u) in_id(%u) input_open fail=%d\n", in_path, in_id, ret);
	}

	return ret;
}


static HD_RESULT input_close(NET_PATH_ID in_path)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + in_path;

	mem_free(&p_net->input_mem);
	
	return ret;
}

static HD_RESULT input_start(NET_PATH_ID in_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT input_stop(NET_PATH_ID in_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT input_pull_buf(NET_PATH_ID in_path, VENDOR_AI3_BUF *p_in, INT32 wait_ms)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + in_path;
	
	memcpy((void*)p_in, (void*)&(p_net->src_img), sizeof(VENDOR_AI3_BUF));
	return ret;
}
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Network Functions                                                             */
/*-----------------------------------------------------------------------------*/

typedef struct _NET_PROC_CONFIG {

	CHAR model_filename[256];
	INT32 binsize;
	int job_method;
	int job_wait_ms;
	int buf_method;
	void *p_share_model;

	CHAR label_filename[256];

} NET_PROC_CONFIG;

typedef struct _NET_PROC {

	NET_PROC_CONFIG net_cfg;
	MEM_PARM proc_mem;
	UINT32 proc_id;
	
	CHAR out_class_labels[MAX_CLASS_NUM * VENDOR_AIS_LBL_LEN];
	MEM_PARM rslt_mem;
	MEM_PARM io_mem;
	MEM_PARM intl_mem;
    MEM_PARM *out_mem;
	VENDOR_AI3_NET_INFO net_info;
		
} NET_PROC;

static NET_PROC *g_net = NULL;

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
	//DBG_DUMP("model addr = %#lx\r\n", model_addr);

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
	
	printf("load model(%s) size(%d) ok\r\n", filename, read_size);
	return read_size;
}

static HD_RESULT _load_label(UINTPTR addr, UINT32 line_len, const CHAR *filename)
{
	FILE *fd;
	CHAR *p_line = (CHAR *)addr;

	fd = fopen(filename, "r");
	if (!fd) {
		printf("load label(%s) fail\r\n", filename);
		return HD_ERR_NG;
	}

	while (fgets(p_line, line_len, fd) != NULL) {
		p_line[strlen(p_line) - 1] = '\0'; // remove newline character
		p_line += line_len;
	}

	if (fd) {
		fclose(fd);
	}

	printf("load label(%s) ok\r\n", filename);
	return HD_OK;
}

static HD_RESULT network_init(void)
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

static HD_RESULT network_uninit(void)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		printf("vendor_ai3_dev_uninit fail=%d\n", ret);
	}
	
	return ret;
}

static INT32 network_mem_config(NET_PATH_ID net_path, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg)
{
	NET_PROC* p_net = g_net + net_path;
	NET_PROC_CONFIG* p_proc_cfg = (NET_PROC_CONFIG*)p_cfg;
#if defined(_NVT_NVR_SDK_)
	int sys_fd;
	struct nvtmem_hdal_base sys_hdal;
	uintptr_t hdal_start_addr0, hdal_start_addr1;

	sys_fd = open("/dev/nvtmem0", O_RDWR);
	if (sys_fd < 0) {
		printf("Error: cannot open /dev/nvtmem0 device.\n");
		exit(0);
	}
	if (ioctl(sys_fd, NVTMEM_GET_DTS_HDAL_BASE, &sys_hdal) < 0) {
		printf("PCIE_SYS_IOC_HDALBASE! \n");
		close(sys_fd);
		exit(0);
	}
	close(sys_fd);

	/* init ddr0 user_blk */
	hdal_start_addr0 = sys_hdal.base[0];
	p_mem_cfg->pool_info[0].start_addr = hdal_start_addr0;
	p_mem_cfg->pool_info[0].blk_cnt = 1;
	p_mem_cfg->pool_info[0].blk_size = 200 * 1024 * 1024;
	p_mem_cfg->pool_info[0].type = HD_COMMON_MEM_USER_BLK;
	p_mem_cfg->pool_info[0].ddr_id = sys_hdal.ddr_id[0];
	printf("create ddr%d: hdal_memory(%#lx, %ldKB), usr_blk(%#lx, %dKB)\n", p_mem_cfg->pool_info[0].ddr_id,
			hdal_start_addr0, sys_hdal.size[0] / 1024, p_mem_cfg->pool_info[0].start_addr,
			p_mem_cfg->pool_info[0].blk_size * p_mem_cfg->pool_info[0].blk_cnt / 1024);

	/* init ddr1 user_blk, if ddr1 is exist */
	if (sys_hdal.size[1] != 0) {
		hdal_start_addr1 = sys_hdal.base[1];
		p_mem_cfg->pool_info[1].start_addr = hdal_start_addr1;
		p_mem_cfg->pool_info[1].blk_cnt = 1;
		p_mem_cfg->pool_info[1].blk_size = 200 * 1024 * 1024;
		p_mem_cfg->pool_info[1].type = HD_COMMON_MEM_USER_BLK;
		p_mem_cfg->pool_info[1].ddr_id = sys_hdal.ddr_id[1];
		printf("create ddr%d: hdal_memory(%#lx, %ldKB) usr_blk(%#lx, %dKB)\n", p_mem_cfg->pool_info[1].ddr_id,
				hdal_start_addr1, sys_hdal.size[1] / 1024, p_mem_cfg->pool_info[1].start_addr,
				p_mem_cfg->pool_info[1].blk_size * p_mem_cfg->pool_info[1].blk_cnt / 1024);
	} else {
		printf("create ddr1: hdal_memory(%#lx, %ldKB) is not exist\n", sys_hdal.base[1], sys_hdal.size[1] / 1024);
	}
	usleep(30000); // wait for printf completely
#endif
	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));
	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("net_path(%u) input model is null\r\n", net_path);
		return HD_ERR_NG;
	}

	/* config nvt_model.bin
	 * use common user pool */
	p_net->net_cfg.binsize = _getsize_model(p_net->net_cfg.model_filename);
	if (p_net->net_cfg.binsize <= 0) {
		printf("net_path(%u) input model is not exist?\r\n", net_path);
		return HD_ERR_NG;
	}
	
	printf("net_path(%u) set net_mem_cfg: model-file(%s), binsize=%d\r\n",
		net_path,
		p_net->net_cfg.model_filename,
		p_net->net_cfg.binsize);

	printf("net_path(%u) set net_mem_cfg: label-file(%s)\r\n",
		net_path,
		p_net->net_cfg.label_filename);

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


static HD_RESULT network_open(NET_PATH_ID net_path)
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

	// load label
	ret = _load_label((UINTPTR)p_net->out_class_labels, VENDOR_AIS_LBL_LEN, p_net->net_cfg.label_filename);
	if (ret != HD_OK) {
		printf("net_path(%u) load_label fail=%d\n", net_path, ret);
		return HD_ERR_FAIL;
	}


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
#if DBG_OUT_DUMP
		proc_cfg.ctrl           = CTRL_BUF_DEBUG | CTRL_JOB_DEBUG | CTRL_JOB_DUMPOUT;
#endif

		ret = vendor_ai3_net_open(&p_net->proc_id, &proc_cfg, &p_net->net_info);
		if (ret != HD_OK) {
			printf("net_path(%u) vendor_ai3_net_open() fail=%d\n", net_path, ret);
			return HD_ERR_FAIL;
		} else {
			printf("net_path(%u) open success => get proc_id(%u)\r\n", net_path, p_net->proc_id);
		}		
	}
	
	return ret;
}

static HD_RESULT network_close(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
    UINT32 i ;

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

	mem_free(&p_net->proc_mem);

	for (i = 0 ; i < p_net->net_info.out_buf_cnt; i++){
		if(p_net->out_mem && p_net->out_mem[i].va)
			mem_free(&p_net->out_mem[i]);
	}
	if(p_net->out_mem)
		free(p_net->out_mem);
    
    memset(&p_net->net_info, 0, sizeof(VENDOR_AI3_NET_INFO));

	return ret;
}
#if DUMP_POSTPROC_INFO
static HD_RESULT network_dump_ai_buf(VENDOR_AI3_BUF *p_outbuf)
{
	HD_RESULT ret = HD_OK;
	printf("dump ai_outbuf:\n");
	printf("   sign(0x%x)\n", p_outbuf->sign);
	printf("   pa(0x%lx)\n", p_outbuf->pa);
	printf("   va(0x%lx)\n", p_outbuf->va);
	printf("   size(%lu)\n", p_outbuf->size);
	printf("   width(%lu)\n", p_outbuf->width);
	printf("   height(%lu)\n", p_outbuf->height);
	printf("   channel(%lu)\n", p_outbuf->channel);
	printf("   batch_num(%lu)\n", p_outbuf->batch_num);
	printf("   line_ofs(%lu)\n", p_outbuf->line_ofs);
	printf("   channel_ofs(%lu)\n", p_outbuf->channel_ofs);
	printf("   batch_ofs(%lu)\n", p_outbuf->batch_ofs);
	printf("   time_ofs(%lu)\n", p_outbuf->time_ofs);
	printf("   layout(%s)\n", p_outbuf->layout);
	printf("   name(%s)\n", p_outbuf->name);
	printf("   scale_ratio(%.6f)\n", p_outbuf->scale_ratio);
	printf("\n");

	// parsing pixel format
	printf("parsing pixel format:\n");
	printf("   fmt(0x%lx)\n", p_outbuf->fmt);
	switch (HD_VIDEO_PXLFMT_TYPE(p_outbuf->fmt))) {
	case HD_VIDEO_PXLFMT_AI_UINT8:
	case HD_VIDEO_PXLFMT_AI_SINT8:
	case HD_VIDEO_PXLFMT_AI_UINT16:
	case HD_VIDEO_PXLFMT_AI_SINT16:
	case HD_VIDEO_PXLFMT_AI_UINT32:
	case HD_VIDEO_PXLFMT_AI_SINT32:
		{
			INT8 int_bits = HD_VIDEO_PXLFMT_INT(p_outbuf->fmt);
			INT8 frac_bits = HD_VIDEO_PXLFMT_FRAC(p_outbuf->fmt);
			printf("   int_bits(0x%x)\n", int_bits);
			printf("   frac_bits(0x%x)\n", frac_bits);
		}
		break;
	case HD_VIDEO_PXLFMT_AI_FLOAT32:
		{
		}
		break;
	default:
		switch ((UINT32)p_outbuf->fmt) {
		case HD_VIDEO_PXLFMT_BGR888_PLANAR:
			{
				printf("   -> BGR888_PLANAR\n");
			}
			break;
		case HD_VIDEO_PXLFMT_UV:
			{
				printf("   -> UV only\n");
			}
			break;
		case 0:
			{
				printf("   -> AI BUF\n");
			}
			break;
		default:
			printf("unknown pxlfmt(0x%lx)\n", p_outbuf->fmt);
			break;
		}
	}
	printf("\n");

	return ret;
}
#endif
///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_LIVEVIEW {

	// (1) input 
	NET_IN_CONFIG net_in_cfg;
	NET_PATH_ID in_path;

	// (2) network 
	NET_PROC_CONFIG net_proc_cfg;
	NET_PATH_ID net_path;
	pthread_t  proc_thread_id;
	UINT32 proc_start;
	UINT32 proc_exit;
	UINT32 proc_oneshot;
	UINT32 input_blob_num;

} VIDEO_LIVEVIEW;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = input_init()) != HD_OK)
		return ret;
	if ((ret = network_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	//if ((ret = input_open(p_stream->in_path)) != HD_OK)
	//	return ret;
	if ((ret = network_open(p_stream->net_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	//if ((ret = input_close(p_stream->in_path)) != HD_OK)
	//	return ret;
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
///////////////////////////////////////////////////////////////////////////////

static VOID *network_user_thread(VOID *arg);

static HD_RESULT set_buf_by_in_path_list(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	NET_PATH_ID net_path = p_stream->net_path;
	NET_PROC* p_net = g_net + net_path;
	VENDOR_AI3_NET_INFO net_info = p_net->net_info;
	VENDOR_AI3_BUF in_buf = {0};
	VENDOR_AI3_BUF tmp_buf = {0};
	UINT32 proc_id = p_net->proc_id;
	UINT32 i = 0,  idx = 0, k = 0;
	UINT32 in_buf_cnt = 0;
	UINT32 *in_path_list = NULL;

	
	/* get in path list */
	in_path_list = p_net->net_info.in_path_list;
	in_buf_cnt = net_info.in_buf_cnt;
	
	for (i = 0; i < in_buf_cnt; i++) {
		/* get in buf (by in path list) */
		ret = vendor_ai3_net_get(proc_id, in_path_list[i], &tmp_buf);
		if (HD_OK != ret) {
			printf("net_path(%u), proc_id(%u) get in buf fail, i(%d), in_path(0x%lx)\n",net_path, proc_id, i, in_path_list[i]);
			goto exit;
		}

#if DUMP_POSTPROC_INFO
		// dump in buf
		printf("dump_in_buf: path_id: 0x%lx\n", in_path_list[i]);
		ret = network_dump_ai_buf(&tmp_buf);
		if (HD_OK != ret) {
			printf("net_path(%u), proc_id(%u) dump in buf fail !!\n", net_path, proc_id);
			goto exit;
		}
#endif
	}

	for (i = 0; i < p_stream->input_blob_num; i++) {
		NET_IN* p_in = g_in + i;

		/* load input bin */
		ret = input_pull_buf((p_stream->in_path + i), &in_buf, 0);
		if (HD_OK != ret) {
			printf("in_path(%u) pull input fail !!\n", (p_stream->in_path + i));
			goto exit;
		}

		if (p_in->in_cfg.is_comb_img == 0) { 
			for(k =1 ; k < in_buf.batch_num; k++) {
				UINTPTR dst_va = in_buf.va + (k * in_buf.size);
				memcpy((VOID*)dst_va , (VOID*)in_buf.va, in_buf.size);
				hd_common_mem_flush_cache((VOID *)dst_va, in_buf.size);
			}
		}
	

			printf(" path_%d(0x%x) pa(0x%lx) va(0x%lx) size(%u) whcb(%d,%d,%d,%d)\n",
					idx, in_path_list[idx], in_buf.pa, in_buf.va, in_buf.size, in_buf.width, in_buf.height, in_buf.channel, in_buf.batch_num);
			ret = vendor_ai3_net_set(proc_id, in_path_list[idx], &in_buf);
			if (HD_OK != ret) {
				printf("proc_id(%u)push input fail !! i(%lu)\n", proc_id, i);
				goto exit;
			}
			idx++;
		
	
	}

exit:
	return ret;
}

static HD_RESULT set_buf_by_out_path_list(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	UINT32 i;
	VENDOR_AI3_BUF ai_buf = {0};

	/* get out buf */
	for (i = 0; i < p_net->net_info.out_buf_cnt; i++) {
		// get out buf (by out path list)
		ret = vendor_ai3_net_get(proc_id, p_net->net_info.out_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u) get out buf fail, i(%d), out_path(0x%lx)\n", proc_id, i, p_net->net_info.out_path_list[i]);
			goto exit;
		}
		if (ai_buf.size > p_net->out_mem[i].size){
			printf("output size %u < ai_buf.size %u\r\n", p_net->out_mem[i].size, ai_buf.size);
			goto exit;
		}

		ai_buf.va = p_net->out_mem[i].va ;
		ai_buf.pa = p_net->out_mem[i].pa ;
		ai_buf.size = p_net->out_mem[i].size ;

		ret = vendor_ai3_net_set(proc_id, p_net->net_info.out_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u)set output buf fail !! (%lu)\n", proc_id, i);
			goto exit;
		}
	}

exit:
	return ret;
}

static HD_RESULT get_buf_by_out_path_list(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	UINT32 i;
	VENDOR_AI3_BUF ai_buf = {0}; 

	

	/* get out buf */
	for (i = 0; i < p_net->net_info.out_buf_cnt; i++) {
		// get out buf (by out path list)
		ret = vendor_ai3_net_get(proc_id, p_net->net_info.out_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("net_path(%u), proc_id(%u) get out buf fail, i(%d), out_path(0x%lx)\n", net_path, proc_id, i,  p_net->net_info.out_path_list[i]);
			goto exit;
		}
#if DUMP_POSTPROC_INFO
		// dump out buf
		if (hd_common_mem_flush_cache((VOID *)ai_buf.va, ai_buf.size) != HD_OK) {
			printf("flush cache failed.\r\n");
		}
		printf("dump_out_buf: path_id: 0x%lx\n", p_net->net_info.out_path_list[i]);
		ret = network_dump_ai_buf(&ai_buf);
		if (HD_OK != ret) {
			printf("net_path(%u) dump out buf fail !!\n", net_path);
			goto exit;
		}
#endif
	}

exit:
	return ret;
}

static HD_RESULT allocate_buf_by_out_path_list(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	UINT32 i;
	VENDOR_AI3_BUF ai_buf = {0};
	CHAR mem_name[23] ;


	/* get out path list */
	p_net->out_mem = (MEM_PARM *)malloc(sizeof(MEM_PARM) * p_net->net_info.out_buf_cnt);
	memset(p_net->out_mem, 0, sizeof(MEM_PARM) * p_net->net_info.out_buf_cnt);

	/* get out buf */
	for (i = 0; i < p_net->net_info.out_buf_cnt; i++) {
		// get out buf (by out path list)
		ret = vendor_ai3_net_get(proc_id, p_net->net_info.out_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u) get out buf fail, i(%d), out_path(0x%lx)\n", proc_id, i, p_net->net_info.out_path_list[i]);
			goto exit;
		}

		// // allocate in buf
		snprintf(mem_name, 23, "output_buf %u", i);
		ret = mem_alloc(&p_net->out_mem[i], mem_name,  ai_buf.size);
		if (ret != HD_OK) {
			printf("proc_id(%lu) alloc ai_in_buf fail\r\n", proc_id);
			goto exit;
		}
		printf("alloc_outbuf: pa = 0x%lx, va = 0x%lx, size = %u\n", p_net->out_mem[i].pa, p_net->out_mem[i].va, p_net->out_mem[i].size);
	}
	return ret;
exit:
	for (i = 0 ; i < p_net->net_info.out_buf_cnt; i++){
		if(p_net->out_mem && p_net->out_mem[i].va)
			mem_free(&p_net->out_mem[i]);
	}
	if(p_net->out_mem)
		free(p_net->out_mem);

	return ret;
}

static HD_RESULT network_user_start(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;

	p_stream->proc_start = 0;
	p_stream->proc_exit = 0;
	p_stream->proc_oneshot = 0;

	ret = vendor_ai3_net_start(proc_id);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_start fail !!\n", p_stream->net_path, proc_id);
	}
	
	ret = pthread_create(&p_stream->proc_thread_id, NULL, network_user_thread, (VOID*)(p_stream));
	if (ret < 0) {
		return HD_ERR_FAIL;
	}

	p_stream->proc_start = 1;
	p_stream->proc_exit = 0;
	p_stream->proc_oneshot = 0;
	
	return ret;
}

static HD_RESULT network_user_oneshot(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	p_stream->proc_oneshot = 1;
	return ret;
}

static HD_RESULT network_user_stop(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;

	p_stream->proc_exit = 1;
	
	if (p_stream->proc_thread_id) {
		pthread_join(p_stream->proc_thread_id, NULL);
	}

	//stop: should be call after last time proc
	ret = vendor_ai3_net_stop(proc_id);
	if (HD_OK != ret) {
		printf("net_path(%u), proc_id(%u) vendor_ai3_net_stop fail !!\n", p_stream->net_path, proc_id);
	}
	
	return ret;
}

static VOID *network_user_thread(VOID *arg)
{
	HD_RESULT ret = HD_OK;
	
	VIDEO_LIVEVIEW *p_stream = (VIDEO_LIVEVIEW*)arg;
	NET_PROC* p_net = g_net + p_stream->net_path;
	UINT32 proc_id = p_net->proc_id;

	while (p_stream->proc_start == 0) sleep(1);

	while (p_stream->proc_exit == 0) {

		if (p_stream->proc_oneshot) {
			p_stream->proc_oneshot = 0;

			// set buf by in_path_list
			ret = set_buf_by_in_path_list(p_stream);
			if (HD_OK != ret) {
				printf("net_path(%u), proc_id(%u) set in_buf fail(%d) !!\n", p_stream->net_path, proc_id, ret);
				goto skip;
			}

            printf("net_path(%u), proc_id(%u) oneshot ...\n", p_stream->net_path, proc_id);
            
            ret = set_buf_by_out_path_list(p_stream->net_path);
			if (HD_OK != ret) {
				printf("net_path(%u), proc_id(%u) set in_buf fail(%d) !!\n", p_stream->net_path, proc_id, ret);
				goto skip;
			}

			// do net proc
			ret = vendor_ai3_net_proc(proc_id);
			if (HD_OK != ret) {
				printf("net_path(%u), proc_id(%u) proc fail !!\n", p_stream->net_path, proc_id);
				goto skip;
			}
			
			printf("net_path(%u), proc_id(%u) oneshot done!\n", p_stream->net_path, proc_id);
			
			// get buf by out_path_list
			ret = get_buf_by_out_path_list(p_stream->net_path);
			if (HD_OK != ret) {
				printf("net_path(%u), proc_id(%u) get out_buf fail(%d) !!\n", p_stream->net_path, proc_id, ret);
				goto skip;
			}
		}
		usleep(100);
	}
	
skip:

	return 0;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
MAIN(argc, argv)
{
	VIDEO_LIVEVIEW stream[1] = {0}; //0: net proc
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	INT32 idx;
	HD_RESULT ret;
	INT key;
	UINT32 j;

#if MULTI_BLOB_IN
	/* multi-blob with multi-batch */
	stream[0].input_blob_num = 2;

	//net_in
	NET_IN_CONFIG in_cfg = {
		.input_filename = "/mnt/sd/jpg/YUV420_SP_W512H376.bin",
		.w = 512,
		.h = 376,
		.c = 2,
		.b = 1,
		.loff = 512,
		.fmt = HD_VIDEO_PXLFMT_YUV420
	};
	NET_IN_CONFIG in_cfg2 = {
		.input_filename = "/mnt/sd/jpg/YUV420_SP_W512H376.bin",
		.w = 512,
		.h = 376,
		.c = 2,
		.b = 1,
		.loff = 512,
		.fmt = HD_VIDEO_PXLFMT_YUV420
	};

	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "/mnt/sd/para/multi_scale/nvt_model_mblob.bin",
		.label_filename = "/mnt/sd/accuracy/labels.txt"
	};
#else
	
	stream[0].input_blob_num = 1;

	//net_in
	NET_IN_CONFIG in_cfg = {
		.input_filename = "/mnt/sd/jpg/RGB_W1920H1080.bin",
		.w = 1920,
		.h = 1080,
		.c = 3,
		.b = 1,
		.loff = 1920,
		.fmt = 0x2b180888,
	};
	NET_IN_CONFIG in_cfg2 = {0};
	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "/mnt/sd/para/multi_scale/nvt_model.bin",
		.label_filename = "/mnt/sd/accuracy/labels.txt"
	};
	
#endif
	

	idx = 1;

	// parse network config
	
#if SET_DUMP_DIR_MODE
    if (argc > idx) {
		strncpy(dump_dir, argv[idx++], 256-1);
		dump_dir[256-1] = '\0';
	} else {
        snprintf(dump_dir, 256 - 1, "/mnt/sd/output_idx%d", dump_idx);
    }
#else	
	if (argc > idx) {
		strncpy(in_cfg.input_filename, argv[idx++], 256-1);
		in_cfg.input_filename[256-1] = '\0';
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%lu", &in_cfg.w);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%lu", &in_cfg.h);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%lu", &in_cfg.c);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%lu", &in_cfg.loff);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%x", (unsigned int *)&in_cfg.fmt);
	}
	if (argc > idx) {
		strncpy(net_cfg.model_filename, argv[idx++], 256-1);
		net_cfg.model_filename[256-1] = '\0';
	}
#endif
	printf("\r\n\r\n");

	// malloc for g_in & g_net
	g_in  = (NET_IN *)malloc(sizeof(NET_IN)*16);
	g_net = (NET_PROC *)malloc(sizeof(NET_PROC)*16);
	if ((g_in == NULL) || (g_net == NULL)) {
		printf("fail to malloc g_in/g_net\n");
		goto exit;
	}

	stream[0].in_path = 0;
	stream[0].net_path = 0;
	
	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("hd_common_init fail=%d\n", ret);
		goto exit;
	}
//this is no longer need in 690
/*
#if defined(_BSP_NS02201_) || defined(_BSP_NS02302_)
	// set project config for AI
	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine
#endif
*/


	// init mem
	{
		// config common pool
		network_mem_config(stream[0].net_path, &mem_cfg, &net_cfg);
	}
#if defined(_BSP_NS02201_) || defined(_BSP_NS02302_)
	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		goto exit;
	}
#endif
	
	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	// set open config
	for (j=0; j < stream[0].input_blob_num; j++) {
		if (j == 0) {
			ret = input_set_config((stream[0].in_path + j), &in_cfg);
		} else {
			ret = input_set_config((stream[0].in_path + j), &in_cfg2);
		}
		if (HD_OK != ret) {
			printf("in_path(%u) input_set_config fail=%d\n", (stream[0].in_path + j), ret);
			goto exit;
		}
	}
	
	// open video_liveview modules
	for(j=0; j < stream[0].input_blob_num; j++) {
		ret = input_open((stream[0].in_path + j));
		if (ret != HD_OK) {
			printf("in_path(%u) input open fail=%d\n", (stream[0].in_path + j), ret);
			goto exit;
		}
	}
	ret = open_module(&stream[0]);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

    // start
	input_start(stream[0].in_path);
	network_user_start(&stream[0]);
	allocate_buf_by_out_path_list(stream[0].net_path);

	printf("Enter q to quit\n");
	printf("Enter r to run once\n");
    printf("Enter y to set resolution width, height (immediately)\n");
    printf("Enter d to dump result_layer bin\n");
	do {
		key = GETCHAR();
		if (key == 'r') {
		
			// run once
			network_user_oneshot(&stream[0]);
			continue;
		}

        if (key == 'y') {
            HD_DIM dim = {0};
            NET_PROC* p_net = g_net + stream[0].net_path;
            printf("Enter resolution width, height: \n");
            printf("Input width:\n");
            scanf("%u", &dim.w);
            printf("Input height:\n");
            scanf("%u", &dim.h);
            vendor_ai3_net_set(p_net->proc_id, VENDOR_AI3_NET_PARAM_SHAPE_DIM_IMM, &dim);
            printf("set VENDOR_AI3_NET_PARAM_SHAPE_DIM_IMM dim(%ux%u)\n", dim.w, dim.h);
            continue;
		}
		
		if (key == 'd') {
			printf("dump\n");
			ai_test_debug_dumpout();
			continue;
		}
		if (key == 'q' || key == 0x3) {
			printf("quit\n");
			break;
		}
	} while(1);

    // stop
	input_stop(stream[0].in_path);
	network_user_stop(&stream[0]);

exit:

	// close video_liveview modules
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

    for(j=0; j < stream[0].input_blob_num; j++) {
		ret = input_close((stream[0].in_path + j));
		if (ret != HD_OK) {
			printf("in_path(%u) input close fail=%d\n", (stream[0].in_path + j), ret);
			goto exit;
		}
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

#if defined(_BSP_NS02201_) || defined(_BSP_NS02302_)
	// uninit memory
	ret = hd_common_mem_uninit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}
#endif

	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}

	// free g_in & g_net
	if (g_in)  free(g_in);
	if (g_net) free(g_net);

	return ret;
}
