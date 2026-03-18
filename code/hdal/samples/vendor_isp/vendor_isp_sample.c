
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "vendor_isp.h"

#define MEASURE_TIME 1
#if (MEASURE_TIME)
#include <sys/time.h>
#endif

//============================================================================
// global
//============================================================================
pthread_t isp_vd_thread_id;
static UINT32 vd_conti_run = 1;
pthread_t isp_frmend_thread_id;
static UINT32 frmend_conti_run = 1;
pthread_t isp_procend_thread_id;
static UINT32 procend_conti_run = 1;
static UINT32 isp_id = 0;

static INT32 get_choose_int(void)
{
	char buf[256];
	INT val, rt;

	rt = scanf("%d", &val);

	if (rt != 1) {
		printf("Invalid option. Try again.\n");
		clearerr(stdin);
		fgets(buf, sizeof(buf), stdin);
		val = -1;
	}

	return val;
}

static void *isp_vd_thread(void *arg)
{
	ISPT_WAIT_VD *wait_vd = (ISPT_WAIT_VD *)arg;
	#if (MEASURE_TIME)
	UINT64 time_diff;
	struct timeval start, end;
	#endif

	while (vd_conti_run) {
		#if (MEASURE_TIME)
		gettimeofday(&start, NULL);
		#endif

		vendor_isp_get_common(ISPT_ITEM_WAIT_VD, wait_vd);

		#if (MEASURE_TIME)
		gettimeofday(&end, NULL);
		time_diff = 1000000 * (UINT64)(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
		printf("time =  %llu, cnt = %d \n", time_diff, wait_vd->frame_count);
		#endif
	};

	return 0;
}

static void *isp_frmend_thread(void *arg)
{
	ISPT_WAIT_FRMEND *wait_frmend = (ISPT_WAIT_FRMEND *)arg;
	#if (MEASURE_TIME)
	UINT64 time_diff;
	struct timeval start, end;
	#endif

	while (frmend_conti_run) {
		#if (MEASURE_TIME)
		gettimeofday(&start, NULL);
		#endif

		vendor_isp_get_common(ISPT_ITEM_WAIT_FRMEND, wait_frmend);

		#if (MEASURE_TIME)
		gettimeofday(&end, NULL);
		time_diff = 1000000 * (UINT64)(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
		printf("time =  %llu, cnt = %d \n", time_diff, wait_frmend->frame_count);
		#endif
	};

	return 0;
}

static void *isp_procend_thread(void *arg)
{
	ISPT_WAIT_PROCEND *wait_procend = (ISPT_WAIT_PROCEND *)arg;
	#if (MEASURE_TIME)
	UINT64 time_diff;
	struct timeval start, end;
	#endif

	while (procend_conti_run) {
		#if (MEASURE_TIME)
		gettimeofday(&start, NULL);
		#endif

		vendor_isp_get_common(ISPT_ITEM_WAIT_PROCEND, wait_procend);

		#if (MEASURE_TIME)
		gettimeofday(&end, NULL);
		time_diff = 1000000 * (UINT64)(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
		printf("procend time =  %llu, cnt = %d \n", time_diff, wait_procend->frame_count);
		#endif
	};

	return 0;
}

int main(int argc, char *argv[])
{
	INT rt;
	INT32 option;
	UINT32 trig = 1;
	INT fz_pos = 0;
	UINT32 tmp = 0;
	UINT32 id;
	UINT32 i, j;
	UINT32 va_type, va_opt;
	UINT32 va_sum, va_sum_g1, va_sum_g2;
	CHAR char_tmp[80] = {0};
	UINT32 version = 0;
	ISPT_SENSOR_REG sensor_reg = {0};
	ISPT_SENSOR_INFO sensor_info = {0};
	ISPT_SENSOR_MODE_INFO sensor_mode_info = {0};
	ISPT_WAIT_VD wait_vd = {0};
	ISPT_FUNC func_info = {0};
	ISPT_YUV_INFO yuv_info = {0};
	ISPT_RAW_INFO raw_info = {0};
	AET_CFG_INFO cfg_info = {0};
	static ISPT_CA_DATA ca_data = {0};
	ISPT_LA_DATA la_data = {0};
	ISPT_IPE_VA_DATA va_data = {0};
	ISPT_IPE_VA_INDEP_DATA va_indep_data = {0};
	ISPT_SENSOR_EXPT sensor_expt = {0};
	ISPT_SENSOR_GAIN sensor_gain = {0};
	ISPT_D_GAIN d_gain = {0};
	ISPT_C_GAIN c_gain = {0};
	ISPT_TOTAL_GAIN total_gain = {0};
	ISPT_LV lv = {0};
	ISPT_CT ct = {0};
	ISPT_MOTOR_IRIS motor_iris = {0};
	ISPT_MOTOR_FOCUS motor_focus = {0};
	ISPT_MOTOR_ZOOM motor_zoom = {0};
	ISPT_SENSOR_DIRECTION sensor_direction = {0};
	ISPT_HISTO_DATA histo_data = {0};
	ISPT_3DNR_STA_INFO _3dnr_sta_info = {0};
	ISPT_BNR_STA_INFO bnr_sta_info = {0};
	ISPT_CA_ROI ca_roi = {0};
	ISPT_LA_ROI la_roi = {0};
	ISPT_VA_INDEP_ROI va_indep_roi = {0};
	ISPT_CA_ENABLE ca_enable = {0};
	ISPT_LA_ENABLE la_enable = {0};
	ISPT_WAIT_FRMEND wait_frmend = {0};
	ISPT_WAIT_PROCEND wait_procend = {0};
	ISPT_VPRC_INDEP vprc_indep = {0};
	ISPT_SENSOR_MODE_MANUAL sensor_mode_manual = {0};
	UINT32 low_power_lv = 0;

	// open MCU device
	if (vendor_isp_init() == HD_ERR_NG) {
		return -1;
	}

	while (trig) {
		printf("----------------------------------------\n");
		printf("   1. Get version \n");
		printf("   2. Get isp function \n");
		printf("   3. Get YUV \n");
		printf("   4. Get RAW \n");
		printf("   7. Get sensor info \n");
		printf("   8. Get sensor register \n");
		printf("   9. Get sensor mode info \n");
		printf("  10. Get ca data \n");
		printf("  11. Get la data \n");
		printf("  12. Get va data \n");
		printf("  13. Get va_indep data \n");
		printf("  14. Get wait VD \n");
		printf("  20. Get motor iris \n");
		printf("  21. Get motor focus \n");
		printf("  22. Get motor zoom \n");
		printf("  23  Get sensor direction \n");
		printf("  24. Get histo data \n");
		printf("  25. Get 3dnr sta info \n");
		printf("  33. Get sensor expt \n");
		printf("  34. Get sensor gain \n");
		printf("  40. Get ca roi \n");
		printf("  41. Get la roi \n");
		printf("  42. Get va indep roi \n");
		printf("  43. Get ca enable \n");
		printf("  44. Get la enable \n");
		printf("  45. Get wait FRMEND \n");
		printf("  46. Get wait PROCEND \n");
		printf("  49. Get vprc indep \n");
		printf("  52. Reload cfg \n");
		printf("  53. Set YUV \n");
		printf("  54. Set RAW \n");
		printf("  55. Set sensor sleep \n");
		printf("  56. Set sensor wakeup \n");
		printf("  58. Set sensor register \n");
		printf("  59. Set motor focus init \n");
		printf("  60. Set motor iris \n");
		printf("  61. Set motor focus \n");
		printf("  62. Set motor zoom \n");
		printf("  63  Set sensor direction \n");
		printf("  64  Set iris init \n");
		printf("  65  Set zoom init \n");
		printf("  83. Set sensor expt \n");
		printf("  84. Set sensor gain \n");
		printf("  85. Set d gain \n");
		printf("  86. Set c gain \n");
		printf("  87. Set total gain \n");
		printf("  88. Set lv \n");
		printf("  89. Set ct \n");
		printf("  90. Set ca roi \n");
		printf("  91. Set la roi \n");
		printf("  92. Set va indep roi \n");
		printf("  93. Set ca enable \n");
		printf("  94. Set la enable \n");
		printf("  96. Set vprc indep \n");
		printf("----------------------------------------\n");
		printf("  500. Get sensor mode manual \n");
		printf("  501. Get low power lv \n");
		printf("  502. Get bnr sta info \n");
		printf("  600. Set sensor mode manual \n");
		printf("  601. Get low power lv \n");
		printf("----------------------------------------\n");
		printf("  999. Set id, current isp_id: %d \n", isp_id);
		printf("----------------------------------------\n");
		printf(" 0.  Quit\n");
		printf("----------------------------------------\n");
		printf(">> ");
		option = get_choose_int();

		switch (option) {
		case 1:
			vendor_isp_get_common(ISPT_ITEM_VERSION, &version);
			printf("version = 0x%X \n", version);
			break;

		case 2:
			func_info.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_FUNC, &func_info);
			printf("id = %d, sie %d, ipp %d, ae %d, af %d, awb %d, defog %d, shdr %d, wdr %d \n", func_info.id, func_info.func_info.sie_valid, func_info.func_info.ipp_valid,
																												func_info.func_info.ae_valid, func_info.func_info.af_valid,
																												func_info.func_info.awb_valid, func_info.func_info.defog_valid,
																												func_info.func_info.shdr_valid, func_info.func_info.wdr_valid);
			for (tmp = 0; tmp < ISP_YUV_OUT_CH; tmp++) {
				printf(" %dx%d", func_info.func_info.yuv_out_ch[tmp].w, func_info.func_info.yuv_out_ch[tmp].h);
			}
			printf(" \n");
			break;

		case 3:
			yuv_info.id = isp_id;
			printf("Set pid (0 ~ 5)>> \n");
			yuv_info.yuv_info.pid = (UINT32)get_choose_int();

			vendor_isp_get_common(ISPT_ITEM_YUV, &yuv_info);
			printf("get yuv, id = %d, pid = %d, %d x %d \n", yuv_info.id, yuv_info.yuv_info.pid, yuv_info.yuv_info.size.w, yuv_info.yuv_info.size.h);
			break;

		case 4:
			raw_info.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_RAW, &raw_info);
			printf("get raw, id = %d, %d x %d \n", raw_info.id, raw_info.raw_info.size.w, raw_info.raw_info.size.h);
			break;

		case 7:
			vendor_isp_get_common(ISPT_ITEM_SENSOR_INFO, &sensor_info);
			for (i = 0; i < ISP_ID_MAX_NUM; i++) {
				printf("id %d, name %s, mask 0x%X, ipp_id_table 0x%X\n", i, sensor_info.sensor_info.name[i], sensor_info.sensor_info.src_id_mask[i], sensor_info.sensor_info.ipp_id_table[i]);
			}
			break;

		case 8:
			sensor_reg.id = isp_id;

			printf("Set addr (0x )>> \n");
			rt = scanf("%10s", char_tmp);
			if (rt != 1) {
				printf("read fail. \n");
				break;
			}
			sensor_reg.addr = strtol(char_tmp, NULL, 0);

			vendor_isp_get_common(ISPT_ITEM_SENSOR_REG, &sensor_reg);
			printf("id = %d, addr = 0x%X, data = 0x%X \n", sensor_reg.id, sensor_reg.addr, sensor_reg.data);
			break;

		case 9:
			sensor_mode_info.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_SENSOR_MODE_INFO, &sensor_mode_info);
			printf("id = %d, gain: %d %d, act: %d %d %d %d, crop: %d %d \n", sensor_mode_info.id, sensor_mode_info.info.min_gain, sensor_mode_info.info.max_gain,
																			sensor_mode_info.info.act_size[0].x, sensor_mode_info.info.act_size[0].y, sensor_mode_info.info.act_size[0].w, sensor_mode_info.info.act_size[0].h,
																			sensor_mode_info.info.crp_size.w, sensor_mode_info.info.crp_size.h);
			break;

		case 10:
			ca_data.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_CA_DATA, &ca_data);
			printf("id = %d, no. 528 data = %d, %d, %d \n", ca_data.id, ca_data.ca_rslt.r[528], ca_data.ca_rslt.g[528], ca_data.ca_rslt.b[528]);
			break;

		case 11:
			la_data.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_LA_DATA, &la_data);
			printf("id = %d, no. 528 data = %d, %d \n", la_data.id, la_data.la_rslt.lum_1[528], la_data.la_rslt.lum_2[528]);
			break;

		case 12:
			va_data.id = isp_id;
			printf("0. [g1h+g1v+g2h+g2v]\n");
			printf("1. [g1h+g1v, g2h+g2v]\n");
			printf("2. [g1h, g1v, g2h, g2v]\n");
			printf("Select display type >> \n");

			va_opt = (UINT32)get_choose_int();
			vendor_isp_get_common(ISPT_ITEM_VA_DATA, &va_data);

			printf(" \n");
			va_sum_g1 = va_sum_g2 = va_sum = 0;

			if(va_opt == 0) {
				for (i = 0; i < ISP_VA_H_WINNUM; i++) {
					for (j = 0; j < ISP_VA_W_WINNUM; j++) {
						printf("%6d ", (va_data.va_rslt.g1_h[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g1_v[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g2_h[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g2_v[i*ISP_VA_W_WINNUM+j]));
						va_sum += (va_data.va_rslt.g1_h[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g1_v[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g2_h[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g2_v[i*ISP_VA_W_WINNUM+j]);
					}
					printf("\n");
				}		
				printf("va_sum = %d\n", va_sum);
			} else if(va_opt == 1) {
				for (i = 0; i < ISP_VA_H_WINNUM; i++) {
					for (j = 0; j < ISP_VA_W_WINNUM; j++) {
						printf("[%6d,%6d] ", (va_data.va_rslt.g1_h[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g1_v[i*ISP_VA_W_WINNUM+j]),(va_data.va_rslt.g2_h[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g2_v[i*ISP_VA_W_WINNUM+j]));
						va_sum_g1 += (va_data.va_rslt.g1_h[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g1_v[i*ISP_VA_W_WINNUM+j]);
						va_sum_g2 += (va_data.va_rslt.g2_h[i*ISP_VA_W_WINNUM+j]+va_data.va_rslt.g2_v[i*ISP_VA_W_WINNUM+j]);
					}
					printf("\n");
				}			
				printf("va_sum_g1 = %d\n", va_sum_g1);
				printf("va_sum_g2 = %d\n", va_sum_g2);
			} else {
				for (i = 0; i < 8; i++) {
					for (j = 0; j < 8; j++) {
						printf("[%6d,%6d,%6d,%6d] ", va_data.va_rslt.g1_h[i*ISP_VA_W_WINNUM+j],va_data.va_rslt.g1_v[i*ISP_VA_W_WINNUM+j],va_data.va_rslt.g2_h[i*ISP_VA_W_WINNUM+j],va_data.va_rslt.g2_v[i*ISP_VA_W_WINNUM+j]);
					}
					printf("\n");
				}
			}
			break;

		case 13:
			va_indep_data.id = isp_id;

			printf("0. IFE_VA (before 2DNR/3DNR)\n");
			printf("1. IPE_VA (after 2DNR, before 3DNR)\n");
			printf("Select display type >> \n");
			va_type = (UINT32)get_choose_int();

			if (va_type == 0) {
				vendor_isp_get_common(ISPT_ITEM_IFE_VA_INDEP_DATA, &va_indep_data);
				printf("get IFE_VA_INDEP (before 2DNR/3DNR)\n");
			} else if (va_type == 1) {
				vendor_isp_get_common(ISPT_ITEM_VA_INDEP_DATA, &va_indep_data);
				printf("get IPE_VA_INDEP (after 2DNR, before 3DNR)\n");
			} else {
				vendor_isp_get_common(ISPT_ITEM_VA_INDEP_DATA, &va_indep_data);
				printf("get IPE_VA_INDEP (after 2DNR, before 3DNR)\n");
			}

			printf(" \n");
			for (tmp = 0; tmp < 5; tmp++) {
				printf(" no. (%d) data = %d, %d, %d, %d \n", tmp, va_indep_data.va_indep_rslt.g1_h[tmp], va_indep_data.va_indep_rslt.g1_v[tmp], va_indep_data.va_indep_rslt.g2_h[tmp], va_indep_data.va_indep_rslt.g2_v[tmp]);
			}
			break;

		case 14:
			wait_vd.id = isp_id;
			printf("Set timeout (ms)>> \n");
			wait_vd.timeout = (UINT32)get_choose_int();

			vd_conti_run = 1;
			if (pthread_create(&isp_vd_thread_id, NULL, isp_vd_thread, (void *)&wait_vd) < 0) {
				printf("create vd thread failed");
				break;
			}

			printf("Enter 0 to exit >> \n");
			vd_conti_run = (UINT32)get_choose_int();

			pthread_join(isp_vd_thread_id, NULL);

			break;

		case 20:
			motor_iris.cmd_type = MTR_GET_APERTURE_POSITION;
			vendor_isp_get_common(ISPT_ITEM_MOTOR_IRIS, &motor_iris);
			printf("get iris, %d \n", motor_iris.ctl_cmd.data[0]);
			break;

		case 21:
			motor_focus.cmd_type = MTR_GET_FOCUS_POSITION;
			vendor_isp_get_common(ISPT_ITEM_MOTOR_FOCUS, &motor_focus);
			printf("get focus, %d \n", (motor_focus.ctl_cmd.data[0] & 0xFFFF));
			break;

		case 22:
			motor_zoom.cmd_type = MTR_GET_ZOOM_POSITION;
			vendor_isp_get_common(ISPT_ITEM_MOTOR_ZOOM, &motor_zoom);
			printf("get zoom, %d \n", (motor_zoom.ctl_cmd.data[0] & 0xFFFF));
			break;

		case 23:
			sensor_direction.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_SENSOR_DIRECTION, &sensor_direction);
			printf("mirror = %d, flip = %d \n", sensor_direction.direction.mirror, sensor_direction.direction.flip);
			break;

		case 24:
			histo_data.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_HISTO_DATA, &histo_data);
			printf("id = %d,\n", histo_data.id);
			printf("pre data = ,,, %d, %d,,, %d\n", histo_data.histo_rslt.hist_stcs_pre_wdr[10], histo_data.histo_rslt.hist_stcs_pre_wdr[11], histo_data.histo_rslt.hist_stcs_pre_wdr[127]);
			printf("post data = ,,, %d, %d,,, %d\n", histo_data.histo_rslt.hist_stcs_post_wdr[10], histo_data.histo_rslt.hist_stcs_post_wdr[11], histo_data.histo_rslt.hist_stcs_post_wdr[127]);
			break;

		case 25:
			_3dnr_sta_info.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_3DNR_STA, &_3dnr_sta_info);
			printf("id = %d \n", _3dnr_sta_info.id);
			printf("info_vaild = %d \n", _3dnr_sta_info._3dnr_sta_info.info_vaild);
			printf("enable = %d \n", _3dnr_sta_info._3dnr_sta_info.enable);
			printf("buf_addr = 0x%x \n", _3dnr_sta_info._3dnr_sta_info.buf_addr);
			break;

		case 33:
			sensor_expt.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_SENSOR_EXPT, &sensor_expt);
			printf("exposure time (us) = %d, %d,\n", sensor_expt.time[0], sensor_expt.time[1]);
			break;

		case 34:
			sensor_gain.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_SENSOR_GAIN, &sensor_gain);
			printf("gain ratio (1000 = 1X) = %d, %d,\n", sensor_gain.ratio[0], sensor_gain.ratio[1]);
			break;

		case 40:
			ca_roi.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_CA_ROI, &ca_roi);
			printf("ca roi = %d, %d, %d, %d \n", ca_roi.roi.x, ca_roi.roi.y, ca_roi.roi.w, ca_roi.roi.h);
			break;

		case 41:
			la_roi.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_LA_ROI, &la_roi);
			printf("ca roi = %d, %d, %d, %d \n", la_roi.roi.x, la_roi.roi.y, la_roi.roi.w, la_roi.roi.h);
			break;

		case 42:
			va_indep_roi.id = isp_id;

			printf("0. IFE_VA (before 2DNR/3DNR)\n");
			printf("1. IPE_VA (after 2DNR, before 3DNR)\n");
			printf("Select display type >> \n");
			va_type = (UINT32)get_choose_int();

			if (va_type == 0) {
				vendor_isp_get_common(ISPT_ITEM_IFE_VA_INDEP_ROI, &va_indep_roi);
				printf("ife_va roi = %d, %d, %d, %d \n", va_indep_roi.roi[0].x, va_indep_roi.roi[0].y, va_indep_roi.roi[0].w, va_indep_roi.roi[0].h);
			} else if (va_type == 1) {
				vendor_isp_get_common(ISPT_ITEM_VA_INDEP_ROI, &va_indep_roi);
				printf("ipe_va roi = %d, %d, %d, %d \n", va_indep_roi.roi[0].x, va_indep_roi.roi[0].y, va_indep_roi.roi[0].w, va_indep_roi.roi[0].h);
			} else {
				vendor_isp_get_common(ISPT_ITEM_VA_INDEP_ROI, &va_indep_roi);
				printf("ipe_va roi = %d, %d, %d, %d \n", va_indep_roi.roi[0].x, va_indep_roi.roi[0].y, va_indep_roi.roi[0].w, va_indep_roi.roi[0].h);
			}
			break;

		case 43:
			ca_enable.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_CA_ENABLE, &ca_enable);
			printf("ca_enable = %d \n", ca_enable.enable);
			break;

		case 44:
			la_enable.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_LA_ENABLE, &la_enable);
			printf("la_enable = %d \n", la_enable.enable);
			break;

		case 45:
			wait_frmend.id = isp_id;
			printf("Set timeout (ms)>> \n");
			wait_frmend.timeout = (UINT32)get_choose_int();

			frmend_conti_run = 1;
			if (pthread_create(&isp_frmend_thread_id, NULL, isp_frmend_thread, (void *)&wait_frmend) < 0) {
				printf("create frmend thread failed");
				break;
			}

			printf("Enter 0 to exit >> \n");
			frmend_conti_run = (UINT32)get_choose_int();

			pthread_join(isp_frmend_thread_id, NULL);
			break;

		case 46:
				wait_procend.id = isp_id;
				printf("Set timeout (ms)>> \n");
				wait_procend.timeout = (UINT32)get_choose_int();

			procend_conti_run = 1;
			if (pthread_create(&isp_procend_thread_id, NULL, isp_procend_thread, (void *)&wait_procend) < 0) {
				printf("create encode thread failed");
				break;
			}

			printf("Enter 0 to exit >> \n");
			procend_conti_run = (UINT32)get_choose_int();

			pthread_join(isp_procend_thread_id, NULL);
			break;

		case 49:
			vprc_indep.id = isp_id;
			vendor_isp_get_common(ISPT_ITEM_VPRC_INDEP, &vprc_indep);
			printf("vprc_indep enable = %d \n", vprc_indep.enable);
			break;

		case 52:
			cfg_info.id = isp_id;

			printf("Select chg file>> \n");
			printf("0: isp_os02k10_0.cfg \n");
			printf("1: isp_ar0237ir_0.cfg \n");
			tmp = (UINT32)get_choose_int();

			switch (tmp) {
			case 0:
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os02k10_0.cfg", CFG_NAME_LENGTH);
				break;

			case 1:
				strncpy(cfg_info.path, "/mnt/app/isp/isp_ar0237ir_0.cfg", CFG_NAME_LENGTH);
				break;

			default:
				printf("Not support item (%d) \n", tmp);
				break;
			}
			rt = vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
			if (rt < 0) {
				printf("set AET_ITEM_RLD_CONFIG fail!\n");
				break;
			}
			rt = vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
			if (rt < 0) {
				printf("set AWBT_ITEM_RLD_CONFIG fail!\n");
				break;
			}
			rt = vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
			if (rt < 0) {
				printf("set IQT_ITEM_RLD_CONFIG fail!\n");
				break;
			}
			break;

		case 53:
			id = isp_id;

			vendor_isp_set_common(ISPT_ITEM_YUV, &id);
			printf("set yuv \n");
			break;

		case 54:
			id = isp_id;

			vendor_isp_set_common(ISPT_ITEM_RAW, &id);
			printf("set raw \n");
			break;

		case 55:
			id = isp_id;

			vendor_isp_set_common(ISPT_ITEM_SENSOR_SLEEP, &id);
			printf("set sleep \n");
			break;

		case 56:
			id = isp_id;

			vendor_isp_set_common(ISPT_ITEM_SENSOR_WAKEUP, &id);
			printf("set wakeup \n");
			break;

		case 58:
			sensor_reg.id = isp_id;

			printf("Set addr (0x )>> \n");
			rt = scanf("%10s", char_tmp);
			if (rt != 1) {
				printf("read fail. \n");
				break;
			}
			sensor_reg.addr = strtol(char_tmp, NULL, 0);

			printf("Set data (0x )>> \n");
			rt = scanf("%10s", char_tmp);
			if (rt != 1) {
				printf("read fail. \n");
				break;
			}
			sensor_reg.data = strtol(char_tmp, NULL, 0);

			vendor_isp_set_common(ISPT_ITEM_SENSOR_REG, &sensor_reg);
			printf("id = %d, addr = 0x%X, data = 0x%X \n", sensor_reg.id, sensor_reg.addr, sensor_reg.data);
			break;

		case 59:
			motor_focus.cmd_type = MTR_SET_FOCUS_INIT;			
			vendor_isp_set_common(ISPT_ITEM_MOTOR_FOCUS, &motor_focus);			
			break;
		
		case 60:
			motor_iris.cmd_type = MTR_SET_APERTURE_POSITION;
			motor_iris.ctl_cmd.argu[0] = 1;
			vendor_isp_set_common(ISPT_ITEM_MOTOR_IRIS, &motor_iris);
			printf("set iris, %d, %d \n", motor_iris.ctl_cmd.argu[0], motor_iris.ctl_cmd.data[0]);
			break;

		case 61:
			motor_focus.cmd_type = MTR_SET_FOCUS_POSITION_IN_QUEUE;

			printf("Set focus pos >> \n");
			rt = scanf("%d", &fz_pos);
			if (rt != 1) {
				printf("read fail. \n");
				break;
			}

			motor_focus.ctl_cmd.argu[0] = fz_pos;
			vendor_isp_set_common(ISPT_ITEM_MOTOR_FOCUS, &motor_focus);
			printf("set focus, %d, %d \n", motor_focus.ctl_cmd.argu[0], motor_focus.ctl_cmd.data[0]);
			break;

		case 62:
			motor_zoom.cmd_type = MTR_SET_ZOOM_POSITION;

			printf("Set zoom pos >> \n");
			rt = scanf("%d", &fz_pos);
			if (rt != 1) {
				printf("read fail. \n");
				break;
			}

			motor_zoom.ctl_cmd.argu[0] = fz_pos;
			vendor_isp_set_common(ISPT_ITEM_MOTOR_ZOOM, &motor_zoom);
			printf("set zoom, %d, %d \n", motor_zoom.ctl_cmd.argu[0], motor_zoom.ctl_cmd.data[0]);
			break;

		case 63:
			sensor_direction.id = isp_id;
			printf("Set mirror (0: disable, 1: enable)>> \n");
			sensor_direction.direction.mirror = (UINT32)get_choose_int();
			printf("Set flip (0: disable, 1: enable)>> \n");
			sensor_direction.direction.flip = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_SENSOR_DIRECTION, &sensor_direction);
			break;

		case 64:
			motor_iris.cmd_type = MTR_SET_APERTURE_INIT;
			vendor_isp_set_common(ISPT_ITEM_MOTOR_IRIS, &motor_iris);
			printf("SET APERTURE_INIT  \n");
			break;

		case 65:
			motor_zoom.cmd_type = MTR_SET_ZOOM_INIT;
			vendor_isp_set_common(ISPT_ITEM_MOTOR_ZOOM, &motor_zoom);
			printf("SET ZOOM_INIT  \n");
			break;

		case 83:
			sensor_expt.id = isp_id;
			printf("Set exposure time (us)>> \n");
			sensor_expt.time[0] = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_SENSOR_EXPT, &sensor_expt);
			break;

		case 84:
			sensor_gain.id = isp_id;
			printf("Set gain ratio (1000 = 1X)>> \n");
			sensor_gain.ratio[0] = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_SENSOR_GAIN, &sensor_gain);
			break;

		case 85:
			d_gain.id = isp_id;
			printf("Set d gain(128 = 1X)>> \n");
			d_gain.gain = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_D_GAIN, &d_gain);
			break;

		case 86:
			c_gain.id = isp_id;
			printf("Set r gain(256 = 1X)>> \n");
			c_gain.gain[0] = (UINT32)get_choose_int();
			printf("Set g gain(256 = 1X)>> \n");
			c_gain.gain[1] = (UINT32)get_choose_int();
			printf("Set b gain(256 = 1X)>> \n");
			c_gain.gain[2] = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
			break;

		case 87:
			total_gain.id = isp_id;
			printf("Set total gain(100 = 1X)>> \n");
			total_gain.gain = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			break;

		case 88:
			lv.id = isp_id;
			printf("Set lv(100 = 1X)>> \n");
			lv.lv = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_LV, &lv);
			break;

		case 89:
			ct.id = isp_id;
			printf("Set ct>> \n");
			ct.ct = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_CT, &ct);
			break;

		case 90:
			ca_roi.id = isp_id;
			printf("Set x (0, 1000)>> \n");
			ca_roi.roi.x = (UINT32)get_choose_int();
			printf("Set y (0, 1000)>> \n");
			ca_roi.roi.y = (UINT32)get_choose_int();
			printf("Set w (0, 1000)>> \n");
			ca_roi.roi.w = (UINT32)get_choose_int();
			printf("Set h (0, 1000)>> \n");
			ca_roi.roi.h = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_CA_ROI, &ca_roi);
			break;

		case 91:
			la_roi.id = isp_id;
			printf("Set x (0, 1000)>> \n");
			la_roi.roi.x = (UINT32)get_choose_int();
			printf("Set y (0, 1000)>> \n");
			la_roi.roi.y = (UINT32)get_choose_int();
			printf("Set w (0, 1000)>> \n");
			la_roi.roi.w = (UINT32)get_choose_int();
			printf("Set h (0, 1000)>> \n");
			la_roi.roi.h = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_LA_ROI, &la_roi);
			printf("ca roi = %d, %d, %d, %d \n", la_roi.roi.x, la_roi.roi.y, la_roi.roi.w, la_roi.roi.h);
			break;

		case 92:
			va_indep_roi.id = isp_id;
			printf("0. IFE_VA (before 2DNR/3DNR)\n");
			printf("1. IPE_VA (after 2DNR, before 3DNR)\n");
			printf("Select display type >> \n");
			va_type = (UINT32)get_choose_int();
			printf("Set x (0, 1000)>> \n");
			va_indep_roi.roi[0].x = (UINT32)get_choose_int();
			printf("Set y (0, 1000)>> \n");
			va_indep_roi.roi[0].y = (UINT32)get_choose_int();
			printf("Set w (0, 1000)>> \n");
			va_indep_roi.roi[0].w = (UINT32)get_choose_int();
			printf("Set h (0, 1000)>> \n");
			va_indep_roi.roi[0].h = (UINT32)get_choose_int();

			if (va_type == 0) {
				vendor_isp_set_common(ISPT_ITEM_IFE_VA_INDEP_ROI, &va_indep_roi);
				printf("ife_va roi[0] = %d, %d, %d, %d \n", va_indep_roi.roi[0].x, va_indep_roi.roi[0].y, va_indep_roi.roi[0].w, va_indep_roi.roi[0].h);
			} else if (va_type == 1) {
				vendor_isp_set_common(ISPT_ITEM_VA_INDEP_ROI, &va_indep_roi);
				printf("ipe_va roi[0] = %d, %d, %d, %d \n", va_indep_roi.roi[0].x, va_indep_roi.roi[0].y, va_indep_roi.roi[0].w, va_indep_roi.roi[0].h);
			} else {
				vendor_isp_set_common(ISPT_ITEM_VA_INDEP_ROI, &va_indep_roi);
				printf("ipe_va roi[0] = %d, %d, %d, %d \n", va_indep_roi.roi[0].x, va_indep_roi.roi[0].y, va_indep_roi.roi[0].w, va_indep_roi.roi[0].h);
			}
			break;

		case 93:
			ca_enable.id = isp_id;
			printf("Set ca enable (0, 1)>> \n");
			ca_enable.enable = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_CA_ENABLE, &ca_enable);
			break;

		case 94:
			la_enable.id = isp_id;
			printf("Set la enable (0, 1)>> \n");
			la_enable.enable = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_LA_ENABLE, &la_enable);
			break;

		case 96:
			vprc_indep.id = isp_id;
			printf("Set vprc indep (0, 1)>> \n");
			vprc_indep.enable = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_VPRC_INDEP, &vprc_indep);
			break;

		case 500:
			sensor_mode_manual.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_SENSOR_MODE_MANUAL, &sensor_mode_manual);
			printf("enable = %d, mode_type = %d, frame_num = %d, data_fmt = %d \n", sensor_mode_manual.manual.enable, sensor_mode_manual.manual.mode_type, sensor_mode_manual.manual.frame_num, sensor_mode_manual.manual.data_fmt);
			break;

		case 501:
			vendor_isp_get_common(ISPT_ITEM_LOW_POWER_LV, &low_power_lv);
			printf("low_power_lv = %d \n", low_power_lv);
			break;

		case 502:
			bnr_sta_info.id = isp_id;

			vendor_isp_get_common(ISPT_ITEM_BNR_STA, &bnr_sta_info);
			printf("id = %d \n", bnr_sta_info.id);
			printf("info_vaild = %d \n", bnr_sta_info.bnr_sta_info.info_vaild);
			printf("enable = %d \n", bnr_sta_info.bnr_sta_info.enable);
			printf("buf_addr = 0x%x \n", bnr_sta_info.bnr_sta_info.buf_addr);
			printf("buf_phyaddr = 0x%x \n", bnr_sta_info.bnr_sta_info.buf_phyaddr);
			printf("lofs = %d \n", bnr_sta_info.bnr_sta_info.lofs);
			break;

		case 600:
			sensor_mode_manual.id = isp_id;
			printf("Set enable (0, 1)>> \n");
			sensor_mode_manual.manual.enable = (UINT32)get_choose_int();
			printf("Set mode_type>> \n");
			sensor_mode_manual.manual.mode_type = (UINT32)get_choose_int();
			printf("Set frame_num>> \n");
			sensor_mode_manual.manual.frame_num = (UINT32)get_choose_int();
			printf("Set data_fmt)>> \n");
			sensor_mode_manual.manual.data_fmt = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_SENSOR_MODE_MANUAL, &sensor_mode_manual);
			break;

		case 601:
			printf("Set low_power_lv (0, 5)>> \n");
			low_power_lv = (UINT32)get_choose_int();

			vendor_isp_set_common(ISPT_ITEM_LOW_POWER_LV, &low_power_lv);
			break;

		case 999:
				printf("Set isp id (0, %d)>> \n", ISP_ID_MAX_NUM - 1);
				isp_id = (UINT32)get_choose_int();
			break;

		default:
			printf("wrong input (%d) \n", option);
			break;

		case 0:
			trig = 0;
			break;
		}
	}

	if (vendor_isp_uninit() == HD_ERR_NG) {
		return -1;
	}

	return 0;
}

