#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <hdal.h>
#include <kwrap/file.h>
#include <kwrap/util.h>
#include "dpc_lib.h"
#include "dpc.h"
#include <FreeRTOS_POSIX.h>
#include <FileSysTsk.h>

#define DBG_WRN(fmtstr, args...) printf("\033[33mWRN:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_ERR(fmtstr, args...) printf("\033[31mERR:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)

#if 0
#define DBG_IND(fmtstr, args...)  printf("%s(): " fmtstr, __func__, ##args)
#else
#define DBG_IND(fmtstr, args...)
#endif

static DPC_STATUS dp_status = DPC_NONE;
#if (_PACKAGE_SDCARD_) &&  (_PACKAGE_FILESYS_)
static CAL_ALG_DP_RST dp_rst = {0};
#endif

DPC_STATUS dpc_get_status(void){
	//check if dp.bin exists
#if (_PACKAGE_SDCARD_) &&  (_PACKAGE_FILESYS_)
	FST_FILE h_file_dpc = FileSys_OpenFile("A:\\dp.bin", FST_OPEN_READ | FST_OPEN_EXISTING);
	if (h_file_dpc != 0){
		FileSys_CloseFile(h_file_dpc);
		h_file_dpc = 0;
		DBG_DUMP("dp.bin is ready. dpc_status = %d\n", dpc_get_status());
	} else {
		DBG_WRN("dp.bin has not been generated\n");
		dp_status = DPC_NONE;
	}
#else
	DBG_ERR("SDCARD or FILESYS package not enabled!\r\n");
#endif
	return dp_status;
}

ER read_dpc_config(UINT8 *buf){
#if (_PACKAGE_SDCARD_) &&  (_PACKAGE_FILESYS_)
	FST_FILE h_file_dpc = FileSys_OpenFile("A:\\dp.bin", FST_OPEN_READ);
	UINT32 buf_len = sizeof(UINT32) * IQ_DPC_MAX_NUM;
	if (h_file_dpc == 0){
		DBG_WRN("no dp.bin\r\n");
		return E_SYS;
	} else {
		if (FileSys_ReadFile(h_file_dpc, buf, (UINT32 *)&buf_len, 0, NULL) != 0){
			DBG_ERR("dp.bin read failed!\r\n");
			return E_SYS;
		}
	}
	return E_OK;
#else
	DBG_ERR("SDCARD or FILESYS package not enabled!\r\n");
	return E_SYS;
#endif
}

INT32 dpc_bright_start(void){
#if (_PACKAGE_SDCARD_) &&  (_PACKAGE_FILESYS_)
	BOOL ret = 0;
	FST_FILE h_file_dpc = 0;
	char *raw_buf = NULL, *tmp_buf = NULL;
	UINT32 *dp_buf = NULL;
	UINT32 raw_size = 0, avg = 0, j = 0;
	ISPT_SENSOR_MODE_INFO sensor_mode_info = {0};
	ISPT_RAW_INFO raw_info = {0};
	AET_MANUAL dpc_ae_info = {0};
	CAL_ALG_DP_PARAM dp_param = {0};

	const unsigned int dpc_th = 20; //defual 20
	const unsigned int sensor_id = 0;

	if (vendor_isp_init() == HD_ERR_NG) {
		DBG_ERR("vendor_isp_init() fails.\n");
		ret = -1;
		goto BRIGHT_DPC_END;
	}

	//check if calibration hsa been done once
	if (dp_status == DPC_DARK_DONE){
		DBG_DUMP("a dark pixel result has beed detected.\r\n");
		h_file_dpc = FileSys_OpenFile("A:\\dp.bin", FST_OPEN_WRITE | FST_OPEN_EXISTING);
		if (h_file_dpc == 0){
			DBG_ERR("dp.bin open failed\r\n");
			ret = -1;
			goto BRIGHT_DPC_END;
		}
	} else {
		DBG_DUMP("no dark pixel result has been detected. create new dp.bin\r\n");
		h_file_dpc = FileSys_OpenFile("A:\\dp.bin", FST_OPEN_WRITE | FST_CREATE_ALWAYS);
		if (h_file_dpc == 0){
			DBG_ERR("dp.bin open failed\r\n");
			ret = -1;
			goto BRIGHT_DPC_END;
		}
	}

	//get raw info
	dpc_get_sensor_info(sensor_id, &sensor_mode_info);
	raw_size = sensor_mode_info.info.crp_size.w* sensor_mode_info.info.crp_size.h;

	//get raw/tmp buffer
	raw_buf = (char *)malloc(sizeof(char)*raw_size);
	if (raw_buf != NULL) {
		memset(raw_buf, 0, sizeof(char)*raw_size);
	} else {
		DBG_ERR("fail to allocate raw_buf\n");
		ret = -1;
		goto BRIGHT_DPC_END;
	}
	raw_info.raw_info.addr = (UINT32)raw_buf;

	tmp_buf = (char *)malloc(sizeof(char)*raw_size * 3 / 2);
	if (tmp_buf != NULL) {
		memset(tmp_buf, 0, sizeof(char)*raw_size * 3 / 2);
	} else {
		DBG_ERR("fail to allocate tmp_buf\n");
		ret = -1;
		goto BRIGHT_DPC_END;
	}

	//get dp buffer
	dp_buf = (UINT32 *)malloc(IQ_DPC_MAX_NUM*sizeof(UINT32));
	if (dp_buf != NULL) {
		memset(dp_buf, 0, IQ_DPC_MAX_NUM*sizeof(UINT32));
	} else {
		DBG_ERR("fail to allocate dp_buf\r\n");
		ret = -1;
		goto BRIGHT_DPC_END;
	}

	//bright pixel
	DBG_DUMP("Bright Pixel: ExpT=50000, IsoGain=6400, dpc_th=20\n");
	dpc_ae_info.id = sensor_id;
	dpc_ae_info.manual.mode = MANUAL_MODE;
	dpc_ae_info.manual.totalgain = 0;
	dpc_ae_info.manual.expotime = 50000;
	dpc_ae_info.manual.iso_gain = 6400;
	vendor_isp_set_ae(AET_ITEM_MANUAL, &dpc_ae_info);

	if (dpc_get_raw(sensor_id, &raw_info, (UINT32)tmp_buf) == 0) {
		for (j = 0; j < raw_size; j++) {
			avg += raw_buf[j];
		}
		avg /= raw_size;
		DBG_DUMP("average lum = %d\r\n", avg);

		dp_param.raw_info = &raw_info;
		dp_param.dp_pool_addr = (unsigned int)&dp_buf[0];
		dp_param.setting.threshold = dpc_th;
		if (dp_status == DPC_DARK_DONE){
			dp_param.ori_dp_cnt = dp_rst.pixel_cnt;
			dp_param.b_chg_dp_format = DP_PARAM_CHGFMT_AUTO;
			dp_status = DPC_BOTH_DONE;
		} else {
			dp_param.ori_dp_cnt = 0;
			dp_param.b_chg_dp_format = DP_PARAM_CHGFMT_SKIP;
			dp_status = DPC_BRIGHT_DONE;
		}
		cal_dp_process(sensor_id, dp_param.raw_info, &dp_param, &dp_rst, &sensor_mode_info);
		DBG_DUMP("cnt=%d,data_length=%d \r\n", dp_rst.pixel_cnt, dp_rst.data_length);

		//save dp.bin
		FileSys_WriteFile(h_file_dpc, (UINT8 *)dp_buf, &dp_rst.data_length, FST_FLAG_NONE, 0);
		FileSys_CloseFile(h_file_dpc);
		h_file_dpc = 0;
		DBG_DUMP("dp_buf:%x\r\n", dp_buf);
	} else {
		DBG_DUMP("get raw fail, free allocated buffer\n");
		ret = -1;
		goto BRIGHT_DPC_END;
	}

BRIGHT_DPC_END:
	if (raw_buf != NULL){
		free(raw_buf);
	}
	if (tmp_buf != NULL){
		free(tmp_buf);
	}
	if (dp_buf != NULL){
		free(dp_buf);
	}
	if (h_file_dpc != 0){
		FileSys_CloseFile(h_file_dpc);
	}
	return ret;
#else
	DBG_ERR("SDCARD or FILESYS package not enabled!\r\n");
	return -1;
#endif
}

INT32 dpc_dark_start(void){
#if (_PACKAGE_SDCARD_) &&  (_PACKAGE_FILESYS_)
	BOOL ret = 0;
	FST_FILE h_file_dpc = 0;
	char *raw_buf = NULL, *tmp_buf = NULL;
	UINT32 *dp_buf = NULL;
	UINT32 raw_size = 0, avg = 0, j = 0;
	ISPT_SENSOR_MODE_INFO sensor_mode_info = {0};
	ISPT_RAW_INFO raw_info = {0};
	AET_MANUAL dpc_ae_info = {0};
	CAL_ALG_DP_PARAM dp_param = {0};

	const unsigned int dpc_th = 20; //defual 20
	const unsigned int sensor_id = 0;

	if (vendor_isp_init() == HD_ERR_NG) {
		DBG_ERR("vendor_isp_init() fails.\n");
		ret = -1;
		goto DARK_DPC_END;
	}

	//check if calibration hsa been done once
	if (dp_status == DPC_BRIGHT_DONE){
		DBG_DUMP("a bright pixel result has beed detected.\r\n");
		h_file_dpc = FileSys_OpenFile("A:\\dp.bin", FST_OPEN_WRITE | FST_OPEN_EXISTING);
		if (h_file_dpc == 0){
			DBG_ERR("dp.bin open failed\r\n");
			ret = -1;
			goto DARK_DPC_END;
		}
	} else {
		DBG_DUMP("no bright pixel result has been detected. create new dp.bin\r\n");
		h_file_dpc = FileSys_OpenFile("A:\\dp.bin", FST_OPEN_WRITE | FST_CREATE_ALWAYS);
		if (h_file_dpc == 0){
			DBG_ERR("dp.bin open failed\r\n");
			ret = -1;
			goto DARK_DPC_END;
		}
	}

	//get raw info
	dpc_get_sensor_info(sensor_id, &sensor_mode_info);
	raw_size = sensor_mode_info.info.crp_size.w* sensor_mode_info.info.crp_size.h;

	//get raw/tmp buffer
	raw_buf = (char *)malloc(sizeof(char)*raw_size);
	if (raw_buf != NULL) {
		memset(raw_buf, 0, sizeof(char)*raw_size);
	} else {
		DBG_ERR("fail to allocate raw_buf\n");
		ret = -1;
		goto DARK_DPC_END;
	}
	raw_info.raw_info.addr = (UINT32)raw_buf;

	tmp_buf = (char *)malloc(sizeof(char)*raw_size * 3 / 2);
	if (tmp_buf != NULL) {
		memset(tmp_buf, 0, sizeof(char)*raw_size * 3 / 2);
	} else {
		DBG_ERR("fail to allocate tmp_buf\n");
		ret = -1;
		goto DARK_DPC_END;
	}

	//get dp buffer
	dp_buf = (UINT32 *)malloc(IQ_DPC_MAX_NUM*sizeof(UINT32));
	if (dp_buf != NULL) {
		memset(dp_buf, 0, IQ_DPC_MAX_NUM*sizeof(UINT32));
	} else {
		DBG_ERR("fail to allocate dp_buf\r\n");
		ret = -1;
		goto DARK_DPC_END;
	}

	//dark pixel
	DBG_DUMP("Dark Pixel: ExpT=5000, IsoGain=800, dpc_th=20\n");
	dpc_ae_info.id = sensor_id;
	dpc_ae_info.manual.mode = MANUAL_MODE;
	dpc_ae_info.manual.totalgain = 0;
	dpc_ae_info.manual.expotime = 5000;
	dpc_ae_info.manual.iso_gain = 800;
	vendor_isp_set_ae(AET_ITEM_MANUAL, &dpc_ae_info);

	if (dpc_get_raw(sensor_id, &raw_info, (UINT32)tmp_buf) == 0) {
		for (j = 0; j < raw_size; j++) {
			avg += raw_buf[j];
		}
		avg /= raw_size;
		DBG_DUMP("average lum = %d\r\n", avg);

		dp_param.raw_info = &raw_info;
		dp_param.dp_pool_addr = (unsigned int)&dp_buf[0];
		dp_param.setting.threshold = dpc_th;
		if (dp_status == DPC_BRIGHT_DONE){
			dp_param.ori_dp_cnt = dp_rst.pixel_cnt;
			dp_param.b_chg_dp_format = DP_PARAM_CHGFMT_AUTO;
			dp_status = DPC_BOTH_DONE;
		} else {
			dp_param.ori_dp_cnt = 0;
			dp_param.b_chg_dp_format = DP_PARAM_CHGFMT_SKIP;
			dp_status = DPC_BRIGHT_DONE;
		}
		cal_dp_process(sensor_id, dp_param.raw_info, &dp_param, &dp_rst, &sensor_mode_info);
		DBG_DUMP("cnt=%d,data_length=%d \r\n", dp_rst.pixel_cnt, dp_rst.data_length);

		//save dp.bin
		FileSys_WriteFile(h_file_dpc, (UINT8 *)dp_buf, &dp_rst.data_length, FST_FLAG_NONE, 0);
		FileSys_CloseFile(h_file_dpc);
		h_file_dpc = 0;
		DBG_DUMP("dp_buf:%x\r\n", dp_buf);
	} else {
		DBG_DUMP("get raw fail, free allocated buffer\n");
		ret = -1;
		goto DARK_DPC_END;
	}

DARK_DPC_END:
	if (raw_buf != NULL){
		free(raw_buf);
	}
	if (tmp_buf != NULL){
		free(tmp_buf);
	}
	if (dp_buf != NULL){
		free(dp_buf);
	}
	if (h_file_dpc != 0){
		FileSys_CloseFile(h_file_dpc);
	}
	return ret;
#else
	DBG_ERR("SDCARD or FILESYS package not enabled!\r\n");
	return -1;
#endif
}
