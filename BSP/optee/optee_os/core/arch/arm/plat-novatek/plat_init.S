/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
 */

#include <arm64.h>
#include <asm.S>

#define CPUECTLR_A53_SMPEN	BIT(6)

#ifndef ALIGN
#define ALIGN .align		4
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

/*
 * a53 aarch32 configuration early configuration
 */
#ifdef CFG_CA53_ARMV7
FUNC plat_cpu_reset_early , :
UNWIND(	.fnstart)
	/*
	 * Write the CPU Extended Control Register
	 * Set the SMPEN bit, this Cortex-A53 core's register
	 */
	mrrc	p15, 1, r0, r1, c15
	orr	r0, r0, #CPUECTLR_A53_SMPEN
	mcrr	p15, 1, r0, r1, c15
	bx	lr
UNWIND(	.fnend)
END_FUNC plat_cpu_reset_early
#endif
#ifdef CFG_ARM64_core
#if defined(_BSP_NA51102_) || defined(_BSP_NA51103_) || defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_)
//ARM64's NA51102 & NA51103 & NS02201 & NS02302
ENTRY(local_load_addr)
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
ENDPROC(local_load_addr)

ENTRY(local_store_addr)
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
ENDPROC(local_store_addr)
#else
//ARM64 's others (!=NA51102 & !=NA51103 & !=NS02201 & !=NS02302)
ENTRY(local_load_addr)
    mov	    w2, w1                          //w2 = efuse_base_addr
    mov	    w3, #0x1                        //w3 = 0x1 (set up en)
    str	    wzr, [x2]                       //efuse_base_addr = 0x0 (config_reg)

    add	    w2, w1, #0x20                   //w2 = efuse_base + 0x20 (control reg)
    str	    w3, [x2]                        //efuse_base + 0x20 = 0x1 (set up en)


    add	    w3, w1, #0x30                   //w3 = efuse_base + 0x30 (status reg)
POLL_STATUS:
    ldr	    w4, [x3]                        //w4 = status reg
    tbz	    w4, #0, POLL_STATUS             //polling 0x30 bit[1] = 1 -> break;


    add	    w4, w1, #0x4                    //w4 = address reg 0x4
    and	    w0, w0, #0x1f                   //get row address
    str	    w0, [x4]                        //0x04 = address

    ldr	    w0, [x2]                        //w0 = 0x20
    orr	    w0, w0, #0x10                   //orr 0x10 = op_en
    str	    w0, [x2]                        //operation enable
POLL_OP:
    tbnz	w0, #4, POLL_OP_DONE
    add	    w1, w1, #0x8                    //w1 = base addr + 0x8 => data out

    ldr	    w0, [x1]                        //w0 = data
    str	    wzr, [x2]                       //set en = 0

POLL_SET_EN_DONE:
    ldr	    w1, [x3]
    tbnz	w1, #0, POLL_SET_EN_DONE

    str	    w1, [x3]
    ret

POLL_OP_DONE:
	ldr	    w0, [x2]
	b       POLL_OP
ENDPROC(local_load_addr)

ENTRY(local_store_addr)
    mov	    w2, w1                          //w2 = efuse_base_addr
    mov	    w3, #0x4                        //write operation
    str	    w3, [x2]                        //config reg(0x0) = 0x4 (write op)

    add	    w2, w1, #0x20                   //w2 = efuse_base + 0x20 (control reg)
    mov	    w3, #0x1                   	    //w3 = 1
    str	    w3, [x2]                        //efuse_base + 0x20 = 0x1 (set up en)

    add	    w3, w1, #0x30                   //w3 = efuse_base + 0x30 (status reg)

POLL_STATUS_W:
    ldr	    w4, [x3]
    tbz	    w4, #0, POLL_STATUS_W

    add	    w1, w1, #0x4                    //w1 = 0x4 (address)
    str	    w0, [x1]                        //0x04 = address

    ldr	    w0, [x2]                        //x2 = 0x20
    orr	    w0, w0, #0x10                   //operation enable
    str	    w0, [x2]

POLL_OP_W:
    tbnz	w0, #4, POLL_OP_DONE_W

    str	    wzr, [x2]

POLL_SET_EN_DONE_W:
    ldr	    w0, [x3]
    tbnz	w0, #0, POLL_SET_EN_DONE_W
    str	    w0, [x3]
    mov	    w0, #0x0
    ret

POLL_OP_DONE_W:
	ldr	    w0, [x2]

	b       POLL_OP_W
ENDPROC(local_store_addr)
#endif //NA51102 & NA51103 & NS02201's MACRO define
#else
//ARM32 MACRO define
#if defined(_BSP_NA51103_) || defined(_BSP_NS02301_)
ENTRY(local_load_addr)
    STMFD   r13!, {r4-r8, r14}
    MOV	  r4, r0
    MOV   r2, r1

    @ ==========================================================================
    @ Check if address < 4 (only allow read address 0~3)
    @ ==========================================================================
    MOV   r0, #-33				        @r0 = E_PAR

	MOV   r8, #0	                	@param != this No. break

    MOV   r5, #0x1F
    AND   r5, r4, r5
    CMP   r8, r5

    BGT   END_PROG_LOAD_ADDR

    @ ==========================================================================
    @ each key set with 4 fields
    @ ==========================================================================
    MOV   r0, r2

    LDR   r2, [r0, #0x30]
    STR   r2, [r0, #0x30]


    MOV   r1, #0x01<<0
    STR   r1, [r0, #0x20]       @ R1 = EFUSE_SETUP_EN
1:
    LDR   r1, [r0, #0x30]
    UBFX  r1, r1, #0, #1		@ R1 = EFUSE_SETUP_EN
    CMP   r1, #0x01
    BNE   1b

    @ ======================================================================== @
    @ Start to read address n                                                  @
    @ ======================================================================== @
    MOV   r1, #0x0				@ eFuse index from = 16 (chip ID efuse data)
    ADD   r1, r1, r4	        @ r1 = r1 + (r4) now r1 is address to read

    MOV   r5, #0x0
    STR   r5, [r0, #0x00]

    STR   r1, [r0, #0x04]
    LDR   r2, [r0, #0x20]
    ORR   r2, r2, #0x01<<4		@ OPERATION_EN
    STR   r2, [r0, #0x20]
2:
    LDR   r2, [r0, #0x20]
    UBFX  r2, r2, #4, #1        @ R2 = OPERATION_EN, polling until bit[4] = 0
    CMP   r2, #0x01
    BEQ   2b


3:
    LDR   r2, [r0, #0x30]
    UBFX  r2, r2, #1, #1		@ R2 = OPERATION_EN
    CMP   r2, #0x01
    BNE   3b

    LDR   r2, [r0, #0x30]
    STR	  r2, [r0, #0x30]


    MOV   r2, #0
    STR   r2, [r0, #0x20]

    LDR   r0, [r0, #0x8]

END_PROG_LOAD_ADDR:
    LDMFD   r13!, {r4-r8, pc}   //If r7 !=1 than return
    nop
ENDPROC(local_load_addr)

ENTRY(local_store_addr)
    STMFD   r13!, {r4-r8, r14}		@ Only allow program efuse once if column address >= 16
    MOV	  r4, r0
    MOV   r2, r1

    MOV   r0, #-33					@r0 = E_PAR

    MOV   r8, #0                    @addr

    MOV   r5, #0x1F
    AND   r5, r4, r5
    CMP   r8, r5
	BGT   END_PROG_STORE_ADDR
    @ ==========================================================================
    @ now r0 = 0xF0660000
    @ ==========================================================================
    MOV   r0, r2

    LDR   r2, [r0, #0x30]
    STR   r2, [r0, #0x30]

    //Do not issue SETUP_EN
#if (0)
    MOV   r1, #0x01<<0
    STR   r1, [r0, #0x20]
1:
    LDR   r1, [r0, #0x30]
    UBFX  r1, r1, #0, #1		    @ R1 = EFUSE_SETUP_EN
    CMP   r1, #0x01
	BNE   1b
#endif
    @ ======================================================================== @
    @ Start to program address n                                               @
    @ ======================================================================== @
    MOV   r1, #0x0				    @ eFuse index from = 16 (chip ID efuse data)
	ADD   r1, r1, r4	            @ r1 = r1 + (r4) now r1 is address to read

    MOV   r5, #0x4
    STR   r5, [r0, #0x00]           @ config as program mode


    STR   r1, [r0, #0x04]
    LDR   r2, [r0, #0x20]
    ORR   r2, r2, #0x01<<4		    @ OPERATION_EN
    STR   r2, [r0, #0x20]
2:
    LDR   r2, [r0, #0x20]
    UBFX  r2, r2, #4, #1            @ R2 = OPERATION_EN polling bit[4] until 0
    CMP   r2, #0x01
    BEQ   2b

3:
    LDR   r2, [r0, #0x30]
    UBFX  r2, r2, #1, #1		@ R2 = OPERATION_EN
    CMP   r2, #0x01
    BNE   3b

    LDR   r2, [r0, #0x30]
    STR	  r2, [r0, #0x30]

    MOV   r2, #0
    //Do not issue SETUP_EN
#if (0)
    STR   r2, [r0, #0x20]
#endif
    STR   r2, [r0, #0x04]

    MOV   r0, #0

END_PROG_STORE_ADDR:
    LDMFD   r13!, {r4-r8, pc}   	//If r7 !=1 than return
	nop
ENDPROC(local_store_addr)
#elif defined(_BSP_NA51089_) || defined(_BSP_NA51055_)
ENTRY(local_load_addr)
    STMFD   r13!, {r4-r8, r14}
    MOV	  r4, r0
    MOV   r2, r1

    @ ==========================================================================
    @ Check if address < 4 (only allow read address 0~3)
    @ ==========================================================================
    MOV   r0, #-33				        @r0 = E_PAR

	MOV   r8, #8	                	@param != this No. break
#   MOV   r8, #31		                @param > 31. break(all address can be read)

    MOV   r5, #0x1F
    AND   r5, r4, r5
    CMP   r8, r5

    BGT   END_PROG_LOAD_ADDR

    @ ==========================================================================
    @ each key set with 4 fields
    @ ==========================================================================
    MOV   r0, r2

    MOV   r1, #0x01<<0
    STR   r1, [r0, #0x20]       @ R1 = EFUSE_SETUP_EN
1:
    LDR   r1, [r0, #0x30]
    UBFX  r1, r1, #0, #1		@ R1 = EFUSE_SETUP_EN
    CMP   r1, #0x01
    BNE   1b

    @ ======================================================================== @
    @ Start to read address n                                                  @
    @ ======================================================================== @
    MOV   r1, #0x0				@ eFuse index from = 16 (chip ID efuse data)
    ADD   r1, r1, r4	        @ r1 = r1 + (r4) now r1 is address to read

    MOV   r5, #0x0
    STR   r5, [r0, #0x00]

    STR   r1, [r0, #0x04]
    LDR   r2, [r0, #0x20]
    ORR   r2, r2, #0x01<<4		@ OPERATION_EN
    STR   r2, [r0, #0x20]
2:
    LDR   r2, [r0, #0x20]
    UBFX  r2, r2, #4, #1        @ R2 = OPERATION_EN, polling until bit[4] = 0
    CMP   r2, #0x01
    BEQ   2b


3:
    LDR   r2, [r0, #0x30]
    UBFX  r2, r2, #1, #1		@ R2 = OPERATION_EN
    CMP   r2, #0x01
    BNE   3b

    LDR   r2, [r0, #0x30]
    STR	  r2, [r0, #0x30]


    MOV   r2, #0
    STR   r2, [r0, #0x20]

    LDR   r0, [r0, #0x8]

END_PROG_LOAD_ADDR:
    LDMFD   r13!, {r4-r8, pc}   //If r7 !=1 than return
    nop
ENDPROC(local_load_addr)
    
ENTRY(local_store_addr)
    STMFD   r13!, {r4-r8, r14}		@ Only allow program efuse once if column address >= 16
    MOV	  r4, r0
    MOV   r2, r1

    MOV   r0, #-33					@r0 = E_PAR

    MOV   r8, #8                    @addr

    MOV   r5, #0x1F
    AND   r5, r4, r5
    CMP   r8, r5
	BGT   END_PROG_STORE_ADDR
    @ ==========================================================================
    @ now r0 = 0xF0660000
    @ ==========================================================================
    MOV   r0, r2

    MOV   r1, #0x01<<0
    STR   r1, [r0, #0x20]
1:
    LDR   r1, [r0, #0x30]
    UBFX  r1, r1, #0, #1		    @ R1 = EFUSE_SETUP_EN
    CMP   r1, #0x01
	BNE   1b

    @ ======================================================================== @
    @ Start to program address n                                               @
    @ ======================================================================== @
    MOV   r1, #0x0				    @ eFuse index from = 16 (chip ID efuse data)
	ADD   r1, r1, r4	            @ r1 = r1 + (r4) now r1 is address to read

    MOV   r5, #0x4
    STR   r5, [r0, #0x00]           @ config as program mode


    STR   r1, [r0, #0x04]
    LDR   r2, [r0, #0x20]
    ORR   r2, r2, #0x01<<4		    @ OPERATION_EN
    STR   r2, [r0, #0x20]
2:
    LDR   r2, [r0, #0x20]
    UBFX  r2, r2, #4, #1            @ R2 = OPERATION_EN polling bit[4] until 0
    CMP   r2, #0x01
    BEQ   2b

    LDR   r2, [r0, #0x30]
    STR	  r2, [r0, #0x30]


    MOV   r2, #0
    STR   r2, [r0, #0x20]
    STR   r2, [r0, #0x04]

    MOV   r0, #0

END_PROG_STORE_ADDR:
    LDMFD   r13!, {r4-r8, pc}   	//If r7 !=1 than return
	nop
ENDPROC(local_store_addr)
#endif
#endif
