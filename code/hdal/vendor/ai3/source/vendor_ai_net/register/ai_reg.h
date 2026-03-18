#ifndef __AI_REG_H__
#define __AI_REG_H__


#define e_AI_NT98690 0xFFF98690


#define AI_CHIP_ID e_AI_NT98690


#if (AI_CHIP_ID == e_AI_NT98690)


	#include "NT98690_NUE2_Reg.h"
	#include "NT98690_CAL_Reg.h"
	#include "NT98690_PPU_Reg.h"

	typedef NT98690_NUE2_REG_STRUCT 	NUE2_REG_TYPE;
	typedef NT98690_CAL_REG_STRUCT 		CAL_REG_TYPE;
	typedef NT98690_PPU_REG_STRUCT 		PPU_REG_TYPE;


#elif (AI_CHIP_ID == e_AI_NT98xxx)

#endif


#endif // !__AI_REG_H__