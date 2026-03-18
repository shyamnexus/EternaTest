/**
	@brief Header file of internal definition of kdrv_ai.

	@file ai_emu.h

	@ingroup ai_emu

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _AI_EMU_H_
#define _AI_EMU_H_

#define KDRV_AI_FUNC_TEST 0

/********************************************************************
	INCLUDE FILES
********************************************************************/
typedef enum {
	AI_RUN_OK,
	AI_RUN_FAIL,
	AI_RUN_SKIP,
	ENUM_DUMMY4WORD(KDRV_AI_TEST_RESULT)
} KDRV_AI_TEST_RESULT;
/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
#if KDRV_AI_FUNC_TEST
extern int emu_ai(char **pargv);
#endif

#endif  /* _AI_EMU_H_ */
