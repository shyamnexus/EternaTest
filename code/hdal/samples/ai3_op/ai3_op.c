/**
	@brief Source file of vendor ai net sample code.

	@file ai3_op.c

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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai3_op, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////

#define NET_PATH_ID		UINT32

#define VENDOR_AI_CFG  				0x000f0000  //vendor ai config

#define AI_RGB_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))
#define AI_YUV_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * (h) * 3 / 2))

#define NET_VDO_SIZE_W	1920 //max for net
#define NET_VDO_SIZE_H	1080 //max for net

#define SV_LENGTH 		10240
#define SV_FEA_LENGTH 	256

#define SCALE_DIM_W 384
#define SCALE_DIM_H 282

#define AI_MAU_BUFSIZE(w, h, bit_depth)    ((w) * (h) * ((bit_depth)/8))

// fc
#define FC_A_W 512
#define FC_A_H 1
#define FC_A_FRAC_BIT 1
#define FC_A_BIT_DEPTH 8
#define FC_A_FMT HD_VIDEO_PXLFMT_AI_SFIXED8(FC_A_FRAC_BIT)
#define FC_A_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg15701/DI0_15701.bin"

#define FC_B_W 512
#define FC_B_H 261
#define FC_B_FRAC_BIT 1
#define FC_B_BIT_DEPTH 8
#define FC_B_FMT HD_VIDEO_PXLFMT_AI_SFIXED8(FC_B_FRAC_BIT)
#define FC_B_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg15701/DI1_15701.bin"

#define FC_C_BIT_DEPTH 32
#define FC_C_FMT HD_VIDEO_PXLFMT_AI_FLOAT32

// mat_multi
#define MAT_MULTI_A_W 8
#define MAT_MULTI_A_H 64
#define MAT_MULTI_A_FRAC_BIT 5
#define MAT_MULTI_A_BIT_DEPTH 8
#define MAT_MULTI_A_FMT HD_VIDEO_PXLFMT_AI_SFIXED8(MAT_MULTI_A_FRAC_BIT)
#define MAT_MULTI_A_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg15701/DI0_15701.bin"

#define MAT_MULTI_B_W 8
#define MAT_MULTI_B_H 16704
#define MAT_MULTI_B_FRAC_BIT 5
#define MAT_MULTI_B_BIT_DEPTH 8
#define MAT_MULTI_B_FMT HD_VIDEO_PXLFMT_AI_SFIXED8(MAT_MULTI_B_FRAC_BIT)
#define MAT_MULTI_B_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg15701/DI1_15701.bin"

#define MAT_MULTI_C_BIT_DEPTH 32
#define MAT_MULTI_C_FMT HD_VIDEO_PXLFMT_AI_FLOAT32

// mat_add
#define MAT_ADD_A_W 144
#define MAT_ADD_A_H 146
#define MAT_ADD_A_FRAC_BIT 0
#define MAT_ADD_A_BIT_DEPTH 8
#define MAT_ADD_A_FMT HD_VIDEO_PXLFMT_AI_SFIXED8(MAT_ADD_A_FRAC_BIT)
#define MAT_ADD_A_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg16501/DI0_16501.bin"

#define MAT_ADD_B_W 144
#define MAT_ADD_B_H 146
#define MAT_ADD_B_FRAC_BIT 0
#define MAT_ADD_B_BIT_DEPTH 8
#define MAT_ADD_B_FMT HD_VIDEO_PXLFMT_AI_SFIXED8(MAT_ADD_B_FRAC_BIT)
#define MAT_ADD_B_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg16501/DI1_16501.bin"

#define MAT_ADD_C_W 144
#define MAT_ADD_C_H 146
#define MAT_ADD_C_BIT_DEPTH 32
#define MAT_ADD_C_FMT HD_VIDEO_PXLFMT_AI_FLOAT32

// mat_sub
#define MAT_SUB_A_W 144
#define MAT_SUB_A_H 146
#define MAT_SUB_A_FRAC_BIT 0
#define MAT_SUB_A_BIT_DEPTH 8
#define MAT_SUB_A_FMT HD_VIDEO_PXLFMT_AI_UFIXED8(MAT_SUB_A_FRAC_BIT)
#define MAT_SUB_A_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg16501/DI0_16501.bin"

#define MAT_SUB_B_W 144
#define MAT_SUB_B_H 146
#define MAT_SUB_B_FRAC_BIT 0
#define MAT_SUB_B_BIT_DEPTH 8
#define MAT_SUB_B_FMT HD_VIDEO_PXLFMT_AI_UFIXED8(MAT_SUB_B_FRAC_BIT)
#define MAT_SUB_B_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg16501/DI1_16501.bin"

#define MAT_SUB_C_W 144
#define MAT_SUB_C_H 146
#define MAT_SUB_C_BIT_DEPTH 32
#define MAT_SUB_C_FMT HD_VIDEO_PXLFMT_AI_FLOAT32

// topn
#define TOPN_SORT_A_W 131000
#define TOPN_SORT_A_H 1
#define TOPN_SORT_A_FRAC_BIT 0
#define TOPN_SORT_A_BIT_DEPTH 16
#define TOPN_SORT_A_FMT HD_VIDEO_PXLFMT_AI_SFIXED16(TOPN_SORT_A_FRAC_BIT)
#define TOPN_SORT_A_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg17501/DI0_17501.bin"
#define TOPN_SORT_N 128

#define TOPN_SORT_B_W TOPN_SORT_N
#define TOPN_SORT_B_H 1
#define TOPN_SORT_B_FRAC_BIT 0
#define TOPN_SORT_B_BIT_DEPTH 16
#define TOPN_SORT_B_FMT HD_VIDEO_PXLFMT_AI_SFIXED16(TOPN_SORT_B_FRAC_BIT)

#define TOPN_SORT_IDX_W TOPN_SORT_N
#define TOPN_SORT_IDX_H 1
#define TOPN_FMT_SEL(x, y) (x)>(y)?HD_VIDEO_PXLFMT_AI_UINT32:HD_VIDEO_PXLFMT_AI_UINT16
#define TOPN_SORT_IDX_FMT TOPN_FMT_SEL(TOPN_SORT_A_W, 65536)
#define TOPN_BIT_DEPTH_SEL(x, y) (x)>(y)?32:16
#define TOPN_SORT_IDX_BIT_DEPTH TOPN_BIT_DEPTH_SEL(TOPN_SORT_A_W, 65536)

#define MAU_L2_NORM_A_W 54
#define MAU_L2_NORM_A_H 30
#define MAU_L2_NORM_A_FRAC_BIT 61
#define MAU_L2_NORM_A_BIT_DEPTH 16
#define MAU_L2_NORM_A_FMT HD_VIDEO_PXLFMT_AI_UFIXED16(MAU_L2_NORM_A_FRAC_BIT)
#define MAU_L2_NORM_A_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg16701/DI0_16701.bin"

#define MAU_L2_NORM_C_BIT_DEPTH 32
#define MAU_L2_NORM_C_FRAC_BIT 0
#define MAU_L2_NORM_C_FMT HD_VIDEO_PXLFMT_AI_FLOAT32

#define MAU_L2_NORM_ITER 0


#define MAU_L2_NORM_INV_A_W 183
#define MAU_L2_NORM_INV_A_H 31
#define MAU_L2_NORM_INV_A_FRAC_BIT 53
#define MAU_L2_NORM_INV_A_BIT_DEPTH 8
#define MAU_L2_NORM_INV_A_FMT HD_VIDEO_PXLFMT_AI_UFIXED8(MAU_L2_NORM_INV_A_FRAC_BIT)
#define MAU_L2_NORM_INV_A_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg16703/DI0_16703.bin"

#define MAU_L2_NORM_INV_C_BIT_DEPTH 32
#define MAU_L2_NORM_INV_C_FRAC_BIT 0
#define MAU_L2_NORM_INV_C_FMT HD_VIDEO_PXLFMT_AI_FLOAT32

#define MAU_L2_NORM_INV_ITER 1


#define MAU_VECTOR_NORMALIZAION_A_W 120
#define MAU_VECTOR_NORMALIZAION_A_H 27
#define MAU_VECTOR_NORMALIZAION_A_FRAC_BIT 0
#define MAU_VECTOR_NORMALIZAION_A_BIT_DEPTH 16
#define MAU_VECTOR_NORMALIZAION_A_FMT HD_VIDEO_PXLFMT_AI_FLOAT16
#define MAU_VECTOR_NORMALIZAION_A_FILE_PATH "/mnt/sd/jpg/MAU/NUEP/nueg17001/DI0_17001.bin"

#define MAU_VECTOR_NORMALIZAION_C_BIT_DEPTH 8
#define MAU_VECTOR_NORMALIZAION_C_FRAC_BIT 121
#define MAU_VECTOR_NORMALIZAION_C_FMT HD_VIDEO_PXLFMT_AI_UFIXED8(MAU_VECTOR_NORMALIZAION_C_FRAC_BIT)

#define MAU_VECTOR_NORMALIZAION_ITER 1
// static UINT32 mau_mat_outid = 0;
// static VENDOR_AI3_BUF mau_mat_out;

typedef enum _AI_OP {
    AI_OP_FC                             =  0,  //deprecated 
	AI_OP_PREPROC_YUV2RGB                =  1,  
	AI_OP_PREPROC_YUV2RGB_SCALE          =  2,
	AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE  =  3,
	AI_OP_PREPROC_YUV2RGB_MEANSUB_DC     =  4,
	AI_OP_PREPROC_Y2Y_UV2UV				 =  5,
	AI_OP_FC_LL_MODE                     =  6,
	AI_OP_TOPN_SORT					     =  7,
	AI_OP_MAT_MUL					     =  8,
	AI_OP_MAT_ADD					     =  9,
	AI_OP_MAT_SUB					     = 10,
	AI_OP_PREPROC_RGB_SCALE              = 14,
	AI_OP_PREPROC_Y8_SCALE               = 15,
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
	VENDOR_AI3_OP_FC_CFG cfg = {0};

	// alloc buffer
	switch (p_op->op_opt) {
		case AI_OP_FC_LL_MODE:
        {
			{	// Alloc input buf
				INT32 file_len;
				ret = mem_alloc(&p_op->input_mem, "user_in_buf_1", AI_MAU_BUFSIZE(FC_A_W, FC_A_H, FC_A_BIT_DEPTH));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				
				file_len = mem_load(&p_op->input_mem, FC_A_FILE_PATH);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", FC_A_FILE_PATH);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok, pa (%lx) va(%lx), size = %ld\r\n", FC_A_FILE_PATH, p_op->input_mem.pa, p_op->input_mem.pa, file_len);

				ret = mem_alloc(&p_op->input2_mem, "user_in_buf_2", AI_MAU_BUFSIZE(FC_B_W, FC_B_H, FC_B_BIT_DEPTH));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				
				file_len = mem_load(&p_op->input2_mem, FC_B_FILE_PATH);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", FC_B_FILE_PATH);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok,  pa (%lx) va(%lx) size = %ld\r\n", FC_B_FILE_PATH, p_op->input2_mem.pa, p_op->input2_mem.pa, file_len);
			}
			{   // Alloc work buf 
				VENDOR_AI3_OP_FC_MEM op_work_buf = {0};
				op_work_buf.input1.h =  (UINT32)FC_A_H;
				op_work_buf.input1.w =  (UINT32)FC_A_W;
				op_work_buf.input2.h =  (UINT32)FC_B_H;
				op_work_buf.input2.w =  (UINT32)FC_B_W;
				op_work_buf.out_fmt = FC_C_FMT;
				vendor_ai3_op_eval_fullyconnect (&op_work_buf);
				p_op->op_out_temp = op_work_buf.swap_out;
				
				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
				memcpy(&cfg.op_mem, &op_work_buf, sizeof(VENDOR_AI3_OP_FC_MEM));
				memcpy(&cfg.op_mem.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF));
			}
			{   // Alloc out buf 
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_MAU_BUFSIZE(FC_A_H, FC_B_H, 32));
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				else {
					printf("proc_id(%u) alloc out_buf OK, pa(%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);

				}
				if (p_op->op_out_temp){
					ret = mem_alloc(&p_op->output2_mem, "user_tmp_buf", AI_MAU_BUFSIZE(FC_A_H, FC_B_H, 32)); //always need h * w * 4bytes
					if (ret != HD_OK) {
						printf("proc_id(%lu) alloc tmp_buf fail\r\n", proc_id);
						return HD_ERR_FAIL;
					}
					else {
						printf("proc_id(%lu) alloc tmp_buf OK, pa(%lx) va(%lx), size = %lu\r\n", proc_id,p_op->output2_mem.pa, p_op->output2_mem.va, p_op->output2_mem.size);
					}
				}
			}
	        break;
		}
		case AI_OP_MAT_MUL:
		{
			{	// Alloc input buf
				INT32 file_len;
				ret = mem_alloc(&p_op->input_mem, "user_in_buf_1", AI_MAU_BUFSIZE(MAT_MULTI_A_W, MAT_MULTI_A_H, MAT_MULTI_A_BIT_DEPTH));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				
				file_len = mem_load(&p_op->input_mem, MAT_MULTI_A_FILE_PATH);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", MAT_MULTI_A_FILE_PATH);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok, pa (%lx) va(%lx), size = %ld\r\n", MAT_MULTI_A_FILE_PATH, p_op->input_mem.pa, p_op->input_mem.pa, file_len);

				ret = mem_alloc(&p_op->input2_mem, "user_in_buf_2", AI_MAU_BUFSIZE(MAT_MULTI_B_W, MAT_MULTI_B_H, MAT_MULTI_B_BIT_DEPTH));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				
				file_len = mem_load(&p_op->input2_mem, MAT_MULTI_B_FILE_PATH);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", MAT_MULTI_B_FILE_PATH);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok,  pa (%lx) va(%lx) size = %ld\r\n", MAT_MULTI_B_FILE_PATH, p_op->input2_mem.pa, p_op->input2_mem.pa, file_len);
			}
			{   // Alloc work buf 
				VENDOR_AI3_OP_MATRIX_MEM op_work_buf = {0};
				op_work_buf.input1.h =  (UINT32)MAT_MULTI_A_H;
				op_work_buf.input1.w =  (UINT32)MAT_MULTI_A_W;
				op_work_buf.input2.h =  (UINT32)MAT_MULTI_B_H;
				op_work_buf.input2.w =  (UINT32)MAT_MULTI_B_W;
				op_work_buf.out_fmt = MAT_MULTI_C_FMT;
				vendor_ai3_op_eval_matrix (&op_work_buf);
				
				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
				memcpy(&cfg.op_mem, &op_work_buf, sizeof(VENDOR_AI3_OP_MATRIX_MEM));
				memcpy(&cfg.op_mem.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF));
			}
			{   // Alloc out buf 
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_MAU_BUFSIZE(MAT_MULTI_A_H, MAT_MULTI_B_H, 32));
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				else {
					printf("proc_id(%u) alloc out_buf OK, pa(%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
				}
			}
	        break;
		}
		case AI_OP_MAT_ADD:
		{
			{	// Alloc input buf
				INT32 file_len;
				ret = mem_alloc(&p_op->input_mem, "user_in_buf_1", AI_MAU_BUFSIZE(MAT_ADD_A_W, MAT_ADD_A_H, MAT_ADD_A_BIT_DEPTH));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				
				file_len = mem_load(&p_op->input_mem, MAT_ADD_A_FILE_PATH);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", MAT_ADD_A_FILE_PATH);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok, pa (%lx) va(%lx), size = %ld\r\n", MAT_ADD_A_FILE_PATH, p_op->input_mem.pa, p_op->input_mem.pa, file_len);

				ret = mem_alloc(&p_op->input2_mem, "user_in_buf_2", AI_MAU_BUFSIZE(MAT_ADD_B_W, MAT_ADD_B_H, MAT_ADD_B_BIT_DEPTH));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				
				file_len = mem_load(&p_op->input2_mem, MAT_ADD_B_FILE_PATH);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", MAT_ADD_B_FILE_PATH);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok,  pa (%lx) va(%lx) size = %ld\r\n", MAT_ADD_B_FILE_PATH, p_op->input2_mem.pa, p_op->input2_mem.pa, file_len);
			}
			{   // Alloc work buf 
				VENDOR_AI3_OP_MATRIX_MEM op_work_buf = {0};
				op_work_buf.input1.h =  (UINT32)MAT_ADD_A_H;
				op_work_buf.input1.w =  (UINT32)MAT_ADD_A_W;
				op_work_buf.input2.h =  (UINT32)MAT_ADD_B_H;
				op_work_buf.input2.w =  (UINT32)MAT_ADD_B_W;
				op_work_buf.out_fmt = MAT_ADD_C_FMT;
				vendor_ai3_op_eval_matrix (&op_work_buf);
				
				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
				memcpy(&cfg.op_mem, &op_work_buf, sizeof(VENDOR_AI3_OP_MATRIX_MEM));
				memcpy(&cfg.op_mem.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF));
			}
			{   // Alloc out buf 
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_MAU_BUFSIZE(MAT_ADD_A_W, MAT_ADD_A_H, 32));
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				else {
					printf("proc_id(%u) alloc out_buf OK, pa(%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
				}
			}
	        break;
		}
		case AI_OP_MAT_SUB:
        {
			{	// Alloc input buf
				INT32 file_len;
				ret = mem_alloc(&p_op->input_mem, "user_in_buf_1", AI_MAU_BUFSIZE(MAT_SUB_A_W, MAT_SUB_A_H, MAT_SUB_A_BIT_DEPTH));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				
				file_len = mem_load(&p_op->input_mem, MAT_SUB_A_FILE_PATH);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", MAT_SUB_A_FILE_PATH);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok, pa (%lx) va(%lx), size = %ld\r\n", MAT_SUB_A_FILE_PATH, p_op->input_mem.pa, p_op->input_mem.pa, file_len);

				ret = mem_alloc(&p_op->input2_mem, "user_in_buf_2", AI_MAU_BUFSIZE(MAT_SUB_B_W, MAT_SUB_B_H, MAT_SUB_B_BIT_DEPTH));
				if (ret != HD_OK) {
					printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				
				file_len = mem_load(&p_op->input2_mem, MAT_SUB_B_FILE_PATH);
				if (file_len < 0) {
					printf("load buf(%s) fail\r\n", MAT_SUB_B_FILE_PATH);
					return HD_ERR_NG;
				}
				printf("load buf(%s) ok,  pa (%lx) va(%lx) size = %ld\r\n", MAT_SUB_B_FILE_PATH, p_op->input2_mem.pa, p_op->input2_mem.pa, file_len);
			}
			{   // Alloc work buf 
				VENDOR_AI3_OP_MATRIX_MEM op_work_buf = {0};
				op_work_buf.input1.h =  (UINT32)MAT_SUB_A_H;
				op_work_buf.input1.w =  (UINT32)MAT_SUB_A_W;
				op_work_buf.input2.h =  (UINT32)MAT_SUB_B_H;
				op_work_buf.input2.w =  (UINT32)MAT_SUB_B_W;
				op_work_buf.out_fmt = MAT_SUB_C_FMT;
				vendor_ai3_op_eval_matrix (&op_work_buf);
				
				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
				memcpy(&cfg.op_mem, &op_work_buf, sizeof(VENDOR_AI3_OP_MATRIX_MEM));
				memcpy(&cfg.op_mem.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF));
			}
			{   // Alloc out buf 
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_MAU_BUFSIZE(MAT_SUB_A_W, MAT_SUB_A_H, 32));
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				else {
					printf("proc_id(%u) alloc out_buf OK, pa(%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
				}
			}
	        break;
		}
		case AI_OP_PREPROC_YUV2RGB:
		case AI_OP_PREPROC_YUV2RGB_SCALE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_DC:
		case AI_OP_PREPROC_Y2Y_UV2UV:
        {
			{   // Alloc input buf 
				if(p_op->op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE)
					ret = mem_alloc(&p_op->input_mem, "user_in_buf", 2 * AI_RGB_BUFSIZE(p_in->in_cfg.w, p_in->in_cfg.h));
				else
					ret = mem_alloc(&p_op->input_mem, "user_in_buf", AI_RGB_BUFSIZE(p_in->in_cfg.w, p_in->in_cfg.h));
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
				VENDOR_AI3_OP_PREPROC_MEM op_work_buf = {0};
				vendor_ai3_op_eval_preprocess (&op_work_buf);
				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
			}
			
			{   // Alloc out buf 
				if(p_op->op_opt == AI_OP_PREPROC_YUV2RGB_SCALE) {
					ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_RGB_BUFSIZE(SCALE_DIM_W, SCALE_DIM_H));
					if (ret != HD_OK) {
						printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
						return HD_ERR_FAIL;
					}
					else {
						printf("proc_id(%u) alloc out_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
					}
                } else if(p_op->op_opt == AI_OP_PREPROC_Y2Y_UV2UV) {
					ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_YUV_BUFSIZE(SCALE_DIM_W, SCALE_DIM_H));
					if (ret != HD_OK) {
						printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
						return HD_ERR_FAIL;
					}
					else {
						printf("proc_id(%u) alloc out_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
					}
				} else {
					ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_RGB_BUFSIZE(p_in->in_cfg.w, p_in->in_cfg.h));
					if (ret != HD_OK) {
						printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
						return HD_ERR_FAIL;
					}
					else {
						printf("proc_id(%u) alloc out_buf OK, pa(%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
					}
				}
			}
        }
        break;
		case AI_OP_PREPROC_RGB_SCALE:
		case AI_OP_PREPROC_Y8_SCALE:
		{
			{   // Alloc input buf
				if (p_op->op_opt == AI_OP_PREPROC_RGB_SCALE)
					ret = mem_alloc(&p_op->input_mem, "user_in_buf", AI_RGB_BUFSIZE(p_in->in_cfg.w, p_in->in_cfg.h));
				else
					ret = mem_alloc(&p_op->input_mem, "user_in_buf", ALIGN_CEIL_4(p_in->in_cfg.w * p_in->in_cfg.h));

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
				VENDOR_AI3_OP_PREPROC_MEM op_work_buf = {0};
				vendor_ai3_op_eval_preprocess (&op_work_buf);
				ret = mem_alloc(&p_op->work_mem, "op_work_buf", op_work_buf.buf.size);
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
			}
			
			{   // Alloc out buf 
				if (p_op->op_opt == AI_OP_PREPROC_RGB_SCALE)
					ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_RGB_BUFSIZE(SCALE_DIM_W, SCALE_DIM_H));
				else
					ret = mem_alloc(&p_op->output_mem, "user_out_buf", ALIGN_CEIL_4(SCALE_DIM_W * SCALE_DIM_H));

				if (ret != HD_OK) {
					printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				else {
					printf("proc_id(%u) alloc out_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->output_mem.pa, p_op->output_mem.va, p_op->output_mem.size);
				}
			}
		}
		break;
		case AI_OP_TOPN_SORT:
		{
			{/*Alloc input buf */
				INT32 file_len;
				ret = mem_alloc(&p_op->input_mem, "user_in_buf_1", AI_MAU_BUFSIZE(TOPN_SORT_A_W, TOPN_SORT_A_H, TOPN_SORT_A_BIT_DEPTH));
				if (ret != HD_OK) {
	    	    	printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
	    	    	return HD_ERR_FAIL;
	        	}
            
            	file_len = mem_load(&p_op->input_mem, TOPN_SORT_A_FILE_PATH);
	        	if (file_len < 0) {
		    	    printf("load buf(%s) fail\r\n", TOPN_SORT_A_FILE_PATH);
		    	    return HD_ERR_NG;
	        	}
        		printf("load buf(%s) ok, size = %ld\r\n", TOPN_SORT_A_FILE_PATH, file_len);
			}
			{/*Alloc work buf */
				VENDOR_AI3_OP_TOPNSORT_MEM op_work_buf = {0};
				op_work_buf.input1.h =  (UINT32)TOPN_SORT_A_H;
				op_work_buf.input1.w =  (UINT32)TOPN_SORT_A_W;

				vendor_ai3_op_eval_topnsort(&op_work_buf);

				ret = mem_alloc(&p_op->work_mem, "op_work_buf",  op_work_buf.buf.size);
				if (ret != HD_OK) {
					printf("proc_id(%u) alloc op_work_buf fail\r\n", proc_id);
					return HD_ERR_FAIL;
				}
				printf("proc_id(%u) alloc work_buf OK pa (%lx) va(%lx), size = %u\r\n", proc_id, p_op->work_mem.pa, p_op->work_mem.va, p_op->work_mem.size);
			}
			{/*Alloc out buf */
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_MAU_BUFSIZE(TOPN_SORT_B_W, TOPN_SORT_B_H, TOPN_SORT_B_BIT_DEPTH));
				if (ret != HD_OK) {
	        		printf("proc_id(%lu) alloc out_buf fail\r\n", proc_id);
	        		return HD_ERR_FAIL;
	        	}
				else {
            	    printf("proc_id(%lu) alloc out_buf OK, size = %lu\r\n", proc_id, AI_MAU_BUFSIZE(TOPN_SORT_B_W, TOPN_SORT_B_H, TOPN_SORT_B_BIT_DEPTH));
            	}

				ret = mem_alloc(&p_op->output2_mem, "user_idx_buf", AI_MAU_BUFSIZE(TOPN_SORT_IDX_W, TOPN_SORT_IDX_H, TOPN_SORT_IDX_BIT_DEPTH));
				if (ret != HD_OK) {
	        		printf("proc_id(%lu) alloc tmp_buf fail\r\n", proc_id);
	        		return HD_ERR_FAIL;
	        	}
				else {
            	    printf("proc_id(%lu) alloc tmp_buf OK, size = %lu\r\n", proc_id, AI_MAU_BUFSIZE(TOPN_SORT_IDX_W, TOPN_SORT_IDX_H, TOPN_SORT_IDX_BIT_DEPTH));
            	}
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

static HD_RESULT operator_do_init_fc(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    VENDOR_AI3_OP_FC_CFG fc_cfg = {0};
    UINT32 p_op_id = 0;
    HD_RESULT ret = HD_OK;

	fc_cfg.op_mem.input1.h = (UINT32)FC_A_H;
	fc_cfg.op_mem.input1.w = (UINT32)FC_A_W;
	fc_cfg.op_mem.input2.h = (UINT32)FC_B_H;
	fc_cfg.op_mem.input2.w = (UINT32)FC_B_W;
	fc_cfg.op_mem.out_fmt = FC_C_FMT;
	fc_cfg.op_mem.swap_out = p_op->op_out_temp;
	memcpy(&fc_cfg.op_mem.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF));
    ret = vendor_ai3_op_init_fullyconnect (&p_op_id, &fc_cfg);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_init_fullyconnect failed.\n");
        return ret;
    }
    p_op->proc_id = p_op_id;
    return ret;
}

static HD_RESULT operator_do_uninit_fc(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    UINT32 p_op_id = p_op->proc_id;
    HD_RESULT ret = HD_OK;

    ret = vendor_ai3_op_uninit_fullyconnect(p_op_id);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_uninit_fullyconnect failed.\n");
        return ret;
    }
    p_op->proc_id = 0;
    return ret;
}

static HD_RESULT operator_do_fc(VIDEO_LIVEVIEW *p_stream)
{
	OP_PROC* p_op = g_op + p_stream->op_path;
	UINT32 p_op_id = p_op->proc_id;
	VENDOR_AI3_BUF src[2] = {0};
	VENDOR_AI3_BUF dest[2] = {0};
	HD_RESULT ret = HD_OK;
	MEM_PARM* in_A_buf = &p_op->input_mem;
	MEM_PARM* in_B_buf = &p_op->input2_mem;
	MEM_PARM* out_buf = &p_op->output_mem;
	MEM_PARM* tmp_buf = &p_op->output2_mem;

	// flush input
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->input_mem)->va, (&p_op->input_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->input2_mem)->va, (&p_op->input2_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	// if it needs temp buf
	if (p_op->op_out_temp){
		memset((VOID *)tmp_buf->va,0x0, tmp_buf->size);
		ret = hd_common_mem_flush_cache((VOID *)tmp_buf->va, tmp_buf->size);
		if(HD_OK != ret) {
			printf("flush cache failed.\n");
		}
	}
	
	//set src1 as input matrix 1
	src[0].sign = MAKEFOURCC('A','B','U','F');
	src[0].va = in_A_buf->va; //< input address
	src[0].pa = in_A_buf->pa;
	src[0].size = AI_MAU_BUFSIZE(FC_A_W, FC_A_H, FC_A_BIT_DEPTH);
	src[0].fmt = FC_A_FMT; //fixpoint s16.11
	src[0].width = FC_A_W;
	src[0].height = FC_A_H;
	src[0].channel = 1;
	src[0].batch_num = 1;
	src[0].line_ofs = FC_A_W * FC_A_BIT_DEPTH/8;
	
	//set src2 as input matrix 2
	src[1].sign = MAKEFOURCC('A','B','U','F');
	src[1].va = in_B_buf->va; //< input address
	src[1].pa = in_B_buf->pa;
	src[1].size = AI_MAU_BUFSIZE(FC_B_W, FC_B_H, FC_B_BIT_DEPTH);
	src[1].fmt = FC_B_FMT; //fixpoint s16.11
	src[1].width = FC_B_W;
	src[1].height = FC_B_H;
	src[1].channel = 1;
	src[1].batch_num = 1;
	src[1].line_ofs = FC_B_W * FC_B_BIT_DEPTH/8;
	
	//set dest1 as output matrix
	dest[0].sign = MAKEFOURCC('A','B','U','F');
	dest[0].va = out_buf->va;
	dest[0].pa = out_buf->pa;
	dest[0].size = AI_MAU_BUFSIZE(FC_A_H, FC_B_H, 32); //always need h * w * 4bytes
	dest[0].fmt = FC_C_FMT;
	dest[0].width = FC_B_H;
	dest[0].height = FC_A_H;
	dest[0].channel = 1;
	dest[0].batch_num = 1;
	dest[0].line_ofs = FC_B_H * FC_C_BIT_DEPTH/8;

	if (p_op->op_out_temp){
	// //set dest2 as temp buf
		dest[1].sign = MAKEFOURCC('A','B','U','F');
		dest[1].va = tmp_buf->va; //< output address	 (size = SV_LENGTH*4 bytes)
		dest[1].pa = tmp_buf->pa;
		dest[1].size = AI_MAU_BUFSIZE(FC_A_H, FC_B_H, 32); //always need h * w * 4bytes
		dest[1].fmt = FC_C_FMT;
		dest[1].width = FC_B_H;
		dest[1].height = FC_A_H;
		dest[1].channel = 1;
		dest[1].batch_num = 1;
		dest[1].line_ofs = FC_B_H * FC_C_BIT_DEPTH/8;
	}
	
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->work_mem)->va, (&p_op->work_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}

	ret = vendor_ai3_op_do_fullyconnect (p_op_id, src, src + 1, dest);
	if (ret != HD_OK) {
		printf("proc_id(%u) vendor_ai3_op_do_fullyconnect for run fc fail\r\n", p_stream->op_path);
		return ret;
	}
			
	printf("inference done!\n");

	if (p_op->op_out_temp == 0 || p_op->op_out_temp == 2){
		ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
		if(HD_OK != ret) {
			printf("flush cache failed.\n");
		}
		mem_save(&p_op->output_mem, "/mnt/sd/fc_out.bin");
	}else {
		ret = hd_common_mem_flush_cache((VOID *)tmp_buf->va, tmp_buf->size);
		if(HD_OK != ret) {
			printf("flush cache failed.\n");
		}
		mem_save(&p_op->output2_mem, "/mnt/sd/fc_out.bin");
	}
	
	return ret;
}

static HD_RESULT operator_do_init_matrix(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    VENDOR_AI3_OP_MATRIX_CFG matrix_cfg = {0};
    UINT32 p_op_id = 0;
    HD_RESULT ret = HD_OK;

	if(p_op->op_opt == AI_OP_MAT_MUL) {
		matrix_cfg.op_mem.input1.h = (UINT32)MAT_MULTI_A_H;
		matrix_cfg.op_mem.input1.w = (UINT32)MAT_MULTI_A_W;
		matrix_cfg.op_mem.input2.h = (UINT32)MAT_MULTI_B_H;
		matrix_cfg.op_mem.input2.w = (UINT32)MAT_MULTI_B_W;
		matrix_cfg.op_mem.out_fmt = MAT_MULTI_C_FMT;
	}else if(p_op->op_opt == AI_OP_MAT_ADD) {
		matrix_cfg.op_mem.input1.h = (UINT32)MAT_ADD_A_H;
		matrix_cfg.op_mem.input1.w = (UINT32)MAT_ADD_A_W;
		matrix_cfg.op_mem.input2.h = (UINT32)MAT_ADD_B_H;
		matrix_cfg.op_mem.input2.w = (UINT32)MAT_ADD_B_W;
		matrix_cfg.op_mem.out_fmt = MAT_ADD_C_FMT;
	}else {//MAT_SUB
		matrix_cfg.op_mem.input1.h = (UINT32)MAT_SUB_A_H;
		matrix_cfg.op_mem.input1.w = (UINT32)MAT_SUB_A_W;
		matrix_cfg.op_mem.input2.h = (UINT32)MAT_SUB_B_H;
		matrix_cfg.op_mem.input2.w = (UINT32)MAT_SUB_B_W;
		matrix_cfg.op_mem.out_fmt = MAT_SUB_C_FMT;
	}
	memcpy(&matrix_cfg.op_mem.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF));

    ret = vendor_ai3_op_init_matrix(&p_op_id, &matrix_cfg);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_init_matrix failed.\n");
        return ret;
    }
    p_op->proc_id = p_op_id;
    
	return ret;
}

static HD_RESULT operator_do_uninit_matrix(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    UINT32 p_op_id = p_op->proc_id;
    HD_RESULT ret = HD_OK;

    ret = vendor_ai3_op_uninit_matrix(p_op_id);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_uninit_matrix failed.\n");
        return ret;
    }
    p_op->proc_id = 0;

    return ret;
}

static HD_RESULT operator_do_matrix(VIDEO_LIVEVIEW *p_stream)
{
	OP_PROC* p_op = g_op + p_stream->op_path;
	UINT32 p_op_id = p_op->proc_id;
	VENDOR_AI3_BUF src[2] = {0};
	VENDOR_AI3_BUF dest[1] = {0};
	HD_RESULT ret = HD_OK;
	MEM_PARM* in_A_buf = &p_op->input_mem;
	MEM_PARM* in_B_buf = &p_op->input2_mem;
	MEM_PARM* out_buf = &p_op->output_mem;

	// flush input
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->input_mem)->va, (&p_op->input_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->input2_mem)->va, (&p_op->input2_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	
	if(p_op->op_opt == AI_OP_MAT_MUL) {
		//set src1 as input matrix 1
		src[0].sign = MAKEFOURCC('A','B','U','F');
		src[0].va = in_A_buf->va; //< input address
		src[0].pa = in_A_buf->pa;
		src[0].size = AI_MAU_BUFSIZE(MAT_MULTI_A_W, MAT_MULTI_A_H, MAT_MULTI_A_BIT_DEPTH);
		src[0].fmt = MAT_MULTI_A_FMT; //fixpoint s8.5
		src[0].width = MAT_MULTI_A_W;
		src[0].height = MAT_MULTI_A_H;
		src[0].channel = 1;
		src[0].batch_num = 1;
		src[0].line_ofs = MAT_MULTI_A_W * MAT_MULTI_A_BIT_DEPTH / 8;

		//set src2 as input matrix 2
		src[1].sign = MAKEFOURCC('A','B','U','F');
		src[1].va = in_B_buf->va; //< input address
		src[1].pa = in_B_buf->pa;
		src[1].size = AI_MAU_BUFSIZE(MAT_MULTI_B_W, MAT_MULTI_B_H, MAT_MULTI_B_BIT_DEPTH);
		src[1].fmt = MAT_MULTI_B_FMT; //fixpoint s8.5
		src[1].width = MAT_MULTI_B_W;
		src[1].height = MAT_MULTI_B_H;
		src[1].channel = 1;
		src[1].batch_num = 1;
		src[1].line_ofs = MAT_MULTI_B_W * MAT_MULTI_B_BIT_DEPTH / 8;

		//set dest1 as output matrix
		dest[0].sign = MAKEFOURCC('A','B','U','F');
		dest[0].va = out_buf->va;
		dest[0].pa = out_buf->pa;
		dest[0].size = AI_MAU_BUFSIZE(MAT_MULTI_A_H, MAT_MULTI_B_H, 32); //always need h * w * 4bytes
		dest[0].fmt = MAT_MULTI_C_FMT;
		dest[0].width = MAT_MULTI_B_H;
		dest[0].height = MAT_MULTI_A_H;
		dest[0].channel = 1;
		dest[0].batch_num = 1;
		dest[0].line_ofs = MAT_MULTI_B_H * MAT_MULTI_C_BIT_DEPTH / 8;
	}else if(p_op->op_opt == AI_OP_MAT_ADD) {
		//set src1 as input matrix 1
		src[0].sign = MAKEFOURCC('A','B','U','F');
		src[0].va = in_A_buf->va; //< input address
		src[0].pa = in_A_buf->pa;
		src[0].size = AI_MAU_BUFSIZE(MAT_ADD_A_W, MAT_ADD_A_H, MAT_ADD_A_BIT_DEPTH);
		src[0].fmt = MAT_ADD_A_FMT; //fixpoint s8.0
		src[0].width = MAT_ADD_A_W;
		src[0].height = MAT_ADD_A_H;
		src[0].channel = 1;
		src[0].batch_num = 1;
		src[0].line_ofs = MAT_ADD_A_W * MAT_ADD_A_BIT_DEPTH / 8;
		
		//set src2 as input matrix 2
		src[1].sign = MAKEFOURCC('A','B','U','F');
		src[1].va = in_B_buf->va; //< input address
		src[1].pa = in_B_buf->pa;
		src[1].size = AI_MAU_BUFSIZE(MAT_ADD_B_W, MAT_ADD_B_H, MAT_ADD_B_BIT_DEPTH);
		src[1].fmt = MAT_ADD_B_FMT; //fixpoint s8.0
		src[1].width = MAT_ADD_B_W;
		src[1].height = MAT_ADD_B_H;
		src[1].channel = 1;
		src[1].batch_num = 1;
		src[1].line_ofs = MAT_ADD_B_W * MAT_ADD_B_BIT_DEPTH / 8;
		
		//set dest1 as output matrix
		dest[0].sign = MAKEFOURCC('A','B','U','F');
		dest[0].va = out_buf->va;
		dest[0].pa = out_buf->pa;
		dest[0].size = AI_MAU_BUFSIZE(MAT_ADD_C_W, MAT_ADD_C_H, 32); //always need h * w * 4bytes
		dest[0].fmt = MAT_ADD_C_FMT;
		dest[0].width = MAT_ADD_C_W;
		dest[0].height = MAT_ADD_C_H;
		dest[0].channel = 1;
		dest[0].batch_num = 1;
		dest[0].line_ofs = MAT_ADD_C_W * MAT_ADD_C_BIT_DEPTH / 8;
	}else{//MAT_SUB
		//set src1 as input matrix 1
		src[0].sign = MAKEFOURCC('A','B','U','F');
		src[0].va = in_A_buf->va; //< input address
		src[0].pa = in_A_buf->pa;
		src[0].size = AI_MAU_BUFSIZE(MAT_SUB_A_W, MAT_SUB_A_H, MAT_SUB_A_BIT_DEPTH);
		src[0].fmt = MAT_SUB_A_FMT; //fixpoint s8.0
		src[0].width = MAT_SUB_A_W;
		src[0].height = MAT_SUB_A_H;
		src[0].channel = 1;
		src[0].batch_num = 1;
		src[0].line_ofs = MAT_SUB_A_W * MAT_SUB_A_BIT_DEPTH / 8;
		
		//set src2 as input matrix 2
		src[1].sign = MAKEFOURCC('A','B','U','F');
		src[1].va = in_B_buf->va; //< input address
		src[1].pa = in_B_buf->pa;
		src[1].size = AI_MAU_BUFSIZE(MAT_SUB_B_W, MAT_SUB_B_H, MAT_SUB_B_BIT_DEPTH);
		src[1].fmt = MAT_SUB_B_FMT; //fixpoint s8.0
		src[1].width = MAT_SUB_B_W;
		src[1].height = MAT_SUB_B_H;
		src[1].channel = 1;
		src[1].batch_num = 1;
		src[1].line_ofs = MAT_SUB_B_W * MAT_SUB_B_BIT_DEPTH / 8;
		
		//set dest1 as output matrix
		dest[0].sign = MAKEFOURCC('A','B','U','F');
		dest[0].va = out_buf->va;
		dest[0].pa = out_buf->pa;
		dest[0].size = AI_MAU_BUFSIZE(MAT_SUB_C_W, MAT_SUB_C_H, 32); //always need h * w * 4bytes
		dest[0].fmt = MAT_SUB_C_FMT;
		dest[0].width = MAT_SUB_C_W;
		dest[0].height = MAT_SUB_C_H;
		dest[0].channel = 1;
		dest[0].batch_num = 1;
		dest[0].line_ofs = MAT_SUB_C_W * FC_C_BIT_DEPTH / 8;
	}
	
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->work_mem)->va, (&p_op->work_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}

	if(p_op->op_opt == AI_OP_MAT_MUL) {
		ret = vendor_ai3_op_do_matrix (p_op_id, src, src + 1, dest, VENDOR_AI3_MATRIX_MUL);
		if (ret != HD_OK) {
			printf("proc_id(%u) vendor_ai3_op_do_matrix for run matmul fail\r\n", p_stream->op_path);
			return ret;
		}
	}else if(p_op->op_opt == AI_OP_MAT_ADD) {
		ret = vendor_ai3_op_do_matrix (p_op_id, src, src + 1, dest, VENDOR_AI3_MATRIX_ADD);
		if (ret != HD_OK) {
			printf("proc_id(%u) vendor_ai3_op_do_matrix for run matadd fail\r\n", p_stream->op_path);
			return ret;
		}
	}else{//MAT_SUB
		ret = vendor_ai3_op_do_matrix (p_op_id, src, src + 1, dest, VENDOR_AI3_MATRIX_SUB);
		if (ret != HD_OK) {
			printf("proc_id(%u) vendor_ai3_op_do_matrix for run matsub fail\r\n", p_stream->op_path);
			return ret;
		}
	}
			
	printf("inference done!\n");

	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	mem_save(&p_op->output_mem, "/mnt/sd/matrix.bin");
	
	return ret;
}

static HD_RESULT operator_do_init_topn(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    VENDOR_AI3_OP_TOPNSORT_CFG topnsort_cfg = {0};
    UINT32 p_op_id = 0;
    HD_RESULT ret = HD_OK;

	memcpy(&topnsort_cfg.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF));

    ret = vendor_ai3_op_init_topnsort(&p_op_id, &topnsort_cfg);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_init_topnsort failed.\n");
        return ret;
    }
    p_op->proc_id = p_op_id;
    
	return ret;
}

static HD_RESULT operator_do_uninit_topn(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    UINT32 p_op_id = p_op->proc_id;
    HD_RESULT ret = HD_OK;

    ret = vendor_ai3_op_uninit_topnsort(p_op_id);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_uninit_topnsort failed.\n");
        return ret;
    }
    p_op->proc_id = 0;

    return ret;
}

static HD_RESULT operator_do_topn(VIDEO_LIVEVIEW *p_stream)
{
	OP_PROC* p_op = g_op + p_stream->op_path;
	HD_RESULT ret = HD_OK;

	// flush input
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->input_mem)->va, (&p_op->input_mem)->size);
	if(HD_OK != ret) {
	    printf("flush cache failed.\n");
	}
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
	if(HD_OK != ret) {
	    printf("flush cache failed.\n");
	}
	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output2_mem)->va, (&p_op->output2_mem)->size);
	if(HD_OK != ret) {
	     printf("flush cache failed.\n");
	}

	VENDOR_AI3_BUF src[1] = {0};
	VENDOR_AI3_BUF dest_value[1] = {0};
	VENDOR_AI3_BUF dest_idx[1] = {0};
	MEM_PARM* in_A_buf = &p_op->input_mem;
	MEM_PARM* out_buf = &p_op->output_mem;
	MEM_PARM* idx_buf = &p_op->output2_mem;

	//pprintf("input addr pa = 0x%08X\n", (unsigned int)(in_buf->pa));
	//pprintf("output addr pa = 0x%08X\n", (unsigned int)(out_buf->pa));
	//pprintf("weight addr pa = 0x%08X\n", (unsigned int)(idx_buf->pa));
				    
	//set src1 as 1d tensor
	src[0].sign = MAKEFOURCC('A','B','U','F');
	src[0].va = in_A_buf->va; //< input address
	src[0].pa = in_A_buf->pa;
	src[0].size = AI_MAU_BUFSIZE(TOPN_SORT_A_W, TOPN_SORT_A_H, TOPN_SORT_A_BIT_DEPTH);
	src[0].fmt = TOPN_SORT_A_FMT;
	src[0].width = TOPN_SORT_A_W;
	src[0].height = TOPN_SORT_A_H;
	src[0].channel = 1;
	src[0].batch_num = 1;
	src[0].line_ofs = TOPN_SORT_A_W * TOPN_SORT_A_BIT_DEPTH / 8;
	src[0].channel_ofs = TOPN_SORT_A_W * TOPN_SORT_A_BIT_DEPTH / 8 * TOPN_SORT_A_H;
					
	//set dest1 as 1d tensor
	dest_value[0].sign = MAKEFOURCC('A','B','U','F');
	dest_value[0].va = out_buf->va;
	dest_value[0].pa = out_buf->pa;
	dest_value[0].size = AI_MAU_BUFSIZE(TOPN_SORT_B_W, TOPN_SORT_B_H, TOPN_SORT_B_BIT_DEPTH);
	dest_value[0].fmt = TOPN_SORT_B_FMT;
	dest_value[0].width = TOPN_SORT_B_W;
	dest_value[0].height = TOPN_SORT_B_H;
	dest_value[0].channel = 1;
	dest_value[0].batch_num = 1;
	dest_value[0].line_ofs = TOPN_SORT_B_W * TOPN_SORT_B_BIT_DEPTH / 8;
	dest_value[0].channel_ofs = TOPN_SORT_B_W * TOPN_SORT_B_BIT_DEPTH / 8 * TOPN_SORT_B_H;

	//set dest2 as 1d tensor
	dest_idx[0].sign = MAKEFOURCC('A','B','U','F');
	dest_idx[0].va = idx_buf->va;
	dest_idx[0].pa = idx_buf->pa;
	dest_idx[0].size = AI_MAU_BUFSIZE(TOPN_SORT_IDX_W, TOPN_SORT_IDX_H, TOPN_SORT_IDX_BIT_DEPTH);
	dest_idx[0].fmt = TOPN_SORT_IDX_FMT;
	dest_idx[0].width = TOPN_SORT_IDX_W;
	dest_idx[0].height = TOPN_SORT_IDX_H;
	dest_idx[0].channel = 1;
	dest_idx[0].batch_num = 1;
	dest_idx[0].line_ofs = TOPN_SORT_IDX_W * TOPN_SORT_IDX_BIT_DEPTH / 8;
	dest_idx[0].channel_ofs = TOPN_SORT_IDX_W * TOPN_SORT_IDX_BIT_DEPTH / 8 * TOPN_SORT_IDX_H;

	ret = hd_common_mem_flush_cache((VOID *)(&p_op->work_mem)->va, (&p_op->work_mem)->size);
	if(HD_OK != ret) {
	   	printf("flush cache failed.\n");
	}

	ret = vendor_ai3_op_do_topnsort(p_op->proc_id, src, dest_value, dest_idx, VENDOR_AI3_SORT_DESC, 128); //VENDOR_AI3_SORT_ASC
	if (ret != HD_OK) {
		printf("proc_id(%u) vendor_ai3_op_do_topnsort for run topk fail\r\n", p_stream->op_path);
		return ret;
	}
			
	printf("inference done!\n");

	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	mem_save(&p_op->output_mem, "/mnt/sd/sort_value.bin");

	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output2_mem)->va, (&p_op->output2_mem)->size);
	if(HD_OK != ret) {
		printf("flush cache failed.\n");
	}
	mem_save(&p_op->output2_mem, "/mnt/sd/sort_index.bin");

	return ret;
}

static HD_RESULT operator_do_init_preproc(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    VENDOR_AI3_OP_PREPROC_CFG preproc_cfg = {0};
    UINT32 p_op_id = 0;
    HD_RESULT ret = HD_OK;

    memcpy(&preproc_cfg.op_mem.buf, &p_op->work_mem, sizeof(VENDOR_AI3_CFG_BUF));
    ret = vendor_ai3_op_init_preprocess (&p_op_id, &preproc_cfg);
    if(HD_OK != ret) {
        printf("operator_do_init_preproc failed.\n");
        return ret;
    }
    p_op->proc_id = p_op_id;
    return ret;
}

static HD_RESULT operator_do_uninit_preproc(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
    UINT32 p_op_id = p_op->proc_id;
    HD_RESULT ret = HD_OK;

    ret = vendor_ai3_op_uninit_preprocess(p_op_id);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_uninit_preprocess failed.\n");
        return ret;
    }
    p_op->proc_id = 0;
    return ret;
}

static HD_RESULT operator_do_preprocess(VIDEO_LIVEVIEW *p_stream)
{
    OP_PROC* p_op = g_op + p_stream->op_path;
	NET_IN* p_in = g_in + p_stream->in_path;
	VENDOR_AI3_OP_PREPROC_PARAM preproc_param = {0};
    UINT32 p_op_id = p_op->proc_id;
    VENDOR_AI3_BUF src[3] = {0};
    VENDOR_AI3_BUF dest[3] = {0};
    MEM_PARM* in_buf = &p_op->input_mem;
    MEM_PARM* out_buf = &p_op->output_mem;
    NET_IN_CONFIG in_cfg = p_in->in_cfg;
    HD_RESULT ret = HD_OK;

    if(p_op->op_opt == AI_OP_PREPROC_RGB_SCALE) {
		//set src1 as 1d tensor
		src[0].sign = MAKEFOURCC('A','B','U','F');
		src[0].va = in_buf->va; //< input address
		src[0].pa = in_buf->pa; //must 2 bytes align!
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
		src[1].pa = in_buf->pa + src[0].size; //must 2 bytes align!
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
		src[2].pa = in_buf->pa + 2*src[0].size; //must 2 bytes align!
		src[2].size = in_cfg.loff * in_cfg.h;
		src[2].fmt = HD_VIDEO_PXLFMT_B8;
		src[2].width = in_cfg.w;
		src[2].height = in_cfg.h;
		src[2].line_ofs = in_cfg.loff;
		src[2].channel = 1;
		src[2].batch_num = 1;
	}
	else if(p_op->op_opt == AI_OP_PREPROC_Y8_SCALE) {
		//set src1 as 1d tensor
		src[0].sign = MAKEFOURCC('A','B','U','F');
		src[0].va = in_buf->va; //< input address
		src[0].pa = in_buf->pa; //must 2 bytes align!
		src[0].size = in_cfg.loff * in_cfg.h;
		src[0].fmt = HD_VIDEO_PXLFMT_Y8;
		src[0].width = in_cfg.w;
		src[0].height = in_cfg.h;
		src[0].line_ofs = in_cfg.loff;
		src[0].channel = 1;
		src[0].batch_num = 1;
	}
	else{
		//set src1 as 1d tensor
		src[0].sign = MAKEFOURCC('A','B','U','F');
		src[0].va = in_buf->va; //< input address	 
		src[0].pa = in_buf->pa; //must 2 bytes align!
		src[0].size = in_cfg.loff * in_cfg.h;
		src[0].fmt = HD_VIDEO_PXLFMT_Y8;
		src[0].width = in_cfg.w;
		src[0].height = in_cfg.h;
		src[0].line_ofs = in_cfg.loff;
		src[0].channel = 1;
		src[0].batch_num = 1;

        //set src2 as 1d tensor
		src[1].sign = MAKEFOURCC('A','B','U','F');
		src[1].va = in_buf->va + src[0].size; //< input address	 
		src[1].pa = in_buf->pa + src[0].size; //must 2 bytes align!
		src[1].size = in_cfg.loff * in_cfg.h;
		src[1].fmt = HD_VIDEO_PXLFMT_UV;
		src[1].width = in_cfg.w;
		src[1].height = in_cfg.h;
		src[1].line_ofs = in_cfg.loff;
		src[1].channel = 1;
		src[1].batch_num = 1;
	}

    if((p_op->op_opt == AI_OP_PREPROC_YUV2RGB_SCALE) || (p_op->op_opt == AI_OP_PREPROC_RGB_SCALE)) {
        //set dest1 as 1d tensor
        dest[0].sign = MAKEFOURCC('A','B','U','F');
        dest[0].va = out_buf->va; //< output address	 
        dest[0].pa = out_buf->pa;
        dest[0].size = SCALE_DIM_W * SCALE_DIM_H;
        dest[0].fmt = HD_VIDEO_PXLFMT_R8;
        dest[0].width = SCALE_DIM_W;
        dest[0].height = SCALE_DIM_H;
        dest[0].line_ofs = SCALE_DIM_W;
        dest[0].channel = 1;
        dest[0].batch_num = 1;

        //set dest2 as 1d tensor
        dest[1].sign = MAKEFOURCC('A','B','U','F');
        dest[1].va = out_buf->va + dest[0].size; //< output address	 
        dest[1].pa = out_buf->pa + dest[0].size;
        dest[1].size = SCALE_DIM_W * SCALE_DIM_H;
        dest[1].fmt = HD_VIDEO_PXLFMT_G8;
        dest[1].width = SCALE_DIM_W;
        dest[1].height = SCALE_DIM_H;
        dest[1].line_ofs = SCALE_DIM_W;
        dest[1].channel = 1;
        dest[1].batch_num = 1;

        //set dest3 as 1d tensor
        dest[2].sign = MAKEFOURCC('A','B','U','F');
        dest[2].va = out_buf->va + 2*dest[0].size; //< output address		 
        dest[2].pa = out_buf->pa + 2*dest[0].size;
        dest[2].size = SCALE_DIM_W * SCALE_DIM_H;
        dest[2].fmt = HD_VIDEO_PXLFMT_B8;
        dest[2].width = SCALE_DIM_W;
        dest[2].height = SCALE_DIM_H;
        dest[2].line_ofs = SCALE_DIM_W;
        dest[2].channel = 1;
        dest[2].batch_num = 1;
    }else if(p_op->op_opt == AI_OP_PREPROC_Y2Y_UV2UV){
        //set src1 width & height
        src[1].width = in_cfg.w / 2;
        src[1].height = in_cfg.h / 2;
        //set dest1 as 1d tensor
        dest[0].sign = MAKEFOURCC('A','B','U','F');
        dest[0].va = out_buf->va; //< output address	 
        dest[0].pa = out_buf->pa;
        dest[0].size = SCALE_DIM_W * SCALE_DIM_H;
        dest[0].fmt = HD_VIDEO_PXLFMT_Y8;
        dest[0].width = SCALE_DIM_W;
        dest[0].height = SCALE_DIM_H;
        dest[0].line_ofs = SCALE_DIM_W;
        dest[0].channel = 1;
        dest[0].batch_num = 1;
        //set dest2 as 1d tensor
        dest[1].sign = MAKEFOURCC('A','B','U','F');
        dest[1].va = out_buf->va + dest[0].size; //< output address	 
        dest[1].pa = out_buf->pa + dest[0].size;
        dest[1].size = SCALE_DIM_W * SCALE_DIM_H / 2;
        dest[1].fmt = HD_VIDEO_PXLFMT_UV;
        dest[1].width = SCALE_DIM_W/2;
        dest[1].height = SCALE_DIM_H/2;
        dest[1].line_ofs = SCALE_DIM_W;
        dest[1].channel = 1;
        dest[1].batch_num = 1;
	}else if(p_op->op_opt == AI_OP_PREPROC_Y8_SCALE){
		//set dest1 as 1d tensor
		dest[0].sign = MAKEFOURCC('A','B','U','F');
		dest[0].va = out_buf->va; //< output address
		dest[0].pa = out_buf->pa;
		dest[0].size = SCALE_DIM_W * SCALE_DIM_H;
		dest[0].fmt = HD_VIDEO_PXLFMT_Y8;
		dest[0].width = SCALE_DIM_W;
		dest[0].height = SCALE_DIM_H;
		dest[0].line_ofs = SCALE_DIM_W;
		dest[0].channel = 1;
		dest[0].batch_num = 1;
	}else {
        //set dest1 as 1d tensor
        dest[0].sign = MAKEFOURCC('A','B','U','F');
        dest[0].va = out_buf->va; //< output address	 
        dest[0].pa = out_buf->pa;
        dest[0].size = in_cfg.loff * in_cfg.h;
        dest[0].fmt = HD_VIDEO_PXLFMT_R8;
        dest[0].width = in_cfg.w;
        dest[0].height = in_cfg.h;
        dest[0].line_ofs = in_cfg.w;
        dest[0].channel = 1;
        dest[0].batch_num = 1;

        //set dest2 as 1d tensor
        dest[1].sign = MAKEFOURCC('A','B','U','F');
        dest[1].va = out_buf->va + dest[0].size; //< output address	 
        dest[1].pa = out_buf->pa + dest[0].size;
        dest[1].size = in_cfg.loff * in_cfg.h;
        dest[1].fmt = HD_VIDEO_PXLFMT_G8;
        dest[1].width = in_cfg.w;
        dest[1].height = in_cfg.h;
        dest[1].line_ofs = in_cfg.w;
        dest[1].channel = 1;
        dest[1].batch_num = 1;

        //set dest3 as 1d tensor
        dest[2].sign = MAKEFOURCC('A','B','U','F');
        dest[2].va = out_buf->va + 2*dest[0].size; //< output address		 
        dest[2].pa = out_buf->pa + 2*dest[0].size;
        dest[2].size = in_cfg.loff * in_cfg.h;
        dest[2].fmt = HD_VIDEO_PXLFMT_B8;
        dest[2].width = in_cfg.w;
        dest[2].height = in_cfg.h;
        dest[2].line_ofs = in_cfg.w;
        dest[2].channel = 1;
        dest[2].batch_num = 1;
    }

    //scale
    if (p_op->op_opt == AI_OP_PREPROC_YUV2RGB_SCALE || p_op->op_opt == AI_OP_PREPROC_Y2Y_UV2UV || p_op->op_opt == AI_OP_PREPROC_RGB_SCALE || p_op->op_opt == AI_OP_PREPROC_Y8_SCALE) {
        preproc_param.scale_dim.w = SCALE_DIM_W;
        preproc_param.scale_dim.h = SCALE_DIM_H;
    }
    
    // plane mode
    if (p_op->op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE) {
        memset((VOID *)(in_buf->va + src[0].size*3), 0x80808080, src[0].size*3); //clear buffer for sub
        ret = hd_common_mem_flush_cache((VOID *)(in_buf->va + src[0].size*3), src[0].size*3);
        if(HD_OK != ret) {
            printf("flush cache failed.\n");
            return ret;
        }
        preproc_param.p_out_sub.pa = in_buf->pa + 3*src[0].size;
        preproc_param.p_out_sub.va = in_buf->va + 3*src[0].size;
        preproc_param.p_out_sub.width = in_cfg.w;
        preproc_param.p_out_sub.height = in_cfg.h;
        preproc_param.p_out_sub.line_ofs = in_cfg.w*3;
    }

    // dc mode
    if (p_op->op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_DC) {                 
        preproc_param.out_sub_color[0] = 128;
        preproc_param.out_sub_color[1] = 127;
        preproc_param.out_sub_color[2] = 126;
    }

    ret = vendor_ai3_op_do_preprocess (p_op_id, src, dest, &preproc_param);
    if(HD_OK != ret) {
        printf("vendor_ai3_op_do_preprocess failed.\n");
        return ret;
    }

    printf("inference done !\n");
    ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
    if(HD_OK != ret) {
        printf("flush cache failed.\n");
        return ret;
    }
        
	mem_save(&p_op->output_mem, "/mnt/sd/op_user_out.bin");

	return ret;
}
/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
MAIN(argc, argv)
{
	VIDEO_LIVEVIEW stream[2] = {0}; //0: net proc, 1: op path
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
    INT32 idx;
	HD_RESULT ret;

	NET_IN_CONFIG in_cfg = {
		.input_filename = "/mnt/sd/jpg/YUV420_SP_W512H376.bin",
		.w = 512,
		.h = 376,
		.c = 2,
		.loff = 512,
		.fmt = HD_VIDEO_PXLFMT_YUV420
	};

	NET_IN_CONFIG in_cfg_RGB = {
		.input_filename = "/mnt/sd/jpg/RGB888_SP_W500H375.bin",
		.w = 500,
		.h = 375,
		.c = 3,
		.loff = 500,
		.fmt = HD_VIDEO_PXLFMT_RGB888_PLANAR
	};

	NET_IN_CONFIG in_cfg_Y8 = {
		.input_filename = "/mnt/sd/jpg/Y8_SP_W500H375.bin",
		.w = 500,
		.h = 375,
		.c = 1,
		.loff = 500,
		.fmt = HD_VIDEO_PXLFMT_Y8
	};

    if(argc < 2){
		printf("usage : ai3_op (op_opt)\n"
			   "op-opt:\n"
			   "1  PREPROC (YUV2RGB)\n"
			   "2  PREPROC (YUV2RGB & scale)\n"
			   "3  PREPROC (YUV2RGB & meansub_plane)\n"
			   "4  PREPROC (YUV2RGB & meansub_dc)\n"
			   "5  PREPROC (Y2Y_UV2UV)\n"
			   "6  FC      (LL MODE)\n"
			   "7  SORT	   (Topn Sorting)\n"
			   "8  MATRIX  (MATMUL)\n"
			   "9  MATRIX  (MATADD)\n"
			   "10 MATRIX  (MATDUB)\n"
			   "14 PREPROC (RGB & SCALE)\n"
			   "15 PREPROC (Y8 & SCALE)\n");
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

	if (stream[0].net_op_opt == AI_OP_PREPROC_YUV2RGB)
		printf("Run PREPROC (YUV2RGB)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_YUV2RGB_SCALE)
		printf("Run PREPROC (YUV2RGB & scale)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE)
		printf("Run PREPROC (YUV2RGB & meansub_plane)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_DC)
		printf("Run PREPROC (YUV2RGB & meansub_dc)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_Y2Y_UV2UV)
		printf("Run PREPROC (Y2Y_UV2UV)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_FC_LL_MODE)
		printf("Run FC (LL MODE)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_TOPN_SORT)
		printf("Run SORT (Topn Sorting)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_MAT_MUL)
		printf("Run MATRIX (MATMUL)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_MAT_ADD)
		printf("Run MATRIX (MATADD)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_MAT_SUB)
		printf("Run MATRIX (MATSUB)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_RGB_SCALE)
		printf("Run PREPROC (RGB & scale)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_Y8_SCALE)
		printf("Run PREPROC (Y8 & scale)!\r\n");
	else{
		printf("Unknown op-opt = %d\r\n",stream[0].net_op_opt);
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
	// init mem
	if(stream[0].net_op_opt != AI_OP_FC && stream[0].net_op_opt != AI_OP_FC_LL_MODE && stream[0].net_op_opt != AI_OP_TOPN_SORT && stream[0].net_op_opt != AI_OP_MAT_MUL && stream[0].net_op_opt != AI_OP_MAT_ADD && stream[0].net_op_opt != AI_OP_MAT_SUB) { //for preproc
		INT32 idx = 0; // mempool index
		input_mem_config(stream[0].in_path, &mem_cfg, 0, idx);
	}

    if(stream[0].net_op_opt != AI_OP_FC && stream[0].net_op_opt != AI_OP_FC_LL_MODE && stream[0].net_op_opt != AI_OP_TOPN_SORT && stream[0].net_op_opt != AI_OP_MAT_MUL && stream[0].net_op_opt != AI_OP_MAT_ADD && stream[0].net_op_opt != AI_OP_MAT_SUB) { //for preproc
        // set open config
		if (stream[0].net_op_opt == AI_OP_PREPROC_RGB_SCALE){
			ret = input_set_config(stream[0].in_path, &in_cfg_RGB);
	    	if (HD_OK != ret) {
	    		printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
			    goto exit;
	    	}
		}else if (stream[0].net_op_opt == AI_OP_PREPROC_Y8_SCALE){
			ret = input_set_config(stream[0].in_path, &in_cfg_Y8);
	    	if (HD_OK != ret) {
	    		printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
			    goto exit;
	    	}
		}else{
	    	ret = input_set_config(stream[0].in_path, &in_cfg);
	    	if (HD_OK != ret) {
	    		printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
			    goto exit;
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
		case AI_OP_FC_LL_MODE:
        {
			ret = operator_do_init_fc(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_init_fc fail = %d\n", ret);
				goto exit;
			}
			ret = operator_do_fc(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_fc fail = %d\n", ret);
				goto exit;
			}
			ret = operator_do_uninit_fc(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_uninit_fc fail = %d\n", ret);
				goto exit;
			}
	        break;
		}
		case AI_OP_PREPROC_YUV2RGB:
		case AI_OP_PREPROC_YUV2RGB_SCALE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_DC:
		case AI_OP_PREPROC_Y2Y_UV2UV:
		case AI_OP_PREPROC_RGB_SCALE:
		case AI_OP_PREPROC_Y8_SCALE:
		{
            ret = operator_do_init_preproc(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_init_preproc fail = %d\n", ret);
				goto exit;
			}
			ret = operator_do_preprocess(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_preprocess fail = %d\n", ret);
				goto exit;
			}
            ret = operator_do_uninit_preproc(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_uninit_preproc fail = %d\n", ret);
				goto exit;
			}
			break;
		}
		case AI_OP_MAT_MUL:
		case AI_OP_MAT_ADD:
		case AI_OP_MAT_SUB:
        {
			ret = operator_do_init_matrix(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_init_matrix fail = %d\n", ret);
				goto exit;
			}
			ret = operator_do_matrix(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_matrix fail = %d\n", ret);
				goto exit;
			}
			ret = operator_do_uninit_matrix(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_uninit_matrix fail = %d\n", ret);
				goto exit;
			}
	        break;
		}
		case AI_OP_TOPN_SORT:
		{
			ret = operator_do_init_topn(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_init_topn fail = %d\n", ret);
				goto exit;
			}
			ret = operator_do_topn(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_topn fail = %d\n", ret);
				goto exit;
			}
			ret = operator_do_uninit_topn(&stream[0]);
			if (ret != HD_OK) {
				printf("operator_do_uninit_topn fail = %d\n", ret);
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
	if(HD_OK != ret) {
		printf("operator do function failed.\n");
		return ret;
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
