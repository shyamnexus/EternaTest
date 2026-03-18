#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/task.h"
#include "kwrap/type.h"

#include "iq_task_int.h"

//=============================================================================
// global variable
//=============================================================================
IQ_TASK_OBJ iq_task_obj[IQ_ID_MAX_NUM];
SEM_HANDLE iq_semi_id;

//=============================================================================
// extern functions
//=============================================================================
void iq_task_id_install(void)
{
	UINT32 i;

	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		OS_CONFIG_FLAG(iq_task_obj[i].flag_id);
	}

	SEM_CREATE(iq_semi_id, 1);
}

void iq_task_id_uninstall(void)
{
	UINT32 i;

	for (i = 0; i < IQ_ID_MAX_NUM; i++) {
		rel_flg(iq_task_obj[i].flag_id);
	}

	SEM_DESTROY(iq_semi_id);
}

