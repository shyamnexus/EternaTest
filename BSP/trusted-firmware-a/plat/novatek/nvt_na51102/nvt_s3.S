
/*
 * Copyright (c) 2024, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>


    .globl  plat_boot_cpu_save_data
    .globl  plat_restore_cpu_data
    .globl  plat_restore_content
    .globl  plat_nonboot_cpu_save_data


func plat_boot_cpu_save_data
    mov x1, x0
    mov x2, sp
    add x2, x2, 16
    stp x29, x30, [sp, #-16]!
    ldr x0, = S3_DATA_PTR_ADDR
    ldr x0, [x0]
    ldr x3 , =S3_CPU_CORE0_DATA_OFFSET
    add x3, x3, x0
    stp x1, x2, [x3]
    add x0, x3, 16
    bl  plat_save_cpu_data
    ldp x29, x30, [sp], #16
    ret
endfunc plat_boot_cpu_save_data

func plat_nonboot_cpu_save_data
        ldr  x1, = S3_MAGIC_ADDR
        ldr  x1, [x1]
        ldr  x2, = S3_MAGIC_NUMBER
        cmp  x1, x2
        bne  4f
        mov  x2, x30
        bl   plat_calc_core_pos
        mov  x30, x2
        ldr  x2, = S3_DATA_PTR_ADDR
        ldr  x2, [x2]
        cmp  x0, #0
        beq  0f
        cmp  x0, #1
        beq  1f
        cmp  x0, #2
        beq  2f
        cmp  x0, #3
        beq  3f
3:
        ldr  x1 , =S3_CPU_CORE3_DATA_OFFSET
        add  x1, x1, 16
        add  x0, x1, x2
        b    save_data
2:
        ldr  x1 , =S3_CPU_CORE2_DATA_OFFSET
        add  x1, x1, 16
        add  x0, x1, x2
        b    save_data
1:
        ldr  x1 , =S3_CPU_CORE1_DATA_OFFSET
        add  x1, x1, 16
        add  x0, x1, x2
        b    save_data
0:
        ldr  x1 , =S3_CPU_CORE0_DATA_OFFSET
        add  x1, x1, 16
        add  x0, x1, x2
save_data:
        mov  x1, sp
        str  x1, [x0, -8]
        mov  x1, x30
        str  x1, [x0, -16]
        bl   plat_save_cpu_data
        ldr  x30, [x0, -16]
4:
        ret
endfunc plat_nonboot_cpu_save_data


func plat_save_cpu_data
    stp    x1, x2, [x0, #0]
    stp    x3, x4, [x0, #16]
    stp    x5, x6, [x0, #32]
    stp    x7, x8, [x0, #48]
    stp    x9, x10, [x0, #64]
    stp    x11, x12, [x0, #80]
    stp    x13, x14, [x0, #96]
    stp    x15, x16, [x0, #112]
    stp    x17, x18, [x0, #128]
    stp    x19, x20, [x0, #144]
    stp    x21, x22, [x0, #160]
    stp    x23, x24, [x0, #176]
    stp    x25, x26, [x0, #192]
    stp    x27, x28, [x0, #208]
    stp    x29, x30, [x0, #224]
    mrs    x2, par_el1
    mrs    x3, daif
    mrs    x4, spsr_el1
    mrs    x5, spsr_el2
    mrs    x6, spsr_el3
    mrs    x7, elr_el1
    mrs    x8, elr_el2
    mrs    x9, elr_el3
    mrs    x10, scr_el3
    mrs    x11, hcr_el2
    mrs    x12, vbar_el1
    mrs    x13, vbar_el2
    mrs    x14, vbar_el3
    mrs    x15, sctlr_el1
    mrs    x16, sctlr_el2
    mrs    x17, tpidr_el1
    mrs    x18, actlr_el1
    mrs    x19, actlr_el2
    mrs    x20, actlr_el3
    mrs    x21, esr_el1
    mrs    x22, esr_el2
    mrs    x23, esr_el3
    mrs    x24, afsr0_el1
    mrs    x25, afsr0_el2
    mrs    x26, afsr0_el3
    mrs    x27, afsr1_el1
    stp    x2, x3, [x0, #240]
    stp    x4, x5, [x0, #256]
    stp    x6, x7, [x0, #272]
    stp    x8, x9, [x0, #288]
    stp    x10, x11, [x0, #304]
    stp    x12, x13, [x0, #320]
    stp    x14, x15, [x0, #336]
    stp    x16, x17, [x0, #352]
    stp    x18, x19, [x0, #368]
    stp    x20, x21, [x0, #384]
    stp    x22, x23, [x0, #400]
    stp    x24, x25, [x0, #416]
    stp    x26, x27, [x0, #432]
    mrs    x2, afsr1_el2
    mrs    x3, afsr1_el3
    mrs    x4, far_el1
    mrs    x5, far_el2
    mrs    x6, far_el3
    mrs    x7, mair_el1
    mrs    x8, mair_el2
    mrs    x9, mair_el3
    mrs    x10, amair_el1
    mrs    x11, amair_el2
    mrs    x12, amair_el3
    mrs    x13, tcr_el1
    mrs    x14, tcr_el2
    mrs    x15, tcr_el3
    mrs    x16, ttbr0_el1
    mrs    x17, ttbr0_el2
    mrs    x18, ttbr0_el3
    mrs    x19, ttbr1_el1
    mrs    x20, vttbr_el2
    mrs    x21, cptr_el2
    mrs    x22, cptr_el3
    mrs    x23, cpacr_el1
    mrs    x24, cntfrq_el0
    mrs    x25, cnthp_ctl_el2
    mrs    x26, cnthp_tval_el2
    mrs    x27, cnthp_cval_el2
    stp    x2, x3, [x0, #448]
    stp    x4, x5, [x0, #464]
    stp    x6, x7, [x0, #480]
    stp    x8, x9, [x0, #496]
    add    x0, x0, #512
    stp    x10, x11, [x0, #0]
    stp    x12, x13, [x0, #16]
    stp    x14, x15, [x0, #32]
    stp    x16, x17, [x0, #48]
    stp    x18, x19, [x0, #64]
    stp    x20, x21, [x0, #80]
    stp    x22, x23, [x0, #96]
    stp    x24, x25, [x0, #112]
    stp    x26, x27, [x0, #128]
    mrs    x2, cntps_ctl_el1
    mrs    x3, cntps_tval_el1
    mrs    x4, cntps_cval_el1
    mrs    x5, cntp_ctl_el0
    mrs    x6, cntp_tval_el0
    mrs    x7, cntp_cval_el0
    mrs    x8, cnthctl_el2
    mrs    x9, tpidr_el3
    mrs    x10, cntvoff_el2
    mrs    x11, vpidr_el2
    mrs    x12, vmpidr_el2
    mrs    x13, mdcr_el2
    mrs    x14, mdcr_el3
    mrs    x15, hstr_el2
    mrs    x16, S3_1_C15_C2_0
    msr    spsel, #0x1
    mov    x17, sp
    msr    spsel, #0x0
    mrs    x18, tpidr_el0
    mrs    x19, tpidrro_el0
    mrs    x20, contextidr_el1
    mrs    x21, osdlr_el1
    mrs    x22, mdscr_el1
    mrs    x23, sp_el1
    mrs    x24, sp_el2
    mrs    x25, S3_1_C15_C2_1
    mrs    x26, pmintenset_el1
    mrs    x27, pmcntenset_el0
    stp    x2, x3, [x0, #144]
    stp    x4, x5, [x0, #160]
    stp    x6, x7, [x0, #176]
    stp    x8, x9, [x0, #192]
    stp    x10, x11, [x0, #208]
    stp    x12, x13, [x0, #224]
    stp    x14, x15, [x0, #240]
    stp    x16, x17, [x0, #256]
    stp    x18, x19, [x0, #272]
    stp    x20, x21, [x0, #288]
    stp    x22, x23, [x0, #304]
    stp    x24, x25, [x0, #320]
    stp    x26, x27, [x0, #336]
    mrs    x2, pmuserenr_el0
    mrs    x3, pmcr_el0
    mrs    x4, pmccfiltr_el0
    mrs    x5, cntkctl_el1
    mrs    x6, sctlr_el3
    stp    x2, x3, [x0, #352]
    stp    x4, x5, [x0, #368]
    stp    x6, x7, [x0, #384]
    sub    x0, x0, #512
    ret
endfunc plat_save_cpu_data

func plat_restore_cpu_data
    ldp    x2, x3, [x0, #240]
    ldp    x4, x5, [x0, #256]
    ldp    x6, x7, [x0, #272]
    ldp    x8, x9, [x0, #288]
    ldp    x10, x11, [x0, #304]
    ldp    x12, x13, [x0, #320]
    ldp    x14, x15, [x0, #336]
    ldp    x16, x17, [x0, #352]
    ldp    x18, x19, [x0, #368]
    ldp    x20, x21, [x0, #384]
    ldp    x22, x23, [x0, #400]
    ldp    x24, x25, [x0, #416]
    ldp    x26, x27, [x0, #432]
    msr    par_el1, x2
    msr    daif, x3
    msr    spsr_el1, x4
    msr    spsr_el2, x5
    msr    spsr_el3, x6
    msr    elr_el1, x7
    msr    elr_el2, x8
    msr    elr_el3, x9
    msr    scr_el3, x10
    msr    hcr_el2, x11
    msr    vbar_el1, x12
    msr    vbar_el2, x13
    msr    vbar_el3, x14
    msr    sctlr_el1, x15
    msr    sctlr_el2, x16
    msr    tpidr_el1, x17
    msr    actlr_el1, x18
    msr    actlr_el2, x19
    msr    actlr_el3, x20
    msr    esr_el1, x21
    msr    esr_el2, x22
    msr    esr_el3, x23
    msr    afsr0_el1, x24
    msr    afsr0_el2, x25
    msr    afsr0_el3, x26
    msr    afsr1_el1, x27
    ldp    x2, x3, [x0, #448]
    ldp    x4, x5, [x0, #464]
    ldp    x6, x7, [x0, #480]
    ldp    x8, x9, [x0, #496]
    add    x0, x0, #512
    ldp    x10, x11, [x0, #0]
    ldp    x12, x13, [x0, #16]
    ldp    x14, x15, [x0, #32]
    ldp    x16, x17, [x0, #48]
    ldp    x18, x19, [x0, #64]
    ldp    x20, x21, [x0, #80]
    ldp    x22, x23, [x0, #96]
    ldp    x24, x25, [x0, #112]
    ldp    x26, x27, [x0, #128]
    msr    afsr1_el2, x2
    msr    afsr1_el3, x3
    msr    far_el1, x4
    msr    far_el2, x5
    msr    far_el3, x6
    msr    mair_el1, x7
    msr    mair_el2, x8
    msr    mair_el3, x9
    msr    amair_el1, x10
    msr    amair_el2, x11
    msr    amair_el3, x12
    msr    tcr_el1, x13
    msr    tcr_el2, x14
    msr    tcr_el3, x15
    msr    ttbr0_el1, x16
    msr    ttbr0_el2, x17
    msr    ttbr0_el3, x18
    msr    ttbr1_el1, x19
    msr    vttbr_el2, x20
    msr    cptr_el2, x21
    msr    cptr_el3, x22
    msr    cpacr_el1, x23
    msr    cntfrq_el0, x24
    msr    cnthp_ctl_el2, x25
    msr    cnthp_tval_el2, x26
    msr    cnthp_cval_el2, x27
    ldp    x2, x3, [x0, #144]
    ldp    x4, x5, [x0, #160]
    ldp    x6, x7, [x0, #176]
    ldp    x8, x9, [x0, #192]
    ldp    x10, x11, [x0, #208]
    ldp    x12, x13, [x0, #224]
    ldp    x14, x15, [x0, #240]
    ldp    x16, x17, [x0, #256]
    ldp    x18, x19, [x0, #272]
    ldp    x20, x21, [x0, #288]
    ldp    x22, x23, [x0, #304]
    ldp    x24, x25, [x0, #320]
    ldp    x26, x27, [x0, #336]
    msr    cntps_ctl_el1, x2
    msr    cntps_tval_el1, x3
    msr    cntps_cval_el1, x4
    msr    cntp_ctl_el0, x5
    msr    cntp_tval_el0, x6
    msr    cntp_cval_el0, x7
    msr    cnthctl_el2, x8
    msr    tpidr_el3, x9
    msr    cntvoff_el2, x10
    msr    vpidr_el2, x11
    msr    vmpidr_el2, x12
    msr    mdcr_el2, x13
    msr    mdcr_el3, x14
    msr    hstr_el2, x15
    msr    S3_1_C15_C2_0, x16
    msr    spsel, #0x1
    mov    sp, x17
    msr    spsel, #0x0
    msr    tpidr_el0, x18
    msr    tpidrro_el0, x19
    msr    contextidr_el1, x20
    msr    osdlr_el1, x21
    msr    mdscr_el1, x22
    msr    sp_el1, x23
    msr    sp_el2, x24
    msr    S3_1_C15_C2_1, x25
    msr    pmintenset_el1, x26
    msr    pmcntenset_el0, x27
    ldp    x2, x3, [x0, #352]
    ldp    x4, x5, [x0, #368]
    ldp    x6, x7, [x0, #384]
    msr    pmuserenr_el0, x2
    msr    pmcr_el0, x3
    msr    pmccfiltr_el0, x4
    msr    cntkctl_el1, x5
    msr    sctlr_el3, x6        
    sub    x0, x0, #512
    ldp    x1, x2, [x0, #0]
    ldp    x3, x4, [x0, #16]
    ldp    x5, x6, [x0, #32]
    ldp    x7, x8, [x0, #48]
    ldp    x9, x10, [x0, #64]
    ldp    x11, x12, [x0, #80]
    ldp    x13, x14, [x0, #96]
    ldp    x15, x16, [x0, #112]
    ldp    x17, x18, [x0, #128]
    ldp    x19, x20, [x0, #144]
    ldp    x21, x22, [x0, #160]
    ldp    x23, x24, [x0, #176]
    ldp    x25, x26, [x0, #192]
    ldp    x27, x28, [x0, #208]
    ldp    x29, x30, [x0, #224]
    sub    x0, x0, #16
    ldr    x30, [x0]
    ldr    x0, [x0, #8]
    mov    sp, x0
    ret
endfunc plat_restore_cpu_data

func plat_restore_content
    bl   plat_calc_core_pos
    ldr  x1, =S3_ATF_DATA_PTR_ADDR
    ldr  x1, [x1]
    cmp  x0, #0
    beq  0f
    cmp  x0, #1
    beq  1f
    cmp  x0, #2
    beq  2f
    cmp  x0, #3
    beq  3f
3:
    ldr  x2, = S3_CPU_CORE3_DATA_OFFSET
    add  x2, x2, 16
    add  x0, x2, x1
    bl   plat_restore_cpu_data
2:
    ldr  x2, = S3_CPU_CORE2_DATA_OFFSET
    add  x2, x2, 16
    add  x0, x2, x1
    bl   plat_restore_cpu_data
1:
    ldr  x2, = S3_CPU_CORE1_DATA_OFFSET
    add  x2, x2, 16
    add  x0, x2, x1
    bl   plat_restore_cpu_data
0:
    ldr  x2, = S3_CPU_CORE0_DATA_OFFSET
    add  x2, x2, 16
    add  x0, x2, x1
    bl   plat_restore_cpu_data
    ret
endfunc plat_restore_content        
