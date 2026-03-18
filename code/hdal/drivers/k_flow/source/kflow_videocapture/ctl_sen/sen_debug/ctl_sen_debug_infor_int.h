#ifndef _CTL_SEN_DEBUG_INT_H_
#define _CTL_SEN_DEBUG_INT_H_

#include "ctl_sen_int.h"
#include "ctl_sen_if_int.h"

#define SEN_STR_CUR_MODE        "===> CUR_MODE :"
#define CTL_SEN_PROC_MAP_CNT_MAX 40
#define CTL_SEN_PROC_TIME_CNT_MAX 60
#define CTL_SEN_PROC_TIME_CNT_ADV_MAX 20

typedef struct {
	UINT32  sen_id;
	UINT64  time_us_u64; // clock, us
	UINT32  tag;
	UINT32	para1;
	UINT32	para2;
	INT32   rt;
} CTL_SEN_PROC_MAP;

typedef struct {
	UINT32  sen_id;
	CHAR    func_name[6];
	UINT64  time_us_u64; // clock, us
	UINT32  tag;
	INT32   rt;
	UINT32  cfg;
} CTL_SEN_PROC_TIME;

#define tag_op_init_map     0
#define tag_op_uninit_map	1
#define tag_op_add_map		2
#define tag_op_del_map      3
#define tag_op_max_map		4

#define tag_step_begin           0
#define tag_step_end             1
#define tag_step_begin_sendrv    2
#define tag_step_end_sendrv      3
#define tag_step_max             4

#define __tag_timestmp 				(hwclock_get_longcounter() & 0xffffff)
#define __tag_manualstmp(manual) 	(manual & 0xffffff)

#define __tag_op_set      0
#define __tag_op_get      1
#define __tag_op_write    2
#define __tag_op_read     3
#define __tag_op_max      4

#define tag_timestmp_begin(op)          ((tag_step_begin << 28)          | ((op & 0xf) << 24) | __tag_timestmp)
#define tag_timestmp_end(op)            ((tag_step_end << 28)            | ((op & 0xf) << 24) | __tag_timestmp)
#define tag_timestmp_begin_sendrv(op)   ((tag_step_begin_sendrv << 28)   | ((op & 0xf) << 24) | __tag_timestmp)
#define tag_timestmp_end_sendrv(op)     ((tag_step_end_sendrv << 28)     | ((op & 0xf) << 24) | __tag_timestmp)

#define tag_manualstmp_end(op, manualstmp)  		((tag_step_end << 28)            | ((op & 0xf) << 24) | __tag_manualstmp(manualstmp))
#define tag_manualstmp_end_sendrv(op, manualstmp)  	((tag_step_end_sendrv << 28)     | ((op & 0xf) << 24) | __tag_manualstmp(manualstmp))

void ctl_sen_rec_process_map(UINT32 sen_id, UINT32 tag, UINT32 para1, UINT32 para2, INT32 rt);
void ctl_sen_rec_process(UINT32 sen_id, CHAR *func_name, UINT32 tag, INT32 rt);
void ctl_sen_rec_process_adv(UINT32 sen_id, CHAR *func_name, UINT32 tag, INT32 rt, UINT32 cfg);

extern void ctl_sen_dbg_dump_info(int (*dump)(const char *fmt, ...));
extern void ctl_sen_dbg_dump_signal(int (*dump)(const char *fmt, ...), UINT32 sen_id, CTL_SEN_INTE inte);
extern void ctl_sen_dbg_dump_process(int (*dump)(const char *fmt, ...));


#endif //_CTL_SEN_DEBUG_INT_H_
