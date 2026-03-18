/**

    CC module internal header file

    CC module internal header file

    @file       cc_int.h
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _CC_INT_H
#define _CC_INT_H

//#include <rcw_macro.h>

#define CC_INT_EVENT_FROM_CPU1          0x00000001
#define CC_INT_EVENT_FROM_CPU2          0x00000002
#define CC_INT_EVENT_FROM_DSP           0x00000004
#define CC_INT_EVENT_FROM_DSP2          0x00000008

#define CC_INT_ACK_FROM_CPU1            0x00010000
#define CC_INT_ACK_FROM_CPU2            0x00020000
#define CC_INT_ACK_FROM_DSP             0x00040000
#define CC_INT_ACK_FROM_DSP2            0x00080000

#define CC_UNIT_TEST                    1


#define CC_SET_DSP2_DSP_BASE1_ADDRESS_OFS     0x184
#define CC_SET_DSP2_DSP_BASE2_ADDRESS_OFS     0x188
#define CC_SET_DSP2_DSP_BASE3_ADDRESS_OFS     0x18C


#if CC_UNIT_TEST
void cc_core3_ack_core1_cmd(void);
void cc_core4_ack_core1_cmd(void);
CC_ER cc_core3_send_core1_cmd(PCC_CMD p_cmd);
CC_ER cc_core4_send_core1_cmd(PCC_CMD p_cmd);
#endif

//@}

#endif
