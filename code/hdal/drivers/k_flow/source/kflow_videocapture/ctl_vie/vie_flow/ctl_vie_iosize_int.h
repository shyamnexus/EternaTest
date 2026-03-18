#ifndef _CTL_VIE_IOSIZE_INT_H_
#define _CTL_VIE_IOSIZE_INT_H_

#include "ctl_vie.h"
#include "ctl_vie_utility_int.h"

#define CTL_VIE_ALIGN_MAX       256
#define CTL_VIE_DFT_WIDTH       640
#define CTL_VIE_DFT_HEIGHT      480

typedef struct {
	URECT(*get_vie_crp_win)(CTL_VIE_ID id, CTL_SEN_GET_MODE_BASIC_PARAM *sen_mode_param);    ///< vie crop window
	USIZE(*get_vie_out_sz)(CTL_VIE_ID id, CTL_SEN_GET_MODE_BASIC_PARAM *sen_mode_param);      ///< vie out size
	URECT(*get_dest_crp_win)(CTL_VIE_ID id, CTL_SEN_GET_MODE_BASIC_PARAM *sen_mode_param);   ///< need to get after get_vie_crp_win()
} CTL_VIE_IOSIZE;

CTL_VIE_IOSIZE *ctl_vie_iosize_get_obj(void);
void ctl_vie_iosize_init(CTL_VIE_ID id);

#endif //_CTL_VIE_IOSIZE_INT_H_
