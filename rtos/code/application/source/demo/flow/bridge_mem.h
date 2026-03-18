#ifndef _BRIDGE_MEM_H
#define _BRIDGE_MEM_H

int bridge_mem_init(void);
int bridge_mem_add_tag(unsigned int tag, unsigned int val);
int bridge_mem_get_tag(unsigned int tag, unsigned int *p_val);
int bridge_mem_modify_tag(unsigned int tag, unsigned int val);

#endif