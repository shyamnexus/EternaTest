
//#include <config.h>

#include "arch.h"
//#include <macro.h>
#include "linkage.h"
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

/**
 * void asm_disable_mmu_el3(void);
 */
ENTRY(asm_disable_mmu_el3)
mrs x0, SCTLR_EL3
bic x0, x0, #SCTLR_M_BIT
msr SCTLR_EL3, x0
isb
ret
ENDPROC(asm_disable_mmu_el3)

/**
 * void asm_disable_icache_el3(void);
 */
ENTRY(asm_disable_icache_el3)
mrs x0, SCTLR_EL3
bic x0, x0, #SCTLR_I_BIT
msr SCTLR_EL3, x0
isb
ret
ENDPROC(asm_disable_icache_el3)

/**
 * void asm_disable_dcache_el3(void);
 */
ENTRY(asm_disable_dcache_el3)
mrs x0, SCTLR_EL3
bic x0, x0, #SCTLR_C_BIT
msr SCTLR_EL3, x0
isb
ret
ENDPROC(asm_disable_dcache_el3)

/*----------------------------------------
 * functions for EL1
  ----------------------------------------*/

ENTRY(enable_caches_el1)
mrs x0, SCTLR_EL1
#if defined(CONFIG_ENABLE_ICACHE)
orr x0, x0, #SCTLR_I_BIT
#endif
#if defined(CONFIG_ENABLE_DCACHE)
orr x0, x0, #SCTLR_C_BIT
#endif
msr     SCTLR_EL1, x0

isb
ret
ENDPROC(enable_caches_el1)

ENTRY(disable_caches_el1)
mrs x0, SCTLR_EL1
and x0, x0, #~SCTLR_I_BIT
and x0, x0, #~SCTLR_C_BIT
msr     SCTLR_EL1, x0

isb
ret
ENDPROC(disable_caches_el1)

/**
 * void asm_disable_mmu_el1(void)
 */
ENTRY(asm_disable_mmu_el1)
mrs x0, SCTLR_EL1
bic x0, x0, #SCTLR_M_BIT
msr SCTLR_EL1, x0
isb
ret
ENDPROC(asm_disable_mmu_el1)

/**
 * void asm_disable_icache_el1(void);
 */
ENTRY(asm_disable_icache_el1)
mrs x0, SCTLR_EL1
bic x0, x0, #SCTLR_I_BIT
msr SCTLR_EL1, x0
isb
ret
ENDPROC(asm_disable_icache_el1)

/**
 * void asm_disable_dcache_el1(void);
 */
ENTRY(asm_disable_dcache_el1)
mrs x0, SCTLR_EL1
bic x0, x0, #SCTLR_C_BIT
msr SCTLR_EL1, x0
isb
ret
ENDPROC(asm_disable_dcache_el1)

ENTRY(asm_invalidate_udcaches)
/* --------------------------------------------------------------
 * don't invalidate until all prior accesses have
 * been observed
 * --------------------------------------------------------------
 */
dmb ish

/* --------------------------------------------------------------
 * get CLIDR_EL1.LoC into w2, and give up if there aren't any
 * --------------------------------------------------------------
 */
mrs     x0, CLIDR_EL1
ubfx    w2, w0, #24, #3
cbz  w2, invalidate_udcaches_end

mov w1, #0                  /* w1 = level iterator */

flush_level:
add  w3, w1, w1, lsl #1     /* w3 = w1 * 3 (right-shift for cache type) */
lsr  w3, w0, w3             /* w3 = w0 >> w3 */
ubfx w3, w3, #0, #3         /* w3 = cache type of this level */
cmp  w3, #2                 /* No cache at this level? */
b.lt next_level

lsl w4, w1, #1
msr CSSELR_EL1, x4          /* Select current cache level in CSSELR */
isb                         /* ISB required to reflect new CSIDR */
mrs x4, CCSIDR_EL1          /* w4 = CSIDR */

ubfx w3, w4, #0, #3
add  w3, w3, #2             /* w3 = log2(line size) */
ubfx w5, w4, #13, #15
ubfx w4, w4, #3, #10        /* w4 = Way number */
clz  w6, w4                 /* w6 = 32 - log2(number of ways) */

flush_set:
mov  w8, w4                 /* w8 = Way number */
flush_way:
lsl  w7, w1, #1             /* Fill level field */
lsl  w9, w5, w3
orr  w7, w7, w9             /* Fill index field */
lsl  w9, w8, w6
orr  w7, w7, w9             /* Fill way field */
dc   cisw, x7               /* Invalidate by set/way to point of coherency */
subs w8, w8, #1             /* Decrement way */
b.ge flush_way
subs w5, w5, #1             /* Decrement set */
b.ge flush_set

next_level:
add  w1, w1, #1             /* Next level */
cmp  w2, w1
b.gt flush_level

/* --------------------------------------------------------------
 * one last barrier, to ensure these operations by set/way are
 * observed before any subsequent explicit memory access
 * --------------------------------------------------------------
 */
invalidate_udcaches_end:
dmb ish
ret
ENDPROC(asm_invalidate_udcaches)

/*
 * void asm_flush_dcache_level(level)
 *
 * clean and invalidate one level cache.
 *
 * x0: cache level
 * x1: 0 flush & invalidate, 1 invalidate only
 * x2~x9: clobbered
 */
ENTRY(asm_flush_dcache_level)
lsl x12, x0, #1
msr csselr_el1, x12     /* select cache level */
isb             /* sync change of cssidr_el1 */
mrs x6, ccsidr_el1      /* read the new cssidr_el1 */
and x2, x6, #7      /* x2 <- log2(cache line size)-4 */
add x2, x2, #4      /* x2 <- log2(cache line size) */
mov x3, #0x3ff
and x3, x3, x6, lsr #3  /* x3 <- max number of #ways */
clz w5, w3          /* bit position of #ways */
mov x4, #0x7fff
and x4, x4, x6, lsr #13 /* x4 <- max number of #sets */
/* x12 <- cache level << 1 */
/* x2 <- line length offset */
/* x3 <- number of cache ways - 1 */
/* x4 <- number of cache sets - 1 */
/* x5 <- bit position of #ways */

loop_set:
mov x6, x3          /* x6 <- working copy of #ways */
loop_way:
lsl x7, x6, x5
orr x9, x12, x7     /* map way and level to cisw value */
lsl x7, x4, x2
orr x9, x9, x7      /* map set number to cisw value */
tbz w1, #0, 1f
dc  isw, x9
b   2f
1:
dc  cisw, x9        /* clean & invalidate by set/way */
2:
subs    x6, x6, #1      /* decrement the way */
b.ge    loop_way
subs    x4, x4, #1      /* decrement the set */
b.ge    loop_set

ret
ENDPROC(asm_flush_dcache_level)

ENTRY(asm_dcache_all)
mov x1, x0
dsb sy
mrs x10, clidr_el1      /* read clidr_el1 */
lsr x11, x10, #24
and x11, x11, #0x7      /* x11 <- loc */
cbz x11, finished       /* if loc is 0, exit */
#mov x15, lr
mov x15, x30
mov x0, #0              /* start flush at cache level 0 */

/* x0  <- cache level */
/* x10 <- clidr_el1 */
/* x11 <- loc */
/* x15 <- return address */
loop_level:
lsl x12, x0, #1
add x12, x12, x0        /* x0 <- tripled cache level */
lsr x12, x10, x12
and x12, x12, #7        /* x12 <- cache type */
cmp x12, #2
b.lt    skip            /* skip if no cache or icache */
bl  asm_flush_dcache_level    /* x1 = 0 flush, 1 invalidate */
skip:
add x0, x0, #1      /* increment cache level */
cmp x11, x0
b.gt    loop_level

mov x0, #0
msr csselr_el1, x0      /* resotre csselr_el1 */
dsb sy
isb
#mov lr, x15
mov x30, x15

finished:
ret
ENDPROC(asm_dcache_all)

/*
 * void asm_flush_dcache_all(int invalidate_only)
 *
 * x0: 0 flush & invalidate, 1 invalidate only
 *
 * clean and invalidate all data cache by SET/WAY.
 */
ENTRY(asm_flush_dcache_all)
#mov x16, lr
mov x16, x30
mov x0, #0
bl  asm_dcache_all
#mov lr, x16
mov x30, x16
ret
ENDPROC(asm_flush_dcache_all)

ENTRY(asm_invalidate_dcache_all)
#mov x16, lr
mov x16, x30
mov x0, #0x1
bl  asm_dcache_all
#mov lr, x16
mov x30, x16
ret
ENDPROC(asm_invalidate_dcache_all)

/*
 * void asm_invalidate_icache_all(void)
 *
 * invalidate all tlb entries.
 */
ENTRY(asm_invalidate_icache_all)
ic  ialluis
isb sy
ret
ENDPROC(asm_invalidate_icache_all)

/*
 * void asm_flush_dcache_range(start, end)
 *
 * clean & invalidate data cache in the range
 *
 * x0: start address
 * x1: end address
 */
ENTRY(asm_flush_dcache_range)
mrs x3, ctr_el0
lsr x3, x3, #16
and x3, x3, #0xf
mov x2, #4
lsl x2, x2, x3      /* cache line size */

/* x2 <- minimal cache line size in cache system */
sub x3, x2, #1
bic x0, x0, x3
1:
dc  civac, x0   /* clean & invalidate data or unified cache */
add x0, x0, x2
cmp x0, x1
b.lo    1b
dsb sy
ret
ENDPROC(asm_flush_dcache_range)

/*
 * void asm_clean_dcache_range(start, end)
 *
 * clean data cache in the range
 *
 * x0: start address
 * x1: end address
 */
ENTRY(asm_clean_dcache_range)
mrs x3, ctr_el0
lsr x3, x3, #16
and x3, x3, #0xf
mov x2, #4
lsl x2, x2, x3      /* cache line size */

/* x2 <- minimal cache line size in cache system */
sub x3, x2, #1
bic x0, x0, x3
1:
dc  cvac, x0   /* clean data or unified cache */
add x0, x0, x2
cmp x0, x1
b.lo    1b
dsb sy
ret
ENDPROC(asm_clean_dcache_range)

/*
 * void asm_invalidate_dcache_range(start, end)
 *
 * invalidate data cache only in the range
 *
 * x0: start address
 * x1: end address
 */
ENTRY(asm_invalidate_dcache_range)
mrs x3, ctr_el0
lsr x3, x3, #16
and x3, x3, #0xf
mov x2, #4
lsl x2, x2, x3      /* cache line size */

/* x2 <- minimal cache line size in cache system */
sub x3, x2, #1
bic x0, x0, x3
1:
dc  ivac, x0     /* invalidate data or unified cache only */
add x0, x0, x2
cmp x0, x1
b.lo    1b
dsb sy
ret
ENDPROC(asm_invalidate_dcache_range)
