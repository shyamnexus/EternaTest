#ifndef _NN_VERINFO_H_
#define _NN_VERINFO_H_

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

typedef enum {
	NN_CHIP_CNN20   = 0,         ///< chip CNN 313 
	NN_CHIP_CNN25_A = 1,         ///< chip CNN 520 
	NN_CHIP_CNN25_B = 2,         ///< chip CNN 321 
	NN_CHIP_CNN25_C = 3,         ///< chip CNN 528 
	NN_CHIP_CNN25_D = 4,         ///< chip CNN 560
	NN_CHIP_CNN25_E = 5,         ///< chip CNN 336 64bit
    NN_CHIP_CNN25_F = 6,         ///< chip CNN 530 64bit
    NN_CHIP_CNN25_G = 7,         ///< chip CNN 331 32bit
    NN_CHIP_CNN25_H = 8,         ///< chip CNN 331 64bit
    NN_CHIP_CNN30_A = 30,        ///< chip CNN 690 64bit
    NN_CHIP_CNN30_B = 31,        ///< chip CNN 538 64bit
    NN_CHIP_CNN30_C = 32,        ///< chip CNN 539A_64bits
	NN_CHIP_CNN30_D = 33,        ///< chip CNN 635 64bit
	ENUM_DUMMY4WORD(NN_CHIP_ID)
} NN_CHIP_ID;

typedef enum { 
	NN_CHIP_AI1 = 0,         //v2
	NN_CHIP_AI2 = 1,         //v3
	ENUM_DUMMY4WORD(NN_CHIP_FMT)
} NN_CHIP_FMT;

typedef enum {
	NN_CHIP_AI_SUBVER0 = 0,         //vx.0
	NN_CHIP_AI_SUBVER1 = 1,         //vx.1
	NN_CHIP_AI_SUBVER2 = 2,         //vx.2
	NN_CHIP_AI_SUBVER3 = 4,         //vx.3
	ENUM_DUMMY4WORD(NN_CHIP_FMT_SUBVER)
} NN_CHIP_FMT_SUBVER;



#define NN_VERSION_ENTRY(_major, _minor, _bugfix, _branch)  ((_major << 24) + (_minor << 16) + (_bugfix << 8) + _branch)
#define NN_GEN_VERSION			NN_VERSION_ENTRY(0x15, 0x0, 0x0, 0x0)
#define NN_CHIP                 NN_VERSION_ENTRY(NN_CHIP_AI_SUBVER1, NN_CHIP_AI2, 0x0, NN_CHIP_CNN25_A)

#endif
