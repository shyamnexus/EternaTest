#include "include/dai_int.h"

#ifdef __KERNEL__
#include <linux/clk.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
//#include "kdrv_audioio_drv_dai2.h"
#else
#include "kwrap/type.h"

#endif


#ifdef __KERNEL__
void __iomem * dai_reg_base[PLATFORM_MAX_CHIP_CNT][MODULE_REG_NUM];

static char* dai_clk_name[PLATFORM_MAX_CHIP_CNT][MODULE_REG_NUM]= {
	{"clk_dai"},
};
/*
static char* dai_clk_name_standard_pcie[PLATFORM_MAX_CHIP_CNT][MODULE_REG_NUM]= {
	{"2f0151000.dai"}	
};*/
//void __iomem * daiep_reg_base[MODULE_REG_NUM];
#endif



UINT64 dai_platform_va2pa(uintptr_t addr)
{
#ifdef __KERNEL__
	UINT64 paddr;
	paddr = (UINT64)addr;//fmem_lookup_pa(addr);
	return paddr;
#else
	SYS_PA pa;

	pa = sys_va2pa(addr);
	return pa.addr64;
#endif
}
#if 0
UINT64 dai_platform_pcie_addr(int loc_chipid, int tar_ddrid, uintptr_t addr)
{
#ifdef __KERNEL__

	phys_addr_t pcie_addr;

	pcie_addr = nvtpcie_get_pcie_addr((nvtpcie_chipid_t)loc_chipid, (nvtpcie_ddrid_t)tar_ddrid, (phys_addr_t)addr);
	if (pcie_addr == NVTPCIE_INVALID_PA)
		return NVTPCIE_INVALID_PA;

	return (uintptr_t)pcie_addr;
#else
	SYS_PA pa;

	pa = sys_va2pa(addr);
	return pa.addr64;

#endif
}
#endif
#ifdef __KERNEL__


// enable clk and pinmux
void _dai_enableclk_platform(int id)
{
#ifdef __KERNEL__
	struct clk *dai_clk;

	//if(nvtpcie_is_common_pci() == 0){
		dai_clk = clk_get(NULL, dai_clk_name[id][0]);
	/*} else {
		dai_clk = clk_get(NULL, dai_clk_name_standard_pcie[id][0]);
	}*/
	
	if (IS_ERR(dai_clk)) {
		printk("failed to get dai clk\n");
	}
	clk_prepare(dai_clk);
	clk_enable(dai_clk);
	clk_put(dai_clk);
#endif		
}

void _dai_disableclk_platform(int id)
{
}

void _dai_setclkrate_platform(int id, unsigned long clkrate)
{
#ifdef __KERNEL__
		struct clk *dai_clk;

		//if(nvtpcie_is_common_pci() == 0){
			dai_clk = clk_get(NULL, dai_clk_name[id][0]);
		/*} else {
			dai_clk = clk_get(NULL, dai_clk_name_standard_pcie[id][0]);
		}*/
		
		if (IS_ERR(dai_clk)) {
			printk("failed to get dai clk\n");
		}
		clk_set_rate(dai_clk, clkrate);
		clk_put(dai_clk);
#endif	

}

#endif