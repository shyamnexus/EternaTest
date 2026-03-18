//#include "regdef.h"

# use CC_CPU2_CPU1_CMDBUF_REG1 to store uboot starting address
#define CC_CPU2_CPU1_CMDBUF_REG1 0xF07F800C
#define CPU_TIMER_SETTING 0x00B71B00

/*
 ************************************************************************
 *    Multiprocessor Affubuty register, MPIDR (cp15, 0, c0, c0, 5)      *
 ************************************************************************
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |1|U|Reserved                           |Cluster|SBZ        |CPU|
 * | | |                                   |ID     |           |ID |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define S_MPIDR_U				30
#define M_MPIDR_U				(0x1 << S_MPIDR_U)
#define S_MPIDR_CLUSTER_ID		8
#define M_MPIDR_CLUSTER_ID		(0xf << S_MPIDR_CLUSTER_ID)
#define S_MPIDR_CPU_ID			0
#define M_MPIDR_CPU_ID			(0x3 << S_MPIDR_CPU_ID)
.text
.section .smp,"ax"

_smp_:
	ldr     r0, =0xF0290000
    ldr     r1, ='2'
    str     r1, [r0]



	ldr     r0, =0xF0290000 			/* print $ */
	ldr     r1, ='c'
	str     r1, [r0]
	ldr     r1, ='o'
	str     r1, [r0]
	ldr     r1, ='r'
	str     r1, [r0]
	ldr     r1, ='2'
	str     r1, [r0]



	LDR		r1, =CC_CPU2_CPU1_CMDBUF_REG1			// check core2 entry address ready
	LDR		r0, [r1]

	isb
	dsb
	MOV		pc, r0
	nop
	nop
.end
