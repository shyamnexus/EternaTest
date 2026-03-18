#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "iq_ui.h"

#include "iq_dbg.h"

static IQ_UI_INFO_TAB iq_ui_info_tab[IQ_ID_MAX_NUM][IQ_UI_ITEM_MAX_CNT];

ER iq_ui_init(void)
{
	UINT32 i;

	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		iq_ui_info_tab[i][IQ_UI_ITEM_NR_LV].value             = IQ_UI_NR_LV_NORMAL;
		iq_ui_info_tab[i][IQ_UI_ITEM_NR_LV].min_value         = IQ_UI_NR_LV_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_NR_LV].max_value         = IQ_UI_NR_LV_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_3DNR_LV].value           = IQ_UI_3DNR_LV_NORMAL;
		iq_ui_info_tab[i][IQ_UI_ITEM_3DNR_LV].min_value       = IQ_UI_3DNR_LV_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_3DNR_LV].max_value       = IQ_UI_3DNR_LV_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_SHARPNESS_LV].value      = IQ_UI_SHARPNESS_LV_NORMAL;
		iq_ui_info_tab[i][IQ_UI_ITEM_SHARPNESS_LV].min_value  = IQ_UI_SHARPNESS_LV_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_SHARPNESS_LV].max_value  = IQ_UI_SHARPNESS_LV_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_SATURATION_LV].value     = IQ_UI_SATURATION_LV_NORMAL;
		iq_ui_info_tab[i][IQ_UI_ITEM_SATURATION_LV].min_value = IQ_UI_SATURATION_LV_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_SATURATION_LV].max_value = IQ_UI_SATURATION_LV_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_CONTRAST_LV].value       = IQ_UI_CONTRAST_LV_NORMAL;
		iq_ui_info_tab[i][IQ_UI_ITEM_CONTRAST_LV].min_value   = IQ_UI_CONTRAST_LV_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_CONTRAST_LV].max_value   = IQ_UI_CONTRAST_LV_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_BRIGHTNESS_LV].value     = IQ_UI_BRIGHTNESS_LV_NORMAL;
		iq_ui_info_tab[i][IQ_UI_ITEM_BRIGHTNESS_LV].min_value = IQ_UI_BRIGHTNESS_LV_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_BRIGHTNESS_LV].max_value = IQ_UI_BRIGHTNESS_LV_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_NIGHT_MODE].value        = IQ_UI_NIGHT_MODE_OFF;
		iq_ui_info_tab[i][IQ_UI_ITEM_NIGHT_MODE].min_value    = IQ_UI_NIGHT_MODE_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_NIGHT_MODE].max_value    = IQ_UI_NIGHT_MODE_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_YCC_FORMAT].value        = IQ_UI_YCC_OUT_FULL;
		iq_ui_info_tab[i][IQ_UI_ITEM_YCC_FORMAT].min_value    = IQ_UI_YCC_OUT_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_YCC_FORMAT].max_value    = IQ_UI_YCC_OUT_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_OPERATION].value         = IQ_UI_OPERATION_MOVIE;
		iq_ui_info_tab[i][IQ_UI_ITEM_OPERATION].min_value     = IQ_UI_OPERATION_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_OPERATION].max_value     = IQ_UI_OPERATION_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_IMAGEEFFECT].value       = IQ_UI_IMAGEEFFECT_OFF;
		iq_ui_info_tab[i][IQ_UI_ITEM_IMAGEEFFECT].min_value   = IQ_UI_IMAGEEFFECT_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_IMAGEEFFECT].max_value   = IQ_UI_IMAGEEFFECT_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_CCID].value              = IQ_UI_CCID_NO_EFFECT;
		iq_ui_info_tab[i][IQ_UI_ITEM_CCID].min_value          = IQ_UI_CCID_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_CCID].max_value          = IQ_UI_CCID_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_HUE_SHIFT].value         = IQ_UI_HUE_SHIFT_0;
		iq_ui_info_tab[i][IQ_UI_ITEM_HUE_SHIFT].min_value     = IQ_UI_HUE_SHIFT_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_HUE_SHIFT].max_value     = IQ_UI_HUE_SHIFT_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_TONE_LV].value           = IQ_UI_TONE_LV_NORMAL;
		iq_ui_info_tab[i][IQ_UI_ITEM_TONE_LV].min_value       = IQ_UI_TONE_LV_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_TONE_LV].max_value       = IQ_UI_TONE_LV_MAX_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_GAMMA_LV].value          = IQ_UI_GAMMA_LV_NORMAL;
		iq_ui_info_tab[i][IQ_UI_ITEM_GAMMA_LV].min_value      = IQ_UI_GAMMA_LV_MIN_CNT;
		iq_ui_info_tab[i][IQ_UI_ITEM_GAMMA_LV].max_value      = IQ_UI_GAMMA_LV_MAX_CNT;
	}

	return E_OK;
}

ER iq_ui_set_info(IQ_ID id, IQ_UI_ITEM index, INT32 value)
{
	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("Error iq id = 0x%.8x\r\n", id);
		return E_SYS;
	}

	if (index >= IQ_UI_ITEM_MAX_CNT) {
		DBG_ERR("id: %d index = 0x%.8x over than max count\r\n", id, index);
		return E_SYS;
	}

	if (value <= iq_ui_info_tab[id][index].min_value) {
		DBG_ERR("id: %d, index:0x%x, Value = %d <= Min = %d\r\n", id, index, value, iq_ui_info_tab[id][index].min_value);
		return E_SYS;
	}
	if (value >= iq_ui_info_tab[id][index].max_value) {
		DBG_ERR("id: %d, index:0x%x, Value = %d >= Max = %d\r\n", id, index, value, iq_ui_info_tab[id][index].max_value);
		return E_SYS;
	}

	iq_ui_info_tab[id][index].value = value;

	return E_OK;
}

INT32 iq_ui_get_info(IQ_ID id, IQ_UI_ITEM index)
{
	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("Error iq id = 0x%.8x\r\n", id);
		return IQ_UI_INFOR_ERR;
	}

	if (index >= IQ_UI_ITEM_MAX_CNT) {
		DBG_ERR("id: %d index = 0x%.8x over than max count\r\n", id, index);
		return IQ_UI_INFOR_ERR;
	}

	return iq_ui_info_tab[id][index].value;
}
