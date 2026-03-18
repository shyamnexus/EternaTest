/*
 * ECDSA API.
 *
 * Support for Novatek NVT ECDSA Hardware acceleration.
 *
 * Copyright (c) 2021 Novatek Inc.
 *
 */

#ifdef CONFIG_NVT_ECDSA
#include <common.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/nvt_ecdsa.h>

#include <linux/delay.h>

#define SUPPORT_SIZE                	NVT_ECDSA_KEY_WIDTH_MAX/8

/*************************************************************************************
 *  IO Read/Write Definition
 *************************************************************************************/
#define IO_REG_RD(_base, _ofs)          readl(_base+_ofs)
#define IO_REG_WR(_base, _ofs, _v)      writel(_v, (_base+_ofs))

/*************************************************************************************
 *  Global Definition
 *************************************************************************************/
#ifdef CONFIG_NVT_FPGA_EMULATION
#define NVT_ECDSA_TIMEOUT_USEC            (1000000*60)
#else
#define NVT_ECDSA_TIMEOUT_USEC            (1000000*10)
#endif

/*************************************************************************************
 *  Debug Message Print Definition
 *************************************************************************************/
#ifdef NVT_ECDSA_DEBUG
#define ecdsa_dbg(...)                    printf("[ECDSA_DBG]: " __VA_ARGS__)
#else
#define ecdsa_dbg(...)
#endif

#define ecdsa_err(...)                    printf("[ECDSA_ERR]: " __VA_ARGS__)
#define ecdsa_inf(...)                    printf("[ECDSA_INF]: " __VA_ARGS__)

/*************************************************************************************
 *  Register read/write Definition
 *************************************************************************************/
#define ecdsa_read(_oft)                 IO_REG_RD(IOADDR_ECDSA_REG_BASE, _oft)
#define ecdsa_write(_oft, _v)            IO_REG_WR(IOADDR_ECDSA_REG_BASE, _oft, _v)

/*************************************************************************************
 *  Register Definition
 *************************************************************************************/
#define NVT_ECDSA_TRG_REG_REG  		0x00   		///< ECDSA Trigger Register
#define NVT_ECDSA_STS_REG      		0x04   		///< ECDSA Status Register
#define NVT_ECDSA_INTEN_REG     	0x08   		///< ECDSA Interrupt Enable Register
#define NVT_ECDSA_SIGN_MSG_REG 		0x40   		///< ECDSA Signature MSG (0x6C-0x40)
#define NVT_ECDSA_PRIV_KEY_REG 		0x80   		///< ECDSA Private Key (0xAC-0x80)
#define NVT_ECDSA_VRIFY_S_REG  		0xC0   		///< ECDSA Verify S Value (0xEC-0xC0)
#define NVT_ECDSA_VPKEY_X_REG  		0x100  		///< ECDSA Verify Public Key X (0x12C-0x100)
#define NVT_ECDSA_VPKEY_Y_REG  		0x140  		///< ECDSA Verify Public Key Y (0x16C-0x140)
#define NVT_ECDSA_BASE_X_REG   		0x180  		///< ECC Base X (0x1AC-0x180)
#define NVT_ECDSA_BASE_Y_REG   		0x1C0  		///< ECC Base Y (0x1EC-0x1C0)
#define NVT_ECDSA_SIGN_R_REG   		0x200  		///< ECDSA Signature R Value (0x22C-0x200)
#define NVT_ECDSA_SIGN_S_REG   		0x240  		///< ECDSA Signature S Value (0x26C-0x240)
#define NVT_ECDSA_GPKEY_X_REG   	0x280  		///< ECDSA Generated Public Key X (0x2AC-0x280)
#define NVT_ECDSA_GPKEY_Y_REG  		0x2C0  		///< ECDSA Generated Public Key Y (0x2EC-0x2C0)
#define NVT_ECDSA_A_VALUE_REG   	0x300  		///< ECDSA A_Value (0x32C-0x300)
#define NVT_ECDSA_PRIME_REG   		0x340  		///< ECDSA Prime (0x36C-0x340)
#define NVT_ECDSA_ORDER_REG   		0x380  		///< ECDSA Order (0x3AC-0x380)

/*************************************************************************************
 *  Local Definition
 *************************************************************************************/
static int nvt_ecdsa_opened = 0;

static void nvt_ecdsa_platform_init(void)
{
	uint32_t tmp;

	/* ECDSA clock select, 0:fix(240MHz) 1:fix(320MHz) */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x1C);
	tmp &= ~(0x3<<6);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x1C, tmp);

	/* ECDSA master clock enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x80);
	tmp |= (0x1<<4);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x80, tmp);

	/* ECDSA program clock enable */
	tmp = IO_REG_RD(IOADDR_TOP_REG_BASE, 0xE8);
	tmp |= (0x1<<1);
	IO_REG_WR(IOADDR_TOP_REG_BASE, 0xE8, tmp);

	/* ECDSA reset disable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0xA0);
	tmp |= (0x1<<5);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0xA0, tmp);
}

static void nvt_ecdsa_platform_exit(void)
{
	uint32_t tmp;

	/* ECDSA reset enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0xA0);
	tmp &= ~(0x1<<5);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0xA0, tmp);

	/* ECDSA master clock disable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x80);
	tmp &= ~(0x1<<4);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x80, tmp);
}

int nvt_ecdsa_open(void)
{
	if (!nvt_ecdsa_opened) {
		/* platform pmu init for ecdsa engine */
		nvt_ecdsa_platform_init();

		nvt_ecdsa_opened = 1;
	}

	return 0;
}

void nvt_ecdsa_close(void)
{
	if (nvt_ecdsa_opened) {
		nvt_ecdsa_platform_exit();
		nvt_ecdsa_opened = 0;
	}
}

int nvt_ecdsa_init(NVT_ECDSA_KEY_WIDTH_T eccSize)
{
	ECC_INFO ecc;
	int i = 0, j = 0;
	uint32_t avalue_reg = 0, prime_reg = 0, order_reg = 0,\
			basex = 0, basey = 0;
	uint32_t tmp = 0;

	switch (eccSize) {
    case NVT_ECDSA_KEY_WIDTH_384:
        ecc = NVT_SECP384R1;
		tmp = (0x0<<4) | (0x0<<7);
        ecdsa_dbg("Use NVT_SECP384R1\r\n");
        break;

    case NVT_ECDSA_KEY_WIDTH_256:
        ecc = NVT_SECP256R1;
		tmp = (0x1<<4) | (0x1<<7);	
        printf("Use NVT_SECP256R1\r\n");
        break;

    case NVT_ECDSA_KEY_WIDTH_224:
        ecc = NVT_SECP224R1;
        tmp = (0x2<<4) | (0x1<<7);	
        ecdsa_dbg("Use NVT_SECP224R1\r\n");
        break;

    case NVT_ECDSA_KEY_WIDTH_192:
        ecc = NVT_SECP192R1;
       	tmp = (0x3<<4) | (0x1<<7);	
        ecdsa_dbg("Use NVT_SECP192R1\r\n");
        break;

    default:
        ecdsa_err("Not support\r\n");
        return -1;
    }
	ecdsa_write(NVT_ECDSA_INTEN_REG,  tmp);

    for (i = SUPPORT_SIZE - 1; i >= 3; i-=4) {
        avalue_reg = (ecc.A_Value[i - 3] << 24) | (ecc.A_Value[i - 2] << 16) | (ecc.A_Value[i - 1] << 8) | ecc.A_Value[i];
        prime_reg = (ecc.prime[i - 3] << 24) | (ecc.prime[i - 2] << 16) | (ecc.prime[i - 1] << 8) | ecc.prime[i];
        order_reg = (ecc.order[i - 3] << 24) | (ecc.order[i - 2] << 16) | (ecc.order[i - 1] << 8) | ecc.order[i];
        basex = (ecc.base_x[i - 3] << 24) | (ecc.base_x[i - 2] << 16) | (ecc.base_x[i - 1] << 8) | ecc.base_x[i];
        basey = (ecc.base_y[i - 3] << 24) | (ecc.base_y[i - 2] << 16) | (ecc.base_y[i - 1] << 8) | ecc.base_y[i];

        ecdsa_write(NVT_ECDSA_A_VALUE_REG + j, avalue_reg);
        ecdsa_write(NVT_ECDSA_PRIME_REG + j, prime_reg);
        ecdsa_write(NVT_ECDSA_ORDER_REG + j, order_reg);
        ecdsa_write(NVT_ECDSA_BASE_X_REG + j,  basex);
        ecdsa_write(NVT_ECDSA_BASE_Y_REG + j,  basey);

        j+=4;
    }

    return 0;

}

int nvt_ecdsa_pio_normal(struct nvt_ecdsa_pio_t *p_ecdsa)
{
	int    i, j;
	uint32_t cnt;
	uint32_t base_size;
	uint32_t reg_value, trig_value;

	if (!nvt_ecdsa_opened) {
		ecdsa_err("ecdsa engine not opened!\n");
		return -1;
	}

	/* check parameter */
	if (!p_ecdsa) {
		ecdsa_err("invalid parameter\n");
		return -1;
	}

	/* check syaye is busy */
	if (ecdsa_read(NVT_ECDSA_TRG_REG_REG)) {
		ecdsa_err("engine is busy\n");
		return -1;
	}

	ecdsa_dbg("ECDSA       => size%d\n", p_ecdsa->key_w);
	ecdsa_dbg("Mode      => mode:0x%08lx\n", (uintptr_t)p_ecdsa->mode);
	ecdsa_dbg("Src       => addr:0x%08lx\n", (uintptr_t)p_ecdsa->src);

	/* check key width */
	switch (p_ecdsa->key_w) {
	case NVT_ECDSA_KEY_WIDTH_384:
		base_size = NVT_ECDSA_KEY_WIDTH_384/8;
        ecdsa_dbg("Use NVT_SECP384R1\r\n");
        break;

    case NVT_ECDSA_KEY_WIDTH_256:
		base_size = NVT_ECDSA_KEY_WIDTH_256/8;
        ecdsa_dbg("Use NVT_SECP256R1\r\n");
        break;

    case NVT_ECDSA_KEY_WIDTH_224:
		base_size = NVT_ECDSA_KEY_WIDTH_224/8;
        ecdsa_dbg("Use NVT_SECP224R1\r\n");
        break;

    case NVT_ECDSA_KEY_WIDTH_192:
		base_size = NVT_ECDSA_KEY_WIDTH_192/8;
        ecdsa_dbg("Use NVT_SECP192R1\r\n");
        break;

    default:
        ecdsa_err("Not support\r\n");
        return -1;
	}

	/* check pio busy */
	if (ecdsa_read(NVT_ECDSA_TRG_REG_REG)) {
		ecdsa_err("ecdsa PIO mode busy!!\n");
		return -1;
	}

	/* set input data */
	if (p_ecdsa->mode == NVT_ECDSA_GPK) {

		// input private ket
		for (i = 0; i < base_size; i+=4) {
        	reg_value = (p_ecdsa->key_priv[i + 3] << 24) | (p_ecdsa->key_priv[i + 2] << 16) | \
			(p_ecdsa->key_priv[i + 1] << 8) | p_ecdsa->key_priv[i];
        	ecdsa_write(NVT_ECDSA_PRIV_KEY_REG + j,  reg_value);
        	j+=4;
    	}
		trig_value = (0x1<<2);
	}

	/* set input data */
	if (p_ecdsa->mode == NVT_ECDSA_SIGN) {

		// set hash + private key + random k
    	for (i = 0; i < base_size; i+=4) {
        	reg_value = (p_ecdsa->src[i + 3] << 24) | (p_ecdsa->src[i + 2] << 16) | \
			(p_ecdsa->src[i + 1] << 8) | p_ecdsa->src[i];
        	ecdsa_write(NVT_ECDSA_SIGN_MSG_REG + i, reg_value);
			reg_value = (p_ecdsa->key_priv[i + 3] << 24) | (p_ecdsa->key_priv[i + 2] << 16) | \
			(p_ecdsa->key_priv[i + 1] << 8) | p_ecdsa->key_priv[i];
        	ecdsa_write(NVT_ECDSA_PRIV_KEY_REG + i, reg_value);
			reg_value = (p_ecdsa->randk[i + 3] << 24) | (p_ecdsa->randk[i + 2] << 16) | \
			(p_ecdsa->randk[i + 1] << 8) | p_ecdsa->randk[i];
        	ecdsa_write(NVT_ECDSA_VRIFY_S_REG + i, reg_value);
    	}
		
		trig_value = (0x1<<1);
	}

	/* set input data */
	if (p_ecdsa->mode == NVT_ECDSA_VERIFY) {

		// set hash + signx + signy + pubx + puby
    	for (i = 0; i < base_size; i+=4) {
        	reg_value = (p_ecdsa->src[i + 3] << 24) | (p_ecdsa->src[i + 2] << 16) | \
			(p_ecdsa->src[i + 1] << 8) | p_ecdsa->src[i];
        	ecdsa_write(NVT_ECDSA_SIGN_MSG_REG + i, reg_value);
			reg_value = (p_ecdsa->sign_r[i + 3] << 24) | (p_ecdsa->sign_r[i + 2] << 16) | \
			(p_ecdsa->sign_r[i + 1] << 8) | p_ecdsa->sign_r[i];
        	ecdsa_write(NVT_ECDSA_PRIV_KEY_REG + i, reg_value);
			reg_value = (p_ecdsa->sign_s[i + 3] << 24) | (p_ecdsa->sign_s[i + 2] << 16) | \
			(p_ecdsa->sign_s[i + 1] << 8) | p_ecdsa->sign_s[i];
        	ecdsa_write(NVT_ECDSA_VRIFY_S_REG + i, reg_value);
			reg_value = (p_ecdsa->key_pub_x[i + 3] << 24) | (p_ecdsa->key_pub_x[i + 2] << 16) | \
			(p_ecdsa->key_pub_x[i + 1] << 8) | p_ecdsa->key_pub_x[i];
        	ecdsa_write(NVT_ECDSA_VPKEY_X_REG + i, reg_value);
			reg_value = (p_ecdsa->key_pub_y[i + 3] << 24) | (p_ecdsa->key_pub_y[i + 2] << 16) | \
			(p_ecdsa->key_pub_y[i + 1] << 8) | p_ecdsa->key_pub_y[i];
        	ecdsa_write(NVT_ECDSA_VPKEY_Y_REG+ i, reg_value);	
    	}

		trig_value = (0x1<<0);
	}

	/* clear status */
	ecdsa_write(NVT_ECDSA_STS_REG, 0x07);

	/* trigger transfer */
	ecdsa_write(NVT_ECDSA_TRG_REG_REG, trig_value);

	/* polling status */
	cnt = 0;
	reg_value = ecdsa_read(NVT_ECDSA_STS_REG);

	while ((reg_value & 0x7) == 0) {
		udelay(2);
		reg_value = ecdsa_read(NVT_ECDSA_STS_REG);
		cnt++;
		if ((cnt%500000) == 0)
			ecdsa_inf("wait ecdsa data complete...\n");
		if (cnt > NVT_ECDSA_TIMEOUT_USEC)
			break;
	}

	/* copy output data to destination buffer */
	if ((reg_value & 0x7) == (0x1 << NVT_ECDSA_GPK)) {
		ecdsa_write(NVT_ECDSA_STS_REG, 0x07);
		for (i=0; i<base_size; i+=4) {
			reg_value = ecdsa_read(NVT_ECDSA_GPKEY_X_REG + i);
			ecdsa_dbg("ECDSA Pub Out X  => 0x%08x\n", reg_value);
			for (j=0; j<4; j++) {
				p_ecdsa->key_pub_x[i+j] = (reg_value>>(j*8))&0xff;
			}
			reg_value = ecdsa_read(NVT_ECDSA_GPKEY_Y_REG + i);
			ecdsa_dbg("ECDSA Pub Out Y   => 0x%08x\n", reg_value);
			for (j=0; j<4; j++) {
				p_ecdsa->key_pub_y[i+j] = (reg_value>>(j*8))&0xff;
			}
		}
	}
	else if ((reg_value & 0x7) == (0x1 << NVT_ECDSA_SIGN)) {
		ecdsa_write(NVT_ECDSA_STS_REG, 0x07);
		for (i=0; i<base_size; i+=4) {
			reg_value = ecdsa_read(NVT_ECDSA_SIGN_R_REG + i);
			ecdsa_dbg("ECDSA Sign Out R  => 0x%08x\n", reg_value);
			for (j=0; j<4; j++) {
				p_ecdsa->sign_r[i+j] = (reg_value>>(j*8))&0xff;
			}
			reg_value = ecdsa_read(NVT_ECDSA_SIGN_S_REG + i);
			ecdsa_dbg("ECDSA Sign Out S  => 0x%08x\n", reg_value);
			for (j=0; j<4; j++) {
				p_ecdsa->sign_s[i+j] = (reg_value>>(j*8))&0xff;
			}
		}
		
	}
	else if ((reg_value & 0x7) == (0x1 << NVT_ECDSA_VERIFY)) {

		reg_value = ecdsa_read(NVT_ECDSA_STS_REG);
		p_ecdsa->result = (reg_value >> 4);
		ecdsa_write(NVT_ECDSA_STS_REG, 0x07);
		ecdsa_dbg("verify result   => 0x%08x\n", result);
		
	}
	else {
		ecdsa_err("ecdsa_read(NVT_ECDSA_STS_REG) = 0x%x\n", ecdsa_read(NVT_ECDSA_STS_REG));
		ecdsa_err("ecdsa PIO mode timeout!!\n");
		return -1;
	}

	return 0;
}
#endif /* CONFIG_NVT_ECDSA */
