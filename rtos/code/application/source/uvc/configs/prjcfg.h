#ifndef _PRJCFG_H_
#define _PRJCFG_H_

#ifndef ENABLE
#define ENABLE 1
#define DISABLE 0
#endif

//==============================================================================
//   SYSTEM FUNCTIONS
//==============================================================================
//..............................................................................
// boot
#define POWERON_FAST_BOOT      ENABLE    ///< enable to use multi-thread init
#define POWERON_FAST_BOOT_MSG  ENABLE    ///< disable boot msg for fast boot, but hard to debug
#define POWERON_BOOT_REPORT    DISABLE   ///< enable to show report after booting
#define POWERON_EXAM_FLASH_LOAD DISABLE  ///< exam load 16MB data from flash

// share info memory on fdt
#define SHMEM_PATH "/nvt_memory_cfg/shmem"

// flash storage mapping
#define STRG_OBJ_FW_FDT    STRG_OBJ_FW_RSV1
#define STRG_OBJ_FW_APP    STRG_OBJ_FW_RSV2
#define STRG_OBJ_FW_RTOS   STRG_OBJ_FW_RSV3
#define STRG_OBJ_FW_RTOS1  STRG_OBJ_FW_RSV4
#define STRG_OBJ_FW_BANK   STRG_OBJ_FW_RSV5
#define STRG_OBJ_FW_BANK1  STRG_OBJ_FW_RSV6
#define STRG_OBJ_FW_ALL    STRG_OBJ_FW_RSV7
#define STRG_OBJ_FW_PST    STRG_OBJ_PSTORE1


// index for BININFO:: Resv[64]
#define BININFO_RESV_IDX_TAG            0x0
#define BININFO_RESV_IDX_VER            0x1
#define BININFO_RESV_IDX_RELEASE_BUILD  0x2
#define BININFO_RESV_IDX_BOOT_BANK_FROM 0x3
#define BININFO_RESV_IDX_DATE_SEC       0x4


// UVC functions
#define UVC_LED DISABLE //UVC led indicator
#define UVC_SHUTTER DISABLE //UVC Privacy shutter
#define UVC_SHUTTER_GPIO D_GPIO_1 //UVC Privacy shutter GPIO number
#define UVC_SHUTTER_GPIO_INT GPIO_INT_33 //UVC Privacy inturrprt GPIO number
#define UVC_DPC_FUNC 0	// 1: enable, 0: disable, DPC needs filesys and SD card support, and uses D2D mode
#define UVC_MSOS20_FUNC DISABLE
#endif
