/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       kdrv_rpc_debug_cmd.h

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _KDRV_RPC_DEBUG_CMD_H
#define _KDRV_RPC_DEBUG_CMD_H

#define  KDRV_RPC_TEST_CMD    ENABLE

extern void kdrv_rpc_install_cmd(void);
extern int  kdrv_rpc_cmd_init(void);
extern void kdrv_rpc_cmd_exit(void);
extern int  kdrv_rpc_cmd_showhelp(int (*dump)(const char *fmt, ...));
extern int  kdrv_rpc_cmd_execute(unsigned char argc, char **argv);
#endif

