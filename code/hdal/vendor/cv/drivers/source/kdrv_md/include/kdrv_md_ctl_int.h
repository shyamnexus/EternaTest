#ifndef KDRV_MD_CTL_INT_H_
#define KDRV_MD_CTL_INT_H_

#include <kwrap/nvt_type.h>
#include <kwrap/spinlock.h>
#include <kwrap/util.h>
#include "md_drv_util_int.h"
#include "kdrv_md_ctl.h"

enum kdrv_md_sts {
    KDRV_MD_STS_IDLE = 0x0,
    KDRV_MD_STS_OPEN = 0x1,
    KDRV_MD_STS_BLK_MODE = 0x2,
};

struct kdrv_md_job {
	struct md_drv_job_head *job_head;
	struct vos_list_head list;
	enum md_drv_sts sts;
};

struct kdrv_md_ch_info {
	struct vos_list_head job_list_root;

	unsigned long last_put_time;
	unsigned long last_cb_time;

	atomic_t put_cnt;
	atomic_t cb_cnt;
};

struct kdrv_md_handle {

    vk_spinlock_t lock; //handle lock

	ID flg_id; //for blocking mode used

    enum kdrv_md_sts sts;

	struct kdrv_md_ch_info *ch;

	unsigned int pri_idx;

	struct vos_list_head list;
};

#define KDRV_MD_EVT_CHAR_MAX 8
#define KDRV_MD_MSG_CHAR_MAX 10
struct kdrv_md_log {
	char event[KDRV_MD_EVT_CHAR_MAX];		//event
	char msg[KDRV_MD_MSG_CHAR_MAX]; 			//event message
	unsigned long data;							//event data
	unsigned int sts;							//1:api enter, 2:api exit
	unsigned long time;							//event time

	struct vos_list_head list;
};

#define KDRV_MD_LOG_NUM 10

#define KDRV_MD_CTL_COUNTER_HDL_MAX 2
struct kdrv_md_ctl {
	u32 chip_num;
	u32 eng_num;
	u32 ch_num;

	u32 hdl_num;

    vk_spinlock_t eng_lock;
	u32 eng_open_cnt;

	uintptr_t hdl_buf_hdl;
    struct kdrv_md_handle *hdl_pool;
    vk_spinlock_t hdl_lock;
	struct vos_list_head free_hdl_list_root;
	struct vos_list_head used_hdl_list_root;
	atomic_t hdl_used_cnt;

	uintptr_t ch_buf_hdl;
    struct kdrv_md_ch_info *ch_pool;

	uintptr_t job_buf_hdl;

	uintptr_t counter_hdl;
	union {
		atomic_t *counter_ptr[KDRV_MD_CTL_COUNTER_HDL_MAX];

		struct {
			atomic_t *put_cnt;		//0
			atomic_t *cb_cnt;		//1
		} counter;
	};

	uintptr_t log_buf_hdl;
    struct kdrv_md_log *log_pool;
    vk_spinlock_t log_lock;
	struct vos_list_head free_log_list_root;
	struct vos_list_head used_log_list_root;
};

struct kdrv_md_func_item {
	s32(*set_fp)(struct kdrv_md_handle *, void *p_param);

	s32(*get_fp)(struct kdrv_md_handle *, void *p_param);

	char *msg;
};

int kdrv_md_ctl_init(struct kdrv_md_ctl *pctl, int chip_num, int eng_num);
int kdrv_md_ctl_uninit(struct kdrv_md_ctl *pctl);
int kdrv_md_ctl_dump_info(void);
#endif //KDRV_MD_CTL_INT_H_
