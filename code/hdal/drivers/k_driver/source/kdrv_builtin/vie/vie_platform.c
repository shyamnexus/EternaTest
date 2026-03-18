#include "vie_platform.h"

// debug level
#if defined(__FREERTOS)
unsigned int vie_debug_level = (NVT_DBG_IND | NVT_DBG_WRN | NVT_DBG_ERR);
#else
static struct clk* vie_clk[MODULE_CLK_NUM];
static struct clk* vie_pxclkA[MODULE_CLK_NUM];
static struct clk* vie_pxclkB[MODULE_CLK_NUM];
static struct clk* vie_pxclkpad;
static struct clk* vie_sram;
#endif

static VK_DEFINE_SPINLOCK(vie_spin_lock);
//static VK_DEFINE_SPINLOCK(vie_spin_lock2);
static VIE_CLKSRC_SEL g_vieClkSrc[MODULE_CLK_NUM]={VIE_CLKSRC_320};
static UINT32 g_vieClkRate[MODULE_CLK_NUM]={320000000};

ULONG _VIE_REG_BASE_ADDR_SET[MODULE_CLK_NUM];
VIE_PADA_SRC _vie_padA_src = VIE_PADA_SRC_SGPIO1; 


static SEM_HANDLE SEMID_VIE;
static ID     FLG_ID_VIE;
//static SEM_HANDLE SEMID_VIE2;
//static ID     FLG_ID_VIE2;


void vie_platform_disable_sram_shutdown(VIE_ENGINE_ID id)
{
	switch (id) {
	case VIE_ENGINE_ID_1:
		nvt_disable_sram_shutdown(VIE_SD);
		break;

	//case VIE_ENGINE_ID_2:
	//	//nvt_disable_sram_shutdown(VIE2_SD); //fix build error
	//	break;

	default:
		nvt_dbg(ERR,"id = %d overflow\r\n", (int)id);
		break;
	}
}

UINT32 vie_platform_dma_flush_dev2mem(ULONG addr, UINT32 size, UINT32 type)
{
	if (type == 0)
		vos_cpu_dcache_sync((VOS_ADDR)addr, ALIGN_CEIL(size, VOS_ALIGN_BYTES), VOS_DMA_FROM_DEVICE);
	else
		vos_cpu_dcache_sync_vb((VOS_ADDR)addr, ALIGN_CEIL(size, VOS_ALIGN_BYTES), VOS_DMA_FROM_DEVICE);
	return 0;
}

ULONG vie_platform_va2pa(ULONG addr)
{
	VOS_ADDR phy_adr;

	phy_adr = vos_cpu_get_phy_addr((VOS_ADDR)addr);
	if (phy_adr == VOS_ADDR_INVALID) {
		nvt_dbg(ERR,"addr(0x%lx) conv to phy addr fail\r\n", (unsigned long)addr);
		return 0;
	}
	return phy_adr;
}

void vie_platform_enable_sram_shutdown(VIE_ENGINE_ID id)
{
	switch (id) {
	case VIE_ENGINE_ID_1:
		nvt_enable_sram_shutdown(VIE_SD);
		break;

	//case VIE_ENGINE_ID_2:
	//	//nvt_enable_sram_shutdown(VIE2_SD); //fix build error
	//	break;

	default:
		nvt_dbg(ERR,"id = %d overflow\r\n", (int)id);
		break;
	}
}

void vie_platform_prepare_clk(VIE_ENGINE_ID id)
{
#if defined(__FREERTOS)
#else
	clk_prepare(vie_clk[id]);
	clk_prepare(vie_pxclkA[id]);
	clk_prepare(vie_pxclkB[id]);
#endif
}

void vie_platform_unprepare_clk(VIE_ENGINE_ID id)
{
#if defined(__FREERTOS)
#else
	//clk_unprepare(vie_clk[id]);
	//clk_unprepare(vie_pxclkA[id]);
	//clk_unprepare(vie_pxclkB[id]);
#endif
}

void vie_platform_enable_clk(VIE_ENGINE_ID id)
{
#if defined(__FREERTOS)
	switch (id) {
	case VIE_ENGINE_ID_1:
		pll_enableClock(VIE_CLK);
		break;

	//case VIE_ENGINE_ID_2:
	//	pll_enableClock(VIE2_CLK);
	//	break;
		
	default:
		nvt_dbg(ERR,"id = %d overflow\r\n", (int)id);
		break;
	}
#else
	clk_enable(vie_clk[id]);
	//clk_enable(vie_pxclkA[id]);
	
#endif
}

void vie_platform_disable_clk(VIE_ENGINE_ID id)
{
#if defined(__FREERTOS)
	VIE_CLKSRC_SEL clksrc = VIE_CLKSRC_480;
	UINT32 clkrate;

	switch (id) {
	case VIE_ENGINE_ID_1:
		pll_disableClock(VIE_CLK);
		vie_getClock(VIE_ENGINE_ID_1,&clksrc, &clkrate);
		break;

	//case VIE_ENGINE_ID_2:
	//	pll_disableClock(VIE2_CLK);
	//	vie_getClock(VIE_ENGINE_ID_2,&clksrc, &clkrate);
	//	break;

	default:
		nvt_dbg(ERR,"id = %d overflow\r\n", (int)id);
		break;
	}
#if 0
	switch (clksrc) {
	case SIE_CLKSRC_PLL5:
		pll_setPLLEn(PLL_ID_5, FALSE);
		break;

	case SIE_CLKSRC_PLL12:
		pll_setPLLEn(PLL_ID_12, FALSE);
		break;

	default:
		break;
	}
	#endif
#else
	clk_disable(vie_clk[id]);
	
	//clk_disable(vie_pxclkA[id]);
	
#endif
}

ER vie_platform_sem_wait(VIE_ENGINE_ID id)
{
	ER erReturn = E_OK;

	switch (id) {
	case VIE_ENGINE_ID_1:
		erReturn = SEM_WAIT(SEMID_VIE);
		break;

	//case VIE_ENGINE_ID_2:
	//	erReturn = SEM_WAIT(SEMID_VIE2);
	//	break;

	default:
		nvt_dbg(ERR,"id = %d overflow\r\n", (int)id);
		break;
	}

	return erReturn;
}

ER vie_platform_sem_signal(VIE_ENGINE_ID id)
{
	switch (id) {
	case VIE_ENGINE_ID_1:
		SEM_SIGNAL(SEMID_VIE);
		break;

	//case VIE_ENGINE_ID_2:
	//	SEM_SIGNAL(SEMID_VIE2);
	//	break;

	default:
		nvt_dbg(ERR,"id = %d overflow\r\n", (int)id);
		break;
	}
	return E_OK;
}

ER vie_platform_flg_clear(VIE_ENGINE_ID id, FLGPTN flg)
{
	switch (id) {
	case VIE_ENGINE_ID_1:
		vos_flag_clr(FLG_ID_VIE, flg);
		break;

	//case VIE_ENGINE_ID_2:
	//	vos_flag_clr(FLG_ID_VIE2, flg);
	//	break;

	default:
		nvt_dbg(ERR,"id = %d overflow\r\r\n", (int)id);
		break;
	}

	return E_OK;
}

ER vie_platform_flg_set(VIE_ENGINE_ID id, FLGPTN flg)
{
	switch (id) {
	case VIE_ENGINE_ID_1:
		vos_flag_iset(FLG_ID_VIE, flg);
		break;

	//case VIE_ENGINE_ID_2:
	//	vos_flag_iset(FLG_ID_VIE2, flg);
	//	break;

	default:
		nvt_dbg(ERR,"id = %d overflow\r\n", (int)id);
		break;
	}

	return E_OK;
}

ER vie_platform_flg_wait(VIE_ENGINE_ID id, PFLGPTN p_flgptn, FLGPTN flg)
{
	switch (id) {
	case VIE_ENGINE_ID_1:
		if (vos_flag_wait_timeout(p_flgptn, FLG_ID_VIE, flg, TWF_CLR|TWF_ORW, vos_util_msec_to_tick(SIE_DRVWAIT_FLG_TIMEOUT_MS)) != 0) {
			nvt_dbg(ERR,"id: %d, wait flag timeout (0x%08lx,0x%08lx)\r\n",id,(ULONG)p_flgptn,(ULONG)flg);
		}
		break;

	//case VIE_ENGINE_ID_2:
	//	if (vos_flag_wait_timeout(p_flgptn, FLG_ID_VIE2, flg, TWF_CLR|TWF_ORW, vos_util_msec_to_tick(SIE_DRVWAIT_FLG_TIMEOUT_MS)) != 0) {
	//		nvt_dbg(ERR,"id: %d, wait flag timeout (0x%08lx,0x%08lx)\r\n",id,(ULONG)p_flgptn,(ULONG)flg);
	//	}
	//	break;

	default:
		nvt_dbg(ERR,"id = %d overflow\r\n", (int)id);
		break;
	}
	return E_OK;
}

unsigned long vie_platform_spin_lock(void)
{
	unsigned long sie_spin_flags = 0;
#if 0
	switch (type) {
	case 0:	//sie_int 1
		vk_spin_lock_irqsave(&vie_spin_lock, sie_spin_flags);
		break;

	case 1:	//sie_int 2
		vk_spin_lock_irqsave(&vie_spin_lock2, sie_spin_flags);
		break;

	default:
		nvt_dbg(ERR,"type = %d overflow\r\n", (int)type);
		break;
	}
#else
	vk_spin_lock_irqsave(&vie_spin_lock, sie_spin_flags);
#endif
	return sie_spin_flags;
}

void vie_platform_spin_unlock(unsigned long flags)
{
#if 0	
	switch (type) {
	case 0:	//vie_int 1
		vk_spin_unlock_irqrestore(&vie_spin_lock, flags);
		break;

	case 1:	//vie_int 2
		vk_spin_unlock_irqrestore(&vie_spin_lock2, flags);
		break;

	default:
		nvt_dbg(ERR,"type = %d overflow\r\n", (int)type);
		break;
	}
#else
	vk_spin_unlock_irqrestore(&vie_spin_lock, flags);
#endif

}

#if defined(__FREERTOS)
irqreturn_t vie1_platform_isr(int irq, void *devid)
{
	vie_isr(VIE_ENGINE_ID_1);
	return IRQ_HANDLED;
}
irqreturn_t vie2_platform_isr(int irq, void *devid)
{
	vie_isr(VIE_ENGINE_ID_2);
	return IRQ_HANDLED;
}

void vie_platform_create_resource(void)
{
	vos_flag_create(&FLG_ID_VIE, NULL, "vie");
	vos_sem_create(&SEMID_VIE, 1, "SEM_ID_VIE");

	//vos_flag_create(&FLG_ID_VIE2, NULL, "vie2");
	//vos_sem_create(&SEMID_VIE2, 1, "SEM_ID_VIE2");

	_VIE_REG_BASE_ADDR_SET[VIE_ENGINE_ID_1] = 0x2F0320000;
	//_VIE_REG_BASE_ADDR_SET[VIE_ENGINE_ID_2] = 0xF0DD0000;

	request_irq(INT_ID_VIE, vie1_platform_isr, IRQF_TRIGGER_HIGH, "vie1", 0);
	//request_irq(INT_ID_VIE2, vie2_platform_isr, IRQF_TRIGGER_HIGH, "vie2", 0);
}
#else
void vie_platform_create_resource(VIE_MODULE_INFO *pmodule_info)
{
	vos_flag_create(&FLG_ID_VIE, NULL, "vie");
	vos_sem_create(&SEMID_VIE, 1, "SEM_ID_VIE");

	//vos_flag_create(&FLG_ID_VIE2, NULL, "vie2");
	//vos_sem_create(&SEMID_VIE2, 1, "SEM_ID_VIE2");

	_VIE_REG_BASE_ADDR_SET[VIE_ENGINE_ID_1] = (ULONG)pmodule_info->io_addr[VIE_ENGINE_ID_1];
	//_VIE_REG_BASE_ADDR_SET[VIE_ENGINE_ID_2] = (ULONG)pmodule_info->io_addr[VIE_ENGINE_ID_2];

	vie_clk[VIE_ENGINE_ID_1] = pmodule_info->vie_clk[0];//clk_get(NULL, "2f0dc0000.vie1");
	//vie_clk[VIE_ENGINE_ID_2] = pmodule_info->vie_clk[1];//clk_get(NULL, "2f0dd0000.vie2");

	vie_pxclkA[VIE_ENGINE_ID_1] = pmodule_info->vie_pxclkA[0];//clk_get(NULL, "vie1_pxclk");
	//vie_pxclkA[VIE_ENGINE_ID_2] = pmodule_info->vie_pxclkA[1];//clk_get(NULL, "vie2_pxclk");

	vie_pxclkB[VIE_ENGINE_ID_1] = pmodule_info->vie_pxclkB[0];//clk_get(NULL, "vie1_pxclk");
	//vie_pxclkB[VIE_ENGINE_ID_2] = pmodule_info->vie_pxclkB[1];//clk_get(NULL, "vie2_pxclk");

	vie_pxclkpad = pmodule_info->vie_pxclkpad;
	//nvt_dbg(IND, "CLK GET done\r\n");

	vie_sram = pmodule_info->vie_sram;
}
#endif
void vie_platform_release_resource(void)
{
	vos_sem_destroy(FLG_ID_VIE);
	vos_sem_destroy(SEMID_VIE);

	//vos_sem_destroy(FLG_ID_VIE2);
	//vos_sem_destroy(SEMID_VIE2);
}

void vie_platform_set_clk_rate(VIE_ENGINE_ID id, VIE_OPENOBJ *pObjCB)
{
    vie_setClock(id, pObjCB->VieClkSel, pObjCB->uiVieClockRate);
	vie_setPxClock(id, pObjCB->PxClkSel);
}

ER vie1_setClock(VIE_CLKSRC_SEL ClkSrc, UINT32 uiClkRate)
{
#if defined(__FREERTOS)

#if (defined(_NVT_FPGA_))
	pll_setClockRate(PLL_CLKSEL_VIE_CLKSRC, PLL_CLKSEL_VIE_CLKSRC_PLL19);
	g_vieClkSrc[VIE_ENGINE_ID_1] = VIE_CLKSRC_PLL19;
	g_vieClkRate[VIE_ENGINE_ID_1] = pll_get_pll_freq(PLL_ID_19);
	pll_setPLLEn(PLL_ID_19, TRUE);
#else
	pll_setClockRate(PLL_CLKSEL_VIE_CLKSRC, PLL_CLKSEL_VIE_CLKSRC_PLL19);
	g_vieClkSrc[VIE_ENGINE_ID_1] = VIE_CLKSRC_PLL19;
	g_vieClkRate[VIE_ENGINE_ID_1] = pll_get_pll_freq(PLL_ID_19);
	pll_setPLLEn(PLL_ID_19, TRUE);
#endif

	//nvt_dbg(ERR, "SIE ClkSrc = %d, ClkRate = %d\r\n",g_sieClkSrc[SIE_ENGINE_ID_1],g_sieClkRate[SIE_ENGINE_ID_1]);

	// Enable PLL clock source
	if (g_vieClkRate[VIE_ENGINE_ID_1] != 0) {
		pll_setClockFreq(VIECLK_FREQ, g_vieClkRate[VIE_ENGINE_ID_1]);
	}
#else
	struct clk *parent_clk=NULL;
	UINT32 vie_src_freq;
	#if defined(CONFIG_NVT_FPGA_EMULATION)
	struct clk *parent2_clk=NULL;
	#endif
	
	#if defined(CONFIG_NVT_FPGA_EMULATION)
	parent_clk = clk_get(NULL, "fix480m");
	vie_src_freq = clk_get_rate(parent_clk);
	g_vieClkSrc[VIE_ENGINE_ID_1] = VIE_CLKSRC_480;
	g_vieClkRate[VIE_ENGINE_ID_1] = vie_src_freq;
	// enable pll19
	parent2_clk = clk_get(NULL, "pll5");
	if (parent2_clk==NULL) {
		nvt_dbg(ERR, "pll5 nod null\r\n");
	}
	clk_prepare(parent2_clk);
	clk_enable(parent2_clk);
	#else
	parent_clk = clk_get(NULL, "fix320m");
	vie_src_freq = clk_get_rate(parent_clk);
	g_vieClkSrc[VIE_ENGINE_ID_1] = VIE_CLKSRC_320;
	if (vie_src_freq<=VIE_MAX_CLK_FREQ)
		g_vieClkRate[VIE_ENGINE_ID_1] = vie_src_freq;
	//else if (sie_src_freq==(2*VIE_MAX_CLK_FREQ))
	//	g_vieClkRate[VIE_ENGINE_ID_1] = sie_src_freq>>1;
	#endif
	
	
	//nvt_dbg(ERR, "SIE ClkSrc = %d, ClkRate = %d\r\n",g_sieClkSrc[SIE_ENGINE_ID_1],g_sieClkRate[SIE_ENGINE_ID_1]);

	if (!IS_ERR(parent_clk)) {
		clk_set_parent(vie_clk[VIE_ENGINE_ID_1], parent_clk);
	}

	clk_put(parent_clk);

	if (g_vieClkRate[VIE_ENGINE_ID_1] != 0) {
		clk_set_rate(vie_clk[VIE_ENGINE_ID_1], g_vieClkRate[VIE_ENGINE_ID_1]);
	}
#endif

	return 0;
}

ER vie2_setClock(VIE_CLKSRC_SEL ClkSrc, UINT32 uiClkRate)
{
#if defined(__FREERTOS)

#if (defined(_NVT_FPGA_))
	pll_setClockRate(PLL_CLKSEL_VIE2_CLKSRC, PLL_CLKSEL_VIE2_CLKSRC_PLL19);
	g_vieClkSrc[VIE_ENGINE_ID_2] = VIE_CLKSRC_PLL19;
	g_vieClkRate[VIE_ENGINE_ID_2] = pll_get_pll_freq(PLL_ID_19);
	pll_setPLLEn(PLL_ID_19, TRUE);
#else
	pll_setClockRate(PLL_CLKSEL_VIE2_CLKSRC, PLL_CLKSEL_VIE2_CLKSRC_PLL19);
	g_vieClkSrc[VIE_ENGINE_ID_2] = VIE_CLKSRC_PLL19;
	g_vieClkRate[VIE_ENGINE_ID_2] = pll_get_pll_freq(PLL_ID_19);
	pll_setPLLEn(PLL_ID_19, TRUE);
#endif

	//nvt_dbg(ERR, "SIE ClkSrc = %d, ClkRate = %d\r\n",g_sieClkSrc[SIE_ENGINE_ID_1],g_sieClkRate[SIE_ENGINE_ID_1]);

	// Enable PLL clock source
	if (g_vieClkRate[VIE_ENGINE_ID_2] != 0) {
		pll_setClockFreq(VIE2CLK_FREQ, g_vieClkRate[VIE_ENGINE_ID_2]);
	}
#else
	struct clk *parent_clk=NULL;
	UINT32 vie_src_freq;
	#if defined(CONFIG_NVT_FPGA_EMULATION)
	struct clk *parent2_clk=NULL;
	#endif

	#if defined(CONFIG_NVT_FPGA_EMULATION)
	parent_clk = clk_get(NULL, "fix480m");
	vie_src_freq = clk_get_rate(parent_clk);
	g_vieClkSrc[VIE_ENGINE_ID_2] = VIE_CLKSRC_480;
	g_vieClkRate[VIE_ENGINE_ID_2] = vie_src_freq;
	// enable pll5
	parent2_clk = clk_get(NULL, "pll19");
	if (parent2_clk==NULL) {
		nvt_dbg(ERR, "pll19 nod null\r\n");
	}
	clk_prepare(parent2_clk);
	clk_enable(parent2_clk);
	#else
	parent_clk = clk_get(NULL, "pll23");
	vie_src_freq = clk_get_rate(parent_clk);
	g_vieClkSrc[VIE_ENGINE_ID_2] = VIE_CLKSRC_PLL23;
	if (vie_src_freq<=VIE_MAX_CLK_FREQ)
		g_vieClkRate[VIE_ENGINE_ID_2] = vie_src_freq;
	//else if (sie_src_freq==(2*VIE_MAX_CLK_FREQ))
	//	g_vieClkRate[VIE_ENGINE_ID_2] = sie_src_freq>>1;
	#endif

	//nvt_dbg(ERR, "SIE2 ClkSrc = %d, ClkRate = %d\r\n",g_sieClkSrc[SIE_ENGINE_ID_2],g_sieClkRate[SIE_ENGINE_ID_2]);

	if (!IS_ERR(parent_clk)) {
		clk_set_parent(vie_clk[VIE_ENGINE_ID_2], parent_clk);
	}

	clk_put(parent_clk);

	if (g_vieClkRate[VIE_ENGINE_ID_2] != 0) {
		clk_set_rate(vie_clk[VIE_ENGINE_ID_2], g_vieClkRate[VIE_ENGINE_ID_2]);
	}
	
#endif

	return 0;
}

ER vie_setClock(VIE_ENGINE_ID id, VIE_CLKSRC_SEL ClkSrc, UINT32 uiClkRate)
{
    if (uiClkRate > VIE_MAX_CLK_FREQ) {
		nvt_dbg(ERR, "id %d, Input Clk rate %d > MAX_CLK_FREQ, clamp to %d\r\n",id, uiClkRate,VIE_MAX_CLK_FREQ);
		uiClkRate = VIE_MAX_CLK_FREQ;
	}
	
	switch (id) {
	case VIE_ENGINE_ID_1:
		vie1_setClock(ClkSrc, uiClkRate);
		break;

	case VIE_ENGINE_ID_2:
		vie2_setClock(ClkSrc, uiClkRate);
		break;

	default:
		nvt_dbg(ERR,"id = %d out of range !!\r\n", id);
	return E_SYS;
	}
	return E_OK;
}

ER vie_getClock(VIE_ENGINE_ID id, VIE_CLKSRC_SEL *ClkSrc, UINT32 *uiClkRate)
{
	*ClkSrc = g_vieClkSrc[id];
	*uiClkRate = g_vieClkRate[id];

	return E_OK;
}

ER vie1_setPxClock(VIE_PXCLKSRC PxClkSel)
{
#if defined(__FREERTOS)
	//#if defined(_BSP_NA51055_)
	switch (PxClkSel) {
	case VIE_PXCLKSRC_OFF:
		break;

	case VIE_PXCLKSRC_PAD_A:
		pll_enableClock(VIE_A_PXCLK);
		break;

	case VIE_PXCLKSRC_PAD_B:
		pll_enableClock(VIE_B_PXCLK);
		break;

	case VIE_PXCLKSRC_PAD_AB:
		pll_enableClock(VIE_A_PXCLK);
		pll_enableClock(VIE_B_PXCLK);
		break;

	default:
		nvt_dbg(ERR,"VIE PX_SRC %d out of range !!\r\n", PxClkSel);
		break;
	}
	//#endif
	return 0;
#else
	struct clk *pclk = NULL;
	struct clk *parent_clk = NULL;
	
	const char *pclk_name;
	char *golden = "vie_padA_pgpio0";

	switch (PxClkSel) {
	case VIE_PXCLKSRC_OFF:
		break;

	case VIE_PXCLKSRC_PAD_A:
		pclk = clk_get(NULL, "vie1_pxclk1");
		parent_clk = clk_get_parent(pclk);
		pclk_name = __clk_get_name(parent_clk);
		clk_put(pclk);
		
		//nvt_dbg(ERR,"name2 = %s\r\n",pclk_name);
	
		
		if (strncmp(golden, (CHAR *)pclk_name, 15) == 0) {
			_vie_padA_src = VIE_PADA_SRC_PGPIO0;
			//nvt_dbg(ERR, "padA src is vie_padA_pgpio0\r\n");
		}else {
			_vie_padA_src = VIE_PADA_SRC_SGPIO1;
			//nvt_dbg(ERR, "padA src is vie_padA_sgpio1\r\n");
		}
		
		clk_enable(vie_pxclkA[0]);
		break;

	case VIE_PXCLKSRC_PAD_B:
		clk_enable(vie_pxclkB[0]);
		break;

	case VIE_PXCLKSRC_PAD_AB:
		pclk = clk_get(NULL, "vie1_pxclk1");
		parent_clk = clk_get_parent(pclk);
		pclk_name = __clk_get_name(parent_clk);
		clk_put(pclk);	
		
		if (strncmp(golden, (CHAR *)pclk_name, 15) == 0) {
			_vie_padA_src = VIE_PADA_SRC_PGPIO0;
		}else {
			_vie_padA_src = VIE_PADA_SRC_SGPIO1;
		}
		clk_enable(vie_pxclkA[0]);
		clk_enable(vie_pxclkB[0]);
		break;

	default:
		nvt_dbg(ERR,"VIE PX_SRC %d out of range !!\r\n", PxClkSel);
		break;
	}
#if 0
	//nvt_dbg(IND, "PxClkSel = %d\r\n",PxClkSel);

	if (!IS_ERR(parent_clk)) {
		clk_set_parent(vie_pxclk[VIE_ENGINE_ID_1], parent_clk);
	}

	clk_put(parent_clk);

	if (parent_clk2 != NULL) {
		if (!IS_ERR(parent_clk2)) {
		//	clk_set_parent(vie_io_pxclk[VIE_ENGINE_ID_1], parent_clk2);
		}

		//clk_put(parent_clk2);
	}
	#endif
#endif

	return 0;
}

ER vie2_setPxClock(VIE_PXCLKSRC PxClkSel)
{
#if defined(__FREERTOS)
	//#if defined(_BSP_NA51055_)
	switch (PxClkSel) {
	case VIE_PXCLKSRC_OFF:
		break;

	case VIE_PXCLKSRC_PAD_A:
		pll_enableClock(VIE2_A_PXCLK);
		break;

	case VIE_PXCLKSRC_PAD_B:
		pll_enableClock(VIE2_B_PXCLK);
		break;

	case VIE_PXCLKSRC_PAD_AB:
		pll_enableClock(VIE2_A_PXCLK);
		pll_enableClock(VIE2_B_PXCLK);
		break;
	default:
		nvt_dbg(ERR,"VIE2 PX_SRC %d out of range !!\r\n", PxClkSel);
		break;
	}
	//#endif
#else
	//struct clk *parent_clk = NULL;
	//struct clk *parent_clk2 = NULL;

	switch (PxClkSel) {
	case VIE_PXCLKSRC_OFF:
		break;

	case VIE_PXCLKSRC_PAD_A:
		clk_enable(vie_pxclkA[1]);
		//parent_clk = clk_get(NULL, "sie2_io_pxclk");
		//parent_clk2 = clk_get(NULL, "sie2_pxclkpad");
		break;

	case VIE_PXCLKSRC_PAD_B:
		clk_enable(vie_pxclkB[1]);
		//parent_clk = clk_get(NULL, "sie2_io_pxclk");
		//parent_clk2 = clk_get(NULL, "sie2_pxclkpad");
		break;

	case VIE_PXCLKSRC_PAD_AB:
		clk_enable(vie_pxclkA[1]);
		clk_enable(vie_pxclkB[1]);
		break;

	default:
		nvt_dbg(ERR,"VIE2 PX_SRC %d out of range !!\r\n", PxClkSel);
		break;
	}
#if 0
	if (!IS_ERR(parent_clk)) {
		clk_set_parent(vie_pxclk[VIE_ENGINE_ID_2], parent_clk);
	}

	clk_put(parent_clk);

	if (parent_clk2 != NULL) {
		if (!IS_ERR(parent_clk2)) {
		//	clk_set_parent(sie_io_pxclk[SIE_ENGINE_ID_2], parent_clk2);
		}

		//clk_put(parent_clk2);
	}
	#endif
#endif

	return 0;
}

ER vie_setPxClock(VIE_ENGINE_ID id, VIE_PXCLKSRC PxClkSel)
{
	switch (id) {
	case VIE_ENGINE_ID_1:
		vie1_setPxClock(PxClkSel);
		break;

	case VIE_ENGINE_ID_2:
		vie2_setPxClock(PxClkSel);
		break;

	default:
		nvt_dbg(ERR,"id = %d out of range !!\r\n", id);
		return E_SYS;
	}
	return E_OK;
}
ER vie_sram_switch(void)
{
#if defined(__FREERTOS)
	pll_set_clock_rate(PLL_CLKSEL_SIE5_SRAM, PLL_CLKSEL_SIE5_SRAM_VIE);
#else
	struct clk *parent_clk;
	
	parent_clk = clk_get(NULL, "vcap_sm_sel_vie");
	if (!IS_ERR(parent_clk)) {
		clk_set_parent(vie_sram, parent_clk);
		clk_put(parent_clk);
	}else {
		nvt_dbg(ERR, "can't get clk vcap_sram_sel_vie \r\n");
	}
#endif
	return E_OK;
}

/**
	register log
*/

#define VIE_REG_LOG_NUM 256
static VIE_REG_INFO *reg_log[MODULE_CLK_NUM]={NULL};
static UINT32 reg_log_cnt[MODULE_CLK_NUM]={0};
static UINT32 b_reg_log_en[MODULE_CLK_NUM] = {DISABLE};

void vie_platform_set_reg(ULONG addr, UINT32 ofs, UINT32 val)
{
    UINT32 i,vie_id=0;

    for (i=0;i<MODULE_CLK_NUM;i++) {
		if(addr==_VIE_REG_BASE_ADDR_SET[i]) {
			vie_id = i;
		}
    }

	if (b_reg_log_en[vie_id] && reg_log_cnt[vie_id] < VIE_REG_LOG_NUM) {
		//if (sie_id == 1)
		//	nvt_dbg(ERR,"record sie%d register\r\n",(int)sie_id);
		reg_log[vie_id][reg_log_cnt[vie_id]].ofs = ofs;
		reg_log[vie_id][reg_log_cnt[vie_id]].val = val;
		reg_log_cnt[vie_id]++;

		if (reg_log_cnt[vie_id] >= VIE_REG_LOG_NUM) {
			nvt_dbg(ERR,"record register buffer overflow %d\r\n",(int)VIE_REG_LOG_NUM);
		}
	}
#if defined(__FREERTOS)
	OUTW((addr + ofs), (val));
#else
	iowrite32(val, (void *)(addr + ofs));
#endif
}

INT32 vie_platform_set_reg_log_enable(UINT32 id, UINT32 en)
{
#if defined(__FREERTOS)
#else
	void* p_buf;

	if (b_reg_log_en[id] != 0) {
		DBG_ERR("SIE %d log already enable\r\n",id);
		return E_SYS;
	}

	//pr_err("SIE alloc register log buffer OK\r\n");
	//pr_err("SIE alloc register log buffer OK\r\n");
	//pr_err("SIE alloc register log buffer OK\r\n");


	#if 0
	for (count=0;count<MODULE_CLK_NUM;count++) {
		p_buf = kmalloc(sizeof(SIE_REG_INFO) * SIE_REG_LOG_NUM, GFP_KERNEL);
		if (p_buf == NULL) {
			DBG_ERR("SIE %d alloc register log buffer failed\r\n", count);
			return E_NOMEM;
		}
		reg_log_cnt[count] = 0;
		reg_log[count] = p_buf;
		//pr_err("SIE%d register log buffer 0x%08x\r\n",count,(int)reg_log[count]);
    }

	b_reg_log_en = en;
	#else
	if (id<MODULE_CLK_NUM) {
		p_buf = kmalloc(sizeof(VIE_REG_INFO) * VIE_REG_LOG_NUM, GFP_KERNEL);
		if (p_buf == NULL) {
			DBG_ERR("SIE %d alloc register log buffer failed\r\n", id);
			return E_NOMEM;
		}
		reg_log_cnt[id] = 0;
		reg_log[id] = p_buf;


		b_reg_log_en[id] = en;
	}
	#endif
#endif
	return E_OK;
}

INT32 vie_platform_set_reg_log_disable(UINT32 id, UINT32 b_freebuf)
{
#if defined(__FREERTOS)
#else
	b_reg_log_en[id] = DISABLE;

	//pr_err("SIE disable register log buffer OK,log_en %d, free_en %d, id %d\r\n",b_reg_log_en,b_freebuf, id);
	//pr_err("SIE disable register log buffer OK,log_en %d, free en %d, id %d\r\n",b_reg_log_en,b_freebuf, id);
	//pr_err("SIE disable register log buffer OK,log_en %d, free en %d, id %d\r\n",b_reg_log_en,b_freebuf, id);

	if (b_freebuf) {
		//pr_err("id = %d\r\n",id);
		if ((reg_log[id] != NULL) && (id<MODULE_CLK_NUM)) {
			//pr_err("free %d buf 0x%08x\r\n",id, (int)reg_log[id]);
			kfree((void *)reg_log[id]);
			reg_log[id] = NULL;
		}

	}
#endif
	return E_OK;
}

VIE_REG_LOG_RESULT vie_platform_get_reg_log(UINT32 id)
{
	VIE_REG_LOG_RESULT rst;

	//need support multi sie
	rst.cnt = reg_log_cnt[id];
	rst.p_reg = reg_log[id];
	//pr_err("get_reg_log, id = %d, cnt = %d\r\n",id, reg_log_cnt[id]);

	return rst;
}

EXPORT_SYMBOL(vie_platform_release_resource);
EXPORT_SYMBOL(vie_setClock);
EXPORT_SYMBOL(vie_getClock);
EXPORT_SYMBOL(vie_setPxClock);
EXPORT_SYMBOL(vie_platform_set_reg_log_enable);
EXPORT_SYMBOL(vie_platform_get_reg_log);
EXPORT_SYMBOL(vie_platform_set_reg_log_disable);
