#ifndef _CSI_TX_PLATFORM_H_
#define _CSI_TX_PLATFORM_H_


extern void dsiphy_platform_clk_set_freq(unsigned long p_clk);
extern unsigned long dsiphy_platform_clk_get_freq(void);
//extern unsigned long dsiphy_platform_clk_get_lp_freq(void);

extern void dsiphy2_platform_clk_set_freq(unsigned long p_clk);
extern unsigned long dsiphy2_platform_clk_get_freq(void);
//extern unsigned long dsiphy_platform_clk_get_lp_freq(void);

#endif
