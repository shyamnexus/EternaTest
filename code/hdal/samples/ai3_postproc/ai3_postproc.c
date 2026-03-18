/**
	@brief Source file of vendor ai net sample code.

	@file ai_op.c

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
#include "hd_debug.h"
#include "vendor_ai.h"
#if defined(_NVT_NVR_SDK_)
#include <comm/nvtmem_if.h>
#include <sys/ioctl.h>
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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai3_postproc, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////

#define NET_PATH_ID		UINT32

#define AI_RGB_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))
#define AI_YUV_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * (h) * 3 / 2))
#define AI_BAYER12_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * (h)))
#define AI_BAYER16_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * (h)))

#define SCALE_DIM_W 384
#define SCALE_DIM_H 282

typedef enum _AI_OP {
    AI_OP_POSTPROC_RGB2YUV          =  0,
	AI_OP_POSTPROC_BGR2YUV          =  1,
	AI_OP_POSTPROC_BAYER122BAYER16  =  2,
	AI_OP_POSTPROC_BAYER162BAYER12  =  3,
	ENUM_DUMMY4WORD(AI_OP)
} AI_OP;
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

typedef struct _MEM_PARM {
	uintptr_t pa;
	uintptr_t va;
	UINT32 size;
} MEM_PARM;

/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/
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

	return HD_OK;
}


static INT32 mem_save(MEM_PARM *mem_parm, const CHAR *filename)
{
	FILE *fd;
	UINT32 size = 0;

	fd = fopen(filename, "wb");

	if (!fd) {
		printf("ERR: cannot open %s for write!\r\n", filename);
		return -1;
	}

	size = (INT32)fwrite((VOID *)mem_parm->va, 1, mem_parm->size, fd);
	if (size != mem_parm->size) {
		printf("ERR: write %s with size %ld < wanted %ld?\r\n", filename, size, mem_parm->size);
	} else {
		printf("write %s with %ld bytes.\r\n", filename, mem_parm->size);
	}

	if (fd) {
		fclose(fd);
	}

	return size;
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
typedef struct _NET_IN_CONFIG {

	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
	
} NET_IN_CONFIG;

typedef struct _NET_IN {

	NET_IN_CONFIG in_cfg;
	//MEM_PARM input_mem;
	UINT32 in_id;
	//VENDOR_AI_BUF src_img;
		
} NET_IN;

static NET_IN *g_in = NULL;

static INT32 input_mem_config(NET_PATH_ID net_path, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	return i;
}

static HD_RESULT input_set_config(NET_PATH_ID net_path, NET_IN_CONFIG* p_in_cfg)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_in = g_in + net_path;
	UINT32 proc_id = p_in->in_id;
	
	memcpy((void*)&p_in->in_cfg, (void*)p_in_cfg, sizeof(NET_IN_CONFIG));
	printf("proc_id(%u) set in_cfg: file(%s), buf=(%u,%u,%u,%u,%08x)\r\n", 
		proc_id,
		p_in->in_cfg.input_filename,
		p_in->in_cfg.w,
		p_in->in_cfg.h,
		p_in->in_cfg.c,
		p_in->in_cfg.loff,
		p_in->in_cfg.fmt);
	
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
typedef struct _OP_PROC {

	UINT32 proc_id;
	int op_opt;
	UINT32 need_size;
	MEM_PARM input_mem;
	MEM_PARM input2_mem;
	MEM_PARM weight_mem;
	MEM_PARM output_mem;
	MEM_PARM output2_mem;
	MEM_PARM work_mem;
	UINT32 op_out_temp;
} OP_PROC;

static OP_PROC *g_op = NULL;

static HD_RESULT operator_set_config(NET_PATH_ID net_path, int in_op_opt)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + net_path;
    p_op->op_opt = in_op_opt;
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_LIVEVIEW {

    // (1) input 
	NET_PATH_ID in_path;
    
	// (2) operator	
	NET_PATH_ID op_path;
    int net_op_opt;
	

} VIDEO_LIVEVIEW;

static HD_RESULT operator_free_buf(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + p_stream->op_path;
	
	// free work buff
	ret = mem_free(&p_op->work_mem);
	if (ret != HD_OK) {
	    printf("free work_mem fail ret = %d\r\n", ret);
	    return HD_ERR_FAIL;
	}

	ret = mem_free(&p_op->output_mem);
	if (ret != HD_OK) {
	    printf("free output_mem fail ret = %d\r\n", ret);
	    return HD_ERR_FAIL;
	}
	if (p_op->output2_mem.va){
		ret = mem_free(&p_op->output2_mem);
		if (ret != HD_OK) {
			printf("free output2_mem fail ret = %d\r\n", ret);
			return HD_ERR_FAIL;
		}
	}

	ret = mem_free(&p_op->input_mem);
	if (ret != HD_OK) {
	    printf("free input_mem fail ret = %d\r\n", ret);
	    return HD_ERR_FAIL;
	}
	if (p_op->input2_mem.va){
		ret = mem_free(&p_op->input2_mem);
		if (ret != HD_OK) {
			printf("free input2_mem fail ret = %d\r\n", ret);
			return HD_ERR_FAIL;
		}
	}
	
	return ret;
}

static HD_RESULT operator_alloc_buf(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + p_stream->op_path;
	NET_IN* p_in = g_in + p_stream->in_path;
	UINT32 proc_id = p_op->proc_id;

	// alloc buffer
	switch (p_op->op_opt) {
		case AI_OP_POSTPROC_RGB2YUV:
		case AI_OP_POSTPROC_BGR2YUV:
		{
			{   // Alloc input buf 
				ret = mem_alloc(&p_op->input_mem, "user_in_buf", AI_RGB_BUFSIZE(p_in->in_cfg.loff, p_in->in_cfg.h));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail ret = %d\r\n", proc_id, ret);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%lu) alloc in_buf pa(%lx) va(%lx), size = %u\r\n", proc_id, p_op->input_mem.pa, p_op->input_mem.va, p_op->input_mem.size);
				
				INT32 file_len;
				file_len = mem_load(&p_op->input_mem, p_in->in_cfg.input_filename);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", p_in->in_cfg.input_filename);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok, size = %d\r\n", p_in->in_cfg.input_filename, file_len);
			}

			{   // Alloc work buf 
				VENDOR_AI3_OP_POSTPROC_MEM op_work_buf = {0} ; 
				vendor_ai3_op_eval_postprocess (&op_work_buf) ; 
				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
			}
			
			{   // Alloc out buf 
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_YUV_BUFSIZE(p_in->in_cfg.loff, p_in->in_cfg.h));
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc out_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
			}
        }
        break;

		case AI_OP_POSTPROC_BAYER122BAYER16:
		{
			{   // Alloc input buf 
				ret = mem_alloc(&p_op->input_mem, "user_in_buf", AI_BAYER12_BUFSIZE(p_in->in_cfg.loff, p_in->in_cfg.h));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail ret = %d\r\n", proc_id, ret);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%lu) alloc in_buf pa(%lx) va(%lx), size = %u\r\n", proc_id, p_op->input_mem.pa, p_op->input_mem.va, p_op->input_mem.size);
				
				INT32 file_len;
				file_len = mem_load(&p_op->input_mem, p_in->in_cfg.input_filename);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", p_in->in_cfg.input_filename);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok, size = %d\r\n", p_in->in_cfg.input_filename, file_len);
			}

			{   // Alloc work buf 
				VENDOR_AI3_OP_POSTPROC_MEM op_work_buf = {0} ; 
				vendor_ai3_op_eval_postprocess (&op_work_buf) ; 
				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
			}
			
			{   // Alloc out buf 
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_BAYER16_BUFSIZE(p_in->in_cfg.loff / 3 * 4, p_in->in_cfg.h));
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc out_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
			}
        }
		break;

		case AI_OP_POSTPROC_BAYER162BAYER12:
        {
			{   // Alloc input buf 
				ret = mem_alloc(&p_op->input_mem, "user_in_buf", AI_BAYER16_BUFSIZE(p_in->in_cfg.loff, p_in->in_cfg.h));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail ret = %d\r\n", proc_id, ret);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%lu) alloc in_buf pa(%lx) va(%lx), size = %u\r\n", proc_id, p_op->input_mem.pa, p_op->input_mem.va, p_op->input_mem.size);
				
				INT32 file_len;
				file_len = mem_load(&p_op->input_mem, p_in->in_cfg.input_filename);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", p_in->in_cfg.input_filename);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok, size = %d\r\n", p_in->in_cfg.input_filename, file_len);
			}

			{   // Alloc work buf 
				VENDOR_AI3_OP_POSTPROC_MEM op_work_buf = {0} ; 
				vendor_ai3_op_eval_postprocess (&op_work_buf) ; 
				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
			}
			
			{   // Alloc out buf 
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_BAYER12_BUFSIZE(p_in->in_cfg.loff / 4 * 3, p_in->in_cfg.h));
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc out_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
			}
        }
        break;
		
        default:
        {
            printf("Unknown op_opt");
	        return HD_ERR_LIMIT;
        }
        break;
	}
		
	return ret;
}

static HD_RESULT mem_config(HD_COMMON_MEM_INIT_CONFIG *p_mem_cfg)
{
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
	return HD_OK;
}

static HD_RESULT operator_do_init_postproc(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    VENDOR_AI3_OP_POSTPROC_CFG postproc_cfg = {0} ; 
    UINT32 p_op_id = 0;
    HD_RESULT ret = HD_OK;

    memcpy(&postproc_cfg.op_mem.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF)) ;
    ret = vendor_ai3_op_init_postprocess (&p_op_id, &postproc_cfg);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_init_postprocess failed.\n");
        return ret ; 
    }
    p_op->proc_id = p_op_id;
    return ret;
}

static HD_RESULT operator_do_uninit_postproc(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    UINT32 p_op_id = p_op->proc_id;
    HD_RESULT ret = HD_OK;

    ret = vendor_ai3_op_uninit_postprocess(p_op_id);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_uninit_postprocess failed.\n");
        return ret ; 
    }
    p_op->proc_id = 0;
    return ret;
}

static HD_RESULT operator_do_postprocess(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    NET_IN* p_in = g_in + p_stream->in_path;
    VENDOR_AI3_OP_POSTPROC_PARAM postproc_param = {0} ; 
    UINT32 p_op_id = p_op->proc_id;
    VENDOR_AI3_BUF src[3] = {0};
    VENDOR_AI3_BUF dest[2] = {0};
    MEM_PARM* in_buf = &p_op->input_mem;
    MEM_PARM* out_buf = &p_op->output_mem;
    NET_IN_CONFIG in_cfg = p_in->in_cfg;
    HD_RESULT ret = HD_OK;

	if((p_op->op_opt == AI_OP_POSTPROC_RGB2YUV) || p_op->op_opt == AI_OP_POSTPROC_BGR2YUV) {
		//set src1 as 1d tensor
        src[0].sign = MAKEFOURCC('A','B','U','F');
        src[0].va = in_buf->va; //< input address	 
        src[0].pa = in_buf->pa;
        src[0].size = in_cfg.loff * in_cfg.h;
        src[0].fmt = HD_VIDEO_PXLFMT_R8;
        src[0].width = in_cfg.w;
        src[0].height = in_cfg.h;
        src[0].line_ofs = in_cfg.loff;
        src[0].channel = 1;
        src[0].batch_num = 1;

        //set src2 as 1d tensor
        src[1].sign = MAKEFOURCC('A','B','U','F');
        src[1].va = in_buf->va + src[0].size; //< input address	 
        src[1].pa = in_buf->pa + src[0].size;
        src[1].size = in_cfg.loff * in_cfg.h;
        src[1].fmt = HD_VIDEO_PXLFMT_G8;
        src[1].width = in_cfg.w;
        src[1].height = in_cfg.h;
        src[1].line_ofs = in_cfg.loff;
        src[1].channel = 1;
        src[1].batch_num = 1;

        //set src3 as 1d tensor
        src[2].sign = MAKEFOURCC('A','B','U','F');
        src[2].va = in_buf->va + 2*src[0].size; //< input address		 
        src[2].pa = in_buf->pa + 2*src[0].size;
        src[2].size = in_cfg.loff * in_cfg.h;
        src[2].fmt = HD_VIDEO_PXLFMT_B8;
        src[2].width = in_cfg.w;
        src[2].height = in_cfg.h;
        src[2].line_ofs = in_cfg.loff;
        src[2].channel = 1;
        src[2].batch_num = 1;

		if(p_op->op_opt == AI_OP_POSTPROC_BGR2YUV){
			src[0].fmt = HD_VIDEO_PXLFMT_B8;
			src[2].fmt = HD_VIDEO_PXLFMT_R8;
		}

    	//set dest1 as 1d tensor
    	dest[0].sign = MAKEFOURCC('A','B','U','F');
    	dest[0].va = out_buf->va; //< output address	 
    	dest[0].pa = out_buf->pa;
    	dest[0].size = in_cfg.loff * in_cfg.h;
    	dest[0].fmt = HD_VIDEO_PXLFMT_Y8;
    	dest[0].width = in_cfg.w;
    	dest[0].height = in_cfg.h;
    	dest[0].line_ofs = in_cfg.loff;
    	dest[0].channel = 1;
    	dest[0].batch_num = 1;

    	//set dest2 as 1d tensor
    	dest[1].sign = MAKEFOURCC('A','B','U','F');
    	dest[1].va = out_buf->va + dest[0].size; //< output address	 
    	dest[1].pa = out_buf->pa + dest[0].size;
    	dest[1].size = in_cfg.loff * in_cfg.h;
    	dest[1].fmt = HD_VIDEO_PXLFMT_UV; 
    	dest[1].width = in_cfg.w;
    	dest[1].height = in_cfg.h;
    	dest[1].line_ofs = in_cfg.loff;
    	dest[1].channel = 1;
    	dest[1].batch_num = 1;
	}else if(p_op->op_opt == AI_OP_POSTPROC_BAYER122BAYER16){
		//set src1 as 1d tensor
        src[0].sign = MAKEFOURCC('A','B','U','F');
        src[0].va = in_buf->va; //< input address	 
        src[0].pa = in_buf->pa;
        src[0].size = in_cfg.loff * in_cfg.h;
        src[0].fmt = HD_VIDEO_PXLFMT_RAW12;
        src[0].width = in_cfg.w;
        src[0].height = in_cfg.h;
        src[0].line_ofs = in_cfg.loff;
        src[0].channel = 1;
        src[0].batch_num = 1;

    	//set dest1 as 1d tensor
    	dest[0].sign = MAKEFOURCC('A','B','U','F');
    	dest[0].va = out_buf->va; //< output address	 
    	dest[0].pa = out_buf->pa;
    	dest[0].size = in_cfg.loff / 3 * 4 * in_cfg.h;
    	dest[0].fmt = HD_VIDEO_PXLFMT_RAW16;
    	dest[0].width = in_cfg.w;
    	dest[0].height = in_cfg.h;
    	dest[0].line_ofs = in_cfg.loff / 3 * 4;
    	dest[0].channel = 1;
    	dest[0].batch_num = 1;
	}else if(p_op->op_opt == AI_OP_POSTPROC_BAYER162BAYER12){\
		//set src1 as 1d tensor
        src[0].sign = MAKEFOURCC('A','B','U','F');
        src[0].va = in_buf->va; //< input address	 
        src[0].pa = in_buf->pa;
        src[0].size = in_cfg.loff * in_cfg.h;
        src[0].fmt = HD_VIDEO_PXLFMT_RAW16;
        src[0].width = in_cfg.w;
        src[0].height = in_cfg.h;
        src[0].line_ofs = in_cfg.loff;
        src[0].channel = 1;
        src[0].batch_num = 1;

    	//set dest1 as 1d tensor
    	dest[0].sign = MAKEFOURCC('A','B','U','F');
    	dest[0].va = out_buf->va; //< output address	 
    	dest[0].pa = out_buf->pa;
    	dest[0].size = in_cfg.loff / 4 * 3 * in_cfg.h;
    	dest[0].fmt = HD_VIDEO_PXLFMT_RAW12;
    	dest[0].width = in_cfg.w;
    	dest[0].height = in_cfg.h;
    	dest[0].line_ofs = in_cfg.loff / 4 * 3;
    	dest[0].channel = 1;
    	dest[0].batch_num = 1;
	}

    ret = vendor_ai3_op_do_postprocess (p_op_id, src, dest, &postproc_param);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_do_postprocess failed.\n");
        return ret ; 
    }

    printf("inference done !\n");
    ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
    if(HD_OK != ret) {
        printf("flush cache failed.\n");
        return ret ; 
    }
        
    mem_save(&p_op->output_mem, "./op_user_out.bin"); 

	return ret ; 
}
/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	VIDEO_LIVEVIEW stream[2] = {0}; //0: net proc, 1: op path
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    INT32 idx;
	HD_RESULT ret;

	NET_IN_CONFIG in_cfg_RGB = {
		.input_filename = "jpg/RGB_W3500H2618.bin",
		.w = 3500,
		.h = 2618,
		.c = 3,
		.loff = 3500,
		.fmt = HD_VIDEO_PXLFMT_RGB888_PLANAR
	};
	NET_IN_CONFIG in_cfg_BGR = {
		.input_filename = "jpg/BGR_W3500H2618.bin",
		.w = 3500,
		.h = 2618,
		.c = 3,
		.loff = 3500,
		.fmt = HD_VIDEO_PXLFMT_RGB888_PLANAR
	};
	NET_IN_CONFIG in_cfg_B12 = {
		.input_filename = "jpg/BAYER12_W2688H1520.raw",
		.w = 2688,
		.h = 1520,
		.c = 1,
		.loff = 4032,
		.fmt = HD_VIDEO_PXLFMT_RAW12
	};
	NET_IN_CONFIG in_cfg_B16 = {
		.input_filename = "jpg/BAYER16_W2688H1520.raw",
		.w = 2688,
		.h = 1520,
		.c = 1,
		.loff = 5376,
		.fmt = HD_VIDEO_PXLFMT_RAW16
	};

    if(argc < 2){
		printf("usage : ai3_op (op_opt)\n"
			   "op-opt:\n"
			   "0  POSTPROC (RGB2YUV)\n"
			   "1  POSTPROC (BGR2YUV)\n"
			   "2  POSTPROC (BAYER122BAYER16)\n"
			   "3  POSTPROC (BAYER162BAYER12)\n");
		return -1;
	}

	idx = 1;
	// parse operator config
	if (argc > idx) {
		sscanf(argv[idx++], "%d", &stream[0].net_op_opt);
	}		
	
	printf("\r\n\r\n");

	// malloc for g_in & g_op
	g_in  = (NET_IN *)malloc(sizeof(NET_IN)*16);
	g_op  = (OP_PROC *)malloc(sizeof(OP_PROC)*16);
	if ((g_in == NULL) || (g_op == NULL)) {
		printf("fail to malloc g_in/g_op\n");
		goto exit;
	}

	stream[0].op_path = 1;
    stream[0].in_path = 1;

	if (stream[0].net_op_opt == AI_OP_POSTPROC_RGB2YUV)
		printf("Run POSTPROC (RGB2YUV)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_POSTPROC_BGR2YUV)
		printf("Run POSTPROC (BGR2YUV)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_POSTPROC_BAYER122BAYER16)
		printf("Run POSTPROC (BAYER122BAYER16)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_POSTPROC_BAYER162BAYER12)
		printf("Run POSTPROC (BAYER162BAYER12)!\r\n");
	else{
		printf("Unknown op-opt = %d",stream[0].net_op_opt);
		return -1;
	}
	
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("hd_common_init fail=%d\n", ret);
		goto exit;
	}

	// init mem
	{
		// config common pool
		mem_config(&mem_cfg);
	}
#if defined(_BSP_NS02201_) || defined(_BSP_NS02302_)
	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		goto exit;
	}
#endif
	
	{ // init mem
		INT32 idx = 0; // mempool index
		input_mem_config(stream[0].in_path, &mem_cfg, 0, idx);
	}


	{ // set open config
		switch (stream[0].net_op_opt) {
			case AI_OP_POSTPROC_RGB2YUV:
			{
				ret = input_set_config(stream[0].in_path, &in_cfg_RGB);
	    		if (HD_OK != ret) {
	    			printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
				    goto exit;
	    		}
				break;
			}
			case AI_OP_POSTPROC_BGR2YUV:
			{
				ret = input_set_config(stream[0].in_path, &in_cfg_BGR);
	    		if (HD_OK != ret) {
	    			printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
				    goto exit;
	    		}
				break;
			}
			case AI_OP_POSTPROC_BAYER122BAYER16:
			{
				ret = input_set_config(stream[0].in_path, &in_cfg_B12);
	    		if (HD_OK != ret) {
	    			printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
				    goto exit;
	    		}
				break;
			}
			case AI_OP_POSTPROC_BAYER162BAYER12:
			{
				ret = input_set_config(stream[0].in_path, &in_cfg_B16);
	    		if (HD_OK != ret) {
	    			printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
				    goto exit;
	    		}
				break;
			}
			default:
    	    {
    	        printf("Unsupport op_opt");
		        return HD_ERR_LIMIT;
    	    }
		}
	}

    // set operator config
    ret = operator_set_config(stream[0].op_path, stream[0].net_op_opt);
    if (HD_OK != ret) {
	    printf("proc_id(%u) operator_set_config fail=%d\n", stream[0].in_path, ret);
		goto exit;
	}

	// open video_liveview modules
	ret = operator_alloc_buf(&stream[0]);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}
	
	switch (stream[0].net_op_opt) {
		case AI_OP_POSTPROC_RGB2YUV:
		case AI_OP_POSTPROC_BGR2YUV:
		case AI_OP_POSTPROC_BAYER122BAYER16:
		case AI_OP_POSTPROC_BAYER162BAYER12:
		{
            ret = operator_do_init_postproc(&stream[0]) ; 
			ret = operator_do_postprocess(&stream[0]) ; 
            ret = operator_do_uninit_postproc(&stream[0]) ; 
			break;
		}
		default:
        {
            printf("Unsupport op_opt");
	        return HD_ERR_LIMIT;
        }
	}
	if(HD_OK != ret) {
		printf("operator do function failed.\n");
		return ret ; 
	}
		
	
exit:	
	ret = operator_free_buf(&stream[0]);
	if(HD_OK != ret) {
		printf("operator_free_buf failed.\n");
	}
#if defined(_BSP_NS02201_) || defined(_BSP_NS02302_)
	// uninit memory
	ret = hd_common_mem_uninit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}
#endif

	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}

	// free g_in & g_op
	if (g_in)  free(g_in);
	if (g_op)  free(g_op);

	return ret;
}
