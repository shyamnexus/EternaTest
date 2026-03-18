#if defined(__KERNEL__)
#else
#include <string.h>
#endif

#include "kwrap/debug.h"
#include "sensor_builtin.h"

#if (NVT_FAST_ISP_FLOW)
#define MAX_VD_PERIOD              0xFFFF
#define MAX_EXPOSURE_LINE          0xFFFF
#define MIN_EXPOSURE_LINE          2
#define NON_EXPOSURE_LINE          8
#define MIN_HDR_EXPOSURE_LINE      2
#define NON_HDR_EXPOSURE_LINE      12
#define HCG_ENABLE                 0

static UINT32 compensation_ratio[ISP_BUILTIN_ID_MAX_NUM][ISP_SEN_MFRAME_MAX_NUM] = {0};
static UINT32 exp_line[ISP_BUILTIN_ID_MAX_NUM][ISP_SEN_MFRAME_MAX_NUM] = {0};

UINT32 sensor_builtin_os04c10_get_min_expt_time(UINT32 id, SENSOR_BUILTIN_DFT_PARAM *sensor_param)
{

	return (sensor_param->row_time * MIN_EXPOSURE_LINE / 10 + 1);
}

void sensor_builtin_os04c10_set_gain(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl, SENSOR_BUILTIN_DFT_PARAM *sensor_param)
{
	UINT32 data1[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 data2[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 frame_cnt, total_frame;
	SENSOR_BUILTIN_I2C_CMD cmd;
	UINT32 a_gain = 0, d_gain = 0, conversion_gain = 0, timing_reg = 0;
	static UINT32 hcg_change_flag[ISP_BUILTIN_ID_MAX_NUM] = {0};
	UINT32 temp_gain[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 conversion_gain_reg[26] = {0};
	ISP_BUILTIN_SENSOR_CTRL gain_info;

	memcpy(&gain_info, sensor_ctrl, sizeof(ISP_BUILTIN_SENSOR_CTRL));

	total_frame = sensor_param->frame_num;

	for (frame_cnt = 0; frame_cnt < total_frame; frame_cnt++) {
		if (100 <= (compensation_ratio[id][frame_cnt])) {
			gain_info.gain_ratio[frame_cnt] = (gain_info.gain_ratio[frame_cnt]) * (compensation_ratio[id][frame_cnt]) / 100;
		}
		if (gain_info.gain_ratio[frame_cnt] < 1000) {
			gain_info.gain_ratio[frame_cnt] = 1000;
		} else if (gain_info.gain_ratio[frame_cnt] > sensor_param->max_gain) {
			gain_info.gain_ratio[frame_cnt] = sensor_param->max_gain;
		}

		temp_gain[frame_cnt] = gain_info.gain_ratio[frame_cnt];

		if (!isp_builtin_get_shdr_enable(id)) {
			if ((HCG_ENABLE) && (3000 <= (gain_info.gain_ratio[frame_cnt]))) {
				{
					static BOOL is_first = TRUE;
					if (is_first) {
						hcg_change_flag[id] = 1;
						is_first = FALSE;
					}
				}
				temp_gain[frame_cnt] /= 3;
				if (!hcg_change_flag[id]) {
					timing_reg = 0xF8;
				} else {
					timing_reg = 0x88;
				}
				hcg_change_flag[id] = 1;
			} else {
				if (hcg_change_flag[id]) {
					timing_reg = 0xF8;
				} else {
					timing_reg = 0x88;
				}
				hcg_change_flag[id] = 0;
			}
		}

		if (isp_builtin_get_shdr_enable(id)) {
			if ((HCG_ENABLE) && (3000 <= (gain_info.gain_ratio[0])) && (3000 <= (gain_info.gain_ratio[1]))) {
				{
					static BOOL is_first = TRUE;
					if (is_first) {
						hcg_change_flag[id] = 1;
						is_first = FALSE;
					}
				}
				temp_gain[frame_cnt] /= 3;
				if (!frame_cnt) {
					if (!hcg_change_flag[id]) {
						timing_reg = 0xF8;
					} else {
						timing_reg = 0x88;
					}
					hcg_change_flag[id] = 1;
				}
			} else {
				if (!frame_cnt) {
					if (hcg_change_flag[id]) {
						timing_reg = 0xF8;
					} else {
						timing_reg = 0x88;
					}
					hcg_change_flag[id] = 0;
				}
			}
		}

		if((temp_gain[frame_cnt]) <= 15500) {
			a_gain = (temp_gain[frame_cnt]) * 128 / 1000;
			d_gain = 0x400;
		} else {
			d_gain = (temp_gain[frame_cnt]) * 1024 / 15500;
			a_gain = 0x7C0;
		}

		if (0x400 > d_gain) {
			d_gain = 0x400;
		} else if (0x3FFF < d_gain) {
			d_gain = 0x3FFF;
		}

		data1[frame_cnt] = a_gain;
		data2[frame_cnt] = d_gain;
	}

	if (!isp_builtin_get_shdr_enable(id)) {
		if (hcg_change_flag[id]) {
			conversion_gain = 0x40;
		} else {
			conversion_gain = 0xC0;
		}
	}

	if (isp_builtin_get_shdr_enable(id)) {
		if (hcg_change_flag[id]) {
			conversion_gain_reg [0] = 0x42;
			conversion_gain_reg [1] = 0x18;
			conversion_gain_reg [2] = 0x18;
			conversion_gain_reg [3] = 0x14;
			conversion_gain_reg [4] = 0x14;
			conversion_gain_reg [5] = 0xA6;
			conversion_gain_reg [6] = 0x53;
			conversion_gain_reg [7] = 0x2A;
			conversion_gain_reg [8] = 0x15;
			conversion_gain_reg [9] = 0x53;
			conversion_gain_reg [10] = 0x0C;
			conversion_gain_reg [11] = 0x00;
			conversion_gain_reg [12] = 0x0C;
			conversion_gain_reg [13] = 0x04;
			conversion_gain_reg [14] = 0x08;
			conversion_gain_reg [15] = 0x08;
			conversion_gain_reg [16] = 0x25;
			conversion_gain_reg [17] = 0xA6;
			conversion_gain_reg [18] = 0x53;
			conversion_gain_reg [19] = 0x2A;
			conversion_gain_reg [20] = 0x15;
			conversion_gain_reg [21] = 0x04;
			conversion_gain_reg [22] = 0x00;
			conversion_gain_reg [23] = 0x00;
			conversion_gain_reg [24] = 0x08;
			conversion_gain_reg [25] = 0x25;
		} else {
			conversion_gain_reg [0] = 0x00;
			conversion_gain_reg [1] = 0x80;
			conversion_gain_reg [2] = 0x80;
			conversion_gain_reg [3] = 0x1F;
			conversion_gain_reg [4] = 0x1F;
			conversion_gain_reg [5] = 0x80;
			conversion_gain_reg [6] = 0x40;
			conversion_gain_reg [7] = 0x21;
			conversion_gain_reg [8] = 0x12;
			conversion_gain_reg [9] = 0xDD;
			conversion_gain_reg [10] = 0x00;
			conversion_gain_reg [11] = 0x04;
			conversion_gain_reg [12] = 0x00;
			conversion_gain_reg [13] = 0x00;
			conversion_gain_reg [14] = 0x26;
			conversion_gain_reg [15] = 0xA8;
			conversion_gain_reg [16] = 0x00;
			conversion_gain_reg [17] = 0x80;
			conversion_gain_reg [18] = 0x40;
			conversion_gain_reg [19] = 0x21;
			conversion_gain_reg [20] = 0x12;
			conversion_gain_reg [21] = 0x00;
			conversion_gain_reg [22] = 0x04;
			conversion_gain_reg [23] = 0xA0;
			conversion_gain_reg [24] = 0x26;
			conversion_gain_reg [25] = 0x00;
		}
	}

	if (!isp_builtin_get_shdr_enable(id)) {
		cmd = sensor_builtin_set_cmd_info(0x320D, 1, 0x00, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3208, 1, 0x00, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3501, 1, (exp_line[id][0]>>  8) & 0xFF, 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
	
		cmd = sensor_builtin_set_cmd_info(0x3502, 1, exp_line[id][0] & 0xFF , 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		//Set A gain
		cmd = sensor_builtin_set_cmd_info(0x3508, 1, (data1[0] >> 8) & 0x3F, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3509, 1, data1[0] & 0xFF, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		//Set D gain
		cmd = sensor_builtin_set_cmd_info(0x350A, 1, (data2[0] >> 8) & 0x3F, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x350B, 1, (data2[0]) & 0xFF, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3798, 1, conversion_gain, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3503, 1, timing_reg, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3208, 1, 0x10, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x320D, 1, 0x00, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3208, 1, 0xA0, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
	}

	if (isp_builtin_get_shdr_enable(id)) {
		cmd = sensor_builtin_set_cmd_info(0x320D, 1, 0x00, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3208, 1, 0x00, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		//Set A gain
		cmd = sensor_builtin_set_cmd_info(0x3508, 1, (data1[0] >> 8) & 0x3F, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3509, 1, data1[0] & 0xFF, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x350C, 1, (data1[1] >> 8) & 0x3F, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x350D, 1, data1[1] & 0xFF, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		//Set D gain
		cmd = sensor_builtin_set_cmd_info(0x350A, 1, (data2[0] >> 8) & 0x3F, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x350B, 1, (data2[0]) & 0xFF, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x350E, 1, (data2[1] >> 8) & 0x3F, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x350F, 1, data2[1] & 0xFF, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3698, 1, conversion_gain_reg[0], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3699, 1, conversion_gain_reg[1], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x369A, 1, conversion_gain_reg[2], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x369B, 1, conversion_gain_reg[3], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x369C, 1, conversion_gain_reg[4], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x369D, 1, conversion_gain_reg[5], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x369E, 1, conversion_gain_reg[6], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x369F, 1, conversion_gain_reg[7], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x35A0, 1, conversion_gain_reg[8], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x36A1, 1, conversion_gain_reg[9], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x370E, 1, conversion_gain_reg[10], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3713, 1, conversion_gain_reg[11], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x379C, 1, conversion_gain_reg[12], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x379D, 1, conversion_gain_reg[13], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x37BE, 1, conversion_gain_reg[14], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x37C7, 1, conversion_gain_reg[15], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3881, 1, conversion_gain_reg[16], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3681, 1, conversion_gain_reg[17], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3682, 1, conversion_gain_reg[18], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3683, 1, conversion_gain_reg[19], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3684, 1, conversion_gain_reg[20], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x370F, 1, conversion_gain_reg[21], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x379F, 1, conversion_gain_reg[22], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x37AC, 1, conversion_gain_reg[23], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x37BF, 1, conversion_gain_reg[24], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3880, 1, conversion_gain_reg[25], 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3503, 1, timing_reg, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3208, 1, 0x10, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x320D, 1, 0x00, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);

		cmd = sensor_builtin_set_cmd_info(0x3208, 1, 0xA0, 0x0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
	}
}

void sensor_builtin_os04c10_set_expt(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl, SENSOR_BUILTIN_DFT_PARAM *sensor_param)
{
	UINT32 line[ISP_SEN_MFRAME_MAX_NUM];
	UINT32 frame_cnt, total_frame;
	SENSOR_BUILTIN_I2C_CMD cmd;
	UINT32 expt_time = 0, sensor_vd = 0, cur_fps = 0, clac_fps = 0, t_row = 0, temp_max_short_line = 0;
	UINT32 temp_line[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 mawb_l_g = 1024;
	UINT32 mawb_s_g = 1024;
	UINT32 sensor_dft_vd, dft_fps;

	sensor_dft_vd = sensor_param->vd;
	dft_fps =       sensor_param->dft_fps;
	total_frame =   sensor_param->frame_num;

	// Calculate exposure line
	for (frame_cnt = 0; frame_cnt < total_frame; frame_cnt++) {
		// Calculates the exposure setting
		if (0 == sensor_param->row_time) {
			DBG_WRN("t_row  = 0, must >= 1 \r\n");
			sensor_param->row_time = 1;
		}
		line[frame_cnt] = (sensor_ctrl->exp_time[frame_cnt]) * 10 / sensor_param->row_time;

		if (!isp_builtin_get_shdr_enable(id)) {
			// Limit minimun exposure line
			if (line[frame_cnt] < MIN_EXPOSURE_LINE) {
				line[frame_cnt] = MIN_EXPOSURE_LINE;
			}
		} else {
			// Limit minimun exposure line
			if (line[frame_cnt] < MIN_HDR_EXPOSURE_LINE) {
				line[frame_cnt] = MIN_HDR_EXPOSURE_LINE;
			}
		}
	}

	if (isp_builtin_get_shdr_enable(id)) {
		expt_time = (line[0] + line[1])  * sensor_param->row_time / 10;
		temp_line[0] = line[0];
		temp_line[1] = line[1];
	} else {
		expt_time = (line[0]) * sensor_param->row_time / 10;
		temp_line[0] = line[0];
	}

	// Calculate fps
	if (0 == expt_time) {
		DBG_WRN("expt_time  = 0, must >= 1 \r\n");
		expt_time = 1;
	}

	cur_fps = (clac_fps < dft_fps) ? clac_fps : dft_fps;

	// Calculate new vd
	sensor_vd = sensor_dft_vd * dft_fps / isp_builtin_get_chgmode_fps(id);

	//Check max vts
	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("max vts overflow\r\n");
		sensor_vd = MAX_VD_PERIOD;
	}

	cmd = sensor_builtin_set_cmd_info(0x380E, 1, (sensor_vd >> 8) & 0xFF, 0);
	sensor_builtin_write_i2c_2b1b(id, &cmd);
	cmd = sensor_builtin_set_cmd_info(0x380F, 1, sensor_vd & 0xFF, 0);
	sensor_builtin_write_i2c_2b1b(id, &cmd);

	if (!isp_builtin_get_shdr_enable(id)) {
		if (line[0] > MAX_EXPOSURE_LINE) {
			DBG_ERR("max line overflow \r\n");
			line[0] = MAX_EXPOSURE_LINE;
		}

		if (line[0] > (MAX_EXPOSURE_LINE - NON_EXPOSURE_LINE)) {
			DBG_ERR("over max exposure line \r\n");
			line[0] = (MAX_EXPOSURE_LINE - NON_EXPOSURE_LINE);
		}

		if (line[0] > (sensor_vd - NON_EXPOSURE_LINE)) {
			line[0] = sensor_vd - NON_EXPOSURE_LINE;
		}
		compensation_ratio[id][0] = 100 * temp_line[0] / line[0];

		mawb_l_g = 1024;
		mawb_s_g = 1024;
		exp_line[id][0] = line[0];
/*
		// set exposure line to sensor
		cmd = sensor_builtin_set_cmd_info(0x3501, 1, (line[0] >> 8) & 0xFF, 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3502, 1, line[0] & 0xFF , 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
        DBG_ERR("line[0] =%d, row= %d \r\n",line[0], sensor_param->row_time );		
*/
		//set MAWB
		cmd = sensor_builtin_set_cmd_info(0x5102, 1, (mawb_l_g >> 8) & 0xFF, 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x5103, 1, mawb_l_g & 0xFF , 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x5142, 1, (mawb_s_g >> 8) & 0xFF, 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x5143, 1, mawb_s_g & 0xFF , 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
	} else {
		temp_max_short_line = (100000000 / cur_fps ) * 10 / 17 / t_row - 1; //max support 1:16

		if (line[0] > MAX_EXPOSURE_LINE) {
			DBG_ERR("max line overflow \r\n");
			line[0] = MAX_EXPOSURE_LINE;
		}

		if (line[1] > MAX_EXPOSURE_LINE) {
			DBG_ERR("max line overflow \r\n");
			line[1] = MAX_EXPOSURE_LINE;
		}

		if (line[1] > temp_max_short_line) {
			line[1] = temp_max_short_line;
		}

		if (line[0] > (sensor_vd - line[1] - NON_HDR_EXPOSURE_LINE)) {
			line[0] = sensor_vd - line[1] - NON_HDR_EXPOSURE_LINE;
		}
		compensation_ratio[id][0] = 100 * temp_line[0] / line[0];
		compensation_ratio[id][1] = 100 * temp_line[1] / line[1];

		mawb_l_g = 1024 * (line[0] * 100 + 75) / (line[0] * 100 + 25);
		mawb_s_g = (1024 * (line[0] * 100 + 25) / (line[0] * 100 + 75)) * (line[1] * 100 + 75) / (line[1] * 100 + 25);

		// set exposure line to sensor
		cmd = sensor_builtin_set_cmd_info(0x3501, 1, (line[0] >> 8) & 0xFF, 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3502, 1, line[0] & 0xFF , 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3511, 1, (line[1] >> 8) & 0xFF, 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x3512, 1, line[1] & 0xFF , 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		//set MAWB
		cmd = sensor_builtin_set_cmd_info(0x5102, 1, (mawb_l_g >> 8) & 0xFF, 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x5103, 1, mawb_l_g & 0xFF , 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x5142, 1, (mawb_s_g >> 8) & 0xFF, 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
		cmd = sensor_builtin_set_cmd_info(0x5143, 1, mawb_s_g & 0xFF , 0);
		sensor_builtin_write_i2c_2b1b(id, &cmd);
	}

}
#endif
