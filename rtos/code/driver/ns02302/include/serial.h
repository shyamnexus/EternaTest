/* {{{1 License
    FreeRTOS V8.2.0 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

	***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
	***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
	the FAQ page "My application does not run, what could be wrong?".  Have you
	defined configASSERT()?

	http://www.FreeRTOS.org/support - In return for receiving this top quality
	embedded software for free we request you assist our global community by
	participating in the support forum.

	http://www.FreeRTOS.org/training - Investing in training allows your team to
	be as productive as possible as early as possible.  Now you can receive
	FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
	Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!

    Author:
      Dr. Johann Pfefferl <johann.pfefferl@siemens.com>
      Siemens AG
}}} */

#ifndef _UART_PRINTF_H_
#define _UART_PRINTF_H_
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include "uart.h"

#if defined(_CPU2_RTOS_)
#if defined(_NVT_FPGA_)
#define _UART_OPEN()		uart2_open()
#define _UART_INITHW(m)		uart2_initHW(m)
#define _UART_PUTCHAR(m)	uart2_putChar(m)
#define _UART_GETCHAR(m)	uart2_getChar(m)
#else
#define _UART_OPEN()		uart3_open()
#define _UART_INITHW(m)		uart3_initHW(m)
#define _UART_PUTCHAR(m)	uart3_putChar(m)
#define _UART_GETCHAR(m)	uart3_getChar(m)
#endif
#else
#define _UART_OPEN()		uart_open()
#define _UART_INITHW(m)		uart_initHW(m)
#define _UART_PUTCHAR(m)	uart_putChar(m)
#define _UART_GETCHAR(m)	uart_getChar(m)
#endif
inline void serial_open(void) {
	UART_INIT_PARA  UartParaBasic;

	_UART_OPEN();

	UartParaBasic.BaudRate       = UART_BAUDRATE_115200;
	UartParaBasic.BaudRate       = 115200;
	UartParaBasic.Length         = UART_LEN_L8_S1;
	UartParaBasic.Parity         = UART_PARITY_NONE;
	UartParaBasic.FlowCtrlMode   = UART_FC_MODE_NONE;
	UartParaBasic.DataTransMode  = UART_DATA_TRANS_PIO;

	_UART_INITHW(&UartParaBasic);
 
}

inline void serial_putchar(uint32_t c) {
	_UART_PUTCHAR(c);
}

inline int serial_getchar(void) {
	CHAR c;
	_UART_GETCHAR(&c);
	return c;
}
#endif
