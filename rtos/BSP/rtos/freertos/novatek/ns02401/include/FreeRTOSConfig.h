#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Here is a good place to include header files that are required across
your application. */

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      12000000 /* for arm32 reference */
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    32
#define configMINIMAL_STACK_SIZE                128
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 0
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_ALTERNATIVE_API               0 			/* Deprecated! */
#define configQUEUE_REGISTRY_SIZE               10
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  0
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5
#define configRECORD_STACK_HIGH_ADDRESS         1
#define configTIMER_PRIVIATE                    1
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP  0
#if defined(_NVT_FPGA_)
#define configUSE_CA53CORE_TIMER_RATE_MHZ		24
#else
#define configUSE_CA53CORE_TIMER_RATE_MHZ		12
#endif

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   0x80000
#define configAPPLICATION_ALLOCATED_HEAP        0

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     1
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES-1)
#define configTIMER_QUEUE_LENGTH                64
#define configTIMER_TASK_STACK_DEPTH            (8192)

#define configUSE_NVT_PATH                      1

void traceTick(uint32_t tick);
#define traceTASK_INCREMENT_TICK( xTickCount )	//traceTick(xTickCount)


/*
 * The application must provide a function that configures a peripheral to
 * create the FreeRTOS tick interrupt, then define configSETUP_TICK_INTERRUPT()
 * in FreeRTOSConfig.h to call the function.  This file contains a function
 * that is suitable for use on the Zynq MPU.  FreeRTOS_Tick_Handler() must
 * be installed as the peripheral's interrupt handler.
 */
void vConfigureTickInterrupt( void );
#define configSETUP_TICK_INTERRUPT() vConfigureTickInterrupt();

/* Interrupt nesting behaviour configuration. */
//#define configKERNEL_INTERRUPT_PRIORITY         1
//#define configMAX_SYSCALL_INTERRUPT_PRIORITY    2

/* The following constant describe the hardware, and are correct for the
Cyclone V SoC. */
#if defined(__aarch64__) //non-secure
#define configINTERRUPT_CONTROLLER_BASE_ADDRESS (0x4FF800000)
#else
#define configINTERRUPT_CONTROLLER_BASE_ADDRESS (0xFFF00000)
#endif
#define configINTERRUPT_CONTROLLER_CPU_DISTRIBUTOR_OFFSET (0x1000)
#define configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET (0x02000)

#if defined(__aarch64__) //non-secure
#define configUNIQUE_INTERRUPT_PRIORITIES (16)
#define configMAX_API_CALL_INTERRUPT_PRIORITY (10)
#else
#define configUNIQUE_INTERRUPT_PRIORITIES (32)
#define configMAX_API_CALL_INTERRUPT_PRIORITY (18)
#endif

/* If configUSE_TASK_FPU_SUPPORT is set to 1 (or undefined) then each task will
be created without an FPU context, and a task must call vTaskUsesFPU() before
making use of any FPU registers.  If configUSE_TASK_FPU_SUPPORT is set to 2 then
tasks are created with an FPU context by default, and calling vTaskUsesFPU() has
no effect. */
#define configUSE_TASK_FPU_SUPPORT                              2

#define configUSE_APPLICATION_TASK_TAG 1

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
void vAssertCalled( const char * pcFile, unsigned long ulLine );
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ );

/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1
#define INCLUDE_xSemaphoreGetMutexHolder        1

/* A header file that defines trace macro can be included here. */
#define configUSE_POSIX_ERRNO 0
#define configUSE_APPLICATION_TASK_TAG 1


#endif /* FREERTOS_CONFIG_H */
