#ifndef SYS_FASTBOOT_H
#define SYS_FASTBOOT_H

typedef enum _BOOT_INIT_ {
	BOOT_INIT_SYSTEM,
	BOOT_INIT_SENSOR,
	BOOT_INIT_CAPTURE,
	BOOT_INIT_DISPLAY,
	BOOT_INIT_STORAGE,
	BOOT_INIT_MEIDA_ENCODER,
	BOOT_INIT_OTHERS,
	BOOT_FLOW_BOOT,
	BOOT_INIT_MAX_CNT = 32, //flag max bit cnt is 32
	ENUM_DUMMY4WORD(BOOT_INIT),
} BOOT_INIT;

void fastboot_init(void);
void fastboot_set_done(BOOT_INIT boot_init);
void fastboot_wait_done(BOOT_INIT boot_init);
void fastboot_msg_en(int en);
void fastboot_thread(void *p_param);

#endif