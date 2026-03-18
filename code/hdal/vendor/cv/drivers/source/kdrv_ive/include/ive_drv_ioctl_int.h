#ifndef _IVE_DRV_IOCTL_INT_H_
#define _IVE_DRV_IOCTL_INT_H_
#include "kdrv_ive_ctl.h"
#include "ive_drv_platform_int.h"

#define FLAG_ID_MAX 4
#define BIT_NUM_PER_FLAG 32
#define FLAG_BIT_NUM_MAX (FLAG_ID_MAX * BIT_NUM_PER_FLAG)

struct ive_drv_ioctl_flag_info {
	int id_idx;
	int bit;
};

#define IVE_DRV_IOCTL_EVT_CHAR_MAX 8
#define IVE_DRV_IOCTL_MSG_CHAR_MAX 8
struct ive_drv_ioctl_log {
	char event[IVE_DRV_IOCTL_EVT_CHAR_MAX];		//event
	char msg[IVE_DRV_IOCTL_MSG_CHAR_MAX]; 			//event message
	unsigned int data[3];
	unsigned long time;								//event time

	struct vos_list_head list;
};

struct ive_drv_ioctl_job {
	unsigned int ioc_id;	//unique code for query used

	int self_rel_flag;	//1:release job at callback func(default)
	struct ive_drv_ioctl_flag_info flag_info;
	struct ive_drv_job_head job_head;

	struct vos_list_head list;
};

struct ive_drv_ioctl_ctl {
	uintptr_t ioctl_job_hdl;
	uintptr_t ioctl_job_cfg_hdl;

    vk_spinlock_t job_list_lock;
	struct vos_list_head job_list_root;

	ID flag_id[FLAG_ID_MAX];
    vk_spinlock_t free_bit_lock;
	unsigned int free_bit_mask[FLAG_ID_MAX]; //1:free, 0:used
	unsigned int free_bit_num; 			 	 //0 ~ FLAG_BIT_NUM_MAX
	unsigned int last_search_idx; 			 //0 ~ FLAG_BIT_NUM_MAX


    vk_spinlock_t job_id_lock;
	unsigned int ioc_id;

	uintptr_t log_buf_hdl;
	unsigned int max_log_num;
    vk_spinlock_t log_lock;
	struct vos_list_head free_log_list_root;
	struct vos_list_head used_log_list_root;
};

int ive_drv_ioctl_init(struct ive_drv_ioctl_ctl *pioctl, int chip_num, int eng_num);
int ive_drv_ioctl_uninit(struct ive_drv_ioctl_ctl *pioctl);
long ive_drv_ioctl(unsigned int cmd, unsigned long arg);
int ive_drv_ioctl_dump_info(void);
int ive_drv_ioctl_log_realloc(unsigned int max_cnt);
void ive_drv_ioctl_dump_log(void);
#endif //_IVE_DRV_IOCTL_INT_H_
