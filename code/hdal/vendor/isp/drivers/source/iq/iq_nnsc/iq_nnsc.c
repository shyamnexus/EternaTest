#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "iq_nnsc.h"

#include "iq_dbg.h"

static IQ_NNSC_INFO_TAB iq_nnsc_info_tab[IQ_ID_MAX_NUM][IQ_NNSC_ITEM_MAX_CNT] = {
	//IQ_ID_1
	{
		{IQ_NNSC_DARK_ENH_RATIO_OFF,             IQ_NNSC_DARK_ENH_RATIO_MAX          },
		{IQ_NNSC_CONTRAST_ENH_RATIO_OFF,         IQ_NNSC_CONTRAST_ENH_RATIO_MAX      },
		{IQ_NNSC_GREEN_ENH_RATIO_OFF,            IQ_NNSC_GREEN_ENH_RATIO_MAX         },
		{IQ_NNSC_SKIN_ENH_RATIO_OFF,             IQ_NNSC_SKIN_ENH_RATIO_MAX          },
	},
	//IQ_ID_2
	{
		{IQ_NNSC_DARK_ENH_RATIO_OFF,             IQ_NNSC_DARK_ENH_RATIO_MAX          },
		{IQ_NNSC_CONTRAST_ENH_RATIO_OFF,         IQ_NNSC_CONTRAST_ENH_RATIO_MAX      },
		{IQ_NNSC_GREEN_ENH_RATIO_OFF,            IQ_NNSC_GREEN_ENH_RATIO_MAX         },
		{IQ_NNSC_SKIN_ENH_RATIO_OFF,             IQ_NNSC_SKIN_ENH_RATIO_MAX          },
	},
	//IQ_ID_3
	{
		{IQ_NNSC_DARK_ENH_RATIO_OFF,             IQ_NNSC_DARK_ENH_RATIO_MAX          },
		{IQ_NNSC_CONTRAST_ENH_RATIO_OFF,         IQ_NNSC_CONTRAST_ENH_RATIO_MAX      },
		{IQ_NNSC_GREEN_ENH_RATIO_OFF,            IQ_NNSC_GREEN_ENH_RATIO_MAX         },
		{IQ_NNSC_SKIN_ENH_RATIO_OFF,             IQ_NNSC_SKIN_ENH_RATIO_MAX          },
	},
	//IQ_ID_4
	{
		{IQ_NNSC_DARK_ENH_RATIO_OFF,             IQ_NNSC_DARK_ENH_RATIO_MAX          },
		{IQ_NNSC_CONTRAST_ENH_RATIO_OFF,         IQ_NNSC_CONTRAST_ENH_RATIO_MAX      },
		{IQ_NNSC_GREEN_ENH_RATIO_OFF,            IQ_NNSC_GREEN_ENH_RATIO_MAX         },
		{IQ_NNSC_SKIN_ENH_RATIO_OFF,             IQ_NNSC_SKIN_ENH_RATIO_MAX          },
	},
	//IQ_ID_5
	{
		{IQ_NNSC_DARK_ENH_RATIO_OFF,             IQ_NNSC_DARK_ENH_RATIO_MAX          },
		{IQ_NNSC_CONTRAST_ENH_RATIO_OFF,         IQ_NNSC_CONTRAST_ENH_RATIO_MAX      },
		{IQ_NNSC_GREEN_ENH_RATIO_OFF,            IQ_NNSC_GREEN_ENH_RATIO_MAX         },
		{IQ_NNSC_SKIN_ENH_RATIO_OFF,             IQ_NNSC_SKIN_ENH_RATIO_MAX          },
	},
	//IQ_ID_6
	{
		{IQ_NNSC_DARK_ENH_RATIO_OFF,             IQ_NNSC_DARK_ENH_RATIO_MAX          },
		{IQ_NNSC_CONTRAST_ENH_RATIO_OFF,         IQ_NNSC_CONTRAST_ENH_RATIO_MAX      },
		{IQ_NNSC_GREEN_ENH_RATIO_OFF,            IQ_NNSC_GREEN_ENH_RATIO_MAX         },
		{IQ_NNSC_SKIN_ENH_RATIO_OFF,             IQ_NNSC_SKIN_ENH_RATIO_MAX          },
	},
	//IQ_ID_7
	{
		{IQ_NNSC_DARK_ENH_RATIO_OFF,             IQ_NNSC_DARK_ENH_RATIO_MAX          },
		{IQ_NNSC_CONTRAST_ENH_RATIO_OFF,         IQ_NNSC_CONTRAST_ENH_RATIO_MAX      },
		{IQ_NNSC_GREEN_ENH_RATIO_OFF,            IQ_NNSC_GREEN_ENH_RATIO_MAX         },
		{IQ_NNSC_SKIN_ENH_RATIO_OFF,             IQ_NNSC_SKIN_ENH_RATIO_MAX          },
	},
	//IQ_ID_8
	{
		{IQ_NNSC_DARK_ENH_RATIO_OFF,             IQ_NNSC_DARK_ENH_RATIO_MAX          },
		{IQ_NNSC_CONTRAST_ENH_RATIO_OFF,         IQ_NNSC_CONTRAST_ENH_RATIO_MAX      },
		{IQ_NNSC_GREEN_ENH_RATIO_OFF,            IQ_NNSC_GREEN_ENH_RATIO_MAX         },
		{IQ_NNSC_SKIN_ENH_RATIO_OFF,             IQ_NNSC_SKIN_ENH_RATIO_MAX          },
	},
};

ER iq_nnsc_set_info(IQ_ID id, IQ_NNSC_ITEM index, UINT32 value)
{
	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("Error iq id = 0x%.8x\r\n", id);
		return E_SYS;
	}

	if (index >= IQ_NNSC_ITEM_MAX_CNT) {
		DBG_ERR("id: %d index = 0x%.8x over than max count\r\n", id, index);
		return E_SYS;
	}

	if (value >= iq_nnsc_info_tab[id][index].max_value) {
		DBG_ERR("id: %d, index:0x%x, Value = %d >= Max = %d\r\n", id, index, value, iq_nnsc_info_tab[id][index].max_value);
		return E_SYS;
	}

	iq_nnsc_info_tab[id][index].value = value;

	return E_OK;
}

UINT32 iq_nnsc_get_info(IQ_ID id, IQ_NNSC_ITEM index)
{
	if (id >= IQ_ID_MAX_NUM) {
		DBG_ERR("Error iq id = 0x%.8x\r\n", id);
		return IQ_NNSC_INFOR_ERR;
	}

	if (index >= IQ_NNSC_ITEM_MAX_CNT) {
		DBG_ERR("id: %d index = 0x%.8x over than max count\r\n", id, index);
		return IQ_NNSC_INFOR_ERR;
	}

	return iq_nnsc_info_tab[id][index].value;
}
