#include "kwrap/error_no.h"
#include "kwrap/cpu.h"
#include "kdrv_ipp_int_dbg.h"
#include "kdrv_ipp_int_util.h"
#if defined(__LINUX)
#include <linux/of.h>
#elif defined(__FREERTOS)
#include "comm/compiler.h"
#include "comm/libfdt.h"
#include <plat/rtosfdt.h>
// for vsnprintf defined
#include "plat/top.h"
#endif

UINT32 kdrv_ipp_util_get_dtsi_clock(CHAR eng_name[8])
{
#define KDRV_IPP_UTIL_CLOCK_DFT	(450000000)

#if defined(__LINUX)

	struct device_node *from = NULL;
	struct device_node* of_node = of_find_node_by_name(from, eng_name);
	UINT32 clk = KDRV_IPP_UTIL_CLOCK_DFT;

	//DBG_DUMP("ENG clock name: %s\r\n", eng_name);

	if (of_node) {
		if (of_property_read_u32(of_node, "current_rate", &clk) != 0) {
			DBG_WRN("cannot find /%s/current_rate\r\n", eng_name);
		}
	} else {
		DBG_WRN("cannot find node %s\r\n", eng_name);
		clk = KDRV_IPP_UTIL_CLOCK_DFT;
	}

	/* return clk unit is MHz */
	return (clk / 1000000);

#elif defined(__FREERTOS)
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */
	UINT32 clk = KDRV_IPP_UTIL_CLOCK_DFT;

	// get linux space
	if (p_fdt != NULL) {
		nodeoffset = fdt_path_offset(p_fdt, "/kdrv_ipp");
		if (nodeoffset < 0) {
			DBG_WRN("failed to offset for  %s = %d \r\n", "/kdrv_ipp", nodeoffset);
		} else {
			nodep = fdt_getprop(p_fdt, nodeoffset, "clock-frequency", &len);
			if (len == 0 || nodep == NULL) {
				DBG_WRN("failed to access clock-frequency.\r\n");
			} else {
				clk = be32_to_cpu(*(unsigned int *)nodep);
			}
		}
	} else {
		DBG_WRN("null fdt_base\r\n");
	}

	/* return clk unit is MHz */
	return (clk / 1000000);
#endif
}

INT32 kdrv_ipp_int_printf(const char *fmtstr, ...)
{
	char    buf[512];
	int     len;

	va_list marker;

	va_start(marker, fmtstr);

	len = vsnprintf(buf, sizeof(buf), fmtstr, marker);
	va_end(marker);
	if (len > 0) {
		DBG_DUMP("%s", buf);
	}

	return 0;
}