#ifndef _VOS_CLK_H_
#define _VOS_CLK_H_
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------*/
/* Including Files                                                                                                                 */
/*-----------------------------------------------------------------------------*/
#include <kwrap/nvt_type.h>

struct vk_clk;
struct vk_device;

void vk_clk_put(struct vk_clk *vos_clk);
struct vk_clk *vk_clk_get(struct vk_device *vos_dev, const char *id);

unsigned long vk_clk_get_rate(struct vk_clk *vos_clk);
int vk_clk_set_rate(struct vk_clk *vos_clk, unsigned long rate);
long vk_clk_round_rate(struct vk_clk *vos_clk, unsigned long rate);

struct vk_clk *vk_clk_get_parent(struct vk_clk *vos_clk);
int vk_clk_set_parent(struct vk_clk *vos_clk, struct vk_clk *vos_parent);

int vk_clk_prepare_enable(struct vk_clk *vos_clk);
void vk_clk_disable_unprepare(struct vk_clk *vos_clk);

#ifdef __cplusplus
}
#endif

#endif /* _VOS_CLK_H_ */

