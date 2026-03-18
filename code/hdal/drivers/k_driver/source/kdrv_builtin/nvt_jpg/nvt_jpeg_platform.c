
#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/soc/nvt/fmem.h>
// #include <mach/fmem.h>

#include "kwrap/cpu.h"
#elif defined(__FREERTOS)

#include <kwrap/spinlock.h>
#include "kwrap/cpu.h"
#include <stdio.h>
#include <string.h>
#endif
#include "jpeg_platform.h"
#include "jpeg.h"
#include "jpeg_main.h"
#include "jpeg_api.h"
#include <kwrap/perf.h> //VOS_TICK
//#include <linux/clk-provider.h> //__clk_is_enabled

#ifdef __KERNEL__
int jpeg_gating_en = 1;
module_param(jpeg_gating_en, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(jpeg_gating_en, "1 for enable sw auto gating, 0 for always release.");
#endif

struct clk *p_jpeg_clk[JPEG_MAX_ENTITYS];
KDRV_JPEG_TRIG_INFO g_jpeg_trig_info;

#if defined(__FREERTOS)
unsigned int jpeg_debug_level = NVT_DBG_WARN;
static int is_create = 0;

#endif

#ifdef __KERNEL__
//static DEFINE_SPINLOCK(my_lock);

//#define loc_cpu(myflags)   spin_lock_irqsave(&my_lock, myflags)
//#define unl_cpu(myflags)   spin_unlock_irqrestore(&my_lock, myflags)

static spinlock_t		jpeg_spin_lock;
#if JPEG_SW_TIMEOUT
static vk_spinlock_t	jpeg_timer_lock;
#endif
#else
//static  VK_DEFINE_SPINLOCK(my_lock);
//#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
//#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)

static vk_spinlock_t jpeg_spin_lock;
#endif

// semaphore & interrupt flag id
#ifdef __KERNEL__
static SEM_HANDLE	SEMID_JPEG[JPEG_MAX_ENTITYS] = {0};
#else
static SEM_HANDLE	SEMID_JPEG;
#endif
static SEM_HANDLE	SEMID_JPEG_Q;

unsigned int utilization_record_unit = 5;
unsigned int jpeg_sw_timeout_period = 3000; /* ms, videograph timeout is 3.5s */ //20000 ~= 20s

struct workqueue_struct *jpeg_wq = NULL;

static ID			FLG_ID_JPEG;
// static FLGPTN	FLG_ID_JPEG;
#define FLGPTN_JPEG  FLGPTN_BIT(0)

//BRC control
extern JPEG_ENGINE_DATA jpeg_eng_data[JPEG_MAX_ENTITYS];

// extern u64 get_nvt_jiffies_u64(void);

/* time */
void jpeg_get_timems(void *p_tick)
{
#if 1
	vos_perf_mark((VOS_TICK *)p_tick);
#else
	*((UINT32 *)p_tick) = jiffies
#endif
}

BOOL jpeg_platform_dma_flush_dev2mem(BOOL enable, uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS
	return dma_flushReadCacheWithoutCheck(addr, size);

#elif defined(__FREERTOS)
	vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_FROM_DEVICE);
	return 0;

#elif defined __KERNEL__
	if (enable)
		vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_FROM_DEVICE);
	else
		vos_cpu_dcache_sync_vb((VOS_ADDR)addr, size, VOS_DMA_FROM_DEVICE);

	return 0;

#else
	return 0;

#endif
}


BOOL jpeg_platform_dma_flush_mem2dev(BOOL enable, uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS
	return dma_flushWriteCacheWithoutCheck(addr, size);

#elif defined(__FREERTOS)
	vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_TO_DEVICE);
	return 0;

#elif defined __KERNEL__
	if (enable)
		vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_TO_DEVICE);
	else
		vos_cpu_dcache_sync_vb((VOS_ADDR)addr, size, VOS_DMA_TO_DEVICE);

	return 0;

#else
	return 0;

#endif
}


BOOL jpeg_platform_dma_flush_dev2mem_width_neq_loff(BOOL enable, uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS
	return dma_flushReadCacheWidthNEQLineOffsetWithoutCheck(addr, size);

#elif defined(__FREERTOS)
	vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_BIDIRECTIONAL);
	return 0;

#elif defined __KERNEL__
	if (enable)
		vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_BIDIRECTIONAL);
	else
		vos_cpu_dcache_sync_vb((VOS_ADDR)addr, size, VOS_DMA_BIDIRECTIONAL);

	return 0;

#else
	return 0;

#endif
}


UINT32 jpeg_platform_dma_post_flush_dev2mem(uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS
	return dma_flushReadCacheWithoutCheck(addr, size);

#elif defined(__FREERTOS)
	vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_FROM_DEVICE);
	return 0;

#else
	vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_FROM_DEVICE);
	return 0;

#endif
}


UINT32 jpeg_platform_dma_is_cacheable(uintptr_t addr)
{
#if defined __UITRON || defined __ECOS ||defined __FREERTOS
	return dma_isCacheAddr(addr);
#else
	return 1;
#endif
}


void jpeg_platform_clk_set_freq(UINT32 engine_idx, UINT32 freq)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
// Set JPEG clock
	switch (freq) {
#if defined(_BSP_NA51055_)
	case PLL_CLKSEL_JPEG_320:
		if (pll_getPLLEn(PLL_ID_FIXED320) == FALSE) {
			pll_setPLLEn(PLL_ID_FIXED320, TRUE);
		}
		break;

	case PLL_CLKSEL_JPEG_PLL9:
		if (pll_getPLLEn(PLL_ID_9) == FALSE) {
			pll_setPLLEn(PLL_ID_9, TRUE);
		}
		break;
#elif defined(_BSP_NA51000_)
	case PLL_CLKSEL_JPEG_PLL6:
		if (pll_getPLLEn(PLL_ID_6) == FALSE) {
			pll_setPLLEn(PLL_ID_6, TRUE);
		}
		break;

	case PLL_CLKSEL_JPEG_PLL13:
		if (pll_getPLLEn(PLL_ID_13) == FALSE) {
			pll_setPLLEn(PLL_ID_13, TRUE);
		}
		break;
#endif
	default:
		break;
	}

	if (engine_idx == 0) {
		DBG_DUMP("PLL_CLKSEL_JPEG = 0x%x, freq = 0x%x \n", PLL_CLKSEL_JPEG, freq);
		pll_setClockRate(PLL_CLKSEL_JPEG, freq);
	}
	// if (engine_idx == 1) {
	// 	DBG_DUMP("PLL_CLKSEL_JPEG2 = 0x%x \n", PLL_CLKSEL_JPEG2);
	// 	pll_setClockRate(PLL_CLKSEL_JPEG2, freq);
	// }
	// if (engine_idx == 2) {
	// 	DBG_DUMP("PLL_CLKSEL_JPEG_LITE = 0x%x \n", PLL_CLKSEL_JPEG_LITE);
	// 	pll_setClockRate(PLL_CLKSEL_JPEG_LITE, freq);
	// }

#elif defined __KERNEL__
	struct clk *parent_clk;

	if (IS_ERR(p_jpeg_clk[engine_idx])) {
		//printk("%s: get clk fail %d\r\n", __func__, (int)(freq));
		printk("%s: get clk fail \r\n", __func__);
		return;
	}

	if (freq < 240) {
		printk("input frequency %d round to 240MHz\r\n", (int)(freq));
		parent_clk = clk_get(NULL, "fix240m");
	} else if (freq == 240) {
		parent_clk = clk_get(NULL, "fix240m");
	} else if (freq < 320) {
		printk("input frequency %d round to 240MHz\r\n", (int)(freq));
		parent_clk = clk_get(NULL, "fix240m");
	} else if (freq == 320) {
		parent_clk = clk_get(NULL, "pllf320");
	} else if (freq < 480) {
		printk("input frequency %d round to 320MHz\r\n", (int)(freq));
		parent_clk = clk_get(NULL, "pllf320");
	} else if (freq == 480) {
		printk("freq = 480, clk_get fix480m \n");
		parent_clk = clk_get(NULL, "fix480m");
	} else if (freq == 600) {
		printk("freq = 600, clk_get pll26 \n");
		parent_clk = clk_get(NULL, "pll26");
	} else {
		printk("input frequency %d round to 320MHz\r\n", (int)(freq));
		parent_clk = clk_get(NULL, "pllf320");
	}

	clk_set_parent(p_jpeg_clk[engine_idx], parent_clk);
	// clk_put(parent_clk);
#else

#endif
}


void jpeg_platform_clk_get_freq(UINT32 *p_freq)
{
#if defined __UITRON || defined __ECOS
    UINT32 freq;

		switch (pll_getClockRate(PLL_CLKSEL_JPEG)) {
		case PLL_CLKSEL_JPEG_240:
			return 240;

		case PLL_CLKSEL_JPEG_480:
			return 480;

		case PLL_CLKSEL_JPEG_PLL6:
			freq = pll_getPLLFreq(PLL_ID_6);
			freq += 500000;
			freq /= 1000000;
			return freq;

		default:
		case PLL_CLKSEL_JPEG_PLL13:
			freq = pll_getPLLFreq(PLL_ID_13);
			freq += 500000;
			freq /= 1000000;

			return freq;
		}
#elif defined __KERNEL__
	if (IS_ERR(p_jpeg_clk[0])) {
		printk("%s: get clk fail \r\n", __func__);
		*p_freq = 0;
		return;
	}
	printk("%s: get p_jpeg_clk[0] \n", __func__);
	*p_freq = clk_get_rate(p_jpeg_clk[0]);
#else

#endif
}


void jpeg_platform_clear_flg(void)
{
	clr_flg(FLG_ID_JPEG, FLGPTN_JPEG);
}


/* set by jpeg_isr() */
void jpeg_platform_set_flg(void)
{
	iset_flg(FLG_ID_JPEG, FLGPTN_JPEG);
}


BOOL jpeg_platform_check_flg(void)
{
	if (kchk_flg(FLG_ID_JPEG, FLGPTN_JPEG) & FLGPTN_JPEG)
		return TRUE;
	else
		return FALSE;
}


/* wait frame done */
void jpeg_platform_wait_flg(void)
{
	FLGPTN	ptn;
	INT32	wait_status = 0;

	// wai_flg(&ptn, FLG_ID_JPEG, FLGPTN_JPEG, TWF_ORW | TWF_CLR);
	wait_status = vos_flag_wait_timeout(&ptn, FLG_ID_JPEG, FLGPTN_JPEG, TWF_ORW | TWF_CLR, vos_util_msec_to_tick(20000)); //20s
	if (wait_status != 0)
		JPGDBG_ERR("error, wait_status = 0x%08x \n", wait_status);
}


ER jpeg_platform_sem_wait(unsigned int chip, unsigned int engine)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(SEMID_JPEG);
#else
#ifdef __KERNEL__
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	return SEM_WAIT(SEMID_JPEG[eng_data_idx]);
#else
	return SEM_WAIT(SEMID_JPEG);
#endif
#endif
}


ER jpeg_platform_sem_signal(unsigned int chip, unsigned int engine)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(SEMID_JPEG);
#else
#ifdef __KERNEL__
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	SEM_SIGNAL(SEMID_JPEG[eng_data_idx]);
#else
	SEM_SIGNAL(SEMID_JPEG);
#endif
	return E_OK;
#endif
}


ER jpeg_platform_queue_sem_wait(void)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(SEMID_JPEG_Q);
#else
	return SEM_WAIT(SEMID_JPEG_Q);
#endif
}


ER jpeg_platform_queue_sem_signal(void)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(SEMID_JPEG_Q);
#else
	SEM_SIGNAL(SEMID_JPEG_Q);
	return E_OK;
#endif
}


////////// engine busy/idle //////////
//0:sucessful(engine not busy,set it busy)
//1:engine is busy
int jpeg_test_and_set_engine_busy(unsigned int chip, unsigned int engine)
{
	int ret = 0;
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	unsigned long flags;

	if (chip >= JPEG_MAX_CHIP || engine >= JPEG_MAX_ENG) {
		DBG_ERR("chip/eng id(%d/%d) over max entity id\n", chip, engine);
		return 1;	// busy
	}

	vk_spin_lock_irqsave(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
	ret = jpeg_eng_data[eng_data_idx].engine_busy;
	if (0 == jpeg_eng_data[eng_data_idx].engine_busy) {
		jpeg_eng_data[eng_data_idx].engine_busy = 1;
	}
	vk_spin_unlock_irqrestore(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
	DBG_FUNC("{chip%d,engine%d} engine busy = %d\n", chip, engine, ret);
	return ret;
}

void jpeg_set_engine_idle(unsigned int chip, unsigned int engine)
{
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	unsigned long flags;

	if (chip >= JPEG_MAX_CHIP || engine >= JPEG_MAX_ENG) {
		DBG_ERR("chip/eng id(%d/%d) over max entity id\n", chip, engine);
		return;
	}

	vk_spin_lock_irqsave(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
	jpeg_eng_data[eng_data_idx].engine_busy = 0;
	vk_spin_unlock_irqrestore(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
}

void jpeg_set_codec_type(JPEG_ENGINE_DATA *p_eng_data, int codec_type)
{
	unsigned long flags;
	int eng_data_idx;

	eng_data_idx = JPEG_ENTITY_IDX(p_eng_data->chip_idx, p_eng_data->engine_idx);

	vk_spin_lock_irqsave(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
	p_eng_data->active_type = codec_type;
	vk_spin_unlock_irqrestore(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
}

#ifdef __KERNEL__
/* ---------------------------------------------------------------------------- */
/* software decode */
#if JPEG_SW_DECODE
static int jpg_sw_dec_thread(void *data)
{
	extern int jpg_sw_decoder(struct job_item_t *job_item);
	unsigned int gSWDecodePeriod = 1000;
	JPEG_ENGINE_DATA *eng_data = (JPEG_ENGINE_DATA *)data;
	int status;

	eng_data->sw_dec_thread_ready = 1;
	do {
		status = wait_event_timeout(eng_data->sw_dec_waitqueue, eng_data->sw_dec_wakeup_event, vos_util_msec_to_tick(gSWDecodePeriod));
		if (0 == status)
			continue;
		eng_data->sw_dec_wakeup_event = 0;
		jpg_sw_decoder(eng_data->sw_dec_job_item);
	} while (!kthread_should_stop());
	eng_data->sw_dec_thread_ready = 0;
	return 0;
}

void jpg_sw_dec_wakeup(unsigned int chip, unsigned int engine)
{
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);

	if (eng_data_idx >= JPEG_MAX_ENTITYS) {
		DBG_ERR("chip/eng id(%d/%d) over max entity id(%d)\n", chip, engine, JPEG_MAX_ENTITYS);
		return;
	}

	jpeg_eng_data[eng_data_idx].sw_dec_wakeup_event = 1;
	wake_up(&jpeg_eng_data[eng_data_idx].sw_dec_waitqueue);
}

static int jpeg_platform_create_swdec_thread(unsigned int chip_idx, unsigned int eng_idx, char *task_name)
{
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip_idx, eng_idx);

	init_waitqueue_head(&jpeg_eng_data[eng_data_idx].sw_dec_waitqueue);
	jpeg_eng_data[eng_data_idx].sw_dec_task = kthread_create(jpg_sw_dec_thread, (void *)(&jpeg_eng_data[eng_data_idx]), "sw decode thread");
	if (!jpeg_eng_data[eng_data_idx].sw_dec_task) {
		DBG_ERR("create sw dec task fail\n");
		return -2;
	}
	wake_up_process(jpeg_eng_data[eng_data_idx].sw_dec_task);
	jpeg_eng_data[eng_data_idx].sw_dec_job_item = NULL;

	return 0;
}

static int jpeg_platform_del_swdec_thread(unsigned int chip_idx, unsigned int eng_idx)
{
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip_idx, eng_idx);

	if (jpeg_eng_data[eng_data_idx].sw_dec_task) {
		kthread_stop(jpeg_eng_data[eng_data_idx].sw_dec_task);
		while (jpeg_eng_data[eng_data_idx].sw_dec_thread_ready)
			msleep(10);
	}
	return 0;
}
#endif //JPEG_SW_DECODE
/* ---------------------------------------------------------------------------- */
#if JPEG_SW_TIMEOUT
/* timer wq */
static void jpeg_platform_timeout_wq(struct work_struct *twork)
{
	struct delayed_work	*dwork = container_of(twork, struct delayed_work, work);
	platform_timer_list_t *timelist = container_of(dwork, platform_timer_list_t, worker);

	timelist->timeout_handle(timelist->chip_idx, timelist->eng_idx);
}

void jpeg_platform_setup_timer(unsigned int chip_idx, unsigned int eng_idx, platform_timer_list_t *timerlist, void (*timeout_handle)(unsigned int chip_idx, unsigned int eng_idx), unsigned int period_ms)
{
	timerlist->chip_idx = chip_idx;
	timerlist->eng_idx = eng_idx;

	if (jpeg_wq == NULL) {
		jpeg_wq = create_singlethread_workqueue("jpeg_wq");
		if (jpeg_wq == NULL)
			panic("%s, fail to create workq! \n", __func__);
	}

	if (timerlist->init_done == 0) {
		INIT_DELAYED_WORK(&timerlist->worker, jpeg_platform_timeout_wq);
		timerlist->init_done = 1;
	}
	timerlist->timeout_handle = timeout_handle;
	queue_delayed_work(jpeg_wq, &timerlist->worker, msecs_to_jiffies(period_ms + 5)); /* start timer */ //tolrence to timeout
}

//////// timeout ////////
int jpeg_check_engine_and_clear_timer(JPEG_ENGINE_DATA *p_eng_data)
{
	unsigned long flags;
	int status;

	vk_spin_lock_irqsave(&jpeg_timer_lock, flags);
	status = p_eng_data->encode_status;
	p_eng_data->timer_status = 0;
	vk_spin_unlock_irqrestore(&jpeg_timer_lock, flags);

	return status;
}

/* setup and start the timer */
static int jpeg_start_timer(JPEG_ENGINE_DATA *p_eng_data, unsigned int period_ms)
{
	extern void jpeg_timeout_handle(unsigned int chip_idx, unsigned int eng_idx);

	/* setup and start timer */
	jpeg_platform_setup_timer(p_eng_data->chip_idx, p_eng_data->engine_idx, &p_eng_data->timer, jpeg_timeout_handle, period_ms);

	p_eng_data->timer_status = 1;	/* kick off */
	DBG_FUNC("[%d] add timer with period_ms:%d\n", p_eng_data->chip_idx, period_ms);
	return 0;
}

void jpeg_platform_mod_timer(platform_timer_list_t *timerlist, unsigned int period_ms)
{
	cancel_delayed_work(&timerlist->worker);
	queue_delayed_work(jpeg_wq, &timerlist->worker, msecs_to_jiffies(period_ms)); /* start timer */
}

/* work in jpeg_timer_lock protection */
static int jpeg_mod_timer(JPEG_ENGINE_DATA *p_eng_data, unsigned int period_ms)
{
	jpeg_platform_mod_timer(&p_eng_data->timer, period_ms);

	p_eng_data->timer_status = 1;	/* kick off timer */
	DBG_FUNC("{chip:%d} mod timer\n", p_eng_data->chip_idx);
	return 0;
}

/* don't call it in atomic context */
void jpeg_platform_del_timer_sync(platform_timer_list_t *timerlist)
{
	cancel_delayed_work(&timerlist->worker);

	if (jpeg_wq != NULL) {
		flush_workqueue(jpeg_wq);
		destroy_workqueue(jpeg_wq);
		jpeg_wq = NULL;
	}
}

static int jpeg_del_timer(JPEG_ENGINE_DATA *p_eng_data)
{
	unsigned long flags;
	int	del_timer = 0;

	vk_spin_lock_irqsave(&jpeg_timer_lock, flags);
	if (0 == p_eng_data->encode_status && p_eng_data->timer_status) {
		del_timer = p_eng_data->timer_status;
		p_eng_data->timer_status = 0;
	}
	vk_spin_unlock_irqrestore(&jpeg_timer_lock, flags);

	if (del_timer)
		jpeg_platform_del_timer_sync(&p_eng_data->timer);

	DBG_FUNC("[%d] del timer\n", p_eng_data->chip_idx);
	return 0;
}
#endif //JPEG_SW_TIMEOUT

/* gating mclk & aclk. Regards to pclk, it is very minor and use pclk autogating is enough */
void jpeg_platform_clk_enable(struct clk *mclk, int enable, JPEG_ENGINE_DATA *p_eng_data)
{
	if ((mclk == NULL) /*|| (gating_en == 0)*/)
		return;

	if (mclk) {
		#if 0
		enable ? clk_enable(mclk) : clk_disable(mclk);
		#else
		if (enable && p_eng_data->clock_status == 0) {
			/*
			if (__clk_is_enabled(mclk)) {
				p_eng_data->clock_status = 1;
			}
			else
			*/
			{
				clk_enable(mclk);
				p_eng_data->clock_status = 1;
			}
		}
		else if (0 == enable && p_eng_data->clock_status && jpeg_gating_en) {
			clk_disable(mclk);
			p_eng_data->clock_status = 0;
		}
		#endif
	}
}

/* disable clock */
int jpeg_platform_clk_exit(struct clk *mclk)
{
	if (mclk) {
		clk_disable(mclk);
		// clk_put(mclk);
		// clk = NULL;
	}
	return 0;
}

/* A tricky point here that the clock must be enabled before calling this function.
 * Because jpeg_platform_clk_reset() will envoke disable and enable clock to do module reset. It must be under clock enabled state otherwise the kernel
 * will report warning message.
 */
void jpeg_platform_clk_reset(struct clk *mclk)
{
	/* module reset */
	clk_disable_unprepare(mclk);
	clk_prepare_enable(mclk);
}

int jpeg_engine_init(void)
{
	unsigned int chip, engine, eng_data_idx;
	int ret = 0;

	//memset(jpeg_eng_data, 0, sizeof(jpeg_eng_data));
	for (chip = 0; chip < JPEG_MAX_CHIP; chip++) {
		for (engine = 0; engine < JPEG_MAX_ENG; engine++) {
			eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
			
			//check io_addr
			if (jpeg_eng_data[eng_data_idx].engine_base_va == 0) {
				DBG_DUMP("io_addr error in engine(%d) \n", eng_data_idx);
				ret = -1;
			}

			/* spinlock init */
			vk_spin_lock_init(&jpeg_eng_data[eng_data_idx].engine_lock);
			vk_spin_lock_init(&jpeg_eng_data[eng_data_idx].func_lock);

			// DBG_DUMP("jpeg_eng_data[%d].engine_base_va = 0x%lx \n", eng_data_idx, (unsigned long)jpeg_eng_data[eng_data_idx].engine_base_va);
		
			/* init hw */
			jpeg_open(chip, engine, jpeg_eng_data[eng_data_idx].engine_base_va);
			/* sw decode */
			#if JPEG_SW_DECODE
			jpeg_platform_create_swdec_thread(chip, engine, "sw decode thread");
			#endif
		}
	}

#if JPEG_SW_TIMEOUT
	vk_spin_lock_init(&jpeg_timer_lock);
	for (chip = 0; chip < JPEG_MAX_CHIP; chip++) {
		for (engine = 0; engine < JPEG_MAX_ENG; engine++) {
			eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
			jpeg_start_timer(&jpeg_eng_data[eng_data_idx], jpeg_sw_timeout_period);
		}
	}
#endif //JPEG_SW_TIMEOUT

#if JPEGENC_CHUCKSUM
	v_heavyload_addr = ioremap_nocache(HEAVYLOAD_BASE, (size_t)0x1000);
#endif

	return ret;
}

void jpeg_engine_exit(void)
{
	unsigned int chip, engine, eng_data_idx;

	for (chip = 0; chip < JPEG_MAX_CHIP; chip++) {
		for (engine = 0; engine < JPEG_MAX_ENG; engine ++) {
			eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
#if JPEG_SW_TIMEOUT
			jpeg_del_timer(&jpeg_eng_data[eng_data_idx]);
#endif
			// clear
			jpeg_close(chip, engine, jpeg_eng_data[eng_data_idx].engine_base_va);
			#if JPEG_SW_DECODE
			jpeg_platform_del_swdec_thread(chip, engine);
			#endif
		}
	}
}

////////// engine util //////////
void jpeg_mark_engine_start(unsigned int chip, unsigned int engine)
{
	unsigned long flags;
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);

	if (chip >= JPEG_MAX_CHIP || engine >= JPEG_MAX_ENG) {
		DBG_ERR("chip/eng id(%d/%d) over max entity id\n", chip, engine);
		return;
	}
	DBG_IND("{chip:%d,eng:%d}%s \n", chip, engine, __func__);

	vk_spin_lock_irqsave(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
	if (jpeg_eng_data[eng_data_idx].engine_start != 0)
		DBG_WRN(BHYEL"Warning to nested use chip[%d-%d] mark_engine_start function !! "COLOR_END, chip, engine);

	jpeg_get_timems(&jpeg_eng_data[eng_data_idx].engine_start); //jpeg_eng_data[eng_data_idx].engine_start = jpeg_get_timems();
	DBG_IND("start engine time %u\n", jpeg_eng_data[eng_data_idx].engine_start);
	jpeg_eng_data[eng_data_idx].engine_end = 0;
	if (jpeg_eng_data[eng_data_idx].utilization_start == 0) {
		jpeg_eng_data[eng_data_idx].utilization_start = jpeg_eng_data[eng_data_idx].engine_start;
		jpeg_eng_data[eng_data_idx].engine_time = 0;
	}
	jpeg_eng_data[eng_data_idx].isr_timestamp = 0;
	vk_spin_unlock_irqrestore(&jpeg_eng_data[eng_data_idx].engine_lock, flags);

#if JPEG_SW_TIMEOUT
	vk_spin_lock_irqsave(&jpeg_timer_lock, flags);
	jpeg_eng_data[eng_data_idx].encode_status = 1;
	jpeg_mod_timer(&jpeg_eng_data[eng_data_idx], jpeg_sw_timeout_period);
	vk_spin_unlock_irqrestore(&jpeg_timer_lock, flags);
#endif
}

void jpeg_mark_engine_end(unsigned int chip, unsigned int engine, int mod_timer)
{
	unsigned long flags;
	int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);

	if (chip >= JPEG_MAX_CHIP || engine >= JPEG_MAX_ENG) {
		DBG_ERR("chip/eng id(%d/%d) over max entity id\n", chip, engine);
		return;
	}

	DBG_IND("{chip:%d,eng:%d}%s \n", chip, engine, __func__);

	vk_spin_lock_irqsave(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
	if (jpeg_eng_data[eng_data_idx].engine_end != 0) {
		vk_spin_unlock_irqrestore(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
		DBG_WRN(BHYEL"Warning to nested use chip[%d-%d] mark_engine_finish function !! "COLOR_END, chip, engine);
		return;
	}

	jpeg_get_timems(&jpeg_eng_data[eng_data_idx].engine_end); //jpeg_eng_data[eng_data_idx].engine_end = jpeg_get_timems();
	DBG_IND("end engine time %u\n", jpeg_eng_data[eng_data_idx].engine_end);
	if (jpeg_eng_data[eng_data_idx].engine_end > jpeg_eng_data[eng_data_idx].engine_start)
		jpeg_eng_data[eng_data_idx].engine_time += vos_perf_duration(jpeg_eng_data[eng_data_idx].engine_start, jpeg_eng_data[eng_data_idx].engine_end);

	if (jpeg_eng_data[eng_data_idx].utilization_start > jpeg_eng_data[eng_data_idx].engine_end) {
		jpeg_eng_data[eng_data_idx].utilization_start = 0;
		jpeg_eng_data[eng_data_idx].engine_time = 0;
	} else if ((jpeg_eng_data[eng_data_idx].engine_start < jpeg_eng_data[eng_data_idx].engine_end) &&
		   (vos_perf_duration(jpeg_eng_data[eng_data_idx].utilization_start, jpeg_eng_data[eng_data_idx].engine_end) >= utilization_record_unit * 1000000)) {
		unsigned int util;

		util = (100 * jpeg_eng_data[eng_data_idx].engine_time) / (vos_perf_duration(jpeg_eng_data[eng_data_idx].utilization_start, jpeg_eng_data[eng_data_idx].engine_end));
		jpeg_eng_data[eng_data_idx].utilization_record = util;
		jpeg_eng_data[eng_data_idx].utilization_start = 0;
		jpeg_eng_data[eng_data_idx].engine_time = 0;
	}
	jpeg_eng_data[eng_data_idx].engine_start = 0;
	vk_spin_unlock_irqrestore(&jpeg_eng_data[eng_data_idx].engine_lock, flags);
#if JPEG_SW_TIMEOUT
	vk_spin_lock_irqsave(&jpeg_timer_lock, flags);
	jpeg_eng_data[eng_data_idx].encode_status = 0;
	if (mod_timer) {
		/* in order to keep timer running and avoid racing condition */
		jpeg_mod_timer(&jpeg_eng_data[eng_data_idx], jpeg_sw_timeout_period);
	}
	vk_spin_unlock_irqrestore(&jpeg_timer_lock, flags);
#endif //JPEG_SW_TIMEOUT
}
#endif //__KERNEL__

unsigned int nvt_jpeg_get_engine_util(unsigned int chip, unsigned int engine)
{
	int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	unsigned int current_time;

	if (chip >= JPEG_MAX_CHIP || engine >= JPEG_MAX_ENG) {
		DBG_ERR("chip/eng id(%d/%d) over max entity id\n", chip, engine);
		return 0;
	}

	jpeg_get_timems(&current_time);
	if (jpeg_eng_data[eng_data_idx].engine_end > current_time) {
		DBG_WRN("[chip%d/engine%d] engine end time=%u > current_time=%u \n", chip, engine, jpeg_eng_data[eng_data_idx].engine_end, current_time);
		return 0;
	}

	//5s
	if ((jpeg_eng_data[eng_data_idx].engine_start < jpeg_eng_data[eng_data_idx].engine_end) &&
		(vos_perf_duration(jpeg_eng_data[eng_data_idx].engine_end, current_time) >= 5000000)) {
		DBG_FUNC("[chip%d/engine%d] engine duration time=%u (engine_end=%u, current_time=%u) \n", chip, engine, 
					current_time - jpeg_eng_data[eng_data_idx].engine_end, jpeg_eng_data[eng_data_idx].engine_end, current_time);
		return 0;
	}

	return jpeg_eng_data[eng_data_idx].utilization_record;
}

unsigned int nvt_jpeg_clear_engine_util(unsigned int chip, unsigned int engine)
{
	int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	if (chip >= JPEG_MAX_CHIP || engine >= JPEG_MAX_ENG) {
		DBG_ERR("chip/eng id(%d/%d) over max entity id\n", chip, engine);
		return 0;
	}
	jpeg_eng_data[eng_data_idx].utilization_record = 0;
	return 0;
}

int nvt_jpeg_get_engine_info(unsigned int chip, unsigned int engine, JPEG_ENG_INFO *pEngInfo)
{
	int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	if (chip >= JPEG_MAX_CHIP || engine >= JPEG_MAX_ENG) {
		DBG_ERR("chip/eng id(%d/%d) over max entity id\n", chip, engine);
		return 0;
	}
	if (jpeg_eng_data[eng_data_idx].active == 0) {
		pEngInfo->active = 0;
	}
	else {
		pEngInfo->engine_base_pa = jpeg_eng_data[eng_data_idx].engine_base_pa;
		pEngInfo->engine_base_va = jpeg_eng_data[eng_data_idx].engine_base_va;
		pEngInfo->active = jpeg_eng_data[eng_data_idx].active;
		pEngInfo->engine_busy = jpeg_eng_data[eng_data_idx].engine_busy;
		pEngInfo->active_type = jpeg_eng_data[eng_data_idx].active_type;
		pEngInfo->clk_flag = jpeg_eng_data[eng_data_idx].clock_status;
	}
	return 0;
}

/*
 lookup engine index from io_addr
 0xFF: fail
*/
int jpeg_lookup_engine_idx(uintptr_t io_addr)
{
	UINT32 eng_idx;
	for (eng_idx = 0; eng_idx < JPEG_MAX_ENTITYS; eng_idx++) {
		if (jpeg_eng_data[eng_idx].active && jpeg_eng_data[eng_idx].engine_base_va == io_addr)
			return eng_idx;
	}
	return 0xff;
}

KDRV_JPEG_TRIG_INFO *kdrv_jpeg_get_triginfo_by_coreid(void)
{
	return &g_jpeg_trig_info;
}


void jpeg_platform_triginfo_init(void)
{
	// init trig info
	memset(&g_jpeg_trig_info, 0x00, sizeof(g_jpeg_trig_info));
    g_jpeg_trig_info.p_queue = kdrv_jpeg_get_queue_by_coreid();
	g_jpeg_trig_info.tri_func = (JPEG_TRI_FUNC) jpeg_trigger;
}


void jpeg_create_resource(unsigned int chip, unsigned int engine)
{
#ifdef __KERNEL__
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	OS_CONFIG_FLAG(FLG_ID_JPEG);
	SEM_CREATE(SEMID_JPEG[eng_data_idx], 1);
	SEM_CREATE(SEMID_JPEG_Q, 1);
	// coverity[side_effect_free]: driver doses not use return value
	spin_lock_init(&jpeg_spin_lock);
#else
	if (!is_create) {
		//OS_CONFIG_FLAG(FLG_ID_JPEG);
		cre_flg(&FLG_ID_JPEG, NULL, "FLG_ID_JPEG");
		vos_sem_create(&SEMID_JPEG, 1, "SEMID_JPEG");
		vos_sem_create(&SEMID_JPEG_Q, 1, "SEMID_JPEG");
		is_create = 1;

		vk_spin_lock_init(&jpeg_spin_lock);

		jpeg_platform_triginfo_init();
	}
#endif
}


void jpeg_release_resource(unsigned int chip, unsigned int engine)
{
#ifdef __KERNEL__
	unsigned int eng_data_idx = JPEG_ENTITY_IDX(chip, engine);
	rel_flg(FLG_ID_JPEG);
	SEM_DESTROY(SEMID_JPEG[eng_data_idx]);
#else
	vos_sem_destroy(SEMID_JPEG);
	is_create = 0;
#endif
}


void jpeg_platform_set_ist_event(void)
{
	/*  Tasklet for bottom half mechanism */
	//tasklet_schedule(jpeg_module_info->jpg_tasklet);
}


void jpeg_isr_bottom(UINT32 engine_idx)
{
	KDRV_JPEG_TRIG_INFO *p_trig;
	KDRV_JPEG_QUEUE_ELEMENT   element;
	//unsigned long flag;
	unsigned long spin_flags;
	UINT32 bufsize;

	uintptr_t io_addr = jpeg_eng_data[engine_idx].engine_base_va;
	// printk("%s: io_addr(engine1) = 0x%lx", __func__, io_addr);

	// Enter critical section
	spin_flags = jpeg_platform_spin_lock();
	p_trig = kdrv_jpeg_get_triginfo_by_coreid();

	if (p_trig->cb == NULL) {
		//printk("callback == NULL\r\n");
	} else {
		//printk("callback != NULL\r\n");
		if(p_trig->jpeg_dec_param == NULL)
		{
			p_trig->jpeg_enc_param->bs_buf_size[0] = jpeg_get_bssize(io_addr) + p_trig->jpeg_enc_param->bs_hdr_size;
			jpeg_flush_bsbuf(p_trig->jpeg_enc_param->bs_addr_va[0], p_trig->jpeg_enc_param->bs_buf_size[0]);
#if 0//JPEG_DEBUG
#if defined __KERNEL__
			printk("p_enc_param->bs_buf_size[0] = 0x%x\r\n", (unsigned int)(p_trig->jpeg_enc_param->bs_buf_size[0]));
#endif
#endif
			p_trig->cb->callback(p_trig->jpeg_enc_param, p_trig->user_data);
		} else {
			bufsize = p_trig->jpeg_dec_param->uiWidth*p_trig->jpeg_dec_param->uiHeight;
			vos_cpu_dcache_sync((UINT32)p_trig->jpeg_dec_param->y_addr_va, (bufsize*3)>>1, VOS_DMA_FROM_DEVICE);

			/*
			if(events==1)
			{
				//CB return not decoded
				p_trig->jpeg_dec_param->errorcode = 0xFFFF;
			}
			*/
			p_trig->cb->callback(p_trig->jpeg_dec_param, NULL);
		}
	}

	//loc_cpu(flag);
	if (kdrv_jpeg_queue_is_empty_p(p_trig->p_queue)) {
		p_trig->is_busy = FALSE;
		//unl_cpu(flag);
		jpeg_platform_spin_unlock(spin_flags);
	} else {
		//KDRV_JPEG_QUEUE_ELEMENT   element;

		kdrv_jpeg_queue_del_p(p_trig->p_queue, &element);
		p_trig->cb        = element.p_cb_func;
		//unl_cpu(flag);
		jpeg_platform_spin_unlock(spin_flags);

		if (element.jpeg_mode)
			p_trig->tri_func(1, element.jpeg_dec_param, element.p_cb_func, io_addr);
		else
			p_trig->tri_func(0, element.jpeg_enc_param, element.p_cb_func, io_addr);
	}

}


int jpeg_platform_ist(UINT32 engine_idx)
{
	jpeg_isr_bottom(engine_idx);

	return 0;
}


#ifdef __KERNEL__
void *jpeg_platform_malloc(UINT32 size)
{
	return (void *)kmalloc(size, GFP_KERNEL);
}

void jpeg_platform_free(void *ptr)
{
	kfree(ptr);
}

uintptr_t jpeg_va2pa(uintptr_t addr)
{
	return fmem_lookup_pa(addr);
}

uintptr_t jpeg_use_pa(uintptr_t addr)
{
#if JPEGSDK_USE_PA
	return (addr);
#else
	return fmem_lookup_pa(addr);
#endif
}
#endif //__KERNEL__


UINT32 jpeg_platform_spin_lock(void)
{
#if defined __UITRON || defined __ECOS
	loc_cpu();
#elif defined(__FREERTOS)
	unsigned long flags;
	vk_spin_lock_irqsave(&jpeg_spin_lock, flags);
	return flags;
#else
	unsigned long flags;
	spin_lock_irqsave(&jpeg_spin_lock, flags);
	return flags;
#endif
}


void jpeg_platform_spin_unlock(unsigned long flags)
{
#if defined __UITRON || defined __ECOS
	unl_cpu();
#elif defined(__FREERTOS)
	vk_spin_unlock_irqrestore(&jpeg_spin_lock, flags);
#else
	spin_unlock_irqrestore(&jpeg_spin_lock, flags);
#endif
}


UINT32 jpeg_platform_atomic_context(void)
{
	int ret = 0;
#ifdef __KERNEL__
	if (in_atomic()) {
		// if (printk_ratelimit())
		// 	printk("scheduling while in_atomic !! \n");
		ret = 1;
	}
	if (irqs_disabled()) {
		// if (printk_ratelimit())
		// 	printk("scheduling while irqs_disabled !! \n");
		ret = 1;
	}
	if (in_interrupt()) {
		// if (printk_ratelimit())
		// 	printk("scheduling while in_interrupt !! \n");
		ret = 1;
	}
#endif
	return ret;
}


#if JPEGENC_CHUCKSUM
static volatile uintptr_t *v_heavyload_addr = NULL;
UINT32 heavyload_checksum(uintptr_t phy_addr, unsigned int length)
{
	u32 checksum = 0, value;
	int ch = 0;

	if (phy_addr & 0x03) {
		printk("addr need word align\n");
		return -1;
	}
	if (length & 0x03) {
		printk("length need word align\n");
		return -1;
	}

	value = readl(v_heavyload_addr + DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS);
	if (value & (0x1 << ch)) {
		printk("Error! ch%d is enabled already! \n", ch);
		//return 0x0;
	}

	// set heavy load region
	writel(HVY_LOAD_TEST_TIMES | HVY_LOAD_SKIP_COMPARE | HVY_LOAD_READ_CHKSUM | HVY_LOAD_BURST_64, v_heavyload_addr + HEAVY_LOAD_CTRL_OFS(ch));
	writel(phy_addr, v_heavyload_addr + HEAVY_LOAD_ADDR_OFS(ch));
	writel(length, v_heavyload_addr + HEAVY_LOAD_SIZE_OFS(ch));

	// trigger heavy load
	value = readl(v_heavyload_addr + DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS);
	value |= (0x1 << ch);
	writel(value, v_heavyload_addr + DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS);

	while (1) {
		//cond_resched();

		value = readl(v_heavyload_addr + DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS);
		if (!(value & (0x1 << ch)))
			break;
	}

	//checksum = ((readl(v_heavyload_addr + 0x06C) & 0xFFFF0000) >> 16) + (((1 + length) * length) >> 1);
	checksum = (readl(v_heavyload_addr + HEAVY_LOAD_WAIT_CYCLE_OFS(ch)) & 0xFFFF0000) >> 16;

	// printk("HW checksum = 0x%x\n", checksum);

	return (UINT32)checksum;
}
#endif //JPEGENC_CHUCKSUM


#ifdef __KERNEL__
EXPORT_SYMBOL(nvt_jpeg_get_engine_util);
EXPORT_SYMBOL(nvt_jpeg_clear_engine_util);
EXPORT_SYMBOL(nvt_jpeg_get_engine_info);
#endif
