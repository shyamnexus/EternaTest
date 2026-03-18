/*
    Library for ide regiseter control

    This is low level control library for ide display.

    @file       ide2_int.c
    @ingroup    mIDrvDisp_IDE
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/

#include "./include/ide_reg.h"
#include "./include/ide2_int.h"

/**
    @addtogroup mIDrvDisp_IDE
*/
//@{
/*
   ide set register

   ide set register

   @param[in] id   ide ID
   @param[in] offset   The register offset
   @param[in] value    The value of specific register offset

   @return void
*/
void idec_set_reg(UINT32 id, IDEDATA_TYPE offset, REGVALUE value)
{
	if (id == LCD_ID_1) {
		LCD_SETREG(offset, value);
	} else if (id == LCD_ID_2) {
		LCD2_SETREG(offset, value);
	} else{
		LCD3_SETREG(offset, value);
	}
}
/*
   ide get register

   ide get register

   @param[in] id   ide ID
   @param[in] offset   The register offset

   @return The value of specific register offset
*/
REGVALUE idec_get_reg(UINT32 id, IDEDATA_TYPE offset)
{
	//printf("get %d\n", (int)offset);
	if (id == LCD_ID_1) {
		return LCD_GETREG(offset);
	} else if (id == LCD_ID_2) {
		return LCD2_GETREG(offset);
	}else{
		return LCD3_GETREG(offset);
	}
}


//@}
