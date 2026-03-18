#include <string.h>
#include <pthread.h>

#include <kwrap/file.h>

#include "hdal.h"
#include "vendor_isp.h"
#include "vendor_common.h"
#include "dpc_lib.h"

#define USER_FPN_RAW_2048       "/mnt/sd/imx485_fpn_iso_204800_12bit_2byte_crop_4M.raw"

#define HD_VIDEOCAP_PATH(dev_id, in_id, out_id) (((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))

typedef struct _DDR_INFO {
	void *va;
	UINTPTR pa;
	UINT32 size;
} DDR_INFO;

//============================================================================
// global
//============================================================================
static INT32 get_choose_int(void)
{
	CHAR buf[256];
	INT val, error;

	error = scanf("%d", &val);

	if (error != 1) {
		printf("Invalid option. Try again.\n");
		clearerr(stdin);
		fgets(buf, sizeof(buf), stdin);
		val = -1;
	}

	return val;
}

static BOOL dbg_print = FALSE;
static BOOL dbg_save_raw = FALSE;
static BOOL is_dpc_finish = FALSE;
static INT32 fpn_get_raw(UINT32 sensor_id, UINT32* raw_va, UINT32* raw_pa)
{
	HD_PATH_ID vcap_id;
	HD_VIDEO_FRAME video_frame = {0};

	HD_RESULT ret = 0;
	char dbg_raw_file_path[256];
	//VOS_FILE fp;
	FILE *fp;
	static int g_file_idx = 0;

	g_file_idx++;

	vcap_id = HD_VIDEOCAP_PATH(HD_DAL_VIDEOCAP(sensor_id), HD_IN(0), HD_OUT(0));

	ret = hd_videocap_pull_out_buf(vcap_id, &video_frame, -1);// -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
	if (ret != HD_OK) {
		printf("pull_out (%d) error = %d \n", -1, ret);
	}

	ret = (ULONG)hd_gfx_memcpy((UINTPTR)raw_pa, video_frame.phy_addr[0], video_frame.loff[0] * video_frame.ph[0]);
	if (ret == 0) {
		printf("hd_gfx_memcpy fail (%d) (0x%llx, 0x%llx, %d)\n", ret , raw_pa, video_frame.phy_addr[0], video_frame.loff[0] * video_frame.ph[0]);
	}

	ret = hd_videocap_release_out_buf(vcap_id, &video_frame);
	if (ret != HD_OK) {
		printf("cap_release fial \n\n");
	}

	if(dbg_save_raw){
		snprintf(dbg_raw_file_path, 256, "/mnt/sd/raw_w%d_h%d_12bit_pack_%d.raw", video_frame.pw[0], video_frame.ph[0], g_file_idx);
		printf("save file: %s \n", dbg_raw_file_path);
		fp = fopen(dbg_raw_file_path, "wb");
		if(fp == NULL){
			printf("open %s fail \r\n",dbg_raw_file_path);
		}else{
			fwrite(raw_va, 1, video_frame.loff[0] * video_frame.ph[0], fp);
			fflush(fp);
		}
		fclose(fp);
	}

	return ret;
}


static unsigned int fpn_dpc_get_sensor_info(int sensor_id,  ISPT_SENSOR_MODE_INFO *p_sensor_mode_info)
{
	HD_RESULT ret = 0;

	p_sensor_mode_info->id = sensor_id;
	ret = vendor_isp_get_common(ISPT_ITEM_SENSOR_MODE_INFO, p_sensor_mode_info);
	
	if (ret < 0) {				
			printf("GET_SENSOR_MODE_INFO fail! \n");
			return -1;
		}
	return ret;
}

static void set_ae(UINT32 id,	BOOL b_manual, AET_MANUAL *info)
{
	if (b_manual){
		info->id = id;
		info->manual.mode = MANUAL_MODE;
		vendor_isp_set_ae(AET_ITEM_MANUAL, info);
		printf("set manual expT %d, iso %d \r\n",info->manual.expotime, info->manual.iso_gain);
	}else{
		info->id = id;
		info->manual.mode = AUTO_MODE;
		vendor_isp_set_ae(AET_ITEM_MANUAL, info);

	}
	
}

static AET_MANUAL fpn_manual = {
	 .id = 0,
	 .manual = {MANUAL_MODE, 40000, 204800, 0, 0}
	
};
static AET_MANUAL dpc_manual = {
	 .id = 0,
	 .manual = {MANUAL_MODE, 40000, 12800, 0, 0}

};

static BOOL hd_mem_init = FALSE;
static BOOL ap_init = FALSE;

static IQT_DPC_PARAM dpc = {0};

int main(int argc, char *argv[])
{
	INT32 option;
	UINT32 trig = 1;
	UINT32 id;
	UINT32 frame_num = 100;
	UINT32 dpc_frame_num = 1;

	ISPT_SENSOR_INFO sensor_info = {0};
	ISPT_RAW_INFO raw_info = {0};
	ISPT_WAIT_VD wait_vd;
	AET_STATUS_INFO ae_status = {0};
	IQT_FPN_PARAM fpn = {0};

	VENDOR_COMM_MAX_FREE_BLOCK max_free_block = {0};
	UINT32 raw_buffer_size = 0, fpn_32_buffer_size = 0;
	DDR_INFO fpn_buffer_32 = {0};
	DDR_INFO raw_buffer = {0};
	DDR_INFO fpn_buffer = {0};
	DDR_INFO dpc_buffer = {0};	
	UINT32 dpc_buffer_size = 0;
	UINT32 ob = 16;
	UINT32 dpc_th = 16;
	FILE *fp;
	char fpn_file_path[256];
	UINT32 i;
	HD_RESULT ret;

	static int file_idx = 0;
	static UINT32 vcap_id = 0;
	static UINT32 dpc_max_percentage = 10;


	while (trig) {
		printf("----------------------------------------\r\n");
		printf("   1. Init \n");
		printf("   2. calibrate  DP \n");
		printf("   3. set DP param \n");
		printf("   4. get RAW and calibration FPN\n");
		printf("   5. set FPN \n");
		printf("   9. Uninit \n");
		printf("----------------------------------------\r\n");
		printf("---------------set parameter_-----------\r\n");
		printf("---------------DPC-----------\r\n");
		printf("  10. set id, now id(%d)\r\n",vcap_id);
		printf("  11. set ob for DP, now ob(%d)@8bit\r\n",ob);
		printf("  12. set threshold for DP, now th(%d)\r\n",dpc_th);
		printf("  13. set expT iso for DP, expT(%d), ISO(%d)\r\n",dpc_manual.manual.expotime,dpc_manual.manual.iso_gain);
		printf("  14. set max dp number,  max is (%d) of total_pixel\r\n",dpc_max_percentage);
		printf("  15. set frame number for DPC, frame number (%d)\r\n",dpc_frame_num);
		printf("---------------FPN-----------\r\n");
		printf("  31. set frame number for FPN, frame number (%d)\r\n",frame_num);
		printf("  32. set expT iso for FPN, expT(%d), ISO(%d)\r\n",fpn_manual.manual.expotime,fpn_manual.manual.iso_gain);

		
		printf("----------------------------------------\r\n");

		
		printf("   0. Quit \n");
		printf("----------------------------------------\n");

		printf(">> ");
		option = get_choose_int();

		switch (option) {
				case 1:
					if(ap_init){
							printf("already init!!\r\n");
						break;
					}
					if (hd_mem_init == FALSE) {
						hd_common_init(2);
						hd_common_mem_init(NULL);
						hd_mem_init = TRUE;
					}

					// open MCU device
					if (vendor_isp_init() == HD_ERR_NG) {
						return -1;
					}
					if (hd_gfx_init() != HD_OK) {
						return -1;
					}
					ap_init = TRUE;				
				break;
				case 2:
					//DPC
					{
						CAL_ALG_DP_RST dp_rst;
						CAL_ALG_DP_PARAM dp_param;
						UINT32* dp_buf = NULL;
						ISPT_SENSOR_MODE_INFO sensor_mode_info;
					
						unsigned int raw_size, avg, j;
						UINT32 dpc_max_cnt = 4096;
						//UINT32 raw_8_buffer_size = 0;
			
						DDR_INFO raw8_buffer = {0}; 			
						//prepare buffer
						//cali_dpc_set_dbg_out(1);
				
						if(!ap_init){
							printf("Please init first!!\r\n");
							break;
						}
						

						id = vcap_id;
				///// first disable dpc
						dpc.id = vcap_id;
						vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc);
						dpc.dpc.enable = 0;
						vendor_isp_set_iq(IQT_ITEM_DPC_PARAM, &dpc);
				///// set ae
						set_ae(vcap_id, TRUE, &dpc_manual);
			
				///// wait vd	
						wait_vd.id = id;
						vendor_isp_get_common(ISPT_ITEM_WAIT_VD, &wait_vd);					
						vendor_isp_get_common(ISPT_ITEM_WAIT_VD, &wait_vd);
					raw_info.id = id;
					ret = vendor_isp_get_common(ISPT_ITEM_RAW, &raw_info);
					if (ret < 0) {
						printf("GET ISPT_ITEM_RAW fail! \n");
						//set raw
						ret = vendor_isp_set_common(ISPT_ITEM_RAW, &raw_info.id);
						break;
					}
					raw_buffer_size = raw_info.raw_info.pw * raw_info.raw_info.ph * 12 / 8;
					fpn_32_buffer_size = raw_info.raw_info.pw * raw_info.raw_info.ph * 32 / 8;
					//raw_8_buffer_size = raw_info.raw_info.pw * raw_info.raw_info.ph;
					dpc_max_cnt = raw_info.raw_info.pw * raw_info.raw_info.ph*dpc_max_percentage/100;
					dpc_buffer_size =  dpc_max_cnt*sizeof(UINT32);
					
					printf("sensor %d size = %d x %d \n", id, raw_info.raw_info.pw, raw_info.raw_info.ph);
					ret = vendor_isp_set_common(ISPT_ITEM_RAW, &raw_info.id);
					
					// allocate FPN and RAW buffer
					max_free_block.ddr = 0;
					vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
					printf("DDR0 max_free_block size = %d \n", max_free_block.size);
					
					if (max_free_block.size < (raw_buffer_size + fpn_32_buffer_size + dpc_buffer_size)) {
						printf("DDR0 max_free_block size is too small \n");
					
						max_free_block.ddr = 1;
						vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
						printf("DDR1 max_free_block size = %d \n", max_free_block.size);
						if (max_free_block.size < (raw_buffer_size + fpn_32_buffer_size + dpc_buffer_size)) {
							printf("DDR1 max_free_block size is too small \n");
							break;
						};
					};
					ret = hd_common_mem_alloc("DPC", &dpc_buffer.pa, (void **)&dpc_buffer.va, dpc_buffer_size, 0);
					if (ret != HD_OK) {
						printf("memory allocate DPC size %d NG \n", dpc_buffer_size);
						break;
					};				
					ret = hd_common_mem_alloc("FPN_32", &fpn_buffer_32.pa, (void **)&fpn_buffer_32.va, fpn_32_buffer_size, 0);
					if (ret != HD_OK) {
						printf("memory allocate FPN size %d NG \n", fpn_32_buffer_size);
						break;
					};
					ret = hd_common_mem_alloc("RAW", &raw_buffer.pa, (void **)&raw_buffer.va, raw_buffer_size, 0);
					if (ret != HD_OK) {
						printf("memory allocate RAW size %d NG \n", raw_buffer_size);
						break;
					};
					
			
			
					
					memset(fpn_buffer_32.va, 0, fpn_32_buffer_size);
					memset(raw_buffer.va, 0, raw_buffer_size);
					memset(dpc_buffer.va, 0, dpc_buffer_size);			
					printf("memory allocate FPN_32 size %d OK, va(0x%lx) \n", fpn_32_buffer_size, fpn_buffer_32.va);
					printf("memory allocate RAW size %d OK, va(0x%lx) \n", raw_buffer_size, raw_buffer.va);
					printf("memory allocate DPC size %d OK, va(0x%lx) \n", dpc_buffer_size, dpc_buffer.va); 
					

					
			
					{
							UINT32 offset;
							UINT32 fpn_32_offset = 0;
							
							UINT32 raw1_h, raw1_l, raw1;
							UINT32 raw2_h, raw2_l, raw2;
							UINT32 buffer1, buffer2;
							UINT32 *fpn_32_img_buff = (UINT32 *)fpn_buffer_32.va;
					
							for (i = 0; i < dpc_frame_num; i++) {
								wait_vd.id = id;
								vendor_isp_get_common(ISPT_ITEM_WAIT_VD, &wait_vd);
					
								fpn_get_raw(id, (UINT32*)raw_buffer.va, (UINT32*)raw_buffer.pa);
					
								fpn_32_offset = 0;
								for (offset = 0; offset < raw_buffer_size; offset += 3) {
									raw1_h = (UINT32)(*((UINT8 *)raw_buffer.va + offset + 1) & 0x0F);
									raw1_l = (UINT32)(*((UINT8 *)raw_buffer.va + offset + 0) & 0xFF);
									raw1 = (raw1_h << 8) + raw1_l;
					
									raw2_h = (UINT32)(*((UINT8 *)raw_buffer.va + offset + 2) & 0xFF);
									raw2_l = (UINT32)((*((UINT8 *)raw_buffer.va + offset + 1) & 0xF0) >> 4);
									raw2 = (raw2_h << 4) + raw2_l;
					
									if ((dbg_print) && (offset == 0)) {
										printf("raw_buffer[0] 0x%x, 0x%x, 0x%x, \r\n", *((UINT8 *)raw_buffer.va + 0), *((UINT8 *)raw_buffer.va + 1), *((UINT8 *)raw_buffer.va + 2));
										printf("raw data = 0x%x, 0x%x \r\n", raw1, raw2);
									}
					
									if (fpn_32_offset > (fpn_32_buffer_size - 2)) {
										printf("fpn_32_offset(%d) out of range !!!! \r\n", fpn_32_offset);
										break;
									}
									*(fpn_32_img_buff + fpn_32_offset + 0) = (UINT32)(*(fpn_32_img_buff + fpn_32_offset + 0)) + (UINT32)raw1;
									*(fpn_32_img_buff + fpn_32_offset + 1) = (UINT32)(*(fpn_32_img_buff + fpn_32_offset + 1)) + (UINT32)raw2;
									if ((dbg_print) && (offset == 0)) {
										printf("fpn_buffer_32[0] data = 0x%x, 0x%x \r\n", (UINT32)(*(fpn_32_img_buff + fpn_32_offset + 0)), (UINT32)(*(fpn_32_img_buff + fpn_32_offset + 1)));
									}
					
									fpn_32_offset = fpn_32_offset + 2;
								}
								printf(" frame %d done \r\n", i + 1);
								fflush(stdout); 
								}
					
							offset = 0;
							
							memset(raw_buffer.va, 0, raw_buffer_size);
							raw8_buffer = raw_buffer;
							
							fflush(stdout);
							for (fpn_32_offset = 0; fpn_32_offset < (raw_info.raw_info.pw * raw_info.raw_info.ph); fpn_32_offset += 2) {
								buffer1 = (UINT32)(*((UINT32 *)fpn_buffer_32.va + fpn_32_offset + 0))/dpc_frame_num;
								buffer2 = (UINT32)(*((UINT32 *)fpn_buffer_32.va + fpn_32_offset + 1))/dpc_frame_num;
								if (buffer1 > 0xFFF) {
									buffer1 = 0xFFF;
								}
								if(buffer1>ob*16){
									buffer1 = buffer1-(ob*16) ;
								}else{
									buffer1 = 0;
									}
								if (buffer2 > 0xFFF) {
									buffer2 = 0xFFF;
								}
								if(buffer2>ob*16){
									buffer2 = buffer2-(ob*16) ;
								}else{
									buffer2 = 0;
									}
					
								if (offset > (raw_buffer_size - 2)) {
									printf("offset(%d) out of range !!!! \r\n", offset);
									break;
								}
								*((UINT8 *)raw8_buffer.va + offset + 0) = (UINT8)((buffer1>>4)&0xFF);
								*((UINT8 *)raw8_buffer.va + offset + 1) = (UINT8)((buffer2>>4)&0xFF);
								
					
								if ((dbg_print) && (offset == 0)) {
									printf("fpn data = 0x%x, 0x%x \r\n", buffer1, buffer2);
									printf("fpn_buffer[0] 0x%x, 0x%x, 0x%x, \r\n", *((UINT8 *)fpn_buffer.va + 0), *((UINT8 *)fpn_buffer.va + 1), *((UINT8 *)raw_buffer.va + 2));
								}
					
								offset = offset + 2;
							}
					}
							fflush(stdout);
					//	dpc_buffer = fpn_buffer_32;
					//	memset(dpc_buffer.va, 0, dpc_buffer_size);
			
						fpn_dpc_get_sensor_info(id, &sensor_mode_info);
			
			
						
						dp_param.raw_info.pw = raw_info.raw_info.pw;
						dp_param.raw_info.ph = raw_info.raw_info.ph;
						dp_param.raw_info.loff = raw_info.raw_info.pw;
						dp_param.raw_info.addr = (ULONG)raw8_buffer.va;
						dp_param.sie_act_str.x = sensor_mode_info.info.act_size[0].x;
						dp_param.sie_act_str.y = sensor_mode_info.info.act_size[0].y;
						
						dp_param.sie_crp_str.x = (sensor_mode_info.info.act_size[0].w - sensor_mode_info.info.crp_size.w) / 2;
						dp_param.sie_crp_str.y = (sensor_mode_info.info.act_size[0].h - sensor_mode_info.info.crp_size.h) / 2;					
						raw_size = sensor_mode_info.info.crp_size.w* sensor_mode_info.info.crp_size.h;				
			
						{
									char* p_tmp = (char*)dp_param.raw_info.addr;
									avg = 0;
									for (j = 0; j < raw_size; j++) {
										avg += p_tmp[j];
									}
									avg /= raw_size;
									printf("buffer %lx, average lum = %d\r\n", (ULONG)p_tmp, avg);
						}
						
							
						dp_buf = (UINT32 *)dpc_buffer.va; 
						
						
						memset(dpc_buffer.va, 0, dpc_buffer_size);			
						dp_param.dp_pool_addr = (ULONG)&dp_buf[0];
						dp_param.setting.threshold = dpc_th;
						dp_param.ori_dp_cnt = 0;
						dp_param.max_dp_cnt = dpc_max_cnt;
						dp_param.b_chg_dp_format = DP_PARAM_CHGFMT_AUTO;
						if (dbg_print) {
							printf("addr %lx, addr %lx\r\n", dp_param.raw_info.addr, dp_param.dp_pool_addr);
						}
						cal_dp_process(&dp_param, &dp_rst);
						
						printf("cnt=%d,data_length=%d \r\n", dp_rst.pixel_cnt, dp_rst.data_length);
						if(dp_rst.pixel_cnt <= dpc_max_cnt){
							is_dpc_finish = TRUE;
						}else{
							printf("--------------------------\r\n");
							printf("     dp overflow!!\r\n");
							printf("--------------------------\r\n");
						}
						
						ret = vendor_isp_get_common(ISPT_ITEM_SENSOR_INFO, &sensor_info);
			
						ae_status.id = id;
						vendor_isp_get_ae(AET_ITEM_STATUS, &ae_status); 				
						snprintf(fpn_file_path, 256, "/mnt/sd/%s_%d_DP_exp%d_iso%d.bin", sensor_info.sensor_info.name[id], file_idx,  ae_status.status_info.expotime[0], ae_status.status_info.iso_gain[0]);
			
						dpc_buffer_size = dp_rst.data_length;
						
						// save dp bin
						FILE *fp;
						fp = fopen(fpn_file_path, "wb");
						if(fp != NULL){
							fwrite(dp_buf, 1, dp_rst.data_length, fp);			
							fflush(fp);
							printf("dp file name: %s \r\n",fpn_file_path);
							if (dbg_print) {							
								printf("dp_buf:%x length:%x\r\n",dp_buf, dp_rst.data_length);
							}
						
						} else {
							printf("fp is NULL\r\n");
						}
						fclose(fp);
			
			
				
				if (raw_buffer.va != NULL) {
					ret = hd_common_mem_free(raw_buffer.pa, raw_buffer.va);
					if (ret != HD_OK) {
						printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(raw_buffer.pa), (UINTPTR)(raw_buffer.va));
					}else{
						raw_buffer.va = NULL;
					}
				}
				
				if (fpn_buffer_32.va != NULL) {
					ret = hd_common_mem_free(fpn_buffer_32.pa, fpn_buffer_32.va);
					if (ret != HD_OK) {
						printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(raw8_buffer.pa), (UINTPTR)(raw8_buffer.va));
					}else{
						fpn_buffer_32.va = NULL;
					
					}
				}
				
			
			}	
				
				set_ae(vcap_id, FALSE, &dpc_manual);
			
				break;		
			
			case 3:{

				if(!is_dpc_finish){
					printf("please do DPC first!! \r\n");
					break;
				}
			
				
				IQT_EXPAND_DPC_PARAM expand_dpc = {0};

				expand_dpc.id = vcap_id;
			
				printf("Set enable :(0: disable, 1: enable) \r\n ");
				printf(">> ");

				dpc.id = vcap_id;
				vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc);
				dpc.dpc.enable = (UINT32)get_choose_int();
				vendor_isp_set_iq(IQT_ITEM_DPC_PARAM, &dpc);

				if(dpc.dpc.enable == 1){
						expand_dpc.expand_dpc.enable = 1;
						expand_dpc.expand_dpc.size = dpc_buffer_size;
						expand_dpc.expand_dpc.table_phyaddr = dpc_buffer.pa;
					
						vendor_isp_set_iq(IQT_ITEM_EXPAND_DPC_PARAM, &expand_dpc);
						printf("DPC id = %d \n", expand_dpc.id);
						printf("DPC enable = %d \n", expand_dpc.expand_dpc.enable);
						printf("DPC buf_size = %d \n", expand_dpc.expand_dpc.size);
						printf("DPC buf_phyaddr = 0x%x \n", expand_dpc.expand_dpc.table_phyaddr);
						
						printf("DPC buf = {0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ...} \n"
							, *((UINT32 *)dpc_buffer.va + 0), *((UINT32 *)dpc_buffer.va + 1), *((UINT32 *)dpc_buffer.va + 2), *((UINT32 *)dpc_buffer.va + 3)
							, *((UINT32 *)dpc_buffer.va + 4), *((UINT32 *)dpc_buffer.va + 5), *((UINT32 *)dpc_buffer.va + 6), *((UINT32 *)dpc_buffer.va + 7));
				}else{
					printf("\r\n DPC disable \r\n");
	
				}
				
				
				break;
				}
			

		case 4:


			if(!ap_init){
				printf("Please init first!!\r\n");
				break;
			}

			
			//check dpc enable
			dpc.id = vcap_id;
			vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc);

			if(dpc.dpc.enable == 0){
				printf("please enable dpc function, or do dp calibration \r\n");
				break;
			}

			
			//set ae
			set_ae(vcap_id, TRUE, &fpn_manual);
			wait_vd.id = vcap_id;
			vendor_isp_get_common(ISPT_ITEM_WAIT_VD, &wait_vd); 
			vendor_isp_get_common(ISPT_ITEM_WAIT_VD, &wait_vd); 

			
			id = vcap_id;

			raw_info.id = id;
			ret = vendor_isp_get_common(ISPT_ITEM_RAW, &raw_info);
			if (ret < 0) {
				printf("GET ISPT_ITEM_RAW fail! \n");
				//set raw
				ret = vendor_isp_set_common(ISPT_ITEM_RAW, &raw_info.id);
				break;
			}
			raw_buffer_size = raw_info.raw_info.pw * raw_info.raw_info.ph * 12 / 8;
			fpn_32_buffer_size = raw_info.raw_info.pw * raw_info.raw_info.ph * 32 / 8;
			printf("sensor %d size = %d x %d \n", id, raw_info.raw_info.pw, raw_info.raw_info.ph);
			ret = vendor_isp_set_common(ISPT_ITEM_RAW, &raw_info.id);

			// allocate FPN and RAW buffer
			max_free_block.ddr = 0;
			vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
			printf("DDR0 max_free_block size = %d \n", max_free_block.size);

			if (max_free_block.size < (raw_buffer_size + fpn_32_buffer_size)) {
				printf("DDR0 max_free_block size is too small \n");

				max_free_block.ddr = 1;
				vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
				printf("DDR1 max_free_block size = %d \n", max_free_block.size);
				if (max_free_block.size < (raw_buffer_size + fpn_32_buffer_size)) {
					printf("DDR1 max_free_block size is too small \n");
					break;
				};
			};

			ret = hd_common_mem_alloc("FPN_32", &fpn_buffer_32.pa, (void **)&fpn_buffer_32.va, fpn_32_buffer_size, 0);
			if (ret != HD_OK) {
				printf("memory allocate FPN size %d NG \n", fpn_32_buffer_size);
				break;
			};
			ret = hd_common_mem_alloc("RAW", &raw_buffer.pa, (void **)&raw_buffer.va, raw_buffer_size, 0);
			if (ret != HD_OK) {
				printf("memory allocate RAW size %d NG \n", raw_buffer_size);
				break;
			};

			memset(fpn_buffer_32.va, 0, fpn_32_buffer_size);
			memset(raw_buffer.va, 0, raw_buffer_size);
			printf("memory allocate FPN_32 size %d OK, va(0x%lx) \n", fpn_32_buffer_size, fpn_buffer_32.va);
			printf("memory allocate RAW size %d OK, va(0x%lx) \n", raw_buffer_size, raw_buffer.va);



			{
			UINT32 offset;
			UINT32 fpn_32_offset = 0;
			
			UINT32 raw1_h, raw1_l, raw1;
			UINT32 raw2_h, raw2_l, raw2;
			UINT32 fpn1, fpn2;
			UINT32 *fpn_32_img_buff = (UINT32 *)fpn_buffer_32.va;

			for (i = 0; i < frame_num; i++) {
				wait_vd.id = id;
				vendor_isp_get_common(ISPT_ITEM_WAIT_VD, &wait_vd);

				fpn_get_raw(id, (UINT32*)raw_buffer.va, (UINT32*)raw_buffer.pa);

				fpn_32_offset = 0;
				for (offset = 0; offset < raw_buffer_size; offset += 3) {
					raw1_h = (UINT32)(*((UINT8 *)raw_buffer.va + offset + 1) & 0x0F);
					raw1_l = (UINT32)(*((UINT8 *)raw_buffer.va + offset + 0) & 0xFF);
					raw1 = (raw1_h << 8) + raw1_l;

					raw2_h = (UINT32)(*((UINT8 *)raw_buffer.va + offset + 2) & 0xFF);
					raw2_l = (UINT32)((*((UINT8 *)raw_buffer.va + offset + 1) & 0xF0) >> 4);
					raw2 = (raw2_h << 4) + raw2_l;

					if ((dbg_print) && (offset == 0)) {
						printf("raw_buffer[0] 0x%x, 0x%x, 0x%x, \r\n", *((UINT8 *)raw_buffer.va + 0), *((UINT8 *)raw_buffer.va + 1), *((UINT8 *)raw_buffer.va + 2));
						printf("raw data = 0x%x, 0x%x \r\n", raw1, raw2);
					}

					if (fpn_32_offset > (fpn_32_buffer_size - 2)) {
						printf("fpn_32_offset(%d) out of range !!!! \r\n", fpn_32_offset);
						break;
					}
					*(fpn_32_img_buff + fpn_32_offset + 0) = (UINT32)(*(fpn_32_img_buff + fpn_32_offset + 0)) + (UINT32)raw1;
					*(fpn_32_img_buff + fpn_32_offset + 1) = (UINT32)(*(fpn_32_img_buff + fpn_32_offset + 1)) + (UINT32)raw2;
					if ((dbg_print) && (offset == 0)) {
						printf("fpn_buffer_32[0] data = 0x%x, 0x%x \r\n", (UINT32)(*(fpn_32_img_buff + fpn_32_offset + 0)), (UINT32)(*(fpn_32_img_buff + fpn_32_offset + 1)));
					}

					fpn_32_offset = fpn_32_offset + 2;
				}
				printf(" frame %d done \r\n", i + 1);
				fflush(stdout); 
			}

			offset = 0;
			fpn_buffer = raw_buffer;
			memset(fpn_buffer.va, 0, raw_buffer_size);
			for (fpn_32_offset = 0; fpn_32_offset < (raw_info.raw_info.pw * raw_info.raw_info.ph); fpn_32_offset += 2) {
				fpn1 = (UINT32)(*((UINT32 *)fpn_buffer_32.va + fpn_32_offset + 0)) / frame_num;
				fpn2 = (UINT32)(*((UINT32 *)fpn_buffer_32.va + fpn_32_offset + 1)) / frame_num;
				if (fpn1 > 0xFFF) {
					fpn1 = 0xFFF;
				}
				if (fpn2 > 0xFFF) {
					fpn2 = 0xFFF;
				}

				if (offset > (raw_buffer_size - 3)) {
					printf("offset(%d) out of range !!!! \r\n", offset);
					break;
				}
				*((UINT8 *)fpn_buffer.va + offset + 0) = (UINT8)(fpn1 & 0x0FF);
				*((UINT8 *)fpn_buffer.va + offset + 1) = (UINT8)((fpn1 & 0xF00) >> 8 | (fpn2 & 0x00F) << 4);
				*((UINT8 *)fpn_buffer.va + offset + 2) = (UINT8)((fpn2 & 0xFF0) >> 4);

				if ((dbg_print) && (offset == 0)) {
					printf("fpn data = 0x%x, 0x%x \r\n", fpn1, fpn2);
					printf("fpn_buffer[0] 0x%x, 0x%x, 0x%x, \r\n", *((UINT8 *)fpn_buffer.va + 0), *((UINT8 *)fpn_buffer.va + 1), *((UINT8 *)raw_buffer.va + 2));
				}

				offset = offset + 3;
			}
			}

			ret = vendor_isp_get_common(ISPT_ITEM_SENSOR_INFO, &sensor_info);

			ae_status.id = id;
			vendor_isp_get_ae(AET_ITEM_STATUS, &ae_status);

			snprintf(fpn_file_path, 256, "/mnt/sd/%s_%d_fpn_w%d_h%d_12b_pack_exp%d_iso%d.fpn", sensor_info.sensor_info.name[id], file_idx, raw_info.raw_info.pw, raw_info.raw_info.ph, ae_status.status_info.expotime[0], ae_status.status_info.iso_gain[0]);
			printf("save file: %s \n", fpn_file_path);
			fp = fopen(fpn_file_path, "wb");
			if(fp == NULL){
				printf("open %s fail \r\n",fpn_file_path);
			}else{
				fwrite(fpn_buffer.va, 1, raw_buffer_size, fp);
				fflush(fp);
			}
			fclose(fp);
			
			set_ae(vcap_id, FALSE, &fpn_manual);
		break;

		case 5:
			{

				if(!ap_init){
					printf("Please init first!!\r\n");
				break;
			}				
			
			printf("Set isp id (0, 5)>> \n");
			fpn.id = (UINT32)get_choose_int();

			printf("Set enable (0, 1)>> \n");
			fpn.fpn.enable = (UINT32)get_choose_int();

			fpn.fpn.mode = IQ_OP_TYPE_MANUAL;
			fpn.fpn.manual_param.buf_phyaddr = fpn_buffer.pa;
			fpn.fpn.manual_param.buf_size = raw_buffer_size;
			fpn.fpn.manual_param.gain = 256;

			vendor_isp_set_iq(IQT_ITEM_FPN_PARAM, &fpn);
			printf("fpn id = %d \n", fpn.id);
			printf("fpn enable = %d \n", fpn.fpn.enable);
			printf("fpn mode = %d \n", fpn.fpn.mode);
			printf("fpn manual_param.buf_size = %d \n", fpn.fpn.manual_param.buf_size);
			printf("fpn manual_param.buf_phyaddr = 0x%x \n", fpn.fpn.manual_param.buf_phyaddr);
			printf("fpn manual_param.gain = %d \n", fpn.fpn.manual_param.gain);

			printf("fpn manual_param.buf = {0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ...} \n"
				, *((UINT32 *)fpn_buffer.va + 0), *((UINT32 *)fpn_buffer.va + 1), *((UINT32 *)fpn_buffer.va + 2), *((UINT32 *)fpn_buffer.va + 3)
				, *((UINT32 *)fpn_buffer.va + 4), *((UINT32 *)fpn_buffer.va + 5), *((UINT32 *)fpn_buffer.va + 6), *((UINT32 *)fpn_buffer.va + 7));
			}
		break;


		case 9:
			if(!ap_init){
				printf("init not yet!!\r\n");
				break;
			}

			if (hd_mem_init) {
				if (fpn_buffer_32.va != NULL) {
					ret = hd_common_mem_free(fpn_buffer_32.pa, fpn_buffer_32.va);
					if (ret != HD_OK) {
						printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(fpn_buffer_32.pa), (UINTPTR)(fpn_buffer_32.va));
					}
				}
				if (raw_buffer.va != NULL) {
					ret = hd_common_mem_free(raw_buffer.pa, raw_buffer.va);
					if (ret != HD_OK) {
						printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(raw_buffer.pa), (UINTPTR)(raw_buffer.va));
					}
				}
				if (dpc_buffer.va != NULL) {
					ret = hd_common_mem_free(dpc_buffer.pa, dpc_buffer.va);
					if (ret != HD_OK) {
						printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(dpc_buffer.pa), (UINTPTR)(dpc_buffer.va));
					}else{
						dpc_buffer.va = NULL;
					}
				}		
				
				
				hd_common_mem_uninit();
				hd_common_uninit();
				hd_mem_init = FALSE;
			}
			
				vendor_isp_uninit();
				hd_gfx_uninit();

				ap_init = FALSE;

			break;

		

		case 10:
			printf("set id >>>> ");
			vcap_id = get_choose_int();
			break;

		case 11:
			printf("set ob >>>> ");
			ob = get_choose_int();

			break;

		case 12:
			printf("set dpc th >>>> ");
			dpc_th = get_choose_int();

			break;
		case 13:
			printf("set expT for DPC >>>> ");
			dpc_manual.manual.expotime = get_choose_int();
			printf("set ISO for DPC >>>> ");
			dpc_manual.manual.iso_gain= get_choose_int();
			
			break;
		case 14:
			printf("set max dp number of total_pixel (1~50%)>>>> ");
			dpc_max_percentage = get_choose_int();
			break;		
		case 15:
				printf("set frame number for dpc >>>> ");
				dpc_frame_num = get_choose_int();
				break;




		case 31:
			printf("set frame number for FPN >>>> ");;
			frame_num = get_choose_int();
			break;			
		case 32:
			printf("set expT for FPN >>>> ");
			fpn_manual.manual.expotime = get_choose_int();
			printf("set ISO for FPN >>>> ");
			fpn_manual.manual.iso_gain= get_choose_int();

			break;
		default:
			printf("wrong input (%d) \n", option);
			break;

		case 0:
			trig = 0;
			break;
		}
	}
	if(ap_init){
		if (hd_mem_init) {
			if (fpn_buffer_32.va != NULL) {
				ret = hd_common_mem_free(fpn_buffer_32.pa, fpn_buffer_32.va);
				if (ret != HD_OK) {
					printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(fpn_buffer_32.pa), (UINTPTR)(fpn_buffer_32.va));
				}
			}
			if (raw_buffer.va != NULL) {
				ret = hd_common_mem_free(raw_buffer.pa, raw_buffer.va);
				if (ret != HD_OK) {
					printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(raw_buffer.pa), (UINTPTR)(raw_buffer.va));
				}
			}
		    if (dpc_buffer.va != NULL) {
				ret = hd_common_mem_free(dpc_buffer.pa, dpc_buffer.va);
				if (ret != HD_OK) {
					printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(dpc_buffer.pa), (UINTPTR)(dpc_buffer.va));
				}else{
					dpc_buffer.va = NULL;
				}
			}		
			
			
			hd_common_mem_uninit();
			hd_common_uninit();
		}

		vendor_isp_uninit();
		hd_gfx_uninit();
		}
	return 0;
}
