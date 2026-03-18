#ifndef __MODULE_PLATFORM_H__
#define __MODULE_PLATFORM_H__


#if (defined __UITRON || defined __ECOS)
	#include  "Type.h"
	#include "dma_protected.h"

	#define JPGDBG_ERR  DBG_ERR
	#define JPGDBG_WRN  DBG_WRN
	#define JPGDBG_IND  DBG_IND
#elif defined __KERNEL__
	#include "kwrap/type.h"
	#include "kwrap/flag.h"
	#include "kwrap/semaphore.h"
	#include <linux/clk.h>

	#define JPGDBG_ERR(fmtstr, args...) nvt_dbg(ERR, fmtstr, ##args)
	#define JPGDBG_WRN(fmtstr, args...) nvt_dbg(WARN, fmtstr, ##args)
	#define JPGDBG_IND(fmtstr, args...) nvt_dbg(INFO, fmtstr, ##args)

	// extern uintptr_t IOADDR_JPEG_REG_BASE;
#else
	#include "kwrap/type.h"
	#include "kwrap/flag.h"
	#include "kwrap/semaphore.h"
	#include "io_address.h"
	#include "pll_protected.h"
	#include "dma_protected.h"
	#include "jpeg_dbg.h"

	#define JPGDBG_ERR  DBG_ERR
	#define JPGDBG_WRN  DBG_WRN
	#define JPGDBG_IND  DBG_IND
#endif

#include "kwrap/util.h" //for vos_flag_wait_timeout
#include "kwrap/spinlock.h" //vk_spinlock_t

#if defined(__FREERTOS)
#include "plat/top.h"
#else
#include <plat/top.h>
#include "pll_protected.h"
#endif


//_BSP_NS02302_
#define JPEG_MAX_CHIP	1
#define JPEG_MAX_ENG	1

#define JPEG_ENGINE_LITE_IDX	2

#define JPEG_MAX_ENTITYS	((JPEG_MAX_CHIP)*(JPEG_MAX_ENG))
#define JPEG_ALL_ENGINE		0xFF

#define JPEG_ENTITY_IDX(chip, engine)   ((chip*JPEG_MAX_ENG) + engine)
#define JPEG_CHIP_ID(idx)               ((idx)/JPEG_MAX_ENG)
#define JPEG_ENGINE_ID(idx)             ((idx)%JPEG_MAX_ENG)

#define JPEG_BRC2     1
#define JEPG_BRC_NEW_UPDATE_MODE	1

//BRC def
#define  DEFAULT_BRC_QF  500

#define  IMAGE_RETRY_COUNTS    4
//VBR mode
#define  DEFAULT_BRC_VBR_QUALITY       256

//support non-block mode (0: block mode only)
#define SUP_NON_BLK_MODE	0
#define ENC_DEC_PROTECT_METHOD	1
#define JPEG_ADJUST_ROI_TABLE	1

#if SUP_NON_BLK_MODE
#define JPEG_SW_DECODE		1
#define JPEG_SW_TIMEOUT		1
#else
#define JPEG_SW_DECODE		1
#define JPEG_SW_TIMEOUT		0
#endif

//heavyload checksum
#define JPEGENC_CHUCKSUM							(0)
#define HVY_LOAD_SKIP_COMPARE						(0x01 << 2)
#define HVY_LOAD_READ_CHKSUM						(0x02 << 4)
#define HVY_LOAD_BURST_64							(0x3F << 8)
#define HVY_LOAD_TEST_TIMES							(0x1 << 16)
#define DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS			0x60
#define DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS		0x64
#define DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS		0x68
#define DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS		0x6C
#define DMA_CHANNEL0_HEAVY_LOAD_TRIG_OFS			0x5C
#define HEAVY_LOAD_CTRL_OFS(ch)						(DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_ADDR_OFS(ch)						(DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_SIZE_OFS(ch)						(DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_WAIT_CYCLE_OFS(ch)				(DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS + ((ch) * 0x10))
#define HEAVYLOAD_BASE								0x2F00A0000

//cache
#define CACHE_LINE_SZ				64
#define CACHE_LINE_SHIFT			6
#define CACHE_LINE_MASK				(CACHE_LINE_SZ - 1)
#define CACHE_ALIGN_UP(x)			((((x) + CACHE_LINE_SZ - 1) >> CACHE_LINE_SHIFT) << CACHE_LINE_SHIFT)
#define CACHE_ALIGN_DOWN(x)			(((x) >> CACHE_LINE_SHIFT) << CACHE_LINE_SHIFT)
#define CACHE_HANDLE				0

//both ipc/xvr use this pa define
#define JPEGSDK_USE_PA			ENABLE

#ifdef __KERNEL__
typedef struct task_struct		platform_task_struct_t;
typedef wait_queue_head_t		platform_wait_queue_head_t;

/* clock */
typedef struct {
	unsigned int chip_idx;
	unsigned int eng_idx;
	struct clk *pclk;
	struct clk *mclk;
} platform_clk_t;

typedef struct {
	unsigned int chip_idx;
	unsigned int eng_idx;
	struct delayed_work	 worker;
	void (*timeout_handle)(unsigned int chip_idx, unsigned int eng_idx);
	unsigned int init_done;
} platform_timer_list_t;
#else
typedef int platform_task_struct_t;
typedef int	platform_wait_queue_head_t;

typedef struct {
	unsigned int chip_idx;
	unsigned int eng_idx;
	int	dummy;
} platform_clk_t;

typedef int platform_timer_list_t;
#endif


typedef struct jpg_engine_data_t
{
	UINT32 chip_idx;
	UINT32 engine_idx;
	uintptr_t engine_base_pa;
	uintptr_t engine_base_va;
	int active;
	struct clk *mclk;
	// platform_clk_t clock; //unused

	/**** utilization ****/
	UINT32 utilization_start;
	UINT32 utilization_record;
	UINT32 engine_start;
	UINT32 engine_end;
	UINT32 engine_time;
	/**** engine status ****/
	int engine_busy;
	// int engine_interrupt; //unused

	vk_spinlock_t engine_lock;
	UINT32 active_type;
#if SUP_NON_BLK_MODE
	struct job_item_t *fire_job;
#endif
	int clock_status;
#if JPEG_SW_DECODE
	platform_task_struct_t		*sw_dec_task;
	platform_wait_queue_head_t	sw_dec_waitqueue;
	int sw_dec_wakeup_event;
	volatile int sw_dec_thread_ready;
	struct job_item_t *sw_dec_job_item;
#endif
	/* statistic */
	unsigned int timeout_count;
	unsigned int job_enc_err_cnt;
	unsigned int job_dec_err_cnt;
	vk_spinlock_t func_lock;
	unsigned int isr_timestamp;
	platform_timer_list_t reset_timer;
#if JPEG_SW_TIMEOUT
	platform_timer_list_t timer;
	unsigned int encode_status;
	unsigned int timer_status;
#endif
} JPEG_ENGINE_DATA;

#if defined(__FREERTOS)
	// #define JPEG_SETREG(ofs, value)     OUTW(IOADDR_JPEG_REG_BASE+(ofs), (value))
	// #define JPEG_GETREG(ofs)            INW(IOADDR_JPEG_REG_BASE+(ofs))
	#define JPEG_SETREG(ofs, value) OUTW(io_addr+(ofs), (value))
	#define JPEG_GETREG(ofs)        INW(io_addr+(ofs))
#else
	extern void (*nvt_jpeg_set_reg)(uintptr_t vaddr, unsigned int val);
	#define JPEG_SETREG(ofs, value) nvt_jpeg_set_reg(io_addr + (ofs), value)	//OUTW(io_addr+(ofs), (value))
	#define JPEG_GETREG(ofs)        INW(io_addr+(ofs))
#endif

extern UINT32 jpeg_platform_dma_is_cacheable(uintptr_t addr);
extern BOOL jpeg_platform_dma_flush_dev2mem(BOOL enable, uintptr_t addr, UINT32 size);
extern BOOL jpeg_platform_dma_flush_mem2dev(BOOL enable, uintptr_t addr, UINT32 size);
extern BOOL jpeg_platform_dma_flush_dev2mem_width_neq_loff(BOOL enable, uintptr_t addr, UINT32 size);
extern UINT32 jpeg_platform_dma_post_flush_dev2mem(uintptr_t addr, UINT32 size);

extern int jpeg_platform_ist(UINT32 engine_idx);
extern void jpeg_isr_bottom(UINT32 engine_idx);

extern void jpeg_create_resource(unsigned int chip, unsigned int engine);
extern void jpeg_release_resource(unsigned int chip, unsigned int engine);

extern void jpeg_platform_clear_flg(void);
extern void jpeg_platform_set_flg(void);
extern BOOL jpeg_platform_check_flg(void);
extern void jpeg_platform_wait_flg(void);

extern ER jpeg_platform_sem_wait(unsigned int chip, unsigned int engine);
extern ER jpeg_platform_sem_signal(unsigned int chip, unsigned int engine);

extern void jpeg_platform_clk_get_freq(UINT32 *p_freq);
extern void jpeg_platform_clk_set_freq(UINT32 engine_idx, UINT32 freq);

extern void jpeg_platform_triginfo_init(void);
extern UINT32 jpeg_platform_spin_lock(void);
extern void jpeg_platform_spin_unlock(unsigned long flags);

extern int		jpeg_lookup_engine_idx(uintptr_t io_addr);
void			jpeg_get_timems(void *p_tick);

/* clock init and enable */
int				jpeg_platform_clk_exit(struct clk *mclk);
void			jpeg_platform_clk_enable(struct clk *mclk, int enable, JPEG_ENGINE_DATA *p_eng_data);
void			jpeg_platform_clk_reset(struct clk *mclk);

/* multi engine */
extern int		jpeg_engine_init(void);
extern void		jpeg_engine_exit(void);
extern int		jpeg_test_and_set_engine_busy(unsigned int chip, unsigned int engine);
extern void		jpeg_set_engine_idle(unsigned int chip, unsigned int engine);
extern void		jpeg_mark_engine_start(unsigned int chip, unsigned int engine);
extern void		jpeg_mark_engine_end(unsigned int chip, unsigned int engine, int mod_timer);
#if JPEG_SW_TIMEOUT
extern int		jpeg_check_engine_and_clear_timer(JPEG_ENGINE_DATA *p_eng_data);
#endif

#if JPEG_SW_DECODE
extern void jpg_sw_dec_wakeup(unsigned int chip, unsigned int engine);
#endif

extern ER jpeg_platform_queue_sem_wait(void);
extern ER jpeg_platform_queue_sem_signal(void);
#endif
