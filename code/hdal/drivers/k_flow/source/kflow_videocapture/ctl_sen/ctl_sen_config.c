/*
    sensor control configuration file.

    sensor control configuration file. Define semaphore ID, flag ID, etc.

    @file       ctl_sen_config.c
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#include "ctl_sen_int.h"

/* flag */
ID ctl_sen_flag_glb = 0;
ID ctl_sen_flag_map[CTL_SEN_NUM_SEN_ID] = {0};
ID ctl_sen_flag_chip[CTL_SEN_NUM_SEN_ID] = {0};

#define flag_map_cnt (sizeof(ctl_sen_flag_map)/sizeof(ID))
#define flag_chip_cnt (sizeof(ctl_sen_flag_chip)/sizeof(ID))

/* semphore */
SEM_HANDLE ctl_sen_sem_proc;
SEM_HANDLE ctl_sen_sem_mclken; // pretect mclk_en_chip_id

void ctl_sen_install_id(void)
{
	UINT32 i;

	/* flag */
	vos_flag_create(&ctl_sen_flag_glb, NULL, "ctl_sen_glb");
	vos_flag_set(ctl_sen_get_flag_id(CTL_SEN_FLAG_GLB, 0), CTL_SEN_FLAG_GLB_INIT);

	for (i = 0; i < flag_map_cnt; i++) {
		vos_flag_create(&ctl_sen_flag_map[i], NULL, "ctl_sen_map");
		vos_flag_set(ctl_sen_get_flag_id(CTL_SEN_FLAG_MAP, i), CTL_SEN_FLAG_MAP_INIT);
	}

	for (i = 0; i < flag_chip_cnt; i++) {
		vos_flag_create(&ctl_sen_flag_chip[i], NULL, "ctl_sen_sendrv");
		vos_flag_set(ctl_sen_get_flag_id(CTL_SEN_FLAG_CHIP, i), CTL_SEN_FLAG_CHIP_INIT);
	}

	/* semphore */
	#ifdef __KERNEL__
	vos_sem_create(&ctl_sen_sem_proc, 1, "ctl_sen_sem_proc");
	#endif
	vos_sem_create(&ctl_sen_sem_mclken, 1, "ctl_sen_sem_mclken");
}

void ctl_sen_uninstall_id(void)
{
	UINT32  i;

	/* flag */
	vos_flag_destroy(ctl_sen_flag_glb);

	for (i = 0; i < flag_map_cnt; i++) {
		vos_flag_destroy(ctl_sen_flag_map[i]);
	}

	for (i = 0; i < flag_chip_cnt; i++) {
		vos_flag_destroy(ctl_sen_flag_chip[i]);
	}

	/* semphore */
	#ifdef __KERNEL__
	vos_sem_destroy(ctl_sen_sem_proc);
	#endif
	vos_sem_destroy(ctl_sen_sem_mclken);
}

ID ctl_sen_get_flag_id(CTL_SEN_FLAG type, UINT32 idx)
{
	if (type == CTL_SEN_FLAG_GLB) {
		return ctl_sen_flag_glb;
	} else if (type == CTL_SEN_FLAG_MAP) {
		if (idx < flag_map_cnt) {
			return ctl_sen_flag_map[idx];
		}
	} else if (type == CTL_SEN_FLAG_CHIP) {
		if (idx < flag_chip_cnt) {
			return ctl_sen_flag_chip[idx];
		}
	}

	ctl_sen_dbg_err("type %d, ofs %d\r\n", type, idx);
	return 0;
}
