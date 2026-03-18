#ifndef __ARM_CHACHE_H__
#define __ARM_CHACHE_H__

#define LEVEL_1 (1 - 1)
#define LEVEL_2 (2 - 1)

/*
     csselrcache level & type selection

     Detailed reference to [trm p4-177 CSSELR]
*/
typedef enum _CACHE_LV_TYPE_ {
	LEVEL_1_DCACHE = 0x0,   //< Level 1 data cache
	LEVEL_1_ICACHE,         //< Level 1 instruction cache
	LEVEL_2_DCACHE,         //< Level 2 data cache

	ENUM_DUMMY4WORD(CACHE_LV_TYPE)
} CACHE_LV_TYPE;

/*
     Values for Ctype fields in CLIDR
*/
typedef enum _CLIDR_CTYPE_ {
	CLIDR_CTYPE_NO_CACHE        = 0x0,
	CLIDR_CTYPE_INSTRUCTION_ONLY = 0x1,
	CLIDR_CTYPE_DATA_ONLY       = 0x2,
	CLIDR_CTYPE_INSTRUCTION_DATA = 0x3,
	CLIDR_CTYPE_UNIFIED         = 0x4,
	ENUM_DUMMY4WORD(CLIDR_CTYPE)
} CLIDR_CTYPE;

//Instruction Synchronization Barrier.
#define _ISB() \
	__asm__ __volatile__("isb sy\n\t")

//Data Synchronization Barrier
#define _DSB() \
	__asm__ __volatile__("dsb sy\n\t")

#define CLIDR() \
	({ \
		unsigned long val; \
		__asm__ __volatile__( \
							  "mrs %0, CLIDR_EL1\n\t" \
							  : "=r" (val)); \
		val; \
	})

#define CCSIDR() \
	({ \
		unsigned long val; \
		__asm__ __volatile__( \
							  "mrs %0, CCSIDR_EL1\n\t" \
							  : "=r" (val)); \
		val; \
	})

#define sel_CSSELR(InD) \
	__asm__ __volatile__(   \
							"msr CSSELR_EL1, %0\n\t" \
							: \
							: "r"(InD));

#define CTR() \
	({ \
		unsigned int val; \
		__asm__ __volatile__( \
							  "mrs %0, CTR_EL0\n\t" \
							  : "=r" (val)); \
		val; \
	})

#define read_CBAR() \
	({ \
		unsigned long cfg; \
		__asm__ __volatile__(\
							 "mrs %0, S3_1_C15_C3_0\n\t" \
							 : "=r"(cfg) \
							);\
		cfg;\
	})
#define _ICACHE_INV_ALL() _ICIALLU()

/* IC IALLU: Instruction Cache Invalidate All to PoU */
#define _ICIALLU() \
	__asm__ __volatile__("IC IALLU\n\t");

#define _ICACHE_INV_MVAU(addr) _ICIMVAU(addr)

/* IC IMVAU = IC IVAU: Instruction Cache line Invalidate by VA to PoU */
#define _ICIMVAU(addr) \
	__asm__ __volatile__(   \
							"IC IVAU, %0\n\t"   \
							: \
							: "r"(addr));

#define _DCACHE_INV_MVAC(addr) _DCIMVAC(addr)

/* DC IMVAC = DC IVAC: Data or unified Cache line Invalidate by VA to PoC */
#define _DCIMVAC(addr) \
	__asm__ __volatile__(   \
							"DC IVAC, %0\n\t"   \
							: \
							: "r"(addr));

#define _DCACHE_WBACK_MVAC(addr) _DCCMVAC(addr)

/* DC CMVAC = DC CVAC: Data or unified Cache line Clean by VA to PoC */
#define _DCCMVAC(addr) \
	__asm__ __volatile__(   \
							"DC CVAC, %0\n\t"   \
							: \
							: "r"(addr));

#define _DCACHE_WBACK_INV_MVAC(addr) _DCCIMVAC(addr)

/* DC CIMVAC = DC CIVAC: Data or unified Cache line Clean and Invalidate by VA to PoC */
#define _DCCIMVAC(addr) \
	__asm__ __volatile__(   \
							"DC CIVAC, %0\n\t"  \
							: \
							: "r"(addr));

/* DC CISW: Data or unified Cache line Clean and Invalidate by Set/Way */
#define _DCCISW(way_set) \
	__asm__ __volatile__(   \
							"DC CISW, %0\n\t"   \
							:   \
							: "r"(way_set));

/* DC ISW: Data or unified Cache line Invalidate by Set/Way */
#define _DCISW(way_set) \
	__asm__ __volatile__(   \
							"DC ISW, %0\n\t"    \
							:   \
							: "r"(way_set));


#endif /* __ARM_CHACHE_H__ */

