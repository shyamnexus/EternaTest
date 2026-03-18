# ******************************************************************************
#   Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
#
#   @file       cpu_s.S
#   @ingroup
#
#   @brief      CPU cache related file
#
#   @note       Nothing.
# ******************************************************************************
#ifndef ALIGN
#define ALIGN .align        4
#endif

#ifndef ENTRY
#define ENTRY(name) \
	.globl name; \
	ALIGN; \
	name:
#endif

#ifndef WEAK
#define WEAK(name) \
	.weak name; \
	name:
#endif

#ifndef END
#define END(name) \
	.size name, .-name
#endif

#define ENDPROC(name) \
	.type name, %function; \
	END(name)
#if 0
ENTRY(_lockdownICache)
// This function lockdown instructions into ICache.
// r0 is the starting address and r1 is the ending address
// All instructions located from starting to ending address will be lockdowned into ICache
STMFD   r13!, {r2 - r3, r14}
// 8 words alignment
MOV     r2, #0x1F
BIC     r0, r0, r2
AND     r3, r1, r2
CMP     r3, #0x0
BEQ     I_EightWordAlign
BIC     r1, r1, r2
ADD     r1, r1, #16

I_EightWordAlign:
MOV     r2, #0x80000000
MCR     p15, 0, r2, c9, c0, 1

I_Lockdown:
MCR     p15, 0, r0, c7, c13, 1
ADD     r0, r0, #16
CMP     r0, r1
BNE     I_Lockdown

MOV     r2, #0x0
MCR     p15, 0, r2, c9, c0, 1

LDMFD   r13!, {r2 - r3, pc}
ENDPROC(_lockdownICache)

// This function lockdown data into DCache
// r0 is the starting address and r1 is the ending address
// All data located from starting to ending address will be lockdowned into DCache
ENTRY(_lockdownDCache)
STMFD   r13!, {r2 - r3, r14}

// 8 Words alignment
MOV     r2, #0x1F
BIC     r0, r0, r2
AND     r3, r1, r2
CMP     r3, #0x0
BEQ     D_EightWordAlign
BIC     r1, r1, r2
ADD     r1, r1, #16

D_EightWordAlign:
MOV     r2, #0x80000000
MCR     p15, 0, r2, c9, c0, 0

D_Lockdown:
LDR     r2, [r0], #4
CMP     r0, r1
BNE     D_Lockdown

MOV     r2, #0x0
MCR     p15, 0, r2, c9, c0, 0

LDMFD   r13!, {r2 - r3, pc}
ENDPROC(_lockdownDCache)
#endif
ENTRY(INCR_64)
/************************************************************************
 * r0 is address of key set that user want to configure
 * r1 is the binary starting address
 ************************************************************************/
#if 0
STMFD   r13!, {r0 - r12, r14}
LDR r0, = 0xF0290000
		  LDR r1, = 'I'
					STR r1, [r0]
					LDR r1, = 'N'
							  STR r1, [r0]
							  LDR r1, = 'C'
										STR r1, [r0]
										LDR r1, = 'R'
												STR r1, [r0]
												LDR r1, = '6'
														STR r1, [r0]
														LDR r1, = '4'
																LDMFD   r13!, {r0 - r12, pc} //If r7 !=1 than return
																nop
#else
STMIA   r13!, {r2 - r10, r14}
LDR r0, = 0xF0290000
		  LDR r1, = 'I'
					STR r1, [r0]
					LDR r1, = 'N'
							  STR r1, [r0]
							  LDR r1, = 'C'
										STR r1, [r0]
										LDR r1, = 'R'
												STR r1, [r0]
												LDR r1, = '6'
														STR r1, [r0]
														LDR r1, = '4'
																STR r1, [r0]
																LDMIA   r13!, {r2 - r10, pc} //If r7 !=1 than return
																nop

#endif
																ENDPROC(INCR_64)
