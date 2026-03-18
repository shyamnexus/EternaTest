#include "emu_arb.h"

AUTOTEST_RESULT emu_ddr_arb_auto(void)
{
	DMA_WRITEPROT_ATTR attrib = {0};

	attrib.mask.CPU_S = 1;
	attrib.level = DMA_WPLEL_UNWRITE;
	attrib.protect_mode = DMA_PROT_IN;
	attrib.protect_rgn_attr.en = 1;
	attrib.protect_rgn_attr.starting_addr = 0x04000000;
	attrib.protect_rgn_attr.size = 128;

	printk("%s: test protect phy addr 0x%lx, size 0x%x\r\n", __func__,
		attrib.protect_rgn_attr.starting_addr, attrib.protect_rgn_attr.size);
	arb_enable_wp(DDR_ARB_1, WPSET_0, &attrib);

	return AUTOTEST_RESULT_OK;
}
