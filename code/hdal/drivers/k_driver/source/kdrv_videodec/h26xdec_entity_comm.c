#include "platform_port.h"

#include "h26xdec_flow.h"
#include "h26xdec_entity_comm.h"

static int engine_busy[ENTITY_CHIPS][ENTITY_ENGINES];
struct utilization_record_t eng_util[ENTITY_CHIPS][ENTITY_ENGINES];

/* version information */
extern char *plt_str;
extern char h26xd_ver_str[128];

extern struct buffer_info_t h265d_cabac_buffer[ENTITY_CHIPS][2];

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<< flow function >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/*
* Allocate memory for ptDecHandle
*/
void *H26XDec_Handler_Create(const H26XD_DEC_INIT_PARAM *ptParam, unsigned int chn_st_size, int chip, int ndev)
{
	DecoderParams *p_Dec;
	MALLOC_PTR_dec pfnMalloc = ptParam->pfnMalloc;
	FREE_PTR_dec   pfnFree = ptParam->pfnFree;

	if (pfnMalloc == NULL || pfnFree == NULL) {
		printk("null allocate/free function pointer\n");
		return NULL;
	}

	if ((p_Dec = (DecoderParams *)pfnMalloc(sizeof(DecoderParams))) == NULL) {
		printk("allocate dec handler fail\n");
		return NULL;
	}

	if ((p_Dec->pu32ChnStAddr = (uintptr_t *)pfnMalloc(chn_st_size)) == NULL) {
		printk("allocate dec chn struct base fail\n");
		return NULL;
	}

	p_Dec->u32ChnStSize = chn_st_size;

	/* copy function pointers */
	p_Dec->pfnMalloc = pfnMalloc;
	p_Dec->pfnFree = pfnFree;

	/* set chip idx */
	p_Dec->chip_idx = chip;

	/* set chn idx */
	p_Dec->chn_idx = ndev;

	/* copy module parameter */
	p_Dec->u16MaxWidth = ptParam->u16MaxWidth;
	p_Dec->u16MaxHeight = ptParam->u16MaxHeight;

	return p_Dec;
}

/*
* Release memory for ptDecHandle
*/
void H26XDec_Handler_Release(DecoderParams *p_Dec)
{
	FREE_PTR_dec pfnFree;

	if (p_Dec) {
		pfnFree = p_Dec->pfnFree;
		if (pfnFree == NULL) {
			printk("invalid free function pointer\n");
			return;
		}
		p_Dec->stVpsCtx_H265.priv = NULL;
		p_Dec->stVpuCtx_H265.priv = NULL;
		if (p_Dec->pu32ChnStAddr)
			pfnFree(p_Dec->pu32ChnStAddr);

		pfnFree(p_Dec);
	}
}

/*
* Bind DecHandle to a specified engine
*/
int H26XDec_BindEngine(DecoderEngInfo *eng_info, int real_chn_idx, int ll_buf_idx)
{
	if (eng_info->ll_buf_idx != -1) {
		printk("[DE] decoder_bind_engine for chn %d failed: chip %d engine %d ll_buf_idx error\n", real_chn_idx, eng_info->chip_idx, eng_info->engine_idx);
	}

	eng_info->ll_buf_idx = ll_buf_idx;

	/* enable clk for first time */
	pf_h26xd_xclk_on(eng_info->chip_idx);

	return EN_VPU_STATUS_SUCCESS;
}

/*
* Unbind engine for DecHandle
*/
int H26XDec_UnbindEngine(DecoderEngInfo *eng_info)
{
	eng_info->ll_buf_idx = -1;

	return EN_VPU_STATUS_SUCCESS;
}

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<< engine control >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
static uint64_t div_long(uint32_t high, uint32_t low, uint32_t den)
{
	unsigned int f1, f2, f3, f4, f5;
	unsigned int r1, r2, r3, r4;
	unsigned int temp;
	uint64_t result;
	if (0 == den) {
		result = (((uint64_t)high << 32) | low);
		return result;
	}
	if (den > 0xFFFFFF) {
		low = (low >> 8) | ((high & 0xFF) << 24);
		high = (high >> 8);
		den = (den >> 8);
	}
	f1 = high / den;
	r1 = high % den;
	temp = (r1 << 8) + ((low & 0xff000000) >> 24);
	f2 = temp / den ;
	r2 = temp % den ;
	temp = (r2 << 8) + ((low & 0xff0000) >> 16);
	f3 = temp / den ;
	r3 = temp % den ;
	temp = (r3 << 8) + ((low & 0xff00) >> 8);
	f4 = temp / den ;
	r4 = temp % den ;
	temp = (r4 << 8) + (low & 0xff);
	f5 = temp / den ;
	result = ((uint64_t)f1 << 24);
	result = (result << 8) + (((uint64_t)f2 << 24) + ((uint64_t)f3 << 16) + ((uint64_t)f4 << 8) + (uint64_t)f5);
	return result;
}

int64_t h26xdec_division(int64_t a_num, int b_den)
{
	unsigned int sign;
	unsigned int high, low;
	int64_t acc;

	if (0 == b_den)
		return a_num;
	sign = (a_num < 0) ^ (b_den < 0);
	if (a_num < 0)
		a_num = -a_num;
	if (b_den < 0)
		b_den = -b_den;
	high = (a_num >> 32) & 0xFFFFFFFF;
	low = a_num & 0xFFFFFFFF;

	if (0 == high)
		acc = low / b_den;
	else
		acc = div_long(high, low, b_den);
	if (sign)
		acc = -acc;
	return acc;
}

/*
* record engine starting time for performance profiling
*/
void mark_engine_start(int chip, int engine, unsigned int utilization_period)
{
	struct utilization_record_t *util = &eng_util[chip][engine];
	unsigned int curr_time = 0;

	if (utilization_period == 0) {
		return;
	}

	curr_time = H26XD_JIFFIES;

	if (util->engine_start != 0) {
		printk("{chip: %d, eng: %d) Warning to nested use dev mark_engine_start!\n", chip, engine);
	}

	util->engine_start = curr_time;
	util->engine_end = 0;
	if (0 == util->utilization_start) {
		util->utilization_start = util->engine_start;
		util->engine_time = 0;
	}
}

/*
* record engine ending time for performance profiling
*/
void mark_engine_finish(int chip, int engine, unsigned int utilization_period)
{
	struct utilization_record_t *util = &eng_util[chip][engine];
	unsigned int curr_time = 0;

	if (utilization_period == 0) {
		return;
	}

	curr_time = H26XD_JIFFIES;

	if (util->engine_end != 0) {
		printk("{chip: %d, eng: %d) Warning to nested use engine mark_engine_finish!\n", chip, engine);
	}

	/* accumulate engine running time */
	util->engine_end = curr_time;

	if (util->engine_end > util->engine_start)
		util->engine_time += util->engine_end - util->engine_start;

	/* caculate engine utilization within a utilization_period-seconds time */
	if (util->utilization_start > util->engine_end) {
		/* exception: engine aborted at the last time or jiffies overflow */
		util->utilization_start = 0;
		util->engine_time = 0;
	} else if ((util->utilization_start <= util->engine_end) &&
		   (util->engine_end - util->utilization_start >= utilization_period * H26XD_HZ)) {
		/* a utilization measurment period has passed, caculate the engine utilization in this period */
		unsigned int utilization;

		int64_t eng_time_64 = (int64_t)util->engine_time;
		utilization = (unsigned int)h26xdec_division((eng_time_64 * 10000), (util->engine_end - util->utilization_start));
		if (utilization)
			util->utilization_record = utilization;
		/* clear 'measurement start time' and 'engine running time' for the next measurement */
		util->utilization_start = 0;
		util->engine_time = 0;
	}
	/* clear engine start time to indicate a running time measurement is done (it is ok to do the next measurement) */
	util->engine_start = 0;
}

/*
 *  1:sucessful(engine not busy,set it busy)
 *  0:engine is busy
 *  This function must work in atomic context
 *  Return: 1 for engine busy now.
*/
int test_and_set_engine_busy(int chip, int engine)
{
	int ret = 0;

	if (engine_busy[chip][engine] == 0) {
		engine_busy[chip][engine] = 1;
		ret = 1;
	}

	return ret;
}

/*
 *  1:engine is idle
 *  0:engine is busy
 *  This function must work in atomic context
 */
int test_engine_idle(int chip, int engine)
{
	if (engine_busy[chip][engine] == 0) {
		return 1;
	}
	return 0;
}

/* This function must work in atomic context */
void set_engine_busy(int chip, int engine)
{
	engine_busy[chip][engine] = 1;
}

/* This function must work in atomic context */
void set_engine_idle(int chip, int engine)
{
	engine_busy[chip][engine] = 0;
}

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<< module of decode function >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/*
* Show dirver info message at module loading
* NOTE: The meesage can be quaried via config proc node
*/
void h26xd_msg(void)
{
	snprintf(h26xd_ver_str, sizeof(h26xd_ver_str),
		 "H26X Decoder IRQ mode PLT:%s, version %d.%d.%d.%d built @ %s %s", plt_str,
		 H26XD_VER_MAJOR, H26XD_VER_MINOR, H26XD_VER_MINOR2, H26XD_VER_BRANCH, __DATE__, __TIME__);

	printk("%s\n", h26xd_ver_str);
}

int __init h26xd_init(void)
{
	int retval = 0;

	memset(h265d_cabac_buffer, 0, sizeof(h265d_cabac_buffer));
	memset(engine_busy, 0, sizeof(engine_busy));
	memset(eng_util, 0, sizeof(eng_util));

	retval = h26xd_kdrv_init();
	if (retval < 0) {
		printk("%s, init fail! \n", __func__);
		return retval;
	}

	h26xd_msg();

	return retval;
}

void __exit h26xd_cleanup(void)
{
	h26xd_kdrv_cleanup(0);
}

module_init(h26xd_init);
module_exit(h26xd_cleanup);

MODULE_AUTHOR("Novatek Corp.");
MODULE_LICENSE("GPL");
MODULE_VERSION(H26XD_VER_STR);
