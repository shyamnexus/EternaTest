/**
 * @file alg_md_mdbc.c
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
#define IMG_BUF_NUM       3

#define OUTPUT_BMP 		0
#define DEBUG_FILE		1

#pragma pack(2)
struct BmpFileHeader {
    UINT16 bfType;
    UINT32 bfSize;
    UINT16 bfReserved1;
    UINT16 bfReserved2;
    UINT32 bfOffBits;
};
struct BmpInfoHeader {
    UINT32 biSize;
    UINT32 biWidth;
    UINT32 biHeight;
    UINT16 biPlanes; // 1=defeaul, 0=custom
    UINT16 biBitCount;
    UINT32 biCompression;
    UINT32 biSizeImage;
    UINT32 biXPelsPerMeter; // 72dpi=2835, 96dpi=3780
    UINT32 biYPelsPerMeter; // 120dpi=4724, 300dpi=11811
    UINT32 biClrUsed;
    UINT32 biClrImportant;
};
#pragma pack()

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
		if(i==0)blk_size = IMG_BUF_SIZE*2;
		else if(i==1)blk_size = IMG_BUF_SIZE*48;
		else if(i==2)blk_size = IMG_BUF_SIZE;

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
		//printf("share_mem pa = 0x%x, va=0x%x, size =0x%x\r\n", p_share_mem[i].addr, p_share_mem[i].va, p_share_mem[i].size);
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
	mem_cfg.pool_info[0].blk_size = IMG_BUF_SIZE*2;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;

	mem_cfg.pool_info[1].type = HD_COMMON_MEM_USER_BLK;
	mem_cfg.pool_info[1].blk_size = IMG_BUF_SIZE*48;
	mem_cfg.pool_info[1].blk_cnt = 1;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;

	mem_cfg.pool_info[2].type = HD_COMMON_MEM_USER_BLK;
	mem_cfg.pool_info[2].blk_size = IMG_BUF_SIZE;
	mem_cfg.pool_info[2].blk_cnt = 1;
	mem_cfg.pool_info[2].ddr_id = DDR_ID0;

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
    // Write Header
	/*
	INT32 FileHandleStatus = 0;
	FST_FILE filehdl = NULL;
	filehdl = FileSys_OpenFile(name, FST_OPEN_WRITE | FST_CREATE_ALWAYS);
	if (!filehdl) {
		emu_msg(("^ROpen file fail - %s...\r\n", name));
	}
	length = sizeof(file_h);
	FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&file_h, &length, 0, NULL);
	length = sizeof(info_h);
	FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&info_h, &length, 0, NULL);
	*/
    FILE *pFile = fopen(name,"wb+");
    if(!pFile) {
        printf("Error opening file.");
        return;
    }
    fwrite((char*)&file_h, sizeof(char), sizeof(file_h), pFile);
    fwrite((char*)&info_h, sizeof(char), sizeof(info_h), pFile);

	//length = 1;
	// Write colormap
	//printf("Write colormap...\r\n");
    if(bits == 8) {
        for(i = 0; i < 256; ++i) {
            UINT8 c = i;
			//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
			//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
			//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
			//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
            fwrite((char*)&c, sizeof(char), sizeof(UINT8), pFile);
            fwrite((char*)&c, sizeof(char), sizeof(UINT8), pFile);
            fwrite((char*)&c, sizeof(char), sizeof(UINT8), pFile);
            fwrite("", sizeof(char), sizeof(UINT8), pFile);
        }
    }
    // Write raw img
	//printf("Write raw img...\r\n");
    UINT8 alig = ((width*bits/8)*3) % 4;
    for(j = height-1; j >= 0; --j) {
		//printf("j : %d\r\n",j);
        for(i = 0; i < width; ++i) {
			UINT8 c;
            if(bits == 24) {
				//c = raw_img[(j*width+i)*3 + 2];
				//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
				//c = raw_img[(j*width+i)*3 + 1];
				//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
				//c = raw_img[(j*width+i)*3 + 0];
				//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
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
				//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
                fwrite((char*)&c, sizeof(char), sizeof(UINT8), pFile);
            }
        }
        // 4byte align
        for(i = 0; i < alig; ++i) {
			//UINT8 c = 0;
			//FileHandleStatus = FileSys_WriteFile(filehdl, (UINT8 *)&c, &length, 0, NULL);
            fwrite("", sizeof(char), sizeof(UINT8), pFile);
        }
    }
	//FileHandleStatus = FileSys_CloseFile(filehdl);
	//if (FileHandleStatus != FST_STA_OK) {
	//	emu_msg(("^RClose file fail - %s...\r\n", name));
	//}
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
	int frmidx = 0, pattern_end_id = 10;
    UINT32 idx,file_size = 0;
	LIB_MD_BC_MODE_SEL is_Init=LIB_MD_BC_INIT_MODE;
	MD_SRC_IMAGE_S src_img_info = {0};
	MD_SRC_IMAGE_S dst_img_info = {0};
	MD_MDBC_CTRL_S mdbc_param = {0};
    MEM_RANGE share_mem[IMG_BUF_NUM] = {0};
	VOS_TICK    hw_tick_begin, hw_tick_end;
	char *chip_name = getenv("NVT_CHIP_ID");
#if OUTPUT_BMP
	char ImgFilePath[64];
#endif	
#if DEBUG_FILE
	char out_file1[64];
	char out_file2[64];
	char out_file3[64];
	char out_file4[64];
	FILE  *fd;
#endif

	if(argc>1) {
		idx = 1;
		sscanf(argv[idx++], "%d", &pattern_end_id);
	} else {
		pattern_end_id = 10;
	}
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

	ret = vendor_md_init();
	if (ret != HD_OK) {
		printf("err:vendor_md_init error %d\r\n",ret);
		goto exit;
	}

	for(frmidx = 0; frmidx <= pattern_end_id; frmidx++)
	{
		printf("------------ frmidx = %d ------------\r\n", frmidx);

		/// load file
#if defined(__FREERTOS)
		snprintf(in_file1, 64, "A:\\MDBCP\\DI\\pic_%04d\\yuv420_160x120.bin", frmidx);
#else
		snprintf(in_file1, 64, "//mnt//sd//MDBCP//DI//pic_%04d//yuv420_160x120.bin", frmidx);
#endif
		file_size = md_load_file(in_file1, share_mem[0].va);
		if (file_size == 0) {
			printf("[ERR]load dram_in_y.bin : %s\r\n", in_file1);
		}
		hd_common_mem_flush_cache((VOID *)share_mem[0].va, IMG_BUF_SIZE*2);

		src_img_info.u32Width 	= IMG_WIDTH;
		src_img_info.u32Height	= IMG_HEIGHT;
		src_img_info.u32Stride	= IMG_WIDTH;
		src_img_info.u64Pa		= share_mem[0].addr;	
		src_img_info.u64Va		= share_mem[0].va;		

		dst_img_info.u32Width 	= IMG_WIDTH;
		dst_img_info.u32Height	= IMG_HEIGHT;
		dst_img_info.u32Stride	= IMG_WIDTH;
		dst_img_info.u64Pa		= share_mem[2].addr;	
		dst_img_info.u64Va		= share_mem[2].va;	

		if(chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			//=> CHIP_NT98539A
			mdbc_param.enInFmt	= LIB_MD_IMG_YUV420SP_LIGHT;
			mdbc_param.enBgNum 	= LIB_MD_BG_NUM_16;
		}else{
			//=> CHIP_NT98538
			mdbc_param.enInFmt	= LIB_MD_IMG_YUV420SP;
			mdbc_param.enBgNum 	= LIB_MD_BG_NUM_8;
		}
		mdbc_param.enIsInit		= is_Init;
		mdbc_param.enSensiLevel	= LIB_MD_BC_HIGH_SENSI;
		mdbc_param.enConvgSpd	= LIB_MD_BC_HIGH_SPD;
		mdbc_param.stMorph[0].enMorph	= LIB_MD_MORPH_ENABLE;
		mdbc_param.stMorph[1].enMorph	= LIB_MD_MORPH_ENABLE;
		mdbc_param.stMorph[2].enMorph	= LIB_MD_MORPH_ENABLE;
		mdbc_param.stMorph[3].enMorph	= LIB_MD_MORPH_BYPASS;
		mdbc_param.stMorph[0].u8MorphThres	= 4; //0~8
		mdbc_param.stMorph[1].u8MorphThres	= 0; //0~8
		mdbc_param.stMorph[2].u8MorphThres	= 8; //0~8
		mdbc_param.stMorph[3].u8MorphThres	= 0; //0~8
		mdbc_param.u32TmpThres				= 0; //0~15
		mdbc_param.stTempMem.u64Va 		= share_mem[1].va;	
		mdbc_param.stTempMem.u64Pa 		= share_mem[1].addr;	
		if(chip_name != NULL && strcmp(chip_name, "CHIP_NT98539A") == 0) {
			//=> CHIP_NT98539A
			mdbc_param.stTempMem.u32Size	= IMG_BUF_SIZE*42;
		}else{
			//=> CHIP_NT98538
			mdbc_param.stTempMem.u32Size	= IMG_BUF_SIZE*48;
		}
		vos_perf_mark(&hw_tick_begin);
		lib_md_ret = NVT_MD_MDBC(&src_img_info, &dst_img_info, &mdbc_param);
		vos_perf_mark(&hw_tick_end);
		printf("NVT_MD_MDBC time = %d\r\n", vos_perf_duration(hw_tick_begin, hw_tick_end));
		if (lib_md_ret != LIB_MD_OK) {
			printf("err:NVT_MD_MDBC error %d\r\n",lib_md_ret);
			goto exit;
		}
		if(is_Init == LIB_MD_BC_NORM_MODE) {
#if OUTPUT_BMP
#if defined(__FREERTOS)
			snprintf(ImgFilePath, 64, "A:\\MDBCP\\Debug\\output_bmp\\output_%04d.bmp", frmidx);
#else
			snprintf(ImgFilePath, 64, "//mnt//sd//MDBCP//Debug//output_bmp//output_%04d.bmp", frmidx);
#endif
			bc_writebmpfile(ImgFilePath, (UINT8*)share_mem[2].va, IMG_WIDTH, IMG_HEIGHT, 8);
#endif

#if DEBUG_FILE
			hd_common_mem_flush_cache((VOID *)share_mem[1].va, IMG_BUF_SIZE*48);
#if defined(__FREERTOS)
			snprintf(out_file1, 64, "A:\\MDBCP\\DO_MDBC\\pic_%04d\\dram_out_s1.bin", frmidx);
#else
			snprintf(out_file1, 64, "//mnt//sd//MDBCP//DO_MDBC//pic_%04d//dram_out_s1.bin", frmidx);
#endif
			fd = fopen(out_file1, "wb");
			if (!fd) {
				printf("cannot read %s\r\n", out_file1);
			} else {
    			file_size = fwrite((const void *)(share_mem[1].va+IMG_BUF_SIZE*47),1,((IMG_BUF_SIZE+7)/8),fd);
    			if (file_size == 0) {
    				printf("load dram_out_s1.bin : %s\r\n", out_file1);
    			}
    			fclose(fd);
			}

#if defined(__FREERTOS)
			snprintf(out_file2, 64, "A:\\MDBCP\\DO_MDBC\\pic_%04d\\dram_out_bgYUV.bin", frmidx);
#else
			snprintf(out_file2, 64, "//mnt//sd//MDBCP//DO_MDBC//pic_%04d//dram_out_bgYUV.bin", frmidx);
#endif
			fd = fopen(out_file2, "wb");
			if (!fd) {
				printf("cannot read %s\r\n", out_file2);
			} else {
				file_size = fwrite((const void *)(share_mem[1].va+IMG_BUF_SIZE/2),1,IMG_BUF_SIZE*28,fd);
    			if (file_size == 0) {
    				printf("load dram_out_bgYUV.bin : %s\r\n", out_file2);
    			}
    			fclose(fd);
			}
#endif
		}
#if DEBUG_FILE
#if defined(__FREERTOS)
		snprintf(out_file3, 64, "A:\\MDBCP\\DO_MDBC\\pic_%04d\\dram_out_var1.bin", frmidx);
#else
		snprintf(out_file3, 64, "//mnt//sd//MDBCP//DO_MDBC//pic_%04d//dram_out_var1.bin", frmidx);
#endif
		fd = fopen(out_file3, "wb");
		if (!fd) {
			printf("cannot read %s\r\n", out_file3);
		} else {
    		file_size = fwrite((const void *)(share_mem[1].va+IMG_BUF_SIZE*57/2),1,IMG_BUF_SIZE*13/2,fd);
    		if (file_size == 0) {
    			printf("load dram_out_var1.bin : %s\r\n", out_file3);
    		}
    		fclose(fd);
		}

#if defined(__FREERTOS)
		snprintf(out_file4, 64, "A:\\MDBCP\\DO_MDBC\\pic_%04d\\dram_out_var2.bin", frmidx);
#else
		snprintf(out_file4, 64, "//mnt//sd//MDBCP//DO_MDBC//pic_%04d//dram_out_var2.bin", frmidx);
#endif
		fd = fopen(out_file4, "wb");
		if (!fd) {
			printf("cannot read %s\r\n", out_file4);
		} else {
    		file_size = fwrite((const void *)(share_mem[1].va+IMG_BUF_SIZE*35),1,((IMG_BUF_SIZE+15)/16)*16*12,fd);
    		if (file_size == 0) {
    			printf("load dram_out_var2.bin : %s\r\n", out_file4);
    		}
    		fclose(fd);
		}
#endif
		if(is_Init==LIB_MD_BC_INIT_MODE){
			is_Init=LIB_MD_BC_NORM_MODE;
		}
	}
	
	printf("output writeout size : %d \n",file_size);
    printf("[mdbc sample] write result done!\n");
	
	ret = vendor_md_uninit();
	if (HD_OK != ret) {
		printf("%s: vendor_md_uninit, uninit failed (%d)\n\r", __func__, ret);
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

