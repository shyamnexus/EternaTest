#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <kwrap/cpu.h>
#include <kwrap/debug.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <kdrv_builtin/fdtfast.h>
#include "bridge_fourcc.h"
#include "bridge_mem.h"

// bridge memory description
#define BRIDGE_FOURCC 0x47445242 ///< MAKEFOURCC('B', 'R', 'D', 'G');
#define BRIDGE_MAX_OPT_CNT 128

// bridge memory description !! (DO NOT MODIFY ANY MEMBER IN BRIDGE_DESC and BRIDGE_BOOT_OPTION)
typedef struct _BRIDGE_BOOT_OPTION {
	unsigned int tag; //a fourcc tag
	unsigned int val; //the value
} BRIDGE_BOOT_OPTION;

typedef struct _BRIDGE_DESC {
	unsigned int bridge_fourcc;     ///< always BRIDGE_FOURCC
	unsigned int bridge_size;       ///< sizeof(BRIDGE_DESC) for check if struct match on rtos and linux
	unsigned long phy_addr;          ///< address of bridge memory described on fdt
	unsigned int phy_size;          ///< size of whole bridge memory described on fdt
	BRIDGE_BOOT_OPTION opts[BRIDGE_MAX_OPT_CNT]; ///< boot options from rtos
} BRIDGE_DESC;

static BRIDGE_DESC *mp_bridge = NULL;

int bridge_mem_init(void)
{
	// plan the bridge memory
	unsigned bridge_addr = 0;
	unsigned bridge_size = 0;
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	// read from fdt
	int len, nodeoffset;
	const void *nodep;
	const char path[] = "/nvt_memory_cfg/bridge";
	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", path, nodeoffset);
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return -1;
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		bridge_addr = be32_to_cpu(p_data[0]);
		bridge_size = be32_to_cpu(p_data[1]);
	}

	// start to plan
	mp_bridge = (BRIDGE_DESC *)(uintptr_t)bridge_addr;
	memset(mp_bridge, 0, sizeof(BRIDGE_DESC));
	mp_bridge->bridge_fourcc = BRIDGE_FOURCC;
	mp_bridge->bridge_size = sizeof(BRIDGE_DESC);
	mp_bridge->phy_addr = vos_cpu_get_phy_addr((VOS_ADDR) bridge_addr);
	mp_bridge->phy_size = bridge_size;

	return 0;
}

int bridge_mem_add_tag(unsigned int tag, unsigned int val)
{
	int i;
	for (i = 0; i < BRIDGE_MAX_OPT_CNT; i++) {
		if (mp_bridge->opts[i].tag == 0) {
			mp_bridge->opts[i].tag = tag;
			mp_bridge->opts[i].val = val;
			return 0;
		}
		if (mp_bridge->opts[i].tag == tag) {
			DBG_ERR("tag: 0x%08X has been registered.\n", tag);
			return -1;
		}
	}

	DBG_ERR("BRIDGE_MAX_OPT_CNT is not enough.\n");
	return -2;
}

int bridge_mem_get_tag(unsigned int tag, unsigned int *p_val)
{
	int i;
	for (i = 0; i < BRIDGE_MAX_OPT_CNT; i++) {
		if (mp_bridge->opts[i].tag == tag) {
			*p_val = mp_bridge->opts[i].val;
			return 0;
		}
	}

	DBG_ERR("unable to find tag: 0x%08X\n", tag);
	return -1;
}

int bridge_mem_modify_tag(unsigned int tag, unsigned int val)
{
	int i;
	for (i = 0; i < BRIDGE_MAX_OPT_CNT; i++) {
		if (mp_bridge->opts[i].tag == tag) {
			mp_bridge->opts[i].val = val;
			return 0;
		}
	}

	DBG_ERR("unable to find tag: 0x%08X\n", tag);
	return -1;
}
