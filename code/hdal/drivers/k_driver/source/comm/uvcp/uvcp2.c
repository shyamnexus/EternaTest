#include <kwrap/util.h>
#include <linux/gpio.h>
#include <plat/nvt-gpio.h>

#include "uvcp2.h"
#include "uvcp_reg.h"
#include "uvcp_drv.h"
#include "uvcp_main.h"
#include "uvcp_dbg.h"

#include "uvcp2_int.h"

#define __MODULE__ nvt_uvcp
#define __DBGLVL__ 8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__ "*"

unsigned int nvt_uvcp2_debug_level = NVT_DBG_WRN;


static  VK_DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)

extern uintptr_t _UVCP_REG_BASE_ADDR[MODULE_REG_NUM];
#define UVCP_SETREG(_OFS, value)          OUTW(_UVCP_REG_BASE_ADDR[1]+(_OFS), (value))
#define UVCP_GETREG(_OFS)                 INW(_UVCP_REG_BASE_ADDR[1]+(_OFS))


static BOOL _uvcp2_opened;

static UINT32 uvcp2_max_packet_size = 1024;
static UINT32 uvcp2_reference_clk = 30; // 30 Mhz
static UINT32 uvcp2_counter_start;
static BOOL   uvcp2_fid;
static UINT32 uvcp2_last_counter_val;
static UINT32 uvcp2_last_sof_val;
static uintptr_t uvcp2_output_address;

//static ID SEMID_UVCP;
static ID FLG_ID_UVCP2;

#ifdef _NVT_EMULATION_
static UINT32 uvcp2_lastout;
#endif

irqreturn_t uvcp2_isr(int irq, void *devid)
{
	T_UVCP_INTSTS_REG regintsts;

	//DBG_DUMP("uvcp2_isr 1\r\n");

	regintsts.reg  = UVCP_GETREG(UVCP_INTSTS_REG_OFS);
	regintsts.reg &= UVCP_GETREG(UVCP_INTEN_REG_OFS);
	UVCP_SETREG(UVCP_INTSTS_REG_OFS, regintsts.reg);


	if (regintsts.bit.BREAKPOINT) {
		//DBG_DUMP("B");
		iset_flg(FLG_ID_UVCP2, FLGPTN_UVCP2_BRK);
	}

	if(regintsts.bit.COMPLETE) {
#ifdef _NVT_EMULATION_
		UINT8 *pbuf;
		UINT32 bound;

		bound = UVCP_GETREG(UVCP_OUTSIZE_REG_OFS);
		if((bound%uvcp2_max_packet_size) > 0)
			bound = bound - (bound%uvcp2_max_packet_size);
		else
			bound = bound - uvcp2_max_packet_size;

		pbuf = (UINT8 *) (UVCP_GETREG(UVCP_OUTADDR_REG_OFS)+bound)
		bound = (pbuf[9]<<24)+(pbuf[8]<<16)+(pbuf[7]<<8)+(pbuf[6]<<0);
		uvcp2_last_counter_val = bound;
		//DBG_DUMP("LSTCNT=0x%08X\r\n", (int)uvcp2_last_counter_val);

		bound = (pbuf[11]<<8)+(pbuf[10]<<0);
		uvcp2_last_sof_val = bound & 0x7FF;

//#ifdef _NVT_EMULATION_
		uvcp2_lastout = UVCP_GETREG(UVCP_OUTSIZE_REG_OFS);
#endif

		iset_flg(FLG_ID_UVCP2, FLGPTN_UVCP2);
	}

	return IRQ_HANDLED;
}

#if 1//ndef __KERNEL__
static BOOL uvcp2_init_done = 0;
void uvcp2_init(void)
{
	vos_flag_create(&FLG_ID_UVCP2, NULL, "FLG_ID_UVCP2");
	//vos_sem_create(&SEMID_UVCP, 1, "SEMID_UVCP");
}

void uvcp2_uninit(void)
{
	//vos_sem_destroy(SEMID_UVCP);
	vos_flag_destroy(FLG_ID_UVCP2);
}
#endif

#if UVCP2_POLLING
static ER uvcp2_lock(void)
{
#ifdef __KERNEL__
	SEM_WAIT(SEMID_UVCP);
#else
	vos_sem_wait(SEMID_UVCP);
#endif
	return E_OK;
}

/*
    LVDS ReSource Un-Lock
*/
static ER uvcp2_unlock(void)
{
#ifdef __KERNEL__
	SEM_SIGNAL(SEMID_UVCP);
#else
	vos_sem_sig(SEMID_UVCP);
#endif
	return E_OK;
}
#endif

ER uvcp2_open(void)
{
	T_UVCP_AXI_MON_REG aximon;
	T_UVCP_OP_REG opreg;

	if(_uvcp2_opened) {
		DBG_ERR("driver already opened\r\n");
		return E_OACV;
	}

	_uvcp2_opened = TRUE;

	if(!uvcp2_init_done) {
		uvcp2_init();
		uvcp2_init_done = 1;
	}

#ifndef __KERNEL__
	// open clock
	pll_enable_clock(UVCP2_CLK);

	// open irq
	request_irq(INT_ID_UVCP2, uvcp2_isr ,IRQF_TRIGGER_HIGH, "uvcp2", 0);
#else
	DBG_DUMP("open axi ch\r\n");
	aximon.reg = UVCP_GETREG(UVCP_AXI_MON_REG_OFS);
	aximon.bit.UVCP_CH_DIS = 0;
	UVCP_SETREG(UVCP_AXI_MON_REG_OFS, aximon.reg);

	opreg.reg = UVCP_GETREG(UVCP_OP_REG_OFS);
	opreg.bit.BURST_32W = 1;
	UVCP_SETREG(UVCP_OP_REG_OFS, opreg.reg);
#endif

	#if !UVCP2_POLLING
	UVCP_SETREG(UVCP_INTEN_REG_OFS, 0x3);
	#endif

	// controller init
	UVCP_SETREG(UVCP_BREAKSIZE_REG_OFS, 4);

	return E_OK;
}


ER uvcp2_close(void)
{
	if(!_uvcp2_opened) {
		DBG_ERR("driver not opened\r\n");
		return E_OACV;
	}

#ifndef __KERNEL__
	free_irq(INT_ID_UVCP2, 0);

	pll_disable_clock(UVCP2_CLK);
#endif

	_uvcp2_opened = FALSE;
	return E_OK;
}

BOOL uvcp2_is_opened(void)
{
	return _uvcp2_opened;
}

ER uvcp2_trigger(BOOL eof, BOOL wait_break)
{
	T_UVCP_OP_REG		regop;
	T_UVCP_CTRL2_REG	regctrl2;
	unsigned long		flags;
	FLGPTN              uiflag = 0;

	loc_cpu(flags);

	regop.reg = UVCP_GETREG(UVCP_OP_REG_OFS);
	if(regop.bit.START) {
		unl_cpu(flags);
		DBG_ERR("\r\n");
		return E_OACV;
	}

	#if UVCP2_POLLING
	UVCP_SETREG(UVCP_INTSTS_REG_OFS, 0x3);
	#endif

	regctrl2.reg = UVCP_GETREG(UVCP_CTRL2_REG_OFS);
	regctrl2.bit.EOF_SET = eof;
	regctrl2.bit.FID_VAL = uvcp2_fid;
	UVCP_SETREG(UVCP_CTRL2_REG_OFS, regctrl2.reg);

	clr_flg(FLG_ID_UVCP2, FLGPTN_UVCP2|FLGPTN_UVCP2_BRK);

	regop.bit.START = 1;
	UVCP_SETREG(UVCP_OP_REG_OFS, regop.reg);

	unl_cpu(flags);

	if(wait_break) {
		if (vos_flag_wait_timeout(&uiflag, FLG_ID_UVCP2, FLGPTN_UVCP2_BRK, TWF_ORW | TWF_CLR, vos_util_msec_to_tick(2000)) != E_OK) {
			DBG_ERR("wait_break timeout\r\n");
			return E_SYS;
		}
	}

	if (eof) {
		uvcp2_fid = !uvcp2_fid;
	}

	return E_OK;
}


ER uvcp2_wait_complete(void)
{
#if UVCP2_POLLING
	UINT32 timeot = 0;
	UINT32 regval;

	uvcp2_lock();

	do {
		regval = UVCP_GETREG(UVCP_INTSTS_REG_OFS);
		timeot++;
		if(timeot > 10000000)
			break;
	} while(!(regval & 0x1));

	if(regval & 0x1) {
		UVCP_SETREG(UVCP_INTSTS_REG_OFS, 0x1);
	} else {
		uvcp2_unlock();
		DBG_ERR("uvcp2_wait_complete timeout\r\n");
		return E_SYS;
	}

	uvcp2_unlock();

#else

	FLGPTN              uiflag = 0;

	if (vos_flag_wait_timeout(&uiflag, FLG_ID_UVCP2, FLGPTN_UVCP2, TWF_ORW | TWF_CLR, vos_util_msec_to_tick(2000)) != E_OK) {
		DBG_ERR("uvcp2_wait_complete timeout\r\n");
		return E_SYS;
	}
#endif

	return E_OK;
}

ER uvcp2_wait_break(void)
{
	FLGPTN              uiflag = 0;

	if (vos_flag_wait_timeout(&uiflag, FLG_ID_UVCP2, FLGPTN_UVCP2_BRK, TWF_ORW | TWF_CLR, vos_util_msec_to_tick(2000)) != E_OK) {
		DBG_ERR("uvcp2_wait_break timeout\r\n");
		return E_SYS;
	}

	return E_OK;
}


ER uvcp2_set_config(UVCP2_CONFIG_ID configID, UINT32 configContext)
{
	unsigned long	flags;

	loc_cpu(flags);

	switch (configID) {
	case UVCP2_CONFIG_ID_HEADER_LENGTH:
	{
		T_UVCP_CTRL0_REG regctrl0;
		T_UVCP_CTRL2_REG regctrl2;

		if ((configContext == 2)||(configContext == 12)) {
			regctrl2.reg = UVCP_GETREG(UVCP_CTRL2_REG_OFS);
			regctrl2.bit.HEADER_LEN_VALUE 	= configContext;
			regctrl2.bit.PTS_VAL 			= (configContext>2);
			regctrl2.bit.SCR_VAL 			= (configContext>2);
			#if 0
			regctrl2.bit.RES_VAL 			= 1;
			regctrl2.bit.STI_VAL 			= 1;
			regctrl2.bit.ERR_VAL 			= 1;
			regctrl2.bit.EOH_VAL 			= 0;
			DBG_DUMP("TESTONLY\r\n");
			#endif
			UVCP_SETREG(UVCP_CTRL2_REG_OFS, regctrl2.reg);

			regctrl0.reg = UVCP_GETREG(UVCP_CTRL0_REG_OFS);
			regctrl0.bit.HEADER_LEN_CTRL = (configContext>2);
			regctrl0.bit.PAYLOAD_SLICE_SIZE = uvcp2_max_packet_size - regctrl2.bit.HEADER_LEN_VALUE;
			UVCP_SETREG(UVCP_CTRL0_REG_OFS, regctrl0.reg);

		} else {
			DBG_ERR("err header length %d\r\n", configContext);
		}
	} break;

	case UVCP2_CONFIG_ID_DATALEN_PER_UF:
	{
		T_UVCP_CTRL0_REG regctrl0;
		T_UVCP_CTRL2_REG regctrl2;

		uvcp2_max_packet_size = configContext;

		regctrl2.reg = UVCP_GETREG(UVCP_CTRL2_REG_OFS);

		regctrl0.reg = UVCP_GETREG(UVCP_CTRL0_REG_OFS);
		regctrl0.bit.PAYLOAD_SLICE_SIZE = uvcp2_max_packet_size - regctrl2.bit.HEADER_LEN_VALUE;
		UVCP_SETREG(UVCP_CTRL0_REG_OFS, regctrl0.reg);
	} break;

	case UVCP2_CONFIG_ID_FID:
	{
		uvcp2_fid = (configContext>0);
	} break;

	case UVCP2_CONFIG_ID_REFERENCE_CLK:
	{
		T_UVCP_CTRL1_REG regctrl1;
		T_UVCP_CTRL4_REG regctrl4;
		UINT32 count;

		uvcp2_reference_clk = configContext;

		count = (UINT32)(125* uvcp2_reference_clk);

		regctrl1.reg = UVCP_GETREG(UVCP_CTRL1_REG_OFS);
		regctrl1.bit.SCR_INCRE31_0 = count;
		#if 0
		regctrl1.bit.SCR_INCRE42_32 = 0x2;
		regctrl1.bit.SCR_REPEAT     = 0x1;
		DBG_DUMP("TESTONLY\r\n");
		#endif
		UVCP_SETREG(UVCP_CTRL1_REG_OFS, regctrl1.reg);

		regctrl4.reg = UVCP_GETREG(UVCP_CTRL4_REG_OFS);
		regctrl4.bit.SCR31_0_START_VAL = uvcp2_counter_start - regctrl1.bit.SCR_INCRE31_0;
		UVCP_SETREG(UVCP_CTRL4_REG_OFS, regctrl4.reg);
	}
	break;

	case UVCP2_CONFIG_ID_SOF_START_VAL:
	{
		T_UVCP_CTRL5_REG regctrl5;

		regctrl5.reg = UVCP_GETREG(UVCP_CTRL5_REG_OFS);
		regctrl5.bit.SCR42_32_START_VAL = configContext-1;
		UVCP_SETREG(UVCP_CTRL5_REG_OFS, regctrl5.reg);
	}
	break;

	case UVCP2_CONFIG_ID_COUNTER_START_VAL:
	{
		T_UVCP_CTRL4_REG regctrl4;
		T_UVCP_CTRL1_REG regctrl1;

		uvcp2_counter_start = configContext;

		regctrl1.reg = UVCP_GETREG(UVCP_CTRL1_REG_OFS);
		regctrl4.reg = UVCP_GETREG(UVCP_CTRL4_REG_OFS);
		regctrl4.bit.SCR31_0_START_VAL = uvcp2_counter_start - regctrl1.bit.SCR_INCRE31_0;
		UVCP_SETREG(UVCP_CTRL4_REG_OFS, regctrl4.reg);
	}
	break;

	case UVCP2_CONFIG_ID_BREAK_SIZE:
	{
		if(configContext == 0)
			configContext = 2;

		UVCP_SETREG(UVCP_BREAKSIZE_REG_OFS, configContext);
	}
	break;

	case UVCP2_CONFIG_ID_PTS:
	{
		UVCP_SETREG(UVCP_CTRL3_REG_OFS, configContext);
	}
	break;

	default:
		break;
	}


	unl_cpu(flags);

	return E_OK;
}



UINT32 uvcp2_get_config(UVCP2_CONFIG_ID configID)
{
	UINT32 ret = 0;

	switch (configID) {
	case UVCP2_CONFIG_ID_GETOUT_SIZE:
	{
		ret = UVCP_GETREG(UVCP_OUTSIZE_REG_OFS);
#ifdef _NVT_EMULATION_
		if(ret == 0)
			ret = uvcp2_lastout;
#endif
	} break;

	case UVCP2_CONFIG_ID_FID:
	{
		ret = uvcp2_fid;
	} break;

	case UVCP2_CONFIG_ID_GET_COUNTER_END:
	{
		ret = uvcp2_last_counter_val;
	} break;

	case UVCP2_CONFIG_ID_GET_SOF_END:
	{
		ret = uvcp2_last_sof_val;
	} break;

	case UVCP2_CONFIG_ID_PTS:
	{
		ret = UVCP_GETREG(UVCP_CTRL3_REG_OFS);
	}
	break;

	case UVCP2_CONFIG_ID_GET_CNTSTEP:
	{
		T_UVCP_CTRL1_REG regctrl1;

		regctrl1.reg = UVCP_GETREG(UVCP_CTRL1_REG_OFS);
		ret = regctrl1.bit.SCR_INCRE31_0;
	}
	break;

	case UVCP2_CONFIG_ID_COUNTER_START_VAL:
	{
		ret = uvcp2_counter_start;
	}
	break;

	case UVCP2_CONFIG_ID_SOF_START_VAL:
	{
		T_UVCP_CTRL5_REG regctrl5;

		regctrl5.reg = UVCP_GETREG(UVCP_CTRL5_REG_OFS);
		ret = (regctrl5.bit.SCR42_32_START_VAL + 1)&0x7FF;
	}
	break;

	default:
		DBG_ERR("id err %d\r\n", (int)configID);
		break;

	}

	return ret;
}



void uvcp2_config_dma(uintptr_t input_address, UINT32 input_size, uintptr_t output_address, UINT32 *output_size)
{
	uvcp2_output_address = output_address;

	dma_flushWriteCache((uintptr_t)input_address, input_size);
#if _UVCP_KERN_SELFTEST
	dma_flushReadCache((uintptr_t)input_address, input_size);
	dma_flushWriteCache((uintptr_t)output_address, *output_size);
#endif
	dma_flushReadCache((uintptr_t)output_address, *output_size);

	UVCP_SETREG(UVCP_INADDR_REG_OFS, dma_getPhyAddr((uintptr_t)input_address));
	UVCP_SETREG(UVCP_INSIZE_REG_OFS, input_size);
	UVCP_SETREG(UVCP_OUTADDR_REG_OFS,dma_getPhyAddr((uintptr_t)output_address));

}




