/**
    Header file to register debug message output

    uart_putChar is default to use

    @file       console.h
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _CONSOLE_H
#define _CONSOLE_H

typedef struct _CONSOLE {
	void (*hook)(void); ///< start this object
	void (*unhook)(void); ///< terminal this object
	void (*puts)(const char *s); ///< console input funciton pointer
	int (*gets)(char *s, int len); ///< console output function point
} CONSOLE;

int console_set_curr(CONSOLE *p_console);
int console_get_curr(CONSOLE *p_console);
int console_get_uart1(CONSOLE *p_console);
int console_get_uart2(CONSOLE *p_console);
int console_get_uart3(CONSOLE *p_console);
int console_get_uart4(CONSOLE *p_console);
int console_get_uart5(CONSOLE *p_console);
int console_get_uart6(CONSOLE *p_console);
int console_get_uart7(CONSOLE *p_console);
int console_get_uart8(CONSOLE *p_console);
int console_get_uart9(CONSOLE *p_console);
int console_get_none(CONSOLE *p_console);
int console_get_erronly_uart1(CONSOLE *p_console);
int console_get_erronly_uart2(CONSOLE *p_console);
int console_get_erronly_uart3(CONSOLE *p_console);
int console_get_erronly_uart4(CONSOLE *p_console);
int console_get_erronly_uart5(CONSOLE *p_console);
int console_get_erronly_uart6(CONSOLE *p_console);
int console_get_erronly_uart7(CONSOLE *p_console);
int console_get_erronly_uart8(CONSOLE *p_console);
int console_get_erronly_uart9(CONSOLE *p_console);

#endif
