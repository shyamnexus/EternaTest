#include "sie_init_int.h"
#include <libfdt.h>
#include <fdtfast.h>
#if defined (__KERNEL__)
#include <linux/of.h>
#include <asm/io.h>
#else
#include <stdlib.h>
#include <plat/rtosfdt.h>
#include <compiler.h>
#endif

int sie_init_plat_get_nodeoffset(CHAR *node, int *nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -FDT_ERR_NOTFOUND;
	}

	*nodeoffset = fdt_path_offset(p_fdt, node);
	return 0;
}

/*
    get subnode list in #define SIE_BUILTIN_NODE_HEAD "/fastboot/sie"

    include dbg/sie1_ctrl/sie1_clk/sie1_reg/sie2_ctrl/sie2_clk/sie2_reg/ ...
*/
void sie_init_plat_get_nodeoffset_list(int nodeoffset, int *nodeoffset_list, int *nodeoffset_list2)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	int i, node_idx, child, len;
	BOOL find = 0;
	/* ctl */
	int sieid_sft = 3; 		// "sie"
	int subnode_sft = 5; 	// "sie1_", "sie2_", "sie3_", ...
	CHAR *subnode_name[SIE_INIT_NODE_NUM] = {"ctrl", "clk", "reg"};
	int subnode_len[SIE_INIT_NODE_NUM] = {4, 3, 3};
	/* dbg */
	CHAR *subnode_dbg_name[SIE_INIT_NODE_GLB_NUM] = {"dbg"};
	int subnode_dbg_len[SIE_INIT_NODE_GLB_NUM] = {3};

	if (p_fdt != NULL) {
		fdt_for_each_subnode(child, p_fdt, nodeoffset) {
			const char *name = fdt_get_name(p_fdt, child, &len);
			find = 0;

			if (name != NULL) {
				for (i = 0; i < SIE_MAX_ENG_NUM; i++) {
					if (name[sieid_sft] == (char)('1' + i)) {
						for (node_idx = 0; node_idx < SIE_INIT_NODE_NUM; node_idx++) {
							if (strncmp(name + subnode_sft, subnode_name[node_idx], subnode_len[node_idx]) == 0) {
								*(int *)(&nodeoffset_list[i * SIE_INIT_NODE_NUM] + node_idx) = child; // nodeoffset_list[i][node_idx] = child;
								find = 1;
								goto next_subnode;
							}
						}
					}
				}

				for (node_idx = 0; node_idx < SIE_INIT_NODE_GLB_NUM; node_idx++) {
					if (strncmp(name, subnode_dbg_name[node_idx], subnode_dbg_len[node_idx]) == 0) {
						*(int *)(&nodeoffset_list2[node_idx]) = child;
						find = 1;
						goto next_subnode;
					}
				}
			}

	next_subnode:
			if (find == 0 && name != NULL) {
				DBG_ERR("miss %s\r\n", name);
			} else {
	//			DBG_DUMP("[%s] find %s %d !!\r\n", __func__, name, child);
			}
		}
	} else {
		DBG_ERR("[%s] find base fail !!\r\n", __func__);
	}
}

INT32 sie_init_plat_read_dtsi_array(int nodeoffset, CHAR *tag, UINT32 *buf, UINT32 num)
{
	const void *nodep;  /* property node pointer */
	int len, i;
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return E_SYS;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, tag, &len);
	if (len == 0 || nodep == NULL) {
//		DBG_ERR("Failed to read %s\r\n", tag);
		return -1;
	}

	for (i = 0; i < len; i = i + 4) {
		*(UINT32 *)((unsigned long)buf + i) = be32_to_cpu(*(unsigned int *)((unsigned long)nodep + i));
	}

#if 0
	DBG_DUMP("[%s] find tag %s:", __func__, tag);
	for (i = 0; i < len; i = i + 4) {
		if (len < 10) {
			DBG_DUMP("%d,", buf[i]);
		} else {
			//DBG_DUMP("0x%.8x\r\n", buf[i]);
		}
	}
	DBG_DUMP("\r\n");
#endif

	return E_OK;
}

void *kdrv_sie_builtin_plat_malloc(UINT32 size)
{
	void *p_buf = NULL;

#if defined (__KERNEL__)
	p_buf = kzalloc(size, GFP_KERNEL);
#else
	p_buf = malloc(size);
#endif
	return p_buf;
}

void kdrv_sie_builtin_plat_free(void *p_buf)
{
#if defined (__KERNEL__)
	kfree(p_buf);
#else
	free(p_buf);
#endif
}

void *kdrv_sie_builtin_ioremap(ULONG addr, UINT32 size)
{
#if defined (__KERNEL__)
	return ioremap(addr, size);
#else
	return (void *)addr;
#endif
}
