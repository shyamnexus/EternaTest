
#ifndef __H265D_VPS_CODE_H__
#define __H265D_VPS_CODE_H__

#include "../vps_comm.h"

typedef struct _ST_H265_VPS_CONTEXT ST_H265_VPS_CONTEXT, *PST_H265_VPS_CONTEXT;

struct _ST_H265_VPS_CONTEXT
{
    hevc_bstream	rw;
    void		*priv;
};

#endif

