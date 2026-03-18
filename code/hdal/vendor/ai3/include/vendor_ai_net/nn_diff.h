/**
@brief Header file of NN engine definition of vendor net flow sample.

@file nn_diff.h

@ingroup net_flow_sample

@note Nothing.

Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/


#ifndef _NN_DIFF_H_
#define _NN_DIFF_H_



//#include "type.h"

typedef struct _NN_DIFF_MODE_CTRL { ///< mode control of nvt_diff_model.bin
	uintptr_t offset;                    

} NN_DIFF_MODE_CTRL;

/// ##########  diff model struct #############
typedef struct _NN_DIFF_MODEL_HEAD {
    uintptr_t ker_va;
    uintptr_t pa;
    UINT32 dyScale_num;
    UINT32 dyBatch_num;
    UINT32 total_size;
    int8_t __alignment_padding[4];
}__attribute__((packed)) NN_DIFF_MODEL_HEAD;

typedef struct _NN_DIFF_CONFIG {
   uint32_t scale_w;          //width in current resolution
   uint32_t scale_h;          //height in current resolution
   uint32_t batch_size;       //batch size in current resolution
   int8_t reserved[3];
   int8_t version;            // record diff model version
}__attribute__((packed))  NN_DIFF_CONFIG;

typedef struct _NN_DIFF_HEAD
{
    NN_DIFF_CONFIG diff_id;       ///< configuration of scale model
    uint32_t aiparm_info_num;    // size of diff  parameters(NN_DIFF_PARM) 
    uint32_t output_info_num; 
}__attribute__((packed))  NN_DIFF_HEAD;

typedef struct _NN_DIFF_OFS {
  uint64_t line_ofs;
  uint64_t channel_ofs;
  uint64_t batch_ofs;
}NN_DIFF_OFS;

typedef struct _OUTPUT_INFO {
  UINT32 mctrl_id;
  UINT32 addr_offset;
  UINT32 width;
  UINT32 height;
  UINT32 channel;
  UINT32 batch;
  NN_DIFF_OFS out_ofs;
}OUTPUT_INFO;

typedef struct _AIPARM_INFO{
    uint64_t addr;
    uint64_t size;
}  AIPARM_INFO;

typedef struct _JOBLIST_INFO{
    uint64_t addr;
    uint64_t size;
}  JOBLIST_INFO;

#endif 

