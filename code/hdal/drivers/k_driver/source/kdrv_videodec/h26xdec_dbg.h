#ifndef _H26X_DBG_H_
#define _H26X_DBG_H_

#include "vpu/hevc_dec_entity.h"

#define INCLUDE_HVYLOAD             0   /* use heavyload ddr_chksum for debugging */

// 0U: unknown
#define JOB_STATUS_STANDBY 	0x1 // 1S not process yet.
#define JOB_STATUS_ONGOING 	0x2	// 2O the job is being in hardware or LL
#define JOB_STATUS_KEEP		0x4 // 3K wait poc. ISR done. Job is processed by HW engine and is waiting for POC to be determined by lowwer level decoder driver
#define JOB_STATUS_DPB_REL	0x8	// 4D DPB release.
#define JOB_STATUS_FLUSH	0x10// 5FL channel stop. All standby jobs change to flush without entering hw.
#define JOB_STATUS_FAIL   	0x20// 6FA fail
#define JOB_STATUS_DONE    	(JOB_STATUS_KEEP | JOB_STATUS_DPB_REL | JOB_STATUS_FLUSH |  JOB_STATUS_FAIL)

/* Get job status string */
const char *h26xd_job_status_str(unsigned int status);
const char *h26xd_job_status_long_str(unsigned int status);
const char *h26xd_err_num_str(enum h26xd_err_type err_type);

/* OR job_item status in VG flow */
#define SET_JOB_STATUS(job_ptr, st) do{         \
            (job_ptr)->status = (unsigned char)(st);         	\
        } while(0)

#define GET_JOB_STATUS(job_ptr)	(job_ptr)->status

int h26xd_log_printout_handler(uintptr_t data);
void h26xd_save_bitstream_to_file(struct h26xd_job_item_t *job_item);

#if INCLUDE_HVYLOAD
void h26xdec_debug_init(void);
u16 h26xdec_heavyload_checksum(uintptr_t phy_addr, unsigned int length);
void h26xdec_debug_uninit(void);
#endif

#endif /* _H26X_DBG_H_ */