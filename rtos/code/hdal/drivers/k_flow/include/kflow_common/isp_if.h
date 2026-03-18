#ifndef _ISP_IF_API_
#define _ISP_IF_API_

#if defined __UITRON || defined __ECOS
#include "Type.h"
#include "mIType.h"
#else
#include "kwrap/type.h"
#include "kwrap/nvt_type.h"
#include "kdrv_type.h"
#include "kwrap/perf.h"
#endif

typedef enum _ISP_ID {
	ISP_ID_1 = 0,
	ISP_ID_2,
	ISP_ID_3,
	ISP_ID_4,
	ISP_ID_5,
	ISP_ID_6,
	ISP_ID_7,
	ISP_ID_8,
	ISP_ID_9,
	ISP_ID_MAX_NUM,
	ISP_ID_IGNORE = 0xffffffff,
} ISP_ID;

typedef enum _ISP_FUNC_EN {
	ISP_FUNC_EN_AE      = 0x00000001,
	ISP_FUNC_EN_AWB     = 0x00000002,
	ISP_FUNC_EN_AF      = 0x00000004,
	ISP_FUNC_EN_WDR     = 0x00000008,
	ISP_FUNC_EN_SHDR	= 0x00000010,
	ISP_FUNC_EN_DEFOG   = 0x00000020,
	ISP_FUNC_EN_DIS   	= 0x00000040,
	ISP_FUNC_EN_RSC  	= 0x00000080,
	ISP_FUNC_EN_ETH  	= 0x00000100,
	ISP_FUNC_EN_GDC		= 0x00000200,
	ISP_FUNC_EN_3DNR	= 0x00000400,
	ISP_FUNC_EN_COMBINE	= 0x00000800,
	ISP_FUNC_EN_NN_IFE_0= 0x00001000,
	ISP_FUNC_EN_NN_IFE_1= 0x00002000,
	ISP_FUNC_EN_NN_IME_0= 0x00004000,
	ISP_FUNC_EN_NN_IME_1= 0x00008000,
	ISP_FUNC_EN_THERMAL	= 0x00010000,
	ISP_FUNC_EN_BNR		= 0x00020000,
	ISP_FUNC_EN_AI_AIDED= 0x00040000,
} ISP_FUNC_EN;

/**
	isp event type
	note: _IMM event will trigger cb directly(by isr/task),
	      otherwise will call fp through another task
*/
typedef enum {
	ISP_EVENT_NONE					= 0x00000000,
	ISP_EVENT_VCAP_VD				= 0x00000001,	///< SIE/VIE VD, parameters latch
	ISP_EVENT_VCAP_VD_IMM			= 0x00000002,	///< SIE/VIE VD, SIE parameters latch
	ISP_EVENT_VCAP_BP1				= 0x00000004,	///< SIE interrupt: Break point 1
	ISP_EVENT_VCAP_BP1_IMM			= 0x00000008,	///< SIE interrupt: Break point 1
	ISP_EVENT_VCAP_MD_HIT			= 0x00000010,	///< SIE interrupt: MD
	ISP_EVENT_VCAP_MD_HIT_IMM		= 0x00000020,	///< SIE interrupt: MD
	ISP_EVENT_VCAP_ACTST			= 0x00000040,	///< SIE interrupt: Active window start
	ISP_EVENT_VCAP_ACTST_IMM		= 0x00000080,	///< SIE interrupt: Active window start
	ISP_EVENT_VCAP_CRPST			= 0x00000100,	///< SIE/VIE interrupt: Crop window start
	ISP_EVENT_VCAP_CRPST_IMM		= 0x00000200,	///< SIE/VIE interrupt: Crop window start
	ISP_EVENT_VCAP_DRAM_OUT0_END	= 0x00000400,	///< SIE/VIE interrupt: Dram output channel 0 end
	ISP_EVENT_VCAP_DRAM_OUT0_END_IMM= 0x00000800,	///< SIE/VIE interrupt: Dram output channel 0 end
	ISP_EVENT_VCAP_ACTEND			= 0x00001000,	///< SIE interrupt: active end
	ISP_EVENT_VCAP_ACTEND_IMM		= 0x00002000,	///< SIE interrupt: active end
	ISP_EVENT_VCAP_CRPEND			= 0x00004000,	///< SIE/VIE interrupt: crop end
	ISP_EVENT_VCAP_CRPEND_IMM		= 0x00008000,	///< SIE/VIE interrupt: crop end
	ISP_EVENT_IPP_CFGSTART			= 0x00010000,	///< IPP task config parameter start
	ISP_EVENT_IPP_CFGSTART_IMM		= 0x00020000,	///< IPP task config parameter start
	ISP_EVENT_IPP_CFGEND			= 0x00040000,	///< IPP task config parameter end
	ISP_EVENT_IPP_CFGEND_IMM		= 0x00080000,	///< IPP task config parameter end
	ISP_EVENT_IPP_PROCEND			= 0x00100000,	///< IPP interrupt: Process end
	ISP_EVENT_IPP_PROCEND_IMM		= 0x00200000,	///< IPP interrupt: Process end
	ISP_EVENT_ENC_TNR				= 0x00400000,	///< ENC 3DNR
	ISP_EVENT_ENC_RATIO				= 0x00800000,	///< ENC isp ratio
	ISP_EVENT_ENC_SHARPEN			= 0x01000000,	///< ENC Sharpen
	ISP_EVENT_VPE_CFG_IMM			= 0x02000000,	///< VPE Config
	ISP_EVENT_DRE_CFG_IMM			= 0x04000000,	///< DRE Config
	ISP_EVENT_CFGINIT				= 0x40000000,	///< event for init config
	ISP_EVENT_PARAM_RST				= 0x80000000,	///< isp parameters reset
} ISP_EVENT;

/**
	isp event callback function prototype
*/
typedef INT32 (*ISP_EVENT_FP)(ISP_ID id, ISP_EVENT evt, UINT32 frame_cnt, void *param);

#endif
