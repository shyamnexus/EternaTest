#include <string.h>
#include <plat/sdio.h>
#include <plat/strg_def.h>
#include <hdal.h>
#include <FileSysTsk.h>
#include <kwrap/task.h>
#include <kwrap/util.h>
#include "sys_filesys.h"
#include "sys_mempool.h"
#include "gpio.h"
#include <FreeRTOS.h>
#include <task.h>
#include <rtosfdt.h>
#include <libfdt.h>
#include <compiler.h>

#ifdef _NVT_FPGA_
int GPIO_CARD_DETECT = D_GPIO_0;
#else
int GPIO_CARD_DETECT = C_GPIO_9;
#endif
int GPIO_CARD_INSERT_LEVEL = FALSE; // low active

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
	if (FST_STA_OK != FileSys_Init(FileSys_GetOPS_uITRON())) {
		printf("FileSys_Init failed\r\n");
	}
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

static void sys_detect_card_task(void)
{
	THREAD_ENTRY();

	int old_detect;
	unsigned char *fdt_addr = (unsigned char *)fdt_get_base();
	int nodeoffset;
	uint32_t *cell = NULL;

	if(fdt_addr) {
		const char *mmc0_path = fdt_get_alias(fdt_addr, "mmc0");
		nodeoffset = fdt_path_offset((const void*)fdt_addr, mmc0_path);
		if(nodeoffset < 0) {
			printf("error in capture mmc node %d\r\n", nodeoffset);
		}
		cell = (uint32_t*)fdt_getprop((const void*)fdt_addr, nodeoffset, "cd_gpio", NULL);
		if (cell > 0) {
			GPIO_CARD_DETECT = (int)be32_to_cpu(cell[0]);
			GPIO_CARD_INSERT_LEVEL = (int)be32_to_cpu(cell[1]);
		}
	}



	// force 1st time card detect
	old_detect = !GPIO_CARD_INSERT_LEVEL;

	//coverity[no_escape]
	while (1) {
		int curr_detect;
		vTaskDelay(pdMS_TO_TICKS(1000));
		curr_detect= gpio_getPin(GPIO_CARD_DETECT);
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
}

void filesys_init(void)
{
	FileSys_InstallID(FileSys_GetOPS_uITRON());
	task_hdl = vos_task_create(sys_detect_card_task,  0, "SysDetectCard",   16,  8192);
	vos_task_resume(task_hdl);
}

