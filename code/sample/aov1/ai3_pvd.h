
#include "limit_fdet_lib.h"
#include "pvdcnn_lib.h"
#include "ext_module_api.h"
#include "aov1_common.h"
#include "hd_type.h"

typedef struct _PVD_THREAD_MEM{
	NN_CFG_BUF_M pvd_mem;
	NN_CFG_BUF_M pvd_model_mem;
	NN_CFG_BUF_M limit_fdet_mem;
	NN_CFG_BUF_M limit_model_mem;
	NN_CFG_BUF_M scale_mem;
	int pvd_ret;
} PVD_THREAD_MEM;

VOID *nn_thread_api(VOID *arg);
HD_RESULT system_get_mem(UINT32 ddr, NN_CFG_BUF_M *buf, UINT32 size, INT32 index);
HD_RESULT system_free_mem(NN_CFG_BUF_M *buf, INT32 index);
HD_RESULT mem_alloc(VENDOR_AIS_FLOW_MEM_PARM *mem_parm, CHAR* name, UINT32 size);
HD_RESULT mem_free(VENDOR_AIS_FLOW_MEM_PARM *mem_parm);
HD_RESULT ai_buffer_total_size_get(UINT32 *ai_buff_size);
HD_RESULT ai_get_model_name(CHAR **name);
HD_RESULT assign_ai_buf(NN_CFG_BUF_M *buf, PVD_THREAD_MEM *parm);
HD_RESULT ai_mem_init(void);
HD_RESULT ai_mem_uninit(void);