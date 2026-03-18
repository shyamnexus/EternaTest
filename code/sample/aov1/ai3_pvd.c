#include "ai3_pvd.h"

#define MD_SUPPORT		0
#define VENDOR_AI_CFG               0x000f0000  //vendor ai config
#define AI_RGB_BUFSIZE(w, h)        (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))
#define SAVE_SCALE          0
#define LIMIT_PVD           1
#define MAX_FRAME_WIDTH     2880
#define MAX_FRAME_HEIGHT    1620
#define SCALE_BUF_SIZE      (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3)
#define PVD_IMG_WIDTH			1024
#define PVD_IMG_HEIGHT          576
#define PVD_IMG_BUF_SIZE        (PVD_IMG_WIDTH * PVD_IMG_HEIGHT)

// PVD setting
static UINT32 AI_PVD_MODEL_SIZE;
static UINT32 AI_PVD_BUF_SIZE;
static UINT32 AI_LIMIT_FDET_MODEL_SIZE;
static UINT32 AI_LIMIT_FDET_BUF_SIZE;

CHAR *combin_model_name;// combine_model_320prune_ppdw_novlcen
UINT PVD_MAX_DISTANCE_MODE = 0;


static HD_COMMON_MEM_VB_BLK g_blk[3];

PVD_THREAD_MEM pvd_thread_parms = {0};
HD_VIDEO_FRAME video_frame_PVD[SENSOR_MAX_NUM] = {0};
UINT32     ai_thread_flow_start = 0;
BOOL exit_ai_thread = FALSE;
BOOL start_ai_thread = FALSE;
pthread_t nn_thread_id;

//--------------------------------------//
extern unsigned int wait_ai_thd_done;
extern BOOL SYS_STATUS;
extern HD_PATH_ID ai_thread_pvd_path[SENSOR_MAX_NUM];
extern UINT32 g_sensor_num;
//--------------------------------------//

HD_RESULT system_get_mem(UINT32 ddr, NN_CFG_BUF_M *buf, UINT32 size, INT32 index)
{
	HD_RESULT ret;
	uintptr_t pa   = 0;
	void  *va   = NULL;
	HD_COMMON_MEM_DDR_ID ddr_id = ddr;
	// HD_COMMON_MEM_VB_BLK blk;

	g_blk[index] = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, size, ddr_id);     //HD_COMMON_MEM_USER_DEFINIED_POOL + id HD_COMMON_MEM_CNN_POOL
	if (HD_COMMON_MEM_VB_INVALID_BLK == g_blk[index]) {
		printf("hd_common_mem_get_block fail\r\n");
		return HD_ERR_NG;
	}
	pa = hd_common_mem_blk2pa(g_blk[index]);
	if (pa == 0) {
		printf("not get buffer, pa=%08x\r\n", (int)pa);
		return HD_ERR_NOMEM;
	}
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, size);

	if (va == 0) {
		ret = hd_common_mem_munmap(va, size);
		if (ret != HD_OK) {
			printf("mem unmap fail\r\n");
			return ret;
		}
	}

	buf->pa = pa;
	buf->va = (uintptr_t)va;
	buf->size = size;

	return HD_OK;
}

HD_RESULT system_free_mem(NN_CFG_BUF_M *buf, INT32 index)
{
	HD_RESULT ret = HD_OK;
	/* Release in buffer */
	if (buf->va) {
		ret = hd_common_mem_munmap((void *)buf->va, buf->size);
		if (ret != HD_OK) {
			printf("mem_uninit : (g_mem.va)hd_common_mem_munmap fail.\r\n");
			return ret;
		}
	}
	ret = hd_common_mem_release_block(g_blk[index]);
	if (ret != HD_OK) {
		printf("hd_common_mem_release_block fail.\r\n");
		return ret;
	}
	buf->pa = 0;
	buf->va = 0;
	buf->size = 0;
	
	return HD_OK;
}

HD_RESULT mem_alloc(VENDOR_AIS_FLOW_MEM_PARM *mem_parm, CHAR* name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	UINTPTR pa   = 0;
	void  *va   = NULL;

	//alloc private pool
	ret = hd_common_mem_alloc(name, &pa, (void**)&va, size, DDR_ID0);
	if (ret!= HD_OK) {
		printf("[ERR] %s hd_common_mem_alloc fail=%d\n", __func__, ret);
		return ret;
	}

	mem_parm->pa   = pa;
	mem_parm->va   = (UINTPTR)va;
	mem_parm->size = size;
	//mem_parm->blk  = (UINT32)-1;
	return HD_OK;
}

HD_RESULT mem_free(VENDOR_AIS_FLOW_MEM_PARM *mem_parm)
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
	//mem_parm->blk = (UINT32)-1;
	return HD_OK;
}

INT32 load_bin_file_to_mem(CHAR *filename, UINTPTR va)
{
	FILE *fd;
	INT32 size = 0;
	fd = fopen(filename, "rb");
	if (!fd) {
		printf("cannot read %s\n", filename);
		return HD_ERR_NOT_OPEN;
	}

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	if (size < 0) {
		printf("getting %s size failed\n", filename);
	} else if ((INT32)fread((VOID *)va, 1, size, fd) != size) {
		printf("read size < %ld\n", size);
		size = -1;
	};
	//mem_parm->size = size;

	if (fd) {
		fclose(fd);
	}
	return size;
}

HD_RESULT ai_get_model_name(CHAR **name)
{
	char *chip_name = getenv("NVT_CHIP_ID");

	if (NULL == *name) {
		return HD_ERR_PARAM;
	}

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) { ////-> 539A flow
		combin_model_name = "/etc/combin_model_539A.bin";
	} else { ////-> 538 flow
		combin_model_name = "/etc/combin_model.bin";
	}

	*name = combin_model_name;

	return HD_OK;
}

HD_RESULT ai_buffer_total_size_get(UINT32 *ai_buff_size)
{
	char *chip_name = getenv("NVT_CHIP_ID");

	if (chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) { ////-> 539A flow
		AI_PVD_MODEL_SIZE = (794416);
		AI_PVD_BUF_SIZE = (1895296);
		AI_LIMIT_FDET_MODEL_SIZE = (250296);
		AI_LIMIT_FDET_BUF_SIZE = (342080);
	} else { ////-> 538 flow
		AI_PVD_MODEL_SIZE = (793840);
		AI_PVD_BUF_SIZE = (1893632);
		AI_LIMIT_FDET_MODEL_SIZE = (249784);
		AI_LIMIT_FDET_BUF_SIZE = (340544);
	}

	*ai_buff_size = AI_PVD_BUF_SIZE + AI_LIMIT_FDET_BUF_SIZE + 0x2000;

	return HD_OK;
}

HD_RESULT assign_ai_buf(NN_CFG_BUF_M *buf, PVD_THREAD_MEM *parm)
{
	HD_RESULT ret = HD_OK;

	parm->pvd_model_mem.pa = buf->pa;
	parm->pvd_model_mem.va = buf->va;
	parm->pvd_model_mem.size = ALIGN_CEIL(AI_PVD_MODEL_SIZE, 2048); // PVD model bin

	parm->limit_model_mem.pa = parm->pvd_model_mem.pa + parm->pvd_model_mem.size;
	parm->limit_model_mem.va = parm->pvd_model_mem.va + parm->pvd_model_mem.size;
	parm->limit_model_mem.size = ALIGN_CEIL(AI_LIMIT_FDET_MODEL_SIZE, 2048); // Limit fdet model bin

	parm->pvd_mem.pa = parm->limit_model_mem.pa + parm->limit_model_mem.size;
	parm->pvd_mem.va = parm->limit_model_mem.va + parm->limit_model_mem.size;
	parm->pvd_mem.size = AI_PVD_BUF_SIZE - ALIGN_CEIL(AI_PVD_MODEL_SIZE, 2048); // PVD working buffer

	parm->limit_fdet_mem.pa = parm->pvd_mem.pa + parm->pvd_mem.size;
	parm->limit_fdet_mem.va = parm->pvd_mem.va + parm->pvd_mem.size;
	parm->limit_fdet_mem.size = AI_LIMIT_FDET_BUF_SIZE + 2048 - ALIGN_CEIL(AI_LIMIT_FDET_MODEL_SIZE, 2048); // Limit fdet model bin

	return ret;
}

VOID *nn_thread_api(VOID *arg)
{
	start_ai_thread = FALSE;

	HD_RESULT ret = HD_OK;
	VENDOR_AI3_BUF p_src_img;
	UINT32 i;
	
	PVD_THREAD_MEM *pvd_parm = (PVD_THREAD_MEM *)arg;
	NN_CFG_BUF_M pvdcnn_buf = pvd_parm->pvd_mem;
	NN_CFG_BUF_M pvd_model_mem = pvd_parm->pvd_model_mem;
	FLOAT dump_ratios[2] = {1.0};
	
	UINT32 DEBUG_COUNT = 0;
	UINT32 is_first_yuv = 1;	

	#define DBGINFO_BUFSIZE()	(0x200)
	#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
	
	UINTPTR phy_addr_main, vir_addr_main = 0;
	UINT32 yuv_size;

	int size = load_bin_file_to_mem(combin_model_name, pvd_model_mem.va);
	if (size <= 0) {
		printf("[ERR] %s load_bin_file_to_mem fail=%d\n", __func__, ret);
		goto exit_thread;
	}

	UINT32 debug_enable = 0;

	// PVD Init
	NN_FILE_PATH pvd_files = {0};
	snprintf(pvd_files.model_file, PATH_LENGTH_M, "%s", "");
	snprintf(pvd_files.para_file, PATH_LENGTH_M, "%s", "");
	ret = pvdcnn_init(pvdcnn_buf, 0, 0, &pvd_model_mem, (VOID *)NULL, &pvd_files, debug_enable);
	if(ret != HD_OK){
		printf("[ERR] %s pvdcnn_init fail=%d\n", __func__, ret);
		goto exit_thread;
	}

	// Turn off bounding box stabilization
	PVD_ADJUSTABLE_PARAM pvd_params = {0};
	ret = pvdcnn_params_get(pvdcnn_buf, &pvd_params);
	pvd_params.TRACKER_ENBALE = 0;
	pvd_params.confindence_threshold = 0.3;
	ret = pvdcnn_params_set(pvdcnn_buf, &pvd_params);

	// Limit Fdet Init
	NN_FILE_PATH limit_files = { 0 };
	snprintf(limit_files.model_file, PATH_LENGTH_M, "%s", "");
	snprintf(limit_files.para_file, PATH_LENGTH_M, "%s", "");
	NN_CFG_BUF_M limit_fdet_buf = pvd_parm->limit_fdet_mem;
	NN_CFG_BUF_M limit_fdet_model_mem = pvd_parm->limit_model_mem;
	ret = limit_fdet_init(pvd_parm->limit_fdet_mem, 1, &limit_fdet_model_mem, (VOID *)NULL, &limit_files, debug_enable);
	if (ret != HD_OK) {
		printf("[ERR] %s limit_fdet_init fail=%d\n", __func__, ret);
		goto exit_thread;
	}
	LIMIT_FDET_ADJUSTABLE_PARAM limit_fdet_params = {0};
	ret = limit_fdet_params_get(limit_fdet_buf, &limit_fdet_params);
	limit_fdet_params.detection_threshold = 0.95f;
	limit_fdet_params.cls_threshold[0] = 0.5f; // Increase people detection rate
	ret = limit_fdet_params_set(limit_fdet_buf, &limit_fdet_params);

	INT32 pvd_out_num[SENSOR_MAX_NUM] = {0};
	NN_RESULT_M *pvd_final_result[SENSOR_MAX_NUM] ={NULL};
	printf("=============== Wait flag_ai_start ===================\r\n");
	while (ai_thread_flow_start == 0) usleep(3 * 1000);
	printf("=============== flag_ai_start ===================\r\n");
	
	int pvd_count=0;
	do {
			while (wait_ai_thd_done == 0 && (!exit_ai_thread)) 
			{
				usleep(10);
			}
			if(exit_ai_thread)
				break;
			
			
#if 0
	if(APP_MODE>=1)
	{
	
		wait_md_thd_done=1;	// Trigger MD
		if(APP_MODE!=3)		// If APP_MODE = 3, always use AI 
		{
			if(SYS_STATUS == LOW_POWER_MODE)
			{
			    while(wait_md_thd_done==1)
			    {
				    usleep(500);
			    }
    	    	if(APP_MODE==2)		// Always no AI
    	    	{
				    goto RELEASE_AI;
			    }	
			    if(MD_result)	// MD hit
			    {
					printf("MD\r\n");
					osg_blink(stream[0].enc_mask_path[1], 1);
			    }
				else			// MD not hit
				{
					osg_blink(stream[0].enc_mask_path[1], 0);
				    goto RELEASE_AI;
				}

			}	
			else
			{
				//printf("pvd_count %d\n",(int)pvd_count);
				if(APP_MODE==2)		// If APP_MODE = 2, always no AI
    	    	{
				    goto RELEASE_AI;
			    }	
			}
		}
	}
#endif
	
	if(SYS_STATUS == LOW_POWER_MODE || 
	(  SYS_STATUS == NORMAL_MODE && pvd_count % 3 == 0 ))	// Do AI PVD per 3 frame
	{
		for (i = 0; i < g_sensor_num; i++) {
			pvd_out_num[i] = 0;
			//check_t2 = hd_gettime_ms();
			// Get Frame va Start
			phy_addr_main = hd_common_mem_blk2pa(video_frame_PVD[i].blk); // Get physical addr
			if (phy_addr_main == 0) {
				printf("[WRN] %s hd_common_mem_blk2pa fail=%d\n", __func__, ret);
				goto RELEASE_AI;
			}

			yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(PVD_IMG_WIDTH, PVD_IMG_HEIGHT, HD_VIDEO_PXLFMT_YUV420);
			// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
			vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, yuv_size);
			if (vir_addr_main == 0) {
				printf("[WRN] %s hd_common_mem_mmap fail=%d\n", __func__, ret);
					goto RELEASE_AI;
			}
			#define PHY2VIRT_MAIN_AI(pa) (vir_addr_main + ((pa) - phy_addr_main))
			// printf("AI sub stream dim: width = %ld, height = %ld.\r\n", video_frame_PVD.dim.w, video_frame_PVD.dim.h);
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN_AI(video_frame_PVD[i].phy_addr[0]);

			// trans video_frame_PVD to p_src_img
			p_src_img.fmt      = HD_VIDEO_PXLFMT_YUV420;
			p_src_img.channel  = 2;
			p_src_img.pa       = video_frame_PVD[i].phy_addr[0];
			p_src_img.va       = (UINTPTR)ptr; 
			p_src_img.line_ofs = PVD_IMG_WIDTH;
			p_src_img.width    = video_frame_PVD[i].dim.w;
			p_src_img.height   = video_frame_PVD[i].dim.h;
			p_src_img.sign     = MAKEFOURCC('A', 'B', 'U', 'F');
			p_src_img.size     = p_src_img.line_ofs * video_frame_PVD[i].dim.h * 3 / 2;

			//check_t3 = hd_gettime_ms();
			// ret = pvdcnn_process_inplace(pvdcnn_buf, &src_img, &limit_fdet_buf, &pvd_final_result, &pvd_out_num, (PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
			printf("A");
			//gpio_set_value(38, 1);
			ret = pvdcnn_process_inplace(pvd_parm->pvd_mem, (void*)&p_src_img, &(pvd_parm->limit_fdet_mem), &pvd_final_result[i], &pvd_out_num[i], (PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW));
			//printf("run pvdcnn_process_inplace_getone_pd done\r\n");
			//gpio_set_value(38, 0);
			// mummap for frame buffer
		}
UNMAP_VA :
		ret = hd_common_mem_munmap((void *)vir_addr_main, yuv_size);
		if (ret != HD_OK) {
			printf("[WRN] %s hd_common_mem_munmap fail=%d\n", __func__, ret);
    			goto RELEASE_AI;
		}
	}
	pvd_count++;

RELEASE_AI:

		for (i = 0; i < g_sensor_num; i++) {
			ret = hd_videoproc_release_out_buf(ai_thread_pvd_path[i], &video_frame_PVD[i]);
			if (ret != HD_OK) {
				printf("[WRN] %s hd_videoproc_release_out_buf fail=%d\n", __func__, ret);
			}
		}
		
		// AI PVD result processing
		for (i = 0; i < g_sensor_num; i++) {
			if (pvd_out_num[i] > 0) {
				switch(pvd_final_result[i]->category) {
					case 1:
						pvd_parm->pvd_ret = 1;
						break;
					default:
						pvd_parm->pvd_ret = 0;
				}
				break;
			}
			pvd_parm->pvd_ret = 0;
		}
		
		// AI PVD Done
SKIP_AI:
		wait_ai_thd_done = 0;

	} while (!exit_ai_thread);//

exit_thread:
	ret = pvdcnn_uninit(pvdcnn_buf);
	if (HD_OK != ret) {
		printf("[ERR] %s pvdcnn_uninit fail=%d\n", __func__, ret);
	}

#if (LIMIT_FDET_PVD)
	ret = limit_fdet_uninit(limit_fdet_buf);
	if (HD_OK != ret) {
		printf("[ERR] %s limit_fdet_uninit fail=%d\n", __func__, ret);
	}
#endif

	ret = vendor_ai3_dev_uninit();
	if (ret != HD_OK) {
		printf("[ERR] %s vendor_ai_uninit fail=%d\n", __func__, ret);
	}

	start_ai_thread = TRUE;

	return 0;
}

//HD_RESULT ai_mem_init(void)
//{
//    HD_RESULT ret = 0;
//    //mem init for AI PVD
//	{
//		VENDOR_AI3_DEV_CFG dev_cfg = {0};
//
//		ret = vendor_ai3_dev_init(&dev_cfg);
//		if (ret != HD_OK) {
//			printf("vendor_ai3_dev_init fail=%d\n", ret);
//			return ret;
//		}
//	}
//
//	NN_CFG_BUF_M ai_allbuf_mem = {0};
//	ret = system_get_mem(DDR_ID0, &(ai_allbuf_mem), (UINT32)ai_buffer_total_size, (INT32)0);
//	if (ret != HD_OK) {
//		printf("get pvd_buf fail=%d\n", ret);
//		return ret;
//	}
//
//	ret = assign_ai_buf(&ai_allbuf_mem, &pvd_thread_parms);
//	if (ret != HD_OK) {
//		printf("ERR: ai buf assign fail (%d)!!\n", ret);
//		return -1;
//	}
//
//	ret = system_get_mem(DDR_ID0, &(pvd_thread_parms.scale_mem), (UINT32)(SCALE_BUF_SIZE), (INT32)1);
//	if (ret != HD_OK) {
//		printf("get scale_mem fail=%d\n", ret);
//		return ret;
//	}
//
//	printf("pvdcnn_allbuf: pa=(%#x), va=(%#x), size=(%ld)\n", 
//        (unsigned int)pvd_thread_parms.pvd_mem.pa, (unsigned int)pvd_thread_parms.pvd_mem.va, pvd_thread_parms.pvd_mem.size);
//	printf("limit_fdet_buf: pa=(%#x), va=(%#x), size=(%ld)\n", 
//        (unsigned int)pvd_thread_parms.limit_fdet_mem.pa, (unsigned int)pvd_thread_parms.limit_fdet_mem.va, pvd_thread_parms.limit_fdet_mem.size);
//    return ret;
//}

HD_RESULT ai_mem_uninit(void)
{
    HD_RESULT ret = 0;
    // uninit memory
	//ret = mem_free();
	//if (ret != HD_OK) {
	//	printf("[ERR] %s AI mem_free fail=%d\n", __func__, ret);
	//}
    return ret;
}

