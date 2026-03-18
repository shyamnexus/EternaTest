#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "vpet_api_int.h"
#include "vpe_msg.h"
#include "vpe_dbg.h"
#include "vpe_main.h"
#include "vpe_proc.h"

//=============================================================================
// avoid GPL api
//=============================================================================
#define MUTEX_ENABLE DISABLE
extern BOOL vpe_id_valid[VPE_ID_MAX_NUM][VPE_IDX_MAX_NUM];
extern UINT32 vpe_dcout_mode[VPE_ID_MAX_NUM];

//=============================================================================
// global
//=============================================================================
#if MUTEX_ENABLE
static struct semaphore mutex;
#endif
static struct proc_dir_entry *proc_root;
static struct proc_dir_entry *proc_info;
static struct proc_dir_entry *proc_dbglevel;
static struct proc_dir_entry *proc_command;

static struct proc_dir_entry *proc_help;

static VPE_PROC_MSG_BUF vpe_proc_cmd_msg;
static VPE_PROC_R_ITEM vpe_proc_r_item = VPE_PROC_R_ITEM_NONE;
static VPE_ID vpe_proc_id;
static UINT32 vpe_proc_idx;

//=============================================================================
// routines
//=============================================================================
static inline VPE_MODULE *vpe_proc_get_mudule_from_file(struct file *file)
{
	return (VPE_MODULE *)((struct seq_file *)file->private_data)->private;
}

//=============================================================================
// proc "info" file operation functions
//=============================================================================
static INT32 vpe_proc_info_show(struct seq_file *sfile, void *v)
{
	vpe_msg_show_info(sfile);

	return 0;
}

static INT32 vpe_proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, vpe_proc_info_show, PDE_DATA(inode));
}

static const struct proc_ops vpe_proc_info_ops = {
	.proc_open    = vpe_proc_info_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

//=============================================================================
// proc "cmd" file operation functions
//=============================================================================
static inline INT32 vpe_proc_msg_buf_alloc(void)
{
	vpe_proc_cmd_msg.buf = kzalloc(PROC_MSG_BUFSIZE, GFP_KERNEL);

	if (vpe_proc_cmd_msg.buf == NULL) {
		DBG_WRN("fail to allocate VPE message buffer \r\n");
		return -ENOMEM;
	}

	vpe_proc_cmd_msg.size = PROC_MSG_BUFSIZE;
	vpe_proc_cmd_msg.count = 0;
	return 0;
}

static inline void vpe_proc_msg_buf_free(void)
{
	if (vpe_proc_cmd_msg.buf) {
		kfree(vpe_proc_cmd_msg.buf);
		vpe_proc_cmd_msg.buf = NULL;
	}
}

static inline void vpe_proc_msg_buf_clean(void)
{
	vpe_proc_cmd_msg.buf[0] = '\0';
	vpe_proc_cmd_msg.count = 0;
}

static INT32 vpe_proc_cmd_printf(const s8 *f, ...)
{
	INT32 len;
	va_list args;

	if (vpe_proc_cmd_msg.count < vpe_proc_cmd_msg.size) {
		va_start(args, f);
		len = vsnprintf(vpe_proc_cmd_msg.buf + vpe_proc_cmd_msg.count, vpe_proc_cmd_msg.size - vpe_proc_cmd_msg.count, f, args);
		va_end(args);

		if ((vpe_proc_cmd_msg.count + len) < vpe_proc_cmd_msg.size) {
			vpe_proc_cmd_msg.count += len;
			return 0;
		}
	}

	vpe_proc_cmd_msg.count = vpe_proc_cmd_msg.size;

	return -1;
}

static INT32 vpe_proc_cmd_get_dce_ctl_param(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		vpe_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	vpe_proc_id = simple_strtoul(argv[2], NULL, 0);

	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	if ((vpe_id_valid[vpe_proc_id][vpe_proc_idx]) && (vpe_proc_id < VPE_ID_MAX_NUM)) {
		vpe_proc_r_item = VPE_PROC_R_ITEM_DCE_CTL;
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", vpe_proc_id, vpe_proc_idx);
		vpe_proc_r_item = VPE_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 vpe_proc_cmd_get_sharpen_param(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	vpe_proc_id = simple_strtoul(argv[2], NULL, 0);

	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	if ((vpe_id_valid[vpe_proc_id][vpe_proc_idx]) && (vpe_proc_id < VPE_ID_MAX_NUM)) {
		vpe_proc_r_item = VPE_PROC_R_ITEM_SHARPEN;
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", vpe_proc_id, vpe_proc_idx);
		vpe_proc_r_item = VPE_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 vpe_proc_cmd_get_2dlut_param(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	vpe_proc_id = simple_strtoul(argv[2], NULL, 0);

	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	if ((vpe_id_valid[vpe_proc_id][vpe_proc_idx]) && (vpe_proc_id < VPE_ID_MAX_NUM)) {
		vpe_proc_r_item = VPE_PROC_R_ITEM_2DLUT;
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", vpe_proc_id, vpe_proc_idx);
		vpe_proc_r_item = VPE_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 vpe_proc_cmd_get_2dlut_expand_param(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	vpe_proc_id = simple_strtoul(argv[2], NULL, 0);

	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	if ((vpe_id_valid[vpe_proc_id][vpe_proc_idx]) && (vpe_proc_id < VPE_ID_MAX_NUM)) {
		vpe_proc_r_item = VPE_PROC_R_ITEM_2DLUT_EXPAND;
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", vpe_proc_id, vpe_proc_idx);
		vpe_proc_r_item = VPE_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 vpe_proc_cmd_get_drt_param(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	vpe_proc_id = simple_strtoul(argv[2], NULL, 0);

	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	if ((vpe_id_valid[vpe_proc_id][vpe_proc_idx]) && (vpe_proc_id < VPE_ID_MAX_NUM)) {
		vpe_proc_r_item = VPE_PROC_R_ITEM_DRT;
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", vpe_proc_id, vpe_proc_idx);
		vpe_proc_r_item = VPE_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 vpe_proc_cmd_get_dctg_param(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	vpe_proc_id = simple_strtoul(argv[2], NULL, 0);

	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	if ((vpe_id_valid[vpe_proc_id][vpe_proc_idx]) && (vpe_proc_id < VPE_ID_MAX_NUM)) {
		vpe_proc_r_item = VPE_PROC_R_ITEM_DCTG;
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", vpe_proc_id, vpe_proc_idx);
		vpe_proc_r_item = VPE_PROC_R_ITEM_NONE;
	}

	return 0;
}

static INT32 vpe_proc_cmd_get_flip_rot_param(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	if (argc < 3) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	vpe_proc_id = simple_strtoul(argv[2], NULL, 0);

	vpe_proc_idx = (vpe_proc_id >> 8) & 0xFF;
	vpe_proc_id = vpe_proc_id & 0xFF;

	if ((vpe_id_valid[vpe_proc_id][vpe_proc_idx]) && (vpe_proc_id < VPE_ID_MAX_NUM)) {
		vpe_proc_r_item = VPE_PROC_R_ITEM_FLIP_ROT;
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", vpe_proc_id, vpe_proc_idx);
		vpe_proc_r_item = VPE_PROC_R_ITEM_NONE;
	}

	return 0;
}

static VPE_PROC_CMD vpe_proc_cmd_r_list[] = {
	// keyword                   function name
	{ "get_dce_ctl_param",       vpe_proc_cmd_get_dce_ctl_param,      "get vpe dce ctl param, param1 is vpe_id(0~15)"},
	{ "get_sharpen_param",       vpe_proc_cmd_get_sharpen_param,      "get vpe sharpen param, param1 is vpe_id(0~15)"},
	{ "get_2dlut_param",         vpe_proc_cmd_get_2dlut_param,        "get vpe 2dlut param, param1 is vpe_id(0~15)"},
	{ "get_2dlut_param_expand",  vpe_proc_cmd_get_2dlut_expand_param, "get vpe 2dlut_expand param, param1 is vpe_id(0~15)"},
	{ "get_drt_param",           vpe_proc_cmd_get_drt_param,          "get vpe drt param, param1 is vpe_id(0~15)"},
	{ "get_dctg_param",          vpe_proc_cmd_get_dctg_param,         "get vpe dctg param, param1 is vpe_id(0~15)"},
	{ "get_flip_rot_param",      vpe_proc_cmd_get_flip_rot_param,     "get vpe flip_rot param, param1 is vpe_id(0~15)"},
};
#define NUM_OF_R_CMD (sizeof(vpe_proc_cmd_r_list) / sizeof(VPE_PROC_CMD))

static INT32 vpe_proc_cmd_set_dbg(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	u32 cmd;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	cmd = simple_strtoul(argv[3], NULL, 0);

	vpe_proc_cmd_printf("set vpe(%d) dbg level(0x%x) \n", id, cmd);
	vpe_dbg_set_dbg_mode(id, cmd);
	return 0;
}

static INT32 vpe_proc_cmd_set_ioc_control(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	UINT32 cmd;

	if (argc < 3) {
		vpe_proc_cmd_printf("wrong argument:%d", argc);
		return -EINVAL;
	}

	cmd = simple_strtoul(argv[2], NULL, 0);

	vpe_proc_cmd_printf("set vpe ioc control(0x%X) \n", (unsigned int)cmd);
	vpe_dbg_set_ioc_control(cmd);

	return 0;
}

static INT32 vpe_proc_cmd_ctrl_en(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT8 ctrl_en;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	ctrl_en = simple_strtoul(argv[3], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_param[id][idx]->dce_ctl->enable = ctrl_en;
		vpet_set_param_update(id, idx, VPET_ITEM_DCE_CTL_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

static INT32 vpe_proc_cmd_dce_mode(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT8 dce_mode;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	dce_mode = simple_strtoul(argv[3], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_param[id][idx]->dce_ctl->dce_mode = dce_mode;
		vpet_set_param_update(id, idx, VPET_ITEM_DCE_CTL_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

static INT32 vpe_proc_cmd_dcout(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT8 dcout;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	dcout = simple_strtoul(argv[3], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_dcout_mode[id] = dcout;
		vpet_set_param_update(id, idx, VPET_ITEM_DCOUT_MODE_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

#if 0
static INT32 vpe_proc_cmd_sharpen_en(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT8 sharpen_en;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	sharpen_en = simple_strtoul(argv[3], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_param[id][idx]->sharpen->enable = sharpen_en;
		vpet_set_param_update(id, VPET_ITEM_SHARPEN_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

static INT32 vpe_proc_cmd_edge_sharp_str(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT8 edge_sharp_str1;
	UINT8 edge_sharp_str2;

	if (argc < 5) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	edge_sharp_str1 = simple_strtoul(argv[3], NULL, 0);
	edge_sharp_str2 = simple_strtoul(argv[4], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_param[id][idx]->sharpen->edge_sharp_str1 = edge_sharp_str1;
		vpe_param[id][idx]->sharpen->edge_sharp_str2 = edge_sharp_str2;
		vpet_set_param_update(id, VPET_ITEM_SHARPEN_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

static INT32 vpe_proc_cmd_flat_sharp_str(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT8 flat_sharp_str;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	flat_sharp_str = simple_strtoul(argv[3], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_param[id][idx]->sharpen->flat_sharp_str = flat_sharp_str;
		vpet_set_param_update(id, VPET_ITEM_SHARPEN_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

static INT32 vpe_proc_cmd_edge_weight_gain(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT8 edge_weight_gain;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	edge_weight_gain = simple_strtoul(argv[3], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_param[id][idx]->sharpen->edge_weight_gain = edge_weight_gain;
		vpet_set_param_update(id, VPET_ITEM_SHARPEN_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

static INT32 vpe_proc_cmd_dbs_gain_en(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT8 dbs_gain_en;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	dbs_gain_en = simple_strtoul(argv[3], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_param[id][idx]->sharpen->dbs_gain_en = dbs_gain_en;
		vpet_set_param_update(id, VPET_ITEM_SHARPEN_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

static INT32 vpe_proc_cmd_quad_area_clamping(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx;
	UINT32 quad_area_clamping;

	if (argc < 4) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);
	quad_area_clamping = simple_strtoul(argv[3], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		vpe_param[id][idx]->sharpen->quad_area_clamping = quad_area_clamping;
		vpet_set_param_update(id, VPET_ITEM_SHARPEN_PARAM, TRUE);
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

static INT32 vpe_proc_cmd_dbs_gain(VPE_MODULE *pdrv, INT32 argc, CHAR **argv)
{
	VPE_ID id;
	UINT32 idx, i;
	UINT8 dbs_gain[VPE_SHARPEN_DBS_GAIN_NUM];

	if (argc < 20) {
		vpe_proc_cmd_printf("wrong argument: %d", argc);
		return -EINVAL;
	}

	id = simple_strtoul(argv[2], NULL, 0);

	idx = (id >> 8) & 0xFF;
	id = id & 0xFF;

	for (i = 0; i < VPE_SHARPEN_DBS_GAIN_NUM; i++) {
		dbs_gain[i] = simple_strtoul(argv[3+i], NULL, 0);
	}

	if ((vpe_id_valid[id][idx]) && (id < VPE_ID_MAX_NUM)) {
		for(i = 0; i < VPE_SHARPEN_DBS_GAIN_NUM; i++){
			vpe_param[id][idx]->sharpen->dbs_gain[i] = dbs_gain[i];
			vpet_set_param_update(id, VPET_ITEM_SHARPEN_PARAM, TRUE);
		}
	} else {
		DBG_DUMP("id list not valid (%d) (%d) \r\n", id, idx);
		return 0;
	}

	return 0;
}

#endif

static VPE_PROC_CMD vpe_proc_cmd_w_list[] = {
	// keyword                    function name
	{ "dbg",                     vpe_proc_cmd_set_dbg,                "set vpe dbg level, param1 is vpe_id(0~15), param2 is dbg_lv"},
	{ "ioc",                     vpe_proc_cmd_set_ioc_control,        "set vpe ioc control, param1 is ioc_control."},
	{ "ctrl_en",                 vpe_proc_cmd_ctrl_en,                "set ctrl enable, param1 is vpe_id(0~15), param2 is ctrl enable (0: disable; 1: enable)"},
	{ "ctrl_mode",               vpe_proc_cmd_dce_mode,               "set dce mode, param1 is vpe_id(0~15), param2 is dce mode (1: 2dlut; 2: dctg)"},
	{ "dcout",                   vpe_proc_cmd_dcout,                  "set dce mode, param1 is vpe_id(0~15), param2 is dcout mode (0: auto; 1: output)"},
	#if 0
	{ "sharpen_en",              vpe_proc_cmd_sharpen_en,             "set sharpen_en, param1 is vpe_id(0~7), param2 is sharpen_en"},
	{ "edge_sharp_str",          vpe_proc_cmd_edge_sharp_str,         "set edge sharp str, param1 is vpe_id(0~7), param2 is edge sharp str1 value, param3 is edge sharp str2 value"},
	{ "flat_sharp_str",          vpe_proc_cmd_flat_sharp_str,         "set flat_sharp_str, param1 is vpe_id(0~7), param2 is flat_sharp_str value"},
	{ "edge_weight_gain",        vpe_proc_cmd_edge_weight_gain,       "set edge_weight_gain, param1 is vpe_id(0~7), param2 is edge_weight_gain"},
	{ "dbs_gain_en",             vpe_proc_cmd_dbs_gain_en,            "set dbs_gain_en, param1 is vpe_id(0~7), param2 is dbs_gain_en"},
	{ "quad_area_clamping",      vpe_proc_cmd_quad_area_clamping,     "set quad_area_clamping, param1 is vpe_id(0~7), param2 is quad_area_clamping"},
	{ "dbs_gain",                vpe_proc_cmd_dbs_gain,               "set dbs_gain, param1 is vpe_id(0~7), param2-19 is dbs_gain"},
	#endif
};
#define NUM_OF_W_CMD (sizeof(vpe_proc_cmd_w_list) / sizeof(VPE_PROC_CMD))

static INT32 vpe_proc_command_show(struct seq_file *sfile, void *v)
{
	if ((vpe_proc_cmd_msg.buf == NULL) && (vpe_proc_r_item == VPE_PROC_R_ITEM_NONE)) {
		return -EINVAL;
	}

	if (vpe_proc_cmd_msg.buf > 0) {
		seq_printf(sfile, "%s\n", vpe_proc_cmd_msg.buf);
		vpe_proc_msg_buf_clean();
	}

	if (vpe_proc_r_item == VPE_PROC_R_ITEM_DCE_CTL) {
		vpe_msg_show_dce_ctrl(sfile, vpe_proc_id, vpe_proc_idx);
	}

	if (vpe_proc_r_item == VPE_PROC_R_ITEM_SHARPEN) {
		vpe_msg_show_sharpen(sfile, vpe_proc_id, vpe_proc_idx);
	}

	if (vpe_proc_r_item == VPE_PROC_R_ITEM_2DLUT) {
		vpe_msg_show_2dlut(sfile, vpe_proc_id, vpe_proc_idx);
	}

	if (vpe_proc_r_item == VPE_PROC_R_ITEM_2DLUT_EXPAND) {
		vpe_msg_show_2dlut_expand(sfile, vpe_proc_id, vpe_proc_idx);
	}

	if (vpe_proc_r_item == VPE_PROC_R_ITEM_DRT) {
		vpe_msg_show_drt(sfile, vpe_proc_id, vpe_proc_idx);
	}

	if (vpe_proc_r_item == VPE_PROC_R_ITEM_DCTG) {
		vpe_msg_show_dctg(sfile, vpe_proc_id, vpe_proc_idx);
	}

	if (vpe_proc_r_item == VPE_PROC_R_ITEM_FLIP_ROT) {
		vpe_msg_show_flip_rot(sfile, vpe_proc_id, vpe_proc_idx);
	}

	return 0;
}

static INT32 vpe_proc_command_open(struct inode *inode, struct file *file)
{
	//return single_open(file, vpe_proc_command_show, PDE_DATA(inode));
	return single_open_size(file, vpe_proc_command_show, PDE_DATA(inode), 5000 * sizeof(u32));
}

static ssize_t vpe_proc_command_write(struct file *file, const CHAR __user *buf, size_t size, loff_t *off)
{
	INT32 len = size;
	INT32 ret = -EINVAL;

	CHAR cmd_line[MAX_CMDLINE_LENGTH];
	CHAR *cmdstr = cmd_line;
	const CHAR delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	CHAR *argv[MAX_CMD_ARGUMENTS] = {NULL};
	INT32 argc = 0;
	UINT32 i;
	VPE_MODULE *vpe_module = vpe_proc_get_mudule_from_file(file);

	vpe_proc_msg_buf_clean();

	// check command length
	if ((len <= 1) || (len > MAX_CMDLINE_LENGTH)) {
		DBG_ERR("command is too short or long \r\n");
	} else {
		// copy command string from user space
		if (copy_from_user(cmd_line, buf, len)) {
			;
		} else {
			cmd_line[len-1] = '\0';

			// parse command string
			for (i = 0; i < MAX_CMD_ARGUMENTS; i++) {
				argv[i] = strsep(&cmdstr, delimiters);
				if (argv[i] != NULL) {
					argc++;
				} else {
					break;
				}
			}

			// dispatch command handler
			ret = -EINVAL;

			if (strncmp(argv[0], "r", 2) == 0) {
				for (i = 0; i < NUM_OF_R_CMD; i++) {
					if (strncmp(argv[1], vpe_proc_cmd_r_list[i].cmd, MAX_CMD_LENGTH) == 0) {
						#if MUTEX_ENABLE
						down(&mutex);
						#endif
						ret = vpe_proc_cmd_r_list[i].execute(vpe_module, argc, argv);
						#if MUTEX_ENABLE
						up(&mutex);
						#endif
						break;
					}
				}

				if (i >= NUM_OF_R_CMD) {
					DBG_ERR("[VOE_ERR]: => ");
					for (i = 0; i < argc; i++) {
						DBG_ERR("%s ", argv[i]);
					}
					DBG_ERR("is not in r_cmd_list \r\n");
				}
			} else if (strncmp(argv[0], "w", 2) == 0) {
				for (i = 0; i < NUM_OF_W_CMD; i++) {
					if (strncmp(argv[1], vpe_proc_cmd_w_list[i].cmd, MAX_CMD_LENGTH) == 0) {
						#if MUTEX_ENABLE
						down(&mutex);
						#endif
						ret = vpe_proc_cmd_w_list[i].execute(vpe_module, argc, argv);
						#if MUTEX_ENABLE
						up(&mutex);
						#endif
						break;
					}
				}

				if (i >= NUM_OF_W_CMD) {
					DBG_ERR("[VPE_ERR]: =>");
					for (i = 0; i < argc; i++) {
						DBG_ERR("%s ", argv[i]);
					}
					DBG_ERR("is not in w_cmd_list \r\n");
				}
			} else {
				DBG_ERR("[VPE_ERR]: =>");
				for (i = 0; i < argc; i++) {
					DBG_ERR("%s ", argv[i]);
				}
				DBG_ERR("is not legal command \r\n");
			}
		}
	}

	if (ret < 0) {
		DBG_ERR("[VPE_ERR]: fail to execute: ");
		for (i = 0; i < argc; i++) {
			DBG_ERR("%s ", argv[i]);
		}
		DBG_ERR("\r\n");
	}

	return size;
}

static const struct proc_ops vpe_proc_command_ops = {
	.proc_open    = vpe_proc_command_open,
	.proc_read    = seq_read,
	.proc_write   = vpe_proc_command_write,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
unsigned int vpe_debug_level = 3;

static INT32 vpe_proc_dbglevel_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "0: fatal; 1: err; 2: wrn; 3: msg; 4: ind; 5: func \n");
	seq_printf(sfile, "vpe_debug_level = %d \n", vpe_debug_level);

	return 0;
}

static INT32 vpe_proc_dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, vpe_proc_dbglevel_show, PDE_DATA(inode));
}

static ssize_t vpe_proc_dbglevel_write(struct file *file, const CHAR __user *buf, size_t size, loff_t *off)
{
	INT32 len = size;
	INT32 ret = -EINVAL;
	CHAR cmd_line[MAX_CMDLINE_LENGTH];
	CHAR *cmdstr = cmd_line;
	const CHAR delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	CHAR *argv[MAX_CMD_ARGUMENTS] = {NULL};
	INT32 argc = 0;
	UINT32 i;

	vpe_proc_msg_buf_clean();

	// check command length
	if ((len <= 1) || (len > MAX_CMDLINE_LENGTH)) {
		DBG_ERR("command is too short or long \n");
	} else {
		// copy command string from user space
		if (copy_from_user(cmd_line, buf, len)) {
			;
		} else {
			cmd_line[len-1] = '\0';

			// parse command string
			for (i = 0; i < MAX_CMD_ARGUMENTS; i++) {
				argv[i] = strsep(&cmdstr, delimiters);
				if (argv[i] != NULL) {
					argc++;
				} else {
					break;
				}
			}

			// dispatch command handler
			ret = -EINVAL;

			if (argc != 1) {
				vpe_proc_cmd_printf("wrong argument:%d", argc);
				return -EINVAL;
			}

			vpe_debug_level = simple_strtoul(argv[0], NULL, 0);
			vpe_proc_cmd_printf("set dbglevel %d \n", vpe_debug_level);
		}
	}

	return size;
}

static const struct proc_ops vpe_proc_dbglevel_ops = {
	.proc_open    = vpe_proc_dbglevel_open,
	.proc_read    = seq_read,
	.proc_write   = vpe_proc_dbglevel_write,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static INT32 vpe_proc_help_show(struct seq_file *sfile, void *v)
{
	UINT32 loop;

	seq_printf(sfile, "1. 'cat /proc/hdal/vendor/vpe/info' will show all the vpe info\r\n");
	seq_printf(sfile, "2. 'echo r/w xxx > /proc/hdal/vendor/vpe/cmd' can input command for some debug purpose\r\n");
	seq_printf(sfile, "The currently support input command are below:\r\n");

	seq_printf(sfile, "---------------------------------------------------------------------\r\n");
	seq_printf(sfile, "  %s\n", "vpe");
	seq_printf(sfile, "---------------------------------------------------------------------\r\n");

	for (loop = 0 ; loop < NUM_OF_R_CMD ; loop++) {
		seq_printf(sfile, "r %23s : %s\r\n", vpe_proc_cmd_r_list[loop].cmd, vpe_proc_cmd_r_list[loop].text);
	}

	for (loop = 0 ; loop < NUM_OF_W_CMD ; loop++) {
		seq_printf(sfile, "w %23s : %s\r\n", vpe_proc_cmd_w_list[loop].cmd, vpe_proc_cmd_w_list[loop].text);
	}

	seq_printf(sfile, "\r\n");
	seq_printf(sfile, "-- Example of get/set parameter ------------------------------------------------\r\n");
	seq_printf(sfile, "echo r get_dce_ctl_param 0 > /proc/hdal/vendor/vpe/cmd; cat /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo r get_sharpen_param 0 > /proc/hdal/vendor/vpe/cmd; cat /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo r get_2dlut_param 0 > /proc/hdal/vendor/vpe/cmd; cat /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo r get_2dlut_param_expand 0 > /proc/hdal/vendor/vpe/cmd; cat /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo r get_drt_param 0 > /proc/hdal/vendor/vpe/cmd; cat /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo r get_dctg_param 0 > /proc/hdal/vendor/vpe/cmd; cat /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo r get_flip_rot_param 0 > /proc/hdal/vendor/vpe/cmd; cat /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo w ctrl_en 0 0 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo w dcout 0 0 > /proc/hdal/vendor/vpe/cmd \r\n");
	#if 0
	seq_printf(sfile, "echo w sharpen_en 0 1 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo w edge_sharp_str 0 50 60 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo w flat_sharp_str 0 30 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "echo w edge_weight_gain 0 190 > /proc/hdal/vendor/vpe/cmd \r\n");
	#endif
	seq_printf(sfile, "\r\n");
	seq_printf(sfile, "--Example of enable vpe cb message ---------------------------------------------\r\n");
	seq_printf(sfile, "echo w dbg 16 0x1 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "\r\n");
	seq_printf(sfile, "-- Example of enable vpe update message ----------------------------------------\r\n");
	seq_printf(sfile, "echo w dbg 16 0x4 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "\r\n");
	seq_printf(sfile, "-- Example of enable warning message -------------------------------------------\r\n");
	seq_printf(sfile, "echo w dbg 16 0x20000000 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "\r\n");
	seq_printf(sfile, "-- Example of disable message --------------------------------------------------\r\n");
	seq_printf(sfile, "echo w dbg 16 0x0 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "\r\n");
	seq_printf(sfile, "-- Example of enable ioc bypass ------------------------------------------------\r\n");
	seq_printf(sfile, "echo w ioc 0x1 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "\r\n");
	seq_printf(sfile, "-- Example of enable ioc print -------------------------------------------------\r\n");
	seq_printf(sfile, "echo w ioc 0x10000 > /proc/hdal/vendor/vpe/cmd \r\n");
	seq_printf(sfile, "\r\n");
	seq_printf(sfile, "-- Example of disable ioc control ----------------------------------------------\r\n");
	seq_printf(sfile, "echo w ioc 0x0 > /proc/hdal/vendor/vpe/cmd \r\n");

	return 0;
}

static INT32 vpe_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, vpe_proc_help_show, PDE_DATA(inode));
}

static const struct proc_ops vpe_proc_help_ops = {
	.proc_open    = vpe_proc_help_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

//=============================================================================
// extern functions
//=============================================================================
INT32 vpe_proc_init(VPE_DRV_INFO *pdrv_info)
{
	INT32 ret = 0;
	struct proc_dir_entry *root = NULL, *pentry = NULL;

	// initialize synchronization mechanism
	#if MUTEX_ENABLE
	sema_init(&mutex, 1);
	#endif

	root = proc_mkdir("hdal/vendor/vpe", NULL);

	if (root == NULL) {
		DBG_ERR("fail to create VPE proc root \r\n");
		vpe_proc_remove(pdrv_info);
		return -EINVAL;
	}
	proc_root = root;

	// create "info" entry
	pentry = proc_create_data("info", S_IRUGO | S_IXUGO, root, &vpe_proc_info_ops, (void *)pdrv_info);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc info \r\n");
		vpe_proc_remove(pdrv_info);
		return -EINVAL;
	}
	proc_info = pentry;

	// create "command" entry
	pentry = proc_create_data("cmd", S_IRUGO | S_IXUGO, root, &vpe_proc_command_ops, (void *)pdrv_info);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc cmd \r\n");
		vpe_proc_remove(pdrv_info);
		return -EINVAL;
	}
	proc_command = pentry;

	// create "dbglevel" entry
	pentry = proc_create_data("dbglevel", S_IRUGO | S_IXUGO, root, &vpe_proc_dbglevel_ops, (void *)pdrv_info);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc dbglevel \n");
		vpe_proc_remove(pdrv_info);
		return -EINVAL;
	}
	proc_dbglevel = pentry;

	// create "help" entry
	pentry = proc_create_data("help", S_IRUGO | S_IXUGO, root, &vpe_proc_help_ops, (void *)pdrv_info);
	if (pentry == NULL) {
		DBG_ERR("fail to create proc help \r\n");
		vpe_proc_remove(pdrv_info);
		return -EINVAL;
	}
	proc_help = pentry;

	// allocate memory for massage buffer
	ret = vpe_proc_msg_buf_alloc();
	if (ret < 0) {
		vpe_proc_remove(pdrv_info);
	}

	return ret;
}

void vpe_proc_remove(VPE_DRV_INFO *pdrv_info)
{
	if (proc_root == NULL) {
		return;
	}

	// remove "info"
	if (proc_info) {
		proc_remove(proc_info);
	}
	// remove "cmd"
	if (proc_info) {
		proc_remove(proc_command);
	}
	// remove "dbglevel"
	if (proc_dbglevel) {
		proc_remove(proc_dbglevel);
	}
	// remove "help"
	if (proc_help) {
		proc_remove(proc_help);
	}

	// remove root entry
	proc_remove(proc_root);

	// free message buffer
	vpe_proc_msg_buf_free();
}

