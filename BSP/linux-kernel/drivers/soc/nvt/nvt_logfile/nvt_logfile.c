/**
    NVT logfile function
    This file will handle NVT logfile function
    @file logfile.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/kexec.h>
#include <linux/profile.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/capability.h>
#include <linux/compiler.h>
#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/sched/clock.h>
#include <plat/hardware.h>
#include <linux/module.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt_logfile.h>
#include <linux/semaphore.h>

#include "nvt_logfile_internal.h"

#define LOGFILE_INTERFACE_VER   0x21120200
#define LOGFILE_SYS_ERROR_KEY   MAKEFOURCC('S','Y','S','E')
#define PRINT_CTRL_CHAR_LEN     7
#define NVT_TIMER_TM0_CNT       0x108
#define LOGFILE_TMP_BUF_SIZE    (1 << CONFIG_LOG_CPU_MAX_BUF_SHIFT)

enum log_flags {
	LOG_ROLLBACK = 1,	/* log already rollback */
};

typedef struct {
	unsigned int        interface_ver;     ///< the Interface version of logfile ring buffer
	unsigned long       buf_size;          ///< the total buffer size for store log msg
	void *              data_head;         ///< head position of the data buffer
	void *              data_cur;          ///< current position of the data buffer
	unsigned int        sys_error;         ///< the kernel panic or some system error
	unsigned int        flags;             ///< the log flags
	unsigned int        reserved[8];       ///< reserved
} LOGFILE_RINGBUF_INFO;

static DEFINE_SEMAPHORE(g_logfile_sem);
static u64 g_timer0_offset = 0;
static char *gp_tmpbuf = NULL;

static LOGFILE_RINGBUF_INFO* logfile_get_info(void)
{
	static LOGFILE_RINGBUF_INFO *st_info = NULL;

	unsigned long buf_size;
	LOGFILE_RINGBUF_INFO *p_tmpinfo = NULL;
	struct dma_buf* logfile_dmabuf;

	if (st_info) {
		if (st_info->interface_ver != LOGFILE_INTERFACE_VER) {
			return NULL;
		}
		return st_info; //already initialized
	}

	down(&g_logfile_sem);

	logfile_dmabuf = logfile_dmabuf_get_obj();
	if (!logfile_dmabuf) {
		up(&g_logfile_sem);
		return NULL;
	}

	/*
	 * Get buffer information from shared buffer object and
	 * map a page of the buffer object into kernel address space.
	 */
	buf_size = (unsigned long)logfile_dmabuf->size;

	p_tmpinfo = (LOGFILE_RINGBUF_INFO *)dma_buf_kmap(logfile_dmabuf, buf_size/PAGE_SIZE);
	if (NULL == p_tmpinfo) {
		up(&g_logfile_sem);
		return NULL;
	}

	p_tmpinfo->interface_ver = LOGFILE_INTERFACE_VER;
	p_tmpinfo->buf_size = buf_size - ALIGN(sizeof(LOGFILE_RINGBUF_INFO), 16);
	p_tmpinfo->data_head = (void *)p_tmpinfo + ALIGN(sizeof(LOGFILE_RINGBUF_INFO), 16);
	p_tmpinfo->data_cur = p_tmpinfo->data_head;
	p_tmpinfo->sys_error = 0;
	p_tmpinfo->flags = 0;

	/* Read nvt timer0 for log time string. */
	//g_timer0_offset = nvt_readl(NVT_TIMER_BASE_VIRT + NVT_TIMER_TM0_CNT);
	//g_timer0_offset *= 1000;

	st_info = p_tmpinfo;
	up(&g_logfile_sem);

	return st_info;
}

static size_t logfile_print_time(u64 ts, char *buf, int bufsize)
{
	unsigned long rem_nsec;

	rem_nsec = do_div(ts, 1000000000);

	return snprintf(buf, bufsize, "[%5lu.%06lu] ", (unsigned long)ts, rem_nsec / 1000);
}

static int logfile_skip_head(const char *s)
{
	char *in = (char*)s;

	if (strncmp(s, "\033[0;3", 5) == 0 || strncmp(s, "\033[1;3", 5) == 0) {
		in += 6;
		if (*in == 'm')
			return 7;
	}

	return 0;
}

static void logfile_memcpy(void *dest, const void *src, size_t count, int kernel_space)
{
	if (gp_tmpbuf) {
		//use a local temp buffer to avoid the limitations of io buffers
		if (kernel_space) {
			memcpy(gp_tmpbuf, src, count);
		} else {
			_copy_from_user(gp_tmpbuf, src, count);
		}
		memcpy_toio(dest, gp_tmpbuf, count);
	} else {
		if (kernel_space) {
			memcpy(dest, src, count);
		} else {
			_copy_from_user(dest, src, count);
		}
	}
}

void logfile_save_str(const char *s, size_t count, int kernel_space)
{
	const char          *p_instr;
	char                strbuf_time[64] = {0};
	unsigned int        time_len = 0;
	void                *p_data_cur;
	unsigned int        instrlen, remain_len;
	LOGFILE_RINGBUF_INFO *p_info;
	u64                 ts_nsec;
	int                 is_addtime_nextline = 0;
	static int          is_addtime = 1;
	char                strbuf_start[PRINT_CTRL_CHAR_LEN];
	char                strbuf_end[1];
	int                 skip_len;

	p_info = logfile_get_info();
	if (NULL == p_info) {
		return;
	}

	ts_nsec = local_clock() + g_timer0_offset;

	p_data_cur = p_info->data_cur;

	instrlen = count;
	p_instr = s;

	//skip the first color code
	if (count >= PRINT_CTRL_CHAR_LEN) {
		logfile_memcpy(&strbuf_start[0], p_instr, PRINT_CTRL_CHAR_LEN, kernel_space);

		skip_len = logfile_skip_head(&strbuf_start[0]);
		instrlen -= skip_len;
		p_instr += skip_len;
	}

	if (instrlen < 1 || instrlen > p_info->buf_size)
		return;

	/* Check new line and need to add time string at next line. */
	logfile_memcpy(&strbuf_end[0], p_instr + instrlen - 1, 1, kernel_space);
	if (strbuf_end[0] == '\n')
		is_addtime_nextline = 1;

	/* Add log time string. */
	if (is_addtime) {
		time_len = logfile_print_time(ts_nsec, strbuf_time, sizeof(strbuf_time));

		remain_len = (p_info->data_head + p_info->buf_size - p_data_cur);
		if (time_len <= remain_len) {
			logfile_memcpy(p_data_cur, &strbuf_time[0], time_len, 1);
			p_data_cur += time_len;
		} else {
			p_info->flags |= LOG_ROLLBACK;
			logfile_memcpy(p_data_cur, &strbuf_time[0], remain_len, 1);
			p_data_cur = p_info->data_head;
			logfile_memcpy(p_data_cur, &strbuf_time[0] + remain_len, time_len - remain_len, 1);
			p_data_cur += (time_len - remain_len);
		}
	}

	/* Add log text string */
	remain_len = (p_info->data_head + p_info->buf_size - p_data_cur);
	if (instrlen <= remain_len) {
		logfile_memcpy(p_data_cur, p_instr, instrlen, kernel_space);
		p_data_cur += instrlen;
	} else {
		p_info->flags |= LOG_ROLLBACK;
		logfile_memcpy(p_data_cur, p_instr, remain_len, kernel_space);
		p_data_cur = p_info->data_head;
		logfile_memcpy(p_data_cur, p_instr + remain_len, instrlen - remain_len, kernel_space);
		p_data_cur += (instrlen - remain_len);
	}

	p_info->data_cur = p_data_cur;
	is_addtime = is_addtime_nextline;
}
EXPORT_SYMBOL(logfile_save_str);

void logfile_save_syserr(void)
{
	LOGFILE_RINGBUF_INFO *p_info;

	p_info = logfile_get_info();
	if (NULL == p_info) {
		return;
	}

	/* Add sys error key. */
	p_info->sys_error = LOGFILE_SYS_ERROR_KEY;
}
EXPORT_SYMBOL(logfile_save_syserr);

static int logfile_check_ep_module(void)
{
	nvtpcie_chipid_t my_chipid;
	int ep_count;
	int ep_idx;
	logfile_shmblk_t *p_shm;
	int ret = 0;

	my_chipid = nvtpcie_get_my_chipid();
	ep_count = nvtpcie_get_ep_count();

	p_shm = nvtpcie_shmblk_get(NVT_LOGFILE_NAME, sizeof(logfile_shmblk_t));
	if (NULL == p_shm) {
		DBG_ERR("shmblk_get failed\r\n");
		return -1;
	}

	if (CHIP_RC == my_chipid) { //I am RC
		for (ep_idx = 0; ep_idx < ep_count; ep_idx++) {
			if (p_shm->flag[ep_idx] & LOGFILE_FLAG_USING) {
				DBG_INFO("ERR: Please remove ep%d module first\r\n", ep_idx);
				ret = -1;
			}
		}
	} else { //I am EP
		ep_idx = my_chipid - CHIP_EP0;
		p_shm->flag[ep_idx] &= ~LOGFILE_FLAG_USING;
	}

	return ret;
}

static int __init logfile_init(void)
{
	if (CHIP_RC != nvtpcie_get_my_chipid()) {
		DBG_IND("alloc tmp buf 0x%X\r\n", LOGFILE_TMP_BUF_SIZE);
		gp_tmpbuf = kzalloc(LOGFILE_TMP_BUF_SIZE, GFP_KERNEL);
		if (NULL == gp_tmpbuf) {
			DBG_ERR("alloc tmp buf failed\r\n");
			return -1;
		}
	}

	logfile_set_printk_ptr(logfile_save_str);
	logfile_set_tty_ptr(logfile_save_str);
	logfile_set_panic_ptr(logfile_save_syserr);

	return 0;
}

static void __exit logfile_exit(void)
{
	logfile_set_printk_ptr(NULL);
	logfile_set_tty_ptr(NULL);
	logfile_set_panic_ptr(NULL);

	if (gp_tmpbuf) {
		kfree(gp_tmpbuf);
	}

	//release the reference count, or the dmabuf module can not be removed
	if (0 == logfile_check_ep_module()) {
		dma_buf_put(logfile_dmabuf_get_obj());
	}
}

module_init(logfile_init);
module_exit(logfile_exit);

MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("NVT_LOGFILE");
MODULE_VERSION(NVT_LOGFILE_VERSION);
