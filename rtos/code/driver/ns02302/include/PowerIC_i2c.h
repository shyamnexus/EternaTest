#ifndef _PWR_IC_I2C_H
#define _PWR_IC_I2C_H

#include "i2c/i2c.h"

#define I2C_POWER_IC_NONE       0
#define I2C_POWER_IC_RT5024     1
#define I2C_POWER_IC_TPS53915   2
#define I2C_POWER_IC            I2C_POWER_IC_TPS53915

#define POWER_08V               0
#define POWER_09V               1
#define BOARD_POWER             POWER_09V


#if (I2C_POWER_IC == I2C_POWER_IC_RT5024)
#define I2C_SLAVE_ADDRESS 0x45  //0010010
#define PWRIC_READ_ADDRESS  (i2c_conv7bSARToData(I2C_SLAVE_ADDRESS) | 0x1)
#define PWRIC_WRITE_ADDRESS (i2c_conv7bSARToData(I2C_SLAVE_ADDRESS) | 0x0)

#elif (I2C_POWER_IC == I2C_POWER_IC_TPS53915)
#define PWRIC_READ_ADDRESS          0x1F
#define PWRIC_WRITE_ADDRESS         0x1E
#define PWRIC_VOLTAGE_ADDRESS       0xD4
#define PWRIC_ADJUST_CMD_ADDRESS    0xD4
#define PWRIC_MARGIN_CMD_ADDRESS    0xD5

//adjust reg setting
#define TPS53915_ADJ_P9_00   (0x1C)  //+9.00%
#define TPS53915_ADJ_P8_25   (0x1B)  //+8.25%
#define TPS53915_ADJ_P7_50   (0x1A)  //+7.50%
#define TPS53915_ADJ_P6_75   (0x19)  //+6.75%
#define TPS53915_ADJ_P6_00   (0x18)  //+6.00%
#define TPS53915_ADJ_P5_25   (0x17)  //+5.25%
#define TPS53915_ADJ_P4_50   (0x16)  //+4.50%
#define TPS53915_ADJ_P3_75   (0x15)  //+3.75%
#define TPS53915_ADJ_P3_00   (0x14)  //+3.00%
#define TPS53915_ADJ_P2_25   (0x13)  //+2.25%
#define TPS53915_ADJ_P1_50   (0x12)  //+1.50%
#define TPS53915_ADJ_P0_75   (0x11)  //+0.75%
#define TPS53915_ADJ_P0_00   (0x10)  //+0.00%
#define TPS53915_ADJ_N0_00   (0x0F)  //-0.00%
#define TPS53915_ADJ_N0_75   (0x0E)  //-0.75%
#define TPS53915_ADJ_N1_50   (0x0D)  //-1.50%
#define TPS53915_ADJ_N2_25   (0x0C)  //-2.25%
#define TPS53915_ADJ_N3_00   (0x0B)  //-3.00%
#define TPS53915_ADJ_N3_75   (0x0A)  //-3.75%
#define TPS53915_ADJ_N4_50   (0x09)  //-4.50%
#define TPS53915_ADJ_N5_25   (0x08)  //-5.25%
#define TPS53915_ADJ_N6_00   (0x07)  //-6.00%
#define TPS53915_ADJ_N6_75   (0x06)  //-6.75%
#define TPS53915_ADJ_N7_50   (0x05)  //-7.50%
#define TPS53915_ADJ_N8_25   (0x04)  //-8.25%
#define TPS53915_ADJ_N9_00   (0x03)  //-9.00%
//margin reg setting postive
#define TPS53915_MARG_P12_00 (0xC0)  //+12.0%
#define TPS53915_MARG_P10_90 (0xB0)  //+10.9%
#define TPS53915_MARG_P09_90 (0xA0)  //+9.9%
#define TPS53915_MARG_P08_80 (0x90)  //+8.8%
#define TPS53915_MARG_P07_70 (0x80)  //+7.7%
#define TPS53915_MARG_P06_70 (0x70)  //+6.7%
#define TPS53915_MARG_P05_70 (0x60)  //+5.7%
#define TPS53915_MARG_P04_70 (0x50)  //+4.7%
#define TPS53915_MARG_P03_70 (0x40)  //+3.7%
#define TPS53915_MARG_P02_80 (0x30)  //+2.8%
#define TPS53915_MARG_P01_80 (0x20)  //+1.8%
#define TPS53915_MARG_P00_90 (0x10)  //+0.9%
#define TPS53915_MARG_P00_00 (0x00)  //+0.0%
//margin reg setting negative
#define TPS53915_MARG_N00_00 (0x00)  //-0.0%
#define TPS53915_MARG_N01_10 (0x01)  //-1.1%
#define TPS53915_MARG_N02_10 (0x02)  //-2.1%
#define TPS53915_MARG_N03_20 (0x03)  //-3.2%
#define TPS53915_MARG_N04_20 (0x04)  //-4.2%
#define TPS53915_MARG_N05_20 (0x05)  //-5.2%
#define TPS53915_MARG_N06_20 (0x06)  //-6.2%
#define TPS53915_MARG_N07_10 (0x07)  //-7.1%
#define TPS53915_MARG_N08_10 (0x08)  //-8.1%
#define TPS53915_MARG_N09_00 (0x09)  //-9.0%
#define TPS53915_MARG_N09_90 (0x0A)  //-9.9%
#define TPS53915_MARG_N10_70 (0x0B)  //-10.7%
#define TPS53915_MARG_N11_60 (0x0C)  //-11.6%

typedef enum {
	TPS53915_VOUT_MARGIN_REG = 0x0,
	TPS53915_VOUT_ADJUST_REG,
	TPS53915_TOTAL_REG,
} POWERIC_VOLTAGE_TUNE_REG;

typedef enum {
	TPS53915_POWERIC_MARGIN_LOW_EN = 0x0,
	TPS53915_POWERIC_MARGIN_HIGH_EN,
	TPS53915_POWERIC_MARGIN_OP_CNT,
} POWERIC_MARGIN_EN_BIT;

#if (BOARD_POWER == POWER_09V)
typedef enum {
	TPS53915_HIGHEST_VOLTAGE = 0x0,
	TPS53915_1P_10V = TPS53915_HIGHEST_VOLTAGE, //1.10V
	TPS53915_1P_09V,                            //1.09V
	TPS53915_1P_08V,                            //1.08V
	TPS53915_1P_07V,                            //1.07V
	TPS53915_1P_06V,                            //1.06V
	TPS53915_1P_05V,                            //1.05V
	TPS53915_1P_04V,                            //1.04V
	TPS53915_1P_03V,                            //1.03V
	TPS53915_1P_02V,                            //1.02V
	TPS53915_1P_01V,                            //1.01V
	TPS53915_1P_00V,                            //1.00V
	TPS53915_0P_99V,                            //0.99V
	TPS53915_0P_98V,                            //0.98V
	TPS53915_0P_97V,                            //0.97V
	TPS53915_0P_96V,                            //0.96V
	TPS53915_0P_95V,                            //0.95V
	TPS53915_0P_94V,                            //0.94V
	TPS53915_0P_93V,                            //0.93V
	TPS53915_0P_92V,                            //0.92V
	TPS53915_0P_91V,                            //0.91V
	TPS53915_0P_90V,                            //0.90V => 19
	TPS53915_0P_89V,                            //0.89V
	TPS53915_0P_88V,                            //0.88V
	TPS53915_0P_87V,                            //0.87V
	TPS53915_0P_86V,                            //0.86V
	TPS53915_0P_85V,                            //0.85V
	TPS53915_0P_84V,                            //0.84V
	TPS53915_0P_83V,                            //0.83V
	TPS53915_0P_82V,                            //0.82V
	TPS53915_0P_81V,                            //0.81V
	TPS53915_0P_80V,                            //0.80V
	TPS53915_0P_79V,                            //0.79V
	TPS53915_0P_78V,                            //0.78V
	TPS53915_0P_77V,                            //0.77V
	TPS53915_0P_76V,                            //0.76V
	TPS53915_0P_75V,                            //0.75V
	TPS53915_0P_74V,                            //0.74V
	TPS53915_0P_73V,                            //0.73V
	TPS53915_LOWEST_VOLTAGE = TPS53915_0P_73V,
	TPS53915_TOTAL_VOLTAGE_LEVEL,
	TPS53915_DEFAULT_VOLTAGE = TPS53915_0P_90V,
	ENUM_DUMMY4WORD(POWERIC_VOLTAGE_VAL)
} POWERIC_VOLTAGE_VAL;
STATIC_ASSERT(TPS53915_TOTAL_VOLTAGE_LEVEL <= 39);
typedef enum {
	TPS53915_INCREASE_VOLTAGE = TPS53915_TOTAL_VOLTAGE_LEVEL + 0x10,
	TPS53915_DECREASE_VOLTAGE,
	TPS53915_SEND_MANUAL_SETTING,
	ENUM_DUMMY4WORD(POWERIC_VOLTAGE_CHANGE_OP)
} POWERIC_VOLTAGE_CHANGE_OP;

typedef struct {
        UINT32  adjust_cmd_value;   //0xD4
        UINT32  margin_cmd_value;   //0xD5
//      float    margin_percentage;
//      float    adjust_percentage;
        float   actual_voltage;
        float   target_voltage;
} PowerIC_VoltageTBL, *pPowerIC_VoltageTBL;

#elif (BOARD_POWER == POWER_08V)
typedef enum {
        TPS53915_HIGHEST_VOLTAGE = 0x0,
        TPS53915_1P_00V = TPS53915_HIGHEST_VOLTAGE, //1.00V
        TPS53915_0P_99V,                            //0.99V
        TPS53915_0P_98V,                            //0.98V
        TPS53915_0P_97V,                            //0.97V
        TPS53915_0P_96V,                            //0.96V
        TPS53915_0P_95V,                            //0.95V
        TPS53915_0P_94V,                            //0.94V
        TPS53915_0P_93V,                            //0.93V
        TPS53915_0P_92V,                            //0.92V
        TPS53915_0P_91V,                            //0.91V
        TPS53915_0P_90V,                            //0.90V
        TPS53915_0P_89V,                            //0.89V
        TPS53915_0P_88V,                            //0.88V
        TPS53915_0P_87V,                            //0.87V
        TPS53915_0P_86V,                            //0.86V
        TPS53915_0P_85V,                            //0.85V
        TPS53915_0P_84V,                            //0.84V
        TPS53915_0P_83V,                            //0.83V
        TPS53915_0P_82V,                            //0.82V
        TPS53915_0P_81V,                            //0.81V
        TPS53915_0P_80V,                            //0.80V => typical
        TPS53915_0P_79V,                            //0.79V
        TPS53915_0P_78V,                            //0.78V
        TPS53915_0P_77V,                            //0.77V
        TPS53915_0P_76V,                            //0.76V
        TPS53915_0P_75V,                            //0.75V
        TPS53915_0P_74V,                            //0.74V
        TPS53915_0P_73V,                            //0.73V
        TPS53915_0P_72V,                            //0.72V
        TPS53915_0P_71V,                            //0.71V
        TPS53915_0P_70V,                            //0.70V
        TPS53915_0P_69V,                            //0.69V
        TPS53915_0P_68V,                            //0.68V
        TPS53915_0P_67V,                            //0.67V
        TPS53915_0P_66V,                            //0.66V
        TPS53915_0P_65V,                            //0.65V
	TPS53915_LOWEST_VOLTAGE = TPS53915_0P_65V,
        TPS53915_TOTAL_VOLTAGE_LEVEL,
        TPS53915_DEFAULT_VOLTAGE = TPS53915_0P_80V,
        ENUM_DUMMY4WORD(POWERIC_VOLTAGE_VAL)
} POWERIC_VOLTAGE_VAL;
STATIC_ASSERT(TPS53915_TOTAL_VOLTAGE_LEVEL <= 39);
typedef enum {
        TPS53915_INCREASE_VOLTAGE = TPS53915_TOTAL_VOLTAGE_LEVEL + 0x10,
        TPS53915_DECREASE_VOLTAGE,
	TPS53915_SEND_MANUAL_SETTING,
        ENUM_DUMMY4WORD(POWERIC_VOLTAGE_CHANGE_OP)
} POWERIC_VOLTAGE_CHANGE_OP;

typedef struct {
	UINT32  adjust_cmd_value;   //0xD4
	UINT32  margin_cmd_value;   //0xD5
//	float    margin_percentage;
//	float    adjust_percentage;
	float   actual_voltage;
	float   target_voltage;
	UINT32  margin_hi_low_enable;
} PowerIC_VoltageTBL, *pPowerIC_VoltageTBL;
#endif
#endif

extern BOOL     PwrIC_i2c_Open(void);
extern BOOL     PwrIC_Close(void);
extern BOOL     PwrIC_DumpAll(void);
extern BOOL     PwrIC_DoCmd(const UINT32 *pCmdList, UINT32 param, UINT32 *ret);

extern I2C_STS  PwrIC_Receive(UINT32 *value, UINT32 addr, BOOL bStop);
extern I2C_STS  PwrIC_Transmit(UINT32 value, UINT32 addr, BOOL bStop);
extern UINT32   PwrIC_ShowCurrentVoltage(void);
extern BOOL     PwrIC_i2c_ChangeCurrentVoltage(UINT32 op);

extern BOOL     PwrIC_Open(void);
extern BOOL     PwrIC_ChangeCurrentVoltage(UINT32 op);

//2022/04/15
extern BOOL PwrIC_ChangeVoltageWithSetting(UINT32 op, UINT32 adj, UINT32 margin);
extern void PwrIC_enableMarginLow(void);
extern void PwrIC_enableMarginHigh(void);
extern void PwrIC_switchMargin(UINT32 on_off);



extern UINT32 cmd_PwrIC_init[];
extern UINT32 cmd_PwrIC_read[];
extern UINT32 cmd_PwrIC_write[];
extern UINT32 cmd_Sensor_open[];
extern UINT32 cmd_Sensor_close[];
extern UINT32 cmd_LCD_setbl[];
extern UINT32 cmd_LCD_getbl[];
extern UINT32 cmd_Charge_en[];
extern UINT32 cmd_Charge_setCurrent[];
extern UINT32 cmd_Charge_geten[];
extern UINT32 cmd_Charge_getstat[];

// ------ reg 0x07 ----------
// 1. charge en
#define CHARGE_EN_MASK            0x08
#define CHARGE_EN_OFF             0x08
#define CHARGE_EN_ON              0x00
// 2. charge time
#define CHARGE_TIME_MASK          0xF0
#define CHARGE_TIME_1HOUR         0x00
#define CHARGE_TIME_2HOUR         0x10
#define CHARGE_TIME_3HOUR         0x20
#define CHARGE_TIME_4HOUR         0x30
#define CHARGE_TIME_5HOUR         0x40
#define CHARGE_TIME_6HOUR         0x50
#define CHARGE_TIME_7HOUR         0x60
#define CHARGE_TIME_8HOUR         0x70
#define CHARGE_TIME_9HOUR         0x80
#define CHARGE_TIME_10HOUR        0x90
#define CHARGE_TIME_11HOUR        0xA0
#define CHARGE_TIME_12HOUR        0xB0
#define CHARGE_TIME_13HOUR        0xC0
#define CHARGE_TIME_14HOUR        0xD0
#define CHARGE_TIME_15HOUR        0xE0
#define CHARGE_TIME_16HOUR        0xF0
// 3. charge input current limit
#define INPUT_CURRENT_LIMIT_MASK  0x03
#define INPUT_CURRENT_LIMIT_95mA  0x00
#define INPUT_CURRENT_LIMIT_475mA 0x02
#define INPUT_CURRENT_LIMIT_1A    0x01
#define INPUT_CURRENT_LIMIT_1_5A  0x03


//getstat: return reg 0x0A
#define CHARGE_STAT_MASK    0x1C
#define CHARGE_STAT_NOBATT  0x10
#define CHARGE_STAT_FULL    0x08
#define CHARGE_POWER_GOOD   0x04


#define BATT_TEMP_MASK      0xE0   // battery temperature mask
#define BATT_TEMP_UNDER_0   0xC0   // temperature < 0
#define BATT_TEMP_0_10      0x80   // temperature 0 ~ 10
#define BATT_TEMP_10_45     0x00   // temperature 10 ~ 45
#define BATT_TEMP_45_60     0x20   // temperature 45 ~ 60
#define BATT_TEMP_LARGE_60  0x60   // temperature > 60

#endif
