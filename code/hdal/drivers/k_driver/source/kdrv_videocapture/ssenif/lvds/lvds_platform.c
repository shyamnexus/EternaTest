#include <kwrap/type.h>
#include <kwrap/util.h>
#include "lvds_platform.h"
#include "lvds_int.h"

#ifdef __KERNEL__
#include "lvds_drv.h"
extern PLVDS_MODULE_INFO plvds_mod_info;

static struct completion lvds_completion;
static struct completion lvds2_completion;
static struct completion lvds3_completion;
static struct completion lvds4_completion;
//static struct completion lvds5_completion;
#else

#include <kwrap/debug.h>
#endif

void lvds_platform_clk_enable(LVDS_ID id)
{
#ifdef __KERNEL__
	DBG_IND("%s() id: %d\r\n", __func__, id);
	clk_enable(plvds_mod_info->pclk[id]);
#endif
}

void lvds_platform_clk_disable(LVDS_ID id)
{
#ifdef __KERNEL__
	DBG_IND("%s() id: %d\r\n", __func__, id);
	clk_disable(plvds_mod_info->pclk[id]);
#endif
}

#ifdef __KERNEL__
void lvds_platform_init_completion(LVDS_ID id)
{
	if (id == LVDS_ID_LVDS)
		init_completion(&lvds_completion);
	else if (id == LVDS_ID_LVDS2)
		init_completion(&lvds2_completion);
	else if (id == LVDS_ID_LVDS3)
		init_completion(&lvds3_completion);
	else // if (id == LVDS_ID_LVDS4)
		init_completion(&lvds4_completion);
}

void lvds_platform_reinit_completion(LVDS_ID id)
{
	if (id == LVDS_ID_LVDS)
		reinit_completion(&lvds_completion);
	else if (id == LVDS_ID_LVDS2)
		reinit_completion(&lvds2_completion);
	else if (id == LVDS_ID_LVDS3)
		reinit_completion(&lvds3_completion);
	else // if (id == LVDS_ID_LVDS4)
		reinit_completion(&lvds4_completion);
}

void lvds_platform_wait_completion(LVDS_ID id)
{
	if (id == LVDS_ID_LVDS)
		wait_for_completion(&lvds_completion);
	else if (id == LVDS_ID_LVDS2)
		wait_for_completion(&lvds2_completion);
	else if (id == LVDS_ID_LVDS3)
		wait_for_completion(&lvds3_completion);
	else // if (id == LVDS_ID_LVDS4)
		wait_for_completion(&lvds4_completion);
}

unsigned long lvds_platform_wait_completion_timeout(LVDS_ID id, unsigned long timeout)
{
	if (id == LVDS_ID_LVDS)
		return wait_for_completion_timeout(&lvds_completion, msecs_to_jiffies(timeout));
	else if (id == LVDS_ID_LVDS2)
		return wait_for_completion_timeout(&lvds2_completion, msecs_to_jiffies(timeout));
	else if (id == LVDS_ID_LVDS3)
		return wait_for_completion_timeout(&lvds3_completion, msecs_to_jiffies(timeout));
	else // if (id == LVDS_ID_LVDS4)
		return wait_for_completion_timeout(&lvds4_completion, msecs_to_jiffies(timeout));
}

void lvds_platform_complete(LVDS_ID id)
{
	if (id == LVDS_ID_LVDS)
		complete(&lvds_completion);
	else if (id == LVDS_ID_LVDS2)
		complete(&lvds2_completion);
	else if (id == LVDS_ID_LVDS3)
		complete(&lvds3_completion);
	else // if (id == LVDS_ID_LVDS4)
		complete(&lvds4_completion);
}
#endif
