/*
 *   @file   info.h
 *
 *   @brief  util info header.
 *
 *   Here defines some query APIs about fd/bindfd/vch/pch conversion.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef _INFO_H_
#define _INFO_H_
#include <linux/version.h>
#include "kwrap/list.h"

#define VG_INFO_STRUCT_MAGIC    0x321C361
#define INFO_MAX_NAME_LEN       31

typedef struct vg_info_item_tag {
	struct vos_list_head    list;
	int     fd;
	int     graph_type;
	int     bindfd;
	char    name[INFO_MAX_NAME_LEN + 1];
	int     vch;
	int     pch;
	unsigned long   struct_magic;
} vg_info_item_t;

#define TYPE_DISPLAY    0
#define TYPE_ENCODE     1
#define TYPE_AUDIO      2
#define TYPE_IV         3


//++ Foster 636 TODO
#if (HZ == 1000)
#define get_nvt_jiffies()            (jiffies)
#define nvt_jiffies_to_msecs(time)   jiffies_to_msecs(time)
#define nvt_jiffies_to_usecs(time)   jiffies_to_usecs(time)
#define nvt_msecs_to_jiffies(time)   msecs_to_jiffies(time)
#else
#include <plat/nvt_jiffies.h>
#define nvt_jiffies_to_msecs(time)   (time)
#define nvt_jiffies_to_usecs(time)   (time * 1000)
#define nvt_msecs_to_jiffies(time)   (time)
#endif

int util_del_vg_info_item(vg_info_item_t *p_vg_info_item);
int util_del_info_by_fd(int fd);
int util_find_vg_info_item_by_fd(int fd);
int util_get_fd_by_name(char *name);
int util_get_graph_type_by_name(char *name);
int util_get_bindfd_by_name(char *name);
int util_get_vch_by_name(char *name);
int util_get_pch_by_name(char *name);
int util_get_graph_type_by_fd(int fd);
int util_get_bindfd_by_fd(int fd);
int util_get_vch_by_fd(int fd);
int util_get_pch_by_fd(int fd);
char *util_get_name_by_fd(int fd);
char *util_get_name_by_bindfd(int bindfd);
char *util_get_name_by_vch(int vch);
char *util_get_name_by_pch(int pch);
int util_set_graph_type_by_fd(int fd, int graph_type);
int util_set_bindfd_by_fd(int fd, int bindfd);
int util_set_name_by_fd(int fd, char *name);
int util_set_vch_by_fd(int fd, int vch);
int util_set_pch_by_fd(int fd, int pch);
int util_is_file_exist(char *full_pathname);
unsigned int util_get_fd_by_bindfd(int bindfd, int type);

#endif /* _INFO_H_ */
