#include <linux/soc/nvt/rcw_macro.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <plat/hardware.h>

#include "plat/i2c_reg.h"

static DEFINE_MUTEX(flash_lock);
#define NAND_CTRL_STS_REG_OFS	0x48
#define NAND_COMP_STS           0x00001000  // bit[12]
#define NAND_SPI_CFG_REG_OFS	0x14
#define NAND_SPI_CS_MODE        0x4
#define NAND_SPI_OP_MODE        0x1

static DEFINE_MUTEX(mmc_lock);
#define SDIO_STATUS_REG_OFS     0x28
#define RSP_CRC_FAIL            0x1
#define DATA_CRC_FAIL           0x2
#define RSP_TIMEOUT             0x4
#define DATA_TIMEOUT            0x8
#define DATA_END                0x80
#define DMA_ERROR               0x80000

static DEFINE_MUTEX(i2c_lock);
#define I2C_CTRL_STS_REG_OFS	0x04
#define I2C_BERR_STS			0x40
#define I2C_AL_STS				0x400
#define I2C_DMAED				0x800
#define I2C_SCL_TIMEOUT_STS		0x1000
#define I2C_DMA_DESC_ERR_STS    0x8000

int nvt_check_preload_finish(void)
{
	void __iomem *flash_address;
	static int ret = 0;
	u32 status = 0x0, io_reg = 0x0;

	mutex_lock(&flash_lock);
	if (ret) {
		mutex_unlock(&flash_lock);
		return ret;
	}

	flash_address = ioremap(NVT_NAND_BASE_PHYS, 0x200);
	if (unlikely(flash_address == 0)) {
		pr_err("%s fails: ioremap fail\n", __FUNCTION__);
		mutex_unlock(&flash_lock);
		return -1;
	}

	do {
		status = readl(flash_address + NAND_CTRL_STS_REG_OFS);
		if (status & NAND_COMP_STS) {
			writel(status, flash_address + NAND_CTRL_STS_REG_OFS);

			// set cs pin to high
			io_reg = readl(flash_address + NAND_SPI_CFG_REG_OFS);
			io_reg |= (0x1 << NAND_SPI_CS_MODE);
			writel(io_reg, flash_address + NAND_SPI_CFG_REG_OFS);

			// reset cs pin to auto mode
			io_reg = readl(flash_address + NAND_SPI_CFG_REG_OFS);
			io_reg &=~ (0x1 << NAND_SPI_OP_MODE);
			writel(io_reg, flash_address + NAND_SPI_CFG_REG_OFS);

			ret = 1;

			break;
		}
		usleep_range(1, 2);
	} while (1);

	mutex_unlock(&flash_lock);

	iounmap(flash_address);

	return ret;
}
EXPORT_SYMBOL(nvt_check_preload_finish);

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

	mmc_address = ioremap(NVT_SDIO3_BASE_PHYS, 0x1000);
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
