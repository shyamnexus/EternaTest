#ifndef _IQ_BUILTIN_H_
#define _IQ_BUILTIN_H_

#include "kwrap/type.h"
//#include "siep_lib.h"
#include "isp_builtin_int.h"
#include "iq_param.h"

//=============================================================================
// version
//=============================================================================
#define IQ_BUILTIN_VERSION 0x01000000

//=============================================================================
// define
//=============================================================================
#if defined(__KERNEL__)
#define SIE_BASE_ADDR   0x2F0310000
#define SIE2_BASE_ADDR  0x2F0311000
#define SIE3_BASE_ADDR  0x2F0312000
#define SIE4_BASE_ADDR  0x2F0313000
#define SIE5_BASE_ADDR  0x2F0314000
#define PRE_BASE_ADDR   0x2F0341000
#define IFE_BASE_ADDR   0x2F0340000
#define IPE_BASE_ADDR   0x2F0400000
#define IME_BASE_ADDR   0x2F0410000
#define ENC_BASE_ADDR   0x2F00634C4
#else
#define SIE_BASE_ADDR   0xF0310000
#define SIE2_BASE_ADDR  0xF0311000
#define SIE3_BASE_ADDR  0xF0312000
#define SIE4_BASE_ADDR  0xF0313000
#define SIE5_BASE_ADDR  0xF0314000
#define PRE_BASE_ADDR   0xF0341000
#define IFE_BASE_ADDR   0xF0340000
#define IPE_BASE_ADDR   0xF0400000
#define IME_BASE_ADDR   0xF0410000
#define ENC_BASE_ADDR   0xF00634C4
#endif
#define SIE_SIZE        0xBEC
#define PRE_SIZE        0xEDC
#define IFE_SIZE        0xD88
#define IPE_SIZE        0x18BC
#define IME_SIZE        0xC8C
#define ENC_SIZE        0x3C

#define GAMMA_BUFFER_SIZE  208 * 4      // 208, 64 aligned
#define YCURVE_BUFFER_SIZE 80 * 4       // 80, 64 aligned
#define DPC_BUFFER_SIZE    4096 * 4     // 4095, 64 aligned
#define DPC_TABLE_SIZE     4096         // 4095, 64 aligned
#define ECS_BUFFER_SIZE    65 * 65 * 4  // 65*65, 64 aligned
#define ECS_TABLE_SIZE     65 * 65

#define IQ_EDGE_GAMMA_LEN                  65
#define IQ_DEFOG_FOG_MOD_LEN               17
#define TMNR_PROBABILITY_LEN               8

#define IQ_WDR_SUBIMG_MIN 8
#define IQ_WDR_SUBIMG_MAX 48

#define IQ_CLAMP(in, lb, ub) (((INT32)(in) <= (INT32)(lb)) ? (INT32)(lb) : (((INT32)(in) >= (INT32)(ub)) ? (INT32)(ub) : (INT32)(in)))

//=============================================================================
// struct & enum definition
//=============================================================================
typedef enum _FRONT_FACTOR {
	FRONT_FACTOR_VALUE = 0,
	FRONT_FACTOR_START,
	FRONT_FACTOR_END,
	FRONT_FACTOR_NUM
} FRONT_FACTOR;

typedef struct _IQ_BUILTIN_FRONT_FACTOR {
	UINT32 rth_nlm;
	UINT32 rth_nlm_lut;
	UINT32 clamp_th;
	UINT32 c_con;
	UINT32 overshoot;
} IQ_BUILTIN_FRONT_FACTOR;

typedef struct _IQ_BUILTIN_FRONT_PARAM {
	UINT32 smooth_thr;                          ///< range : 0~30, frame number for spectial image adjustment in early stage.
	UINT32 iso_w[IQ_GAIN_ID_MAX_NUM];           ///< range : 0~100, weighting of front IQ for each ISO. 0: no front IQ. 100: front IQ.
	UINT32 rth_nlm;                             ///< range : 100~1000, ratio of IQ_NR_PARAM filter_th for the first frame. 100: no change.
	UINT32 rth_nlm_lut;                         ///< range : 100~1000, ratio of IQ_NR_PARAM filter_lut for the first frame. 100: no change.
	UINT32 clamp_th;                            ///< range : 0~100, ratio of IQ_NR_PARAM filter_clamp_th for the first frame. 100: no change.
	UINT32 c_con;                               ///< range : 0~100, ratio of IQ_COLOR_PARAM c_con for the first frame. 100: no change.
	UINT32 overshoot;                           ///< range : 0~100, ratio of IQ_CONTRAST_PARAM y_con for the first frame. 100: no change.
} IQ_BUILTIN_FRONT_PARAM;

typedef enum _BUILTIN_IQ_ITEM {
	BUILTIN_IQ_ITEM_ENC_PARAM,
	BUILTIN_IQ_ITEM_MAX,
	ENUM_DUMMY4WORD(BUILTIN_IQ_ITEM)
} BUILTIN_IQ_ITEM;

typedef struct _ISOMAP {
	unsigned int  iso;
	unsigned int  index;
} ISOMAP;

//=============================================================================
// extern functions
//=============================================================================
extern void iq_builtin_get_histo(UINT32 id, ISP_BUILTIN_HISTO_RSLT *histo_rslt);
extern void iq_builtin_get_airlight(UINT32 id, ISP_BUILTIN_DEFOG_STCS *defog_stcs);
extern void iq_builtin_get_ipe_subout(UINT32 id, ISP_BUILTIN_IPE_SUBOUT_BUF *ipe_subout);
extern void iq_builtin_set_ca_crop(UINT32 id, BOOL enable);
extern void iq_builtin_get_param(UINT32 id, ULONG *param);
extern INT32 iq_builtin_init(UINT32 id);
extern INT32 iq_builtin_uninit(UINT32 id);
extern INT32 iq_builtin_trig(UINT32 id, ISP_BUILTIN_IQ_TRIG_MSG msg);

#endif

