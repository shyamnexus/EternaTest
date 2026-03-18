#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#endif

#include <libfdt.h>
#include <fdtfast.h>

#include "kwrap/debug.h"

#include "awb_builtin.h"

#if (NVT_FAST_ISP_FLOW)
//=============================================================================
// define
//=============================================================================
#define MIN(a, b)       ((a) <= (b) ? (a) : (b))
#define MAX(a, b)       ((a) >= (b) ? (a) : (b))
#define CLAMP(x, a, b)  MAX(MIN(x, b), a)

#define SUB_NODE_LENGTH 64

//=============================================================================
// global
//=============================================================================
static AWB_BUILTIN_INFO awb_info[ISP_BUILTIN_ID_MAX_NUM] = {0};
static AWB_BUILTIN_CT_INFO awb_builtin_ct_info[ISP_BUILTIN_ID_MAX_NUM] = {0};
static AWB_BUILTIN_TH awb_th[ISP_BUILTIN_ID_MAX_NUM] = {0};

static AWB_BUILTIN_CT_INFO awb_builtin_ct_info_os04c10 = {
	{2300, 2800, 3700, 4700, 6500, 11000},
	{278, 315, 386, 470, 573, 730},
	{256, 256, 256, 256, 256, 256},
	{1052, 890, 708, 585, 421, 360}
};

static AWB_BUILTIN_TH awb_th_os04c10 = {5, 245, 49, 165, 20, 90};

AWB_BUILTIN_FRONT_DTSI awb_front_dtsi = {
	DISABLE,
	{256, 256, 256}
};

//=============================================================================
// function declaration
//=============================================================================
void awb_builtin_get_cur_ct(UINT32 id, ISP_BUILTIN_CGAIN cur_gain, UINT32 *ct);

//=============================================================================
// external functions
//=============================================================================
// TODO: ISP builtin
#if defined(__KERNEL__)
static void awb_builtin_front_dtsi_sub_load(CHAR *node_path, UINT8 *param_ptr, CHAR *node_name, INT32 node_size)
{
	UINT8 *pfdt_addr = (unsigned char *)fdtfast_get_base();
	CHAR sub_node_name[SUB_NODE_LENGTH];
	const void *pfdt_node;
	UINT32 size = 0;
	INT32 node_ofst = 0, data_size;

	sprintf(sub_node_name, "%s/%s", node_path, node_name);
	node_ofst = fdt_path_offset(pfdt_addr, (CHAR *)&sub_node_name);
	if (node_ofst >= 0) {
		pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "size", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			size = *(UINT32 *)pfdt_node;
			if (size != node_size) {
				DBG_DUMP("%s/size mismatch (dtsi:%d, sdk:%d) \r\n", sub_node_name, size, node_size);
				return;
			}
		} else {
			DBG_DUMP("cannot find %s/size \r\n", sub_node_name);
			return;
		}

		pfdt_node = fdt_getprop(pfdt_addr, node_ofst, "data", (int *)&data_size);
		if ((pfdt_node != NULL) && (data_size != 0)) {
			memcpy(param_ptr, pfdt_node, node_size);
		} else {
			DBG_DUMP("cannot find %s/data \r\n", sub_node_name);
		}
	} else {
		DBG_DUMP("cannot find %s \r\n", sub_node_name);
	}
}

void awb_builtin_front_dtsi_load(void)
{
	CHAR node_path[SUB_NODE_LENGTH];

	sprintf(node_path, "/fastboot/front_awb_0");

	awb_builtin_front_dtsi_sub_load(node_path, (UINT8 *)&awb_front_dtsi.enable, "enable", sizeof(awb_front_dtsi.enable));

	awb_builtin_front_dtsi_sub_load(node_path, (UINT8 *)&awb_front_dtsi.color_gain, "color_gain", sizeof(awb_front_dtsi.color_gain));

}
#endif

INT32 awb_builtin_init(UINT32 id)
{
	ISP_BUILTIN_CGAIN *preset_c_gain[ISP_BUILTIN_ID_MAX_NUM];

	awb_info[id].awb_app_mode = 0;

	if (isp_builtin_get_shdr_enable(id) == 0) {
		awb_info[id].awb_app_mode = 0;
	} else if(isp_builtin_get_shdr_enable(id) == 1) {
		awb_info[id].awb_app_mode = 1;
	}

	switch (isp_builtin_get_sensor_name(id)) {
		default:
		case 8:
			awb_info[id].awb_gain.r = 470;
			awb_info[id].awb_gain.g = 256;
			awb_info[id].awb_gain.b = 585;
			awb_builtin_ct_info[id] = awb_builtin_ct_info_os04c10;
			awb_th[id] = awb_th_os04c10;
		break;
	}

	// TODO: ISP builtin
	#if defined(__KERNEL__)
	awb_builtin_front_dtsi_load();
	#endif

	if(awb_front_dtsi.enable == ENABLE) {
		awb_info[id].awb_gain.r = awb_front_dtsi.color_gain.r;
		awb_info[id].awb_gain.g = awb_front_dtsi.color_gain.g;
		awb_info[id].awb_gain.b = awb_front_dtsi.color_gain.b;
	}

	#if 0
	DBG_DUMP("awb_gain ====== %d, %d, %d\r\n", awb_info.awb_gain.r, awb_info.awb_gain.g, awb_info.awb_gain.b);
	#endif

	awb_info[id].cur_ct = 4700;
	awb_info[id].status = AWBALG_STATUS_INIT;

	preset_c_gain[id] = isp_builtin_get_cgain(id);
	if ((preset_c_gain[id]->r == 256) && (preset_c_gain[id]->g == 256) && (preset_c_gain[id]->b == 256)) {
		isp_builtin_set_cgain(id, &awb_info[id].awb_gain);
	}
	isp_builtin_set_ct(id, awb_info[id].cur_ct);
	return 0;
}

INT32 awb_builtin_trig(UINT32 id, ISP_BUILTIN_AWB_TRIG_MSG msg)
{
	ISP_BUILTIN_CA_RSLT *ca_rslt = NULL;
	UINT16 i;
	UINT16 r = 0, g = 0, b = 0;
	UINT16 y = 0, r2g = 0, b2g = 0, g2r = 0, g2b = 0;
	UINT32 gray_g2r_sum = 0, gray_g2b_sum = 0;
	UINT16 gray_w_cnt = 0;

	if (msg == ISP_BUILTIN_AWB_TRIG_RESET) {
		return 0;
	}

	if (awb_info[id].awb_app_mode == 0) {
		ca_rslt = isp_builtin_get_ca(id);
	} else if (awb_info[id].awb_app_mode == 1) {
		ca_rslt = isp_builtin_get_ca(1);
	} else {
		DBG_DUMP("awb_app_mode = %d is not support\r\n", awb_info[id].awb_app_mode);
	}

	if(ca_rslt == NULL) {
		DBG_DUMP("isp_builtin_get_ca is NULL\r\n");
		return 0;
	}

	for (i = 0; i < 1024; i++) {
		r = ca_rslt->r[i];
		g = ca_rslt->g[i];
		b = ca_rslt->b[i];

		if (r == 0 || g == 0 || b == 0) {
			continue;
		}

		y = (r + b + g + g) >> 2;
		r2g = (r << 7) / g;
		b2g = (b << 7) / g;

		if ((y > (awb_th[id].y_l << 4)) && (y < (awb_th[id].y_u << 4)) && (r2g > (awb_th[id].r2g_l)) &&
			(r2g < (awb_th[id].r2g_u)) && (b2g > (awb_th[id].b2g_l)) && (b2g < (awb_th[id].b2g_u))) {

			g2r = (g << 8) / r;
			g2b = (g << 8) / b;

			// Gray world process
			gray_g2r_sum += g2r;
			gray_g2b_sum += g2b;
			gray_w_cnt++;
		}
	}

	if(gray_w_cnt == 0) {
		awb_info[id].status = AWBALG_STATUS_FREEZE;
		return -1;
	}

	awb_info[id].awb_gain.r = gray_g2r_sum / gray_w_cnt;
	awb_info[id].awb_gain.g = 256;
	awb_info[id].awb_gain.b = gray_g2b_sum / gray_w_cnt;

	awb_builtin_get_cur_ct(id, awb_info[id].awb_gain, &awb_info[id].cur_ct);

	awb_info[id].status = AWBALG_STATUS_CONVERGE;

	isp_builtin_set_cgain(id, &awb_info[id].awb_gain);
	isp_builtin_set_ct(id, awb_info[id].cur_ct);
	//DBG_DUMP("awb id = %d, status: %d, WB(%d, %d, %d), total win = %d \r\n",id , awb_info[id].status, awb_info[id].awb_gain.r, awb_info[id].awb_gain.g, awb_info[id].awb_gain.b, gray_w_cnt);

	return 0;
}

void awb_builtin_get_cur_ct(UINT32 id, ISP_BUILTIN_CGAIN cur_gain, UINT32 *ct)
{
	UINT16 i;
	INT32 r2b_table[ISP_AWB_CT_NUM];
	INT32 cur_r2b, cur_ct;
	INT32 idx1, idx2, delta1, delta2, delta3;

	for (i = 0; i < ISP_AWB_CT_NUM; i++) {
		r2b_table[i] = awb_builtin_ct_info[id].r_gain[i] * 256 / awb_builtin_ct_info[id].b_gain[i];
	}

	cur_r2b = cur_gain.r * 256 / cur_gain.b;

	if (cur_r2b <= r2b_table[0]) {
		cur_ct = awb_builtin_ct_info[id].temperature[0];
	} else if (cur_r2b >= r2b_table[ISP_AWB_CT_NUM - 1]) {
		cur_ct = awb_builtin_ct_info[id].temperature[ISP_AWB_CT_NUM - 1];
	} else {
		for (idx2 = 1; idx2 <= (ISP_AWB_CT_NUM - 1); idx2++) {
			if (cur_r2b < r2b_table[idx2]) {
				break;
			}
		}
		idx2 = CLAMP(idx2, 1, ISP_AWB_CT_NUM - 1);
		idx1 = idx2 - 1;
		delta1 = cur_r2b - r2b_table[idx1];
		delta2 = r2b_table[idx2] - cur_r2b;
		if ((delta1 + delta2) == 0) {
			DBG_DUMP("Interpolation Divide by 0");
			delta3 = 1;
		} else {
			delta3 = delta1 + delta2;
		}
		cur_ct = ((awb_builtin_ct_info[id].temperature[idx1] * delta2) + (awb_builtin_ct_info[id].temperature[idx2] * delta1)) / delta3;
	}

	*ct = (UINT32)cur_ct;

	return;
}

#endif

