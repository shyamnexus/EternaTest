#ifndef _AOV1_CTRL_H_
#define _AOV1_CTRL_H_

#include "aov1_common.h"
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <kwrap/examsys.h> 

#if defined(__LINUX)

#else	// __FREERTOS
#include <plat/gpio.h>
#endif

void set_powerdown(int delay_time);
void system_powerdown(void);
int gpio_set_dir(unsigned gpio, int value);
int gpio_set_value(unsigned gpio, int value);
int gpio_get_value(unsigned gpio);
int pwm_en(unsigned pwm_id, int en);
int yuv_save(HD_VIDEO_FRAME *video_frame, char* str);
char getche(void);
int kbhit(void) ;

#endif	// _AOV1_CTRL_H_

