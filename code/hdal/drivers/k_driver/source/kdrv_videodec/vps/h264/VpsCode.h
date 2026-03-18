
#ifndef __H264D_VPS_CODE_H__
#define __H264D_VPS_CODE_H__

#include "../vps_comm.h"

typedef struct _ST_H264_VPS_CONTEXT ST_H264_VPS_CONTEXT, *PST_H264_VPS_CONTEXT;

struct _ST_H264_VPS_CONTEXT
{
    bstream rw;
    unsigned int old_write;
    void *priv;
};

#endif

