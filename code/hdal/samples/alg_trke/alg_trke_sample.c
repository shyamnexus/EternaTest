/**
 * @file vendor_trke_sample.c
 * @brief start trke sample.
 * @author ALG1-CV
 * @date in the year 2018
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hd_debug.h"
#include "hdal.h"
#include "vendor_trke.h"
#include "libtrke/libtrke.h"
#include <kwrap/perf.h>
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>            //for pthread API
#define MAIN(argc, argv)        int main(int argc, char** argv)
#define GETCHAR()               getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/signal.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>     //for sleep API
#define sleep(x)                vos_util_delay_ms(1000*(x))
#define msleep(x)               vos_util_delay_ms(x)
#define usleep(x)               vos_util_delay_us(x)
#include <kwrap/examsys.h>  //for MAIN(), GETCHAR() API
#define MAIN(argc, argv)        EXAMFUNC_ENTRY(vendor_ive, argc, argv)
#define GETCHAR()               NVT_EXAMSYS_GETCHAR()
#endif

#define MAX_IMG_WIDTH         4000
#define MAX_IMG_HEIGHT        4000
#define IMG_BUF_SIZE          (MAX_IMG_WIDTH * MAX_IMG_HEIGHT)
#define memory_num             12
#define BYTE_ALIGN(a)         ((a+7)>>3)

typedef struct _MEM_RANGE {
	UINTPTR va;        ///< Memory buffer starting address
	UINTPTR addr;      ///< Memory buffer starting address
	UINT32 size;      ///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} MEM_RANGE, *PMEM_RANGE;

static void share_memory_init(MEM_RANGE *p_share_mem)
{

    HD_COMMON_MEM_VB_BLK blk;
	UINT8 i;
	UINTPTR pa, va;
	UINT32 blk_size = IMG_BUF_SIZE;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT ret;
    
	//mem : pstSrc/pstCorner/tmp buffer
	for(i=0;i<memory_num;i++){
		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}
	for(i=0;i<memory_num;i++){
		blk_size = IMG_BUF_SIZE;		
		blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_BLK, blk_size, ddr_id);
		if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("err:get block fail\r\n", blk);
			return;
		}
		pa = hd_common_mem_blk2pa(blk);
		if (pa == 0) {
			printf("err:blk2pa fail, blk = 0x%x\r\n", blk);
			goto blk2pa_err;
		}
		
		if (pa > 0) {
			va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size);
			if (va == 0) {
				goto map_err;
			}
		}
		p_share_mem[i].addr = pa;
		p_share_mem[i].va   = va;
		p_share_mem[i].size = blk_size;
		p_share_mem[i].blk  = blk;
		
	}
	return;
blk2pa_err:
map_err:
	for (; i > 0 ;) {
		i -= 1;
		ret = hd_common_mem_release_block(p_share_mem[i].blk);
		if (HD_OK != ret) {
			printf("err:release blk fail %d\r\n", ret);
			return;
		}
	}
}

static void share_memory_exit(MEM_RANGE *p_share_mem)
{
	UINT8 i;
	HD_RESULT ret;
	for(i=0;i<memory_num;i++){
		if (p_share_mem[i].va != 0) {
			ret = hd_common_mem_munmap((void*)p_share_mem[i].va, p_share_mem[i].size);
			if (HD_OK != ret) {
                printf("err:hd_common_mem_munmap err: \r\n\r\n");
            }
		}
		if (p_share_mem[i].blk != HD_COMMON_MEM_VB_INVALID_BLK) {
			ret = hd_common_mem_release_block(p_share_mem[i].blk);
			if (HD_OK != ret) {
				printf("err:hd_common_mem_release_block err: %d\r\n", ret);
			}
		}
		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}
}

static int mem_init(void)
{
	HD_RESULT                 ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg  = {0};

	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_BLK;
	mem_cfg.pool_info[0].blk_size = IMG_BUF_SIZE;
	mem_cfg.pool_info[0].blk_cnt = memory_num; 
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;

	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("err:hd_common_mem_init err: %d\r\n", ret);
	}

	return ret;



}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	//share_memory_exit();
	ret = hd_common_mem_uninit();
	return ret;
}

static UINT32 trke_load_file(CHAR *p_filename, UINTPTR va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const UINTPTR addr = va;

	fd = fopen(p_filename, "rb");
	if (!fd) {
		printf("cannot read %s\r\n", p_filename);
		return 0;
	}

	fseek ( fd, 0, SEEK_END );
	file_size = ALIGN_CEIL_4( ftell(fd) );
	fseek ( fd, 0, SEEK_SET );

	read_size = fread ((void *)addr, 1, file_size, fd);
	if (read_size != file_size) {
		printf("size mismatch, real = %d, idea = %d\r\n", (int)read_size, (int)file_size);
	}
	fclose(fd);
	return read_size;
}



MAIN(argc, argv)
{
    HD_RESULT                          ret;
	LIB_TRKE_ERROR_CODE lib_trke_ret;
	char in_file1[64],in_file2[64],in_file3[64],in_file4[64],in_file5[64],in_file6[64],in_file7[64],in_file8[64],in_file9[64];
	char out_file1[64], out_file2[64],out_file3[64];
    FILE  *fd;
    UINT32 file_size = 0;    
    UINT32 points_size = 0 ;
	int width = 1280 ;
	int height = 720;	
	int out_err_size,out_status_size; //out_nextpts_size, 
    UINT16 PtsNum = 500 ;
	UINT8 MinEigThr = 1;
	UINT8 IterCnt = 20;
	UINT8 Eps = 1 ;
	BOOL enUseInitFlow = FALSE;
	UINT8 MaxLevel = 3;
	UINT8 PatchSize = 0;
	VOS_TICK    hw_tick_begin, hw_tick_end;

	TRKE_SRC_PYR_S pstPyr = {0} ;
    TRKE_PYR_CTRL_S pstTRKECtrl = {0} ;
	TRKE_MEM_INFO_S pstPrevPt = {0};
	TRKE_MEM_INFO_S pstNextPt = {0};
	TRKE_MEM_INFO_S pstStatus = {0};
	TRKE_MEM_INFO_S pstErr = {0};
    MEM_RANGE share_mem[12] = {0};
	
	// init common module
	ret = hd_common_init(0);
    if(ret != HD_OK) {
        printf("init fail=%d\n", ret);
        goto comm_init_fail;
    }
	// init memory
	ret = mem_init();
    if(ret != HD_OK) {
        printf("init fail=%d\n", ret);
        goto mem_init_fail;
    }

    // init share memory
	share_memory_init(share_mem);

	//trke_load_input();
#if defined(__FREERTOS)
	snprintf(in_file1, 64, "A:\\trke\\input\\trke63\\DI0.bin"); 
	snprintf(in_file2, 64, "A:\\trke\\input\\trke63\\DI1.bin");
	snprintf(in_file3, 64, "A:\\trke\\input\\trke63\\DI3.bin");	
	snprintf(in_file4, 64, "A:\\trke\\input\\trke62\\DI0.bin");  
	snprintf(in_file5, 64, "A:\\trke\\input\\trke62\\DI1.bin");	
	snprintf(in_file6, 64, "A:\\trke\\input\\trke61\\DI0.bin");
	snprintf(in_file7, 64, "A:\\trke\\input\\trke61\\DI1.bin");	
	snprintf(in_file8, 64, "A:\\trke\\input\\trke60\\DI0.bin");
	snprintf(in_file9, 64, "A:\\trke\\input\\trke60\\DI1.bin");
	
#else
    snprintf(in_file1, 64, "//mnt//sd//trke//input//trke63//DI0.bin");  //layer 3 , prevPyr
	snprintf(in_file2, 64, "//mnt//sd//trke//input//trke63//DI1.bin");  //layer 3 , nextPyr
	snprintf(in_file3, 64, "//mnt//sd//trke//input//trke63//DI3.bin");	
	snprintf(in_file4, 64, "//mnt//sd//trke//input//trke62//DI0.bin");  //layer 2 , prevPyr
	snprintf(in_file5, 64, "//mnt//sd//trke//input//trke62//DI1.bin");  //layer 2 , nextPyr		
	snprintf(in_file6,  64, "//mnt//sd//trke//input//trke61//DI0.bin"); //layer 1 , prevPyr
	snprintf(in_file7,  64, "//mnt//sd//trke//input//trke61//DI1.bin"); //layer 1 , nextPyr	
	snprintf(in_file8, 64, "//mnt//sd//trke//input//trke60//DI0.bin");  //layer 0 , prevPyr
	snprintf(in_file9, 64, "//mnt//sd//trke//input//trke60//DI1.bin");  //layer 0 , nextPyr
	
#endif
	
    file_size = trke_load_file(in_file1, share_mem[0].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file1);
			goto exit;			
    }	
	file_size = trke_load_file(in_file2, share_mem[1].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file2);
			goto exit;			
    }	
	file_size = trke_load_file(in_file3, share_mem[2].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file3);
			goto exit;			
    }	
	file_size = trke_load_file(in_file4, share_mem[3].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file4);
			goto exit;			
    }	
	file_size = trke_load_file(in_file5, share_mem[4].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file5);
			goto exit;			
    }	
	file_size = trke_load_file(in_file6, share_mem[5].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file6);
			goto exit;			
    }	
	file_size = trke_load_file(in_file7, share_mem[6].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file7);
			goto exit;			
    }	
	file_size = trke_load_file(in_file8, share_mem[7].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file8);
			goto exit;			
    }	
	file_size = trke_load_file(in_file9, share_mem[8].va);
    if (file_size == 0) {
			printf("load %s fail\r\n", in_file9);
			goto exit;			
    }	
	
	
	// call vendor_trke_init once for initialize TRKE
	ret = vendor_trke_init();
	if (ret != HD_OK) {
		printf("%s: vendor_trke_init error (%d)\r\n", __func__, ret);
		goto exit;
	}


	//set input & output info   	
	pstPyr.pstPrevPyr[3].enDataType = LIB_TRKE_DATA_TYPE_U8C1 ;
	pstPyr.pstPrevPyr[3].u64Pa = share_mem[0].addr;
	pstPyr.pstPrevPyr[3].u64Va = share_mem[0].va;
	pstPyr.pstPrevPyr[3].u32Width = (width >> 3);
	pstPyr.pstPrevPyr[3].u32Height = (height>>3);
	pstPyr.pstPrevPyr[3].u32Stride = (width>>3);
	
	pstPyr.pstNextPyr[3].enDataType = LIB_TRKE_DATA_TYPE_U8C1 ;
	pstPyr.pstNextPyr[3].u64Pa = share_mem[1].addr;
	pstPyr.pstNextPyr[3].u64Va = share_mem[1].va;
	pstPyr.pstNextPyr[3].u32Width = (width >> 3);
	pstPyr.pstNextPyr[3].u32Height = (height >> 3);
	pstPyr.pstNextPyr[3].u32Stride = (width >> 3);
		
	pstPyr.pstPrevPyr[2].enDataType = LIB_TRKE_DATA_TYPE_U8C1 ;
	pstPyr.pstPrevPyr[2].u64Pa = share_mem[3].addr;
	pstPyr.pstPrevPyr[2].u64Va = share_mem[3].va;
	pstPyr.pstPrevPyr[2].u32Width = (width >> 2);
	pstPyr.pstPrevPyr[2].u32Height = (height >> 2);
	pstPyr.pstPrevPyr[2].u32Stride = (width >> 2);
	
	pstPyr.pstNextPyr[2].enDataType = LIB_TRKE_DATA_TYPE_U8C1 ;
	pstPyr.pstNextPyr[2].u64Pa = share_mem[4].addr;
	pstPyr.pstNextPyr[2].u64Va = share_mem[4].va;
	pstPyr.pstNextPyr[2].u32Width = (width >>2);
	pstPyr.pstNextPyr[2].u32Height = (height >>2);
	pstPyr.pstNextPyr[2].u32Stride = (width >>2);	
		
	pstPyr.pstPrevPyr[1].enDataType = LIB_TRKE_DATA_TYPE_U8C1 ;
	pstPyr.pstPrevPyr[1].u64Pa = share_mem[5].addr;
	pstPyr.pstPrevPyr[1].u64Va = share_mem[5].va;
	pstPyr.pstPrevPyr[1].u32Width = (width >>1);
	pstPyr.pstPrevPyr[1].u32Height = (height >>1);
	pstPyr.pstPrevPyr[1].u32Stride = (width >>1);
	
	pstPyr.pstNextPyr[1].enDataType = LIB_TRKE_DATA_TYPE_U8C1 ;
	pstPyr.pstNextPyr[1].u64Pa = share_mem[6].addr;
	pstPyr.pstNextPyr[1].u64Va = share_mem[6].va;
	pstPyr.pstNextPyr[1].u32Width = (width >>1);
	pstPyr.pstNextPyr[1].u32Height = (height >>1);
	pstPyr.pstNextPyr[1].u32Stride = (width >>1);	


	pstPyr.pstPrevPyr[0].enDataType = LIB_TRKE_DATA_TYPE_U8C1 ;
	pstPyr.pstPrevPyr[0].u64Pa = share_mem[7].addr;
	pstPyr.pstPrevPyr[0].u64Va = share_mem[7].va;
	pstPyr.pstPrevPyr[0].u32Width = width;
	pstPyr.pstPrevPyr[0].u32Height = height;
	pstPyr.pstPrevPyr[0].u32Stride = width;
	
	pstPyr.pstNextPyr[0].enDataType = LIB_TRKE_DATA_TYPE_U8C1 ;
	pstPyr.pstNextPyr[0].u64Pa = share_mem[8].addr;
	pstPyr.pstNextPyr[0].u64Va = share_mem[8].va;
	pstPyr.pstNextPyr[0].u32Width = width;
	pstPyr.pstNextPyr[0].u32Height = height;
	pstPyr.pstNextPyr[0].u32Stride = width;	

	points_size = PtsNum*sizeof(UINT32)*2;	
	pstPrevPt.u64Pa = share_mem[2].addr;
	pstPrevPt.u64Va = share_mem[2].va;	
	pstPrevPt.u32Size = points_size ;
	
	pstNextPt.u64Pa = share_mem[9].addr;
	pstNextPt.u64Va = share_mem[9].va;	
	pstNextPt.u32Size = points_size ;
	
	pstErr.u64Pa = share_mem[10].addr;
	pstErr.u64Va = share_mem[10].va;	
	pstErr.u32Size = PtsNum *sizeof(UINT16);
		
	pstStatus.u64Pa = share_mem[11].addr;
	pstStatus.u64Va = share_mem[11].va;	
	pstStatus.u32Size = (PtsNum *sizeof(UINT8)/8) +1;	
		
	
	pstTRKECtrl.enUseInitFlow = enUseInitFlow ;
	pstTRKECtrl.u16PtsNum = PtsNum;
	pstTRKECtrl.u8MaxLevel = MaxLevel;
	pstTRKECtrl.u0q8MinEigThr = MinEigThr;	
	pstTRKECtrl.u8IterCnt = IterCnt;
	pstTRKECtrl.u0q8Eps = Eps;		
	if(trke_is_539a()){
		pstTRKECtrl.patch_size = PatchSize;
	}
	
	hd_common_mem_flush_cache((VOID *)pstPyr.pstPrevPyr[3].u64Va, pstPyr.pstPrevPyr[3].u32Width*pstPyr.pstPrevPyr[3].u32Height);	
	hd_common_mem_flush_cache((VOID *)pstPyr.pstPrevPyr[2].u64Va, pstPyr.pstPrevPyr[2].u32Width*pstPyr.pstPrevPyr[2].u32Height);	
	hd_common_mem_flush_cache((VOID *)pstPyr.pstPrevPyr[1].u64Va, pstPyr.pstPrevPyr[1].u32Width*pstPyr.pstPrevPyr[1].u32Height);	
	hd_common_mem_flush_cache((VOID *)pstPyr.pstPrevPyr[0].u64Va, pstPyr.pstPrevPyr[0].u32Width*pstPyr.pstPrevPyr[0].u32Height);	
	
	hd_common_mem_flush_cache((VOID *)pstPyr.pstNextPyr[3].u64Va, pstPyr.pstNextPyr[3].u32Width*pstPyr.pstNextPyr[3].u32Height);	
	hd_common_mem_flush_cache((VOID *)pstPyr.pstNextPyr[2].u64Va, pstPyr.pstNextPyr[2].u32Width*pstPyr.pstNextPyr[2].u32Height);	
	hd_common_mem_flush_cache((VOID *)pstPyr.pstNextPyr[1].u64Va, pstPyr.pstNextPyr[1].u32Width*pstPyr.pstNextPyr[1].u32Height);	
	hd_common_mem_flush_cache((VOID *)pstPyr.pstNextPyr[0].u64Va, pstPyr.pstNextPyr[0].u32Width*pstPyr.pstNextPyr[0].u32Height);
	
	hd_common_mem_flush_cache((VOID *)pstPrevPt.u64Va, points_size);	
	hd_common_mem_flush_cache((VOID *)pstNextPt.u64Va, points_size);	

	hd_common_mem_flush_cache((VOID *)pstStatus.u64Va, PtsNum *sizeof(UINT8));	
	hd_common_mem_flush_cache((VOID *)pstErr.u64Va, PtsNum *sizeof(UINT16));
	vos_perf_mark(&hw_tick_begin);	
	lib_trke_ret = NVT_TRKE_PYR(&pstPyr, &pstPrevPt, &pstNextPt, &pstStatus,&pstErr, &pstTRKECtrl);
	vos_perf_mark(&hw_tick_end);
	if (lib_trke_ret != LIB_TRKE_OK) {
		printf("err:NVT_TRKE_PYR error %d\r\n",ret);
		goto exit;
	}
	printf("NVT_TRKE_PYR time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end));
	
	hd_common_mem_flush_cache((VOID *)pstNextPt.u64Va, points_size);
	hd_common_mem_flush_cache((VOID *)pstErr.u64Va, PtsNum*2);
	
	
	out_status_size =BYTE_ALIGN(PtsNum) ; //1bit stauts
	out_err_size = PtsNum * sizeof(UINT16);  //16bit err
	
	// call vendor_ive_uninit once for uninitialize IVE 
	ret = vendor_trke_uninit();
	if (ret != HD_OK) {
		printf("%s: vendor_trke_uninit error (%d)\r\n", __func__, ret);
		goto exit;
	}

	
	// set func enable
#if defined(__FREERTOS)
	snprintf(out_file1, 64, "A:\\trke\\output\\pstNextPt.bin");
	snprintf(out_file2, 64, "A:\\trke\\output\\pstErr.bin");
	snprintf(out_file3, 64, "A:\\trke\\output\\pstStatus.bin");
#else
    snprintf(out_file1, 64, "//mnt//sd//trke//output//pstNextPt.bin");
	snprintf(out_file2, 64, "//mnt//sd//trke//output//pstErr.bin");
	snprintf(out_file3, 64, "//mnt//sd//trke//output//pstStatus.bin");
#endif
	fd = fopen(out_file1, "wb");
	if (!fd) {
		printf("cannot open %s\r\n", out_file1);
		goto exit;
	}
	
	file_size = fwrite((const void *)share_mem[9].va,1,points_size,fd);
	fclose(fd);
	
	
	fd = fopen(out_file2, "wb");
	if (!fd) {
		printf("cannot open %s\r\n", out_file2);
		goto exit;
	}
	
	file_size = fwrite((const void *)share_mem[10].va,1,out_err_size,fd);
	fclose(fd);
	
	
	fd = fopen(out_file3, "wb");
	if (!fd) {
		printf("cannot open %s\r\n", out_file3);
		goto exit;
	}
	
	file_size = fwrite((const void *)share_mem[11].va,1,out_status_size,fd);
	fclose(fd);
	
	
	
   
exit:	
	share_memory_exit(share_mem);
	ret = mem_exit();
	if (ret != HD_OK) {
        printf("mem fail=%d\n", ret);
    }
mem_init_fail:
	ret = hd_common_uninit();
    if(ret != HD_OK) {
        printf("uninit fail=%d\n", ret);

    }
comm_init_fail:
	return 0;


}


