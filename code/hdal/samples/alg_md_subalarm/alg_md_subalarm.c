/**
 * @file alg_md_subalarm.c
 * @brief start md sample.
 * @author ALG1-CV
 * @date in the year 2022
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hd_debug.h"
#include "hdal.h"
#include "libmd/libmd.h"
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

#define IMG_WIDTH         160
#define IMG_HEIGHT        120
#define IMG_BUF_SIZE      (IMG_WIDTH * IMG_HEIGHT)
#define IMG_BUF_NUM       1

#define SUBREGION_NUM     4

#define OUTPUT_BMP 		0

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

	for(i=0;i<IMG_BUF_NUM;i++){
		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}
	for(i=0;i<IMG_BUF_NUM;i++){
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
		//printf("pa = 0x%x\r\n", pa);
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
	for(i=0;i<IMG_BUF_NUM;i++){
		if (p_share_mem[i].va != 0) {
			ret = hd_common_mem_munmap((void *)p_share_mem[i].va, p_share_mem[i].size);
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
	mem_cfg.pool_info[0].blk_cnt = 1;
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
#if OUTPUT_BMP
static void bc_writebmpfile(char* name, UINT8* raw_img,
    int width, int height, UINT16 bits)
{
    if(!(name && raw_img)) {
        printf("Error bmpWrite.");
        return;
    }
	int i,j;//,length;
    // FileHeader
    struct BmpFileHeader file_h = {
        .bfType=0x4d42,
        .bfSize=0,
        .bfReserved1=0,
        .bfReserved2=0,
        .bfOffBits=54,
    };
    file_h.bfSize = file_h.bfOffBits + width*height * bits/8;
    if(bits==8) {file_h.bfSize += 1024, file_h.bfOffBits += 1024;}
    // BmpInfoHeader
    struct BmpInfoHeader info_h = {
        .biSize=40,
        .biWidth=0,
        .biHeight=0,
        .biPlanes=1,
        .biBitCount=0,
        .biCompression=0,
        .biSizeImage=0,
        .biXPelsPerMeter=0,
        .biYPelsPerMeter=0,
        .biClrUsed=0,
        .biClrImportant=0,
    };
    info_h.biWidth = width;
    info_h.biHeight = height;
    info_h.biBitCount = bits;
    info_h.biSizeImage = width*height * bits/8;
    if(bits == 8) {
        info_h.biClrUsed=256;
        info_h.biClrImportant=256;
    }
    FILE *pFile = fopen(name,"wb+");
    if(!pFile) {
        printf("Error opening file.");
        return;
    }
    fwrite((char*)&file_h, sizeof(char), sizeof(file_h), pFile);
    fwrite((char*)&info_h, sizeof(char), sizeof(info_h), pFile);

	// Write colormap
    if(bits == 8) {
        for(i = 0; i < 256; ++i) {
            UINT8 c = i;
            fwrite((char*)&c, sizeof(char), sizeof(UINT8), pFile);
            fwrite((char*)&c, sizeof(char), sizeof(UINT8), pFile);
            fwrite((char*)&c, sizeof(char), sizeof(UINT8), pFile);
            fwrite("", sizeof(char), sizeof(UINT8), pFile);
        }
    }
    // Write raw img
    UINT8 alig = ((width*bits/8)*3) % 4;
    for(j = height-1; j >= 0; --j) {
        for(i = 0; i < width; ++i) {
			UINT8 c;
            if(bits == 24) {
                fwrite((char*)&raw_img[(j*width+i)*3 + 2], sizeof(char), sizeof(UINT8), pFile);
                fwrite((char*)&raw_img[(j*width+i)*3 + 1], sizeof(char), sizeof(UINT8), pFile);
                fwrite((char*)&raw_img[(j*width+i)*3 + 0], sizeof(char), sizeof(UINT8), pFile);
            } else if(bits == 8) {
				if(raw_img[j*width+i]==1){ c = 255;}
				else if(raw_img[j*width+i]==0){ c = 0;}
				else{
					printf("raw_img[j*width+i] = %d, (%d,%d)\n",raw_img[j*width+i],i,j);
					c = 0;
				}
                fwrite((char*)&c, sizeof(char), sizeof(UINT8), pFile);
            }
        }
        // 4byte align
        for(i = 0; i < alig; ++i) {
            fwrite("", sizeof(char), sizeof(UINT8), pFile);
        }
    }
    fclose(pFile);
}
#endif

static UINT32 md_load_file(CHAR *p_filename, UINTPTR va)
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

/*-------------------------------------------------------------------------------------------------*/
/* Main Function                                                                                   */
/*-------------------------------------------------------------------------------------------------*/

MAIN(argc, argv)
{
    HD_RESULT	ret;
	LIB_MD_ERROR_CODE lib_md_ret;
    char in_file1[64];
	int idx;
	UINT32 file_size = 0;
	MD_SRC_IMAGE_S src_img_info = {0};
	UINT8 u8SubAlarmRst[SUBREGION_NUM] = {0};
	MD_SUBPARAM_CTRL_S stSubParam[SUBREGION_NUM] = {0};
	MD_SUBALARM_CTRL_S stSubAlarmCtrl = {0};
    MEM_RANGE share_mem[IMG_BUF_NUM] = {0};
	VOS_TICK    hw_tick_begin, hw_tick_end;

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
	
	/// load file
#if defined(__FREERTOS)
	snprintf(in_file1, 64, "A:\\MDBCP\\DI\\motion_detect_160x120.bin");
#else
	snprintf(in_file1, 64, "//mnt//sd//MDBCP//DI//motion_detect_160x120.bin");
#endif
	file_size = md_load_file(in_file1, share_mem[0].va);
	if (file_size == 0) {
		printf("[ERR]load motion_detect_160x120.bin : %s\r\n", in_file1);
	}

	src_img_info.u32Width	= IMG_WIDTH;
	src_img_info.u32Height	= IMG_HEIGHT;
	src_img_info.u32Stride	= IMG_WIDTH;
	src_img_info.u64Pa		= share_mem[0].addr;	
	src_img_info.u64Va		= share_mem[0].va;		

	stSubParam[0].enSubRegion 		= 1;
	stSubParam[0].u32XStart 		= 1;
	stSubParam[0].u32YStart 		= 1;
	stSubParam[0].u32XEnd 			= 10;
	stSubParam[0].u32YEnd 			= 10;
	stSubParam[0].u8SubAlarmThres	= 50;
	
	stSubParam[1].enSubRegion 		= 1;
	stSubParam[1].u32XStart 		= 61;
	stSubParam[1].u32YStart 		= 61;
	stSubParam[1].u32XEnd 			= 80;
	stSubParam[1].u32YEnd 			= 80;
	stSubParam[1].u8SubAlarmThres	= 50;
	
	stSubParam[2].enSubRegion 		= 0;
	stSubParam[2].u32XStart 		= 61;
	stSubParam[2].u32YStart 		= 61;
	stSubParam[2].u32XEnd 			= 80;
	stSubParam[2].u32YEnd 			= 80;
	stSubParam[2].u8SubAlarmThres	= 50;
	
	stSubParam[3].enSubRegion 		= 1;
	stSubParam[3].u32XStart 		= 61;
	stSubParam[3].u32YStart 		= 61;
	stSubParam[3].u32XEnd 			= 80;
	stSubParam[3].u32YEnd 			= 80;
	stSubParam[3].u8SubAlarmThres	= 80;

	stSubAlarmCtrl.pstSubParam		= stSubParam;
	stSubAlarmCtrl.u8SubNum			= SUBREGION_NUM;
	vos_perf_mark(&hw_tick_begin);
	lib_md_ret = NVT_MD_SubAlarm(&src_img_info, u8SubAlarmRst, &stSubAlarmCtrl);
	vos_perf_mark(&hw_tick_end);
	printf("NVT_MD_SubAlarm time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end));
	if (lib_md_ret != LIB_MD_OK) {
		printf("err:NVT_MD_SubAlarm error %d\r\n",lib_md_ret);
		goto exit;
	}
	for(idx = 0;idx<SUBREGION_NUM;idx++){
		if(u8SubAlarmRst[idx]==1)
	    	printf("Subregion[%d] Motion Alarm!\n",idx);
	}
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

