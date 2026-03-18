#ifndef KDRV_VPE_CTL_INT_H_
#define KDRV_VPE_CTL_INT_H_

#include <kwrap/nvt_type.h>
#include <kwrap/spinlock.h>
#include <kwrap/util.h>
#include "vpe_drv_util_int.h"
#include "vpe_drv_ctl.h"

enum kdrv_vpe_sts {
    KDRV_VPE_STS_IDLE = 0x0,
    KDRV_VPE_STS_OPEN = 0x1,
    KDRV_VPE_STS_BLK_MODE = 0x2,
};

struct kdrv_vpe_job {
	struct vpe_drv_job_head *job_head;
	struct vos_list_head list;
	enum vpe_drv_sts sts;
};

struct kdrv_vpe_ch_info {
	struct vos_list_head job_list_root;

	unsigned long last_put_time;
	unsigned long last_cb_time;

	atomic_t put_cnt;
	atomic_t cb_cnt;
};

struct kdrv_vpe_handle {

    vk_spinlock_t lock; //handle lock

	ID flg_id; //for blocking mode used

    enum kdrv_vpe_sts sts;

	struct kdrv_vpe_ch_info *ch;

	struct vos_list_head list;
};

#define KDRV_VPE_EVT_CHAR_MAX 8
#define KDRV_VPE_MSG_CHAR_MAX 10
struct kdrv_vpe_log {
	char event[KDRV_VPE_EVT_CHAR_MAX];		//event
	char msg[KDRV_VPE_MSG_CHAR_MAX]; 			//event message
	unsigned long data;							//event data
	unsigned int sts;							//1:api enter, 2:api exit
	unsigned long time;							//event time

	struct vos_list_head list;
};

#define KDRV_VPE_LOG_NUM 20

#define KDRV_VPE_CTL_COUNTER_HDL_MAX 2
struct kdrv_vpe_ctl {
	u32 chip_num;
	u32 eng_num;
	u32 ch_num;

	u32 hdl_num;

    vk_spinlock_t eng_lock;
	u32 eng_open_cnt;

	uintptr_t hdl_buf_hdl;
    struct kdrv_vpe_handle *hdl_pool;
    vk_spinlock_t hdl_lock;
	struct vos_list_head free_hdl_list_root;
	struct vos_list_head used_hdl_list_root;
	atomic_t hdl_used_cnt;

	uintptr_t ch_buf_hdl;
    struct kdrv_vpe_ch_info *ch_pool;

	uintptr_t job_buf_hdl;

	uintptr_t counter_hdl;
	union {
		atomic_t *counter_ptr[KDRV_VPE_CTL_COUNTER_HDL_MAX];

		struct {
			atomic_t *put_cnt;		//0
			atomic_t *cb_cnt;		//1
		} counter;
	};

	uintptr_t log_buf_hdl;
    struct kdrv_vpe_log *log_pool;
    vk_spinlock_t log_lock;
	struct vos_list_head free_log_list_root;
	struct vos_list_head used_log_list_root;
};

struct kdrv_vpe_func_item {
	s32(*set_fp)(struct kdrv_vpe_handle *, void *p_param);

	s32(*get_fp)(struct kdrv_vpe_handle *, void *p_param);

	char *msg;
};

int kdrv_vpe_ctl_init(struct kdrv_vpe_ctl *pctl, int chip_num, int eng_num);
int kdrv_vpe_ctl_uninit(struct kdrv_vpe_ctl *pctl);
int kdrv_vpe_ctl_dump_info(void);
#endif //KDRV_VPE_CTL_INT_H_
