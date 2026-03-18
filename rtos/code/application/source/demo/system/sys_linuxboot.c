#include <kwrap/type.h>
#include <kwrap/cpu.h>
#include <kwrap/perf.h>
#include <hdal.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <nvtpack.h>
#include <hwclock.h>
#include <plat/cpu.h>
#include <FwSrvApi.h>
#include <zlib.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include "prjcfg.h"
#include "sys_storage_partition.h"
#include "sys_linuxboot.h"
#include "sys_fdt.h"
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <arm_gic.h>
#include <FileSysTsk.h>
#include <shm_info.h>
#include <bin_info.h>
#include <plat/interrupt.h>
#include <uart.h>
#include <console.h>
#include <lz.h>

#define CFG_INDEP_RAMDISK ENABLE
#if defined(__aarch64__)
#define CFG_LINUX_START_OFFSET 0x200000 //MIN_KIMG_ALIGN SZ_2M
#define CFG_MULTI_MKIMAGE_LEN 0x0 //no using mkimage's multi image
#else
#define CFG_LINUX_START_OFFSET 0x8000
#define CFG_MULTI_MKIMAGE_LEN 0x8 //mkimage's multi image always comes 8 bytes for sub-image size
#endif
#define CFG_BOOTARG_MAX_LEN 0x1000
#define CFG_LINUX_MAX_CODE_SIZE 0x01000000 //for 64MB dram, assume linux (ro+rw+bss) limited on this size
#define CFG_LINUX_COMP_MAX_SIZE 0xA00000 //max uImage size
#define CFG_RAMFS_COMP_MAX_SIZE 0x500000 //max ramdisk size
#define CFG_GZ_WORK_SIZE 0x10000 //64KB are enough
#define CFG_CORE2_ENTRY_REG 0xF07F8000
#ifndef CONFIG_SYS_FDT_PAD
#define CONFIG_SYS_FDT_PAD 0x3000  // from uboot
#endif

#define CACHE_LINE 32

#define MEM_PATH_LINUX "/memory"
#define MEM_PATH_DRAM "/nvt_memory_cfg/dram"
#define MEM_PATH_LINUXTMP "/nvt_memory_cfg/linuxtmp"
#define MEM_PATH_BRIDGE "/nvt_memory_cfg/bridge"
#define MEM_PATH_FDT "/nvt_memory_cfg/fdt"
#define MEM_PATH_ATF "/nvt_memory_cfg/atf"
#define MEM_PATH_RTOS "/nvt_memory_cfg/rtos"
#define MEM_PATH_HDAL "/hdal-memory/media"
#define MEM_PATH_CPU_RELEASE "/cpus/cpu@1"
#define MEM_PATH_SHMEM SHMEM_PATH

#define IS_BIN_OVERLAP(addr1, size1, addr2, size2) (!(((addr1 + size1 - 1) < addr2) || (addr1 > (addr2 + size2 - 1))))

// sync from uboot
#if defined(__aarch64__)
#define CONFIG_BOOTARGS_COMMON "rootwait console=ttyS0,115200 debug_boot_weak_hash print-fatal-signals=1 "
#else
#define CONFIG_BOOTARGS_COMMON "rootwait nprofile_irq_duration=on "
#endif
#if defined(_NVT_ROOTFS_TYPE_RAMDISK_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "root=/dev/ram0 rootfstype=ramfs rdinit=/linuxrc "
#elif defined(_NVT_ROOTFS_TYPE_NAND_UBI_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "root=ubi0:rootfs rootfstype=ubifs ubi.fm_autoconvert=1 init=/linuxrc "
#elif defined(_NVT_ROOTFS_TYPE_SQUASH_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "rootfstype=squashfs ro "
#elif defined(_NVT_ROOTFS_TYPE_NAND_JFFS2_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "rootfstype=jffs2 rw "
#elif defined(_NVT_ROOTFS_TYPE_NOR_JFFS2_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "rootfstype=jffs2 rw "
#elif defined(_NVT_ROOTFS_TYPE_EMMC_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "rootfstype=ext4 rw "
#else //SD
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "root=/dev/mmcblk0p2 noinitrd rootfstype=ext3 init=/linuxrc "
#endif /* _NVT_ROOTFS_TYPE_ */

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
#define IH_MAGIC 0x27051956  /* Image Magic Number       */
#define IH_NMLEN 32          /* Image Name Length        */
typedef struct image_header {
	unsigned int ih_magic;   /* Image Header Magic Number    */
	unsigned int ih_hcrc;    /* Image Header CRC Checksum    */
	unsigned int ih_time;    /* Image Creation Timestamp */
	unsigned int ih_size;    /* Image Data Size      */
	unsigned int ih_load;    /* Data  Load  Address      */
	unsigned int ih_ep;      /* Entry Point Address      */
	unsigned int ih_dcrc;    /* Image Data CRC Checksum  */
	unsigned char ih_os;     /* Operating System     */
	unsigned char ih_arch;   /* CPU architecture     */
	unsigned char ih_type;   /* Image Type           */
	unsigned char ih_comp;   /* Compression Type     */
	unsigned char ih_name[IH_NMLEN];  /* Image Name       */
} image_header_t;

typedef struct _GZ_BUF {
	unsigned char *p_begin;
	unsigned char *p_curr;
	unsigned char *p_end;
} GZ_BUF;

typedef struct _UNGZIP_INPUT {
	unsigned char *in;
	unsigned char *out;
	unsigned int insize;
	unsigned int outsize;
	unsigned int method; // 0: gz, 1: lz77
	GZ_BUF gz_buf; //only for CFG_CORE2_DECOMP_RAMDISK
} UNGZIP_INPUT;

typedef enum _RAMDISK_METHOD {
	RAMDISK_METHOD_PARTIAL, ///< partial load and decompress
	RAMDISK_METHOD_FULL,    ///< full loaded the decompress by thread
	RAMDISK_METHOD_NVTPACK, ///< boot linux from all-in-one on sd card
} RAMDISK_METHOD;

#if (CFG_INDEP_RAMDISK)
#if defined(_NVT_RAMDISK_COMPRESS_NONE_) || defined(_NVT_RAMDISK_COMPRESS_LZ77_)
static RAMDISK_METHOD ramdisk_methold = RAMDISK_METHOD_FULL;
#else // default use partial decompress
static RAMDISK_METHOD ramdisk_methold = RAMDISK_METHOD_PARTIAL;
#endif
#endif //#if (CFG_INDEP_RAMDISK)

#if (CFG_INDEP_RAMDISK)
static pthread_t handle_unzip_ramdisk;
_ALIGNED(64) static UNGZIP_INPUT ungzip_input = {0};
#endif

static int fdt_find_or_add_subnode(void *fdt, int parentoffset, const char *name)
{
	int offset;

	offset = fdt_subnode_offset(fdt, parentoffset, name);

	if (offset == -FDT_ERR_NOTFOUND) {
		offset = fdt_add_subnode(fdt, parentoffset, name);
	}

	if (offset < 0) {
		printf("%s: %s: %s\n", __func__, name, fdt_strerror(offset));
	}

	return offset;
}

static int fdt_chosen(LINUXTMP_PARTITION *p_linuxtmp)
{
	int   nodeoffset;
	int   err;
	char  *str;     /* used to set string properties */

	void *fdt = (void *)p_linuxtmp->fdt_addr;

	err = fdt_check_header(fdt);
	if (err < 0) {
		DBG_DUMP("fdt_chosen: %s\n", fdt_strerror(err));
		return err;
	}

	/* find or create "/chosen" node. */
	nodeoffset = fdt_find_or_add_subnode(fdt, 0, "chosen");
	if (nodeoffset < 0) {
		return nodeoffset;
	}

	str = (char *)p_linuxtmp->bootargs_addr;
	if (str) {
		err = fdt_setprop(fdt, nodeoffset, "bootargs", str,
				strlen(str) + 1);
		if (err < 0) {
			DBG_DUMP("WARNING: could not set bootargs %s.\n",
				fdt_strerror(err));
			return err;
		}
	}

	if (p_linuxtmp->ramfs_addr) {
#if defined(__aarch64__)
		fdt64_t tmp = cpu_to_fdt64(p_linuxtmp->ramfs_addr);
#else
		fdt32_t tmp = cpu_to_fdt32(p_linuxtmp->ramfs_addr);
#endif
		err = fdt_setprop(fdt, nodeoffset, "linux,initrd-start", &tmp, sizeof(tmp));
		if (err < 0) {
			DBG_DUMP("WARNING: could not set initrd-start %s.\n",
				fdt_strerror(err));
			return err;
		}

#if defined(__aarch64__)
		tmp = cpu_to_fdt64(p_linuxtmp->ramfs_addr + p_linuxtmp->ramfs_size);
#else
		tmp = cpu_to_fdt32(p_linuxtmp->ramfs_addr + p_linuxtmp->ramfs_size);
#endif
		err = fdt_setprop(fdt, nodeoffset, "linux,initrd-end", &tmp, sizeof(tmp));
		if (err < 0) {
			DBG_DUMP("WARNING: could not set initrd-end %s.\n",
				fdt_strerror(err));
			return err;
		}
	}

	return 0;
}

static int fdt_read_mem_node(const char *path, unsigned long *p_addr, unsigned long *p_size)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	// get linux space
	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", path, nodeoffset);
		return -1;
	}

#if defined(__aarch64__)
	if (strncmp(path, "/nvt_memory_cfg/", strlen("/nvt_memory_cfg/")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned int *p_data = (unsigned int *)nodep;
			*p_addr = be32_to_cpu(p_data[0]);
			*p_size = be32_to_cpu(p_data[1]);
		}
	} else if (strncmp(path, "/memory", strlen("/memory")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned long *p_data = (unsigned long *)nodep;
			*p_addr = be64_to_cpu(p_data[0]);
			*p_size = be64_to_cpu(p_data[1]);
		}
	} else if (strncmp(path, "/hdal-memory/", strlen("/hdal-memory/")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned long *p_data = (unsigned long *)nodep;
			*p_addr = be64_to_cpu(p_data[2]);
			*p_size = be64_to_cpu(p_data[3]);
		}
	} else {
		DBG_ERR("failed to parse %s\n", path);
		return -1;
	}
#else
	if (strncmp(path, "/cpus/", strlen("/cpus/")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "cpu-release-addr", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access cpu-release-addr.\n");
			return -1;
		} else {
			unsigned long *p_data = (unsigned long *)nodep;
			*p_addr = be32_to_cpu(p_data[0]);
		}
	} else if (strncmp(path, "/hdal-memory/", strlen("/hdal-memory/")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned long *p_data = (unsigned long *)nodep;
			int addr_cells = fdt_address_cells(p_fdt, nodeoffset);
			if (addr_cells == 2) {
				//new, chip_id addr size
				*p_addr = be64_to_cpu(p_data[1]);
				*p_size = be64_to_cpu(p_data[2]);
			} else {
				//new, chip_id addr size
				*p_addr = be64_to_cpu(p_data[0]);
				*p_size = be64_to_cpu(p_data[1]);
			}
		}
	} else {
	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return -1;
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		*p_addr = be32_to_cpu(p_data[0]);
		*p_size = be32_to_cpu(p_data[1]);
	}
	}
#endif
	return 0;
}


static int fdt_read_reserved_mem_node(FDT_INFO *p_info)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const char path[] = "/reserved-memory";
	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", path, nodeoffset);
		return -1;
	}

	int child;
	fdt_for_each_subnode(child, p_fdt, nodeoffset) {
		LINUX_RESERVED_MEM *p = (LINUX_RESERVED_MEM *)&p_info->reserved[p_info->reserved_cnt];
		strncpy(p->name, fdt_get_name(p_fdt, child, &len), sizeof(p->name) - 1);
		unsigned long *p_data = (unsigned long *)fdt_getprop(p_fdt, child, "reg", &len);
		if (p_data) {
#if defined(__aarch64__)
			p->addr = be64_to_cpu(p_data[0]);
			p->size = be64_to_cpu(p_data[1]);
#else
			p->addr = be32_to_cpu(p_data[0]);
			p->size = be32_to_cpu(p_data[1]);
#endif
			p_info->reserved_cnt++;
			if (p_info->reserved_cnt > sizeof(p_info->reserved) / sizeof(p_info->reserved[0])) {
				DBG_ERR("linux_reserved_mem_cnt too large: %ld\n", p_info->reserved_cnt);
				return -1;
			}
		}

	}
	return 0;
}

static int fdt_get_info(FDT_INFO *p_info)
{
	int er;
	if ((er = fdt_read_mem_node(MEM_PATH_LINUX, &p_info->linux_addr, &p_info->linux_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_LINUXTMP, &p_info->linuxtmp_addr, &p_info->linuxtmp_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_BRIDGE, &p_info->bridge_addr, &p_info->bridge_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_FDT, &p_info->fdt_addr, &p_info->fdt_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_RTOS, &p_info->rtos_addr, &p_info->rtos_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_HDAL, &p_info->hdal_addr, &p_info->hdal_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_SHMEM, &p_info->shmem_addr, &p_info->shmem_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_DRAM, &p_info->dram_addr, &p_info->dram_size)) != 0) {
		return er;
	}
#if defined(__aarch64__)
	if ((er = fdt_read_mem_node(MEM_PATH_ATF, &p_info->atf_addr, &p_info->atf_size)) != 0) {
		return er;
	}
#else
	if ((er = fdt_read_mem_node(MEM_PATH_CPU_RELEASE, &p_info->cpu_release_addr, NULL)) != 0) {
		return er;
	}
#endif
	if ((er = fdt_read_reserved_mem_node(p_info)) != 0) {
		return er;
	}
	if ((er = fdt_read_reserved_mem_node(p_info)) != 0) {
		return er;
	}
	return 0;
}


static int create_new_fdt(LINUXTMP_PARTITION *p_linuxtmp)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int new_size = ALIGN_CEIL(fdt_totalsize(p_fdt) + CONFIG_SYS_FDT_PAD, 0x1000);

	if (p_linuxtmp->tmp2_begin) {
		p_linuxtmp->fdt_addr = p_linuxtmp->tmp2_curr;
		p_linuxtmp->fdt_size = new_size;
		p_linuxtmp->tmp2_curr += new_size;
	} else {
		p_linuxtmp->fdt_addr = p_linuxtmp->tmp_curr;
		p_linuxtmp->fdt_size = new_size;
		p_linuxtmp->tmp_curr += new_size;

	}

	if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
		DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
		return -1;
	}

	int err = fdt_open_into(p_fdt, (void *)p_linuxtmp->fdt_addr, new_size);
	if (err != 0) {
		DBG_ERR("fdt move failed!");
		return -1;
	}

	return 0;
}

static void *gzalloc_full(void *x, unsigned items, unsigned size)
{
	size *= items;
	void *p = malloc(size);
	if (p) {
		memset(p, 0, size);
	}
	return p;
}

static void gzfree_full(void *x, void *addr, unsigned nb)
{
	free(addr);
}

#if (CFG_INDEP_RAMDISK)
static void *thread_ungzip_ramdisk(void *ptr)
{
	if (ungzip_input.method == 0) {
	int err;
	z_stream stream = {0};
	stream.next_in = (z_const Bytef *)ungzip_input.in;
	stream.avail_in = ungzip_input.insize;
	stream.next_out = (z_const Bytef *)ungzip_input.out;
	stream.avail_out = ungzip_input.outsize;
	stream.zalloc = (alloc_func)gzalloc_full;
	stream.zfree = (free_func)gzfree_full;
	stream.opaque = (voidpf)0;
	err = inflateInit(&stream);
	if (err != Z_OK) {
		DBG_ERR("Failed to inflateInit, err = %d\r\n", err);
		inflateEnd(&stream);
		pthread_exit((void *) -1);
		return NULL;
	}

	err = inflate(&stream, Z_NO_FLUSH);

	inflateEnd(&stream);

	if (err == Z_STREAM_END) {
		pthread_exit((void *)0);
		return NULL;
	}
	} else {
		LZ_Uncompress((void *)ungzip_input.in, (void *)ungzip_input.out, ungzip_input.insize);
	}

	pthread_exit((void *) -1);
	return NULL;
}

static void *thread_do_nothing_ramdisk(void *ptr)
{
	pthread_exit((void *) -1);
	return NULL;
}
#endif

#if (CFG_INDEP_RAMDISK)
static int load_ramdisk_partial(LINUXTMP_PARTITION *p_linuxtmp)
{
	FWSRV_CMD cmd = {0};
	MEM_RANGE mem_range = {0};
	FWSRV_FASTLOAD fastload = {0};

	unsigned int comp_ramfs_addr = p_linuxtmp->tmp_curr;
	unsigned int comp_ramfs_size = CFG_RAMFS_COMP_MAX_SIZE;
	p_linuxtmp->tmp_curr += comp_ramfs_size;

	if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
		DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
		return -1;
	}

	fastload.pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_ROOTFS);
	fastload.MemComp.addr = comp_ramfs_addr;
	fastload.MemComp.size = comp_ramfs_size;
	if (p_linuxtmp->tmp2_begin) {
		fastload.MemUnComp.addr = p_linuxtmp->tmp2_curr;
		fastload.MemUnComp.size = p_linuxtmp->tmp2_end - p_linuxtmp->tmp2_curr;
	} else {
		fastload.MemUnComp.addr = p_linuxtmp->tmp_curr;
		fastload.MemUnComp.size = p_linuxtmp->tmp_end - p_linuxtmp->tmp_curr;
	}

	cmd.Idx = FWSRV_CMD_IDX_FASTLOAD;
	cmd.In.pData = &fastload;
	cmd.In.uiNumByte = sizeof(fastload);
	cmd.Out.pData = &mem_range;
	cmd.Out.uiNumByte = sizeof(mem_range);
	cmd.Prop.bExitCmdFinish = TRUE;

	FWSRV_ER er;
	if ((er = FwSrv_Open()) != FWSRV_ER_OK) {
		DBG_ERR("FwSrv_Open failed!\r\n");
		return -1;
	}

	if ((er = FwSrv_Cmd(&cmd)) != FWSRV_ER_OK) {
		DBG_ERR("FwSrv_Cmd failed!\r\n");
		return -1;
	}

	FwSrv_Close();

	p_linuxtmp->ramfs_addr = mem_range.addr;
	p_linuxtmp->ramfs_size = mem_range.size;

	if (p_linuxtmp->tmp2_begin) {
		p_linuxtmp->tmp2_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);
	} else {
		p_linuxtmp->tmp_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);
	}

	return 0;
}
#endif

#if (CFG_INDEP_RAMDISK)
static int load_ramdisk_full(LINUXTMP_PARTITION *p_linuxtmp)
{
	unsigned long bfc_data = p_linuxtmp->tmp_curr;
	STORAGE_OBJ *p_strg = EMB_GETSTRGOBJ(STRG_OBJ_FW_ROOTFS);
#if defined(_NVT_RAMDISK_COMPRESS_NONE_)
	int compressed = 0;
#elif defined(_NVT_RAMDISK_COMPRESS_GZ_) || defined(_NVT_RAMDISK_COMPRESS_LZ77_)
	int compressed = 1;
#else
# error "nvt-info.dtsi not defined NVT_RAMDISK_COMPRESS"
#endif

	if (p_strg == NULL) {
		DBG_ERR("failed to get STRG_OBJ_FW_ROOTFS");
		return -1;
	}

	if (compressed) {

		//load 1st block
		p_strg->Lock();
		p_strg->Open();
		if (p_strg->RdSectors((INT8 *)bfc_data, 0, 1) != 0) {
			DBG_ERR("RdSectors for ramdisk 1st block\r\n");
			return -1;
		}

		NVTPACK_BFC_HDR *p_hdr = (NVTPACK_BFC_HDR *)bfc_data;
		if (p_hdr->uiFourCC != MAKEFOURCC('B', 'C', 'L', '1')) {
			DBG_ERR("BCL1 header is wrong\r\n");
			return -1;
		}

		unsigned int bfc_size = be32_to_cpu(p_hdr->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);

		unsigned long comp_ramfs_addr = bfc_data + sizeof(NVTPACK_BFC_HDR);
		unsigned int comp_ramfs_size = be32_to_cpu(p_hdr->uiSizeComp);
		p_linuxtmp->tmp_curr += ALIGN_CEIL_64(bfc_size);
		if (p_linuxtmp->tmp2_begin) {
			p_linuxtmp->ramfs_addr = p_linuxtmp->tmp2_curr;
			p_linuxtmp->ramfs_size = be32_to_cpu(p_hdr->uiSizeUnComp);
			p_linuxtmp->tmp2_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);
			if (p_linuxtmp->tmp2_curr >= p_linuxtmp->tmp2_end) {
				DBG_ERR("rootfs is too large, need reduce %d\n", p_linuxtmp->tmp2_curr - p_linuxtmp->tmp2_end);
				return -1;
			}
		} else {
			p_linuxtmp->ramfs_addr = p_linuxtmp->tmp_curr;
			p_linuxtmp->ramfs_size = be32_to_cpu(p_hdr->uiSizeUnComp);
			p_linuxtmp->tmp_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);
			if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
				DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
				return -1;
			}
		}

		// load rest of image data
		unsigned int block_size = 0;
		p_strg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&block_size, 0);
		if (block_size == 0) {
			DBG_ERR("block_size is 0.\n");
			return -1;
		}
		unsigned int remain_blocks = ALIGN_CEIL(bfc_size - block_size, block_size) / block_size;
		if (p_strg->RdSectors((INT8 *)(bfc_data + block_size), 1, remain_blocks) != 0) {
			DBG_ERR("RdSectors for ramdisk\r\n");
			return -1;
		}
		p_strg->Close();
		p_strg->Unlock();

		//uncompress
		ungzip_input.in = (unsigned char *)comp_ramfs_addr;
		ungzip_input.out = (unsigned char *)p_linuxtmp->ramfs_addr;
		ungzip_input.insize = comp_ramfs_size;
		ungzip_input.outsize = p_linuxtmp->ramfs_size;
#if defined(_NVT_RAMDISK_COMPRESS_GZ_)
		ungzip_input.method = 0; // gz
#elif defined(_NVT_RAMDISK_COMPRESS_LZ77_)
		ungzip_input.method = 1; // lz77
#endif
		int pthread_ret = pthread_create(&handle_unzip_ramdisk, NULL, thread_ungzip_ramdisk, NULL);
		if (0 != pthread_ret) {
			DBG_ERR("create thread_ungzip_ramdisk failed, ret %d\r\n", pthread_ret);
			return -1;
		}
		return 0;
	} else {
		p_linuxtmp->tmp_curr += ALIGN_CEIL_64(sizeof(image_header_t));
		p_linuxtmp->ramfs_addr = p_linuxtmp->tmp_curr;

		uintptr_t  load_addr = p_linuxtmp->ramfs_addr  - sizeof(image_header_t);

		//load 1st block
		p_strg->Lock();
		p_strg->Open();
		if (p_strg->RdSectors((INT8 *)load_addr, 0, 1) != 0) {
			DBG_ERR("RdSectors for ramdisk 1st block\r\n");
			return -1;
		}

		image_header_t *p_hdr = (image_header_t *)load_addr;
		if (be32_to_cpu(p_hdr->ih_magic) != IH_MAGIC) {
			DBG_ERR("IH_MAGIC is wrong\r\n");
			return -1;
		}

		//IH_TYPE_RAMDISK = 3
		if (p_hdr->ih_type != 3) {
			DBG_ERR("ih_type (0x%X) != IH_TYPE_RAMDISK\r\n", (int)p_hdr->ih_type);
			return -1;
		}
		// load rest of image data
		unsigned int image_size = be32_to_cpu(p_hdr->ih_size) + sizeof(image_header_t);
		p_linuxtmp->ramfs_size = be32_to_cpu(p_hdr->ih_size);
		p_linuxtmp->tmp_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);

		unsigned int block_size = 0;
		p_strg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&block_size, 0);
		if (block_size == 0) {
			DBG_ERR("block_size is 0.\n");
			return -1;
		}
		unsigned int remain_blocks = ALIGN_CEIL(image_size - block_size, block_size) / block_size;
		if (p_strg->RdSectors((INT8 *)(load_addr + block_size), 1, remain_blocks) != 0) {
			DBG_ERR("RdSectors for linux\r\n");
			return -1;
		}
		p_strg->Close();
		p_strg->Unlock();

		int pthread_ret = pthread_create(&handle_unzip_ramdisk, NULL, thread_do_nothing_ramdisk, NULL);
		if (0 != pthread_ret) {
			DBG_ERR("create thread_do_nothing_ramdisk failed, ret %d\r\n", pthread_ret);
			return -1;
		}
		return 0;
	}
}
#endif

static int find_index_nvtpack(LINUXTMP_PARTITION *p_linuxtmp, char *partition_name)
{
#if (_PACKAGE_FILESYS_ && _PACKAGE_SDCARD_)
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL\r\n");
		return -1;
	}
	// find out nvtpack index to partition_rootfs
	int i;
	char *path_fdt = NULL;
	char *pathes_fdt[] = {
		"/nor/nvtpack",
		"/nand/nvtpack",
		"mmc2/nvtpack",
	};

	for (i = 0; i < 3; i++) {
		int nodeoffset = fdt_path_offset(p_fdt, pathes_fdt[i]);
		if (nodeoffset >= 0) {
			path_fdt = pathes_fdt[i];
			break;
		}
	}
	if (path_fdt == NULL) {
		DBG_ERR("unable to find nvtpack in fdt on sd card\r\n");
		return -1;
	}

	for (i = 0; i < 64; i++) {
		char path[64] = {0};
		sprintf(path, "%s/index/id%d", path_fdt, i);
		int nodeoffset = fdt_path_offset(p_fdt, path);
		if (nodeoffset < 0) {
			continue;
		}
		int len = 0;
		const void *nodep = fdt_getprop(p_fdt, nodeoffset, "partition_name", &len);
		if (nodep == NULL || len == 0) {
			continue;
		}
		if (strncmp((const char *)nodep, partition_name, len) == 0) {
			return i;
		}
	}
	return -1;

#else
	DBG_ERR("unsupported\r\n");
	return -1;
#endif
}

#if (CFG_INDEP_RAMDISK)
static int load_ramdisk_nvtpack(LINUXTMP_PARTITION *p_linuxtmp)
{
#if (_PACKAGE_FILESYS_ && _PACKAGE_SDCARD_)
	NVTPACK_ER er;
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_MEM mem_in = {(void *)p_linuxtmp->nvtpack_addr, (unsigned int)p_linuxtmp->nvtpack_size};

	int nvtpack_rootfs_index = find_index_nvtpack(p_linuxtmp, "rootfs");
	if (nvtpack_rootfs_index == -1) {
		DBG_ERR("unable to find rootfs-nvtpack-index in fdt on sd card\r\n");
		return -1;
	}

	NVTPACK_MEM rootfs_mem;
	np_get_input.id = nvtpack_rootfs_index;
	np_get_input.mem = mem_in;
	er = nvtpack_get_partition(&np_get_input, &rootfs_mem);
	if (er == NVTPACK_ER_NOT_FOUND) {
		DBG_ERR("unable to find rootfs in all-in-one\r\n");
		return -1;
	}

	NVTPACK_CHKSUM_HDR *p_sum = (NVTPACK_CHKSUM_HDR *)rootfs_mem.p_data;
	if (p_sum->uiFourCC != MAKEFOURCC('C', 'K', 'S', 'M')) {
		DBG_ERR("CKSM header is wrong on sd card\r\n");
		return -1;
	}

#if defined(_NVT_RAMDISK_COMPRESS_NONE_)
	p_linuxtmp->tmp_curr += ALIGN_CEIL_64(sizeof(image_header_t));
	p_linuxtmp->ramfs_addr = p_linuxtmp->tmp_curr;

	uintptr_t  load_addr = p_linuxtmp->ramfs_addr  - sizeof(image_header_t);

	image_header_t *p_hdr = (image_header_t *)&p_sum[1];
	if (be32_to_cpu(p_hdr->ih_magic) != IH_MAGIC) {
		DBG_ERR("IH_MAGIC is wrong\r\n");
		return -1;
	}
	//IH_TYPE_RAMDISK = 3
	if (p_hdr->ih_type != 3) {
		DBG_ERR("ih_type != IH_TYPE_RAMDISK\r\n");
		return -1;
	}

	unsigned int image_size = be32_to_cpu(p_hdr->ih_size) + sizeof(image_header_t);
	p_linuxtmp->ramfs_size = be32_to_cpu(p_hdr->ih_size);
	p_linuxtmp->tmp_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);

	memcpy((void *)load_addr, (void *)p_hdr, image_size);
	return 0;
#elif defined(_NVT_RAMDISK_COMPRESS_GZ_)
	NVTPACK_BFC_HDR *p_hdr = (NVTPACK_BFC_HDR *)&p_sum[1];
	if (p_hdr->uiFourCC != MAKEFOURCC('B', 'C', 'L', '1')) {
		DBG_ERR("rootfs BCL1 header is wrong on sd card\r\n");
		return -1;
	}

	unsigned long comp_ramfs_addr = (unsigned long)&p_hdr[1];
	unsigned int comp_ramfs_size = be32_to_cpu(p_hdr->uiSizeComp);
	p_linuxtmp->ramfs_addr = p_linuxtmp->tmp_curr;
	p_linuxtmp->ramfs_size = be32_to_cpu(p_hdr->uiSizeUnComp);
	p_linuxtmp->tmp_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);

	if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
		DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
		return -1;
	}

	//uncompress
	ungzip_input.in = (unsigned char *)comp_ramfs_addr;
	ungzip_input.out = (unsigned char *)p_linuxtmp->ramfs_addr;
	ungzip_input.insize = comp_ramfs_size;
	ungzip_input.outsize = p_linuxtmp->ramfs_size;

	int pthread_ret = pthread_create(&handle_unzip_ramdisk, NULL, thread_ungzip_ramdisk, NULL);
	if (0 != pthread_ret) {
		DBG_ERR("create thread_ungzip_ramdisk failed, ret %d\r\n", pthread_ret);
		return -1;
	}

	return 0;
#else
	DBG_ERR("unsupported NVT_RAMDISK_COMPRESS\r\n");
	return -1;
#endif
#else
	DBG_ERR("unsupported\r\n");
	return -1;
#endif
}
#endif

#if (CFG_INDEP_RAMDISK)
static int load_ramdisk(LINUXTMP_PARTITION *p_linuxtmp)
{
	switch (ramdisk_methold) {
	case RAMDISK_METHOD_PARTIAL:
		return load_ramdisk_partial(p_linuxtmp);
	case RAMDISK_METHOD_FULL:
		return load_ramdisk_full(p_linuxtmp);
	case RAMDISK_METHOD_NVTPACK:
		return load_ramdisk_nvtpack(p_linuxtmp);
	default:
		DBG_ERR("unknown method: %d\r\n", ramdisk_methold);
		return -1;
	}
}
#endif

#if (CFG_INDEP_RAMDISK)
static int wait_ramdisk(void)
{
#if !defined(_NVT_RAMDISK_COMPRESS_NONE_)
	int join_ret;
	int pthread_ret;
#endif
	switch (ramdisk_methold) {
	case RAMDISK_METHOD_PARTIAL:
		return 0; //partial load + parital decompress have finished on load_ramdisk
	case RAMDISK_METHOD_FULL:
	case RAMDISK_METHOD_NVTPACK:
#if defined(_NVT_RAMDISK_COMPRESS_NONE_)
#elif defined(_NVT_RAMDISK_COMPRESS_GZ_) || defined(_NVT_RAMDISK_COMPRESS_LZ77_)
		pthread_ret = pthread_join(handle_unzip_ramdisk, (void *)&join_ret);
		if (0 != pthread_ret) {
			DBG_ERR("handle_unzip_ramdisk pthread_join failed, ret %d\r\n", pthread_ret);
			return -1;
		}
#endif
		return 0;
	default:
		DBG_ERR("unknown method: %d\r\n", ramdisk_methold);
		return -1;
	}
}
#endif

static int make_bootargs(LINUXTMP_PARTITION *p_linuxtmp, FDT_INFO *p_fdt_info, unsigned int bootts_begin)
{
	//static char bootargs[] = "root=/dev/ram0 rootfstype=ramfs rdinit=/linuxrc bootts=568047,1720128 resume_addr=0x00007e88 user_debug=0xff";

	// read extend bootarg in fdt
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset = fdt_path_offset(p_fdt, "/chosen");
	const char *bootargs = (const char *)fdt_getprop(p_fdt, nodeoffset, "bootargs", &len);

	p_linuxtmp->bootargs_addr = p_linuxtmp->tmp_curr;
	p_linuxtmp->bootargs_size = CFG_BOOTARG_MAX_LEN;
	p_linuxtmp->tmp_curr += p_linuxtmp->bootargs_size;

	if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
		DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
		return -1;
	}

	int rtos_boot = (is_fastboot())? 2: 1;

	memset((char *)p_linuxtmp->bootargs_addr, 0, p_linuxtmp->bootargs_size);
	snprintf((char *)p_linuxtmp->bootargs_addr, CFG_BOOTARG_MAX_LEN - 1, CONFIG_BOOTARGS "bootts=%u,%u user_debug=0xff rtos_boot=%d %s ", bootts_begin, (unsigned int)hwclock_get_counter(), rtos_boot, bootargs);

#if defined(_EMBMEM_SPI_NOR_)
	SHMINFO *p_shm = (SHMINFO *)p_fdt_info->shmem_addr;
	if (p_shm->boot.LdCtrl2 & LDCF_DTR80_BOOT) {
		char strtmp[32] = {0};
                if (snprintf((char *)strtmp, sizeof(strtmp), " flash_dtr=on") >= (int)sizeof(strtmp)) {
                        DBG_ERR("strtmp size too small\n");
                } else {
			strncat((char *)p_linuxtmp->bootargs_addr, strtmp, CFG_BOOTARG_MAX_LEN - 1);
                }
	}
#endif

	return 0;
}

#if defined(_ENCRYPT_KERNEL_ON_)
static int load_linux_from_flash(LINUXTMP_PARTITION *p_linuxtmp, FDT_INFO *p_fdt_info)
{
#if defined(_NVT_LINUX_COMPRESS_GZ_) //partital load + partial decompress
	DBG_ERR("unsupported secured-gz-linux\n");
	return -1;
#elif defined(_NVT_LINUX_COMPRESS_NONE_) // only full load
	uintptr_t  load_addr = p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET - sizeof(image_header_t) - CFG_MULTI_MKIMAGE_LEN;
	STORAGE_OBJ *p_strg = EMB_GETSTRGOBJ(STRG_OBJ_FW_LINUX);
	//load 1st block
	p_strg->Lock();
	p_strg->Open();
	if (p_strg->RdSectors((INT8 *)load_addr, 0, 1) != 0) {
		DBG_ERR("RdSectors for linux 1st block\r\n");
		return -1;
	}

	// load rest of image data
	int bin_size = ((HEADINFO *)(load_addr))->BinLength;
	unsigned int block_size = 0;
	p_strg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&block_size, 0);
	if (block_size == 0) {
		DBG_ERR("block_size is 0.\n");
		return -1;
	}
	unsigned int remain_blocks = ALIGN_CEIL(bin_size - block_size, block_size) / block_size;
	if (p_strg->RdSectors((INT8 *)(load_addr + block_size), 1, remain_blocks) != 0) {
		DBG_ERR("RdSectors for linux\r\n");
		return -1;
	}
	p_strg->Close();
	p_strg->Unlock();

	//decrypt
	FWSRV_CMD cmd = {0};
	FWSRV_DECRYPT decrypt = {0};
#if defined(_SIGNATUREMETHOD_NONE_)
	decrypt.Signature = FWSRV_DECRYPT_SIGNATURE_NONE;
#elif defined(_SIGNATUREMETHOD_AES128_)
	decrypt.Signature = FWSRV_DECRYPT_SIGNATURE_AES128;
#elif defined(_SIGNATUREMETHOD_RSA2048_)
	decrypt.Signature = FWSRV_DECRYPT_SIGNATURE_RSA2048;
#elif defined(_SIGNATUREMETHOD_ECDSA_)
	decrypt.Signature = FWSRV_DECRYPT_SIGNATURE_ECDSA;
#else
#error "SIGNATUREMETHOD required on nvt-info.dtsi"
#endif
	decrypt.EncryptedMem.addr = load_addr;
	decrypt.EncryptedMem.size = bin_size;
	cmd.Idx = FWSRV_CMD_IDX_DECRYPT;
	cmd.In.pData = &decrypt;
	cmd.In.uiNumByte = sizeof(decrypt);
	cmd.Prop.bExitCmdFinish = TRUE;

	FWSRV_ER er;
	if ((er = FwSrv_Open()) != FWSRV_ER_OK) {
		DBG_ERR("FwSrv_Open failed!\r\n");
		return -1;
	}

	if ((er = FwSrv_Cmd(&cmd)) != FWSRV_ER_OK) {
		DBG_ERR("FwSrv_Cmd failed!\r\n");
		return -1;
	}

	FwSrv_Close();

	image_header_t *p_hdr = (image_header_t *)load_addr;
	if (be32_to_cpu(p_hdr->ih_magic) != IH_MAGIC) {
		DBG_ERR("IH_MAGIC is wrong\r\n");
		return -1;
	}

#if defined(__aarch64__)
	//IH_TYPE_KERNEL = 2
	if (p_hdr->ih_type != 2) {
		DBG_ERR("ih_type (0x%X) != IH_TYPE_KERNEL\r\n", (int)p_hdr->ih_type);
		return -1;
	}
#else
	//IH_TYPE_MULTI = 4
	if (p_hdr->ih_type != 4) {
		DBG_ERR("ih_type (0x%X) != IH_TYPE_MULTI\r\n", (int)p_hdr->ih_type);
		return -1;
	}
#endif

	return 0;
#else
	DBG_ERR("unsupported NVT_LINUX_COMPRESS mode.\r\n");
	return -1;
#endif
}
#else
static int load_linux_from_flash(LINUXTMP_PARTITION *p_linuxtmp, FDT_INFO *p_fdt_info)
{
#if defined(_NVT_LINUX_COMPRESS_GZ_) //partital load + partial decompress
	FWSRV_CMD cmd = {0};
	MEM_RANGE mem_range = {0};
	FWSRV_FASTLOAD fastload = {0};

	p_linuxtmp->lz_linux_addr = p_linuxtmp->tmp_curr;
	p_linuxtmp->lz_linux_size = CFG_LINUX_COMP_MAX_SIZE;
	p_linuxtmp->tmp_curr += p_linuxtmp->lz_linux_size;

	if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
		DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
		return -1;
	}

	fastload.pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_LINUX);
	fastload.MemComp.addr = p_linuxtmp->lz_linux_addr;
	fastload.MemComp.size = p_linuxtmp->lz_linux_size;
	fastload.MemUnComp.addr = p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET;
	fastload.MemUnComp.size = p_fdt_info->linux_size - CFG_LINUX_START_OFFSET;

	cmd.Idx = FWSRV_CMD_IDX_FASTLOAD;
	cmd.In.pData = &fastload;
	cmd.In.uiNumByte = sizeof(fastload);
	cmd.Out.pData = &mem_range;
	cmd.Out.uiNumByte = sizeof(mem_range);
	cmd.Prop.bExitCmdFinish = TRUE;

	FWSRV_ER er;
	if ((er = FwSrv_Open()) != FWSRV_ER_OK) {
		DBG_ERR("FwSrv_Open failed!\r\n");
		return -1;
	}

	if ((er = FwSrv_Cmd(&cmd)) != FWSRV_ER_OK) {
		DBG_ERR("FwSrv_Cmd failed!\r\n");
		return -1;
	}

	FwSrv_Close();
	return 0;
#elif defined(_NVT_LINUX_COMPRESS_NONE_) // only full load
	uintptr_t  load_addr = p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET - sizeof(image_header_t) - CFG_MULTI_MKIMAGE_LEN;
	STORAGE_OBJ *p_strg = EMB_GETSTRGOBJ(STRG_OBJ_FW_LINUX);
	//load 1st block
	p_strg->Lock();
	p_strg->Open();
	if (p_strg->RdSectors((INT8 *)load_addr, 0, 1) != 0) {
		DBG_ERR("RdSectors for linux 1st block\r\n");
		return -1;
	}

	image_header_t *p_hdr = (image_header_t *)load_addr;
	if (be32_to_cpu(p_hdr->ih_magic) != IH_MAGIC) {
		DBG_ERR("IH_MAGIC is wrong\r\n");
		return -1;
	}

#if defined(__aarch64__)
	//IH_TYPE_KERNEL = 2
	if (p_hdr->ih_type != 2) {
		DBG_ERR("ih_type (0x%X) != IH_TYPE_KERNEL\r\n", (int)p_hdr->ih_type);
		return -1;
	}
#else
	//IH_TYPE_MULTI = 4
	if (p_hdr->ih_type != 4) {
		DBG_ERR("ih_type (0x%X) != IH_TYPE_MULTI\r\n", (int)p_hdr->ih_type);
		return -1;
	}
#endif
	// load rest of image data
	unsigned int image_size = be32_to_cpu(p_hdr->ih_size) + sizeof(image_header_t);
	unsigned int block_size = 0;
	p_strg->GetParam(STRG_GET_BEST_ACCESS_SIZE, (ULONG)&block_size, 0);
	if (block_size == 0) {
		DBG_ERR("block_size is 0.\n");
		return -1;
	}
	unsigned int remain_blocks = ALIGN_CEIL(image_size - block_size, block_size) / block_size;
	if (p_strg->RdSectors((INT8 *)(load_addr + block_size), 1, remain_blocks) != 0) {
		DBG_ERR("RdSectors for linux\r\n");
		return -1;
	}
	p_strg->Close();
	p_strg->Unlock();
	return 0;
#else
	DBG_ERR("unsupported NVT_LINUX_COMPRESS mode.\r\n");
	return -1;
#endif

}
#endif

static int load_linux_from_nvtpack(LINUXTMP_PARTITION *p_linuxtmp, FDT_INFO *p_fdt_info)
{
	NVTPACK_ER er;
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_MEM mem_in = {(void *)p_linuxtmp->nvtpack_addr, (unsigned int)p_linuxtmp->nvtpack_size};

	int nvtpack_linux_index = find_index_nvtpack(p_linuxtmp, "kernel");
	if (nvtpack_linux_index == -1) {
		DBG_ERR("unable to find linux-nvtpack-index in fdt on sd card\r\n");
		return -1;
	}

	NVTPACK_MEM linux_mem;
	np_get_input.id = nvtpack_linux_index;
	np_get_input.mem = mem_in;
	er = nvtpack_get_partition(&np_get_input, &linux_mem);
	if (er == NVTPACK_ER_NOT_FOUND) {
		DBG_ERR("unable to find linux in all-in-one\r\n");
		return -1;
	}

#if defined(_NVT_LINUX_COMPRESS_GZ_) //partital load + partial decompress
	NVTPACK_BFC_HDR *p_hdr = (NVTPACK_BFC_HDR *)linux_mem.p_data;
	if (p_hdr->uiFourCC != MAKEFOURCC('B', 'C', 'L', '1')) {
		DBG_ERR("linux BCL1 header is wrong on sd card\r\n");
		return -1;
	}

	int err;
	z_stream stream = {0};
	stream.next_in = (z_const Bytef *)&p_hdr[1];
	stream.avail_in = be32_to_cpu(p_hdr->uiSizeComp);
	stream.next_out = (z_const Bytef *)p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET;
	stream.avail_out = be32_to_cpu(p_hdr->uiSizeUnComp);
	stream.zalloc = (alloc_func)gzalloc_full;
	stream.zfree = (free_func)gzfree_full;
	stream.opaque = (voidpf)0;
	err = inflateInit(&stream);
	if (err != Z_OK) {
		DBG_ERR("Failed to inflateInit, err = %d\r\n", err);
		inflateEnd(&stream);
		return -1;
	}

	err = inflate(&stream, Z_NO_FLUSH);

	inflateEnd(&stream);

	if (err == Z_STREAM_END) {
		return 0;
	}

	return -1;
#elif defined(_NVT_LINUX_COMPRESS_NONE_) // only full load
	uintptr_t load_addr = p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET - sizeof(image_header_t) - CFG_MULTI_MKIMAGE_LEN;
	image_header_t *p_hdr = (image_header_t *)(linux_mem.p_data);
	if (be32_to_cpu(p_hdr->ih_magic) != IH_MAGIC) {
		DBG_ERR("IH_MAGIC is wrong\r\n");
		return -1;
	}
	//IH_TYPE_MULTI = 4
	if (p_hdr->ih_type != 4) {
		DBG_ERR("ih_type != IH_TYPE_MULTI\r\n");
		return -1;
	}
	// load rest of image data
	unsigned int image_size = be32_to_cpu(p_hdr->ih_size) + sizeof(image_header_t);
	memcpy((void *)load_addr, linux_mem.p_data, image_size);
	return 0;
#else
	DBG_ERR("unsupported NVT_LINUX_COMPRESS mode.\r\n");
	return -1;
#endif

}

static int load_fdtapp_from_nvtpack(LINUXTMP_PARTITION *p_linuxtmp, FDT_INFO *p_fdt_info)
{
	NVTPACK_ER er;
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_MEM mem_in = {(void *)p_linuxtmp->nvtpack_addr, (unsigned int)p_linuxtmp->nvtpack_size};

	int nvtpack_fdtapp_index = find_index_nvtpack(p_linuxtmp, "fdt.app");
	if (nvtpack_fdtapp_index == -1) {
		DBG_ERR("unable to find fdt.app-nvtpack-index in fdt on sd card\r\n");
		return -1;
	}

	NVTPACK_MEM fdtapp_mem;
	np_get_input.id = nvtpack_fdtapp_index;
	np_get_input.mem = mem_in;
	er = nvtpack_get_partition(&np_get_input, &fdtapp_mem);
	if (er == NVTPACK_ER_NOT_FOUND) {
		DBG_ERR("unable to find linux in all-in-one\r\n");
		return -1;
	}

	er = fdt_set_app(fdtapp_mem.p_data);
	if (er != 0) {
		return er;
	}
	return 0;
}

static int load_linux(LINUXTMP_PARTITION *p_linuxtmp, FDT_INFO *p_fdt_info)
{
	SHMINFO *p_shm = (SHMINFO *)p_fdt_info->shmem_addr;
	if ((p_shm->boot.LdCtrl2 & LDCF_BOOT_CARD) == 0) {
		return load_linux_from_flash(p_linuxtmp, p_fdt_info);
	} else {
		// also read fdtapp
		load_fdtapp_from_nvtpack(p_linuxtmp, p_fdt_info);
		return load_linux_from_nvtpack(p_linuxtmp, p_fdt_info);
	}
}

static int check_mem_overlap(FDT_INFO *p_fdt_info)
{
	//rule1: fdt, rtos, bridge must be in order and continuous
	if (p_fdt_info->fdt_addr + p_fdt_info->fdt_size != p_fdt_info->rtos_addr ||
		p_fdt_info->rtos_addr + p_fdt_info->rtos_size != p_fdt_info->bridge_addr) {
		DBG_ERR("in nvt_memory_cfg, the mem of fdt, rtos, bridge must be in order.\r\n");
		return -1;
	}

	//reule 2: (fdt, rtos, bridge cannot be overlapped with linux)
	unsigned int mem_begin = p_fdt_info->fdt_addr;
	unsigned int mem_end = mem_begin + p_fdt_info->fdt_size + p_fdt_info->rtos_size + p_fdt_info->bridge_size;
	unsigned int linux_begin = p_fdt_info->linux_addr;
	unsigned int linux_end = linux_begin + p_fdt_info->linux_size;
	if (linux_end > mem_begin && linux_begin < mem_end) {
		DBG_ERR("linux mem is overlapped with (fdt+rtos+bridge)\r\n");
		return -1;
	}

	//reule 3: (fdt, rtos, bridge cannot be overlapped with hdal)
	unsigned int hdal_begin = p_fdt_info->hdal_addr;
	unsigned int hdal_end = hdal_begin + p_fdt_info->hdal_size;
	if (hdal_end > mem_begin && hdal_begin < mem_end) {
		DBG_ERR("hdal mem is overlapped with (fdt+rtos+bridge)\r\n");
		return -1;
	}

	return 0;
}

static int merge_node(void *fdt_dst, void *fdt_src, const char *node_name)
{
	int er;
	int len;
	int child;

	if (fdt_dst == NULL || fdt_src == NULL) {
		printf("merge_node: one of fdt is NULL\n");
		return -1;
	}

	int src_parent = fdt_path_offset(fdt_src, node_name);
	int dst_parent = fdt_path_offset(fdt_dst, node_name);

	if (dst_parent <= 0) {
		printf("unable to find %s on dst fdt.\n", node_name);
		return -1;
	}

	if (src_parent <= 0) {
		printf("unable to find %s on src fdt.\n", node_name);
		return -1;
	}

	fdt_for_each_property_offset(child, fdt_src, src_parent) {
		const struct fdt_property *fdt_prop = fdt_get_property_by_offset(fdt_src, child, &len);
		if (fdt_prop == NULL) {
			printf("failed to fdt_get_property_by_offset\n");
			return -1;
		}
		const char *name = fdt_string(fdt_src, fdt32_to_cpu(fdt_prop->nameoff));
		len = fdt32_to_cpu(fdt_prop->len);
		const void *nodep = fdt_prop->data;
		if ((er = fdt_setprop(fdt_dst, dst_parent, name, nodep, len)) != 0) {
			printf("failed to fdt_setprop %s, er = %d\n", name, er);
			return er;
		}
	}

	fdt_for_each_subnode(child, fdt_src, src_parent) {
		const char *name = fdt_get_name(fdt_src, child, &len);
		if (fdt_find_or_add_subnode(fdt_dst, dst_parent, name) <= 0) {
			return -1;
		}
		char *path = (char *)malloc(256);
		if (path == NULL) {
			printf("unable to malloc.\n");
			return -1;
		}
		memset(path, 0, 256);
		sprintf(path, "%s/%s", node_name, name);
		if ((er = merge_node(fdt_dst, fdt_src, path)) != 0) {
			free(path);
			return er;
		}
		free(path);
	}
	return 0;
}

#if (_PACKAGE_FILESYS_ && _PACKAGE_SDCARD_)
static int load_nvtpack(LINUXTMP_PARTITION *p_linuxtmp)
{
#if defined(_NVT_EMULATION_)
	vos_util_delay_ms(3000); // on FPGA, fs mounting slowly
#endif
	//polling filesys ready first
	int n_retry = 5; // 5 sec
	while (n_retry--) {
		if (FileSys_WaitFinishEx('A') == FST_STA_OK) {
			break;
		}
		vos_util_delay_ms(1000);
	}

	char bint_path[] = "A:\\" _BIN_NAME_T_ ".bin";
	char bin_path[] = "A:\\" _BIN_NAME_ ".bin";
	char *nvtpack_path = bint_path;

	FST_FILE h_file = FileSys_OpenFile(bint_path, FST_OPEN_READ);

	if (h_file == 0) {
		h_file = FileSys_OpenFile(bin_path, FST_OPEN_READ);
		nvtpack_path = bin_path;
	}

	if (h_file == 0) {
		DBG_ERR("unable to open %s or %s\r\n", bint_path, bin_path);
		return -1;
	}


	int nvtpack_len = FileSys_GetFileLen(nvtpack_path);



	unsigned int alloc_size = ALIGN_CEIL_64(nvtpack_len);
	if (p_linuxtmp->tmp_end - p_linuxtmp->tmp_curr < alloc_size) {
		DBG_ERR("linuxtmp size too small to alloc nvtpack size %d bytes\r\n", alloc_size);
		return -1;
	}

	p_linuxtmp->nvtpack_size = alloc_size;
	p_linuxtmp->nvtpack_addr = p_linuxtmp->tmp_end - p_linuxtmp->nvtpack_size;
	p_linuxtmp->tmp_end = p_linuxtmp->nvtpack_addr; // use the bottom of linuxtmp memory

	UINT32 fw_size = (UINT32)nvtpack_len;
	INT32 fst_er = FileSys_ReadFile(h_file, (UINT8 *)p_linuxtmp->nvtpack_addr, &fw_size, 0, NULL);
	FileSys_CloseFile(h_file);
	if (fst_er != FST_STA_OK) {
		DBG_ERR("FW bin read fail\r\n");
		return -1;
	}
	return 0;
}
#endif

static int chk_mem_overlap(FDT_INFO *p_fdt_info, const char *name, uintptr_t addr, unsigned long size)
{
	int i;
	for (i = 0; i < (int)p_fdt_info->reserved_cnt; i++) {
		LINUX_RESERVED_MEM *p = (LINUX_RESERVED_MEM *)&p_fdt_info->reserved[i];
		if (IS_BIN_OVERLAP(addr, size,  p->addr, p->size)) {

			DBG_ERR("OVERLAP: %s(%lX - %lX) with %s(%lX - %lX)\n",
				name, addr, addr + size, p->name, p->addr, p->addr + p->size);
			DBG_ERR("Please move reserved memory in nvt-mem-tbl.dtsi\n");
			return -1;
		}
	}
	return 0;
}

static int get_except_irqs(int *p_list, int *p_cnt, int max_cnt)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	//const void *nodep;  /* property node pointer */

	// get linux space
	const char path[] = "/interrupt-controller";
	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", path, nodeoffset);
		return -1;
	}

	int i, n = 0;
	unsigned int *p_data = (unsigned int *)fdt_getprop(p_fdt, nodeoffset, "direction", &len);

	if (p_data == NULL) {
		DBG_ERR("unable to find /interrupt-controller/direction\n");
		return -1;
	}
	for (i = 0; i < len / (int)sizeof(int); i += 2) {
		int id = fdt32_to_cpu(p_data[i]);
		int dir = fdt32_to_cpu(p_data[i + 1]);
		if (dir != 0) {
			if (n < max_cnt) {
				p_list[n++] = id;
			} else {
				DBG_ERR("except_irq array is too small.");
				return -1;
			}
		}
	}

	*p_cnt = n;
	return 0;
}


int linuxboot_setup(LINUXBOOT_INFO *p_info)
{
	int er;

	unsigned int bootts_begin = hwclock_get_counter();

	LINUXTMP_PARTITION *p_linuxtmp = &p_info->linuxtmp;
	FDT_INFO *p_fdt_info = &p_info->fdt_info;

	if ((er = fdt_get_info(p_fdt_info)) != 0) {
		return er;
	}

	if ((er = check_mem_overlap(p_fdt_info)) != 0) {
		return er;
	}

	p_linuxtmp->tmp_begin = p_fdt_info->linuxtmp_addr;
	p_linuxtmp->tmp_curr = p_linuxtmp->tmp_begin;
	p_linuxtmp->tmp_end = p_linuxtmp->tmp_begin + p_fdt_info->linuxtmp_size;

	if (p_fdt_info->dram_size <= 0x04000000) {
		DBG_WRN("small dram.\n");
		p_linuxtmp->tmp2_begin = p_fdt_info->linux_addr + CFG_LINUX_MAX_CODE_SIZE;
		p_linuxtmp->tmp2_curr = p_linuxtmp->tmp2_begin;
		p_linuxtmp->tmp2_end = p_fdt_info->fdt_addr;
	}

	SHMINFO *p_shm = (SHMINFO *)p_fdt_info->shmem_addr;
	if (p_shm->boot.LdCtrl2 & LDCF_BOOT_CARD) {
#if (_PACKAGE_FILESYS_ && _PACKAGE_SDCARD_)
		// for boot linux from all-in-one in sd card
		if ((er = load_nvtpack(p_linuxtmp)) != 0) {
			return er;
		}
		// force load ramdisk by this method
		ramdisk_methold = RAMDISK_METHOD_NVTPACK;
#else
		DBG_ERR("for boot from sd, _PACKAGE_FILESYS_ and _PACKAGE_SDCARD_ must enable\r\n");
		return -1;
#endif
	}

	// create new fdt for nodes of bootargs and ramdisk
	if ((er = create_new_fdt(p_linuxtmp)) != 0) {
		return er;
	}

#if (CFG_INDEP_RAMDISK)
	// load ramdisk into memory and reserved memory
	if ((er = load_ramdisk(p_linuxtmp)) != 0) {
		return er;
	}
#endif

	if ((er = load_linux(p_linuxtmp, p_fdt_info)) != 0) {
		return er;
	}

#if (CFG_INDEP_RAMDISK)
	// wait ramdisk decompression thread finish (only for _NVT_LINUX_COMPRESS_NONE_)
	if ((er = wait_ramdisk()) != 0) {
		return er;
	}
#endif

	// make bootargs
	if ((er = make_bootargs(p_linuxtmp, p_fdt_info, bootts_begin)) != 0) {
		return er;
	}

	// insert bootargs and ramdisk into fdt
	if ((er = fdt_chosen(p_linuxtmp)) != 0) {
		return er;
	}

	// merge fdt.app into fdt
	if (0) {
		if ((er = merge_node((void *)p_linuxtmp->fdt_addr, fdt_get_app(), "/fastboot")) != 0) {
			printf("fdt skip merge /fastboot node\n");
		}
	}

	if ((er = chk_mem_overlap(p_fdt_info, "fdt", p_linuxtmp->fdt_addr, p_linuxtmp->fdt_size)) != 0) {
		return er;
	}
#if (CFG_INDEP_RAMDISK)
	if ((er = chk_mem_overlap(p_fdt_info, "ramfs", p_linuxtmp->ramfs_addr, p_linuxtmp->ramfs_size)) != 0) {
		return er;
	}
#endif
	if ((er = chk_mem_overlap(p_fdt_info, "bootargs", p_linuxtmp->bootargs_addr, p_linuxtmp->bootargs_size)) != 0) {
		return er;
	}

	return 0;
}

int linuxboot_set_flash_preload(LINUXBOOT_INFO *p_info)
{
	int value = 1;
	void *fdt = (void *)p_info->linuxtmp.fdt_addr;

	int nodeoffset = fdt_path_offset(fdt, "/nor");
	if (nodeoffset < 0) {
		return nodeoffset;
	}

	int err = fdt_setprop(fdt, nodeoffset, "preload", &value, sizeof(value));
	if (err < 0) {
		DBG_DUMP("WARNING: could not set nor/preload %s.\n",
			fdt_strerror(err));
		return err;
	}
	return 0;
}


void linuxboot_go(LINUXBOOT_INFO *p_info)
{
	LINUXTMP_PARTITION *p_linuxtmp = &p_info->linuxtmp;
	FDT_INFO *p_fdt_info = &p_info->fdt_info;

#if 0
	DBG_DUMP("tmp_begin=0x%lX\n", p_linuxtmp->tmp_begin);
	DBG_DUMP("tmp_curr=0x%lX\n", p_linuxtmp->tmp_curr);
	DBG_DUMP("tmp_end=0x%lX\n", p_linuxtmp->tmp_end);
	DBG_DUMP("tmp2_begin=0x%lX\n", p_linuxtmp->tmp2_begin);
	DBG_DUMP("tmp2_curr=0x%lX\n", p_linuxtmp->tmp2_curr);
	DBG_DUMP("tmp2_end=0x%lX\n", p_linuxtmp->tmp2_end);
	DBG_DUMP("fdt_addr=0x%lX\n", p_linuxtmp->fdt_addr);
	DBG_DUMP("fdt_size=0x%lX\n", p_linuxtmp->fdt_size);
	DBG_DUMP("ramfs_addr=0x%lX\n", p_linuxtmp->ramfs_addr);
	DBG_DUMP("ramfs_size=0x%lX\n", p_linuxtmp->ramfs_size);
	DBG_DUMP("lz_linux_addr=0x%lX\n", p_linuxtmp->lz_linux_addr);
	DBG_DUMP("lz_linux_size=0x%lX\n", p_linuxtmp->lz_linux_size);
	DBG_DUMP("bootargs_addr=0x%lX\n", p_linuxtmp->bootargs_addr);
	DBG_DUMP("bootargs_size=0x%lX\n", p_linuxtmp->bootargs_size);
	DBG_DUMP("nvtpack_addr=0x%lX\n", p_linuxtmp->nvtpack_addr);
	DBG_DUMP("nvtpack_size=0x%lX\n", p_linuxtmp->nvtpack_size);
#endif

#if defined(__aarch64__)
	unsigned long x0 = p_linuxtmp->fdt_addr;
#else
	unsigned int r2 = p_linuxtmp->fdt_addr;
#endif
	unsigned long kernel_addr = p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET;

	int core_id;
	cpu_get_core_id(&core_id);
	if (core_id == 0) {
		arm_gic_disable_interrupt(NULL, 0, 1); //just cpu_disable_interrupt();
#if defined(__aarch64__)
		*(volatile UINT32 *)0xFFE41014 = 1;
		{
			void (*kernel_entry)(unsigned long fdt, unsigned long x1, unsigned long x2, unsigned long x3);
			kernel_entry = (void (*)(unsigned long, unsigned long, unsigned long, unsigned long))((kernel_addr));
			kernel_entry(x0, 0, 0, 0); // never returned
		}
#else
		vos_cpu_dcache_sync((VOS_ADDR)r2, ALIGN_CEIL_64(fdt_totalsize(p_linuxtmp->fdt_addr)), VOS_DMA_TO_DEVICE);
		cpu_disable_mmu();
		//alway enter #Mode_SVC before start linux
		__asm__("cps #0x13");
		{
			void (*kernel_entry)(int zero, int arch, unsigned int params);
			kernel_entry = (void (*)(int, int,  unsigned int))((kernel_addr));
			kernel_entry(0, 0, r2); // never returned
		}
#endif
	} else {
#if defined(__aarch64__)
		// parpare address
		unsigned long long *p_atf_share_mem = (unsigned long long *)(p_fdt_info->atf_addr + p_fdt_info->atf_size - 0x1000);
		printf("%08llX = %08lX\n", (unsigned long long)&p_atf_share_mem[16], x0);
		printf("%08llX = %08lX\n", (unsigned long long)&p_atf_share_mem[0], kernel_addr);

		// set console to uart2
		CONSOLE console = {0};
		uart2_open();
		uart2_init(UART_BAUDRATE_115200, UART_LEN_L8_S1, UART_PARITY_NONE);
		console_get_uart2(&console);
		console_set_curr(&console);

		//int except_irqss[120] = {0};
		//get_except_irqs(except_irqss, 120);

		// release some irq and fiq for linux boot

		int except_irqs_cnt;
		int except_irqs[64] = {0};
		if (get_except_irqs(except_irqs, &except_irqs_cnt, sizeof(except_irqs) / sizeof(except_irqs[0])) == 0) {
			arm_gic_disable_interrupt(except_irqs, except_irqs_cnt, 1);
		} else {
			// if failed to get get_except_irqs, we use default
			int except_irqs_default[] = {INT_ID_DMA, INT_ID_UART2, INT_ID_UART3, INT_ID_CC2, INT_ID_TIMER};
			except_irqs_cnt = sizeof(except_irqs_default) / sizeof(except_irqs_default[1]);
			arm_gic_disable_interrupt(except_irqs_default, except_irqs_cnt, 1);
		}

		// before run linux, clean core[core_id] address of p_atf_share_mem
		// atf will writeback it with linux secondary pointer
		p_atf_share_mem[core_id] = 0;

		// notify atf run linux
		p_atf_share_mem[16] = x0;
		p_atf_share_mem[0] = kernel_addr;
		vos_cpu_dcache_sync((VOS_ADDR)p_atf_share_mem, 0x1000, VOS_DMA_TO_DEVICE);
#else
		// set console to uart2
		CONSOLE console = {0};
#if defined(_NVT_FPGA_)
		uart2_open();
		uart2_init(UART_BAUDRATE_115200, UART_LEN_L8_S1, UART_PARITY_NONE);
		console_get_uart2(&console);
		console_set_curr(&console);
#else
		uart3_open();
		uart3_init(UART_BAUDRATE_115200, UART_LEN_L8_S1, UART_PARITY_NONE);
		console_get_uart3(&console);
		console_set_curr(&console);
#endif
		int except_irqs_cnt;
		int except_irqs[64] = {0};

		if (get_except_irqs(except_irqs, &except_irqs_cnt, sizeof(except_irqs) / sizeof(except_irqs[0])) == 0) {
			arm_gic_disable_interrupt(except_irqs, except_irqs_cnt, 1);
		} else {
			// if failed to get get_except_irqs, we use default
			int except_irqs_default[] = {INT_ID_DMA, INT_ID_UART, INT_ID_PRCC2};
			except_irqs_cnt = sizeof(except_irqs_default) / sizeof(except_irqs_default[1]);
			arm_gic_disable_interrupt(except_irqs_default, except_irqs_cnt, 1);
		}

		// before start linux, clean smp entry address
		*(volatile unsigned int *)p_fdt_info->cpu_release_addr = 0;

		SHMINFO *p_shm = (SHMINFO *)p_fdt_info->shmem_addr;
		vos_cpu_dcache_sync((VOS_ADDR)kernel_addr, p_fdt_info->linux_size, VOS_DMA_TO_DEVICE);
		vos_cpu_dcache_sync((VOS_ADDR)r2, ALIGN_CEIL_64(fdt_totalsize(p_linuxtmp->fdt_addr)), VOS_DMA_TO_DEVICE);
		p_shm->comm.Resv[COMM_RESV_IDX_CORE1_LINUX_ADDR] = kernel_addr;
		p_shm->comm.Resv[COMM_RESV_IDX_CORE1_FDT_ADDR] = r2;
		vos_cpu_dcache_sync((VOS_ADDR)p_shm, sizeof(SHMINFO), VOS_DMA_TO_DEVICE);
#endif
	}
}

void linuxboot_release_rtos(LINUXBOOT_INFO *p_info, FP fp_notify_linux_go_next_step)
{
	void (*kernel_entry)(void);
	FDT_INFO *p_fdt_info = &p_info->fdt_info;

	// disable others interrupt
	int except_irqs_cnt;
	int except_irqs[64] = {0};
	if (get_except_irqs(except_irqs, &except_irqs_cnt, sizeof(except_irqs) / sizeof(except_irqs[0])) == 0) {
		arm_gic_disable_interrupt(except_irqs, except_irqs_cnt, 0);
	}
	cpu_disable_interrupt();

	fp_notify_linux_go_next_step();

#if defined(__aarch64__)
	// get smp entry address
	int core_id;
	unsigned long long *p_atf_share_mem = (unsigned long long *)(p_fdt_info->atf_addr + p_fdt_info->atf_size - 0x1000);

	cpu_get_core_id(&core_id);
	while(p_atf_share_mem[core_id] == 0) {
		vos_cpu_dcache_sync((VOS_ADDR)p_atf_share_mem, 0x40, VOS_DMA_FROM_DEVICE);
	}
	kernel_entry = (void (*)(void))((uintptr_t)(p_atf_share_mem[core_id]));
#else
	// get smp entry address
	volatile unsigned int *mp_core2_addr = (volatile unsigned int *)p_fdt_info->cpu_release_addr;
	//alway enter #Mode_SVC before start linux
	__asm__("cps #0x13");
	// polling wait for linux-kernel to write smp entry
	while(*mp_core2_addr == 0) {
		// cc reg, no need to flush cache
	}
	kernel_entry = (void (*)(void))((*mp_core2_addr));
#endif
	cpu_disable_mmu();
	kernel_entry(); // never returned
}
