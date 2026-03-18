

.org 0
.text
.globl _sram_vector_table
.extern Reset
.extern UndefinedHandler
.extern SWIHandler
.extern PrefetchAbortHandler
.extern DataAbortHandler
.extern ReservedHandler
.extern IRQHandler
.extern FIQHandler
.extern dispatch_interrupt
.section .vtrs, "ax"

#ifdef __aarch64__
#else
_sram_vector_table:
/**
 * vector table
 * ===========================
 * 00 - Reset
 * 04 - Undefined instructions
 * 08 - SWI instructions
 * 0C - prefetch abort
 * 10 - Data abort
 * 14 - Reserved
 * 18 - IRQ interrupts
 * 1C - FIQ interrupts
 */

	ldr     pc, _vectorReset
    ldr     pc, _vectorUndefined
    ldr     pc, _vectorSWI
    ldr     pc, _vectorPrefetchAbort
    ldr     pc, _vectorDataAbort
    ldr     pc, _vectorReserved
    ldr     pc, _vectorIRQ
    ldr     pc, _vectorFIQ

_vectorReset:
	.word	Reset
_vectorUndefined:
	.word   UndefinedHandler
_vectorSWI:
	.word   SWIHandler
_vectorPrefetchAbort:
	.word   PrefetchAbortHandler
_vectorDataAbort:
	.word   DataAbortHandler
_vectorReserved:
	.word   ReservedHandler
_vectorIRQ:
	.word   IRQHandler
_vectorFIQ:
	.word   FIQHandler


Reset:
	ldr	r0, =0xF0290000
	ldr r1, ='R'
	str r1, [r0]
	ldr r1, ='S'
	str r1, [r0]
	ldr r1, ='T'
	str r1, [r0]
reset_block:
	b		reset_block
	nop

UndefinedHandler:
	ldr	r0, =0xf0290000
	ldr r1, ='U'
	str r1, [r0]
	ldr r1, ='D'
	str r1, [r0]
	ldr r1, ='F'
	str r1, [r0]
#udf_block:
#	b		udf_block
    subs	pc, lr, #4
	nop


SWIHandler:
	ldr	r0, =0xf0290000
	ldr r1, ='S'
	str r1, [r0]
	ldr r1, ='W'
	str r1, [r0]
	ldr r1, ='I'
	str r1, [r0]
swi_block:
	b		swi_block
	nop

PrefetchAbortHandler:
	ldr	r0, =0xf0290000
	ldr r1, ='P'
	str r1, [r0]
	ldr r1, ='R'
	str r1, [r0]
	ldr r1, ='F'
	str r1, [r0]
#pref_block:
#	b		pref_block
    subs	pc, lr, #4
	nop

DataAbortHandler:
	ldr	r0, =0xf0290000
	ldr r1, ='A'
	str r1, [r0]
	ldr r1, ='B'
	str r1, [r0]
	ldr r1, ='T'
	str r1, [r0]
	ldr r1, ='T'
	str r1, [r0]
#abort_block:
#	b		abort_block
    subs	pc, lr, #4
	nop

ReservedHandler:
	ldr	r0, =0xf0290000
	ldr r1, ='R'
	str r1, [r0]
	ldr r1, ='S'
	str r1, [r0]
	ldr r1, ='V'
	str r1, [r0]
rsv_block:
	b		rsv_block
	nop

IRQHandler:
	ldr	r0, =0xf0290000
	ldr r1, =0x24
	str r1, [r0]
	sub	sp, sp, #72
	stmia	sp, {r0 - r12}
	add	r8, sp, #60
	str r1, [r0]
	stmdb	r8, {sp, lr}^
	str r1, [r0]
	str	lr, [r8, #0]
	str r1, [r0]
	mrs	r6, spsr
	str r1, [r0]
	str	r6, [r8, #4]
	str r1, [r0]
	str	r0, [r8, #8]
	str r1, [r0]

	ldr	r0, =0xf0290000
	ldr r1, ='I'
	str r1, [r0]
	ldr r1, ='R'
	str r1, [r0]
	ldr r1, ='Q'
	str r1, [r0]

	mov	r0, sp
	bl dispatch_interrupt
#   mcr p15, 0, sp, c7, c6, 1
	ldmia	sp, {r0 - lr}^
	mov	r0, r0
	ldr	lr, [sp, #60]
	add	sp, sp, #72
	subs	pc, lr, #4
	nop

FIQHandler:
	sub	sp, sp, #52
	stmia	sp, {r0 - r7}
	add	r8, sp, #40
	stmdb	r8, {sp, lr}^
	str	lr, [r8, #0]
	mrs	r6, spsr
	str	r6, [r8, #4]
	str	r0, [r8, #8]
	mov	r0, sp
	bl dispatch_interrupt

	ldr	r0, =0xf0290000
	ldr r1, ='F'
	str r1, [r0]
	ldr r1, ='I'
	str r1, [r0]
	ldr r1, ='Q'
	str r1, [r0]

	ldmia	sp, {r0 - r7}^
	ldr	lr, [sp, #40]
	add	sp, sp, #52
	subs	pc, lr, #4
	nop
.end
#endif