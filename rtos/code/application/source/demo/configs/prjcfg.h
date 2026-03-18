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
#define POWERON_FAST_BOOT_MSG  ENABLE   ///< disable boot msg for fast boot, but hard to debug
#define POWERON_BOOT_REPORT    ENABLE    ///< enable to show report after booting
#define POWERON_EXAM_FLASH_LOAD DISABLE  ///< exam load 16MB data from flash

// seletc boot mode
#define POWERON_MODE_NONE 0
#define POWERON_MODE_PREVIEW 1    ///< fast preview
#define POWERON_MODE_ENCODE 2     ///< fast encode
#define POWERON_MODE_BOOT_LINUX 3 ///< fast boot linux (only for cfg_RTOS_BOOT_LINUX_EVB)
#define POWERON_MODE_AOV 4        ///< fast alway on video
#define POWERON_MODE_PREROLL 5    ///< fast pre-roll

#if defined(_RTOS2A_AOV_)
#define POWERON_MODE POWERON_MODE_AOV
#elif defined(_RTOS2A_PREROLL_)
#define POWERON_MODE POWERON_MODE_PREROLL
#elif defined(_BOOT_OS_RTOS_BOOT_LINUX_)
#define POWERON_MODE POWERON_MODE_BOOT_LINUX
#else
#define POWERON_MODE POWERON_MODE_PREVIEW
#endif

#if defined(_RTOS2A_FAST2A_)
#define FASTBOOT_2A ENABLE
#else
#define FASTBOOT_2A DISABLE
#endif

#if defined(_RTOS2A_SENSOR_)
#define SENSOR_EMBEDDED_2A ENABLE
#else
#define SENSOR_EMBEDDED_2A DISABLE
#endif

#if defined(_RTOS2A_PREROLL_)
#define PREROLL_2A ENABLE
#else
#define PREROLL_2A DISABLE
#endif

#if defined(_RTOS2A_NORMAL_)
#define NORMAL_2A ENABLE
#else
#define NORMAL_2A DISABLE
#endif

// share info memory on fdt
#define SHMEM_PATH "/nvt_memory_cfg/shmem"

// flash storage mapping
#define STRG_OBJ_FW_FDT   STRG_OBJ_FW_RSV1
#define STRG_OBJ_FW_APP   STRG_OBJ_FW_RSV2
#define STRG_OBJ_FW_UBOOT STRG_OBJ_FW_RSV3
#define STRG_OBJ_FW_RTOS  STRG_OBJ_FW_RSV4
#define STRG_OBJ_FW_LINUX STRG_OBJ_FW_RSV5
#define STRG_OBJ_FW_ROOTFS STRG_OBJ_FW_RSV6
#define STRG_OBJ_FW_ALL   STRG_OBJ_FW_RSV7

// special for lcd demo with max speed
#if defined(_EMBMEM_SPI_NOR_) && (_PACKAGE_DISPLAY_)
#undef POWERON_FAST_BOOT_MSG
#define POWERON_FAST_BOOT_MSG DISABLE
#define FASTBOOT_LCD_DEMO
#endif

#define FASTBOOT20

#define _TODO_FAST2A 0
#define TODO_HERE printf("\033[0;35mTODO: %d, %s\r\n\033[0m", __LINE__, __func__)

#endif
