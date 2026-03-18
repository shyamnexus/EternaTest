#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwrap/task.h>
#include <kwrap/spinlock.h>

#include "freertos_ext_kdrv.h"

#define __MODULE__    rtos_dumpstack
#define __DBGLVL__    8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__    "*"
#include <kwrap/debug.h>

#include <FreeRTOS.h>
#include <task.h> //FreeRTOS header file

#ifndef configNUMBER_OF_CORES
#define configNUMBER_OF_CORES 1 /* rtos-smp backward compatible */
#endif

/*
\033[0m ---- reset to default
\033[0;30m --- dark gray (black)
\033[0;31m --- dark red
\033[0;32m --- dark green
\033[0;33m --- dark yellow
\033[0;34m --- dark blue
\033[0;35m --- dark magenta
\033[0;36m --- dark cyan
\033[0;37m --- dark white (light gray)
\033[1;30m --- gray (dark gray)
\033[1;31m --- red
\033[1;32m --- green
\033[1;33m --- yellow
\033[1;34m --- blue
\033[1;35m --- magenta
\033[1;36m --- cyan
\033[1;37m --- white
*/

typedef int  KER_PRINT_FUNC(char *fmtstr, ...);

typedef struct {
	UINT32           frame_index;
	ULONG            frame_begin;
	ULONG            frame_end;
} STACK_FRAME_INFO;

#define OS_LogSaveStr   vk_print_isr
#define STACK_CHECK_TAG 0xA5A5A5A5

typedef void TCB_t;
#if (configNUMBER_OF_CORES == 1)
extern volatile TCB_t * volatile pxCurrentTCB;
#else
extern volatile TCB_t * volatile pxCurrentTCBs[ configNUMBER_OF_CORES ];
#endif
extern char _section_zi_addr[];
extern unsigned char _stack_end[];
extern unsigned char _stack[];


/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
unsigned int rtos_dumpstack_debug_level = NVT_DBG_WRN;


//static void vos_print_frame_detail_p(UINT32 frame_index, UINT32 frame_begin, UINT32 frame_end, UINT32 stack_end, UINT32 pc ,KER_PRINT_FUNC  print_func)
static void vos_print_frame_detail_p(STACK_FRAME_INFO *p_stack_frame, ULONG stack_end, ULONG pc, KER_PRINT_FUNC  print_func)
{
	UINT32 index;
	ULONG  frame_begin = p_stack_frame->frame_begin;
	ULONG  frame_end = p_stack_frame->frame_end;


	if (frame_end + 4 >= stack_end) {
		frame_end = stack_end;
	}

	print_func("  %2ld frame(0x%08lx - 0x%08lx) ............................ $pc : 0x%08lx\r\n",
			   p_stack_frame->frame_index, frame_begin, frame_end, pc);
	{
		ULONG *stack_word = (ULONG *)ALIGN_FLOOR_16(frame_begin);
		index = 0;
		while ((ULONG)stack_word < (frame_end)) {
			if ((index % 4) == 0) {
				print_func("%4s + 0x%08lx : ", "", (stack_word));
			}
			if ((ULONG)stack_word < frame_begin) {
				print_func("           ");
			} else {
				print_func("0x%08x ", *(stack_word));
			}
			if ((index % 4) == 3) {
				print_func("\r\n");
			} else if ((ULONG)(stack_word + 1) == (frame_end)) {
				print_func("\r\n");
			}
			index++;
			stack_word++;
		}
	}
}

static void vos_print_backtrace_p(ULONG epc, ULONG sp, ULONG topfp, ULONG h_stack_base, ULONG h_stack_size, BOOL print_detail, UINT32 max_frame, KER_PRINT_FUNC  print_func)
{
	UINT32 cur_frame = 0;
	ULONG  curfp = topfp;
	ULONG  fp;
	ULONG  lr;
	ULONG  stack_end = h_stack_base + h_stack_size;
	BOOL   trace_until_stack_end = 0;
	STACK_FRAME_INFO    stackframe;
	ULONG  sys_stack_base;
	ULONG  sys_stack_end;

	sys_stack_base = (ULONG)_stack_end;
	sys_stack_end  = (ULONG)_stack;
	if (!print_detail) {
		print_func("0x%08lx\r\n", epc);
	} else {
		if ((sp < h_stack_base || sp > stack_end) && (sp < sys_stack_base || sp > sys_stack_end) ) {
			print_func("\033[1;31msp(0x%lx) out of stack (0x%08lx~0x%08lx)\x1B[0m\r\n", sp, h_stack_base, stack_end);
		} else {
			stackframe.frame_index = cur_frame;
			stackframe.frame_begin = sp;
			stackframe.frame_end = topfp;
			vos_print_frame_detail_p(&stackframe, stack_end, epc, print_func);
		}
	}
	cur_frame = 1;
	while (curfp && cur_frame < max_frame) {

		// end of stack
		#if defined(__aarch64__)
		if (curfp == 0 || curfp == 0x2929292929292929) {
			break;
		}
		#else
		if (curfp == 0 || curfp == 0x11111111) {
			break;
		}
		#endif
		
		if ((curfp < h_stack_base || curfp > stack_end) && (curfp < sys_stack_base || curfp > sys_stack_end)) {
			print_func("\033[1;31mfp(0x%lx) out of stack(0x%08lx~0x%08lx)\x1B[0m\r\n", curfp, h_stack_base, stack_end);
			break;
		}
		if (curfp & 0x3) {
			print_func("\033[1;31mInvalid fp(0x%x)\x1B[0m\r\n", curfp);
			//debug_dumpmem(curfp-0x200,0x300);
			break;
		}
		#if defined(__aarch64__)
		fp = *(((ULONG *)curfp));
		lr = *(((ULONG *)curfp) + 1);
		#else
		fp = *(((ULONG *)curfp) - 1);
		lr = *(((ULONG *)curfp));
		#endif

		// end of stack
		#if defined(__aarch64__)
		if (fp == 0 || fp == 0x2929292929292929) {
			break;
		}
		#else
		if (fp == 0 || fp == 0x11111111) {
			break;
		}
		#endif
		//print_func("fp = 0x%08lx, lr  = 0x%08lx\r\n", fp, lr );
		if (!print_detail) {
			print_func("0x%08lx\r\n", (int)lr);
		}
		if (print_detail) {
			stackframe.frame_index = cur_frame;
			stackframe.frame_begin = curfp;
			stackframe.frame_end = fp;
			vos_print_frame_detail_p(&stackframe, stack_end, lr, print_func);
		}
		curfp = fp;
		cur_frame++;
	}
	if (curfp + 4 >= stack_end) {
		trace_until_stack_end = 1;
	}
	if (trace_until_stack_end) {
		if (print_detail) {
			print_func("  end\r\n");
		} else {
			print_func("end\r\n");
		}
	}
}
void vos_dump_stack_backtrace(ULONG *info, ULONG h_stack_base, ULONG h_stack_size, UINT32 level)
{
	ULONG      pc, sp, fp, ra;
	BOOL       print_detail;
	UINT32     max_frame = 20;
	KER_PRINT_FUNC  *print_func;

	//#NT#2019/07/24#Nestor Yang -begin
	//#NT# The same code (Warning by Coverity)
	//if (level >= 2) {
		print_func = (KER_PRINT_FUNC *)vk_printk;
	//} else {
	//	print_func = (KER_PRINT_FUNC *)vk_printk; //should log to file ?
	//}
	//#NT#2019/07/24#Nestor Yang -end

	if (level >= 2) {
		print_func("stack      : \r\n");
		print_func("    range(0x%08lx - 0x%08lx)\r\n",
				   h_stack_base,
				   h_stack_base + h_stack_size);
	}
	/* scan code and dump call stack */
	//print_func("call stack :\r\n");
	print_func("\033[1;33mcall stack :\r\n");
	sp = *(info + 0); //current sp
	fp = *(info + 1); //current fp
	ra = *(info + 2); //current ra
	//DBG_DUMP("sp = 0x%08x, pc = 0x%08x, fp = 0x%08x\r\n", (int)sp, (int)pc, (int)fp);
	pc = ra;
	print_detail = 0;
	// print stack backtrace
	vos_print_backtrace_p(pc, sp, fp, h_stack_base, h_stack_size, print_detail, max_frame, print_func);
	print_func("\x1B[0m");
	// print frame detail
	print_detail = 1;
	if (level >= 2) {
		vos_print_backtrace_p(pc, sp, fp, h_stack_base, h_stack_size, print_detail, max_frame, print_func);
	} else if (level >= 1) {
		max_frame = 3;
		vos_print_backtrace_p(pc, sp, fp, h_stack_base, h_stack_size, print_detail, max_frame, print_func);
	}

}

void vos_dump_stack_content(ULONG h_stack_pointer, ULONG h_stack_base, ULONG h_stack_size, UINT32 level)
{
	ULONG           StackSize = h_stack_size;
	ULONG           StackTop = h_stack_base;
	ULONG           StackBottom = h_stack_base + h_stack_size - sizeof(ULONG);
	ULONG           StackPointer = h_stack_pointer;
	UINT32 *pMem;
	UINT32          k, l, m;
	BOOL   bBreakForLoop;
	ULONG    uiCodeBase = 0;
	ULONG    uiCodeLimit = 0;


	uiCodeBase =  0; //first section start
	uiCodeLimit = (ULONG)_section_zi_addr; //last section end

	DBG_DUMP("Stack Content:\r\n");
	if ((level == 0) && (StackSize > 16)) {
		//dump from stack top until 16 bytes
		pMem = (UINT32 *)StackTop;
		bBreakForLoop   = FALSE;

		for (l = 0, m = 0; (l < 1) && (bBreakForLoop == FALSE) ; l++) {
			DBG_DUMP("0x%.8lX: ", (ULONG)pMem);

			for (k = 0; k < 4; k++, pMem++) {
				if (StackPointer == (ULONG)pMem) {
					DBG_DUMP("\033[1;32m[0x%.8X] \x1B[0m", (int)*pMem);
				} else if (StackTop == (ULONG)pMem) {
					DBG_DUMP("\033[1;35m[0x%.8X> \x1B[0m", (int)*pMem);
				} else if (StackBottom == (ULONG)pMem) {
					DBG_DUMP("\033[1;36m<0x%.8X] \x1B[0m", (int)*pMem);
					bBreakForLoop = TRUE;
				} else {
					DBG_DUMP("[0x%.8X] ", (int)*pMem);
				}
			}
			DBG_DUMP("\r\n");
		}
		DBG_DUMP("             ..........   ..........   ..........   .......... \r\n");
	}

	if (level == 0) {
		// Dump from stack pointer to stack bottom
		pMem = (UINT32 *)ALIGN_FLOOR_16(StackPointer); //dump from 16 bytes align
	} else { //level==1
		// Dump from stack top to stack bottom
		pMem = (UINT32 *)StackTop; //dump from 16 bytes align
	}
	bBreakForLoop   = FALSE;

	for (l = 0, m = 0; (l < 1024) && (bBreakForLoop == FALSE) ; l++) {
		DBG_DUMP("0x%.8lX: ", (ULONG)pMem);

		for (k = 0; k < 4; k++, pMem++, m++) {
			if (StackPointer == (ULONG)pMem) {
				DBG_DUMP("\033[1;32m[0x%.8X] \x1B[0m", (int)*pMem);
			} else if (StackTop == (ULONG)pMem) {
				DBG_DUMP("\033[1;35m[0x%.8X> \x1B[0m", (int)*pMem);
			} else if (StackBottom == (ULONG)pMem) {
				DBG_DUMP("\033[1;36m<0x%.8X] \x1B[0m", (int)*pMem);
				bBreakForLoop = TRUE;
			}
			else {
				if (
					((*pMem) >= ((ULONG)uiCodeBase))
					&& ((*pMem) < ((ULONG)uiCodeLimit))
				) {
					DBG_DUMP("\033[0;33m<0x%.8X> \x1B[0m", (int)*pMem);
				} else {
					DBG_DUMP("[0x%.8X] ", (int)*pMem);
				}
			}
		}
		DBG_DUMP("\r\n");
	}
}

static int vos_stack_get_percent(ULONG numerator, ULONG denominator)
{
	int ret;

	if (0 == denominator) {
		return -1; //invalid denominator
	}

	ret = (int)( (100 * numerator + denominator - 1) / denominator );

	return ret;
}

void vos_show_stack(VK_TASK_HANDLE takhdl, UINT32 level)
{
	ULONG           StackSize;
	ULONG           StackUsed;
	ULONG           StackMaxUsed;
	StackType_t     *pStackTop;
	StackType_t     *pStackBottom;
	volatile StackType_t *pStackPointer;
	UINT32          backtrace_level = 0;
	TaskDetailStatus_t   taskinfo, *pTask;
	eTaskState      tskstate;
	ULONG           pc, sp, fp, lr, ra;
	ULONG           info[3] = {0};
	UINT32          fpu_offset;
	ULONG           sys_stack_base;
	ULONG           sys_stack_end;

	sys_stack_base = (ULONG)_stack_end;
	sys_stack_end  = (ULONG)_stack;
#if (configUSE_TASK_FPU_SUPPORT == 2)
	#if defined(__aarch64__)
	fpu_offset = 64; //FreeRTOS: portFPU_REGISTER_DOUBLE_WORDS
	#else
	fpu_offset = 65; //FreeRTOS: portFPU_REGISTER_WORDS
	#endif
#else
	fpu_offset = 0;
#endif

	pTask = &taskinfo;
	if (NULL == takhdl) {
#if (configNUMBER_OF_CORES == 1)
		takhdl = (VK_TASK_HANDLE)pxCurrentTCB;
#else
		takhdl = (VK_TASK_HANDLE)&pxCurrentTCBs[portGET_CORE_ID()];
#endif
	}
	tskstate = eTaskGetState(takhdl);
	vTaskGetDetailInfo( (TaskHandle_t)takhdl, &taskinfo, pdTRUE, tskstate );
	pStackTop = pTask->pxStackBase;
	pStackBottom = pTask->pxEndOfStack;
	pStackPointer = pTask->pxTopOfStack;
	StackSize = (ULONG)pStackBottom - (ULONG)pStackTop;
	StackMaxUsed = StackSize - (((ULONG)pTask->usStackHighWaterMark) * sizeof(StackType_t));
	OS_LogSaveStr("pStackTop = 0x%lx, pStackBottom = 0x%lx, pStackPointer= 0x%lx, StackSize = 0x%lx\r\n", (ULONG)pStackTop, (ULONG)pStackBottom, (ULONG)pStackPointer, StackSize);
	if (pTask->pxStackBase != 0) {
		StackUsed = (ULONG)pStackBottom - (ULONG)pStackPointer;
	} else {
		StackUsed = 0;
	}

	// Display stack information
	OS_LogSaveStr("Stack Area: ");
	OS_LogSaveStr("\033[1;35m[0x%.8lX>\x1B[0m", (ULONG)pStackTop);
	OS_LogSaveStr(" ~ ");
	OS_LogSaveStr("\033[1;36m<0x%.8lX]\x1B[0m", (ULONG)pStackBottom);
	OS_LogSaveStr(", Pointer 0x%.8lX\r\n", (ULONG)pStackPointer);
	OS_LogSaveStr("Stack Size: 0x%08lX, Curr Used: 0x%08lX, Max Used: 0x%08lX (%d%%)\r\n",
				StackSize, StackUsed, StackMaxUsed, vos_stack_get_percent(StackMaxUsed, StackSize));

	if (level == 0) {
		return;
	}
	// Check if not start or not exist
	if (tskstate >= eSuspended) {
		OS_LogSaveStr("(Ignore)\r\n");
		return;
	}

	// Check if stack range is valid
	if ((StackSize == 0)
		|| (StackSize > 0x00100000) //1MB
		|| ((ULONG)pStackTop == 0)
		|| ((ULONG)pStackBottom == 0)) {
		OS_LogSaveStr("\033[1;31m###Invalid stack range!###\x1B[0m\r\n");
		return;
	}
	if (((ULONG)pStackPointer == 0)
		|| (((ULONG)pStackPointer < (ULONG)pStackTop || ((ULONG)pStackPointer > (ULONG)pStackBottom))
		&& ((ULONG)pStackPointer < (ULONG)sys_stack_base || ((ULONG)pStackPointer > (ULONG)sys_stack_end)))){
		OS_LogSaveStr("\033[1;31m###Invalid stack point!###\x1B[0m\r\n");
		return;
	}
	if (level == 2) {
		vos_dump_stack_content((ULONG)pStackPointer, (ULONG)pStackTop, StackSize, 1);
		return;
	}
	// check if this is running task
#if (configNUMBER_OF_CORES == 1)
	if (takhdl == (VK_TASK_HANDLE)pxCurrentTCB) {
#else
	if (takhdl == (VK_TASK_HANDLE)&pxCurrentTCBs[portGET_CORE_ID()]) {
#endif

#if defined(__aarch64__)
		__asm__ __volatile__("mov %0, x30\n\t" : "=r"(lr));  //get $ra
		__asm__ __volatile__("mov %0, x29\n\t" : "=r"(fp));  //get $fp
#else
		__asm__ __volatile__("mov %0, r14\n\t" : "=r"(lr));  //get $ra
		__asm__ __volatile__("mov %0, r11\n\t" : "=r"(fp));  //get $fp
#endif
	} else {
#if defined(__aarch64__)
		fp = pStackPointer[7 + fpu_offset];
		lr = pStackPointer[2 + fpu_offset];
#else
		fp = pStackPointer[13 + fpu_offset];
		lr = pStackPointer[16 + fpu_offset];
#endif
	}
	if (tskstate != eRunning) {
		backtrace_level = 0;
	}
	ra = lr;
	pc = lr;
	sp = (ULONG)(pStackPointer + 0 + fpu_offset);
	OS_LogSaveStr("registers : $pc  - 0x%08lx  $ra  - 0x%08lx  $sp  - 0x%08lx  $fp  - 0x%08lx\r\n",
					pc, ra, sp, fp);
	info[0] = sp;
	info[1] = fp;
	info[2] = ra;
	vos_dump_stack_backtrace(info, (ULONG)pStackTop, (ULONG)StackSize, backtrace_level);
}

void vos_dump_stack(void)
{
	vos_show_stack(NULL, 1);
}
