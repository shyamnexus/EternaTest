#include "hwcopy_platform.h"
#if defined __UITRON || defined __ECOS


static const ID hwcopy_flgId[] = {FLG_ID_HWCOPY};
static const FLGPTN hwcopy_flag[] = {FLGPTN_HWCOPY};
static const DRV_SEM hwcopy_sem[] = {SEMID_HWCOPY};
static const CG_EN hwcopy_clk_en[] = {HWCPY_CLK};
static const DRV_INT_NUM hwcopy_int_en[] = {DRV_INT_COPY};

#else // __LINUX || __FREERTOS


static ID v_flg_id[HWCOPY_ID_MAX];

static SEM_HANDLE *v_sem[HWCOPY_ID_MAX];

static SEM_HANDLE *v_sem_open_close[HWCOPY_ID_MAX];

static SEM_HANDLE SEMID_HWCOPY_OPEN_CLOSE;

static ID	FLG_ID_HWCOPY;

// semaphore & interrupt flag id
static SEM_HANDLE SEMID_HWCOPY;

static vk_spinlock_t hwcopy_spinlock;

#if defined __LINUX

void __iomem *IOADDR_HWCP_REG_BASE[HWCOPY_ID_MAX];
static struct clk *hwcopy_clk[HWCOPY_ID_MAX];
static BOOL hwcopy_clk_en[HWCOPY_ID_MAX];
static struct tasklet_struct * v_p_timer_tasklet[HWCOPY_ID_MAX];
//static spinlock_t v_hwcopy_spin_locks[HWCOPY_ID_1+1];


#elif defined __FREERTOS

#define ARRAY_SIZE(a)        ((sizeof(a) / sizeof(a[0])))
unsigned int hwcopy_debug_level = 2;
#endif
#endif

#define HWCOPY_REQ_POLL_SIZE	16
static HWCOPY_REQ_LIST_NODE v_req_pool[HWCOPY_ID_1+1][HWCOPY_REQ_POLL_SIZE];
static UINT32 v_req_front[HWCOPY_ID_1+1];
static UINT32 v_req_tail[HWCOPY_ID_1+1];

KDRV_CALLBACK_FUNC v_hwcopy_callback[HWCOPY_ID_1+1];

/**
    @addtogroup mIDrvIPP_Hwcopy
*/
//@{

/**
    Get hwcopy controller register.

    Get hwcopy controller register.
    @param[in] id           hwcopy controller ID
    @param[in] offset       register offset in hwcopy controller (word alignment)
    @return register value
        - @b E_ID: invalidate id number
*/
REGVALUE hwcopy_get_reg(HWCOPY_ID id, UINT32 offset)
{
#if defined __LINUX
	if (id < HWCOPY_ID_MAX) {
        DBG_USER("get reg id = 0x%d, offset = 0x%x\r\n",id,offset);
		return HWCOPY_GETREG(id,offset);
	} else {
		return E_ID;
	}
#elif defined __FREERTOS
	if (id == HWCOPY_ID_0) {
		return HWCOPY_GETREG(offset);
	} else {
		return E_ID;
	}
#endif
}

#if defined __UITRON || defined __ECOS
#else
#if defined __LINUX
#define TYPEVAFLUSH
#elif defined __FREERTOS

static irqreturn_t hwcopy_platform_isr(int irq, void *devid)
{
	copy_isr(HWCOPY_ID_0);
	return IRQ_HANDLED;
}

#endif
#endif

void hwcopy_platform_clk_enable(HWCOPY_ID id)
{
#if defined __UITRON || defined __ECOS
	pll_enableClock(hwcopy_clk_en[id]);
#else
#if defined __LINUX
	if (!hwcopy_clk_en[id]) {
		if (hwcopy_clk[id]) {
			clk_enable(hwcopy_clk[id]);
			hwcopy_clk_en[id] = ENABLE;
		}
	} else {
		DBG_WRN("hwcopy clock already enable\r\n");
	}

#elif defined __FREERTOS
	pll_enable_clock(HWCOPY_CLK);
#endif
#endif
}

void hwcopy_platform_clk_disable(HWCOPY_ID id)
{
#if defined __UITRON || defined __ECOS
	pll_disableClock(hwcopy_clk_en[id]);
#else
#if defined __LINUX
	if (hwcopy_clk_en[id]) {
		if (hwcopy_clk[id]) {
			clk_disable(hwcopy_clk[id]);
			hwcopy_clk_en[id] = DISABLE;
		}
	} else {
		DBG_WRN("remote clock must be enable before disable\r\n");
	}
#elif defined __FREERTOS
	pll_disable_clock(HWCOPY_CLK);
#endif
#endif
}


void hwcopy_platform_flg_clear(HWCOPY_ID id, FLGPTN flg)
{
	clr_flg(v_flg_id[id], flg);
}

void hwcopy_platform_flg_set(HWCOPY_ID id, FLGPTN flg)
{
	 iset_flg(v_flg_id[id], flg);
}

void hwcopy_platform_flg_wait(HWCOPY_ID id, FLGPTN flg)
{
	FLGPTN              ptn;

//	printk("%s: flg id %d\r\n", __func__, v_flg_id[id]);
	wai_flg(&ptn, v_flg_id[id], flg, TWF_ORW | TWF_CLR);
//	printk("%s: done\r\n", __func__);
}

ER hwcopy_platform_sem_wait(HWCOPY_ID id)
{
	if (id >= HWCOPY_ID_MAX)
		return E_NOSPT;

#if defined __UITRON || defined __ECOS
	return wai_sem(v_sem[id]);
#else
	#if defined __FREERTOS
	return vos_sem_wait(*v_sem[id]);
	#elif defined __LINUX
	return SEM_WAIT(*v_sem[id]);
	#endif
#endif
}

ER hwcopy_platform_oc_sem_wait(HWCOPY_ID id)
{
	if (id >= HWCOPY_ID_MAX)
		return E_NOSPT;

#if defined __UITRON || defined __ECOS
	return wai_sem(v_sem[id]);
#else

#if defined __FREERTOS

	return vos_sem_wait(*v_sem_open_close[id]);

#elif defined __LINUX

	return SEM_WAIT(*v_sem_open_close[id]);

#endif
#endif
}

ER hwcopy_platform_sem_signal(HWCOPY_ID id)
{
	if (id >= HWCOPY_ID_MAX)
		return E_NOSPT;

#if defined __UITRON || defined __ECOS
	return sig_sem(v_sem[id]);
#else
//	printk("%s: id %d, sem 0x%p\r\n", __func__, id, v_sem[id]);
#if defined __FREERTOS

vos_sem_sig(*v_sem[id]);
return E_OK;

#elif defined __LINUX

SEM_SIGNAL(*v_sem[id]);
return E_OK;

#endif
#endif
}

ER hwcopy_platform_oc_sem_signal(HWCOPY_ID id)
{
	if (id >= HWCOPY_ID_MAX)
		return E_NOSPT;

#if defined __UITRON || defined __ECOS
	return sig_sem(v_sem[id]);
#else
//	printk("%s: id %d, sem 0x%p\r\n", __func__, id, v_sem[id]);
#if defined __FREERTOS

	vos_sem_sig(*v_sem_open_close[id]);
	return E_OK;

#elif defined __LINUX

	SEM_SIGNAL(*v_sem_open_close[id]);
	return E_OK;

#endif
#endif
}

ULONG hwcopy_platform_spin_lock(HWCOPY_ID id)
{
#if defined __UITRON || defined __ECOS
	loc_cpu();
#else
	unsigned long flags;
	vk_spin_lock_irqsave(&hwcopy_spinlock, flags);
	return flags;
#endif
}

void hwcopy_platform_spin_unlock(HWCOPY_ID id, ULONG flags)
{
#if defined __UITRON || defined __ECOS
	unl_cpu();
#else
	vk_spin_unlock_irqrestore(&hwcopy_spinlock, flags);
#endif
}

void hwcopy_platform_sram_enable(HWCOPY_ID id)
{
#if defined __UITRON || defined __ECOS
	pll_disableSramShutDown(HWCOPY_RSTN);
#else
#endif
}

void hwcopy_platform_int_enable(HWCOPY_ID id)
{
#if defined __UITRON || defined __ECOS
	drv_enableInt(v_inten[id]);
#elif defined(__FREERTOS)
    request_irq(INT_ID_HWCP, hwcopy_platform_isr, IRQF_TRIGGER_HIGH, "hwcopy", 0);
#endif
}

void hwcopy_platform_int_disable(HWCOPY_ID id)
{
#if defined __UITRON || defined __ECOS
	drv_disableInt(v_inten[id]);
#else
#endif
}

UINT32 hwcopy_platform_dma_is_cacheable(uintptr_t addr)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	return dma_isCacheAddr(addr);
#else
	return 1;
#endif
}

UINT32 hwcopy_platform_dma_flush_dev2mem_width_neq_loff(uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	return dma_flushReadCacheWidthNEQLineOffsetWithoutCheck(addr, size);

#else
/*
	if (size > 256*1024)
	{
		flush_cache_all();
		return 1;
	}
	else
*/
	{
#ifdef TYPEVAFLUSH
        //uintptr_t address = (uintptr_t) addr;
		vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_BIDIRECTIONAL);
#endif
		return 0;
	}
#endif
}

UINT32 hwcopy_platform_dma_flush_dev2mem(uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
    //vos_cpu_dcache_sync_vb(addr, size, VOS_DMA_FROM_DEVICE);
	dma_flushReadCache(addr, size);
	return addr;

#else
	UINT32 ret;

#if 0
	if (size > 256*1024) {
		flush_cache_all();
		ret = 1;
	} else
#endif
	{
#ifdef  TYPEVAFLUSH
        //printk("addr=%ld,size=%d,%d\r\n",(VOS_ADDR)addr,size,VOS_DMA_TO_DEVICE); // debug.
        //uintptr_t address = (uintptr_t) addr;
		vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_BIDIRECTIONAL);
#endif
		ret = 0;
	}

	return ret;
#endif
}

UINT32 hwcopy_platform_dma_flush_mem2dev(uintptr_t addr, UINT32 size)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	dma_flushWriteCache(addr, size);
	return addr;

#else
	UINT32 ret;

#if 0
	if (size > 256*1024) {
		flush_cache_all();
		ret = 1;
	} else
#endif
	{
#ifdef TYPEVAFLUSH
        //uintptr_t address = (uintptr_t) addr;
		vos_cpu_dcache_sync((VOS_ADDR)addr, size, VOS_DMA_TO_DEVICE);
#endif
		ret = 0;
	}

	return ret;
#endif
}

BOOL hwcopy_platform_is_valid_va(uintptr_t addr)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	return TRUE;	// assume user always fed valid va
#else
	return TRUE;	// assume user always fed valid va
#endif
}


UINT64 hwcopy_platform_va2pa(uintptr_t addr)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	return dma_getPhyAddr(addr);
#else
#ifdef  TYPEVAFLUSH
	return (UINT64)fmem_lookup_pa(addr);
#else
    return addr;
#endif

#endif
}


/*
	Cp contents from KDRV_HWCOPY_TRIGGER_PARAM to HWCOPY_REQ_LIST_NODE

	Because pointer in HWCOPY_REQ_LIST_NODE may points to stack of caller,
	these data may be not accessed by hwcopy ISR.
	Thus we should copy data pointed by pointer.

	KDRV_HWCOPY_TRIGGER_PARAM: structure passed from kdrv_hwcopy_trigger()
	HWCOPY_REQ_LIST_NODE: internal structure to queue jobs
*/

static ER param_to_queue_element(KDRV_HWCOPY_TRIGGER_PARAM *p_param,
				HWCOPY_REQ_LIST_NODE *p_node)
{
	UINT32 i=0;
	HWCOPY_MEM *p_mem;
	HWCOPY_MEM *p_mem_node;
	HWCOPY_MEM mem_local;

	memcpy(&p_node->trig_param, p_param,
		sizeof(KDRV_HWCOPY_TRIGGER_PARAM));
    //DBG_ERR("param_to_queue_element pimg 0x%x\r\n",
			//p_node->trig_param.p_ctex_descript->datalength);
    p_node->trig_param.p_ctex_descript->datalength = p_param->p_ctex_descript->datalength;
	// image list
	i = 0;
	p_mem = p_param->p_memory;
//	p_node->trig_param->p_memory = &p_node->memory[0];
	p_mem_node = &mem_local;
	p_mem_node->p_next = &p_node->memory[0];
	while ((p_mem != NULL) &&
		(i < 2)) {
		p_mem_node = p_mem_node->p_next;
		//DBG_ERR("next 0x%p\r\n", p_mem_node);
		memcpy(&p_node->memory[i++], p_mem, sizeof(HWCOPY_MEM));
		p_mem = p_mem->p_next;
		p_mem_node->p_next = &p_node->memory[i];
//		p_mem_node = p_mem_node->p_next;
	}
	if (p_mem != NULL) {
		// still have data not copied
		DBG_ERR("param_to_queue_element: img exceed\r\n");
		return -1;
	}
	p_mem_node->p_next = NULL;	// terminate image list
	p_node->trig_param.p_memory = &p_node->memory[0];
	//DBG_ERR(" param pimg 0x%x\r\n",	p_param->p_memory);
    //DBG_ERR("param_to_queue_element pimg 0x%x\r\n",
			//p_node->trig_param.p_ctex_descript->datalength);


	return E_OK;
}

/*
        Check if service queue is empty
*/
BOOL hwcopy_platform_list_empty(HWCOPY_ID id)
{
	if (id >= HWCOPY_ID_MAX) {
//                printk("%s: invalid id %d\r\n", __func__, id);
                return E_SYS;
        }

	if (v_req_front[id] == v_req_tail[id]) {
		// queue empty
		return TRUE;
	} else {
		return FALSE;
	}
}

/**
    Add request descriptor to service queue
*/

ER hwcopy_platform_add_list(HWCOPY_ID id, KDRV_HWCOPY_TRIGGER_PARAM *p_param,
			KDRV_CALLBACK_FUNC *p_callback)
{
	UINT32 next;
	const UINT32 tail = v_req_tail[id];

	if (id >= HWCOPY_ID_MAX) {
//		printk("%s: invalid id %d\r\n", __func__, id);
		return E_SYS;
	}

	next = (tail+1) % HWCOPY_REQ_POLL_SIZE;
//	printk("%s: next %d\r\n", __func__, next);

	if (next == v_req_front[id]) {
		// queue full
//		printk("%s: queue full, front %d, tail %d\r\n", __func__,
//			v_req_front[id], tail);
		return E_SYS;
	}

	if (param_to_queue_element(p_param, &v_req_pool[id][tail]) != E_OK)
		return E_SYS;
//DBG_ERR("Enter hwcopy_platform_add_list \r\n datalen = %x \r\n", v_req_pool[id][tail].trig_param.p_ctex_descript->datalength);

	if (p_callback) {
		memcpy(&v_req_pool[id][tail].callback,
			p_callback,
			sizeof(KDRV_CALLBACK_FUNC));
	} else {
		memset(&v_req_pool[id][tail].callback,
                        0,
                        sizeof(KDRV_CALLBACK_FUNC));
	}

	v_req_tail[id] = next;

//	list_add_tail(, &v_grph_req_list[id]);
        return E_OK;
}

/*
	Get head request descriptor from service queue
*/
HWCOPY_REQ_LIST_NODE* hwcopy_platform_get_head(HWCOPY_ID id)
{
	HWCOPY_REQ_LIST_NODE *p_node;

	p_node = &v_req_pool[id][v_req_front[id]];
    //DBG_ERR(" v_req_front %x || v_req_tail %x \r\n",  v_req_front[id],v_req_tail[id]);
	if (id >= HWCOPY_ID_MAX) {
		DBG_ERR(" invalid id %d\r\n",  id);
		return NULL;
	}

	if (v_req_front[id] == v_req_tail[id]) {
		// queue empty
		DBG_ERR(" queue empty\r\n" );
		return NULL;
	}

	return p_node;

//	memcpy(p_param, &p_node->trig_param, sizeof(KDRV_GRPH_TRIGGER_PARAM));

//	return E_OK;
}

/*
	Delete request descriptor from service queue
*/
ER hwcopy_platform_del_list(HWCOPY_ID id)
{
	if (id >= HWCOPY_ID_MAX) {
//		printk("%s: invalid id %d\r\n", __func__, id);
		return E_SYS;
	}

	if (v_req_front[id] == v_req_tail[id]) {
//		printk("%s: queue already empty, front %d, tail %d\r\n",
			//__func__, v_req_front[id], v_req_tail[id]);
		return E_SYS;
	}

	v_req_front[id] = (v_req_front[id]+1) % HWCOPY_REQ_POLL_SIZE;

	return E_OK;
}

int hwcopy_platform_ist(HWCOPY_ID id, UINT32 event)
{
	hwcopy_isr_bottom(id, event);

	return 0;
}

void hwcopy_platform_set_ist_event(HWCOPY_ID id)
{
	/*  Tasklet for bottom half mechanism */
#if defined __LINUX
        DBG_USER("hwcopy set_ist_event id = %d\r\n", id);

        tasklet_schedule(v_p_timer_tasklet[id]);

#elif defined __FREERTOS
		hwcopy_platform_ist(HWCOPY_ID_0 , 0);

 #endif
}

ER hwcopy_platform_create_resource(void *pmodule_info)
{

#if defined __UITRON || defined __ECOS
#else
#if defined __LINUX
	MODULE_INFO *_pmodule_info = (MODULE_INFO *)pmodule_info;
    IOADDR_HWCP_REG_BASE[_pmodule_info->chip_idx] = _pmodule_info->io_addr[0];

#elif defined __FREERTOS
#endif
#endif

vk_spin_lock_init(&hwcopy_spinlock);
#if defined __UITRON || defined __ECOS
#else
#if defined __LINUX
	if (!IS_ERR(_pmodule_info->pclk)) {
		hwcopy_clk[_pmodule_info->chip_idx] = _pmodule_info->pclk[0];
		hwcopy_clk_en[_pmodule_info->chip_idx] = DISABLE;
		clk_prepare(hwcopy_clk[_pmodule_info->chip_idx]);
	} else {
		DBG_ERR("hwcopy clock error\r\n");
		hwcopy_clk[_pmodule_info->chip_idx] = NULL;
		return E_SYS;
	}

	OS_CONFIG_FLAG(FLG_ID_HWCOPY);
	SEM_CREATE(SEMID_HWCOPY, 1);
	SEM_CREATE(SEMID_HWCOPY_OPEN_CLOSE, 1);
	v_p_timer_tasklet[_pmodule_info->chip_idx] = &_pmodule_info->hwcopy_tasklet[0];

	v_req_front[_pmodule_info->chip_idx] = 0;
	v_req_tail[_pmodule_info->chip_idx] = 0;

	v_sem[_pmodule_info->chip_idx] = &SEMID_HWCOPY;
	v_sem_open_close[_pmodule_info->chip_idx] = &SEMID_HWCOPY_OPEN_CLOSE;
	v_flg_id[_pmodule_info->chip_idx] = FLG_ID_HWCOPY;

#elif defined __FREERTOS


	cre_flg(&FLG_ID_HWCOPY,NULL,"hwcopy_flag");
	vos_sem_create(&SEMID_HWCOPY, 1, "hwcopy_semaphore");
	vos_sem_create(&SEMID_HWCOPY_OPEN_CLOSE, 1, "hwcopy_openclose_semaphore");

	v_req_front[HWCOPY_ID_0] = 0;
	v_req_tail[HWCOPY_ID_0] = 0;

	v_sem[HWCOPY_ID_0] = &SEMID_HWCOPY;
	v_sem_open_close[HWCOPY_ID_0] = &SEMID_HWCOPY_OPEN_CLOSE;
	v_flg_id[HWCOPY_ID_0] = FLG_ID_HWCOPY;

#endif
#endif


//	INIT_LIST_HEAD(&v_grph_req_list[GRPH_ID_1]);
//	INIT_LIST_HEAD(&v_grph_req_list[GRPH_ID_2]);


    return E_OK;

//	printk("SEM ID: 0x%p, 0x%p\r\n", v_sem[0], v_sem[1]);
}

void hwcopy_platform_release_resource(void)
{
	rel_flg(FLG_ID_HWCOPY);
	SEM_DESTROY(SEMID_HWCOPY);
	SEM_DESTROY(SEMID_HWCOPY_OPEN_CLOSE);
}
