#if defined(__FREERTOS)
#include <string.h>
#include <stdio.h>
#else
#include <linux/seq_file.h>
#endif
#include <plat/top.h>

#include "kwrap/type.h"

#include "isp_uti.h"

#if defined(__KERNEL__)
#include "isp_builtin.h"
#endif
#include "isp_api_int.h"
#include "isp_dbg.h"
#include "isp_main.h"
#include "isp_msg.h"
#include "isp_version.h"

//=============================================================================
// global
//=============================================================================
#if defined(__KERNEL__)
extern UINT32 isp_id_list;
#endif

//=============================================================================
// external functions
//=============================================================================
#if defined(__FREERTOS)
void isp_msg_show_info(void)
#else
void isp_msg_show_info(struct seq_file *sfile)
#endif
{
	UINT32 i, id, frame_num = 1;
	UINT32 version = isp_get_version();
	ISP_FUNC_EN fun_en = ISP_FUNC_EN_AE;
	ISP_DEV_INFO *pdev_info = isp_get_dev_info();
	ISP_FUNC_INFO *func_info = NULL;
	ISP_SENSOR_INFO *sensor_info = NULL;
	ISP_SENSOR_DIRECTION sensor_direction[ISP_ID_MAX_NUM] = {0};
	INT32 sie_ret = CTL_SIE_E_OK;
	ER ipp_ret = E_OK;

	if (pdev_info == NULL) {
		return;
	}

	if (func_info == NULL) {
		func_info = isp_uti_vmem_alloc(sizeof(ISP_FUNC_INFO) * ISP_ID_MAX_NUM);
		if (func_info == NULL) {
			DBG_ERR("Allocate func_info fail \n");
			goto exit;
		}
	}
	if (sensor_info == NULL) {
		sensor_info = isp_uti_vmem_alloc(sizeof(ISP_SENSOR_INFO));
		if (sensor_info == NULL) {
			DBG_ERR("Allocate sensor_info fail \n");
			goto exit;
		}
	}

	isp_api_get_sensor_info(sensor_info);
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if ((sensor_info->src_id_mask[id] != 0) || ((isp_get_ipp_table(id) != ISP_ID_IGNORE) && ((isp_get_ipp_table(id) >> 15) & 0x1) && pdev_info->ipp_to_sie_id_map_en[id])) {
			isp_api_get_func(id, &func_info[id]);
		}
		if (sensor_info->src_id_mask[id] != 0) {
			isp_api_get_direction(id, &sensor_direction[id]);
		}
	}

	PRINT_ISP_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_ISP_INFO(sfile, "NVT_ISP %s v%d.%d.%d.%d\n", (nvt_get_chip_id() == CHIP_NS02402) ? "NT98539A" : "NT98538", (version>>24) & 0xFF, (version>>16) & 0xFF, (version>>8) & 0xFF, version & 0xFF);
	#if defined(__KERNEL__)
	if (isp_api_get_fastboot_valid()) {
		PRINT_ISP_INFO(sfile, "NVT_ISP_BUILTIN    v%d.%d.%d.%d\n", (isp_builtin_get_isp_version()>>24) & 0xFF, (isp_builtin_get_isp_version()>>16) & 0xFF, (isp_builtin_get_isp_version()>>8) & 0xFF, isp_builtin_get_isp_version() & 0xFF);
		PRINT_ISP_INFO(sfile, "NVT_AE_BUILTIN     v%d.%d.%d.%d\n", (isp_builtin_get_ae_version()>>24) & 0xFF, (isp_builtin_get_ae_version()>>16) & 0xFF, (isp_builtin_get_ae_version()>>8) & 0xFF, isp_builtin_get_ae_version() & 0xFF);
		PRINT_ISP_INFO(sfile, "NVT_AWB_BUILTIN    v%d.%d.%d.%d\n", (isp_builtin_get_awb_version()>>24) & 0xFF, (isp_builtin_get_awb_version()>>16) & 0xFF, (isp_builtin_get_awb_version()>>8) & 0xFF, isp_builtin_get_awb_version() & 0xFF);
		PRINT_ISP_INFO(sfile, "NVT_IQ_BUILTIN     v%d.%d.%d.%d\n", (isp_builtin_get_iq_version()>>24) & 0xFF, (isp_builtin_get_iq_version()>>16) & 0xFF, (isp_builtin_get_iq_version()>>8) & 0xFF, isp_builtin_get_iq_version() & 0xFF);
		PRINT_ISP_INFO(sfile, "NVT_SENSOR_BUILTIN v%d.%d.%d.%d\n", (isp_builtin_get_sensor_version()>>24) & 0xFF, (isp_builtin_get_sensor_version()>>16) & 0xFF, (isp_builtin_get_sensor_version()>>8) & 0xFF, isp_builtin_get_sensor_version() & 0xFF);
	}
	#endif

	PRINT_ISP_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_ISP_INFO(sfile, "Module Info: \n");
	#if defined(__KERNEL__)
	PRINT_ISP_INFO(sfile, "isp_id_list:  0x%X \n", isp_id_list);
	#endif
	PRINT_ISP_INFO(sfile, "AE: %s, AF: %s, AWB: %s, IQ: %s \n", pdev_info->isp_ae->name, pdev_info->isp_af->name, pdev_info->isp_awb->name, pdev_info->isp_iq->name);
	PRINT_ISP_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_ISP_INFO(sfile, "           id: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		PRINT_ISP_INFO(sfile, " %8d", id);
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_ISP_INFO(sfile, "  AE:     0x1,    AWB:    0x2,   AF:    0x4,  WDR:    0x8           \n");
	PRINT_ISP_INFO(sfile, "SHDR:    0x10,  DEFOG:   0x20,  DIS:   0x40,  RSC:   0x80 \n");
	PRINT_ISP_INFO(sfile, " ETH:   0x100,    GDC:  0x200, 3DNR:  0x400, COMB:  0x800 \n");
	PRINT_ISP_INFO(sfile, " NN0:  0x1000,    NN1: 0x2000,  NN3: 0x4000,  NN4: 0x8000 \n");
	PRINT_ISP_INFO(sfile, "THER: 0x10000,   BNR: 0x20000 \n");

	PRINT_ISP_INFO(sfile, "    CAP func.: ");
	for (id = ISP_ID_1; id < SIE_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			sie_ret = isp_get_sie_func(id, &fun_en);
			if (sie_ret == CTL_SIE_E_OK) {
				PRINT_ISP_INFO(sfile, " %8X", fun_en);
			} else {
				PRINT_ISP_INFO(sfile, "         ");
			}
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "   PROC func.: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if ((sensor_info->src_id_mask[id] != 0) || ((isp_get_ipp_table(id) != ISP_ID_IGNORE) && ((isp_get_ipp_table(id) >> 15) & 0x1) && pdev_info->ipp_to_sie_id_map_en[id])) {
			ipp_ret = isp_get_ipp_func(id, &fun_en);
			if (ipp_ret == E_OK) {
				PRINT_ISP_INFO(sfile, " %8X", fun_en);
			} else {
				PRINT_ISP_INFO(sfile, "         ");
			}
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "    CAP valid: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8d", func_info[id].sie_valid);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "   PROC valid: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8d", func_info[id].ipp_valid);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "     AE valid: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8d", func_info[id].ae_valid);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}

	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "     AF valid: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8d", func_info[id].af_valid);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "    AWB valid: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8d", func_info[id].awb_valid);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "  Defog valid: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8d", func_info[id].defog_valid);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "   SHDR valid: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8d", func_info[id].shdr_valid);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "    WDR valid: ");
	for (id = ISP_ID_1; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8d", func_info[id].wdr_valid);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "     Emulator: ");
	for (id = 0; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8s", isp_api_get_emu_enable() ? "true" : "false");
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "  Sensor name: ");
	for (id = 0; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8s", &sensor_info->name[id][4]);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "  Sensor mask: ");
	for (id = 0; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, " %8X", sensor_info->src_id_mask[id]);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "   Sensor dir: ");
	for (id = 0; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			PRINT_ISP_INFO(sfile, "  M:%1d/F:%1d", sensor_direction[id].mirror, sensor_direction[id].flip);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, " Sensor frame: ");
	for (id = 0; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if (sensor_info->src_id_mask[id] != 0) {
			isp_api_get_frame_num(id, TRUE, &frame_num);
			PRINT_ISP_INFO(sfile, " %8d", frame_num);
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, " IPP id table: ");
	for (id = 0; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if ((sensor_info->src_id_mask[id] != 0) || ((isp_get_ipp_table(id) != ISP_ID_IGNORE) && ((isp_get_ipp_table(id) >> 15) & 0x1) && pdev_info->ipp_to_sie_id_map_en[id])) {
			PRINT_ISP_INFO(sfile, " %8X", isp_get_ipp_table(id));
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	PRINT_ISP_INFO(sfile, "    IPP indep: ");
	for (id = 0; id < ISP_ID_MAX_NUM; id++) {
		if (!isp_get_id_valid(id)) {
			continue;
		}
		if ((sensor_info->src_id_mask[id] != 0) || ((isp_get_ipp_table(id) != ISP_ID_IGNORE) && ((isp_get_ipp_table(id) >> 15) & 0x1) && pdev_info->ipp_to_sie_id_map_en[id])) {
			PRINT_ISP_INFO(sfile, " %8d", isp_get_ipp_indep(id));
		} else {
			PRINT_ISP_INFO(sfile, "         ");
		}
	}
	PRINT_ISP_INFO(sfile, "\n");

	for (i = 0; i < ISP_YUV_OUT_CH; i++) {
		PRINT_ISP_INFO(sfile, " YUV output %d: ", i);
		for (id = 0; id < ISP_ID_MAX_NUM; id++) {
			if (!isp_get_id_valid(id)) {
				continue;
			}
			if ((sensor_info->src_id_mask[id] != 0) || ((isp_get_ipp_table(id) != ISP_ID_IGNORE) && ((isp_get_ipp_table(id) >> 15) & 0x1) && pdev_info->ipp_to_sie_id_map_en[id])) {
				PRINT_ISP_INFO(sfile, " %4d%4d", func_info[id].yuv_out_ch[i].w, func_info[id].yuv_out_ch[i].h);
			} else {
				PRINT_ISP_INFO(sfile, "         ");
			}
		}
		PRINT_ISP_INFO(sfile, "\n");
	}

	PRINT_ISP_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_ISP_INFO(sfile, " Low power lv: ");
	PRINT_ISP_INFO(sfile, " %8d \n", isp_api_get_low_power_lv());
	PRINT_ISP_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");
	PRINT_ISP_INFO(sfile, "    Error Cnt: ");
	PRINT_ISP_INFO(sfile, " %8d \n", isp_dbg_get_err_msg());
	PRINT_ISP_INFO(sfile, "  Warning Cnt: ");
	PRINT_ISP_INFO(sfile, " %8d \n", isp_dbg_get_wrn_msg());
	PRINT_ISP_INFO(sfile, "-----------------------------------------------------------------------------------------\r\n");

	exit:
	if (func_info != NULL) {
		isp_uti_vmem_free(func_info);
	}
	if (sensor_info != NULL) {
		isp_uti_vmem_free(sensor_info);
	}
}

#if defined(__FREERTOS)
void isp_msg_show_ca(UINT32 isp_proc_id)
#else
void isp_msg_show_ca(struct seq_file *sfile, UINT32 isp_proc_id)
#endif
{
	ISP_CA_RSLT *ca_rslt;
	UINT32 i, j;

	ca_rslt = isp_dev_get_ca(isp_proc_id);
	
	if (ca_rslt == NULL) {
		PRINT_ISP_INFO(sfile, "Get ca result fail. \n");
		return;
	}
	
	PRINT_ISP_INFO(sfile, "CA R Result: \n");
	for (i = 0; i < ISP_CA_W_WINNUM; i++) {
		for (j = 0; j < ISP_CA_H_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%4d ", ca_rslt->r[i*ISP_CA_W_WINNUM+j]);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
	PRINT_ISP_INFO(sfile, "\n");
	PRINT_ISP_INFO(sfile, "CA G Result: \n");
	for (i = 0; i < ISP_CA_W_WINNUM; i++) {
		for (j = 0; j < ISP_CA_H_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%4d ", ca_rslt->g[i*ISP_CA_W_WINNUM+j]);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
	PRINT_ISP_INFO(sfile, "\n");
	PRINT_ISP_INFO(sfile, "CA B Result: \n");
	for (i = 0; i < ISP_CA_W_WINNUM; i++) {
		for (j = 0; j < ISP_CA_H_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%4d ", ca_rslt->b[i*ISP_CA_W_WINNUM+j]);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}

}

#if defined(__FREERTOS)
void isp_msg_show_ca_acc_cnt(UINT32 isp_proc_id)
#else
void isp_msg_show_ca_acc_cnt(struct seq_file *sfile, UINT32 isp_proc_id)
#endif
{
	ISP_CA_RSLT *ca_rslt;
	UINT32 i, j;

	ca_rslt = isp_dev_get_ca(isp_proc_id);
	
	if (ca_rslt == NULL) {
		PRINT_ISP_INFO(sfile, "Get ca result fail. \n");
		return;
	}
	
	PRINT_ISP_INFO(sfile, "CA ACC Result: \n");
	for (i = 0; i < ISP_CA_W_WINNUM; i++) {
		for (j = 0; j < ISP_CA_H_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%4d ", ca_rslt->acc_cnt[i*ISP_CA_W_WINNUM+j]);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
}

#if defined(__FREERTOS)
void isp_msg_show_la(UINT32 isp_proc_id)
#else
void isp_msg_show_la(struct seq_file *sfile, UINT32 isp_proc_id)
#endif
{
	ISP_LA_RSLT *la_rslt;
	UINT32 i, j;

	la_rslt = isp_dev_get_la(isp_proc_id);
	
	if (la_rslt == NULL) {
		PRINT_ISP_INFO(sfile, "Get la result fail. \n");
		return;
	}
	
	PRINT_ISP_INFO(sfile, "LA Result: \n");
	for (i = 0; i < ISP_LA_W_WINNUM; i++) {
		for (j = 0; j < ISP_LA_H_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%4d ", la_rslt->lum_1[i*ISP_LA_W_WINNUM+j]);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
}

#if defined(__FREERTOS)
void isp_msg_show_va(UINT32 isp_proc_id)
#else
void isp_msg_show_va(struct seq_file *sfile, UINT32 isp_proc_id)
#endif
{
	ISP_IFE_VA_RSLT *ife_va_rslt;
	ISP_IPE_VA_RSLT *ipe_va_rslt;
	ISP_IFE_VA_INDEP_RSLT *ife_va_indep_rslt;
	ISP_IPE_VA_INDEP_RSLT *ipe_va_indep_rslt;
	UINT32 i, j;
	BOOL is_nt98539a = (nvt_get_chip_id() == CHIP_NS02402) ? TRUE : FALSE;
	CHAR str[10];

	if (is_nt98539a) {
		snprintf(str, sizeof(str) - 1, "PRE");
	} else {
		snprintf(str, sizeof(str) - 1, "IFE");
	}

	PRINT_ISP_INFO(sfile, "===== %s ===== \n", str);

	ife_va_rslt = isp_dev_get_ife_va(isp_proc_id);

	if (ife_va_rslt == NULL) {
		PRINT_ISP_INFO(sfile, "Get %s va result fail. \n", str);
		return;
	}

	PRINT_ISP_INFO(sfile, "%s VA g1_h Result(>>4): \n", str);
	for (i = 0; i < ISP_VA_H_WINNUM; i++) {
		for (j = 0; j < ISP_VA_W_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%8d ", ife_va_rslt->g1_h[i * ISP_VA_H_WINNUM + j] >> 4);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
	PRINT_ISP_INFO(sfile, "%s VA g1_v Result(>>4): \n", str);
	for (i = 0; i < ISP_VA_H_WINNUM; i++) {
		for (j = 0; j < ISP_VA_W_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%8d ", ife_va_rslt->g1_v[i * ISP_VA_H_WINNUM + j] >> 4);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
	PRINT_ISP_INFO(sfile, "%s VA g2_h Result(>>4): \n", str);
	for (i = 0; i < ISP_VA_H_WINNUM; i++) {
		for (j = 0; j < ISP_VA_W_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%8d ", ife_va_rslt->g2_h[i * ISP_VA_H_WINNUM + j] >> 4);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
	PRINT_ISP_INFO(sfile, "%s VA g2_v Result(>>4): \n", str);
	for (i = 0; i < ISP_VA_H_WINNUM; i++) {
		for (j = 0; j < ISP_VA_W_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%8d ", ife_va_rslt->g2_v[i * ISP_VA_H_WINNUM + j] >> 4);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}

	ife_va_indep_rslt = isp_dev_get_ife_va_indep(isp_proc_id);

	if (ife_va_indep_rslt == NULL) {
		PRINT_ISP_INFO(sfile, "Get %s va indep result fail. \n", str);
		return;
	}
	PRINT_ISP_INFO(sfile, "%s VA_INDEP Result : \n", str);
	for (i = 0; i < ISP_INDEP_VA_WIN_NUM; i++) {
		PRINT_ISP_INFO(sfile, "%8d %8d %8d %8d \n", ife_va_indep_rslt->g1_h[i], ife_va_indep_rslt->g1_v[i], ife_va_indep_rslt->g2_h[i], ife_va_indep_rslt->g2_v[i]);
	}

	if (is_nt98539a) {
		return;
	}

	PRINT_ISP_INFO(sfile, "===== IPE ===== \n");

	ipe_va_rslt = isp_dev_get_ipe_va(isp_proc_id);

	if (ipe_va_rslt == NULL) {
		PRINT_ISP_INFO(sfile, "Get ipe va result fail. \n");
		return;
	}
	PRINT_ISP_INFO(sfile, "IPE VA g1_h Result(>>4): \n");
	for (i = 0; i < ISP_VA_H_WINNUM; i++) {
		for (j = 0; j < ISP_VA_W_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%8d ", ipe_va_rslt->g1_h[i * ISP_VA_H_WINNUM + j] >> 4);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
	PRINT_ISP_INFO(sfile, "IPE VA g1_v Result(>>4): \n");
	for (i = 0; i < ISP_VA_H_WINNUM; i++) {
		for (j = 0; j < ISP_VA_W_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%8d ", ipe_va_rslt->g1_v[i * ISP_VA_H_WINNUM + j] >> 4);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
	PRINT_ISP_INFO(sfile, "IPE VA g2_h Result(>>4): \n");
	for (i = 0; i < ISP_VA_H_WINNUM; i++) {
		for (j = 0; j < ISP_VA_W_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%8d ", ipe_va_rslt->g2_h[i * ISP_VA_H_WINNUM + j] >> 4);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}
	PRINT_ISP_INFO(sfile, "IPE VA g2_v Result(>>4): \n");
	for (i = 0; i < ISP_VA_H_WINNUM; i++) {
		for (j = 0; j < ISP_VA_W_WINNUM; j++) {
			PRINT_ISP_INFO(sfile, "%8d ", ipe_va_rslt->g2_v[i * ISP_VA_H_WINNUM + j] >> 4);
		}
		PRINT_ISP_INFO(sfile, "\n");
	}

	ipe_va_indep_rslt = isp_dev_get_ipe_va_indep(isp_proc_id);

	if (ipe_va_indep_rslt == NULL) {
		PRINT_ISP_INFO(sfile, "Get ipe va indep result fail. \n");
		return;
	}
	PRINT_ISP_INFO(sfile, "IPE VA_INDEP Result : \n");
	for (i = 0; i < ISP_INDEP_VA_WIN_NUM; i++) {
		PRINT_ISP_INFO(sfile, "%8d %8d %8d %8d \n", ipe_va_indep_rslt->g1_h[i], ipe_va_indep_rslt->g1_v[i], ipe_va_indep_rslt->g2_h[i], ipe_va_indep_rslt->g2_v[i]);
	}
}

#if defined(__FREERTOS)
void isp_msg_show_histo(UINT32 isp_proc_id)
#else
void isp_msg_show_histo(struct seq_file *sfile, UINT32 isp_proc_id)
#endif
{
	ISP_HISTO_RSLT *histo_rslt;
	UINT32 i;

	histo_rslt = isp_dev_get_histo(isp_proc_id);
	
	if (histo_rslt == NULL) {
		PRINT_ISP_INFO(sfile, "Get histo result fail. \n");
		return;
	}
	
	PRINT_ISP_INFO(sfile, "HISTO Result: \n");
	PRINT_ISP_INFO(sfile, "pre : ");
	for (i = 0; i < ISP_HISTO_MAX_SIZE; i++) {
		PRINT_ISP_INFO(sfile, "%4d ", histo_rslt->hist_stcs_pre_wdr[i]);
	}
	PRINT_ISP_INFO(sfile, "\n");
	PRINT_ISP_INFO(sfile, "post : ");
	for (i = 0; i < ISP_HISTO_MAX_SIZE; i++) {
		PRINT_ISP_INFO(sfile, "%4d ", histo_rslt->hist_stcs_post_wdr[i]);
	}
	PRINT_ISP_INFO(sfile, "\n");
}

