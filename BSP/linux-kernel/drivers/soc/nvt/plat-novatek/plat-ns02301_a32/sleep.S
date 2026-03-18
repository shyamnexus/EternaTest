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


        .extern nvt_wakeup_int_padding
    .text
    .align 5
ENTRY(ns02301_print_char_virt)
        ldr     r5, = NVT_UART1_BASE_VIRT
        str     r0, [r5]
        ldr     r0, =0x40000
loop1:
        subs    r0, r0, #4
        cmp     r0, #0x0
        bne     loop1
        nop
        isb
        mov     pc, lr
ENDPROC(ns02301_print_char_virt)

    .text
    .align 5

ENTRY(nvt_cpu_suspend)
        STMFD   sp!, {r0-r6,lr}
        ldr     r0, =NVT_CG_BASE_VIRT
        add     r0, r0, 0x74
        ldr     r1, [r0]
        orr     r1, r1, 0x400
        str     r1, [r0]
        mov     r0, #80         //show 'P'
        bl      ns02301_print_char_virt
        ldr     r0, =nvt_wakeup_int_padding
        ldr     r1, [r0]
        cmp     r1, #0x0
        bne     ncs_exit
        nop
        ldr     r5, =NVT_PR_TCM_SRAM_BASE_VIRT
        mov     r4, sp
        str     r4, [r5]
        mov     r0, #3         //Sleep3 mode
        add     r6, r5, #0x74  //code start at 0xf0900074
        blx     r6
        ldr     r5, =NVT_PR_TCM_SRAM_BASE_VIRT
        ldr     r4, [r5]
        mov     sp, r4
ncs_exit:
        mov     r0, #83        //show 'S'
        bl      ns02301_print_char_virt
        LDMIA   sp!, {r0-r6,lr}
        mov     pc, lr
ENDPROC(nvt_cpu_suspend)
