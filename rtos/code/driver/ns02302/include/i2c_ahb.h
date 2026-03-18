#ifndef _AHB_IC_I2C_H
#define _AHB_IC_I2C_H

#include "i2c/i2c.h"

#define I2C_AHB_NONE            0
#define I2C_AHB_TC690B          1
#define I2C_AHB_TC690C          2
#define I2C_AHB_IC              I2C_AHB_TC690B

extern BOOL     tc690_ic_open(void);
extern BOOL     tc690_ic_close(void);

extern I2C_STS  tc690_unlock_cfg(void);
extern I2C_STS  tc690_read_reg(UINT32 address, UINT32 *value);
extern I2C_STS  tc690_write_reg(UINT32 address, UINT32 data);
#endif
