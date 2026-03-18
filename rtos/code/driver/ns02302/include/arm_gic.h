#ifndef _ARM_GIC_H
#define _ARM_GIC_H

extern void arm_gic_init(void);
extern int arm_gic_raise_sgi(int it, unsigned char cpu_mask);
extern void arm_gic_disable_interrupt(int *p_irqs, int irqs_cnt, int except_mode);

#endif

