#ifndef __PLATFORM_PORT_H__
#define __PLATFORM_PORT_H__

/*
 * This module is ported in Visual C++ and the compile flags are both WIN32 and __FREERTOS.
 */
#include <kdrv_type.h>

#define MAX_ENGINE		1
#define ENGINE_BASE		0xF0A20000

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <asm/uaccess.h> /* for get_fs, set_fs */
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <plat/nvt_jiffies.h>
//#include <comm/log.h> //it is not used in ipcam sdk.

#elif defined(WIN32) /* WIN32 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#else /* __FREERTOS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/nvt_type.h>
#include <kwrap/type.h>
#include <kwrap/semaphore.h>
#include <kwrap/task.h>
#include <interrupt.h>
//#include <comm/log.h>
#endif /* __FREERTOS */

/* All OS */

/* ----------------------------------------------------------------------
 * WIN32 only
 * ----------------------------------------------------------------------
 */
#ifdef WIN32

#define inline			__inline
#define TickType_t		unsigned int
#define vk_spinlock_t	int
#define __VK_SPIN_LOCK_UNLOCKED(x)	0
#define vk_spin_lock_init(x)		do {*(x) = 0;} while (0)
#define vk_spin_lock_irqsave(x,y)	do { *(x) = 1;} while(0)
#define vk_spin_unlock_irqrestore(x,y)	do { *(x) = 0; (y) = 0; } while(0)
#define vos_cpu_dcache_sync(x,y,z)
#define vos_sem_create(x, y, z)		do { *(x) = (y); } while(0)
#define vos_sem_sig(x)	do {(x) ++; } while(0)
#define vos_sem_wait(x)	do {(x) --; } while(0)
#define snprintf					_snprintf
#define configTICK_RATE_HZ	1000		/* if per HZ 100tick, each tick=10ms */
#define vos_util_delay_ms(x)	Sleep(x)	//unit is msec
#define free_irq(x, y)
#define THREAD_HANDLE	int
#define ID	int
#define SEM_HANDLE	int
#define VDEC_GCLK		1
#define H264D_M_GCLK	2
#define H265D_M_GCLK	3
#define H264D_CLK		153
#define H265D_CLK		154
#define INT_ID_VDEC		168
#define IRQF_TRIGGER_HIGH	0x4
#define pll_set_pclk_auto_gating(x)
#define pll_clear_pclk_auto_gating(x)
#define pll_set_clk_auto_gating(x)
#define pll_clear_clk_auto_gating(x)
#define pll_enable_clock(x)
#define	pll_disable_clock(x)
extern void vdec_win32_printm(char *module, const char *fmt, ...);
/* rtos has printm(). Thus here is only for WIN32 */
#define printm	vdec_win32_printm

enum irqreturn {
	IRQ_NONE                = (0 << 0),
    IRQ_HANDLED             = (1 << 0),
    IRQ_WAKE_THREAD         = (1 << 1),
};

typedef enum irqreturn irqreturn_t;
typedef irqreturn_t (* irq_handler_t)(int irq, void *dev);
int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev);

void win32_persudo_interrupt(int chip_idx);
#define kick_bh(x,y,z)
#define request_irq_bh(x,y,z)	0

#endif /* WIN32 */

/* -----------------------------------------------------------------
 * __FREERTOS. The following code is compiled in Windows as well.
 * -----------------------------------------------------------------
 */
#ifdef __FREERTOS

typedef UINT32	u32;
typedef INT32	s32;
typedef UINT16	u16;
typedef INT16	s16;
typedef UINT64	u64;
typedef INT8	s8;
typedef UINT8	u8;

#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif

struct resource {
	uintptr_t start;
	uintptr_t end;
};

struct device_node {
	int dummy;
};

struct device {
	struct device_node	*of_node;
	int dummy;
};

struct platform_device {
	int	irq_num;
	int device_id;
	struct resource engine[MAX_ENGINE];
	struct device	dev;
};

struct platform_driver {
	int (*probe)(struct platform_device *);
	int (*remove)(struct platform_device *);

	struct {
		void *owner;
		char *name;
	} driver;
};

struct work_struct {
	THREAD_HANDLE	task;
	ID	flag_ist;
	void (*work_func)(struct work_struct *work);
};

struct tasklet_struct {
	THREAD_HANDLE	task;
	ID	flag_ist;
	union {
		void (*func)(unsigned long data);
		void (*callback)(struct tasklet_struct *t);
	};
	unsigned long data;
};

typedef enum {
	H26XD_PCLK,
	H264D_MCLK,
	H264D_ACLK,
	H265D_MCLK,
	H265D_ACLK,
	PLL24_PARENT,
	PLL0_PARENT,
} h26xd_clk_type_t;

struct clk {
	const char *name;
	unsigned long rate;
	h26xd_clk_type_t type;
	int	rtos_clk_type;	/* GATECLK / M_GATECLK */
	int	cg_en;			/* CG_EN */
	int enable;
};

#ifndef bool
#define bool	int
#endif

#define	__init
#define __exit
#define EFAULT					14
#define EXPORT_SYMBOL(x)
#define iowrite32(val, addr)    (*(volatile unsigned int *)(addr) = (val))
#define ioread32(addr)          (*(volatile unsigned int *)(addr))
#define printk					printf
#define printk_ratelimited		printf

#define atomic_t				unsigned int
#define dma_addr_t				uintptr_t
#define ALIGN(x, mask)			(((x) + (mask)-1) & ~((mask)-1))
#define pr_warn(fmt, ...)		//printf(fmt, ##__VA_ARGS__)
#define panic					printf
#define isb()
#define	__func__				__FUNCTION__
#define __iomem
#define module_init(x)
#define module_exit(x)
#define GFP_KERNEL				0
#define __builtin_expect(x,y)	(x)
#define THIS_MODULE				0
#define H26X_DEV_NAME			"h26xdec"
#define MODULE_AUTHOR(x)
#define MODULE_LICENSE(x)
#define MODULE_VERSION(x)
#define MODULE_PARM_DESC(x,y)
#define IORESOURCE_IO		0x00000100	/* PCI/ISA I/O ports */
#define IORESOURCE_MEM		0x00000200
#define module_param_string(a,b,c,d)
#define module_param(x,y,z)
#define module_param_array(a,b,c,d)
#define atomic_read(x)		*((unsigned int *)(x))
#define atomic_set(x, v)	*((unsigned int *)(x)) = (v)
#define pfn_valid(x)		0
#define jiffies				platform_get_tick()
#define vfree		free
#define vmalloc		malloc
#define ktime_t		unsigned int
#define ktime_get	platform_get_ms
#define ktime_sub(n,o)	((int)(n) - (int)(o))
#define ktime_to_ms(x)	(x/100000)	//have zero. Because we can't print something in ISR
#define memset_io	platform_memset
#define printk_ratelimit()	1
#define ioremap_nocache(x,y)	((uintptr_t *)(x))
#define ioremap(x, y)			((uintptr_t *)(x))
#define iounmap(x)
#define dma_set_coherent_mask(x,y)	(0)
#define platform_get_irq(x, y)	((x)->irq_num)
#define in_interrupt()	0
#define in_irq()		0
#define in_atomic()		0
#define irqs_disabled()	0
#define IS_ERR(x)		((x) == NULL)
#define PAGE_SIZE		0x1000

int of_property_read_u32(const struct device_node *np, const char *propname, unsigned int *out_value);
int platform_driver_register(struct platform_driver *);
void platform_driver_unregister(struct platform_driver *);

unsigned int __builtin_clz(unsigned int);
unsigned long platform_get_ms(void);
unsigned long platform_get_tick(void);
unsigned long jiffies_to_usecs(unsigned long dummy_tick);

//#define LOG_PRINT(X,Y, fmt, ...)	printf(fmt, ##__VA_ARGS__) /* skip X,Y */
//#define printm(X, fmt, ...)		printf(fmt, ##__VA_ARGS__)
void *dma_alloc_coherent(void *dev, int size, dma_addr_t *paddr, int flag);
void dma_free_coherent(void *dev, int size, void *addr_va, dma_addr_t addr_pa);
void *kzalloc(int size, int flag);
void kfree(void *vaddr);
void platform_memset(void *vaddr, int value, int size);
struct resource *platform_get_resource(struct platform_device *, unsigned int, unsigned int);
int schedule_work(struct work_struct *work);
void tasklet_schedule(struct tasklet_struct *t);
void tasklet_kill(struct tasklet_struct *t);
void tasklet_init(struct tasklet_struct *t, void (*func)(unsigned long), unsigned long data);
void __init_work(struct work_struct *work, void (*func)(struct work_struct *work));
#define INIT_WORK(_work, _func)	__init_work(_work, _func)
struct clk *clk_get(void *dev, const char *name);
void clk_put(struct clk *clk);
int clk_prepare_enable(struct clk *clk);
void clk_disable_unprepare(struct clk *clk);
void clk_unprepare(struct clk *clk);
int clk_set_phase(struct clk *clk, int degrees);
int clk_enable(struct clk *clk);
void clk_disable(struct clk *clk);
unsigned long clk_get_rate(struct clk *clk);
int h26xd_proc_init(void);
void h26xd_proc_close(void);
int damnit(char *module);
int platform_rtos_request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev);
int clk_set_parent(struct clk *clk, struct clk *parent_clk);

#endif /* __FREERTOS */

/* put here due to inline in WIN32 */
#include <kwrap/list.h>

#endif /* __PLATFORM_PORT_H__ */