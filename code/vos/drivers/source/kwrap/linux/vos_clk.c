/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include <linux/clk.h>

#include <kwrap/clk.h>
#include <kwrap/debug.h>

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
void vk_clk_put(struct vk_clk *vos_clk)
{
	struct clk *native_clk = (struct clk *)vos_clk;

	clk_put(native_clk);
}
EXPORT_SYMBOL(vk_clk_put);

struct vk_clk *vk_clk_get(struct vk_device *vos_dev, const char *id)
{
	struct device *native_dev = (struct device *)vos_dev;

	return (struct vk_clk *)clk_get(native_dev, id);
}
EXPORT_SYMBOL(vk_clk_get);

unsigned long vk_clk_get_rate(struct vk_clk *vos_clk)
{
	struct clk *native_clk = (struct clk *)vos_clk;

	return clk_get_rate(native_clk);
}
EXPORT_SYMBOL(vk_clk_get_rate);

int vk_clk_set_rate(struct vk_clk *vos_clk, unsigned long rate)
{
	struct clk *native_clk = (struct clk *)vos_clk;

	return clk_set_rate(native_clk, rate);
}
EXPORT_SYMBOL(vk_clk_set_rate);

long vk_clk_round_rate(struct vk_clk *vos_clk, unsigned long rate)
{
	struct clk *native_clk = (struct clk *)vos_clk;

	return clk_round_rate(native_clk, rate);
}
EXPORT_SYMBOL(vk_clk_round_rate);

struct vk_clk *vk_clk_get_parent(struct vk_clk *vos_clk)
{
	struct clk *native_clk = (struct clk *)vos_clk;

	return (struct vk_clk *)clk_get_parent(native_clk);
}
EXPORT_SYMBOL(vk_clk_get_parent);

int vk_clk_set_parent(struct vk_clk *vos_clk, struct vk_clk *vos_parent)
{
	struct clk *native_clk = (struct clk *)vos_clk;
	struct clk *native_parent = (struct clk *)vos_parent;

	return clk_set_parent(native_clk, native_parent);
}
EXPORT_SYMBOL(vk_clk_set_parent);

int vk_clk_prepare_enable(struct vk_clk *vos_clk)
{
	struct clk *native_clk = (struct clk *)vos_clk;

	return clk_prepare_enable(native_clk);
}
EXPORT_SYMBOL(vk_clk_prepare_enable);

void vk_clk_disable_unprepare(struct vk_clk *vos_clk)
{
	struct clk *native_clk = (struct clk *)vos_clk;

	clk_disable_unprepare(native_clk);
}
EXPORT_SYMBOL(vk_clk_disable_unprepare);