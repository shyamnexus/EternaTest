#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <linux/of_device.h>
#include <linux/bits.h>

#include "comm/gyro_spi.h"
#include "gyro_spi_drv.h"
#include "gyro_spi_dbg.h"

#if (NVT_GYRO_528_PINMUX == ENABLE || NVT_GYRO_530_PINMUX == ENABLE)
#include <rcw_macro.h>
#endif


/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
irqreturn_t nvt_gyro_spi_drv_isr(int irq, void *devid);
void nvt_gyro_spi_linux_act_handler(SPI_LINUX_ACT spiAct, void *param);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
struct clk *nvt_gyro_spi_clk = NULL;
static SPI_ID gTestSpiID = SPI_ID_3;
static volatile BOOL bSpiGyroDtRdy = FALSE;
static volatile UINT32 uiSpiGyroRunState = 0;   // 0: idle, 1: run, 2: stop, 0x8000: buffer over run, 0x8001: seq err
static UINT32 vRecvBuf[2][32 * 2];


/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/

int nvt_gyro_spi_drv_open(GYRO_MODULE_INFO *pmodule_info, unsigned char ucIF)
{
	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}


int nvt_gyro_spi_drv_release(GYRO_MODULE_INFO *pmodule_info, unsigned char ucIF)
{
	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

#if (NVT_GYRO_528_PINMUX == ENABLE)
void nvt_gyro_spi_drv_pinmux_setting(void)
{
    void __iomem *reg_addr;
    UINT32 tmp_reg;

    // set TOP reg
    reg_addr = ioremap(TOP_REG_ADDR, 0x200);

    // disable SDIO
    tmp_reg = INW(reg_addr + 0x04);
    tmp_reg &= ~(BIT(14));
    OUTW(reg_addr + 0x04, tmp_reg);

    // SPI3_1: 0x14[13:12] = 0x1, SPI3_2: 0x14[13:12] = 0x2
    // 1-bit full duplex: 0x14[18] = 1
    tmp_reg = INW(reg_addr + 0x14);
    tmp_reg &= ~(BIT(13) | BIT(12));
    tmp_reg |= (NVT_GYRO_528_PINMUX_SEL << 12);
    tmp_reg |= BIT(18);
    OUTW(reg_addr + 0x14, tmp_reg);

    #if (NVT_GYRO_528_PINMUX_SEL == 1)
    // set P_GPIO17 ~ P_GPIO20 to normal functionality
    tmp_reg = INW(reg_addr + 0xA8);
    tmp_reg &= ~(BIT(20) | BIT(19) | BIT(18) | BIT(17));
    OUTW(reg_addr + 0xA8, tmp_reg);
    #elif (NVT_GYRO_528_PINMUX_SEL == 2)
    // set C_GPIO11 ~ C_GPIO14 to normal functionality
    tmp_reg = INW(reg_addr + 0xA0);
    tmp_reg &= ~(BIT(14) | BIT(13) | BIT(12) | BIT(11));
    OUTW(reg_addr + 0xA0, tmp_reg);
    #else
    #error, NVT_GYRO_528_PINMUX_SEL unexcepted value
    #endif

    DBG_IND("top 0x04: 0x%08X, 0x14: 0x%08X, 0xA0: 0x%08X, 0xA8: 0x%08X\n",
            INW(reg_addr + 0x04), INW(reg_addr + 0x14),
            INW(reg_addr + 0xA0), INW(reg_addr + 0xA8));

    iounmap(reg_addr);

    #if (NVT_GYRO_528_PINMUX_SEL == 2)
    // set PAD reg
    reg_addr = ioremap(PAD_REG_ADDR, 0x200);

    // PAD_MC_1_REGULATOR_EN = 1
    tmp_reg = INW(reg_addr + 0x74);
    tmp_reg |= BIT(4);
    OUTW(reg_addr + 0x74, tmp_reg);

    iounmap(reg_addr);
    #endif
}
#elif (NVT_GYRO_530_PINMUX == ENABLE)
void nvt_gyro_spi_drv_pinmux_setting(void)
{
    void __iomem *reg_addr;
    UINT32 tmp_reg;

    // set TOP reg
    reg_addr = ioremap(TOP_REG_ADDR, 0x200);

    // check pinmux conflict
    #if (NVT_GYRO_530_PINMUX_SEL == 1)
    tmp_reg = INW(reg_addr + 0x34);
    if(((tmp_reg >> 12) & 0xF) == 0x1) { 
        DBG_ERR("Conflict: Make sure disable UART4_1\r\n");
    }

    tmp_reg = INW(reg_addr + 0x10);
    if(((tmp_reg >> 24) & 0xF) == 0x1) { 
        DBG_ERR("Conflict: Make sure disable I2C7_1\r\n");
    }

    tmp_reg = INW(reg_addr + 0x10);
    if(((tmp_reg >> 28) & 0xF) == 0x1) {
        DBG_ERR("Conflict: Make sure disable I2C8_1\r\n");
    }

    tmp_reg = INW(reg_addr + 0x48);
    if(((tmp_reg >> 8) & 0xF) == 0x1) {
        DBG_ERR("Conflict: Make sure disable SIF2_1\r\n");
    }

    tmp_reg = INW(reg_addr + 0x1C);
    if(((tmp_reg >> 16) & 0xF) == 0x1) {
        DBG_ERR("Conflict: Make sure disable PICNT\r\n");
    }
    
    tmp_reg = INW(reg_addr + 0x1C);
    if(((tmp_reg >> 20) & 0xF) == 0x1) {
        DBG_ERR("Conflict: Make sure disable PICNT2\r\n");
    }
    
    tmp_reg = INW(reg_addr + 0x1C);
    if(((tmp_reg >> 24) & 0xF) == 0x1) {
        DBG_ERR("Conflict: Make sure disable PICNT3\r\n");
    }

    tmp_reg = INW(reg_addr + 0x28);
    if(((tmp_reg >> 16) & 0x3) == 0x2) {
        DBG_ERR("Conflict: Make sure disable SN5_MCLK=2\r\n");
    }

    tmp_reg = INW(reg_addr + 0x50);
    if(((tmp_reg >> 19) & 0x1) == 0x1) {
        DBG_ERR("Conflict: Make sure disable RTC_DIV_OUT\r\n");
    }

    tmp_reg = INW(reg_addr + 0x50);
    if(((tmp_reg >> 18) & 0x1) == 0x1) {
        DBG_ERR("Conflict: Make sure disable RTC_EXT_CLK\r\n");
    }

    #elif (NVT_GYRO_530_PINMUX_SEL == 2)
    tmp_reg = INW(reg_addr + 0x04);
    if(((tmp_reg >> 8) & 0x1) == 0x1) {
        DBG_ERR("Conflict: Make sure disable SDIO2\r\n");
    }

    tmp_reg = INW(reg_addr + 0x34);
    if(((tmp_reg >> 28) & 0xF) == 0x2) {
        DBG_ERR("Conflict: Make sure disable UART8_2\r\n");
    }

    tmp_reg = INW(reg_addr + 0x30);
    if(((tmp_reg >> 0) & 0x3) == 0x2) {
        DBG_ERR("Conflict: Make sure disable I2S=2\r\n");
    }

    tmp_reg = INW(reg_addr + 0x20);
    if(((tmp_reg >> 8) & 0x1) == 0x1) {
        DBG_ERR("Conflict: Make sure disable DSP_EJTAG\r\n");
    }

    tmp_reg = INW(reg_addr + 0x40);
    if(((tmp_reg >> 16) & 0x3) == 0x2) {
        DBG_ERR("Conflict: Make sure disable SDP=2\r\n");
    }
    #endif

    DBG_IND("top 0x44: 0x%08X, 0xA0: 0x%08X, 0xA8: 0x%08X\n",
            INW(reg_addr + 0x44), INW(reg_addr + 0xA0), INW(reg_addr + 0xA8));
    iounmap(reg_addr);

}
#endif

int nvt_gyro_spi_drv_init(GYRO_MODULE_INFO *pmodule_info)
{
	int iRet = 0;

	init_waitqueue_head(&pmodule_info->gyro_wait_queue);

	sema_init(&pmodule_info->gyro_sem, 1);

	/* initial clock here */
    clk_set_rate(pmodule_info->pclk[0], pmodule_info->clock_rate);
	clk_prepare(pmodule_info->pclk[0]);
	clk_enable(pmodule_info->pclk[0]);
    
	nvt_gyro_spi_clk = pmodule_info->pclk[0];

	/* register IRQ here*/
	// gyro spi3
	if(request_irq(pmodule_info->intr_id[0], nvt_gyro_spi_drv_isr, IRQF_TRIGGER_HIGH, "GYRO_INT", pmodule_info)) {
		DBG_ERR("failed to register an IRQ Int:%d\n", pmodule_info->intr_id[0]);
		iRet = -ENODEV;
		goto FAIL_FREE_IRQ;
    }

	/* Add HW Module initialization here when driver loaded */
    spi_platform_init(nvt_gyro_spi_linux_act_handler);


    spi_setRegAddr(SPI_ID_3, pmodule_info->io_addr[0]);

	return iRet;

FAIL_FREE_IRQ:

	free_irq(pmodule_info->intr_id[0], pmodule_info);

	/* Add error handler here */

	return iRet;
}

int nvt_gyro_spi_drv_remove(GYRO_MODULE_INFO *pmodule_info)
{
    spi_platform_uninit();

	//Free IRQ
	free_irq(pmodule_info->intr_id[0], pmodule_info);

	/* Add HW Moduel release operation here*/
	clk_disable(pmodule_info->pclk[0]);
	clk_unprepare(pmodule_info->pclk[0]);

    // TODO: implement gyro.c to reset SPI3 register
    //gyro_reset();

	return 0;
}

int nvt_gyro_spi_drv_suspend(GYRO_MODULE_INFO *pmodule_info)
{
	/* Add suspend operation here*/

	return 0;
}

int nvt_gyro_spi_drv_resume(GYRO_MODULE_INFO *pmodule_info)
{
	/* Add resume operation here*/

	return 0;
}

irqreturn_t nvt_gyro_spi_drv_isr(int irq, void *devid)
{
    spi3_isr();
    return IRQ_HANDLED;
}

#if 0
irqreturn_t nvt_spi_drv_isr(int irq, void *devid)
{
    spi_isr();
    return IRQ_HANDLED;
}

irqreturn_t nvt_spi2_drv_isr(int irq, void *devid)
{
    spi2_isr();
    return IRQ_HANDLED;
}

irqreturn_t nvt_spi4_drv_isr(int irq, void *devid)
{
    spi4_isr();
    return IRQ_HANDLED;
}

irqreturn_t nvt_spi5_drv_isr(int irq, void *devid)
{
    spi5_isr();
    return IRQ_HANDLED;
}
#endif


void nvt_gyro_spi_linux_act_handler(SPI_LINUX_ACT spi_act, void *param)
{
    switch (spi_act) {
        case SPI_LINUX_ACT_CLK_SET:
            {
                UINT32 clk_rate = *(UINT32 *)param;
                if (nvt_gyro_spi_clk) {
                    clk_set_rate(nvt_gyro_spi_clk, clk_rate);
                }
            }
            break;
        default:
            break;
    }
}


static void emu_gyroHdl(SPI_GYRO_INT uiIntSts)
{
	if (uiIntSts & SPI_GYRO_INT_SYNC_END) {
		bSpiGyroDtRdy = TRUE;
	}

	if (uiIntSts & SPI_GYRO_INT_CHANGE_END) {
		bSpiGyroDtRdy = TRUE;
	}

	if (uiIntSts & SPI_GYRO_INT_SEQ_ERR) {
		uiSpiGyroRunState = 0x8001;
		bSpiGyroDtRdy = FALSE; //2022/06/17
		DBG_IND("Seq ERR\n");
	}

	if (uiIntSts & SPI_GYRO_INT_OVERRUN) {
		bSpiGyroDtRdy = FALSE; //2022/06/17
		uiSpiGyroRunState = 0x8002;
		DBG_IND("FIFO overrun\n");
	}

	if (uiIntSts & SPI_GYRO_INT_TRS_TIMEOUT) {
		uiSpiGyroRunState = 0x8003;
		bSpiGyroDtRdy = FALSE; //2022/06/17
		DBG_IND("Transfer TIMEOUT\n");
	}

	if (uiIntSts & SPI_GYRO_INT_QUEUE_THRESHOLD) {
        DBG_IND("Queue exceed threshold\n");
		uiSpiGyroRunState = 0x8004;
		bSpiGyroDtRdy = FALSE; //2022/06/17
	}

    if (uiIntSts & SPI_GYRO_INT_QUEUE_OVERRUN) {
        DBG_IND("Queue overrun\n");
		uiSpiGyroRunState = 0x8005;
		bSpiGyroDtRdy = FALSE; //2022/06/17
	}
}

static int selection = 1;
void emu_gyroAuto(UINT32 cur_clk_rate)
{
	UINT32 i = 0;
	UINT32 uiIntMask = 0;
	UINT32 uiStartIdx = 0;
	SPI_GYRO_INFO gyroInfo = {0};
	UINT32 uiQueueDepth = 0;

    DBG_WRN("START GYRO AUTO!!!! ensure that you have connect to slave device\n");
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_BUSMODE, SPI_MODE_0);
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_MSB_LSB, SPI_MSB);
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_WIDE_BUS_ORDER, SPI_WIDE_BUS_ORDER_NORMAL);
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_CS_CK_DLY, 1);
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_ENG_GYRO_UNIT, TRUE);   // enable engineer emulation mode
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_CS_ACT_LEVEL, SPI_CS_ACT_LEVEL_LOW);
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_GYRO_SYNC_END_OFFSET, 0);
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_PKT_DLY, 0);
	spi_setConfig(gTestSpiID, SPI_CONFIG_ID_VD_SRC, SPI_VD_SRC_SIE5);

    // just notify clock rate to gyro host layer
    // gyro host layer calculate timing parameter (e.x. op_interval, transfer_interval)
    spi_setConfig(gTestSpiID, SPI_CONFIG_ID_FREQ, cur_clk_rate);

	if(selection == 0) {
		gyroInfo.gyroMode = SPI_GYRO_MODE_ONE_SHOT;
	} else if(selection == 1) {
		gyroInfo.gyroMode = SPI_GYRO_MODE_FREE_RUN;
	} else if(selection == 2) {
		gyroInfo.gyroMode = SPI_GYRO_MODE_SIE_SYNC;
	}

	gyroInfo.uiTransferCount = 32;
	gyroInfo.uiTransferLen = 4;         // 4 OP per transfer

	gyroInfo.uiOpInterval = 0x8;
	gyroInfo.uiTransferInterval = 0x800;

	gyroInfo.uiOp0Length = 8;
	gyroInfo.vOp0OutData[0] = 0x9F;
	for (i = 1; i < gyroInfo.uiOp0Length; i++) {
		gyroInfo.vOp0OutData[i] = 0;
	}

	gyroInfo.uiOp1Length = 8;
	gyroInfo.vOp1OutData[0] = 0x03;
	gyroInfo.vOp1OutData[1] = 0x00;
	gyroInfo.vOp1OutData[2] = 0x00;
	gyroInfo.vOp1OutData[3] = 0x00;
	for (i = 4; i < gyroInfo.uiOp1Length; i++) {
		gyroInfo.vOp1OutData[i] = 0;
	}

	gyroInfo.uiOp2Length = 8;
	gyroInfo.vOp2OutData[0] = 0x04;
	gyroInfo.vOp2OutData[1] = 0x00;
	gyroInfo.vOp2OutData[2] = 0x00;
	gyroInfo.vOp2OutData[3] = 0x00;
	for (i = 4; i < gyroInfo.uiOp2Length; i++) {
		gyroInfo.vOp2OutData[i] = 0;
	}

	gyroInfo.uiOp3Length = 8;
	gyroInfo.vOp3OutData[0] = 0x05;
	gyroInfo.vOp3OutData[1] = 0x00;
	gyroInfo.vOp3OutData[2] = 0x00;
	gyroInfo.vOp3OutData[3] = 0x00;
	for (i = 4; i < gyroInfo.uiOp3Length; i++) {
		gyroInfo.vOp3OutData[i] = 0;
	}

	for (i = 0; i < gyroInfo.uiTransferCount; i++) {
		uiIntMask |= 1 << i;
	}
    DBG_WRN("OP intEn will be 0x%x\n", uiIntMask);
	spi_setConfig(gTestSpiID, SPI_CONFIG_ID_ENG_GYRO_INTMSK, uiIntMask);

	gyroInfo.pEventHandler = emu_gyroHdl;

	spi_open(gTestSpiID);

	uiSpiGyroRunState = 1;
	bSpiGyroDtRdy = FALSE;
	spi_startGyro(gTestSpiID, &gyroInfo);
	while (1) {
		if ((uiSpiGyroRunState == 2) && (bSpiGyroDtRdy == FALSE)) {
			uiSpiGyroRunState = 1;
		}

		if (uiSpiGyroRunState == 0x8001) {
            DBG_WRN("Seq ERR\n");
			break;
		}

		if (uiSpiGyroRunState == 0x8002) {
            DBG_WRN("FIFO overrun ERR\n");
			break;
		}
		
		if (uiSpiGyroRunState == 0x8003) {
            DBG_WRN("Transfer timeout ERR\n");
			break;
		}
		
		if (uiSpiGyroRunState == 0x8004) {
            DBG_WRN("Queue execeed threshold ERR\n");
			break;
		}

		if (uiSpiGyroRunState == 0x8005) {
            DBG_WRN("Queue overrun ERR\n");
			break;
		}

		if (bSpiGyroDtRdy == TRUE) {
			uiQueueDepth = spi_getGyroQueueCount(gTestSpiID);
			bSpiGyroDtRdy = FALSE;

			for (i = 0; i < uiQueueDepth; i++)
			{
				GYRO_BUF_QUEUE gyroData;
				UINT32 uiNextWord;

				spi_getGyroData(gTestSpiID, &gyroData);
				uiNextWord = (uiStartIdx & 0xFF) |
							 (((uiStartIdx + 1) << 8) & 0xFF00) |
							 (((uiStartIdx + 2) << 16) & 0xFF0000) |
							 (((uiStartIdx + 3) << 24) & 0xFF000000UL);
				vRecvBuf[0][i * 2] = gyroData.vRecvWord[0];
				if (uiNextWord != vRecvBuf[0][i * 2]) {
                    DBG_WRN("Expect 0x%x, but 0x%x\n", uiNextWord, vRecvBuf[0][i * 2]);
				}
				uiStartIdx += 4;

				uiNextWord = (uiStartIdx & 0xFF) |
							 (((uiStartIdx + 1) << 8) & 0xFF00) |
							 (((uiStartIdx + 2) << 16) & 0xFF0000) |
							 (((uiStartIdx + 3) << 24) & 0xFF000000UL);
				vRecvBuf[0][i * 2 + 1] = gyroData.vRecvWord[1];
				if (uiNextWord != vRecvBuf[0][i * 2 + 1]) {
                    DBG_WRN("Expect 0x%x, but 0x%x\n", uiNextWord, vRecvBuf[0][i * 2 + 1]);
				}
				uiStartIdx += 4;
			}

            DBG_WRN(".");
		}
	}

	spi_stopGyro(gTestSpiID);
	if (uiSpiGyroRunState == 0x8001) {
        DBG_ERR("Seq ERR\n");
	}
    DBG_WRN("%s: exit\n", __func__);
	uiSpiGyroRunState = 0;

	spi_close(gTestSpiID);
}