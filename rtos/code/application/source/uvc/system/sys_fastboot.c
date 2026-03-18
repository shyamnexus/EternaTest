#include <kwrap/flag.h>
#include <kwrap/util.h>
#include <kwrap/task.h>
#include <console.h>
#include "sys_fastboot.h"

static ID fastboot_flg_id = 0;

void fastboot_init(void)
{
	vos_flag_create(&fastboot_flg_id, NULL, "fastboot_flg_id");
	vos_flag_clr(fastboot_flg_id, (FLGPTN)-1);
}

void fastboot_set_done(BOOT_INIT boot_init)
{
	vos_flag_set(fastboot_flg_id, (FLGPTN)(1 << boot_init));
}

void fastboot_wait_done(BOOT_INIT boot_init)
{
	FLGPTN flgptn;
	vos_flag_wait(&flgptn, fastboot_flg_id, (FLGPTN)(1 << boot_init), TWF_ANDW);
}

void fastboot_msg_en(int en)
{
	CONSOLE console = {0};

	if (en) {
		console_get_uart1(&console);
	} else {
		//console_get_erronly_uart1(&console);
		console_get_none(&console);
	}
	console_set_curr(&console);
}

void fastboot_thread(void *p_param)
{
	typedef void (*INSMOD_MOUDLE)(void);
	INSMOD_MOUDLE insmod_module = (INSMOD_MOUDLE)p_param;
	vos_task_enter();
	insmod_module();
	THREAD_RETURN(0);
}
