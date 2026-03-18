#if defined(__KERNEL__)
#include<linux/module.h>
#include "dsiphy.h"


EXPORT_SYMBOL(dsiphy_enable_config);
EXPORT_SYMBOL(dsiphy_set_enable);
EXPORT_SYMBOL(dsiphy_set_phase);
EXPORT_SYMBOL(dsiphy_open);
EXPORT_SYMBOL(dsiphy_set_config);
EXPORT_SYMBOL(dsiphy_get_config);
EXPORT_SYMBOL(dsiphy_check_boostrap);

#endif

