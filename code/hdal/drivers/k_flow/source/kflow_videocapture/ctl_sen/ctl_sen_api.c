#include "ctl_sen.h"
#include "kdrv_videocapture/kdrv_sie.h"
#include "ctl_sen_api.h"
#include "ctl_sen_dbg.h"
#include "ctl_sen_int.h"
#include "ctl_sen_debug_infor_int.h"
#include "kflow_common/nvtmpp.h"
#include <kwrap/cmdsys.h>
#include "kwrap/file.h"
#include "kwrap/mem.h"
#include "kwrap/util.h"
#if defined(__FREERTOS) || defined(__ECOS) || defined(__UITRON)
#include "stdlib.h"
#else
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h> // header file Dma(cache handle)
#include <linux/soc/nvt/fmem.h>
#endif


#if defined(__FREERTOS) || defined(__ECOS) || defined(__UITRON)
#define CLT_SEN_REGVALUE                uint32_t
#define CLT_SEN_OUTW(addr,value)    (*(CLT_SEN_REGVALUE volatile *)(addr) = (CLT_SEN_REGVALUE)(value))
#define CLT_SEN_INW(addr)           (*(CLT_SEN_REGVALUE volatile *)(addr))
#define WRITE_ENG_REG(addr, value) CLT_SEN_OUTW(addr, value)//iowrite32(value, (void __iomem *)(0xFD000000 + (addr & 0xFFFFF)))
#define READ_ENG_REG(addr) CLT_SEN_INW(addr) //ioread32((void __iomem *)(addr | 0xFD000000))
#define WRITE_ENG_REG_BIT(addr, bit, lv) WRITE_ENG_REG(addr, (READ_ENG_REG(addr) & (~(0x1 << bit))) | ((BOOL)lv << bit))

#define simple_strtoul(param1, param2, param3) strtoul(param1, param2, param3)

#define EINVAL (1)

#else
#define WRITE_ENG_REG(addr, value) iowrite32(value, (void __iomem *)(0xFD000000 + (addr & 0xFFFFF)))
#define READ_ENG_REG(addr) ioread32((void __iomem *)(addr | 0xFD000000))
#define WRITE_ENG_REG_BIT(addr, bit, lv) WRITE_ENG_REG(addr, (READ_ENG_REG(addr) & (~(0x1 << bit))) | ((BOOL)lv << bit))
#endif

BOOL nvt_ctl_sen_api_w_reg(unsigned char argc, char **pargv)
{
	ER rt = E_OK;

	UINT32 sen_id = 0;
	CTL_SEN_CMD cmd = {0};

	/*[sen_id][ui_addr][ui_data_len][ui_data[0]][ui_data[1]]*/
	if (argc != 5) {
		ctl_sen_dbg_err("wrong argument:%d\r\n", argc);
		return -EINVAL;
	}

	sen_id = simple_strtoul(pargv[0], NULL, 0);
	cmd.addr = simple_strtoul(pargv[1], NULL, 0);
	cmd.data_len = simple_strtoul(pargv[2], NULL, 0);
	cmd.data[0] = simple_strtoul(pargv[3], NULL, 0);
	cmd.data[1] = simple_strtoul(pargv[4], NULL, 0);

	ctl_sen_dbg_dump("start: sen_id %d, addr 0x%.8x, len %d, data %d(0x%x) %d(0x%x)\r\n", sen_id, cmd.addr, cmd.data_len, cmd.data[0], cmd.data[0], cmd.data[1], cmd.data[1]);

	rt = ctl_sen_write_reg(sen_id, &cmd);

	ctl_sen_dbg_dump("rt = %d\r\n", rt);

	return (int)rt;
}

BOOL nvt_ctl_sen_api_r_reg(unsigned char argc, char **pargv)
{
	ER rt = E_OK;

	UINT32 sen_id = 0;
	CTL_SEN_CMD cmd = {0};

	/*[sen_id][ui_addr][ui_data_len]*/
	if (argc != 3) {
		ctl_sen_dbg_err("wrong argument:%d", argc);
		return -EINVAL;
	}

	sen_id = simple_strtoul(pargv[0], NULL, 0);
	cmd.addr = simple_strtoul(pargv[1], NULL, 0);
	cmd.data_len = simple_strtoul(pargv[2], NULL, 0);

	ctl_sen_dbg_dump("start: id %d, addr 0x%.8x\r\n", sen_id, cmd.addr);

	rt = ctl_sen_read_reg(sen_id, &cmd);

	ctl_sen_dbg_dump("rt = %d: data[0] 0x%x, data[1] 0x%x\r\n", rt, cmd.data[0], cmd.data[1]);

	return (int)rt;
}

BOOL nvt_ctl_sen_api_info(unsigned char argc, char **pargv)
{
	ctl_sen_dbg_dump_info(vk_printk);
	ctl_sen_dbg_dump_process(vk_printk);
	return TRUE;
}

BOOL nvt_ctl_sen_api_signal(unsigned char argc, char **pargv)
{
	UINT32 i;
	CTL_SEN_CHGMODE_INFO chgmode_info;
	INT32 rt;

	for (i = CTL_SEN_MIN_SEN_ID; i <= CTL_SEN_MAX_SEN_ID; i++) {
		if (ctl_sen_status(i) & CTL_SEN_STATUS_OPEN) {
			rt = ctl_sen_senid_info(i, NULL, NULL, &chgmode_info);
			if ((rt == CTL_SEN_E_OK) && (chgmode_info.output_dest != CTL_SEN_MAP_NULL)) {
				if (chgmode_info.output_dest >= ctl_sen_output_vie_base) {
					ctl_sen_dbg_dump_signal(vk_printk, i, CTL_SEN_INTE_VD_TO_VIE0_CH0 + (chgmode_info.output_dest - ctl_sen_output_vie_base));
					ctl_sen_dbg_dump_signal(vk_printk, i, CTL_SEN_INTE_FMD_TO_VIE0_CH0 + (chgmode_info.output_dest - ctl_sen_output_vie_base));
				} else {
					ctl_sen_dbg_dump_signal(vk_printk, i, CTL_SEN_INTE_VD_TO_SIE0 + chgmode_info.output_dest);
					ctl_sen_dbg_dump_signal(vk_printk, i, CTL_SEN_INTE_FMD_TO_SIE0 + chgmode_info.output_dest);
				}
			} else {
				ctl_sen_dbg_dump_signal(vk_printk, i, CTL_SEN_INTE_VD_TO_SIE0);
				ctl_sen_dbg_dump_signal(vk_printk, i, CTL_SEN_INTE_FMD_TO_SIE0);
			}
		}
	}
	return TRUE;
}

BOOL nvt_ctl_sen_api_dbg(unsigned char argc, char **pargv)
{
	ER rt = E_OK;
	UINT32 id = CTL_SEN_ID_1;
	CTL_SEN_DBG_SEL dbg_sel;
	UINT32 param = 0;

	/*[id][CTL_SEN_DBG_SEL][param]*/
	if (argc < 2) {
		ctl_sen_dbg_err("wrong argument:%d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(pargv[0], NULL, 0);
	dbg_sel = simple_strtoul(pargv[1], NULL, 0);
	if (argc > 2) {
		param = simple_strtoul(pargv[2], NULL, 0);
	}

	ctl_sen_dbg_ind("start: sen_id %d dbg_sel 0x%.8x\r\n", id, dbg_sel);

	if ((dbg_sel & CTL_SEN_DBG_SEL_DUMP) | (dbg_sel & CTL_SEN_DBG_SEL_DUMP_EXT) | (dbg_sel & CTL_SEN_DBG_SEL_DUMP_DRV) |
		(dbg_sel & CTL_SEN_DBG_SEL_DUMP_MAP_TBL) | (dbg_sel & CTL_SEN_DBG_SEL_DUMP_PROC_TIME) | (dbg_sel & CTL_SEN_DBG_SEL_DUMP_CTL)) {
		ctl_sen_dbg_dump_info(vk_printk);
		ctl_sen_dbg_dump_process(vk_printk);
	}

	if (dbg_sel & CTL_SEN_DBG_SEL_WAIT_VD_TO_SIE) {
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_VD_TO_SIE0 + param);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAIT_FMD_TO_SIE) {
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_FMD_TO_SIE0 + param);
	}

	if (dbg_sel & CTL_SEN_DBG_SEL_WAIT_VD_TO_VIE) {
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_VD_TO_VIE0_CH0 + param);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAIT_FMD_TO_VIE) {
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_FMD_TO_VIE0_CH0 + param);
	}

	if ((dbg_sel & CTL_SEN_DBG_SEL_WAITVD_TGE) == CTL_SEN_DBG_SEL_WAITVD_TGE) {
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_TGE_VD);
	}

	// ============== Backward compatible ====================
	if (dbg_sel & CTL_SEN_DBG_SEL_WAITVD) {
		ctl_sen_dbg_err("force to CTL_SEN_DBG_SEL_WAIT_VD_TO_SIE%d\r\n", 0);
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_VD_TO_SIE0);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAITVD2) {
		ctl_sen_dbg_err("force to CTL_SEN_DBG_SEL_WAIT_VD_TO_SIE%d\r\n", 1);
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_VD_TO_SIE1);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAITVD3) {
		ctl_sen_dbg_err("force to CTL_SEN_DBG_SEL_WAIT_VD_TO_SIE%d\r\n", 2);
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_VD_TO_SIE2);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAITVD4) {
		ctl_sen_dbg_err("force to CTL_SEN_DBG_SEL_WAIT_VD_TO_SIE%d\r\n", 3);
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_VD_TO_SIE3);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAITFMD) {
		ctl_sen_dbg_err("force to CTL_SEN_DBG_SEL_WAIT_FMD_TO_SIE%d\r\n", 0);
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_FMD_TO_SIE0);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAITFMD2) {
		ctl_sen_dbg_err("force to CTL_SEN_DBG_SEL_WAIT_FMD_TO_SIE%d\r\n", 1);
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_FMD_TO_SIE1);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAITFMD3) {
		ctl_sen_dbg_err("force to CTL_SEN_DBG_SEL_WAIT_FMD_TO_SIE%d\r\n", 2);
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_FMD_TO_SIE2);
	}
	if (dbg_sel & CTL_SEN_DBG_SEL_WAITFMD4) {
		ctl_sen_dbg_err("force to CTL_SEN_DBG_SEL_WAIT_FMD_TO_SIE%d\r\n", 3);
		ctl_sen_dbg_dump_signal(vk_printk, id, CTL_SEN_INTE_FMD_TO_SIE3);
	}



	return (int)rt;
}

BOOL nvt_ctl_sen_api_set_dbg_lv(unsigned char argc, char **pargv)
{
	ER rt = E_OK;
	UINT32 err = 0;

	/*[CTL_SEN_DBG_LV]*/
	if (argc < 1) {
		ctl_sen_dbg_err("wrong argument:%d", argc);
		return -EINVAL;
	}

	ctl_sen_dbg_level = simple_strtoul(pargv[0], NULL, 0);
	ctl_sen_dbg_dump("dbg_lv %d\r\n", ctl_sen_dbg_level);

	if (argc >= 2) {
		ctl_sen_dbg_trc_type = simple_strtoul(pargv[1], NULL, 0);
		ctl_sen_dbg_dump("trc type %d\r\n", ctl_sen_dbg_trc_type);

		if (strcmp(pargv[1], "none") == 0) {
			ctl_sen_dbg_trc_type = CTL_SEN_DBG_TRC_NONE;
		} else if (strcmp(pargv[1], "getset") == 0) {
			ctl_sen_dbg_trc_type = CTL_SEN_DBG_TRC_GET_SET;
		} else if (strcmp(pargv[1], "reg") == 0) {
			ctl_sen_dbg_trc_type = CTL_SEN_DBG_TRC_REG;
		} else if (strcmp(pargv[1], "all") == 0) {
			ctl_sen_dbg_trc_type = CTL_SEN_DBG_TRC_ALL;
		} else {
			ctl_sen_dbg_dump("Unknown type %s\r\n", pargv[1]);
			err = 1;
		}

		if (err == 0) {
			ctl_sen_dbg_dump("set trc type to %s\r\n", pargv[1]);
		}
	}

	return (int)rt;
}


#if CTL_SEN_TEST

typedef struct {

	BOOL init;

	UINT32 chip_id;
	UINT32 frm_idx;
	CTL_SEN_MODE sen_mode;
	UINT32 fps;
	UINT32 output_dest;

	CTL_SEN_INIT_CFG_OBJ init_cfg_obj;
	CTL_SEN_PINMUX init_cfg_obj_pinmux[CTL_SEN_PINMUX_MAX_NUM];

} CTL_SEN_TEST_PARAM;

static UINT32 ctl_sen_g_init_cnt = 0;
static UINT32 ctl_sen_add_map_cnt = 0;
static CTL_SEN_TEST_PARAM ctl_sen_test_param[CTL_SEN_NUM_SEN_ID] = {0};


#define CONFI_DEV_CSI0_SIE0 0
#define CONFI_DEV_CSI1_SIE2 1
#define CTL_SEN_CONFIG_DEV CONFI_DEV_CSI0_SIE0

CTL_SEN_INIT_CFG_OBJ *init_cfg_obj;
static void _ctl_sen_test_param_init(UINT32 sen_id)
{

	if (ctl_sen_test_param[sen_id].init) {
		// avoid reset user parm.
		return;
	}
	ctl_sen_test_param[sen_id].init = TRUE;


	/*
	    hdal param
	*/
	ctl_sen_test_param[sen_id].chip_id = 0;
	ctl_sen_test_param[sen_id].frm_idx = sen_id;
	ctl_sen_test_param[sen_id].sen_mode = CTL_SEN_MODE_1;
	ctl_sen_test_param[sen_id].fps = 3000;
	ctl_sen_test_param[sen_id].output_dest = ctl_sen_output_sie_0 + sen_id;

#if (CTL_SEN_CONFIG_DEV == CONFI_DEV_CSI0_SIE0)
	ctl_sen_test_param[sen_id].output_dest = ctl_sen_output_sie_0;
#elif (CTL_SEN_CONFIG_DEV == CONFI_DEV_CSI1_SIE2)
	ctl_sen_test_param[sen_id].output_dest = ctl_sen_output_sie_2;
#else
#endif


#if CTL_SEN_KDRV_TOP_READY
	/*
	    sendrv param
	*/
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.pinmux.func = PIN_FUNC_SENSORMISC;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.pinmux.cfg = PIN_SENSORMISC_CFG_SN_MCLK_1;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.pinmux.pnext = &ctl_sen_test_param[sen_id].init_cfg_obj_pinmux[0];

	ctl_sen_test_param[sen_id].init_cfg_obj_pinmux[0].func = PIN_FUNC_MIPI_LVDS;
	ctl_sen_test_param[sen_id].init_cfg_obj_pinmux[0].cfg = PIN_MIPI_LVDS_CFG_HSI2HSI3_TO_CSI;
	ctl_sen_test_param[sen_id].init_cfg_obj_pinmux[0].pnext = &ctl_sen_test_param[sen_id].init_cfg_obj_pinmux[1];

	ctl_sen_test_param[sen_id].init_cfg_obj_pinmux[1].func = PIN_FUNC_I2C;
	ctl_sen_test_param[sen_id].init_cfg_obj_pinmux[1].cfg = PIN_I2C_CFG_I2C_1;
	ctl_sen_test_param[sen_id].init_cfg_obj_pinmux[1].pnext = NULL;

	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.clk_lane_sel = CTL_SEN_CLANE_SEL_CSI0_USE_C0;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.sen_2_serial_pin_map[0] = 0;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.sen_2_serial_pin_map[1] = 1;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.sen_2_serial_pin_map[2] = 2;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.sen_2_serial_pin_map[3] = 3;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.sen_2_serial_pin_map[4] = CTL_SEN_IGNORE;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.sen_2_serial_pin_map[5] = CTL_SEN_IGNORE;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.sen_2_serial_pin_map[6] = CTL_SEN_IGNORE;
	ctl_sen_test_param[sen_id].init_cfg_obj.pin_cfg.sen_2_serial_pin_map[7] = CTL_SEN_IGNORE;

#else
	ctl_sen_dbg_err("top drv not ready, pls set pinmux hardcode or dtsi\r\n");
#endif

	ctl_sen_test_param[sen_id].init_cfg_obj.if_cfg.type = CTL_SEN_IF_TYPE_MIPI;
	ctl_sen_test_param[sen_id].init_cfg_obj.drvdev = CTL_SEN_DRVDEV_CSI_0;
#if (CTL_SEN_CONFIG_DEV == CONFI_DEV_CSI0_SIE0)
	ctl_sen_test_param[sen_id].init_cfg_obj.drvdev = CTL_SEN_DRVDEV_CSI_0;
#elif (CTL_SEN_CONFIG_DEV == CONFI_DEV_CSI1_SIE2)
	ctl_sen_test_param[sen_id].init_cfg_obj.drvdev = CTL_SEN_DRVDEV_CSI_1;
#endif
}

static void _ctl_sen_test_param_uninit(UINT32 sen_id)
{
	ctl_sen_test_param[sen_id].init = FALSE;
}

int _ctl_sen_api_on(UINT32 sen_id, UINT32 chip_id, CHAR *name)
{
	INT32 rt = E_OK;
	CTL_SEN_INIT_MAP init_map = {0};
	CTL_SEN_MAP map = {0};

	if (ctl_sen_g_init_cnt == 0) {
		rt = ctl_sen_init_buf(0, 0);
		if (rt) {
			ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
		}
	}
	ctl_sen_g_init_cnt++;

	init_map.chip_id = chip_id;
	init_map.chip_name = name;
	init_map.chip_cfg = &ctl_sen_test_param[sen_id].init_cfg_obj;
	rt = ctl_sen_init_map(sen_id, &init_map);
	if (rt) {
		ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
	}

	map.sen_mode = ctl_sen_test_param[sen_id].sen_mode;
	map.frm_idx = ctl_sen_test_param[sen_id].frm_idx;

	rt = ctl_sen_add_map(sen_id, &map);
	if (rt) {
		ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
	}
	ctl_sen_add_map_cnt++;

	rt = ctl_sen_open(sen_id);
	if (rt) {
		ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
	}

	rt = ctl_sen_pwr_ctrl(sen_id, CTL_SEN_PWR_CTRL_TURN_ON);
	if (rt) {
		ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
	}

	return rt;
}


int _ctl_sen_api_trigger(UINT32 sen_id)
{
	INT32 rt = E_OK;
	CTL_SEN_CHGMODE_INFO info;

	info.fps = ctl_sen_test_param[sen_id].fps;
	info.output_dest = ctl_sen_test_param[sen_id].output_dest;
	rt = ctl_sen_chgmode(sen_id, &info);
	if (rt) {
		ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
	}

	// check linear output
	{
		UINT64 stamp[5];

		sen_id = 0;
		ctl_sen_wait_intruupt(sen_id, CTL_SEN_INTE_FMD_TO_SIE0 + ctl_sen_test_param[sen_id].output_dest);
		stamp[0] = hwclock_get_longcounter();
		ctl_sen_wait_intruupt(sen_id, CTL_SEN_INTE_FMD_TO_SIE0 + ctl_sen_test_param[sen_id].output_dest);
		stamp[1] = hwclock_get_longcounter();
		ctl_sen_wait_intruupt(sen_id, CTL_SEN_INTE_FMD_TO_SIE0 + ctl_sen_test_param[sen_id].output_dest);
		stamp[2] = hwclock_get_longcounter();
		ctl_sen_wait_intruupt(sen_id, CTL_SEN_INTE_FMD_TO_SIE0 + ctl_sen_test_param[sen_id].output_dest);
		stamp[3] = hwclock_get_longcounter();
		ctl_sen_wait_intruupt(sen_id, CTL_SEN_INTE_FMD_TO_SIE0 + ctl_sen_test_param[sen_id].output_dest);
		stamp[4] = hwclock_get_longcounter();

		ctl_sen_dbg_dump("[%d] %lld us,%lld us %lld us,%lld us\r\n", __LINE__, stamp[4] - stamp[3], stamp[3] - stamp[2], stamp[2] - stamp[1], stamp[1] - stamp[0]);
	}

	return 0;
}

int _ctl_sen_api_off(UINT32 sen_id)
{
	INT32 rt = E_OK;

	rt = ctl_sen_pwr_ctrl(sen_id, CTL_SEN_PWR_CTRL_TURN_OFF);
	if (rt) {
		ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
	}

	rt = ctl_sen_close(sen_id);
	if (rt) {
		ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
	}

	rt = ctl_sen_del_map(sen_id);
	if (rt) {
		ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
	}
	ctl_sen_add_map_cnt--;

	if (ctl_sen_add_map_cnt == 0) {
		rt = ctl_sen_uninit_map(sen_id);
		if (rt) {
			ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
		}
	}

	ctl_sen_g_init_cnt--;
	if (ctl_sen_g_init_cnt == 0) {
		rt = ctl_sen_uninit_buf();
		if (rt) {
			ctl_sen_dbg_dump("[%s][%d]rt=%d\r\n", __func__, __LINE__, rt);
		}
	}

	return 0;
}

BOOL nvt_ctl_sen_api_on(unsigned char argc, char **pargv)
{
	int rt = E_OK;
	UINT32 sen_id = 0, chip_id = 0;
	CHAR name[CTL_SEN_NAME_LEN] = "nvt_sen_imx290";

	if (argc > 0) {
		sscanf_s(pargv[0], "%d", (int *)&sen_id);
	}
	if (argc > 1) {
		sscanf_s(pargv[1], "%d", (int *)&chip_id);
	}
	if (argc > 2) {
		sscanf_s(pargv[2], "%s", (int *)&name);
	}

	ctl_sen_dbg_dump("%s : sen_id=%d, chip_id=%d, name=%s\r\n", __func__, sen_id, chip_id, name);

	rt = _ctl_sen_api_on(sen_id, chip_id, name);

	ctl_sen_dbg_dump("[CMD DONE]\r\n");


	return rt;
}

BOOL nvt_ctl_sen_api_off(unsigned char argc, char **pargv)
{
	int rt = E_OK;
	UINT32 sen_id = 0;

	if (argc > 0) {
		sscanf_s(pargv[0], "%d", (int *)&sen_id);
	}

	ctl_sen_dbg_dump("%s : sen_id=%d\r\n", __func__, sen_id);
	rt = _ctl_sen_api_off(sen_id);

	ctl_sen_dbg_dump("[CMD DONE]\r\n");


	return rt;
}

BOOL nvt_ctl_sen_api_init(unsigned char argc, char **pargv)
{
	int rt = E_OK;
	UINT32 sen_id = 0;

	if (argc > 0) {
		sscanf_s(pargv[0], "%d", (int *)&sen_id);
	}

	ctl_sen_dbg_dump("%s : sen_id=%d\r\n", __func__, sen_id);

	_ctl_sen_test_param_init(sen_id);

	ctl_sen_dbg_dump("[CMD DONE]\r\n");

	return rt;
}

BOOL nvt_ctl_sen_api_uninit(unsigned char argc, char **pargv)
{
	int rt = E_OK;
	UINT32 sen_id = 0;

	if (argc > 0) {
		sscanf_s(pargv[0], "%d", (int *)&sen_id);
	}

	ctl_sen_dbg_dump("%s : sen_id=%d\r\n", __func__, sen_id);

	_ctl_sen_test_param_uninit(sen_id);

	ctl_sen_dbg_dump("[CMD DONE]\r\n");

	return rt;
}


BOOL nvt_ctl_sen_api_trigger(unsigned char argc, char **pargv)
{
	int rt = E_OK;
	UINT32 sen_id = 0;

	if (argc > 0) {
		sscanf_s(pargv[0], "%d", (int *)&sen_id);
	}

	ctl_sen_dbg_dump("%s : sen_id=%d\r\n", __func__, sen_id);
	rt = _ctl_sen_api_trigger(sen_id);

	ctl_sen_dbg_dump("[CMD DONE]\r\n");


	return rt;
}

BOOL nvt_ctl_sen_api_cfg(unsigned char argc, char **pargv)
{
	UINT32 sen_id = 0;

	if (argc < 2) {
		ctl_sen_dbg_dump("argc < 2\r\n");
		return FALSE;
	}

	sscanf_s(pargv[0], "%d", (int *)&sen_id);

	if (strcmp("sen_mode", pargv[1]) == 0) {
		sscanf_s(pargv[2], "%d", (int *)&ctl_sen_test_param[sen_id].sen_mode);
		ctl_sen_dbg_dump("sen_id=%d, %s=%s\r\n", sen_id, pargv[1], pargv[2]);
	}

	if (strcmp("fps", pargv[1]) == 0) {
		sscanf_s(pargv[2], "%d", (int *)&ctl_sen_test_param[sen_id].fps);
		ctl_sen_dbg_dump("sen_id=%d, %s=%s\r\n", sen_id, pargv[1], pargv[2]);
	}

	if (strcmp("output_dest", pargv[1]) == 0) {
		sscanf_s(pargv[2], "%d", (int *)&ctl_sen_test_param[sen_id].output_dest);
		ctl_sen_dbg_dump("sen_id=%d, %s=%s\r\n", sen_id, pargv[1], pargv[2]);
	}


	ctl_sen_dbg_dump("[CMD DONE]\r\n");

	return 0;
}

#endif

/*
    PROC CMD ENTRY
*/
static SXCMD_BEGIN(ctl_sen, CTL_SEN_PROC_NAME)
SXCMD_ITEM("info",          nvt_ctl_sen_api_info,            "input : (CTL_SEN_ID)(CTL_SEN_DBG_SEL)(param)")
SXCMD_ITEM("signal",        nvt_ctl_sen_api_signal,          "input : (CTL_SEN_ID)(CTL_SEN_DBG_SEL)(param)")

SXCMD_ITEM("dbg",           nvt_ctl_sen_api_dbg,            "input : (CTL_SEN_ID)(CTL_SEN_DBG_SEL)(param)") // TODO // Backward compatible

SXCMD_ITEM("w_reg",         nvt_ctl_sen_api_w_reg,          "input : (CTL_SEN_ID)(addr)(data_len)(data0)(data1)")
SXCMD_ITEM("r_reg",         nvt_ctl_sen_api_r_reg,          "input : (CTL_SEN_ID)(addr)(data_len)")
SXCMD_ITEM("dbg_lv",        nvt_ctl_sen_api_set_dbg_lv,     "input : (CTL_SEN_DBG_LV)") // Backward compatible
SXCMD_ITEM("dbglevel",      nvt_ctl_sen_api_set_dbg_lv,     "input : (CTL_SEN_DBG_LV)")
#if CTL_SEN_TEST
SXCMD_ITEM("on",            nvt_ctl_sen_api_on,             "on")
SXCMD_ITEM("off",           nvt_ctl_sen_api_off,            "off")
SXCMD_ITEM("trigger",       nvt_ctl_sen_api_trigger,        "trigger")
SXCMD_ITEM("cfg",           nvt_ctl_sen_api_cfg,            "cfg")
SXCMD_ITEM("init",          nvt_ctl_sen_api_init,           "init")
SXCMD_ITEM("uninit",        nvt_ctl_sen_api_uninit,         "uninit")
#endif
SXCMD_END()

int ctl_sen_cmd_showhelp(void)
{
	UINT32 cmd_num = SXCMD_NUM(ctl_sen);
	UINT32 loop = 1;

	ctl_sen_dbg_dump("---------------------------------------------------------------------\r\n");
	ctl_sen_dbg_dump("  %s\n", CTL_SEN_PROC_NAME);
	ctl_sen_dbg_dump("---------------------------------------------------------------------\r\n");

	for (loop = 1 ; loop <= cmd_num ; loop++) {
		ctl_sen_dbg_dump("%15s : %s\r\n", ctl_sen[loop].p_name, ctl_sen[loop].p_desc);
	}
	return 0;
}

#if defined(__FREERTOS) || defined(__ECOS) || defined(__UITRON)
MAINFUNC_ENTRY(ctl_sen, argc, argv)
#else
int ctl_sen_cmd_execute(unsigned char argc, char **argv)
#endif
{
	UINT32 cmd_num = SXCMD_NUM(ctl_sen);
	UINT32 loop;
	int    ret;
	unsigned char ucargc = 0;

	//DBG_DUMP("%d, %s, %s, %s, %s\r\n", (int)argc, argv[0], argv[1], argv[2], argv[3]);

	if (strncmp(argv[1], "?", 2) == 0) {
		ctl_sen_cmd_showhelp();
		return -1;
	}

	if (argc < 1) {
		ctl_sen_dbg_err("input param error\r\n");
		return -1;
	}
	ucargc = argc - 2;
	for (loop = 1 ; loop <= cmd_num ; loop++) {
		if (strncmp(argv[1], ctl_sen[loop].p_name, strlen(argv[1])) == 0) {
			ret = ctl_sen[loop].p_func(ucargc, &argv[2]);
			return ret;
		}
	}

	if (loop > cmd_num) {
		ctl_sen_dbg_err("Invalid CMD !!\r\n");
		ctl_sen_cmd_showhelp();
		return -1;
	}
	return 0;
}

