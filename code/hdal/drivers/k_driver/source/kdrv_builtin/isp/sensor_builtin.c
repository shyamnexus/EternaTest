#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include "kwrap/error_no.h"
#else
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "plat/kdrv_i2c.h"
#endif

#include "kwrap/debug.h"
#include "sensor_builtin.h"

#if defined(__KERNEL__)
#define I2C_NEW_DEVICE(x, y) i2c_new_client_device((x), (y))
#else
#define I2C_NEW_DEVICE(x, y) i2c_new_device((x), (y))
#endif

#if (NVT_FAST_ISP_FLOW)
static SENSOR_BUILTIN_DFT_PARAM sensor_dft_param[2];

//=============================================================================
// define
//=============================================================================
#define SENSOR_BUILTIN_I2C_NAME_1 "sensor_builtin_1"
#define SENSOR_BUILTIN_I2C_ADDR_1 0x0
#define SENSOR_BUILTIN_I2C_NAME_2 "sensor_builtin_2"
#define SENSOR_BUILTIN_I2C_ADDR_2 0x0
#define SENSOR_BUILTIN_I2C_NAME_3 "sensor_builtin_3"
#define SENSOR_BUILTIN_I2C_ADDR_3 0x0
#define SENSOR_BUILTIN_I2C_NAME_4 "sensor_builtin_4"
#define SENSOR_BUILTIN_I2C_ADDR_4 0x0
#define SENSOR_BUILTIN_I2C_NAME_5 "sensor_builtin_5"
#define SENSOR_BUILTIN_I2C_ADDR_5 0x0


//=============================================================================
// global
//=============================================================================
static struct i2c_board_info sensor_builtin_i2c_device[ISP_BUILTIN_ID_MAX_NUM] = {
	{
		.type = SENSOR_BUILTIN_I2C_NAME_1,
		.addr = SENSOR_BUILTIN_I2C_ADDR_1
	},
	{
		.type = SENSOR_BUILTIN_I2C_NAME_2,
		.addr = SENSOR_BUILTIN_I2C_ADDR_2
	},
	{
		.type = SENSOR_BUILTIN_I2C_NAME_3,
		.addr = SENSOR_BUILTIN_I2C_ADDR_3
	},
		{
		.type = SENSOR_BUILTIN_I2C_NAME_4,
		.addr = SENSOR_BUILTIN_I2C_ADDR_4
	},
	{
		.type = SENSOR_BUILTIN_I2C_NAME_5,
		.addr = SENSOR_BUILTIN_I2C_ADDR_5
	}
};

static SENSOR_BUILTIN_I2C_INFO *sensor_builtin_i2c_info[ISP_BUILTIN_ID_MAX_NUM];
static const struct i2c_device_id sensor_builtin_sen_i2c_id[][ISP_BUILTIN_ID_MAX_NUM] = {
	{
		{ SENSOR_BUILTIN_I2C_NAME_1, 0 },
		{ }
	},
	{
		{ SENSOR_BUILTIN_I2C_NAME_2, 0 },
		{ }
	},
	{
		{ SENSOR_BUILTIN_I2C_NAME_3, 0 },
		{ }
	},
	{
		{ SENSOR_BUILTIN_I2C_NAME_4, 0 },
		{ }
	},
	{
		{ SENSOR_BUILTIN_I2C_NAME_5, 0 },
		{ }
	}
};
//=============================================================================
// function declaration
//=============================================================================

//=============================================================================
// internal functions
//=============================================================================
static INT sensor_builtin_i2c_probe_1(struct i2c_client *client, const struct i2c_device_id *id)
{
	if (!(sensor_builtin_i2c_info[0] = MALLOC(sizeof(SENSOR_BUILTIN_I2C_INFO)))) {
		DBG_DUMP(" malloc fail. \r\n");
		return -1;
	}

	sensor_builtin_i2c_info[0]->iic_client  = client;
	sensor_builtin_i2c_info[0]->iic_adapter = client->adapter;

	i2c_set_clientdata(client, sensor_builtin_i2c_info[0]);

	return 0;
}

static INT sensor_builtin_i2c_remove_1(struct i2c_client *client)
{
	FREE(sensor_builtin_i2c_info[0]);
	sensor_builtin_i2c_info[0] = NULL;
	return 0;
}

static INT sensor_builtin_i2c_probe_2(struct i2c_client *client, const struct i2c_device_id *id)
{
	if (!(sensor_builtin_i2c_info[1] = MALLOC(sizeof(SENSOR_BUILTIN_I2C_INFO)))) {
		DBG_DUMP(" malloc fail. \r\n");
		return -1;
	}

	sensor_builtin_i2c_info[1]->iic_client  = client;
	sensor_builtin_i2c_info[1]->iic_adapter = client->adapter;

	i2c_set_clientdata(client, sensor_builtin_i2c_info[1]);

	return 0;
}

static INT sensor_builtin_i2c_remove_2(struct i2c_client *client)
{
	FREE(sensor_builtin_i2c_info[1]);
	sensor_builtin_i2c_info[1] = NULL;
	return 0;
}

static INT sensor_builtin_i2c_probe_3(struct i2c_client *client, const struct i2c_device_id *id)
{
	if (!(sensor_builtin_i2c_info[2] = MALLOC(sizeof(SENSOR_BUILTIN_I2C_INFO)))) {
		DBG_DUMP(" malloc fail. \r\n");
		return -1;
	}

	sensor_builtin_i2c_info[2]->iic_client  = client;
	sensor_builtin_i2c_info[2]->iic_adapter = client->adapter;

	i2c_set_clientdata(client, sensor_builtin_i2c_info[2]);

	return 0;
}

static INT sensor_builtin_i2c_remove_3(struct i2c_client *client)
{
	FREE(sensor_builtin_i2c_info[2]);
	sensor_builtin_i2c_info[2] = NULL;
	return 0;
}

static INT sensor_builtin_i2c_probe_4(struct i2c_client *client, const struct i2c_device_id *id)
{
	if (!(sensor_builtin_i2c_info[3] = MALLOC(sizeof(SENSOR_BUILTIN_I2C_INFO)))) {
		DBG_DUMP(" malloc fail. \r\n");
		return -1;
	}

	sensor_builtin_i2c_info[3]->iic_client  = client;
	sensor_builtin_i2c_info[3]->iic_adapter = client->adapter;

	i2c_set_clientdata(client, sensor_builtin_i2c_info[3]);

	return 0;
}

static INT sensor_builtin_i2c_remove_4(struct i2c_client *client)
{
	FREE(sensor_builtin_i2c_info[3]);
	sensor_builtin_i2c_info[3] = NULL;
	return 0;
}

static INT sensor_builtin_i2c_probe_5(struct i2c_client *client, const struct i2c_device_id *id)
{
	if (!(sensor_builtin_i2c_info[4] = MALLOC(sizeof(SENSOR_BUILTIN_I2C_INFO)))) {
		DBG_DUMP(" malloc fail. \r\n");
		return -1;
	}

	sensor_builtin_i2c_info[4]->iic_client  = client;
	sensor_builtin_i2c_info[4]->iic_adapter = client->adapter;

	i2c_set_clientdata(client, sensor_builtin_i2c_info[4]);

	return 0;
}

static INT sensor_builtin_i2c_remove_5(struct i2c_client *client)
{
	FREE(sensor_builtin_i2c_info[4]);
	sensor_builtin_i2c_info[4] = NULL;
	return 0;
}

static struct i2c_driver sensor_builtin_i2c_driver[ISP_BUILTIN_ID_MAX_NUM] = {
	{
		.driver = {
			.name  = SENSOR_BUILTIN_I2C_NAME_1,
			.owner = THIS_MODULE,
		},
		.probe    = sensor_builtin_i2c_probe_1,
		.remove   = sensor_builtin_i2c_remove_1,
		.id_table = sensor_builtin_sen_i2c_id[0]
	},
	{
		.driver = {
			.name  = SENSOR_BUILTIN_I2C_NAME_2,
			.owner = THIS_MODULE,
		},
		.probe    = sensor_builtin_i2c_probe_2,
		.remove   = sensor_builtin_i2c_remove_2,
		.id_table = sensor_builtin_sen_i2c_id[1]
	},
	{
		.driver = {
			.name  = SENSOR_BUILTIN_I2C_NAME_3,
			.owner = THIS_MODULE,
		},
		.probe    = sensor_builtin_i2c_probe_3,
		.remove   = sensor_builtin_i2c_remove_3,
		.id_table = sensor_builtin_sen_i2c_id[2]
	},
	{
		.driver = {
			.name  = SENSOR_BUILTIN_I2C_NAME_4,
			.owner = THIS_MODULE,
		},
		.probe    = sensor_builtin_i2c_probe_4,
		.remove   = sensor_builtin_i2c_remove_4,
		.id_table = sensor_builtin_sen_i2c_id[3]
	},
	{
		.driver = {
			.name  = SENSOR_BUILTIN_I2C_NAME_5,
			.owner = THIS_MODULE,
		},
		.probe    = sensor_builtin_i2c_probe_5,
		.remove   = sensor_builtin_i2c_remove_5,
		.id_table = sensor_builtin_sen_i2c_id[4]
	}
};

#if defined(__KERNEL__)
static INT32 sensor_builtin_transfer_i2c(UINT32 id, struct i2c_msg *msgs, INT32 num)
{
	if (unlikely(sensor_builtin_i2c_info[id]->iic_adapter == NULL)) {
		DBG_DUMP(" sensen_i2c_info->ii2c_adapter fail. \r\n");
		return -1;
	}

	if (unlikely(i2c_transfer(sensor_builtin_i2c_info[id]->iic_adapter, msgs, num) != num)) {
		DBG_DUMP(" i2c_transfer fail. \r\n");
		return -1;
	}

	return 0;
}
#endif

SENSOR_BUILTIN_I2C_CMD sensor_builtin_set_cmd_info(UINT32 addr, UINT32 data_length, UINT32 data0, UINT32 data1)
{
	SENSOR_BUILTIN_I2C_CMD cmd;

	cmd.addr = addr;
	cmd.data_len = data_length;
	cmd.data[0] = data0;
	cmd.data[1] = data1;
	return cmd;
}

UINT32 sensor_builtin_calc_log_2(UINT32 devider, UINT32 devident)
{
	UINT32 ratio;
	UINT32 idx, ret;
	static UINT32 log_tbl[20] = {104, 107, 112, 115, 119, 123, 128, 131, 137, 141, 147, 152, 157, 162, 168, 174, 181, 187, 193, 200};

	if (devident == 0) {
		return 1;
	}

	ret = 0;
	ratio = (devider * 100) / devident;
	while (ratio >= 200) {
		ratio /= 2;
		ret += 100;
	}

	for (idx = 0; idx < 20; idx++) {
		if (ratio < log_tbl[idx]) {
			break;
		}
	}

	return ret + idx * 5;
}

ER sensor_builtin_write_i2c_2b1b(UINT32 id, SENSOR_BUILTIN_I2C_CMD *cmd)
{
	struct i2c_msg msgs;
	unsigned char buf[3];

	buf[0]     = (cmd->addr >> 8) & 0xFF;
	buf[1]     = cmd->addr & 0xFF;
	buf[2]     = cmd->data[0] & 0xFF;
	msgs.addr = isp_builtin_get_i2c_addr(id);
	msgs.flags = 0;
	msgs.len   = 3;
	msgs.buf   = buf;

	#if defined(__KERNEL__)
	if (sensor_builtin_transfer_i2c(id, &msgs, 1) != 0) {
	#else
	if (sensor_builtin_set_transfer_i2c(id, &msgs, 1) != 0) {
	#endif
		DBG_DUMP("i2c write 0x%X,0x%X fail \r\n", cmd->addr, cmd->data[0]);
	}

	return E_OK;
}

ER sensor_builtin_read_i2c_2b1b(UINT32 id, SENSOR_BUILTIN_I2C_CMD *cmd)
{
	struct i2c_msg  msgs[2];
	unsigned char   tmp[2], tmp2[2];

	tmp[0]        = (cmd->addr >> 8) & 0xFF;
	tmp[1]        = cmd->addr & 0xFF;
	msgs[0].addr = isp_builtin_get_i2c_addr(id);
	msgs[1].addr = isp_builtin_get_i2c_addr(id);
	msgs[0].flags = 0;
	msgs[0].len   = 2;
	msgs[0].buf   = tmp;

	tmp2[0]       = 0;
	msgs[1].flags = 1;
	msgs[1].len   = 1;
	msgs[1].buf   = tmp2;

	#if defined(__KERNEL__)
	if (sensor_builtin_transfer_i2c(id, msgs, 2) != 0) {
	#else
	if (sensor_builtin_set_transfer_i2c(id, msgs, 2) != 0) {
	#endif
		DBG_DUMP("i2c read 0x%X fail \r\n", cmd->addr);
	}

	cmd->data[0] = tmp2[0];

	return E_OK;
}

ER sensor_builtin_write_i2c_1b1b(UINT32 id, SENSOR_BUILTIN_I2C_CMD *cmd)
{
	struct i2c_msg msgs;
	unsigned char buf[2];

	buf[0]     = cmd->addr & 0xFF;
	buf[1]     = cmd->data[0] & 0xFF;
	msgs.addr = isp_builtin_get_i2c_addr(id);
	msgs.flags = 0;
	msgs.len   = 2;
	msgs.buf   = buf;

	#if defined(__KERNEL__)
	if (sensor_builtin_transfer_i2c(id, &msgs, 1) != 0) {
	#else
	if (sensor_builtin_set_transfer_i2c(id, &msgs, 1) != 0) {
	#endif
		DBG_DUMP("i2c write 0x%X,0x%X fail \r\n", cmd->addr, cmd->data[0]);
	}

	return E_OK;
}

ER sensor_builtin_read_i2c_1b1b(UINT32 id, SENSOR_BUILTIN_I2C_CMD *cmd)
{
	struct i2c_msg  msgs[2];
	unsigned char   tmp[2], tmp2[2];

	tmp[0]        = cmd->addr & 0xFF;
	msgs[0].addr = isp_builtin_get_i2c_addr(id);
	msgs[0].flags = 0;
	msgs[0].len   = 1;
	msgs[0].buf   = tmp;

	tmp2[0]       = 0;
	msgs[1].addr = isp_builtin_get_i2c_addr(id);
	msgs[1].flags = 1;
	msgs[1].len   = 1;
	msgs[1].buf   = tmp2;

	#if defined(__KERNEL__)
	if (sensor_builtin_transfer_i2c(id, msgs, 2) != 0) {
	#else
	if (sensor_builtin_set_transfer_i2c(id, msgs, 2) != 0) {
	#endif
		DBG_DUMP("i2c read 0x%X fail \r\n", cmd->addr);
	}

	cmd->data[0] = tmp2[0];

	return E_OK;
}

//=============================================================================
// external functions
//=============================================================================
#if defined(__FREERTOS)
static ISP_BUILTIN_I2C_CB_FP sen_i2c_cb_fp = NULL;

void sensor_builtin_reg_i2c_cb(void *cb_fp)
{
	sen_i2c_cb_fp  = (ISP_BUILTIN_I2C_CB_FP)cb_fp;
}
#endif

UINT32 sensor_builtin_get_row_time(UINT32 id)
{
	return sensor_dft_param[id].row_time;
}

UINT32 sensor_builtin_get_min_expt_time(UINT32 id)
{
	switch (isp_builtin_get_sensor_name(id)) {
		default:
		case 8:
			return sensor_builtin_os04c10_get_min_expt_time(id, &sensor_dft_param[id]);
		break;
	}
}

void sensor_builtin_print_info(UINT32 id)
{
	CHAR sen_name[32];
	UINT32 min_expt_time;

	switch (isp_builtin_get_sensor_name(id)) {
		default:
		case 8:
			snprintf(sen_name, sizeof(sen_name) - 1, "os04c10");
			min_expt_time = sensor_builtin_os04c10_get_min_expt_time(id, &sensor_dft_param[id]);
		break;
	}

	DBG_DUMP("%s: row_time %d, vd %d, dft fps %d, frame_num %d, max gin %d, min expt %d \r\n",
																	sen_name,
																	sensor_dft_param[id].row_time,
																	sensor_dft_param[id].vd,
																	sensor_dft_param[id].dft_fps,
																	sensor_dft_param[id].frame_num,
																	sensor_dft_param[id].max_gain,
																	min_expt_time);
}

void sensor_builtin_set_gain(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl)
{
	switch (isp_builtin_get_sensor_name(id)) {
		default:
		case 8:
			sensor_builtin_os04c10_set_gain(id, sensor_ctrl, &sensor_dft_param[id]);
		break;
	}
}

void sensor_builtin_set_expt(UINT32 id, ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl)
{
	switch (isp_builtin_get_sensor_name(id)) {
		default:
		case 8:
			sensor_builtin_os04c10_set_expt(id, sensor_ctrl, &sensor_dft_param[id]);
		break;
	}
}

INT32 sensor_builtin_set_transfer_i2c(UINT32 id, void *msgs, INT32 num)
{
	#if defined(__KERNEL__)
	return sensor_builtin_transfer_i2c(id, msgs, num);
	#else
	if (sen_i2c_cb_fp != NULL) {
		return sen_i2c_cb_fp(id, msgs, num);
	} else {
		DBG_DUMP(" sen_i2c_cb_fp NULL, sensor name = %d \r\n", isp_builtin_get_sensor_name(id));
		return -1;
	}
	#endif
}

void sensor_builtin_set_dft_param(UINT32 id, SENSOR_BUILTIN_DFT_PARAM *param)
{
	memcpy(&sensor_dft_param[id], param, sizeof(SENSOR_BUILTIN_DFT_PARAM));
}

void sensor_builtin_init_i2c(UINT32 id)
{
	sensor_builtin_i2c_device[id].addr = isp_builtin_get_i2c_addr(id);

	if (I2C_NEW_DEVICE(i2c_get_adapter(isp_builtin_get_i2c_id(id)), &sensor_builtin_i2c_device[id]) == NULL) {
		DBG_DUMP(" i2c_new_device fail. \r\n");
		return;
	}

	// bind i2c client driver to i2c bus
	if (i2c_add_driver(&sensor_builtin_i2c_driver[id]) != 0) {
		DBG_DUMP(" i2c_add_driver fail. \r\n");
		return;
	}
}

void sensor_builtin_uninit_i2c(UINT32 id)
{
	i2c_unregister_device(sensor_builtin_i2c_info[id]->iic_client);
	i2c_del_driver(&sensor_builtin_i2c_driver[id]);
}

#endif

