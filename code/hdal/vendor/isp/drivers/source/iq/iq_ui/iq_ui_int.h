#ifndef _IQ_UI_INT_H_
#define _IQ_UI_INT_H_

#include "iq_alg.h"
#include "iqt_api.h"
#include "iq_ui.h"

extern ER iq_ui_init(void);
extern ER iq_ui_set_info(IQ_ID id, IQ_UI_ITEM index, INT32 value);
extern INT32 iq_ui_get_info(IQ_ID id, IQ_UI_ITEM index);

#endif

