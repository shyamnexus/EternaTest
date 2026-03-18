#include "vdoenc_builtin_platform.h"
#include "vdoenc_builtin.h"
#if defined(__LINUX)
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#elif defined(__FREERTOS)
#include <malloc.h>
#include <stdlib.h>
#include <plat/rtosfdt.h>
#include <compiler.h>
#endif
#include <libfdt.h>
#include <fdtfast.h>

#if defined(__LINUX)
void* vdoenc_builtin_alloc(int size)
{
	return vmalloc(size);
}

void vdoenc_builtin_free(void *buf)
{
	vfree(buf);
}

ULONG vdoenc_builtin_ioremap(ULONG addr, UINT32 size)
{
	return (ULONG)ioremap_cache((ULONG)addr, size);
}

void vdoenc_builtin_iounmap(void *addr)
{
	iounmap(addr);
}
#elif defined(__FREERTOS)
void* vdoenc_builtin_alloc(int size)
{
	return malloc(size);
}

void vdoenc_builtin_free(void *buf)
{
	free(buf);
}
#endif

int vdoenc_plat_get_nodeoffset(CHAR *node, int *nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}

	*nodeoffset = fdt_path_offset(p_fdt, node);
	return 0;
}

/*
    get subnode list in #define VENC_BUILTIN_NODE_HEAD "/fastboot/venc"

    /venc0/venc1/venc2/venc3/venc4/venc5/
*/
void vdoenc_plat_get_nodeoffset_list(int nodeoffset, int *nodeoffset_list)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	int node_idx, child, len;
	BOOL find = 0;
	CHAR *subnode_name[BUILTIN_VDOENC_PATH_ID_MAX] = {"venc0", "venc1", "venc2", "venc3", "venc4", "venc5"};

	if(p_fdt == NULL) {
		DBG_ERR("vdoenc_plat_get_nodeoffset_list p_fdt is NULL\r\n");
		return;
	}

	fdt_for_each_subnode(child, p_fdt, nodeoffset) {
		const char *name = fdt_get_name(p_fdt, child, &len);
		if(name == NULL) {
			DBG_ERR("vdoenc_plat_get_nodeoffset_list name is NULL\r\n");
			return;
		}
		find = 0;

		for (node_idx = 0; node_idx < BUILTIN_VDOENC_PATH_ID_MAX; node_idx++) {
			if (strncmp(name, subnode_name[node_idx], 5) == 0) {
				*(int *)(&nodeoffset_list[node_idx]) = child;
				find = 1;
				goto next_subnode;
			}
		}

next_subnode:
		if (find == 0) {
			DBG_ERR("miss %s\r\n", name);
		} else {
	//		DBG_DUMP("[%s] find %s %d !!\r\n", __func__, name, child);
		}
	}
}

INT32 vdoenc_plat_read_dtsi(int nodeoffset, CHAR *tag, UINT32 *value)
{

	const void *nodep;  /* property node pointer */
	int len;
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, tag, &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", tag);
		return -1;
	}
	*value = be32_to_cpu(*(unsigned int *)nodep);

	return 0;

}

