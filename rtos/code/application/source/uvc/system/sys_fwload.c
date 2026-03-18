#include <stdlib.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <kwrap/flag.h>
#include <kwrap/util.h>
#include <kwrap/debug.h>
#include <kwrap/task.h>
#include <kwrap/perf.h>
#include <comm/shm_info.h>
#include <strg_def.h>
#include <FwSrvApi.h>
#include "prjcfg.h"
#include "sys_fwload.h"
#include "sys_storage_partition.h"

//#NT#PARTIAL_COMPRESS, we use rtos-main size + 1 block size
#define FW_PARTIAL_COMPRESS_WORK_BUF_SIZE 0x500000


#if defined(_FW_TYPE_PARTIAL_) || defined(_FW_TYPE_PARTIAL_COMPRESS_)

static ID fwload_flg_id = 0;

//This array is sort by section id
static UINT32 UserSection_Load[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//This array is sort by loading order
static UINT32 UserSection_Order_full[] = {
	CODE_SECTION_02, CODE_SECTION_03, CODE_SECTION_04, CODE_SECTION_05, CODE_SECTION_06,
	CODE_SECTION_07, CODE_SECTION_08, CODE_SECTION_09, CODE_SECTION_10, FWSRV_PL_BURST_END_TAG
};

int fwload_init(void)
{
	vos_flag_create(&fwload_flg_id, NULL, "fwload_flg_id");
	vos_flag_clr(fwload_flg_id, (FLGPTN)-1);
	// CODE_SECTION_01 has loaded by loader or u-boot
	vos_flag_set(fwload_flg_id, (FLGPTN)(1 << CODE_SECTION_01));
	return 0;
}

int fwload_set_done(CODE_SECTION section)
{
	vos_perf_list_mark("pl", __LINE__, (int)section);
	vos_flag_set(fwload_flg_id, (FLGPTN)(1 << section));
	return 0;
}

int fwload_wait_done(CODE_SECTION section)
{
	FLGPTN flgptn;
	vos_flag_wait(&flgptn, fwload_flg_id, (FLGPTN)(1 << section), TWF_ANDW);
	return 0;
}

static void UserSection_LoadCb(const UINT32 Idx)
{
	DBG_DUMP("Section-%.2ld: (LOAD)\r\n", Idx + 1);
	UserSection_Load[Idx] = 1; //mark loaded
	fwload_set_done(Idx);
}

int fwload_partload(void)
{
	SHMINFO *p_shm;
	FWSRV_INIT Init = {0};
	FWSRV_CMD Cmd = {0};
	FWSRV_PL_LOAD_BURST_IN pl_in = {0};
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	// read SHMEM_PATH
	nodeoffset = fdt_path_offset(p_fdt, SHMEM_PATH);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", SHMEM_PATH, nodeoffset);
	} else {
		DBG_DUMP("offset for  %s = %d \n", SHMEM_PATH, nodeoffset);
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return 0;
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		p_shm = (SHMINFO *)be32_to_cpu(p_data[0]);
		DBG_DUMP("p_shm = 0x%08X\n", (int)p_shm);
	}

	// init fwsrv
	Init.uiApiVer = FWSRV_API_VERSION;
	Init.StrgMap.pStrgFdt = EMB_GETSTRGOBJ(STRG_OBJ_FW_FDT);
	Init.StrgMap.pStrgApp = EMB_GETSTRGOBJ(STRG_OBJ_FW_APP);
	Init.StrgMap.pStrgUboot = EMB_GETSTRGOBJ(STRG_OBJ_FW_UBOOT);
	Init.StrgMap.pStrgRtos = EMB_GETSTRGOBJ(STRG_OBJ_FW_RTOS);
	Init.PlInit.uiApiVer = PARTLOAD_API_VERSION;
	Init.PlInit.pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_RTOS);
#if defined(_FW_TYPE_PARTIAL_COMPRESS_)
	Init.PlInit.DataType = PARTLOAD_DATA_TYPE_COMPRESS_GZ;
	Init.PlInit.uiWorkingAddr = (UINT32)malloc(FW_PARTIAL_COMPRESS_WORK_BUF_SIZE);
	Init.PlInit.uiWorkingSize = FW_PARTIAL_COMPRESS_WORK_BUF_SIZE ;
#else
	Init.PlInit.DataType = PARTLOAD_DATA_TYPE_UNCOMPRESS;
	Init.PlInit.uiWorkingAddr = (UINT32)malloc(_EMBMEM_BLK_SIZE_);
	Init.PlInit.uiWorkingSize = _EMBMEM_BLK_SIZE_ ;
#endif
	Init.PlInit.uiAddrBegin = _BOARD_RTOS_ADDR_ + p_shm->boot.LdLoadSize;
	FwSrv_Init(&Init);
	FwSrv_Open();

	// start partial load
	void (*LoadCallback)(const UINT32 Idx) = UserSection_LoadCb;
	UINT32 *SecOrderTable = UserSection_Order_full;

	LoadCallback(CODE_SECTION_01); // 1st part has loaded by loader


	// partial load not support on T.bin
	if ((p_shm->boot.LdCtrl2 & LDCF_BOOT_CARD) == 0) {
		ER er;
		pl_in.puiIdxSequence = SecOrderTable;
		pl_in.fpLoadedCb = LoadCallback;
		Cmd.Idx = FWSRV_CMD_IDX_PL_LOAD_BURST; //continue load
		Cmd.In.pData = &pl_in;
		Cmd.In.uiNumByte = sizeof(pl_in);
		Cmd.Prop.bExitCmdFinish = TRUE;

		er = FwSrv_Cmd(&Cmd);
		if (er != FWSRV_ER_OK) {
			FwSrv_Close();
			free((void *)Init.PlInit.uiWorkingAddr);
			DBG_ERR("Process failed!\r\n");
			return -1;
		} else {
			FwSrv_Close();
			free((void *)Init.PlInit.uiWorkingAddr);
		}
	} else {
		UINT32 i;
		for (i = CODE_SECTION_02; i <= CODE_SECTION_10; i++) {
			if (i == CODE_SECTION_10) {
				FwSrv_Close();
				free((void *)Init.PlInit.uiWorkingAddr);
			}
			LoadCallback(i);
		}

	}



	return 0;
}

#else

int fwload_init(void)
{
	return 0;
}

int fwload_set_done(CODE_SECTION section)
{
	return 0;
}

int fwload_wait_done(CODE_SECTION section)
{
	return 0;
}

int fwload_partload(void)
{
	return 0;
}

#endif

