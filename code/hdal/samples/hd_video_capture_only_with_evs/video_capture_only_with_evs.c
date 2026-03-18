/**
	@brief Sample code of videocapture only.\n

	@file video_capture_only.c

	@author Ben Wang

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_videocapture.h"

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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_capture_only, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 1

#define CHKPNT      					printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)						printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x) 						printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////

//header
#define DBGINFO_BUFSIZE()	(0x200)

///////////////////////////////////////////////////////////////////////////////

#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW8
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW8


#define VDO_SIZE_W     1280
#define VDO_SIZE_H     720

#define MAX_OUT_PIXEL_CNT (VDO_SIZE_W*VDO_SIZE_H)

#define EVS_DRAM_OUT_SIZE  (MAX_OUT_PIXEL_CNT*2)
#define CAP_OUT_Q_DEPTH  1

#define RAW_BUFFER_DDR_ID DDR_ID0

#define HD_VIDEO_MULTI_FRAME_SHIFT       8


static UINT32 sen1_vcap_id = 4;

static ULONG  evs_pa = 0, evs_va = 0;

static UINT32 shot_num = 1;
static BOOL save_raw = 0;
///////////////////////////////////////////////////////////////////////////////

static HD_RESULT mem_init(UINT32 *p_blk_size)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	// config common pool (cap)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE()+ EVS_DRAM_OUT_SIZE;
	mem_cfg.pool_info[0].blk_cnt = 2 + CAP_OUT_Q_DEPTH;

	mem_cfg.pool_info[0].ddr_id = RAW_BUFFER_DDR_ID;
	ret = hd_common_mem_init(&mem_cfg);
	*p_blk_size = mem_cfg.pool_info[0].blk_size;
	return ret;
}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	hd_common_mem_uninit();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static HD_RESULT get_cap_sysinfo(HD_PATH_ID video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_SYSINFO sys_info = {0};

	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSINFO, &sys_info);
	printf("sys_info.devid =0x%X, cur_fps[0]=%d/%d, vd_count=%llu\r\n", sys_info.dev_id, GET_HI_UINT16(sys_info.cur_fps[0]), GET_LO_UINT16(sys_info.cur_fps[0]), sys_info.vd_count);
	return ret;
}

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl, HD_OUT_ID _out_id, UINT32 *p_cap_pin_map, char *p_driver_name)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	UINT32 i;

	if (strlen(p_driver_name) <= (HD_VIDEOCAP_SEN_NAME_LEN - 1)) {
        strncpy(cap_cfg.sen_cfg.sen_dev.driver_name, p_driver_name, (HD_VIDEOCAP_SEN_NAME_LEN - 1));
    } else {
        printf("driver_name(%s) exceeds max length\r\n", p_driver_name);
        return HD_ERR_INV;
    }
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(2, 0);

	for (i=0; i < HD_VIDEOCAP_SEN_SER_MAX_DATALANE; i++) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = *(p_cap_pin_map+i);
	}
	ret = hd_videocap_open(0, _out_id, &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, HD_VIDEOCAP_SEN_FRAME_NUM frame_num)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.frc = HD_VIDEO_FRC_RATIO(250,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		video_in_param.out_frame_num = frame_num;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		//printf("set_cap_param MODE=%d\r\n", ret);
		if (ret != HD_OK) {
			return ret;
		}
	}
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.pxlfmt = CAP_OUT_FMT;

		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.depth = CAP_OUT_Q_DEPTH*frame_num;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}
	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.ddr_id = RAW_BUFFER_DDR_ID;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_cap_param PATH_CONFIG=0x%X\r\n", ret);
	}
	{ // Set EVS info
		VENDOR_VIDEOCAP_EVS_INFO evs_info = {0};

		evs_info.max_out_pixel_cnt = MAX_OUT_PIXEL_CNT;
		ret = vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_EVS_INFO, &evs_info);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_LIVEVIEW {

	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;
	CHAR cap_drv_name[HD_VIDEOCAP_SEN_NAME_LEN];
	UINT32 cap_pin_map[HD_VIDEOCAP_SEN_SER_MAX_DATALANE];
	HD_VIDEOCAP_SEN_FRAME_NUM cap_frame_num;
	HD_DIM  proc_max_dim;
	// (4) user pull
	pthread_t  cap_thread_id;
	UINT32     cap_exit;
	UINT32     cap_snap;
	UINT32     flow_start;
	UINT32     blk_size;
	INT32    wait_ms;
	UINT32 	show_ret;
} VIDEO_LIVEVIEW;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl, HD_VIDEOCAP_CTRL(sen1_vcap_id), p_stream->cap_pin_map, p_stream->cap_drv_name);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videocap_open(HD_VIDEOCAP_IN(sen1_vcap_id, 0), HD_VIDEOCAP_OUT(sen1_vcap_id, 0), &p_stream->cap_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;

	if ((ret = hd_videocap_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}

///////////////////////////////////////////////////////////////////////////////
typedef struct {
    UINT32  sz_x;            ///< horizontal window size
    UINT32  sz_y;            ///< vertical window size
    UINT32  evs_event_cnt;   ///< evs event count, unit: byte
} EVS_WIN_PARAM;

typedef struct {
    UINT8  positive_code;     ///< the output code of positive motion
    UINT8  negative_code;    ///< the output code of negative motion
} EVS_RSLT_OUT_PARAM;

static UINT32 get_evs_rslt(ULONG addr_va, UINT8 *rslt_buf, EVS_WIN_PARAM *evs_win, EVS_RSLT_OUT_PARAM *out_code)
{
    UINT32 i;
    UINT32 uiEventCnt;
	UINT32 type, x=0, y=0, pol = 0, idx = 0;
	UINT16 code;
	UINT32 temp, total_point = 0;

	uiEventCnt = evs_win->evs_event_cnt/2;

	//printf("uiEventCnt = %d\n",uiEventCnt);

	for (i=0; i<uiEventCnt; i++) {
		code = *(UINT16 *)(addr_va + (i << 1));
		temp = ((*(UINT16 *)(addr_va + (i << 1) + 2)) << 16) | code;
		if (temp == 0xFEFEFEFE) { //end-code
			printf("end-code hit(@%d, total %d point)\r\n", i << 1, total_point);
			break;
		}
		type = (code >> 12)&0xf;
		if (type == 0x0) { // y
			y = code&0x7ff;
		} else if (type == 0x2) { // x
			x = code&0x7ff;
			pol = (code >> 11)&0x1;

			if (x < evs_win->sz_x && y < evs_win->sz_y) {
				total_point++;
				idx = y*evs_win->sz_x+x;
				if (pol == 0)
					*(rslt_buf + idx) = out_code->negative_code;
				else
					*(rslt_buf + idx) = out_code->positive_code;
			} else {
				printf("invalid code, i = %d, pol = %d, X/Y = %d/%d\r\n",i, pol, x, y);
			}
		}
	}
    return i << 1;
}


static void *cap_raw_thread(void *arg)
{
	VIDEO_LIVEVIEW *p_stream0 = (VIDEO_LIVEVIEW *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame = {0};
	char file_path_main[32] = {0};
	FILE *f_out_main;
	UINTPTR phy_addr_main, vir_addr_main;
	UINT32 shot_count = 0;
	UINT32 index;
	UINT32 k;
	 #define PHY2VIRT_MAIN(pa) (vir_addr_main + ((pa) - phy_addr_main))

	//------ wait flow_start ------
	while (p_stream0->flow_start == 0) sleep(1);

	// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
	//hd_videoenc_get(video_enc_path0, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);

	// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
	//vir_addr_main = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);

	printf("\r\nif you want to capture raw, enter \"s\" to trigger !!\r\n");
	printf("\r\nif you want to stop, enter \"q\" to exit !!\r\n\r\n");

	//--------- pull data test ---------
	while (p_stream0->cap_exit == 0) {

		if(p_stream0->cap_snap) {
			p_stream0->cap_snap = 0;
			for (k = 0; k < shot_num; k++) {
				ret = hd_videocap_pull_out_buf(p_stream0->cap_path, &video_frame, p_stream0->wait_ms);// -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
				if (ret != HD_OK) {
					if(p_stream0->show_ret) printf("pull_out(%d) error = %d!!\r\n", p_stream0->wait_ms, ret);
					goto skip;
				}

				phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
				if (phy_addr_main == 0) {
					printf("hd_common_mem_blk2pa error !!\r\n\r\n");
					goto release_out;
				}
				vir_addr_main = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, p_stream0->blk_size);
				if (vir_addr_main == 0) {
					printf("memory map error !!\r\n\r\n");
					goto release_out;
				}

				hd_common_mem_flush_cache((void *)vir_addr_main, EVS_DRAM_OUT_SIZE);

				if (evs_pa && evs_va) {
					EVS_WIN_PARAM evs_win = {0};
					EVS_RSLT_OUT_PARAM out_code = {0};

					out_code.positive_code = 255;
					out_code.negative_code = 127;

					evs_win.sz_x = VDO_SIZE_W;
					evs_win.sz_y = VDO_SIZE_H;
					evs_win.evs_event_cnt = EVS_DRAM_OUT_SIZE;
					get_evs_rslt((ULONG)PHY2VIRT_MAIN(video_frame.phy_addr[0]), (UINT8 *)evs_va, &evs_win, &out_code);
				}


				if (save_raw) {
					printf("fmt           resolution     lineoffset     PA             blk_size\r\n");
					printf("0x08%X    %4dx%4d      %4d           0x%08X     %d\r\n",video_frame.pxlfmt,
																						video_frame.dim.w,
																						video_frame.dim.h,
																						video_frame.loff[0],
																						video_frame.phy_addr[0],
																						p_stream0->blk_size);


					index = video_frame.ddr_id >> HD_VIDEO_MULTI_FRAME_SHIFT;
					snprintf(file_path_main, 32, "/mnt/sd/vdocap%u_%u.raw", index, shot_count);
					printf("dump raw data file (%s) ....\r\n", file_path_main);


					//----- open output files -----
					if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
						printf("open file (%s) fail....\r\n\r\n", file_path_main);
						goto skip;
					}
					if (f_out_main) {
						UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(video_frame.phy_addr[0]);
						UINT32 len = EVS_DRAM_OUT_SIZE;//video_frame.loff[0]*video_frame.ph[0];

						fwrite(ptr, 1, len, f_out_main);
						fflush(f_out_main);
					}
					// close output file
					fclose(f_out_main);
					hd_common_mem_munmap((void *)vir_addr_main, p_stream0->blk_size);

					printf("dump raw ok\r\n\r\n");
				}
				shot_count ++;
	release_out:
				if(p_stream0->show_ret) printf("release_out() ....\r\n");
				ret = hd_videocap_release_out_buf(p_stream0->cap_path, &video_frame);
				//printf("cap_released ....\r\n");
				if (ret != HD_OK) {
					printf("cap_release error !!\r\n\r\n");
				}

			}
		}
skip:
		usleep(200000);
	}



	return 0;
}

MAIN(argc, argv)
{
    HD_RESULT ret;
    INT key;
    VIDEO_LIVEVIEW stream[2] = {0}; //0: shdr main stream, 1: shdr sub stream
    UINT32 i;

	if (argc > 1) {
		shot_num = atoi(argv[1]);
		printf("shot_num %d\r\n", shot_num);
	}

	if (argc > 2) {
		save_raw = atoi(argv[2]);
		printf("shot_num %d\r\n", save_raw);
	}

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit;
	}

	// init memory
	ret = mem_init(&stream[0].blk_size);
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		goto exit;
	}

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	stream[0].wait_ms = -1;

	// open video liview modules (main)
	for (i = 0; i < HD_VIDEOCAP_SEN_SER_MAX_DATALANE; i++) {
		stream[0].cap_pin_map[i] = HD_VIDEOCAP_SEN_IGNORE;
	}
	stream[0].cap_pin_map[0] = 0;
	stream[0].cap_pin_map[1] = 1;

	snprintf(stream[0].cap_drv_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx636");

	ret = open_module(&stream[0]);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

	// set videocap parameter (main)
	stream[0].cap_dim.w = VDO_SIZE_W; //assign by user
	stream[0].cap_dim.h = VDO_SIZE_H; //assign by user
	stream[0].cap_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;

	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, stream[0].cap_frame_num);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}


	ret = pthread_create(&stream[0].cap_thread_id, NULL, cap_raw_thread, (void *)stream);
	if (ret < 0) {
		printf("create encode thread failed");
		return -1;
	}

	// start capture modules (main)
	ret = hd_videocap_start(stream[0].cap_path);
	if (ret != HD_OK) {
		printf("start fail=%d\n", ret);
		goto exit;
	}

	// let cap_raw_thread start to work
	stream[0].flow_start = 1;

	if (hd_common_mem_alloc("evs_file", (UINTPTR *)&evs_pa, (void **)&evs_va, MAX_OUT_PIXEL_CNT, DDR_ID0) != HD_OK) {
		printf("%s mem alloc fail\r\n", __func__);
		return -1;
	}
	memset((void *)evs_va, 0, MAX_OUT_PIXEL_CNT);


	printf("Enter q to exit\n");
	while (1) {
		key = GETCHAR();
		if (key == 's') {
			stream[0].cap_snap = 1;
		}
		if (key == 'e') {

			FILE *f_evs;
			char file_path_evs[32] = {0};
			static UINT32 evs_file_cnt = 0;

			snprintf(file_path_evs, 32, "/mnt/sd/vdocap%u_%u.evs", sen1_vcap_id, evs_file_cnt);
			printf("dump evs data file (%s) ....\r\n", file_path_evs);
			if ((f_evs = fopen(file_path_evs, "wb")) == NULL) {
				printf("open file (%s) fail....\r\n\r\n", file_path_evs);
			}
			if (f_evs) {
				fwrite((UINT8 *)evs_va, 1, MAX_OUT_PIXEL_CNT, f_evs);
				fflush(f_evs);
				fclose(f_evs);
			}
			memset((void *)evs_va, 0, MAX_OUT_PIXEL_CNT);
			evs_file_cnt++;
		}


		if (key == 'q' || key == 0x3) {
			// quit program
			stream[0].cap_exit = 1;
			break;
		}

		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			// enter debug menu
			hd_debug_run_menu();
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif
		if (key == '0') {
			get_cap_sysinfo(stream[0].cap_ctrl);
		}
		if (key == '1') {
			//just a test code
			char filepath_raw[128];
			ULONG  evs_pa, evs_va;
			ULONG  raw_pa, raw_va;
			UINT32 raw_size = 0x200000;
			FILE *f_raw;


			if (hd_common_mem_alloc("test_raw", (UINTPTR *)&raw_pa, (void **)&raw_va, raw_size, DDR_ID0) != HD_OK) {
				printf("%s mem alloc fail\r\n", __func__);
			}

			snprintf(filepath_raw, sizeof(filepath_raw), "/mnt/sd/sample_evt30.raw");
			//test only
			if ((f_raw = fopen(filepath_raw, "rb")) == NULL) {
				printf("open file (%s) fail !!....\r\nPlease copy test pattern to SD Card !!\r\n\r\n", filepath_raw);
				return 0;
			}
			fread((void *)raw_va, 1, raw_size, f_raw);
			// close file
			fclose(f_raw);


			if (hd_common_mem_alloc("test_out", (UINTPTR *)&evs_pa, (void **)&evs_va, MAX_OUT_PIXEL_CNT, DDR_ID0) != HD_OK) {
				printf("%s mem alloc fail\r\n", __func__);
			}
			{
				FILE *f_evs;
				char file_path_evs[32] = {0};
				EVS_WIN_PARAM evs_win = {0};
				EVS_RSLT_OUT_PARAM out_code = {0};
				UINT32 len = EVS_DRAM_OUT_SIZE;

				memset((void *)evs_va, 0, len);

				out_code.positive_code = 255;
				out_code.negative_code = 127;

				evs_win.sz_x = VDO_SIZE_W;
				evs_win.sz_y = VDO_SIZE_H;
				evs_win.evs_event_cnt = len;
				get_evs_rslt((ULONG)raw_va, (UINT8 *)evs_va, &evs_win, &out_code);

				snprintf(file_path_evs, 32, "/mnt/sd/test_out.evs");
				printf("dump evs data file (%s) ....\r\n", file_path_evs);
				if ((f_evs = fopen(file_path_evs, "wb")) == NULL) {
					printf("open file (%s) fail....\r\n\r\n", file_path_evs);
				}
				if (f_evs) {
					fwrite((UINT8 *)evs_va, 1, MAX_OUT_PIXEL_CNT, f_evs);
					fflush(f_evs);
				}
				fclose(f_evs);
				printf("done\r\n");
				if (evs_pa && evs_va) {
					if (hd_common_mem_free((UINTPTR)evs_pa, (void *)evs_va) != HD_OK) {
						printf("%s mem free fail\r\n", __func__);
					}
				}
			}
			if (raw_pa && raw_va) {
				if (hd_common_mem_free((UINTPTR)raw_pa, (void *)raw_va) != HD_OK) {
					printf("%s mem free fail\r\n", __func__);
				}
			}
		}
	}

	// stop capture modules (main)
	hd_videocap_stop(stream[0].cap_path);

	// destroy capture thread
	pthread_join(stream[0].cap_thread_id, NULL);


exit:
	if (evs_pa && evs_va) {
		if (hd_common_mem_free((UINTPTR)evs_pa, (void *)evs_va) != HD_OK) {
			printf("%s mem free fail\r\n", __func__);
		}
	}
	// close capture modules (main)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

	// uninit memory
	ret = mem_exit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}

	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}

	return 0;
}
