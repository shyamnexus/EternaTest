#if defined (__KERNEL__)
#include <linux/of.h>
#include <linux/io.h>       // for ioremap
#include <linux/of_device.h>
#else
#include <stdlib.h>
//#include <libfdt.h>
#include <plat/rtosfdt.h>
#include <compiler.h>
#endif
#include <libfdt.h>
#include <fdtfast.h>
#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kwrap/task.h"
#include "kwrap/debug.h"
#include "nvtmpp_init.h"


void *nvtmpp_ioremap_cache(unsigned long offset, unsigned long size)
{
	#if defined(__KERNEL__)
	void *va;
	va = ioremap_cache(offset, size);
	DBG_IND("nvtmpp_ioremap_cache addr=0x%x, size = 0x%x\r\n", (int)va, (int) size);
	return va;
	#else
	return (void *)offset;
	#endif
}

void nvtmpp_iounmap(void *addr)
{
	#if defined(__KERNEL__)
	DBG_IND("nvtmpp_iounmap addr=0x%x\r\n", (int)addr);
	return iounmap(addr);
	#endif
}

#if defined (__KERNEL__)

INT32 nvtmpp_get_dtsi_node(CHAR *path, NVTMPP_DTSI_NODE_INFO *node)
{
	struct device_node *of_node;

	of_node = of_find_node_by_path(path);
	if (!of_node) {
		return E_SYS;
    }
	node->of_node = of_node;
	return E_OK;
}

INT32 nvtmpp_read_dtsi_array(NVTMPP_DTSI_NODE_INFO *node, const CHAR *tag, UINT32 *buf, UINT32 num)
{
	if (of_property_read_u32_array(node->of_node, tag, (UINT32 *)buf, num)) {
		return E_SYS;
	}
	return E_OK;
}

INT32 nvtmpp_read_dtsi_array_u64(NVTMPP_DTSI_NODE_INFO *node, const CHAR *tag, UINT64 *buf, UINT32 num)
{
	if (of_property_read_u64_array(node->of_node, tag, buf, num)) {
		return E_SYS;
	}
	return E_OK;
}

#else

INT32 nvtmpp_get_dtsi_node(CHAR *path, NVTMPP_DTSI_NODE_INFO *node)
{
	int nodeoffset;
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt== NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return E_SYS;
	}

	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		//DBG_DUMP("failed to offset for %s = %d \n", path, nodeoffset);
		return E_SYS;
	}
	node->p_fdt = p_fdt;
	node->nodeoffset = nodeoffset;
	return E_OK;
}

INT32 nvtmpp_read_dtsi_array(NVTMPP_DTSI_NODE_INFO *node, const CHAR *tag, UINT32 *buf, UINT32 num)
{
	const void *nodep;  /* property node pointer */
	int i, len;

	nodep = fdt_getprop(node->p_fdt, node->nodeoffset, tag, &len);
	if (len == 0 || nodep == NULL) {
		//DBG_ERR("len = %d, nodep = 0x%x\r\n", len, nodep);
		return E_SYS;
	}
	if (len != num * 4) {
		//DBG_ERR("len = %d, num*4 = %d\r\n", len, num * 4);
		return E_SYS;
	}
	for (i = 0; i < num; i++) {
		*(buf + i) = be32_to_cpu(*((unsigned int *)nodep + i));
	}
	return E_OK;
}

INT32 nvtmpp_read_dtsi_array_u64(NVTMPP_DTSI_NODE_INFO *node, const CHAR *tag, UINT64 *buf, UINT32 num)
{
	const void *nodep;  /* property node pointer */
	int i, len;

	nodep = fdt_getprop(node->p_fdt, node->nodeoffset, tag, &len);
	if (len == 0 || nodep == NULL) {
		//DBG_ERR("len = %d, nodep = 0x%x\r\n", len, nodep);
		return E_SYS;
	}
	if (len != num * 8) {
		//DBG_ERR("len = %d, num*8 = %d\r\n", len, num * 8);
		return E_SYS;
	}
	for (i = 0; i < num; i++) {
		*(buf + i) = be64_to_cpu(*((UINT64 *)nodep + i));
		//DBG_ERR("0x%lx\r\n", *(buf + i));
	}
	return E_OK;
}
#endif

INT32 nvtmpp_get_fastboot_dtsi_node(CHAR *path, NVTMPP_FASTBOOT_DTSI_NODE_INFO *node)
{
	int nodeoffset;
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();

	if (p_fdt== NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return E_SYS;
	}

	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		//DBG_DUMP("failed to offset for %s = %d \n", path, nodeoffset);
		return E_SYS;
	}
	node->p_fdt = p_fdt;
	node->nodeoffset = nodeoffset;
	return E_OK;
}

INT32 nvtmpp_read_fastboot_dtsi_array(NVTMPP_FASTBOOT_DTSI_NODE_INFO *node, const CHAR *tag, UINT32 *buf, UINT32 num)
{
	const void *nodep;  /* property node pointer */
	int i, len;

	nodep = fdt_getprop(node->p_fdt, node->nodeoffset, tag, &len);
	if (len == 0 || nodep == NULL) {
		//DBG_ERR("len = %d, nodep = 0x%x\r\n", len, nodep);
		return E_SYS;
	}
	if (len != num * 4) {
		//DBG_ERR("len = %d, num*4 = %d\r\n", len, num * 4);
		return E_SYS;
	}
	for (i = 0; i < num; i++) {
		*(buf + i) = be32_to_cpu(*((unsigned int *)nodep + i));
	}
	return E_OK;
}

INT32 nvtmpp_read_fastboot_dtsi_array_u64(NVTMPP_FASTBOOT_DTSI_NODE_INFO *node, const CHAR *tag, UINT64 *buf, UINT32 num)
{
	const void *nodep;  /* property node pointer */
	int i, len;

	nodep = fdt_getprop(node->p_fdt, node->nodeoffset, tag, &len);
	if (len == 0 || nodep == NULL) {
		//DBG_ERR("len = %d, nodep = 0x%x\r\n", len, nodep);
		return E_SYS;
	}
	if (len != num * 8) {
		//DBG_ERR("len = %d, num*8 = %d\r\n", len, num * 8);
		return E_SYS;
	}
	for (i = 0; i < num; i++) {
		*(buf + i) = be64_to_cpu(*((UINT64 *)nodep + i));
		//DBG_ERR("0x%lx\r\n", *(buf + i));
	}
	return E_OK;
}

#if defined(__KERNEL__)
EXPORT_SYMBOL(nvtmpp_ioremap_cache);
EXPORT_SYMBOL(nvtmpp_iounmap);
#endif

