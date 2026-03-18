/**
    NVT sleep function
    To handle CPU context save/restore during suspend/resume stage
    @file sleep.S
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/


#include <linux/linkage.h>
#include <asm/assembler.h>
#include "plat/hardware.h"

	.extern nvt_pm_func_mode
	.extern nvt_pm_ddr_clock
    .text
    .align 5
ENTRY(na51055_print_char_virt)
        ldr     r5, = NVT_PM_UART_BASE_VIRT
        str     r0, [r5]
        ldr     r0, =0x40000
loop1:
        subs    r0, r0, #4
        cmp     r0, #0x0
        bne     loop1
        nop
        isb
        mov     pc, lr
ENDPROC(na51055_print_char_virt)


ENTRY(nvt_cpu_suspend)
        STMFD   sp!, {r0-r6,lr}
        mov     r0, #80         //show 'P'
        bl      na51055_print_char_virt
        ldr     r5, =NVT_IFE_SRAM_BASE_VIRT
        mov     r4, sp
        str     r4, [r5]
		ldr     r0, =nvt_pm_func_mode
		ldr     r1, [r0]
		ldr     r0, =nvt_pm_ddr_clock
		ldr     r2, [r0]
        mov     r0, #3         //Sleep3 mode
        add     r6, r5, #0x74  //code start at 0xf07c0074
        blx     r6
        mov     r0, #83        //show 'S'
        bl      na51055_print_char_virt
        ldr     r5, =NVT_IFE_SRAM_BASE_VIRT
        ldr     r4, [r5]
        mov     sp, r4
        LDMIA   sp!, {r0-r6,lr}
        mov     pc, lr
ENDPROC(nvt_cpu_suspend)

