#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hdal.h"
#include "hd_debug.h"
#include <kwrap/examsys.h>
#include "math.h"
#include "vendor_md.h"
#include "libmd.h"
#include "kwrap/perf.h"                       
#include "aov1_common.h"                   
                 
UINT32 wait_md_thd_done = 0;

extern UINT32 md_threshold;
extern int suspend;
//int alg_path_start = 1;
UINT32 MD_result = 1;
char *lib_md_version;
UINT32 g_endFlag=0;
extern int suspend;

#define SHARE_MEM_NUM		9
#define SUBREGION_NUM   2
#define CROSSREGION_NUM 2
//#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
//#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
//#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
///////////////////////////////////////////////////////////////////////////////

//header
#define DBGINFO_BUFSIZE()	(0x200)

#define VDO_SIZE_W			1920
#define VDO_SIZE_H			1080
#define	VDO_FRAME_FORMAT	HD_VIDEO_PXLFMT_YUV420

#define MDBC_ALIGN(a, b) (((a) + ((b) - 1)) / (b) * (b))
#define IMG_WIDTH           320
#define IMG_HEIGHT          180
#define IMG_BUF_SIZE        (IMG_WIDTH * IMG_HEIGHT)
#define VENDOR_CV_CFG  		0x00f00000  //cv project config
#define MD_THREAD_NUM		1
#define IMG_BUF_NUM       3


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

//MD
typedef struct _MEM_RANGE {
	UINTPTR va;        ///< Memory buffer starting address
	UINTPTR addr;      ///< Memory buffer starting address
	UINT32               size;      ///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} MEM_RANGE, *PMEM_RANGE;
MEM_RANGE share_mem[IMG_BUF_NUM] = {0};
LIB_MD_BC_MODE_SEL is_Init=LIB_MD_BC_INIT_MODE;

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
		if(i==1)blk_size = IMG_BUF_SIZE*70;
		else blk_size = IMG_BUF_SIZE;

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

#define OUTPUT_BMP 0	// For Debug MD, dump BMP
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

void alg_md_init(void)
{
	HD_RESULT ret = HD_OK;
	ret = vendor_md_init();
	if (HD_OK != ret) {
		printf("init fail, error code = %d\r\n", ret);
	}
	share_memory_init(share_mem);
}

void *md_thread_api(VOID *arg)
{
	HD_VIDEO_FRAME video_frame = {0};
	HD_RESULT ret = HD_OK;
	LIB_MD_ERROR_CODE lib_md_ret;
	VIDEO_RECORD *pstream = (VIDEO_RECORD *)arg;
	UINTPTR imgY_va;
	LIB_MD_BC_MODE_SEL is_Init=LIB_MD_BC_INIT_MODE;
	MD_SRC_IMAGE_S src_img_info = {0};
	MD_SRC_IMAGE_S dst_img_info = {0};
	MD_MDBC_CTRL_S mdbc_param = {0};
	UINT8 u8GAlarmRst = 0; 
	MD_GALARM_CTRL_S stGAlarmCtrl = {0};
	UINTPTR phy_addr_main, vir_addr_main;
	UINT32 yuv_size;


	alg_md_init();
	printf("md_thread_api start\r\n");
	
	while(g_endFlag==0)
	{
		while ((pstream->flag_ai_start == 1)&&(wait_md_thd_done == 0)) {
			usleep(10);
		}
		
		ret = hd_videoproc_pull_out_buf(pstream->proc_alg_path, &video_frame, 100); 
		if(ret != HD_OK) {
			printf("%s: MD pull fail (%d)\r\n", __func__, ret);
			continue;
		}
		//printf("%s:%d\r\n", __func__, video_frame.count);
		
		{

			phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
			if (phy_addr_main == 0) {
				printf("blk2pa fail, blk = 0x%x\r\n", (int)video_frame.blk);
        		return 0;
			}
			yuv_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(video_frame.dim.w, video_frame.dim.h, HD_VIDEO_PXLFMT_YUV420);
			// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
			vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, yuv_size);
			if (vir_addr_main == 0) {
				printf("mmap error !!\r\n\r\n");
        		return 0;
			}
			#define PHY2VIRT_MAIN(pa) (vir_addr_main + ((pa) - phy_addr_main))

			// yuv_va = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, video_frame.phy_addr[0], video_frame.pw[0] * video_frame.ph[0] * 3 / 2);
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(video_frame.phy_addr[0]);
			imgY_va = (UINTPTR)ptr;
			memcpy((void *)share_mem[0].va , (void *)imgY_va , IMG_BUF_SIZE);
		
			hd_common_mem_flush_cache((VOID *)share_mem[0].va, IMG_BUF_SIZE);


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

			mdbc_param.enInFmt		= LIB_MD_IMG_Y_ONLY;
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
			mdbc_param.stTempMem.u32Size	= IMG_BUF_SIZE*60;

			lib_md_ret = NVT_MD_MDBC(&src_img_info, &dst_img_info, &mdbc_param);

			if (lib_md_ret != LIB_MD_OK) {
				printf("err:NVT_MD_MDBC error %d\r\n",lib_md_ret);
				goto gen_init_fail;
			}
			
			if(is_Init == LIB_MD_BC_NORM_MODE) 
			{
				stGAlarmCtrl.u8AlarmThres = md_threshold;     
				lib_md_ret = NVT_MD_GlobalAlarm(&dst_img_info, &u8GAlarmRst, &stGAlarmCtrl);

				if (lib_md_ret != LIB_MD_OK) {
					printf("err:NVT_MD_GlobalAlarm error %d\r\n",lib_md_ret);
					goto gen_init_fail;
				}
				
				MD_result = u8GAlarmRst;
				//if(u8GAlarmRst == 1){
				//	printf("MD\n");
				//	MD_result = 1;				
				//}
				//else
				//{
				//	MD_result = 0;
				//}
			}
			
			ret = hd_common_mem_munmap((void *)vir_addr_main, yuv_size);
			if (ret != HD_OK) {
				printf("ERR: (yuv_va)hd_common_mem_munmap fail\r\n");
				goto gen_init_fail;
			}
			//ping_pong_id = (ping_pong_id+1)%2;
			if(is_Init==LIB_MD_BC_INIT_MODE){
				is_Init=LIB_MD_BC_NORM_MODE;
			}
		}

		ret = hd_videoproc_release_out_buf(pstream->proc_alg_path, &video_frame);
		if(ret != HD_OK) {
			printf("hd_videoproc_release_out_buf fail (%d)\n\r", ret);
			goto gen_init_fail;
		}
		wait_md_thd_done = 0;
	}

gen_init_fail:	
	ret = vendor_md_uninit();
	if (HD_OK != ret) {
		printf("uninit fail, error code = %d\r\n", ret);
	}

	share_memory_exit(share_mem);

	return 0;
}   


//int alg_md_api(VIDEO_RECORD *pstream)	// TODO Ttrigger mode
//{
//	HD_VIDEO_FRAME video_frame = {0};
//	HD_RESULT ret = HD_OK;
//	LIB_MD_ERROR_CODE lib_md_ret;
//	
//	
//	//VIDEO_RECORD *pstream = (VIDEO_RECORD *)arg;
//	UINT32 imgY_va;
//
//	
//	MD_SRC_IMAGE_S src_img_info = {0};
//	MD_SRC_IMAGE_S dst_img_info = {0};
//	MD_MDBC_CTRL_S mdbc_param = {0};
//	
//	UINT8 u8GAlarmRst = 0; 
//	MD_GALARM_CTRL_S stGAlarmCtrl = {0};
//	VOS_TICK    hw_tick_begin, hw_tick_end;
//	int counter = 0;
//	UINT32 phy_addr_main, vir_addr_main;
//	UINT32 yuv_size;
//	
//	int md_ready=0;
//		
//	//while(g_endFlag==0)
//	{
//		//while ((pstream->flag_ai_start == 1)&&(wait_md_thd_done == 0)) {
//		//	usleep(10);
//		//}
//		
//		ret = hd_videoproc_pull_out_buf(pstream->proc_alg_path, &video_frame, -1); 
//		if(ret != HD_OK) {
//			printf("hd_videoproc_pull_out_buf fail (%d)\n\r", ret);
//			//continue;
//		}
//		wait_md_thd_done = 0;
//		//if(counter >30) 
//		{
//
//			phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
//			if (phy_addr_main == 0) {
//				printf("blk2pa fail, blk = 0x%x\r\n", (int)video_frame.blk);
//        		return 0;
//			}
//			yuv_size = DBGINFO_BUFSIZE() + VDO_YUV_BUFSIZE(video_frame.dim.w, video_frame.dim.h, HD_VIDEO_PXLFMT_YUV420);
//			// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
//			vir_addr_main = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, yuv_size);
//			if (vir_addr_main == 0) {
//				printf("mmap error !!\r\n\r\n");
//        		return 0;
//			}
//			#define PHY2VIRT_MAIN(pa) (vir_addr_main + ((pa) - phy_addr_main))
//
//			// yuv_va = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, video_frame.phy_addr[0], video_frame.pw[0] * video_frame.ph[0] * 3 / 2);
//			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(video_frame.phy_addr[0]);
//			imgY_va = (UINT32)ptr;
//			memcpy((UINT32 *)share_mem[0].va , (UINT32 *)imgY_va , IMG_BUF_SIZE);
//		
//			hd_common_mem_flush_cache((VOID *)share_mem[0].va, IMG_BUF_SIZE);
//
//
//			src_img_info.u32Width 	= IMG_WIDTH;
//			src_img_info.u32Height	= IMG_HEIGHT;
//			src_img_info.u32Stride	= IMG_WIDTH;
//			src_img_info.u64Pa		= share_mem[0].addr;	
//			src_img_info.u64Va		= share_mem[0].va;		
//
//			dst_img_info.u32Width 	= IMG_WIDTH;
//			dst_img_info.u32Height	= IMG_HEIGHT;
//			dst_img_info.u32Stride	= IMG_WIDTH;
//			dst_img_info.u64Pa		= share_mem[2].addr;	
//			dst_img_info.u64Va		= share_mem[2].va;	
//
//			mdbc_param.enInFmt		= LIB_MD_IMG_Y_ONLY;
//			mdbc_param.enIsInit		= is_Init;
//			mdbc_param.enSensiLevel	= LIB_MD_BC_HIGH_SENSI;
//			mdbc_param.enConvgSpd	= LIB_MD_BC_HIGH_SPD;
//			mdbc_param.stMorph[0].enMorph	= LIB_MD_MORPH_ENABLE;
//			mdbc_param.stMorph[1].enMorph	= LIB_MD_MORPH_ENABLE;
//			mdbc_param.stMorph[2].enMorph	= LIB_MD_MORPH_ENABLE;
//			mdbc_param.stMorph[3].enMorph	= LIB_MD_MORPH_BYPASS;
//			mdbc_param.stMorph[0].u8MorphThres	= 4; //0~8
//			mdbc_param.stMorph[1].u8MorphThres	= 0; //0~8
//			mdbc_param.stMorph[2].u8MorphThres	= 8; //0~8
//			mdbc_param.stMorph[3].u8MorphThres	= 0; //0~8
//			mdbc_param.u32TmpThres				= 0; //0~15
//			mdbc_param.stTempMem.u64Va 		= share_mem[1].va;	
//			mdbc_param.stTempMem.u64Pa 		= share_mem[1].addr;	
//			mdbc_param.stTempMem.u32Size	= IMG_BUF_SIZE*60;
//
//			vos_perf_mark(&hw_tick_begin);
//
//
//			lib_md_ret = NVT_MD_MDBC(&src_img_info, &dst_img_info, &mdbc_param);
//
//			vos_perf_mark(&hw_tick_end);
//
//			if (lib_md_ret != LIB_MD_OK) {
//				printf("err:NVT_MD_MDBC error %d\r\n",lib_md_ret);
//				//goto gen_init_fail;
//			}
//			
//			if(is_Init == LIB_MD_BC_NORM_MODE) 
//			{
//				stGAlarmCtrl.u8AlarmThres = md_threshold;     
//				vos_perf_mark(&hw_tick_begin);
//				lib_md_ret = NVT_MD_GlobalAlarm(&dst_img_info, &u8GAlarmRst, &stGAlarmCtrl);
//				vos_perf_mark(&hw_tick_end);
//
//				if (lib_md_ret != LIB_MD_OK) {
//					printf("err:NVT_MD_GlobalAlarm error %d\r\n",lib_md_ret);
//					//goto gen_init_fail;
//				}
//				if(u8GAlarmRst==1)
//   				{
//   					 printf("MD!");
//   				}
//					
//				if(u8GAlarmRst == 1){
//					MD_result = 1;
//					MD_done	= 1;					
//				}
//				else
//				{
//					MD_result = 0;
//					MD_done	= 1;
//				}
//				if((counter > 30)&&(md_ready==0)){
//					md_ready = 1;
//				}
//			}
//			ret = hd_common_mem_munmap((void *)vir_addr_main, yuv_size);
//			if (ret != HD_OK) {
//				printf("ERR: (yuv_va)hd_common_mem_munmap fail\r\n");
//				//goto gen_init_fail;
//			}
//			//ping_pong_id = (ping_pong_id+1)%2;
//			if(is_Init==LIB_MD_BC_INIT_MODE){
//					is_Init=LIB_MD_BC_NORM_MODE;
//			}
//		}
//		//else if(counter <=30){
//			counter++;
//		//}
//		ret = hd_videoproc_release_out_buf(pstream->proc_alg_path, &video_frame);
//		if(ret != HD_OK) {
//			printf("hd_videoproc_release_out_buf fail (%d)\n\r", ret);
//			//goto gen_init_fail;
//		}
//	}
//
//	return 0;
//}   
