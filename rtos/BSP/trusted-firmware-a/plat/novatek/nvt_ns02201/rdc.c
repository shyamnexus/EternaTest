#include "ddr_para.h"


//#define RDC_SYS_CHECK_TRAINING_BYPASS

static struct rdc_list _rdc[DDR_NUM] = {0};
static struct rdc_list *_trained_rdc[DDR_NUM];

#define _MASK_REG(name) \
	{ \
		REG_ADDR(name),	\
		REG_MASK(name)	\
	} \

#define _DLLPHASE_REGS(byte, name) \
	_MASK_REG(R_##byte##_BYPASS_DLL_PHASE_##name), \
	_MASK_REG(byte##_RDC_##name##_CNTL_DLL_PHASE_6_0_)

#define _DLLPHASE_WDQ_REGS(byte, name) \
	_MASK_REG(R_BYTE##byte##_BYPASS_DLL_PHASE_##name), \
	_MASK_REG(WEYE_DELAY_BYTE_0##byte##_7_0_)

#define _RDC_REGS(name) {			\
		REG_ADDR(name##_CNTL_REG_7_0_),	\
		REG_ADDR(name##_CNTL_REG_8_),	\
		_MASK_REG(name##_DLY_OUT_EN),	\
	}
#define _NULL_MASK_REG() { 0, 0 }

typedef struct mask_reg {
	unsigned int addr;
	unsigned int  mask;
} mask_reg_t;
static struct _rdc_reg {
	unsigned int has_dllphase_module;
	mask_reg_t bypass_dllphase;
	mask_reg_t dllphase;
	struct {
		unsigned int reg_low;
		unsigned int reg_high;
		mask_reg_t dly_out_en;
	};
} _rdc_reg[RDC_NUM] = {
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA0_RDC_CK) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA0_RDC_CKE_CS) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA0_RDC_CA) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA0_RDC_CKE_CS_DUAL) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA1_RDC_CK) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA1_RDC_CKE_CS) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA1_RDC_CA) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA1_RDC_CKE_CS_DUAL) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA2_RDC_CA) },
	{ 0, _NULL_MASK_REG(), _NULL_MASK_REG(), _RDC_REGS(CA3_RDC_CA) },
	{ 1, _DLLPHASE_REGS(BYTE0, RDQS_M), _RDC_REGS(BYTE0_RDC_RDQS_M) },
	{ 1, _DLLPHASE_REGS(BYTE0, RDQS_S), _RDC_REGS(BYTE0_RDC_RDQS_S) },
	{ 1, _DLLPHASE_REGS(BYTE0, DQS),    _RDC_REGS(BYTE0_RDC_DQS) },
	{ 1, _DLLPHASE_WDQ_REGS(0, DQ),     _RDC_REGS(BYTE0_RDC_DQ) },
	{ 1, _DLLPHASE_REGS(BYTE1, RDQS_M), _RDC_REGS(BYTE1_RDC_RDQS_M) },
	{ 1, _DLLPHASE_REGS(BYTE1, RDQS_S), _RDC_REGS(BYTE1_RDC_RDQS_S) },
	{ 1, _DLLPHASE_REGS(BYTE1, DQS),    _RDC_REGS(BYTE1_RDC_DQS) },
	{ 1, _DLLPHASE_WDQ_REGS(1, DQ),     _RDC_REGS(BYTE1_RDC_DQ) },
	{ 1, _DLLPHASE_REGS(BYTE2, RDQS_M), _RDC_REGS(BYTE2_RDC_RDQS_M) },
	{ 1, _DLLPHASE_REGS(BYTE2, RDQS_S), _RDC_REGS(BYTE2_RDC_RDQS_S) },
	{ 1, _DLLPHASE_REGS(BYTE2, DQS),    _RDC_REGS(BYTE2_RDC_DQS) },
	{ 1, _DLLPHASE_WDQ_REGS(2, DQ),     _RDC_REGS(BYTE2_RDC_DQ) },
	{ 1, _DLLPHASE_REGS(BYTE3, RDQS_M), _RDC_REGS(BYTE3_RDC_RDQS_M) },
	{ 1, _DLLPHASE_REGS(BYTE3, RDQS_S), _RDC_REGS(BYTE3_RDC_RDQS_S) },
	{ 1, _DLLPHASE_REGS(BYTE3, DQS),    _RDC_REGS(BYTE3_RDC_DQS) },
	{ 1, _DLLPHASE_WDQ_REGS(3, DQ),     _RDC_REGS(BYTE3_RDC_DQ) },
	{ 1, _MASK_REG(R_BYPASS_DLL_PHASE_ZQK),
	     _MASK_REG(ZQK_RDC_CNTL_DLL_PHASE_6_0_), _RDC_REGS(ZQK_RDC) },
};

#define _TRAINING_BYPASS_REGS(name)					\
		_MASK_REG(name##_TRAINING_BYPASS_REG),			\
		_MASK_REG(name##_DLY_OUT_EN_TRAINING_BYPASS_REG)
#define _TRAINING_BYPASS_DQ_REGS(byte, name)				\
		_MASK_REG(R_##byte##_##name##_TRAINING_BYPASS),		\
		_MASK_REG(R_##byte##_DLY_OUT_EN_##name##_TRAINING_BYPASS)
struct _training_bypass_reg {
	/* training_bypass */
	mask_reg_t reg1;
	/* dly_out_en_training_bypass */
	mask_reg_t reg2;
} _training_bypass[RDC_NUM] = {
	{ _TRAINING_BYPASS_REGS(CK0) },
	{ _NULL_MASK_REG(), _NULL_MASK_REG() }, /* CA0_RDC_CKE_CS */
	{ _TRAINING_BYPASS_REGS(CA0) },
	{ _NULL_MASK_REG(), _NULL_MASK_REG() }, /* CA0_RDC_CKE_CS_DUAL */
	{ _TRAINING_BYPASS_REGS(CK1) },
	{ _NULL_MASK_REG(), _NULL_MASK_REG() }, /* CA1_RDC_CKE_CS */
	{ _TRAINING_BYPASS_REGS(CA1) },
	{ _NULL_MASK_REG(), _NULL_MASK_REG() }, /* CA1_RDC_CKE_CS_DUAL */
	{ _TRAINING_BYPASS_REGS(CA2) },
	{ _TRAINING_BYPASS_REGS(CA3) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE0, RDQS_M) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE0, RDQS_S) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE0, DQS) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE0, DQ) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE1, RDQS_M) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE1, RDQS_S) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE1, DQS) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE1, DQ) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE2, RDQS_M) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE2, RDQS_S) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE2, DQS) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE2, DQ) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE3, RDQS_M) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE3, RDQS_S) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE3, DQS) },
	{ _TRAINING_BYPASS_DQ_REGS(BYTE3, DQ) },
	{ _MASK_REG(TRAINING_BYPASS_REG),
	  _MASK_REG(DLY_OUT_EN_TRAINING_BYPASS_REG) }, /* ZQK_RDC */
};

/*void dump_sw_dllphase_matrix(void)
{
	int i;
	printf("_rdc_reg[%d] = {\n", RDC_NUM);
	for (i = 0; i < RDC_NUM; i++) {
		printf("\t");
		printf("{%d, ", _rdc_reg[i].has_dllphase_module);
			printf("{0x%03x, 0x%02x}, ",
				_rdc_reg[i].bypass_dllphase.addr,
				_rdc_reg[i].bypass_dllphase.mask);
			printf("{0x%03x, 0x%02x}, ",
				_rdc_reg[i].dllphase.addr,
				_rdc_reg[i].dllphase.mask);
			printf("{0x%03x, 0x%03x, ",
				_rdc_reg[i].reg_low,
				_rdc_reg[i].reg_high);
				printf("{0x%03x, 0x%02x}",
					_rdc_reg[i].dly_out_en.addr,
					_rdc_reg[i].dly_out_en.mask);
			printf("}");
		printf("},\n");
	}
	printf("};\n");
	printf("_training_bypass[%d] = {\n", RDC_NUM);
	for (i = 0; i < RDC_NUM; i++) {
		printf("\t");
		printf("{");
			printf("{0x%03x, 0x%02x}, ",
				_training_bypass[i].reg1.addr,
				_training_bypass[i].reg1.mask);
			printf("{0x%03x, 0x%02x}",
				_training_bypass[i].reg2.addr,
				_training_bypass[i].reg2.mask);
		printf("},\n");
	}
	printf("};\n");
}*/
#if 0
void rdc_init(int phy_id, unsigned int ddr_pll_khz)
{
	int i;
	unsigned int addr;
	int ddr_speed, default_ui2rdc;

	/* default_ui2rdc is related to ddr_speed */
	/*
	 * (1 / ddr_speed(MHz) * 10^6) / (2.25 * 10^-12)
	 * = 10^6 / (2.25 * ddr_speed)
	 * = (8 * 10^6) / (18 * ddr_speed)
	 * rounding
	 * => (8 * 10^6 + 9 * ddr_speed) / (18 * ddr_speed)
	 */
	ddr_speed = (ddr_pll_khz*8)/1000;
	default_ui2rdc = (8000000 + 9 * ddr_speed) / (18 * ddr_speed);

	for (i = 0, addr = 0x560; i < RDC_NUM; i++, addr += 2) {
		_rdc[phy_id].raw[i] = default_ui2rdc;
	}
	_trained_rdc[phy_id] = &(_rdc[phy_id]);
}
#endif

void rdc_store(int phy_id)
{
	int i;
	unsigned int addr;

	for (i = 0, addr = 0x560; i < RDC_NUM; i++, addr += 2) {
		_rdc[phy_id].raw[i] = rphy(phy_id, addr) |
			(phy_mask_get(phy_id, addr + 1, 0x80) << 1);

        //printf("%d\r\n", _rdc[phy_id].raw[i]);
	}

	_trained_rdc[phy_id] = &(_rdc[phy_id]);
}

static int is_not_valid_rdc_id(int phy_id, rdc_t rdc_id)
{
	if (rdc_id >= RDC_NUM || rdc_id < 0) {
		return -1;/*ddr_err(ERROR_RDC_OUT_RANGE,
			"rdc id(%d) >= RDC_NUM(%d)\n", rdc_id, RDC_NUM);*/
	}

	if (_trained_rdc[phy_id] == NULL) {
		rdc_store(phy_id);
		/*ddr_warn(WARN_RDC_NOT_INIT,
			"_trained_rdc[%d] is not init\n", phy_id);*/

		if (_trained_rdc[phy_id] == NULL) {
			return -1;/*ddr_err(ERROR_RDC_NOT_INIT,
				"_trained_rdc[%d] is not init\n", phy_id);*/
		}
	}

	return 0;
}

int get_trained_ui2rdc(int phy_id, rdc_t rdc_id, unsigned int *return_ui2rdc)
{
	int ret;
	*return_ui2rdc = 0;
	if ((ret = is_not_valid_rdc_id(phy_id, rdc_id)))
		return ret;

    //printf("%d\r\n", _trained_rdc[phy_id]->raw[rdc_id]);
	*return_ui2rdc = _trained_rdc[phy_id]->raw[rdc_id];
	return 0;
}

void set_training_bypass(int phy_id, rdc_t phase_id, int is_bypass)
{
	if (is_not_valid_rdc_id(phy_id, phase_id))
		return;
	/* return if no training bypass reg */
	if (_training_bypass[phase_id].reg1.addr == 0x00) {
		/*ddr_warn(WARN_RDC_NO_TRAINING_BYPASS,
			"phase_id[%d] has no training bypass\n", phase_id);*/
		return;
	}
	if (is_bypass) {
		/* training_bypass */
		phy_mask_set_1(phy_id, _training_bypass[phase_id].reg1.addr,
				_training_bypass[phase_id].reg1.mask);
		/* dly_out_en_training_bypass */
		phy_mask_set_1(phy_id, _training_bypass[phase_id].reg2.addr,
				_training_bypass[phase_id].reg2.mask);
#if 0//(CHIP_ID == TC96690B)
		phy_reg_set_1(phy_id, TRAINING_BYPASS_REG);
#endif
	}
	else {
		/* training_bypass */
		phy_mask_set_0(phy_id, _training_bypass[phase_id].reg1.addr,
				_training_bypass[phase_id].reg1.mask);
		/* dly_out_en_training_bypass */
		phy_mask_set_0(phy_id, _training_bypass[phase_id].reg2.addr,
				_training_bypass[phase_id].reg2.mask);
#if 0//(CHIP_ID == TC96690B)
		phy_reg_set_0(phy_id, TRAINING_BYPASS_REG);
#endif
	}
}

int phy_dll_phase_get(int phy_id, rdc_t phase_id)
{
	int ret;
	unsigned int ui2rdc = 0;
	unsigned int rdc;
	int dll_phase_64x;

	//DDR_FLOW(phy_dll_phase_get, start);
	if ((ret = is_not_valid_rdc_id(phy_id, phase_id)))
		return ret;
#if 1//(CHIP_ID != TC96690B) /* bypass_dllphase in Rev.B has no function */
	if (_rdc_reg[phase_id].has_dllphase_module &&
		(0 == phy_mask_get(phy_id,
			_rdc_reg[phase_id].bypass_dllphase.addr,
			_rdc_reg[phase_id].bypass_dllphase.mask))) {
		/* Get by dllphase module */
		//DDR_FLOW_CMNT("%s: get by dllphase\n", __func__);
		dll_phase_64x = phy_mask_get(phy_id,
			_rdc_reg[phase_id].dllphase.addr,
			_rdc_reg[phase_id].dllphase.mask);
		//DDR_FLOW_CMNT("%s: dllphase=%d\n", __func__, dll_phase_64x);
	}
	else
#endif
	{
		/* Get by rdc */
		//DDR_FLOW_CMNT("%s: get by rdc\n", __func__);
		if ((ret = get_trained_ui2rdc(phy_id, phase_id, &ui2rdc)))
			return ret;
		if (ui2rdc == 0) {
			/*uart_put_char('T');
			uart_put_char('R');
			uart_put_char('D');
			uart_put_char('C');
			uart_put_char('0');
			uart_put_char('E');
			uart_put_char('\r');
			uart_put_char('\n');*/
			return -1;/*ddr_err(ERROR_RDC_TRAINED_UI2RDC_IS_ZERO,
			"phase_id[%d] ui2rdc=0\n", phase_id);*/
		}

		rdc = (phy_mask_get(phy_id,
			_rdc_reg[phase_id].reg_high, 0x80) << 1) |
			rphy(phy_id, _rdc_reg[phase_id].reg_low);
		dll_phase_64x = convert_rdc2dllphase_round(rdc, ui2rdc);
		/*DDR_FLOW_CMNT("%s: ui2rdc=%d dllphase=%d rdc=%d\n",
			__func__, ui2rdc, dll_phase_64x, rdc);*/
	}
	return dll_phase_64x;
}

int _phy_dll_phase_set(int phy_id, rdc_t phase_id, unsigned int dll_phase_64x)
{
	int ret;
	unsigned int ui2rdc = 0;
	unsigned int rdc = 0;
	unsigned int tmp = 0;

#if defined(RDC_SYS_CHECK_TRAINING_BYPASS)
	if (phy_mask_get(phy_id, _training_bypass[phase_id].reg1,
				_training_bypass[phase_id].mask1) ||
	    phy_mask_get(phy_id, _training_bypass[phase_id].reg2,
				_training_bypass[phase_id].mask2))
		return ddr_err(ERROR_NOT_TRAINING_BYPASS,
			"phase_id[%d] is not training_bypass\n", phase_id);
#endif

	//DDR_FLOW(phy_dll_phase_set, start);
	if ((ret = is_not_valid_rdc_id(phy_id, phase_id)))
		return ret;

	if (is_cmd_phase_rdc(phase_id)) {
		tmp = rphy(phy_id, REG_ADDR(ASYNC_FIFO_SHIFT_IN));
		/* set async_fifo_shift_in 0 */
		wphy(phy_id, REG_ADDR(ASYNC_FIFO_SHIFT_IN),
			clrbits8(tmp, REG_MASK(ASYNC_FIFO_SHIFT_IN)));
		/* delay # 1us */
		udelay(1);
	}
	

	/* Set dly_out_en 0 */
	
	phy_mask_set_0(phy_id, _rdc_reg[phase_id].dly_out_en.addr,
				_rdc_reg[phase_id].dly_out_en.mask);

#if 1//(CHIP_ID != TC96690B) /* bypass_dllphase in Rev.B has no function */
	if (_rdc_reg[phase_id].has_dllphase_module &&
		(0 == phy_mask_get(phy_id,
			_rdc_reg[phase_id].bypass_dllphase.addr,
			_rdc_reg[phase_id].bypass_dllphase.mask))) {
		/* Set by dllphase module */
		//DDR_FLOW_CMNT("%s: set by dllphase\n", __func__);
		phy_mask_set(phy_id, _rdc_reg[phase_id].dllphase.addr,
			_rdc_reg[phase_id].dllphase.mask, dll_phase_64x);
		//DDR_FLOW_CMNT("%s: dllphase=%d\n", __func__, dll_phase_64x);
        //printf("dly out en %x, mask %x\n",_rdc_reg[phase_id].dly_out_en.addr, _rdc_reg[phase_id].dly_out_en.mask);
        //printf("dllphase reg %x\n",_rdc_reg[phase_id].dllphase.addr);
	}
	else
#endif
	{
		/* Set by rdc */
		//DDR_FLOW_CMNT("%s: set by rdc\n", __func__);
		if ((ret = get_trained_ui2rdc(phy_id, phase_id, &ui2rdc)))
			return ret;
		if (ui2rdc == 0) {
			/*uart_put_char('T');
			uart_put_char('R');
			uart_put_char('D');
			uart_put_char('C');
			uart_put_char('0');
			uart_put_char('E');
			uart_put_char('\r');
			uart_put_char('\n');*/
			return -1;/*ddr_err(ERROR_RDC_TRAINED_UI2RDC_IS_ZERO,
			"phase_id[%d] ui2rdc=0\n", phase_id);*/
		}

		rdc = convert_dllphase2rdc_round(dll_phase_64x, ui2rdc);
		/*DDR_FLOW_CMNT("%s: ui2rdc=%d dllphase=%d rdc=%d\n",
			__func__, ui2rdc, dll_phase_64x, rdc);*/
		wphy(phy_id, _rdc_reg[phase_id].reg_low, (rdc & 0xff));
		phy_mask_set(phy_id, _rdc_reg[phase_id].reg_high,
					0x80, ((rdc & 0x100) >> 1));
        //printf("rdc reg low %x\n",_rdc_reg[phase_id].reg_low);
        //printf("rdc reg high %x\n",_rdc_reg[phase_id].reg_high);
	}

	/* Set dly_out_en 1 */
	phy_mask_set_1(phy_id, _rdc_reg[phase_id].dly_out_en.addr,
				_rdc_reg[phase_id].dly_out_en.mask);

	if (is_cmd_phase_rdc(phase_id)) {
		/* set async_fifo_shift_in 1 */
		wphy(phy_id, REG_ADDR(ASYNC_FIFO_SHIFT_IN),
			setbits8(tmp, REG_MASK(ASYNC_FIFO_SHIFT_IN)));
		/* delay # 1us */
		udelay(1);
	}

	return 0;
}

int phy_dll_phase_set(int phy_id, rdc_t phase_id, unsigned int dll_phase_64x)
{
	if (dll_phase_64x > 96) {
		/*uart_put_char('D');
		uart_put_char('L');
		uart_put_char('L');
		uart_put_char('2');
		uart_put_char('L');
		uart_put_char('E');
		uart_put_char('\r');
		uart_put_char('\n');*/
		return -1;/*ddr_err(ERROR_DLLPHASE_TOO_LARGE,
			"dllphase %d > 64\n", dll_phase_64x);*/
	}
	return _phy_dll_phase_set(phy_id, phase_id, dll_phase_64x);
}

