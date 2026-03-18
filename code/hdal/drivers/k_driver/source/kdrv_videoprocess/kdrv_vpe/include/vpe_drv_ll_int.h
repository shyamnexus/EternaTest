#ifndef _VPE_DRV_LL_INT_H_
#define _VPE_DRV_LL_INT_H_

#include "vpe_drv_platform_int.h"
#include <kwrap/spinlock.h>

struct vpe_drv_ll_blk {
	void *vaddr;
	uintptr_t paddr;
	u8 ddr_id;
	u32 size;
	u16 total_cmd_num;

	u16 curr_cmd_idx;

    struct vos_list_head job_cfg_list_root; //for keep 'struct vpe_drv_ctl_job' if it is in ll block, always 1 job

	void *parent;
    struct vos_list_head list; /* ll_wait_list_root or ll_eng_list_root. Moved to ll_eng_list_root if engine is available. */
	struct vpe_drv_ll_blk *next;
};

struct vpe_drv_wbuf_cmd {
	u8 sts; //bit8: ignore, bit0: updated
	u32 val;
};

struct vpe_drv_wbuf_info {
	void *reg;
	u32 reg_size;

	void *sts;
	u32 sts_size;

	u32 wbuf_num;
};

struct vpe_drv_ll_ch {
	u16 chip_id;
	u16 eng_id;

	unsigned long ll_mem_hdl;
	void *ll_mem_vaddr;
	uintptr_t ll_mem_paddr;

	unsigned long ll_blk_hdl;
	struct vpe_drv_ll_blk *ll_blk;

	u32 total_size;
	u16 blk_num;
	u16 blk_size;

    vk_spinlock_t ll_lock;
	u16 ll_free_blk_cnt;
	struct vos_list_head ll_free_root;

	unsigned long wbuf_hdl;
	void *wbuf_reg;
	void *wbuf_sts;
	u32 wbuf_reg_size;
	u32 wbuf_sts_size;
	u32 wbuf_num;
};

struct vpe_drv_ll {
	u16 chip_num;
	u16 eng_num;
	u16 total_ch;

	unsigned long ch_hdl;
	struct vpe_drv_ll_ch *ch;
};

int vpe_drv_ll_init(struct vpe_drv_ll *pllctl, int chip_num, int eng_num);
int vpe_drv_ll_uninit(struct vpe_drv_ll *pllctl);
struct vpe_drv_wbuf_info vpe_drv_ll_get_wbuf(int chip_id, int eng_id);
struct vpe_drv_ll_blk* vpe_drv_ll_get_free_blk(int chip_id, int eng_id);
int vpe_drv_ll_flush_blk(struct vpe_drv_ll_blk *blk);
int vpe_drv_ll_get_free_blk_cnt(int chip_id, int eng_id);

int vpe_drv_ll_write_reg(struct vpe_drv_ll_blk *blk, u32 reg_ofs, u32 val);
int vpe_drv_ll_fire_null_cmd(struct vpe_drv_ll_blk *blk, unsigned int tab_idx);
uintptr_t vpe_drv_ll_get_cur_paddr(struct vpe_drv_ll_blk *blk);
int vpe_drv_ll_fire_next_cmd(struct vpe_drv_ll_blk *blk, uintptr_t next_job_addr, unsigned int tab_idx);
int vpe_drv_ll_add_next_cmd(struct vpe_drv_ll_blk *blk, unsigned int tab_idx);
void vpe_drv_ll_link_blk(struct vpe_drv_ll_blk *parent, struct vpe_drv_ll_blk *child);
void vpe_drv_ll_reset_blk(struct vpe_drv_ll_blk *blk);
int vpe_drv_ll_get_total_entry(void);

int vpe_drv_ll_dump_info(void);
void vpe_drv_ll_dump_blk_info(struct vpe_drv_ll_blk *blk, int dump_cmd_tab_flag);
#endif //_VPE_DRV_LL_INT_H_
