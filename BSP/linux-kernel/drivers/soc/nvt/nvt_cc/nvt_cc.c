#ifdef __KERNEL__
#include <linux/soc/nvt/rcw_macro.h>
#else
#include <rcw_macro.h>
#endif
#include <plat/nvt_cc.h>
#include "nvt_cc_int.h"
#include "nvt_cc_reg.h"

static DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags)   spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags)   spin_unlock_irqrestore(&my_lock, flags)

#ifdef __KERNEL__
static void            *_REGIOBASE;
#else
static uintptr_t        _REGIOBASE;
#endif

static void cc_ack_core_cmd(CC_CORE_ID core_id);

#define CC_SEND_TIMEOUT_CNT   1000000
#define CC_GRANT_TIMEOUT_CNT  1000000
#define ENABLE_CC_ACK_INTERRUPT_MODE 1

#define DBG_ERR(fmt, args...)      printk("ERR:%s:" fmt, __func__, ##args);
#define DBG_WRN(fmt, args...)      printk("WRN:%s:" fmt, __func__, ##args);
#define DBG_DUMP(fmt, args...)     printk(fmt, ##args)
#define DBG_MSG(fmt, args...)      //printk(fmt, ##args)
#define DBG_IND(fmt, args...)      //printk(fmt, ##args)


#define RCW_LD2(RCW, grpidx)	t##RCW.reg = INW(_REGIOBASE+RCW##_OFS+(grpidx)*_REGGRPOFS)
#define RCW_ST2(RCW, grpidx)	OUTW(_REGIOBASE+RCW##_OFS+(grpidx)*_REGGRPOFS, t##RCW.reg)


static CC_HANDLER       cb_handler;
static CC_U32 g_cc_intsts;
static const int _REGGRPOFS = 4;
static CC_U32 g_local_core_id = CC_CORE_CPU1;
static CC_ULONG g_cc_cmdbuf_addr = 0;
static CC_I32 cc_get_int_grpidx(void)
{
	// 0x14, 0x24, 0x34, 0x04, 0xA4, 0xB4, 0xC4
	CC_I32 grpidx = 0;
#if defined(CONFIG_NVT_IVOT_PLAT_NS02301)
	switch (g_local_core_id) {
	case CC_CORE_CPU1:
		grpidx = 0;
		break;
	case CC_CORE_CPU2:
		grpidx = 1*4;
		break;
	case CC_CORE_MCU:
		grpidx = 2*4;
		break;
	default:
		DBG_ERR("unknow core_id = %d\r\n", g_local_core_id);
	}
#else
	switch (g_local_core_id) {
	case CC_CORE_CPU1:
		grpidx = 0;
		break;
	case CC_CORE_CPU2:
		grpidx = 1*4;
		break;
	case CC_CORE_CPU3:
		grpidx = 2*4;
		break;
	case CC_CORE_CPU4:
		grpidx = -4;
		break;
	#ifdef CC_SUPPORT_7_CORES
	case CC_CORE_DSP1:
		grpidx = 9*4;
		break;
	case CC_CORE_DSP2:
		grpidx = 10*4;
		break;
	case CC_CORE_MCU:
		grpidx = 11*4;
		break;
	#endif
	default:
		DBG_ERR("unknow core_id = %d\r\n", g_local_core_id);
	}
#endif
	return grpidx;
}

static CC_I32 cc_get_cmd_grpidx(CC_CORE_ID src_core_id, CC_CORE_ID dst_core_id)
{
	CC_I32 cmd_grpidx = 0;

	cmd_grpidx = dst_core_id - CC_CORE_CPU1;
	if (dst_core_id > src_core_id) {
    	cmd_grpidx -= 1;
	}
	cmd_grpidx *= 3;
	cmd_grpidx += ((src_core_id - CC_CORE_CPU1) * 3 * (CC_CORE_MAX - 1));
	return cmd_grpidx;
}

void nvt_cc_isr(void)
{
	CC_U32 cc_intsts;
	unsigned long      flags;

	RCW_DEF(CC_CPU1STS_REG);
	RCW_DEF(CC_CPU1INTEN_REG);

	DBG_IND("cc_isr\r\n");

	RCW_LD(CC_CPU1STS_REG);
	RCW_LD(CC_CPU1INTEN_REG);

	// Filter bits which INTEN are disabled
	RCW_VAL(CC_CPU1STS_REG) &= RCW_VAL(CC_CPU1INTEN_REG);
	cc_intsts = RCW_VAL(CC_CPU1STS_REG);
	// clear interrupt status
	RCW_ST(CC_CPU1STS_REG);

	loc_cpu(flags);
	if (g_cc_intsts & cc_intsts) {
		DBG_ERR("cc losing event 0x%x\r\n", (int)(g_cc_intsts & cc_intsts));
	}
	g_cc_intsts |= cc_intsts;
	unl_cpu(flags);

	DBG_IND("cc_intsts=0x%x\r\n", (int)cc_intsts);

}


void nvt_cc_ist(void)
{
	CC_U32 cc_intsts;
	unsigned long      flags;

	loc_cpu(flags);
	cc_intsts = g_cc_intsts;
	g_cc_intsts = 0;
	unl_cpu(flags);

	DBG_IND("\r\n");
	if (cb_handler) {
		cb_handler(cc_intsts);
	}

#if defined(CONFIG_NVT_IVOT_PLAT_NS02301)
	if (cc_intsts & CC_EVT_FROM_CPU1) {
		cc_ack_core_cmd(CC_CORE_CPU1);
	}
	if (cc_intsts & CC_EVT_FROM_CPU2) {
		cc_ack_core_cmd(CC_CORE_CPU2);
	}
	if (cc_intsts & CC_EVT_FROM_MCU) {
		cc_ack_core_cmd(CC_CORE_MCU);
	}
#else
	if (cc_intsts & CC_EVT_FROM_CPU1) {
		cc_ack_core_cmd(CC_CORE_CPU1);
	}
	if (cc_intsts & CC_EVT_FROM_CPU2) {
		cc_ack_core_cmd(CC_CORE_CPU2);
	}
	if (cc_intsts & CC_EVT_FROM_CPU3) {
		cc_ack_core_cmd(CC_CORE_CPU3);
	}
	if (cc_intsts & CC_EVT_FROM_CPU4) {
		cc_ack_core_cmd(CC_CORE_CPU4);
	}
	#ifdef CC_SUPPORT_7_CORES
	if (cc_intsts & CC_EVT_FROM_DSP1) {
		cc_ack_core_cmd(CC_CORE_DSP1);
	}
	if (cc_intsts & CC_EVT_FROM_DSP2) {
		cc_ack_core_cmd(CC_CORE_DSP2);
	}
	if (cc_intsts & CC_EVT_FROM_MCU) {
		cc_ack_core_cmd(CC_CORE_MCU);
	}
	#endif
#endif
}

static void cc_en_int(void)
{
	RCW_DEF(CC_CPU1INTEN_REG);
	unsigned int int_en_bits ;

	int_en_bits = 1;
	#ifdef CC_SUPPORT_7_CORES
	int_en_bits = (0x7F & (~int_en_bits));
	#else
	int_en_bits = (0x0F & (~int_en_bits));
	#endif

	RCW_LD(CC_CPU1INTEN_REG);
	RCW_OF(CC_CPU1INTEN_REG).event_from_cpux_int_en = int_en_bits;
#if ENABLE_CC_ACK_INTERRUPT_MODE
	RCW_OF(CC_CPU1INTEN_REG).ack_from_cpux_int_en = int_en_bits;
#endif
	RCW_ST(CC_CPU1INTEN_REG);
}

static void cc_dis_int(void)
{
	RCW_DEF(CC_CPU1INTEN_REG);

	RCW_LD(CC_CPU1INTEN_REG);
	RCW_OF(CC_CPU1INTEN_REG).event_from_cpux_int_en = 0;
#if ENABLE_CC_ACK_INTERRUPT_MODE
	RCW_OF(CC_CPU1INTEN_REG).ack_from_cpux_int_en = 0;
#endif
	RCW_ST(CC_CPU1INTEN_REG);
}

void cc_register_cb_handler(CC_HANDLER hdl)
{
	cb_handler = hdl;
}

void nvt_cc_config(CC_DEVICE  *cc_device)
{
	#ifdef __KERNEL__
	_REGIOBASE = cc_device->reg_base;
	#else
	_REGIOBASE = (uintptr_t)cc_device->reg_base;
	#endif

	DBG_IND("cc reg_base = 0x%lx\r\n", (CC_ULONG)cc_device->reg_base);
}

CC_ER nvt_cc_open(void)
{
	DBG_IND("cc_open");
	// Enable cc interrupt
	cc_en_int();
	return CC_ER_OK;
}

CC_ER nvt_cc_close(void)
{
	DBG_IND("cc_close");
	// Disable cc interrupt
	cc_dis_int();
	return CC_ER_OK;
}

CC_ER    cc_send_core_cmd(CC_CORE_ID core_id, PCC_CMD p_cmd)
{
	unsigned long      flags;
	CC_I32  sts_grpidx = 0, cmd_grpidx = 0;
#if (ENABLE_CC_ACK_INTERRUPT_MODE == 0)
	CC_U32  timeout_cnt = 0;

	RCW_DEF(CC_CPU1STS_REG);
	RCW_DEF(CC_CPU1STS_TMP_REG);

#endif
#ifndef CC_SUPPORT_7_CORES
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG1);
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG2);
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG3);
#endif
	RCW_DEF(CC_CPU1ACT_REG);

	if (g_local_core_id == core_id) {
		DBG_ERR("local_core_id %d, send to core_id = %d\r\n", g_local_core_id, core_id);
		return CC_ER_PARM;
	}
	if (core_id < CC_CORE_CPU1 || core_id > CC_CORE_MAX) {
		DBG_ERR("send to core_id = %d\r\n", core_id);
		return CC_ER_PARM;
	}
	sts_grpidx = cc_get_int_grpidx();
	cmd_grpidx = cc_get_cmd_grpidx(g_local_core_id, core_id);
	DBG_IND("cmd_grpidx = %d\r\n", cmd_grpidx);
	// wait for status bit ready
#if (ENABLE_CC_ACK_INTERRUPT_MODE == 0)
	RCW_VAL(CC_CPU1STS_TMP_REG) = 0;
	RCW_OF(CC_CPU1STS_TMP_REG).ack_from_cpux = (1 << (core_id -1));
	do {
		RCW_LD2(CC_CPU1STS_REG, sts_grpidx);
		if (timeout_cnt++ == CC_SEND_TIMEOUT_CNT) {
			DBG_ERR("cc_send_core3_cmd timeout\r\n");
			//SEM_SIGNAL(ccSemID);
			return CC_ER_TMOUT;
		}
	} while (!(RCW_OF(CC_CPU1STS_REG).ack_from_cpux & RCW_VAL(CC_CPU1STS_TMP_REG)));
#endif

	// Enter critical section
	loc_cpu(flags);

#if (ENABLE_CC_ACK_INTERRUPT_MODE == 0)
	// clear status bit
	RCW_VAL(CC_CPU1STS_REG) = 0;
	RCW_OF(CC_CPU1STS_REG).ack_from_dsp1 = 1;
	RCW_ST2(CC_CPU1STS_REG, sts_grpidx);
#endif

	DBG_IND("cmd_buf[0]=0x%x,cmd_buf[1]=0x%x,cmd_buf[2]=0x%x\r\n", (int)p_cmd->cmd_buf[0], (int)p_cmd->cmd_buf[1], (int)p_cmd->cmd_buf[2]);


	#ifdef CC_SUPPORT_7_CORES
	{
		CC_U32 *pCmd = (CC_U32 *)g_cc_cmdbuf_addr;

		if (!g_cc_cmdbuf_addr) {
			DBG_ERR("cmdbuf not init\r\n");
			return CC_ER_PARM;
		}
		pCmd +=cmd_grpidx;
		DBG_IND("pCmd=0x%lx\r\n", (CC_ULONG)pCmd);
		pCmd[0] = p_cmd->cmd_buf[0];
		pCmd[1] = p_cmd->cmd_buf[1];
		pCmd[2] = p_cmd->cmd_buf[2];
	}
	#else
	// set register data
	RCW_VAL(CC_CPU1_CPU2_CMDBUF_REG1) = p_cmd->cmd_buf[0];
	RCW_ST2(CC_CPU1_CPU2_CMDBUF_REG1, cmd_grpidx);

	RCW_VAL(CC_CPU1_CPU2_CMDBUF_REG2) = p_cmd->cmd_buf[1];
	RCW_ST2(CC_CPU1_CPU2_CMDBUF_REG2, cmd_grpidx);

	RCW_VAL(CC_CPU1_CPU2_CMDBUF_REG3) = p_cmd->cmd_buf[2];
	RCW_ST2(CC_CPU1_CPU2_CMDBUF_REG3, cmd_grpidx);
	#endif


	// trigger command
	RCW_VAL(CC_CPU1ACT_REG) = 0;
	RCW_OF(CC_CPU1ACT_REG).trigger_cpux = (1 << (core_id -1));
	RCW_ST2(CC_CPU1ACT_REG, sts_grpidx);

	// Leave critical section
	unl_cpu(flags);

	return CC_ER_OK;
}

void cc_get_core_cmd(CC_CORE_ID core_id, PCC_CMD p_cmd)
{
	CC_I32  cmd_grpidx = 0;
#ifndef CC_SUPPORT_7_CORES
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG1);
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG2);
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG3);
#endif

	DBG_IND("cc_get_core_cmd\r\n");

	if (g_local_core_id == core_id) {
		DBG_ERR("local_core_id %d, get cmd from core_id = %d\r\n", g_local_core_id, core_id);
		return;
	}
	if (core_id < CC_CORE_CPU1 || core_id > CC_CORE_MAX) {
		DBG_ERR("core_id = %d\r\n", core_id);
		return;
	}
	cmd_grpidx = cc_get_cmd_grpidx(core_id, g_local_core_id);
	#ifdef CC_SUPPORT_7_CORES
	{
		CC_U32 *pCmd = (CC_U32 *)g_cc_cmdbuf_addr;

		if (!g_cc_cmdbuf_addr) {
			DBG_ERR("cmdbuf not init\r\n");
			return;
		}
		pCmd +=cmd_grpidx;
		DBG_IND("pCmd=0x%lx\r\n", (CC_ULONG)pCmd);
		p_cmd->cmd_buf[0] = pCmd[0];
		p_cmd->cmd_buf[1] = pCmd[1];
		p_cmd->cmd_buf[2] = pCmd[2];
	}
	#else
	p_cmd->cmd_buf[0] = RCW_LD2(CC_CPU1_CPU2_CMDBUF_REG1, cmd_grpidx);
	p_cmd->cmd_buf[1] = RCW_LD2(CC_CPU1_CPU2_CMDBUF_REG2, cmd_grpidx);
	p_cmd->cmd_buf[2] = RCW_LD2(CC_CPU1_CPU2_CMDBUF_REG3, cmd_grpidx);
	#endif
}

static void cc_ack_core_cmd(CC_CORE_ID core_id)
{
	unsigned long      flags;
	CC_I32             act_grpidx = 0;
	RCW_DEF(CC_CPU1ACT_REG);

	DBG_IND("cc_ack_core_cmd\r\n");

	if (g_local_core_id == core_id) {
		DBG_ERR("local_core_id %d, send to core_id = %d\r\n", g_local_core_id, core_id);
		return;
	}
	if (core_id < CC_CORE_CPU1 || core_id > CC_CORE_MAX) {
		DBG_ERR("send to core_id = %d\r\n", core_id);
		return;
	}
	act_grpidx = cc_get_int_grpidx();
	// Enter critical section
	loc_cpu(flags);

	RCW_VAL(CC_CPU1ACT_REG) = 0;
	RCW_OF(CC_CPU1ACT_REG).ack_cpux = (1 << (core_id -1));
	RCW_ST2(CC_CPU1ACT_REG, act_grpidx);

	// Leave critical section
	unl_cpu(flags);
}

#if CC_UNIT_TEST

void     cc_corex_ack_cmd(CC_CORE_ID core_id)
{
	#if 1
	unsigned long      flags;
	CC_I32             act_grpidx = 0, sts_grpidx = 0;
	RCW_DEF(CC_CPU1ACT_REG);
	RCW_DEF(CC_CPU1STS_REG);

	if (core_id < CC_CORE_CPU1 || core_id > CC_CORE_MAX) {
		DBG_ERR("core_id = %d\r\n", core_id);
		return;
	}
	DBG_IND("cc_corex_ack_cmd\r\n");

	act_grpidx = cc_get_int_grpidx();
	sts_grpidx = act_grpidx;
	// Enter critical section
	loc_cpu(flags);

	RCW_VAL(CC_CPU1ACT_REG) = 0;
	RCW_OF(CC_CPU1ACT_REG).ack_cpux = (1 << (g_local_core_id -1));
	RCW_ST2(CC_CPU1ACT_REG, act_grpidx);

	RCW_LD2(CC_CPU1STS_REG, sts_grpidx);
	// clear interrupt status
	RCW_ST2(CC_CPU1STS_REG, sts_grpidx);

	// Leave critical section
	unl_cpu(flags);
	#else
	if (core_id == CC_CORE_CA53_CORE3) {
		cc_core3_ack_core1_cmd();
	} else if (core_id == CC_CORE_CA53_CORE4) {
		cc_core4_ack_core1_cmd();
	} else {
		DBG_ERR("unknow core_id %d\r\n", core_id);
	}
	#endif
}

CC_ER    cc_corex_send_cmd(CC_CORE_ID core_id, PCC_CMD p_cmd)
{
	#if 1
	unsigned long      flags;
	CC_I32  sts_grpidx = 0, cmd_grpidx = 0;
#if (ENABLE_CC_ACK_INTERRUPT_MODE == 0)
	CC_U32  timeout_cnt = 0;

	RCW_DEF(CC_CPU1STS_REG);
	RCW_DEF(CC_CPU1STS_TMP_REG);
#endif
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG1);
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG2);
	RCW_DEF(CC_CPU1_CPU2_CMDBUF_REG3);
	RCW_DEF(CC_CPU1ACT_REG);

	if (g_local_core_id == core_id) {
		DBG_ERR("core_id %d, send to local core_id = %d\r\n", core_id, g_local_core_id);
		return CC_ER_PARM;
	}
	if (core_id < CC_CORE_CPU1 || core_id > CC_CORE_MAX) {
		DBG_ERR("send to core_id = %d\r\n", core_id);
		return CC_ER_PARM;
	}
	sts_grpidx = cc_get_int_grpidx();
	cmd_grpidx = cc_get_cmd_grpidx(core_id, g_local_core_id);
	DBG_ERR("cmd_grpidx = %d\r\n", cmd_grpidx);
	// wait for status bit ready
#if (ENABLE_CC_ACK_INTERRUPT_MODE == 0)
	RCW_VAL(CC_CPU1STS_TMP_REG) = 0;
	RCW_OF(CC_CPU1STS_TMP_REG).ack_from_cpux = (1 << (core_id -1));
	do {
		RCW_LD2(CC_CPU1STS_REG, sts_grpidx);
		if (timeout_cnt++ == CC_SEND_TIMEOUT_CNT) {
			DBG_ERR("cc_send_core3_cmd timeout\r\n");
			//SEM_SIGNAL(ccSemID);
			return CC_ER_TMOUT;
		}
	} while (!(RCW_OF(CC_CPU1STS_REG).ack_from_cpux & RCW_VAL(CC_CPU1STS_TMP_REG)));
#endif

	// Enter critical section
	loc_cpu(flags);

#if (ENABLE_CC_ACK_INTERRUPT_MODE == 0)
	// clear status bit
	RCW_VAL(CC_CPU1STS_REG) = 0;
	RCW_OF(CC_CPU1STS_REG).ack_from_cpux = 1;
	RCW_ST2(CC_CPU1STS_REG, sts_grpidx);
#endif

	DBG_IND("cmd_buf[0]=0x%x,cmd_buf[1]=0x%x,cmd_buf[2]=0x%x\r\n", (int)p_cmd->cmd_buf[0], (int)p_cmd->cmd_buf[1], (int)p_cmd->cmd_buf[2]);


	// set register data
	RCW_VAL(CC_CPU1_CPU2_CMDBUF_REG1) = p_cmd->cmd_buf[0];
	RCW_ST2(CC_CPU1_CPU2_CMDBUF_REG1, cmd_grpidx);

	RCW_VAL(CC_CPU1_CPU2_CMDBUF_REG2) = p_cmd->cmd_buf[1];
	RCW_ST2(CC_CPU1_CPU2_CMDBUF_REG2, cmd_grpidx);

	RCW_VAL(CC_CPU1_CPU2_CMDBUF_REG3) = p_cmd->cmd_buf[2];
	RCW_ST2(CC_CPU1_CPU2_CMDBUF_REG3, cmd_grpidx);


	// trigger command
	RCW_VAL(CC_CPU1ACT_REG) = 0;
	RCW_OF(CC_CPU1ACT_REG).trigger_cpux = (1 << (core_id -1));
	RCW_ST2(CC_CPU1ACT_REG, sts_grpidx);

	// Leave critical section
	unl_cpu(flags);
	return CC_ER_OK;

	#else
	if (core_id == CC_CORE_CA53_CORE3) {
		return cc_core3_send_core1_cmd(p_cmd);
	} else if (core_id == CC_CORE_CA53_CORE4) {
		return cc_core4_send_core1_cmd(p_cmd);
	}
	DBG_ERR("unknow core_id %d\r\n", core_id);
	return CC_ER_PARM;
	#endif
}

#endif

static CC_I32 cc_get_hw_res_grant(CC_CORE_ID core_id)
{
	CC_I32 result;

	RCW_DEF(CC_CPU1GRANT_REG);

	if (core_id == CC_CORE_CPU1) {
		result = RCW_LD(CC_CPU1GRANT_REG);
	} else {
		DBG_ERR("unknow core_id %d\r\n", core_id);
		result = E_SYS;
	}
	return result;
}

CC_ER cc_hwlock_resource(CC_CORE_ID core_id, CC_RES_ID res_id)
{
	unsigned long      flags;
	RCW_DEF(CC_CPU1REQ_REG);
	CC_U32 timeout_cnt = 0;

	if (res_id >= CC_RES_ID_NUM) {
		DBG_ERR("Unknow res_id %d\r\n", res_id);
		return CC_ER_PARM;
	}
	if (core_id == CC_CORE_CPU1) {
		loc_cpu(flags);
		RCW_OF(CC_CPU1REQ_REG).request = (1 << res_id);
		RCW_ST(CC_CPU1REQ_REG);
		unl_cpu(flags);

	} else {
		DBG_ERR("Unknow coreID %d\r\n", core_id);
		return CC_ER_PARM;
	}
	// Polling until relative grant bit assert to 1
	while (!(cc_get_hw_res_grant(core_id) & (1 << res_id))) {
		if (timeout_cnt++ == CC_GRANT_TIMEOUT_CNT) {
			DBG_ERR("timeout, res_id=%d\r\n", res_id);
			return E_TMOUT;
		}
	}
	return CC_ER_OK;
}



CC_ER cc_hwunlock_resource(CC_CORE_ID core_id, CC_RES_ID res_id)
{
	unsigned long      flags;
	RCW_DEF(CC_CPU1REL_REG);

	if (res_id >= CC_RES_ID_NUM) {
		DBG_ERR("Unknow res_id %d\r\n", res_id);
		return CC_ER_PARM;
	}
	if (core_id == CC_CORE_CPU1) {
		loc_cpu(flags);
		RCW_OF(CC_CPU1REL_REG).release = (1 << res_id);
		RCW_ST(CC_CPU1REL_REG);
		unl_cpu(flags);
	} else {
		DBG_ERR("Unknow coreID %d\r\n", core_id);
		return E_SYS;
	}
	return CC_ER_OK;
}

void cc_set_reg(CC_U32 reg_offset, CC_U32 value)
{
	DBG_MSG("_REGIOBASE = 0x%lx, reg_offset = 0x%x\r\n",
			(CC_ULONG)_REGIOBASE, reg_offset);
	OUTW(_REGIOBASE+reg_offset, value);
}

void cc_set_cmdbuf_addr(CC_ULONG value)
{
	DBG_MSG("cmdbuf_addr = 0x%lx\r\n", value);
	g_cc_cmdbuf_addr = value;
}

/*
EXPORT_SYMBOL(nvt_cc_config);
EXPORT_SYMBOL(nvt_cc_isr);
EXPORT_SYMBOL(nvt_cc_ist);
EXPORT_SYMBOL(nvt_cc_open);
EXPORT_SYMBOL(nvt_cc_close);
*/
EXPORT_SYMBOL(cc_register_cb_handler);
EXPORT_SYMBOL(cc_send_core_cmd);
EXPORT_SYMBOL(cc_get_core_cmd);
EXPORT_SYMBOL(cc_corex_ack_cmd);
EXPORT_SYMBOL(cc_corex_send_cmd);
EXPORT_SYMBOL(cc_hwlock_resource);
EXPORT_SYMBOL(cc_hwunlock_resource);
EXPORT_SYMBOL(cc_set_reg);
EXPORT_SYMBOL(cc_set_cmdbuf_addr);



