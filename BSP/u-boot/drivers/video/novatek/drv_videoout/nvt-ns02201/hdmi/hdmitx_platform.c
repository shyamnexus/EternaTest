#include "include/hdmitx_int.h"
#ifdef __KERNEL__
#include <linux/clk.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include "hdmitx_drv.h"
#endif


#ifdef __KERNEL__
static char* hdmitx_clk_name[PLATFORM_MAX_CHIP_CNT][MODULE_REG_NUM]= {
	{"hdmi_vdo_clk", "hdmi2_vdo_clk"},
};
static char* hdmitx_ado_clk_name[PLATFORM_MAX_CHIP_CNT][MODULE_REG_NUM]= {
	{"hdmi_ado_clk", "hdmi2_ado_clk"},
};
#endif




// enable clk and pinmux
void _hdmitx_enableclk_platform(int id)
{
#ifdef __KERNEL__
	struct clk *hdmitx_clk;

	hdmitx_clk = clk_get(NULL, hdmitx_clk_name[id][0]);
	
	if (IS_ERR(hdmitx_clk)) {
		printk("failed to get hdmitx clk when enable clk\n");
	}
	clk_prepare(hdmitx_clk);
	clk_enable(hdmitx_clk);
	clk_put(hdmitx_clk);

	hdmitx_clk = clk_get(NULL, hdmitx_ado_clk_name[id][0]);

	if (IS_ERR(hdmitx_clk)) {
		printk("failed to get hdmitx ado clk when enable clk\n");
	}
	clk_prepare(hdmitx_clk);
	clk_enable(hdmitx_clk);
	clk_put(hdmitx_clk);	
#endif		
}
void _hdmitx2_enableclk_platform(int id)
{
#ifdef __KERNEL__
	struct clk *hdmitx_clk;

	hdmitx_clk = clk_get(NULL, hdmitx_clk_name[id][1]);

	if (IS_ERR(hdmitx_clk)) {
		printk("failed to get hdmitx2 clk when enable clk\n");
	}
	clk_prepare(hdmitx_clk);
	clk_enable(hdmitx_clk);
	clk_put(hdmitx_clk);

	hdmitx_clk = clk_get(NULL, hdmitx_ado_clk_name[id][1]);

	if (IS_ERR(hdmitx_clk)) {
		printk("failed to get hdmitx2 ado clk when enable clk\n");
	}
	clk_prepare(hdmitx_clk);
	clk_enable(hdmitx_clk);
	clk_put(hdmitx_clk);	
#endif	
	
}

void _hdmitx_disableclk_platform(int id)
{
#ifdef __KERNEL__
	struct clk *hdmitx_clk;
	
	hdmitx_clk = clk_get(NULL, hdmitx_clk_name[id][0]);
	
	if (IS_ERR(hdmitx_clk)) {
		printk("failed to get hdmitx clk when disable clk\n");
	}
	clk_prepare(hdmitx_clk);
	clk_disable(hdmitx_clk);
	clk_put(hdmitx_clk);

	hdmitx_clk = clk_get(NULL, hdmitx_ado_clk_name[id][0]);
	
	if (IS_ERR(hdmitx_clk)) {
		printk("failed to get hdmitx ado clk when disable clk\n");
	}
	clk_prepare(hdmitx_clk);
	clk_disable(hdmitx_clk);
	clk_put(hdmitx_clk);	
#endif	

}
void _hdmitx2_disableclk_platform(int id)
{
#ifdef __KERNEL__
	struct clk *hdmitx_clk;
		
	hdmitx_clk = clk_get(NULL, hdmitx_clk_name[id][1]);
			
	if (IS_ERR(hdmitx_clk)) {
		printk("failed to get hdmitx clk when disable clk\n");
	}
	clk_prepare(hdmitx_clk);
	clk_disable(hdmitx_clk);
	clk_put(hdmitx_clk);

	hdmitx_clk = clk_get(NULL, hdmitx_ado_clk_name[id][1]);
			
	if (IS_ERR(hdmitx_clk)) {
		printk("failed to get hdmitx ado clk when disable clk\n");
	}
	clk_prepare(hdmitx_clk);
	clk_disable(hdmitx_clk);
	clk_put(hdmitx_clk);	
#endif	

}


