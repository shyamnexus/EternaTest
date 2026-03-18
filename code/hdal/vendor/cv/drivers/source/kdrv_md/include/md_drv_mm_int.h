#ifndef _MD_DRV_MM_INT_H_
#define _MD_DRV_MM_INT_H_

#if defined(__LINUX)
#define COHERENT_CACHE_BUF 0
#else
#define COHERENT_CACHE_BUF 1
#endif

struct md_drv_mm_blk {
	void *va_addr;
	uintptr_t pa_addr;
	uint size;
};

enum md_drv_alloc_type_t {
	COHERENT = 0,
	KALLOC,
	MM_ALLOC_TYPE_MAX_NUM,
};

enum md_drv_alloc_mode_t {
	STATIC_MODE = 0,
	DYNAMIC_MODE,
	STATIC_DYNAMIC_MODE,
	MM_ALLOC_MODE_MAX_NUM,
};


struct md_drv_mm_param_t {
	enum md_drv_alloc_type_t type;
	enum md_drv_alloc_mode_t mode;
};

int md_drv_mm_init(void);
int md_drv_mm_uninit(void);
unsigned long md_drv_mm_open(char *name, int blk_size, int blk_cnt, struct md_drv_mm_param_t *param);
int md_drv_mm_close(unsigned long hdl);
struct md_drv_mm_blk md_drv_mm_alloc_blk(unsigned long hdl);
struct md_drv_mm_blk md_drv_mm_alloc_blk_zi(unsigned long hdl);
int md_drv_mm_free_blk(unsigned long hdl, struct md_drv_mm_blk *pblk);
int md_drv_mm_free_blk_by_va(unsigned long hdl, void *va_addr);
int md_drv_mm_get_free_blk_num(unsigned long hdl);
int md_drv_mm_get_used_blk_num(unsigned long hdl);
int md_drv_mm_get_total_blk_num(unsigned long hdl);
int md_drv_mm_dump_info(int free_blk, int used_blk);
#endif //_MD_DRV_MM_INT_H_