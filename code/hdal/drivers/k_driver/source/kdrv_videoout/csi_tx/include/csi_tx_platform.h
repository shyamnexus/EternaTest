#ifndef _CSI_TX_PLATFORM_H_
#define _CSI_TX_PLATFORM_H_


extern void csi_tx_platform_clk_set_freq(unsigned long p_clk);
extern unsigned long csi_tx_platform_clk_get_freq(void);
extern unsigned long csi_tx_platform_clk_get_lp_freq(void);

extern void csi_tx2_platform_clk_set_freq(unsigned long p_clk);
extern unsigned long csi_tx2_platform_clk_get_freq(void);
extern unsigned long csi_tx2_platform_clk_get_lp_freq(void);

#endif
