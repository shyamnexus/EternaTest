#ifndef _SYS_STORAGE_PARTITION_H
#define _SYS_STORAGE_PARTITION_H

#if defined(_EMBMEM_NONE_)
#define EMB_GETSTRGOBJ(x) NULL //Always NULL
#define MAKE_FDT_PARTITION_PATH(x) "/null/partition_"#x
#elif defined(_EMBMEM_NAND_) || defined(_EMBMEM_SPI_NAND_)
#include "nand.h"
#define EMB_GETSTRGOBJ(x) nand_getStorageObject(x)
#define MAKE_FDT_PARTITION_PATH(x) "/nand/partition_"#x
#elif defined(_EMBMEM_SPI_NOR_)
#include "nand.h"
#include "RamDisk/RamDisk.h"
#define EMB_GETSTRGOBJ(x) spiflash_getStorageObject(x)
#define MAKE_FDT_PARTITION_PATH(x) "/nor/partition_"#x
#elif defined(_EMBMEM_EMMC_)
#include "sdio.h"
#define EMB_GETSTRGOBJ(x) sdio3_getStorageObject(x)
#define MAKE_FDT_PARTITION_PATH(x) "mmc2/partition_"#x
#elif defined(_EMBMEM_COMBO_)
#include "nand.h"
#include "sdio.h"
#define EMB_GETSTRGOBJ(x) storage_partition_getObject(x)
#define MAKE_FDT_PARTITION_PATH(x) storage_partition_make_fdt_path(#x)
PSTORAGE_OBJ storage_partition_getObject(STRG_OBJ_ID strgObjID);
char *storage_partition_make_fdt_path(char *name);
#endif

#define PARTITION_PATH_FDT    MAKE_FDT_PARTITION_PATH(fdt)
#define PARTITION_PATH_APP    MAKE_FDT_PARTITION_PATH(fdt.app)
#define PARTITION_PATH_RTOS   MAKE_FDT_PARTITION_PATH(rtos)
#define PARTITION_PATH_UBOOT  MAKE_FDT_PARTITION_PATH(uboot)
#define PARTITION_PATH_LINUX  MAKE_FDT_PARTITION_PATH(kernel)
#define PARTITION_PATH_ROOTFS MAKE_FDT_PARTITION_PATH(rootfs)
#define PARTITION_PATH_AI     MAKE_FDT_PARTITION_PATH(ai)
#define PARTITION_PATH_ALL    MAKE_FDT_PARTITION_PATH(all)

int storage_partition_init1(void);
int storage_partition_init2(void);

/**
     read partition

     @param[in] pathname  partition path on fdt
     @param[in] buff      buffer for read
     @param[in] offset    offset on certain partition (!!! must block size alignment !!!)
     @param[in] size      read size (!!! must block size alignment !!!)
     @return 0 for success.
     Example:
     @code
     {
        #include "sys_storage_partition.h"
        storage_partition_read_part("/nor/partition_kernel", (unsigned char*)buf, 0x0, 0x10000);
        storage_partition_read_part("/nor/partition_rtos", (unsigned char*)buf, 0x0, 0x10000);
     }
     @endcode
*/
int storage_partition_read_part(const char *pathname, unsigned char *buff, int offset, int size);

/**
     write partition

     for spi-nand, this api does not support offset-write
     because raw partition has bad block issue.

     DO NOT use this function to write rootfs.
     for example, to update ubi partition is needing erase whole partiiton first.

     @param[in] pathname  partition path on fdt
     @param[in] buff      buffer for write
     @param[in] size      write size (!!! must block size alignment !!!)
     @return 0 for success.
     Example:
     @code
     {
        #include "sys_storage_partition.h"
        storage_partition_write_part("/nor/partition_kernel", (unsigned char*)buf, 0x0, 0x10000);
        storage_partition_write_part("/nor/partition_rtos", (unsigned char*)buf, 0x0, 0x10000);
     }
     @endcode
*/
int storage_partition_write_part(const char *pathname, unsigned char *buff, int offset, int size);


#endif
