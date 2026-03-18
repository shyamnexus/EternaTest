#include <stdio.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <kwrap/cpu.h>
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/mailbox.h>
#include <kwrap/task.h>
#include <kwrap/debug.h>
#include <kwrap/cmdsys.h>
#include <kwrap/ker_dump.h>
#include <comm/timer.h>
#include "comm/hwclock.h"
#include <plat/interrupt.h>
#include <timers.h>
#include <arm_gic.h>
#include <stdlib.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include "startup.h"

#define DAEMON_TASK_TIMEOUT         5000000
TimerHandle_t       wdt_timer;
static UINT64       prev_wdt_timer_time = 0;



/**
 * @brief User defined assertion call. This function is plugged into configASSERT.
 * See FreeRTOSConfig.h to define configASSERT to something different.
 */
void vAssertCalled(const char * pcFile,
	uint32_t ulLine)
{
	volatile uint32_t ulBlockVariable = 0UL;
	volatile char * pcFileName = (volatile char *)pcFile;
	volatile uint32_t ulLineNumber = ulLine;

	(void)pcFileName;
	(void)ulLineNumber;

	printf("vAssertCalled %s, %ld\n", pcFile, (long)ulLine);

	/* Setting ulBlockVariable to a non-zero value in the debugger will allow
	* this function to be exited. */
	taskDISABLE_INTERRUPTS();
	vos_dump_stack();
	//coverity[no_escape]
	while (ulBlockVariable == 0UL)
	{
	}
	taskENABLE_INTERRUPTS();
}

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    char * pcTaskName )
{
	printf("### Stack overflow, Task: %s ###\r\n", pcTaskName);

	( void ) pcTaskName;
	( void ) xTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */

	taskDISABLE_INTERRUPTS();
	//coverity[no_escape]
	for( ; ; )
	{
		/* Loop here */
	};
}

void vApplicationIdleHook( void )
{
	static BOOL is_hang_dump = FALSE;
	UINT64 hook_time, cur_time;

	hook_time = prev_wdt_timer_time;
	cur_time = hwclock_get_longcounter();
	if ((FALSE == is_hang_dump) && hook_time && (cur_time - hook_time > DAEMON_TASK_TIMEOUT)) {
		printf("\r\n### TimerTask my hang, diff %lld us! ht = %lld us, cur = %lld us\r\n", (cur_time - hook_time), hook_time, cur_time);
		nvt_cmdsys_runcmd("ker dump");
		is_hang_dump= TRUE;
	}
	// cpu enter sleep
	asm volatile ("wfi");
}

/*-----------------------------------------------------------*/
static ID sem_shutdown;

static void wdt_timer_cb(TimerHandle_t expiredTimer)
{
	prev_wdt_timer_time = hwclock_get_longcounter();
	//printf("prev_wdt_timer_time = %lld\r\n", prev_wdt_timer_time);
	//vos_sem_wait(sem_shutdown);
}

static void start_wdt_timer(void)
{
	wdt_timer = xTimerCreate( "wdtTimer",                       // Just a text name, not used by the kernel.
								( 1000 / portTICK_PERIOD_MS),   // The timer period in ticks.
								pdTRUE,                        // The timer is a one-shot timer.
								0,                              // The id is not used by the callback so can take any value.
								wdt_timer_cb                    // The callback function that switches the LCD back-light off.
								);
	if( wdt_timer != NULL ) {
		if( xTimerStart( wdt_timer, 0 ) != pdPASS ) {
			printf("start wdt_timer fail\r\n");
	    }
	}
}

void constructors_init(void)
{
	typedef void (*pfunc) (void);
    extern pfunc __CTOR_LIST__[];
    extern pfunc __CTOR_END__[];

	pfunc *p;
	//coverity[negative_returns]
    for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--) {
        (*p) ();
    }
}

static void env_init(void)
{
	unsigned char *fdt = (unsigned char *)fdt_get_base();

	if (fdt == NULL) {
		printf("p_fdt is NULL.\n");
		return;
	}

	int len;
	int nodeoffset;
	nodeoffset = fdt_path_offset(fdt, "/nvt_info");
	if (nodeoffset != -FDT_ERR_NOTFOUND) {
		// other fields in nvt_info
		for (nodeoffset = fdt_first_property_offset(fdt, nodeoffset);
			(nodeoffset >= 0);
			(nodeoffset = fdt_next_property_offset(fdt, nodeoffset))) {
			const struct fdt_property *prop_reg;
			if (!(prop_reg = fdt_get_property_by_offset(fdt, nodeoffset, &len))) {
				break;
			}
			const char *name = fdt_string(fdt, fdt32_to_cpu(prop_reg->nameoff));
			const char *value = (const char *)prop_reg->data;
			setenv(name, value, 1);
		}
	}
}

static void os_init(void)
{
	extern unsigned char _section_01_addr[];
	extern unsigned char _image_general_zi_zi_base[];
	extern unsigned char _image_general_zi_zi_limit[];
	extern unsigned char _rtos_heap_start[];
	extern unsigned char _rtos_heap_end[];
	extern unsigned char _libc_heap_start[];
	extern unsigned char *_libc_heap_end;
	OSINIT_PARAM init_parm = {0};

	init_parm.total.addr = 0;
	init_parm.total.size = (UINT32)_libc_heap_end;
	init_parm.resv.addr = 0;
	init_parm.resv.size = (UINT32)_section_01_addr;
	init_parm.code.addr = (UINT32)_section_01_addr;
	init_parm.code.size = (UINT32)_image_general_zi_zi_base - (UINT32)_section_01_addr;
	init_parm.zi.addr   = (UINT32)_image_general_zi_zi_base;
	init_parm.zi.size   = (UINT32)_image_general_zi_zi_limit - (UINT32)_image_general_zi_zi_base;
	init_parm.os_heap.addr = (UINT32)_rtos_heap_start;
	init_parm.os_heap.size = (UINT32)_rtos_heap_end - (UINT32)_rtos_heap_start;
	init_parm.libc_heap.addr = (UINT32)_libc_heap_start;
	init_parm.libc_heap.size = (UINT32)_libc_heap_end - (UINT32)_libc_heap_start;

	ker_dump_os_init(&init_parm);
}

static void rtos_main_tsk(void * p_param)
{
	vos_sem_create(&sem_shutdown, 1, "sem_shutdown");
	vos_sem_wait(sem_shutdown); //clear shutdown semaphore

	// init os info for ker dump memory
	os_init();
	// init necessary for debug message
	// others hw init please do at insmod
	arm_gic_init();
	irq_init();
#if !defined(_FW_TYPE_PARTIAL_) && !defined(_FW_TYPE_PARTIAL_COMPRESS_)
	// !!!!! global c++'s constructors will be out of control on partial load tech. !!!!
	// !!!!! so we only allow to use c++ after section_10 loaded. !!!!
	// c++'s constructors init
	constructors_init();
#endif
	// start wdt timer for monitor if freertos daemon task work normally
	start_wdt_timer();
	// environment init from fdt/nvt-info
	env_init();
	// do the first program
	rtos_main();

	// wait system trigger shutdown
	vos_sem_wait(sem_shutdown);
	vos_sem_destroy(sem_shutdown);
}

int main(void)
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!! Here only initiate OS dependency, others initialization please do at rtos_main()
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// to avoid calling malloc on printf.
	THREAD_HANDLE main_hdl;

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
	// set freertos heap size for heap_5 method (https://www.freertos.org/a00111.html)
	extern unsigned char _rtos_heap_start[];
	extern unsigned char _rtos_heap_end[];
	HeapRegion_t xHeapRegions[] = {
		{ (uint8_t *) _rtos_heap_start, _rtos_heap_end-_rtos_heap_start },
		{ NULL, 0 }  //Terminates the array.
	};
	vPortDefineHeapRegions(xHeapRegions);
	// init cpu (cache, memory)
	rtos_cpu_init(NULL);
	// init semaphore and set the initial state
	rtos_sem_init(512);
	// init flag and set the initial state
	rtos_flag_init(256);
	// init mailbox and set the initial state
	vos_mbx_init(16);
	// create test tasks
	rtos_task_init(64);
	// create first task to do initialization
	main_hdl = vos_task_create(rtos_main_tsk, NULL, "rtos_main_tsk", 20, 8192);
	vos_task_resume(main_hdl);
	// start the scheduler.
	vTaskStartScheduler();
#if 0   // here not be reached.
	rtos_sem_exit();
	rtos_flag_exit();
	rtos_task_exit();
#endif
	return 0;
}
