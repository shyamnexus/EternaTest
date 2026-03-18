#include <string.h>
#include <plat/sdio.h>
#include <plat/strg_def.h>
#include <hdal.h>
#include <FileSysTsk.h>
#include <kwrap/task.h>
#include <kwrap/util.h>
#include <kwrap/debug.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include "sys_filesys.h"
#include "sys_mempool.h"
#include "gpio.h"

#define GPIO_CARD_INSERT_LEVEL			(FALSE)		// low active

#define MAX_OPENED_FILE_NUM 10

static THREAD_HANDLE task_hdl;

static void card_insert_job(void)
{
	UINT32 uiPoolAddr;
	int    ret;

	FILE_TSK_INIT_PARAM     Param = {0};
	FS_HANDLE               StrgDXH;

	printf("filesys_init b\r\n");
	memset(&Param, 0, sizeof(FILE_TSK_INIT_PARAM));
	StrgDXH = (FS_HANDLE)sdio_getStorageObject(STRG_OBJ_FAT1);

	uiPoolAddr = mempool_filesys;
	Param.FSParam.WorkBuf = uiPoolAddr;
	Param.FSParam.WorkBufSize = (POOL_SIZE_FILESYS);
	// support exFAT
	Param.FSParam.bSupportExFAT   = TRUE;
	//Param.pDiskErrCB = (FileSys_CB)Card_InitCB;
	strncpy(Param.FSParam.szMountPath, "/mnt/sd", sizeof(Param.FSParam.szMountPath) - 1); //only used by FsLinux
	Param.FSParam.szMountPath[sizeof(Param.FSParam.szMountPath) - 1] = '\0';
	Param.FSParam.MaxOpenedFileNum = MAX_OPENED_FILE_NUM;
	ret = FileSys_OpenEx('A', StrgDXH, &Param);
	if (FST_STA_OK != ret) {
		printf("FileSys_Open err %d\r\n", ret);
	}
	// call the function to wait init finish
	FileSys_WaitFinishEx('A');
	printf("filesys_init e\r\n");
}

static void card_remove_job(void)
{
	FileSys_WaitFinishEx('A');
	FileSys_CloseEx('A', FST_TIME_INFINITE);
}

static int card_get_gpio_num(UINT32 *p_pin)
{
	*p_pin = 0;
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	// read SHMEM_PATH
	nodeoffset = fdt_path_offset(p_fdt, "mmc0");
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", "mmc0", nodeoffset);
		return -1;
	}

	nodep = fdt_getprop(p_fdt, nodeoffset, "cd_gpio", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return -1;
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		*p_pin = be32_to_cpu(p_data[0]);
		return 0;
	}
	return -1;
}

static void sys_detect_card_task(void)
{
	UINT32 old_detect;
	UINT32 pin_card_detect;

	THREAD_ENTRY();

	if (card_get_gpio_num(&pin_card_detect) !=0) {
		THREAD_RETURN(0);
	}

	// force 1st time card detect
	old_detect = !GPIO_CARD_INSERT_LEVEL;

	//coverity[no_escape]
	while (1) {
		UINT32 curr_detect;
		vos_util_delay_ms(1000);
		curr_detect= gpio_getPin(pin_card_detect);
		if (old_detect != curr_detect) {
			if (curr_detect == GPIO_CARD_INSERT_LEVEL) {
				printf("%s: Card Insert\r\n", __func__);
				card_insert_job();
			} else {
				printf("%s: Card Remove\r\n", __func__);
				card_remove_job();
			}
		}

		old_detect = curr_detect;
	}

	THREAD_RETURN(0);
}

void filesys_init(void)
{
	FileSys_InstallID(FileSys_GetOPS_uITRON());
	if (FST_STA_OK != FileSys_Init(FileSys_GetOPS_uITRON())) {
		printf("FileSys_Init failed\r\n");
		return;
	}

	task_hdl = vos_task_create(sys_detect_card_task,  0, "SysDetectCard",   16,  8192);
	vos_task_resume(task_hdl);
}

