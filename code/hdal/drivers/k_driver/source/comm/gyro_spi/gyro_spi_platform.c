#include "include/gyro_spi_platform.h"

extern struct clk *nvt_gyro_spi_clk;
vk_spinlock_t spi_spinlock;
SEM_HANDLE vSpiSemaphore[SPI_ID_COUNT];
ID FLG_ID_SPI;
DRV_CB pfIstCB_SPI3 = NULL;
SPI_LINUX_ACT_CB pfLinuxActCB = NULL;
UINT64 spi3_reg_base;

extern UINT32 *P_HOST_CAP;
extern UINT32 vSpiHostCapability[2][SPI_ID_COUNT];
extern UINT32 vSpiHostCapability538[2][SPI_ID_COUNT];

const FLGPTN vSpiFlags[SPI_ID_COUNT] = {
	FLGPTN_SPI,	
	FLGPTN_SPI2,	
	FLGPTN_SPI3,
	FLGPTN_SPI4,	
	FLGPTN_SPI5,
};

const FLGPTN vSpiDmaFlags[SPI_ID_COUNT] = {
	FLGPTN_SPI_DMA, 
	FLGPTN_SPI_DMA2,
	FLGPTN_SPI_DMA3,
	FLGPTN_SPI_DMA4,
	FLGPTN_SPI_DMA5,
};

const FLGPTN vSpiTimeoutFlags[SPI_ID_COUNT] = {
	FLGPTN_SPI_TIMEOUT,     
	FLGPTN_SPI_TIMEOUT2,    
	FLGPTN_SPI_TIMEOUT3,
	FLGPTN_SPI_TIMEOUT4,	
	FLGPTN_SPI_TIMEOUT5,
};

void spi_platform_cache_sync_dma_to_dev(VOS_ADDR addr, ULONG size)
{
	vos_cpu_dcache_sync(addr, size, VOS_DMA_TO_DEVICE_NON_ALIGN);
}

void spi_platform_cache_sync_dma_from_dev(VOS_ADDR addr, ULONG size)
{
	vos_cpu_dcache_sync(addr, size, VOS_DMA_FROM_DEVICE_NON_ALIGN);
}

void spi_platform_get_clk_freq(UINT32 *freq)
{
	*freq = clk_get_rate(nvt_gyro_spi_clk);
}

void spi_platform_set_clk_freq(UINT32 freq)
{
	clk_set_rate(nvt_gyro_spi_clk, freq);
}

void spi_platform_init(SPI_LINUX_ACT_CB act_cb)
{
	if (nvt_get_chip_id() == CHIP_NS02402) {
		P_HOST_CAP = &vSpiHostCapability[0][0];
	} else {
		P_HOST_CAP = &vSpiHostCapability538[0][0];
	}

	OS_CONFIG_FLAG(FLG_ID_SPI);

	vk_spin_lock_init(&spi_spinlock);
	//SEM_CREATE(vSpiSemaphore[0], 1);
	//SEM_CREATE(vSpiSemaphore[1], 1);
	SEM_CREATE(vSpiSemaphore[2], 1);
	//SEM_CREATE(vSpiSemaphore[3], 1);
	//SEM_CREATE(vSpiSemaphore[4], 1);

	pfLinuxActCB = act_cb;
}

void spi_platform_uninit(void)
{
	P_HOST_CAP = NULL;

	rel_flg(FLG_ID_SPI);

	//SEM_DESTROY(vSpiSemaphore[0]);
	//SEM_DESTROY(vSpiSemaphore[1]);
	SEM_DESTROY(vSpiSemaphore[2]);
	//SEM_DESTROY(vSpiSemaphore[3]);
	//SEM_DESTROY(vSpiSemaphore[4]);
}

unsigned long spi_platform_spinlock_lock(void)
{
	unsigned long flags;
	vk_spin_lock_irqsave(&spi_spinlock, flags);
	return flags;
}

void spi_platform_spinlock_unlock(unsigned long flags)
{
	vk_spin_unlock_irqrestore(&spi_spinlock, flags);
}

ER spi_platform_wait_sem(SPI_ID spiID)
{
	vos_sem_wait(vSpiSemaphore[spiID]);
	return E_OK;
}

ER spi_platform_sig_sem(SPI_ID spiID)
{
	vos_sem_sig(vSpiSemaphore[spiID]);
	return E_OK;
}

void spi_platform_clr_flg(FLGPTN flags)
{
	vos_flag_clr(FLG_ID_SPI, flags);
}

void spi_platform_set_flg(FLGPTN flags)
{
	vos_flag_set(FLG_ID_SPI, flags);
}

void spi_platform_wait_flg(PFLGPTN flagPtr, FLGPTN flags, UINT mode)
{
	vos_flag_wait(flagPtr, FLG_ID_SPI, flags, mode);
}