/*
 * tps53915-regulator.c - Voltage and current regulation for the TPS53915
 *
 * Copyright (C) 2018 NOVATEK MICROELECTRONICS CORP.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/libfdt.h>

#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>

#define MARGIN_HIGH_EN    0xA8
#define MARGIN_LOW_EN     0x98

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


#define POWER_08V               0
#define POWER_09V               1

#if defined(CONFIG_NVT_IVOT_PLAT_NS02301) || defined (CONFIG_NVT_IVOT_PLAT_NS02302)
#define BOARD_POWER             POWER_09V
#else
#define BOARD_POWER             POWER_08V
#endif

#if (BOARD_POWER == POWER_09V)
#if defined (CONFIG_NVT_IVOT_PLAT_NS02302)
enum {
	TPS53915_HIGHEST_VOLTAGE = 0x0,
	TPS53915_1P_14V = TPS53915_HIGHEST_VOLTAGE, //1.14V
	TPS53915_1P_13V,                            //1.13V
	TPS53915_1P_12V,                            //1.12V
	TPS53915_1P_11V,                            //1.11V
	TPS53915_1P_10V,                            //1.10V
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
	TPS53915_0P_95V,                            //0.95V => default
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
	TPS53915_0P_80V,                            //0.80V
	TPS53915_0P_79V,                            //0.79V
	TPS53915_0P_78V,                            //0.78V
	TPS53915_0P_77V,                            //0.77V
	TPS53915_0P_76V,                            //0.76V
	TPS53915_LOWEST_VOLTAGE = TPS53915_0P_76V,
	TPS53915_TOTAL_VOLTAGE_LEVEL,
	TPS53915_DEFAULT_VOLTAGE = TPS53915_0P_95V,
};
#endif

#if defined(CONFIG_NVT_IVOT_PLAT_NS02301)
enum {
	TPS53915_HIGHEST_VOLTAGE = 0x0,
	TPS53915_1P_11V = TPS53915_HIGHEST_VOLTAGE ,//1.11V
	TPS53915_1P_10V,                            //1.10V
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
	TPS53915_0P_95V,                            //0.95V => default
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
	TPS53915_0P_80V,                            //0.80V
	TPS53915_0P_79V,                            //0.79V
	TPS53915_0P_78V,                            //0.78V
	TPS53915_0P_77V,                            //0.77V
	TPS53915_LOWEST_VOLTAGE = TPS53915_0P_77V,
	TPS53915_TOTAL_VOLTAGE_LEVEL,
	TPS53915_DEFAULT_VOLTAGE = TPS53915_0P_95V,
};
#endif

typedef struct {
	u32 adjust_cmd_value;
	u32 margin_cmd_value;
	int actual_voltage;
	int target_voltage;

} poweric_voltagetbl;
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
	TPS53915_DEFAULT_VOLTAGE = TPS53915_0P_85V,
} POWERIC_VOLTAGE_VAL;

typedef struct {
	u32 adjust_cmd_value;
	u32 margin_cmd_value;
	int actual_voltage;
	int target_voltage;
	u8  margin_hi_low_enable;
} poweric_voltagetbl;
#endif

typedef enum {
	MP8870_HIGHEST_VOLTAGE = 0x0,
	MP8870_1P_10V = MP8870_HIGHEST_VOLTAGE, //1.10V
	MP8870_1P_09V,                            //1.09V
	MP8870_1P_08V,                            //1.08V
	MP8870_1P_07V,                            //1.07V
	MP8870_1P_06V,                            //1.06V
	MP8870_1P_05V,                            //1.05V
	MP8870_1P_04V,                            //1.04V
	MP8870_1P_03V,                            //1.03V
	MP8870_1P_02V,                            //1.02V
	MP8870_1P_01V,                            //1.01V
	MP8870_1P_00V,                            //1.00V

	MP8870_0P_99V,                            //0.99V
	MP8870_0P_98V,                            //0.98V
	MP8870_0P_97V,                            //0.97V
	MP8870_0P_96V,                            //0.96V
	MP8870_0P_95V,                            //0.95V
	MP8870_0P_94V,                            //0.94V
	MP8870_0P_93V,                            //0.93V
	MP8870_0P_92V,                            //0.92V
	MP8870_0P_91V,                            //0.91V
	MP8870_0P_90V,                            //0.90V => 19

	MP8870_0P_89V,                            //0.89V
	MP8870_0P_88V,                            //0.88V
	MP8870_0P_87V,                            //0.87V
	MP8870_0P_86V,                            //0.86V
	MP8870_0P_85V,                            //0.85V
	MP8870_0P_84V,                            //0.84V
	MP8870_0P_83V,                            //0.83V
	MP8870_0P_82V,                            //0.82V
	MP8870_0P_81V,                            //0.81V
	MP8870_0P_80V,                            //0.80V

	MP8870_0P_79V,                            //0.79V
	MP8870_0P_78V,                            //0.78V
	MP8870_0P_77V,                            //0.77V
	MP8870_0P_76V,                            //0.76V
	MP8870_0P_75V,                            //0.75V
	MP8870_0P_74V,                            //0.74V
	MP8870_LOWEST_VOLTAGE = MP8870_0P_74V,
	MP8870_TOTAL_VOLTAGE_LEVEL,

	MP8870_DEFAULT_VOLTAGE = MP8870_0P_90V,

} POWERIC_MP8870_VOLTAGE_VAL;

#if (BOARD_POWER == POWER_09V)

#if defined (CONFIG_NVT_IVOT_PLAT_NS02302)
poweric_voltagetbl  voltage_table[TPS53915_TOTAL_VOLTAGE_LEVEL] = {
	/*adj cmd(0xD4)  mgin cmd(0xD5) real vtg  desire vtg*/
	{0x1a,          0xB0,       1141000,    1140000},  //[00][1.14][v]
	{0x19,          0xB0,       1133000,    1130000},  //[01][1.13][v]
	{0x18,          0xB0,       1125000,    1120000},  //[02][1.12][v]
	{0x16,          0xB0,       1109000,    1110000},  //[03][1.11][v]
	{0x15,          0xB0,       1102000,    1100000},  //[04][1.10][v]
	{0x14,          0xB0,       1093000,    1090000},  //[05][1.09][v]
	{0x12,          0xB0,       1077000,    1080000},  //[06][1.08][v]
	{0x11,          0xB0,       1069000,    1070000},  //[07][1.07][v]
	{0x10,          0xB0,       1061000,    1060000},  //[08][1.06][v]
	{0x1c,          0x10,       1053000,    1050000},  //[09][1.05][v]
	{0x1b,          0x0B,       1037000,    1040000},  //[10][1.04][v]
	{0x1a,          0x0a,       1029000,    1030000},  //[11][1.03][v]
	{0x18,          0x0a,       1015000,    1020000},  //[12][1.02][v]
	{0x17,          0x07,       1008000,    1010000},  //[13][1.01][v]
	{0x16,          0x06,       1008000,    1000000},  //[14][1.00][v]
	{0x15,          0x05,        993000,     990000},  //[15][0.99][v]
	{0x14,          0x04,        986000,     980000},  //[16][0.98][v]
	{0x12,          0x02,        971000,     970000},  //[17][0.97][v]
	{0x11,          0x01,        964000,     960000},  //[18][0.96][v]
	{0x0E,          0x00,        949000,     950000},  //[19][0.95][v]<= default
	{0x0D,          0x01,        942000,     940000},  //[20][0.94][v]
	{0x0C,          0x00,        935000,     930000},  //[21][0.93][v]
	{0x0A,          0x00,        920000,     920000},  //[22][0.92][v]
	{0x09,          0x00,        913000,     910000},  //[23][0.91][v]
	{0x07,          0x00,        898000,     900000},  //[24][0.90][v]
	{0x06,          0x00,        891000,     890000},  //[25][0.89][v]
	{0x04,          0x00,        877000,     880000},  //[26][0.88][v]
	{0x03,          0x00,        869000,     870000},  //[27][0.87][v]
	{0x03,          0x01,        860000,     860000},  //[28][0.86][v]
	{0x03,          0x02,        851000,     850000},  //[29][0.85][v]
	{0x03,          0x03,        841000,     840000},  //[30][0.84][v]
	{0x03,          0x04,        833000,     830000},  //[31][0.83][v]
	{0x03,          0x06,        816000,     820000},  //[32][0.82][v]
	{0x03,          0x07,        807000,     810000},  //[33][0.81][v]
	{0x03,          0x08,        799000,     800000},  //[34][0.80][v]
	{0x03,          0x09,        791000,     790000},  //[35][0.79][v]
	{0x03,          0x0a,        783000,     780000},  //[36][0.78][v]
	{0x03,          0x0b,        775000,     770000},  //[37][0.77][v]
	{0x03,          0x0c,        768000,     760000},  //[38][0.76][v]
};
#endif

#if defined (CONFIG_NVT_IVOT_PLAT_NS02301)
poweric_voltagetbl  voltage_table[TPS53915_TOTAL_VOLTAGE_LEVEL] = {
//adj cmd(0xD4)  mgin cmd(0xD5) real vtg  desire vtg
	{0x15,          0xB0,       1101000,    1100000},  //[00][1.10][v]
	{0x14,          0xB0,       1093000,    1090000},  //[01][1.09][v]
	{0x12,          0xB0,       1077000,    1080000},  //[02][1.08][v]
	{0x11,          0xB0,       1069000,    1070000},  //[03][1.07][v]
	{0x10,          0xB0,       1061000,    1060000},  //[04][1.06][v]
	{0x1c,          0x10,       1052000,    1050000},  //[05][1.05][v]
	{0x1c,          0x01,       1043000,    1040000},  //[06][1.04][v]
	{0x1b,          0x0B,       1035000,    1030000},  //[07][1.03][v]
	{0x1a,          0x0a,       1028000,    1020000},  //[08][1.02][v]
	{0x18,          0x08,       1014000,    1010000},  //[09][1.01][v]
	{0x17,          0x07,       1007000,    1000000},  //[10][1.00][v]
	{0x15,          0x05,        993000,     990000},  //[11][0.99][v]
	{0x14,          0x04,        985000,     980000},  //[12][0.98][v]
	{0x12,          0x02,        971000,     970000},  //[13][0.97][v]
	{0x11,          0x01,        963000,     960000},  //[14][0.96][v]
	{0x10,          0x00,        956000,     950000},  //[15][0.95][v]<= default
	{0x0D,          0x01,        942000,     940000},  //[16][0.94][v]
	{0x0C,          0x00,        934000,     930000},  //[17][0.93][v]
	{0x0A,          0x00,        920000,     920000},  //[18][0.92][v]
	{0x09,          0x00,        913000,     910000},  //[19][0.91][v]
	{0x07,          0x00,        898000,     900000},  //[20][0.90][v]
	{0x06,          0x00,        891000,     890000},  //[21][0.89][v]
	{0x04,          0x00,        877000,     880000},  //[22][0.88][v]
	{0x03,          0x00,        869000,     870000},  //[23][0.87][v]
	{0x03,          0x01,        860000,     860000},  //[24][0.86][v]
	{0x03,          0x02,        851000,     850000},  //[25][0.85][v]
	{0x03,          0x03,        841000,     840000},  //[26][0.84][v]
	{0x03,          0x04,        832000,     830000},  //[27][0.83][v]
	{0x03,          0x05,        824000,     820000},  //[28][0.82][v]
	{0x03,          0x06,        815000,     810000},  //[29][0.81][v]
	{0x03,          0x08,        799000,     800000},  //[30][0.80][v]
	{0x03,          0x09,        791000,     790000},  //[31][0.79][v]
	{0x03,          0x0a,        783000,     780000},  //[32][0.78][v]
	{0x03,          0x0c,        767000,     770000},  //[33][0.77][v]
};
#endif

#elif (BOARD_POWER == POWER_08V)
poweric_voltagetbl  voltage_table[TPS53915_TOTAL_VOLTAGE_LEVEL] = {
	//adj cmd(0xD4)          mgin cmd(0xD5)              real vtg    desire vtg    margin Hi/Low enable
	{TPS53915_ADJ_P9_00,     TPS53915_MARG_P12_00,      997100,   1000000,        MARGIN_HIGH_EN },  //[00][1.00]
	{TPS53915_ADJ_P8_25,     TPS53915_MARG_P12_00,      990200,   990000,         MARGIN_HIGH_EN },  //[01][0.99]
	{TPS53915_ADJ_P8_25,     TPS53915_MARG_P10_90,      981400,   980000,         MARGIN_HIGH_EN },  //[02][0.98]
	{TPS53915_ADJ_P8_25,     TPS53915_MARG_P09_90,      971500,   970000,         MARGIN_HIGH_EN },  //[03][0.97]
	{TPS53915_ADJ_P6_00,     TPS53915_MARG_P10_90,      961000,   960000,         MARGIN_HIGH_EN },  //[04][0.96]
	{TPS53915_ADJ_P6_00,     TPS53915_MARG_P09_90,      951500,   950000,         MARGIN_HIGH_EN },  //[05][0.95]
	{TPS53915_ADJ_P9_00,     TPS53915_MARG_P05_70,      940000,   940000,         MARGIN_HIGH_EN },  //[06][0.94]
	{TPS53915_ADJ_P6_75,     TPS53915_MARG_P06_70,      929600,   930000,         MARGIN_HIGH_EN },  //[07][0.93]
	{TPS53915_ADJ_P6_75,     TPS53915_MARG_P05_70,      920600,   920000,         MARGIN_HIGH_EN },  //[08][0.92]
	{TPS53915_ADJ_P4_50,     TPS53915_MARG_P06_70,      910000,   910000,         MARGIN_HIGH_EN },  //[09][0.91]
	{TPS53915_ADJ_P5_25,     TPS53915_MARG_P04_70,      899000,   900000,         MARGIN_HIGH_EN },  //[10][0.90]
	{TPS53915_ADJ_P9_00,     TPS53915_MARG_P00_00,      889700,   890000,         MARGIN_HIGH_EN },  //[11][0.89]
	{TPS53915_ADJ_P9_00,     TPS53915_MARG_N01_10,      880200,   880000,         MARGIN_LOW_EN  },  //[12][0.88]
	{TPS53915_ADJ_P3_75,     TPS53915_MARG_P02_80,      870700,   870000,         MARGIN_HIGH_EN },  //[13][0.87]
	{TPS53915_ADJ_P4_50,     TPS53915_MARG_P00_90,      860800,   860000,         MARGIN_HIGH_EN },  //[14][0.86]
	{TPS53915_ADJ_N1_50,     TPS53915_MARG_P05_70,      849200,   850000,         MARGIN_HIGH_EN },  //[15][0.85]
	{TPS53915_ADJ_N0_75,     TPS53915_MARG_P03_70,      839400,   840000,         MARGIN_HIGH_EN },  //[16][0.84]
	{TPS53915_ADJ_N3_75,     TPS53915_MARG_P05_70,      829600,   830000,         MARGIN_HIGH_EN },  //[17][0.83]
	{TPS53915_ADJ_N2_25,     TPS53915_MARG_P02_80,      819800,   820000,         MARGIN_HIGH_EN },  //[18][0.82]
	{TPS53915_ADJ_N5_25,     TPS53915_MARG_P04_70,      808500,   810000,         MARGIN_HIGH_EN },  //[19][0.81]
	{TPS53915_ADJ_P4_50,     TPS53915_MARG_N06_20,      800500,   800000,         MARGIN_LOW_EN  },  //[20][0.80]<= default
	{TPS53915_ADJ_P0_00,     TPS53915_MARG_N03_20,      790100,   790000,         MARGIN_LOW_EN  },  //[21][0.79]
	{TPS53915_ADJ_P0_75,     TPS53915_MARG_N05_20,      779900,   780000,         MARGIN_LOW_EN  },  //[22][0.78]
	{TPS53915_ADJ_P6_75,     TPS53915_MARG_N11_60,      770300,   770000,         MARGIN_LOW_EN  },  //[23][0.77]
	{TPS53915_ADJ_N0_75,     TPS53915_MARG_N06_20,      759900,   760000,         MARGIN_LOW_EN  },  //[24][0.76]
	{TPS53915_ADJ_N9_00,     TPS53915_MARG_P00_90,      748800,   750000,         MARGIN_HIGH_EN },  //[25][0.75]
	{TPS53915_ADJ_N8_25,     TPS53915_MARG_N01_10,      740200,   740000,         MARGIN_LOW_EN  },  //[26][0.74]
	{TPS53915_ADJ_N3_75,     TPS53915_MARG_N07_10,      729400,   730000,         MARGIN_LOW_EN  },  //[27][0.73]
	{TPS53915_ADJ_N6_00,     TPS53915_MARG_N06_20,      719300,   720000,         MARGIN_LOW_EN  },  //[28][0.72]
	{TPS53915_ADJ_N1_50,     TPS53915_MARG_N11_60,      710700,   710000,         MARGIN_LOW_EN  },  //[29][0.71]
	{TPS53915_ADJ_N3_00,     TPS53915_MARG_N11_60,      699700,   700000,         MARGIN_LOW_EN  },  //[30][0.70]
	{TPS53915_ADJ_N9_00,     TPS53915_MARG_N07_10,      689300,   690000,         MARGIN_LOW_EN  },  //[31][0.69]
	{TPS53915_ADJ_N7_50,     TPS53915_MARG_N09_90,      679900,   680000,         MARGIN_LOW_EN  },  //[32][0.68]
	{TPS53915_ADJ_N9_00,     TPS53915_MARG_N09_90,      668800,   670000,         MARGIN_LOW_EN  },  //[33][0.67]
	{TPS53915_ADJ_N9_00,     TPS53915_MARG_N10_70,      662300,   660000,         MARGIN_LOW_EN  },  //[34][0.66]
	{TPS53915_ADJ_N9_00,     TPS53915_MARG_N11_60,      656100,   650000,         MARGIN_LOW_EN  }   //[35][0.65]
};
#endif

#define V_COMMAND_VAL(v)	(unsigned int)(((v)*1024))

poweric_voltagetbl  voltage_table_8870[MP8870_TOTAL_VOLTAGE_LEVEL] = {
//adj cmd(0x21)  		mgin cmd(N/A) 	real vtg  desire vtg
	{V_COMMAND_VAL(110),          0xB0,       1102700,    1100000},
	{V_COMMAND_VAL(109),          0xB0,       1092600,    1090000},
	{V_COMMAND_VAL(108),          0xB0,       1082500,    1080000},
	{V_COMMAND_VAL(107),          0xB0,       1072500,    1070000},
	{V_COMMAND_VAL(106),          0xB0,       1062500,    1060000},
	{V_COMMAND_VAL(105),          0x10,       1052500,    1050000},
	{V_COMMAND_VAL(104),          0x01,       1042500,    1040000},
	{V_COMMAND_VAL(103),          0x0B,       1032500,    1030000},
	{V_COMMAND_VAL(102),          0x0a,       1022500,    1020000},
	{V_COMMAND_VAL(101),          0x08,       1012500,    1010000},
	{V_COMMAND_VAL(100),          0x07,       1002500,    1000000},
	{V_COMMAND_VAL( 99),          0x05,       992500,     990000},
	{V_COMMAND_VAL( 98),          0x04,       982500,     980000},
	{V_COMMAND_VAL( 97),          0x02,       972300,     970000},
	{V_COMMAND_VAL( 96),          0x01,       962300,     960000},
	{V_COMMAND_VAL( 95),          0x00,       952100,     950000},
	{V_COMMAND_VAL( 94),          0x01,       942100,     940000},
	{V_COMMAND_VAL( 93),          0x00,       932200,     930000},
	{V_COMMAND_VAL( 92),          0x00,       922000,     920000},
	{V_COMMAND_VAL( 91),          0x00,       912000,     910000},
	{V_COMMAND_VAL( 90),          0x00,       901900,     900000},//<-default
	{V_COMMAND_VAL( 89),          0x00,       891900,     890000},
	{V_COMMAND_VAL( 88),          0x00,       881800,     880000},
	{V_COMMAND_VAL( 87),          0x00,       871800,     870000},
	{V_COMMAND_VAL( 86),          0x01,       861700,     860000},
	{V_COMMAND_VAL( 85),          0x02,       855000,     850000},
	{V_COMMAND_VAL( 84),          0x03,       841500,     840000},
	{V_COMMAND_VAL( 83),          0x04,       834800,     830000},
	{V_COMMAND_VAL( 82),          0x05,       821400,     820000},
	{V_COMMAND_VAL( 81),          0x06,       814700,     810000},
	{V_COMMAND_VAL( 80),          0x08,       804700,     800000},
	{V_COMMAND_VAL( 79),          0x00,       794600,     790000},
	{V_COMMAND_VAL( 78),          0x00,       784600,     780000},
	{V_COMMAND_VAL( 77),          0x00,       774400,     770000},
	{V_COMMAND_VAL( 76),          0x01,       764400,     760000},
	{V_COMMAND_VAL( 75),          0x02,       754200,     750000},
	{V_COMMAND_VAL( 74),          0x03,       744200,     740000},
};

enum tps53915_types {
	TYPE_TPS53915_CORE,
	TYPE_TPS53915_CNN,
	TYPE_TPS53915_CPU,
	TYPE_TPS53915_DSP,
	TYPE_TPSMP8870_MAIN,
	TYPE_TPSMP8870_PREROLL,
};


struct tps53915_data {
	struct i2c_client   *client;
	int margin_status;
};

static int tps53915_read_reg(struct tps53915_data *tps53915, u8 reg)
{
	int ret = i2c_smbus_read_byte_data(tps53915->client, reg);

	if (ret > 0) {
		ret &= 0xff;
	}

	return ret;
}

static int tps53915_write_reg(struct tps53915_data *tps53915,
							  u8 reg, u8 value)
{
	return i2c_smbus_write_byte_data(tps53915->client, reg, value);
}

static int tpsMP8870_write_reg(struct tps53915_data *tps53915,
							  u8 reg, u32 value)
{
	return i2c_smbus_write_word_data(tps53915->client, reg, value);
}

static int tpsMP8870_set_voltage_sel(struct regulator_dev *rdev,
									unsigned selector)
{
	struct tps53915_data *tps53915 = rdev_get_drvdata(rdev);
	const struct regulator_desc *desc = rdev->desc;
	u32 adjust_cmd;
	int ret = 0;

	adjust_cmd = voltage_table_8870[selector].adjust_cmd_value;
	adjust_cmd = adjust_cmd / desc->enable_val;

	ret = tpsMP8870_write_reg(tps53915, 0x21, adjust_cmd);

	return ret;
}

static int tpsMP8870_map_voltage(struct regulator_dev *rdev, int min_uV, int max_uV)
{
	int cnt;

	for (cnt = 0; cnt < MP8870_TOTAL_VOLTAGE_LEVEL; cnt++) {
		if ((max_uV >= voltage_table_8870[cnt].target_voltage) && \
			(min_uV <= voltage_table_8870[cnt].target_voltage)) {
			break;
		}
	}

	return cnt;
}

static int tpsMP8870_list_voltage(struct regulator_dev *rdev,
								 unsigned int selector)
{
	if (rdev_get_id(rdev) != 0) {
		return -EINVAL;
	}

	return voltage_table_8870[selector].target_voltage;
}

static int tpsMP8870_get_voltage_sel(struct regulator_dev *rdev)
{
	struct tps53915_data *tps53915 = rdev_get_drvdata(rdev);
	u32 val, val_margin;
	int cnt;

	val = tps53915_read_reg(tps53915, 0xD4);
	val_margin = tps53915_read_reg(tps53915, 0xD5);

	for (cnt = 0; cnt < MP8870_TOTAL_VOLTAGE_LEVEL; cnt++) {
		if ((val == voltage_table_8870[cnt].adjust_cmd_value) && \
			(val_margin == voltage_table_8870[cnt].margin_cmd_value)) {
			break;
		}
	}

	return cnt;
}

static int tps53915_set_init_value(struct tps53915_data *tps53915)
{
	int ret = 0;
	u32 adjust_cmd, margin_cmd;

	ret = tps53915_write_reg(tps53915, 0x10, 0);
	if (ret) {
		return -EINVAL;
	}

	ret = tps53915_write_reg(tps53915, 0x1, MARGIN_HIGH_EN);
	if (ret) {
		return -EINVAL;
	}

	tps53915->margin_status = 1;

	ret = tps53915_write_reg(tps53915, 0x2, 0xF);
	if (ret) {
		return -EINVAL;
	}

	margin_cmd = voltage_table[TPS53915_DEFAULT_VOLTAGE].margin_cmd_value;
	adjust_cmd = voltage_table[TPS53915_DEFAULT_VOLTAGE].adjust_cmd_value;

	/* Set TPS53915 1MHz */
	ret = tps53915_write_reg(tps53915, 0xD3, 0x7);
	if (ret) {
		printk("Set tps53915 freq fail\r\n");
		return -EINVAL;
	}

	if (tps53915_write_reg(tps53915, 0xD5, margin_cmd) == 0) {
		ret = tps53915_write_reg(tps53915, 0xD4, adjust_cmd);
	}

	return ret;
}


static int tps53915_list_voltage(struct regulator_dev *rdev,
								 unsigned int selector)
{
	if (rdev_get_id(rdev) != 0) {
		return -EINVAL;
	}

	return voltage_table[selector].target_voltage;
}

static int tps53915_get_voltage_sel(struct regulator_dev *rdev)
{
	struct tps53915_data *tps53915 = rdev_get_drvdata(rdev);
	u32 val, val_margin;
	int cnt;

	val = tps53915_read_reg(tps53915, 0xD4);
	val_margin = tps53915_read_reg(tps53915, 0xD5);

	for (cnt = 0; cnt < TPS53915_TOTAL_VOLTAGE_LEVEL; cnt++) {
		if ((val == voltage_table[cnt].adjust_cmd_value) && \
			(val_margin == voltage_table[cnt].margin_cmd_value)) {
			break;
		}
	}

	return cnt;
}

static int tps53915_set_voltage_sel(struct regulator_dev *rdev,
									unsigned selector)
{
	struct tps53915_data *tps53915 = rdev_get_drvdata(rdev);
	u32 margin_cmd, adjust_cmd;
	int ret = 0;
	u32 val, val_margin;
	int cnt;

	margin_cmd = voltage_table[selector].margin_cmd_value;
	adjust_cmd = voltage_table[selector].adjust_cmd_value;

	val = tps53915_read_reg(tps53915, 0xD4);
	val_margin = tps53915_read_reg(tps53915, 0xD5);

	for (cnt = 0; cnt < TPS53915_TOTAL_VOLTAGE_LEVEL; cnt++) {
		if ((val == voltage_table[cnt].adjust_cmd_value) && \
			(val_margin == voltage_table[cnt].margin_cmd_value)) {
			break;
		}
	}
#if (BOARD_POWER == POWER_09V)
	if (selector >= TPS53915_0P_84V) {
		if (tps53915->margin_status == 1) {
			ret = tps53915_write_reg(tps53915, 0x01, MARGIN_LOW_EN);
			tps53915->margin_status = 0;
		}
	} else {
		if (tps53915->margin_status == 0) {
			ret = tps53915_write_reg(tps53915, 0x01, MARGIN_HIGH_EN);
			tps53915->margin_status = 1;
		}
	}
#elif (BOARD_POWER == POWER_08V)
	ret = tps53915_write_reg(tps53915, 0x01, voltage_table[selector].margin_hi_low_enable);
#endif


	if (ret) {
		dev_err(&tps53915->client->dev, "i2c write error\n");
		return ret;
	}

	if (tps53915_write_reg(tps53915, 0xD4, adjust_cmd) == 0) {
		ret = tps53915_write_reg(tps53915, 0xD5, margin_cmd);
	}

	return ret;
}

static int tps53915_map_voltage(struct regulator_dev *rdev, int min_uV, int max_uV)
{
	int cnt;

	for (cnt = 0; cnt < TPS53915_TOTAL_VOLTAGE_LEVEL; cnt++) {
		if ((max_uV >= voltage_table[cnt].target_voltage) && \
			(min_uV <= voltage_table[cnt].target_voltage)) {
			break;
		}
	}

	return cnt;
}

static struct regulator_ops tps53915_ops = {
	.list_voltage       = tps53915_list_voltage,
	.get_voltage_sel    = tps53915_get_voltage_sel,
	.set_voltage_sel    = tps53915_set_voltage_sel,
	.map_voltage        = tps53915_map_voltage,
};

static struct regulator_ops tpsMP8870_ops = {
	.list_voltage       = tpsMP8870_list_voltage,
	.get_voltage_sel    = tpsMP8870_get_voltage_sel,
	.set_voltage_sel    = tpsMP8870_set_voltage_sel,
	.map_voltage        = tpsMP8870_map_voltage,
};

static const struct regulator_desc regulator_core = {
	.name       = "tps53915_vout_core",
	.id     = 0,
	.n_voltages = TPS53915_TOTAL_VOLTAGE_LEVEL,
	.ops        = &tps53915_ops,
	.type       = REGULATOR_VOLTAGE,
	.owner      = THIS_MODULE,
};

static const struct regulator_desc regulator_cnn = {
	.name       = "tps53915_vout_cnn",
	.id     = 0,
	.n_voltages = TPS53915_TOTAL_VOLTAGE_LEVEL,
	.ops        = &tps53915_ops,
	.type       = REGULATOR_VOLTAGE,
	.owner      = THIS_MODULE,
};

static const struct regulator_desc regulator_cpu = {
	.name       = "tps53915_vout_cpu",
	.id     = 0,
	.n_voltages = TPS53915_TOTAL_VOLTAGE_LEVEL,
	.ops        = &tps53915_ops,
	.type       = REGULATOR_VOLTAGE,
	.owner      = THIS_MODULE,
};

static const struct regulator_desc regulator_dsp = {
	.name       = "tps53915_vout_dsp",
	.id     = 0,
	.n_voltages = TPS53915_TOTAL_VOLTAGE_LEVEL,
	.ops        = &tps53915_ops,
	.type       = REGULATOR_VOLTAGE,
	.owner      = THIS_MODULE,
};

static struct regulator_desc regulator_mp8870_main = {
	.name       = "tpsMP8870_vout_main",
	.id     = 0,
	.n_voltages = MP8870_TOTAL_VOLTAGE_LEVEL,
	.ops        = &tpsMP8870_ops,
	.type       = REGULATOR_VOLTAGE,
	.owner      = THIS_MODULE,
};

static struct regulator_desc regulator_mp8870_preroll = {
	.name       = "tpsMP8870_vout_preroll",
	.id     = 0,
	.n_voltages = MP8870_TOTAL_VOLTAGE_LEVEL,
	.ops        = &tpsMP8870_ops,
	.type       = REGULATOR_VOLTAGE,
	.owner      = THIS_MODULE,
};

static struct regulator_init_data tps53915_init_data = {
	.constraints = {
#if (BOARD_POWER == POWER_09V)
		.max_uV         = 1130000,
		.min_uV         = 740000,
#elif (BOARD_POWER == POWER_08V)
		.max_uV         = 1000000,
		.min_uV         = 650000,
#endif
		.valid_ops_mask     = REGULATOR_CHANGE_VOLTAGE,
	},
};

static const struct of_device_id tps53915_dt_match[] = {
	{ .compatible = "nvt,tps53915-core", .data = (void *)TYPE_TPS53915_CORE },
	{ .compatible = "nvt,tps53915-cnn", .data = (void *)TYPE_TPS53915_CNN},
	{ .compatible = "nvt,tps53915-cpu", .data = (void *)TYPE_TPS53915_CPU},
	{ .compatible = "nvt,tps53915-dsp", .data = (void *)TYPE_TPS53915_DSP},
	{ .compatible = "nvt,tpsMP8870-main", .data = (void *)TYPE_TPSMP8870_MAIN},
	{ .compatible = "nvt,tpsMP8870-preroll", .data = (void *)TYPE_TPSMP8870_PREROLL},
	{},
};
MODULE_DEVICE_TABLE(of, tps53915_dt_match);

static int tps53915_pmic_probe(struct i2c_client *client,
							   const struct i2c_device_id *i2c_id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct regulator_config config = { };
	struct tps53915_data *tps53915;
	struct regulator_dev *rdev;
	const struct of_device_id *match;
	unsigned int prop_val;
	int ret = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		return -EIO;
	}

	tps53915 = devm_kzalloc(&client->dev, sizeof(struct tps53915_data),
							GFP_KERNEL);
	if (!tps53915) {
		return -ENOMEM;
	}

	tps53915->client = client;
	tps53915->margin_status = 0;

	config.dev = &client->dev;
	config.driver_data = tps53915;
	config.of_node = client->dev.of_node;
	config.init_data = &tps53915_init_data;

	match = of_match_node(tps53915_dt_match, client->dev.of_node);
	if (!match) {
		dev_err(&client->dev, "Failed to get of_match_node\n");
		return -EINVAL;
	}

	switch ((unsigned long)match->data) {
	case TYPE_TPS53915_CORE:
		rdev = devm_regulator_register(&client->dev, &regulator_core, &config);
		if (IS_ERR(rdev)) {
			ret = PTR_ERR(rdev);
			dev_err(&client->dev, "regulator init failed (%d)\n", ret);
			return ret;
		}
		break;
	case TYPE_TPS53915_CNN:
		rdev = devm_regulator_register(&client->dev, &regulator_cnn, &config);
		if (IS_ERR(rdev)) {
			ret = PTR_ERR(rdev);
			dev_err(&client->dev, "regulator init failed (%d)\n", ret);
			return ret;
		}
		break;
	case TYPE_TPS53915_CPU:
		rdev = devm_regulator_register(&client->dev, &regulator_cpu, &config);
		if (IS_ERR(rdev)) {
			ret = PTR_ERR(rdev);
			dev_err(&client->dev, "regulator init failed (%d)\n", ret);
			return ret;
		}
		break;

	case TYPE_TPS53915_DSP:
		rdev = devm_regulator_register(&client->dev, &regulator_dsp, &config);
		if (IS_ERR(rdev)) {
			ret = PTR_ERR(rdev);
			dev_err(&client->dev, "regulator init failed (%d)\n", ret);
			return ret;
		}
		break;
	case TYPE_TPSMP8870_MAIN:
		ret = of_property_read_u32(client->dev.of_node, "vpcb", &prop_val);
		regulator_mp8870_main.enable_val = prop_val;
		rdev = devm_regulator_register(&client->dev, &regulator_mp8870_main, &config);
		if (IS_ERR(rdev)) {
			ret = PTR_ERR(rdev);
			dev_err(&client->dev, "regulator init failed (%d)\n", ret);
			return ret;
		}
		break;
	case TYPE_TPSMP8870_PREROLL:
		ret = of_property_read_u32(client->dev.of_node, "vpcb", &prop_val);
		regulator_mp8870_preroll.enable_val = prop_val;
		rdev = devm_regulator_register(&client->dev, &regulator_mp8870_preroll, &config);
		if (IS_ERR(rdev)) {
			ret = PTR_ERR(rdev);
			dev_err(&client->dev, "regulator init failed (%d)\n", ret);
			return ret;
		}
		break;
	}


	i2c_set_clientdata(client, tps53915);

	if ((unsigned long)match->data != TYPE_TPSMP8870_PREROLL && (unsigned long)match->data != TYPE_TPSMP8870_MAIN) {
		if (tps53915_set_init_value(tps53915)) {
			dev_err(&client->dev, "init tps53915 failed\n");
			return -EINVAL;
		}
	}

	return 0;
}


static struct i2c_driver tps53915_pmic_driver = {
	.probe      = tps53915_pmic_probe,
	.driver     = {
		.name   = "tps53915",
		.of_match_table = of_match_ptr(tps53915_dt_match),
	},
};

static int __init tps53915_pmic_init(void)
{
	return i2c_add_driver(&tps53915_pmic_driver);
}
subsys_initcall(tps53915_pmic_init);

static void __exit tps53915_pmic_exit(void)
{
	i2c_del_driver(&tps53915_pmic_driver);
}
module_exit(tps53915_pmic_exit);

MODULE_DESCRIPTION("TPS53915 voltage regulator driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.00.002");
