#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <serial.h>
#include <io_address.h>
#include <interrupt.h>
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>
#include <kwrap/util.h>
#include "gpio.h"
#include "pad.h"
#include "top.h"

#define Delay_DelayUsPolling vos_util_delay_us_polling

// for master use
UINT32 PBUS_master_pinmux_init(void);
UINT32 PBUS_master_writeReg(UINT32 address, UINT32 datain);
UINT32 PBUS_master_readReg(UINT32 address, UINT32* dataout);