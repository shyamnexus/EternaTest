#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "kdrv_vdocdc_dbg.h"
#include "kdrv_vdocdc_proc.h"
#include "kdrv_vdocdc_api.h"
#include "kdrv_vdocdc_main.h"
#include "kdrv_vdocdc_comn.h"

#include "h26x_def.h"
#include "h26x.h"
#include "../kdrv_builtin/nvt_jpg/include/jpeg_platform.h"
#include "../kdrv_builtin/nvt_jpg/include/jpeg_api.h"
#if H26X_MEM_USAGE
#include "h26x_common.h"
#include "kdrv_videoenc/kdrv_videoenc.h"
#endif
// proc entries
static struct proc_dir_entry *proc_root_entry = NULL;
//#if (H26X_SAVE_KO_SIZE == 0)
#if H26X_PROC_CMD
static struct proc_dir_entry *proc_cmd_entry = NULL;
static struct proc_dir_entry *proc_version_entry = NULL;
#endif
#if LPM_PROC_ENABLE
static struct proc_dir_entry *proc_lpm_entry = NULL;
#endif
#if H26X_MEM_USAGE
static struct proc_dir_entry *proc_mem_usage_entry = NULL;
#endif
static struct proc_dir_entry *proc_aq_entry = NULL;

#if H26X_SET_PROC_PARAM
static struct proc_dir_entry *proc_param_entry = NULL;
#endif
static struct proc_dir_entry *proc_venc_dbglevel_entry = NULL;
static struct proc_dir_entry *proc_chn_info_entry = NULL;
static struct proc_dir_entry *proc_util_entry = NULL;

#ifdef JPGCDC_SIM
static struct proc_dir_entry *proc_jpg_sim_entry = NULL;
#endif
static struct proc_dir_entry *proc_jpg_dbglevel_entry = NULL;
static struct proc_dir_entry *proc_jpg_util_entry = NULL;
static struct proc_dir_entry *proc_jpg_dbg_entry = NULL;

//#if (H26X_SAVE_KO_SIZE == 0)
#if (H26X_PROC_CMD)
//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     8

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================

//============================================================================
// Function define
//============================================================================


//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static PROC_CMD cmd_read_list[] = {
	// keyword          function name
};

#define NUM_OF_READ_CMD (sizeof(cmd_read_list) / sizeof(PROC_CMD))

static PROC_CMD cmd_write_list[] = {
	// keyword          function name
	{ "sim",		kdrv_vdocdc_api_wt_sim		},
	{ "dbg",		kdrv_vdocdc_api_wt_dbg		},
	{ "int",		kdrv_vdocdc_api_wt_int		},
};

#define NUM_OF_WRITE_CMD (sizeof(cmd_write_list) / sizeof(PROC_CMD))

static int proc_cmd_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
    seq_printf(sfile, " kdrv_vdocdc_proc_cmd : \n");
    seq_printf(sfile, "=====================================================================\n");

	return 0;
}

static int proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_cmd_show, NULL);
}

static ssize_t proc_cmd_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	const char delim[4] = {' ', 0x0A, 0x0D, '\0'};
	char cmd[MAX_CMD_LENGTH];
	char *p_cmd = cmd;
	unsigned char loop;
	unsigned char argc;
	char *argv[MAX_ARG_NUM] = {0};
	int ret = -EINVAL;

	if (copy_from_user(cmd, buf, size) != 0) {
		DBG_ERR("h26x proc comd copy error\r\n");
        return -1;
    }

	if (size != 0)
		cmd[size - 1] = '\0';

	nvt_dbg(INFO, "cmd_wt : %s\n", cmd);

	for (argc = 0; argc < 8; argc++) {
		argv[argc] = strsep(&p_cmd, delim);

		if (argv[argc] == NULL)
			break;
	}

	if (strncmp(argv[0], "r", 2) == 0) {
        for (loop = 0 ; loop < NUM_OF_READ_CMD; loop++) {
            if (strncmp(argv[1], cmd_read_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
                ret = cmd_read_list[loop].execute(argc - 2, &argv[2]);
                break;
            }
        }
        if (loop >= NUM_OF_READ_CMD)
			return -1;

    } else if (strncmp(argv[0], "w", 2) == 0)  {
        for (loop = 0 ; loop < NUM_OF_WRITE_CMD ; loop++) {
            if (strncmp(argv[1], cmd_write_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
                ret = cmd_write_list[loop].execute(argc - 2, &argv[2]);
                break;
            }
        }

        if (loop >= NUM_OF_WRITE_CMD)
			return -1;

    } else
		return -1;

	return size;
}

static struct proc_ops proc_cmd_fops = {
    .proc_open   = proc_cmd_open,
    .proc_release = single_release,
    .proc_read   = seq_read,
    .proc_write  = proc_cmd_write,
    .proc_lseek = seq_lseek,
};

static int proc_util_show(struct seq_file *sfile, void *v)
{
	UINT32 util_rate, fps;

	h26x_get_utilization(0, &util_rate, &fps);

	seq_printf(sfile, "======== VENC ======== \r\n");
	seq_printf(sfile, "chip eng usage    fps\r\n");
	//seq_printf(sfile, "  0     0   %02d.%02d  %02d.%02d\r\n", util_rate/100, util_rate%100, fps/100, fps%100);
	seq_printf(sfile, "%4d %3d %5d %6d\r\n", 0, 0, (util_rate/100) + ((util_rate%100) > 50), fps);

	return 0;
}

static int proc_util_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_util_show, NULL);
}

static struct proc_ops proc_util_fops = {
    .proc_open = proc_util_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};


#endif //H26X_SAVE_KO_SIZE

#if H26X_SET_PROC_PARAM
//=============================================================================
// proc "rc" file operation functions
//=============================================================================
#include "h264enc_api.h"
#include "h265enc_api.h"
#include "h26xenc_api.h"

#define PROC_MAX_BUFFER 256
#define MAX_PARAM_NUM   20

typedef struct h26x_param_mapping_st
{
    char *tokenName;
    int (*get_param)(void);
    int (*set_param)(int);
    int lb;
    int ub;
    char *note;
} H26XParamMapInfo;

/*
static int h26xEnc_setVersion(int val)
{
    return 0;
}
*/
static const H26XParamMapInfo h26x_enc_syntax[] = {
	//#if (H26X_SAVE_KO_SIZE == 0)
    {"H264RowRCStopFactor",     &h264Enc_getRowRCStopFactor,    &h264Enc_setRowRCStopFactor,    0,  0xFFFF, "H264 Row RC stop factor"},
    {"H265RowRCStopFactor",     &h265Enc_getRowRCStopFactor,    &h265Enc_setRowRCStopFactor,    0,  0xFFFF, "H265 Row RC stop factor"},
    {"H264PReduce16Planar",     &h264Enc_getPReduce16Planar,    &h264Enc_setPReduce16Planar,    0,  1,      "H264 P frame reduce intra16 planar"},
    //{"H264FrameNumGapAllow",    &gH264FrameNumGapAllow, STX_INT32,  1,      0,  1,      1,  "H264 frame_num gap allow"},
    //{"H264FixLog2Poc",          &gFixSPSLog2Poc,        STX_INT32,  0,      0,  16,     1,  "H264 fix log2 poc lsb"},

	{"RRCNDQPStep",				&h26xEnc_getNDQPStep,			&h26xEnc_setNDQPStep,			0,	15,		"Row rc negative qp step"},
	{"RRCNDQPRange",			&h26xEnc_getNDQPRange,			&h26xEnc_setNDQPRange,			0,	15,		"Row rc negative qp range"},
	{"H264FroSync",				&h264Enc_getFroSync,			&h264Enc_setFroSync,			0,	2,		"H264 fro sync"},
	{"H265FroSync",				&h265Enc_getFroSync,			&h265Enc_setFroSync,			0,	1,		"H265 fro sync"},
	#if H264_SET_COEF_COST_THD
	{"H264YCoefCostTh",			&h264Enc_getYCoefCostTh,		&h264Enc_setYCoefCostTh,		0,	15,		"H264 Y coef cost th"},
	{"H264CCoefCostTh",			&h264Enc_getCCoefCostTh,		&h264Enc_setCCoefCostTh,		0,	15,		"H264 C coef cost th"},
	#endif
	//{"EncIMethod",				&h264Enc_getH264NonTileI,		&h264Enc_setH264NonTileI,		0,	1,		"I encode method"},
	//{"MaxBSRatio",				&h26xEnc_getMaxBSRatio,			&h26xEnc_setMaxBSRatio,			100,	0xFFFF, "maximal bs ratio"},
	//#endif
//	#if TILE_FRAME_LEVEL_BUGFIX
//	{"H264TBias",				&h26xEnc_get264TBias,			&h26xEnc_set264TBias,			0,	1,		"H264 TBias"},
//	#endif
	{"RRCSyncRCQPCond",			&h26xEnc_getRRCSyncQPCond,		&h26xEnc_setRRCSyncQPCond,		0,	1,		"Sync RRC max/min qp"},
	#if H26X_DIS_OSG_CACHE_OPERATION
	{"OsgCacheFlush",			&h26xEnc_getOsgCacheFlush,		&h26xEnc_setOsgCacheFlush,		0,	1,		"Osg cahce flush"},
	#endif
	{"ESMVTh",					&h26xEnc_getESMVTh,				&h26xEnc_setESMVTh,				0,	63,		"early skip mv threshold"},
	{"MDMode",					&h26xEnc_getMDMode,				&h26xEnc_setMDMode,				0,	1,		"0: CODEC_MD, 1: EXTERNAL_MD"},
	{"CustQPPriority",			&h26xEnc_getCustQPPriority,		&h26xEnc_setCustQPPriority,		0,	1,		"customized qp priority"},
	{"RCDumpLog",               &h26xEnc_getRCDumpLog,          &h26xEnc_setRCDumpLog,          0,  1,      "RC dump log"},
	#if H264_AEFIFO_RRC_BIAS
	{"H264TBias",				&h26xEnc_get264TBias,			&h26xEnc_set264TBias,			0,	1,		"H264 TBias"},
	#endif
	#if H26X_SET_MB_LIMIT_QP_IND
	{"IMinDeltaMBQP",			&h26xEnc_getIMinDeltaMBQP,		&h26xEnc_setIMinDeltaMBQP,		-26,0,		"Min delta MB QP of I frame"},
	{"IMaxDeltaMBQP",			&h26xEnc_getIMaxDeltaMBQP,		&h26xEnc_setIMaxDeltaMBQP,		0,	25,		"Max delta MB QP of I frame"},
	{"PMinDeltaMBQP",			&h26xEnc_getPMinDeltaMBQP,		&h26xEnc_setPMinDeltaMBQP,		-26,0,		"Min delta MB QP of P frame"},
	{"PMaxDeltaMBQP",			&h26xEnc_getPMaxDeltaMBQP,		&h26xEnc_setPMaxDeltaMBQP,		0,	25,		"Max delta MB QP of P frame"},
	#endif
    //{"Version",                 &h26xEnc_getVersion,            &h26xEnc_setVersion,            0,  1,      "version"},
    {NULL,                      NULL,                           NULL,                           0,  0,      NULL}
};

static int h26xenc_show_enc_param_syntax(struct seq_file *sfile)
{
    int idx = 0;
    seq_printf(sfile, "      parameter        value                   note\r\n");
    seq_printf(sfile, "=====================  =====  ======================================\r\n");
    for (idx = 0; idx < (int)(sizeof(h26x_enc_syntax)/sizeof(H26XParamMapInfo)); idx++) {
        if (NULL == h26x_enc_syntax[idx].tokenName)
            break;
        seq_printf(sfile, "%-22s  ", h26x_enc_syntax[idx].tokenName);

        seq_printf(sfile, "%3d   ", h26x_enc_syntax[idx].get_param());
        seq_printf(sfile, "%s (range: %d ~ %d)\r\n", h26x_enc_syntax[idx].note, h26x_enc_syntax[idx].lb, h26x_enc_syntax[idx].ub);
    }
    return 0;
}

int h26xenc_set_enc_param_syntax(char *str)
{
    int value;
    char cmd_str[0x80];
    int i, idx = -1;

    sscanf(str, "%s %d\n", cmd_str, &value);

    for (i = 0; i < (int)(sizeof(h26x_enc_syntax)/sizeof(H26XParamMapInfo)); i++) {
        if (NULL == h26x_enc_syntax[i].tokenName)
            break;
        if (strcmp(h26x_enc_syntax[i].tokenName, cmd_str) == 0) {
            idx = i;
            break;
        }
    }
    if (idx >= 0) {
        if (value < h26x_enc_syntax[idx].lb || value > h26x_enc_syntax[idx].ub) {
            DBG_ERR("%s(%d) is out of range! (%d ~ %d)\n", h26x_enc_syntax[idx].tokenName, value, h26x_enc_syntax[idx].lb, h26x_enc_syntax[idx].ub);
        }
        else {
            h26x_enc_syntax[idx].set_param(value);
        }
    }
    else {
        DBG_ERR("unknown \"%s\"\n", cmd_str);
    }
    return 0;
}


static int proc_param_show(struct seq_file *sfile, void *v)
{
    seq_printf(sfile, "usage:\n");
    h26xenc_show_enc_param_syntax(sfile);
    return 0;
}

static int proc_param_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_param_show, NULL);
}

static ssize_t proc_param_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    char proc_buffer[PROC_MAX_BUFFER];

    if (count > sizeof(proc_buffer) - 1) {
        DBG_ERR("input is too large\n");
        return -EINVAL;
    }
    if (copy_from_user(proc_buffer, buffer, count)) {
        DBG_ERR("proc dbg copy from user failed\n");
        return -EFAULT;
    }
    proc_buffer[count] = '\0';

    h26xenc_set_enc_param_syntax(proc_buffer);

    return count;
}

static struct proc_ops proc_param_fops = {
    .proc_open = proc_param_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_param_write
};
#endif

#if LPM_PROC_ENABLE
static int proc_lpm_show(struct seq_file *sfile, void *v)
{
	H26XEncLpmCfg lpm_param;
	UINT8 inter_only;
	int ch;

	seq_printf(sfile, "lpm vdoenc_max_ch:%d\r\n", vdoenc_max_channel);

	for (ch = 0; ch < vdoenc_max_channel; ch++) {
		if (g_enc_info[ch].b_enable) {
			h26XEnc_getLpmCfg(&g_enc_info[ch].enc_var, &lpm_param);
			inter_only = h26xEnc_getInterOnlyCfg(&g_enc_info[ch].enc_var);
			//seq_printf(sfile, "usage: echo [en] [str] [level] [th] > /proc/kdrv_vdocdc/lpm\n");
			seq_printf(sfile, "lpm (ch:%d, en:%d, RmdSadEn:%d, ChrmDmEn:%d, IME:(%d, %d), Rdo:(%d, %d), (%d, %d, %d, %d, %d, %d)),interonly:%d, codec_md_ira_stop_en:%d ==\n",
								g_enc_info[ch].enc_var.uiEncId, lpm_param.bEnable, lpm_param.ucRmdSadEn, lpm_param.ucChrmDmEn,
								lpm_param.ucIMEStopEn, lpm_param.ucIMEStopTh,
								lpm_param.ucRdoStopEn, lpm_param.ucRdoStopTh,
								lpm_param.ucChrmDmEn, lpm_param.ucQPMapDefulat,lpm_param.ucI16On, lpm_param.ucIraEn,lpm_param.ucIraTh, lpm_param.ucILFLpm, inter_only, lpm_param.ucCodecMDLpmIraStopEn);
		}
	}
	return 0;

}

static int proc_lpm_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_lpm_show, NULL);
}

static ssize_t proc_lpm_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    char proc_buffer[PROC_MAX_BUFFER];

	int ch_id, en, rmdsad_en, chrmdm_en, ime_stop_en, ime_stop_th, rdo_stop_en, rdo_stop_th;
	int chrm_dm_en, qpmap_default, i16_on, ira_en, ira_th, ilf_lpm;
	int inter_only, codec_md_ira_stop_en;

    H26XEncLpmCfg stLpmCfg = {0};

    if (count > sizeof(proc_buffer) - 1) {
        DBG_ERR("input is too large\n");
        return -EINVAL;
    }
    if (copy_from_user(proc_buffer, buffer, count)) {
        DBG_ERR("proc dbg copy from user failed\n");
        return -EFAULT;
    }
    proc_buffer[count] = '\0';

    sscanf(proc_buffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &ch_id,	&en, &rmdsad_en, &chrmdm_en,
															&ime_stop_en, &ime_stop_th,
															&rdo_stop_en, &rdo_stop_th,
															&chrm_dm_en, &qpmap_default, &i16_on, &ira_en, &ira_th, &ilf_lpm,
															&inter_only, &codec_md_ira_stop_en);

	stLpmCfg.bEnable = en;
	stLpmCfg.ucRmdSadEn = rmdsad_en;
	stLpmCfg.ucChrmDmEn = chrmdm_en;
	stLpmCfg.ucIMEStopEn = ime_stop_en;
	stLpmCfg.ucIMEStopTh = ime_stop_th;
	stLpmCfg.ucRdoStopEn = rdo_stop_en;
	stLpmCfg.ucRdoStopTh = rdo_stop_th;

	stLpmCfg.ucChrmDmEn = chrm_dm_en;
	stLpmCfg.ucQPMapDefulat = qpmap_default;
	stLpmCfg.ucI16On = i16_on;
	stLpmCfg.ucIraEn = ira_en;
	stLpmCfg.ucIraTh = ira_th;
	stLpmCfg.ucILFLpm = ilf_lpm;

    stLpmCfg.ucCodecMDLpmIraStopEn = codec_md_ira_stop_en;

    // LPM by codec md will be mutually exclusive with INTER_ONLY and IRA_STOP_EN. on NT98539A //

	if (g_enc_info == NULL) {
		DBG_ERR("enc is not ready\r\n");
	} else if (ch_id >= vdoenc_max_channel) {
		DBG_ERR("ch_id:%d is over max channel number:%d\r\n", ch_id, vdoenc_max_channel);
	} else {
		h26XEnc_setLpmCfg(&g_enc_info[ch_id].enc_var, &stLpmCfg);
		h26xEnc_setInterOnlyCfg(&g_enc_info[0].enc_var, inter_only);
	}

	return count;

}

static struct proc_ops proc_lpm_fops = {
    .proc_open = proc_lpm_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_lpm_write
};
#endif

//#if (H26X_SAVE_KO_SIZE == 0)
#if H26X_PROC_CMD
static int proc_version_show(struct seq_file *sfile, void *v)
{
    int nvt_drv_ver = h26xEnc_getVersion();
    seq_printf(sfile, "h26x kdrv version: %s, nvt codec version %d.%d.%d.%d\n", H26XE_VER_STR,
        (nvt_drv_ver>>24)&0xFF, (nvt_drv_ver>>16)&0xFF, (nvt_drv_ver>>8)&0xFF, nvt_drv_ver&0xFF);
    return 0;
}

static int proc_version_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_version_show, NULL);
}

static ssize_t proc_version_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    return count;
}

static struct proc_ops proc_version_fops = {
    .proc_open = proc_version_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_version_write
};
#endif

#if H26X_MEM_USAGE
static int proc_mem_usage_show(struct seq_file *sfile, void *v)
{
	H26XMemUsage *p_mem_usage;
	UINT32 i=0, cdc_type, enc_id;

	for (cdc_type = 0; cdc_type < 2; cdc_type++) {
		for (enc_id = 0; enc_id < KDRV_VDOENC_ID_MAX; enc_id++) {
			p_mem_usage = (H26XMemUsage *)h26xEnc_getMemUsage(cdc_type, enc_id);
			if (p_mem_usage->cxt_size) {
				if (cdc_type == 0)
					seq_printf(sfile, "codec_type: h265, chn:%d, enc_buf:%d\r\n", enc_id, p_mem_usage->cxt_size);
				else if (cdc_type == 1)
					seq_printf(sfile, "codec_type: h264, chn:%d, enc_buf:%d\r\n", enc_id, p_mem_usage->cxt_size);
				seq_printf(sfile, "addr_start	addr_end	size	name\r\n");

				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"vdo_ctx");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"func_ctx");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"comn_ctx");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"frm_st0");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"colmv");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"extra_st0");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"si_st0");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"frm_st1");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"colmv_st1");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"extra_st1");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"si_st1");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"frm_lt");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"colmv_lt");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"extra_lt");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"si_lt");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"apb");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"ll");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"bsdma");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"nalu_len");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"rc_ref");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"seq_hdr");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"pic_hdr");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"usrqp");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"md");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"hist");
				i++;
				seq_printf(sfile, "x%08x	x%08x	x%08x	%s\r\n",
						p_mem_usage->buf_cmd[i].st_adr,
						p_mem_usage->buf_cmd[i].st_adr+p_mem_usage->buf_cmd[i].size,
						p_mem_usage->buf_cmd[i].size,
						"gcac");
				i++;
			}
		}
	}
	return 0;
}

static int proc_mem_usage_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_mem_usage_show, NULL);
}

static ssize_t proc_mem_usage_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    return count;
}

static struct proc_ops proc_mem_usage_fops = {
    .proc_open = proc_mem_usage_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_mem_usage_write
};
#endif

static int proc_venc_dbglevel_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "VENC dbg level = %d\n", (int)kdrv_venc_get_dbg_level());
    return 0;
}

static int proc_venc_dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_venc_dbglevel_show, NULL);
}

static ssize_t proc_venc_dbglevel_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int level;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		DBG_ERR("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		DBG_ERR("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d", &level);

	kdrv_venc_set_dbg_level(level);

    return count;
}

static struct proc_ops proc_venc_dbglevel_fops = {
    .proc_open = proc_venc_dbglevel_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_venc_dbglevel_write
};

extern int kdrv_vdocdc_get_aq_param(UINT32 *mode, UINT32 *i_str1, UINT32 *p_str1, UINT32 *i_str2, UINT32 *p_str2);
extern int kdrv_vdocdc_set_aq_param(UINT32 mode, UINT32 i_str1, UINT32 p_str1, UINT32 i_str2, UINT32 p_str2);

static int proc_aq_show(struct seq_file *sfile, void *v)
{
	UINT32 mode, i_str1, p_str1, i_str2, p_str2;
	kdrv_vdocdc_get_aq_param(&mode, &i_str1, &p_str1, &i_str2, &p_str2);
	seq_printf(sfile, "usage: echo [mode] [i str1] [p str1] [i str2] [p str2] > /proc/kdrv_vdocdc/aq\n");
	seq_printf(sfile, "== current set: aq mode = %d, str1 = %d/%d, str2 = %d/%d ==\n", (int)mode, (int)i_str1, (int)p_str1, (int)i_str2, (int)p_str2);
    return 0;
}

static int proc_aq_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_aq_show, NULL);
}

static ssize_t proc_aq_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int mode, i_str1, p_str1, i_str2, p_str2;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		DBG_ERR("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		DBG_ERR("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d %d %d %d %d", &mode, &i_str1, &p_str1, &i_str2, &p_str2);

	kdrv_vdocdc_set_aq_param((UINT32)mode, (UINT32)i_str1, (UINT32)p_str1, (UINT32)i_str2, (UINT32)p_str2);

    return count;
}

static struct proc_ops proc_aq_fops = {
    .proc_open = proc_aq_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_aq_write
};

static int proc_chn_info_show(struct seq_file *sfile, void *v)
{
#if 1
	UINT32 chn;
	H26XEncInfoCfg encInfo;

	seq_printf(sfile, "chn  codec  resolution  gop   rc    reference ([num] [addr0]/[addr1]/[addr2] (size))\n");
	seq_printf(sfile, "===  =====  ==========  ====  ==  ===================================================\n");

	for (chn = 0; chn < vdoenc_max_channel; chn++) {
		if (g_enc_info[chn].b_enable) {
			// get chn info
			h26xEnc_getEncInfo(&g_enc_info[chn].enc_var, &encInfo);
			// dump chn
			seq_printf(sfile, "%3d   %3s  %4dx%4d   %3d   %2d  ", encInfo.uiEncId, (encInfo.eCodecType == VCODEC_H264 ? "H264" : "H265"),
				encInfo.uiWidth, encInfo.uiHeight, encInfo.uiGop, encInfo.uiRcMode);
			seq_printf(sfile, "[%d] 0x%09lx/0x%09lx/0x%09lx (%d)\n", encInfo.uiRefNum, (unsigned long)encInfo.uiRefAddr[0],
				(unsigned long)encInfo.uiRefAddr[1], (unsigned long)encInfo.uiRefAddr[2], encInfo.uiRefSize);
		}
	}
#endif
    return 0;
}

static int proc_chn_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_chn_info_show, NULL);
}

static ssize_t proc_chn_info_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    return count;
}

static struct proc_ops proc_chn_info_fops = {
    .proc_open = proc_chn_info_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_chn_info_write
};


#ifdef JPGCDC_SIM
extern int jpeg_enc_main(int pat_idx, char *out_name, int frame_num, int sce_en, int rot_type, int osg_mask_en, int restart, int rc_enable, int block_mode_en);
extern int jpeg_dec_main(int pat_idx, char *out_name, int frame_num, int block_mode_en);
static int proc_jpg_sim_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "================ encode test simulation ================\n");
	seq_printf(sfile, " encode test:       echo 0 0 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " osg test:          echo 0 1 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " osg test(mask):    echo 0 2 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " mask test:         echo 0 3 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " mask test(mosaic): echo 0 4 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " mask test(border): echo 0 5 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " gray test:         echo 0 6 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " gray test(src):    echo 0 7 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " rc test:           echo 0 8 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " rc test(block):    echo 0 9 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " rotation(90):      echo 0 10 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " rotation(270):     echo 0 11 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " rotation(180):     echo 0 12 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " encode(block):     echo 0 15 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " encode 1080p:      echo 0 16 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " enc 1080p(block):  echo 0 17 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " encode test all:   echo 0 99 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, "================ decode test simulation ================\n");
	seq_printf(sfile, " decode test:       echo 1 0 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " decode(block):     echo 1 1 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " decode 1080p:      echo 1 2 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " dec 1080p(block):  echo 1 3 > /proc/kdrv_vdocdc/jpg_sim\n");
	seq_printf(sfile, " decode test all:   echo 0 99 > /proc/kdrv_vdocdc/jpg_sim\n");
    return 0;
}

static int proc_jpg_sim_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_sim_show, NULL);
}

static int proc_jpeg_sim_main(int codec_type, int test_case)
{
	if (0 == codec_type) {
		switch (test_case) {
		/* normal */
		case 0:
			jpeg_enc_main(0, "test0", 30, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* osg */
		case 1:
			jpeg_enc_main(0, "test1", 10, 0/*sce_en*/, 0/*rot_type*/, 1/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 2:
			jpeg_enc_main(0, "test2", 10, 0/*sce_en*/, 0/*rot_type*/, 2/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* mask */
		case 3:
			jpeg_enc_main(0, "test3", 10, 0/*sce_en*/, 0/*rot_type*/, (0x1<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 4:
			jpeg_enc_main(0, "test4", 10, 0/*sce_en*/, 0/*rot_type*/, (0x2<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 5:
			jpeg_enc_main(0, "test5", 10, 0/*sce_en*/, 0/*rot_type*/, (0x3<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* gray */
		case 6:
			jpeg_enc_main(0, "test6", 10, 0/*sce_en*/, 0/*rot_type*/, 2|(0x1<<8)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 7:
			jpeg_enc_main(0, "test7", 10, 0/*sce_en*/, 0/*rot_type*/, 2|(0x2<<8)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* rc */
		case 8:
			jpeg_enc_main(1, "test8", 120, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 1/*rc*/, 0/*block_mode_en*/);
			break;
		case 9:
			jpeg_enc_main(1, "test9", 120, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 1/*rc*/, 1/*block_mode_en*/);
			break;
		/* rotation */
		case 10:
			jpeg_enc_main(0, "test10", 10, 0/*sce_en*/, 1/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 11:
			jpeg_enc_main(0, "test11", 10, 0/*sce_en*/, 2/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 12:
			jpeg_enc_main(0, "test12", 10, 0/*sce_en*/, 3/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* block mode */
		case 15:
			jpeg_enc_main(0, "test15", 10, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			break;
		case 16:
			jpeg_enc_main(2, "test16", 10, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			break;
		/* non-block mode */
		case 17:
			jpeg_enc_main(2, "test17", 10, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		default:
			break;
		}
	}
	else {
		switch (test_case) {
		/* decode */
		case 0:
			jpeg_dec_main(0, "test0", 10, 0);
			break;
		case 1:
			jpeg_dec_main(0, "test1", 10, 1);
			break;
		case 2:
			jpeg_dec_main(1, "test2", 10, 0);
			break;
		case 3:
			jpeg_dec_main(1, "test3", 10, 1);
			break;
		default:
			break;
		}
	}
	return 0;
}

static ssize_t proc_jpg_sim_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int codec_type, test_case;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		DBG_ERR("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		DBG_ERR("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d %d", &codec_type, &test_case);

	if (test_case == 99) {
		int i;
		for (i = 0; i < 20; i++)
			proc_jpeg_sim_main(codec_type, i);
	}
	else {
		proc_jpeg_sim_main(codec_type, test_case);
	}

    return count;
}

static struct proc_ops proc_jpg_sim_fops = {
    .proc_open = proc_jpg_sim_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_sim_write
};
#endif

///////// jpeg dbg level /////////
static int proc_jpg_dbglevel_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "JPEG dbg level = %d\n", (int)nvt_jpg_get_dbg_level());
    return 0;
}

static int proc_jpg_dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_dbglevel_show, NULL);
}

static ssize_t proc_jpg_dbglevel_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int level;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		DBG_ERR("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		DBG_ERR("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d", &level);

	nvt_jpg_set_dbg_level(level);

    return count;
}

static struct proc_ops proc_jpg_dbglevel_fops = {
    .proc_open = proc_jpg_dbglevel_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_dbglevel_write
};

///////// jpeg utilization /////////
static int proc_jpg_util_show(struct seq_file *sfile, void *v)
{
	//seq_printf(sfile, "JPEG utildbg level = %d\n", (int)nvt_jpg_get_dbg_level());
	unsigned int util;
	int engine;
	for (engine = 0; engine < JPEG_MAX_ENG; engine++) {
		util = nvt_jpeg_get_engine_util(0, engine);
		if (util > 0)
		seq_printf(sfile, "JPEG eng%d: HW Utilization Period=%d(sec) Utilization=%d\n", engine, nvt_jpg_get_util_record(), util);
	}
    return 0;
}

static int proc_jpg_util_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_util_show, NULL);
}

static ssize_t proc_jpg_util_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int util;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		DBG_ERR("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		DBG_ERR("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d", &util);

	nvt_jpg_set_util_record(util);

    return count;
}

static struct proc_ops proc_jpg_util_fops = {
    .proc_open = proc_jpg_util_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_util_write
};

///////// jpeg dbg /////////
typedef struct jpeg_param_mapping_st
{
    char *tokenName;
    int (*get_param)(void);
    int (*set_param)(int);
    int lb;
    int ub;
    char *note;
} JPEGParamMapInfo;

static const JPEGParamMapInfo jpeg_syntax[] = {
	{"BrcUpdateMode", 		&jpeg_get_quality_update_mode,	&jpeg_set_quality_update_mode,	0,	2,		"JPEG brc update mode"},
	{"FixQualityRatio",		&jpeg_get_quality_fix_ratio,	&jpeg_set_quality_fix_ratio,	1,	99,		"JPEG fix quality ratio"},
	{"MaxQualityStep",		&jpeg_get_max_quality_step,		&jpeg_set_max_quality_step,		1,	100,	"JPEG max quality step"},
    {NULL,                      NULL,                           NULL,                           0,  0,      NULL}
};

static int jpeg_show_param_syntax(struct seq_file *sfile)
{
    int idx = 0;
    seq_printf(sfile, "      parameter        value                   note\r\n");
    seq_printf(sfile, "=====================  =====  ======================================\r\n");
    for (idx = 0; idx < (int)(sizeof(jpeg_syntax)/sizeof(H26XParamMapInfo)); idx++) {
        if (NULL == jpeg_syntax[idx].tokenName)
            break;
        seq_printf(sfile, "%-22s  ", jpeg_syntax[idx].tokenName);

        seq_printf(sfile, "%3d   ", jpeg_syntax[idx].get_param());
        seq_printf(sfile, "%s (range: %d ~ %d)\r\n", jpeg_syntax[idx].note, jpeg_syntax[idx].lb, jpeg_syntax[idx].ub);
    }
    return 0;
}

static int jpeg_set_param_syntax(char *str)
{
    int value;
    char cmd_str[0x80];
    int i, idx = -1;

    sscanf(str, "%s %d\n", cmd_str, &value);

    for (i = 0; i < (int)(sizeof(jpeg_syntax)/sizeof(H26XParamMapInfo)); i++) {
        if (NULL == jpeg_syntax[i].tokenName)
            break;
        if (strcmp(jpeg_syntax[i].tokenName, cmd_str) == 0) {
            idx = i;
            break;
        }
    }
    if (idx >= 0) {
        if (value < jpeg_syntax[idx].lb || value > jpeg_syntax[idx].ub) {
            DBG_ERR("%s(%d) is out of range! (%d ~ %d)\n", jpeg_syntax[idx].tokenName, value, jpeg_syntax[idx].lb, jpeg_syntax[idx].ub);
        }
        else {
            jpeg_syntax[idx].set_param(value);
        }
    }
    else {
        DBG_ERR("unknown \"%s\"\n", cmd_str);
    }
    return 0;
}

static int proc_jpg_dbg_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "usage:\n");
	jpeg_show_param_syntax(sfile);
    return 0;
}

static int proc_jpg_dbg_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_dbg_show, NULL);
}

static ssize_t proc_jpg_dbg_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	char proc_buffer[PROC_MAX_BUFFER];

    if (count > sizeof(proc_buffer) - 1) {
        DBG_ERR("input is too large\n");
        return -EINVAL;
    }
    if (copy_from_user(proc_buffer, buffer, count)) {
        DBG_ERR("proc dbg copy from user failed\n");
        return -EFAULT;
    }
    proc_buffer[count] = '\0';

    jpeg_set_param_syntax(proc_buffer);

    return count;
}

static struct proc_ops proc_jpg_dbg_fops = {
    .proc_open = proc_jpg_dbg_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_dbg_write
};


int kdrv_vdocdc_proc_init(void)
{
	int ret = 0;

	if ((proc_root_entry = proc_mkdir("kdrv_vdocdc", NULL)) == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
        ret = -EINVAL;
		goto FAIL_PROC;
	}

//#if (H26X_SAVE_KO_SIZE == 0)
#if H26X_PROC_CMD
	if ((proc_cmd_entry = proc_create("cmd", S_IRUGO | S_IXUGO, proc_root_entry, &proc_cmd_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
    }
#endif

#if LPM_PROC_ENABLE
	if ((proc_lpm_entry = proc_create("lpm", S_IRUGO | S_IXUGO, proc_root_entry, &proc_lpm_fops)) == NULL) {
        nvt_dbg(ERR, "failed to create proc lpm!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
    }
#endif

//#if (H26X_SAVE_KO_SIZE == 0)
#if H26X_PROC_CMD
    if ((proc_version_entry = proc_create("version", S_IRUGO | S_IXUGO, proc_root_entry, &proc_version_fops)) == NULL) {
        nvt_dbg(ERR, "failed to create proc version!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
    }
#endif
#if H26X_MEM_USAGE
    if ((proc_mem_usage_entry = proc_create("mem_usage", S_IRUGO | S_IXUGO, proc_root_entry, &proc_mem_usage_fops)) == NULL) {
        nvt_dbg(ERR, "failed to create proc mem_usage!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
    }
#endif
#if H26X_SET_PROC_PARAM
    if ((proc_param_entry = proc_create("param", S_IRUGO | S_IXUGO, proc_root_entry, &proc_param_fops)) == NULL) {
        nvt_dbg(ERR, "failed to create proc dbg!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
    }
#endif

	if ((proc_venc_dbglevel_entry = proc_create("venc_dbglevel", S_IRUGO | S_IXUGO, proc_root_entry, &proc_venc_dbglevel_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc venc_dbglevel!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
	}

	if ((proc_aq_entry = proc_create("aq", S_IRUGO | S_IXUGO, proc_root_entry, &proc_aq_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc aq!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
	}

	if ((proc_chn_info_entry = proc_create("chn_info", S_IRUGO | S_IXUGO, proc_root_entry, &proc_chn_info_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc chn_info!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
	}
	if ((proc_util_entry = proc_create("utilization", S_IRUGO | S_IXUGO, proc_root_entry, &proc_util_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc utilization!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
	}
#ifdef JPGCDC_SIM
	if ((proc_jpg_sim_entry = proc_create("jpg_sim", S_IRUGO | S_IXUGO, proc_root_entry, &proc_jpg_sim_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc jpg_sim!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
	}
#endif

	if ((proc_jpg_dbglevel_entry = proc_create("jpg_level", S_IRUGO | S_IXUGO, proc_root_entry, &proc_jpg_dbglevel_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc jpg_dbg!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
	}

	if ((proc_jpg_util_entry = proc_create("jpg_util", S_IRUGO | S_IXUGO, proc_root_entry, &proc_jpg_util_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc jpg_util!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
	}

	if ((proc_jpg_dbg_entry = proc_create("jpg_dbg", S_IRUGO | S_IXUGO, proc_root_entry, &proc_jpg_dbg_fops)) == NULL) {
		nvt_dbg(ERR, "failed to create proc jpg_util!\n");
		ret = -EINVAL;
		goto FAIL_PROC;
	}

	return ret;

FAIL_PROC:
//#if (H26X_SAVE_KO_SIZE == 0)
	if (proc_jpg_dbg_entry)
		proc_remove(proc_jpg_dbg_entry);
	if (proc_jpg_util_entry)
		proc_remove(proc_jpg_util_entry);
	if (proc_jpg_dbglevel_entry)
		proc_remove(proc_jpg_dbglevel_entry);
#ifdef JPGCDC_SIM
	if (proc_jpg_sim_entry)
		proc_remove(proc_jpg_sim_entry);
#endif
	if (proc_chn_info_entry)
		proc_remove(proc_chn_info_entry);
	if (proc_aq_entry)
		proc_remove(proc_aq_entry);
#if H26X_PROC_CMD
    if (proc_cmd_entry)
    	proc_remove(proc_cmd_entry);
    if (proc_version_entry)
        proc_remove(proc_version_entry);
#endif
#if H26X_MEM_USAGE
    if (proc_param_entry)
	    proc_remove(proc_mem_usage_entry);
#endif
#if LPM_PROC_ENABLE
	if (proc_lpm_entry)
		proc_remove(proc_lpm_entry);
#endif
#if H26X_SET_PROC_PARAM
    if (proc_param_entry)
	    proc_remove(proc_param_entry);
#endif
    if (proc_root_entry) {
    	proc_remove(proc_root_entry);
    }
	if (proc_venc_dbglevel_entry)
		proc_remove(proc_venc_dbglevel_entry);

	return ret;
}

void kdrv_vdocdc_proc_remove(void)
{
//#if (H26X_SAVE_KO_SIZE == 0)
	if (proc_jpg_dbg_entry)
		proc_remove(proc_jpg_dbg_entry);
	if (proc_jpg_util_entry)
		proc_remove(proc_jpg_util_entry);
	if (proc_jpg_dbglevel_entry)
		proc_remove(proc_jpg_dbglevel_entry);
#ifdef JPGCDC_SIM
	if (proc_jpg_sim_entry)
		proc_remove(proc_jpg_sim_entry);
#endif
	if (proc_util_entry)
		proc_remove(proc_util_entry);
	if (proc_chn_info_entry)
		proc_remove(proc_chn_info_entry);
	if (proc_aq_entry)
		proc_remove(proc_aq_entry);
#if H26X_PROC_CMD
    if (proc_cmd_entry)
    	proc_remove(proc_cmd_entry);
    if (proc_version_entry)
        proc_remove(proc_version_entry);
#endif
#if H26X_MEM_USAGE
    if (proc_mem_usage_entry)
        proc_remove(proc_mem_usage_entry);
#endif
#if LPM_PROC_ENABLE
	if (proc_lpm_entry)
		proc_remove(proc_lpm_entry);
#endif
#if H26X_SET_PROC_PARAM
    if (proc_param_entry)
        proc_remove(proc_param_entry);
#endif
	if (proc_venc_dbglevel_entry)
		proc_remove(proc_venc_dbglevel_entry);

    if (proc_root_entry)
    	proc_remove(proc_root_entry);
}
