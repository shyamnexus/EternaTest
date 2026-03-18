/*
    ECDSA controller register setting

    @file       ecdsa_reg.h
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

*/

#ifndef _ECDSA_REG_H
#define _ECDSA_REG_H

#include <rcw_macro.h>

//-------------------------------------------------------
//  ECDSA registers
//-------------------------------------------------------

// 0x00, ECDSA Trigger Register
REGDEF_OFFSET(ECDSA_TRG_REG, 0x00)
REGDEF_BEGIN(ECDSA_TRG_REG)
REGDEF_BIT(VERFY_EN, 1)
REGDEF_BIT(SIGN_EN, 1) 
REGDEF_BIT(GPK_EN, 1)               
REGDEF_BIT(, 29)
REGDEF_END(ECDSA_TRG_REG)

// 0x04, ECDSA Status Register
REGDEF_OFFSET(ECDSA_STS_REG, 0x04)
REGDEF_BEGIN(ECDSA_STS_REG)
REGDEF_BIT(VERFY_DONE, 1)         
REGDEF_BIT(SIGN_DONE, 1)   
REGDEF_BIT(GPK_DONE, 1)         
REGDEF_BIT(, 1)
REGDEF_BIT(VERFY_PASS, 1) 
REGDEF_BIT(, 27)
REGDEF_END(ECDSA_STS_REG)

// 0x08, ECDSA Interrupt Enable Register
REGDEF_OFFSET(ECDSA_INTEN_REG, 0x08)
REGDEF_BEGIN(ECDSA_INTEN_REG)
REGDEF_BIT(VERFY_INTEN, 1)       
REGDEF_BIT(SIGN_INTEN, 1)    
REGDEF_BIT(GPK_INTEN, 1)    
REGDEF_BIT(, 1)
REGDEF_BIT(KEYLGTH_SEL, 2) 
REGDEF_BIT(, 1)
REGDEF_BIT(SET_384, 1) 
REGDEF_BIT(, 24)
REGDEF_END(ECDSA_INTEN_REG)

// ECDSA Signature MSG (0x6C-0x40)
// Verify: Input signature hash to verify.
// Signature: Input msg hash to sign.
// GPK: none
REGDEF_OFFSET(ECDSA_SIGN_MSG_REG, 0x40)
REGDEF_BEGIN(ECDSA_SIGN_MSG_REG)
REGDEF_BIT(SIGN_KEY, 32)
REGDEF_END(ECDSA_SIGN_MSG_REG)

// ECDSA Private Key (0xAC-0x80)
// Verify: Input signature R value to verify.
// Signature: Input private key used to sign.
// GPK: Input private key to generate public key.
REGDEF_OFFSET(ECDSA_PRIV_KEY_REG, 0x80)
REGDEF_BEGIN(ECDSA_PRIV_KEY_REG)
REGDEF_BIT(PRIV_KEY, 32)
REGDEF_END(ECDSA_PRIV_KEY_REG)

// ECDSA Verify S Value (0xEC-0xC0)
// Verify: Input signature S value to verify.
// Signature: Input random parameter to multiplied by signature.
// GPK: none.
REGDEF_OFFSET(ECDSA_VRIFY_S_REG, 0xC0)
REGDEF_BEGIN(ECDSA_VRIFY_S_REG)
REGDEF_BIT(VRIFY_S, 32)
REGDEF_END(ECDSA_VRIFY_S_REG)

// ECDSA Verify Public Key X (0x12C-0x100)
// Verify: Input public key x to verify.
// Signature: none.
// GPK: none.
REGDEF_OFFSET(ECDSA_VPKEY_X_REG, 0x100)
REGDEF_BEGIN(ECDSA_VPKEY_X_REG)
REGDEF_BIT(VPKEY_X, 32)
REGDEF_END(ECDSA_VPKEY_X_REG)

// ECDSA Verify Public Key Y (0x16C-0x140)
// Verify: Input public key y to verify.
// Signature: none.
// GPK: none.
REGDEF_OFFSET(ECDSA_VPKEY_Y_REG, 0x140)
REGDEF_BEGIN(ECDSA_VPKEY_Y_REG)
REGDEF_BIT(VPKEY_Y, 32)
REGDEF_END(ECDSA_VPKEY_Y_REG)

// ECC Base X (0x1AC-0x180)
// Verify: Input ECC base x to verify.
// Signature: Input ECC base x to sign.
// GPK: Input ECC base x to generate key.
REGDEF_OFFSET(BASE_X_REG, 0x180)
REGDEF_BEGIN(BASE_X_REG)
REGDEF_BIT(BASE_X, 32)
REGDEF_END(BASE_X_REG)

// ECC Base Y (0x1EC-0x1C0)
// Verify: Input ECC base y to verify.
// Signature: Input ECC base y to sign.
// GPK: Input ECC base y to generate key.
REGDEF_OFFSET(BASE_Y_REG, 0x1C0)
REGDEF_BEGIN(BASE_Y_REG)
REGDEF_BIT(BASE_Y, 32)
REGDEF_END(BASE_Y_REG)

// ECDSA Signature R Value (0x22C-0x200)
// Verify: none.
// Signature: Output signature R value.
// GPK: none.
REGDEF_OFFSET(SIGN_R_REG, 0x200)
REGDEF_BEGIN(SIGN_R_REG)
REGDEF_BIT(SIGN_R, 32)
REGDEF_END(SIGN_R_REG)

// ECDSA Signature S Value (0x26C-0x240)
// Verify: none.
// Signature: Output signature S value.
// GPK: none.
REGDEF_OFFSET(SIGN_S_REG, 0x240)
REGDEF_BEGIN(SIGN_S_REG)
REGDEF_BIT(SIGN_S, 32)
REGDEF_END(SIGN_S_REG)

// ECDSA Generated Public Key X (0x2AC-0x280)
// Verify: none.
// Signature: none.
// GPK: Output public key x.
REGDEF_OFFSET(GPKEY_X_REG, 0x280)
REGDEF_BEGIN(GPKEY_X_REG)
REGDEF_BIT(GPKEY_X, 32)
REGDEF_END(GPKEY_X_REG)

// ECDSA Generated Public Key Y (0x2EC-0x2C0)
// Verify: none.
// Signature: none.
// GPK: Output public key y.
REGDEF_OFFSET(GPKEY_Y_REG, 0x2C0)
REGDEF_BEGIN(GPKEY_Y_REG)
REGDEF_BIT(GPKEY_Y, 32)
REGDEF_END(GPKEY_Y_REG)

// A_Value (0x32C-0x300)
REGDEF_OFFSET(A_VALUE_REG, 0x300)
REGDEF_BEGIN(A_VALUE_REG)
REGDEF_BIT(A, 32)
REGDEF_END(A_VALUE_REG)

// Prime (0x36C-0x340)
REGDEF_OFFSET(PRIME_REG, 0x340)
REGDEF_BEGIN(PRIME_REG)
REGDEF_BIT(PRIME, 32)
REGDEF_END(PRIME_REG)

// Order (0x3AC-0x380)
REGDEF_OFFSET(ORDER_REG, 0x380)
REGDEF_BEGIN(ORDER_REG)
REGDEF_BIT(ORDER, 32)
REGDEF_END(ORDER_REG)

#endif