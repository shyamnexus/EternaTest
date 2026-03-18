#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of_device.h>
#include <plat/top.h>
#include <kwrap/verinfo.h>
#include "kwrap/debug.h"
#include <nvtmpp_init.h>
#include <kdrv_builtin.h>
#include "sie_init.h"
#include "isp_builtin_int.h"
#include "nvt_vdocdc_main.h"
#include "kdrv_ipp_builtin.h"
#include "bridge.h"
#include "vdoenc_builtin.h"
#include "audcap_builtin.h"
#include "kflow_ai_builtin.h"
#include "kdrv_builtin_debug.h" //must behind include kwarp/debug.h
#include "osg_internal.h"


#define SIE_SKIP_ID_BIT         0x42495353 //MAKEFOURCC('S', 'S', 'I', 'B');

#define IPP_RTOS_BIT            0x42525049 //MAKEFOURCC('I', 'P', 'R', 'B');
#define IPP_RTOS_CNT            0x43525049 //MAKEFOURCC('I', 'P', 'R', 'C');
#define IPP_RTOS_ERR            0x45525049 //MAKEFOURCC('I', 'P', 'R', 'E');
#define IPP_RTOS_FRM            0x46525049 //MAKEFOURCC('I', 'P', 'R', 'F');

#define AI_DETECT_RESULT        0x52444941 //MAKEFOURCC('A', 'I', 'D', 'R');

#define VENC_PRE_INIT           0x49504556 //MAKEFOURCC('V', 'E', 'P', 'I');
#define VENC_PRE_OBJ            0x4A424F50 //MAKEFOURCC('P', 'O', 'B', 'J');
#define VENC_PRE_VAR            0x52415650 //MAKEFOURCC('P', 'V', 'A', 'R');
#define VENC_EN_INT_FIRST       0x544E4945 //MAKEFOURCC('E', 'I', 'N', 'T');
#define VENC_PRE_NUM_0          0x304E5056 //MAKEFOURCC('V', 'P', 'N', '0');
#define AMP_TAKEOVER_FROM_RTOS  0x52465441 //MAKEFOURCC('A', 'T', 'F', 'R');

extern int nvt_ime_builtin_module_init(void);
extern int nvt_ime_builtin_module_exit(void);
extern int nvt_sie_builtin_module_init(void);
extern int nvt_sie_builtin_module_exit(void);
extern int nvt_vie_module_init(void);
extern int nvt_vie_module_exit(void);
extern int nvt_eac_module_init(void);
extern int nvt_eac_module_exit(void);
extern int nvt_dai_module_init(void);
extern int nvt_dai_module_exit(void);
extern int nvt_builtinjpg_module_init(void);
extern int nvt_builtinjpg_module_exit(void);
extern int nvt_ai_module_init(void);
extern void nvt_ai_module_exit(void);
extern int nvt_graphic_module_init(void);
extern void nvt_graphic_module_exit(void);
extern int nvt_kdrv_graph_module_init(void);
extern int nvt_kdrv_graph_module_exit(void);

extern void nvt_bootts_add_ts(char *name);

int __init kdrv_builtin_init(void)
{
	int ret = 0;
	int grph_init = 0;

#if defined(_539A_)
	if (nvt_get_chip_id() != CHIP_NS02402) {
		pr_err("invalid chip id, insmod wrong kdrv_builtin ko?\n");
		return -EINVAL;
	}
#else
	if (nvt_get_chip_id() != CHIP_NS02302) {
		pr_err("invalid chip id, insmod wrong kdrv_builtin ko?\n");
		return -EINVAL;
	}
#endif

	printk("%s init module ==========\n",__func__);
	nvt_bootts_add_ts("builtin");

	#if FASTBOOT_FUNC_EN
	if (kdrv_builtin_is_fastboot()) {
		unsigned int take_over = 1;
		kdrv_bridge_map(); //map bridge memory for all built-in
		kdrv_fdt_map();
		if (kdrv_bridge_set_tag(AMP_TAKEOVER_FROM_RTOS, take_over) == 0) {

			while (take_over) {
				if (kdrv_bridge_get_tag(AMP_TAKEOVER_FROM_RTOS, &take_over) != 0) {
					DBG_ERR("failed to get take over status-2.\n");
					break;
				}
			}
		} else {
			DBG_ERR("failed to get take over status-1.\n");
		}
	}
	if (kdrv_builtin_is_fastboot()) {
		nvtmpp_init_mmz();
	}
	#endif
	if ((ret=nvt_ime_builtin_module_init()) !=0 ) {
		return ret;
	}
	#if FASTBOOT_FUNC_EN
	if (kdrv_builtin_is_fastboot()) {
		unsigned int sensor_name_1 = 0, sensor_name_2 = 0;
		UINT32 rst_id = 0;

		// get sensor 1
		kdrv_bridge_get_tag(SENSOR_PRESET_NAME, &sensor_name_1);
		// get sensor 2
		kdrv_bridge_get_tag(SENSOR_PRESET_NAME_2, &sensor_name_2);
		if (sensor_name_1 != 0xFFFFFFFF) {
			rst_id |= 0x1;
		}
		if (sensor_name_2 != 0xFFFFFFFF) {
			rst_id |= 0x2;
		}
		//If RTOS has enabled SIE, the interrupt status will trigger ISR before driver ready.
		sie_builtin_rst(rst_id);
	}
	#endif
	if ((ret=nvt_sie_builtin_module_init()) !=0 ) {
		return ret;
	}

	if ((ret = nvt_vie_module_init()) != 0) {
		return ret;
	}

	if ((ret=nvt_vdocdc_module_init()) !=0 ) {
		return ret;
	}

	if ((ret=nvt_eac_module_init()) !=0 ) {
		return ret;
	}

	if ((ret=nvt_dai_module_init()) !=0 ) {
		return ret;
	}

	if ((ret=osg_module_init()) !=0 ) {
		return ret;
	}

	if ((ret=nvt_builtinjpg_module_init()) !=0 ) {
		return ret;
	}

	if (kdrv_builtin_is_fastboot()) {
		if ((ret = nvt_ai_module_init()) != 0) {
			return ret;
		}
	}
	#if FASTBOOT_FUNC_EN
	if (kdrv_builtin_is_fastboot()) {
		NVTMPP_FASTBOOT_MEM_S *p_fastboot_mem;
		KDRV_IPP_BUILTIN_INIT_INFO ipp_init_info;
		KDRV_IPP_BUILTIN_IMG_INFO img_info[KDRV_IPP_BUILTIN_PATH_ID_MAX] = {0};
		VDOENC_BUILTIN_INIT_INFO venc_info[BUILTIN_VDOENC_PATH_ID_MAX] = {0};
		SIE_BUILTIN_INIT_INFO sie_init_info;
		AUDCAP_BUILTIN_INIT_INFO acap_info;
		UINT32 i;
		#if (NVT_FAST_AI_FLOW)
		KFLOW_AI_BUILTIN_INIT_INFO ai_info;
		#endif
		UINT32 pool_id;
		unsigned int sensor_name_1 = 0, sensor_name_2 = 0;
		UINT32 vprc_src_dev = 0, vprc_src_path = 0;
		UINT32 venc_pre_init = 0, pre_enc_var_addr = 0, pre_enc_obj_addr = 0, en_h26x_int_first = 0;
		UINT32 sie_skip_id_bit = 0;

		UINT32 ipp_rtos_bit = 0;
		UINT32 ipp_rtos_cnt = 0;
		UINT32 ipp_rtos_err = 0;
		UINT32 ipp_rtos_frm = 0;

		// yuv queue func
		kdrv_builtin_parse_dts_yuv();

		// get fastboot memory from dts
		if (nvtmpp_parse_fastboot_mem_dt() < 0) {
			return ret;
		}
		p_fastboot_mem = nvtmpp_get_fastboot_mem();

		if(p_fastboot_mem == NULL){
			DBG_ERR("p_fastboot_mem is null\n");
			return -1;
		}

		// get sensor 1
		kdrv_bridge_get_tag(SENSOR_PRESET_NAME, &sensor_name_1);
		// get sensor 2
		kdrv_bridge_get_tag(SENSOR_PRESET_NAME_2, &sensor_name_2);

		// init isp
		#if (NVT_FAST_ISP_FLOW)
		isp_builtin_init();
		#else
		#if !defined(CONFIG_NVT_FAST_ISP_FLOW)
		printk("ERR: CONFIG_NVT_FAST_ISP_FLOW not define!!! \n");
		#endif
		#endif
		// init ipp
		memset((void *)&ipp_init_info, 0, sizeof(KDRV_IPP_BUILTIN_INIT_INFO));

		ipp_init_info.hdl_num = 2;
		ipp_init_info.hdl_info[0].name = "vdoprc0";
		ipp_init_info.hdl_info[0].isp_id = 0x0;
		ipp_init_info.hdl_info[0].src_sie_id_bit = 0x1;
		ipp_init_info.hdl_info[0].ctrl_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC_CTRL].addr;
		ipp_init_info.hdl_info[0].ctrl_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC_CTRL].size;
		ipp_init_info.hdl_info[0].shdr_ring_buf_addr[0] = p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].addr;
		ipp_init_info.hdl_info[0].shdr_ring_buf_size[0] = p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].size;

		ipp_init_info.hdl_info[1].name = "vdoprc1";
		ipp_init_info.hdl_info[1].isp_id = 0x01;
		ipp_init_info.hdl_info[1].src_sie_id_bit = 0x2;
		ipp_init_info.hdl_info[1].ctrl_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC1_CTRL].addr;
		ipp_init_info.hdl_info[1].ctrl_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VPRC1_CTRL].size;
		ipp_init_info.hdl_info[1].shdr_ring_buf_addr[0] = 0;
		ipp_init_info.hdl_info[1].shdr_ring_buf_size[0] = 0;

		if (sensor_name_1 != 0xFFFFFFFF) {
			ipp_init_info.valid_src_id_bit |= 0x1;
		}
		if (sensor_name_2 != 0xFFFFFFFF) {
			ipp_init_info.valid_src_id_bit |= 0x2;
		}

		kdrv_bridge_get_tag(IPP_RTOS_BIT, &ipp_rtos_bit);
		printk("ipp_rtos_bit %d ==========\n", (int)ipp_rtos_bit);

		kdrv_bridge_get_tag(IPP_RTOS_CNT, &ipp_rtos_cnt);
		printk("ipp_rtos_cnt %d ==========\n", (int)ipp_rtos_cnt);

		kdrv_bridge_get_tag(IPP_RTOS_ERR, &ipp_rtos_err);
		printk("ipp_rtos_err %d ==========\n", (int)ipp_rtos_err);

		kdrv_bridge_get_tag(IPP_RTOS_FRM, &ipp_rtos_frm);
		printk("ipp_rtos_frm %d ==========\n", (int)ipp_rtos_frm);

		kdrv_bridge_get_tag(SENSOR_CHGMODE_FPS, &ipp_init_info.sensor_fps);
		printk("ipp_sesor_fps %d ==========\n", (int)ipp_init_info.sensor_fps);

		ipp_init_info.rtos_ipp_bit = ipp_rtos_bit;
		ipp_init_info.ipp_rtos_frm_idx = ipp_rtos_frm;

		if(kdrv_ipp_builtin_init(&ipp_init_info)){
			DBG_ERR("ipp buiiltin init fail\n");
			return ret;
		}

		if ((ret=nvt_graphic_module_init()) !=0 ) {
			DBG_ERR("fail to init nvt graphic\n");
			return ret;
		}
		if ((ret=nvt_kdrv_graph_module_init()) !=0 ) {
			DBG_ERR("fail to init kdrv graphic\n");
			return ret;
		}
		grph_init = 1;

		// init codec
		vdoenc_builtin_get_of_node_venc();

		kdrv_bridge_get_tag(VENC_PRE_INIT, &venc_pre_init);
		vdoenc_builtin_set_pre_init_flag(venc_pre_init);

		kdrv_bridge_get_tag(VENC_PRE_OBJ, &pre_enc_obj_addr);
		vdoenc_builtin_set_enc_obj_addr(pre_enc_obj_addr);

		kdrv_bridge_get_tag(VENC_PRE_VAR, &pre_enc_var_addr);
		vdoenc_builtin_set_enc_var_addr(pre_enc_var_addr);

		kdrv_bridge_get_tag(VENC_EN_INT_FIRST, &en_h26x_int_first);
		vdoenc_builtin_set_en_lnx_h26x_int_first(en_h26x_int_first);

		for (i = 0; i < BUILTIN_VDOENC_PATH_ID_MAX; i++) {
			UINT32 venc_pre_num = 0;

			kdrv_bridge_get_tag(VENC_PRE_NUM_0 + i, &venc_pre_num);
			vdoenc_builtin_set_pre_num(i, venc_pre_num);

			ret = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_ENC_EN);
			if (ret == 1) {
				vprc_src_dev = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SRC_VPRC_DEV);
				vprc_src_path = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_SRC_VPRC_PATH);
				if (vprc_src_dev == 0) {
					img_info[vprc_src_path] = kdrv_ipp_builtin_get_path_info(0x0, vprc_src_path);
				} else if (vprc_src_dev == 1) {
					img_info[vprc_src_path] = kdrv_ipp_builtin_get_path_info(0x01, vprc_src_path);
				}
				venc_info[i].width = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_WIDTH);
				venc_info[i].height = VdoEnc_builtin_get_dtsi_param(i, BUILTIN_VDOENC_DTSI_PARAM_HEIGHT);
				venc_info[i].fmt = (UINT32)img_info[vprc_src_path].fmt;
			}
		}
		venc_info[BUILTIN_VDOENC_PATH_ID_0].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_0_MAX].addr;
		venc_info[BUILTIN_VDOENC_PATH_ID_0].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_0_MAX].size;
		venc_info[BUILTIN_VDOENC_PATH_ID_1].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_1_MAX].addr;
		venc_info[BUILTIN_VDOENC_PATH_ID_1].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_1_MAX].size;
		venc_info[BUILTIN_VDOENC_PATH_ID_2].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_2_MAX].addr;
		venc_info[BUILTIN_VDOENC_PATH_ID_2].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_2_MAX].size;
		venc_info[BUILTIN_VDOENC_PATH_ID_3].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_3_MAX].addr;
		venc_info[BUILTIN_VDOENC_PATH_ID_3].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_3_MAX].size;
		venc_info[BUILTIN_VDOENC_PATH_ID_4].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_4_MAX].addr;
		venc_info[BUILTIN_VDOENC_PATH_ID_4].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_4_MAX].size;
		venc_info[BUILTIN_VDOENC_PATH_ID_5].max_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_5_MAX].addr;
		venc_info[BUILTIN_VDOENC_PATH_ID_5].max_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VENC_5_MAX].size;

		if ((ret=VdoEnc_builtin_init(venc_info)) !=0 ) {
			return ret;
		}

		// init sie
		sie_init_info.ring_buf_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].addr;
		sie_init_info.ring_buf_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_VCAP_SHDR_BUF].size;

		kdrv_bridge_get_tag(SIE_SKIP_ID_BIT, &sie_skip_id_bit);

		sie_init_info.sie_id_bit = 0x0;
		if (sensor_name_1 != 0xFFFFFFFF) {
			sie_init_info.sie_id_bit = 0x1;
		}
		// get sensor 2
		kdrv_bridge_get_tag(SENSOR_PRESET_NAME_2, &sensor_name_2);
		if (sensor_name_2 != 0xFFFFFFFF) {
			unsigned int isp_sensor_path_2 = 0;
			kdrv_bridge_get_tag(ISP_PATH_2, &isp_sensor_path_2);
			sie_init_info.sie_id_bit |= 1 << isp_sensor_path_2;
		}
		sie_init_info.rtos_streaming_id_bit = sie_skip_id_bit;
		sie_builtin_init(&sie_init_info);



		acap_info.ctrl_blk_addr = p_fastboot_mem->pv_pools[FBOOT_POOL_ACAP_CTRL].addr;
		acap_info.ctrl_blk_size = p_fastboot_mem->pv_pools[FBOOT_POOL_ACAP_CTRL].size;

		if ((acap_info.ctrl_blk_addr != 0) && (acap_info.ctrl_blk_size != 0)) {
			audcap_builtin_init(&acap_info);
		}

		// init ai
		#if (NVT_FAST_AI_FLOW)
		if (kdrv_builtin_is_fastboot()) {
			for (pool_id = 0; pool_id < FBOOT_MISC_CPOOL_MAX; pool_id++) {
				if (p_fastboot_mem->misc_cpool[pool_id].blk_cnt > FBOOT_MISC_CPOOLBLK_MAX_CNT) {
					p_fastboot_mem->misc_cpool[pool_id].blk_cnt = FBOOT_MISC_CPOOLBLK_MAX_CNT;
				}
				for (i = 0; i < p_fastboot_mem->misc_cpool[pool_id].blk_cnt; i++) {
					// check pool_type(0x6a) for CNN_POOL
					if (p_fastboot_mem->misc_cpool[pool_id].pool_type == 0x6a &&
						p_fastboot_mem->misc_cpool[pool_id].blk[i].addr != 0) {
						ai_info.blk_addr = p_fastboot_mem->misc_cpool[pool_id].blk[i].addr;
						ai_info.blk_size = p_fastboot_mem->misc_cpool[pool_id].blk[i].size;
					}
				}
			}
			kflow_ai_builtin_init(&ai_info);
		}
		#endif

		for (pool_id = 0; pool_id < FBOOT_MISC_CPOOL_MAX; pool_id++) {
			if (p_fastboot_mem->misc_cpool[pool_id].blk_cnt > FBOOT_MISC_CPOOLBLK_MAX_CNT) {
				p_fastboot_mem->misc_cpool[pool_id].blk_cnt = FBOOT_MISC_CPOOLBLK_MAX_CNT;
			}
			for (i = 0; i < p_fastboot_mem->misc_cpool[pool_id].blk_cnt; i++) {
				// check pool_type(103) for HD_COMMON_MEM_OSG_POOL
				if (p_fastboot_mem->misc_cpool[pool_id].pool_type == 103 &&
					p_fastboot_mem->misc_cpool[pool_id].blk[i].addr != 0) {
					if(fastboot_osg_demo_init(&(p_fastboot_mem->misc_cpool[pool_id]))){
						DBG_ERR("fail to initialize customer's OSG setting\n");
						return -1;
					}
					break;
				}
			}
		}
	}

	if (kdrv_builtin_is_fastboot()) {
		kdrv_fdt_unmap();
		kdrv_bridge_unmap();
	}
	#endif

	if(grph_init == 0){
		if ((ret=nvt_graphic_module_init()) !=0 ) {
			DBG_ERR("fail to init nvt graphic\n");
			return ret;
		}
		if ((ret=nvt_kdrv_graph_module_init()) !=0 ) {
			DBG_ERR("fail to init kdrv graphic\n");
			return ret;
		}
	}

#if defined(CONFIG_NVT_FAST_SMP_FLOW) && defined(__KDRV_BUILTIN_USE_INITCALL)
	if (1) {
		extern int nvt_cpu_plug(unsigned int cpu);
		nvt_cpu_plug(1);
	}
#endif
    printk("%s init module ==========\n",__func__);
    nvt_bootts_add_ts("builtin");

	return ret;
}

void __exit kdrv_builtin_exit(void)
{

	nvt_ime_builtin_module_exit();
	nvt_sie_builtin_module_exit();
	nvt_vie_module_exit();
	nvt_dai_module_exit();
	nvt_eac_module_exit();
	nvt_builtinjpg_module_exit();
	nvt_vdocdc_module_exit();
	if (kdrv_builtin_is_fastboot()) {
		nvt_ai_module_exit();
	}
	#if (NVT_FAST_ISP_FLOW)
	isp_builtin_uninit();
	#endif
	nvt_kdrv_graph_module_exit();
	nvt_graphic_module_exit();
	VdoEnc_BuiltIn_TskStop_H26X();
	VdoEnc_BuiltIn_TskStop_JPEG();
	VdoEnc_BuiltIn_Uninstall_ID();
	#if (NVT_FAST_AI_FLOW)
	kflow_ai_builtin_exit();
	#endif
	if (kdrv_builtin_is_fastboot()) {
		nvtmpp_exit_mmz();
	}
}

#ifdef __KDRV_BUILTIN_USE_INITCALL
fs_initcall(kdrv_builtin_init);
__exitcall(kdrv_builtin_exit);
#else
module_init(kdrv_builtin_init);
module_exit(kdrv_builtin_exit);
#endif

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("kdrv built-in driver");
MODULE_LICENSE("GPL");

#else
#endif
