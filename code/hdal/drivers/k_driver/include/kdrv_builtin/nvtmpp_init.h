#ifndef _NVTMPP_INIT_H
#define _NVTMPP_INIT_H
#if defined (__KERNEL__)
#include <linux/of.h>
#include <linux/io.h>       // for ioremap
#include <linux/of_device.h>
#else
#include <stdlib.h>
//#include <libfdt.h>
#include <plat/rtosfdt.h>
#endif

#include "kwrap/type.h"

#define FBOOT_COMNBLK_MAX_CNT       250

#define FBOOT_MISC_CPOOL_MAX        3
#define FBOOT_MISC_CPOOLBLK_MAX_CNT 3

typedef enum {
	FBOOT_POOL_VPRC_CTRL         = 0,
	FBOOT_POOL_VENC_0_MAX           ,
	FBOOT_POOL_ACAP_CTRL            ,
	FBOOT_POOL_VCAP_SHDR_BUF        ,
	FBOOT_POOL_VCAP_SHDR_BUF2       ,
	FBOOT_POOL_VENC_1_MAX           ,
	FBOOT_POOL_VENC_2_MAX           ,
	FBOOT_POOL_VENC_3_MAX           ,
	FBOOT_POOL_VENC_4_MAX		,
	FBOOT_POOL_VENC_5_MAX		,
	FBOOT_POOL_VPRC1_CTRL       ,
	FBOOT_POOL_VENC_CTRL_TILE       ,
	FBOOT_POOL_CNT,
	ENUM_DUMMY4WORD(NVTMPP_FBOOT_PRV_POOL)
} NVTMPP_FBOOT_PRV_POOL;


typedef int (*NVTMPP_LOCK_BLK_CB)(uintptr_t blk_addr);
typedef int (*NVTMPP_UNLOCK_BLK_CB)(uintptr_t blk_addr);

typedef struct {
	uintptr_t va;    				    ///< Memory buffer virtual starting address
	uintptr_t pa;    				    ///< Memory buffer physical starting address
	UINT32 size;     					///< Memory buffer size
} NVTMPP_MMZ_DDR_S;

typedef struct {
	NVTMPP_MMZ_DDR_S  ddr_mem[3];       ///< Memory buffer virtual starting address
	UINT32            max_pools_cnt;
	UINT32            max_yuv_lineoffset;
	int               init_sts;
} NVTMPP_MMZ_INFO_S;


typedef struct {
	// should not change the sequence
	UINT32      pa;    				    ///< Memory buffer physical starting address
	UINT32    size;     			    ///< Memory buffer size
	uintptr_t addr;    				    ///< Memory buffer virtual starting address
	UINT32    ref_cnt;                  ///< block reference count
} NVTMPP_FASTBOOT_BLK_S;

typedef struct {
	UINT32 pool_type;                   ///< The common pool type
	UINT32 blk_cnt;
	NVTMPP_FASTBOOT_BLK_S  blk[FBOOT_MISC_CPOOLBLK_MAX_CNT];
} NVTMPP_FBOOT_MISC_CPOOL_S;

typedef struct {
	const CHAR *pool_name;              ///< The private pool name on nvtmpp
	const CHAR *dts_node;               ///< The dts node name
	BOOL        optional;               ///< The pool is optional or not
} NVTMPP_FBOOT_POOL_DTS_INFO_S;

typedef struct {
	UINT32                 init_tag;
	// common blks
	UINT32                 comn_blk_cnt;
	NVTMPP_FASTBOOT_BLK_S  comn_blk[FBOOT_COMNBLK_MAX_CNT];
	// misc common pool type
	NVTMPP_FBOOT_MISC_CPOOL_S misc_cpool[FBOOT_MISC_CPOOL_MAX];
	// private pools
	NVTMPP_FASTBOOT_BLK_S  pv_pools[FBOOT_POOL_CNT];
} NVTMPP_FASTBOOT_MEM_S;

typedef struct {
	#if defined(__KERNEL__)
	struct device_node *of_node;
	#else
	unsigned char *p_fdt;
	int       nodeoffset;
	#endif
} NVTMPP_DTSI_NODE_INFO;

typedef struct {
	unsigned char *p_fdt;
	int       nodeoffset;
} NVTMPP_FASTBOOT_DTSI_NODE_INFO;



extern int  nvtmpp_init_mmz(void);
extern void nvtmpp_exit_mmz(void);
extern NVTMPP_MMZ_INFO_S *nvtmpp_get_mmz(void);
extern int  nvtmpp_parse_fastboot_mem_dt(void);
extern NVTMPP_FASTBOOT_MEM_S *nvtmpp_get_fastboot_mem(void);
extern void nvtmpp_reg_fastboot_lock_cb(NVTMPP_LOCK_BLK_CB lock_cb);
extern void nvtmpp_reg_fastboot_unlock_cb(NVTMPP_UNLOCK_BLK_CB unlock_cb);
extern int  nvtmpp_lock_fastboot_blk(uintptr_t blk_addr);
extern int  nvtmpp_unlock_fastboot_blk(uintptr_t blk_addr);
extern unsigned long nvtmpp_fastboot_spin_lock(void);
extern void   nvtmpp_fastboot_spin_unlock(unsigned long flags);
extern NVTMPP_FBOOT_POOL_DTS_INFO_S *nvtmpp_get_fastboot_pvpool_dts_info(void);
extern uintptr_t  nvtmpp_buitin_sys_pa2va(uintptr_t phys_addr);
extern uintptr_t  nvtmpp_sys_va2pa(uintptr_t virt_addr);
// return 0 if get blk fail
extern uintptr_t  nvtmpp_get_fastboot_blk(UINT32 blk_size);

extern INT32   nvtmpp_get_dtsi_node(CHAR *path, NVTMPP_DTSI_NODE_INFO *node);
extern INT32   nvtmpp_read_dtsi_array(NVTMPP_DTSI_NODE_INFO *node, const CHAR *tag, UINT32 *buf, UINT32 num);
extern INT32   nvtmpp_read_dtsi_array_u64(NVTMPP_DTSI_NODE_INFO *node, const CHAR *tag, UINT64 *buf, UINT32 num);
extern INT32   nvtmpp_get_fastboot_dtsi_node(CHAR *path, NVTMPP_FASTBOOT_DTSI_NODE_INFO *node);
extern INT32   nvtmpp_read_fastboot_dtsi_array(NVTMPP_FASTBOOT_DTSI_NODE_INFO *node, const CHAR *tag, UINT32 *buf, UINT32 num);
extern INT32   nvtmpp_read_fastboot_dtsi_array_u64(NVTMPP_FASTBOOT_DTSI_NODE_INFO *node, const CHAR *tag, UINT64 *buf, UINT32 num);
extern void   *nvtmpp_ioremap_cache(unsigned long offset, unsigned long size);
extern void    nvtmpp_iounmap(void *addr);
extern int     nvtmpp_is_dynamic_map(void);
extern UINT32  nvtmpp_get_vcap_fixed_map_poolcnt(void);
extern UINT32  nvtmpp_get_dynamic_map_blk_threshold(void);

#endif //_NVTMPP_INIT_H
