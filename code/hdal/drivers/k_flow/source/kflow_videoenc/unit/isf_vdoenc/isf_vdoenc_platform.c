#include "kflow_videoenc/isf_vdoenc_platform.h"
#include "kwrap/debug.h"
#include "kwrap/task.h"
#if defined(__LINUX)
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/uaccess.h>
#elif defined(__FREERTOS)
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#endif

void* vdoenc_alloc(int size)
{
#if defined(__LINUX)
	return vmalloc(size);
#elif defined(__FREERTOS)
	return malloc(size);
#endif
}

void vdoenc_free(void *buf)
{
#if defined(__LINUX)
	vfree(buf);
#elif defined(__FREERTOS)
	free(buf);
#endif
}

void vdoenc_delay_m_sec(UINT32 m_sec)
{
#if defined(__LINUX)
	msleep(m_sec);
#elif defined(__FREERTOS)
	vos_task_delay_ms(m_sec);
#endif
}

UINT32 vdoenc_read_dtsi_value(CHAR *node, CHAR *tag)
{
	UINT32 value = 0;
#if defined(__LINUX)
	struct device_node *dt_node;

	dt_node = of_find_node_by_path(node);
	if (!dt_node) {
		DBG_ERR("Failed to find node by path: %s.\r\n", node);
		return 0;
	}
	if (of_property_read_u32(dt_node, tag, &value)) {
		DBG_ERR("Failed to read %s\r\n", tag);
		return 0;
	}
#endif
	return value;
}

INT32 vdoenc_read_dtsi_array(CHAR *node, CHAR *tag, UINT32 *array, UINT32 num)
{
	INT32 ret = -1;
#if defined(__LINUX)
	struct device_node *of_node;

	of_node = of_find_node_by_path(node);

	if(of_node != NULL)
		ret = of_property_read_u32_array(of_node, tag, (UINT32 *)array, num);
	else
		ret = -1;
#endif
	return ret;
}

BOOL vdoenc_copy_from_user(void *p_dest, void *p_src, UINT32 size)
{
#if defined(__LINUX)
	// copy from user
	if (unlikely(copy_from_user(p_dest, p_src, size))) {
		return FALSE;
	}
#elif defined(__FREERTOS)
	memcpy(p_dest, p_src, size);
#endif
	return TRUE;
}