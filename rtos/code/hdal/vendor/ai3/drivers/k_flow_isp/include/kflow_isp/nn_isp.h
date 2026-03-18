/**
    @brief Header file of NN_ISP definition of vendor net flow sample.

    @file nn_isp.h

    @ingroup net_flow_sample

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _NN_ISP_H_
#define _NN_ISP_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "kwrap/type.h"


/********************************************************************
    PATH_LIST_CMD
********************************************************************/

typedef struct _PATH_LIST_TRIG_CMD {
    unsigned mode: 4;
    unsigned func_sel: 4;
    unsigned flag_id: 8;
    unsigned : 8;
    unsigned : 8;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 4;
    unsigned func_idx : 12;
    unsigned : 8;
	unsigned addr_type : 4;
} __attribute__((packed)) PATH_LIST_TRIG_CMD ;

typedef struct _PATH_LIST_WAIT_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned flag_id: 8;
    unsigned : 16;
} __attribute__((packed)) PATH_LIST_WAIT_CMD ;

typedef struct _PATH_LIST_NULL_CMD {
    unsigned mode: 4;
    unsigned : 28;
} __attribute__((packed)) PATH_LIST_NULL_CMD ;

typedef struct _PATH_LIST_UPD_CMD {
    unsigned mode: 4;
    unsigned byte_en: 4;
    unsigned reg_ofs: 12;
    unsigned : 12;
    unsigned value : 32;
} __attribute__((packed)) PATH_LIST_UPD_CMD ;

typedef struct _PATH_LIST_JUMP_CMD {
    unsigned mode: 4;
    unsigned : 12;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 8;
	unsigned addr_type : 4;
} __attribute__((packed)) PATH_LIST_JUMP_CMD;

typedef struct _PATH_LIST_CONDITION_JUMP_CMD {
    unsigned mode: 4;
    unsigned reg_ofs: 12;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 4;
    unsigned byte_en: 4;
	unsigned addr_type : 4;
    unsigned condi_val: 32;
} __attribute__((packed)) PATH_LIST_CONDITION_JUMP_CMD;

typedef struct _PATH_LIST_MOV_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned src_reg_ofs: 12;
	unsigned dst_reg_ofs: 12;
} __attribute__((packed)) PATH_LIST_MOV_CMD;

typedef struct _PATH_LIST_LOGIC_WAIT_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned op: 4;
    unsigned : 20;
    unsigned long long flag_status: 64;
} __attribute__((packed)) PATH_LIST_LOGIC_WAIT_CMD;

typedef struct _PATH_LIST_ALU_CMD {
    unsigned mode: 4;
    unsigned : 4;
    unsigned op: 4;
    unsigned : 4;
    unsigned reg_ofs: 12;
    unsigned : 4;
	unsigned value: 32;
} __attribute__((packed)) PATH_LIST_ALU_CMD;

typedef struct _LINK_LIST_JUMP_CMD {
    unsigned mode: 4;
    unsigned : 12;
    unsigned address : 32;
    unsigned msb_addr : 4;
    unsigned : 8;
	unsigned addr_type : 4;
} __attribute__((packed)) LINK_LIST_JUMP_CMD;

typedef union {
	UINT64 reg;
	struct {
		unsigned : 32;
		unsigned addr : 32;
	} bit;
} ENG_ADDR_REG;

typedef struct _CONV_LL_PARM_SMALL {
    ENG_ADDR_REG input;
	ENG_ADDR_REG input_msb;
} CONV_LL_PARM_SMALL;

typedef enum {
    NN_GEN_ENG_UNKNOWN = -1,
    NN_GEN_ENG_CONV,
    NN_GEN_ENG_UTIL,
    NN_GEN_ENG_CAL,
    NN_GEN_ENG_LSU,
    NN_GEN_ENG_ROU,
    NN_GEN_ENG_NUE2,
    NN_GEN_ENG_PPU,
    NN_GEN_ENG_CPU,
    NN_GEN_ENG_DSP,
    NN_GEN_ENG_POU,
    NN_GEN_ENG_EXT = 0xFFFF,     // for general backend
    ENUM_DUMMY4WORD(NN_GEN_ENG_TYPE),
} NN_GEN_ENG_TYPE;

#define NN_LL_CMD_GET_MODE(cmd)        (0x000000000000000f & cmd)
#define NN_JOBLL_CMD_GET_MODE(cmd)     (0x000000000000000f & cmd)
#define GET_MSB(v)          (((v) & 0xFFFFFFFF00000000) >> 32)
#define GET_LSB(v)          (((v) & 0x00000000FFFFFFFF))
#endif  /* NN_ISP */