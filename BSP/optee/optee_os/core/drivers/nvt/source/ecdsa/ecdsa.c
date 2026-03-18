/*
    NT98567 ecdsa driver

    NT98567 ecdsa driver

    @file       ecdsac
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#include <string.h>
#include <stdlib.h>
#include <kwrap/flag.h>
#include <kwrap/semaphore.h>
#include <kwrap/spinlock.h>
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <platform_config.h>
#include <initcall.h>
#include <io.h>
#include <kernel/interrupt.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <tee/cache.h>

#include "plat/pll.h"
#include "plat/interrupt.h"
#include "plat/nvt-sramctl.h"
#include "plat/ecdsa.h"
#include "pll_protected.h"
#include "ecdsa_reg.h"
#include "ecdsa_int.h"
#include "ecdsa_platform.h"

#define FLGPTN_VERIFY_DONE          FLGPTN_BIT(0)
#define FLGPTN_SIGN_DONE          	FLGPTN_BIT(1)
#define FLGPTN_GPK_DONE          	FLGPTN_BIT(2)

static vaddr_t ECDSA_ADDR_VBASE = 0;

#define ECDSA_SETREG(_ofs, _val)   io_write32(ECDSA_ADDR_VBASE+(_ofs), (_val))
#define ECDSA_GETREG(_ofs)         io_read32(ECDSA_ADDR_VBASE+(_ofs))

#define SUPPORT_SIZE                384/8

static  BOOL    bECDSAOpened     = FALSE;
static  BOOL    verifyResult     = FALSE;
static  BOOL    bECDSASem        = FALSE;

const ECC_INFO NVT_SECP384R1 = {
	.bits_type = 384 / 8,
	.A_Value = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFC},
	.prime = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},
	.order = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0x63, 0x4D, 0x81, 0xF4, 0x37, 0x2D, 0xDF, 0x58, 0x1A, 0x0D, 0xB2, 0x48, 0xB0, 0xA7, 0x7A, 0xEC, 0xEC, 0x19, 0x6A, 0xCC, 0xC5, 0x29, 0x73},
	.base_x = {0xAA, 0x87, 0xCA, 0x22, 0xBE, 0x8B, 0x05, 0x37, 0x8E, 0xB1, 0xC7, 0x1E, 0xF3, 0x20, 0xAD, 0x74, 0x6E, 0x1D, 0x3B, 0x62, 0x8B, 0xA7, 0x9B, 0x98, 0x59, 0xF7, 0x41, 0xE0, 0x82, 0x54, 0x2A, 0x38, 0x55, 0x02, 0xF2, 0x5D, 0xBF, 0x55, 0x29, 0x6C, 0x3A, 0x54, 0x5E, 0x38, 0x72, 0x76, 0x0A, 0xB7},
	.base_y = {0x36, 0x17, 0xDE, 0x4A, 0x96, 0x26, 0x2C, 0x6F, 0x5D, 0x9E, 0x98, 0xBF, 0x92, 0x92, 0xDC, 0x29, 0xF8, 0xF4, 0x1D, 0xBD, 0x28, 0x9A, 0x14, 0x7C, 0xE9, 0xDA, 0x31, 0x13, 0xB5, 0xF0, 0xB8, 0xC0, 0x0A, 0x60, 0xB1, 0xCE, 0x1D, 0x7E, 0x81, 0x9D, 0x7A, 0x43, 0x1D, 0x7C, 0x90, 0xEA, 0x0E, 0x5F}
};

const ECC_INFO NVT_SECP256R1 = {
	.bits_type = 256 / 8,
	.A_Value = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC},
	.prime = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	.order = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84, 0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51},
	.base_x = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2, 0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96},
	.base_y = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16, 0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE, 0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5}
};

const ECC_INFO NVT_SECP224R1 = {
	.bits_type = 224 / 8,
	.A_Value = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE},
	.prime = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
	.order = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x16, 0xA2, 0xE0, 0xB8, 0xF0, 0x3E, 0x13, 0xDD, 0x29, 0x45, 0x5C, 0x5C, 0x2A, 0x3D},
	.base_x = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB7, 0x0E, 0x0C, 0xBD, 0x6B, 0xB4, 0xBF, 0x7F, 0x32, 0x13, 0x90, 0xB9, 0x4A, 0x03, 0xC1, 0xD3, 0x56, 0xC2, 0x11, 0x22, 0x34, 0x32, 0x80, 0xD6, 0x11, 0x5C, 0x1D, 0x21},
	.base_y = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBD, 0x37, 0x63, 0x88, 0xB5, 0xF7, 0x23, 0xFB, 0x4C, 0x22, 0xDF, 0xE6, 0xCD, 0x43, 0x75, 0xA0, 0x5A, 0x07, 0x47, 0x64, 0x44, 0xD5, 0x81, 0x99, 0x85, 0x00, 0x7E, 0x34}
};

const ECC_INFO NVT_SECP192R1 = {
	.bits_type = 192 / 8,
	.A_Value = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC},
	.prime = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	.order = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0xDE, 0xF8, 0x36, 0x14, 0x6B, 0xC9, 0xB1, 0xB4, 0xD2, 0x28, 0x31},
	.base_x = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x8D, 0xA8, 0x0E, 0xB0, 0x30, 0x90, 0xF6, 0x7C, 0xBF, 0x20, 0xEB, 0x43, 0xA1, 0x88, 0x00, 0xF4, 0xFF, 0x0A, 0xFD, 0x82, 0xFF, 0x10, 0x12},
	.base_y = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x19, 0x2B, 0x95, 0xFF, 0xC8, 0xDA, 0x78, 0x63, 0x10, 0x11, 0xED, 0x6B, 0x24, 0xCD, 0xD5, 0x73, 0xF9, 0x77, 0xA1, 0x1E, 0x79, 0x48, 0x11}
};

/*
  Lock ECDSA driver

  Get access right of ECDSA driver.

  @param void

  @return
    - @b E_ID:      Outside semaphore ID number range
    - @b E_NOEXS:   Semaphore does not yet exist
    - @b E_OK:      if lock succeed

  @note to wai_sem()
*/
static ER ecdsa_lock(void)
{
    ER erReturn;

    if ((erReturn = ecdsa_platform_sem_wait()) == E_OK) {
        bECDSASem = TRUE;
    }
    return erReturn;
}

/*
  Unlock ECDSA driver

  Release access right of ECDSA driver.

  @return
    - @b E_ID:      Outside semaphore ID number range
    - @b E_NOEXS:   Semaphore does not yet exist
    - @b E_QOVR:    Semaphore's counter error, maximum counter < counter
    - @b E_OK:      OK

  @note to sig_sem()
*/
static ER ecdsa_unlock(void)
{
    if (bECDSASem == TRUE) {
        bECDSASem = FALSE;
        ecdsa_platform_sem_signal();
        return E_OK;
    }
    else {
        return E_SYS;
    }
}

#ifdef ECDSA_INTERRUPT_MODE
static ID FLG_ID_ECDSA;
static struct itr_handler ecdsa_it_handler;

static enum itr_return ecdsa_isr(struct itr_handler *h __unused)
{
	T_ECDSA_STS_REG sts_reg = {0};

    /* read interrupt status */
    sts_reg.reg = ECDSA_GETREG(ECDSA_STS_REG_OFS);

    /* clear interrupt status */
    ECDSA_SETREG(ECDSA_STS_REG_OFS, sts_reg.reg);

    /* done, issue flag */
    if(sts_reg.bit.GPK_DONE) {
        iset_flg(FLG_ID_ECDSA, FLGPTN_GPK_DONE);    
    }

    if(sts_reg.bit.SIGN_DONE) {
        iset_flg(FLG_ID_ECDSA, FLGPTN_SIGN_DONE);    
    }

    if(sts_reg.bit.VERFY_DONE) {
        iset_flg(FLG_ID_ECDSA, FLGPTN_VERIFY_DONE); 
    }

    if(sts_reg.bit.VERFY_PASS) {
        verifyResult = TRUE;
    }

    return IRQ_HANDLED;
}
#endif

static void ecdsa_clkSel(UINT32 clkSel) {

    switch (clkSel) {
    case 320:
        // if (pll_get_pll_enable(PLL_ID_FIXED320) == DISABLE) {
        //    pll_set_pll_enable(PLL_ID_FIXED320, ENABLE);
        //    DBG_WRN("PLL FIXED320 is opened\r\n");
        // }
        // pll_setClockRate(PLL_CLKSEL_ECDSA, 0);
        break;
    case 240:
        // if (pll_get_pll_enable(PLL_ID_FIXED240) == DISABLE) {
        //     pll_set_pll_enable(PLL_ID_FIXED240, ENABLE);
        //     DBG_WRN("PLL FIXED240 is opened\r\n");
        // }
        // pll_setClockRate(PLL_CLKSEL_ECDSA, 1);
        break;
    default:
        break;
    }

}

/**
    Open ECDSA engine.

    @return Open ECDSA status
        - @b E_OK: Success
        - @b Others: Open ECDSA failed
*/
ER ecdsa_open(void)
{
    ER ret;

    if ((ret = ecdsa_lock()) != E_OK) {
		return ret;
	}

    bECDSAOpened = TRUE;

    return E_OK;
}


/**
    Close ECDSA engine.

    Let other tasks to use specific hash engine, including disable clock\n
    and unlock semphare

    @return Open ECDSA status
        - @b E_OK: Success
        - @b Others: Close hash failed
*/
ER ecdsa_close(void)
{
    ER ret;

	if (!bECDSAOpened)
		return E_OK;

	bECDSAOpened = FALSE;

	ret = ecdsa_unlock();

	return ret;
}

ER ecc_set_parameters(UINT32 eccSize) {

    T_A_VALUE_REG avalue_reg = {0};
    T_PRIME_REG prime_reg = {0};
    T_ORDER_REG order_reg = {0};
    T_BASE_X_REG basex = {0};
    T_BASE_Y_REG basey = {0};
    T_ECDSA_INTEN_REG intEn_reg = {0};

    ECC_INFO ecc;
    UINT32 i = 0, j = 0;
    

    switch (eccSize) {
    case 384:
        ecc = NVT_SECP384R1;
        intEn_reg.bit.KEYLGTH_SEL = 0;
        intEn_reg.bit.SET_384 = 0;
        DBG_IND("Use NVT_SECP384R1\r\n");
        break;

    case 256:
        ecc = NVT_SECP256R1;
        intEn_reg.bit.KEYLGTH_SEL = 1;
        intEn_reg.bit.SET_384 = 1;
        DBG_IND("Use NVT_SECP256R1\r\n");
        break;

    case 224:
        ecc = NVT_SECP224R1;
        intEn_reg.bit.KEYLGTH_SEL = 2;
        intEn_reg.bit.SET_384 = 1;
        DBG_IND("Use NVT_SECP224R1\r\n");
        break;

    case 192:
        ecc = NVT_SECP192R1;
        intEn_reg.bit.KEYLGTH_SEL = 3;
        intEn_reg.bit.SET_384 = 1;
        DBG_IND("Use NVT_SECP192R1\r\n");
        break;

    default:
        DBG_ERR("Not support\r\n");
        return E_NOSPT;
    }
    ECDSA_SETREG(ECDSA_INTEN_REG_OFS,  intEn_reg.reg);

    for (i = SUPPORT_SIZE - 1; i > 4; i-=4) {
        avalue_reg.reg = (ecc.A_Value[i - 3] << 24) | (ecc.A_Value[i - 2] << 16) | (ecc.A_Value[i - 1] << 8) | ecc.A_Value[i];
        prime_reg.reg = (ecc.prime[i - 3] << 24) | (ecc.prime[i - 2] << 16) | (ecc.prime[i - 1] << 8) | ecc.prime[i];
        order_reg.reg = (ecc.order[i - 3] << 24) | (ecc.order[i - 2] << 16) | (ecc.order[i - 1] << 8) | ecc.order[i];
        basex.reg = (ecc.base_x[i - 3] << 24) | (ecc.base_x[i - 2] << 16) | (ecc.base_x[i - 1] << 8) | ecc.base_x[i];
        basey.reg = (ecc.base_y[i - 3] << 24) | (ecc.base_y[i - 2] << 16) | (ecc.base_y[i - 1] << 8) | ecc.base_y[i];

        ECDSA_SETREG(A_VALUE_REG_OFS + j, avalue_reg.reg);
        ECDSA_SETREG(PRIME_REG_OFS + j, prime_reg.reg);
        ECDSA_SETREG(ORDER_REG_OFS + j, order_reg.reg);
        ECDSA_SETREG(BASE_X_REG_OFS + j,  basex.reg);
        ECDSA_SETREG(BASE_Y_REG_OFS + j,  basey.reg);

        j+=4;
    }

    return E_OK;

}

ER ECDSA_set_privateKey(UINT8* privateKey) {

    T_ECDSA_PRIV_KEY_REG privateKey_reg = {0};
    int i = 0, j = 0;

    // set private key
    for (i = 0; i < SUPPORT_SIZE; i+=4) {
        privateKey_reg.reg = (privateKey[i + 3] << 24) | (privateKey[i + 2] << 16) | (privateKey[i + 1] << 8) | privateKey[i];

        ECDSA_SETREG(ECDSA_PRIV_KEY_REG_OFS + j,  privateKey_reg.reg);
        j+=4;
    }

    return E_OK;

}

// This driver use NVT_SECP as ECC curve
ER ECDSA_GPK(UINT8* pubKey_x, UINT8* pubKey_y) {

    T_ECDSA_TRG_REG fire_reg = {0};
    T_ECDSA_INTEN_REG intEn_reg = {0};

    int i = 0;
    UINT32 tempX,tempY;
#ifdef ECDSA_INTERRUPT_MODE
	FLGPTN FlagPtn;
#else
    T_ECDSA_STS_REG ecdsa_sts_reg = {0};
	UINT32 busy_count = 0;
#endif

    // enable gpk interrupt
    intEn_reg.reg = ECDSA_GETREG(ECDSA_INTEN_REG_OFS);
    intEn_reg.bit.GPK_INTEN = TRUE;
    ECDSA_SETREG(ECDSA_INTEN_REG_OFS,  intEn_reg.reg);

    // fire gpk
    fire_reg.bit.GPK_EN = TRUE;
    ECDSA_SETREG(ECDSA_TRG_REG_OFS,  fire_reg.reg);

#ifdef ECDSA_INTERRUPT_MODE
	wai_flg(&FlagPtn, FLG_ID_ECDSA, FLGPTN_GPK_DONE, TWF_ORW | TWF_CLR);
#else
	while (1) {
		ecdsa_sts_reg.reg = ECDSA_GETREG(ECDSA_STS_REG_OFS);

		if (ecdsa_sts_reg.bit.GPK_DONE) {
			/* Write 1 Clear interrupt status */
			ECDSA_SETREG(ECDSA_STS_REG_OFS, ecdsa_sts_reg.reg);
			break;
		}

		busy_count++;
		if (busy_count > 100000000) {
			DBG_ERR("ecdsa busy_count = 0x%x\r\n", busy_count);
            return E_TMOUT;
		}
	}
#endif

    for (i = 0; i < SUPPORT_SIZE; i+=4) {
        tempX = ECDSA_GETREG(GPKEY_X_REG_OFS + i);
        tempY = ECDSA_GETREG(GPKEY_Y_REG_OFS + i);

        pubKey_x[i] = tempX & 0xFF;
        pubKey_x[i + 1] = (tempX & 0xFF00) >> 8;
        pubKey_x[i + 2] = (tempX & 0xFF0000) >> 16;
        pubKey_x[i + 3] = (tempX & 0xFFFF0000) >> 24;

        pubKey_y[i] = tempY & 0xFF;
        pubKey_y[i + 1] = (tempY & 0xFF00) >> 8;
        pubKey_y[i + 2] = (tempY & 0xFF0000) >> 16;
        pubKey_y[i + 3] = (tempY & 0xFFFF0000) >> 24;
    }

    return E_OK;

}

// This driver use NVT_SECP as ECC curve
ER ECDSA_SIGN(UINT8* hash_msg, UINT8* randomK, UINT8* sign_r, UINT8* sign_s) {

    T_ECDSA_TRG_REG fire_reg = {0};
    T_ECDSA_INTEN_REG intEn_reg = {0};
    T_ECDSA_SIGN_MSG_REG hash_msg_reg = {0};
    T_ECDSA_VRIFY_S_REG randomk_reg = {0};
#ifdef ECDSA_INTERRUPT_MODE
	FLGPTN FlagPtn;
#else
    T_ECDSA_STS_REG ecdsa_sts_reg = {0};
	UINT32 busy_count = 0;
#endif

    int i = 0;
    UINT32 tempR = 0;
    UINT32 tempS = 0;
 
    // set hash msg
    for (i = 0; i < SUPPORT_SIZE; i+=4) {
        hash_msg_reg.reg = (hash_msg[i + 3] << 24) | (hash_msg[i + 2] << 16) | (hash_msg[i + 1] << 8) | hash_msg[i];

        ECDSA_SETREG(ECDSA_SIGN_MSG_REG_OFS + i, hash_msg_reg.reg);
    }

    // set random k
    for (i = 0; i < SUPPORT_SIZE; i+=4) {
        randomk_reg.reg = (randomK[i + 3] << 24) | (randomK[i + 2] << 16) | (randomK[i + 1] << 8) | randomK[i];

        ECDSA_SETREG(ECDSA_VRIFY_S_REG_OFS + i, randomk_reg.reg);
    }

    // enable interrupt
    intEn_reg.reg = ECDSA_GETREG(ECDSA_INTEN_REG_OFS);
    intEn_reg.bit.SIGN_INTEN = TRUE;
    ECDSA_SETREG(ECDSA_INTEN_REG_OFS,  intEn_reg.reg);

    // fire
    fire_reg.bit.SIGN_EN = TRUE;
    ECDSA_SETREG(ECDSA_TRG_REG_OFS,  fire_reg.reg);

#ifdef ECDSA_INTERRUPT_MODE
	wai_flg(&FlagPtn, FLG_ID_ECDSA, FLGPTN_SIGN_DONE, TWF_ORW | TWF_CLR);
#else
	while (1) {
		ecdsa_sts_reg.reg = ECDSA_GETREG(ECDSA_STS_REG_OFS);

		if (ecdsa_sts_reg.bit.SIGN_DONE) {
			/* Write 1 Clear interrupt status */
			ECDSA_SETREG(ECDSA_STS_REG_OFS, ecdsa_sts_reg.reg);
			break;
		}

		busy_count++;
		if (busy_count > 100000000) {
			DBG_ERR("ecdsa busy_count = 0x%x\r\n", busy_count);
            return E_TMOUT;
		}
	}
#endif

    for (i = 0; i < SUPPORT_SIZE; i+=4) {
        
        tempR = ECDSA_GETREG(SIGN_R_REG_OFS + i);
        tempS = ECDSA_GETREG(SIGN_S_REG_OFS + i);

        sign_r[i] = tempR & 0xFF;
        sign_r[i + 1] = (tempR & 0xFF00) >> 8;
        sign_r[i + 2] = (tempR & 0xFF0000) >> 16;
        sign_r[i + 3] = (tempR & 0xFFFF0000) >> 24;

        sign_s[i] = tempS & 0xFF;
        sign_s[i + 1] = (tempS & 0xFF00) >> 8;
        sign_s[i + 2] = (tempS & 0xFF0000) >> 16;
        sign_s[i + 3] = (tempS & 0xFFFF0000) >> 24;
    }


    return E_OK;

}

// This driver use NVT_SECP as ECC curve
ER ECDSA_VERIFY(UINT8* hash_msg, UINT8* sign_r, UINT8* sign_s, UINT8* pubKey_x, UINT8* pubKey_y, bool* result, unsigned int key_size) {

    T_ECDSA_TRG_REG fire_reg = {0};
    T_ECDSA_INTEN_REG intEn_reg = {0};
    T_ECDSA_SIGN_MSG_REG hash_msg_reg = {0};
    T_ECDSA_PRIV_KEY_REG verifyR_reg = {0};
    T_ECDSA_VRIFY_S_REG verifyS_reg = {0};
    T_ECDSA_VPKEY_X_REG pubKeyX_reg = {0};
    T_ECDSA_VPKEY_Y_REG pubKeyY_reg = {0};

#ifdef ECDSA_INTERRUPT_MODE
	FLGPTN FlagPtn;
#else
    T_ECDSA_STS_REG ecdsa_sts_reg = {0};
	UINT32 busy_count = 0;
#endif
    unsigned int i = 0;

    // set hash msg
    for (i = 0; i < key_size; i+=4) {
        verifyR_reg.reg = (sign_r[i + 3] << 24) | (sign_r[i + 2] << 16) | (sign_r[i + 1] << 8) | sign_r[i];
        verifyS_reg.reg = (sign_s[i + 3] << 24) | (sign_s[i + 2] << 16) | (sign_s[i + 1] << 8) | sign_s[i];
        pubKeyX_reg.reg = (pubKey_x[i + 3] << 24) | (pubKey_x[i + 2] << 16) | (pubKey_x[i + 1] << 8) | pubKey_x[i];
        pubKeyY_reg.reg = (pubKey_y[i + 3] << 24) | (pubKey_y[i + 2] << 16) | (pubKey_y[i + 1] << 8) | pubKey_y[i];
        
        ECDSA_SETREG(ECDSA_PRIV_KEY_REG_OFS + i, verifyR_reg.reg);
        ECDSA_SETREG(ECDSA_VRIFY_S_REG_OFS + i, verifyS_reg.reg);
        ECDSA_SETREG(ECDSA_VPKEY_X_REG_OFS + i, pubKeyX_reg.reg);
        ECDSA_SETREG(ECDSA_VPKEY_Y_REG_OFS + i, pubKeyY_reg.reg);
    }
    for (i = 0; i < 32; i+=4) {
        hash_msg_reg.reg = (hash_msg[i + 3] << 24) | (hash_msg[i + 2] << 16) | (hash_msg[i + 1] << 8) | hash_msg[i];
        ECDSA_SETREG(ECDSA_SIGN_MSG_REG_OFS + i, hash_msg_reg.reg);
    }

    // enable interrupt
    intEn_reg.reg = ECDSA_GETREG(ECDSA_INTEN_REG_OFS);
    intEn_reg.bit.VERFY_INTEN = TRUE;
    ECDSA_SETREG(ECDSA_INTEN_REG_OFS,  intEn_reg.reg);

    // fire
    fire_reg.bit.VERFY_EN = TRUE;
    ECDSA_SETREG(ECDSA_TRG_REG_OFS,  fire_reg.reg);

#ifdef ECDSA_INTERRUPT_MODE
	wai_flg(&FlagPtn, FLG_ID_ECDSA, FLGPTN_VERIFY_DONE, TWF_ORW | TWF_CLR);
#else
	while (1) {
		ecdsa_sts_reg.reg = ECDSA_GETREG(ECDSA_STS_REG_OFS);

		if (ecdsa_sts_reg.bit.VERFY_DONE) {
			/* Write 1 Clear interrupt status */
			verifyResult = (ECDSA_GETREG(ECDSA_STS_REG_OFS)  >> 4);
			ECDSA_SETREG(ECDSA_STS_REG_OFS, ecdsa_sts_reg.reg);
			break;
		}

		busy_count++;
		if (busy_count > 100000000) {
			DBG_ERR("ecdsa busy_count = 0x%x\r\n", busy_count);
            return E_TMOUT;
		}
	}

#endif

	*result = verifyResult;

	return TEE_SUCCESS;

}

static void ecdsa_platform_init(void)
{
	/* disabel ECDSA module reset */
	pll_enable_system_reset(ECDSA_RSTN);
	pll_disable_system_reset(ECDSA_RSTN);

    ecdsa_clkSel(240);

	/* enable RSA module master clock */
	pll_enable_clock(ECDSA_CLK);
}

static TEE_Result nvt_ecdsa_init(void)
{
	/* get register access base */
	ECDSA_ADDR_VBASE = (vaddr_t)phys_to_virt(IOADDR_ECDSA_REG_BASE, MEM_AREA_IO_SEC, 0x1000);

	/* rsa clock and reset init */
	ecdsa_platform_init();

	/* create platform resource */
	ecdsa_platform_create_resource();

#ifdef ECDSA_INTERRUPT_MODE
	/* create flag */
	cre_flg(&FLG_ID_ECDSA, NULL, (char *)"FLG_ID_ECDSA");
	clr_flg(FLG_ID_ECDSA, 0xFFFFFFFF);

	ecdsa_it_handler.it      = INT_ID_ECDSA;
	ecdsa_it_handler.flags   = ITRF_TRIGGER_LEVEL;
	ecdsa_it_handler.handler = ecdsa_isr;
	ecdsa_it_handler.data    = NULL;

	itr_add(&ecdsa_it_handler);
	itr_enable(ecdsa_it_handler.it);
#endif

	return TEE_SUCCESS;
}

driver_init(nvt_ecdsa_init);
