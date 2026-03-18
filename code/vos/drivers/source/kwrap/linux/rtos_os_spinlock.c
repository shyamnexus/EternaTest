/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/stacktrace.h>
#include <asm/stacktrace.h>
#include <asm/ftrace.h>

#define __MODULE__    rtos_spinlock
#define __DBGLVL__    8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__    "*"
#include <kwrap/debug.h>
#include <kwrap/spinlock.h>


#define VOS_SPINLOCK_DEBUG_BACKTRACE    0 //0: disable, 1: enable
#define VOS_SPINLOCK_DEBUG              0 //0: disable, 1: enable
#define VOS_SPINLOCK_DEBUG_MAX_CORE     4 // modify by platform
#define VOS_SPINLOCK_DEBUG_LIST_NUM     64
/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
#define VOS_SPINLOCK_TAG_INITED MAKEFOURCC('V', 'S', 'P', 'N')

#define loc_cpu(flags) raw_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) raw_spin_unlock_irqrestore(&my_lock, flags)

#ifndef sizeof_field
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif

//Note: additional sizeof(u64) for the buf offset
STATIC_ASSERT(sizeof_field(vk_spinlock_t, buf) >= (sizeof(spinlock_t) + sizeof(u64)));
STATIC_ASSERT(sizeof_field(vk_raw_spinlock_t, buf) >= (sizeof(raw_spinlock_t) + sizeof(u64)));

//Note:
//For RT Linux, spinlock is directed to an rt mutex and use atomic 64
//The buf should aligned to 64-bit (8 bytes)
#define GET_ALIGN_BUF(p) ALIGN_CEIL_8((unsigned long)(p)->buf)

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
unsigned int rtos_spinlock_debug_level = NVT_DBG_WRN;
static DEFINE_RAW_SPINLOCK(my_lock);

module_param_named(rtos_spinlock_debug_level, rtos_spinlock_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rtos_spinlock_debug_level, "Debug message level");

#if VOS_SPINLOCK_DEBUG
typedef struct {
	unsigned int next_idx;
	unsigned long caller_ra[VOS_SPINLOCK_DEBUG_LIST_NUM];
} VOS_SPINLOCK_LIST;

static VOS_SPINLOCK_LIST g_vos_lock_list[VOS_SPINLOCK_DEBUG_MAX_CORE] = {0};
static VOS_SPINLOCK_LIST g_vos_unlock_list[VOS_SPINLOCK_DEBUG_MAX_CORE] = {0};
#endif

#if VOS_SPINLOCK_DEBUG_BACKTRACE
typedef struct {
	unsigned int next_idx;
	unsigned long lock[VOS_SPINLOCK_DEBUG_LIST_NUM];
	unsigned long lock_tmp[VOS_SPINLOCK_DEBUG_LIST_NUM];
	unsigned lock_stauts[VOS_SPINLOCK_DEBUG_LIST_NUM];
	unsigned long caller_ra[VOS_SPINLOCK_DEBUG_LIST_NUM];
	unsigned long caller_ra1[VOS_SPINLOCK_DEBUG_LIST_NUM];
	unsigned long caller_ra2[VOS_SPINLOCK_DEBUG_LIST_NUM];
	unsigned long caller_ra3[VOS_SPINLOCK_DEBUG_LIST_NUM];
	unsigned long caller_ra4[VOS_SPINLOCK_DEBUG_LIST_NUM];
	ktime_t time_s[VOS_SPINLOCK_DEBUG_LIST_NUM];
	ktime_t time_e[VOS_SPINLOCK_DEBUG_LIST_NUM];
	s64 time_diff[VOS_SPINLOCK_DEBUG_LIST_NUM];
} VOS_SPINLOCK_LIST;
static VOS_SPINLOCK_LIST g_vos_spinlock_list[VOS_SPINLOCK_DEBUG_MAX_CORE] = {0};
#define LOCKED 1
#define RELEASED 2
#endif

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
#if VOS_SPINLOCK_DEBUG_BACKTRACE
static inline void 	_vk_spin_add_lockbacktrace_list(vk_spinlock_t *p_voslock)
{
	unsigned long my_flags = 0;
	unsigned int cpu_id;
	VOS_SPINLOCK_LIST *p_list;
	int i;
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	loc_cpu(my_flags);

	cpu_id = smp_processor_id();
	if (cpu_id > VOS_SPINLOCK_DEBUG_MAX_CORE - 1) {
		DBG_ERR("Exceed max core 0x%x\r\n", VOS_SPINLOCK_DEBUG_MAX_CORE);
		unl_cpu(my_flags);
		return;
	}

	p_list = &g_vos_spinlock_list[cpu_id];

	p_list->next_idx++;
	if (p_list->next_idx >= VOS_SPINLOCK_DEBUG_LIST_NUM) {
		p_list->next_idx = 0;
	}

	for( i = 0; i < VOS_SPINLOCK_DEBUG_LIST_NUM; i++ ) {
		if( p_list->lock[i] == 0 ) {
			p_list->caller_ra[i] = 0;
			p_list->caller_ra1[i] = 0;
			p_list->caller_ra2[i] = 0;
			p_list->caller_ra3[i] = 0;
			p_list->caller_ra4[i] = 0;

			p_list->caller_ra[i] = (unsigned long)return_address(0);
			p_list->caller_ra1[i] = (unsigned long)return_address(1);
			p_list->caller_ra2[i] = (unsigned long)return_address(2);
			p_list->caller_ra3[i] = (unsigned long)return_address(3);
			p_list->caller_ra4[i] = (unsigned long)return_address(4);

			p_list->lock[i] = (unsigned long )p_spinlock;
			p_list->time_s[i] = ktime_get();
			p_list->time_e[i] = 0;
			p_list->time_diff[i] = 0;
			p_list->lock_stauts[i] = 1;
			break;
		}
	}

	unl_cpu(my_flags);
}

static inline void 	_vk_spin_add_unlockbacktrace_list(vk_spinlock_t *p_voslock)
{
	unsigned long my_flags = 0;
	unsigned int cpu_id;
	VOS_SPINLOCK_LIST *p_list;
	int i, j;
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	loc_cpu(my_flags);

	cpu_id = smp_processor_id();
	if (cpu_id > VOS_SPINLOCK_DEBUG_MAX_CORE - 1) {
		DBG_ERR("Exceed max core 0x%x\r\n", VOS_SPINLOCK_DEBUG_MAX_CORE);
		unl_cpu(my_flags);
		return;
	}

	for( i = 0; j < VOS_SPINLOCK_DEBUG_MAX_CORE; j++ ) {
		p_list = &g_vos_spinlock_list[j];

		for( i = 0; i < VOS_SPINLOCK_DEBUG_LIST_NUM; i++ ) {
			if(p_list->lock[i] == (unsigned long)p_spinlock) {
				p_list->lock_tmp[i] = p_list->lock[i];
				p_list->lock[i] = 0;
				p_list->time_e[i] = ktime_get();
				p_list->time_diff[i] = ktime_to_ns(ktime_sub(p_list->time_e[i], p_list->time_s[i]));
				p_list->lock_stauts[i] = 2;
				break;
			}
		}
	}

	unl_cpu(my_flags);
}
#endif //#if SPIN_LOCK_DEBUG

void vk_spin_dumpbacktrace_list(int argc, char** argv)
{
#if !VOS_SPINLOCK_DEBUG_BACKTRACE
	DBG_DUMP("VOS_SPINLOCK_DEBUG_BACKTRACE is disable\r\n");
	return;
#else
	unsigned long my_flags = 0;
	unsigned int cpu_id;
	unsigned int list_idx;
	unsigned int lock_status;

	if(argc != 2) {
		DBG_DUMP("please type => echo spinlock cpuid(0-3) lockstatus(2 for already unlocked, 1 for locked) > cmd\r\n");
		return;
	}

	loc_cpu(my_flags);

	cpu_id = simple_strtol(argv[0], NULL, 10);
	lock_status = simple_strtol(argv[1], NULL, 10);

	if(lock_status > 2 || lock_status == 0)
		lock_status = RELEASED;

	if(cpu_id >= VOS_SPINLOCK_DEBUG_MAX_CORE)
		cpu_id = 0;

	DBG_DUMP("cpu%d loc next_idx 0x%X, %s list\r\n", cpu_id, g_vos_spinlock_list[cpu_id].next_idx,
		(lock_status == RELEASED)? "released":"unreleased");

	for (list_idx = 0; list_idx < VOS_SPINLOCK_DEBUG_LIST_NUM; list_idx++) {
		if(((void *)g_vos_spinlock_list[cpu_id].caller_ra[list_idx] == 0) ||
			(g_vos_spinlock_list[cpu_id].lock_stauts[list_idx] != lock_status) ) {
			continue;
		}

		DBG_DUMP("cpu%d[%03d] spinlock_addr=0x%lX starttime=%lld, time(wait+hold)=%lldns, %s, \ncalltrace: %pS\n => %pS\n => %pS\n => %pS\n => %pS\r\n", cpu_id, list_idx,
			g_vos_spinlock_list[cpu_id].lock_tmp[list_idx],
			g_vos_spinlock_list[cpu_id].time_s[list_idx],
			g_vos_spinlock_list[cpu_id].time_diff[list_idx],
			(g_vos_spinlock_list[cpu_id].lock_stauts[list_idx] == LOCKED)? "not release yet": "released",
			(void *)g_vos_spinlock_list[cpu_id].caller_ra[list_idx],
			(void *)g_vos_spinlock_list[cpu_id].caller_ra1[list_idx],
			(void *)g_vos_spinlock_list[cpu_id].caller_ra2[list_idx],
			(void *)g_vos_spinlock_list[cpu_id].caller_ra3[list_idx],
			(void *)g_vos_spinlock_list[cpu_id].caller_ra4[list_idx]);
	}
	unl_cpu(my_flags);
#endif
}
EXPORT_SYMBOL(vk_spin_dumpbacktrace_list);


#if VOS_SPINLOCK_DEBUG
static inline void _vk_spin_add_lock_list(void)
{
	unsigned long my_flags = 0;
	unsigned int cpu_id;
	VOS_SPINLOCK_LIST *p_list;
	static int print_once = 0;

	loc_cpu(my_flags);

	if (!print_once) {
		print_once = 1;
		DBG_DUMP("[VOS_SPIN] max_core %d list_num %d\r\n", VOS_SPINLOCK_DEBUG_MAX_CORE, VOS_SPINLOCK_DEBUG_LIST_NUM);
		for (cpu_id = 0; cpu_id < VOS_SPINLOCK_DEBUG_MAX_CORE; cpu_id++) {
			DBG_DUMP("[VOS_SPIN] cpu%d loc next_idx = 0x%lX\r\n", cpu_id, (unsigned long)&g_vos_lock_list[cpu_id].next_idx);
			DBG_DUMP("[VOS_SPIN] cpu%d loc caller_ra[0] = 0x%lX\r\n", cpu_id, (unsigned long)&g_vos_lock_list[cpu_id].caller_ra[0]);
			DBG_DUMP("[VOS_SPIN] cpu%d unl next_idx = 0x%lX\r\n", cpu_id, (unsigned long)&g_vos_unlock_list[cpu_id].next_idx);
			DBG_DUMP("[VOS_SPIN] cpu%d unl caller_ra[0] = 0x%lX\r\n", cpu_id, (unsigned long)&g_vos_unlock_list[cpu_id].caller_ra[0]);
		}
	}

	cpu_id = smp_processor_id();
	if (cpu_id > VOS_SPINLOCK_DEBUG_MAX_CORE - 1) {
		DBG_ERR("Exceed max core 0x%x\r\n", VOS_SPINLOCK_DEBUG_MAX_CORE);
		unl_cpu(my_flags);
		return;
	}

	p_list = &g_vos_lock_list[cpu_id];
	p_list->caller_ra[p_list->next_idx] = (unsigned long)__builtin_return_address(0);

	p_list->next_idx++;
	if (p_list->next_idx >= VOS_SPINLOCK_DEBUG_LIST_NUM) {
		p_list->next_idx = 0;
	}

	unl_cpu(my_flags);
}

static inline void _vk_spin_add_unlock_list(void)
{
	unsigned long my_flags = 0;
	unsigned int cpu_id;
	VOS_SPINLOCK_LIST *p_list;

	loc_cpu(my_flags);

	cpu_id = smp_processor_id();
	if (cpu_id > VOS_SPINLOCK_DEBUG_MAX_CORE - 1) {
		DBG_ERR("Exceed max core 0x%x\r\n", VOS_SPINLOCK_DEBUG_MAX_CORE);
		unl_cpu(my_flags);
		return;
	}

	p_list = &g_vos_unlock_list[cpu_id];
	p_list->caller_ra[p_list->next_idx] = (unsigned long)__builtin_return_address(0);

	p_list->next_idx++;
	if (p_list->next_idx >= VOS_SPINLOCK_DEBUG_LIST_NUM) {
		p_list->next_idx = 0;
	}

	unl_cpu(my_flags);
}
#endif //#if SPIN_LOCK_DEBUG

void vk_spin_dump_list(void)
{
#if !VOS_SPINLOCK_DEBUG
	DBG_DUMP("VOS_SPINLOCK_DEBUG is disable\r\n");
	return;
#else
	unsigned long my_flags = 0;
	unsigned int cpu_id;
	unsigned int list_idx;

	loc_cpu(my_flags);

	for (cpu_id = 0; cpu_id < VOS_SPINLOCK_DEBUG_MAX_CORE; cpu_id++) {
		DBG_DUMP("cpu%d loc next_idx 0x%X\r\n", cpu_id, g_vos_lock_list[cpu_id].next_idx);
		for (list_idx = 0; list_idx < VOS_SPINLOCK_DEBUG_LIST_NUM; list_idx++) {
			DBG_DUMP("cpu%d[%03d] loc_ra 0x%lX\r\n", cpu_id, list_idx,
				g_vos_lock_list[cpu_id].caller_ra[list_idx]);
		}

		DBG_DUMP("cpu%d unl next_idx 0x%X\r\n", cpu_id, g_vos_unlock_list[cpu_id].next_idx);
		for (list_idx = 0; list_idx < VOS_SPINLOCK_DEBUG_LIST_NUM; list_idx++) {
			DBG_DUMP("cpu%d[%03d] unl_ra 0x%lX\r\n", cpu_id, list_idx,
				g_vos_unlock_list[cpu_id].caller_ra[list_idx]);
		}
	}

	unl_cpu(my_flags);
#endif
}
EXPORT_SYMBOL(vk_spin_dump_list);

/*----------------------------------------------------------------------------*/
/* Normal type lock                                                           */
/*----------------------------------------------------------------------------*/
void vk_spin_lock_init(vk_spinlock_t *p_voslock)
{
	unsigned long my_flags = 0;
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);

	loc_cpu(my_flags);

	//always init spinlock without checking init_tag to prevent dirty data
	//coverity[side_effect_free]: spin_lock_init is kernel API, do not change
	spin_lock_init(p_spinlock);
	p_voslock->init_tag = VOS_SPINLOCK_TAG_INITED;

	unl_cpu(my_flags);
}

static void _vk_spin_lock_init_check_tag(vk_spinlock_t *p_voslock)
{
	unsigned long my_flags = 0;

	loc_cpu(my_flags);

	//after inter lock, check the tag again to make sure not inited yet
	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);
		// coverity[side_effect_free]: spin_lock_init is kernel API, do not change
		spin_lock_init(p_spinlock);
		p_voslock->init_tag = VOS_SPINLOCK_TAG_INITED;
	}

	unl_cpu(my_flags);
}

void vk_spin_lock(vk_spinlock_t *p_voslock)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_spin_lock_init_check_tag(p_voslock);
	}

	spin_lock(p_spinlock);
}

void vk_spin_lock_bh(vk_spinlock_t *p_voslock)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_spin_lock_init_check_tag(p_voslock);
	}

	spin_lock_bh(p_spinlock);
}

void vk_spin_lock_irq(vk_spinlock_t *p_voslock)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_spin_lock_init_check_tag(p_voslock);
	}

	spin_lock_irq(p_spinlock);
}

unsigned long _vk_spin_lock_irqsave(vk_spinlock_t *p_voslock)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);
	unsigned long flags = 0;

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_spin_lock_init_check_tag(p_voslock);
	}

#if VOS_SPINLOCK_DEBUG
	_vk_spin_add_lock_list();
#endif

#if VOS_SPINLOCK_DEBUG_BACKTRACE
	_vk_spin_add_lockbacktrace_list(p_voslock);
#endif
	spin_lock_irqsave(p_spinlock, flags);

	return flags;
}

unsigned long _vk_spin_lock_irqsave_nested(vk_spinlock_t *p_voslock, int subclass)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);
	unsigned long flags = 0;

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_spin_lock_init_check_tag(p_voslock);
	}

#if VOS_SPINLOCK_DEBUG
	_vk_spin_add_lock_list();
#endif

#if VOS_SPINLOCK_DEBUG_BACKTRACE
	_vk_spin_add_lockbacktrace_list(p_voslock);
#endif

	spin_lock_irqsave_nested(p_spinlock, flags, subclass);

	return flags;
}

void vk_spin_unlock(vk_spinlock_t *p_voslock)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)p_voslock);
		return;
	}

	spin_unlock(p_spinlock);
}

void vk_spin_unlock_bh(vk_spinlock_t *p_voslock)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)p_voslock);
		return;
	}

	spin_unlock_bh(p_spinlock);
}

void vk_spin_unlock_irq(vk_spinlock_t *p_voslock)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)p_voslock);
		return;
	}

	spin_unlock_irq(p_spinlock);
}

void vk_spin_unlock_irqrestore(vk_spinlock_t *p_voslock, unsigned long flags)
{
	spinlock_t *p_spinlock = (spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)p_voslock);
		return;
	}

#if VOS_SPINLOCK_DEBUG
	_vk_spin_add_unlock_list();
#endif

#if VOS_SPINLOCK_DEBUG_BACKTRACE
	_vk_spin_add_unlockbacktrace_list(p_voslock);
#endif

	spin_unlock_irqrestore(p_spinlock, flags);
}

EXPORT_SYMBOL(vk_spin_lock_init);
EXPORT_SYMBOL(vk_spin_lock);
EXPORT_SYMBOL(vk_spin_lock_bh);
EXPORT_SYMBOL(vk_spin_lock_irq);
EXPORT_SYMBOL(_vk_spin_lock_irqsave);
EXPORT_SYMBOL(_vk_spin_lock_irqsave_nested);
EXPORT_SYMBOL(vk_spin_unlock);
EXPORT_SYMBOL(vk_spin_unlock_bh);
EXPORT_SYMBOL(vk_spin_unlock_irq);
EXPORT_SYMBOL(vk_spin_unlock_irqrestore);

/*----------------------------------------------------------------------------*/
/* Raw type lock                                                              */
/*----------------------------------------------------------------------------*/
void vk_raw_spin_lock_init(vk_raw_spinlock_t *p_voslock)
{
	unsigned long my_flags = 0;
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	loc_cpu(my_flags);

	//always init spinlock without checking init_tag to prevent dirty data
	raw_spin_lock_init(p_spinlock);
	p_voslock->init_tag = VOS_SPINLOCK_TAG_INITED;

	unl_cpu(my_flags);
}

static void _vk_raw_spin_lock_init_check_tag(vk_raw_spinlock_t *p_voslock)
{
	unsigned long my_flags = 0;

	loc_cpu(my_flags);

	//after inter lock, check the tag again to make sure not inited yet
	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);
		raw_spin_lock_init(p_spinlock);
		p_voslock->init_tag = VOS_SPINLOCK_TAG_INITED;
	}

	unl_cpu(my_flags);
}

void vk_raw_spin_lock(vk_raw_spinlock_t *p_voslock)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_raw_spin_lock_init_check_tag(p_voslock);
	}

	raw_spin_lock(p_spinlock);
}

void vk_raw_spin_lock_bh(vk_raw_spinlock_t *p_voslock)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_raw_spin_lock_init_check_tag(p_voslock);
	}

	raw_spin_lock_bh(p_spinlock);
}

void vk_raw_spin_lock_irq(vk_raw_spinlock_t *p_voslock)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_raw_spin_lock_init_check_tag(p_voslock);
	}

	raw_spin_lock_irq(p_spinlock);
}

unsigned long _vk_raw_spin_lock_irqsave(vk_raw_spinlock_t *p_voslock)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);
	unsigned long flags = 0;

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_raw_spin_lock_init_check_tag(p_voslock);
	}

#if VOS_SPINLOCK_DEBUG
	_vk_spin_add_lock_list();
#endif

#if VOS_SPINLOCK_DEBUG_BACKTRACE
	_vk_spin_add_lockbacktrace_list(p_voslock);
#endif

	raw_spin_lock_irqsave(p_spinlock, flags);

	return flags;
}

unsigned long _vk_raw_spin_lock_irqsave_nested(vk_raw_spinlock_t *p_voslock, int subclass)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);
	unsigned long flags = 0;

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		_vk_raw_spin_lock_init_check_tag(p_voslock);
	}

#if VOS_SPINLOCK_DEBUG
	_vk_spin_add_lock_list();
#endif

#if VOS_SPINLOCK_DEBUG_BACKTRACE
	_vk_spin_add_lockbacktrace_list(p_voslock);
#endif

	raw_spin_lock_irqsave_nested(p_spinlock, flags, subclass);

	return flags;
}

void vk_raw_spin_unlock(vk_raw_spinlock_t *p_voslock)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)p_voslock);
		return;
	}

	raw_spin_unlock(p_spinlock);
}

void vk_raw_spin_unlock_bh(vk_raw_spinlock_t *p_voslock)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)p_voslock);
		return;
	}

	raw_spin_unlock_bh(p_spinlock);
}

void vk_raw_spin_unlock_irq(vk_raw_spinlock_t *p_voslock)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)p_voslock);
		return;
	}

	raw_spin_unlock_irq(p_spinlock);
}

void vk_raw_spin_unlock_irqrestore(vk_raw_spinlock_t *p_voslock, unsigned long flags)
{
	raw_spinlock_t *p_spinlock = (raw_spinlock_t *)GET_ALIGN_BUF(p_voslock);

	if (VOS_SPINLOCK_TAG_INITED != p_voslock->init_tag) {
		DBG_ERR("0x%lX not inited\r\n", (unsigned long)p_voslock);
		return;
	}

#if VOS_SPINLOCK_DEBUG
	_vk_spin_add_unlock_list();
#endif

#if VOS_SPINLOCK_DEBUG_BACKTRACE
	_vk_spin_add_unlockbacktrace_list(p_voslock);
#endif

	raw_spin_unlock_irqrestore(p_spinlock, flags);
}

EXPORT_SYMBOL(vk_raw_spin_lock_init);
EXPORT_SYMBOL(vk_raw_spin_lock);
EXPORT_SYMBOL(vk_raw_spin_lock_bh);
EXPORT_SYMBOL(vk_raw_spin_lock_irq);
EXPORT_SYMBOL(_vk_raw_spin_lock_irqsave);
EXPORT_SYMBOL(_vk_raw_spin_lock_irqsave_nested);
EXPORT_SYMBOL(vk_raw_spin_unlock);
EXPORT_SYMBOL(vk_raw_spin_unlock_bh);
EXPORT_SYMBOL(vk_raw_spin_unlock_irq);
EXPORT_SYMBOL(vk_raw_spin_unlock_irqrestore);