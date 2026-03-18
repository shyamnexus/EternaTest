#if defined(__LINUX)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#if defined (__aarch64__)
#include <linux/soc/nvt/fmem.h>
#else
#include <mach/fmem.h>
#endif
#endif

#include <kwrap/type.h>
#include <kwrap/flag.h>
#include <kdrv_rpc/kdrv_rpc.h>
#include <kflow_rpc/nvt_ipc.h>
#include <kflow_rpc/nvt_ipc_ioctl.h>
#include "nvt_ipc_common.h"
#include "nvt_ipc_debug.h"
#include "nvt_ipc_msg_que.h"
#include "nvt_ipc_drv.h"

#define COPY_FROM_USER_ERR_STR    "copy_from_user"
#define COPY_TO_USER_ERR_STR      "copy_to_user"

#if defined(__FREERTOS)
static NVT_IPC_MODULE_INFO    nvt_ipc_module_info;
#endif

extern struct mutex nvt_ipc_mutex;

#if defined(__LINUX)
int nvt_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int __user *argp = (int __user *)arg;
	struct nvt_ipc_device  *p_hdl_dev;

	DBG_IND("cmd = 0x%x\r\n", cmd);
	switch (cmd) {
	case NVTIOC_IPC_INIT: {
			// do nothing
		}
		break;
	case NVTIOC_IPC_EXIT: {
			// do nothing
		}
		break;
	case NVTIOC_IPC_MSGQUE_GET: {
			int                     msqid;
			NVTIPC_IOC_MSGQUE_GET_S msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			msqid = kflow_nvt_ipc_msg_get(msg.key);
			if (likely(msqid >= 0)) {
				mutex_lock(&nvt_ipc_mutex);
				p_hdl_dev = file->private_data;
				if (p_hdl_dev) {
					p_hdl_dev->req_queue_bits |= (1 << msqid);
				}
				mutex_unlock(&nvt_ipc_mutex);
			}
			msg.rtn = msqid;
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_TO_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;
	case NVTIOC_IPC_MSGQUE_REL: {
			NVTIPC_IOC_MSGQUE_REL_S msg = {0};
			PNVT_IPC_MODULE_INFO pmodule_info = nvt_ipc_drv_get_module_info();
			ID                   flg_id = pmodule_info->flg_id;


			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			if (msg.msqid >= NVTIPC_MSG_QUEUE_NUM) {
				DBG_ERR("msqid = %d\r\n", (int)msg.msqid);
				return -EFAULT;
			}
			msg.rtn = kflow_nvt_ipc_msg_rel(msg.msqid);
			if (likely(msg.rtn >= 0)) {
				mutex_lock(&nvt_ipc_mutex);
				p_hdl_dev = file->private_data;
				if (p_hdl_dev) {
					p_hdl_dev->req_queue_bits &= ~(1 << msg.msqid);
				}
				mutex_unlock(&nvt_ipc_mutex);
				// wake up the waiting process
				//wake_up_interruptible(&p_drv_info->module_info.nvt_read_waitq[msg.msqid]);
				vos_flag_set(flg_id, (1 << msg.msqid));
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_TO_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;
	case NVTIOC_IPC_CONSOLE_CMD: {
			NVTIPC_IOC_MEM_BUF buf = {0};

			if (unlikely(copy_from_user(&buf, argp, sizeof(buf)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;
	case NVTIOC_IPC_SYS_CMD: {
			unsigned int cmd_id;

			if (unlikely(copy_from_user(&cmd_id, argp, sizeof(cmd_id)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			switch (cmd_id) {
			case NVTIPC_SYSCMD_CPU2_POWERON_READY:

				break;
			case NVTIPC_SYSCMD_SYSCALL_ACK:
				kflow_nvt_ipc_syscmd_ack(cmd_id);
				break;
			case NVTIPC_SYSCMD_UART_ACK:
				kflow_nvt_ipc_syscmd_ack(cmd_id);
				break;
			case NVTIPC_SYSCMD_IPC_DUMPINFO:
				nvt_ipc_drv_dump_debug();
				break;
			case NVTIPC_SYSCMD_SUSPEND_REQ:
				#if __IPC_SIG_SUPPORT__
				nvt_ipc_sig_send(NVTIPC_SIG_SUSPEND);
				if (nvt_ipc_sig_wait_all_ack(NVTIPC_SIG_SUSPEND, NVTIPC_SIG_SUSPEND_TIMEOUT_SEC) == -ERESTARTSYS)
					return -ERESTARTSYS;
				#endif
				break;
			case NVTIPC_SYSCMD_GET_LONG_COUNTER_ACK:
				kflow_nvt_ipc_syscmd_ack(cmd_id);
				break;
			default:
				DBG_ERR("Unknown syscmd %d\r\n", cmd_id);
				return -EFAULT;
			}

		}
		break;
	case NVTIOC_IPC_FTOK: {
			NVTIPC_IOC_FTOK_S msg;

			memset(&msg, 0x00, sizeof(msg));
			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			msg.path[NVTIPC_MSG_QUEUE_TOKEN_STR_MAXLEN] = 0;
			msg.rtn = kflow_nvt_ipc_ftok(msg.path);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_TO_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;
	case NVTIOC_IPC_MSG_SND: {
			NVTIPC_IOC_MSG_SND_S msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			msg.rtn = kflow_nvt_ipc_msg_snd_p(msg.msqid, msg.send_to, msg.msg, msg.msgsz, 1);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_TO_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;
	case NVTIOC_IPC_MSG_RCV: {
			NVTIPC_IOC_MSG_RCV_S msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			msg.rtn = kflow_nvt_ipc_msg_rcv(msg.msqid, msg.msg, msg.msgsz, msg.timeout_ms);
			if (msg.rtn == -ERESTARTSYS) {
				return -ERESTARTSYS;
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_TO_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;
		#if __IPC_SIG_SUPPORT__
		case NVTIOC_IPC_SIG_WAIT: {
			NVTIOC_IPC_SIG_WAIT_S msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			msg.rtn = nvt_ipc_sig_wait();
			if (msg.rtn == -ERESTARTSYS) {
				return -ERESTARTSYS;
			}
			// keep wait ack info


			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_TO_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;

		case NVTIOC_IPC_SIG_ACK: {
			NVTIOC_IPC_SIG_ACK_S msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			msg.rtn = nvt_ipc_sig_ack(msg.sig);
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_TO_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;
		#endif
	case NVTIOC_IPC_WIAT_CORE_RDY: {
			NVTIPC_IOC_WIAT_CORE_RDY_S msg = {0};

			if (unlikely(copy_from_user(&msg, argp, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_FROM_USER_ERR_STR);
				return -EFAULT;
			}
			msg.rtn = kflow_nvt_ipc_wait_core_rdy(msg.core_id, msg.timeout_ms);
			if (msg.rtn == -ERESTARTSYS) {
				return -ERESTARTSYS;
			}
			if (unlikely(copy_to_user(argp, &msg, sizeof(msg)))) {
				DBG_ERR("%s\r\n", COPY_TO_USER_ERR_STR);
				return -EFAULT;
			}
		}
		break;
	default:
		DBG_ERR("nvt-ipc Unknown cmd 0x%x\r\n", cmd);
		return -EINVAL;
	}
	return 0;
}
#else
int nvt_ipc_ioctl (int fd, unsigned int cmd, void *p_arg)
{
	DBG_IND("cmd = 0x%x\r\n", cmd);
	switch (cmd) {
	case NVTIOC_IPC_INIT: {
			kdrv_rpc_init();
			nvt_ipc_drv_init(&nvt_ipc_module_info);
		}
		break;
	case NVTIOC_IPC_EXIT: {
			nvt_ipc_drv_exit(&nvt_ipc_module_info);
			kdrv_rpc_exit();
		}
		break;
	case NVTIOC_IPC_MSGQUE_GET: {
			NVTIPC_IOC_MSGQUE_GET_S *p_msg = (NVTIPC_IOC_MSGQUE_GET_S *)p_arg;

			p_msg->rtn = kflow_nvt_ipc_msg_get(p_msg->key);
		}
		break;
	case NVTIOC_IPC_MSGQUE_REL: {
			NVTIPC_IOC_MSGQUE_REL_S *p_msg = (NVTIPC_IOC_MSGQUE_REL_S *)p_arg;
			PNVT_IPC_MODULE_INFO pmodule_info = nvt_ipc_drv_get_module_info();
			ID                   flg_id = pmodule_info->flg_id;

			if (p_msg->msqid >= NVTIPC_MSG_QUEUE_NUM) {
				DBG_ERR("msqid = %d\r\n", (int)p_msg->msqid);
				return -1;
			}
			p_msg->rtn = kflow_nvt_ipc_msg_rel(p_msg->msqid);
			if (p_msg->rtn >= 0) {
				// wake up the waiting process
				vos_flag_set(flg_id, (1 << p_msg->msqid));
			}
		}
		break;

	case NVTIOC_IPC_SYS_CMD: {
			unsigned int cmd_id = *(unsigned int *)p_arg;

			switch (cmd_id) {
			case NVTIPC_SYSCMD_CPU2_POWERON_READY:

				break;
			case NVTIPC_SYSCMD_SYSCALL_ACK:
				kflow_nvt_ipc_syscmd_ack(cmd_id);
				break;
			case NVTIPC_SYSCMD_UART_ACK:
				kflow_nvt_ipc_syscmd_ack(cmd_id);
				break;
			case NVTIPC_SYSCMD_IPC_DUMPINFO:
				nvt_ipc_drv_dump_debug();
				break;
			default:
				DBG_ERR("Unknown syscmd %d\r\n", cmd_id);
				return -1;
			}

		}
		break;
	case NVTIOC_IPC_FTOK: {
			NVTIPC_IOC_FTOK_S *p_msg = (NVTIPC_IOC_FTOK_S *)p_arg;

			p_msg->rtn = kflow_nvt_ipc_ftok(p_msg->path);
		}
		break;
	case NVTIOC_IPC_MSG_SND: {
			NVTIPC_IOC_MSG_SND_S *p_msg = (NVTIPC_IOC_MSG_SND_S *)p_arg;

			p_msg->rtn = kflow_nvt_ipc_msg_snd_p(p_msg->msqid, p_msg->send_to, p_msg->msg, p_msg->msgsz, 0);
		}
		break;
	case NVTIOC_IPC_MSG_RCV: {
			NVTIPC_IOC_MSG_RCV_S *p_msg = (NVTIPC_IOC_MSG_RCV_S *)p_arg;

			p_msg->rtn = kflow_nvt_ipc_msg_rcv(p_msg->msqid, p_msg->msg, p_msg->msgsz, p_msg->timeout_ms);
		}
		break;

	case NVTIOC_IPC_WIAT_CORE_RDY: {
			NVTIPC_IOC_WIAT_CORE_RDY_S *p_msg = (NVTIPC_IOC_WIAT_CORE_RDY_S *)p_arg;

			p_msg->rtn = kflow_nvt_ipc_wait_core_rdy(p_msg->core_id, p_msg->timeout_ms);
		}
		break;
	default:
		DBG_ERR("nvt-ipc Unknown cmd 0x%x\r\n", cmd);
		return -1;
	}
	return 0;
}

#endif

