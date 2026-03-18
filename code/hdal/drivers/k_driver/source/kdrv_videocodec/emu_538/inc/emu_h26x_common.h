#ifndef _EMU_H26X_COMMON_H_
#define _EMU_H26X_COMMON_H_

#include "emu_h26x_job.h"
#include "emu_h26x_mem.h"
#include "emu_h26x_file.h"
#include "kdrv_vdocdc_dbg.h"

#include <kwrap/task.h>
#define sleep_us(x)  vos_task_delay_us(x);
#define sleep_ms(x)  vos_task_delay_ms(x);

#define H264_AUTO_JOB_SEL	(0)
#define	MT_RA_ENABLE		(0)

#endif
