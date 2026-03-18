#include<linux/module.h>
#include "comm/ddr_arb.h"
#include "ddr_arb_int.h"

EXPORT_SYMBOL(arb_init);
EXPORT_SYMBOL(arb_set_priority);
EXPORT_SYMBOL(arb_enable_wp);
EXPORT_SYMBOL(arb_disable_wp);
EXPORT_SYMBOL(arb_get_wp);
EXPORT_SYMBOL(dma_set_channel_outstanding);
EXPORT_SYMBOL(dma_get_channel_outstanding);
EXPORT_SYMBOL(arb_chksum);

EXPORT_SYMBOL(axi_channel_en_dis);
EXPORT_SYMBOL(mau_ch_mon_start);
EXPORT_SYMBOL(mau_ch_mon_stop);

EXPORT_SYMBOL(dma_enable_monitor_eff);
EXPORT_SYMBOL(dma_disable_monitor_eff);

EXPORT_SYMBOL(dma_reset_data_count);
EXPORT_SYMBOL(dma_get_data_count);

