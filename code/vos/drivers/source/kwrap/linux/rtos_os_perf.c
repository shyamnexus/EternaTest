/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/module.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <plat/hardware.h> //for NVT_TIMER_BASE_VIRT

#define __MODULE__    rtos_perf
#define __DBGLVL__    8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__    "*"
#include <kwrap/debug.h>
#include <kwrap/cpu.h>
#include <kwrap/dev.h>
#include <kwrap/perf.h>
#include <kwrap/spinlock.h>
#include "vos_ioctl.h"

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
#define RTOS_PERF_INITED_TAG    MAKEFOURCC('R', 'P', 'E', 'R') ///< a key value

#ifndef NVT_TIMER_OFS_TM0_CNT
#if defined(_BSP_NA51055_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NA51000_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NA51068_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NA51089_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NA51090_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NA51102_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NA51103_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NS02201_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NS02301_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NS02302_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#elif defined(_BSP_NS02401_)
#define NVT_TIMER_OFS_TM0_CNT   0x108
#else
#error NVT_TIMER_OFS_TM0_CNT not defined
#endif
#endif

#ifndef NVT_TM0_BASE_PHYS
#define NVT_TM0_BASE_PHYS   (NVT_TIMER_BASE_PHYS + NVT_TIMER_OFS_TM0_CNT)
#endif

#define VOS_PERF_LIST_NUM       32
#define VOS_PERF_LIST_NAME_SIZE 32

#define VOS_STRCPY(dst, src, dst_size) do { \
	strncpy(dst, src, (dst_size)-1); \
	dst[(dst_size)-1] = '\0'; \
} while(0)

#define loc_cpu(flags) raw_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) raw_spin_unlock_irqrestore(&my_lock, flags)

typedef struct _VOS_PERF_LIST_ITEM {
    CHAR name[VOS_PERF_LIST_NAME_SIZE];
    VOS_TICK tick;
    UINT32 line_no;
    UINT32 cus_val;
    UINT32 diff_time; //diff time to the first same name
    INT column;
} VOS_PERF_LIST_ITEM;

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
unsigned int rtos_perf_debug_level = NVT_DBG_WRN;
static DEFINE_RAW_SPINLOCK(my_lock);

static VOS_PERF_LIST_ITEM g_perf_list[VOS_PERF_LIST_NUM] = {0};
static UINT32 g_perf_list_idx = 0;
static UINT32 g_perf_list_skip_count = 0;
static void __iomem *g_tm0_va = NULL;

module_param_named(rtos_perf_debug_level, rtos_perf_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rtos_perf_debug_level, "Debug message level");

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void vos_perf_init(void *param)
{
	if (NULL == g_tm0_va) {
		g_tm0_va = ioremap(NVT_TM0_BASE_PHYS, 4);
	}
}

void vos_perf_exit(void)
{
	if (g_tm0_va) {
		iounmap(g_tm0_va);
		g_tm0_va = NULL;
	}
}

static VOS_TICK _vos_get_tm0_cnt(void)
{
	if (unlikely(NULL == g_tm0_va)) {
		return 0;
	}

	return (VOS_TICK)readl(g_tm0_va);
}

void vos_perf_mark(VOS_TICK *p_tick)
{
	if (NULL == p_tick) {
		DBG_ERR("p_tick is NULL\r\n");
		return;
	}

	*p_tick = _vos_get_tm0_cnt();
}

VOS_TICK vos_perf_duration(VOS_TICK t_begin, VOS_TICK t_end)
{
	return (t_end - t_begin);
}

void vos_perf_list_reset(void)
{
	unsigned long flags;

	loc_cpu(flags);

	g_perf_list_idx = 0;
	g_perf_list_skip_count = 0;

	unl_cpu(flags);
}

void vos_perf_list_mark(const CHAR *p_name, UINT32 line_no, UINT32 cus_val)
{
	unsigned long flags;

	loc_cpu(flags);

	if (g_perf_list_idx < VOS_PERF_LIST_NUM) {
		VOS_STRCPY(g_perf_list[g_perf_list_idx].name, p_name, sizeof(g_perf_list[g_perf_list_idx].name));
		g_perf_list[g_perf_list_idx].tick = _vos_get_tm0_cnt();
		g_perf_list[g_perf_list_idx].line_no = line_no;
		g_perf_list[g_perf_list_idx].cus_val = cus_val;
		g_perf_list_idx++;
	} else {
		g_perf_list_skip_count++;
	}

	unl_cpu(flags);
}

static void vos_perf_list_dump_by_idx(void)
{
	INT list_idx;
	INT max_idx;

	DBG_DUMP("\r\n===== %s =====\r\n", __func__);

	max_idx = g_perf_list_idx;

	for(list_idx = 0; list_idx < max_idx; list_idx++) {
		DBG_DUMP("[%02d] %s() ln %d ts %d df %d cus %d\r\n",
			list_idx,
			g_perf_list[list_idx].name,
			(UINT)g_perf_list[list_idx].line_no,
			(UINT)g_perf_list[list_idx].tick,
			(list_idx < 1) ? 0 : (UINT)vos_perf_duration(g_perf_list[list_idx-1].tick, g_perf_list[list_idx].tick),
			(UINT)g_perf_list[list_idx].cus_val);
    }
}

static void vos_perf_list_dump_by_name(void)
{
	INT item_idx, item_idx2;
	INT column_cur, column_max;
	INT row_idx;
	INT max_idx;

	pr_cont("\r\n===== %s =====\r\n", __func__);

	max_idx = g_perf_list_idx;

	//reset the column to recount the newest column
	for (item_idx = 0; item_idx < max_idx; item_idx++) {
		g_perf_list[item_idx].column = 0;
	}

	//setup the column of each item
	column_cur = 0;
	for (item_idx = 0; item_idx < max_idx; item_idx++) {
		if (0 != g_perf_list[item_idx].column) {
			continue;
		}
		column_cur++;
		g_perf_list[item_idx].column = column_cur;
		g_perf_list[item_idx].diff_time = 0;

		for (item_idx2 = item_idx + 1; item_idx2 < max_idx; item_idx2++) {
			if (!strcmp(g_perf_list[item_idx2].name, g_perf_list[item_idx].name)) {
				g_perf_list[item_idx2].column = column_cur;
				g_perf_list[item_idx2].diff_time = g_perf_list[item_idx2].tick - g_perf_list[item_idx].tick;
			}
		}
	}
	column_max = column_cur;

#if 0 //for debug
	DBG_DUMP("max_idx %d, column_max %d\r\n", max_idx, column_max);
	for (item_idx = 0; item_idx < max_idx; item_idx++) {
		DBG_DUMP("item[%d], column %d, df %u, %u, %s\r\n",
			item_idx,
			g_perf_list[item_idx].column,
			g_perf_list[item_idx].tick,
			g_perf_list[item_idx].diff_time,
			g_perf_list[item_idx].name);
	}
#endif

	//print column name
	pr_cont("Name");
	for (column_cur = 1; column_cur <= column_max; column_cur++) {
		for (item_idx = 0; item_idx < max_idx; item_idx++) {
			if (column_cur == g_perf_list[item_idx].column) {
				pr_cont(" %10s", g_perf_list[item_idx].name);
				break;
			}
		}
	}
	pr_cont("\n");

	//print item data by column
	for (row_idx = 0; row_idx < max_idx; row_idx++) {
		pr_cont("[%02d]", row_idx);
		for (column_cur = 1; column_cur <= column_max; column_cur++) {
			if (column_cur == g_perf_list[row_idx].column) {
				pr_cont(" %10u", g_perf_list[row_idx].tick);
			} else {
				pr_cont(" %10c", '-');
			}
		}
		pr_cont("\n");
	}

	//print diff time
	pr_cont("-----------------------------------------------\n");
	pr_cont("Diff");
	for (column_cur = 1; column_cur <= column_max; column_cur++) {
		for (item_idx = (max_idx-1); item_idx >= 0; item_idx--) {
			if (column_cur == g_perf_list[item_idx].column) {
				pr_cont(" %10u", g_perf_list[item_idx].diff_time);
				break;
			}
		}
	}
	pr_cont("\n");
}

void vos_perf_list_dump(void)
{
	unsigned long flags;

	loc_cpu(flags);

	vos_perf_list_dump_by_idx();
	vos_perf_list_dump_by_name();

	if (g_perf_list_skip_count) {
		DBG_WRN("skip count %d\r\n", (UINT)g_perf_list_skip_count);
	}

	unl_cpu(flags);
}

int _IOFUNC_PERF_IOCMD_MARK(unsigned long arg)
{
	VOS_PERF_IOARG ioarg = {0};
	VOS_TICK tmp_tick;

	//-------------------
	//skip copy_from_user
	//-------------------

	vos_perf_mark(&tmp_tick);
	ioarg.tick = tmp_tick;

	if (copy_to_user((void *)arg, (void *)&ioarg, sizeof(VOS_PERF_IOARG))) {
		DBG_ERR("copy_to_user failed\n");
		return -EFAULT;
	}

	return 0;
}

int _IOFUNC_PERF_IOCMD_LIST_MARK(unsigned long arg)
{
	VOS_PERF_IOARG ioarg = {0};

	if (copy_from_user((void *)&ioarg, (void *)arg, sizeof(VOS_PERF_IOARG))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	vos_perf_list_mark(ioarg.name, ioarg.line_no, ioarg.cus_val);

	return 0;
}

int _IOFUNC_PERF_IOCMD_LIST_DUMP(unsigned long arg)
{
	vos_perf_list_dump();
	return 0;
}

int _IOFUNC_PERF_IOCMD_LIST_RESET(unsigned long arg)
{
	vos_perf_list_reset();
	return 0;
}

EXPORT_SYMBOL(vos_perf_mark);
EXPORT_SYMBOL(vos_perf_duration);
EXPORT_SYMBOL(vos_perf_list_reset);
EXPORT_SYMBOL(vos_perf_list_mark);
EXPORT_SYMBOL(vos_perf_list_dump);
