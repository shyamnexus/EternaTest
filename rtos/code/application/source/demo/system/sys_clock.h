#ifndef SYS_CLOCK_H
#define SYS_CLOCK_H

/**
 * @brief close unused clocks such as IDE
 *
 */
extern int clock_disable_unused(void);

/**
 * @brief set apb clock if fdt indicated
 *
 */
extern int clock_set_apb_as_fdt(void);

#endif