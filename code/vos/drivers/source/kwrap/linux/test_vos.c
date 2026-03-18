#define __MODULE__    test_vos
#define __DBGLVL__    2 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__    "*"
#include <kwrap/debug.h>

#ifndef __LINUX_USER
#include <kwrap/cpu.h>
#include <kwrap/dev.h>
#include <kwrap/kthread.h>
#include <kwrap/mem.h>
#include <kwrap/malloc.h>
#include <kwrap/mutex.h>
#include <kwrap/tasklet.h>
#include <kwrap/timer.h>
#include <kwrap/workqueue.h>
#endif

#include <kwrap/bitfield.h>
#include <kwrap/file.h>
#include <kwrap/flag.h>
#include <kwrap/perf.h>
#include <kwrap/semaphore.h>
#include <kwrap/spinlock.h>
#include <kwrap/stdio.h>
#include <kwrap/task.h>
#include <kwrap/util.h>

#ifdef __FREERTOS
#include <stdio.h>
#include <string.h>
#include <kwrap/examsys.h>
#include <kwrap/mailbox.h>
#endif

#ifdef __LINUX
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/sizes.h>
#endif

#ifdef __LINUX_USER
#include <string.h>
#include <kwrap/mailbox.h>
#endif

#define FLGPTN_XFLAG_BIT0   FLGPTN_BIT(0)
#define FLGPTN_XFLAG_BIT1   FLGPTN_BIT(1)

#define FLGPTN_DONE_TASK1    FLGPTN_BIT(0)
#define FLGPTN_DONE_TASK2    FLGPTN_BIT(1)
#define FLGPTN_DONE_TASK3    FLGPTN_BIT(2)
#define FLGPTN_DONE_TASK4    FLGPTN_BIT(3)

static ID xSemaphore1; //old usage: static SEM_HANDLE xSemaphore1;
static ID xSemaphore2; //old usage: static SEM_HANDLE xSemaphore2;
static ID xSemTaskDone;

static ID xFlag1; //old usage: static ID xFlag1
static ID xFlagTask3;
static ID xFlagTask4;
static ID xFlagTaskDone;

static VK_TASK_HANDLE g_tskhdl_spin1;
static VK_TASK_HANDLE g_tskhdl_spin2;
static THREAD_HANDLE xTaskHdl1;
static THREAD_HANDLE xTaskHdl2;
static THREAD_HANDLE xTaskHdl3;
static THREAD_HANDLE xTaskHdl4;
static THREAD_HANDLE xTaskHdl5;
static THREAD_HANDLE xTaskHdl6;

typedef struct {
	int val;
	char name[16];
} MBX_DATA;

THREAD_RETTYPE sem_task1( void * pvParameters )
{
	int num_loop = 10;

	if (xTaskHdl1 != vos_task_get_handle()) {
		DBG_ERR("get handle failed\r\n");
	} else {
		CHAR name[32] = {0};
		vos_task_get_name(xTaskHdl1, name, sizeof(name));
		DBG_DUMP("vos_task_get_name = %s\r\n", name);
	}

	vos_perf_list_mark(__func__, __LINE__, 0);

	while (num_loop--) {
		vos_sem_wait(xSemaphore1);//old usage: SEM_WAIT(xSemaphore1);
		DBG_DUMP("I am %s\n", __func__);
		vos_sem_sig(xSemaphore2);//old usage: SEM_SIGNAL(xSemaphore2);
		//vos_util_delay_ms(100);
	}

	vos_perf_list_mark(__func__, __LINE__, 0);
	vos_sem_sig(xSemTaskDone);
	THREAD_RETURN(0);
}

THREAD_RETTYPE sem_task2( void * pvParameters )
{
	int num_loop = 10;

	vos_perf_list_mark(__func__, __LINE__, 0);

	while (num_loop--) {
		vos_sem_wait(xSemaphore2); //old usage: SEM_WAIT(xSemaphore2);
		DBG_DUMP("I am %s\n", __func__);
		vos_sem_sig(xSemaphore1); //old usage: SEM_SIGNAL(xSemaphore1);
		//vos_util_delay_ms(100);
	}

	vos_perf_list_mark(__func__, __LINE__, 0);
	vos_sem_sig(xSemTaskDone);
	THREAD_RETURN(0);
}

THREAD_RETTYPE flag_task1( void * pvParameters )
{
	int num_loop = 10;
	FLGPTN flgptn;

	vos_perf_list_mark(__func__, __LINE__, 0);

	while (num_loop--) {
		wai_flg(&flgptn, xFlag1, FLGPTN_XFLAG_BIT0, TWF_ORW | TWF_CLR);
		DBG_DUMP("I am %s\r\n", __func__);
		set_flg(xFlag1, FLGPTN_XFLAG_BIT1);
	}

	vos_perf_list_mark(__func__, __LINE__, 0);

	set_flg(xFlagTaskDone, FLGPTN_DONE_TASK1);
	THREAD_RETURN(0);
}

THREAD_RETTYPE flag_task2( void * pvParameters )
{
	int num_loop = 10;
	FLGPTN flgptn;
	VOS_TICK t1 = 0, t2 = 0;

	vos_perf_list_mark(__func__, __LINE__, 0);

	vos_perf_mark(&t1);
	while (num_loop--) {
		wai_flg(&flgptn, xFlag1, FLGPTN_XFLAG_BIT1, TWF_ORW | TWF_CLR);
		DBG_DUMP("I am %s\r\n", __func__);
		set_flg(xFlag1, FLGPTN_XFLAG_BIT0);
	}
	vos_perf_mark(&t2);
	DBG_DUMP("%s: t1 %d, t2 %d, duration %d\r\n", __func__, (unsigned int)t1, (unsigned int)t2, (unsigned int)vos_perf_duration(t1, t2));

	vos_perf_list_mark(__func__, __LINE__, 0);
	vos_perf_list_dump();

	set_flg(xFlagTaskDone, FLGPTN_DONE_TASK2);
	THREAD_RETURN(0);
}

THREAD_RETTYPE flag_task3( void * pvParameters )
{
	int bit_num;
	FLGPTN retptn;
	FLGPTN waiptn;
	FLGPTN chkptn;

	DBG_DUMP("%s test begin\r\n", __func__);

	for (bit_num = 0; bit_num < 32; bit_num++) {
		waiptn = FLGPTN_BIT(bit_num);

		//set to trigger task4
		if (0 != vos_flag_set(xFlagTask4, waiptn)) {
			DBG_DUMP("%s vos_flag_set failed\r\n", __func__);
			break;
		}

		//wait task4 to set back
		if (0 != vos_flag_wait(&retptn, xFlagTask3, waiptn, TWF_ORW)) {
			DBG_DUMP("%s vos_flag_wait failed\r\n", __func__);
			break;
		}
		#if 0 //for debug
		DBG_DUMP("%s: bit_num %d, retptn 0x%X\r\n", __func__, bit_num, retptn);
		#endif

		//check the flag status
		if (0 == (waiptn & retptn)) {
			DBG_DUMP("%s: waiptn 0x%X, retptn 0x%X, bit not set\r\n", __func__, waiptn, retptn);
			break;
		}

		if (0 != vos_flag_clr(xFlagTask3, waiptn)) {
			DBG_DUMP("%s vos_flag_clr failed\r\n", __func__);
			break;
		}

		chkptn = vos_flag_chk(xFlagTask3, waiptn);
		if (chkptn & waiptn) {
			DBG_DUMP("%s bit not cleared\r\n", __func__);
			break;
		}
	}

	chkptn = vos_flag_chk(xFlagTask3, FLGPTN_BIT_ALL);
	if (0 != chkptn) {
		DBG_DUMP("%s chkptn 0x%X should be 0\r\n", __func__, chkptn);
	}

	DBG_DUMP("%s test end\r\n", __func__);

	set_flg(xFlagTaskDone, FLGPTN_DONE_TASK3);
	THREAD_RETURN(0);
}

THREAD_RETTYPE flag_task4( void * pvParameters )
{
	int bit_num;
	FLGPTN retptn;
	FLGPTN waiptn;
	FLGPTN chkptn;

	DBG_DUMP("%s test begin\r\n", __func__);

	for (bit_num = 0; bit_num < 32; bit_num++) {
		waiptn = FLGPTN_BIT(bit_num);

		//wait task3 to set flag
		if (0 != vos_flag_wait(&retptn, xFlagTask4, waiptn, TWF_ORW | TWF_CLR)) {
			DBG_DUMP("%s vos_flag_wait failed\r\n", __func__);
			break;
		}
		#if 0 //for debug
		DBG_DUMP("%s: bit_num %d, retptn 0x%X\r\n", __func__, bit_num, retptn);
		#endif

		//check the flag status
		if (waiptn != retptn) {
			DBG_DUMP("%s: waiptn 0x%X, retptn 0x%X, bit not cleared\r\n", __func__, waiptn, retptn);
		}

		chkptn = vos_flag_chk(xFlagTask3, waiptn);
		if (chkptn & waiptn) {
			DBG_DUMP("%s bit not cleared\r\n", __func__);
			break;
		}

		//set to trigger task3
		if (0 != vos_flag_set(xFlagTask3, waiptn)) {
			DBG_DUMP("%s vos_flag_set failed\r\n", __func__);
			break;
		}
	}

	chkptn = vos_flag_chk(xFlagTask4, FLGPTN_BIT_ALL);
	if (0 != chkptn) {
		DBG_DUMP("%s chkptn 0x%X should be 0\r\n", __func__, chkptn);
	}

	DBG_DUMP("%s test end\r\n", __func__);

	set_flg(xFlagTaskDone, FLGPTN_DONE_TASK4);
	THREAD_RETURN(0);
}

#ifndef __LINUX
static ID xMailbox1;

THREAD_RETTYPE mbx_snd_task(void *ptr)
{
	MBX_DATA snd_data = {0, "DATA_MBX"};
	int num_loop = 10;

	DBG_FUNC_BEGIN("\r\n");
	vos_task_enter(); //kent_tsk()

	while(num_loop--) {
		DBG_DUMP("I am %s, SND: val %d, name %s\r\n", __func__, snd_data.val, snd_data.name);
		if (E_OK != vos_mbx_snd(xMailbox1, &snd_data, sizeof(snd_data))) {
			DBG_ERR("vos_mbx_snd failed\r\n");
			break;
		}
		snd_data.val++;
	}

	DBG_FUNC_END("\r\n");

	THREAD_RETURN(0);
}

THREAD_RETTYPE mbx_rcv_task(void *ptr)
{
	MBX_DATA rcv_data = {0};
	int num_loop = 10;

	DBG_FUNC_BEGIN("\r\n");
	vos_task_enter(); //kent_tsk()

	while(num_loop--) {
		if (E_OK != vos_mbx_rcv(xMailbox1, &rcv_data, sizeof(rcv_data))) {
			DBG_ERR("vos_mbx_rcv failed\r\n");
			break;
		}
		DBG_DUMP("I am %s, RCV: val %d, name %s, is_empty %d\r\n", __func__, rcv_data.val, rcv_data.name, vos_mbx_is_empty(xMailbox1));
	}

	DBG_FUNC_END("\r\n");

	THREAD_RETURN(0);
}

int mbx_test(void)
{
	THREAD_HANDLE task1_hdl, task2_hdl;
	VOS_MBX_PARAM mbx_param = {10, sizeof(MBX_DATA)};

	DBG_FUNC_BEGIN("\n");

#ifdef __LINUX_USER
	if (E_OK != vos_mbx_init(8)) {
		DBG_ERR("vos_mbx_init failed\r\n");
		goto testfunc_mailbox_exit;
	}
#endif

	DBG_DUMP("vos_mbx_create maxmsg %d, msgsize %d\r\n", mbx_param.maxmsg, mbx_param.msgsize);
	if (E_OK != vos_mbx_create(&xMailbox1, &mbx_param)) {
		DBG_ERR("vos_mbx_create failed\r\n");
		goto testfunc_mailbox_exit;
	}

	task1_hdl = vos_task_create(mbx_snd_task, NULL, "mbx_snd_task", 10, 8*1024);
	if (0 == task1_hdl) {
		DBG_ERR("vos_task_create failed\r\n");
		goto testfunc_mailbox_exit;
	}
	vos_task_resume(task1_hdl);

	task2_hdl = vos_task_create(mbx_rcv_task, NULL, "mbx_rcv_task", 10, 8*1024);
	if (0 == task2_hdl) {
		DBG_ERR("vos_task_create failed\r\n");
		goto testfunc_mailbox_exit;
	}
	vos_task_resume(task2_hdl);

	vos_util_delay_ms(1000);

	vos_mbx_destroy(xMailbox1);

testfunc_mailbox_exit:
#ifdef __LINUX_USER
	if (E_OK != vos_mbx_exit()) {
		DBG_ERR("vos_mbx_exit failed\r\n");
	}
#endif
	DBG_FUNC_END("\n");
	return 0;
}
#endif //#ifdef __FREERTOS

#ifdef __LINUX
static int dev_fops_open(struct inode *inode, struct file *file)
{
	printk("MAJOR(%d), MINOR(%d)\r\n", imajor(inode), iminor(inode));
	return 0;
}

ssize_t dev_fops_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
	return 0;
}

static struct file_operations dev_fops = {
	.owner      = THIS_MODULE,
	.open       = dev_fops_open,
	.read       = dev_fops_read,
};

static int cdev_unreg_cb(struct device *dev, void *data)
{
	device_unregister(dev);
	return 0;
}

static int cdev_reg_test(dev_t devid, unsigned count, const char* class_name)
{
	struct cdev *p_cdev = NULL;
	struct class *p_class = NULL;
	struct device *p_device = NULL;
	int dev_no;
	int ret = 0;

	p_cdev = cdev_alloc();
	if (NULL == p_cdev) {
		DBG_ERR("cdev_alloc failed\r\n");
		ret = -1;
		goto cdev_reg_test_end;
	}
	p_cdev->ops = &dev_fops;

	ret = cdev_add(p_cdev, devid, count);
	if (0 != ret) {
		DBG_ERR(" cdev_add failed\n");
		ret = -1;
		goto cdev_reg_test_end;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
	p_class = class_create(class_name);
#else
	p_class = class_create(THIS_MODULE, class_name);
#endif
	if (IS_ERR(p_class)) {
		DBG_ERR("class_create failed\n");
		ret = -1;
		goto cdev_reg_test_end;
	}

	for (dev_no = 0; dev_no < count; dev_no++) {
		p_device = device_create(p_class, NULL, MKDEV(MAJOR(devid), MINOR(devid) + dev_no), NULL, "%s_cdev%d", class_name, dev_no);
		if (IS_ERR(p_device)) {
			DBG_ERR("device_create failed\n");
			ret = -1;
			goto cdev_reg_test_end;
		}
	}

	#if 0 //set 1 for debug, do not release to check the result
	DBG_ERR("debug mode, class will not be released\r\n");
	return ret;
	#endif

cdev_reg_test_end:
	if (p_class) {
		class_for_each_device(p_class, NULL, NULL, cdev_unreg_cb);
		class_destroy(p_class);
	}

	if (p_cdev) {
		cdev_del(p_cdev);
	}

	return ret;
}

static int dev_test(void)
{
	dev_t dev_1st = 0, dev_2nd = 0;
	unsigned int major_1st, major_2nd;
	unsigned int minor_1st, minor_2nd;
	unsigned count_1st, count_2nd;

	count_1st = 2;
	count_2nd = 3;

	//1st alloc region
	if (0 != vos_alloc_chrdev_region(&dev_1st, count_1st, "vos_dev_reg1")) {
		DBG_ERR("vos_alloc_chrdev_region 1 failed\r\n");
	}
	major_1st = MAJOR(dev_1st);
	minor_1st = MINOR(dev_1st);
	DBG_DUMP("major_1st %d, minor_1st %d, count_1st %d\r\n", major_1st, minor_1st, count_1st);

	//2nd alloc region
	if (0 != vos_alloc_chrdev_region(&dev_2nd, count_2nd, "vos_dev_reg2")) {
		DBG_ERR("vos_alloc_chrdev_region 2 failed\r\n");
	}
	major_2nd = MAJOR(dev_2nd);
	minor_2nd = MINOR(dev_2nd);
	DBG_DUMP("major_2nd %d, minor_2nd %d, count_2nd %d\r\n", major_2nd, minor_2nd, count_2nd);

	//check alloc results
	if (major_1st != major_2nd) {
		DBG_ERR("major not matched, 1st(%d) 2nd(%d)\r\n", major_1st, major_2nd);
	}

	if ((minor_1st + count_1st) != minor_2nd) {
		DBG_ERR("minor not continuous, 1st(%d) 2nd(%d)\r\n", minor_1st, minor_2nd);
	}

	//try to register device node and release right away
	if (0 != cdev_reg_test(dev_1st, count_1st, "vos_class1")) {
		DBG_ERR("cdev_reg 1 failed\r\n");
	}

	if (0 != cdev_reg_test(dev_2nd, count_2nd, "vos_class2")) {
		DBG_ERR("cdev_reg 2 failed\r\n");
	}

	//unregister region
	vos_unregister_chrdev_region(dev_1st, count_1st);
	vos_unregister_chrdev_region(dev_2nd, count_2nd);

	return 0;
}
#endif //#ifdef __LINUX

static void print_dbg_msg(void)
{
	int dec_val = 12345678;
	int hex_val = 0x12345678;
	DBG_DUMP("pring dbg msg -------- begin\r\n");
	DBG_FATAL("my fatal\r\n");
	DBG_ERR("my error\r\n");
	DBG_WRN("my warning\r\n");
	DBG_UNIT("my unit\r\n");
	DBG_FUNC_BEGIN("my func\r\n");
	DBG_FUNC("my func\r\n");
	DBG_FUNC_END("my func\r\n");
	DBG_IND("my ind\r\n");
	DBG_MSG("my msg\r\n");
	DBG_VALUE("my value\r\n");
	DBG_USER("my user\r\n");
	DBG_DUMP("my dump\r\n");
	CHKPNT;
	DBGD(dec_val);
	DBGH(hex_val);
	DBG_DUMP("pring dbg msg -------- end\r\n");
}

#ifndef __LINUX_USER
static int mem_test(void)
{
	struct vos_mem_info_t cma_info = {0};
	VOS_MEM_CMA_HDL cma_hdl;

	//cache
	DBG_DUMP("test cache memory\r\n");
	if (0 != vos_mem_init_info(&cma_info, VOS_MEM_CMA_TYPE_CACHE, 2048, NULL)) {
		DBG_ERR("init cma failed\r\n");
		return -1;
	}

	cma_hdl = vos_mem_alloc_contiguous(&cma_info);
	if (NULL == cma_hdl) {
		DBG_ERR("alloc cma failed\r\n");
		return -1;
	}
	DBG_DUMP("cma_info.paddr 0x%lX\r\n", (ULONG)cma_info.paddr);
	DBG_DUMP("cma_info.vaddr 0x%lX\r\n", (ULONG)cma_info.vaddr);
	DBG_DUMP("cma_info.size 0x%lX\r\n", (ULONG)cma_info.size);

	if (vos_cpu_get_phy_addr(cma_info.vaddr) != cma_info.paddr) {
		DBG_ERR("phy_addr conv failed, 0x%lX != 0x%lX\r\n", (ULONG)vos_cpu_get_phy_addr(cma_info.vaddr), (ULONG)cma_info.paddr);
	}

	memset((void *)cma_info.vaddr, 0x55, cma_info.size);

	if (0 != vos_mem_release_contiguous(cma_hdl)) {
		DBG_ERR("release cma failed\r\n");
		return -1;
	}

	//non-cache
	DBG_DUMP("test non-cache memory\r\n");
	if (0 != vos_mem_init_info(&cma_info, VOS_MEM_CMA_TYPE_NONCACHE, 2048, NULL)) {
		DBG_ERR("init cma failed\r\n");
		return -1;
	}

	cma_hdl = vos_mem_alloc_contiguous(&cma_info);
	if (NULL == cma_hdl) {
		DBG_ERR("alloc cma failed\r\n");
		return -1;
	}
	DBG_DUMP("cma_info.paddr 0x%lX\r\n", (ULONG)cma_info.paddr);
	DBG_DUMP("cma_info.vaddr 0x%lX\r\n", (ULONG)cma_info.vaddr);
	DBG_DUMP("cma_info.size 0x%lX\r\n", (ULONG)cma_info.size);

	if (vos_cpu_get_phy_addr(cma_info.vaddr) != cma_info.paddr) {
		DBG_ERR("phy_addr conv failed, 0x%lX != 0x%lX\r\n", (ULONG)vos_cpu_get_phy_addr(cma_info.vaddr), (ULONG)cma_info.paddr);
	}

	memset((void *)cma_info.vaddr, 0xAA, cma_info.size);

	if (0 != vos_mem_release_contiguous(cma_hdl)) {
		DBG_ERR("release cma failed\r\n");
		return -1;
	}

	return 0;
}

void tasklet_func(unsigned long arg)
{
	static unsigned long trig_cnt = 0;

	trig_cnt++;
	DBG_DUMP("I am %s, trig_cnt %ld, arg = 0x%lx\r\n", __func__, trig_cnt, arg);

	//Note:
	//1. A tasklet function should not call vos THREAD_RETURN().
	//2. If THREAD_RETURN is called, the handling task will be destroyed
	//   and the tasklet can not be triggered again
}

static int vk_tasklet_test(void)
{
	struct vos_tasklet_struct vos_tasklet;
	unsigned long tasklet_arg = 0x12345678UL;

	vos_tasklet_init(&vos_tasklet, tasklet_func, tasklet_arg);

	DBG_DUMP("trigger tasklet 1\r\n");
	vos_tasklet_schedule(&vos_tasklet);
	vos_util_delay_ms(100);

	DBG_DUMP("trigger tasklet 2\r\n");
	vos_tasklet_schedule(&vos_tasklet);
	vos_util_delay_ms(100);

	DBG_DUMP("trigger tasklet 3\r\n");
	vos_tasklet_schedule(&vos_tasklet);
	vos_util_delay_ms(100);

	vos_tasklet_kill(&vos_tasklet);

	return 0;
}

static int vk_kxalloc_test(void)
{//Linux stype API
	void *p_mem;

	p_mem = vk_kzalloc(SZ_1M, GFP_KERNEL);
	if (NULL == p_mem) {
		DBG_ERR("vk_kzalloc failed\r\n");
		return -1;
	}
	vk_kfree(p_mem);

	p_mem = vk_kmalloc(SZ_1M, GFP_KERNEL);
	if (NULL == p_mem) {
		DBG_ERR("vk_kalloc failed\r\n");
		return -1;
	}
	vk_kfree(p_mem);

	p_mem = vk_vmalloc(SZ_1M);
	if (NULL == p_mem) {
		DBG_ERR("vk_vmalloc failed\r\n");
		return -1;
	}
	vk_vfree(p_mem);

	DBG_DUMP("kxalloc_test done\r\n");

	return 0;
}

static int _kthread_func(void *data)
{
	int func_arg = *(int *)data;

	vos_util_delay_ms(100);

	DBG_DUMP("%s %d enter\r\n", __func__, func_arg);

	while (!vk_kthread_should_stop()) {
		DBG_DUMP("%s func_arg %d\r\n", __func__, func_arg);
		vos_util_delay_ms(100);
	}

	DBG_DUMP("%s %d exit\r\n", __func__, func_arg);
	return 123;
}

static int vk_kthread_test(void)
{
	struct vk_task_struct *p_tsk;
	int func_arg;
	int ret;

	//test vk_get_current
	DBG_DUMP("current 0x%lx\r\n", (unsigned long)current);
	DBG_DUMP("vk_current 0x%lx\r\n", (unsigned long)vk_current);
	DBG_DUMP("vk_get_current 0x%lx\r\n", (unsigned long)vk_get_current());

	//test vk_kthread_create
	func_arg = 1;
	p_tsk = vk_kthread_create(_kthread_func, &func_arg, "kthread_name1");
	if (IS_ERR(p_tsk)) {
		DBG_ERR("vk_kthread_create failed\r\n");
		return -1;
	}

	ret = vk_wake_up_process(p_tsk);
	DBG_DUMP("vk_wake_up_process, ret %d\r\n", ret);

	DBG_DUMP("nice %d before set -10\r\n", vk_task_nice(p_tsk));
	vk_set_user_nice(p_tsk, -10);
	DBG_DUMP("nice %d after set -10\r\n", vk_task_nice(p_tsk));

	vos_util_delay_ms(500);

	ret = vk_kthread_stop(p_tsk);
	if (123 != ret) {
		DBG_ERR("vk_kthread_stop failed, ret %d\r\n", ret);
		return -1;
	}

	//test vk_kthread_run
	func_arg = 2;
	p_tsk = vk_kthread_run(_kthread_func, &func_arg, "kthread_name2");
	if (IS_ERR(p_tsk)) {
		DBG_ERR("vk_kthread_run failed\r\n");
		return -1;
	}

	//do not need vk_wake_up_process here
	vos_util_delay_ms(500);

	ret = vk_kthread_stop(p_tsk);
	if (123 != ret) {
		DBG_ERR("vk_kthread_stop failed, ret %d\r\n", ret);
		return -1;
	}

	return 0;
}

static int _vk_mutex_task1(void *param)
{
	struct vk_mutex *p_mutex = param;
	int num_loop = 10;

	while (num_loop--) {
		vk_mutex_lock(p_mutex);
		DBG_DUMP("I am %s\n", __func__);
		vk_mutex_unlock(p_mutex);
	}

	return 0;
}

static int _vk_mutex_task2(void *param)
{
	struct vk_mutex *p_mutex = param;
	int num_loop = 10;

	while (num_loop--) {
		vk_mutex_lock(p_mutex);
		DBG_DUMP("I am %s\n", __func__);
		vk_mutex_unlock(p_mutex);
	}

	return 0;
}

static int vk_mutex_test(void)
{
	struct vk_task_struct *p_tsk1;
	struct vk_task_struct *p_tsk2;
	VK_DEFINE_MUTEX(tmp_mutex);
	int ret;

	p_tsk1 = vk_kthread_run(_vk_mutex_task1, &tmp_mutex, "vk_mutex_task1");
	if (IS_ERR(p_tsk1)) {
		DBG_ERR("vk_kthread_run 1 failed\r\n");
		return -1;
	}

	p_tsk2 = vk_kthread_run(_vk_mutex_task2, &tmp_mutex, "vk_mutex_task2");
	if (IS_ERR(p_tsk2)) {
		DBG_ERR("vk_kthread_run 2 failed\r\n");
		return -1;
	}

	//wait thread exit
	ret = vk_kthread_stop(p_tsk1);
	if (0 != ret) {
		DBG_ERR("vk_kthread_stop 1 failed, ret %d\r\n", ret);
		return -1;
	}

	ret = vk_kthread_stop(p_tsk2);
	if (0 != ret) {
		DBG_ERR("vk_kthread_stop 2 failed, ret %d\r\n", ret);
		return -1;
	}

	return 0;
}

static void _timer_callback(struct vk_timer_list *p_timer)
{
	unsigned long expire_jiffies;

	if (0 != vk_timer_pending(p_timer)) {
		DBG_ERR("after expired: vk_timer_pending(%d) should be 0\r\n", vk_timer_pending(p_timer));
		return;
	}

	expire_jiffies = jiffies + msecs_to_jiffies(400);

	DBG_DUMP("vk_mod_timer: cur %ld -> expire %ld\r\n", jiffies, expire_jiffies);
	vk_mod_timer(p_timer, expire_jiffies);

	if (1 != vk_timer_pending(p_timer)) {
		DBG_ERR("after vk_mod_timer: vk_timer_pending(%d) should be 1\r\n", vk_timer_pending(p_timer));
		return;
	}
}

static int vk_timer_test(void)
{
	struct vk_timer_list my_timer;
	int is_deactivated = 0;

	//init timer
	vk_timer_setup(&my_timer, _timer_callback, 0);

	//check pending state
	if (0 != vk_timer_pending(&my_timer)) {
		DBG_ERR("before vk_add_timer: vk_timer_pending(%d) should be 0\r\n", vk_timer_pending(&my_timer));
		return -1;
	}

	//set expire time
	my_timer.expires = jiffies + msecs_to_jiffies(1000);

	DBG_DUMP("vk_add_timer: cur %ld -> expire %ld\r\n", jiffies, my_timer.expires);
	vk_add_timer(&my_timer);

	//check pending state
	if (1 != vk_timer_pending(&my_timer)) {
		DBG_ERR("after vk_add_timer: vk_timer_pending(%d) should be 1\r\n", vk_timer_pending(&my_timer));
		return -1;
	}

	//wait timer
	vos_util_delay_ms(3000);

	//delete timer
	is_deactivated = vk_del_timer_sync(&my_timer);
	DBG_DUMP("my_timer is_deactivated %d\r\n", is_deactivated);

	return 0;
}

void _workqueue_func(struct vk_work_struct *work)
{
	int num_loop = 10;

	while (num_loop--) {
		DBG_DUMP("jif %ld: I am %s\r\n", jiffies, __func__);
		vos_util_delay_ms(200);
	}
}

static int vk_workqueue_test(void)
{
	struct vk_delayed_work delayed_work = {0};
	struct vk_workqueue_struct *p_workqueue = NULL;
	int delay_jiffies = 50;

	p_workqueue = vk_create_workqueue("vk_workqueue_test");
	if (NULL == p_workqueue) {
		DBG_ERR("vk_create_workqueue failed\r\n");
		return -1;
	}

	VK_INIT_DELAYED_WORK(&delayed_work, _workqueue_func);

	DBG_DUMP("jif %ld: vk_queue_delayed_work, delay_jiffies %d\r\n", jiffies, delay_jiffies);
	vk_queue_delayed_work(p_workqueue, &delayed_work, delay_jiffies);

	//wait for workqueue start
	vos_util_delay_ms(1000);

	DBG_DUMP("jif %ld: vk_cancel_delayed_work_sync, wait for _workqueue_func\r\n", jiffies);
	vk_cancel_delayed_work_sync(&delayed_work);
	DBG_DUMP("jif %ld: _workqueue_func is done\r\n", jiffies);

	vk_destroy_workqueue(p_workqueue);
	DBG_DUMP("vk_destroy_workqueue done\r\n");

	return 0;
}
#endif

static int sem_test(void)
{
	int task_num = 2;
	int ret;

	DBG_DUMP("vos_sem_max_cnt() = %d\r\n", vos_sem_max_cnt());
	DBG_DUMP("vos_sem_used_cnt() = %d\r\n", vos_sem_used_cnt());

	//old usage: SEM_CREATE(xSemaphore1, 1);
	if (0 != vos_sem_create(&xSemaphore1, 1, "my_xSemaphore1")) {
		return -1;
	}
	if (0 != vos_sem_create(&xSemaphore2, 1, "my_xSemaphore2")) {
		return -1;
	}
	if (0 != vos_sem_create(&xSemTaskDone, 0, "xSemTaskDone")) {
		return -1;
	}
	vos_sem_wait(xSemaphore1); //old usage: SEM_WAIT(xSemaphore1);

	DBG_DUMP("vos_sem_used_cnt() = %d\r\n", vos_sem_used_cnt());

	vos_perf_list_reset();

	//test vos_util_msec_to_tick and use vos_perf to check
	vos_perf_list_mark(__func__, __LINE__, 0);
	ret = vos_sem_wait_timeout(xSemaphore1, vos_util_msec_to_tick(55));
	vos_perf_list_mark(__func__, __LINE__, 0);

	if (ret != E_TMOUT) {
		DBG_ERR("vos_sem_wait_timeout failed\r\n");
	} else {
		DBG_DUMP("vos_sem_wait_timeout, ret 0x%X\r\n", ret);
	}

	vos_perf_list_dump();
	vos_perf_list_reset();

	//old usage: THREAD_CREATE(xTaskHdl1, sem_task1, NULL, "TestRunner1");
	xTaskHdl1 = vos_task_create(sem_task1, NULL, "sem_task1", 10, 4*1024);
	if (0 == xTaskHdl1) {
		DBG_DUMP("create sem_task1 failed\r\n");
		return -1;
	}
	vos_task_resume(xTaskHdl1);
	vos_task_set_priority(xTaskHdl1, 10);

	//old usage: THREAD_CREATE(xTaskHdl2, sem_task2, NULL, "TestRunner2");
	xTaskHdl2 = vos_task_create(sem_task2, NULL, "sem_task2", 10, 4*1024);
	if (0 == xTaskHdl2) {
		DBG_DUMP("create sem_task2 failed\r\n");
		return -1;
	}
	vos_task_resume(xTaskHdl2);

	vos_perf_list_dump();

	//wait task done
	while(task_num--) {
		vos_sem_wait(xSemTaskDone);
	}

	vos_sem_destroy(xSemaphore1);
	vos_sem_destroy(xSemaphore2);
	vos_sem_destroy(xSemTaskDone);

#ifndef __LINUX_USER
	DBG_WRN("sem error check -------- begin\r\n");
	vos_sem_destroy(99999);
	if (0 != vos_sem_wait(99999)) {
		DBG_DUMP("intentional error\r\n");
	}
	if (0 != vos_sem_wait_timeout(99999, 100)) {
		DBG_DUMP("intentional error\r\n");
	}
	if (0 != vos_sem_wait_interruptible(99999)) {
		DBG_DUMP("intentional error\r\n");
	}
	vos_sem_sig(99999);
	DBG_WRN("sem error check -------- end\r\n");
#endif

	return 0;
}

static int flag_test(void)
{
	FLGPTN retptn = 0;
	FLGPTN waiptn = FLGPTN_DONE_TASK1 | FLGPTN_DONE_TASK2 | FLGPTN_DONE_TASK3 | FLGPTN_DONE_TASK4;
	int ret;

	DBG_DUMP("vos_flag_max_cnt() = %d\r\n", vos_flag_max_cnt());
	DBG_DUMP("vos_flag_used_cnt() = %d\r\n", vos_flag_used_cnt());

	//old usage: OS_CONFIG_FLAG(xFlag1);
	if (E_OK != cre_flg(&xFlag1, NULL, "xFlag1")) {
		return -1;
	}
	if (E_OK != cre_flg(&xFlagTask3, NULL, "xFlagTask3")) {
		return -1;
	}
	if (E_OK != cre_flg(&xFlagTask4, NULL, "xFlagTask4")) {
		return -1;
	}
	if (E_OK != cre_flg(&xFlagTaskDone, NULL, "xFlagTaskDone")) {
		return -1;
	}

	DBG_DUMP("vos_flag_used_cnt() = %d\r\n", vos_flag_used_cnt());

	vos_perf_list_reset();

	//test vos_util_msec_to_tick and use vos_perf to check
	vos_perf_list_mark(__func__, __LINE__, 0);
	ret = vos_flag_wait_timeout(&retptn, xFlag1, waiptn, TWF_ANDW, vos_util_msec_to_tick(100));
	vos_perf_list_mark(__func__, __LINE__, 0);

	if (ret != E_TMOUT)
		DBG_ERR("vos_flag_wait_timeout failed\r\n");
	else {
		DBG_DUMP("vos_flag_wait_timeout, ret 0x%X\r\n", ret);
	}

	vos_perf_list_dump();
	vos_perf_list_reset();

	set_flg(xFlag1, FLGPTN_XFLAG_BIT0);

	//old usage: THREAD_CREATE(xTaskHdl3, flag_task1, NULL, "TestRunner3");
	xTaskHdl3 = vos_task_create(flag_task1, NULL, "flag_task1", 10, 4*1024);
	if (0 == xTaskHdl3) {
		DBG_DUMP("create flag_task1 failed\r\n");
		return -1;
	}
	vos_task_resume(xTaskHdl3);

	//old usage: THREAD_CREATE(xTaskHdl4, flag_task2, NULL, "TestRunner4");
	xTaskHdl4 = vos_task_create(flag_task2, NULL, "flag_task2", 10, 4*1024);
	if (0 == xTaskHdl4) {
		DBG_DUMP("create flag_task2 failed\r\n");
		return -1;
	}
	vos_task_resume(xTaskHdl4);

	xTaskHdl5 = vos_task_create(flag_task3, NULL, "flag_task3", 10, 4*1024);
	if (0 == xTaskHdl5) {
		DBG_DUMP("create flag_task3 failed\r\n");
		return -1;
	}
	vos_task_resume(xTaskHdl5);

	xTaskHdl6 = vos_task_create(flag_task4, NULL, "flag_task4", 10, 4*1024);
	if (0 == xTaskHdl6) {
		DBG_DUMP("create flag_task4 failed\r\n");
		return -1;
	}
	vos_task_resume(xTaskHdl6);

	vos_flag_wait(&retptn, xFlagTaskDone, waiptn, TWF_ANDW);
	vos_flag_destroy(xFlag1);
	vos_flag_destroy(xFlagTask3);
	vos_flag_destroy(xFlagTask4);
	vos_flag_destroy(xFlagTaskDone);

#ifndef __LINUX_USER
	{
		FLGPTN dummyptn = 0;
		DBG_WRN("flag error check -------- begin\r\n");
		vos_flag_destroy(99999);
		vos_flag_set(99999, dummyptn);
		if (0 != vos_flag_clr(99999, dummyptn)) {
			DBG_DUMP("intentional error\r\n");
		}
		if (E_OK != vos_flag_wait(&retptn, 99999, waiptn, TWF_ORW)) {
			DBG_DUMP("intentional error\r\n");
		}
		if (E_OK != vos_flag_wait_timeout(&retptn, 99999, waiptn, TWF_ORW, 100)) {
			DBG_DUMP("intentional error\r\n");
		}
		if (0 == vos_flag_chk(99999, dummyptn)) {
			DBG_DUMP("intentional error\r\n");
		}
		DBG_WRN("flag error check -------- end\r\n");
	}
#endif

	return 0;
}

int file_test(void)
{
	int ret = 0;
	VOS_FILE fd;
	int rw_bytes;
	char write_buf[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char read_buf[64] = {0};
	off_t offset;
	struct vos_stat statbuf = {0};

	//linux style path => /mnt/sd
	fd = vos_file_open("/mnt/sd/test.txt", O_RDWR|O_CREAT|O_TRUNC, 666);
	if (VOS_FILE_INVALID == fd) {
		DBG_ERR("vos_file_open wr fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_open write fd = 0x%lX\r\n", (ULONG)fd);

	rw_bytes = vos_file_write(fd, write_buf, sizeof(write_buf));
	if (sizeof(write_buf) != rw_bytes) {
		DBG_ERR("vos_file_write fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_write %d bytes, data [%s]\r\n", rw_bytes, write_buf);

	if (0 > vos_file_fsync(fd)) {
		DBG_ERR("vos_file_fsync fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("after vos_file_fsync\r\n");

	if (-1 == vos_file_close(fd)) {
		DBG_ERR("vos_file_close fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}

#ifdef __FREERTOS
	//uitron style path => A:
	fd = vos_file_open("A:\\test.txt", O_RDONLY, 666);
#else
	fd = vos_file_open("/mnt/sd/test.txt", O_RDONLY, 666);
#endif
	if (VOS_FILE_INVALID == fd) {
		DBG_ERR("vos_file_open read fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_open read fd = 0x%lX, read_buf 0x%lX\r\n", (ULONG)fd, (ULONG)read_buf);

	rw_bytes = vos_file_read(fd, read_buf, sizeof(read_buf));
	if (-1 == rw_bytes) {
		DBG_ERR("vos_file_read fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_read %d bytes, data [%s]\r\n", rw_bytes, read_buf);

	offset = vos_file_lseek(fd, 0, SEEK_END);
	if ((off_t)(-1) == ret) {
		DBG_ERR("vos_file_lseek fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_lseek end, current offset %ld\r\n", offset);

	offset = vos_file_lseek(fd, 7, SEEK_SET);
	if ((off_t)(-1) == ret) {
		DBG_ERR("vos_file_lseek fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_lseek 7 from head, current offset %ld\r\n", offset);

	rw_bytes = vos_file_read(fd, read_buf, sizeof(read_buf));
	if (-1 == rw_bytes) {
		DBG_ERR("vos_file_read fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_read %d bytes, data [%s]\r\n", rw_bytes, read_buf);

	if (-1 == vos_file_fstat(fd, &statbuf)) {
		DBG_ERR("vos_file_fstat fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_fstat size = %ld, mode 0x%X, ISDIR %d\r\n", statbuf.st_size, statbuf.st_mode, S_ISDIR(statbuf.st_mode));

	if (-1 == vos_file_close(fd)) {
		DBG_ERR("vos_file_close fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	fd = VOS_FILE_INVALID; //fd is closed

	memset(&statbuf, 0, sizeof(statbuf));
	if (-1 == vos_file_stat("/mnt/sd/test.txt", &statbuf)) {
		DBG_ERR("vos_file_stat fail\r\n");
		ret = -1;
		goto test_vos_file_end;
	}
	DBG_DUMP("vos_file_stat size = %ld, mode 0x%X, ISDIR %d\r\n", statbuf.st_size, statbuf.st_mode, S_ISDIR(statbuf.st_mode));

test_vos_file_end:
	if (VOS_FILE_INVALID != fd) {
		if (-1 == vos_file_close(fd)) {
			DBG_ERR("vos_file_close fail\r\n");
			ret = -1;
		}
	}

	return ret;
}

static int sscanf_s_test(const char* str)
{
	char path[16] = {0};
	int val = 0;
	int ret;

	ret = sscanf_s(str, "%s %d", path, sizeof(path), &val);
	if (ret != 2) {
		DBG_ERR("sscanf_s failed, ret %d\r\n", ret);
	} else {
		DBG_DUMP("sscanf_s path [%s] val %d, ret %d\r\n", path, val, ret);
	}

	return 0;
}

static VK_DEFINE_SPINLOCK(g_test_lock);

THREAD_RETTYPE spin_task1(void *param)
{
	int num_loop = 10;
	unsigned long flags;

	while (num_loop--) {
		vk_spin_lock_irqsave(&g_test_lock, flags);
		DBG_DUMP("I am %s\n", __func__);
		vk_spin_unlock_irqrestore(&g_test_lock, flags);
	}

	THREAD_RETURN(0);
}

THREAD_RETTYPE spin_task2(void *param)
{
	int num_loop = 10;
	unsigned long flags;

	while (num_loop--) {
		vk_spin_lock_irqsave(&g_test_lock, flags);
		DBG_DUMP("I am %s\n", __func__);
		vk_spin_unlock_irqrestore(&g_test_lock, flags);
	}

	THREAD_RETURN(0);
}

static int spinlock_test(void)
{
	DBG_DUMP("vk_spinlock_t size = %ld\r\n", (ULONG)sizeof(vk_spinlock_t));
	DBG_DUMP("spinlock_t size = %ld\r\n", (ULONG)sizeof(spinlock_t));

	DBG_DUMP("vos_task_max_cnt() = %d\r\n", vos_task_max_cnt());
	DBG_DUMP("vos_task_used_cnt() = %d\r\n", vos_task_used_cnt());

	g_tskhdl_spin1 = vos_task_create(spin_task1, NULL, "spin_task1", 10, 4*1024);
	if (0 == g_tskhdl_spin1) {
		DBG_DUMP("create sem_task1 failed\r\n");
		return -1;
	}
	vos_task_resume(g_tskhdl_spin1);

	g_tskhdl_spin2 = vos_task_create(spin_task2, NULL, "spin_task2", 10, 4*1024);
	if (0 == g_tskhdl_spin2) {
		DBG_DUMP("create sem_task2 failed\r\n");
		return -1;
	}
	vos_task_resume(g_tskhdl_spin2);

	DBG_DUMP("vos_task_used_cnt() = %d\r\n", vos_task_used_cnt());

	return 0;
}

static int bitfield_test(void)
{
	int idx;
	int bit_low, bit_high;
	unsigned long reg;

	for (idx = 0; idx < BITS_PER_LONG; idx++) {
		DBG_DUMP("BIT(%d) = 0x%016lX\r\n", idx, BIT(idx));
	}

	for (idx = 0; idx < BITS_PER_LONG; idx += 8) {
		bit_high = idx + 7;
		bit_low = idx;
		DBG_DUMP("GENMASK(%d, %d) = 0x%016lX\r\n",
			bit_high, bit_low, GENMASK(bit_high, bit_low));
	}

	DBG_DUMP("FIELD_PREP(GENMASK(15, 0), 0x4321) = 0x%08lX\r\n",
		FIELD_PREP(GENMASK(15, 0), 0x4321));

	DBG_DUMP("FIELD_PREP(GENMASK(31, 16), 0x8765) = 0x%08lX\r\n",
		FIELD_PREP(GENMASK(31, 16), 0x8765));

	DBG_DUMP("FIELD_GET(GENMASK(31, 24), 0x12345678UL) = 0x%lX\r\n",
		FIELD_GET(GENMASK(31, 24), 0x12345678UL));

	DBG_DUMP("FIELD_GET(GENMASK(23, 16), 0x12345678UL) = 0x%lX\r\n",
		FIELD_GET(GENMASK(23, 16), 0x12345678UL));

	DBG_DUMP("FIELD_GET(GENMASK(15, 8), 0x12345678UL) = 0x%lX\r\n",
		FIELD_GET(GENMASK(15, 8), 0x12345678UL));

	DBG_DUMP("FIELD_GET(GENMASK(7, 0), 0x12345678UL) = 0x%lX\r\n",
		FIELD_GET(GENMASK(7, 0), 0x12345678UL));

	reg = 0x12345678;
	DBG_DUMP("Before FIELD_SET reg = 0x%lX\r\n", reg);
	DBG_DUMP("FIELD_SET(GENMASK(31, 24), 0xAA, reg) = 0x%lX\r\n",
		FIELD_SET(GENMASK(31, 24), 0xAA, reg));
	DBG_DUMP("FIELD_SET(GENMASK(23, 16), 0xBB, reg) = 0x%lX\r\n",
		FIELD_SET(GENMASK(23, 16), 0xBB, reg));
	DBG_DUMP("FIELD_SET(GENMASK(15, 8), 0xCC, reg) = 0x%lX\r\n",
		FIELD_SET(GENMASK(15, 8), 0xCC, reg));
	DBG_DUMP("FIELD_SET(GENMASK(7, 0), 0xDD, reg) = 0x%lX\r\n",
		FIELD_SET(GENMASK(7, 0), 0xDD, reg));

	DBG_DUMP("FIELD_CLEAR(GENMASK(7, 0), reg) = 0x%lX\r\n",
		FIELD_CLEAR(GENMASK(7, 0), reg));

	DBG_DUMP("FIELD_CLEAR(GENMASK(15, 8), reg) = 0x%lX\r\n",
		FIELD_CLEAR(GENMASK(15, 8), reg));

	DBG_DUMP("FIELD_FIT(0x00FF, 0x1234) = %d\r\n", FIELD_FIT(0x00FF, 0x1234));
	DBG_DUMP("FIELD_FIT(0xFFFF, 0x1234) = %d\r\n", FIELD_FIT(0xFFFF, 0x1234));

	return 0;
}

int test_vos(int argc, char** argv)
{
	print_dbg_msg();

#ifdef __LINUX
	if (0 != dev_test()) {
		DBG_ERR("dev_test failed\r\n");
	}
#endif

#ifndef __LINUX_USER
	if (0 != mem_test()) {
		DBG_ERR("mem_test failed\r\n");
	}

	if (0 != vk_kxalloc_test()) {
		DBG_ERR("kxalloc_test failed\r\n");
	}

	if (0 != vk_kthread_test()) {
		DBG_ERR("kthread_test failed\r\n");
	}

	if (0 != vk_mutex_test()) {
		DBG_ERR("vk_mutex_test failed\r\n");
	}

	if (0 != vk_tasklet_test()) {
		DBG_ERR("vk_tasklet_test failed\r\n");
	}

	if (0 != vk_timer_test()) {
		DBG_ERR("vk_timer_test failed\r\n");
	}

	if (0 != vk_workqueue_test()) {
		DBG_ERR("vk_workqueue_test failed\r\n");
	}
#endif

	if (0 != bitfield_test()) {
		DBG_ERR("bitfield_test failed\r\n");
	}

	if (0 != spinlock_test()) {
		DBG_ERR("spinlock_test failed\r\n");
	}

	if (0 != sem_test()) {
		DBG_ERR("sem_test failed\r\n");
	}

	if (0 != flag_test()) {
		DBG_ERR("flag_test failed\r\n");
	}

	if (0 != file_test()) {
		DBG_ERR("file_test failed\r\n");
	}

	if (0 != sscanf_s_test("/mnt/sd 123")) {
		DBG_ERR("sscanf_s_test failed\r\n");
	}

#ifndef __LINUX
	if (0 != mbx_test()) {
		DBG_ERR("mbx_test failed\r\n");
	}
#endif

	return 0;
}
