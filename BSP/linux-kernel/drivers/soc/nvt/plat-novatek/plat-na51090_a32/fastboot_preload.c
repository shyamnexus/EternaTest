#include <linux/soc/nvt/rcw_macro.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <plat/hardware.h>

static DEFINE_MUTEX(mmc_lock);
#define SDIO_STATUS_REG_OFS     0x28
#define RSP_CRC_FAIL            0x1
#define DATA_CRC_FAIL           0x2
#define RSP_TIMEOUT             0x4
#define DATA_TIMEOUT            0x8
#define DATA_END                0x80
#define DMA_ERROR               0x80000

int nvt_mmc_check_preload_finish(void)
{
	void __iomem *mmc_address;
	static int mmc_ret = 0;
	u32 status = 0x0;

	mutex_lock(&mmc_lock);
	if (mmc_ret) {
		mutex_unlock(&mmc_lock);
		return mmc_ret;
	}

	mmc_address = ioremap_nocache(NVT_SDIO2_BASE_PHYS, 0x1000);
	if (unlikely(mmc_address == 0)) {
		pr_err("%s fails: ioremap_nocache fail\n", __FUNCTION__);
		mutex_unlock(&mmc_lock);
		return -1;
	}

	do {
		status = readl(mmc_address + SDIO_STATUS_REG_OFS);
		if (status & DATA_END) {
			mmc_ret = 1;
			break;
		} else if (status & (RSP_CRC_FAIL | DATA_CRC_FAIL | RSP_TIMEOUT | DATA_TIMEOUT | DMA_ERROR)) {
			pr_err("%s fails: transfer error, status(0x%x)\n", __FUNCTION__, status);
			mmc_ret = 0;
			break;
		}
		usleep_range(1, 2);
	} while (1);

	mutex_unlock(&mmc_lock);

	iounmap(mmc_address);

	return mmc_ret;
}
EXPORT_SYMBOL(nvt_mmc_check_preload_finish);

