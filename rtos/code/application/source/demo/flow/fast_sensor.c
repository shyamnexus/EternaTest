#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/type.h>
#include <kwrap/perf.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <shm_info.h>
#include "prjcfg.h"
#include "sys_fdt.h"
#include <kwrap/debug.h>
#include "vendor_videocapture.h"
#include "isp_api.h"
#include "sen_inc.h"
#include "kwrap/util.h"
#include <kdrv_builtin/fdtfast.h>
#include "isp_builtin.h"
#include "fast_sensor.h"
#include "adc.h"
#include "bridge_fourcc.h"
#include "bridge_mem.h"

#define AE_ADC DISABLE // for Photoresistor

/**
 * @brief sensor name id
 * refer to sensor_builtin_print_info() intsensor_builtin.c
 * 0: imx290,   1: f37,  2: os02k10,  3: os05a10,  4: f35
 * 5: gc4653,   6: f51,  7: sc450ai,  8: os04c10,  9: os04e10
 * 10: nt99436
 */

static FAST_SENSOR_DESC sensors1[] = {
/*       id  name               vdo size      fps   expt   if_type                   lane  shdr    delay  fast2a */
#if defined(_sen_os02k10_)
	{2,  "nvt_sen_os02k10", {1920, 1080}, 3000, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 4, SHDR_OFF, 0, {4, {1920, 1080}, 12000, 8333}},
#endif
#if defined(_sen_os05a10_)
	{3,  "nvt_sen_os05a10", {2592, 1944}, 3000, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 2, SHDR_OFF, 0, {2, {1920, 1080}, 12000, 8333}},
#endif
#if defined(_sen_sc450ai_)
	{7,  "nvt_sen_sc450ai", {2688, 1520}, 1500, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 4, SHDR_OFF, 0, {2, {640, 480},   12000, 8333}},
#endif
#if defined(_sen_os04c10_)
	{8,  "nvt_sen_os04c10", {2688, 1520}, 3000, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 2, SHDR_OFF, 0, {2, {1280, 720},  24000, 4000}},
	//{8,  "nvt_sen_os04c10", {2688, 1520}, 3000, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 2, SHDR_ON, 0, {2, {1280, 720},  12000, 8000}},
#endif
#if defined(_sen_os04e10_)
	#if defined(_RTOS2A_PREROLL_)
	{9,  "nvt_sen_os04e10", {2048, 2048}, 3000, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 2, SHDR_OFF, 0, {2, {2048, 2048}, 3000, 30000}},
	#else
	{9,  "nvt_sen_os04e10", {2048, 2048}, 3000, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 2, SHDR_OFF, 0, {2, {2048, 2048}, 12000, 8333}},
	#endif
#endif
#if defined(_sen_nt99436_)
	{10, "nvt_sen_nt99436", {2688, 1520}, 3000, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 2, SHDR_OFF, 0, {0}},
#endif
};

static FAST_SENSOR_DESC sensors2[] = {
/*       id  name               vdo size      fps   expt   if_type                   lane  shdr    delay  fast2a */
#if defined(_sen_os05a10_)
	{3,  "nvt_sen_os05a10", {2592, 1944}, 3000, 30000, HD_COMMON_VIDEO_IN_MIPI_CSI, 2, SHDR_OFF, 0, {2, {1920, 1080}, 12000, 8333}},
#endif
};


//for single HDR sensor
#define HDR_SEN1_VCAP_ID     0
#define HDR_SEN1_SUB_VCAP_ID 1
#define SEN1_SHDR_VCAP_MAP ((1<<HDR_SEN1_VCAP_ID)|(1<<HDR_SEN1_SUB_VCAP_ID))

//for dual HDR sensor
#define HDR_SEN2_VCAP_ID     3
#define HDR_SEN2_SUB_VCAP_ID 4
#define DUAL_SEN1_SHDR_VCAP_MAP SEN1_SHDR_VCAP_MAP
#define DUAL_SEN2_SHDR_VCAP_MAP ((1<<HDR_SEN2_VCAP_ID)|(1<<HDR_SEN2_SUB_VCAP_ID))

//for dual linear sensor
#define LINEAR_SEN2_VCAP_ID 1

#define SEN_OUT_FMT     HD_VIDEO_PXLFMT_RAW12
#define CLK_LANE_SEL    HD_VIDEOCAP_SEN_CLANE_CSI(0, 0);

static UINT32 isp_sensor_shdr_map_path = 0xFFFFFFFF;
static UINT32 isp_sensor_shdr_id_mask = 0x1;
static UINT32 isp_sensor_path_2 = 1;
static UINT32 isp_sensor_shdr_map_path_2 = 0xFFFFFFFF;
static UINT32 isp_sensor_shdr_id_mask_2 = 0x2;
static UINT32 isp_dgain = 128;
static UINT32 isp_rgain = 0;
static UINT32 isp_ggain = 0;
static UINT32 isp_bgain = 0;
static UINT32 isp_nr_lv = 100;
static UINT32 isp_3dnr_lv = 100;
static UINT32 isp_sharpness_lv = 100;
static UINT32 isp_saturation_lv = 100;
static UINT32 isp_contrast_lv = 100;
static UINT32 isp_brightness_lv = 100;
static UINT32 isp_night_mode = 0;

static UINT32 dtsi_fps, dtsi_expt, dtsi_gain, dtsi_sen_mode, dtsi_i2c_id, dtsi_i2c_addr, dtsi_frame_num, dtsi_row_time, dtsi_vd, dtsi_dft_fps, dtsi_max_gain;

int m_error_on_sensor = 0;

#if (PREROLL_2A == ENABLE)
typedef struct {
	UINT16 ae_expoline;           ///< The exposure line of AE
	UINT8 ae_gain_idx;            ///< The total gain index of AE
	UINT8 ae_hcg_gain_idx;        ///< The total hc gain index of AE
	UINT8 ae_a_gain_idx;          ///< The total A  gain index of AE
	UINT8 ae_d_gain_idx;          ///< The total D  gain index of AE
	UINT8 ae_hcg_en;              ///< The hc gain enable of sensor
	UINT8 dummy;
} PRISP_AE_RESULT;

static PRISP_AE_RESULT prae_result = {0};
#endif

#if (AE_ADC)
#define ADC_CH_PHOTORESISTOR         ADC_CHANNEL_0
#define ADC_READY_RETRY_CNT          (1000000/10) //1 sec
#define SENSOR_GAIN_MAX               15500 // for F37

#define AE_ADC_TABLE_X      3
#define AE_ADC_TABLE_Y      18

typedef struct _AE_ADC_CTRL_ {
	UINT32 enable;
	UINT32 adc_tbl_thr;
	UINT32 ir_mode_en;
	UINT32 ir_mode_thr;
	UINT32 sen_expt;
	UINT32 sen_gain;
	UINT32 isp_gain;
} AE_ADC_CTRL;

AE_ADC_CTRL ae_adc_ctrl = {
	0, 1000, 0, 50, 10000, 1000, 128
};

static UINT32 ae_adc_tbl[AE_ADC_TABLE_Y][AE_ADC_TABLE_X] = {
	{4096,   100,  1000},
	{4096,   100,  1000},
	{2018,   114,  1000},
	{1996,   240,  1000},
	{1980,   480,  1000},
	{1916,   945,  1000},
	{1306,  1892,  1000},
	{ 670,  3758,  1000},
	{ 340,  7478,  1000},
	{ 167, 10000,  1530},
	{  84, 20000,  1420},
	{  44, 30000,  2310},
	{  19, 30000,  4180},
	{   4, 30000,  7310},
	{   0, 30000, 14700},
	{   0, 30000, 14700},
	{   0, 30000, 14700},
	{   0, 30000, 14700},
};

static UINT32 adc_value = 0;

void ae_adc_get_exp(UINT32 adc_value, UINT32 *exptime, UINT32 *isogain, UINT32 *dgain)
{
	UINT32 idx0, idx1;
	UINT32 sen_expt0, sen_expt1, sen_gain0, sen_gain1;
	UINT32 adc_sen_expt, adc_sen_gain, adc_isp_gain;
	UINT32 adc_ev, ev0, ev1;

	for (idx1 = 1; idx1 < AE_ADC_TABLE_Y; idx1++) {
		if (adc_value > ae_adc_tbl[idx1][0]) {
			break;
		}
	}

	if (idx1 >= AE_ADC_TABLE_Y) {
		idx1 = (AE_ADC_TABLE_Y - 1);
	}

	idx0 = idx1 - 1;

	sen_expt0 = ae_adc_tbl[idx0][1];
	sen_gain0 = (ae_adc_tbl[idx0][2]);
	sen_expt1 = ae_adc_tbl[idx1][1];
	sen_gain1 = (ae_adc_tbl[idx1][2]);
	ev0 = (sen_expt0 * sen_gain0);
	ev1 = (sen_expt1 * sen_gain1);

	adc_ev = ((ev1 - ev0) * (adc_value - ae_adc_tbl[idx1][0])) / (ae_adc_tbl[idx0][0] - ae_adc_tbl[idx1][0]) + ev0;

	adc_sen_expt = sen_expt0;
	adc_sen_gain = (adc_ev / sen_expt0);

	adc_isp_gain = 128;

	if (adc_sen_gain > SENSOR_GAIN_MAX) {
		adc_isp_gain = ((adc_sen_gain << 7) / SENSOR_GAIN_MAX);
		adc_sen_gain = SENSOR_GAIN_MAX;
	}

	if (adc_isp_gain > 1023) {
		adc_isp_gain = 1023;
	}

	*exptime = adc_sen_expt;
	*isogain = adc_sen_gain;
	*dgain = adc_isp_gain;
	//DBG_ERR("idx = {%d, %d}, adc = %d, fast open preset exp = %d, %d\r\n", idx0, idx1, adc_value, *exptime, *isogain);
}

static void adc_dtsi_load(UINT32 adc_value)
{
#if defined(FASTBOOT20)
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
#else
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
#endif
	int len;
	int nodeoffset;
	const void *nodep;
	UINT32 size, ix, iy;
	UINT32 *p_ae_adc_tbl;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return;
	}

	nodeoffset = fdt_path_offset(p_fdt, "/fastboot/ae_adc_table");
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for %s = %d \n", "/fastboot/ae_adc_table", nodeoffset);
		return;
	}

	nodep = fdt_getprop(p_fdt, nodeoffset, "valid", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access /fastboot/ae_adc/valid. \n");
		return;
	}
	ae_adc_ctrl.enable = be32_to_cpu(*(UINT32 *)nodep);

	nodep = fdt_getprop(p_fdt, nodeoffset, "def_sen_expt", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access /fastboot/ae_adc/def_sen_expt. \n");
		return;
	}
	ae_adc_ctrl.sen_expt = be32_to_cpu(*(UINT32 *)nodep);

	nodep = fdt_getprop(p_fdt, nodeoffset, "def_sen_gain", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access /fastboot/ae_adc/def_sen_gain. \n");
		return;
	}
	ae_adc_ctrl.sen_gain = be32_to_cpu(*(UINT32 *)nodep);

	nodep = fdt_getprop(p_fdt, nodeoffset, "def_isp_gain", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access /fastboot/ae_adc/def_isp_gain. \n");
		return;
	}
	ae_adc_ctrl.isp_gain = be32_to_cpu(*(UINT32 *)nodep);

	nodep = fdt_getprop(p_fdt, nodeoffset, "adc_tbl_thr", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access /fastboot/ae_adc/adc_tbl_thr. \n");
		return;
	}
	ae_adc_ctrl.adc_tbl_thr = be32_to_cpu(*(UINT32 *)nodep);

	nodep = fdt_getprop(p_fdt, nodeoffset, "ir_mode_en", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access /fastboot/ae_adc/ir_mode_en. \n");
		return;
	}
	ae_adc_ctrl.ir_mode_en = be32_to_cpu(*(UINT32 *)nodep);

	nodep = fdt_getprop(p_fdt, nodeoffset, "ir_mode_thr", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access /fastboot/ae_adc/ir_mode_thr. \n");
		return;
	}
	ae_adc_ctrl.ir_mode_thr = be32_to_cpu(*(UINT32 *)nodep);

#if 0
	DBG_ERR("\r\n");
	DBG_ERR("adc_en = %d, tbl_thr = %d, ir_en = %d, ir_thr = %d, adc_value = %d\r\n", ae_adc_ctrl.enable, ae_adc_ctrl.adc_tbl_thr, ae_adc_ctrl.ir_mode_en, ae_adc_ctrl.ir_mode_thr, adc_value);
	DBG_ERR("def_sen_expt = %d, def_sen_gain = %d, def_isp_gain = %d\r\n", ae_adc_ctrl.sen_expt, ae_adc_ctrl.sen_gain, ae_adc_ctrl.isp_gain);
	DBG_ERR("\r\n");
#endif
	nodep = fdt_getprop(p_fdt, nodeoffset, "size", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access /fastboot/ae_adc/size. \n");
		return;
	}
	size = be32_to_cpu(*(UINT32 *)nodep);

	if (adc_value > ae_adc_ctrl.adc_tbl_thr) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "adc_expt_gain_1", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access /fastboot/ae_adc/adc_expt_gain. \n");
			return;
		}
	} else {
		nodep = fdt_getprop(p_fdt, nodeoffset, "adc_expt_gain_0", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access /fastboot/ae_adc/adc_expt_gain. \n");
			return;
		}
	}

	if (size != (UINT32)len) {
		DBG_ERR("ae adc table size incorrect. %d %d \n", size, (UINT32)len);
		return;
	}

	p_ae_adc_tbl = (UINT32 *)nodep;
	for (iy = 0; iy < AE_ADC_TABLE_Y; iy++) {
		for (ix = 0; ix < AE_ADC_TABLE_X; ix++) {
			// NOTE: update adc table here
			ae_adc_tbl[iy][ix] = be32_to_cpu(p_ae_adc_tbl[iy * AE_ADC_TABLE_X + ix]);
		}

		//DBG_ERR("ae_adc_tbl = %4d %6d %6d \n", ae_adc_tbl[iy][0], ae_adc_tbl[iy][1], ae_adc_tbl[iy][2]);
	}
}

UINT32 adc_get_value(UINT32 times, UINT32 delay)
{
	UINT32 i, retry = 0;
	UINT32 adc_avg, adc_cnt;
	UINT32 adc_mV;

	if (adc_open(ADC_CH_PHOTORESISTOR)) {
		printf("Can't open ADC channel for photoresistor\r\n");
		return 0xFFFFFFFF;
	}

	adc_setChConfig(ADC_CH_PHOTORESISTOR, ADC_CH_CONFIG_ID_SAMPLE_FREQ, 10000); //10K Hz, sample once about 100 us for CONTINUOUS mode
	//adc_setChConfig(ADC_CH_PHOTORESISTOR, ADC_CH_CONFIG_ID_SAMPLE_MODE, ADC_CH_SAMPLEMODE_ONESHOT);
	adc_setChConfig(ADC_CH_PHOTORESISTOR, ADC_CH_CONFIG_ID_SAMPLE_MODE, ADC_CH_SAMPLEMODE_CONTINUOUS);
	adc_setChConfig(ADC_CH_PHOTORESISTOR, ADC_CH_CONFIG_ID_INTEN, FALSE);
	adc_setEnable(TRUE);
	adc_triggerOneShot(ADC_CH_PHOTORESISTOR);
	while (FALSE == adc_isDataReady(ADC_CH_PHOTORESISTOR) && retry < ADC_READY_RETRY_CNT) {
		vos_util_delay_us_polling(10);
		retry++;
	}

	adc_avg = 0;
	adc_cnt = 0;

	for (i = 0; i < times; i++) {
		adc_mV = adc_readVoltage(ADC_CH_PHOTORESISTOR);
		adc_avg += adc_mV;
		//DBG_ERR("photoresistor voltage = %d\r\n", (unsigned int)adc_mV);
		vos_util_delay_us_polling(delay);
		adc_cnt++;
	}

	if (adc_cnt < 1) {
		adc_cnt = 1;
	}

	adc_avg /= adc_cnt;

	adc_close(ADC_CH_PHOTORESISTOR);

	return adc_avg;
}

static void adc_get_preset(UINT32 adc_value, UINT32 *p_exp_time, UINT32 *p_gain_ratio)
{
	UINT32 adc_sen_expt, adc_sen_gain, adc_dgain;

	ae_adc_get_exp(adc_value, &adc_sen_expt, &adc_sen_gain, &adc_dgain);

	*p_exp_time = adc_sen_expt;
	*p_gain_ratio = adc_sen_gain;
	isp_dgain = adc_dgain;

	//DBG_ERR("adc_mV = %d, adc_exp =========== %d, %d, %d\r\n", (unsigned int)adc_value, (unsigned int)adc_sen_expt, (unsigned int)adc_sen_gain, (unsigned int)adc_dgain);
}
#endif

#if (PREROLL_2A == ENABLE)
UINT32 fast_gain_index_to_ratio(UINT32 gain_idx)
{
	UINT32 ratio = 1, gain_int, gain_frac, i;

	gain_int = ((gain_idx >> 4) & 0x0F);
	gain_frac = (gain_idx & 0x0F);

	for (i = 0; i < gain_int; i++) {
		ratio = (ratio << 1);
	}

	return ((ratio * 1000) + ((ratio * gain_frac * 1000) >> 4));
}
#endif

static FAST_SENSOR_DESC *fast_sensor_get_desc(int idx)
{
	char driver_name[64] = {0};
	FAST_SENSOR_DESC *p_sensors = NULL;
	int i, n;
	switch (idx) {
	case 0:
		snprintf(driver_name, 63, "nvt_%s", _SEN1_NAME_);
		n = sizeof(sensors1) / sizeof(FAST_SENSOR_DESC);
		p_sensors = sensors1;
		break;
	case 1:
		snprintf(driver_name, 63, "nvt_%s", _SEN2_NAME_);
		n = sizeof(sensors2) / sizeof(FAST_SENSOR_DESC);
		p_sensors = sensors2;
		break;
	default:
		return NULL;
	}

	for (i = 0; i < n; i++) {
		if (strncmp(driver_name, p_sensors[i].name, 64) == 0) {
			return &p_sensors[i];
		}
	}
	return NULL;
}

void fast_open_sensor(void)
{
	int i;
	VENDOR_VIDEOCAP_FAST_OPEN_SENSOR cap_cfg = {0};
	FAST_SENSOR_DESC *p_desc = fast_sensor_get_desc(0);

	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[0]\n");
		return;
	}

#if 0
	if (p_desc->crop_size.w && p_desc->crop_size.h) {
		VENDOR_VIDEOCAP_FAST_CONFIG sen_cfg = {0};
		sen_cfg.in_crop.mode = HD_CROP_ON;
		sen_cfg.in_crop.win.rect.x = 0;
		sen_cfg.in_crop.win.rect.y = 0;
		sen_cfg.in_crop.win.rect.w = p_desc->crop_size.w;
		sen_cfg.in_crop.win.rect.h = p_desc->crop_size.h;
		vendor_videocap_set_lite(0, VENDOR_VIDEOCAP_PARAM_FAST_CONFIG, &sen_cfg);
	}
#endif
	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, p_desc->name);
	cap_cfg.sen_cfg.sen_dev.if_type = p_desc->if_type;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0;  //use @0 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = CLK_LANE_SEL;

	for (i = 0; i < (int)p_desc->data_lane; i++) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = i;
	}
	for (; i < HD_VIDEOCAP_SEN_SER_MAX_DATALANE; i++) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = HD_VIDEOCAP_SEN_IGNORE;
	}

	if (p_desc->shdr) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, SEN1_SHDR_VCAP_MAP);
		isp_sensor_shdr_map_path = HDR_SEN1_SUB_VCAP_ID;
		isp_sensor_shdr_id_mask = SEN1_SHDR_VCAP_MAP;
		cap_cfg.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
	} else {
		isp_sensor_shdr_map_path = 0xFFFFFFFF;
		isp_sensor_shdr_id_mask = HD_VIDEOCAP_0;
		cap_cfg.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
	}
	cap_cfg.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
	cap_cfg.pxlfmt = SEN_OUT_FMT;
#if (FASTBOOT_2A == ENABLE)
	cap_cfg.dim.w = p_desc->fast2a.vdo_size.w;
	cap_cfg.dim.h = p_desc->fast2a.vdo_size.h;
	cap_cfg.data_lane = p_desc->fast2a.data_lane;
	cap_cfg.frc = HD_VIDEO_FRC_RATIO(p_desc->fast2a.fps / 100, 1);
#else
	cap_cfg.dim.w = p_desc->vdo_size.w;
	cap_cfg.dim.h = p_desc->vdo_size.h;
	cap_cfg.data_lane = p_desc->data_lane;
	cap_cfg.frc = HD_VIDEO_FRC_RATIO(p_desc->fps / 100, 1);
#endif

#if (AE_ADC)
	cap_cfg.ae_preset.enable = TRUE;
	ae_adc_ctrl.enable = ENABLE;

	adc_value = adc_get_value(10, 10);

	adc_dtsi_load(adc_value);

	if (ae_adc_ctrl.ir_mode_en) {
		if (adc_value > ae_adc_ctrl.ir_mode_thr) {
			isp_night_mode = 0;
		} else {
			isp_night_mode = 1;
		}
	} else {
		isp_night_mode = 0;
	}

	adc_get_preset(adc_value, &cap_cfg.ae_preset.exp_time, &cap_cfg.ae_preset.gain_ratio);
	printf("[1st light on sensor] adc_en = %d, adc_value = %d, expt = %d, gain = %d, isp_gain = %d, ir_thr = %d, night_mode = %d \n", (unsigned int)ae_adc_ctrl.enable, (unsigned int)adc_value, (unsigned int)cap_cfg.ae_preset.exp_time, (unsigned int)cap_cfg.ae_preset.gain_ratio, (unsigned int)isp_dgain, (unsigned int)ae_adc_ctrl.ir_mode_thr, (unsigned int)isp_night_mode);

#else
	cap_cfg.ae_preset.enable = TRUE;
#if (FASTBOOT_2A == ENABLE)
	cap_cfg.ae_preset.exp_time = p_desc->fast2a.expt_max;
#else
	cap_cfg.ae_preset.exp_time = p_desc->expt_max;
#endif
	cap_cfg.ae_preset.gain_ratio = 1000;


	#if (PREROLL_2A == ENABLE)
	// NOTE: get ae result from prisp
	{
		#define INREG32(x)						(*((volatile UINT32*)(x)))
		#define IOADDR_PRTCM_REG_BASE			(0xF0900000)

		UINT32 ae_result[2];

		ae_result[0] = INREG32(IOADDR_PRTCM_REG_BASE + 0);
		ae_result[1] = INREG32(IOADDR_PRTCM_REG_BASE + 4);
		prae_result.ae_expoline =     (ae_result[0] >> 16) & 0xFFFF;
		prae_result.ae_gain_idx =     (ae_result[0] >> 8) & 0xFF;
		prae_result.ae_hcg_gain_idx = ae_result[0] & 0xFF;
		prae_result.ae_a_gain_idx =   (ae_result[1] >> 24) & 0xFF;
		prae_result.ae_d_gain_idx =   (ae_result[1] >> 16) & 0xFF;
		prae_result.ae_hcg_en =       (ae_result[1] >> 8) & 0xFF;
		DBG_DUMP("preroll: (0x%08X, 0x%08X) exp %d, gain 0x%X, hcg gain 0x%X, a gain 0x%X, d gain 0x%X, hcg en %d \n",
														ae_result[0], ae_result[1],
														prae_result.ae_expoline, prae_result.ae_gain_idx,
														prae_result.ae_hcg_gain_idx, prae_result.ae_a_gain_idx,
														prae_result.ae_d_gain_idx, prae_result.ae_hcg_en);
	}
	// NOTE: transfer ae result to preset ae
	{
		#define OS04E10_ROW_TIME      257 // 2776/10.8M = 257us
		#define OS04E10_BINNING_RATIO 4
		cap_cfg.ae_preset.exp_time = prae_result.ae_expoline * OS04E10_ROW_TIME * OS04E10_BINNING_RATIO;
		cap_cfg.ae_preset.gain_ratio = fast_gain_index_to_ratio(prae_result.ae_gain_idx);
	}
	#endif

	printf("[1st light on sensor] expt = %d, gain = %d \n", (unsigned int)cap_cfg.ae_preset.exp_time, (unsigned int)cap_cfg.ae_preset.gain_ratio);
#endif

	cap_cfg.wait_det_ms = p_desc->wait_det_ms;

	/* vendor_videocap_set_lite(0, VENDOR_VIDEOCAP_PARAM_FAST_OPEN_SENSOR, &cap_cfg); */
	HD_RESULT result;
	result = vendor_videocap_set_lite(0, VENDOR_VIDEOCAP_PARAM_FAST_OPEN_SENSOR, &cap_cfg);
	if (result != HD_OK) {
		printf("fast_open_sensor(0x%x)\r\n", (unsigned int)result);
		m_error_on_sensor |= 0x1;
	}
}

//this sample is base on dual os02k10
void fast_open_sensor2(void)
{
	int i;
	VENDOR_VIDEOCAP_FAST_OPEN_SENSOR cap_cfg = {0};
	FAST_SENSOR_DESC *p_desc = fast_sensor_get_desc(1);

	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[1]\n");
		return;
	}

#if 0
	if (p_desc->crop_size.w && p_desc->crop_size.h) {
		VENDOR_VIDEOCAP_FAST_CONFIG sen_cfg = {0};
		sen_cfg.in_crop.mode = HD_CROP_ON;
		sen_cfg.in_crop.win.rect.x = 0;
		sen_cfg.in_crop.win.rect.y = 0;
		sen_cfg.in_crop.win.rect.w = p_desc->crop_size.w;
		sen_cfg.in_crop.win.rect.h = p_desc->crop_size.h;
		vendor_videocap_set_lite(1, VENDOR_VIDEOCAP_PARAM_FAST_CONFIG, &sen_cfg);
	}
#endif
	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN - 1, p_desc->name);
	cap_cfg.sen_cfg.sen_dev.if_type = p_desc->if_type;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 1;  //use @1 in peri-dev.dtsi
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_CSI(1, 0);

	for (i = 0; i < (int)p_desc->data_lane; i++) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = i;
	}
	for (; i < HD_VIDEOCAP_SEN_SER_MAX_DATALANE; i++) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[i] = HD_VIDEOCAP_SEN_IGNORE;
	}

	if (p_desc->shdr) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR2, DUAL_SEN2_SHDR_VCAP_MAP);
		isp_sensor_path_2 = HDR_SEN2_VCAP_ID;
		isp_sensor_shdr_map_path_2 = HDR_SEN2_SUB_VCAP_ID;
		isp_sensor_shdr_id_mask_2 = DUAL_SEN2_SHDR_VCAP_MAP;
		cap_cfg.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
	} else {
		isp_sensor_path_2 = LINEAR_SEN2_VCAP_ID;
		isp_sensor_shdr_map_path_2 = 0xFFFFFFFF;
		isp_sensor_shdr_id_mask_2 = (1 << LINEAR_SEN2_VCAP_ID);
		cap_cfg.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
	}
	cap_cfg.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
	cap_cfg.pxlfmt = SEN_OUT_FMT;
#if (FASTBOOT_2A == ENABLE)
	cap_cfg.dim.w = p_desc->fast2a.vdo_size.w;
	cap_cfg.dim.h = p_desc->fast2a.vdo_size.h;
	cap_cfg.data_lane = p_desc->fast2a.data_lane;
	cap_cfg.frc = HD_VIDEO_FRC_RATIO(p_desc->fast2a.fps / 100, 1);
#else
	cap_cfg.dim.w = p_desc->vdo_size.w;
	cap_cfg.dim.h = p_desc->vdo_size.h;
	cap_cfg.data_lane = p_desc->data_lane;
	cap_cfg.frc = HD_VIDEO_FRC_RATIO(p_desc->fps / 100, 1);
#endif

	cap_cfg.wait_det_ms = p_desc->wait_det_ms;

	/* vendor_videocap_set_lite(1, VENDOR_VIDEOCAP_PARAM_FAST_OPEN_SENSOR, &cap_cfg); */
	HD_RESULT result;
	result = vendor_videocap_set_lite(1, VENDOR_VIDEOCAP_PARAM_FAST_OPEN_SENSOR, &cap_cfg);
	if (result != HD_OK) {
		printf("fast_open_sensor2(0x%x)\r\n", (unsigned int)result);
		m_error_on_sensor |= 0x2;
	}
}

void fast_chgmode_sensor(void)
{
	VENDOR_VIDEOCAP_FAST_OPEN_SENSOR cap_cfg = {0};
	FAST_SENSOR_DESC *p_desc = fast_sensor_get_desc(0);
	ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl_temp;

	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[0]\n");
		return;
	}

#if defined(_sen_os04c10_)
	if (p_desc->shdr) {
		// means mode 6, shdr always use this mode on this sensor
		cap_cfg.sen_mode = 5;
	} else {
		// means mode 8, a special mode with less regs for binning mode -> normal mode
		//cap_cfg.sen_mode = 7;
		cap_cfg.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
	}
#else
	cap_cfg.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
#endif
	if (p_desc->shdr) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, SEN1_SHDR_VCAP_MAP);
		cap_cfg.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
	} else {
		cap_cfg.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
	}

	cap_cfg.dim.w = p_desc->vdo_size.w;
	cap_cfg.dim.h = p_desc->vdo_size.h;
	cap_cfg.pxlfmt = SEN_OUT_FMT;
	cap_cfg.data_lane = p_desc->data_lane;

	cap_cfg.frc = HD_VIDEO_FRC_RATIO(p_desc->fps / 100, 1);

	cap_cfg.ae_preset.enable = TRUE;
	sensor_ctrl_temp = isp_builtin_get_sensor_expt(0);
	cap_cfg.ae_preset.exp_time = sensor_ctrl_temp->exp_time[0];
	sensor_ctrl_temp = isp_builtin_get_sensor_gain(0);
	cap_cfg.ae_preset.gain_ratio = sensor_ctrl_temp->gain_ratio[0];
	printf("[2nd light on sensor] expt = %d, gain = %d \n", (unsigned int)cap_cfg.ae_preset.exp_time, (unsigned int)cap_cfg.ae_preset.gain_ratio);

	vendor_videocap_set_lite(0, VENDOR_VIDEOCAP_PARAM_FAST_CHANGE_MODE, &cap_cfg);
}

void fast_chgmode_sensor2(void)
{
	VENDOR_VIDEOCAP_FAST_OPEN_SENSOR cap_cfg = {0};
	FAST_SENSOR_DESC *p_desc = fast_sensor_get_desc(1);
	ISP_BUILTIN_SENSOR_CTRL *sensor_ctrl_temp;

	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[1]\n");
		return;
	}

	cap_cfg.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
	cap_cfg.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO;
	cap_cfg.dim.w = p_desc->vdo_size.w;
	cap_cfg.dim.h = p_desc->vdo_size.h;
	cap_cfg.pxlfmt = SEN_OUT_FMT;
	cap_cfg.data_lane = p_desc->data_lane;

	cap_cfg.frc = HD_VIDEO_FRC_RATIO(p_desc->fps / 100, 1);

	cap_cfg.ae_preset.enable = TRUE;
	sensor_ctrl_temp = isp_builtin_get_sensor_expt(1);
	cap_cfg.ae_preset.exp_time = sensor_ctrl_temp->exp_time[0];
	sensor_ctrl_temp = isp_builtin_get_sensor_gain(1);
	cap_cfg.ae_preset.gain_ratio = sensor_ctrl_temp->gain_ratio[0];
	printf("[2nd light on sensor] expt2 = %d, gain2 = %d \n", (unsigned int)cap_cfg.ae_preset.exp_time, (unsigned int)cap_cfg.ae_preset.gain_ratio);

	vendor_videocap_set_lite(1, VENDOR_VIDEOCAP_PARAM_FAST_CHANGE_MODE, &cap_cfg);
}

static int fastboot_get_version(void)
{
#if defined(FASTBOOT20)
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
#else
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
#endif
	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return 0;
	}

	// get linux space
	nodeoffset = fdt_path_offset(p_fdt, "/fastboot");
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", "/fastboot", nodeoffset);
		return 0;
	}

	nodep = fdt_getprop(p_fdt, nodeoffset, "generation", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access enable.\n");
		return 0;
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		return be32_to_cpu(p_data[0]);
	}
	return 0;
}

int bridge_mem_plan_sensor1(void)
{
#if defined(FASTBOOT20)
	ISP_SENSOR_CTRL sensor_ctrl_expt = {0}, sensor_ctrl_gain = {0};
	UINT32 sensor_name = 0, sensor_i2c_id = 0, sensor_i2c_addr = 0;
	UINT32 cur_mode = 0, row_time = 0;
	CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic = {0};
	ISP_BUILTIN_CGAIN *cgain = NULL;
	FAST_SENSOR_DESC *p_desc = fast_sensor_get_desc(0);

	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[0]\n");
		return -1;
	}

	// start to plan
	// add iq tag with value
	sensor_name = p_desc->id;
	#if (SENSOR_DRIVER_READY)
	SEN_GET_EXPT_GAIN_I2C(_SEN1_, 0, &sensor_ctrl_expt, &sensor_ctrl_gain, &sensor_i2c_id,  &sensor_i2c_addr);
	SEN_GET_CUR_MODE(_SEN1_, 0, &cur_mode);
	SEN_GET_ROW_TIME(_SEN1_, 0, &row_time);
	SEN_GET_MODE_PARAM(_SEN1_, 0, &mode_basic);
	#endif

	// NOTE: preroll
	#if defined(_RTOS2A_PREROLL_)
	{
		ISP_BUILTIN_SENSOR_CTRL *expt = {0};
		ISP_BUILTIN_SENSOR_CTRL *gain = {0};

		DBG_ERR("preroll bridge \n");

		expt = isp_builtin_get_sensor_expt(0);
		sensor_ctrl_expt.exp_time[0] = expt->exp_time[0];

		gain = isp_builtin_get_sensor_gain(0);
		sensor_ctrl_gain.gain_ratio[0] = gain->gain_ratio[0];
	}
	#endif

	if (sensor_ctrl_expt.exp_time[0] == 0) {
		sensor_ctrl_expt.exp_time[0] = 10000;
	}
	if (sensor_ctrl_gain.gain_ratio[0] == 0) {
		sensor_ctrl_gain.gain_ratio[0] = 1000;
	}

	isp_dgain = isp_builtin_get_dgain(1);
	cgain = isp_builtin_get_cgain(0);
	isp_rgain = cgain->r;
	isp_ggain = cgain->g;
	isp_bgain = cgain->b;

	printf("fastboot_version = %d\r\n", fastboot_get_version());

	bridge_mem_modify_tag(SENSOR_PRESET_NAME, sensor_name);
	bridge_mem_add_tag(SENSOR_CHGMODE_FPS, p_desc->fps);
	bridge_mem_add_tag(SENSOR_PRESET_EXPT, sensor_ctrl_expt.exp_time[0]);
	bridge_mem_add_tag(SENSOR_PRESET_GAIN, sensor_ctrl_gain.gain_ratio[0]);
	bridge_mem_add_tag(SENSOR_EXPT_MAX, p_desc->expt_max);
	bridge_mem_add_tag(SENSOR_I2C_ID, sensor_i2c_id);
	bridge_mem_add_tag(SENSOR_I2C_ADDR, sensor_i2c_addr);
	bridge_mem_add_tag(SENSOR_CUR_MODE, cur_mode);
	bridge_mem_add_tag(SENSOR_ROW_TIME, row_time);
	bridge_mem_add_tag(SENSOR_VD, mode_basic.signal_info.vd_period);
	bridge_mem_add_tag(SENSOR_DFT_FPS, mode_basic.dft_fps);
	bridge_mem_add_tag(SENSOR_FRAME_NUM, mode_basic.frame_num);
	bridge_mem_add_tag(SENSOR_MAX_GAIN, mode_basic.gain.max);
	bridge_mem_add_tag(ISP_D_GAIN, isp_dgain);
	bridge_mem_add_tag(ISP_R_GAIN, isp_rgain);
	bridge_mem_add_tag(ISP_G_GAIN, isp_ggain);
	bridge_mem_add_tag(ISP_B_GAIN, isp_bgain);
	bridge_mem_add_tag(ISP_SHDR_ENABLE, p_desc->shdr);
	bridge_mem_add_tag(ISP_SHDR_PATH, isp_sensor_shdr_map_path);
	bridge_mem_add_tag(ISP_SHDR_MASK, isp_sensor_shdr_id_mask);
	bridge_mem_add_tag(ISP_NR_LV, isp_nr_lv);
	bridge_mem_add_tag(ISP_3DNR_LV, isp_3dnr_lv);
	bridge_mem_add_tag(ISP_SHARPNESS_LV, isp_sharpness_lv);
	bridge_mem_add_tag(ISP_SATURATION_LV, isp_saturation_lv);
	bridge_mem_add_tag(ISP_CONTRAST_LV, isp_contrast_lv);
	bridge_mem_add_tag(ISP_BRIGHTNESS_LV, isp_brightness_lv);
	bridge_mem_add_tag(ISP_NIGHT_MODE, isp_night_mode);
#else
	printf("fastboot_version = %d\r\n", fastboot_get_version());
#endif //end of defined(FASTBOOT20)
	return 0;
}

int bridge_mem_plan_sensor2(void)
{
	ISP_SENSOR_CTRL sensor_ctrl_expt = {0}, sensor_ctrl_gain = {0};
	UINT32 sensor_name = 0, sensor_i2c_id = 0, sensor_i2c_addr = 0;
	UINT32 cur_mode = 0, row_time = 0;
	CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic = {0};
	ISP_BUILTIN_CGAIN *cgain = NULL;
	FAST_SENSOR_DESC *p_desc = fast_sensor_get_desc(1);

	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[1]\n");
		return -1;
	}

	// start to plan
	// add iq tag with value
#if (SENSOR_DRIVER_READY) && defined(_SEN2_ENABLED_)
	sensor_name = p_desc->id;
	SEN_GET_EXPT_GAIN_I2C(_SEN2_, 1, &sensor_ctrl_expt, &sensor_ctrl_gain, &sensor_i2c_id,  &sensor_i2c_addr);
	SEN_GET_CUR_MODE(_SEN2_, 0, &cur_mode);
	SEN_GET_ROW_TIME(_SEN2_, 0, &row_time);
	SEN_GET_MODE_PARAM(_SEN2_, 0, &mode_basic);
#else
	sensor_ctrl_expt.exp_time[0] = 10000;
	sensor_ctrl_gain.gain_ratio[0] = 1000;
	sensor_name = 0;
	cur_mode = 0;
	row_time = 0;
	mode_basic.signal_info.vd_period = 0;
	mode_basic.dft_fps = 0;
	mode_basic.frame_num = 0;
	mode_basic.gain.max = 0;
#endif

	if (sensor_ctrl_expt.exp_time[0] == 0) {
		sensor_ctrl_expt.exp_time[0] = 10000;
	}
	if (sensor_ctrl_gain.gain_ratio[0] == 0) {
		sensor_ctrl_gain.gain_ratio[0] = 1000;
	}

	isp_dgain = isp_builtin_get_dgain(1);
	cgain = isp_builtin_get_cgain(1);
	isp_rgain = cgain->r;
	isp_ggain = cgain->g;
	isp_bgain = cgain->b;

	bridge_mem_modify_tag(SENSOR_PRESET_NAME_2, sensor_name);
	bridge_mem_add_tag(SENSOR_CHGMODE_FPS_2, p_desc->fps);
	bridge_mem_add_tag(SENSOR_PRESET_EXPT_2, sensor_ctrl_expt.exp_time[0]);
	bridge_mem_add_tag(SENSOR_PRESET_GAIN_2, sensor_ctrl_gain.gain_ratio[0]);
	bridge_mem_add_tag(SENSOR_EXPT_MAX_2, p_desc->expt_max);
	bridge_mem_add_tag(SENSOR_I2C_ID_2, sensor_i2c_id);
	bridge_mem_add_tag(SENSOR_I2C_ADDR_2, sensor_i2c_addr);
	bridge_mem_add_tag(SENSOR_CUR_MODE_2, cur_mode);
	bridge_mem_add_tag(SENSOR_ROW_TIME_2, row_time);
	bridge_mem_add_tag(SENSOR_VD_2, mode_basic.signal_info.vd_period);
	bridge_mem_add_tag(SENSOR_DFT_FPS_2, mode_basic.dft_fps);
	bridge_mem_add_tag(SENSOR_FRAME_NUM_2, mode_basic.frame_num);
	bridge_mem_add_tag(SENSOR_MAX_GAIN_2, mode_basic.gain.max);
	bridge_mem_add_tag(ISP_PATH_2, isp_sensor_path_2);
	bridge_mem_add_tag(ISP_D_GAIN_2, isp_dgain);
	bridge_mem_add_tag(ISP_R_GAIN_2, isp_rgain);
	bridge_mem_add_tag(ISP_G_GAIN_2, isp_ggain);
	bridge_mem_add_tag(ISP_B_GAIN_2, isp_bgain);
	bridge_mem_add_tag(ISP_SHDR_ENABLE_2, p_desc->shdr);
	bridge_mem_add_tag(ISP_SHDR_PATH_2, isp_sensor_shdr_map_path_2);
	bridge_mem_add_tag(ISP_SHDR_MASK_2, isp_sensor_shdr_id_mask_2);
	bridge_mem_add_tag(ISP_NR_LV_2, isp_nr_lv);
	bridge_mem_add_tag(ISP_3DNR_LV_2, isp_3dnr_lv);
	bridge_mem_add_tag(ISP_SHARPNESS_LV_2, isp_sharpness_lv);
	bridge_mem_add_tag(ISP_SATURATION_LV_2, isp_saturation_lv);
	bridge_mem_add_tag(ISP_CONTRAST_LV_2, isp_contrast_lv);
	bridge_mem_add_tag(ISP_BRIGHTNESS_LV_2, isp_brightness_lv);
	bridge_mem_add_tag(ISP_NIGHT_MODE_2, isp_night_mode);

	return 0;
}

static INT32 fastboot_parsing_sensor_sub_dtsi(UINT32 *data, CHAR *prop, UINT8 *pfdt_addr, INT32 node_ofst)
{
	INT32 data_size;
	const void *pfdt_node;

	pfdt_node = fdt_getprop(pfdt_addr, node_ofst, prop, (int *)&data_size);
	if ((pfdt_node != NULL) && (data_size != 0)) {
		UINT32 *p_data = (unsigned int *)pfdt_node;
		*data = be32_to_cpu(p_data[0]);
		return 0;
	} else {
		DBG_DUMP("cannot find %s \r\n", prop);
		*data = 0;
		return -1;
	}
}

static void fastboot_parsing_sensor_dtsi(void)
{
	UINT8 *pfdt_addr = NULL;
	INT32 node_ofst;
	CHAR node_path[48], sensor_name[24];
	FAST_SENSOR_DESC *p_desc = fast_sensor_get_desc(0);

#if defined(FASTBOOT20)
	pfdt_addr = (unsigned char *)fdtfast_get_base();
#else
	pfdt_addr = (unsigned char *)fdt_get_base();
#endif
	if (pfdt_addr == NULL) {
		DBG_DUMP("fdtfast_get_base fail \r\n");
		return;
	}

	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[0]\n");
		return;
	}

	snprintf(sensor_name, sizeof(sensor_name) - 1, p_desc->name);
	snprintf(node_path, sizeof(node_path) - 1, "/fastboot/sensor/%s", &sensor_name[4]);
	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&node_path);
	if (node_ofst >= 0) {
		fastboot_parsing_sensor_sub_dtsi(&dtsi_sen_mode, "sen_mode", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_fps, "fps", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_expt, "expt", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_gain, "gain", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_i2c_id, "i2c_id", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_i2c_addr, "i2c_addr", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_frame_num, "frame_num", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_row_time, "row_time", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_vd, "vd", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_dft_fps, "dft_fps", pfdt_addr, node_ofst);
		fastboot_parsing_sensor_sub_dtsi(&dtsi_max_gain, "max_gain", pfdt_addr, node_ofst);

		printf("%s dtsi 1, sen_mode %d, fps %d, expt %d, gain %d, i2c_id %d, i2c_addr 0x%X \n",
			&sensor_name[8], dtsi_sen_mode, dtsi_fps, dtsi_expt, dtsi_gain, dtsi_i2c_id, dtsi_i2c_addr);
		printf("%s dtsi 2, frame_num %d, row_time %d, vd %d, dft_fps %d, max_gain %d \n",
			&sensor_name[8], dtsi_frame_num, dtsi_row_time, dtsi_vd, dtsi_dft_fps, dtsi_max_gain);

	} else {
		DBG_DUMP("cannot find %s \r\n", node_path);
	}
}

int fast_2a_delay_ms(void)
{
	int fps;
	int delay_ms = 170; //default 30fps, delay 170 ms
#if 0  // not support LDCF_FAST_SENSOR
	SHMINFO *p_shm = fdt_get_shminfo();
	if (p_shm->boot.LdCtrl2 & LDCF_FAST_SENSOR) {
#else
	if (0) {
#endif // not support LDCF_FAST_SENSOR
#if defined(FASTBOOT20)
		unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
#else
		unsigned char *p_fdt = (unsigned char *)fdt_get_base();
#endif
		int len;
		int nodeoffset;
		const void *nodep;

		if (p_fdt == NULL) {
			DBG_ERR("p_fdt is NULL.\n");
			return delay_ms;
		}

		nodeoffset = fdt_path_offset(p_fdt, "sensor0");
		if (nodeoffset < 0) {
			DBG_WRN("unable to open fdtfast sensor0\n");
			return delay_ms;
		}

		nodep = fdt_getprop(p_fdt, nodeoffset, "fps", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access sensor0/fps. \n");
			return delay_ms;
		}
		fps = be32_to_cpu(*(UINT32 *)nodep) / 100;
	} else {

		FAST_SENSOR_DESC *p_desc = fast_sensor_get_desc(0);

		if (p_desc == NULL) {
			DBG_ERR("unable to find desc to sensor[0]\n");
			return -1;
		}
		fps = p_desc->fast2a.fps / 100;
	}

	// rule is 7vd for AE
	if (fps >= 240) {
		delay_ms = 33;
	} else if (fps >= 120) {
		delay_ms = 66;
	} else if (fps >= 60) {
		delay_ms = 132;
	} else {
		delay_ms = 170;
	}
	DBG_DUMP("2a delay = %d ms\n", delay_ms);
	return delay_ms;
}

void fast_2a(void)
{
#if defined(FASTBOOT20)
	// NOTE: Init ISP builtin
	ISP_BUILTIN_INIT_PARAM init_param = {0};
	ISP_SENSOR_CTRL sensor_ctrl_expt = {0}, sensor_ctrl_gain = {0};
	UINT32 sensor_name = 0, sensor_i2c_id = 0, sensor_i2c_addr = 0;
	UINT32 cur_mode = 0, row_time = 0;
	CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic = {0};
	FAST_SENSOR_DESC *p_desc = NULL;

#if defined(_SEN1_ENABLED_)
	p_desc = fast_sensor_get_desc(0);
	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[0]\n");
		return;
	}
	sensor_name = p_desc->id;

#if (SENSOR_DRIVER_READY)
	SEN_GET_EXPT_GAIN_I2C(_SEN1_, 0, &sensor_ctrl_expt, &sensor_ctrl_gain, &sensor_i2c_id,  &sensor_i2c_addr);
	SEN_GET_CUR_MODE(_SEN1_, 0, &cur_mode);
	SEN_GET_ROW_TIME(_SEN1_, 0, &row_time);
	SEN_GET_MODE_PARAM(_SEN1_, 0, &mode_basic);
#endif

	if (sensor_ctrl_expt.exp_time[0] == 0) {
		sensor_ctrl_expt.exp_time[0] = 10000;
	}
	if (sensor_ctrl_gain.gain_ratio[0] == 0) {
		sensor_ctrl_gain.gain_ratio[0] = 1000;
	}

#if 0  // not support LDCF_FAST_SENSOR
	SHMINFO *p_shm = fdt_get_shminfo();
	if (p_shm->boot.LdCtrl2 & LDCF_FAST_SENSOR) {
#else
	if (0) {
#endif // not support LDCF_FAST_SENSOR
		fastboot_parsing_sensor_dtsi();
		init_param.sensor_preset_name[0] = sensor_name;
		init_param.sensor_chgmode_fps[0] = dtsi_fps;
		init_param.sensor_preset_expt[0] = dtsi_expt;
		init_param.sensor_preset_gain[0] = dtsi_gain;
		init_param.sensor_expt_max[0] = p_desc->fast2a.expt_max;
		init_param.sensor_i2c_id[0] = dtsi_i2c_id;
		init_param.sensor_i2c_addr[0] = dtsi_i2c_addr;
		init_param.sensor_cur_mode[0] = dtsi_sen_mode;
		init_param.sensor_row_time[0] = dtsi_row_time;
		init_param.sensor_vd[0] = dtsi_vd;
		init_param.sensor_dft_fps[0] = dtsi_dft_fps;
		init_param.sensor_frame_num[0] = dtsi_frame_num;
		init_param.sensor_max_gain[0] = dtsi_max_gain;
	} else {
		init_param.sensor_preset_name[0] = sensor_name;
		init_param.sensor_chgmode_fps[0] = p_desc->fast2a.fps;
		init_param.sensor_preset_expt[0] = sensor_ctrl_expt.exp_time[0];
		init_param.sensor_preset_gain[0] = sensor_ctrl_gain.gain_ratio[0];
		init_param.sensor_expt_max[0] = p_desc->fast2a.expt_max;
		init_param.sensor_i2c_id[0] = sensor_i2c_id;
		init_param.sensor_i2c_addr[0] = sensor_i2c_addr;
		init_param.sensor_cur_mode[0] = cur_mode;
		init_param.sensor_row_time[0] = row_time;
		init_param.sensor_vd[0] = mode_basic.signal_info.vd_period;
		init_param.sensor_dft_fps[0] = mode_basic.dft_fps;
		init_param.sensor_frame_num[0] = mode_basic.frame_num;
		init_param.sensor_max_gain[0] = mode_basic.gain.max;
	}
	init_param.isp_path[0] = 0;
	init_param.isp_d_gain[0] = isp_dgain;
	init_param.isp_r_gain[0] = isp_rgain;
	init_param.isp_g_gain[0] = isp_ggain;
	init_param.isp_b_gain[0] = isp_bgain;
	init_param.isp_shdr_enable[0] = p_desc->shdr;
	init_param.isp_shdr_path[0] = isp_sensor_shdr_map_path;
	init_param.isp_shdr_mask[0] = isp_sensor_shdr_id_mask;
	init_param.isp_nr_lv[0] = isp_nr_lv;
	init_param.isp_3dnr_lv[0] = isp_3dnr_lv;
	init_param.isp_sharpness_lv[0] = isp_sharpness_lv;
	init_param.isp_ssaturation_lv[0] = isp_saturation_lv;
	init_param.isp_contrast_lv[0] = isp_contrast_lv;
	init_param.isp_brightness_lv[0] = isp_brightness_lv;
	init_param.isp_night_mode[0] = isp_night_mode;
#else
	init_param.sensor_preset_name[0] = 0xFFFFFFFF;
#endif

#if defined(_SEN2_ENABLED_)
	p_desc = fast_sensor_get_desc(1);
	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[1]\n");
		return;
	}
	sensor_name = p_desc->id;
#if (SENSOR_DRIVER_READY)
	SEN_GET_EXPT_GAIN_I2C(_SEN2_, 1, &sensor_ctrl_expt, &sensor_ctrl_gain, &sensor_i2c_id,  &sensor_i2c_addr);
	SEN_GET_CUR_MODE(_SEN2_, 0, &cur_mode);
	SEN_GET_ROW_TIME(_SEN2_, 0, &row_time);
	SEN_GET_MODE_PARAM(_SEN2_, 0, &mode_basic);
#endif

	if (sensor_ctrl_expt.exp_time[0] == 0) {
		sensor_ctrl_expt.exp_time[0] = 10000;
	}
	if (sensor_ctrl_gain.gain_ratio[0] == 0) {
		sensor_ctrl_gain.gain_ratio[0] = 1000;
	}

	init_param.sensor_preset_name[1] = sensor_name;
	init_param.sensor_chgmode_fps[1] = p_desc->fast2a.fps;
	init_param.sensor_preset_expt[1] = sensor_ctrl_expt.exp_time[0];
	init_param.sensor_preset_gain[1] = sensor_ctrl_gain.gain_ratio[0];
	init_param.sensor_expt_max[1] = p_desc->fast2a.expt_max;
	init_param.sensor_i2c_id[1] = sensor_i2c_id;
	init_param.sensor_i2c_addr[1] = sensor_i2c_addr;
	init_param.sensor_cur_mode[1] = cur_mode;
	init_param.sensor_row_time[1] = row_time;
	init_param.sensor_vd[1] = mode_basic.signal_info.vd_period;
	init_param.sensor_dft_fps[1] = mode_basic.dft_fps;
	init_param.sensor_frame_num[1] = mode_basic.frame_num;
	init_param.sensor_max_gain[1] = mode_basic.gain.max;
	init_param.isp_path[1] = isp_sensor_path_2;
	init_param.isp_d_gain[1] = isp_dgain;
	init_param.isp_r_gain[1] = isp_rgain;
	init_param.isp_g_gain[1] = isp_ggain;
	init_param.isp_b_gain[1] = isp_bgain;
	init_param.isp_shdr_enable[1] = p_desc->shdr;
	init_param.isp_shdr_path[1] = isp_sensor_shdr_map_path;
	init_param.isp_shdr_mask[1] = isp_sensor_shdr_id_mask;
	init_param.isp_nr_lv[1] = isp_nr_lv;
	init_param.isp_3dnr_lv[1] = isp_3dnr_lv;
	init_param.isp_sharpness_lv[1] = isp_sharpness_lv;
	init_param.isp_ssaturation_lv[1] = isp_saturation_lv;
	init_param.isp_contrast_lv[1] = isp_contrast_lv;
	init_param.isp_brightness_lv[1] = isp_brightness_lv;
	init_param.isp_night_mode[1] = isp_night_mode;
#else
	init_param.sensor_preset_name[1] = 0xFFFFFFFF;
#endif
	isp_builtin_init(&init_param);
#else

	fastboot_parsing_sensor_dtsi();
#endif //end of defined(FASTBOOT20)
}

void sensor_embedded_2a(void)
{
	// TODO: get sensor embedded expt and gain for bridge memory
}

#if (PREROLL_2A == ENABLE)
void sensor_preroll_2a(void)
{
	ISP_BUILTIN_INIT_PARAM init_param = {0};
	ISP_SENSOR_CTRL sensor_ctrl_expt = {0}, sensor_ctrl_gain = {0};
	UINT32 sensor_name = 0, sensor_i2c_id = 0, sensor_i2c_addr = 0;
	UINT32 cur_mode = 0, row_time = 0;
	CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic = {0};
	FAST_SENSOR_DESC *p_desc = NULL;

	#if defined(_SEN1_ENABLED_)
	p_desc = fast_sensor_get_desc(0);
	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[0]\n");
		return;
	}
	sensor_name = p_desc->id;

	#if (SENSOR_DRIVER_READY)
	SEN_GET_EXPT_GAIN_I2C(_SEN1_, 0, &sensor_ctrl_expt, &sensor_ctrl_gain, &sensor_i2c_id,  &sensor_i2c_addr);
	SEN_GET_CUR_MODE(_SEN1_, 0, &cur_mode);
	SEN_GET_ROW_TIME(_SEN1_, 0, &row_time);
	SEN_GET_MODE_PARAM(_SEN1_, 0, &mode_basic);
	#endif

	if (sensor_ctrl_expt.exp_time[0] == 0) {
		sensor_ctrl_expt.exp_time[0] = 10000;
	}
	if (sensor_ctrl_gain.gain_ratio[0] == 0) {
		sensor_ctrl_gain.gain_ratio[0] = 1000;
	}

	// NOTE: temp value for preroll
	isp_rgain = 345;
	isp_ggain = 256;
	isp_bgain = 467;

	init_param.sensor_preset_name[0] = sensor_name;
	init_param.sensor_chgmode_fps[0] = p_desc->fast2a.fps;
	init_param.sensor_preset_expt[0] = sensor_ctrl_expt.exp_time[0];
	init_param.sensor_preset_gain[0] = sensor_ctrl_gain.gain_ratio[0];
	init_param.sensor_expt_max[0] = p_desc->fast2a.expt_max;
	init_param.sensor_i2c_id[0] = sensor_i2c_id;
	init_param.sensor_i2c_addr[0] = sensor_i2c_addr;
	init_param.sensor_cur_mode[0] = cur_mode;
	init_param.sensor_row_time[0] = row_time;
	init_param.sensor_vd[0] = mode_basic.signal_info.vd_period;
	init_param.sensor_dft_fps[0] = mode_basic.dft_fps;
	init_param.sensor_frame_num[0] = mode_basic.frame_num;
	init_param.sensor_max_gain[0] = mode_basic.gain.max;
	init_param.isp_path[0] = 0;
	init_param.isp_d_gain[0] = isp_dgain;
	init_param.isp_r_gain[0] = isp_rgain;
	init_param.isp_g_gain[0] = isp_ggain;
	init_param.isp_b_gain[0] = isp_bgain;
	init_param.isp_shdr_enable[0] = p_desc->shdr;
	init_param.isp_shdr_path[0] = isp_sensor_shdr_map_path;
	init_param.isp_shdr_mask[0] = isp_sensor_shdr_id_mask;
	init_param.isp_nr_lv[0] = isp_nr_lv;
	init_param.isp_3dnr_lv[0] = isp_3dnr_lv;
	init_param.isp_sharpness_lv[0] = isp_sharpness_lv;
	init_param.isp_ssaturation_lv[0] = isp_saturation_lv;
	init_param.isp_contrast_lv[0] = isp_contrast_lv;
	init_param.isp_brightness_lv[0] = isp_brightness_lv;
	init_param.isp_night_mode[0] = isp_night_mode;

	init_param.sensor_preset_name[1] = sensor_name;
	init_param.sensor_chgmode_fps[1] = p_desc->fast2a.fps;
	init_param.sensor_preset_expt[1] = sensor_ctrl_expt.exp_time[0];
	init_param.sensor_preset_gain[1] = sensor_ctrl_gain.gain_ratio[0];
	init_param.sensor_expt_max[1] = p_desc->fast2a.expt_max;
	init_param.sensor_i2c_id[1] = sensor_i2c_id;
	init_param.sensor_i2c_addr[1] = sensor_i2c_addr;
	init_param.sensor_cur_mode[1] = cur_mode;
	init_param.sensor_row_time[1] = row_time;
	init_param.sensor_vd[1] = mode_basic.signal_info.vd_period;
	init_param.sensor_dft_fps[1] = mode_basic.dft_fps;
	init_param.sensor_frame_num[1] = mode_basic.frame_num;
	init_param.sensor_max_gain[1] = mode_basic.gain.max;
	init_param.isp_path[1] = 1;
	init_param.isp_d_gain[1] = isp_dgain;
	init_param.isp_r_gain[1] = isp_rgain;
	init_param.isp_g_gain[1] = isp_ggain;
	init_param.isp_b_gain[1] = isp_bgain;
	init_param.isp_shdr_enable[1] = p_desc->shdr;
	init_param.isp_shdr_path[1] = isp_sensor_shdr_map_path;
	init_param.isp_shdr_mask[1] = isp_sensor_shdr_id_mask;
	init_param.isp_nr_lv[1] = isp_nr_lv;
	init_param.isp_3dnr_lv[1] = isp_3dnr_lv;
	init_param.isp_sharpness_lv[1] = isp_sharpness_lv;
	init_param.isp_ssaturation_lv[1] = isp_saturation_lv;
	init_param.isp_contrast_lv[1] = isp_contrast_lv;
	init_param.isp_brightness_lv[1] = isp_brightness_lv;
	init_param.isp_night_mode[1] = isp_night_mode;
	#else
	init_param.sensor_preset_name[0] = 0xFFFFFFFF;
	init_param.sensor_preset_name[1] = 0xFFFFFFFF;
	#endif

	isp_builtin_init(&init_param);
}
#endif

void sensor_normal_2a(void)
{
	// NOTE: Init ISP builtin
	ISP_BUILTIN_INIT_PARAM init_param = {0};
	ISP_SENSOR_CTRL sensor_ctrl_expt = {0}, sensor_ctrl_gain = {0};
	UINT32 sensor_name = 0, sensor_i2c_id = 0, sensor_i2c_addr = 0;
	UINT32 cur_mode = 0, row_time = 0;
	CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic = {0};
	FAST_SENSOR_DESC *p_desc = NULL;

#if defined(_SEN1_ENABLED_)
	p_desc = fast_sensor_get_desc(0);
	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[0]\n");
		return;
	}
	sensor_name = p_desc->id;

#if (SENSOR_DRIVER_READY)
	SEN_GET_EXPT_GAIN_I2C(_SEN1_, 0, &sensor_ctrl_expt, &sensor_ctrl_gain, &sensor_i2c_id,  &sensor_i2c_addr);
	SEN_GET_CUR_MODE(_SEN1_, 0, &cur_mode);
	SEN_GET_ROW_TIME(_SEN1_, 0, &row_time);
	SEN_GET_MODE_PARAM(_SEN1_, 0, &mode_basic);
#endif

	if (sensor_ctrl_expt.exp_time[0] == 0) {
		sensor_ctrl_expt.exp_time[0] = 10000;
	}
	if (sensor_ctrl_gain.gain_ratio[0] == 0) {
		sensor_ctrl_gain.gain_ratio[0] = 1000;
	}

	init_param.sensor_preset_name[0] = sensor_name;
	init_param.sensor_chgmode_fps[0] = p_desc->fps;
	init_param.sensor_preset_expt[0] = sensor_ctrl_expt.exp_time[0];
	init_param.sensor_preset_gain[0] = sensor_ctrl_gain.gain_ratio[0];
	init_param.sensor_expt_max[0] = p_desc->expt_max;
	init_param.sensor_i2c_id[0] = sensor_i2c_id;
	init_param.sensor_i2c_addr[0] = sensor_i2c_addr;
	init_param.sensor_cur_mode[0] = cur_mode;
	init_param.sensor_row_time[0] = row_time;
	init_param.sensor_vd[0] = mode_basic.signal_info.vd_period;
	init_param.sensor_dft_fps[0] = mode_basic.dft_fps;
	init_param.sensor_frame_num[0] = mode_basic.frame_num;
	init_param.sensor_max_gain[0] = mode_basic.gain.max;
	init_param.isp_path[0] = 0;
	init_param.isp_d_gain[0] = isp_dgain;
	init_param.isp_r_gain[0] = isp_rgain;
	init_param.isp_g_gain[0] = isp_ggain;
	init_param.isp_b_gain[0] = isp_bgain;
	init_param.isp_shdr_enable[0] = p_desc->shdr;
	init_param.isp_shdr_path[0] = isp_sensor_shdr_map_path;
	init_param.isp_shdr_mask[0] = isp_sensor_shdr_id_mask;
	init_param.isp_nr_lv[0] = isp_nr_lv;
	init_param.isp_3dnr_lv[0] = isp_3dnr_lv;
	init_param.isp_sharpness_lv[0] = isp_sharpness_lv;
	init_param.isp_ssaturation_lv[0] = isp_saturation_lv;
	init_param.isp_contrast_lv[0] = isp_contrast_lv;
	init_param.isp_brightness_lv[0] = isp_brightness_lv;
	init_param.isp_night_mode[0] = isp_night_mode;
#else
	init_param.sensor_preset_name[0] = 0xFFFFFFFF;
#endif

#if defined(_SEN2_ENABLED_)
	p_desc = fast_sensor_get_desc(1);
	if (p_desc == NULL) {
		DBG_ERR("unable to find desc to sensor[1]\n");
		return;
	}
	sensor_name = p_desc->id;
#if (SENSOR_DRIVER_READY)
	SEN_GET_EXPT_GAIN_I2C(_SEN2_, 1, &sensor_ctrl_expt, &sensor_ctrl_gain, &sensor_i2c_id,  &sensor_i2c_addr);
	SEN_GET_CUR_MODE(_SEN2_, 0, &cur_mode);
	SEN_GET_ROW_TIME(_SEN2_, 0, &row_time);
	SEN_GET_MODE_PARAM(_SEN2_, 0, &mode_basic);
#endif

	if (sensor_ctrl_expt.exp_time[0] == 0) {
		sensor_ctrl_expt.exp_time[0] = 10000;
	}
	if (sensor_ctrl_gain.gain_ratio[0] == 0) {
		sensor_ctrl_gain.gain_ratio[0] = 1000;
	}

	init_param.sensor_preset_name[1] = sensor_name;
	init_param.sensor_chgmode_fps[1] = p_desc->fps;
	init_param.sensor_preset_expt[1] = sensor_ctrl_expt.exp_time[0];
	init_param.sensor_preset_gain[1] = sensor_ctrl_gain.gain_ratio[0];
	init_param.sensor_expt_max[1] = p_desc->expt_max;
	init_param.sensor_i2c_id[1] = sensor_i2c_id;
	init_param.sensor_i2c_addr[1] = sensor_i2c_addr;
	init_param.sensor_cur_mode[1] = cur_mode;
	init_param.sensor_row_time[1] = row_time;
	init_param.sensor_vd[1] = mode_basic.signal_info.vd_period;
	init_param.sensor_dft_fps[1] = mode_basic.dft_fps;
	init_param.sensor_frame_num[1] = mode_basic.frame_num;
	init_param.sensor_max_gain[1] = mode_basic.gain.max;
	init_param.isp_path[1] = isp_sensor_path_2;
	init_param.isp_d_gain[1] = isp_dgain;
	init_param.isp_r_gain[1] = isp_rgain;
	init_param.isp_g_gain[1] = isp_ggain;
	init_param.isp_b_gain[1] = isp_bgain;
	init_param.isp_shdr_enable[1] = p_desc->shdr;
	init_param.isp_shdr_path[1] = isp_sensor_shdr_map_path;
	init_param.isp_shdr_mask[1] = isp_sensor_shdr_id_mask;
	init_param.isp_nr_lv[1] = isp_nr_lv;
	init_param.isp_3dnr_lv[1] = isp_3dnr_lv;
	init_param.isp_sharpness_lv[1] = isp_sharpness_lv;
	init_param.isp_ssaturation_lv[1] = isp_saturation_lv;
	init_param.isp_contrast_lv[1] = isp_contrast_lv;
	init_param.isp_brightness_lv[1] = isp_brightness_lv;
	init_param.isp_night_mode[1] = isp_night_mode;
#else
	init_param.sensor_preset_name[1] = 0xFFFFFFFF;
#endif
	isp_builtin_init(&init_param);
}
