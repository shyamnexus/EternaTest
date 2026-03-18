#include <string.h>
#include <pthread.h>

#include <kwrap/file.h>

#include "vendor_isp.h"
#include "vendor_common.h"

#define USER_EXPAND_DPC_TEST 0
#define USER_EXPAND_BIN_0    "/mnt/sd/dp.bin"

//#define USER_FPN_RAW_64         "/mnt/sd/imx485_fpn_iso_6400_12bit_w3840_h2160.raw"
//#define USER_FPN_RAW_1024       "/mnt/sd/imx485_fpn_iso_102400_12b_w3840_h2160.raw"
//#define USER_FPN_RAW_2048       "/mnt/sd/imx485_fpn_iso_204800_12b_w3840_h2160.raw"
//#define USER_FPN_RAW_1024       "/mnt/sd/imx485_fpn_iso_102400_12bit_2byte_crop_4M.raw"
#define USER_FPN_RAW_2048       "/mnt/sd/os04a10_no_dpc_w2688_h1520_12b_pack_exp_30000_iso204800.fpn"
//#define USER_FPN_RAW_1024       "/mnt/sd/imx485_fpn_iso_102400_12bit_2byte_crop_2M_.raw"
//#define USER_FPN_RAW_2048       "/mnt/sd/imx485_fpn_iso_204800_12bit_2byte_crop_2M_.raw"

#define USER_EXPAND_DPC_BUFFER_SIZE (160000 * 4 + 3 * 4)
#define USER_EXPAND_DPC_X 300
#define USER_EXPAND_DPC_Y 300
#define USER_EXPAND_DPC_W 400
#define USER_EXPAND_DPC_H 400

typedef struct _DDR_INFO {
	void *va;
	UINTPTR pa;
	UINT32 size;
} DDR_INFO;

//============================================================================
// global
//============================================================================
pthread_t iq_tonelv_thread_id;
static UINT32 iq_tonelv_conti_run = 1;

BOOL tone_level_en = FALSE;
BOOL tone_level_dbg = FALSE;

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

static INT32 iq_intpl(INT32 index, INT32 l_value, INT32 h_value, INT32 l_index, INT32 h_index)
{
	INT32 range = h_index - l_index;

	if (l_value == h_value) {
		return l_value;
	} else if (index <= l_index) {
		return l_value;
	} else if (index >= h_index) {
		return h_value;
	}
	if (h_value < l_value) {
		return l_value + ((h_value - l_value) * (index  - l_index) - (range >> 1)) / range;
	} else {
		return l_value + ((h_value - l_value) * (index  - l_index) + (range >> 1)) / range;
	}
}

static void *iq_tonelv_thread(void *arg)
{
	AET_STATUS_INFO ae_status = {0};
	IQT_DR_LEVEL dr_level = {0};
	IQT_TONE_LV tone_lv = {0};
	UINT32 tonelv_max_th_l_iso  =  400, tonelv_max_th_h_iso   = 1600;
	INT32  tonelv_max_th_l_value = 100, tonelv_max_th_h_value =   50;
	INT32 curr_tonelv_max;
	UINT32 tonelv_th_l_dr   = 20, tonelv_th_m_dr    = 50, tonelv_th_h_dr    = 100;
	INT32 tonelv_th_l_value =  0, tonelv_th_m_value = 50, tonelv_th_h_value = 100;
	INT32 smooth_factor = 7;

	static BOOL first_run = TRUE;
	static UINT32 smooth_dr_level = 50;

	UINT32 *id = (UINT32 *)arg;

	ae_status.id = *id;
	dr_level.id = *id;
	tone_lv.id = *id;

	if (tone_level_en == TRUE) {
		while (iq_tonelv_conti_run) {
			// Get current dr level, and smooth it
			vendor_isp_get_iq(IQT_ITEM_DR_LEVEL, &dr_level);
			if (first_run) {
				smooth_dr_level = dr_level.dr_level;
				first_run = FALSE;
			}
			if (smooth_dr_level <= dr_level.dr_level) {
				smooth_dr_level = (smooth_dr_level * smooth_factor + dr_level.dr_level * 1 + smooth_factor) / (smooth_factor + 1); // Unconditional carry
			} else {
				smooth_dr_level = (smooth_dr_level * smooth_factor + dr_level.dr_level * 1) / (smooth_factor + 1); // Unconditional chop
			}

			// Get shdr tone level setting
			if (smooth_dr_level <= tonelv_th_m_dr) {
				tone_lv.lv = iq_intpl(smooth_dr_level, tonelv_th_l_value, tonelv_th_m_value, tonelv_th_l_dr, tonelv_th_m_dr);
			} else {
				tone_lv.lv = iq_intpl(smooth_dr_level, tonelv_th_m_value, tonelv_th_h_value, tonelv_th_m_dr, tonelv_th_h_dr);
			}

			// Get max boundary of tone level
			vendor_isp_get_ae(AET_ITEM_STATUS, &ae_status);
			curr_tonelv_max = iq_intpl(ae_status.status_info.iso_gain[1], tonelv_max_th_l_value, tonelv_max_th_h_value, tonelv_max_th_l_iso, tonelv_max_th_h_iso);
			if (tone_lv.lv > (INT32)curr_tonelv_max) {
				tone_lv.lv = curr_tonelv_max;
			}

			vendor_isp_set_iq(IQT_ITEM_TONE_LV, &tone_lv);
			if (tone_level_dbg == TRUE) {
				printf("dr_level smooth = %d (curr = %d) \n", smooth_dr_level, dr_level.dr_level);
				printf("tone level = %d (max = %d) \n", tone_lv.lv, curr_tonelv_max);
			}
			sleep(1);
		}
	} else {
		tone_lv.lv = 50;
		vendor_isp_set_iq(IQT_ITEM_TONE_LV, &tone_lv);
		printf("tone id = %d \n", tone_lv.id);
		printf("tone level = %d \n", tone_lv.lv);
	}

	return 0;
}

static BOOL hd_mem_init = FALSE;
static IQT_DPC_PARAM dpc = {0};
static IQT_SHADING_PARAM shading = {0};
static IQT_SHADING_EXT_PARAM shading_ext = {0};
static UINT32 isp_id = 0;

int main(int argc, char *argv[])
{
	INT32 option;
	UINT32 trig = 1;
	UINT32 tmp = 0;
	UINT32 version = 0;
	IQT_CFG_INFO cfg_info = {0};
	IQT_DTSI_INFO dtsi_info = {0};
	IQT_NR_LV nr_lv = {0};
	IQT_3DNR_LV nr3d_lv = {0};
	IQT_SHARPNESS_LV sharpness_lv = {0};
	IQT_SATURATION_LV saturation_lv = {0};
	IQT_CONTRAST_LV contrast_lv = {0};
	IQT_BRIGHTNESS_LV brightness_lv = {0};
	IQT_NIGHT_MODE night_mode = {0};
	IQT_YCC_FORMAT ycc_format = {0};
	IQT_OPERATION operation = {0};
	IQT_IMAGEEFFECT imageeffect = {0};
	IQT_CCID ccid = {0};
	IQT_HUE_SHIFT hue_shift = {0};
	IQT_TONE_LV tone_lv = {0};
	IQT_GAMMA_LV gamma_lv = {0};
	IQT_OB_PARAM ob = {0};
	IQT_NR_PARAM nr_2d = {0};
	IQT_CFA_PARAM cfa = {0};
	IQT_RAW_VA_PARAM raw_va = {0};
	IQT_VA_PARAM va = {0};
	IQT_GAMMA_PARAM gamma = {0};
	IQT_CCM_PARAM ccm = {0};
	IQT_COLOR_PARAM color = {0};
	IQT_CONTRAST_PARAM contrast = {0};
	IQT_EDGE_PARAM edge = {0};
	IQT_3DNR_PARAM nr_3d = {0};
	IQT_PFR_PARAM pfr = {0};
	IQT_WDR_PARAM wdr = {0};
	IQT_DEFOG_PARAM defog = {0};
	IQT_SHDR_PARAM shdr = {0};
	IQT_RGBIR_PARAM rgbir = {0};
	IQT_COMPANDING_PARAM companding = {0};
	IQT_SHDR_MODE shdr_mode = {0};
	IQT_3DNR_MISC_PARAM nr_3d_misc = {0};
	IQT_DR_LEVEL dr_level = {0};
	IQT_RGBIR_ENH_PARAM rgbir_enh = {0};
	IQT_ENH_ISO enh_iso = {0};
	IQT_POST_SHARPEN_2_PARAM post_sharpen_2 = {0};
	IQT_CST_PARAM cst = {0};
	IQT_YCURVE_PARAM ycurve = {0};
	IQT_POST_SHARPEN_1_PARAM post_sharpen_1 = {0};
	IQT_TONE_PARAM tone = {0};
	IQT_EXPAND_DPC_PARAM expand_dpc = {0};
	IQT_WDR_ENH_PARAM wdr_enh = {0};
	IQT_FPN_PARAM fpn = {0};
	IQT_3DCC_PARAM _3dcc = {0};
	IQT_3DCC_EXT_PARAM _3dcc_ext = {0};
	IQT_FIXTH_PARAM fixth = {0};
	IQT_OB_MODE_MANUAL ob_mode_manual = {0};
	IQT_BNR_PARAM bnr = {0};
	IQT_DG_MODE_MANUAL dg_mode_manual = {0};
	IQT_AIISP_PARAM aiisp = {0};
	IQT_CG_MODE_MANUAL cg_mode_manual = {0};

	IQT_DARK_ENH_RATIO dark_enh_ratio = {0};
	IQT_CONTRAST_ENH_RATIO contrast_enh_ratio = {0};
	IQT_GREEN_ENH_RATIO green_enh_ratio = {0};
	IQT_SKIN_ENH_RATIO skin_enh_ratio = {0};

	VENDOR_COMM_MAX_FREE_BLOCK max_free_block = {0};
	DDR_INFO dpc_buffer = {0};
	DDR_INFO fpn_buffer_64 = {0};
	DDR_INFO fpn_buffer_1024 = {0};
	DDR_INFO fpn_buffer_2048 = {0};

	BOOL ecs_fisheye_en  = FALSE;
	UINT32 w, h, i, j, idx;

	HD_RESULT ret, file_ret;

	// open MCU device
	if (vendor_isp_init() == HD_ERR_NG) {
		return -1;
	}

	while (trig) {
		printf("----------------------------------------\r\n");
		printf("   1. Get version \n");
		printf("   2. Reload config file \n");
		printf("   3. Reload dtsi file \n");
		printf("  10. Get nr level \n");
		printf("  11. Get sharpness level \n");
		printf("  12. Get saturation level \n");
		printf("  13. Get contrast level \n");
		printf("  14. Get brightness level \n");
		printf("  15. Get nighe mode \n");
		printf("  16. Get ycc format \n");
		printf("  17. Get operation \n");
		printf("  18. Get imageeffect \n");
		printf("  19. Get ccid \n");
		printf("  20. Get hue_shift \n");
		printf("  21. Get tone_lv \n");
		printf("  22. Get nr3d level \n");
		printf("  23. Get gamma_lv \n");
		printf("  31. Get ob \n");
		printf("  32. Get nr_2d \n");
		printf("  33. Get cfa \n");
		printf("  34. Get va \n");
		printf("  35. Get gamma \n");
		printf("  36. Get ccm \n");
		printf("  37. Get color \n");
		printf("  38. Get contrast \n");
		printf("  39. Get edge \n");
		printf("  40. Get nr_3d \n");
		printf("  41. Get dpc \n");
		printf("  42. Get shading \n");
		printf("  43. Get raw_va \n");
		printf("  44. Get pfr \n");
		printf("  45. Get wdr \n");
		printf("  46. Get defog \n");
		printf("  47. Get shdr \n");
		printf("  48. Get rgbir \n");
		printf("  49. Get companding \n");
		printf("  52. Get shdr_mode \n");
		printf("  53. Get 3dnr misc param \n");
		printf("  55. Get dr_level \n");
		printf("  56. Get rgbir_enh \n");
		printf("  57. Get rgbir_enh_iso \n");
		printf("  59. Get post_sharpen_2 \n");
		printf("  60. Get shading_ext \n");
		printf("  61. Get CST \n");
		printf("  63. Get YCURVE \n");
		printf("  64. Get post_va \n");
		printf("  65. Get post_sharpen_1 \n");
		printf("  70. Get tone \n");
		printf("  71. Get expand_dpc \n");
		printf("  72. Get wdr_enh \n");
		printf("  77. Get fpn \n");
		printf("  78. Get 3dcc \n");
		printf("  79. Get 3dcc_ect \n");
		printf("  81. Get fixth \n");
		printf("  82. Get ob_mode_manual \n");
		printf("  83. Get bnr \n");
		printf("  84. Get dg_mode_manual \n");
		printf("  85. Get aiisp \n");
		printf("  86. Get cg_mode_manual \n");
		printf(" 110. Set nr level \n");
		printf(" 111. Set sharpness level \n");
		printf(" 112. Set saturation level \n");
		printf(" 113. Set contrast level \n");
		printf(" 114. Set brightness level \n");
		printf(" 115. Set nighe mode \n");
		printf(" 116. Set ycc format \n");
		printf(" 117. Set operation \n");
		printf(" 118. Set imageeffect \n");
		printf(" 119. Set ccid \n");
		printf(" 120. Set hue_shift \n");
		printf(" 121. Set tone_lv \n");
		printf(" 122. Set nr3d level \n");
		printf(" 123. Set gamma_lv \n");
		printf(" 141. Set dpc \n");
		printf(" 142. Set shading \n");
		printf(" 152. Set shdr_mode \n");
		printf(" 153. Set 3dnr misc param \n");
		printf(" 160. set shading_ext \n");
		printf(" 161. Set CST \n");
		printf(" 171. Set expand_dpc \n");
		printf(" 172. Set wdr_enh \n");
		printf(" 177. Set fpn \n");
		printf(" 181. Set fixth \n");
		printf(" 182. Set ob_mode_manual \n");
		printf(" 183. Get bnr \n");
		printf(" 184. Set dg_mode_manual \n");
		printf(" 186. Set cg_mode_manual \n");
		printf(" 200. Get dark enhance ratio \n");
		printf(" 201. Get contrast enhance ratio \n");
		printf(" 202. Get green enhance ratio \n");
		printf(" 203. Get skin enhance ratio \n");
		printf(" 300. Set dark enhance ratio \n");
		printf(" 301. Set contrast enhance ratio \n");
		printf(" 302. Set green enhance ratio \n");
		printf(" 303. Set skin enhance ratio \n");
		printf(" 500. Set auto tone level enable \n");
		printf("----------------------------------------\n");
		printf(" 999. Set id, current isp_id: %d \n", isp_id);
		printf("----------------------------------------\n");
		printf("   0. Quit\n");
		printf("----------------------------------------\n");

		printf(">> ");
		option = get_choose_int();

		switch (option) {
		case 1:
			vendor_isp_get_iq(IQT_ITEM_VERSION, &version);
			printf("version = 0x%X \n", version);
			break;

		case 2:
			cfg_info.id = isp_id;

			printf("Select chg file>> \n");
			printf("  1: isp_os04a10_0.cfg \n");
			printf("  2: isp_os04a10_0_hdr.cfg \n");
			tmp = (UINT32)get_choose_int();

			switch (tmp) {
			case 1:
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os04a10_0.cfg", CFG_NAME_LENGTH);
				break;

			case 2:
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os04a10_0_hdr.cfg", CFG_NAME_LENGTH);
				break;

			default:
				printf("Not support item (%d) \n", tmp);
				break;
			}
			vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
			break;

		case 3:
			dtsi_info.id = isp_id;

			printf("Select dtsi file>> \n");
			printf("1: os04a10_iq_0.dtsi \n");
			printf("2: os04a10_iq_dpc_0.dtsi \n");
			printf("3: os04a10_iq_expand_dpc_0.dtsi \n");
			printf("4: os04a10_iq_shading_0.dtsi \n");
			tmp = (UINT32)get_choose_int();

			switch (tmp) {
			case 1:
				strncpy(dtsi_info.node_path, "/isp/iq/os04a10_iq_0", DTSI_NAME_LENGTH);
				break;

			case 2:
				strncpy(dtsi_info.node_path, "/isp/iq/os04a10_iq_dpc_0", DTSI_NAME_LENGTH);
				break;

			case 3:
				strncpy(dtsi_info.node_path, "/isp/iq/os04a10_iq_expand_dpc_0", DTSI_NAME_LENGTH);
				break;

			case 4:
				strncpy(dtsi_info.node_path, "/isp/iq/os04a10_iq_shading_0", DTSI_NAME_LENGTH);
				break;

			default:
				printf("Not support item (%d) \n", tmp);
				break;
			}

			strncpy(dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
			dtsi_info.buf_addr = NULL;
			vendor_isp_set_iq(IQT_ITEM_RLD_DTSI, &dtsi_info);
			break;

		case 10:
			nr_lv.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_NR_LV, &nr_lv);
			printf("nr id = %d \n", nr_lv.id);
			printf("nr level = %d \n", nr_lv.lv);
			break;

		case 11:
			sharpness_lv.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_SHARPNESS_LV, &sharpness_lv);
			printf("sharpness id = %d \n", sharpness_lv.id);
			printf("sharpness level = %d \n", sharpness_lv.lv);
			break;

		case 12:
			saturation_lv.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_SATURATION_LV, &saturation_lv);
			printf("saturation id = %d \n", saturation_lv.id);
			printf("saturation level = %d \n", saturation_lv.lv);
			break;

		case 13:
			contrast_lv.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_CONTRAST_LV, &contrast_lv);
			printf("contrast id = %d \n", contrast_lv.id);
			printf("contrast level = %d \n", contrast_lv.lv);
			break;

		case 14:
			brightness_lv.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_BRIGHTNESS_LV, &brightness_lv);
			printf("brightness id = %d \n", brightness_lv.id);
			printf("brightness level = %d \n", brightness_lv.lv);
			break;

		case 15:
			night_mode.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_NIGHT_MODE, &night_mode);
			printf("night id = %d \n", night_mode.id);
			printf("night mode = %d \n", night_mode.mode);
			break;

		case 16:
			ycc_format.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_YCC_FORMAT, &ycc_format);
			printf("ycc_format id = %d \n", ycc_format.id);
			printf("ycc_format format = %d \n", ycc_format.format);
			break;

		case 17:
			operation.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_OPERATION, &operation);
			printf("operation id = %d \n", operation.id);
			printf("operation sel = %d \n", operation.operation);
			break;

		case 18:
			imageeffect.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_IMAGEEFFECT, &imageeffect);
			printf("imageeffect id = %d \n", imageeffect.id);
			printf("imageeffect effect = %d \n", imageeffect.effect);
			break;

		case 19:
			ccid.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_CCID, &ccid);
			printf("color id = %d \n", ccid.id);
			printf("color ccid = %d \n", ccid.ccid);
			break;

		case 20:
			hue_shift.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_HUE_SHIFT, &hue_shift);
			printf("hue id = %d \n", hue_shift.id);
			printf("hue shift = %d \n", hue_shift.hue_shift);
			break;

		case 21:
			tone_lv.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_TONE_LV, &tone_lv);
			printf("tone id = %d \n", tone_lv.id);
			printf("tone level = %d \n", tone_lv.lv);
			break;

		case 22:
			nr3d_lv.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_3DNR_LV, &nr3d_lv);
			printf("3dnr id = %d \n", nr3d_lv.id);
			printf("3dnr level = %d \n", nr3d_lv.lv);
			break;

		case 23:
			gamma_lv.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_GAMMA_LV, &gamma_lv);
			printf("gamma id = %d \n", gamma_lv.id);
			printf("gamma level = %d \n", gamma_lv.lv);
			break;

		case 31:
			ob.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_OB_PARAM, &ob);
			printf("ob id = %d \n", ob.id);
			printf("ob mode = %d \n", ob.ob.mode);
			printf("ob manual_param.cofs = {%d, %d, %d, %d, %d} \n", ob.ob.manual_param.cofs[0], ob.ob.manual_param.cofs[1], ob.ob.manual_param.cofs[2], ob.ob.manual_param.cofs[3], ob.ob.manual_param.cofs[4]);
			printf("ob auto_param[0].cofs = {%d, %d, %d, %d, %d} \n", ob.ob.auto_param[0].cofs[0], ob.ob.auto_param[0].cofs[1], ob.ob.auto_param[0].cofs[2], ob.ob.auto_param[0].cofs[3], ob.ob.auto_param[0].cofs[4]);
			break;

		case 32:
			nr_2d.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_NR_PARAM, &nr_2d);
			printf("nr_2d id = %d \n", nr_2d.id);
			printf("nr_2d outl_enable = %d \n", nr_2d.nr.outl_enable);
			printf("nr_2d gbal_enable = %d \n", nr_2d.nr.gbal_enable);
			printf("nr_2d filter_enable = %d \n", nr_2d.nr.filter_enable);
			printf("nr_2d lca_enable = %d \n", nr_2d.nr.lca_enable);
			printf("nr_2d dbcs_enable = %d \n", nr_2d.nr.dbcs_enable);
			printf("nr_2d mode = %d \n", nr_2d.nr.mode);
			break;

		case 33:
			cfa.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_CFA_PARAM, &cfa);
			printf("cfa id = %d \n", cfa.id);
			printf("cfa mode = %d \n", cfa.cfa.mode);
			printf("cfa manual_param.edge_dth = %d \n", cfa.cfa.manual_param.edge_dth);
			printf("cfa manual_param.edge_dth2 = %d \n", cfa.cfa.manual_param.edge_dth2);
			printf("cfa auto_param[0].edge_dth = %d \n", cfa.cfa.auto_param[0].edge_dth);
			printf("cfa auto_param[0].edge_dth2 = %d \n", cfa.cfa.auto_param[0].edge_dth2);
			break;

		case 34:
			va.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_VA_PARAM, &va);
			printf("va id = %d \n", va.id);
			printf("va pre_filter_enable = %d \n", va.va.pre_filter_enable);
			printf("va mode = %d \n", va.va.mode);
			printf("va g1_tap_a = %d \n", va.va.g1_tap_a);
			printf("va g1_tap_b = %d \n", va.va.g1_tap_b);
			printf("va g1_tap_c = %d \n", va.va.g1_tap_c);
			printf("va g1_tap_d = %d \n", va.va.g1_tap_d);
			printf("va g1_div = %d \n", va.va.g1_div);
			printf("va g2_tap_a = %d \n", va.va.g2_tap_a);
			printf("va g2_tap_b = %d \n", va.va.g2_tap_b);
			printf("va g2_tap_c = %d \n", va.va.g2_tap_c);
			printf("va g2_tap_d = %d \n", va.va.g2_tap_d);
			printf("va g2_div = %d \n", va.va.g2_div);
			break;

		case 35:
			gamma.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_GAMMA_PARAM, &gamma);
			printf("gamma id = %d \n", gamma.id);
			printf("gamma enable = %d \n", gamma.gamma.enable);
			printf("gamma mode = %d \n", gamma.gamma.mode);
			break;

		case 36:
			ccm.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_CCM_PARAM, &ccm);
			printf("ccm id = %d \n", ccm.id);
			printf("ccm enable = %d \n", ccm.ccm.enable);
			printf("ccm mode = %d \n", ccm.ccm.mode);
			printf("ccm manual_param.coef = {%d, %d, %d, %d, %d, %d, %d, %d, %d} \n"
				, ccm.ccm.manual_param.coef[0], ccm.ccm.manual_param.coef[1], ccm.ccm.manual_param.coef[2]
				, ccm.ccm.manual_param.coef[3], ccm.ccm.manual_param.coef[4], ccm.ccm.manual_param.coef[5]
				, ccm.ccm.manual_param.coef[6], ccm.ccm.manual_param.coef[7], ccm.ccm.manual_param.coef[8]);
			printf("ccm auto_param[0].coef = {%d, %d, %d, %d, %d, %d, %d, %d, %d} \n"
				, ccm.ccm.auto_param[0].coef[0], ccm.ccm.auto_param[0].coef[1], ccm.ccm.auto_param[0].coef[2]
				, ccm.ccm.auto_param[0].coef[3], ccm.ccm.auto_param[0].coef[4], ccm.ccm.auto_param[0].coef[5]
				, ccm.ccm.auto_param[0].coef[6], ccm.ccm.auto_param[0].coef[7], ccm.ccm.auto_param[0].coef[8]);
			break;

		case 37:
			color.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_COLOR_PARAM, &color);
			printf("color id = %d \n", color.id);
			printf("color mode = %d \n", color.color.mode);
			printf("color manual_param.c_con = %d \n", color.color.manual_param.c_con);
			printf("color auto_param[0].c_con = %d \n", color.color.auto_param[0].c_con);
			break;

		case 38:
			contrast.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_CONTRAST_PARAM, &contrast);
			printf("contrast id = %d \n", contrast.id);
			printf("contrast lce_enable = %d \n", contrast.contrast.lce_enable);
			printf("contrast mode = %d \n", contrast.contrast.mode);
			printf("contrast manual_param.y_con = %d \n", contrast.contrast.manual_param.y_con);
			printf("contrast auto_param[0].y_con = %d \n", contrast.contrast.auto_param[0].y_con);
			break;

		case 39:
			edge.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_EDGE_PARAM, &edge);
			printf("edge id = %d \n", edge.id);
			printf("edge mode = %d \n", edge.edge.mode);
			printf("edge th_overshoot = %d \n", edge.edge.th_overshoot);
			printf("edge th_undershoot = %d \n", edge.edge.th_undershoot);
			break;

		case 40:
			nr_3d.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_3DNR_PARAM, &nr_3d);
			printf("nr_3d id = %d \n", nr_3d.id);
			printf("nr_3d enable = %d \n", nr_3d._3dnr.enable);
			printf("nr_3d fcvg_enable = %d \n", nr_3d._3dnr.fcvg_enable);
			printf("nr_3d mode = %d \n", nr_3d._3dnr.mode);
			printf("nr_3d manual_param.pf_str = %d \n", nr_3d._3dnr.manual_param.pf_str);
			printf("nr_3d manual_param.cost_blend = %d \n", nr_3d._3dnr.manual_param.cost_blend);
			printf("nr_3d auto_param[0].pf_str = %d \n", nr_3d._3dnr.auto_param[0].pf_str);
			printf("nr_3d auto_param[0].cost_blend = %d \n", nr_3d._3dnr.auto_param[0].cost_blend);
			break;

		case 41:
			dpc.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc);
			printf("dpc id = %d \n", dpc.id);
			printf("dpc enable = %d \n", dpc.dpc.enable);
			printf("dpc table = {0x%x, ...} \n", dpc.dpc.table[0]);
			break;

		case 42:
			shading.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_SHADING_PARAM, &shading);
			printf("shading id = %d \n", shading.id);
			printf("shading ecs_enable = %d \n", shading.shading.ecs_enable);
			printf("shading vig_enable = %d \n", shading.shading.vig_enable);
			printf("shading mode = %d \n", shading.shading.mode);
			printf("shading ecs_dthr_enable = %d \n", shading.shading.ecs_dthr_enable);
			printf("shading ecs_smooth_l_m_ct_lower = %d \n", shading.shading.ecs_smooth_l_m_ct_lower);
			printf("shading ecs_smooth_l_m_ct_upper = %d \n", shading.shading.ecs_smooth_l_m_ct_upper);
			printf("shading ecs_smooth_m_h_ct_lower = %d \n", shading.shading.ecs_smooth_m_h_ct_lower);
			printf("shading ecs_smooth_m_h_ct_upper = %d \n", shading.shading.ecs_smooth_m_h_ct_upper);
			break;

		case 43:
			raw_va.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_RAW_VA_PARAM, &raw_va);
			printf("raw_va id = %d \n", raw_va.id);
			printf("raw_va mode = %d \n", raw_va.raw_va.mode);
			printf("raw_va g1_h_filter_sel = %d \n", raw_va.raw_va.g1_h_filter_sel);
			printf("raw_va g1_fir_sym_sel = %d \n", raw_va.raw_va.g1_fir_sym_sel);
			printf("raw_va g1_fir_tap_a = %d \n", raw_va.raw_va.g1_fir_tap_a);
			printf("raw_va g1_fir_tap_b = %d \n", raw_va.raw_va.g1_fir_tap_b);
			printf("raw_va g1_fir_tap_c = %d \n", raw_va.raw_va.g1_fir_tap_c);
			printf("raw_va g1_fir_tap_d = %d \n", raw_va.raw_va.g1_fir_tap_d);
			printf("raw_va g1_fir_div = %d \n", raw_va.raw_va.g1_fir_div);
			printf("raw_va g2_h_filter_sel = %d \n", raw_va.raw_va.g2_h_filter_sel);
			printf("raw_va g2_fir_sym_sel = %d \n", raw_va.raw_va.g2_fir_sym_sel);
			printf("raw_va g2_fir_tap_a = %d \n", raw_va.raw_va.g2_fir_tap_a);
			printf("raw_va g2_fir_tap_b = %d \n", raw_va.raw_va.g2_fir_tap_b);
			printf("raw_va g2_fir_tap_c = %d \n", raw_va.raw_va.g2_fir_tap_c);
			printf("raw_va g2_fir_tap_d = %d \n", raw_va.raw_va.g2_fir_tap_d);
			printf("raw_va g2_fir_div = %d \n", raw_va.raw_va.g2_fir_div);
			break;

		case 44:
			pfr.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_PFR_PARAM, &pfr);
			printf("pfr id = %d \n", pfr.id);
			printf("pfr mode = %d \n", pfr.pfr.mode);
			printf("pfr luma_lut = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d} \n"
				, pfr.pfr.luma_lut[0], pfr.pfr.luma_lut[1], pfr.pfr.luma_lut[2], pfr.pfr.luma_lut[3], pfr.pfr.luma_lut[4]
				, pfr.pfr.luma_lut[5], pfr.pfr.luma_lut[6], pfr.pfr.luma_lut[7], pfr.pfr.luma_lut[8], pfr.pfr.luma_lut[9]
				, pfr.pfr.luma_lut[10], pfr.pfr.luma_lut[11], pfr.pfr.luma_lut[12]);
			break;

		case 45:
			wdr.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_WDR_PARAM, &wdr);
			printf("wdr id = %d \n", wdr.id);
			printf("wdr mode = %d \n", wdr.wdr.mode);
			printf("wdr subimg_size_h = %d \n", wdr.wdr.subimg_size_h);
			printf("wdr subimg_size_v = %d \n", wdr.wdr.subimg_size_v);
			break;

		case 46:
			defog.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_DEFOG_PARAM, &defog);
			printf("defog id = %d \n", defog.id);
			printf("defog mode = %d \n", defog.defog.mode);
			printf("defog outbld_local_en = %d \n", defog.defog.outbld_local_en);
			printf("defog outbld_diff_wt = %d \n", defog.defog.outbld_diff_wt);
			printf("defog min_diff_ratio = %d \n", defog.defog.min_diff_ratio);
			break;

		case 47:
			shdr.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_SHDR_PARAM, &shdr);
			printf("shdr id = %d \n", shdr.id);
			printf("shdr mode = %d \n", shdr.shdr.mode);
			break;

		case 48:
			rgbir.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_RGBIR_PARAM, &rgbir);
			printf("rgbir id = %d \n", rgbir.id);
			printf("rgbir mode = %d \n", rgbir.rgbir.mode);
			printf("rgbir manual_param.irsub_r_weight = %d \n", rgbir.rgbir.manual_param.irsub_r_weight);
			printf("rgbir manual_param.irsub_g_weight = %d \n", rgbir.rgbir.manual_param.irsub_g_weight);
			printf("rgbir manual_param.irsub_b_weight = %d \n", rgbir.rgbir.manual_param.irsub_b_weight);
			printf("rgbir auto_param.irsub_r_weight = %d \n", rgbir.rgbir.auto_param.irsub_r_weight);
			printf("rgbir auto_param.irsub_g_weight = %d \n", rgbir.rgbir.auto_param.irsub_g_weight);
			printf("rgbir auto_param.irsub_b_weight = %d \n", rgbir.rgbir.auto_param.irsub_b_weight);
			break;

		case 49:
			companding.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_COMPANDING_PARAM, &companding);
			printf("companding id = %d \n", companding.id);
			printf("companding decomp_kpx = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d \n"
				, companding.companding.decomp_kpx[0], companding.companding.decomp_kpx[1], companding.companding.decomp_kpx[2]
				, companding.companding.decomp_kpx[3], companding.companding.decomp_kpx[4], companding.companding.decomp_kpx[5]
				, companding.companding.decomp_kpx[6], companding.companding.decomp_kpx[7], companding.companding.decomp_kpx[8]
				, companding.companding.decomp_kpx[9], companding.companding.decomp_kpx[10]);
			break;

		case 52:
			shdr_mode.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_SHDR_MODE, &shdr_mode);
			printf("shdr_mode id = %d \n", shdr_mode.id);
			printf("shdr_mode mode = %d \n", shdr_mode.shdr_mode);
			break;

		case 53:
			nr_3d_misc.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_3DNR_MISC_PARAM, &nr_3d_misc);
			printf("nr_3d_misc id = %d \n", nr_3d_misc.id);
			printf("nr_3d_misc md_roi = %d, %d \n", nr_3d_misc._3dnr_misc.md_roi[0], nr_3d_misc._3dnr_misc.md_roi[1]);
			printf("nr_3d_misc mc_roi = %d, %d \n", nr_3d_misc._3dnr_misc.mc_roi[0], nr_3d_misc._3dnr_misc.mc_roi[1]);
			printf("nr_3d_misc roi_mv_th = %d \n", nr_3d_misc._3dnr_misc.roi_mv_th);
			printf("nr_3d_misc ds_th_roi = %d \n", nr_3d_misc._3dnr_misc.ds_th_roi);
			break;

		case 55:
			dr_level.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_DR_LEVEL, &dr_level);
			printf("dr_level id = %d \n", dr_level.id);
			printf("dr_level dr_level = %d \n", dr_level.dr_level);
			break;

		case 56:
			rgbir_enh.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_RGBIR_ENH_PARAM, &rgbir_enh);
			printf("rgbir_enh id = %d \n", rgbir_enh.id);
			printf("rgbir_enh enable = %d \n", rgbir_enh.rgbir_enh.enable);
			printf("rgbir_enh mode = %d \n", rgbir_enh.rgbir_enh.mode);
			printf("rgbir_enh min_ir_th = %d \n", rgbir_enh.rgbir_enh.min_ir_th);
			break;

		case 57:
			enh_iso.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_ENH_ISO, &enh_iso);
			printf("enh_iso id = %d \n", enh_iso.id);
			printf("enh_iso enh_iso = %d \n", enh_iso.enh_iso);
			break;

		case 59:
			post_sharpen_2.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_POST_SHARPEN_2_PARAM, &post_sharpen_2);
			printf("post_sharpen_2 id = %d \n", post_sharpen_2.id);
			printf("post_sharpen_2 enable = %d \n", post_sharpen_2.post_sharpen_2.enable);
			printf("post_sharpen_2 mode = %d \n", post_sharpen_2.post_sharpen_2.mode);
			printf("post_sharpen_2 manual_param.noise_level = %d \n", post_sharpen_2.post_sharpen_2.manual_param.noise_level);
			printf("post_sharpen_2 auto_param[0].noise_level = %d \n", post_sharpen_2.post_sharpen_2.auto_param[0].noise_level);
			break;

		case 60:
			shading_ext.id = isp_id;

			printf("Set ecs id (0, 1, 2)>> \n");
			shading_ext.shading_ext_if.ecs_map_idx = (UINT32)get_choose_int();

			vendor_isp_get_iq(IQT_ITEM_SHADING_EXT_PARAM, &shading_ext);
			printf("shading_ext id = %d \n", shading_ext.id);
			printf("shading_ext ecs_map_idx = %d \n", shading_ext.shading_ext_if.ecs_map_idx);
			printf("shading_ext ecs_map_tbl[0] = 0x%x \n", shading_ext.shading_ext_if.ecs_map_tbl[0]);
			break;

		case 61:
			cst.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_CST_PARAM, &cst);
			printf("cst id = %d \n", cst.id);
			printf("cst mode = %d \n", cst.cst.mode);
			printf("cst cst_coef = {%d, %d, %d, %d, %d, %d, %d, %d, %d}, \n"
				, cst.cst.cst_coef[0], cst.cst.cst_coef[1], cst.cst.cst_coef[2], cst.cst.cst_coef[3], cst.cst.cst_coef[4]
				, cst.cst.cst_coef[5], cst.cst.cst_coef[6], cst.cst.cst_coef[7], cst.cst.cst_coef[8]);
			break;

		case 63:
			ycurve.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_YCURVE_PARAM, &ycurve);
			printf("ycurve id = %d \n", cst.id);
			printf("ycurve ycurve = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, ...}, \n"
				, ycurve.ycurve.ycurve_lut[0], ycurve.ycurve.ycurve_lut[1], ycurve.ycurve.ycurve_lut[2]
				, ycurve.ycurve.ycurve_lut[3], ycurve.ycurve.ycurve_lut[4], ycurve.ycurve.ycurve_lut[5]
				, ycurve.ycurve.ycurve_lut[6], ycurve.ycurve.ycurve_lut[7], ycurve.ycurve.ycurve_lut[8]
				, ycurve.ycurve.ycurve_lut[9]);
			break;

		case 65:
			post_sharpen_1.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_POST_SHARPEN_1_PARAM, &post_sharpen_1);
			printf("post_sharpen_1 id = %d \n", post_sharpen_1.id);
			printf("post_sharpen_1 enable = %d \n", post_sharpen_1.post_sharpen_1.enable);
			printf("post_sharpen_1 mode = %d \n", post_sharpen_1.post_sharpen_1.mode);
			printf("post_sharpen_1 manual_param.noise_level = %d \n", post_sharpen_1.post_sharpen_1.manual_param.noise_level);
			printf("post_sharpen_1 auto_param[0].noise_level = %d \n", post_sharpen_1.post_sharpen_1.auto_param[0].noise_level);
			break;

		case 70:
			tone.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_TONE_PARAM, &tone);
			printf("tone id = %d \n", tone.id);
			printf("tone enable = %d \n", tone.tone.enable);
			printf("tone mode = %d \n", tone.tone.mode);
			printf("tone tone_in_yv_blend_lut = {%d, %d, %d, %d, %d, %d, %d, %d, %d} \n"
				, tone.tone.tone_in_yv_blend_lut[0], tone.tone.tone_in_yv_blend_lut[1], tone.tone.tone_in_yv_blend_lut[2]
				, tone.tone.tone_in_yv_blend_lut[3], tone.tone.tone_in_yv_blend_lut[4], tone.tone.tone_in_yv_blend_lut[5]
				, tone.tone.tone_in_yv_blend_lut[6], tone.tone.tone_in_yv_blend_lut[7], tone.tone.tone_in_yv_blend_lut[8]);
			break;

		case 71:
			expand_dpc.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_EXPAND_DPC_PARAM, &expand_dpc);
			printf("expand_dpc id = %d \n", expand_dpc.id);
			printf("expand_dpc enable = %d \n", expand_dpc.expand_dpc.enable);
			printf("expand_dpc size = %d \n", expand_dpc.expand_dpc.size);
			printf("expand_dpc table_phyaddr =0x%lx \n", expand_dpc.expand_dpc.table_phyaddr);
			break;

		case 72:
			wdr_enh.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_WDR_ENH_PARAM, &wdr_enh);
			printf("wdr_enh id = %d \n", wdr_enh.id);
			printf("wdr_enh enable = %d \n", wdr_enh.wdr_enh.enable);
			printf("wdr_enh enh_ratio = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d} \n"
				, wdr_enh.wdr_enh.enh_ratio[0], wdr_enh.wdr_enh.enh_ratio[1], wdr_enh.wdr_enh.enh_ratio[2]
				, wdr_enh.wdr_enh.enh_ratio[3], wdr_enh.wdr_enh.enh_ratio[4], wdr_enh.wdr_enh.enh_ratio[5]
				, wdr_enh.wdr_enh.enh_ratio[6], wdr_enh.wdr_enh.enh_ratio[7], wdr_enh.wdr_enh.enh_ratio[8]
				, wdr_enh.wdr_enh.enh_ratio[9], wdr_enh.wdr_enh.enh_ratio[10], wdr_enh.wdr_enh.enh_ratio[11]
				, wdr_enh.wdr_enh.enh_ratio[12], wdr_enh.wdr_enh.enh_ratio[13], wdr_enh.wdr_enh.enh_ratio[14]
				, wdr_enh.wdr_enh.enh_ratio[15], wdr_enh.wdr_enh.enh_ratio[16]);
			break;

		case 77:
			fpn.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_FPN_PARAM, &fpn);
			printf("fpn id = %d \n", fpn.id);
			printf("fpn enable = %d \n", fpn.fpn.enable);
			printf("fpn mode = %d \n", fpn.fpn.mode);
			printf("fpn manual_param.buf_size = %d \n", fpn.fpn.manual_param.buf_size);
			printf("fpn manual_param.buf_phyaddr = %d \n", fpn.fpn.manual_param.buf_phyaddr);
			printf("fpn manual_param.gain = %d \n", fpn.fpn.manual_param.gain);
			break;

		case 78:
			_3dcc.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_3DCC_PARAM, &_3dcc);
			printf("3dcc id = %d \n", _3dcc.id);
			printf("3dcc enable = %d \n", _3dcc._3dcc.enable);
			printf("3dcc manual_3dcc_lut = {%d, %d, %d, %d, %d, %d, %d, %d, %d, ... , %d} \n"
				, _3dcc._3dcc.manual_3dcc_lut[0], _3dcc._3dcc.manual_3dcc_lut[1], _3dcc._3dcc.manual_3dcc_lut[2]
				, _3dcc._3dcc.manual_3dcc_lut[3], _3dcc._3dcc.manual_3dcc_lut[4], _3dcc._3dcc.manual_3dcc_lut[5]
				, _3dcc._3dcc.manual_3dcc_lut[6], _3dcc._3dcc.manual_3dcc_lut[7], _3dcc._3dcc.manual_3dcc_lut[8]
				, _3dcc._3dcc.manual_3dcc_lut[IQ_3DCC_LEN - 1]);
			break;

		case 79:
			_3dcc_ext.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_3DCC_EXT_PARAM, &_3dcc_ext);
			printf("3dcc_ext id = %d \n", _3dcc_ext.id);
			printf("3dcc_ext _3dcc_idx = %d \n", _3dcc_ext._3dcc_ext_if._3dcc_idx);
			printf("3dcc_ext _3dcc_lut = {%d, %d, %d, %d, %d, %d, %d, %d, %d, ... , %d} \n"
				, _3dcc_ext._3dcc_ext_if._3dcc_lut[0], _3dcc_ext._3dcc_ext_if._3dcc_lut[1], _3dcc_ext._3dcc_ext_if._3dcc_lut[2]
				, _3dcc_ext._3dcc_ext_if._3dcc_lut[3], _3dcc_ext._3dcc_ext_if._3dcc_lut[4], _3dcc_ext._3dcc_ext_if._3dcc_lut[5]
				, _3dcc_ext._3dcc_ext_if._3dcc_lut[6], _3dcc_ext._3dcc_ext_if._3dcc_lut[7], _3dcc_ext._3dcc_ext_if._3dcc_lut[8]
				, _3dcc_ext._3dcc_ext_if._3dcc_lut[IQ_3DCC_LEN - 1]);
			break;

		case 81:
			fixth.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_FIXTH_PARAM, &fixth);
			printf("fixth id = %d \n", fixth.id);
			printf("fixth enable = %d \n", fixth.fixth.enable);
			printf("fixth fixy_sel = %d \n", fixth.fixth.fixy_sel);
			printf("fixth fixy_y_sel = %d \n", fixth.fixth.fixy_y_sel);
			printf("fixth fixy_edge_th = %d \n", fixth.fixth.fixy_edge_th);
			printf("fixth fixy_y_th = %d \n", fixth.fixth.fixy_y_th);
			printf("fixth fixy_y_value = %d \n", fixth.fixth.fixy_y_value);
			printf("fixth fixc_y_th_low = %d \n", fixth.fixth.fixc_y_th_low);
			printf("fixth fixc_y_th_high = %d \n", fixth.fixth.fixc_y_th_high);
			printf("fixth fixc_cb_th_low = %d \n", fixth.fixth.fixc_cb_th_low);
			printf("fixth fixc_cb_th_high = %d \n", fixth.fixth.fixc_cb_th_high);
			printf("fixth fixc_cr_th_low = %d \n", fixth.fixth.fixc_cr_th_low);
			printf("fixth fixc_cr_th_high = %d \n", fixth.fixth.fixc_cr_th_high);
			printf("fixth fixc_cb_value = %d \n", fixth.fixth.fixc_cb_value);
			printf("fixth fixc_cr_value = %d \n", fixth.fixth.fixc_cr_value);
			break;

		case 82:
			ob_mode_manual.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_OB_MODE_MANUAL, &ob_mode_manual);

			printf("ob_mode_manual id = %d \n", ob_mode_manual.id);
			printf("sie en = %d, sie value = %d \n", ob_mode_manual.ob_mode_manual.sie_enable, ob_mode_manual.ob_mode_manual.sie_value);
			printf("ifef en = %d, ifef value = %d %d %d %d \n", ob_mode_manual.ob_mode_manual.ife_f_enable, ob_mode_manual.ob_mode_manual.ife_f_value[0], ob_mode_manual.ob_mode_manual.ife_f_value[1], ob_mode_manual.ob_mode_manual.ife_f_value[2], ob_mode_manual.ob_mode_manual.ife_f_value[3]);
			printf("ife en = %d, ife value = %d %d %d %d \n", ob_mode_manual.ob_mode_manual.ife_enable, ob_mode_manual.ob_mode_manual.ife_value[0], ob_mode_manual.ob_mode_manual.ife_value[1], ob_mode_manual.ob_mode_manual.ife_value[2], ob_mode_manual.ob_mode_manual.ife_value[3]);
			printf("manual_enable = %d, manual_mode = %d \n", ob_mode_manual.ob_mode_manual.manual_enable, ob_mode_manual.ob_mode_manual.manual_mode);
			break;

		case 83:
			bnr.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_BNR_PARAM, &bnr);
			printf("bnr id = %d \n", bnr.id);
			printf("enable = %d \n", bnr.bnr.enable);
			break;

		case 84:
			dg_mode_manual.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_DG_MODE_MANUAL, &dg_mode_manual);

			printf("dg_mode_manual id = %d \n", dg_mode_manual.id);
			printf("sie en = %d, sie value = %d \n", dg_mode_manual.dg_mode_manual.sie_enable, dg_mode_manual.dg_mode_manual.sie_value);
			printf("pre en = %d, pre value = %d \n", dg_mode_manual.dg_mode_manual.pre_enable, dg_mode_manual.dg_mode_manual.pre_value);
			printf("ife en = %d, ife value = %d \n", dg_mode_manual.dg_mode_manual.ife_enable, dg_mode_manual.dg_mode_manual.ife_value);
			printf("manual_enable = %d, manual_mode = %d \n", dg_mode_manual.dg_mode_manual.manual_enable, dg_mode_manual.dg_mode_manual.manual_mode);
			break;

		case 85:
			aiisp.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_AIISP_PARAM, &aiisp);
			printf("aiisp id = %d \n", aiisp.id);
			printf("path_id = %d, version = %d, param_num = %d \n", aiisp.aiisp.path_id, aiisp.aiisp.version, aiisp.aiisp.param_num);
			printf("param_size = %d, %d \n", aiisp.aiisp.param_size[0], aiisp.aiisp.param_size[1]);
			printf("param_name = %s, %s \n", aiisp.aiisp.param_name[0], aiisp.aiisp.param_name[1]);
			printf("param_manual.enable = %d \n", aiisp.aiisp.manual_param.enable);
			printf("param_manual.effect = %d \n", aiisp.aiisp.manual_param.effect);
			printf("param_manual.param = %d, %d \n", aiisp.aiisp.manual_param.param[0], aiisp.aiisp.manual_param.param[1]);
			break;

		case 86:
			cg_mode_manual.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_CG_MODE_MANUAL, &cg_mode_manual);

			printf("cg_mode_manual id = %d \n", cg_mode_manual.id);
			printf("manual_enable = %d, manual_mode = %d \n", cg_mode_manual.cg_mode_manual.manual_enable, cg_mode_manual.cg_mode_manual.manual_mode);
			break;

		case 110:
			nr_lv.id = isp_id;

			printf("Set lv (%d ~ %d)>> \n", IQ_UI_NR_LV_MIN_CNT + 1, IQ_UI_NR_LV_MAX_CNT - 1);
			nr_lv.lv = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_NR_LV, &nr_lv);
			printf("nr id = %d \n", nr_lv.id);
			printf("nr level = %d \n", nr_lv.lv);
			break;

		case 111:
			sharpness_lv.id = isp_id;

			printf("Set lv (%d ~ %d)>> \n", IQ_UI_SHARPNESS_LV_MIN_CNT + 1, IQ_UI_SHARPNESS_LV_MAX_CNT - 1);
			sharpness_lv.lv = get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_SHARPNESS_LV, &sharpness_lv);
			printf("sharpness id = %d \n", sharpness_lv.id);
			printf("sharpness level = %d \n", sharpness_lv.lv);
			break;

		case 112:
			saturation_lv.id = isp_id;

			printf("Set lv (%d ~ %d)>> \n", IQ_UI_SATURATION_LV_MIN_CNT + 1, IQ_UI_SATURATION_LV_MAX_CNT - 1);
			saturation_lv.lv = get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_SATURATION_LV, &saturation_lv);
			printf("saturation id = %d \n", saturation_lv.id);
			printf("saturation level = %d \n", saturation_lv.lv);
			break;

		case 113:
			contrast_lv.id = isp_id;

			printf("Set lv (%d ~ %d)>> \n", IQ_UI_CONTRAST_LV_MIN_CNT + 1, IQ_UI_CONTRAST_LV_MAX_CNT - 1);
			contrast_lv.lv = get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_CONTRAST_LV, &contrast_lv);
			printf("contrast id = %d \n", contrast_lv.id);
			printf("contrast level = %d \n", contrast_lv.lv);
			break;

		case 114:
			brightness_lv.id = isp_id;

			printf("Set lv (%d ~ %d)>> \n", IQ_UI_BRIGHTNESS_LV_MIN_CNT + 1, IQ_UI_BRIGHTNESS_LV_MAX_CNT - 1);
			brightness_lv.lv = get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_BRIGHTNESS_LV, &brightness_lv);
			printf("brightness id = %d \n", brightness_lv.id);
			printf("brightness level = %d \n", brightness_lv.lv);
			break;

		case 115:
			night_mode.id = isp_id;

			printf("Set mode (%d = OFF ; %d = ON)>> \n", IQ_UI_NIGHT_MODE_OFF, IQ_UI_NIGHT_MODE_ON);
			night_mode.mode = get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_NIGHT_MODE, &night_mode);
			printf("night id = %d \n", night_mode.id);
			printf("night mode = %d \n", night_mode.mode);
			break;

		case 116:
			ycc_format.id = isp_id;

			printf("Set format (%d ~ %d)>> \n", IQ_UI_YCC_OUT_MIN_CNT + 1, IQ_UI_YCC_OUT_MAX_CNT - 1);
			ycc_format.format = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_YCC_FORMAT, &ycc_format);
			printf("ycc_format id = %d \n", ycc_format.id);
			printf("ycc_format format = %d \n", ycc_format.format);
			break;

		case 117:
			operation.id = isp_id;

			printf("Set operation (%d = MOVIE ; %d = PHOTO ; %d = CAPTURE)>> \n", IQ_UI_OPERATION_MOVIE, IQ_UI_OPERATION_PHOTO, IQ_UI_OPERATION_CAPTURE);
			operation.operation = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_OPERATION, &operation);
			printf("operation id = %d \n", operation.id);
			printf("operation sel = %d \n", operation.operation);
			break;

		case 118:
			imageeffect.id = isp_id;

			printf("Set effect (%d ~ %d)>> \n", IQ_UI_IMAGEEFFECT_MIN_CNT + 1, IQ_UI_IMAGEEFFECT_MAX_CNT - 1);
			imageeffect.effect = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_IMAGEEFFECT, &imageeffect);
			printf("imageeffect id = %d \n", imageeffect.id);
			printf("imageeffect effect = %d \n", imageeffect.effect);
			break;

		case 119:
			ccid.id = isp_id;

			printf("Set ccid (%d ~ %d)>> \n", IQ_UI_CCID_MIN_CNT + 1, IQ_UI_CCID_MAX_CNT - 1);
			ccid.ccid = isp_id;

			vendor_isp_set_iq(IQT_ITEM_CCID, &ccid);
			printf("color id = %d \n", ccid.id);
			printf("color ccid = %d \n", ccid.ccid);
			break;

		case 120:
			hue_shift.id = isp_id;

			printf("mode_sel (0: Auto ; 1: Manual)>> \n");
			tmp = (UINT32)get_choose_int();

			if (tmp == 0) {
				printf("hue id = %d \n", hue_shift.id);
				for (hue_shift.hue_shift = IQ_UI_HUE_SHIFT_CLOCK_60; hue_shift.hue_shift <= IQ_UI_HUE_SHIFT_COUNTER_60; (hue_shift.hue_shift)++) {
					vendor_isp_set_iq(IQT_ITEM_HUE_SHIFT, &hue_shift);
					printf("hue shift = %d \n", hue_shift.hue_shift);
					usleep(5000);
				}
				printf("Finish and Reset !! \n");
				hue_shift.hue_shift = IQ_UI_HUE_SHIFT_0;
				vendor_isp_set_iq(IQT_ITEM_HUE_SHIFT, &hue_shift);
				printf("hue shift = %d \n", hue_shift.hue_shift);
			} else {
				printf("Set hue_shift (%d ~ %d)>> \n", IQ_UI_HUE_SHIFT_MIN_CNT + 1, IQ_UI_HUE_SHIFT_MAX_CNT - 1);
				hue_shift.hue_shift = (UINT32)get_choose_int();

				vendor_isp_set_iq(IQT_ITEM_HUE_SHIFT, &hue_shift);
				printf("hue id = %d \n", hue_shift.id);
				printf("hue shift = %d \n", hue_shift.hue_shift);
			}
			break;

		case 121:
			tone_lv.id = isp_id;

			printf("Set tone_lv (%d ~ %d)>> \n", IQ_UI_TONE_LV_MIN_CNT + 1, IQ_UI_TONE_LV_MAX_CNT - 1);
			tone_lv.lv = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_TONE_LV, &tone_lv);
			printf("tone id = %d \n", tone_lv.id);
			printf("tone level = %d \n", tone_lv.lv);
			break;

		case 122:
			nr3d_lv.id = isp_id;

			printf("Set lv (%d ~ %d)>> \n", IQ_UI_3DNR_LV_MIN_CNT + 1, IQ_UI_3DNR_LV_MAX_CNT - 1);
			nr3d_lv.lv = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_3DNR_LV, &nr3d_lv);
			printf("3dnr id = %d \n", nr3d_lv.id);
			printf("3dnr level = %d \n", nr3d_lv.lv);
			break;

		case 123:
			gamma_lv.id = isp_id;

			printf("Set gamma_lv (%d ~ %d)>> \n", IQ_UI_GAMMA_LV_MIN_CNT + 1, IQ_UI_GAMMA_LV_MAX_CNT - 1);
			gamma_lv.lv = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_GAMMA_LV, &gamma_lv);
			printf("gamma_lv id = %d \n", gamma_lv.id);
			printf("gamma_lv level = %d \n", gamma_lv.lv);
			break;

		case 141:
			dpc.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_DPC_PARAM, &dpc);

			printf("Set enable (0, 1)>> \n");
			dpc.dpc.enable = (UINT32)get_choose_int();
			vendor_isp_set_iq(IQT_ITEM_DPC_PARAM, &dpc);

			printf("dpc id = %d \n", dpc.id);
			printf("dpc enable = %d \n", dpc.dpc.enable);
			printf("dpc table = {0x%x, ...} \n", dpc.dpc.table[0]);
			break;

		case 142:
			shading.id = isp_id;

			printf("Set ECS fisheye enable (0, 1)>> \n");
			ecs_fisheye_en = (UINT32)get_choose_int();

			shading.shading.ecs_enable = TRUE;
			shading.shading.vig_enable = FALSE;
			shading.shading.mode = IQ_OP_TYPE_MANUAL;
			if (ecs_fisheye_en == FALSE) {
				shading.shading.ecs_dthr_enable = TRUE;
			} else {
				shading.shading.ecs_dthr_enable = FALSE;
			}
			for (h = 0; h < IQ_SHADING_ECS_WIN; h++) {
				for (w = 0; w < IQ_SHADING_ECS_WIN; w++) {
					idx = h * IQ_SHADING_ECS_WIN + w;
					if (ecs_fisheye_en == FALSE) {
						shading.shading.ecs_map_tbl[idx] = 0x10040100;
					} else {
						i = (w > 32) ? IQ_SHADING_ECS_WIN - w - 1 : w;
						j = (h > 32) ? IQ_SHADING_ECS_WIN - h - 1 : h;
						i = 32 - i;
						j = 32 - j;
						if ((256 * i * i + 81 * j * j) > 90000) {
							shading.shading.ecs_map_tbl[idx] = 0x00000000;
						} else {
							shading.shading.ecs_map_tbl[idx] = 0x10040100;
						}
					}
				}
			}

			vendor_isp_set_iq(IQT_ITEM_SHADING_PARAM, &shading);
			printf("shading id = %d \n", shading.id);
			printf("shading ecs_enable = %d \n", shading.shading.ecs_enable);
			printf("shading vig_enable = %d \n", shading.shading.vig_enable);
			printf("shading mode = %d \n", shading.shading.mode);
			printf("shading ecs_dthr_enable = %d \n", shading.shading.ecs_dthr_enable);
			printf("shading ecs_smooth_l_m_ct_lower = %d \n", shading.shading.ecs_smooth_l_m_ct_lower);
			printf("shading ecs_smooth_l_m_ct_upper = %d \n", shading.shading.ecs_smooth_l_m_ct_upper);
			printf("shading ecs_smooth_m_h_ct_lower = %d \n", shading.shading.ecs_smooth_m_h_ct_lower);
			printf("shading ecs_smooth_m_h_ct_upper = %d \n", shading.shading.ecs_smooth_m_h_ct_upper);
			break;

		case 152:
			shdr_mode.id = isp_id;

			printf("Set shdr_mode (0: Fusion; 1: Middle; 2: Long; 3:Short)>> \n");
			shdr_mode.shdr_mode = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_SHDR_MODE, &shdr_mode);
			printf("shdr_mode id = %d \n", shdr_mode.id);
			printf("shdr_mode shdr_mode = %d \n", shdr_mode.shdr_mode);
			break;

		case 153:
			nr_3d_misc.id = isp_id;

			nr_3d_misc._3dnr_misc.md_roi[0] = 3;
			nr_3d_misc._3dnr_misc.md_roi[1] = 4;
			nr_3d_misc._3dnr_misc.mc_roi[0] = 1;
			nr_3d_misc._3dnr_misc.mc_roi[1] = 2;
			nr_3d_misc._3dnr_misc.roi_mv_th = 11;
			nr_3d_misc._3dnr_misc.ds_th_roi = 22;

			vendor_isp_set_iq(IQT_ITEM_3DNR_MISC_PARAM, &nr_3d_misc);
			printf("nr_3d_misc id = %d \n", nr_3d_misc.id);
			printf("nr_3d_misc param = %d %d %d %d %d %d \n", nr_3d_misc._3dnr_misc.mc_roi[0], nr_3d_misc._3dnr_misc.mc_roi[1],
												nr_3d_misc._3dnr_misc.md_roi[0], nr_3d_misc._3dnr_misc.md_roi[1],
												nr_3d_misc._3dnr_misc.roi_mv_th, nr_3d_misc._3dnr_misc.ds_th_roi);
			break;

		case 160:
			shading_ext.id = isp_id;

			printf("Set ecs id (0, 1, 2)>> \n");
			shading_ext.shading_ext_if.ecs_map_idx = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_SHADING_EXT_PARAM, &shading_ext);
			printf("shading_ext id = %d \n", shading_ext.id);
			printf("shading_ext ecs_map_idx = %d \n", shading_ext.shading_ext_if.ecs_map_idx);
			printf("shading_ext ecs_map_tbl[0] = 0x%x \n", shading_ext.shading_ext_if.ecs_map_tbl[0]);
			break;

		case 161:
			cst.id = isp_id;

			cst.cst.cst_coef[0] = 77;
			cst.cst.cst_coef[1] = 150;
			cst.cst.cst_coef[2] = 29;
			cst.cst.cst_coef[3] = 0;
			cst.cst.cst_coef[4] = 0;
			cst.cst.cst_coef[5] = 0;
			cst.cst.cst_coef[6] = 0;
			cst.cst.cst_coef[7] = 0;
			cst.cst.cst_coef[8] = 0;

			printf("Set y_ofs (-128 ~ 127)>> \n");
			cst.cst.y_ofs = (INT16)get_choose_int();

			printf("Set cb_ofs (0 ~ 255)>> \n");
			cst.cst.cb_ofs = (INT16)get_choose_int();

			printf("Set cr_ofs (0 ~ 255)>> \n");
			cst.cst.cr_ofs = (INT16)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_CST_PARAM, &cst);
			printf("cst id = %d \n", cst.id);
			printf("cst mode = %d \n", cst.cst.mode);
			printf("cst cst_coef = {%d, %d, %d, %d, %d, %d, %d, %d, %d}, \n"
				, cst.cst.cst_coef[0], cst.cst.cst_coef[1], cst.cst.cst_coef[2], cst.cst.cst_coef[3], cst.cst.cst_coef[4]
				, cst.cst.cst_coef[5], cst.cst.cst_coef[6], cst.cst.cst_coef[7], cst.cst.cst_coef[8]);
			printf("cst y_ofs = %d \n", cst.cst.y_ofs);
			printf("cst cb_ofs = %d \n", cst.cst.cb_ofs);
			printf("cst cr_ofs = %d \n", cst.cst.cr_ofs);
			break;

		case 171:
			{
			CHAR expand_dpc_bin[64] = {USER_EXPAND_BIN_0};
			VOS_FILE fp;
			struct vos_stat stat;

			UINT32 buffer_size;
			UINT16 *dpc_table_va;
			#if USER_EXPAND_DPC_TEST
			UINT32 i = 0, x, y;
			#endif

			if (hd_mem_init == FALSE) {
				hd_common_init(2);
				hd_common_mem_init(NULL);
				hd_mem_init = TRUE;
			}

			file_ret = vos_file_stat((CHAR *)&expand_dpc_bin, &stat);
			if (file_ret == 0) {
				buffer_size = stat.st_size;
				printf("get stat of %s OK, buffer_size = %d \n", &expand_dpc_bin, stat.st_size);
			} else {
				buffer_size = USER_EXPAND_DPC_BUFFER_SIZE;
				printf("get stat of %s NG, buffer_size = %d  \n", &expand_dpc_bin, USER_EXPAND_DPC_BUFFER_SIZE);
			}

			max_free_block.ddr = 0;
			vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
			printf("DDR0 max_free_block size = %d \n", max_free_block.size);

			if (max_free_block.size < buffer_size) {
				printf("DDR0 max_free_block size is too small \n");

				max_free_block.ddr = 1;
				vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
				printf("DDR1 max_free_block size = %d \n", max_free_block.size);
				if (max_free_block.size < buffer_size) {
					printf("DDR1 max_free_block size is too small \n");
					break;
				};
			};

			ret = hd_common_mem_alloc("DPC", &dpc_buffer.pa, (void **)&dpc_buffer.va, buffer_size, 0);
			if (ret != HD_OK) {
				printf("memory allocate size %d NG \n", buffer_size);
				break;
			};

			dpc_table_va = dpc_buffer.va;
			printf("memory allocate size %d OK, va(0x%lx) \n", buffer_size, dpc_buffer.va);

			expand_dpc.id = isp_id;

			printf("Set enable (0, 1)>> \n");
			expand_dpc.expand_dpc.enable = (UINT32)get_choose_int();;

			expand_dpc.expand_dpc.size = buffer_size;
			expand_dpc.expand_dpc.table_phyaddr = dpc_buffer.pa;

			if (file_ret == HD_OK) {
				fp = vos_file_open((CHAR *)&expand_dpc_bin, O_RDONLY, 0);
				if (fp == VOS_FILE_INVALID) {
					printf("open %s fail \n", &expand_dpc_bin);
					break;
				}
				vos_file_read(fp, (UINT32 *)dpc_buffer.va, stat.st_size);
				vos_file_close(fp);
			} else {
				#if USER_EXPAND_DPC_TEST
				for (y = USER_EXPAND_DPC_Y; y < (USER_EXPAND_DPC_Y + USER_EXPAND_DPC_H); y++) {
					*(dpc_table_va + i) = 0x8000 | y;
					i++;
					for (x = USER_EXPAND_DPC_X; x < (USER_EXPAND_DPC_X + USER_EXPAND_DPC_W); x++) {
						*(dpc_table_va + i) = x;
						i++;
						if (i >= buffer_size / sizeof(UINT16)) {
							break;
						}
					}
					if (i >= buffer_size / sizeof(UINT16)) {
						break;
					}
				}
				if ((i - 2) < buffer_size / sizeof(UINT16)) {
					*(dpc_table_va + i) = 0xFFFF;
					if (i % 2 == 0) {
						*(dpc_table_va + i + 1) = 0x0000;
					} else {
						*(dpc_table_va + i + 1) = 0x0000;
						*(dpc_table_va + i + 2) = 0x0000;
					}
				}
				*(dpc_table_va + buffer_size / sizeof(UINT16) - 1) = 0x0000;
				*(dpc_table_va + buffer_size / sizeof(UINT16) - 2) = 0xFFFF;
				#else
				*(dpc_table_va) = 0xFFFF;
				*(dpc_table_va + 1) = 0x0000;
				#endif
			}

			vendor_isp_set_iq(IQT_ITEM_EXPAND_DPC_PARAM, &expand_dpc);
			printf("expand_dpc id = %d \n", expand_dpc.id);
			printf("expand_dpc enable = %d \n", expand_dpc.expand_dpc.enable);
			printf("expand_dpc size = %d \n", expand_dpc.expand_dpc.size);
			printf("expand_dpc table_phyaddr =0x%lx \n", expand_dpc.expand_dpc.table_phyaddr);
			printf("expand_dpc table = {0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, ..., 0x%x} \n"
				, *(dpc_table_va + 0), *(dpc_table_va + 1), *(dpc_table_va + 2)
				, *(dpc_table_va + 3), *(dpc_table_va + 4), *(dpc_table_va + 5)
				, *(dpc_table_va + 6), *(dpc_table_va + 7), *(dpc_table_va + 8)
				, *(dpc_table_va + buffer_size / sizeof(UINT16) - 1));

			}
		break;

		case 172:
			wdr_enh.id = isp_id;

			wdr_enh.wdr_enh.enable = TRUE;
			wdr_enh.wdr_enh.enh_ratio[0]  = 128;
			wdr_enh.wdr_enh.enh_ratio[1]  = 160;
			wdr_enh.wdr_enh.enh_ratio[2]  = 192;
			wdr_enh.wdr_enh.enh_ratio[3]  = 224;
			wdr_enh.wdr_enh.enh_ratio[4]  = 256;
			wdr_enh.wdr_enh.enh_ratio[5]  = 320;
			wdr_enh.wdr_enh.enh_ratio[6]  = 384;
			wdr_enh.wdr_enh.enh_ratio[7]  = 448;
			wdr_enh.wdr_enh.enh_ratio[8]  = 512;
			wdr_enh.wdr_enh.enh_ratio[9]  = 576;
			wdr_enh.wdr_enh.enh_ratio[10] = 640;
			wdr_enh.wdr_enh.enh_ratio[11] = 704;
			wdr_enh.wdr_enh.enh_ratio[12] = 768;
			wdr_enh.wdr_enh.enh_ratio[13] = 832;
			wdr_enh.wdr_enh.enh_ratio[14] = 896;
			wdr_enh.wdr_enh.enh_ratio[15] = 960;
			wdr_enh.wdr_enh.enh_ratio[16] = 1024;

			vendor_isp_set_iq(IQT_ITEM_WDR_ENH_PARAM, &wdr_enh);
			printf("wdr_enh id = %d \n", wdr_enh.id);
			printf("wdr_enh enable = %d \n", wdr_enh.wdr_enh.enable);
			printf("wdr_enh enh_ratio = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d} \n"
				, wdr_enh.wdr_enh.enh_ratio[0], wdr_enh.wdr_enh.enh_ratio[1], wdr_enh.wdr_enh.enh_ratio[2]
				, wdr_enh.wdr_enh.enh_ratio[3], wdr_enh.wdr_enh.enh_ratio[4], wdr_enh.wdr_enh.enh_ratio[5]
				, wdr_enh.wdr_enh.enh_ratio[6], wdr_enh.wdr_enh.enh_ratio[7], wdr_enh.wdr_enh.enh_ratio[8]
				, wdr_enh.wdr_enh.enh_ratio[9], wdr_enh.wdr_enh.enh_ratio[10], wdr_enh.wdr_enh.enh_ratio[11]
				, wdr_enh.wdr_enh.enh_ratio[12], wdr_enh.wdr_enh.enh_ratio[13], wdr_enh.wdr_enh.enh_ratio[14]
				, wdr_enh.wdr_enh.enh_ratio[15], wdr_enh.wdr_enh.enh_ratio[16]);
			break;

		case 177:
			{
			//CHAR fpn_raw_64[256];
			//CHAR fpn_raw_1024[256];
			CHAR fpn_raw_2048[256];
			VOS_FILE fp;
			struct vos_stat stat;

			UINT32 buffer_size;

			//strncpy(fpn_raw_64, USER_FPN_RAW_64, 256);
			//strncpy(fpn_raw_1024, USER_FPN_RAW_1024, 256);
			strncpy(fpn_raw_2048, USER_FPN_RAW_2048, 256);

			if (hd_mem_init == FALSE) {
				hd_common_init(2);
				hd_common_mem_init(NULL);
				hd_mem_init = TRUE;
			}

			#if 0
			// create buffer for FPN_RAW @ ISO6400
			file_ret = vos_file_stat((CHAR *)&fpn_raw_64, &stat);
			if (file_ret == 0) {
				buffer_size = stat.st_size;
				printf("get stat of %s OK, buffer_size = %d \n", &fpn_raw_64, stat.st_size);
			} else {
				buffer_size = 0;
				printf("get %s FAIL \n", &fpn_raw_64);
				break;
			}

			max_free_block.ddr = 0;
			vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
			printf("DDR0 max_free_block size = %d \n", max_free_block.size);

			if (max_free_block.size < buffer_size) {
				printf("DDR0 max_free_block size is too small \n");

				max_free_block.ddr = 1;
				vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
				printf("DDR1 max_free_block size = %d \n", max_free_block.size);
				if (max_free_block.size < buffer_size) {
					printf("DDR1 max_free_block size is too small \n");
					break;
				};
			};

			ret = hd_common_mem_alloc("FPN_64", &fpn_buffer_64.pa, (void **)&fpn_buffer_64.va, buffer_size, 0);
			if (ret != HD_OK) {
				printf("memory allocate size %d NG \n", buffer_size);
				break;
			};
			#endif

			#if 0
			// create buffer for FPN_RAW @ ISO102400
			file_ret = vos_file_stat((CHAR *)&fpn_raw_1024, &stat);
			if (file_ret == 0) {
				buffer_size = stat.st_size;
				printf("get stat of %s OK, buffer_size = %d \n", &fpn_raw_1024, stat.st_size);
			} else {
				buffer_size = 0;
				printf("get %s FAIL \n", &fpn_raw_1024);
				break;
			}

			max_free_block.ddr = 0;
			vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
			printf("DDR0 max_free_block size = %d \n", max_free_block.size);

			if (max_free_block.size < buffer_size) {
				printf("DDR0 max_free_block size is too small \n");

				max_free_block.ddr = 1;
				vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
				printf("DDR1 max_free_block size = %d \n", max_free_block.size);
				if (max_free_block.size < buffer_size) {
					printf("DDR1 max_free_block size is too small \n");
					break;
				};
			};

			ret = hd_common_mem_alloc("FPN_1024", &fpn_buffer_1024.pa, (void **)&fpn_buffer_1024.va, buffer_size, 0);
			if (ret != HD_OK) {
				printf("memory allocate size %d NG \n", buffer_size);
				break;
			};
			printf("memory allocate size %d OK, va(0x%lx) \n", buffer_size, fpn_buffer_1024.va);
			#endif

			// create buffer for FPN_RAW @ ISO204800
			file_ret = vos_file_stat((CHAR *)&fpn_raw_2048, &stat);
			if (file_ret == 0) {
				buffer_size = stat.st_size;
				printf("get stat of %s OK, buffer_size = %d \n", &fpn_raw_2048, stat.st_size);
			} else {
				buffer_size = 0;
				printf("get %s FAIL \n", &fpn_raw_2048);
				break;
			}

			max_free_block.ddr = 0;
			vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
			printf("DDR0 max_free_block size = %d \n", max_free_block.size);

			if (max_free_block.size < buffer_size) {
				printf("DDR0 max_free_block size is too small \n");

				max_free_block.ddr = 1;
				vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
				printf("DDR1 max_free_block size = %d \n", max_free_block.size);
				if (max_free_block.size < buffer_size) {
					printf("DDR1 max_free_block size is too small \n");
					break;
				};
			};

			ret = hd_common_mem_alloc("FPN_2048", &fpn_buffer_2048.pa, (void **)&fpn_buffer_2048.va, buffer_size, 0);
			if (ret != HD_OK) {
				printf("memory allocate size %d NG \n", buffer_size);
				break;
			};

			// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

			fpn.id = isp_id;

			printf("Set enable (0, 1)>> \n");
			fpn.fpn.enable = (UINT32)get_choose_int();

			fpn.fpn.mode = IQ_OP_TYPE_AUTO;
			fpn.fpn.auto_param[15].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[15].buf_size = buffer_size;
			fpn.fpn.auto_param[15].gain = 256;
			fpn.fpn.auto_param[14].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[14].buf_size = buffer_size;
			fpn.fpn.auto_param[14].gain = 256;
			fpn.fpn.auto_param[13].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[13].buf_size = buffer_size;
			fpn.fpn.auto_param[13].gain = 256;
			fpn.fpn.auto_param[12].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[12].buf_size = buffer_size;
			fpn.fpn.auto_param[12].gain = 256;
			fpn.fpn.auto_param[11].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[11].buf_size = buffer_size;
			fpn.fpn.auto_param[11].gain = 256;
			fpn.fpn.auto_param[10].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[10].buf_size = buffer_size;
			fpn.fpn.auto_param[10].gain = fpn.fpn.auto_param[11].gain >> 1;
			fpn.fpn.auto_param[9].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[9].buf_size = buffer_size;
			fpn.fpn.auto_param[9].gain = fpn.fpn.auto_param[10].gain >> 1;
			fpn.fpn.auto_param[8].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[8].buf_size = buffer_size;
			fpn.fpn.auto_param[8].gain = fpn.fpn.auto_param[9].gain >> 1;
			fpn.fpn.auto_param[7].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[7].buf_size = buffer_size;
			fpn.fpn.auto_param[7].gain = fpn.fpn.auto_param[8].gain >> 1;
			fpn.fpn.auto_param[6].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[6].buf_size = buffer_size;
			fpn.fpn.auto_param[6].gain = fpn.fpn.auto_param[7].gain >> 1;
			fpn.fpn.auto_param[5].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[5].buf_size = buffer_size;
			fpn.fpn.auto_param[5].gain = fpn.fpn.auto_param[6].gain >> 1;
			fpn.fpn.auto_param[4].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[4].buf_size = buffer_size;
			fpn.fpn.auto_param[4].gain = fpn.fpn.auto_param[5].gain >> 1;
			fpn.fpn.auto_param[3].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[3].buf_size = buffer_size;
			fpn.fpn.auto_param[3].gain = fpn.fpn.auto_param[4].gain >> 1;
			fpn.fpn.auto_param[2].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[2].buf_size = buffer_size;
			fpn.fpn.auto_param[2].gain = fpn.fpn.auto_param[3].gain >> 1;
			fpn.fpn.auto_param[1].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[1].buf_size = buffer_size;
			fpn.fpn.auto_param[1].gain = fpn.fpn.auto_param[2].gain >> 1;
			fpn.fpn.auto_param[0].buf_phyaddr = fpn_buffer_2048.pa;
			fpn.fpn.auto_param[0].buf_size = buffer_size;
			fpn.fpn.auto_param[0].gain = fpn.fpn.auto_param[1].gain >> 1;

			#if 0
			// write FPN_RAW @ ISO6400 to DRAM
			if (file_ret == HD_OK) {
				fp = vos_file_open((CHAR *)&fpn_raw_64, O_RDONLY, 0);
				if (fp == VOS_FILE_INVALID) {
					printf("open %s fail \n", &fpn_raw_64);
					break;
				}
				vos_file_read(fp, (UINT32 *)fpn_buffer_64.va, stat.st_size);
				vos_file_close(fp);
			}
			#endif

			#if 0
			// write FPN_RAW @ ISO102400 to DRAM
			if (file_ret == HD_OK) {
				fp = vos_file_open((CHAR *)&fpn_raw_1024, O_RDONLY, 0);
				if (fp == VOS_FILE_INVALID) {
					printf("open %s fail \n", &fpn_raw_1024);
					break;
				}
				vos_file_read(fp, (UINT32 *)fpn_buffer_1024.va, stat.st_size);
				vos_file_close(fp);
			}
			#endif

			// write FPN_RAW @ ISO204800 to DRAM
			if (file_ret == HD_OK) {
				fp = vos_file_open((CHAR *)&fpn_raw_2048, O_RDONLY, 0);
				if (fp == VOS_FILE_INVALID) {
					printf("open %s fail \n", &fpn_raw_2048);
					break;
				}
				vos_file_read(fp, (UINT32 *)fpn_buffer_2048.va, stat.st_size);
				vos_file_close(fp);
			}

			vendor_isp_set_iq(IQT_ITEM_FPN_PARAM, &fpn);
			printf("fpn id = %d \n", fpn.id);
			printf("fpn enable = %d \n", fpn.fpn.enable);
			printf("fpn mode = %d \n", fpn.fpn.mode);
			//printf("fpn auto_param[6].buf_size = %d \n", fpn.fpn.auto_param[6].buf_size);
			//printf("fpn auto_param[6].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[6].buf_phyaddr);
			//printf("fpn auto_param[6].gain = %d \n", fpn.fpn.auto_param[6].gain);
			printf("fpn auto_param[10].buf_size = %d \n", fpn.fpn.auto_param[10].buf_size);
			printf("fpn auto_param[10].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[10].buf_phyaddr);
			printf("fpn auto_param[10].gain = %d \n", fpn.fpn.auto_param[10].gain);
			printf("fpn auto_param[11].buf_size = %d \n", fpn.fpn.auto_param[11].buf_size);
			printf("fpn auto_param[11].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[11].buf_phyaddr);
			printf("fpn auto_param[11].gain = %d \n", fpn.fpn.auto_param[11].gain);

			printf("fpn auto_param[11].buf = {0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ...} \n"
				, *((UINT32 *)fpn_buffer_2048.va + 0), *((UINT32 *)fpn_buffer_2048.va + 1), *((UINT32 *)fpn_buffer_2048.va + 2), *((UINT32 *)fpn_buffer_2048.va + 3)
				, *((UINT32 *)fpn_buffer_2048.va + 4), *((UINT32 *)fpn_buffer_2048.va + 5), *((UINT32 *)fpn_buffer_2048.va + 6), *((UINT32 *)fpn_buffer_2048.va + 7));
			}
			break;

		case 181:
			fixth.id = isp_id;
			printf("Set enable (0, 1)>> \n");
			fixth.fixth.enable = (UINT32)get_choose_int();
			printf("Set y_th (0 ~ 255)>> \n");
			fixth.fixth.fixy_y_th = (UINT32)get_choose_int();
			printf("Set y_value (0 ~ 255)>> \n");
			fixth.fixth.fixy_y_value = (UINT32)get_choose_int();

			fixth.fixth.fixc_y_th_low = fixth.fixth.fixy_y_th;
			fixth.fixth.fixc_y_th_high = 255;
			fixth.fixth.fixc_cb_th_low = 0;
			fixth.fixth.fixc_cb_th_high = 255;
			fixth.fixth.fixc_cr_th_low = 0;
			fixth.fixth.fixc_cr_th_high = 255;
			printf("Set cb_value (0 ~ 255)>> \n");
			fixth.fixth.fixc_cb_value = (UINT32)get_choose_int();
			printf("Set cr_value (0 ~ 255)>> \n");
			fixth.fixth.fixc_cr_value = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_FIXTH_PARAM, &fixth);
			printf("fixth id = %d \n", fixth.id);
			printf("fixth enable = %d \n", fixth.fixth.enable);
			printf("fixth fixy_sel = %d \n", fixth.fixth.fixy_sel);
			printf("fixth fixy_y_sel = %d \n", fixth.fixth.fixy_y_sel);
			printf("fixth fixy_edge_th = %d \n", fixth.fixth.fixy_edge_th);
			printf("fixth fixy_y_th = %d \n", fixth.fixth.fixy_y_th);
			printf("fixth fixy_y_value = %d \n", fixth.fixth.fixy_y_value);
			printf("fixth fixc_y_th_low = %d \n", fixth.fixth.fixc_y_th_low);
			printf("fixth fixc_y_th_high = %d \n", fixth.fixth.fixc_y_th_high);
			printf("fixth fixc_cb_th_low = %d \n", fixth.fixth.fixc_cb_th_low);
			printf("fixth fixc_cb_th_high = %d \n", fixth.fixth.fixc_cb_th_high);
			printf("fixth fixc_cr_th_low = %d \n", fixth.fixth.fixc_cr_th_low);
			printf("fixth fixc_cr_th_high = %d \n", fixth.fixth.fixc_cr_th_high);
			printf("fixth fixc_cb_value = %d \n", fixth.fixth.fixc_cb_value);
			printf("fixth fixc_cr_value = %d \n", fixth.fixth.fixc_cr_value);
			break;

		case 182:
			ob_mode_manual.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_OB_MODE_MANUAL, &ob_mode_manual);

			printf("Set ifef en (0, 1)>> \n");
			ob_mode_manual.ob_mode_manual.ife_f_enable = (UINT32)get_choose_int();
			printf("Set ifef value (0, 4095)>> \n");
			ob_mode_manual.ob_mode_manual.ife_f_value[0] = (UINT32)get_choose_int();
			ob_mode_manual.ob_mode_manual.ife_f_value[1] = ob_mode_manual.ob_mode_manual.ife_f_value[0];
			ob_mode_manual.ob_mode_manual.ife_f_value[2] = ob_mode_manual.ob_mode_manual.ife_f_value[0];
			ob_mode_manual.ob_mode_manual.ife_f_value[3] = ob_mode_manual.ob_mode_manual.ife_f_value[0];
			ob_mode_manual.ob_mode_manual.ife_f_value[4] = ob_mode_manual.ob_mode_manual.ife_f_value[0];
			printf("Set ife en (0, 1)>> \n");
			ob_mode_manual.ob_mode_manual.ife_enable = (UINT32)get_choose_int();
			printf("Set ife value (0, 4095)>> \n");
			ob_mode_manual.ob_mode_manual.ife_value[0] = (UINT32)get_choose_int();
			ob_mode_manual.ob_mode_manual.ife_value[1] = ob_mode_manual.ob_mode_manual.ife_value[0];
			ob_mode_manual.ob_mode_manual.ife_value[2] = ob_mode_manual.ob_mode_manual.ife_value[0];
			ob_mode_manual.ob_mode_manual.ife_value[3] = ob_mode_manual.ob_mode_manual.ife_value[0];
			ob_mode_manual.ob_mode_manual.ife_value[4] = ob_mode_manual.ob_mode_manual.ife_value[0];

			printf("Set manual_mode_enable (0, 1)>> \n");
			ob_mode_manual.ob_mode_manual.manual_enable = (UINT32)get_choose_int();
			printf("Set manual_mode (0, 4)>> \n");
			ob_mode_manual.ob_mode_manual.manual_mode = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_OB_MODE_MANUAL, &ob_mode_manual);
			break;

		case 183:
			bnr.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_BNR_PARAM, &bnr);

			printf("Set bnr en (0, 1)>> \n");
			bnr.bnr.enable = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_BNR_PARAM, &bnr);
			break;

		case 184:
			dg_mode_manual.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_DG_MODE_MANUAL, &dg_mode_manual);

			printf("Set sie en (0, 1)>> \n");
			dg_mode_manual.dg_mode_manual.sie_enable = (UINT32)get_choose_int();
			printf("Set sie value (0, 32767)>> \n");
			dg_mode_manual.dg_mode_manual.sie_value = (UINT32)get_choose_int();

			printf("Set pre en (0, 1)>> \n");
			dg_mode_manual.dg_mode_manual.pre_enable = (UINT32)get_choose_int();
			printf("Set pre value (0, 32767)>> \n");
			dg_mode_manual.dg_mode_manual.pre_value = (UINT32)get_choose_int();

			printf("Set ife en (0, 1)>> \n");
			dg_mode_manual.dg_mode_manual.ife_enable = (UINT32)get_choose_int();
			printf("Set ife value (0, 32767)>> \n");
			dg_mode_manual.dg_mode_manual.ife_value = (UINT32)get_choose_int();

			printf("Set manual_mode_enable (0, 1)>> \n");
			dg_mode_manual.dg_mode_manual.manual_enable = (UINT32)get_choose_int();
			printf("Set manual_mode (0, 5)>> \n");
			dg_mode_manual.dg_mode_manual.manual_mode = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_DG_MODE_MANUAL, &dg_mode_manual);
			break;

		case 186:
			cg_mode_manual.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_CG_MODE_MANUAL, &cg_mode_manual);

			printf("Set manual_mode_enable (0, 1)>> \n");
			cg_mode_manual.cg_mode_manual.manual_enable = (UINT32)get_choose_int();
			printf("Set manual_mode (0, 4)>> \n");
			cg_mode_manual.cg_mode_manual.manual_mode = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_CG_MODE_MANUAL, &cg_mode_manual);
			break;

		case 200:
			dark_enh_ratio.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_NNSC_DARK_ENH_RATIO, &dark_enh_ratio);
			printf("dark_enh_ratio id = %d \n", dark_enh_ratio.id);
			printf("dark_enh_ratio ratio = %d \n", dark_enh_ratio.ratio);
			break;

		case 201:
			contrast_enh_ratio.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_NNSC_CONTRAST_ENH_RATIO, &contrast_enh_ratio);
			printf("contrast_enh_ratio id = %d \n", contrast_enh_ratio.id);
			printf("contrast_enh_ratio ratio = %d \n", contrast_enh_ratio.ratio);
			break;

		case 202:
			green_enh_ratio.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_NNSC_GREEN_ENH_RATIO, &green_enh_ratio);
			printf("green_enh_ratio id = %d \n", green_enh_ratio.id);
			printf("green_enh_ratio ratio = %d \n", green_enh_ratio.ratio);
			break;

		case 203:
			skin_enh_ratio.id = isp_id;

			vendor_isp_get_iq(IQT_ITEM_NNSC_SKIN_ENH_RATIO, &skin_enh_ratio);
			printf("skin_enh_ratio id = %d \n", skin_enh_ratio.id);
			printf("skin_enh_ratio ratio = %d \n", skin_enh_ratio.ratio);
			break;

		case 300:
			dark_enh_ratio.id = isp_id;

			printf("Set ratio (0 ~ 100)>> \n");
			dark_enh_ratio.ratio = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_NNSC_DARK_ENH_RATIO, &dark_enh_ratio);
			printf("dark_enh_ratio id = %d \n", dark_enh_ratio.id);
			printf("dark_enh_ratio ratio = %d \n", dark_enh_ratio.ratio);
			break;

		case 301:
			contrast_enh_ratio.id = isp_id;

			printf("Set ratio (0 ~ 100)>> \n");
			contrast_enh_ratio.ratio = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_NNSC_CONTRAST_ENH_RATIO, &contrast_enh_ratio);
			printf("contrast_enh_ratio id = %d \n", contrast_enh_ratio.id);
			printf("contrast_enh_ratio ratio = %d \n", contrast_enh_ratio.ratio);
			break;

		case 302:
			green_enh_ratio.id = isp_id;

			printf("Set ratio (0 ~ 100)>> \n");
			green_enh_ratio.ratio = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_NNSC_GREEN_ENH_RATIO, &green_enh_ratio);
			printf("green_enh_ratio id = %d \n", green_enh_ratio.id);
			printf("green_enh_ratio ratio = %d \n", green_enh_ratio.ratio);
			break;

		case 303:
			skin_enh_ratio.id = isp_id;

			printf("Set ratio (0 ~ 100)>> \n");
			skin_enh_ratio.ratio = (UINT32)get_choose_int();

			vendor_isp_set_iq(IQT_ITEM_NNSC_SKIN_ENH_RATIO, &skin_enh_ratio);
			printf("skin_enh_ratio id = %d \n", skin_enh_ratio.id);
			printf("skin_enh_ratio ratio = %d \n", skin_enh_ratio.ratio);
			break;

		case 500:
			tmp = (UINT32)get_choose_int();
			printf("Set auto tone level enable (0, 5)>> \n");
			tone_level_en = (BOOL)get_choose_int();
			printf("Set debug enable (0, 5)>> \n");
			tone_level_dbg = (BOOL)get_choose_int();

			printf("dr_level id = %d \n", dr_level.id);

			iq_tonelv_conti_run = 1;
			if (pthread_create(&iq_tonelv_thread_id, NULL, iq_tonelv_thread, &tmp) < 0) {
				printf("create iq_tonelv thread failed");
				break;
			}

			printf("Enter 0 to exit >> \n");
			iq_tonelv_conti_run = (UINT32)get_choose_int();

			// destroy encode thread
			pthread_join(iq_tonelv_thread_id, NULL);
			break;

		case 999:
				printf("Set isp id (0, %d)>> \n", IQ_ID_MAX_NUM - 1);
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

	if (hd_mem_init) {
		if (dpc_buffer.va != NULL) {
			ret = hd_common_mem_free(dpc_buffer.pa, dpc_buffer.va);
			if (ret != HD_OK) {
				printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(dpc_buffer.pa), (UINTPTR)(dpc_buffer.va));
			}
		}
		if (fpn_buffer_64.va != NULL) {
			ret = hd_common_mem_free(fpn_buffer_64.pa, fpn_buffer_64.va);
			if (ret != HD_OK) {
				printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(fpn_buffer_64.pa), (UINTPTR)(fpn_buffer_64.va));
			}
		}
		if (fpn_buffer_1024.va != NULL) {
			ret = hd_common_mem_free(fpn_buffer_1024.pa, fpn_buffer_1024.va);
			if (ret != HD_OK) {
				printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(fpn_buffer_1024.pa), (UINTPTR)(fpn_buffer_1024.va));
			}
		}
		if (fpn_buffer_2048.va != NULL) {
			ret = hd_common_mem_free(fpn_buffer_2048.pa, fpn_buffer_2048.va);
			if (ret != HD_OK) {
				printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(fpn_buffer_2048.pa), (UINTPTR)(fpn_buffer_2048.va));
			}
		}
		hd_common_mem_uninit();
		hd_common_uninit();
	}

	vendor_isp_uninit();

	return 0;
}
