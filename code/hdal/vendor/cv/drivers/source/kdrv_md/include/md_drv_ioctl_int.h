#ifndef _MD_DRV_IOCTL_INT_H_
#define _MD_DRV_IOCTL_INT_H_
#include "kdrv_md_ctl.h"
#include "md_drv_platform_int.h"

#define FLAG_ID_MAX 4
#define BIT_NUM_PER_FLAG 32
#define FLAG_BIT_NUM_MAX (FLAG_ID_MAX * BIT_NUM_PER_FLAG)

struct md_drv_ioctl_flag_info {
	int id_idx;
	int bit;
};

#define MD_DRV_IOCTL_EVT_CHAR_MAX 8
#define MD_DRV_IOCTL_MSG_CHAR_MAX 8
struct md_drv_ioctl_log {
	char event[MD_DRV_IOCTL_EVT_CHAR_MAX];		//event
	char msg[MD_DRV_IOCTL_MSG_CHAR_MAX]; 			//event message
	unsigned int data[3];
	unsigned long time;								//event time

	struct vos_list_head list;
};

struct md_drv_ioctl_job {
	unsigned int ioc_id;	//unique code for query used

	int self_rel_flag;	//1:release job at callback func(default)
	struct md_drv_ioctl_flag_info flag_info;
	struct md_drv_job_head job_head;

	struct vos_list_head list;
};

struct md_drv_rlt_buf {
	unsigned int ioc_id;
	struct md_drv_rlt_info rlt_info;
    struct vos_list_head list; //job link list
};


struct md_drv_ioctl_ctl {
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

	uintptr_t rlt_buf_hdl;
    vk_spinlock_t rlt_lock;
	struct vos_list_head used_rlt_list_root;
};

int md_drv_ioctl_init(struct md_drv_ioctl_ctl *pioctl, int chip_num, int eng_num);
int md_drv_ioctl_uninit(struct md_drv_ioctl_ctl *pioctl);
long md_drv_ioctl(unsigned int cmd, unsigned long arg);
int md_drv_ioctl_dump_info(void);
int md_drv_ioctl_log_realloc(unsigned int max_cnt);
void md_drv_ioctl_dump_log(void);
void md_drv_ioctl_dump_rlt_buf(void);
#endif //_MD_DRV_IOCTL_INT_H_
