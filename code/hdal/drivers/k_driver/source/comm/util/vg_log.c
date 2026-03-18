/*
 *   @file   vg_log.c
 *
 *   @brief  The Log System to record debug information.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#define _LOG_C_
#include <linux/module.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/synclink.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/statfs.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/thread_info.h>
#include <linux/slab.h>
#include <linux/bitmap.h>
#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#else
#include <mach/fmem.h>
#endif
#include <kwrap/cpu.h>
#include "kwrap/util.h"
#include <linux/soc/nvt/nvtmem.h>

#include "vg_common.h"
#include "log.h"
#include "info.h"
#include "vg_memcpy.h"
#include "vg_log_core.h"
#include "isf_debug_log.h"
#include "kwrap/spinlock.h"

#define MODULE_NAME "LO"

#define BIT_BUSY_WRITE      0
#define BIT_PANIC           1
#define BIT_BUSY_DUMP_LOG   2
#define BIT_BUSY_DUMP_BUF   3
DECLARE_BITMAP(log_state, 32);

#define FUNC_LOG        0
#define FUNC_DUMPBUF    1
static unsigned int write_function = FUNC_LOG;

static struct proc_dir_entry *videoproc, *debugproc, *modeproc, *dump2proc, *affinityproc;
static struct proc_dir_entry *delay1proc, *delay2proc, *gmlib_setting_proc, *gmlib_flow_proc;
static struct proc_dir_entry *gmlib_err_proc, *threadproc;
static struct proc_dir_entry *hdal_proc, *hdal_setting_proc, *hdal_flow_proc, *hdal_version_proc, *hdal_err_proc, *hdal_dbglevel_proc;
/*
static unsigned int hdal_version = 0;
static unsigned int impl_version = 0;
*/
struct task_struct  *write_task, *notify_task, *monitor_task;
static struct timespec64 log_mod_insert_time;
static int is_already_write = 0;
static char *global_proc_msg = 0;

/* gmlib_proc_bind, need to sync the value in vg_log.c */
/*
  |--SETTING_MSG_SIZE--|--FLOW_MSG_SIZE--|--ERR_MSG_SIZE--|--HDAL_SETTING_MSG_SIZE--|--HDAL_FLOW_MSG_SIZE--|
                        <-- MSG_LENGTH_SIZE                                          <-- MSG_LENGTH_SIZE
                         <--MSG_OFFSET_SIZE                                           <--MSG_OFFSET_SIZE
                                          <-- MSG_LENGTH_SIZE
                                           <--MSG_OFFSET_SIZE

 */
/*
#define MSG_LENGTH_SIZE       8
#define MSG_OFFSET_SIZE       8

#define SETTING_MSG_SIZE      (96 * 1024)
#define FLOW_MSG_SIZE         (128 * 1024)
#define ERR_MSG_SIZE          (32 * 1024)
#define HDAL_SETTING_MSG_SIZE (96 * 1024)
#define HDAL_FLOW_MSG_SIZE    (128 * 1024)
#define MMAP_MSG_LEN          (SETTING_MSG_SIZE + FLOW_MSG_SIZE + ERR_MSG_SIZE + HDAL_SETTING_MSG_SIZE + HDAL_FLOW_MSG_SIZE)

#define SETTING_MSG_OFFSET(x)         (x)
#define FLOW_MSG_OFFSET(x)            (SETTING_MSG_OFFSET(x) + SETTING_MSG_SIZE)
#define ERR_MSG_OFFSET(x)             (FLOW_MSG_OFFSET(x) + FLOW_MSG_SIZE)
#define HDAL_SETTING_MSG_OFFSET(x)    (ERR_MSG_OFFSET(x) + ERR_MSG_SIZE)
#define HDAL_FLOW_MSG_OFFSET(x)       (HDAL_SETTING_MSG_OFFSET(x) + HDAL_SETTING_MSG_SIZE)

char *mmap_msg = 0;

uintptr_t log_base_start = 0;
unsigned int log_bsize = 0, log_base_ddr = 0, log_base_vmalloc = 0;
static uintptr_t log_base_end = 0;
static uintptr_t log_start_ptr = 0;
static unsigned int log_real_size = 0;
static unsigned int log_ksize = 1024;
*/
#define LOOP_DETECTION  0x1
#define CPUID_LOG       0x2
static unsigned int feature = 0; //0x1:loop detect   0x2:cpuID+ISR

module_param(log_ksize, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(log_ksize, "log_ksize");

module_param(feature, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(feature, "feature");

module_param(mode, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(mode, "mode");

#define MAX_PATH_WIDTH  60
static char dump_path[MAX_PATH_WIDTH + 1] = "/mnt/nfs";

static char *crash_notify = "/mnt/mtd/crash.sh";
module_param(crash_notify, charp, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(crash_notify, "Set crash notify script for user");

#define LOG_VERSION "v2.12"

#define MAX_VERSION_LEN 10
static char version[MAX_VERSION_LEN] = "v0.7";

#define MAX_CHAR 512
#define HDA_FLOW_STR_SZ	256
struct panic_func_t {
	int registered;
	int (*panic_notifier)(uintptr_t);
};

struct printout_func_t {
	int registered;
	int (*printout_notifier)(uintptr_t);
};

#define MAX_CB  40
struct panic_func_t     log_panic_cb[MAX_CB];
struct printout_func_t  log_printout_cb[MAX_CB];
struct printout_func_t log_master_print_cb;
struct printout_func_t log_hdal_proc_cb;
struct printout_func_t log_hdal_flow_cb;
struct printout_func_t log_gmlib_flow_cb;
#define MAX_HDAL_CMD_BUF_LENGTH    63
char hdal_cmd_buffer[MAX_HDAL_CMD_BUF_LENGTH + 1];

wait_queue_head_t log_wq;

#define LOG_NOTIFY_NONE     0
#define LOG_NOTIFY_START    1
#define LOG_NOTIFY_DONE     2
static int log_notify_state = LOG_NOTIFY_NONE; //1:start  2:done
static int panic_notify_state = LOG_NOTIFY_NONE; //1:start  2:done
unsigned int is_panic = 0;

void write_process(unsigned long data);


static void *seq_gmlib_err_start(struct seq_file *s, loff_t *pos);
static void *seq_gmlib_flow_start(struct seq_file *s, loff_t *pos);
static void *seq_gmlib_setting_start(struct seq_file *s, loff_t *pos);
#if VG_LOG_FULL_FUNC
static void *seq_hdal_setting_start(struct seq_file *s, loff_t *pos);
#endif
static void *seq_hdal_flow_start(struct seq_file *s, loff_t *pos);

void *seq_next(struct seq_file *s, void *v, loff_t *pos);
void seq_stop(struct seq_file *s, void *v);
int seq_show(struct seq_file *s, void *v);

static struct seq_operations seq_gmlib_err_ops = {
	.start = seq_gmlib_err_start,
	.next  = seq_next,
	.stop  = seq_stop,
	.show  = seq_show
};
static struct seq_operations seq_gmlib_flow_ops = {
	.start = seq_gmlib_flow_start,
	.next  = seq_next,
	.stop  = seq_stop,
	.show  = seq_show
};
static struct seq_operations seq_gmlib_setting_ops = {
	.start = seq_gmlib_setting_start,
	.next  = seq_next,
	.stop  = seq_stop,
	.show  = seq_show
};
#if VG_LOG_FULL_FUNC
static struct seq_operations seq_hdal_setting_ops = {
	.start = seq_hdal_setting_start,
	.next  = seq_next,
	.stop  = seq_stop,
	.show  = seq_show
};
#endif
static struct seq_operations seq_hdal_flow_ops = {
	.start = seq_hdal_flow_start,
	.next  = seq_next,
	.stop  = seq_stop,
	.show  = seq_show
};

//seq_iteration_read
#define MAX_ITERATION_PRINT_SIZE 4096
typedef struct {
	char *print_buf;
	unsigned int remain_size;
} proc_print_buf_t;

/**
 * move the iterator forward to the next position in the sequence
 */
void *seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	proc_print_buf_t *v_in;

	(*pos)++;

	v_in = (proc_print_buf_t *) v;
	if (v_in->remain_size >= MAX_ITERATION_PRINT_SIZE) { //next->show
		v_in->print_buf = v_in->print_buf + MAX_ITERATION_PRINT_SIZE;
		v_in->remain_size =  v_in->remain_size - MAX_ITERATION_PRINT_SIZE;
	} else { //last one
		v_in->print_buf = NULL;
		v_in->remain_size = 0;
		kfree(v);
		return NULL; //prepare to do end process
	}
	return (void *) v_in;
}

/**
 * stop() is called when iteration is complete (clean up)
 */
void seq_stop(struct seq_file *s, void *v)
{
	if (v) {
		kfree(v);
	}
}

/**
 * success return 0, otherwise return error code
 */
int seq_show(struct seq_file *s, void *v)
{
	proc_print_buf_t *v_in;

	v_in = (proc_print_buf_t *)v;

	if (v_in->remain_size >= MAX_ITERATION_PRINT_SIZE) {
		seq_write(s, (void *) v_in->print_buf, MAX_ITERATION_PRINT_SIZE);
	} else {
		seq_write(s, (void *) v_in->print_buf, v_in->remain_size);
	}

	return 0; //continue to print
}

//return only 0->1
int test_and_wait_bit(int bitmap, unsigned long *state)
{
	int timeout = 1000;
	while (test_and_set_bit(bitmap, state) && (timeout > 0)) { //while if bit is already set
		if (in_interrupt()) {
			udelay(10);
		} else {
			msleep(1);
		}
		timeout--;
	}
	if (timeout <= 0) {
		return 1;
	}
	return 0; //bit is 0, and successful to set the bit
}



unsigned int in_busyloop = 0;
#define PRINTM_PER_LOOPS 100
/* busyloop condiction:
    during 2s printm 160000 (under 800MIPS) ==> printm 100 loops/MIPS,second
 */
inline void busyloop_detection(char *module_char)
{
	static unsigned int start_jiffies = 0, end_jiffies = 0, printm_count = 0;
	unsigned int loops = (unsigned int)(loops_per_jiffy / (500000 / HZ));
	unsigned int diff_ms;

	if ((feature & LOOP_DETECTION) != 0x1) {
		return;
	}
	if (start_jiffies == 0) {
		start_jiffies = get_nvt_jiffies();
		return;
	}
	end_jiffies = get_nvt_jiffies();
	printm_count++;

	diff_ms = (int)end_jiffies - (int)start_jiffies;

#define DETECT_MS 5000
	if (diff_ms > DETECT_MS) {
		//printk("printm_count %u %u\n",printm_count, (loops * PRINTM_PER_LOOPS));
		if (printm_count > (loops * PRINTM_PER_LOOPS * (DETECT_MS / 1000))) {
			uintptr_t start;
			unsigned int size;
			in_busyloop = 1;
			printk("printm busyloop detected! (%ds count %u over %u at %uMIPS)\n",
			       DETECT_MS / 1000, printm_count, (loops * PRINTM_PER_LOOPS * (DETECT_MS / 1000)), loops);
			dump_stack();
			calculate_log(&start, &size);
			printk("==========================================================================\n");
			prepare_dump_console(start, size);
			dump_stack();
			panic("printm busyloop detected! (%ds count %u over %u at %uMIPS)\n",
			      DETECT_MS / 1000, printm_count, (loops * PRINTM_PER_LOOPS * (DETECT_MS / 1000)), loops);
		}
		start_jiffies = printm_count = 0;
	}
}

//
void master_print(const char *fmt, ...)
{
	int len = 0;
	va_list args;
	char log[MAX_CHAR];

	if (log_master_print_cb.registered == 0) {
		return;
	}

	va_start(args, fmt);
	len = vsnprintf(log, sizeof(log), fmt, args);
	va_end(args);
	log_master_print_cb.printout_notifier((uintptr_t) &log[0]);
}

void wait_for_notify(void)
{
	if (write_function == FUNC_LOG) {
		printk("[LOG] Notifying...\n");
		if (is_panic) {
			panic_notify_state = LOG_NOTIFY_START;
		}
		log_notify_state = LOG_NOTIFY_START;
		wake_up_process(notify_task);
	}

	if (write_function == FUNC_LOG) {
		wait_event_timeout(log_wq, (panic_notify_state == LOG_NOTIFY_NONE) &&
				   (log_notify_state == LOG_NOTIFY_NONE), vos_util_msec_to_tick(1000));
		if ((panic_notify_state != LOG_NOTIFY_DONE) && (panic_notify_state != LOG_NOTIFY_NONE)) {
			printk("Error to wait panic notifier function return well (1000ms timeout)!\n");
			panic_notify_state = LOG_NOTIFY_NONE;
		}
		if ((log_notify_state != LOG_NOTIFY_DONE) && (log_notify_state != LOG_NOTIFY_NONE)) {
			printk("Error to wait notifier function return well (1000ms timeout)!\n");
			log_notify_state = LOG_NOTIFY_NONE;
		}
	}
}

void wake_up_log_write(void)
{
	sprintf(log_path[0], "%s/log.txt", dump_path);
	write_function = FUNC_LOG;

	printk("[LOG] Notifying...\n");
	if (is_panic) {
		panic_notify_state = LOG_NOTIFY_START;
	}
	log_notify_state = LOG_NOTIFY_START;
	wake_up_process(notify_task);
	wake_up_process(write_task);
}

void wake_up_file_write(uintptr_t va, dump_info_t *dump_info, unsigned int counts, char *path)
{
	unsigned int i, offset = 0;
	log_slice_ptr[0] = va;

	memset(log_size, 0, sizeof(log_size));
	for (i = 0; i < counts; i++) {
		log_slice_ptr[i] = va + offset;
		log_size[i] = dump_info[i].size;
		sprintf(log_path[i], "%s/%s", path, (strlen(dump_info[i].filename) != 0) ? dump_info[i].filename : "buffer.dat");
		offset += dump_info[i].size;
	}
	write_function = FUNC_DUMPBUF;
	wake_up_process(write_task);
}

void write_process(unsigned long data)
{
	int ret = 0, i;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	mm_segment_t fs;
#endif
	unsigned long long offset = 0;
	struct file *filp[10];
	uintptr_t start = 0;
	unsigned int size = 0;

	if (write_function == FUNC_LOG) {
		wait_event_timeout(log_wq, (panic_notify_state == LOG_NOTIFY_NONE) &&
				   (log_notify_state == LOG_NOTIFY_NONE), vos_util_msec_to_tick(3000));
		if ((panic_notify_state != LOG_NOTIFY_DONE) && (panic_notify_state != LOG_NOTIFY_NONE)) {
			printk("Error to wait panic notifier function return well (1000ms timeout)!\n");
			panic_notify_state = LOG_NOTIFY_NONE;
		}
		if ((log_notify_state != LOG_NOTIFY_DONE) && (log_notify_state != LOG_NOTIFY_NONE)) {
			printk("Error to wait notifier function return well (1000ms timeout)!\n");
			log_notify_state = LOG_NOTIFY_NONE;
		}
		//calculate pointer
		calculate_log(&start, &size);
		if (mode == MODE_STORAGE) {
			prepare_dump_storage(start, size);
		} else {
			prepare_dump_console(start, size);
		}
		printk("[LOG] Dumping log %d bytes...\n", log_size[0]);
	} else { //FUNC_DUMPBUF
		for (i = 0; i < MAX_DUMP_FILE; i++) {
			if (log_size[i] == 0) {
				break;
			}
			printk("[LOG] Dumping buffer %d bytes... va(%#lx)\n", log_size[i], log_slice_ptr[i]);
		}
	}
	if (test_and_set_bit(BIT_BUSY_WRITE, log_state)) { //fail
		printk("[LOG]already busy write process!\n");
		return;
	}
	printk("[LOG] Log Writing...\n");
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	fs = get_fs();
	set_fs(KERNEL_DS);
#else
#endif
	if ((log_slice_ptr[0] == 0) || (log_size[0] == 0) || (strlen(log_path[0]) == 0)) {
		printk("---NO Log Message---\n");
		goto returnit;
	}

	if (write_function == FUNC_LOG) {
		filp[0] = filp_open(log_path[0], O_WRONLY | O_CREAT | O_TRUNC, 0777);
		if (IS_ERR(filp[0])) {
			printk("Error to open %s\n", log_path[0]);
			goto returnit;
		}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
		ret = vfs_write(filp[0], (unsigned char *)log_slice_ptr[0], log_size[0], &offset);
		if (log_size[1] && (ret > 0)) {
			ret = vfs_write(filp[0], (unsigned char *)log_slice_ptr[1], log_size[1], &offset);
		}
#else
		ret = kernel_write(filp[0], (unsigned char *)log_slice_ptr[0], log_size[0], &offset);
		if (log_size[1] && (ret > 0)) {
			ret = kernel_write(filp[0], (unsigned char *)log_slice_ptr[1], log_size[1], &offset);
		}
#endif
		filp_close(filp[0], NULL);
		vfs_fsync(filp[0], 0);
	}
	if (write_function == FUNC_DUMPBUF) {
		for (i = 0; i < MAX_DUMP_FILE; i++) {
			if (log_size[i] == 0) {
				break;
			}
			filp[i] = filp_open(log_path[i], O_WRONLY | O_CREAT | O_TRUNC, 0777);
			if (IS_ERR(filp[i])) {
				printk("Error to open %s\n", log_path[i]);
				goto returnit;
			}
			offset = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
			ret = vfs_write(filp[i], (unsigned char *)log_slice_ptr[i], log_size[i], &offset);
#else
			ret = kernel_write(filp[i], (unsigned char *)log_slice_ptr[i], log_size[i], &offset);
#endif
			filp_close(filp[i], NULL);
			vfs_fsync(filp[i], 0);
			if (ret < 0) {
				break;
			}
		}
	}
	if (write_function == FUNC_LOG) {
		printk("Write Log 0x%lx(%d bytes) && 0x%lx(0x%x) to %s\n",
		       log_slice_ptr[0], log_size[0], log_slice_ptr[1], log_size[1], log_path[0]);
	} else if (write_function == FUNC_DUMPBUF) {
		for (i = 0; i < MAX_DUMP_FILE; i++) {
			if (log_size[i] == 0) {
				break;
			}
			printk("Write Buffer 0x%lx(%d bytes) to %s\n", log_slice_ptr[i], log_size[i], log_path[i]);
		}
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	set_fs(fs);
#endif

	if (ret) {
		printk("\n======================\n Write DONE!!!!!!\n======================\n");
	} else {
		printk("\n======================\n Write FAIL!!!!!!\n======================\n");
	}

returnit:
	clear_bit(BIT_BUSY_WRITE, log_state);
	clear_bit(BIT_BUSY_DUMP_BUF, log_state);
	clear_bit(BIT_BUSY_DUMP_LOG, log_state);
}

static int write_thread(void *__cwq)
{
	set_current_state(TASK_INTERRUPTIBLE);
	set_user_nice(current, -20);
	while (!kthread_should_stop()) {
		schedule();
		__set_current_state(TASK_RUNNING);
		write_process(0);
		set_current_state(TASK_INTERRUPTIBLE);
	}
	__set_current_state(TASK_RUNNING);
	return 0;
}

#define TIMEOUT_VALUE_MS    100
static int thread_nice = 10, timeout_threadhold = 20;
static int cpu_state = 0; //0:normal 1:busy 2:blocking
static int monitor_thread(void *__cwq)
{
	unsigned int start_jiffies, end_jiffies, over_counter = 0, current_diff;
	set_current_state(TASK_INTERRUPTIBLE);
	set_user_nice(current, thread_nice);

	while (!kthread_should_stop()) {
		start_jiffies = get_nvt_jiffies();
		set_current_state(TASK_INTERRUPTIBLE);

		schedule_timeout(vos_util_msec_to_tick(TIMEOUT_VALUE_MS));
		__set_current_state(TASK_RUNNING);
		end_jiffies = get_nvt_jiffies();

		current_diff = (int)end_jiffies - (int)start_jiffies;

		if (current_diff > TIMEOUT_VALUE_MS + (timeout_threadhold * 2)) {
			cpu_state = 2;
			over_counter++;
		} else if (current_diff > TIMEOUT_VALUE_MS + timeout_threadhold) {
			cpu_state = 1;
			over_counter++;
		} else if (over_counter) {
			over_counter--;
		}

		if (over_counter == 0) {
			cpu_state = 0;
		}

		//printk("diff=%d state %d counter %d\n",current_diff, cpu_state, over_counter);
	}
	__set_current_state(TASK_RUNNING);
	return 0;
}

int get_cpu_state(void)
{
	return cpu_state;
}

static int notify_thread(void *__cwq)
{
	int i;
	set_current_state(TASK_INTERRUPTIBLE);
	set_user_nice(current, -20);
	while (!kthread_should_stop()) {
		if ((log_notify_state != LOG_NOTIFY_START) && (panic_notify_state != LOG_NOTIFY_START)) {
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(vos_util_msec_to_tick(100));
		}
		__set_current_state(TASK_RUNNING);

		if (panic_notify_state == LOG_NOTIFY_START) {
			char *argv1[] = {"/bin/sh", "                                ", NULL};
			static char *envp[] = {"HOME=/root", "TERM=vt102", "PATH=/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/bin:/sbin", NULL};
			int ret;

			for (i = 0; i < MAX_CB; i++) {
				if (log_panic_cb[i].registered) {
					printk("[LOG] Damnit calling (%pS)",
					       log_panic_cb[i].panic_notifier);
					printk("  at 0x%x\n", (int)get_nvt_jiffies() & 0xffff);
					log_panic_cb[i].panic_notifier(0);
				}
			}

			if (strlen(crash_notify) > strlen(argv1[1])) {
				printk("Error crash_notify lenth, must less %zd\n", strlen(argv1[1]));
			} else {
				struct file *cfile;
				cfile = filp_open(crash_notify, O_WRONLY, 0600);
				if (IS_ERR(cfile)) {
					printk("Error to open %s\n", crash_notify);
					ret = -1;
				} else {
					filp_close(cfile, NULL);
					strcpy(argv1[1], crash_notify);
					ret = call_usermodehelper(argv1[0], argv1, envp, UMH_WAIT_PROC);
				}
				printk("[AP] Notify %s (return %d)\n", crash_notify, ret);
			}
			panic_notify_state = LOG_NOTIFY_DONE;
		}
		if (log_notify_state == LOG_NOTIFY_START) {
			for (i = 0; i < MAX_CB; i++) {
				if (log_printout_cb[i].registered) {
					printk("[LOG] Log printout calling (%pS)",
					       log_printout_cb[i].printout_notifier);
					printk("  at 0x%x\n", (int)get_nvt_jiffies() & 0xffff);
					log_printout_cb[i].printout_notifier(0);
				}
			}
			log_notify_state = LOG_NOTIFY_DONE;
		}
		set_current_state(TASK_INTERRUPTIBLE);
	}
	__set_current_state(TASK_RUNNING);
	return 0;
}

void register_version(char *ver)
{
	if (strlen(ver) < MAX_VERSION_LEN) {
		strcpy(version, ver);
	}
}

/*
 * @brief register the hdal proc notify function callback
 *
 * @function int register_hdal_proc_notifier(callback)
 * @param callback when it need to print message on Master console.
 * @return 1 on success, !0 on error
*/
int register_hdal_proc_notifier(int (*func)(uintptr_t))
{
	if (log_hdal_proc_cb.registered == 1) {
		printk("Double regiter to master print notifier(%p, %p)!\n",
		       func, log_hdal_proc_cb.printout_notifier);
		return -1;
	}
	log_hdal_proc_cb.registered = 1;
	log_hdal_proc_cb.printout_notifier = func;
	return 1;
}

/*
 * @brief register the hdal flow notify function callback
 *
 * @function int register_hdal_flow_notifier(callback)
 * @param callback when it need to print message on Master console.
 * @return 1 on success, !0 on error
*/
int register_hdal_flow_notifier(int (*func)(uintptr_t))
{
	if (log_hdal_flow_cb.registered == 1) {
		printk("Double regiter to master print notifier(%p, %p)!\n",
		       func, log_hdal_flow_cb.printout_notifier);
		return -1;
	}
	log_hdal_flow_cb.registered = 1;
	log_hdal_flow_cb.printout_notifier = func;
	return 1;
}

/*
 * @brief register the gmlib flow notify function callback
 *
 * @function int register_gmlib_flow_notifier(callback)
 * @param callback when it need to print message on Master console.
 * @return 1 on success, !0 on error
*/
int register_gmlib_flow_notifier(int (*func)(uintptr_t))
{
	if (log_gmlib_flow_cb.registered == 1) {
		printk("Double regiter to master print notifier(%p, %p)!\n",
		       func, log_gmlib_flow_cb.printout_notifier);
		return -1;
	}
	log_gmlib_flow_cb.registered = 1;
	log_gmlib_flow_cb.printout_notifier = func;
	return 1;
}

/*
 * @brief register the master print function callback
 *
 * @function int register_master_print_notifier(callback)
 * @param callback when it need to print message on Master console.
 * @return 1 on success, !0 on error
*/
int register_master_print_notifier(int (*func)(uintptr_t))
{
	if (log_master_print_cb.registered == 1) {
		printk("Double regiter to master print notifier(%p, %p)!\n",
		       func, log_master_print_cb.printout_notifier);
		return -1;
	}
	log_master_print_cb.registered = 1;
	log_master_print_cb.printout_notifier = func;
	return 1;
}

/*
 * @brief register the panic function callback
  *
 * @function int register_panic_notifier(callback)
 * @param callback when panic occured callback will be triggered
 * @return 1 on success, !0 on error
*/
int register_panic_notifier(int (*func)(uintptr_t))
{
	int i;
	for (i = 0; i < MAX_CB; i++) {
		if (log_panic_cb[i].registered == 0) {
			log_panic_cb[i].registered = 1;
			log_panic_cb[i].panic_notifier = func;
			break;
		}
	}
	return 1;
}

/*
 * @brief unregister the panic function callback
  *
 * @function int unregister_panic_notifier(callback)
 * @param callback when panic occured callback will be triggered
 * @return 1 on success, !0 on error
*/
int unregister_panic_notifier(int (*func)(uintptr_t))
{
	int i;
	for (i = 0; i < MAX_CB; i++) {
		if (log_panic_cb[i].panic_notifier == func) {
			log_panic_cb[i].registered = 0;
			log_panic_cb[i].panic_notifier = NULL;
			break;
		}
	}
	return 1;
}

/*
 * @brief register the printout function callback
  *
 * @function int register_printout_notifier(callback)
 * @param callback when printout occured callback will be triggered
 * @return 1 on success, !0 on error
*/
int register_printout_notifier(int (*func)(uintptr_t))
{
	int i;
	for (i = 0; i < MAX_CB; i++) {
		if (log_printout_cb[i].registered == 0) {
			log_printout_cb[i].registered = 1;
			log_printout_cb[i].printout_notifier = func;
			break;
		}
	}
	return 1;
}

/*
 * @brief unregister the printout function callback
  *
 * @function int unregister_printout_notifier(callback)
 * @param callback when printout occured callback will be triggered
 * @return 1 on success, !0 on error
*/
int unregister_printout_notifier(int (*func)(uintptr_t))
{
	int i;
	for (i = 0; i < MAX_CB; i++) {
		if (log_printout_cb[i].printout_notifier == func) {
			log_printout_cb[i].registered = 0;
			log_printout_cb[i].printout_notifier = NULL;
			break;
		}
	}
	return 1;
}

/*
 * @brief dump log without damnit
 *
 * @function int dumplog(char *module)
 * @param module two characters module name
 * @return 1 on success, !0 on error
*/
int dumplog(char *module)
{
	struct timespec64 ts;

	if (is_already_write == 1) {
		printk("[LOG] Log exists and skip dumplog.\n");
		return 0;
	}

	ktime_get_real_ts64(&ts);
	printm("LG", "LOG1: system start H/M/S(%.2lu:%.2lu:%.2lu), log at H/M/S(%.2lu:%.2lu:%.2lu)\n",
	       (log_mod_insert_time.tv_sec / 3600) % (24), (log_mod_insert_time.tv_sec / 60) % (60),
	       log_mod_insert_time.tv_sec % 60, (ts.tv_sec / 3600) % (24), (ts.tv_sec / 60) % (60),
	       ts.tv_sec % 60);
	dump_stack();

	if (test_and_set_bit(BIT_BUSY_DUMP_LOG, log_state)) { //fail
		printk("[LOG]dump buffer already busy write1!\n");
		return 0;
	}
	printk("\nlog1 %s (%s %s) log...\n", LOG_VERSION, __DATE__, __TIME__);
	wake_up_log_write();
	is_already_write = 1;
	return 0;
}


/*
 * @brief enter damnit to notify panic
 *
 * @function int damnit(char *module)
 * @param module two characters module name
 * @return 1 on success, !0 on error
*/
int damnit(char *module)
{
	struct timespec64 ts;
	unsigned long pfun = (unsigned long)__builtin_return_address(0);

	if (is_already_write == 1) {
		printk("[LOG] Log exists and skip dumplog.\n");
		return 0;
	}
	printm(module, "###Error### damnit videograph v%s (%s %s) from 0x%lx\n", version, __DATE__,
	       __TIME__, (unsigned long) pfun);

	ktime_get_real_ts64(&ts);
	printm("LG", "LOG2: system start H/M/S(%.2lu:%.2lu:%.2lu), log at H/M/S(%.2lu:%.2lu:%.2lu)\n",
	       (log_mod_insert_time.tv_sec / 3600) % (24), (log_mod_insert_time.tv_sec / 60) % (60),
	       log_mod_insert_time.tv_sec % 60, (ts.tv_sec / 3600) % (24), (ts.tv_sec / 60) % (60),
	       ts.tv_sec % 60);

	if (test_and_set_bit(BIT_PANIC, log_state)) {
		printk("Damnit from (%pS), PANIC already\n", (void *)pfun);
		return 0;
	} else if (test_and_set_bit(BIT_BUSY_DUMP_LOG, log_state) ||
		   test_bit(BIT_BUSY_DUMP_BUF, log_state)) {
		printk("Damnit from (%pS), BUSY already\n", (void *)pfun);
		return 0;
	} else {
		printk("Damnit from (%pS)\n", (void *)pfun);
	}
	dump_stack();
	is_panic = 1;
	wake_up_log_write();
	is_already_write = 1;
	return 1;
}

void dumpbuf_pa(dump_info_t *dump_info, unsigned int counts, char *path)
{
	unsigned int total_size = 0, i, offset = 0;

	for (i = 0; i < counts; i++) {
		total_size += ALIGN16_UP(dump_info[i].size);
	}
	if (log_bsize < total_size) {
		printk("log_ksize=%dK, needs %dK, write as it can...", log_bsize / 1024, total_size / 1024);
	}

	if (log_base_vmalloc && (counts > 1)) { /* dump_info[0] is config file to skiped, dump_info[1] is real buffer */
		dump_info_t dump_info_va;
		uintptr_t addr_va;

		printk("##### Special dump under vmalloc space %#lx size %d (need DDR0)\n", (unsigned long)dump_info[1].pa, dump_info[1].size);
		addr_va = (uintptr_t)ioremap_cache(dump_info[1].pa, PAGE_ALIGN(dump_info[1].size));
		if (((void *)addr_va) == NULL) {
			printk("Error to do remap for %#lx size 0x%x\n", (unsigned long)dump_info[1].pa, dump_info[1].size);
			return;
		}
		vos_cpu_dcache_sync(addr_va, PAGE_ALIGN(dump_info[1].size), VOS_DMA_FROM_DEVICE);
		printk("dump addr_va %#lx size %d\n", (unsigned long)addr_va, PAGE_ALIGN(dump_info[1].size));
		dump_info_va.ddr_id = 0;
		dump_info_va.pa = 0;
		dump_info_va.size = dump_info[1].size;
		strcpy(dump_info_va.filename, dump_info[1].filename);
		memcpy((void *)log_base_start, (void *)addr_va, dump_info_va.size);
		iounmap((void *)addr_va);
		dumpbuf_va(dump_info_va.ddr_id, log_base_start, dump_info_va.size, dump_info_va.filename, path);
	} else {
		if (test_and_set_bit(BIT_BUSY_DUMP_BUF, log_state)) {
			printk("[LOG]dump buffer already busy write!\n");
			return;
		}
		vos_cpu_dcache_sync((VOS_ADDR)log_base_start, total_size, VOS_DMA_TO_DEVICE);
		for (i = 0; (i < counts) && (offset < log_bsize) ; i++) {
			int ret;
			dump_info[i].size = ALIGN16_UP(dump_info[i].size);
			if (log_bsize < offset + dump_info[i].size) {
				dump_info[i].size = log_bsize - offset;
			}
			ret = vg_memcpy(log_base_ddr, __pa(log_base_start + offset), dump_info[i].ddr_id, dump_info[i].pa, dump_info[i].size);
			if (ret < 0) {
				printk("Error to do vg_memcpy return value %d\n", ret);
			}
			offset += dump_info[i].size;
		}
		vos_cpu_dcache_sync((VOS_ADDR)log_base_start, offset, VOS_DMA_FROM_DEVICE);
		wake_up_file_write(log_base_start, dump_info, counts, path);
	}
}

void dumpbuf_va(int ddr_id, uintptr_t va, unsigned int size, char *filename, char *path)
{
	dump_info_t dump_info;
	if (test_and_set_bit(BIT_BUSY_DUMP_BUF, log_state)) {
		printk("[LOG]dump buffer already busy write!\n");
		return;
	}
	dump_info.ddr_id = ddr_id;
	dump_info.pa = __pa(va);
	dump_info.size = size;
	sprintf(dump_info.filename, "%s", filename);
	wake_up_file_write(va, &dump_info, 1, path);
}

int set_hdal_flow_dbglevel(HDAL_FLOW_DBG_MODULE module, unsigned int module_dbg_lvl)
{
	unsigned int *p_flow_dbg_lvl;

	p_flow_dbg_lvl = (unsigned int *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);
	if (module >= HDAL_FLOW_DBG_MAX) {
		printk("[LOG]ERR: module =%d\r\n", module);
		return -1;
	}
	p_flow_dbg_lvl[module] = module_dbg_lvl;
	return 0;

}

int get_hdal_flow_dbglevel(HDAL_FLOW_DBG_MODULE module)
{
	unsigned int *p_flow_dbg_lvl;

	p_flow_dbg_lvl = (unsigned int *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);
	if (module >= HDAL_FLOW_DBG_MAX) {
		printk("[LOG]ERR: module =%d\r\n", module);
		return 0;
	}
	return p_flow_dbg_lvl[module];
}

static int proc_affinity_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "\necho irq_number cpu_id(0,1,2...) > affinity\n");
	return 0;
}

#include <linux/kallsyms.h>
typedef int (*irq_set_affinity_ptr)(unsigned int irq, const struct cpumask *mask);
static ssize_t proc_affinity_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};
	int irq_num = 0, cpu_num = 0;
	irq_set_affinity_ptr irq_set_affinity = (irq_set_affinity_ptr)__symbol_get("__irq_set_affinity");

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}

	sscanf(ker_buffer, "%d %d\n", &irq_num, &cpu_num);
	printk("Set IRQ %d to CPU%d\n", irq_num, cpu_num);

	if (irq_set_affinity) {
		printk("Set IRQ %d to CPU%d\n", irq_num, cpu_num);
		irq_set_affinity(irq_num, get_cpu_mask(cpu_num));
	}
	return count;
}

static int proc_mode_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "\nDebug Mode %d (0:storage  1:dump console  2:direct print)\n", mode);
	return 0;
}

static ssize_t proc_mode_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};
	int mode_set = 0;

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}

	sscanf(ker_buffer, "%d\n", &mode_set);
	printk("Set to Mode %d (-1:disable 0:storage  1:dump console  2:direct print(forever)\n", mode_set);
	mode = mode_set;

	return count;
}

static int proc_dumplog_seq_show(struct seq_file *s, void *v)
{
	struct timespec64 ts;

	ktime_get_real_ts64(&ts);
	printm("LG", "LOG3: system start H/M/S(%.2lu:%.2lu:%.2lu), log at H/M/S(%.2lu:%.2lu:%.2lu)\n",
	       (log_mod_insert_time.tv_sec / 3600) % (24), (log_mod_insert_time.tv_sec / 60) % (60),
	       log_mod_insert_time.tv_sec % 60, (ts.tv_sec / 3600) % (24), (ts.tv_sec / 60) % (60),
	       ts.tv_sec % 60);
	if (test_and_set_bit(BIT_BUSY_DUMP_LOG, log_state)) { //fail
		seq_printf(s, "[LOG]dump buffer already busy write1!\n");
		return 0;
	}
	printk("\nlog3 %s (%s %s) log...\n", LOG_VERSION, __DATE__, __TIME__);
	wake_up_log_write();
	return 0;
}


static ssize_t proc_dumplog_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	if (count == 0 || count > MAX_PATH_WIDTH) {
		printk("Out-of-range path size count(%zd) max(%d)\n", count, MAX_PATH_WIDTH);
		return 0;
	}
	if (copy_from_user(dump_path, buffer, count)) {
		return 0;
	}
	dump_path[count - 1] = '\0';
	return count;
}

static int proc_delay_non_preemptive_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "Active a delay with non-preemptive:\n    #echo [delay msec] > delay_non_preemptive\n");
	return 0;
}


static VK_DEFINE_SPINLOCK(delay_lock);
static ssize_t proc_delay1_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};
	unsigned int delay;
	unsigned int start_jiffies = jiffies, end_jiffies = 0;

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}

	sscanf(ker_buffer, "%d", &delay);
	vk_spin_lock(&delay_lock); //preemptive disable

	end_jiffies = start_jiffies + vos_util_msec_to_tick(delay);
	while (jiffies != end_jiffies) //busy loop
		;
	vk_spin_unlock(&delay_lock);
	return count;
}

static int proc_delay_irq_disabled_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "Active a delay without ISR:\n    #echo [delay msec] > delay_irq_disabled\n");
	return 0;
}


static ssize_t proc_thread_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}

	sscanf(ker_buffer, "%d", &timeout_threadhold);
	printk("timeout_threadhold=%d\n", timeout_threadhold);
	return count;
}

static int proc_thread_realtime_test_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "CPU:%d threshold:%d\n", cpu_state, timeout_threadhold);
	return 0;
}

/* <----- setting message ----> <len(4bytes) - flow message>  */
static void *seq_gmlib_setting_start(struct seq_file *s, loff_t *pos)
{
	proc_print_buf_t *v;
	uintptr_t start_ptr;
	unsigned int total_print_size;
	unsigned int offset;
	static unsigned int msg_len = 0; //need keep for next time usage

	if (*pos == 0) {
		char *gmlib_setting_msg;
		unsigned int gmlib_setting_len;

		if ((!mmap_msg) || (!global_proc_msg))
			return NULL;
		gmlib_setting_msg = (char *) mmap_msg;
		gmlib_setting_len = strlen((char *)mmap_msg);
		msg_len = gmlib_setting_len;
		memcpy((void *) global_proc_msg, (void *) gmlib_setting_msg, msg_len);
	}

	start_ptr = (uintptr_t) global_proc_msg;  //fill start print pointer
	total_print_size = msg_len;      //fill print size

	offset = (*pos) * MAX_ITERATION_PRINT_SIZE;

	if (offset >= total_print_size) {
		return NULL; //end of print
	}

	v = (proc_print_buf_t *) kzalloc(sizeof(proc_print_buf_t), GFP_KERNEL);
	if (!v) {
		return NULL; //end of print
	}

	v->print_buf = (char *)(start_ptr + offset);
	v->remain_size = total_print_size - offset;

	return v;
}

char gmlib_dbg_mode[11] = "0";
static ssize_t gmlib_flow_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	if (count >= sizeof(gmlib_dbg_mode)) {
		printk("count %zd > max %zd\n", count, sizeof(gmlib_dbg_mode));
		return -EINVAL;
	}
	if (copy_from_user(gmlib_dbg_mode, buffer, count)) {
		return 0;
	}
	gmlib_dbg_mode[count] = '\0';
	if (log_gmlib_flow_cb.registered != 0) {
		log_gmlib_flow_cb.printout_notifier((uintptr_t)gmlib_dbg_mode);
	}
	printk("gmlib flow debug enable=%c (Usage: echo [0/1] > flow)\n", gmlib_dbg_mode[0]);
	return count;
}

static void *seq_gmlib_flow_start(struct seq_file *s, loff_t *pos)
{
	proc_print_buf_t *v;
	uintptr_t start_ptr;
	unsigned int total_print_size;
	unsigned int offset;
	static unsigned int msg_len = 0; //need keep for next time usage
	char *flow_msg, *start_addr;
	unsigned int flow_len;
	unsigned int flow_offset;

	if (gmlib_dbg_mode[0] == '0') {
		printk("/proc/videograph/gmlib_flow debug disabled (Usage: echo [0/1] > flow)\n");
		start_addr = FLOW_MSG_OFFSET(mmap_msg);
		*(unsigned int *)(start_addr) = *(unsigned int *)(start_addr + MSG_LENGTH_SIZE) = 0;
		return NULL;
	}

	if (*pos == 0) {
		if (!mmap_msg)
			return NULL;
		flow_msg = (char *)(FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);
		flow_len = *(unsigned int *)FLOW_MSG_OFFSET(mmap_msg);
		flow_offset = *(unsigned int *)(FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE);
		msg_len = flow_len;

		if (flow_offset == 0) {
			memcpy((void *)global_proc_msg, (void *)(flow_msg), msg_len);
		} else if (msg_len > flow_offset) {//to prevent negative situation
			memcpy((void *)global_proc_msg, (void *)(flow_msg + flow_offset), (msg_len - flow_offset));
			memcpy((void *)(global_proc_msg + msg_len - flow_offset), (void *)flow_msg, flow_offset);
		} else {
			printk("gmlib_flow: Error proc message behavior offset(%d), msg_len(%d)\n", flow_offset, msg_len);
			return NULL;
		}
	}

	start_ptr = (uintptr_t) global_proc_msg;  //fill start print pointer
	total_print_size = msg_len;      //fill print size

	offset = (*pos) * MAX_ITERATION_PRINT_SIZE;

	if (offset >= total_print_size) {
		return NULL; //end of print
	}

	v = (proc_print_buf_t *) kzalloc(sizeof(proc_print_buf_t), GFP_KERNEL);
	if (!v) {
		return NULL; //end of print
	}

	v->print_buf = (char *)(start_ptr + offset);
	v->remain_size = total_print_size - offset;

	return v;
}

static void *seq_gmlib_err_start(struct seq_file *s, loff_t *pos)
{
	proc_print_buf_t *v;
	uintptr_t start_ptr;
	unsigned int total_print_size;
	unsigned int offset;
	static unsigned int msg_len = 0; //need keep for next time usage
	char *err_msg;
	unsigned int err_len;
	unsigned int err_offset;

	if (*pos == 0) {
		if (!mmap_msg)
			return NULL;
		err_msg = (char *)(ERR_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);
		err_len = *(unsigned int *)ERR_MSG_OFFSET(mmap_msg);
		err_offset = *(unsigned int *)(ERR_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE);

		msg_len = err_len;
		if (err_offset == 0) {
			memcpy((void *)global_proc_msg, (void *)(err_msg), msg_len);
		} else if (msg_len > err_offset) { //to prevent negative situation
			memcpy((void *)global_proc_msg, (void *)(err_msg + err_offset), (msg_len - err_offset));
			memcpy((void *)(global_proc_msg + msg_len - err_offset), (void *)err_msg, err_offset);
		} else {
			printk("gmlib_err: Error proc message behavior offset(%d), msg_len(%d)\n", err_offset, msg_len);
			return NULL;
		}
	}

	start_ptr = (uintptr_t) global_proc_msg;  //fill start print pointer
	total_print_size = msg_len;      //fill print size

	offset = (*pos) * MAX_ITERATION_PRINT_SIZE;

	if (offset >= total_print_size) {
		return NULL; //end of print
	}

	v = (proc_print_buf_t *) kzalloc(sizeof(proc_print_buf_t), GFP_KERNEL);
	if (!v) {
		return NULL; //end of print
	}

	v->print_buf = (char *)(start_ptr + offset);
	v->remain_size = total_print_size - offset;

	return v;
}
#if VG_LOG_FULL_FUNC
/* <----- hdal setting message ----> <len(4bytes) - flow message>  */
static void *seq_hdal_setting_start(struct seq_file *s, loff_t *pos)
{
	proc_print_buf_t *v;
	uintptr_t start_ptr;
	unsigned int total_print_size;
	unsigned int offset;
	static unsigned int msg_len = 0;  //need keep for next time usage
	char help_string[] = "Usage: echo [module_type] [all | device_id] > setting\n"
			     "       module_type: graph, videocap, videoproc, videoout, videodec, videoenc, audiocap, audioenc, audiodec, audioout\n";

	if (*pos == 0) {
		char *hdal_setting_msg;
		unsigned int hdal_setting_len;

		if ((!mmap_msg) || (!global_proc_msg)) {
			return NULL;
		}
		if (log_hdal_proc_cb.registered != 0) {
			log_hdal_proc_cb.printout_notifier((uintptr_t) hdal_cmd_buffer);
			set_current_state(TASK_UNINTERRUPTIBLE);
			schedule_timeout(vos_util_msec_to_tick(300));
		}
		hdal_setting_msg = HDAL_SETTING_MSG_OFFSET(mmap_msg);
		hdal_setting_len = strlen(hdal_setting_msg) + strlen(help_string);
		msg_len = hdal_setting_len;

		memcpy((void *) global_proc_msg, (void *) help_string, strlen(help_string));
		memcpy((void *) global_proc_msg + strlen(help_string), (void *) hdal_setting_msg, strlen(hdal_setting_msg));
	}

	start_ptr = (uintptr_t) global_proc_msg;  //fill start print pointer
	total_print_size = msg_len;      //fill print size

	offset = (*pos) * MAX_ITERATION_PRINT_SIZE;

	if (offset >= total_print_size) {
		return NULL; //end of print
	}

	v = (proc_print_buf_t *) kzalloc(sizeof(proc_print_buf_t), GFP_KERNEL);
	if (!v) {
		return NULL; //end of print
	}

	v->print_buf = (char *)(start_ptr + offset);
	v->remain_size = total_print_size - offset;
	return v;
}

static ssize_t hdal_setting_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	if (count > MAX_HDAL_CMD_BUF_LENGTH) {
		printk("count %zd > max %d\n", count, MAX_HDAL_CMD_BUF_LENGTH);
		return -EINVAL;
	}

	if (copy_from_user(hdal_cmd_buffer, buffer, count)) {
		return 0;
	}
	hdal_cmd_buffer[count] = '\0';

	return count;
}
#endif

unsigned long hdal_flow_flag = FLOW_ERR_FLAG;

static void clear_flow_msg(void)
{
	void *flow_msg;
	unsigned int flow_len;

	flow_msg = (void *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);
	flow_len = *(unsigned int *)HDAL_FLOW_MSG_OFFSET(mmap_msg);
	memset(flow_msg, 0, flow_len);
}

static void hdal_set_flow_flag(char *str)
{
	char delim[] = " ";
	char *p = str;
	char *token;
	for (token = strsep(&p, delim); token != NULL; token = strsep(&p, delim)) {
		if (strcmp(token, "dis") == 0) {
			hdal_flow_flag = 0;
			set_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
			clear_flow_msg();
		}
		if (strcmp(token, "all") == 0) {
			hdal_flow_flag = 0xFFFFFFFF;
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
			clear_flow_msg();
		}
		if (strcmp(token, "err") == 0) {
			change_bit(FLOW_ERR_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}

		if (strcmp(token, "videocap") == 0) {
			change_bit(FLOW_CAP_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "videoproc") == 0) {
			change_bit(FLOW_VPE_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "videoenc") == 0) {
			change_bit(FLOW_ENC_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "videodec") == 0) {
			change_bit(FLOW_DEC_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "videoout") == 0) {
			change_bit(FLOW_VOUT_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "audio") == 0) {
			change_bit(FLOW_AUDIO_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "lv") == 0) {
			change_bit(FLOW_LV_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "pb") == 0) {
			change_bit(FLOW_PB_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "rec") == 0) {
			change_bit(FLOW_REC_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "clearwin") == 0) {
			change_bit(FLOW_CLEARWIN_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "nr") == 0) {
			change_bit(FLOW_NR_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "md") == 0) {
			change_bit(FLOW_MD_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "crop") == 0) {
			change_bit(FLOW_CROP_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}

		if (strcmp(token, "trigger") == 0) {
			change_bit(FLOW_TRIGGER_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "osg") == 0) {
			change_bit(FLOW_OSG_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "gfx") == 0) {
			change_bit(FLOW_GFX_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "vendor") == 0) {
			change_bit(FLOW_VENDOR_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
		if (strcmp(token, "common") == 0) {
			change_bit(FLOW_COMMON_BIT, &hdal_flow_flag);
			clear_bit(FLOW_DISABLE_BIT, &hdal_flow_flag);
		}
	}
}

static int get_flow_setup(char *string)
{
	int len = 0;

	if (hdal_flow_flag & FLOW_DISABLE_FLAG) {
		len += sprintf(string + len, "\"dis\"");
	}
	if (hdal_flow_flag & FLOW_ERR_FLAG) {
		len += sprintf(string + len, "\"err\"");
	}
	if (hdal_flow_flag & FLOW_ALL_FLAG) {
		len += sprintf(string + len, "\"all\"");
	}
	if (hdal_flow_flag & FLOW_COMMON_FLAG) {
		len += sprintf(string + len, "\"common\"");
	}
	if (hdal_flow_flag & FLOW_CAP_FLAG) {
		len += sprintf(string + len, "\"videocap\"");
	}
	if (hdal_flow_flag & FLOW_VPE_FLAG) {
		len += sprintf(string + len, "\"videoproc\"");
	}
	if (hdal_flow_flag & FLOW_ENC_FLAG) {
		len += sprintf(string + len, "\"videoenc\"");
	}
	if (hdal_flow_flag & FLOW_DEC_FLAG) {
		len += sprintf(string + len, "\"videodec\"");
	}
	if (hdal_flow_flag & FLOW_VOUT_FLAG) {
		len += sprintf(string + len, "\"videoout\"");
	}
	if (hdal_flow_flag & FLOW_VENDOR_FLAG) {
		len += sprintf(string + len, "\"vendor\"");
	}
	if (hdal_flow_flag & FLOW_AUDIO_FLAG) {
		len += sprintf(string + len, "\"audio\"");
	}
	if (hdal_flow_flag & FLOW_CLEARWIN_FLAG) {
		len += sprintf(string + len, "\"clearwin\"");
	}
	if (hdal_flow_flag & FLOW_NR_FLAG) {
		len += sprintf(string + len, "\"nr\"");
	}
	if (hdal_flow_flag & FLOW_CROP_FLAG) {
		len += sprintf(string + len, "\"crop\"");
	}
	if (hdal_flow_flag & FLOW_OSG_FLAG) {
		len += sprintf(string + len, "\"osg\"");
	}
	if (hdal_flow_flag & FLOW_GFX_FLAG) {
		len += sprintf(string + len, "\"gfx\"");
	}
	if (hdal_flow_flag & FLOW_LV_FLAG) {
		len += sprintf(string + len, "\"lv\"");
	}
	if (hdal_flow_flag & FLOW_PB_FLAG) {
		len += sprintf(string + len, "\"pb\"");
	}
	if (hdal_flow_flag & FLOW_REC_FLAG) {
		len += sprintf(string + len, "\"rec\"");
	}
	if (hdal_flow_flag & FLOW_MD_FLAG) {
		len += sprintf(string + len, "\"md\"");
	}
	if (hdal_flow_flag & FLOW_TRIGGER_FLAG) {
		len += sprintf(string + len, "\"trigger\"");
	}
	return len;
}


static ssize_t hdal_flow_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char string[HDA_FLOW_STR_SZ];
	int str_len = 0;

	if (count == 0) {
		printk("count(%zd)\n", count);
		return -EINVAL;
	}
	if (count >= sizeof(string)) {
		printk("count %zd >= max %zd\n", count, sizeof(string));
		return -EINVAL;
	}
	if (copy_from_user(string, buffer, count)) {
		return 0;
	}
	string[count - 1] = '\0';
	hdal_set_flow_flag(string);
	memset(string, 0, sizeof(string));
	str_len = get_flow_setup(string);
	if (str_len > HDA_FLOW_STR_SZ) {
		printk("get_flow_setup size > HDA_FLOW_STR_SZ\n");
		return 0;
	}
	if (log_hdal_flow_cb.registered != 0) {
		log_hdal_flow_cb.printout_notifier((int)hdal_flow_flag);
	}
	printk("hdal flow enable=%s\n", string);
	printk("Usage: echo [dis/err/all/module_type] > flow)\n");
	printk("   dis: Disable all and clear msg\n");
	printk("   all: Enable all(show it by 'cat /proc/hdal/flow')\n");
	printk("   err: Show warnning/error message on the console\n");
	printk("   module_type: Log HDAL API flow by module(show it by 'cat /proc/hdal/flow')\n");
	printk("   Support module_type: videocap, videoproc, videoout, videodec, videoenc, audio, lv, rec\n");
	printk("                        pb, clearwin, nr, md, crop, trigger, gfx, osg, common, vendor\n");

	return count;
}

static ssize_t hdal_dbglevel_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned int flow_dbg_lvl, i;
	unsigned int *p_flow_dbg_lvl;
	char ker_buffer[64] = {0};

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}
	sscanf(ker_buffer, "%d", &flow_dbg_lvl);
	printk("flow_dbg_lvl=%d\n", flow_dbg_lvl);
	p_flow_dbg_lvl = (unsigned int *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);
	for (i = 0; i < HDAL_FLOW_DBG_MAX; i++, p_flow_dbg_lvl++) {
		*p_flow_dbg_lvl = flow_dbg_lvl;
	}
	return count;
}
#if VG_LOG_FULL_FUNC
static int print_flow_cmd(void *global_proc_offet)
{
	char *buf = kzalloc(1024, GFP_KERNEL);
	char string[HDA_FLOW_STR_SZ];
	int len = 0;
	int str_len = 0;

	memset(string, 0, sizeof(string));
	str_len = get_flow_setup(string);
	if (str_len > HDA_FLOW_STR_SZ) {
		printk("get_flow_setup size > HDA_FLOW_STR_SZ\n");
		kfree(buf);
		return 0;
	}
	len += sprintf(buf + len, "-------------------------------------------------------------------------\n");
	len += sprintf(buf + len, "hdal flow enable=%s\n", string);
	len += sprintf(buf + len, "Usage: echo [dis/err/all/module_type/func_name] > flow)\n");
	len += sprintf(buf + len, "   dis: Disable all and clear msg\n");
	len += sprintf(buf + len, "   all: Enable all (show it by 'cat /proc/hdal/flow')\n");
	len += sprintf(buf + len, "   err: Show warnning/error message on the console\n");
	len += sprintf(buf + len, "   module_type: Log HDAL API flow (show it by 'cat /proc/hdal/flow')\n");
	len += sprintf(buf + len, "   Support module_type: videocap, videoproc, videoout, videodec, videoenc, audio, lv, rec\n");
	len += sprintf(buf + len, "                        pb, clearwin, nr, md, crop, trigger, gfx, osg, common, vendor\n");
	len += sprintf(buf + len, "-------------------------------------------------------------------------\n");
	memcpy(global_proc_offet, (void *)buf, len);
	kfree(buf);
	return len;
}
#else
static int print_flow_cmd(void *global_proc_offet)
{
	return 0;
}
#endif
static void *seq_hdal_flow_start(struct seq_file *s, loff_t *pos)
{
	proc_print_buf_t *v;
	uintptr_t start_ptr;
	unsigned int total_print_size;
	unsigned int offset;
	static unsigned int msg_len = 0; //need keep for next time usage
	char *flow_msg;
	unsigned int flow_len;
	unsigned int flow_offset;
	unsigned int flow_dbg_lvl;

	if (*pos == 0) {
		if ((!mmap_msg) || (!global_proc_msg))
			return NULL;

		flow_msg = (char *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE + HDAL_FLOW_DBG_LVL_SIZE);
		flow_len = *(unsigned int *)HDAL_FLOW_MSG_OFFSET(mmap_msg);
		flow_offset = *(unsigned int *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE);
		flow_dbg_lvl = *(unsigned int *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);

		msg_len = flow_len;

		if (msg_len > (HDAL_FLOW_MSG_SIZE - (MSG_LENGTH_SIZE + MSG_OFFSET_SIZE + HDAL_FLOW_DBG_LVL_SIZE))) {
			printk("hdal_flow: Error msg_len %d\n", msg_len);
			return NULL;
		}

		if (flow_offset == 0) {
			memcpy((void *)global_proc_msg, (void *)(flow_msg), msg_len);
		} else if (msg_len > flow_offset) {
			memcpy((void *)global_proc_msg, (void *)(flow_msg + flow_offset), (msg_len - flow_offset));
			memcpy((void *)(global_proc_msg + msg_len - flow_offset), (void *)flow_msg, flow_offset);
		} else {
			printk("hdal_flow: Error proc message behavior offset(%d), msg_len(%d)\n", flow_offset, msg_len);
			return NULL;
		}
	}

	start_ptr = (uintptr_t) global_proc_msg;  //fill start print pointer
	total_print_size = msg_len;      //fill print size

	offset = (*pos) * MAX_ITERATION_PRINT_SIZE;
	total_print_size += print_flow_cmd((void *)(global_proc_msg + total_print_size));
	if (offset >= total_print_size) {
		return NULL; //end of print
	}

	v = (proc_print_buf_t *) kzalloc(sizeof(proc_print_buf_t), GFP_KERNEL);
	if (!v) {
		return NULL; //end of print
	}
	v->print_buf = (char *)(start_ptr + offset);
	v->remain_size = total_print_size - offset;

	return v;
}

static int proc_hdal_version_show(struct seq_file *s, void *v)
{
	if (s) {
		seq_printf(s, "HDAL: Version: v%x.%02x.%03x\n",
			   (hdal_version & 0xF00000) >> 20,
			   (hdal_version & 0x0FF000) >> 12,
			   (hdal_version & 0x000FFF));
		seq_printf(s, "HDAL: IMPL Version: v%x.%02x.%03x\n",
			   (impl_version & 0xF00000) >> 20,
			   (impl_version & 0x0FF000) >> 12,
			   (impl_version & 0x000FFF));
	} else {
		printk("HDAL: Version: v%x.%02x.%03x\n",
		       (hdal_version & 0xF00000) >> 20,
		       (hdal_version & 0x0FF000) >> 12,
		       (hdal_version & 0x000FFF));
		printk("HDAL: IMPL Version: v%x.%02x.%03x\n",
		       (impl_version & 0xF00000) >> 20,
		       (impl_version & 0x0FF000) >> 12,
		       (impl_version & 0x000FFF));
	}
	return 0;
}

static int proc_hdal_err_show(struct seq_file *s, void *v)
{
	isf_debug_log_dump(s);
	return 0;
}

static void proc_hdal_dbglevel_init(void)
{
	unsigned int *p_flow_dbg_lvl;
	int i;

	p_flow_dbg_lvl = (unsigned int *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);
	for (i = 0; i < HDAL_FLOW_DBG_MAX; i++, p_flow_dbg_lvl++) {
		// DBG_MSG       3
		*p_flow_dbg_lvl = 3;
	}
}
static int proc_hdal_dbglevel_show(struct seq_file *s, void *v)
{
	unsigned int *p_flow_dbg_lvl;
	int i;

	p_flow_dbg_lvl = (unsigned int *)(HDAL_FLOW_MSG_OFFSET(mmap_msg) + MSG_LENGTH_SIZE + MSG_OFFSET_SIZE);
	for (i = 0; i < HDAL_FLOW_DBG_MAX; i++, p_flow_dbg_lvl++) {
		if (i == HDAL_FLOW_DBG_COMM) {
			seq_printf(s, "comm dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_VCAP) {
			seq_printf(s, "vcap dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_VPRC) {
			seq_printf(s, "vprc dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_VOUT) {
			seq_printf(s, "vout dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_VENC) {
			seq_printf(s, "venc dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_VDEC) {
			seq_printf(s, "vdec dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_ACAP) {
			seq_printf(s, "acap dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_AOUT) {
			seq_printf(s, "aout dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_AENC) {
			seq_printf(s, "aenc dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_ADEC) {
			seq_printf(s, "adec dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
		if (i == HDAL_FLOW_DBG_GFX) {
			seq_printf(s, "gfx dbglevel:%d \r\n", *p_flow_dbg_lvl);
		}
	}
	return 0;
}

extern unsigned long loops_per_jiffy;
static ssize_t proc_delay2_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};
	unsigned int delay, loop;
	unsigned long flags;

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}

	sscanf(ker_buffer, "%d", &delay);

	loop = vos_util_msec_to_tick(delay * loops_per_jiffy * 1000);
	vk_spin_lock_irqsave(&delay_lock, flags);
	while ((loop--) > 0) //busy loop
		;
	vk_spin_unlock_irqrestore(&delay_lock, flags);
	return count;
}

/* pa2ddrid function: It is only for RC, EP side physical address not support!
   parameter rc_pa:  RC physical address
   return value: RC ddr_id
*/
int pa2ddrid(uintptr_t rc_pa)
{
	int i;
	nvtmem_ddrinfo_t info;

	nvtmem_get_ddrinfo(&info);
	for (i = 0; i < info.nr_banks; i++) {
		if (info.bank[i].chip != CHIP_RC) {
			continue;
		}
		if ((rc_pa >= info.bank[i].start) &&
		    (rc_pa < info.bank[i].start + info.bank[i].size - 1)) {

			return info.bank[i].ddrid;
		}
	}
	printk("[pa2ddrid] can not get ddrid from physical address !!!\n");
	for (i = 0; i < info.nr_banks; i++) {
		printk("[pa2ddrid] info.bank[%d].chip :%d\n", i, info.bank[i].chip);
		if (info.bank[i].chip != CHIP_RC) {
			continue;
		}
		printk("[pa2ddrid] check pa:0x%lx  i:%d  ddr start addr:%llx ddr size:%lx\n", rc_pa, i, info.bank[i].start, info.bank[i].size);
		if ((rc_pa >= info.bank[i].start) &&
			(rc_pa < info.bank[i].start + info.bank[i].size - 1)) {
			return info.bank[i].ddrid;
		}
	}
	printk("[pa2ddrid] try cat /proc/nvtmem/dram_map to get more information\n");
	printk("[pa2ddrid] please check the hdal-memory parameters in the nvt-mem-tbl.dtsi file\n");
	return -1;
}

static int log_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int log_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static long log_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	char log_string[MAX_STRING_LEN];

	switch (cmd) {
	case IOCTL_PRINTM:
		memset(log_string, 0, sizeof(log_string));
		if (copy_from_user((void *)&log_string, (void *)arg, sizeof(char) * MAX_STRING_LEN)) {
			return -EFAULT;
		}
		printm("UR", "(%d)%s", task_pid_nr(current), log_string);
		break;
	case IOCTL_PRINTM_WITH_PANIC:
		memset(log_string, 0, sizeof(log_string));
		if (copy_from_user((void *)&log_string, (void *)arg, sizeof(char) * MAX_STRING_LEN)) {
			return -EFAULT;
		}
		printm("UR", "(%d)%s", task_pid_nr(current), log_string);
		damnit("UR");
		break;
	case IOCTL_SET_HDAL_VERSION:
		memset(&hdal_version, 0, sizeof(unsigned int));
		if (copy_from_user((void *)&hdal_version, (void *)arg, sizeof(unsigned int))) {
			return -EFAULT;
		}
		break;
	case IOCTL_SET_IMPL_VERSION:
		memset(&impl_version, 0, sizeof(unsigned int));
		if (copy_from_user((void *)&impl_version, (void *)arg, sizeof(unsigned int))) {
			return -EFAULT;
		}
		break;
	case IOCTL_GET_HDAL_DBGMODE: {
		if (copy_to_user((void *)arg, (void *)&hdal_flow_flag, sizeof(unsigned int))) {
			return -EFAULT;
		}
		break;
	}
	default:
		return -EFAULT;
	}
	return 0;
}

int log_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned int pfn;

	if (!mmap_msg) {
		return -EFAULT;
	}

	pfn = __pa(mmap_msg) >> PAGE_SHIFT;
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
		printk("Error to remap mmap\n");
		return -EFAULT;
	}
	return 0;
}

static int affinity_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_affinity_seq_show, NULL);
}

static int mode_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_mode_seq_show, NULL);
}

static int dumplog_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_dumplog_seq_show, NULL);
}

static int delay_non_preemptive_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_delay_non_preemptive_seq_show, NULL);
}

static int delay_irq_disabled_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_delay_irq_disabled_seq_show, NULL);
}

static int gmlib_setting_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seq_gmlib_setting_ops);
}

static int gmlib_flow_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seq_gmlib_flow_ops);
}

static int gmlib_err_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seq_gmlib_err_ops);
}

#if VG_LOG_FULL_FUNC
static int hdal_setting_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seq_hdal_setting_ops);
}
#endif

static int hdal_flow_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seq_hdal_flow_ops);
}

static int hdal_version_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_hdal_version_show, NULL);
}

static int hdal_err_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_hdal_err_show, NULL);
}

static int hdal_dbglevel_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_hdal_dbglevel_show, NULL);
}

static int thread_realtime_test_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_thread_realtime_test_seq_show, NULL);
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops affinity_proc_ops = {
	.proc_open    = affinity_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_affinity_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations affinity_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = affinity_proc_open,
	.read    = seq_read,
	.write   = proc_affinity_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops mode_proc_ops = {
	.proc_open    = mode_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_mode_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations mode_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = mode_proc_open,
	.read    = seq_read,
	.write   = proc_mode_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif


#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops dumplog_proc_ops = {
	.proc_open    = dumplog_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_dumplog_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations dumplog_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = dumplog_proc_open,
	.read    = seq_read,
	.write   = proc_dumplog_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops delay_non_preemptive_proc_ops = {
	.proc_open    = delay_non_preemptive_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_delay1_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations delay_non_preemptive_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = delay_non_preemptive_proc_open,
	.read    = seq_read,
	.write   = proc_delay1_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops delay_irq_disabled_proc_ops = {
	.proc_open    = delay_irq_disabled_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_delay2_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations delay_irq_disabled_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = delay_irq_disabled_proc_open,
	.read    = seq_read,
	.write   = proc_delay2_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops gmlib_setting_proc_ops = {
	.proc_open    = gmlib_setting_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations gmlib_setting_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = gmlib_setting_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops gmlib_flow_proc_ops = {
	.proc_open    = gmlib_flow_proc_open,
	.proc_read    = seq_read,
	.proc_write   = gmlib_flow_write,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations gmlib_flow_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = gmlib_flow_proc_open,
	.read    = seq_read,
	.write   = gmlib_flow_write,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops gmlib_err_proc_ops = {
	.proc_open    = gmlib_err_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations gmlib_err_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = gmlib_err_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if VG_LOG_FULL_FUNC
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops hdal_setting_proc_ops = {
	.proc_open    = hdal_setting_proc_open,
	.proc_read    = seq_read,
	.proc_write   = hdal_setting_write,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations hdal_setting_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = hdal_setting_proc_open,
	.read    = seq_read,
	.write   = hdal_setting_write,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops hdal_flow_proc_ops = {
	.proc_open    = hdal_flow_proc_open,
	.proc_read    = seq_read,
	.proc_write   = hdal_flow_write,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations hdal_flow_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = hdal_flow_proc_open,
	.read    = seq_read,
	.write   = hdal_flow_write,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops hdal_version_proc_ops = {
	.proc_open    = hdal_version_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations hdal_version_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = hdal_version_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops hdal_err_proc_ops = {
	.proc_open    = hdal_err_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations hdal_err_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = hdal_err_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops hdal_dbglevel_proc_ops = {
	.proc_open    = hdal_dbglevel_proc_open,
	.proc_read    = seq_read,
	.proc_write   = hdal_dbglevel_write,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations hdal_dbglevel_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = hdal_dbglevel_proc_open,
	.read    = seq_read,
	.write   = hdal_dbglevel_write,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops thread_realtime_test_proc_ops = {
	.proc_open    = thread_realtime_test_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_thread_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations thread_realtime_test_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = thread_realtime_test_proc_open,
	.read    = seq_read,
	.write   = proc_thread_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif


struct file_operations log_fops = {
owner:
	THIS_MODULE,
unlocked_ioctl:
	log_ioctl,
mmap:
	log_mmap,
open:
	log_open,
release:
	log_release,
};

struct miscdevice log_dev = {
minor:
	MISC_DYNAMIC_MINOR,
name: "log_vg"
	,
fops:
	&log_fops,
};


extern int vg_info_init(void);
extern int vg_info_exit(void);
int __init log_init(void)
{
	int i;

	isf_debug_log_open(HDAL_ERR_MSG_SIZE);
	log_bsize = log_ksize * SZ_1K;
	if (log_bsize > SZ_4M)
		log_base_vmalloc = 1;

	if (log_base_vmalloc)
		log_base_start = log_start_ptr = (uintptr_t)vmalloc(log_bsize);
	else
		log_base_start = log_start_ptr = (uintptr_t)kmalloc(log_bsize, GFP_KERNEL);
	log_base_ddr = 0;
	log_base_end = log_base_start + log_bsize;

	global_proc_msg = kzalloc(MMAP_MSG_LEN, GFP_KERNEL);
	if (!global_proc_msg) {
		panic("Error to allocate global_proc_msg buffer\n");
	}
	mmap_msg = kzalloc(MMAP_MSG_LEN, GFP_KERNEL);
	if (!mmap_msg) {
		panic("Error to allocate mmap buffer\n");
	}
	proc_hdal_dbglevel_init();

	printk("log.ko %s: %s %s (mmap %#lx size 0x%x vmalloc %d)\n", LOG_VERSION, __DATE__, __TIME__, (unsigned long)mmap_msg, MMAP_MSG_LEN, log_base_vmalloc);
	memset((char *)log_base_start, 0, log_bsize);
	if (log_base_start == 0) {
		printk("Error to allocate debug buffer!\n");
		return -1;
	}
	memset(log_slice_ptr, 0, sizeof(int) * MAX_DUMP_FILE);
	memset(log_size, 0, sizeof(int) * MAX_DUMP_FILE);
	memset(log_path, 0, sizeof(char) * MAX_DUMP_FILE * MAX_PATH_WIDTH);
	printk("\nLOG base %lx(ddr%d) size %dK (start pointer %#lx)\n", log_base_start, log_base_ddr,
	       log_ksize, (unsigned long)log_start_ptr);
	printk("PAGE_OFFSET(0x%x) VMALLOC START(0x%x) HZ(%d)\n",
	       (int)PAGE_OFFSET, (int)VMALLOC_START, HZ);

	for (i = 0; i < MAX_CB; i++) {
		log_panic_cb[i].registered = 0;
		log_printout_cb[i].registered = 0;
	}
	log_master_print_cb.registered = 0;
	log_hdal_proc_cb.registered = 0;

	videoproc = proc_mkdir("videograph", NULL);
	if (videoproc == NULL) {
		return -EIO;
	}

	debugproc = proc_mkdir("debug", videoproc);
	if (debugproc == NULL) {
		return -EIO;
	}

	affinityproc = proc_create("affinity", 0, debugproc, &affinity_proc_ops);
	if (affinityproc == NULL) {
		return -EIO;
	}

	modeproc = proc_create("mode", 0, videoproc, &mode_proc_ops);
	if (modeproc == NULL) {
		return -EIO;
	}

	dump2proc = proc_create("dumplog", 0, videoproc, &dumplog_proc_ops);
	if (dump2proc == NULL) {
		return -EIO;
	}

	delay1proc = proc_create("delay_non_preemptive", 0, debugproc, &delay_non_preemptive_proc_ops);
	if (delay1proc == NULL) {
		return -EIO;
	}

	delay2proc = proc_create("delay_irq_disabled", 0, debugproc, &delay_irq_disabled_proc_ops);
	if (delay2proc == NULL) {
		return -EIO;
	}

	gmlib_setting_proc = proc_create("gmlib_setting", 0, videoproc, &gmlib_setting_proc_ops);
	if (gmlib_setting_proc == NULL) {
		return -EIO;
	}

	gmlib_flow_proc = proc_create("gmlib_flow", 0, videoproc, &gmlib_flow_proc_ops);
	if (gmlib_flow_proc == NULL) {
		return -EIO;
	}

	gmlib_err_proc = proc_create("gmlib_err", 0, videoproc, &gmlib_err_proc_ops);
	if (gmlib_err_proc == NULL) {
		return -EIO;
	}

	hdal_proc = proc_mkdir("hdal", NULL);
	if (hdal_proc == NULL) {
		return -EIO;
	}

	#if VG_LOG_FULL_FUNC
	hdal_setting_proc = proc_create("setting", 0, hdal_proc, &hdal_setting_proc_ops);
	if (hdal_setting_proc == NULL) {
		return -EIO;
	}
	#endif

	hdal_flow_proc = proc_create("flow", 0, hdal_proc, &hdal_flow_proc_ops);
	if (hdal_flow_proc == NULL) {
		return -EIO;
	}

	hdal_version_proc = proc_create("version", 0, hdal_proc, &hdal_version_proc_ops);
	if (hdal_version_proc == NULL) {
		return -EIO;
	}

	hdal_err_proc = proc_create("err", 0, hdal_proc, &hdal_err_proc_ops);
	if (hdal_err_proc == NULL) {
		return -EIO;
	}

	hdal_dbglevel_proc = proc_create("dbglevel", 0, hdal_proc, &hdal_dbglevel_proc_ops);
	if (hdal_dbglevel_proc == NULL) {
		return -EIO;
	}

	threadproc = proc_create("thread_realtime_test", 0, debugproc, &thread_realtime_test_proc_ops);
	if (threadproc == NULL) {
		return -EIO;
	}

	write_task = kthread_create(write_thread, 0, "log_thread");
	if (IS_ERR(write_task)) {
		return -EFAULT;
	}
	wake_up_process(write_task);

	monitor_task = kthread_create(monitor_thread, 0, "threadmon");
	if (IS_ERR(monitor_task)) {
		return -EFAULT;
	}
	wake_up_process(monitor_task);

	notify_task = kthread_create(notify_thread, 0, "log_notify");
	if (IS_ERR(notify_task)) {
		return -EFAULT;
	}
	wake_up_process(notify_task);

	init_waitqueue_head(&log_wq);

	vg_info_init();

	ktime_get_real_ts64(&log_mod_insert_time);
	return misc_register(&log_dev);
}

void __exit log_clearnup(void)
{
	if (write_task) {
		kthread_stop(write_task);
	}
	if (monitor_task) {
		kthread_stop(monitor_task);
	}
	if (notify_task) {
		kthread_stop(notify_task);
	}

	if (global_proc_msg) {
		kfree(global_proc_msg);
	}
	if (mmap_msg) {
		kfree(mmap_msg);
	}
	if (log_base_vmalloc)
		vfree((void *)log_base_start);
	else
		kfree((void *)log_base_start);

	vg_info_exit();
	if (affinityproc != 0) {
		remove_proc_entry("affinity", debugproc);
	}
	if (delay1proc != 0) {
		remove_proc_entry("delay_non_preemptive", debugproc);
	}
	if (delay2proc != 0) {
		remove_proc_entry("delay_irq_disabled", debugproc);
	}
	if (threadproc != 0) {
		remove_proc_entry("thread_realtime_test", debugproc);
	}

	if (gmlib_setting_proc != 0) {
		remove_proc_entry("gmlib_setting", videoproc);
	}
	if (gmlib_flow_proc != 0) {
		remove_proc_entry("gmlib_flow", videoproc);
	}
	if (gmlib_err_proc != 0) {
		remove_proc_entry("gmlib_err", videoproc);
	}
	if (debugproc != 0) {
		remove_proc_entry("debug", videoproc);
	}
	if (modeproc != 0) {
		remove_proc_entry("mode", videoproc);
	}
	if (dump2proc != 0) {
		remove_proc_entry("dumplog", videoproc);
	}
	if (videoproc != 0) {
		remove_proc_entry("videograph", NULL);
	}

	if (hdal_setting_proc != 0) {
		remove_proc_entry("setting", hdal_proc);
	}
	if (hdal_flow_proc != 0) {
		remove_proc_entry("flow", hdal_proc);
	}
	if (hdal_version_proc != 0) {
		remove_proc_entry("version", hdal_proc);
	}
	if (hdal_err_proc != 0) {
		remove_proc_entry("err", hdal_proc);
	}
	if (hdal_dbglevel_proc != 0) {
		remove_proc_entry("dbglevel", hdal_proc);
	}
	if (hdal_proc != 0) {
		remove_proc_entry("hdal", NULL);
	}
	misc_deregister(&log_dev);
}
#if defined(_GROUP_KO_)
#undef __init
#undef __exit
#undef module_init
#undef module_exit
#define __init
#define __exit
#define module_init(x)
#define module_exit(x)
#ifndef MODULE // in built-in case
#undef MODULE_VERSION
#undef VOS_MODULE_VERSION
#define MODULE_VERSION(x)
#define VOS_MODULE_VERSION(a1, a2, a3, a4, a5)
#endif
#endif

module_init(log_init);
module_exit(log_clearnup);

MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_LICENSE("GPL");
MODULE_VERSION(LOG_VERSION);

EXPORT_SYMBOL(register_panic_notifier);
EXPORT_SYMBOL(register_printout_notifier);
EXPORT_SYMBOL(register_master_print_notifier);
EXPORT_SYMBOL(register_hdal_proc_notifier);
EXPORT_SYMBOL(register_hdal_flow_notifier);
EXPORT_SYMBOL(register_gmlib_flow_notifier);
EXPORT_SYMBOL(damnit);
EXPORT_SYMBOL(dumplog);
EXPORT_SYMBOL(printm);
EXPORT_SYMBOL(printm2);
EXPORT_SYMBOL(master_print);
EXPORT_SYMBOL(dumpbuf_pa);
EXPORT_SYMBOL(dumpbuf_va);
EXPORT_SYMBOL(register_version);
EXPORT_SYMBOL(unregister_printout_notifier);
EXPORT_SYMBOL(unregister_panic_notifier);
EXPORT_SYMBOL(get_cpu_state);
EXPORT_SYMBOL(pa2ddrid);
EXPORT_SYMBOL(isf_debug_log_print);
EXPORT_SYMBOL(get_hdal_flow_dbglevel);
EXPORT_SYMBOL(set_hdal_flow_dbglevel);