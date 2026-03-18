/**
	To provide the low level init.

    @file       lowlevel_init.S
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/armv8/mmu.h>



ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#if defined(CONFIG_GICV3)
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

	mrs     x15, DAIF
	ldr     x15, =0x3C0
	msr     DAIF, x15

#ifdef CONFIG_ARMV8_MULTIENTRY
	branch_if_master x0, x1, 2f

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#ifdef CONFIG_GICV2
	ldr	x0, =GICC_BASE
#endif
	bl	gic_wait_for_interrupt
#endif

	/*
	 * All slaves will enter EL1
	 */

	/* switch to EL2 */
	adr	x4, lowlevel_in_el2_v1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2
lowlevel_in_el2_v1:

	mov	lr, x29
	ret

#endif /* CONFIG_ARMV8_MULTIENTRY */

2:
	/* switch to EL2 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2
lowlevel_in_el2:


	mov	lr, x29			/* Restore LR */
	ret

ENDPROC(lowlevel_init)

ENTRY(local_load)
    str     wzr, [x1]
    add     x2, x1, #0x30
    str     w0, [x1,#4]
    ldr     w0, [x1,#48]
    str     w0, [x1,#48]

    mov     w0, #0x1                        // #1
    str     w0, [x1,#32]                    //0x20 = 1

POLL_SET_EN_STS_R:
    ldr     w0, [x2]
    tbz     w0, #0, POLL_SET_EN_STS_R       //polling 0x30 bit[0] = 1

    ldr     w0, [x1,#32]
    orr     w0, w0, #0x10
    str     w0, [x1,#32]                    //0x20 = 0x11

POLL_STATUS_R:
    ldr     w0, [x2]
    tbz     w0, #1, POLL_STATUS_R           //polling 0x30 bit[1] = 1

    ldr     w0, [x1,#8]
    str     wzr, [x1,#32]

 POLL_STATUS_R_EN:
    ldr     w3, [x2]
    tbnz    w3, #0, POLL_STATUS_R_EN
    ldr     w2, [x1,#48]
    str     w2, [x1,#48]
    ret
ENDPROC(local_load)

ENTRY(local_store)
    mov     w2, #0x4                        // #4
    str     w2, [x1]
    str     w0, [x1,#4]
    add     x0, x1, #0x30
    ldr     w2, [x1,#48]
    str     w2, [x1,#48]
    mov     w2, #0x1                        // #1
    str     w2, [x1,#32]
POLL_SET_EN_STS_P:
    ldr     w2, [x0]
    tbz     w2, #0, POLL_SET_EN_STS_P
    ldr     w2, [x1,#32]
    orr     w2, w2, #0x10
    str     w2, [x1,#32]
POLL_STATUS_P:
    ldr     w2, [x0]
    tbz     w2, #1, POLL_STATUS_P
    str     wzr, [x1,#32]

POLL_STATUS_P_EN:
    ldr     w2, [x0]
    tbnz    w2, #0, POLL_STATUS_P_EN
    ldr     w0, [x1,#48]
    str     w0, [x1,#48]
    mov     w0, #0x0                        // #0
    ret
ENDPROC(local_store)
