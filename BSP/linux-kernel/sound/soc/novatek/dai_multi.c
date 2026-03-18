/*
    Digital Audio Interface(DAI) module driver

    @file       dai.c
    @ingroup    mIDrvAud_DAI
    @brief      DAI module driver
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

*/
#ifdef __KERNEL__
#include <linux/soc/nvt/rcw_macro.h>
#include <linux/clk.h>
#include "dai_reg.h"
#include "dai_int.h"

static DEFINE_SPINLOCK(my_lock);
#define loc_cpu(myflags)   spin_lock_irqsave(&my_lock, myflags)
#define unl_cpu(myflags)   spin_unlock_irqrestore(&my_lock, myflags)


#define DBG_WRN(fmt, args...) printk(fmt, ##args)
#define DBG_ERR(fmt, args...) printk(fmt, ##args)

void __iomem *_DAI_REG_BASE_ADDR[1];

#define OS_CONFIG_FLAG(x)
#define SEM_CREATE(x, y)
#define rel_flg(x)
#define SEM_DESTROY(x)
#define iset_flg(x,y)
#define SEM_WAIT(x) 0
#define clr_flg(x,y)
#define wai_flg(a,b,c,d)
#define SEM_SIGNAL(x)
#define FLGPTN UINT32


#define DAI_ID_1    0
#define DAI_ID_2    1
#define DAI_ID_3    2
#define DAI_ID_4    3
#define DAI_ID_5    4


char *clk_dai_name[5] = {"clk_dai", "clk_dai2", "clk_dai3", "clk_dai4", "clk_dai5"};

#elif defined(__FREERTOS)

#define __MODULE__ rtos_dai
#define __DBGLVL__ 8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__ "*"

#include "kwrap/error_no.h"
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/debug.h"
#include "kwrap/task.h"
#include "kwrap/spinlock.h"
#include "kwrap/type.h"

#include "include/dai.h"
#include "include/dai_reg.h"
#include "include/dai_int.h"
#include "pll_protected.h"
#include "dma_protected.h"
#include "cache_protected.h"
#include "io_address.h"

static ID	     FLG_ID_DAI;
static SEM_HANDLE SEMID_DAI;

static  VK_DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)

unsigned int rtos_dai_debug_level = NVT_DBG_WRN;

static BOOL rtos_init = 0;


#endif
/**
    @addtogroup mIDrvAud_DAI
*/
//@{

#ifdef __KERNEL__
void dai_create_resource(void)
{
	OS_CONFIG_FLAG(FLG_ID_DAI);
	SEM_CREATE(SEMID_DAI, 1);
}

void dai_release_resource(void)
{
	rel_flg(FLG_ID_DAI);
	SEM_DESTROY(SEMID_DAI);
}
#endif

// enable clk and pinmux

void _dai_enableclk_platform(int dai_id)
{
	struct clk *dai_clk;

	dai_clk = clk_get(NULL, clk_dai_name[dai_id]);
	if (IS_ERR(dai_clk)) {
		DBG_ERR("failed to get dai clk\n");
        return;
	}
    clk_prepare(dai_clk);

	clk_enable(dai_clk);

	clk_put(dai_clk);
}



void _dai_disableclk_platform(int dai_id)
{
	struct clk *dai_clk;

	dai_clk = clk_get(NULL, clk_dai_name[dai_id]);
	if (IS_ERR(dai_clk)) {
		DBG_ERR("failed to get dai clk\n");
        return;
	}
	clk_disable(dai_clk);
	clk_put(dai_clk);
}



void _dai_setclkrate_platform(unsigned long clkrate, int dai_id)
{
	struct clk *dai_clk;

	dai_clk = clk_get(NULL, clk_dai_name[dai_id]);
	if (IS_ERR(dai_clk)) {
		DBG_ERR("failed to get dai clk\n");
        return;
	}
	clk_set_rate(dai_clk, clkrate);
	clk_put(dai_clk);
}


UINT64 dai_platform_va2pa(uintptr_t addr)
{
#ifdef __KERNEL__
	uintptr_t paddr;
	paddr = (uintptr_t)fmem_lookup_pa(addr);
	return paddr;
#else
	SYS_PA pa;

	pa = sys_va2pa(addr);
	return pa.addr64;
#endif
}

UINT64 dai2_platform_va2pa(uintptr_t addr)
{
 #ifdef __KERNEL__
    uintptr_t paddr;
    paddr = (uintptr_t)addr;//fmem_lookup_pa(addr);
    return paddr;

 #else
    SYS_PA pa;
    pa = sys_va2pa(addr);
    return pa.addr64;
#endif
}


#ifdef __KERNEL__
void dai_enableclk(int dai_id)
{
	_dai_enableclk_platform(dai_id);
}

void dai_disableclk(int dai_id)
{
	_dai_disableclk_platform(dai_id);
}

void dai_setclkrate(unsigned long clkrate, int dai_id)
{
	_dai_setclkrate_platform(clkrate, dai_id);
}
#else
void dai_enableclk(void)
{
	pll_enableClock(DAI_CLK);
}

void dai_disableclk(void)
{
	pll_disableClock(DAI_CLK);
}

void dai_setclkrate(unsigned long clkrate)
{

}
#endif

static DRV_CB   dai_cb_funciton;

static BOOL     b_dai_opened  = FALSE;

static UINT32	size_error[2] = {0, 0};

//static UINT32   dai_pinmux_flag = PIN_AUDIO_CFG_NONE;

static UINT32 dai_tasklet_event;
void dai_tasklet(void)
{
	UINT32 evt;
	unsigned long flag;

	if (!b_dai_opened) {
		return;
	}

	loc_cpu(flag);
	evt = dai_tasklet_event;
	dai_tasklet_event = 0;
	unl_cpu(flag);

	if (dai_cb_funciton != NULL) {
		dai_cb_funciton(evt);
	}
}


/*
    DAI ISt

    It's DAI Interrupt Service Routine

    @param void
    @return void
*/
#if defined(__FREERTOS)
irq_bh_handler_t dai_bh_ist(int irq, unsigned long event, void *data)
{
	dai_tasklet();

	return (irq_bh_handler_t) IRQ_HANDLED;
}
#endif


/*
    DAI ISR

    It's DAI Interrupt Service Routine

    @param void
    @return void
*/
#if defined (__FREERTOS)
irqreturn_t dai_isr(int irq, void *devid)
#else
void dai_isr(void)
#endif
{
	T_DAI_INTSTATUS_REG     reg_status;
	T_DAI_INTSTATUS2_REG    reg_status_2;

#ifdef __KERNEL__
	if (!b_dai_opened) {
		return;
	}
#endif

	//DBG_IND("\r\n");

	// Get interrupt status
	reg_status.reg = DAI_GETREG(DAI_INTSTATUS_REG_OFS);
	reg_status_2.reg = DAI_GETREG(DAI_INTSTATUS2_REG_OFS);

	// Only handle interrupts which are enabled
	reg_status.reg  &= DAI_GETREG(DAI_INTEN_REG_OFS);
	reg_status_2.reg &= DAI_GETREG(DAI_INTEN2_REG_OFS);


	if ((reg_status.reg == 0) && (reg_status_2.reg == 0)) {
#if defined(__FREERTOS)
		return IRQ_NONE;
#else
		return;
#endif
	}


	// Clear interrupt status
	DAI_SETREG(DAI_INTSTATUS_REG_OFS,  reg_status.reg);
	DAI_SETREG(DAI_INTSTATUS2_REG_OFS, reg_status_2.reg);


	// INT2 remap
	if (reg_status_2.bit.TXLB_DMABFI) {
		reg_status.reg |= DAI_TXLBDMADONE_INT;
	}
	if (reg_status_2.bit.TXLB_DMALOAD) {
		reg_status.reg |= DAI_TXLBDMALOAD_INT;
	}
	if (reg_status_2.bit.TXLB_STOP) {
		reg_status.reg |= DAI_TXLBSTOP_INT;
	}
	if (reg_status_2.bit.TXLB_BWERR) {
		reg_status.reg |= DAI_TXLBBWERR_INT;
	}

	// Call the isr handler

	dai_tasklet_event |= reg_status.reg;
	#if defined(__FREERTOS)
	kick_bh(INT_ID_DAI, reg_status.reg, NULL);
	#endif

	// Set DAI Flag
	iset_flg(FLG_ID_DAI, reg_status.reg);


#if defined(__FREERTOS)
		return IRQ_HANDLED;
#endif

}

#if 1

/*
    Lock DAI module

    Use semaphore lock for the DAI module

    @return
	@b E_OK: success
	@b Else: fail
*/
ER dai_lock(void)
{
	ER er_ret;

	er_ret        = SEM_WAIT(SEMID_DAI);
	if (er_ret != E_OK) {
		DBG_ERR("wait semaphore fail\r\n");
		return er_ret;
	}

	return E_OK;
}

/*
    Unlock DAI module

    Release semaphore lock for the DAI module

    @return
	@b E_OK: success
	@b Else: fail
*/
ER dai_unlock(void)
{
	SEM_SIGNAL(SEMID_DAI);
	return E_OK;
}

/*
    Wait DAI Interrupt Event
*/
DAI_INTERRUPT dai_wait_interrupt(DAI_INTERRUPT waited_flag)
{
	FLGPTN              ui_flag = 0;

	wai_flg(&ui_flag, FLG_ID_DAI, waited_flag, TWF_ORW | TWF_CLR);

	return ui_flag;
}

/*
    Select DAI I2S pinmux

    @return void
*/
void dai_select_pinmux(BOOL b_en)
{/*
	PIN_GROUP_CONFIG pinmux_i2s[1];
	int				 ret = 0;
	unsigned long flag;

	pinmux_i2s->pin_function = PIN_FUNC_AUDIO;


	if (b_en) {
		loc_cpu(flag);
		dai_pinmux_flag |= PIN_AUDIO_CFG_I2S;
		unl_cpu(flag);
	} else {
		loc_cpu(flag);
		dai_pinmux_flag &= ~PIN_AUDIO_CFG_I2S;
		unl_cpu(flag);
	}

	pinmux_i2s->config = dai_pinmux_flag;

	ret = nvt_pinmux_update(pinmux_i2s, 1);

	if (ret) {
		DBG_WRN("pinmux_mclk update error! \r\n");
	}*/
}


/*
    Select DAI I2S MCLK pinmux

    @return void
*/
void dai_select_mclk_pinmux(BOOL b_en)
{/*
	PIN_GROUP_CONFIG pinmux_mclk[1];
	int				 ret = 0;
	unsigned long flag;

	pinmux_mclk->pin_function = PIN_FUNC_AUDIO;


	if (b_en) {
		loc_cpu(flag);
		dai_pinmux_flag |= PIN_AUDIO_CFG_MCLK;
		unl_cpu(flag);
	} else {
		loc_cpu(flag);
		dai_pinmux_flag &= ~PIN_AUDIO_CFG_MCLK;
		unl_cpu(flag);
	}

	pinmux_mclk->config = dai_pinmux_flag;

	ret = nvt_pinmux_update(pinmux_mclk, 1);

	if (ret) {
		DBG_WRN("pinmux_mclk update error! \r\n");
	}
*/

}

#endif
#if 1

/**
    Open digital audio controller driver.

    Open digital auiod controller.

    @param[in] p_isr_handler      Callback function registered for interrupt notification.

    @return void
*/
void dai_open(DRV_CB p_isr_handler, int dai_id)
{

#if defined(__FREERTOS)
		if (!rtos_init) {
			rtos_init = 1;
			cre_flg(&FLG_ID_DAI, NULL, "FLG_ID_DAI");
			vos_sem_create(&SEMID_DAI, 1, "SEMID_DAI");
		}
#else
	dai_create_resource();
#endif
	if (b_dai_opened) {
		return;
	}

	// log driver opened
	b_dai_opened = TRUE;

	//set dai interrupt handler

	dai_cb_funciton  = p_isr_handler;

	// Clear Interrupt Flag
	clr_flg(FLG_ID_DAI, DAI_INTERRUPT_ALL);

	// Disable DAI Reset
	//pll_disableSystemReset(DAI_RSTN);

	// Enable DAI clock
	dai_enableclk(dai_id);


	dai_set_config(DAI_CONFIG_ID_STADR2_EN , 0);

	dai_tasklet_event = 0;
#if defined(__FREERTOS)
	// Enable dai interrupt
	request_irq(INT_ID_DAI, dai_isr ,IRQF_TRIGGER_HIGH, "dai", 0);
	request_irq_bh(INT_ID_DAI, (irq_bh_handler_t) dai_bh_ist, IRQF_BH_PRI_HIGH);
#endif


}


/**
    Close digital audio controller driver.

    Close digital auiod controller.

    @return void
*/
void dai_close(int dai_id)
{
	if (!b_dai_opened) {
		return;
	}

	// Disable dai interrupt
	//drv_disableInt(DRV_INT_DAI);linux no need

	// Disable DAI clock
	dai_disableclk(dai_id);
#if defined(__FREERTOS)
	rtos_init = 0;
	rel_flg(FLG_ID_DAI);
	vos_sem_destroy(SEMID_DAI);

#else
	dai_release_resource();
#endif
	//clear dai interrupt handler

	dai_cb_funciton  = NULL;


	// log driver closed
	b_dai_opened = FALSE;
}

/**
    Set Digital Audio interface(DAI) General Configurations.

    Set Digital Audio interface(DAI) general configuration.
    Use DAI_CONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_CONFIG_ID for details.
    @param[in] config_value   configuration parameter. Please refer to DAI_CONFIG_ID for details.

    @return void
*/
void dai_set_config(DAI_CONFIG_ID config_id, uintptr_t config_value)
{
	T_DAI_CONFIG_REG reg_ctrl;
	unsigned long flag;

	loc_cpu(flag);

	reg_ctrl.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);

	switch (config_id) {
	case DAI_CONFIG_ID_EXTCODEC_EN: {
			if (config_value == TRUE) {
				reg_ctrl.bit.EXCODEC_EN = 1;
			} else {
				reg_ctrl.bit.EXCODEC_EN = 0;

				// embedded codec fix using CH0/1
				reg_ctrl.bit.HDMI_CH_SEL = DAI_I2SHDMI_SEL_CH01;
			}
		}
		break;

	case DAI_CONFIG_ID_ISRCB: {
			if (!b_dai_opened) {
				break;
			}

			//set dai interrupt handler
			dai_cb_funciton  = (DRV_CB)config_value;

		}
		break;

	case DAI_CONFIG_ID_HDMI_TXEN: {
			if (config_value == TRUE) {
				reg_ctrl.bit.TX_MUX_SEL = 1;     // playback to EAC/HDMI
			} else {
				reg_ctrl.bit.TX_MUX_SEL = 0;     // playback to EAC only
			}
		}
		break;

	case DAI_CONFIG_ID_AVSYNC_EN: {
			if (config_value == TRUE) {
				reg_ctrl.bit.AVSYNC = 1;
			} else {
				reg_ctrl.bit.AVSYNC = 0;
			}
		}
		break;

	case DAI_CONFIG_ID_AVSYNC_SRC: {
			reg_ctrl.bit.AVSYNC_SRC = config_value;
		}
		break;

	case DAI_CONFIG_ID_SET_INTEN_MUTIDONE:{
			T_DAI_INTEN3_REG    reg_int_en_3;
			T_DAI_INTEN4_REG    reg_int_en_4;

			if (config_value & DAI_CH_ALL_DMADONE_INT){
				reg_int_en_3.reg = DAI_GETREG(DAI_INTEN3_REG_OFS);
				reg_int_en_4.reg = DAI_GETREG(DAI_INTEN4_REG_OFS);

				reg_int_en_3.reg |= (config_value&0x1FFFFF);
				reg_int_en_4.reg |= (config_value&0x1FFFFF);

				DAI_SETREG(DAI_INTEN3_REG_OFS, reg_int_en_3.reg);
				DAI_SETREG(DAI_INTEN4_REG_OFS, reg_int_en_4.reg);
			}
		}
		break;

	case DAI_CONFIG_ID_CLR_INTEN_MUTILOAD:{
			T_DAI_INTEN3_REG    reg_int_en_3;

			if (config_value & DAI_CH_ALL_DMADONE_INT){
				reg_int_en_3.reg = DAI_GETREG(DAI_INTEN3_REG_OFS);

				reg_int_en_3.reg &= ~(config_value & DAI_CH_ALL_DMADONE_INT);
				DAI_SETREG(DAI_INTEN3_REG_OFS, reg_int_en_3.reg);
			}
		}
		break;

	case DAI_CONFIG_ID_CLR_INTEN_MUTIDONE:{
			T_DAI_INTEN4_REG    reg_int_en_4;

			if (config_value & DAI_CH_ALL_DMADONE_INT){
				reg_int_en_4.reg = DAI_GETREG(DAI_INTEN4_REG_OFS);

				reg_int_en_4.reg &= ~(config_value & DAI_CH_ALL_DMADONE_INT);
				DAI_SETREG(DAI_INTEN4_REG_OFS, reg_int_en_4.reg);
			}
		}
		break;

	case DAI_CONFIG_ID_SET_INTEN: {
			T_DAI_INTEN_REG     reg_int_en;
			T_DAI_INTEN2_REG    reg_int_en_2;
#if defined (__FREERTOS) // wait for rtos flag issue fixed can be removed.
			if (config_value & DAI_TXLBDMADONE_INT){
				reg_int_en_2.reg = DAI_GETREG(DAI_INTEN2_REG_OFS);
				reg_int_en_2.reg |= 0x1;
				DAI_SETREG(DAI_INTEN2_REG_OFS, reg_int_en_2.reg);
			}
			if (config_value & DAI_TXLBDMALOAD_INT){
				reg_int_en_2.reg = DAI_GETREG(DAI_INTEN2_REG_OFS);
				reg_int_en_2.reg |= 0x10;
				DAI_SETREG(DAI_INTEN2_REG_OFS, reg_int_en_2.reg);
			}
			if (config_value & DAI_TXLBSTOP_INT){
				reg_int_en_2.reg = DAI_GETREG(DAI_INTEN2_REG_OFS);
				reg_int_en_2.reg |= 0x100;
				DAI_SETREG(DAI_INTEN2_REG_OFS, reg_int_en_2.reg);
			}
			if (config_value & DAI_TXLBBWERR_INT){
				reg_int_en_2.reg = DAI_GETREG(DAI_INTEN2_REG_OFS);
				reg_int_en_2.reg |= 0x1000;
				DAI_SETREG(DAI_INTEN2_REG_OFS, reg_int_en_2.reg);
			}
#else
			UINT32              i;
			if (config_value & DAI_INTERRUPT_TXLB_ALL) {
				reg_int_en_2.reg = DAI_GETREG(DAI_INTEN2_REG_OFS);
				for (i = 0; i < 4; i++) {
					if (config_value & (0x1 << (i + 24))) {
						reg_int_en_2.reg |= (0x1 << (i << 2));
					}
				}
				DAI_SETREG(DAI_INTEN2_REG_OFS, reg_int_en_2.reg);

				config_value &= ~DAI_INTERRUPT_TXLB_ALL;
			}
#endif
			if (config_value & DAI_INTERRUPT_ALL) {
				reg_int_en.reg = DAI_GETREG(DAI_INTEN_REG_OFS);
				reg_int_en.reg |= (config_value & DAI_INTERRUPT_ALL);
				DAI_SETREG(DAI_INTEN_REG_OFS, reg_int_en.reg);
			}
		}
		break;

	case DAI_CONFIG_ID_CLR_INTEN: {
			T_DAI_INTEN_REG     reg_int_en;
			T_DAI_INTEN2_REG    reg_int_en_2;
			UINT32              i;

			if (config_value & DAI_INTERRUPT_TXLB_ALL) {
				reg_int_en_2.reg = DAI_GETREG(DAI_INTEN2_REG_OFS);
				for (i = 0; i < 4; i++) {
					if (config_value & (0x1 << (i + 24))) {
						reg_int_en_2.reg &= ~(0x1 << (i << 2));
					}
				}
				DAI_SETREG(DAI_INTEN2_REG_OFS, reg_int_en_2.reg);

				config_value &= ~DAI_INTERRUPT_TXLB_ALL;
			}


			if (config_value & DAI_INTERRUPT_ALL) {
				reg_int_en.reg = DAI_GETREG(DAI_INTEN_REG_OFS);
				reg_int_en.reg &= ~(config_value & DAI_INTERRUPT_ALL);
				DAI_SETREG(DAI_INTEN_REG_OFS, reg_int_en.reg);
			}
		}
		break;

	case DAI_CONFIG_ID_CLR_INTSTS: {
			T_DAI_INTSTATUS_REG     reg_status;
			T_DAI_INTSTATUS2_REG   reg_status_2;


			// Get interrupt status 2
			reg_status_2.reg = DAI_GETREG(DAI_INTSTATUS2_REG_OFS);

			// Only allow to clear interrupt status which are disabled
			reg_status_2.reg &= ~(DAI_GETREG(DAI_INTEN2_REG_OFS));

			// Clear interrupt status 2
			DAI_SETREG(DAI_INTSTATUS2_REG_OFS, reg_status_2.reg);



			// Get interrupt status
			reg_status.reg = DAI_GETREG(DAI_INTSTATUS_REG_OFS);

			// Only allow to clear interrupt status which are disabled
			reg_status.reg &= ~(DAI_GETREG(DAI_INTEN_REG_OFS));

			// Clear interrupt status
			DAI_SETREG(DAI_INTSTATUS_REG_OFS, reg_status.reg);

		}
		break;









	case DAI_CONFIG_ID_CLKSRC: {
			if (config_value != DAI_CODESCK_INT) {
				unl_cpu(flag);
				DBG_WRN("Only support internal audio MCLK\r\n");
				return;
			}
		}
		break;

	case DAI_CONFIG_ID_RX_SRC_SEL: {
			if (config_value == DAI_RX_SRC_I2S) {
				reg_ctrl.bit.RX_SRC_MUX_SEL = 1;
			} else {
				reg_ctrl.bit.RX_SRC_MUX_SEL = 0;
			}
		}
		break;

	case DAI_CONFIG_ID_STADR2_EN: {
			if (config_value == TRUE) {
				reg_ctrl.bit.DMA_STADR2_EN = 1;
			} else {
				reg_ctrl.bit.DMA_STADR2_EN = 0;
			}
		}
		break;

	default:
		unl_cpu(flag);
		DBG_WRN("CfgID Err = %d\r\n", (int)config_id);
		return;

	}

	DAI_SETREG(DAI_CONFIG_REG_OFS, reg_ctrl.reg);
	unl_cpu(flag);
}


/**
    Get Digital Audio interface(DAI) General Configurations.

    Get Digital Audio interface(DAI) general configuration.
    Use DAI_CONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_CONFIG_ID for details.

    @return void
*/
UINT32 dai_get_config(DAI_CONFIG_ID config_id)
{
	UINT32              ret = 0;
	T_DAI_CONFIG_REG    reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);

	switch (config_id) {
	case DAI_CONFIG_ID_EXTCODEC_EN: {
			ret = reg_ctrl.bit.EXCODEC_EN;
		}
		break;

	case DAI_CONFIG_ID_RX_SRC_SEL: {
			ret = reg_ctrl.bit.RX_SRC_MUX_SEL;
		}
		break;

	case DAI_CONFIG_ID_ISRCB: {
			//set dai interrupt handler

			ret = (uintptr_t)dai_cb_funciton;

		}
		break;

	case DAI_CONFIG_ID_HDMI_TXEN: {
			ret = reg_ctrl.bit.TX_MUX_SEL;
		}
		break;

	case DAI_CONFIG_ID_AVSYNC_EN: {
			ret = reg_ctrl.bit.AVSYNC;
		}
		break;

	case DAI_CONFIG_ID_AVSYNC_SRC: {
			ret = reg_ctrl.bit.AVSYNC_SRC;
		}
		break;

	default:
		DBG_WRN("config_id Err = %d\r\n", (int)config_id);
		break;

	}

	return ret;
}


/**
    Set Digital Audio I2S interface configurations

    Use DAI_I2SCONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_I2SCONFIG_ID for details.
    @param[in] config_value   configuration parameter. Please refer to DAI_I2SCONFIG_ID for details.

    @return void
*/
void dai_set_i2s_config(DAI_I2SCONFIG_ID config_id, UINT32 config_value)
{
	T_DAI_I2SCONFIG_REG reg_i2s_ctrl;
	//T_DAI_I2S_TDM_ORDER_REG reg_i2s_order;
	unsigned long flag;
	UINT32 		ret = 0;
	UINT32      ch_amount = 0, order_pos = 0;

	loc_cpu(flag);

	reg_i2s_ctrl.reg = DAI_GETREG(DAI_I2SCONFIG_REG_OFS);
	//reg_i2s_order.reg = DAI_GETREG(DAI_I2S_TDM_ORDER_REG_OFS);

	switch (config_id) {
	case DAI_I2SCONFIG_ID_CLKRATIO: {
			if (config_value == DAI_I2SCLKR_256FS_32BIT) {
				reg_i2s_ctrl.bit.CKRATIO = 0;
			} else if (config_value == DAI_I2SCLKR_256FS_64BIT) {
				reg_i2s_ctrl.bit.CKRATIO = 1;
			} else if (config_value == DAI_I2SCLKR_256FS_128BIT) {
				reg_i2s_ctrl.bit.CKRATIO = 2;
			} else if (config_value == DAI_I2SCLKR_256FS_256BIT) {
				reg_i2s_ctrl.bit.CKRATIO = 3;
			} else if (config_value == DAI_I2SCLKR_320FS_320BIT) {
				reg_i2s_ctrl.bit.CKRATIO = 4;
			} else {
				unl_cpu(flag);
				DBG_ERR("f/m clk ratio 0x%x not support\r\n", (unsigned int)config_value);
				return;
			}
		}
		break;

	case DAI_I2SCONFIG_ID_FORMAT: {
			if (config_value != DAI_I2SFMT_STANDARD) {
				unl_cpu(flag);
				DBG_ERR("only supports I2S standard format\r\n");
				return;
			}
		}
		break;

	case DAI_I2SCONFIG_ID_OPMODE: {
			if (config_value == DAI_OP_SLAVE) {
				reg_i2s_ctrl.bit.SLAVE = 1;
			} else {
				reg_i2s_ctrl.bit.SLAVE = 0;
			}
		}
		break;

	case DAI_I2SCONFIG_ID_CHANNEL_LEN: {
			reg_i2s_ctrl.bit.CHANNEL_LEN = config_value > 0;
		}
		break;

	case DAI_I2SCONFIG_ID_HDMICH_SEL: {
			T_DAI_CONFIG_REG reg_ctrl;

			reg_ctrl.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);

			if (reg_ctrl.bit.EXCODEC_EN == 0) {
				// When embedded codec, fix using CH0/1
				reg_ctrl.bit.HDMI_CH_SEL = DAI_I2SHDMI_SEL_CH01;
			} else {
				reg_ctrl.bit.HDMI_CH_SEL = config_value;
			}

			DAI_SETREG(DAI_CONFIG_REG_OFS, reg_ctrl.reg);
		}
		break;

	case DAI_I2SCONFIG_ID_CLK_INV: {
			if (config_value == 0) {
				//reg_i2s_ctrl.bit.I2S_ASFCK_INV = 0;
			} else {
				//reg_i2s_ctrl.bit.I2S_ASFCK_INV = 1;
			}
		}break;

	case DAI_I2SCONFIG_ID_DATA_ORDER: {
			UINT32   DAI_I2S_DataOrder_2[2][2]={
                {0,1}, // [Type1]
                {0,1}  // [Type2]
            };
            UINT32   DAI_I2S_DataOrder_4[2][4]={
                {0,1,2,3},  // [Type1]
                {0,2,1,3}   // [Type2]
            };
            UINT32   DAI_I2S_DataOrder_6[2][6]={
                {0,1,2,3,4,5},  // [Type1]
                {0,2,4,1,3,5}   // [Type2]
            };
            UINT32   DAI_I2S_DataOrder_8[2][8]={
                {0,1,2,3,4,5,6,7},  // [Type1]
                {0,2,4,6,1,3,5,7}   // [Type2]
            };
            UINT32   DAI_I2S_DataOrder_10[2][10]={
                {0,1,2,3,4,5,6,7,8,9},  // [Type1]
                {0,2,4,6,8,1,3,5,7,9}   // [Type2]
            };
            UINT32   DAI_I2S_DataOrder_16[2][16]={
                {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},  // [Type1]
                {0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15}   // [Type2]
            };
            UINT32   DAI_I2S_DataOrder_20[2][20]={
                {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19},  // [Type1]
                {0,2,4,6,8,10,12,14,16,18,1,3,5,7,9,11,13,15,17,19}   // [Type2]
            };

            if (reg_i2s_ctrl.bit.CKRATIO == 4) { //320 bits
				 ch_amount = (320) / (16 << reg_i2s_ctrl.bit.CHANNEL_LEN);
			} else {
			    ch_amount = (32 << reg_i2s_ctrl.bit.CKRATIO) / (16 << reg_i2s_ctrl.bit.CHANNEL_LEN);
			}
            //DBG_FUNC("ratio = 0x%x,  CH_LEN = %d bits \r\n",(UINT32)reg_i2s_ctrl.bit.CKRATIO, (UINT32)(16 << reg_i2s_ctrl.bit.CHANNEL_LEN) );

            switch (ch_amount) {
					case 2: {
                        for(order_pos =DAI_I2S_DATAORDER_POS0; order_pos < ch_amount; order_pos++ ){
                               dai_set_i2s_order(order_pos, DAI_I2S_DataOrder_2[config_value][order_pos] );
                               //DBG_FUNC("POS %d = [%d] \r\n",order_pos, (UINT32)dai_get_i2s_order(order_pos) );
                            }
                        }break;

                    case 4:{
                        for(order_pos =DAI_I2S_DATAORDER_POS0; order_pos < ch_amount; order_pos++ ){
                               dai_set_i2s_order(order_pos, DAI_I2S_DataOrder_4[config_value][order_pos] );
                               //DBG_FUNC("POS %d = [%d] \r\n",order_pos, (UINT32)dai_get_i2s_order(order_pos) );
                            }
                        }break;

					case 6:{
                        for(order_pos =DAI_I2S_DATAORDER_POS0; order_pos < ch_amount; order_pos++ ){
                               dai_set_i2s_order(order_pos, DAI_I2S_DataOrder_6[config_value][order_pos] );
                               //DBG_FUNC("POS %d = [%d] \r\n",order_pos, (UINT32)dai_get_i2s_order(order_pos) );
                            }
                        }break;

                    case 8: {
                        for(order_pos =DAI_I2S_DATAORDER_POS0; order_pos < ch_amount; order_pos++ ){
                               dai_set_i2s_order(order_pos, DAI_I2S_DataOrder_8[config_value][order_pos] );
                               //DBG_FUNC("POS %d = [%d] \r\n",order_pos, (UINT32)dai_get_i2s_order(order_pos) );
                            }
                        }break;
                    case 10: {
                        for(order_pos =DAI_I2S_DATAORDER_POS0; order_pos < ch_amount; order_pos++ ){
                               dai_set_i2s_order(order_pos, DAI_I2S_DataOrder_10[config_value][order_pos] );
                               //DBG_FUNC("POS %d = [%d] \r\n",order_pos, (UINT32)dai_get_i2s_order(order_pos) );
                            }
                        }break;
                    case 16: {
                        for(order_pos =DAI_I2S_DATAORDER_POS0; order_pos < ch_amount; order_pos++ ){
                               dai_set_i2s_order(order_pos, DAI_I2S_DataOrder_16[config_value][order_pos] );
                               //DBG_FUNC("POS %d = [%d] \r\n",order_pos, (UINT32)dai_get_i2s_order(order_pos) );
                            }
                        }break;
                    case 20: {
                        for(order_pos =DAI_I2S_DATAORDER_POS0; order_pos < ch_amount; order_pos++ ){
                               dai_set_i2s_order(order_pos, DAI_I2S_DataOrder_20[config_value][order_pos] );
                               //DBG_FUNC("POS %d = [%d] \r\n",order_pos, (UINT32)dai_get_i2s_order(order_pos) );
                            }
                        }break;
                    default:
                		DBG_WRN("Cfg I2S Order ERR   ch_amount = %d\r\n", (int)ch_amount);
                        break;
				}

		}break;

	case DAI_I2SCONFIG_ID_CLK_OFS: {
			if(config_value > DAI_I2S_CLK_OFS_MAX) {
				config_value = DAI_I2S_CLK_OFS_MAX;
			}
			ret = reg_i2s_ctrl.bit.CKRATIO; // make sure ofs is at least one bit clk.
			if (config_value < (UINT32)(0x7>>ret)) {
				config_value = (UINT32)(0x7>>ret);
			}
			reg_i2s_ctrl.bit.I2S_ASFCK_OFS = config_value;
		}break;


	default:
		unl_cpu(flag);
		DBG_WRN("CfgID Err = %d\r\n", (int)config_id);
		return;

	}

	DAI_SETREG(DAI_I2SCONFIG_REG_OFS, reg_i2s_ctrl.reg);
	//DAI_SETREG(DAI_I2S_TDM_ORDER_REG_OFS, reg_i2s_order.reg);
	unl_cpu(flag);
}



/**
    Get Digital Audio I2S interface configurations

    Use DAI_I2SCONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_I2SCONFIG_ID for details.

    @return void
*/
UINT32 dai_get_i2s_config(DAI_I2SCONFIG_ID config_id)
{
	UINT32              ret = 0;
	T_DAI_I2SCONFIG_REG reg_i2s_ctrl;
	//T_DAI_I2S_TDM_ORDER_REG reg_i2s_order;

	reg_i2s_ctrl.reg = DAI_GETREG(DAI_I2SCONFIG_REG_OFS);

	switch (config_id) {
	case DAI_I2SCONFIG_ID_CLKRATIO: {
			ret = reg_i2s_ctrl.bit.CKRATIO;
		}
		break;

	case DAI_I2SCONFIG_ID_FORMAT: {
			ret = DAI_I2SFMT_STANDARD;
		}
		break;

	case DAI_I2SCONFIG_ID_OPMODE: {
			ret = !reg_i2s_ctrl.bit.SLAVE;
		}
		break;

	case DAI_I2SCONFIG_ID_CHANNEL_LEN: {
			ret = reg_i2s_ctrl.bit.CHANNEL_LEN;
		}
		break;

	case DAI_I2SCONFIG_ID_HDMICH_SEL: {
			T_DAI_CONFIG_REG reg_ctrl;

			reg_ctrl.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);
			ret = reg_ctrl.bit.HDMI_CH_SEL;
		}
		break;

	case DAI_I2SCONFIG_ID_SLAVEMATCH: {
			ret = reg_i2s_ctrl.bit.I2SCKR_MATCH;
		}
		break;

	case DAI_I2SCONFIG_ID_CURRENT_CLKRATIO: {
			ret = reg_i2s_ctrl.bit.I2SCKR_CUR;
		}
		break;

	case DAI_I2SCONFIG_ID_CLK_INV: {
			//ret = reg_i2s_ctrl.bit.I2S_ASFCK_INV;
		}break;

	case DAI_I2SCONFIG_ID_DATA_ORDER: {
			//ret = reg_i2s_order.bit.I2S_TDM_ORDER;
		}break;
	case DAI_I2SCONFIG_ID_CLK_OFS: {
			ret = reg_i2s_ctrl.bit.I2S_ASFCK_OFS;
		}break;


	default:
		DBG_WRN("CfgID Err = %d\r\n", (int)config_id);
		break;

	}

	return ret;

}

/**
    Set Digital Audio I2S interface TDM DATA ORDER

    Use DAI_I2SCONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_I2S_DATAORDERPOS for details.
    @param[in] config_value   configuration parameter. Please refer to DAI_I2S_DATAORDERPOS for details.

    @return void
*/
void dai_set_i2s_order(DAI_I2S_DATAORDERPOS config_id, UINT32 config_value)
{
	T_DAI_I2S_TDM_ORDER_REG  reg_i2s_order;
    T_DAI_I2S_TDM_ORDER2_REG reg_i2s_order2;
    T_DAI_I2S_TDM_ORDER3_REG reg_i2s_order3;
    T_DAI_I2S_TDM_ORDER4_REG reg_i2s_order4;
    T_DAI_I2S_TDM_ORDER5_REG reg_i2s_order5;

	reg_i2s_order.reg  = DAI_GETREG(DAI_I2S_TDM_ORDER_REG_OFS);
    reg_i2s_order2.reg = DAI_GETREG(DAI_I2S_TDM_ORDER2_REG_OFS);
    reg_i2s_order3.reg = DAI_GETREG(DAI_I2S_TDM_ORDER3_REG_OFS);
    reg_i2s_order4.reg = DAI_GETREG(DAI_I2S_TDM_ORDER4_REG_OFS);
    reg_i2s_order5.reg = DAI_GETREG(DAI_I2S_TDM_ORDER5_REG_OFS);

	switch (config_id) {
	case DAI_I2S_DATAORDER_POS0: {
			reg_i2s_order.bit.I2S_TDM_ORDER_POS0 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS1: {
			reg_i2s_order.bit.I2S_TDM_ORDER_POS1 = config_value;
		}
		break;
	case DAI_I2S_DATAORDER_POS2: {
			reg_i2s_order.bit.I2S_TDM_ORDER_POS2 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS3: {
			reg_i2s_order.bit.I2S_TDM_ORDER_POS3 = config_value;
		}
		break;
	case DAI_I2S_DATAORDER_POS4: {
			reg_i2s_order2.bit.I2S_TDM_ORDER_POS4 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS5: {
			reg_i2s_order2.bit.I2S_TDM_ORDER_POS5 = config_value;
		}
		break;
	case DAI_I2S_DATAORDER_POS6: {
			reg_i2s_order2.bit.I2S_TDM_ORDER_POS6 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS7: {
			reg_i2s_order2.bit.I2S_TDM_ORDER_POS7 = config_value;
		}
		break;
	case DAI_I2S_DATAORDER_POS8: {
			reg_i2s_order3.bit.I2S_TDM_ORDER_POS8 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS9: {
			reg_i2s_order3.bit.I2S_TDM_ORDER_POS9 = config_value;
		}
		break;
	case DAI_I2S_DATAORDER_POS10: {
			reg_i2s_order3.bit.I2S_TDM_ORDER_POS10 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS11: {
			reg_i2s_order3.bit.I2S_TDM_ORDER_POS11 = config_value;
		}
		break;
	case DAI_I2S_DATAORDER_POS12: {
			reg_i2s_order4.bit.I2S_TDM_ORDER_POS12 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS13: {
			reg_i2s_order4.bit.I2S_TDM_ORDER_POS13 = config_value;
		}
		break;
 	case DAI_I2S_DATAORDER_POS14: {
			reg_i2s_order4.bit.I2S_TDM_ORDER_POS14 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS15: {
			reg_i2s_order4.bit.I2S_TDM_ORDER_POS15 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS16: {
			reg_i2s_order5.bit.I2S_TDM_ORDER_POS16 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS17: {
			reg_i2s_order5.bit.I2S_TDM_ORDER_POS17 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS18: {
			reg_i2s_order5.bit.I2S_TDM_ORDER_POS18 = config_value;
		}
		break;
    case DAI_I2S_DATAORDER_POS19: {
			reg_i2s_order5.bit.I2S_TDM_ORDER_POS19 = config_value;
		}
		break;

	default:

		DBG_WRN("CfgID Err = %d\r\n", (int)config_id);
		return;
	}

	DAI_SETREG(DAI_I2S_TDM_ORDER_REG_OFS, reg_i2s_order.reg);
    DAI_SETREG(DAI_I2S_TDM_ORDER2_REG_OFS, reg_i2s_order2.reg);
	DAI_SETREG(DAI_I2S_TDM_ORDER3_REG_OFS, reg_i2s_order3.reg);
    DAI_SETREG(DAI_I2S_TDM_ORDER4_REG_OFS, reg_i2s_order4.reg);
    DAI_SETREG(DAI_I2S_TDM_ORDER5_REG_OFS, reg_i2s_order5.reg);

}


/**
    Get Digital Audio I2S interface configurations

    Use DAI_I2SCONFIG_ID as configuration selection and config_value is the configuration parameter.

    @param[in] config_id     Configuration selection. Please refer to DAI_I2SCONFIG_ID for details.

    @return void
*/
UINT32 dai_get_i2s_order(DAI_I2S_DATAORDERPOS config_id)
{
    T_DAI_I2S_TDM_ORDER_REG  reg_i2s_order;
    T_DAI_I2S_TDM_ORDER2_REG reg_i2s_order2;
    T_DAI_I2S_TDM_ORDER3_REG reg_i2s_order3;
    T_DAI_I2S_TDM_ORDER4_REG reg_i2s_order4;
    T_DAI_I2S_TDM_ORDER5_REG reg_i2s_order5;

	UINT32 		ret = 0;

	reg_i2s_order.reg  = DAI_GETREG(DAI_I2S_TDM_ORDER_REG_OFS);
    reg_i2s_order2.reg = DAI_GETREG(DAI_I2S_TDM_ORDER2_REG_OFS);
    reg_i2s_order3.reg = DAI_GETREG(DAI_I2S_TDM_ORDER3_REG_OFS);
    reg_i2s_order4.reg = DAI_GETREG(DAI_I2S_TDM_ORDER4_REG_OFS);
    reg_i2s_order5.reg = DAI_GETREG(DAI_I2S_TDM_ORDER5_REG_OFS);

	switch (config_id) {
	case DAI_I2S_DATAORDER_POS0: {
			ret = reg_i2s_order.bit.I2S_TDM_ORDER_POS0;
		}
		break;
    case DAI_I2S_DATAORDER_POS1: {
			ret = reg_i2s_order.bit.I2S_TDM_ORDER_POS1;
		}
		break;
	case DAI_I2S_DATAORDER_POS2: {
			ret = reg_i2s_order.bit.I2S_TDM_ORDER_POS2;
		}
		break;
    case DAI_I2S_DATAORDER_POS3: {
			ret = reg_i2s_order.bit.I2S_TDM_ORDER_POS3;
		}
		break;
	case DAI_I2S_DATAORDER_POS4: {
			ret = reg_i2s_order2.bit.I2S_TDM_ORDER_POS4;
		}
		break;
    case DAI_I2S_DATAORDER_POS5: {
			ret = reg_i2s_order2.bit.I2S_TDM_ORDER_POS5;
		}
		break;
	case DAI_I2S_DATAORDER_POS6: {
			ret = reg_i2s_order2.bit.I2S_TDM_ORDER_POS6;
		}
		break;
    case DAI_I2S_DATAORDER_POS7: {
			ret = reg_i2s_order2.bit.I2S_TDM_ORDER_POS7;
		}
		break;
	case DAI_I2S_DATAORDER_POS8: {
			ret = reg_i2s_order3.bit.I2S_TDM_ORDER_POS8;
		}
		break;
    case DAI_I2S_DATAORDER_POS9: {
			ret = reg_i2s_order3.bit.I2S_TDM_ORDER_POS9;
		}
		break;
	case DAI_I2S_DATAORDER_POS10: {
			ret = reg_i2s_order3.bit.I2S_TDM_ORDER_POS10;
		}
		break;
    case DAI_I2S_DATAORDER_POS11: {
			ret = reg_i2s_order3.bit.I2S_TDM_ORDER_POS11;
		}
		break;
	case DAI_I2S_DATAORDER_POS12: {
			ret = reg_i2s_order4.bit.I2S_TDM_ORDER_POS12;
		}
		break;
    case DAI_I2S_DATAORDER_POS13: {
			ret = reg_i2s_order4.bit.I2S_TDM_ORDER_POS13;
		}
		break;
 	case DAI_I2S_DATAORDER_POS14: {
			ret = reg_i2s_order4.bit.I2S_TDM_ORDER_POS14;
		}
		break;
    case DAI_I2S_DATAORDER_POS15: {
			ret = reg_i2s_order4.bit.I2S_TDM_ORDER_POS15;
		}
		break;
    case DAI_I2S_DATAORDER_POS16: {
			ret = reg_i2s_order5.bit.I2S_TDM_ORDER_POS16;
		}
		break;
    case DAI_I2S_DATAORDER_POS17: {
			ret = reg_i2s_order5.bit.I2S_TDM_ORDER_POS17;
		}
		break;
    case DAI_I2S_DATAORDER_POS18: {
			ret = reg_i2s_order5.bit.I2S_TDM_ORDER_POS18;
		}
		break;
    case DAI_I2S_DATAORDER_POS19: {
			ret = reg_i2s_order5.bit.I2S_TDM_ORDER_POS19;
		}
		break;

	default:
		DBG_WRN("CfgID Err = %d\r\n", (int)config_id);
		break;
	}

	return ret;

}

#endif

#if 1

/**
    Set DAI Playback Chaneels Configurations.

    Set DAI Playback Chaneels(TX1 and TX2) Configurations.

    @param[in]  channel        playback Channel selection
    @param[in]  config_id    playback config ID selection
    @param[in]  config_value  configuration value

    @return void
*/
void dai_set_tx_config(DAI_TXCH channel, DAI_TXCFG_ID config_id, UINT32 config_value)
{
	T_DAI_FMTCFG0_REG reg_fmt_0;
	unsigned long flag;

	loc_cpu(flag);

	reg_fmt_0.reg = DAI_GETREG(DAI_FMTCFG0_REG_OFS);

	switch (config_id) {
	case DAI_TXCFG_ID_CHANNEL: {
			if (config_value != DAI_CH_DUAL_MONO) {
				if (channel == DAI_TXCH_TX1) {
					reg_fmt_0.bit.TX1_SOUNDM = config_value;
				} else if (channel == DAI_TXCH_TX2) {
					reg_fmt_0.bit.TX2_SOUNDM = config_value;
				}
			} else {
				unl_cpu(flag);
				DBG_WRN("DAI TX no support dual mono\r\n");
				return;
			}
		}
		break;

	case DAI_TXCFG_ID_TOTAL_CH: {
			T_DAI_CONFIG_REG reg_ctrl;

			reg_ctrl.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);

			if ((reg_ctrl.bit.EXCODEC_EN == 0) && (config_value > DAI_TOTCH_2CH)) {
				DBG_WRN("Embedded codec 2CH only\r\n");
				config_value = DAI_TOTCH_2CH;
			}

			if (channel == DAI_TXCH_TX1) {
				reg_fmt_0.bit.TX1_SOUNDCH = config_value;
			} else if (channel == DAI_TXCH_TX2) {
				reg_fmt_0.bit.TX2_SOUNDCH = config_value;
			}
		}
		break;

	case DAI_TXCFG_ID_PCMLEN: {
			if (channel == DAI_TXCH_TX1) {
				reg_fmt_0.bit.TX1_PCMLEN = config_value;
			} else if (channel == DAI_TXCH_TX2) {
				reg_fmt_0.bit.TX2_PCMLEN = config_value;
			}
		}
		break;

	case DAI_TXCFG_ID_DRAMCH: {
			if (channel == DAI_TXCH_TX1) {
				reg_fmt_0.bit.TX1_DRAMCH = config_value;
			} else if (channel == DAI_TXCH_TX2) {
				reg_fmt_0.bit.TX2_DRAMCH = config_value;
			}
		}
		break;

	case DAI_TXCFG_ID_TIMECODE_TRIG: {
			if (channel == DAI_TXCH_TX1) {
				DAI_SETREG(DAI_TX1TCTRIGGER_REG_OFS, config_value);
			} else {
				unl_cpu(flag);
				DBG_ERR("Only TX CH1 has TimeCode function (1)\r\n");
				return;
			}
		}
		break;

	case DAI_TXCFG_ID_TIMECODE_OFS: {
			if (channel == DAI_TXCH_TX1) {
				DAI_SETREG(DAI_TX1TCOFFSET_REG_OFS, config_value);
			} else {
				unl_cpu(flag);
				DBG_ERR("Only TX CH1 has TimeCode function (2)\r\n");
				return;
			}
		}
		break;




	default:
		unl_cpu(flag);
		DBG_WRN("CfgID Err = %d\r\n", (int)config_id);
		return;

	}

	DAI_SETREG(DAI_FMTCFG0_REG_OFS, reg_fmt_0.reg);
	unl_cpu(flag);
}


/**
    Get DAI Playback Chaneels Configurations.

    Get DAI Playback Chaneels(TX1 and TX2) Configurations.

    @param[in]  channel        playback Channel selection
    @param[in]  config_id    playback config ID selection

    @return configuration value
*/
UINT32 dai_get_tx_config(DAI_TXCH channel, DAI_TXCFG_ID config_id)
{
	T_DAI_FMTCFG0_REG   reg_fmt_0;
	T_DAI_INTSTATUS_REG reg_status;
	UINT32              ret = 0;

	reg_fmt_0.reg = DAI_GETREG(DAI_FMTCFG0_REG_OFS);
	reg_status.reg = DAI_GETREG(DAI_INTSTATUS_REG_OFS);

	switch (config_id) {
	case DAI_TXCFG_ID_CHANNEL: {
			if (channel == DAI_TXCH_TX1) {
				ret = reg_fmt_0.bit.TX1_SOUNDM;
			} else if (channel == DAI_TXCH_TX2) {
				ret = reg_fmt_0.bit.TX2_SOUNDM;
			}
		}
		break;

	case DAI_TXCFG_ID_TOTAL_CH: {
			if (channel == DAI_TXCH_TX1) {
				ret = reg_fmt_0.bit.TX1_SOUNDCH;
			} else if (channel == DAI_TXCH_TX2) {
				ret = reg_fmt_0.bit.TX2_SOUNDCH;
			}
		}
		break;

	case DAI_TXCFG_ID_PCMLEN: {
			if (channel == DAI_TXCH_TX1) {
				ret = reg_fmt_0.bit.TX1_PCMLEN;
			} else if (channel == DAI_TXCH_TX2) {
				ret = reg_fmt_0.bit.TX2_PCMLEN;
			}
		}
		break;

	case DAI_TXCFG_ID_DRAMCH: {
			if (channel == DAI_TXCH_TX1) {
				ret = reg_fmt_0.bit.TX1_DRAMCH;
			} else if (channel == DAI_TXCH_TX2) {
				ret = reg_fmt_0.bit.TX2_DRAMCH;
			}
		}
		break;

	case DAI_TXCFG_ID_TIMECODE_TRIG: {
			if (channel == DAI_TXCH_TX1) {
				ret = DAI_GETREG(DAI_TX1TCTRIGGER_REG_OFS);
			}
		}
		break;

	case DAI_TXCFG_ID_TIMECODE_OFS: {
			if (channel == DAI_TXCH_TX1) {
				ret = DAI_GETREG(DAI_TX1TCOFFSET_REG_OFS);
			}
		}
		break;

	case DAI_TXCFG_ID_TIMECODE_VAL: {
			if (channel == DAI_TXCH_TX1) {
				ret = DAI_GETREG(DAI_TX1TCVALUE_REG_OFS);
			}
		}
		break;

	case  DAI_TXCFG_ID_ADDR_LOAD: {
	    if (channel == DAI_TXCH_TX1) {
				ret = reg_status.bit.TX1_DMALOAD;
			} else if (channel == DAI_TXCH_TX2) {
				ret = reg_status.bit.TX2_DMALOAD;
			}
		}
		break;
   	case  DAI_TXCFG_ID_DMA_DONE: {
	    if (channel == DAI_TXCH_TX1) {
				ret = reg_status.bit.TX1_DMABFI;
			} else if (channel == DAI_TXCH_TX2) {
				ret = reg_status.bit.TX2_DMABFI;
			}
		}
		break;


	default:
		break;

	}

	return ret;
}



/**
    Set DAI Playback Loopback Chaneel Configurations.

    Set DAI Playback Loopback Chaneels(TXLB) Configurations.

    @param[in]  channel        playback Loopback Channel selection
    @param[in]  config_id    playback Loopback config ID selection
    @param[in]  config_value  configuration value

    @return void
*/
void dai_set_txlb_config(DAI_TXLBCFG_ID config_id, UINT32 config_value)
{
	T_DAI_FMTCFG2_REG reg_fmt_2;
	unsigned long flag;

	loc_cpu(flag);

	reg_fmt_2.reg = DAI_GETREG(DAI_FMTCFG2_REG_OFS);

	switch (config_id) {
	case DAI_TXLBCFG_ID_CHANNEL: {
			if (config_value != DAI_CH_DUAL_MONO) {
				reg_fmt_2.bit.TXLB_SOUNDM = config_value;
			} else {
				unl_cpu(flag);
				DBG_WRN("DAI TXLB no support dual mono\r\n");
				return;
			}
		}
		break;

	case DAI_TXLBCFG_ID_TOTAL_CH: {
			T_DAI_CONFIG_REG reg_ctrl;

			reg_ctrl.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);

			if ((reg_ctrl.bit.EXCODEC_EN == 0) && (config_value > DAI_TOTCH_2CH)) {
				DBG_WRN("Embedded codec 2CH only\r\n");
				config_value = DAI_TOTCH_2CH;
			}

			reg_fmt_2.bit.TXLB_SOUNDCH = config_value;

		}
		break;

	case DAI_TXLBCFG_ID_PCMLEN: {

			reg_fmt_2.bit.TXLB_PCMLEN = config_value;

		}
		break;

	case DAI_TXLBCFG_ID_DRAMCH: {
			reg_fmt_2.bit.TXLB_DRAMCH = config_value;
		}
		break;

	case DAI_TXLBCFG_ID_RXSYNC: {
			T_DAI_CONFIG_REG    reg_cfg;

			reg_cfg.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);
			reg_cfg.bit.TXLB_SYNC = config_value > 0;
			DAI_SETREG(DAI_CONFIG_REG_OFS, reg_cfg.reg);
			unl_cpu(flag);
		}
		return;

	default:
		unl_cpu(flag);
		DBG_WRN("CfgID Err = %d\r\n", (int)config_id);
		return;

	}

	DAI_SETREG(DAI_FMTCFG2_REG_OFS, reg_fmt_2.reg);
	unl_cpu(flag);
}



/**
    Set DAI Playback Loopback Chaneel Configurations.

    Set DAI Playback Loopback Chaneels(TXLB) Configurations.

    @param[in]  channel        playback Loopback Channel selection
    @param[in]  config_id    playback Loopback config ID selection
    @param[in]  config_value  configuration value

    @return void
*/
UINT32 dai_get_txlb_config(DAI_TXLBCFG_ID config_id)
{
	T_DAI_FMTCFG2_REG   reg_fmt_2;
	T_DAI_INTSTATUS2_REG reg_status2;
	UINT32              ret = 0;

	reg_fmt_2.reg = DAI_GETREG(DAI_FMTCFG2_REG_OFS);
	reg_status2.reg = DAI_GETREG(DAI_INTSTATUS2_REG_OFS);

	switch (config_id) {
	case DAI_TXLBCFG_ID_CHANNEL: {
			ret = reg_fmt_2.bit.TXLB_SOUNDM;
		}
		break;

	case DAI_TXLBCFG_ID_TOTAL_CH: {
			ret = reg_fmt_2.bit.TXLB_SOUNDCH;
		}
		break;

	case DAI_TXLBCFG_ID_PCMLEN: {
			ret = reg_fmt_2.bit.TXLB_PCMLEN;
		}
		break;

	case DAI_TXLBCFG_ID_DRAMCH: {
			ret = reg_fmt_2.bit.TXLB_DRAMCH;
		}
		break;

	case DAI_TXLBCFG_ID_RXSYNC: {
			T_DAI_CONFIG_REG    reg_cfg;

			reg_cfg.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);
			ret = reg_cfg.bit.TXLB_SYNC;
		}
		break;
	case DAI_TXLBCFG_ID_ADDR_LOAD: {
		ret = reg_status2.bit.TXLB_DMALOAD;
		}
		break;

	default:
		break;

	}

	return ret;
}

/**
    Set DAI Record Channels Configurations.

    Set DAI Record Channels (RX1 & RX2) Configurations.

    @param[in]  config_id    record config ID selection
    @param[in]  config_value  configuration value

    @return void
*/
void dai_set_rx_config(DAI_RXCFG_ID config_id, UINT32 config_value)
{
	T_DAI_FMTCFG1_REG reg_fmt_1;
	unsigned long flag;

	loc_cpu(flag);

	reg_fmt_1.reg = DAI_GETREG(DAI_FMTCFG1_REG_OFS);

	switch (config_id) {
	case DAI_RXCFG_ID_CHANNEL: {
			reg_fmt_1.bit.RX_SOUNDM = config_value;
		}
		break;

	case DAI_RXCFG_ID_TOTAL_CH: {
			T_DAI_CONFIG_REG reg_ctrl;

			reg_ctrl.reg = DAI_GETREG(DAI_CONFIG_REG_OFS);

			if ((reg_ctrl.bit.EXCODEC_EN == 0) && (config_value > DAI_TOTCH_4CH)) {
				DBG_WRN("Embedded codec Rx 4CH max\r\n");
				config_value = DAI_TOTCH_4CH;
			}

			reg_fmt_1.bit.RX_SOUNDCH = config_value;
		}
		break;

	case DAI_RXCFG_ID_PCMLEN: {
			reg_fmt_1.bit.RX_PCMLEN = config_value;
		}
		break;

	case DAI_RXCFG_ID_DRAMCH: {
			reg_fmt_1.bit.RX_DRAMCH = config_value;
		}
		break;

	case DAI_RXCFG_ID_TIMECODE_TRIG: {
			DAI_SETREG(DAI_RXTCTRIGGER_REG_OFS, config_value);
		}
		break;

	case DAI_RXCFG_ID_TIMECODE_OFS: {
			DAI_SETREG(DAI_RXTCOFFSET_REG_OFS, config_value);
		}
		break;






	default:
		unl_cpu(flag);
		DBG_WRN("CfgID Err = %d\r\n", (int)config_id);
		return;

	}

	DAI_SETREG(DAI_FMTCFG1_REG_OFS, reg_fmt_1.reg);
	unl_cpu(flag);
}


/**
    Get DAI Record Channels Configurations.

    Get DAI Record Channels (RX1 & RX2) Configurations.

    @param[in]  config_id    record config ID selection

    @return configuration value
*/
UINT32 dai_get_rx_config(DAI_RXCFG_ID config_id)
{
	T_DAI_FMTCFG1_REG   reg_fmt_1;
	T_DAI_INTSTATUS_REG reg_status;
	UINT32              ret = 0;

	reg_fmt_1.reg = DAI_GETREG(DAI_FMTCFG1_REG_OFS);
	reg_status.reg = DAI_GETREG(DAI_INTSTATUS_REG_OFS);

	switch (config_id) {
	case DAI_RXCFG_ID_CHANNEL: {
			ret = reg_fmt_1.bit.RX_SOUNDM;
		}
		break;

	case DAI_RXCFG_ID_TOTAL_CH: {
			ret = reg_fmt_1.bit.RX_SOUNDCH;
		}
		break;

	case DAI_RXCFG_ID_PCMLEN: {
			ret = reg_fmt_1.bit.RX_PCMLEN;
		}
		break;

	case DAI_RXCFG_ID_DRAMCH: {
			ret = reg_fmt_1.bit.RX_DRAMCH;
		}
		break;

	case DAI_RXCFG_ID_TIMECODE_TRIG: {
			ret = DAI_GETREG(DAI_RXTCTRIGGER_REG_OFS);
		}
		break;

	case DAI_RXCFG_ID_TIMECODE_OFS: {
			ret = DAI_GETREG(DAI_RXTCOFFSET_REG_OFS);
		}
		break;

	case DAI_RXCFG_ID_TIMECODE_VAL: {
			ret = DAI_GETREG(DAI_RXTCVALUE_REG_OFS);
		}
		break;
	case DAI_RXCFG_ID_ADDR_LOAD: {
		ret = reg_status.bit.RX_DMALOAD;
		}
		break;

	case DAI_RXCFG_ID_DMA_DONE: {
		ret = reg_status.bit.RX1_DMABFI;
		}
		break;
	case DAI_RX2CFG_ID_DMA_DONE: {
		ret = reg_status.bit.RX2_DMABFI;
		}
		break;


	default:
		break;

	}

	return ret;
}


/**
    Enable/Disable DAI playback DMA Channel

    Enable/Disable DAI playback DMA Channel

    @param[in]  channel        playback Channel selection
    @param[in]  b_en         TRUE is Enable. FALSE is Disable.

    @return void
*/
void dai_enable_tx_dma(DAI_TXCH channel, BOOL b_en)
{
	T_DAI_CTRL_REG reg_ctrl;
	unsigned long flag;

	loc_cpu(flag);

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (channel == DAI_TXCH_TX1) {
		if (b_en == TRUE) {
			reg_ctrl.bit.DMA_TX1_EN = 1;
		} else {
			reg_ctrl.bit.DMA_TX1_EN = 0;
		}
	} else if (channel == DAI_TXCH_TX2) {
		if (b_en == TRUE) {
			reg_ctrl.bit.DMA_TX2_EN = 1;
		} else {
			reg_ctrl.bit.DMA_TX2_EN = 0;
		}
	}


	DAI_SETREG(DAI_CTRL_REG_OFS, reg_ctrl.reg);

	unl_cpu(flag);
}

/**
    Enable/Disable DAI record DMA Channel

    Enable/Disable DAI record DMA Channel

    @param[in]  b_en         TRUE is Enable. FALSE is Disable.

    @return void
*/
void dai_enable_rx_dma(BOOL b_en)
{
	T_DAI_CTRL_REG reg_ctrl;
	unsigned long flag;

	loc_cpu(flag);

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (b_en == TRUE) {
		reg_ctrl.bit.DMA_RX_EN = 1;
	} else {
		reg_ctrl.bit.DMA_RX_EN = 0;
	}

	DAI_SETREG(DAI_CTRL_REG_OFS, reg_ctrl.reg);

	unl_cpu(flag);
}


/**
    Enable/Disable DAI playback loopback DMA Channel

    Enable/Disable DAI playback loopback DMA Channel

    @param[in]  b_en         TRUE is Enable. FALSE is Disable.

    @return void
*/
void dai_enable_txlb_dma(BOOL b_en)
{
	T_DAI_CTRL_REG reg_ctrl;
	unsigned long flag;

	loc_cpu(flag);

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (b_en == TRUE) {
		reg_ctrl.bit.DMA_TXLB_EN = 1;
	} else {
		reg_ctrl.bit.DMA_TXLB_EN = 0;
	}

	DAI_SETREG(DAI_CTRL_REG_OFS, reg_ctrl.reg);

	unl_cpu(flag);
}


/**
    Start/Stop DAI Playback

    Start/Stop DAI Specified Channel Playback

    @param[in]  channel        playback Channel selection
    @param[in]  b_en         TRUE is Start. FALSE is Stop.

    @return void
*/
void dai_enable_tx(DAI_TXCH channel, BOOL b_en)
{
	T_DAI_CTRL_REG reg_ctrl;
	unsigned long flag;

	loc_cpu(flag);

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (channel == DAI_TXCH_TX1) {
		if (b_en == TRUE) {
			reg_ctrl.bit.TX1_EN = 1;
		} else {
			reg_ctrl.bit.TX1_EN = 0;
			size_error[0] = 0;
		}
	} else if (channel == DAI_TXCH_TX2) {
		if (b_en == TRUE) {
			reg_ctrl.bit.TX2_EN = 1;
		} else {
			reg_ctrl.bit.TX2_EN = 0;
			size_error[1] = 0;
		}
	}

	DAI_SETREG(DAI_CTRL_REG_OFS, reg_ctrl.reg);

	unl_cpu(flag);
}


/**
    Start/Stop DAI Record

    Start/Stop DAI Specified Channel Record

    @param[in]  b_en         TRUE is Start. FALSE is Stop.

    @return void
*/
void dai_enable_rx(BOOL b_en)
{
	T_DAI_CTRL_REG reg_ctrl;
	unsigned long flag;

	loc_cpu(flag);

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (b_en == TRUE) {
		reg_ctrl.bit.RX_EN = 1;
	} else {
		reg_ctrl.bit.RX_EN = 0;
	}

	DAI_SETREG(DAI_CTRL_REG_OFS, reg_ctrl.reg);

	unl_cpu(flag);
}

/**
    Start/Stop DAI playback loopback

    Start/Stop DAI Specified Channel Record

    @param[in]  b_en         TRUE is Start. FALSE is Stop.

    @return void
*/
void dai_enable_txlb(BOOL b_en)
{
	T_DAI_CTRL_REG reg_ctrl;
	unsigned long flag;

	loc_cpu(flag);

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (b_en == TRUE) {
		reg_ctrl.bit.TXLB_EN = 1;
	} else {
		reg_ctrl.bit.TXLB_EN = 0;
	}

	DAI_SETREG(DAI_CTRL_REG_OFS, reg_ctrl.reg);

	unl_cpu(flag);
}

/**
    Set DAI module enable/disable

    Set DAI module enable/disable

    @param[in] b_en      DAI module enable/disable
     - @b TRUE:     module enable
     - @b FALSE:    module disable

    @return void
*/
void dai_enable_dai(BOOL b_en)
{
	T_DAI_CTRL_REG reg_ctrl;
	unsigned long flag;

	loc_cpu(flag);


	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (b_en == TRUE) {
		reg_ctrl.bit.DAIEN = 1;
	} else {
		reg_ctrl.bit.DAIEN = 0;
	}

	DAI_SETREG(DAI_CTRL_REG_OFS, reg_ctrl.reg);

	unl_cpu(flag);
}

#endif

#if 1

/**
    Check whether DAI is enabled or not

    If DAI is enabled this function will return TRUE.

    @return
	@b TRUE:  DAI is enabled
	@b FALSE: DAI is disabled

*/
BOOL dai_is_dai_enable(void)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	return reg_ctrl.bit.DAIEN;
}

/**
    Check if DAI Playback Channel is enabled

    Check if DAI Playback Channel is enabled

    @param[in]  channel        playback Channel selection

    @return
	@b TRUE:  Specified Playback channel is enabled
	@b FALSE: Specified Playback channel is disabled
*/
BOOL dai_is_tx_enable(DAI_TXCH channel)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (channel == DAI_TXCH_TX2) {
		return reg_ctrl.bit.TX2_EN;
	} else {
		return reg_ctrl.bit.TX1_EN;
	}
}

/**
    Check if DAI record Channel is enabled

    Check if DAI record Channel is enabled

    @return
	@b TRUE:  Record is enabled
	@b FALSE: Record is disabled
*/
BOOL dai_is_rx_enable(void)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	return reg_ctrl.bit.RX_EN;
}

/**
    Check if DAI playback loopback Channel is enabled

    Check if DAI playback loopback Channel is enabled

    @return
	@b TRUE:  playback loopback is enabled
	@b FALSE: playback loopback is disabled
*/
BOOL dai_is_txlb_enable(void)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	return reg_ctrl.bit.TXLB_EN;
}

/**
    Check whether DAI is under tx/rx

    If DAI is under tx/rx, this function will return TRUE.

    @return
	@b TRUE:  DAI is under tx/rx.
	@b FALSE: DAI is not under tx/rx.

*/
BOOL dai_is_txrx_enable(void)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	return (reg_ctrl.bit.TX1_EN || reg_ctrl.bit.TX2_EN || reg_ctrl.bit.RX_EN || reg_ctrl.bit.TXLB_EN);
}

/**
    Check if playback DMA Channel is Enabled

    Check if playback DMA Channel is Enabled

    @param[in]  channel        playback Channel selection

    @return
	@b TRUE:  Specified Playback DMA channel is enabled
	@b FALSE: Specified Playback DMA channel is disabled
*/
BOOL dai_is_tx_dma_enable(DAI_TXCH channel)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	if (channel == DAI_TXCH_TX2) {
		return reg_ctrl.bit.DMA_TX2_EN;
	} else {
		return reg_ctrl.bit.DMA_TX1_EN;
	}
}


/**
    Check if Record DMA Channel is Enabled

    Check if Record DMA Channel is Enabled

    @return
	@b TRUE:  Specified Record DMA channel is enabled
	@b FALSE: Specified Record DMA channel is disabled
*/
BOOL dai_is_rx_dma_enable(void)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	return reg_ctrl.bit.DMA_RX_EN;
}


/**
    Check if Playback Loopback DMA Channel is Enabled

    Check if Playback Loopback DMA Channel is Enabled

    @return
	@b TRUE:  Specified Playback Loopback DMA channel is enabled
	@b FALSE: Specified Playback Loopback DMA channel is disabled
*/
BOOL dai_is_txlb_dma_enable(void)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	return reg_ctrl.bit.DMA_TXLB_EN;
}


/**
    Check whether Tx or Rx DMA is enabled or not

    If Tx or Rx DMA is enabled this function will return TRUE.

    @return
	@b TRUE:  DMA is enabled
	@b FALSE: DMA is disabled
*/
BOOL dai_is_dma_enable(void)
{
	T_DAI_CTRL_REG reg_ctrl;

	reg_ctrl.reg = DAI_GETREG(DAI_CTRL_REG_OFS);

	return (reg_ctrl.bit.DMA_RX_EN || reg_ctrl.bit.DMA_TX1_EN || reg_ctrl.bit.DMA_TX2_EN || reg_ctrl.bit.DMA_TXLB_EN);
}



#endif

#if 1

/**
    Set Playback DMA parameter

    Set Playback DMA starting address, buffer size.

    @param[in] dma_channel       Playback DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[in] dma_start_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] dma_buffer_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
void dai_set_tx_dma_para(int id, int ddr_id, UINT32 dma_channel, uintptr_t dma_start_addr, UINT32 dma_buffer_size)
{
	T_DAI_RX1DMASTART_REG   reg_dma_start = {0};
	T_DAI_RX1DMASTARTMSB_REG reg_dma_start_msb = {0};
	T_DAI_RX1DMABUFSIZE_REG reg_dma_bufsz  = {0};
	uintptr_t phy_addr;

	if (dma_start_addr & 0x03) {
		DBG_WRN("DMA address should be word align, but 0x%x, align it to 0x%x\r\n", (unsigned int)dma_start_addr, (unsigned int)(dma_start_addr & (~0x03)));
	}
	if (size_error[dma_channel]) {
		DBG_WRN("Previous DMA size must be multiples of 2 words (%d)(%d)\r\n", (int)dma_channel, (int)size_error[dma_channel]);
	}
	if (dma_buffer_size & 0x0F) {
		size_error[dma_channel] = dma_buffer_size;
	}

#ifdef __KERNEL__
	fmem_dcache_sync((void *)dma_start_addr, dma_buffer_size * 4, DMA_BIDIRECTIONAL);
	phy_addr = dai_platform_va2pa(dma_start_addr);
	//phy_addr = dma_start_addr;
	//reg_dma_start.bit.RX1DMASTADR = phy_addr & 0xFFFFFFFF;
#elif defined(__FREERTOS)
	dma_flushWriteCache(dma_start_addr, dma_buffer_size * 4);
	reg_dma_start.bit.TX1DMASTADR  = dma_get_phy_addr(dma_start_addr);
#endif


#ifdef __KERNEL__
	reg_dma_start.bit.RX1DMASTADR = phy_addr & 0xFFFFFFFF; //dma_getPhyAddr(dma_start_addr);
	reg_dma_start_msb.bit.RX1DMASTADRMSB = phy_addr >> 32;
	reg_dma_bufsz.bit.RX1DMABUFSZ = dma_buffer_size;
#else // pcie
	reg_dma_start.bit.TX1DMASTADR  = pa.addr32.low ;//dma_getPhyAddr(dma_start_addr);
	reg_dma_start_msb.bit.TX1DMASTADRMSB = pa.addr32.high ;
	reg_dma_bufsz.bit.TX1DMABUFSZ   = dma_buffer_size;
#endif



	if (dma_channel == 0) {
        DAI_SETREG(DAI_RX1DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
        DAI_SETREG(DAI_RX1DMASTARTMSB_REG_OFS,reg_dma_start_msb.reg);
        DAI_SETREG(DAI_RX1DMASTART_REG_OFS,   reg_dma_start.reg);
	} else if (dma_channel == 1) {
		DAI_SETREG(DAI_TX2DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
        DAI_SETREG(DAI_TX2DMASTARTMSB_REG_OFS,reg_dma_start_msb.reg);
		DAI_SETREG(DAI_TX2DMASTART_REG_OFS,   reg_dma_start.reg);
	}

	//DBG_WRN("[TX%d] Addr=0x%08lX pAddr=0x%08lX Size=0x%08X(words)\r\n", (int)dma_channel + 1, (uintptr_t)dma_start_addr, (uintptr_t)phy_addr, (int)dma_buffer_size);

#if DAI_DBG_MSG
	DBG_WRN("[TX%d] Addr=0x%08X Size=0x%08X(words)\r\n", (int)dma_channel + 1, (unsigned int)dma_start_addr, (int)dma_buffer_size);
#endif
}

/**
    Set Record DMA parameter

    Set Record DMA starting address, buffer size.

    @param[in] dma_channel       Record DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[in] dma_start_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] dma_buffer_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
void dai_set_rx_dma_para(int id, int ddr_id,UINT32 dma_channel, uintptr_t dma_start_addr, UINT32 dma_buffer_size)
{
	T_DAI_RX2DMASTART_REG   reg_dma_start = {0};
	T_DAI_RX2DMASTARTMSB_REG reg_dma_start_msb = {0};
	T_DAI_RX2DMABUFSIZE_REG reg_dma_bufsz  = {0};
#ifdef __KERNEL__
    UINT64 phy_addr;
#elif defined(__FREERTOS)
    //UINT64 phy_addr;
    SYS_PA pa;
#endif

	if (dma_start_addr & 0x03) {
		DBG_WRN("DMA address should be word align, but 0x%x, align it to 0x%x\r\n", (unsigned int)dma_start_addr, (unsigned int)(dma_start_addr & (~0x03)));
	}
	if (dma_buffer_size & 0x0F) {
		DBG_WRN("DMA size must be multiples of 2 words (%d)\r\n", (int)dma_buffer_size);
	}
#ifdef __KERNEL__
	//fmem_dcache_sync((void *)dma_start_addr, dma_buffer_size * 4, DMA_BIDIRECTIONAL);
#elif defined(__FREERTOS)
	dma_flushReadCache(dma_start_addr, dma_buffer_size * 4);
#endif

#ifdef __KERNEL__
	//fmem_dcache_sync((void *)dma_start_addr, dma_buffer_size * 4, DMA_BIDIRECTIONAL);
	phy_addr = dai_platform_va2pa(dma_start_addr);
	reg_dma_start.bit.RX2DMASTADR = phy_addr & 0xFFFFFFFF;
#elif defined(__FREERTOS)
    //phy_addr = dma_start_addr;
    pa = sys_va2pa(dma_start_addr);
    #if DAI3_DBG_MSG
	DBG_WRN("VA 0x12: PA: 0x%llx (0x%lx, 0x%lx)\r\n", pa.addr64, pa.addr32.high, pa.addr32.low);
	#endif
#endif

#ifdef __KERNEL__
	reg_dma_start.bit.RX2DMASTADR  = phy_addr & 0xFFFFFFFF;//dma_getPhyAddr(dma_start_addr);
	reg_dma_start_msb.bit.RX2DMASTADRMSB = phy_addr >> 32;
	reg_dma_bufsz.bit.RX2DMABUFSZ   = dma_buffer_size;
#else // pcie
	reg_dma_start.bit.RX1DMASTADR  = pa.addr32.low ;//dma_getPhyAddr(dma_start_addr);
	reg_dma_start_msb.bit.RX1DMASTADRMSB = pa.addr32.high ;
	reg_dma_bufsz.bit.RX1DMABUFSZ   = dma_buffer_size;
#endif



	if (dma_channel == 0) {
        DAI_SETREG(DAI_RX2DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
        DAI_SETREG(DAI_RX2DMASTARTMSB_REG_OFS,reg_dma_start_msb.reg);
        DAI_SETREG(DAI_RX2DMASTART_REG_OFS,   reg_dma_start.reg);

	} else if (dma_channel == 1) {
		DAI_SETREG( DAI_1CHRX2DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
        DAI_SETREG( DAI_1CHRX2DMASTARTMSB_REG_OFS,reg_dma_start_msb.reg);
		DAI_SETREG( DAI_1CHRX2DMASTART_REG_OFS,   reg_dma_start.reg);
	}

#if 0
	//DBG_ERR(" reg_dma_bufsz.reg = 0x%08x \r\n",   (UINT32)reg_dma_bufsz.reg );
    //DBG_ERR(" reg_dma_start_msb.reg = 0x%08x \r\n",   (UINT32)reg_dma_start_msb.reg);
    DBG_ERR(" reg_dma_start.reg = 0x%08x \r\n",   (UINT32)reg_dma_start.reg );
#endif

#if 0
	DBG_WRN("[RX%d] Addr=0x%08X Size=0x%08X(words)\r\n", (int)dma_channel + 1, (unsigned int)dma_start_addr, (int)dma_buffer_size);
	DBG_WRN("MSB = 0x%x  , LSB = 0x%08x\r\n",(unsigned int) (phy_addr >> 32),(unsigned int)(phy_addr & 0xFFFFFFFF));
#endif

}

void dai_set_rx_muti_ch_dma_para(int id, int ddr_id,UINT32 dma_channel, uintptr_t dma_start_addr, UINT32 dma_buffer_size)
{
	T_DAI_RX1DMASTART_REG   reg_dma_start = {0};
	T_DAI_RX1DMASTARTMSB_REG   reg_dma_start_msb = {0};
	T_DAI_RX1DMABUFSIZE_REG reg_dma_bufsz = {0};
    UINT64 phy_addr;

	if (dma_start_addr & 0x03) {
		DBG_WRN("DMA address should be word align, but 0x%x, align it to 0x%x\r\n", (unsigned int)dma_start_addr, (unsigned int)(dma_start_addr & (~0x03)));
	}
	if (dma_buffer_size & 0x0F) {
		DBG_WRN("DMA size must be multiples of 2 words (%d)\r\n", (int)dma_buffer_size);
	}

	//fmem_dcache_sync((void *)dma_start_addr, dma_buffer_size * 4, DMA_BIDIRECTIONAL);


	// phy_addr = dai_platform_pcie_addr(id,ddr_id,dma_start_addr);
	phy_addr = dma_start_addr;

	reg_dma_start.bit.RX1DMASTADR  = phy_addr & 0xFFFFFFFF;//dma_getPhyAddr(dma_start_addr);
	reg_dma_start_msb.bit.RX1DMASTADRMSB = phy_addr >> 32;
	reg_dma_bufsz.bit.RX1DMABUFSZ   = dma_buffer_size;


	switch(dma_channel){
		case 0:
			DAI_SETREG(DAI_RX1DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX1DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX1DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 1:
			DAI_SETREG(DAI_RX2DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX2DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX2DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 2:
			DAI_SETREG(DAI_RX3DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX3DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX3DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 3:
			DAI_SETREG(DAI_RX4DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX4DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX4DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 4:
			DAI_SETREG(DAI_RX5DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX5DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX5DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 5:
			DAI_SETREG(DAI_RX6DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX6DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX6DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 6:
			DAI_SETREG(DAI_RX7DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX7DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX7DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 7:
			DAI_SETREG(DAI_RX8DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX8DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX8DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 8:
			DAI_SETREG(DAI_RX9DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX9DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX9DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 9:
			DAI_SETREG(DAI_RX10DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX10DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX10DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 10:
			DAI_SETREG(DAI_RX11DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX11DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX11DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 11:
			DAI_SETREG(DAI_RX12DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX12DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX12DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 12:
			DAI_SETREG(DAI_RX13DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX13DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX13DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 13:
			DAI_SETREG(DAI_RX14DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX14DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX14DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 14:
			DAI_SETREG(DAI_RX15DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX15DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX15DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 15:
			DAI_SETREG(DAI_RX16DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX16DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX16DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 16:
			DAI_SETREG(DAI_RX17DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX17DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX17DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 17:
			DAI_SETREG(DAI_RX18DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX18DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX18DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 18:
			DAI_SETREG(DAI_RX19DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX19DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX19DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 19:
			DAI_SETREG(DAI_RX20DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX20DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX20DMASTART_REG_OFS,   reg_dma_start.reg);
			break;
		case 20:
			DAI_SETREG(DAI_RX21DMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);
			DAI_SETREG(DAI_RX21DMASTARTMSB_REG_OFS,  reg_dma_start_msb.reg);
			DAI_SETREG(DAI_RX21DMASTART_REG_OFS,   reg_dma_start.reg);
			break;

		default:
			DBG_WRN("Error Rx DMA ch number [%d]\r\n", (int)dma_channel);
			break;
	}
}


/**
    Set Playback Loopback DMA parameter

    Set Playback Loopback DMA starting address, buffer size.

    @param[in] dma_start_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] dma_buffer_size      DMA buffer size (unit: 16 words aligned)

    @return void
*/
void dai_set_txlb_dma_para(uintptr_t dma_start_addr, UINT32 dma_buffer_size)
{
	T_DAI_TXLBDMASTART_REG   reg_dma_start = {0};
	T_DAI_TXLBDMABUFSIZE_REG reg_dma_bufsz  = {0};

	uintptr_t phy_addr;

	if (dma_start_addr & 0x03) {
		DBG_WRN("DMA address should be word align, but 0x%x, align it to 0x%x\r\n", (unsigned int)dma_start_addr, (unsigned int)(dma_start_addr & (~0x03)));
	}
	if (dma_buffer_size & 0x0F) {
		DBG_WRN("DMA size must be multiples of 2 words (%d)\r\n", (int)dma_buffer_size);
	}

#ifdef __KERNEL__
	fmem_dcache_sync((void *)dma_start_addr, dma_buffer_size * 4, DMA_BIDIRECTIONAL);
	phy_addr = dai_platform_va2pa(dma_start_addr);
	reg_dma_start.bit.TXLBDMASTADR = phy_addr & 0xFFFFFFFF;
#elif defined(__FREERTOS)
	dma_flushReadCache(dma_start_addr, dma_buffer_size * 4);
	reg_dma_start.bit.TXLBDMASTADR  = dma_get_phy_addr(dma_start_addr);
#endif

	reg_dma_bufsz.bit.TXLBDMABUFSZ  = dma_buffer_size;



	DAI_SETREG(DAI_TXLBDMASTART_REG_OFS,   reg_dma_start.reg);
	DAI_SETREG(DAI_TXLBDMABUFSIZE_REG_OFS, reg_dma_bufsz.reg);

#if DAI_DBG_MSG
	DBG_WRN("[TXLB] Addr=0x%08X Size=0x%08X(words)\r\n", (unsigned int)dma_start_addr, (int)dma_buffer_size);
#endif
}


/**
    Get Playback DMA parameter

    Get Playback DMA starting address, buffer size.

    @param[in] dma_channel       Playback DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[out] dma_start_addr     DMA start address. (unit: byte, should be word-alignment)
    @param[out] dma_buffer_size     DMA buffer size (unit: word)

    @return void
*/
void dai_get_tx_dma_para(UINT32 dma_channel, uintptr_t *p_start_addr, UINT32 *p_buffer_size)
{
	if (dma_channel == 0) {
		if (p_start_addr != NULL) {
			*p_start_addr = dma_get_noncache_addr(DAI_GETREG(DAI_RX1DMASTART_REG_OFS));
		}

		if (p_buffer_size != NULL) {
			*p_buffer_size = DAI_GETREG(DAI_RX1DMABUFSIZE_REG_OFS);
		}
	} else if (dma_channel == 1) {
		if (p_start_addr != NULL) {
			*p_start_addr = dma_get_noncache_addr(DAI_GETREG(DAI_TX2DMASTART_REG_OFS));
		}

		if (p_buffer_size != NULL) {
			*p_buffer_size = DAI_GETREG(DAI_TX2DMABUFSIZE_REG_OFS);
		}
	}
}


/**
    Get Record DMA parameter

    Get Record DMA starting address, buffer size.

    @param[in] dma_channel       Record DMA Channel Selection. Valid value is 0 or 1 for NT96680.
    @param[out] dma_start_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[out] dma_buffer_size      DMA buffer size (unit: word)

    @return void
*/
void dai_get_rx_dma_para(UINT32 dma_channel, uintptr_t *p_start_addr, UINT32 *p_buffer_size)
{
	if (dma_channel == 0) {
		if (p_start_addr != NULL) {
			*p_start_addr = dma_get_noncache_addr(DAI_GETREG(DAI_RX2DMASTART_REG_OFS));
		}

		if (p_buffer_size != NULL) {
			*p_buffer_size = DAI_GETREG(DAI_RX2DMABUFSIZE_REG_OFS);
		}
	} else if (dma_channel == 1) {
		if (p_start_addr != NULL) {
			*p_start_addr = dma_get_noncache_addr(DAI_GETREG(DAI_RX2DMASTART_REG_OFS));
		}

		if (p_buffer_size != NULL) {
			*p_buffer_size = DAI_GETREG(DAI_RX2DMABUFSIZE_REG_OFS);
		}
	}
}

/**
    Get Playback Loopback DMA parameter

    Get Playback Loopback DMA starting address, buffer size.

    @param[in] dma_start_addr      DMA start address. (unit: byte, should be word-alignment)
    @param[in] dma_buffer_size      DMA buffer size (unit: word)

    @return void
*/
void dai_get_txlb_dma_para(uintptr_t *p_start_addr, UINT32 *p_buffer_size)
{
	if (p_start_addr != NULL) {
		*p_start_addr = dma_get_noncache_addr(DAI_GETREG(DAI_TXLBDMASTART_REG_OFS));
	}

	if (p_buffer_size != NULL) {
		*p_buffer_size = DAI_GETREG(DAI_TXLBDMABUFSIZE_REG_OFS);
	}
}


/**
    Get Playback DMA currrent address

    Get Playback DMA currrent address

    @param[in] dma_channel   Playback DMA Channel Selection. Valid value is 0 or 1 for NT96660.

    @return Playback DMA current address
*/
UINT32 dai_get_tx_dma_curaddr(UINT32 dma_channel)
{
	if (dma_channel == 1) {
		return dma_get_noncache_addr(DAI_GETREG(DAI_TX2DMACURRENT_REG_OFS));
	} else {
		return dma_get_noncache_addr(DAI_GETREG(DAI_TX1DMACURRENT_REG_OFS));
	}
}

/**
    Get Record DMA currrent address

    Get Record DMA currrent address

    @param[in] dma_channel   Record DMA Channel Selection. Valid value is 0 or 1 for NT96660.

    @return Record DMA current address
*/
UINT32 dai_get_rx_dma_curaddr(UINT32 dma_channel)
{
	if (dma_channel == 1) {
		return dma_get_noncache_addr(DAI_GETREG(DAI_RX2DMACURRENT_REG_OFS));
	} else {
		return dma_get_noncache_addr(DAI_GETREG(DAI_RX1DMACURRENT_REG_OFS));
	}
}


uintptr_t dai_get_rx_muti_ch_dma_curaddr(int id, int ddr_id,UINT32 dma_channel)
{
	//T_DAI_RX1DMACURR_REG   reg_dma_curr = {0};
	//T_DAI_RX1DMACURRMSB_REG   reg_dma_curr_msb = {0};

	uintptr_t curr_addr = 0;
	uintptr_t curr_msb_addr = 0;
    uintptr_t phy_addr;

	switch(dma_channel){
		case 0:
			curr_msb_addr = DAI_GETREG(DAI_RX1DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX1DMACURR_REG_OFS);
			break;
		case 1:
			curr_msb_addr = DAI_GETREG(DAI_RX2DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX2DMACURR_REG_OFS);
			break;
		case 2:
			curr_msb_addr = DAI_GETREG(DAI_RX3DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX3DMACURR_REG_OFS);
			break;
		case 3:
			curr_msb_addr = DAI_GETREG(DAI_RX4DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX4DMACURR_REG_OFS);
			break;
		case 4:
			curr_msb_addr = DAI_GETREG(DAI_RX5DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX5DMACURR_REG_OFS);
			break;
		case 5:
			curr_msb_addr = DAI_GETREG(DAI_RX6DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX6DMACURR_REG_OFS);
			break;
		case 6:
			curr_msb_addr = DAI_GETREG(DAI_RX7DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX7DMACURR_REG_OFS);
			break;
		case 7:
			curr_msb_addr = DAI_GETREG(DAI_RX8DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX8DMACURR_REG_OFS);
			break;
		case 8:
			curr_msb_addr = DAI_GETREG(DAI_RX9DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX9DMACURR_REG_OFS);
			break;
		case 9:
			curr_msb_addr = DAI_GETREG(DAI_RX10DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX10DMACURR_REG_OFS);
			break;
		case 10:
			curr_msb_addr = DAI_GETREG(DAI_RX11DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX11DMACURR_REG_OFS);
			break;
		case 11:
			curr_msb_addr = DAI_GETREG(DAI_RX12DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX12DMACURR_REG_OFS);
			break;
		case 12:
			curr_msb_addr = DAI_GETREG(DAI_RX13DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX13DMACURR_REG_OFS);
			break;
		case 13:
			curr_msb_addr = DAI_GETREG(DAI_RX14DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX14DMACURR_REG_OFS);
			break;
		case 14:
			curr_msb_addr = DAI_GETREG(DAI_RX15DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX15DMACURR_REG_OFS);
			break;
		case 15:
			curr_msb_addr = DAI_GETREG(DAI_RX16DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX16DMACURR_REG_OFS);
			break;
		case 16:
			curr_msb_addr = DAI_GETREG(DAI_RX17DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX17DMACURR_REG_OFS);
			break;
		case 17:
			curr_msb_addr = DAI_GETREG(DAI_RX18DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX18DMACURR_REG_OFS);
			break;
		case 18:
			curr_msb_addr = DAI_GETREG(DAI_RX19DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX19DMACURR_REG_OFS);
			break;
		case 19:
			curr_msb_addr = DAI_GETREG(DAI_RX20DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX20DMACURR_REG_OFS);
			break;
		case 20:
			curr_msb_addr = DAI_GETREG(DAI_RX21DMACURRMSB_REG_OFS);
			curr_addr     = DAI_GETREG(DAI_RX21DMACURR_REG_OFS);
			break;

		default:
			DBG_WRN("Error Rx DMA ch number [%d]\r\n", (int)dma_channel);
			break;
	}

	phy_addr = (curr_msb_addr << 32) | curr_addr;

	return phy_addr;
}



/**
    Get Playback Loopback DMA currrent address

    Get Playback Loopback DMA currrent address

    @return Playback Loopback DMA current address
*/
UINT32 dai_get_txlb_dma_curaddr(void)
{
	return dma_get_noncache_addr(DAI_GETREG(DAI_TXLBDMACURRENT_REG_OFS));
}

/**
    Clear DAI flag

    Clear DAI flag

    @param[in] int_flag  interrupt flags to clear.

    @return void
*/
void dai_clr_flg(DAI_INTERRUPT int_flag)
{
	// Clear Interrupt Flag

	clr_flg(FLG_ID_DAI, int_flag);
}


#endif
//@}

/*
    DAI self Debug Mode Enable

    After DAI Debug enabled, the DAI RX channel's input is switched from CODEC interface to DAI Playback Mixer output.
*/
void dai_debug(BOOL b_en)
{
	T_DAI_DBG_CONFIG_REG    reg_dbg;

	reg_dbg.reg = DAI_GETREG(DAI_DBG_CONFIG_REG_OFS);
	reg_dbg.bit.DBGEN = b_en > 0;
	DAI_SETREG(DAI_DBG_CONFIG_REG_OFS, reg_dbg.reg);
}

/*
    DAI Debug Channel Enable for EAC

    b_mode_ad is TRUE for AD. b_mode_ad is FALSE for DA.
*/
void dai_debug_eac(BOOL b_en, BOOL b_mode_ad)
{
	T_DAI_DBG_CONFIG_REG    reg_dbg;

	reg_dbg.reg = DAI_GETREG(DAI_DBG_CONFIG_REG_OFS);
	reg_dbg.bit.DBGEN        = 0;
	reg_dbg.bit.EAC_DBG_EN   = b_en > 0;
	reg_dbg.bit.EAC_DBG_MODE = b_mode_ad > 0;
	DAI_SETREG(DAI_DBG_CONFIG_REG_OFS, reg_dbg.reg);
}

/*
    Set Debug Channel DMA parameter

    Set Debug Channel DMA starting address, buffer size.
*/
void dai_set_debug_dma_para(uintptr_t dma_start_addr, UINT32 dma_buffer_size)
{
	T_DAI_DBG_ADDR_REG   reg_dma_start = {0};
	T_DAI_DBG_SIZE_REG   reg_dma_bufsz  = {0};

	reg_dma_start.bit.DMASTADR     = dma_get_phy_addr(dma_start_addr);
	reg_dma_bufsz.bit.DMABUFSZ      = dma_buffer_size;

	if (dma_start_addr) {
		DAI_SETREG(DAI_DBG_ADDR_REG_OFS,    reg_dma_start.reg);
	}

	if (dma_buffer_size) {
		DAI_SETREG(DAI_DBG_SIZE_REG_OFS,    reg_dma_bufsz.reg);
	}
}

/*
    Get Debug Done Status parameter
*/
BOOL dai_get_debug_status(void)
{
	T_DAI_DBG_STS_REG   reg_status = {0};

	reg_status.reg = DAI_GETREG(DAI_DBG_STS_REG_OFS);
	return reg_status.bit.DONE;
}

/*
    Get Debug Done Status parameter
*/
void dai_clr_debug_status(void)
{
	T_DAI_DBG_STS_REG   reg_status;

	reg_status.reg = DAI_GETREG(DAI_DBG_STS_REG_OFS);
	reg_status.bit.DONE = 1;
	DAI_SETREG(DAI_DBG_STS_REG_OFS, reg_status.reg);
}


#if 0//def __KERNEL__
EXPORT_SYMBOL(dai_create_resource);
EXPORT_SYMBOL(dai_release_resource);
EXPORT_SYMBOL(dai_enableclk);
EXPORT_SYMBOL(dai_disableclk);
EXPORT_SYMBOL(dai_setclkrate);
EXPORT_SYMBOL(dai_isr);
EXPORT_SYMBOL(dai_lock);
EXPORT_SYMBOL(dai_unlock);
EXPORT_SYMBOL(dai_wait_interrupt);
EXPORT_SYMBOL(dai_select_pinmux);
EXPORT_SYMBOL(dai_select_mclk_pinmux);
EXPORT_SYMBOL(dai_open);
EXPORT_SYMBOL(dai_close);
EXPORT_SYMBOL(dai_set_config);
EXPORT_SYMBOL(dai_get_config);
EXPORT_SYMBOL(dai_set_i2s_config);
EXPORT_SYMBOL(dai_get_i2s_config);
EXPORT_SYMBOL(dai_set_tx_config);
EXPORT_SYMBOL(dai_get_tx_config);
EXPORT_SYMBOL(dai_set_txlb_config);
EXPORT_SYMBOL(dai_get_txlb_config);
EXPORT_SYMBOL(dai_set_rx_config);
EXPORT_SYMBOL(dai_get_rx_config);
EXPORT_SYMBOL(dai_enable_tx_dma);
EXPORT_SYMBOL(dai_enable_rx_dma);
EXPORT_SYMBOL(dai_enable_txlb_dma);
EXPORT_SYMBOL(dai_enable_tx);
EXPORT_SYMBOL(dai_enable_rx);
EXPORT_SYMBOL(dai_enable_txlb);
EXPORT_SYMBOL(dai_enable_dai);
EXPORT_SYMBOL(dai_is_dai_enable);
EXPORT_SYMBOL(dai_is_tx_enable);
EXPORT_SYMBOL(dai_is_rx_enable);
EXPORT_SYMBOL(dai_is_txlb_enable);
EXPORT_SYMBOL(dai_is_txrx_enable);
EXPORT_SYMBOL(dai_is_tx_dma_enable);
EXPORT_SYMBOL(dai_is_rx_dma_enable);
EXPORT_SYMBOL(dai_is_txlb_dma_enable);
EXPORT_SYMBOL(dai_is_dma_enable);
EXPORT_SYMBOL(dai_set_tx_dma_para);
EXPORT_SYMBOL(dai_set_rx_dma_para);
EXPORT_SYMBOL(dai_set_txlb_dma_para);
EXPORT_SYMBOL(dai_get_tx_dma_para);
EXPORT_SYMBOL(dai_get_rx_dma_para);
EXPORT_SYMBOL(dai_get_txlb_dma_para);
EXPORT_SYMBOL(dai_get_tx_dma_curaddr);
EXPORT_SYMBOL(dai_get_rx_dma_curaddr);
EXPORT_SYMBOL(dai_get_txlb_dma_curaddr);
EXPORT_SYMBOL(dai_clr_flg);
EXPORT_SYMBOL(dai_debug);
EXPORT_SYMBOL(dai_debug_eac);
EXPORT_SYMBOL(dai_set_debug_dma_para);
EXPORT_SYMBOL(dai_get_debug_status);
EXPORT_SYMBOL(dai_clr_debug_status);
#endif


