#ifndef __PCIE_SYS_INTERNAL_H__
#define __PCIE_SYS_INTERNAL_H__

#include <linux/kconfig.h>
#include <linux/printk.h>
#include <linux/soc/nvt/nvt-pci.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <plat/hardware.h>

#ifndef PCIE_ATU_ENABLE
#define PCIE_ATU_ENABLE             (0x1 << 31)
#endif
#ifndef PCIE_ATU_UNR_REGION_CTRL1
#define PCIE_ATU_UNR_REGION_CTRL1   0x00
#endif
#ifndef PCIE_ATU_UNR_REGION_CTRL2
#define PCIE_ATU_UNR_REGION_CTRL2   0x04
#endif
#ifndef PCIE_ATU_UNR_LOWER_BASE
#define PCIE_ATU_UNR_LOWER_BASE     0x08
#endif
#ifndef PCIE_ATU_UNR_UPPER_BASE
#define PCIE_ATU_UNR_UPPER_BASE     0x0C
#endif
#ifndef PCIE_ATU_UNR_LIMIT
#define PCIE_ATU_UNR_LIMIT          0x10
#endif
#ifndef PCIE_ATU_UNR_UPPER_LIMIT
#define PCIE_ATU_UNR_UPPER_LIMIT    0x20
#endif
#ifndef PCIE_ATU_UNR_LOWER_TARGET
#define PCIE_ATU_UNR_LOWER_TARGET   0x14
#endif
#ifndef PCIE_ATU_UNR_UPPER_TARGET
#define PCIE_ATU_UNR_UPPER_TARGET   0x18
#endif

#ifndef PCIE_GET_ATU_OUTB_UNR_REG_OFFSET
#define PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(region)	\
							((0x3 << 20) | ((region) << 9))
#endif

#ifndef PCIE_GET_ATU_INB_UNR_REG_OFFSET
#define PCIE_GET_ATU_INB_UNR_REG_OFFSET(region)				\
							((0x3 << 20) | ((region) << 9) | (0x1 << 8))
#endif

#if IS_ENABLED(CONFIG_PCI)
#else
#include "../nvt-pcie-cascade-drv/pcie-drv-wrap.h"
#endif

#define NVT_PCIE_LIB_VERSION    "0.0.42"
#define NVT_PCIE_LIB_NAME       "nvt-pcie-lib"

#define SHMBLK_NUM              16
#define SHMBLK_NAME_SIZE        32
#define SHMBLK_BUF_SIZE         148
#define SHMBLK_TAG              0xDEADBEEF

#define NAME_SIZE               16
#define MAX_EP_CNT              (CHIP_MAX - 1) //CHIP_MAX - 1(RC)

#if IS_ENABLED(CONFIG_PCI)
#define ATU_DOWNMAP_COUNT       (NVT_PCIE_INBOUND_TOTAL_SEG_SIZE) //including MMIO:7, APB:1
#define ATU_UPMAP_COUNT         2 //MMIO:1, APB:1
#else
#define ATU_DOWNMAP_COUNT       ATU_MAP_COUNT
#define ATU_UPMAP_COUNT         ATU_MAP_COUNT
#endif // IS_ENABLED(CONFIG_PCI)

#define OFFSET_SHM_VALID        0x00
#define OFFSET_SHM_CHKSUM       0x04
#define OFFSET_SHM_ADDR_H       0x08    //EP local pa to RC shared memory
#define OFFSET_SHM_ADDR_L       0x0C    //Note: this address is pre-mapped by RC
#define OFFSET_SHM_DTS_SIZE_H   0x10
#define OFFSET_SHM_DTS_SIZE_L   0x14
#define OFFSET_SHM_SYS_SIZE     0x18

#define DTS_PATH_LIB_CFG        "/nvt-pcie-lib-cfg"
#define DTS_PATH_EP_INFO        "/ep_info"
#define CC_NODE_PATH            "/cc_cmd_buf/chip0/pcie_lib"
#define PROP_RC_SHM_SIZE        "rc_shm_size"
#define PROP_EP_ALLOC_MEM       "ep_alloc_mem"
#define PROP_EPINFO_CHIP_ID     "chip_id"
#define PROP_EPINFO_DAUGHTER    "daughter"

#define CHIP_EP_COMMON          CHIP_EP0  //For common PCIe, all EPs only know itself is EP
#define EDMA_CHN_ID             0
#define BUSID_RC_COMMON         0

#define U32_ELEMS_RC_SHM_SIZE   2
#define U32_ELEMS_EP_ALLOC_MEM  5

#define CHIP_MAU_BASE           0x0
#define CHIP_APB_BASE           NVT_PERIPHERAL_PHYS_BASE

#define TYPE_NAME_MAU           "mau"
#define TYPE_NAME_APB           "apb"

#define HL32_TO_U64(high32, low32) (((u64)(high32) << 32) | (low32))
#define P2U64(pointer)  ((u64)((phys_addr_t)pointer))

#define NVTPCIE_STRNCPY(dst, src, dst_size) do { \
	strncpy(dst, src, (dst_size)-1); \
	dst[(dst_size)-1] = '\0'; \
} while(0)

#define ISVALID_CHIPID(chipid) ((chipid >= CHIP_RC) && (chipid < CHIP_MAX))
#define ISVALID_EP_CHIPID(ep_chipid) ((ep_chipid >= CHIP_EP0) && (ep_chipid < CHIP_MAX))
#define ISVALID_DDRID(ddrid) ((ddrid >= DDR_ID0) && (ddrid < DDR_ID_MAX))

#define DBG_INFO(fmt, args...) printk("%s: "fmt, NVT_PCIE_LIB_NAME, ##args)

#ifndef DBG_ERR
#define DBG_ERR(fmt, args...) printk("ERR:%s() "fmt, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmt, args...) printk("WRN:%s() "fmt, __func__, ##args)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmt, args...) printk(fmt, ##args)
#endif

#ifndef DBG_IND
#if 0
#define DBG_IND(fmt, args...) printk("IND:%s() "fmt, __func__, ##args)
#else
#define DBG_IND(fmt, args...)
#endif
#endif

#ifndef CHKPNT
#define CHKPNT printk("CHK: %d, %s\r\n", __LINE__, __func__)
#endif

typedef struct {
	phys_addr_t         local_pa;
	unsigned long       size;
	char                name[32];
	nvtpcie_chipid_t    ep_chipid;
	phys_addr_t         ep_pa;
	struct pci_dev*     ep_pci_dev; //only used by the common framework
} nvtpcie_rc_downmap_t;

typedef struct {
	phys_addr_t         local_pa;
	unsigned long       size;
	char                name[32];
	nvtpcie_chipid_t    ep_chipid;
	phys_addr_t         rc_pa;
} nvtpcie_ep_upmap_t;

typedef struct {
	unsigned int head_tag; //4 bytes
	unsigned int size; //4 bytes
	char name[SHMBLK_NAME_SIZE]; //32 bytes
	unsigned char buf[SHMBLK_BUF_SIZE];
	unsigned int tail_tag; //4 bytes
} nvtpcie_shmblk_t;

typedef struct {
	nvtpcie_rc_downmap_t rc_downmap[MAX_EP_CNT * ATU_DOWNMAP_COUNT];
	nvtpcie_ep_upmap_t ep_upmap[MAX_EP_CNT * ATU_UPMAP_COUNT];
	nvtpcie_shmblk_t shmblk[SHMBLK_NUM];
} nvtpcie_sysinfo_t;

void _nvtpcie_dump_rc_downmap(void);
void _nvtpcie_dump_ep_upmap(void);
void _nvtpcie_dump_ddr_map(void);
void _nvtpcie_dump_ep_allocmem(void);

int _nvtpcie_atu_board_init(void);
int _nvtpcie_atu_common_init(void);
int _nvtpcie_atu_cascade_init(void);

unsigned long _nvtpcie_sysinfo_size(void);
int _nvtpcie_sysinfo_set_map(void *p_iobuf);
int _nvtpcie_sysinfo_get_map(void *p_iobuf, nvtpcie_chipid_t ep_chipid);


int nvtpcie_platdrv_init(void);
void nvtpcie_platdrv_exit(void);

int nvtpcie_shmem_init(u64 rcshm_size);
int nvtpcie_shmem_exit(void);
void _nvtpcie_shmem_dump_sysinfo(void);
void _nvtpcie_shmem_dump_shmblk(void);
void * _nvtpcie_shmem_get_sysinfo_va(void);
int _nvtpcie_shmem_udpate_sysinfo(void);

int nvtpcie_proc_init(void);
void nvtpcie_proc_exit(void);

void nvtpcie_test_api(void);
void nvtpcie_test_perf(void);
void nvtpcie_test_shmblk(void);

#endif
