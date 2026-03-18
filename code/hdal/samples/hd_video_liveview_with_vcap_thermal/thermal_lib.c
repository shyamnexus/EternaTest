/**
	@brief thermal lib\n

	@file thermal_lib.c

	@author Pin Su

	@ingroup mhdal

	Copyright Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#include "thermal_lib.h"


#define TSEN_IRAY_CALIBRATION_RAW_NUM  16
#define TSEN_RAW_BITDEPTH               2 //bytes


static TSEN_SENSOR_ID g_sensor_id = IRAY_6122C_25HZ;
static TSEN_RAW_INFO g_raw_info = {0};
static UINT32 g_config_id = 0;
static UINT32 g_ooc_id = 0;
static UINT32 g_config_coverge =0, g_ooc_coverge = 0;
static ULONG g_working_buf = 0, g_ooc_diff_buf = 0, g_ooc_buf = 0;
static UINT32 g_working_buf_sz = 0;

#define CALIBRATION_LUT_LEN 64
#define CALIBRATION_STATUS_NOT_READY 0x10
#define CALIBRATION_STATUS_READY     0x11
UINT8 OOC_Calibration_LUT[CALIBRATION_LUT_LEN][3]={0};
#define IRAY_6122C_CONVERGE_VAL 48

//Flow:
//init ooc_id = 0, config_id = 0

//update raw, ooc_id = 0, done, update ooc_id = 1 or config_id = 1
//update raw, ooc_id = 1, done, update ooc_id = 2 or config_id = 2
//...
//update raw, ooc_id = 31, done, update ooc_id = 32 (don't care), update_statue |= UPDATE_DONE;


UINT32 tsen_get_working_buf_size(TSEN_SENSOR_ID sensor_id, TSEN_RAW_INFO *p_raw_info)
{
	UINT32 uiBuf_sz = 0;

	g_sensor_id = sensor_id;
	g_raw_info.sz_x = p_raw_info->sz_x;
	g_raw_info.sz_y = p_raw_info->sz_y;
	g_raw_info.lofs = p_raw_info->lofs;
	g_raw_info.depth = p_raw_info->depth;

	//DBG_DUMP("raw w/h/lofs/depth = %d/%d/%d/%d\r\n",g_raw_info.sz_x,g_raw_info.sz_y,g_raw_info.lofs,g_raw_info.depth);

	if (g_sensor_id == IRAY_6122C_25HZ)
	{
		uiBuf_sz = TSEN_IRAY_CALIBRATION_RAW_NUM*g_raw_info.sz_x*g_raw_info.sz_y*g_raw_info.depth;
	}else if (g_sensor_id == IRAY_6122C_50HZ)
	{
		uiBuf_sz = TSEN_IRAY_CALIBRATION_RAW_NUM*g_raw_info.sz_x*g_raw_info.sz_y*g_raw_info.depth;
	}else {
		uiBuf_sz = 123456;
	}

	return uiBuf_sz; // bytes
}

void tsen_init(ULONG work_buf, UINT32 work_buf_size)
{
	g_working_buf = work_buf;
	g_working_buf_sz = work_buf_size;

	g_ooc_diff_buf = g_working_buf;

	//DBG_DUMP("LIB working buf 0x%08x, sz = %d\r\n",g_working_buf, g_working_buf_sz);
}


void tsen_get_init_config(TSEN_CONFIG *p_config, UINT32 serial_id)
{
	if (g_sensor_id == IRAY_6122C_25HZ) {
		switch (serial_id) {
		case 0:
		case 1:
			p_config->tx_data[0] = 0x20002aff;
			p_config->tx_data[1] = 0x0b3b1427;
			p_config->tx_data[2] = 0x5453d772;
			p_config->tx_data[3] = 0x7ffffab1;
			p_config->tx_data[4] = 0xb00c0c29;
			p_config->tx_data[5] = 0x03a1501e;
			p_config->tx_data[6] = 0x4e9141fe;
			p_config->tx_data[7] = 0x0c784aa7;
			p_config->tx_data[8] = 0x2493d8f6;
			p_config->tx_data[9] = 0x20c49209;
			p_config->tx_data[10] = 0x82008040;
			p_config->tx_data[11] = 0x70df3322;
			p_config->tx_data[12] = 0x008e2f59;
			p_config->tx_data[13] = 0x00000000;
			p_config->tx_data[14] = 0x00000000;
			p_config->tx_data[15] = 0x00000000;
			break;
		case 2:
			p_config->tx_data[0] = 0x20002aff;
			p_config->tx_data[1] = 0x0b3b1427;
			p_config->tx_data[2] = 0x5453d772;
			p_config->tx_data[3] = 0x7ffffab1;
			p_config->tx_data[4] = 0xb00c0c29;
			p_config->tx_data[5] = 0x03a1501e;
			p_config->tx_data[6] = 0x4e9141fe;
			p_config->tx_data[7] = 0x0c784a47;//0x0c784aa7;
			p_config->tx_data[8] = 0x2493d8f6;
			p_config->tx_data[9] = 0x20c49209;
			p_config->tx_data[10] = 0x82008040;
			p_config->tx_data[11] = 0x70df3322;
			p_config->tx_data[12] = 0x008e2f59;
			p_config->tx_data[13] = 0x00000000;
			p_config->tx_data[14] = 0x00000000;
			p_config->tx_data[15] = 0x00000000;
			break;
		default:
			printf("unsupported serial id(%d)\r\n", serial_id);
			break;
		}
		p_config->len = 408+14;
		p_config->id = TSEN_RESTART_CALIBRATION; // don't care
	}

}

void tsen_get_init_ooc(TSEN_OOC_INFO *p_ooc_info)
{
	UINT32 i;

	if (g_sensor_id == IRAY_6122C_25HZ) {

		// update OOC LUT for internal usage
		for (i=0;i<CALIBRATION_LUT_LEN;i++)
		{
			OOC_Calibration_LUT[i][0] = i;
			OOC_Calibration_LUT[i][1] = i+16;
			OOC_Calibration_LUT[i][2] = CALIBRATION_STATUS_NOT_READY;

			g_ooc_buf = g_ooc_diff_buf + (p_ooc_info->width*p_ooc_info->height*4);
			memset((void *)g_ooc_buf, 0x20, p_ooc_info->lofs*p_ooc_info->height);
			memset((void *)g_ooc_diff_buf, 0x0, p_ooc_info->width*p_ooc_info->height*4);
		}
		// update OOC buffer
		memset((void *)p_ooc_info->addr, 0x20, p_ooc_info->lofs*p_ooc_info->height);
		p_ooc_info->id = TSEN_RESTART_CALIBRATION; // don't care
	}

}

TSEN_UPDATE_RESULT tsen_update_params(ULONG raw_buf, TSEN_OOC_INFO *p_ooc_info, TSEN_CONFIG *p_config)
{
	TSEN_UPDATE_RESULT update_statue = UPDATE_NONE;
	UINT8 ooc_value;

	if (p_ooc_info->id == TSEN_RESTART_CALIBRATION && p_config->id == TSEN_RESTART_CALIBRATION) { // re-start calibration flow
		g_config_coverge = 1;
		g_ooc_coverge = 0;
	} else	if (p_ooc_info->id != g_ooc_id && p_config->id != g_config_id) {
		// id is not match with global id (lib), that means the RAW data is un-useful, it's no need to update to working buffer
		return update_statue;
	}

	//========================== start calibration flow =============================

	//update config if necessary
	if (g_config_coverge == 0) {
		// update config data
		if (p_ooc_info->id == TSEN_RESTART_CALIBRATION && p_config->id == TSEN_RESTART_CALIBRATION)
			g_config_id = 0;
		else
			g_config_id++;
		p_config->id = g_config_id;
		update_statue |= UPDATE_CONFIG;
	}

	//updae ooc if necessary
	if (g_ooc_coverge == 0) {
		// update ooc to working buffer

		if (p_ooc_info->id == TSEN_RESTART_CALIBRATION && p_config->id == TSEN_RESTART_CALIBRATION) {// re-start calibration
			g_ooc_id = 0;
			// update new ooc value to ooc buffer
			ooc_value = OOC_Calibration_LUT[g_ooc_id][1];
			memset((void *)p_ooc_info->addr, ooc_value, p_ooc_info->lofs*p_ooc_info->height);
			// reset internal OOC LUT
			UINT32 i;
			for (i=0;i<CALIBRATION_LUT_LEN;i++)
			{
				OOC_Calibration_LUT[i][0] = i;
				OOC_Calibration_LUT[i][1] = i+16;
				OOC_Calibration_LUT[i][2] = CALIBRATION_STATUS_NOT_READY;
				memset((void *)g_ooc_buf, 0x20, p_ooc_info->lofs*p_ooc_info->height);
				memset((void *)g_ooc_diff_buf, 0x0, p_ooc_info->width*p_ooc_info->height*4);
			}
		}else {
			UINT32 x,y;
			UINT32 ooc_cali_src_stx, ooc_cali_src_sty;
			UINT32 ooc_target;
			UINT32 out_x_cnt, out_y_cnt;
			UINTPTR temp;
			UINT32 temp2, temp3, tempdd;
			UINT32 diff;
			UINT32 ooc_out, min_out_diff;
			UINT32 cur_min_diff;
			UINT32 ooc_start_val = OOC_Calibration_LUT[0][1];

			// copy raw data to working buffer

			/////////////// parameters ////////////
			ooc_cali_src_stx = 20+10;
			ooc_cali_src_sty = 8;
			ooc_target = 8000;
			///////////////////////////////////////

			out_y_cnt=5;
			for (y=ooc_cali_src_sty;y<(ooc_cali_src_sty+512);y++)
			{
				out_x_cnt=10;
				for (x=(ooc_cali_src_stx*2);x<(ooc_cali_src_stx*2+640*2);x+=2)
				{
					temp = (UINTPTR)(raw_buf) + (y*g_raw_info.sz_x*g_raw_info.depth+x);
					temp2 = TSEN_GET_8BitsValue(temp);
					temp3 = TSEN_GET_8BitsValue(temp+1);
					tempdd = (temp3<<8)+temp2;

					if (tempdd >=ooc_target)
						diff = tempdd - ooc_target;
					else
						diff = ooc_target - tempdd;
					if (g_ooc_id == 0) { // first run
						ooc_out = 0;
						min_out_diff = diff;

						TSEN_SET_32BitsValue(g_ooc_diff_buf + (out_y_cnt*p_ooc_info->width*4+out_x_cnt*4), min_out_diff);
						TSEN_SET_8BitsValue(g_ooc_buf + (out_y_cnt*p_ooc_info->lofs+out_x_cnt), ooc_out + ooc_start_val);
					}else {
						cur_min_diff = TSEN_GET_32BitsValue(g_ooc_diff_buf + (out_y_cnt*p_ooc_info->width*4+out_x_cnt*4));

						if (diff < cur_min_diff) {
							ooc_out = g_ooc_id;
							min_out_diff = diff;

							TSEN_SET_32BitsValue(g_ooc_diff_buf + (out_y_cnt*p_ooc_info->width*4+out_x_cnt*4), min_out_diff);
							TSEN_SET_8BitsValue(g_ooc_buf + (out_y_cnt*p_ooc_info->lofs+out_x_cnt), ooc_out + ooc_start_val);
						}
					}
					out_x_cnt++;
				}
				out_y_cnt++;
			}
			// update ooc status for internal
			OOC_Calibration_LUT[g_ooc_id][2] = CALIBRATION_STATUS_READY;

			// update ooc id
			g_ooc_id++;

			// update new ooc value to ooc buffer
			if (OOC_Calibration_LUT[g_ooc_id][1] == IRAY_6122C_CONVERGE_VAL) {
				g_ooc_coverge = 1;
				memcpy((void *)p_ooc_info->addr, (void *)g_ooc_buf, p_ooc_info->lofs*p_ooc_info->height);
			}else {
				// fill next ooc value to ooc-buffer
				ooc_value = OOC_Calibration_LUT[g_ooc_id][1];
				memset((void *)p_ooc_info->addr, ooc_value, p_ooc_info->lofs*p_ooc_info->height);
			}
		}

		p_ooc_info->id = g_ooc_id;
		update_statue |= UPDATE_OOC;
	}
	//========================== end calibration flow =============================

	//if config + OOC all done
	if (g_config_coverge && g_ooc_coverge) {
		update_statue |= UPDATE_DONE;
	}


	return update_statue;
}
void tsen_dump_int_status(void)
{
	UINT32 i;

	printf("Calibration lib internal LUT:\r\n");
	printf("======================================\r\n");
	printf("OOC_ID | VAL | STATUS\r\n");
	for (i=0;i<CALIBRATION_LUT_LEN;i++)
	{
		printf("%d  %d  ",OOC_Calibration_LUT[i][0],OOC_Calibration_LUT[i][1]);
		if (OOC_Calibration_LUT[i][2] == CALIBRATION_STATUS_NOT_READY)
			printf("CALIBRATION_STATUS_NOT_READY\r\n");
		else
			printf("CALIBRATION_STATUS_READY\r\n");
	}
	printf("======================================\r\n");
}


