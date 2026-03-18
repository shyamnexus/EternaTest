
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "dpc_lib.h"
#include "vendor_isp.h"

#include "kwrap/file.h"
#include <kwrap/util.h>
#define MEASURE_TIME 0
#if (MEASURE_TIME)
#include "hd_common.h"
#include "hd_util.h"
#endif
#include "hdal.h"
#include "hd_videocapture.h"


#define PAUSE {printf("Press Enter key to continue..."); fgetc(stdin);}

#define CALI_MIN(x,y)    ((x) < (y) ? (x) : (y))
#define CALI_MAX(x,y)    ((x) > (y) ? (x) : (y))

static unsigned int get_choose_uint(void)
{
	char str_buf[32];
	unsigned int val, error;
	do {
		error = scanf(" %d", &val);
		if (error != 1) {
			printf("Invalid option. Try again.\n");
			clearerr(stdin);
			fgets(str_buf, sizeof(str_buf), stdin);
			printf(">> ");
		}
	} while(error != 1);

	return val;
}


static void vendor_dpc_set_AE(AET_MANUAL *ae_manual)
{
	int ret = 0;

	ret = vendor_isp_set_ae(AET_ITEM_MANUAL, ae_manual);

	if (ret < 0) {
		printf("SET AET_ITEM_MANUAL fail! \n");
		return;
	}

	return;
}


static IQT_DPC_PARAM dpc_param;
static void set_manual_ae(unsigned int sensor_id)
{
	AET_MANUAL ae={0};

	ae.id = sensor_id;
	ae.manual.mode= 1;
	ae.manual.totalgain= 0;
	printf("----------------------------------------\n");
	printf("SET AE Exposure&Gain: \r\n");
	printf("Enter Exposure:  \r\n");
	do {
		printf(">> ");
		ae.manual.expotime = get_choose_uint();
		printf("exposure=%d \r\n", ae.manual.expotime);
	} while (0);

	printf("Enter Gain:  \r\n");
	do {
		printf(">> ");
		ae.manual.iso_gain = get_choose_uint();
		printf("gain=%d \r\n", ae.manual.iso_gain);
	} while (0);

	vendor_dpc_set_AE(&ae);
		


}



static IQT_SHADING_PARAM shading_par;

static int bypass_ecs(int sensor_id, UINT32 *ecs_enable)
{
	int ret = 0;

	shading_par.id = sensor_id;

	ret = vendor_isp_get_iq(IQT_ITEM_SHADING_PARAM, &shading_par);
	if (ret < 0) {
		printf("GET IQT_ITEM_SHADING_PARAM fail! \n");
		return -1;
	}

	*ecs_enable = shading_par.shading.ecs_enable;
	shading_par.shading.ecs_enable = DISABLE;

	ret = vendor_isp_set_iq(IQT_ITEM_SHADING_PARAM, &shading_par);
	if (ret < 0) {
		printf("SET IQT_ITEM_SHADING_PARAM fail! \n");
		return -1;
	}
	return ret;
}

static int restore_ecs(int sensor_id, UINT32 ecs_enable)
{
	int ret = 0;

	shading_par.id = sensor_id;

	ret = vendor_isp_get_iq(IQT_ITEM_SHADING_PARAM, &shading_par);
	if (ret < 0) {
		printf("GET IQT_ITEM_SHADING_PARAM fail! \n");
		return -1;
	}

	shading_par.shading.ecs_enable = ecs_enable;

	ret = vendor_isp_set_iq(IQT_ITEM_SHADING_PARAM, &shading_par);
	if (ret < 0) {
		printf("SET IQT_ITEM_SHADING_PARAM fail! \n");
		return -1;
	}
	return ret;
}

unsigned int dpc_get_sensor_info(int sensor_id,  ISPT_SENSOR_MODE_INFO *p_sensor_mode_info)
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

static unsigned int g_dbg_print = 0;
static char* dbg_file_path = NULL;


void convert_12bit_to_8bit(UINT32 *src_addr, UINT32 *dest_addr, unsigned int width,  unsigned int height)
{
	unsigned int temp_addr = 0;
	char *buf = (char*)dest_addr;
	char *img_buff = (char*)src_addr;
	UINT32 image_size = width*height*3/2;
	static UINT32 cnt = 0;
	
		for (unsigned int data_addr = 0; data_addr < image_size; data_addr += 3) {
			int temp1_h = img_buff[data_addr + 1] & 0x0f;
			int temp1_l = img_buff[data_addr] & 0xff;
			int temp1 = (temp1_h << 8) + temp1_l;
			unsigned int data1 = temp1 * 255 / 4095;
	
			if (data1 > 255) {
				buf[temp_addr] = 255;
			} else {
				buf[temp_addr] = data1;
			}
	
			temp_addr++;
	
			int temp2_h = img_buff[data_addr + 2] & 0xff;
			int temp2_l = (img_buff[data_addr + 1] & 0xf0) >> 4;
			int temp2 = (temp2_h << 4) + temp2_l;
	
			unsigned int data2 = temp2 * 255 / 4095;
	
			if (data2 > 255) {
				buf[temp_addr] = 255;
			} else {
				buf[temp_addr] = data2;
			}
	
			temp_addr++;
		}

	if(dbg_file_path != NULL){

		FILE *fp;
		char dbg_raw_file_path[256];
		snprintf(dbg_raw_file_path, 256, "%s_raw_%d_%d_8bit_%d.raw",dbg_file_path,width,height,cnt);
		fp = fopen(dbg_raw_file_path, "wb");

		if(fp == NULL){
			printf("open %s fail \r\n",dbg_raw_file_path);
		}else{

			fwrite(buf, 1, width*height, fp);				
			fflush(fp);
			usleep(100000);
			}
		fclose(fp);		
		cnt++;
	}

}

unsigned int dpc_get_raw(int sensor_id, ISPT_RAW_INFO *final_raw_info, UINT32* raw_buf)
{
	HD_RESULT ret = 0;
	unsigned int data_len, residue_len, data;
	unsigned int image_width, image_height, j, block;
	unsigned int image_size = 1920*1080;
	UINT32 ecs_enable = 0;
	ISPT_RAW_INFO raw_info;
	BOOL dbg_save_raw = FALSE;
	char dbg_raw_file_path[256];
	char *img_buff = (char *)(raw_buf);
	//VOS_FILE fp;
	FILE *fp;
	static ISPT_MEMORY_INFO mem_info = {0};
	static int g_file_idx = 0;

	bypass_ecs(final_raw_info->id, &ecs_enable);
	g_file_idx++;

	//get raw
	final_raw_info->id = sensor_id;
	ret = vendor_isp_get_common(ISPT_ITEM_RAW, final_raw_info);
	if (ret < 0) {
		printf("GET ISPT_ITEM_RAW fail! \n");
		//set raw
		ret = vendor_isp_set_common(ISPT_ITEM_RAW, &final_raw_info->id);
		return -1;
	}
	if((g_dbg_print)&&(dbg_file_path != NULL)){
		dbg_save_raw = TRUE;
		printf("dbg save raw on \n");
	}

	final_raw_info->raw_info.pxlfmt = final_raw_info->raw_info.pxlfmt & 0xffff0000;
	if(g_dbg_print)
		printf("get raw, id = %d, %d x %d x %x\n", final_raw_info->id, final_raw_info->raw_info.pw, final_raw_info->raw_info.ph, final_raw_info->raw_info.pxlfmt);

	if (final_raw_info->raw_info.pxlfmt == VDO_PXLFMT_NRX12) {
		printf("This is compressed raw format. Only support uncompressed raw format\r\n");
		//set raw
		ret = vendor_isp_set_common(ISPT_ITEM_RAW, &final_raw_info->id);
		return -1;
	}

	if(g_dbg_print)
		printf("get lineoffset = %d  \r\n", final_raw_info->raw_info.loff);

	image_width = final_raw_info->raw_info.pw;
	image_height = final_raw_info->raw_info.ph;
	if(final_raw_info->raw_info.pxlfmt == VDO_PXLFMT_RAW12){
		image_size = image_width * image_height * 3 / 2;
	}else if(final_raw_info->raw_info.pxlfmt == VDO_PXLFMT_RAW8){
		image_size = image_width * image_height;
	}
	
	if ((image_width | image_height) == 0) {
		printf("image_width/image_height can not be zero \r\n");
		//set raw
		ret = vendor_isp_set_common(ISPT_ITEM_RAW, &raw_info.id);
		if (ret < 0) {
			printf("SET ISPT_ITEM_RAW fail! \n");
		}
		return -1;
	}

	// allocate raw buffer

	if (img_buff == NULL) {
		printf("fail to allocate memory for image buffer!\n");
		//set raw
		ret = vendor_isp_set_common(ISPT_ITEM_RAW, &final_raw_info->id);
		if (ret < 0) {
			printf("SET ISPT_ITEM_RAW fail! \n");
		}
		return -1;
	}

	if(final_raw_info->raw_info.pxlfmt == VDO_PXLFMT_RAW12){
		memset(img_buff, 0, image_width*image_height*3/2);
	}else if(final_raw_info->raw_info.pxlfmt == VDO_PXLFMT_RAW8){
		memset(img_buff, 0, image_width*image_height);
	}

	

	block = image_size % (32 * 1024) ? (image_size / 1024 / 32 + 1) : (image_size / 1024 / 32);
	printf("block %d \r\n", block);

	//move raw
	data_len = 32*1024;
	mem_info.addr = final_raw_info->raw_info.addr;
	residue_len = image_size;
	data = 0;


	for (j = 0; j < block ; j++) {
	
		mem_info.size = data_len;
			ret = vendor_isp_get_common(ISPT_ITEM_FRAME, &mem_info);
		//printf("finish get frame \r\n");
		if (ret < 0) {
			printf("GET ISPT_ITEM_FRAME fail! \n");
			//set raw
			ret = vendor_isp_set_common(ISPT_ITEM_RAW, &final_raw_info->id);
			return -1;
		}

		memcpy(&img_buff[data], mem_info.buf, data_len);

		mem_info.addr += 32*1024;
		residue_len -= data_len;
		data += data_len;
		data_len = (residue_len >= 32 * 1024) ? 32 * 1024 : residue_len;
	}

	//set raw
	ret = vendor_isp_set_common(ISPT_ITEM_RAW, &final_raw_info->id);
	if (ret < 0) {
		printf("SET ISPT_ITEM_RAW fail! \n");
		return -1;
	}


	if(dbg_save_raw){

		snprintf(dbg_raw_file_path, 256, "%s_raw_%d_%d_12bit_%d.raw",dbg_file_path,image_width,image_height,g_file_idx);
		printf("save file: %s \n", dbg_raw_file_path);
		fp = fopen(dbg_raw_file_path, "wb");
        if(fp == NULL){
            printf("open %s fail \r\n",dbg_raw_file_path);
        }else{
			fwrite(img_buff, 1, image_size, fp);				
			fflush(fp);
	        }
		fclose(fp);	
	
	}



	restore_ecs(final_raw_info->id, ecs_enable);


	printf("ret=%d \r\n", ret);

	return ret;
}


int main(int argc, char *argv[])
{
	unsigned int option, infinite_loop = 1;

	ISPT_RAW_INFO raw_info = {0};

	CAL_ALG_DP_RST dp_rst;
	unsigned int sensor_id;
	AET_MANUAL ae={0};
	unsigned int dpc_th = 20; //defual 20
	unsigned int dbg_en = 0;
	unsigned int dpc_en = 0;
	int open_device;
	IQT_CFG_INFO cfg_info = {0};

	int ret = 0;
	char dbg_raw_file_path[64] = "/mnt/sd/\0";

	ISPT_SENSOR_MODE_INFO sensor_mode_info;
	unsigned int raw_size, avg, j;
	char *raw_buf;
	char *tmp_buf = NULL;
	CAL_ALG_DP_PARAM dp_param;
	UINT32* dp_buf = NULL;
	UINT32 max_cnt = 4095;
	open_device = vendor_isp_init();

	if (open_device < 0) {
		printf("open MCU device fail!\n");
		return E_GET_DEV_FAIL;
	}

	// Enter sensor id

	printf("----------------------------------------\n");
	printf("sample 2.10, lib: %x\r\n",get_dpc_lib_version());

	printf("Enter sensor id: {0,1,2,3,4,5}  \r\n");
	do {
		printf(">> ");
		sensor_id = get_choose_uint();
		printf("cali_sensor_id=%d \r\n", sensor_id);
		raw_info.id = sensor_id;
		cfg_info.id = sensor_id;
		dpc_param.id = sensor_id;
		ae.id = sensor_id;
	} while (0);



	while (infinite_loop) {
		printf("----------------------------------------\n");
		printf(" 1. Start dpc cali\n");
		printf(" 2. load dpc bin\n");
		printf(" 3. Start dpc cali bright and dark pixel\n");
		printf(" 4. dpc on/off\n");
		printf(" 5. Dbg mode on/off\n");
		printf(" 6. Quit\n");
		printf("----------------------------------------\n");

		do {
			printf(">> ");
			option = get_choose_uint();
		} while (0);

		switch (option) {
		case 1:


			{
				set_manual_ae(sensor_id);
				// input threshold
			    printf("Enter threshold:  \r\n");
			    do {
			            printf(">> ");
			            dpc_th = get_choose_uint();
			            printf("dpc threshold = %d \r\n", dpc_th);
			    } while (0);

				dpc_get_sensor_info(sensor_id, &sensor_mode_info);
				//allocte raw buffer
				raw_size = sensor_mode_info.info.crp_size.w* sensor_mode_info.info.crp_size.h;
				raw_buf = (char *)malloc(sizeof(char)*raw_size* 3 / 2);
				if (raw_buf != NULL) {
					memset(raw_buf, 0, sizeof(char)*raw_size);
				} else	{
					printf("fail to allocate memory for image buffer!\n");
					break;
				}

				tmp_buf = (char *)malloc(sizeof(char)*raw_size);
				if(tmp_buf == NULL){
					free(raw_buf);
					raw_buf = 0;
					printf("fail to allocate memory for image buffer!\n");
					break;				
				}else{
					memset(tmp_buf, 0, sizeof(char)*raw_size);
				}


				if (dpc_get_raw(sensor_id, &raw_info, (UINT32*)raw_buf) == 0) {

						if(raw_info.raw_info.pxlfmt == VDO_PXLFMT_RAW8){
							raw_info.raw_info.addr = (ULONG)raw_buf;
							printf("VDO_PXLFMT_RAW8\n");
						}else if(raw_info.raw_info.pxlfmt == VDO_PXLFMT_RAW12){
							printf("VDO_PXLFMT_RAW12\n");
							convert_12bit_to_8bit( (UINT32*)raw_buf, (UINT32*)tmp_buf,raw_info.raw_info.pw,raw_info.raw_info.ph);
							raw_info.raw_info.addr = (ULONG)tmp_buf; //check
						}

					}else{
						free(raw_buf);
						raw_buf = 0;
						printf("error raw format\r\n");
						break;
					}

					dp_param.raw_info.pw = raw_info.raw_info.pw;
					dp_param.raw_info.ph = raw_info.raw_info.ph;
					dp_param.raw_info.loff = raw_info.raw_info.pw;
					dp_param.raw_info.addr = (ULONG)raw_info.raw_info.addr;
					dp_param.sie_act_str.x = sensor_mode_info.info.act_size[0].x;
					dp_param.sie_act_str.y = sensor_mode_info.info.act_size[0].y;
					
					dp_param.sie_crp_str.x = (sensor_mode_info.info.act_size[0].w - sensor_mode_info.info.crp_size.w) / 2;
					dp_param.sie_crp_str.y = (sensor_mode_info.info.act_size[0].h - sensor_mode_info.info.crp_size.h) / 2;					

					printf("buffer %x, x %d y %d\r\n", (ULONG)dp_param.raw_info.addr , dp_param.sie_act_str.x,dp_param.sie_act_str.y);

					
					//	raw_info;
					{
						char* p_tmp = (char*)dp_param.raw_info.addr;
						avg = 0;
						for (j = 0; j < raw_size; j++) {
							avg += p_tmp[j];
						}
						avg /= raw_size;
						printf("buffer %x, average lum = %d\r\n", (ULONG)p_tmp, avg);
					}
					dp_buf = (UINT32 *)malloc(max_cnt*sizeof(UINT32));
					if (dp_buf != NULL) {
						memset(dp_buf, 0, max_cnt*sizeof(UINT32));
					} else{
						printf("fail to allocate memory for image buffer!\n");
						break;
					}


					dp_param.dp_pool_addr = (ULONG)&dp_buf[0];
					dp_param.setting.threshold = dpc_th;
					dp_param.ori_dp_cnt = 0;
					dp_param.max_dp_cnt = max_cnt;
					dp_param.b_chg_dp_format = DP_PARAM_CHGFMT_AUTO;
					printf("addr %x, addr %x\r\n", (ULONG)&dp_param, (ULONG)&dp_rst);
					cal_dp_process(&dp_param, &dp_rst);

					printf("cnt=%d,data_length=%d \r\n", dp_rst.pixel_cnt, dp_rst.data_length);

				
			
			// save dp bin
			FILE *fp;
			fp = fopen("/mnt/sd/dp.bin", "wb");
			if(fp != NULL){
				fwrite(dp_buf, 1, 4096*sizeof(UINT32), fp);				
				fflush(fp);
				
				printf("dp_buf:%x\r\n",dp_buf);

			} else {
				printf("fp is NULL\r\n");
			}
			fclose(fp);

			usleep(1000000);

			if (raw_buf != NULL) {
				printf("free_raw\r\n");
			    free(raw_buf);
			    raw_buf = NULL;
			}
			if (tmp_buf != NULL) {
				printf("free_tmp\r\n");
			    free(tmp_buf);
			    tmp_buf = NULL;
			}
			if (dp_buf != NULL) {
				printf("free_dp\r\n");
			    free(dp_buf);
			    dp_buf = NULL;
			}

			break;

		case 2:
			// set dp bin
			strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
			ret = vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
			if (ret < 0) {
				printf("SET IQT_ITEM_RLD_CONFIG fail!\n");
				break;
			}

			// get dpc par
			ret = vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc_param);
			if (ret < 0) {
				printf("GET IQT_ITEM_DPC_PARAM fail!\n");
				break;
			}
			//printf("dpc_param 0x%x 0x%x\e\n",dpc_param.dpc.table[0],dpc_param.dpc.table[1]);
			break;

		case 3:
			{

				//get raw info
				dpc_get_sensor_info(sensor_id, &sensor_mode_info);
				raw_size = sensor_mode_info.info.crp_size.w* sensor_mode_info.info.crp_size.h;
				//get raw buffer
				raw_buf = (char *)malloc(sizeof(char)*raw_size);
				tmp_buf = (char *)malloc(sizeof(char)*raw_size* 3 / 2);

				if (raw_buf != NULL) {
					memset(raw_buf, 0, sizeof(char)*raw_size);
				} else	{
					printf("fail to allocate memory for image buffer!\n");
					break;
				}
				if (tmp_buf != NULL) {
					memset(tmp_buf, 0, sizeof(char)*raw_size* 3 / 2);
				} else	{
					printf("fail to allocate memory for image buffer!\n");
					break;
				}

				//get dp buffer
				dp_buf = (UINT32 *)malloc(max_cnt*sizeof(UINT32));
				if (dp_buf != NULL) {
					memset(dp_buf, 0, max_cnt*sizeof(UINT32));
				} else{
					printf("fail to allocate memory for image buffer!\n");
					break;
				}

				//bright pixel
				printf("\n Bright Pixel: SET AE Exposure&Gain \r\n\n");
				set_manual_ae(sensor_id);

				// input threshold
		    	printf("\n Bright Pixel: Enter threshold:  \r\n");
		    	do {
		            printf(">> ");
		            dpc_th = get_choose_uint();
		            printf("dpc threshold = %d \r\n", dpc_th);
		    	} while (0);
				printf("\r\n");

				if (dpc_get_raw(sensor_id, &raw_info, (UINT32*)tmp_buf) == 0) {

					if(raw_info.raw_info.pxlfmt == VDO_PXLFMT_RAW8){
						raw_info.raw_info.addr = (ULONG)tmp_buf;
						printf("VDO_PXLFMT_RAW8\n");
					}else if(raw_info.raw_info.pxlfmt == VDO_PXLFMT_RAW12){
										
						convert_12bit_to_8bit( (UINT32*)tmp_buf, (UINT32*)raw_buf,raw_info.raw_info.pw,raw_info.raw_info.ph);
						raw_info.raw_info.addr = (ULONG)raw_buf;

					}else{
						free(raw_buf);
						raw_buf = 0;
						printf("error raw format\r\n");
						break;
					}
					dp_param.raw_info.pw = raw_info.raw_info.pw;
					dp_param.raw_info.ph = raw_info.raw_info.ph;
					dp_param.raw_info.loff = raw_info.raw_info.pw;
					dp_param.raw_info.addr =(ULONG)raw_info.raw_info.addr;
					dp_param.sie_act_str.x = sensor_mode_info.info.act_size[0].x;
					dp_param.sie_act_str.y = sensor_mode_info.info.act_size[0].y;
					
					dp_param.sie_crp_str.x = (sensor_mode_info.info.act_size[0].w - sensor_mode_info.info.crp_size.w) / 2;
					dp_param.sie_crp_str.y = (sensor_mode_info.info.act_size[0].h - sensor_mode_info.info.crp_size.h) / 2;					

					avg = 0;
					for (j = 0; j < raw_size; j++) {
						avg += raw_buf[j];
					}
					avg /= raw_size;
					printf("average lum = %d\r\n", avg);

					dp_param.dp_pool_addr = (ULONG)&dp_buf[0];
					dp_param.setting.threshold = dpc_th;
					dp_param.ori_dp_cnt = 0;
					dp_param.max_dp_cnt = max_cnt;
					dp_param.b_chg_dp_format = DP_PARAM_CHGFMT_SKIP;
					cal_dp_process(&dp_param, &dp_rst);

					printf("cnt=%d,data_length=%d \r\n", dp_rst.pixel_cnt, dp_rst.data_length);

				}else{
					break;
				}//Bright pixel end

				//Dark pixel
				printf("\n Dright Pixel: SET AE Exposure&Gain \r\n");
				set_manual_ae(sensor_id);

				// input threshold
		    	printf("\n Dright Pixel: Enter threshold:  \r\n");
		    	do {
		            printf(">> ");
		            dpc_th = get_choose_uint();
		            printf("dpc threshold = %d \r\n", dpc_th);
		    	} while (0);
				printf("\r\n");

				if (dpc_get_raw(sensor_id, &raw_info, (UINT32*)tmp_buf) == 0) {

					if(raw_info.raw_info.pxlfmt == VDO_PXLFMT_RAW8){
						raw_info.raw_info.addr = (ULONG)tmp_buf;
						printf("VDO_PXLFMT_RAW8\n");
					}else if(raw_info.raw_info.pxlfmt == VDO_PXLFMT_RAW12){
										
						convert_12bit_to_8bit( (UINT32*)tmp_buf, (UINT32*)raw_buf,raw_info.raw_info.pw,raw_info.raw_info.ph);
						raw_info.raw_info.addr = (ULONG)raw_buf;
					}else{
						free(raw_buf);
						raw_buf = 0;
						printf("error raw format\r\n");
						break;
					}
					dp_param.raw_info.pw = raw_info.raw_info.pw;
					dp_param.raw_info.ph = raw_info.raw_info.ph;
					dp_param.raw_info.loff = raw_info.raw_info.pw;
					dp_param.raw_info.addr =raw_info.raw_info.addr;
					dp_param.sie_act_str.x = sensor_mode_info.info.act_size[0].x;
					dp_param.sie_act_str.y = sensor_mode_info.info.act_size[0].y;
					
					dp_param.sie_crp_str.x = (sensor_mode_info.info.act_size[0].w - sensor_mode_info.info.crp_size.w) / 2;
					dp_param.sie_crp_str.y = (sensor_mode_info.info.act_size[0].h - sensor_mode_info.info.crp_size.h) / 2;					

					avg = 0;
					for (j = 0; j < raw_size; j++) {
						avg += raw_buf[j];
					}
					avg /= raw_size;
					printf("average lum = %d\r\n", avg);

					dp_param.dp_pool_addr = (ULONG)&dp_buf[0];
					dp_param.setting.threshold = dpc_th;
					dp_param.ori_dp_cnt = dp_rst.pixel_cnt;
					dp_param.max_dp_cnt = max_cnt;
					dp_param.b_chg_dp_format = DP_PARAM_CHGFMT_AUTO;

					cal_dp_process(&dp_param, &dp_rst);


					printf("cnt=%d,data_length=%d \r\n", dp_rst.pixel_cnt, dp_rst.data_length);

				}//Dark pixel end

				// save dp bin

				FILE *fp;
				fp = fopen("/mnt/sd/dp.bin", "wb");
				if(fp != NULL){
					fwrite(dp_buf, 1, 4096*sizeof(UINT32), fp);				
					fflush(fp);	
					
					printf("dp_buf:%x\r\n",dp_buf);

				} else {
					printf("fp is NULL\r\n");
				}
				fclose(fp);

				usleep(1000000);
				if (raw_buf != NULL) {
				    free(raw_buf);
				    raw_buf = NULL;
				}
				if (tmp_buf != NULL) {
				    free(tmp_buf);
				    tmp_buf = NULL;
				}
				if (dp_buf != NULL) {
				    free(dp_buf);
				    dp_buf = NULL;
				}

			}
			break;


		case 4:
			// set dpc enable
			printf("DPC enable: (0:off, 1:on)  \r\n");
			do {
				printf(">> ");
				dpc_en = get_choose_uint();
				printf("dpc_en=%d \r\n", dpc_en);
			} while (0);
			ret = vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc_param);
			dpc_param.dpc.enable = dpc_en;
			// get dpc par
			ret = vendor_isp_set_iq(IQT_ITEM_DPC_PARAM, &dpc_param);
			if (ret < 0) {
				printf("SET IQT_ITEM_DPC_PARAM fail!\n");
				break;
			}

			break;

		case 5:
			// dbg msg on/off
			printf("dbg enable: (0:off, 1:on)  \r\n");
			do {
				printf(">> ");
				dbg_en = get_choose_uint();
				printf("dbg_en=%d \r\n", dbg_en);
			} while (0);
			cali_dpc_set_dbg_out(dbg_en);
			g_dbg_print =dbg_en;
			dbg_file_path = dbg_raw_file_path ;
			break;

		case 6:
		default:
			infinite_loop = 0;
			break;
		}
    }
		}

/*
	dpc_exit();
*/
	ae.manual.mode= 0;
	vendor_dpc_set_AE(&ae);
	ret = vendor_isp_uninit();
	if(ret != HD_OK) {
		printf("vendor_isp_uninit fail=%d\n", ret);
	}


	return 0;
}

