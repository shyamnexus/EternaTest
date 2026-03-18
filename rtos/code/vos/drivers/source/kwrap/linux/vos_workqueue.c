/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include <kwrap/debug.h>
#include <kwrap/workqueue.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
#define VOS_WORKQUEUE_TAG_INITED MAKEFOURCC('V', 'W', 'K', 'Q')

#ifndef sizeof_field
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif

//Note: additional sizeof(u64) for the buf offset
STATIC_ASSERT(sizeof_field(struct vk_delayed_work, buf) >= (sizeof(struct delayed_work) + sizeof(u64)));

#if 0 //to print sizeof at compiler time
char (*__native_struct)[sizeof(struct delayed_work)] = 1;
char (*__vos_struct)[sizeof_field(struct vk_delayed_work, buf)] = 1;
#endif

//Note:
//For RT Linux, spinlock is directed to an rt mutex and use atomic 64
//The buf should aligned to 64-bit (8 bytes)
#define GET_ALIGN_BUF(p) ALIGN_CEIL_8((unsigned long)(p)->buf)
/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void __vk_init_delayed_work(struct vk_delayed_work *vos_dwork, vk_work_func_t func, unsigned int flags)
{
	struct delayed_work *native_dwork = (struct delayed_work *)GET_ALIGN_BUF(vos_dwork);

	__INIT_DELAYED_WORK(native_dwork, (void *)func, flags);
}
EXPORT_SYMBOL(__vk_init_delayed_work);

struct vk_workqueue_struct* vk_create_workqueue(const char *name)
{
	struct workqueue_struct *native_wq;

	native_wq = create_workqueue(name);

	return (struct vk_workqueue_struct *)native_wq;
}
EXPORT_SYMBOL(vk_create_workqueue);

bool vk_queue_delayed_work(struct vk_workqueue_struct *vos_wq, struct vk_delayed_work *vos_dwork, unsigned long delay_jiffies)
{
	struct workqueue_struct *native_wq = (struct workqueue_struct *)vos_wq;
	struct delayed_work *native_dwork = (struct delayed_work *)GET_ALIGN_BUF(vos_dwork);

	return queue_delayed_work(native_wq, native_dwork, delay_jiffies);
}
EXPORT_SYMBOL(vk_queue_delayed_work);

bool vk_queue_delayed_work_on(int cpu, struct vk_workqueue_struct *vos_wq, struct vk_delayed_work *vos_dwork, unsigned long delay_jiffies)
{
	struct workqueue_struct *native_wq = (struct workqueue_struct *)vos_wq;
	struct delayed_work *native_dwork = (struct delayed_work *)GET_ALIGN_BUF(vos_dwork);

	return queue_delayed_work_on(cpu, native_wq, native_dwork, delay_jiffies);
}
EXPORT_SYMBOL(vk_queue_delayed_work_on);

bool vk_cancel_delayed_work(struct vk_delayed_work *vos_dwork)
{
	struct delayed_work *native_dwork = (struct delayed_work *)GET_ALIGN_BUF(vos_dwork);

	return cancel_delayed_work(native_dwork);
}
EXPORT_SYMBOL(vk_cancel_delayed_work);

bool vk_cancel_delayed_work_sync(struct vk_delayed_work *vos_dwork)
{
	struct delayed_work *native_dwork = (struct delayed_work *)GET_ALIGN_BUF(vos_dwork);

	return cancel_delayed_work_sync(native_dwork);
}
EXPORT_SYMBOL(vk_cancel_delayed_work_sync);

void vk_destroy_workqueue(struct vk_workqueue_struct *vos_wq)
{
	struct workqueue_struct *native_wq = (struct workqueue_struct *)vos_wq;

	destroy_workqueue(native_wq);
}
EXPORT_SYMBOL(vk_destroy_workqueue);
