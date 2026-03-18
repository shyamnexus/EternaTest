/*
 *   @file   info.c
 *
 *   @brief  The Log System information API.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#define _INFO_C_

#include <linux/version.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include "vg_common.h"
#include "log.h"
#include "vg_semaphore.h"
#include "kwrap/list.h"
#include "kwrap/spinlock.h"
#include "info.h"

#define INFO_PANIC(fmt, args...) \
	printm("VG", fmt, ##args);    \
	printk(fmt, ##args);          \
	damnit("VG");

#define EMPTY_VALUE     0xFEFEFEFE
#define PROC_BUF_LEN    (32*1024)

struct vos_list_head vg_info_listhead;
static struct proc_dir_entry *bindfd_all_proc;
static struct proc_dir_entry *entity_by_name_proc, *entity_by_bindfd_proc;
static struct proc_dir_entry *entity_by_fd_proc, *entity_by_pch_proc;
static char *proc_buf = 0;

static vk_spinlock_t vg_info_lock;

int util_is_file_exist(char *full_pathname)
{
	int is_existed = 0;
	mm_segment_t fs;
	struct file *fp = NULL;

	if (!full_pathname) {
		goto exit;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(full_pathname, O_RDONLY, S_IRWXU);
	if (IS_ERR(fp) || NULL == fp) {
		set_fs(fs);
		goto exit;
	}
	if (d_is_dir(fp->f_path.dentry)) {
		set_fs(fs);
		goto exit;
	}
	is_existed = 1;

	set_fs(fs);
exit:
	return is_existed;
}
EXPORT_SYMBOL(util_is_file_exist);


vg_info_item_t *new_vg_info_item(int fd, int graph_type, int bindfd, char *name, int vch, int pch)
{
	vg_info_item_t *p_vg_info_item = NULL;
	unsigned long flags;

	if (fd == 0) {
		INFO_PANIC("Fail to add new info_item with fd = 0.\n");
		goto exit;
	}
	p_vg_info_item = kmalloc(sizeof(struct vg_info_item_tag), GFP_KERNEL);
	if (!p_vg_info_item) {
		INFO_PANIC("Fail to allocate vg_info_item.\n");
		goto exit;
	}

	memset(p_vg_info_item, 0, sizeof(*p_vg_info_item));
	VOS_INIT_LIST_HEAD(&p_vg_info_item->list);
	p_vg_info_item->fd = fd;
	if (graph_type != EMPTY_VALUE) {
		p_vg_info_item->graph_type = graph_type;
	}
	if (bindfd != EMPTY_VALUE) {
		p_vg_info_item->bindfd = bindfd;
	}
	if (strcmp(name, "") != 0) {
		strncpy(p_vg_info_item->name, name, sizeof(p_vg_info_item->name) - 1);
		p_vg_info_item->name[sizeof(p_vg_info_item->name) - 1] = '\0';
	}
	if (vch != EMPTY_VALUE) {
		p_vg_info_item->vch = vch;
	}
	if (pch != EMPTY_VALUE) {
		p_vg_info_item->pch = pch;
	}
	p_vg_info_item->struct_magic = VG_INFO_STRUCT_MAGIC;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	vos_list_add_tail(&p_vg_info_item->list, &vg_info_listhead);
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

exit:
	return p_vg_info_item;
}


int util_del_vg_info_item(vg_info_item_t *p_vg_info_item)
{
	int ret = 0;

	if (p_vg_info_item->struct_magic != VG_INFO_STRUCT_MAGIC) {
		INFO_PANIC("Invalid vg_info_item(0x%p).\n", p_vg_info_item);
		ret = -1;
		goto exit;
	}
	vos_list_del(&p_vg_info_item->list); //already protected by spinlock
	kfree(p_vg_info_item);

exit:
	return ret;
}


int util_del_info_by_fd(int fd)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			ret = util_del_vg_info_item(p_vg_info_item);
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;

}
EXPORT_SYMBOL(util_del_info_by_fd);


int util_find_vg_info_item_by_fd(int fd)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	if (vos_list_empty(&vg_info_listhead)) {
		goto exit;
	}

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			ret = 0;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

exit:
	return ret;
}
EXPORT_SYMBOL(util_find_vg_info_item_by_fd);


int util_get_fd_by_name(char *name)
{
	int ret = 0;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (strcmp(p_vg_info_item->name, name) == 0) {
			ret = p_vg_info_item->fd;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_fd_by_name);


int util_get_graph_type_by_name(char *name)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (strcmp(p_vg_info_item->name, name) == 0) {
			ret = p_vg_info_item->graph_type;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_graph_type_by_name);


int util_get_bindfd_by_name(char *name)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (strcmp(p_vg_info_item->name, name) == 0) {
			ret = p_vg_info_item->bindfd;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_bindfd_by_name);


int util_get_vch_by_name(char *name)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (strcmp(p_vg_info_item->name, name) == 0) {
			ret =  p_vg_info_item->vch;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_vch_by_name);


int util_get_pch_by_name(char *name)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (strcmp(p_vg_info_item->name, name) == 0) {
			ret =  p_vg_info_item->pch;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_pch_by_name);


int util_get_graph_type_by_fd(int fd)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			ret = p_vg_info_item->graph_type;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_graph_type_by_fd);


int util_get_bindfd_by_fd(int fd)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			ret = p_vg_info_item->bindfd;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_bindfd_by_fd);


int util_get_vch_by_fd(int fd)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			ret = p_vg_info_item->vch;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_vch_by_fd);


int util_get_pch_by_fd(int fd)
{
	int ret = -1;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			ret = p_vg_info_item->pch;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_pch_by_fd);


char *util_get_name_by_fd(int fd)
{
	char *ret = NULL;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			ret = p_vg_info_item->name;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_name_by_fd);


char *util_get_name_by_bindfd(int bindfd)
{
	char *ret = NULL;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->bindfd == bindfd) {
			ret = p_vg_info_item->name;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_name_by_bindfd);

#define FD_TYPE(x) (x >> 24)
unsigned int util_get_fd_by_bindfd(int bindfd, int type)
{
	unsigned int fd = 0;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if ((p_vg_info_item->bindfd == bindfd) && (FD_TYPE(p_vg_info_item->fd) == type)) {
			fd = p_vg_info_item->fd;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);
	return fd;
}
EXPORT_SYMBOL(util_get_fd_by_bindfd);

char *util_get_name_by_vch(int vch)
{
	char *ret = NULL;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->vch == vch) {
			ret =  p_vg_info_item->name;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_name_by_vch);


char *util_get_name_by_pch(int pch)
{
	char *ret = NULL;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->pch == pch) {
			ret = p_vg_info_item->name;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return ret;
}
EXPORT_SYMBOL(util_get_name_by_pch);


int util_set_graph_type_by_fd(int fd, int graph_type)
{
	int ret = -1, is_find = 0;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	if (graph_type == EMPTY_VALUE) {
		goto exit;
	}

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			p_vg_info_item->graph_type = graph_type;
			ret = is_find = 1;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	if (!is_find) {
		new_vg_info_item(fd, graph_type, EMPTY_VALUE, "", EMPTY_VALUE, EMPTY_VALUE);
		ret = 0;
	}

exit:
	return ret;
}
EXPORT_SYMBOL(util_set_graph_type_by_fd);


int util_set_bindfd_by_fd(int fd, int bindfd)
{
	int ret = -1, is_find = 0;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	if (bindfd == EMPTY_VALUE) {
		goto exit;
	}

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			p_vg_info_item->bindfd = bindfd;
			ret = is_find = 1;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	if (!is_find) {
		new_vg_info_item(fd, EMPTY_VALUE, bindfd, "", EMPTY_VALUE, EMPTY_VALUE);
		ret = 0;
	}

exit:
	return ret;
}
EXPORT_SYMBOL(util_set_bindfd_by_fd);


int util_set_name_by_fd(int fd, char *name)
{
	int ret = -1, is_find = 0;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	if (strcmp(name, "") == 0) {
		goto exit;
	}

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			strncpy(p_vg_info_item->name, name, sizeof(p_vg_info_item->name) - 1);
			p_vg_info_item->name[sizeof(p_vg_info_item->name) - 1] = '\0';
			ret = is_find = 1;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	if (!is_find) {
		new_vg_info_item(fd, EMPTY_VALUE, EMPTY_VALUE, name, EMPTY_VALUE, EMPTY_VALUE);
		ret = 0;
	}

exit:
	return ret;

}
EXPORT_SYMBOL(util_set_name_by_fd);


int util_set_vch_by_fd(int fd, int vch)
{
	int ret = -1, is_find = 0;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	if (vch == EMPTY_VALUE) {
		goto exit;
	}

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			p_vg_info_item->vch = vch;
			ret = is_find = 1;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	if (!is_find) {
		new_vg_info_item(fd, EMPTY_VALUE, EMPTY_VALUE, "", vch, EMPTY_VALUE);
		ret = 0;
	}

exit:
	return ret;

}
EXPORT_SYMBOL(util_set_vch_by_fd);


int util_set_pch_by_fd(int fd, int pch)
{
	int ret = -1, is_find = 0;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;

	if (pch == EMPTY_VALUE) {
		goto exit;
	}
	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			p_vg_info_item->pch = pch;
			ret = is_find = 1;
			break;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	if (!is_find) {
		new_vg_info_item(fd, EMPTY_VALUE, EMPTY_VALUE, "", EMPTY_VALUE, pch);
		ret = 0;
	}

exit:
	return ret;

}
EXPORT_SYMBOL(util_set_pch_by_fd);


static int proc_bindfd_all_seq_show(struct seq_file *s, void *v)
{
	vg_info_item_t *p_vg_info_item;
	int is_first;
	unsigned long flags;
	char in_out_str[16];

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	is_first = 1;
	seq_printf(s, "---------------------------------------------------\n");
	seq_printf(s, "hdal_path(IN_OUT) Describe:\n");
	seq_printf(s, "\n");
	seq_printf(s, "    +----------------------------------- hdal_path\n");
	seq_printf(s, "    |                 +----------------- device_id\n");
	seq_printf(s, "    |                 |    +------------ in_id\n");
	seq_printf(s, "    |                 |    |  +--------- out_id\n");
	seq_printf(s, "    |                 |    |  |   +----- in\n");
	seq_printf(s, "    |                 |    |  |   | +--- out\n");
	seq_printf(s, "    V                 V    V  V   V V\n");
	seq_printf(s, "  0x21000901(8_0) = 0x2100 09 01 (8_0)\n");
	seq_printf(s, "\n");
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->graph_type == TYPE_DISPLAY) {
			if (is_first) {
				seq_printf(s, "Liveview/Playback:\n");
				seq_printf(s, "hdal_path(IN_OUT)    name                fd          vch  \n");
				seq_printf(s, "--------------------------------------------------------\n");
			}
			snprintf(in_out_str, sizeof(in_out_str) - 1, "(%d_%d)",
				 ((p_vg_info_item->bindfd & 0xff00) >> 8) - 1, (p_vg_info_item->bindfd & 0xff) - 1);
			seq_printf(s, "%-#10x%-9s  %-18s  %-#10x  %-3d\n", p_vg_info_item->bindfd, in_out_str,
				   p_vg_info_item->name, p_vg_info_item->fd, p_vg_info_item->vch);
			is_first = 0;
		}
	}

	is_first = 1;
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->graph_type == TYPE_ENCODE) {
			if (is_first) {
				seq_printf(s, "\nEncode:\n");
				seq_printf(s, "hdal_path(IN_OUT)    name                fd          vch  \n");
				seq_printf(s, "--------------------------------------------------------\n");
			}
			snprintf(in_out_str, sizeof(in_out_str) - 1, "(%d_%d)",
				 ((p_vg_info_item->bindfd & 0xff00) >> 8) - 1, (p_vg_info_item->bindfd & 0xff) - 1);
			seq_printf(s, "%-#10x%-9s  %-18s  %-#10x  %-3d\n", p_vg_info_item->bindfd, in_out_str,
				   p_vg_info_item->name, p_vg_info_item->fd, p_vg_info_item->vch);
			is_first = 0;
		}
	}

	is_first = 1;
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->graph_type == TYPE_AUDIO) {
			if (is_first) {
				seq_printf(s, "\nAudio:\n");
				seq_printf(s, "hdal_path(IN_OUT)    name                fd          vch  \n");
				seq_printf(s, "--------------------------------------------------------\n");
			}
			snprintf(in_out_str, sizeof(in_out_str) - 1, "(%d_%d)",
				 ((p_vg_info_item->bindfd & 0xff00) >> 8) - 1, (p_vg_info_item->bindfd & 0xff) - 1);
			seq_printf(s, "%-#10x%-9s  %-18s  %-#10x  %-3d\n", p_vg_info_item->bindfd, in_out_str,
				   p_vg_info_item->name, p_vg_info_item->fd, p_vg_info_item->vch);
			is_first = 0;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);
	return 0;
}

static ssize_t proc_entity_by_name_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_name[INFO_MAX_NAME_LEN + 1] = {0}; //+1 for NULL character
	int is_first = 1, name_len;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;
	char in_out_str[16];

	//count includes LF characters, -1 to get the real name length
	name_len = count - 1;

	if (name_len >= sizeof(ker_name)) {
		printk("ERR: count %zd >= max %zd\n", count, sizeof(ker_name));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_name, buffer, name_len)) {
		return -EINVAL;
	}
	ker_name[name_len] = '\0';

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (strncmp(p_vg_info_item->name, ker_name, name_len) == 0) {
			if (is_first) {
				printk("hdal_path(IN_OUT)    name                fd          vch  \n");
				is_first = 0;
			}
			snprintf(in_out_str, sizeof(in_out_str) - 1, "(%d_%d)",
				 ((p_vg_info_item->bindfd & 0xff00) >> 8) - 1, (p_vg_info_item->bindfd & 0xff) - 1);
			printk("%-#10x%-9s  %-18s  %-#10x  %-3d\n", p_vg_info_item->bindfd, in_out_str,
			       p_vg_info_item->name, p_vg_info_item->fd, p_vg_info_item->vch);
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

	return count;
}

static ssize_t proc_entity_by_bindfd_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};
	int is_first = 1;
	int bindfd;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;
	char in_out_str[16];

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}

	if (sscanf(ker_buffer, "%x", &bindfd) != 1) {
		printk("Failed to get hexadecimal number.\n");
		goto exit;
	}

	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->bindfd == bindfd) {
			if (is_first) {
				printk("hdal_path(IN_OUT)    name                fd          vch  \n");
			}
			snprintf(in_out_str, sizeof(in_out_str) - 1, "(%d_%d)",
				 ((p_vg_info_item->bindfd & 0xff00) >> 8) - 1, (p_vg_info_item->bindfd & 0xff) - 1);
			printk("%-#10x%-9s  %-18s  %-#10x  %-3d\n", p_vg_info_item->bindfd, in_out_str,
			       p_vg_info_item->name, p_vg_info_item->fd, p_vg_info_item->vch);
			is_first = 0;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

exit:
	return count;
}

static ssize_t proc_entity_by_fd_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};

	int is_first = 1;
	int fd;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;
	char in_out_str[16];

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}

	if (sscanf(ker_buffer, "%x", &fd) != 1) {
		printk("Failed to get hexadecimal number.\n");
		goto exit;
	}

	printk("hdal_path(IN_OUT)    name                fd          vch  \n");
	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->fd == fd) {
			if (is_first) {
				printk("hdal_path(IN_OUT)    name                fd          vch  \n");
			}
			snprintf(in_out_str, sizeof(in_out_str) - 1, "(%d_%d)",
				 ((p_vg_info_item->bindfd & 0xff00) >> 8) - 1, (p_vg_info_item->bindfd & 0xff) - 1);
			printk("%-#10x%-9s  %-18s  %-#10x  %-3d\n", p_vg_info_item->bindfd, in_out_str,
			       p_vg_info_item->name, p_vg_info_item->fd, p_vg_info_item->vch);
			is_first = 0;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

exit:
	return count;
}

static ssize_t proc_entity_by_pch_write_mode(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char ker_buffer[64] = {0};

	int is_first = 1;
	int pch;
	vg_info_item_t *p_vg_info_item;
	unsigned long flags;
	char in_out_str[16];

	if (count > sizeof(ker_buffer)) {
		printk("count %zd > max %zd\n", count, sizeof(ker_buffer));
		return -EINVAL;
	}

	if (copy_from_user((void *)ker_buffer, buffer, count)) {
		return -EINVAL;
	}

	if (sscanf(ker_buffer, "%x", &pch) != 1) {
		printk("Failed to get hexadecimal number.\n");
		goto exit;
	}

	printk("hdal_path(IN_OUT)    name                fd          vch  \n");
	vk_spin_lock_irqsave(&vg_info_lock, flags);
	list_for_each_entry(p_vg_info_item, &vg_info_listhead, list) {
		if (p_vg_info_item->pch == pch) {
			if (is_first) {
				printk("hdal_path(IN_OUT)    name                fd          vch  \n");
			}
			snprintf(in_out_str, sizeof(in_out_str) - 1, "(%d_%d)",
				 ((p_vg_info_item->bindfd & 0xff00) >> 8) - 1, (p_vg_info_item->bindfd & 0xff) - 1);
			printk("%-#10x%-9s  %-18s  %-#10x  %-3d\n", p_vg_info_item->bindfd, in_out_str,
			       p_vg_info_item->name, p_vg_info_item->fd, p_vg_info_item->vch);
			is_first = 0;
		}
	}
	vk_spin_unlock_irqrestore(&vg_info_lock, flags);

exit:
	return count;
}

static int general_seq_show(struct seq_file *s, void *v)
{
	return 0;
}

static int bindfd_all_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_bindfd_all_seq_show, NULL);
}

static int general_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, general_seq_show, NULL);
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops bindfd_all_proc_ops = {
	.proc_open    = bindfd_all_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations bindfd_all_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = bindfd_all_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops entity_by_name_proc_ops = {
	.proc_open    = general_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_entity_by_name_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations entity_by_name_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = general_proc_open,
	.read    = seq_read,
	.write   = proc_entity_by_name_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops entity_by_bindfd_proc_ops = {
	.proc_open    = general_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_entity_by_bindfd_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations entity_by_bindfd_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = general_proc_open,
	.read    = seq_read,
	.write   = proc_entity_by_bindfd_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops entity_by_fd_proc_ops = {
	.proc_open    = general_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_entity_by_fd_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations entity_by_fd_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = general_proc_open,
	.read    = seq_read,
	.write   = proc_entity_by_fd_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops entity_by_pch_proc_ops = {
	.proc_open    = general_proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_entity_by_pch_write_mode,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations entity_by_pch_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = general_proc_open,
	.read    = seq_read,
	.write   = proc_entity_by_pch_write_mode,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif


int vg_info_init(void)
{
	int ret = -EIO;
	VOS_INIT_LIST_HEAD(&vg_info_listhead);
	// coverity[side_effect_free]: CID 134089, Linux kernel api
	vk_spin_lock_init(&vg_info_lock);

	proc_buf = kzalloc(PROC_BUF_LEN, GFP_KERNEL);
	if (proc_buf == 0) {
		INFO_PANIC("Error allocate proc_buf\n");
		goto exit;
	}

	bindfd_all_proc = proc_create("videograph/bindfd_all", 0, 0, &bindfd_all_proc_ops);
	if (bindfd_all_proc == NULL) {
		goto exit;
	}

	entity_by_name_proc = proc_create("videograph/debug/entity_by_name", 0, 0, &entity_by_name_proc_ops);
	if (entity_by_name_proc == NULL) {
		goto exit;
	}

	entity_by_bindfd_proc = proc_create("videograph/debug/entity_by_hdal_path", 0, 0, &entity_by_bindfd_proc_ops);
	if (entity_by_bindfd_proc == NULL) {
		goto exit;
	}

	entity_by_fd_proc = proc_create("videograph/debug/entity_by_fd", 0, 0, &entity_by_fd_proc_ops);
	if (entity_by_fd_proc == NULL) {
		goto exit;
	}

	entity_by_pch_proc = proc_create("videograph/debug/entity_by_pch", 0, 0, &entity_by_pch_proc_ops);
	if (entity_by_pch_proc == NULL) {
		goto exit;
	}
	ret = 0;

exit:
	return ret;

}

void vg_info_exit(void)
{
	if (bindfd_all_proc != 0) {
		remove_proc_entry("videograph/bindfd_all", NULL);
	}
	if (entity_by_name_proc != 0) {
		remove_proc_entry("videograph/debug/entity_by_name", NULL);
	}
	if (entity_by_bindfd_proc != 0) {
		remove_proc_entry("videograph/debug/entity_by_hdal_path", NULL);
	}
	if (entity_by_fd_proc != 0) {
		remove_proc_entry("videograph/debug/entity_by_fd", NULL);
	}
	if (entity_by_pch_proc != 0) {
		remove_proc_entry("videograph/debug/entity_by_pch", NULL);
	}
	if (proc_buf) {
		kfree(proc_buf);
	}

}

