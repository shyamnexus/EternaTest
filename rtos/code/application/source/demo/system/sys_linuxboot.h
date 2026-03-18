#ifndef SYS_LINUXBOOT_H
#define SYS_LINUXBOOT_H

typedef struct _LINUX_RESERVED_MEM {
	char name[32];
	uintptr_t addr;
	unsigned long size;
} LINUX_RESERVED_MEM;

typedef struct _FDT_INFO {
	unsigned long linux_addr;
	unsigned long linux_size;
	unsigned long linuxtmp_addr;
	unsigned long linuxtmp_size;
	unsigned long bridge_addr;
	unsigned long bridge_size;
	// following are for check memory partition
	unsigned long dram_addr;
	unsigned long dram_size;
	unsigned long fdt_addr;
	unsigned long fdt_size;
	unsigned long rtos_addr;
	unsigned long rtos_size;
	unsigned long hdal_addr;
	unsigned long hdal_size;
	unsigned long shmem_addr;
	unsigned long shmem_size;
	unsigned long atf_addr;
	unsigned long atf_size;
	unsigned long cpu_release_addr;
	unsigned long reserved_cnt;
	LINUX_RESERVED_MEM reserved[8];
} FDT_INFO;

typedef struct _LINUXTMP_PARTITION {
	unsigned long tmp_begin;
	unsigned long tmp_curr;
	unsigned long tmp_end;
	unsigned long tmp2_begin; // if it available, means small dram.
	unsigned long tmp2_curr;  // use the memory 8MB - rtos_addr
	unsigned long tmp2_end;   // to store fdt and ramdisk
	unsigned long fdt_addr;
	unsigned long fdt_size;
	unsigned long ramfs_addr;
	unsigned long ramfs_size;
	unsigned long bootargs_addr;
	unsigned long bootargs_size;
	unsigned long lz_linux_addr;
	unsigned long lz_linux_size;
	unsigned long nvtpack_addr;
	unsigned long nvtpack_size;
} LINUXTMP_PARTITION;

typedef struct _LINUXBOOT_INFO {
	FDT_INFO fdt_info;
	LINUXTMP_PARTITION linuxtmp;
} LINUXBOOT_INFO;

int linuxboot_setup(LINUXBOOT_INFO *p_info);
void linuxboot_go(LINUXBOOT_INFO *p_info);
void linuxboot_release_rtos(LINUXBOOT_INFO *p_info, FP fp_notify_linux_go_next_step);
int linuxboot_set_flash_preload(LINUXBOOT_INFO *p_info); //optional, for fastboot-ai on nor
int linuxboot_set_i2c_preload(int channel); //optional, for fastboot-2a
#endif