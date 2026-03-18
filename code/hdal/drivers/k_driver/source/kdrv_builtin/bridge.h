/**
 * @file bridge.h
 * @brief type definition of KDRV API.
 */

#ifndef __BRIDGE_H__
#define __BRIDGE_H__

extern int kdrv_bridge_map(void);
extern int kdrv_bridge_unmap(void);
extern int kdrv_bridge_get_tag(unsigned int tag, unsigned int *p_val);
extern int kdrv_bridge_set_tag(unsigned int tag, unsigned int val);

extern int kdrv_fdt_map(void);
extern int kdrv_fdt_unmap(void);
extern void *kdrv_fdt_get_fdt(void);

#endif
