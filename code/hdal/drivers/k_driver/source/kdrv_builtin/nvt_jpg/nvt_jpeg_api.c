#ifdef __KERNEL__

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <kwrap/file.h>
#include "jpeg_api.h"
#include "jpeg_drv.h"
#include "jpeg_dbg.h"
#include "jpeg_proc.h"
#include "jpeg_file.h"

extern unsigned int utilization_record_unit;
extern UINT32 g_QualityUpdateMode; // 0: fix step, 1: fix ratio, 2: ratio by quality
extern UINT32 g_QualityFixRatio;
extern UINT32 g_MaxQualityStep;
extern unsigned int g_HeaderChecksumEn;
extern unsigned int jpeg_sw_timeout_period;
extern unsigned int g_ChooseEngineUse;
extern unsigned int jpeg_enc_max_chn;
extern UINT32 g_BRCUpperBound;
extern UINT32 g_BRCLowerBound;
extern int g_VBRUpdatePeriod;
extern int g_CheckDmaIdle;
extern int g_DumpBRChn;

int nvt_jpg_api_write_reg(PJPG_MODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	unsigned long reg_addr, reg_value;

	printk("nvt_jpg_api_write_reg\r\n");

	if (argc != 2) {
		//DBG_ERR("wrong argument:%d", argc);
		return -EINVAL;
	}

	if (kstrtoul(pargv[0], 0, &reg_addr)) {
		//DBG_ERR("invalid reg addr:%s\n", pargv[0]);
		return -EINVAL;
	}

	if (kstrtoul(pargv[1], 0, &reg_value)) {
		//DBG_ERR("invalid rag value:%s\n", pargv[1]);
		return -EINVAL;

	}

	//DBG_INFO("W REG 0x%lx to 0x%lx\n", reg_value, reg_addr);

	//nvt_jpg_drv_write_reg(pmodule_info, reg_addr, reg_value);
	return 0;
}


int nvt_jpg_api_write_pattern(PJPG_MODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
//	mm_segment_t old_fs;
//	int fp;
	int len = 0;
//	unsigned char *pbuffer;
//	FST_FILE filehdl;
//	UINT32 fsize;
//	INT32 fstatus;
//	UINT8 *pbuf;

	printk("nvt_jpg_api_write_pattern\r\n");

	return len;
}

int nvt_jpg_api_read_reg(PJPG_MODULE_INFO pmodule_info, unsigned char argc, char **pargv)
{
	unsigned long reg_addr;
	//unsigned long value;

	printk("nvt_jpg_api_read_reg\r\n");

	if (argc != 1) {
		//DBG_ERR("wrong argument:%d", argc);
		return -EINVAL;
	}

	if (kstrtoul(pargv[0], 0, &reg_addr)) {
		//DBG_ERR("invalid reg addr:%s\n", pargv[0]);
		return -EINVAL;
	}

	//value = nvt_jpg_drv_read_reg(pmodule_info, reg_addr);

	//DBG_INFO("REG 0x%lx = 0x%lx\n", reg_addr, value);
	return 0;
}

unsigned int nvt_jpg_get_dbg_level(void)
{
	return jpeg_debug_level;
}

void nvt_jpg_set_dbg_level(unsigned int level)
{
	jpeg_debug_level = level;
}

unsigned int nvt_jpg_get_util_record(void)
{
	return utilization_record_unit;
}
void nvt_jpg_set_util_record(unsigned int util_record)
{
	if (util_record > 0)
		utilization_record_unit = util_record;
}

////////// global parameter setting //////////
int jpeg_get_quality_update_mode(void)
{
	return (int)g_QualityUpdateMode;
}
int jpeg_set_quality_update_mode(int mode)
{
	if (mode >= 0 && mode <= 2)
		g_QualityUpdateMode = mode;
	return 0;
}
int jpeg_get_quality_fix_ratio(void)
{
	return (int)g_QualityFixRatio;
}
int jpeg_set_quality_fix_ratio(int ratio)
{
	if (ratio > 0 && ratio < 100)
		g_QualityFixRatio = ratio;
	return 0;
}
int jpeg_get_max_quality_step(void)
{
	return (int)g_MaxQualityStep;
}
int jpeg_set_max_quality_step(int step)
{
	if (step > 0 && step < 100)
		g_MaxQualityStep = step;
	return 0;
}
int jpeg_get_header_checksum_en(void)
{
	return (int)g_HeaderChecksumEn;
}
int jpeg_set_header_checksum_en(int enable)
{
	if (enable)
		g_HeaderChecksumEn = 1;
	else
		g_HeaderChecksumEn = 0;

	return 0;
}
int jpeg_get_SWTimeoutPeriod(void)
{
	return (int)jpeg_sw_timeout_period;
}
int jpeg_set_SWTimeoutPeriod(int timeout_period)
{
	jpeg_sw_timeout_period = timeout_period;
	return 0;
}
int jpeg_get_choose_engine_use(void)
{
	return (int)g_ChooseEngineUse;
}
int jpeg_set_choose_engine_use(int engine)
{
	//0: use all, 1~2: single engine, other: out of range
	g_ChooseEngineUse = engine;
	return 0;
}
int jpeg_get_update_period(void)
{
	return g_VBRUpdatePeriod;
}
int jpeg_set_update_period(int period)
{
	g_VBRUpdatePeriod = period;
	return 0;
}
int jpeg_get_upper_bound(void)
{
	return (int)g_BRCUpperBound;
}
int jpeg_set_upper_bound(int bound)
{
	if (bound >= 100)
		g_BRCUpperBound = bound;
	return 0;
}
int jpeg_get_lower_bound(void)
{
	return (int)g_BRCLowerBound;
}
int jpeg_set_lower_bound(int bound)
{
	if (bound > 0 && bound <= 100)
		g_BRCLowerBound = bound;
	return 0;
}
int jpeg_get_check_dma_idle(void)
{
	return (int)g_CheckDmaIdle;
}
int jpeg_set_check_dma_idle(int check_en)
{
	g_CheckDmaIdle = check_en;
	return 0;
}
int jpeg_get_dump_br(void)
{
	return g_DumpBRChn;
}
int jpeg_set_dump_br(int chn)
{
	g_DumpBRChn = chn;
	return 0;
}

unsigned int jpeg_get_enc_max_chn(void)
{
	return jpeg_enc_max_chn;
}


EXPORT_SYMBOL(nvt_jpg_get_dbg_level);
EXPORT_SYMBOL(nvt_jpg_set_dbg_level);
EXPORT_SYMBOL(nvt_jpg_get_util_record);
EXPORT_SYMBOL(nvt_jpg_set_util_record);

EXPORT_SYMBOL(jpeg_get_quality_update_mode);
EXPORT_SYMBOL(jpeg_set_quality_update_mode);
EXPORT_SYMBOL(jpeg_get_quality_fix_ratio);
EXPORT_SYMBOL(jpeg_set_quality_fix_ratio);
EXPORT_SYMBOL(jpeg_get_max_quality_step);
EXPORT_SYMBOL(jpeg_set_max_quality_step);
EXPORT_SYMBOL(jpeg_get_header_checksum_en);
EXPORT_SYMBOL(jpeg_set_header_checksum_en);
EXPORT_SYMBOL(jpeg_get_SWTimeoutPeriod);
EXPORT_SYMBOL(jpeg_set_SWTimeoutPeriod);
EXPORT_SYMBOL(jpeg_get_choose_engine_use);
EXPORT_SYMBOL(jpeg_set_choose_engine_use);
EXPORT_SYMBOL(jpeg_get_update_period);
EXPORT_SYMBOL(jpeg_set_update_period);
EXPORT_SYMBOL(jpeg_get_upper_bound);
EXPORT_SYMBOL(jpeg_set_upper_bound);
EXPORT_SYMBOL(jpeg_get_lower_bound);
EXPORT_SYMBOL(jpeg_set_lower_bound);
EXPORT_SYMBOL(jpeg_get_check_dma_idle);
EXPORT_SYMBOL(jpeg_set_check_dma_idle);

EXPORT_SYMBOL(jpeg_get_dump_br);
EXPORT_SYMBOL(jpeg_set_dump_br);

EXPORT_SYMBOL(jpeg_get_enc_max_chn);
EXPORT_SYMBOL(jpeg_get_rc_info);

EXPORT_SYMBOL(nvt_jpeg_get_version);

#endif
