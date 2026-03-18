	.text
	.global core2_exec
	.extern core2_cb
	.type core2_exec, %function
	.align 4

core2_exec:
#ldr     r0, =0xF0290000
#ldr     r1, ='@'
#str     r1, [r0]
	ldr     sp, =0x01800000
	bl      core2_cb

