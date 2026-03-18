/**
    GPIO header file
    This file will setup gpio related base address
    @file       nvt-gpio.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef __ASM_ARCH_NS02302_NVT_GPIO_H
#define __ASM_ARCH_NS02302_NVT_GPIO_H
#include <kwrap/nvt_type.h>
#include "../comm/driver.h"
/**
    @addtogroup mIDrvIO_GPIO
*/
//@{

/**
    GPIO direction

    GPIO direction definition for gpio_setDir() and gpio_getDir()
*/
typedef enum {
	GPIO_DIR_INPUT =            0,      ///< GPIO is input direction
	GPIO_DIR_OUTPUT =           1,      ///< GPIO is output direction

	ENUM_DUMMY4WORD(GPIO_DIR)
} GPIO_DIR;

/**
    GPIO interrupt type

    GPIO interrupt type definition for type argument of gpio_setIntTypePol()
*/
typedef enum {
	GPIO_INTTYPE_EDGE =         0,      ///< GPIO interrupt is edge trigger
	GPIO_INTTYPE_LEVEL =        1,      ///< GPIO interrupt is level trigger

	ENUM_DUMMY4WORD(GPIO_INTTYPE)
} GPIO_INTTYPE;

/**
    GPIO interrupt polarity

    GPIO interrupt polarity definition for pol argument of gpio_setIntTypePol()
*/
typedef enum {
	GPIO_INTPOL_POSHIGH =       0,      ///< GPIO interrupt polarity is \n
	///< - @b positvie edge for edge trigger
	///< - @b high level for level trigger
	GPIO_INTPOL_NEGLOW =        1,      ///< GPIO interrupt polarity is \n
	///< - @b negative edge for edge trigger
	///< - @b low level for level trigger
	GPIO_INTPOL_BOTHEDGE =      2,      ///< GPIO interrupt polarity is \n
	///< - @b both edge for edge trigger
	ENUM_DUMMY4WORD(GPIO_INTPOL)
} GPIO_INTPOL;



/**
    @name   GPIO pins ID

    GPIO pins ID definition

    For detail GPIO pin out, please refer to NT96650 data sheet.
*/
//@{
/*Storage GPIO - CGPIO*/
/**
    @name   GPIO pins ID

    GPIO pins ID definition

    For detail GPIO pin out, please refer to NT96520 data sheet.
*/
//@{
/*Storage GPIO - CGPIO*/
#define C_GPIO_0    0                       ///< C_GPIO[0]
#define C_GPIO_1    1                       ///< C_GPIO[1]
#define C_GPIO_2    2                       ///< C_GPIO[2]
#define C_GPIO_3    3                       ///< C_GPIO[3]
#define C_GPIO_4    4                       ///< C_GPIO[4]
#define C_GPIO_5    5                       ///< C_GPIO[5]
#define C_GPIO_6    6                       ///< C_GPIO[6]
#define C_GPIO_7    7                       ///< C_GPIO[7]
#define C_GPIO_8    8                       ///< C_GPIO[8]
#define C_GPIO_9    9                       ///< C_GPIO[9]
#define C_GPIO_10   10                      ///< C_GPIO[10]
#define C_GPIO_11   11                      ///< C_GPIO[11]
#define C_GPIO_12   12                      ///< C_GPIO[12]
#define C_GPIO_13   13                      ///< C_GPIO[13]
#define C_GPIO_14   14                      ///< C_GPIO[14]
#define C_GPIO_15   15                      ///< C_GPIO[15]
#define C_GPIO_16   16                      ///< C_GPIO[16]
#define C_GPIO_17   17                      ///< C_GPIO[17]
#define C_GPIO_18   18                      ///< C_GPIO[18]
#define C_GPIO_19   19                      ///< C_GPIO[19]
#define C_GPIO_20   20                      ///< C_GPIO[20]
#define C_GPIO_21   21                      ///< C_GPIO[21]
#define C_GPIO_22   22                      ///< C_GPIO[22]
#define C_GPIO_23   23                      ///< C_GPIO[23]

/*Peripheral GPIO - PGPIO*/
#define P_GPIO_SHIFT_BASE 32
#define P_GPIO_0    (0 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[0]
#define P_GPIO_1    (1 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[1]
#define P_GPIO_2    (2 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[2]
#define P_GPIO_3    (3 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[3]
#define P_GPIO_4    (4 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[4]
#define P_GPIO_5    (5 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[5]
#define P_GPIO_6    (6 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[6]
#define P_GPIO_7    (7 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[7]
#define P_GPIO_8    (8 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[8]
#define P_GPIO_9    (9 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[9]
#define P_GPIO_10   (10+P_GPIO_SHIFT_BASE)  ///< P_GPIO[10]
#define P_GPIO_11   (11+P_GPIO_SHIFT_BASE)  ///< P_GPIO[11]
#define P_GPIO_12   (12+P_GPIO_SHIFT_BASE)  ///< P_GPIO[12]
#define P_GPIO_13   (13+P_GPIO_SHIFT_BASE)  ///< P_GPIO[13]
#define P_GPIO_14   (14+P_GPIO_SHIFT_BASE)  ///< P_GPIO[14]
#define P_GPIO_15   (15+P_GPIO_SHIFT_BASE)  ///< P_GPIO[15]
#define P_GPIO_16   (16+P_GPIO_SHIFT_BASE)  ///< P_GPIO[16]
#define P_GPIO_17   (17+P_GPIO_SHIFT_BASE)  ///< P_GPIO[17]
#define P_GPIO_18   (18+P_GPIO_SHIFT_BASE)  ///< P_GPIO[18]
#define P_GPIO_19   (19+P_GPIO_SHIFT_BASE)  ///< P_GPIO[19]
#define P_GPIO_20   (20+P_GPIO_SHIFT_BASE)  ///< P_GPIO[20]
#define P_GPIO_21   (21+P_GPIO_SHIFT_BASE)  ///< P_GPIO[21]
#define P_GPIO_22   (22+P_GPIO_SHIFT_BASE)  ///< P_GPIO[22]
#define P_GPIO_23   (23+P_GPIO_SHIFT_BASE)  ///< P_GPIO[23]
#define P_GPIO_24   (24+P_GPIO_SHIFT_BASE)  ///< P_GPIO[24]
#define P_GPIO_25   (25+P_GPIO_SHIFT_BASE)  ///< P_GPIO[25]

/*GPIO SGPIO Data register*/
#define S_GPIO_SHIFT_BASE 64
#define S_GPIO_0    (0 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[0]
#define S_GPIO_1    (1 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[1]
#define S_GPIO_2    (2 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[2]
#define S_GPIO_3    (3 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[3]
#define S_GPIO_4    (4 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[4]
#define S_GPIO_5    (5 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[5]
#define S_GPIO_6    (6 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[6]
#define S_GPIO_7    (7 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[7]
#define S_GPIO_8    (8 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[8]
#define S_GPIO_9    (9 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[9]
#define S_GPIO_10   (10+S_GPIO_SHIFT_BASE)  ///< S_GPIO[10]
#define S_GPIO_11   (11+S_GPIO_SHIFT_BASE)  ///< S_GPIO[11]
#define S_GPIO_12   (12+S_GPIO_SHIFT_BASE)  ///< S_GPIO[12]
#define S_GPIO_13   (13+S_GPIO_SHIFT_BASE)  ///< S_GPIO[13]
#define S_GPIO_14   (14+S_GPIO_SHIFT_BASE)  ///< S_GPIO[14]
#define S_GPIO_15   (15+S_GPIO_SHIFT_BASE)  ///< S_GPIO[15]

/*GPIO LGPIO Data register*/
#define L_GPIO_SHIFT_BASE 96
#define L_GPIO_0    (0 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[0]
#define L_GPIO_1    (1 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[1]
#define L_GPIO_2    (2 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[2]
#define L_GPIO_3    (3 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[3]
#define L_GPIO_4    (4 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[4]
#define L_GPIO_5    (5 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[5]
#define L_GPIO_6    (6 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[6]
#define L_GPIO_7    (7 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[7]
#define L_GPIO_8    (8 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[8]
#define L_GPIO_9    (9 +L_GPIO_SHIFT_BASE)  ///< L_GPIO[9]
#define L_GPIO_10   (10+L_GPIO_SHIFT_BASE)  ///< L_GPIO[10]
#define L_GPIO_11   (11+L_GPIO_SHIFT_BASE)  ///< L_GPIO[11]
#define L_GPIO_12   (12+L_GPIO_SHIFT_BASE)  ///< L_GPIO[12]
#define L_GPIO_13   (13+L_GPIO_SHIFT_BASE)  ///< L_GPIO[13]
#define L_GPIO_14   (14+L_GPIO_SHIFT_BASE)  ///< L_GPIO[14]
#define L_GPIO_15   (15+L_GPIO_SHIFT_BASE)  ///< L_GPIO[15]
#define L_GPIO_16   (16+L_GPIO_SHIFT_BASE)  ///< L_GPIO[16]
#define L_GPIO_17   (17+L_GPIO_SHIFT_BASE)  ///< L_GPIO[17]
#define L_GPIO_18   (18+L_GPIO_SHIFT_BASE)  ///< L_GPIO[18]
#define L_GPIO_19   (19+L_GPIO_SHIFT_BASE)  ///< L_GPIO[19]
#define L_GPIO_20   (20+L_GPIO_SHIFT_BASE)  ///< L_GPIO[20]
#define L_GPIO_21   (21+L_GPIO_SHIFT_BASE)  ///< L_GPIO[21]
#define L_GPIO_22   (22+L_GPIO_SHIFT_BASE)  ///< L_GPIO[22]
#define L_GPIO_23   (23+L_GPIO_SHIFT_BASE)  ///< L_GPIO[23]
#define L_GPIO_24   (24+L_GPIO_SHIFT_BASE)  ///< L_GPIO[24]

/*Dedicated GPIO - DGPIO*/
#define D_GPIO_SHIFT_BASE 128
#define D_GPIO_0    (0 +D_GPIO_SHIFT_BASE)  ///< DGPIO[0]
#define D_GPIO_1    (1 +D_GPIO_SHIFT_BASE)  ///< DGPIO[1]
#define D_GPIO_2    (2 +D_GPIO_SHIFT_BASE)  ///< DGPIO[2]
#define D_GPIO_3    (3 +D_GPIO_SHIFT_BASE)  ///< DGPIO[3]
#define D_GPIO_4    (4 +D_GPIO_SHIFT_BASE)  ///< DGPIO[4]
#define D_GPIO_5    (5 +D_GPIO_SHIFT_BASE)  ///< DGPIO[5]
#define D_GPIO_6    (6 +D_GPIO_SHIFT_BASE)  ///< DGPIO[6]
#define D_GPIO_7    (7 +D_GPIO_SHIFT_BASE)  ///< DGPIO[7]
#define D_GPIO_8    (8 +D_GPIO_SHIFT_BASE)  ///< DGPIO[8]
#define D_GPIO_9    (9 +D_GPIO_SHIFT_BASE)  ///< DGPIO[9]
#define D_GPIO_10    (10 +D_GPIO_SHIFT_BASE)  ///< DGPIO[10]
#define D_GPIO_11    (11 +D_GPIO_SHIFT_BASE)  ///< DGPIO[11]
#define D_GPIO_12    (12 +D_GPIO_SHIFT_BASE)  ///< DGPIO[12]
#define D_GPIO_13    (13 +D_GPIO_SHIFT_BASE)  ///< DGPIO[13]
#define D_GPIO_14    (14 +D_GPIO_SHIFT_BASE)  ///< DGPIO[14]

/*GPIO HSI Data register(High speed interface)*/
#define H_GPIO_SHIFT_BASE 160
#define H_GPIO_0    (0 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[0]
#define H_GPIO_1    (1 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[1]
#define H_GPIO_2    (2 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[2]
#define H_GPIO_3    (3 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[3]
#define H_GPIO_4    (4 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[4]
#define H_GPIO_5    (5 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[5]
#define H_GPIO_6    (6 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[6]
#define H_GPIO_7    (7 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[7]
#define H_GPIO_8    (8 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[8]
#define H_GPIO_9    (9 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[9]
#define H_GPIO_10   (10+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[10]
#define H_GPIO_11   (11+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[11]
#define H_GPIO_12   (12+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[12]
#define H_GPIO_13   (13+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[13]
#define H_GPIO_14   (14+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[14]
#define H_GPIO_15   (15+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[15]
#define H_GPIO_16   (16+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[16]
#define H_GPIO_17   (17+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[17]
#define H_GPIO_18   (18+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[18]
#define H_GPIO_19   (19+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[19]
#define H_GPIO_20   (20+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[20]
#define H_GPIO_21   (21+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[21]
#define H_GPIO_22   (22+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[22]
#define H_GPIO_23   (23+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[23]

/*GPIO ADC Data register*/
#define A_GPIO_SHIFT_BASE 192
#define A_GPIO_0    (0 + A_GPIO_SHIFT_BASE)  ///< A_GPIO[0]
#define A_GPIO_1    (1 + A_GPIO_SHIFT_BASE)  ///< A_GPIO[1]
#define A_GPIO_2    (2 + A_GPIO_SHIFT_BASE)  ///< A_GPIO[2]

/*GPIO DSI Data register*/
//not used in 538, for backward compatible
#if 1
#define DSI_GPIO_SHIFT_BASE 224
#define DSI_GPIO_0  (0 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[0]
#define DSI_GPIO_1  (1 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[1]
#define DSI_GPIO_2  (2 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[2]
#define DSI_GPIO_3  (3 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[3]
#define DSI_GPIO_4  (4 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[4]
#define DSI_GPIO_5  (5 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[5]
#define DSI_GPIO_6  (6 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[6]
#define DSI_GPIO_7  (7 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[7]
#define DSI_GPIO_8  (8 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[8]
#define DSI_GPIO_9  (9 + DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[9]
#define DSI_GPIO_10 (10+ DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[10]
#endif
//@}


/**
    @name   GPIO Interrupt ID

    GPIO interrupt ID definition

    GPIO interrupt ID for interrupt related APIs.
*/
//@{
#define GPIO_INT_00                 0       ///< GPIO INT[0]:   C_GPIO[0]
#define GPIO_INT_01                 1       ///< GPIO INT[1]:   C_GPIO[1]
#define GPIO_INT_02                 2       ///< GPIO INT[2]:   C_GPIO[2]
#define GPIO_INT_03                 3       ///< GPIO INT[3]:   C_GPIO[3]
#define GPIO_INT_04                 4       ///< GPIO INT[4]:   C_GPIO[4]
#define GPIO_INT_05                 5       ///< GPIO INT[5]:   C_GPIO[5]
#define GPIO_INT_06                 6       ///< GPIO INT[6]:   C_GPIO[6]
#define GPIO_INT_07                 7       ///< GPIO INT[7]:   C_GPIO[7]
#define GPIO_INT_08                 8       ///< GPIO INT[8]:   C_GPIO[8]
#define GPIO_INT_09                 9       ///< GPIO INT[9]:   C_GPIO[9]
#define GPIO_INT_10                 10      ///< GPIO INT[10]:  C_GPIO[10]
#define GPIO_INT_11                 11      ///< GPIO INT[11]:  C_GPIO[11]
#define GPIO_INT_12                 12      ///< GPIO INT[12]:  C_GPIO[12]
#define GPIO_INT_13                 13      ///< GPIO INT[13]:  C_GPIO[13]
#define GPIO_INT_14                 14      ///< GPIO INT[14]:  C_GPIO[14]
#define GPIO_INT_15                 15      ///< GPIO INT[15]:  C_GPIO[15]
#define GPIO_INT_16                 16      ///< GPIO INT[16]:  C_GPIO[16]
#define GPIO_INT_17                 17      ///< GPIO INT[17]:  C_GPIO[17]
#define GPIO_INT_18                 18      ///< GPIO INT[18]:  C_GPIO[18]
#define GPIO_INT_19                 19      ///< GPIO INT[19]:  C_GPIO[19]
#define GPIO_INT_20                 20      ///< GPIO INT[20]:  C_GPIO[20]
#define GPIO_INT_21                 21      ///< GPIO INT[21]:  C_GPIO[21]
#define GPIO_INT_22                 22      ///< GPIO INT[22]:  C_GPIO[22]
#define GPIO_INT_23                 23      ///< GPIO INT[23]:  C_GPIO[23]
#define GPIO_INT_24                 24      ///< GPIO INT[24]:  P_GPIO[0]
#define GPIO_INT_25                 25      ///< GPIO INT[25]:  P_GPIO[1]
#define GPIO_INT_26                 26      ///< GPIO INT[26]:  P_GPIO[2]
#define GPIO_INT_27                 27      ///< GPIO INT[27]:  P_GPIO[3]
#define GPIO_INT_28                 28      ///< GPIO INT[28]:  P_GPIO[4]
#define GPIO_INT_29                 29      ///< GPIO INT[29]:  P_GPIO[5]
#define GPIO_INT_30                 30      ///< GPIO INT[30]:  P_GPIO[6]
#define GPIO_INT_31                 31      ///< GPIO INT[31]:  P_GPIO[7]
#define GPIO_INT_32                 32      ///< GPIO INT[32]:  P_GPIO[8]
#define GPIO_INT_33                 33      ///< GPIO INT[33]:  P_GPIO[9]
#define GPIO_INT_34                 34      ///< GPIO INT[34]:  P_GPIO[10]
#define GPIO_INT_35                 35      ///< GPIO INT[35]:  P_GPIO[11]
#define GPIO_INT_36                 36      ///< GPIO INT[36]:  P_GPIO[12]
#define GPIO_INT_37                 37      ///< GPIO INT[37]:  P_GPIO[13]
#define GPIO_INT_38                 38      ///< GPIO INT[38]:  P_GPIO[14]
#define GPIO_INT_39                 39      ///< GPIO INT[39]:  P_GPIO[15]
#define GPIO_INT_40                 40      ///< GPIO INT[40]:  P_GPIO[16]
#define GPIO_INT_41                 41      ///< GPIO INT[41]:  P_GPIO[17]
#define GPIO_INT_42                 42      ///< GPIO INT[42]:  P_GPIO[18]
#define GPIO_INT_43                 43      ///< GPIO INT[43]:  P_GPIO[19]
#define GPIO_INT_44                 44      ///< GPIO INT[44]:  P_GPIO[20]
#define GPIO_INT_45                 45      ///< GPIO INT[45]:  P_GPIO[21]
#define GPIO_INT_46                 46      ///< GPIO INT[46]:  P_GPIO[22]
#define GPIO_INT_47                 47      ///< GPIO INT[47]:  P_GPIO[23]
#define GPIO_INT_48                 48      ///< GPIO INT[48]:  P_GPIO[24]
#define GPIO_INT_49                 49      ///< GPIO INT[49]:  P_GPIO[25]
#define GPIO_INT_50                 50      ///< GPIO INT[50]:  S_GPIO[0]
#define GPIO_INT_51                 51      ///< GPIO INT[51]:  S_GPIO[1]
#define GPIO_INT_52                 52      ///< GPIO INT[52]:  S_GPIO[2]
#define GPIO_INT_53                 53      ///< GPIO INT[53]:  S_GPIO[3]
#define GPIO_INT_54                 54      ///< GPIO INT[54]:  S_GPIO[4]
#define GPIO_INT_55                 55      ///< GPIO INT[55]:  S_GPIO[5]
#define GPIO_INT_56                 56      ///< GPIO INT[56]:  S_GPIO[6]
#define GPIO_INT_57                 57      ///< GPIO INT[57]:  S_GPIO[7]
#define GPIO_INT_58                 58      ///< GPIO INT[58]:  S_GPIO[8]
#define GPIO_INT_59                 59      ///< GPIO INT[59]:  S_GPIO[9]
#define GPIO_INT_60                 60      ///< GPIO INT[60]:  S_GPIO[10]
#define GPIO_INT_61                 61      ///< GPIO INT[61]:  S_GPIO[11]
#define GPIO_INT_62                 62      ///< GPIO INT[62]:  S_GPIO[12]
#define GPIO_INT_63                 63      ///< GPIO INT[63]:  S_GPIO[13]
#define GPIO_INT_64                 64      ///< GPIO INT[64]:  S_GPIO[14]
#define GPIO_INT_65                 65      ///< GPIO INT[65]:  S_GPIO[15]
#define GPIO_INT_66                 66      ///< GPIO INT[66]:  L_GPIO[0]
#define GPIO_INT_67                 67      ///< GPIO INT[67]:  L_GPIO[1]
#define GPIO_INT_68                 68      ///< GPIO INT[68]:  L_GPIO[2]
#define GPIO_INT_69                 69      ///< GPIO INT[69]:  L_GPIO[3]
#define GPIO_INT_70                 70      ///< GPIO INT[70]:  L_GPIO[4]
#define GPIO_INT_71                 71      ///< GPIO INT[71]:  L_GPIO[5]
#define GPIO_INT_72                 72      ///< GPIO INT[72]:  L_GPIO[6]
#define GPIO_INT_73                 73      ///< GPIO INT[73]:  L_GPIO[7]
#define GPIO_INT_74                 74      ///< GPIO INT[74]:  L_GPIO[8]
#define GPIO_INT_75                 75      ///< GPIO INT[75]:  L_GPIO[9]
#define GPIO_INT_76                 76      ///< GPIO INT[76]:  L_GPIO[10]
#define GPIO_INT_77                 77      ///< GPIO INT[77]:  L_GPIO[11]
#define GPIO_INT_78                 78      ///< GPIO INT[78]:  L_GPIO[12]
#define GPIO_INT_79                 79      ///< GPIO INT[79]:  L_GPIO[13]
#define GPIO_INT_80                 80      ///< GPIO INT[80]:  L_GPIO[14]
#define GPIO_INT_81                 81      ///< GPIO INT[81]:  L_GPIO[15]
#define GPIO_INT_82                 82      ///< GPIO INT[82]:  L_GPIO[16]
#define GPIO_INT_83                 83      ///< GPIO INT[83]:  L_GPIO[17]
#define GPIO_INT_84                 84      ///< GPIO INT[84]:  L_GPIO[18]
#define GPIO_INT_85                 85      ///< GPIO INT[85]:  L_GPIO[19]
#define GPIO_INT_86                 86      ///< GPIO INT[86]:  L_GPIO[20]
#define GPIO_INT_87                 87      ///< GPIO INT[87]:  L_GPIO[21]
#define GPIO_INT_88                 88      ///< GPIO INT[88]:  L_GPIO[22]
#define GPIO_INT_89                 89      ///< GPIO INT[89]:  L_GPIO[23]
#define GPIO_INT_90                 90      ///< GPIO INT[90]:  L_GPIO[24]
#define GPIO_INT_91                 91      ///< GPIO INT[91]:  D_GPIO[0]
#define GPIO_INT_92                 92      ///< GPIO INT[92]:  D_GPIO[1]
#define GPIO_INT_93                 93      ///< GPIO INT[93]:  D_GPIO[2]
#define GPIO_INT_94                 94      ///< GPIO INT[94]:  D_GPIO[3]
#define GPIO_INT_95                 95      ///< GPIO INT[95]:  D_GPIO[4]
#define GPIO_INT_96                 96      ///< GPIO INT[96]:  D_GPIO[5]
#define GPIO_INT_97                 97      ///< GPIO INT[97]:  D_GPIO[6]
#define GPIO_INT_98                 98      ///< GPIO INT[98]:  D_GPIO[7]
#define GPIO_INT_99                 99      ///< GPIO INT[99]:  D_GPIO[8]
#define GPIO_INT_100               100      ///< GPIO INT[100]: D_GPIO[9]
#define GPIO_INT_101               101      ///< GPIO INT[101]: D_GPIO[10]
#define GPIO_INT_102               102      ///< GPIO INT[102]: D_GPIO[11]
#define GPIO_INT_103               103      ///< GPIO INT[103]: D_GPIO[12]
#define GPIO_INT_104               104      ///< GPIO INT[104]: D_GPIO[13]
#define GPIO_INT_105               105      ///< GPIO INT[105]: D_GPIO[14]
#define GPIO_INT_106               106      ///< GPIO INT[106]: HSI_GPIO[0]
#define GPIO_INT_107               107      ///< GPIO INT[107]: HSI_GPIO[1]
#define GPIO_INT_108               108      ///< GPIO INT[108]: HSI_GPIO[2]
#define GPIO_INT_109               109      ///< GPIO INT[109]: HSI_GPIO[3]
#define GPIO_INT_110               110      ///< GPIO INT[110]: HSI_GPIO[4]
#define GPIO_INT_111               111      ///< GPIO INT[111]: HSI_GPIO[5]
#define GPIO_INT_112               112      ///< GPIO INT[112]: HSI_GPIO[6]
#define GPIO_INT_113               113      ///< GPIO INT[113]: HSI_GPIO[7]
#define GPIO_INT_114               114      ///< GPIO INT[114]: HSI_GPIO[8]
#define GPIO_INT_115               115      ///< GPIO INT[115]: HSI_GPIO[9]
#define GPIO_INT_116               116      ///< GPIO INT[116]: HSI_GPIO[10]
#define GPIO_INT_117               117      ///< GPIO INT[117]: HSI_GPIO[11]
#define GPIO_INT_118               118      ///< GPIO INT[118]: HSI_GPIO[12]
#define GPIO_INT_119               119      ///< GPIO INT[119]: HSI_GPIO[13]
#define GPIO_INT_120               120      ///< GPIO INT[120]: HSI_GPIO[14]
#define GPIO_INT_121               121      ///< GPIO INT[121]: HSI_GPIO[15]
#define GPIO_INT_122               122      ///< GPIO INT[122]: HSI_GPIO[16]
#define GPIO_INT_123               123      ///< GPIO INT[123]: HSI_GPIO[17]
#define GPIO_INT_124               124      ///< GPIO INT[124]: HSI_GPIO[18]
#define GPIO_INT_125               125      ///< GPIO INT[125]: HSI_GPIO[19]
#define GPIO_INT_126               126      ///< GPIO INT[126]: HSI_GPIO[20]
#define GPIO_INT_127               127      ///< GPIO INT[127]: HSI_GPIO[21]
#define GPIO_INT_128               128      ///< GPIO INT[128]: HSI_GPIO[22]
#define GPIO_INT_129               129      ///< GPIO INT[129]: HSI_GPIO[23]
#define GPIO_INT_130               130      ///< GPIO INT[130]: A_GPIO[0]
#define GPIO_INT_131               131      ///< GPIO INT[131]: A_GPIO[1]
#define GPIO_INT_132               132      ///< GPIO INT[132]: A_GPIO[2]
#define GPIO_INT_USBWAKEUP         133      ///< GPIO INT[133]: USB wakeup
#define GPIO_INT_USBPLUGIN         GPIO_INT_USBWAKEUP
#define GPIO_INT_SWTRIG            134      ///< GPIO INT[134]: internal interrupt

//@}

// In order to backward comaptible, DGPIO is used as " GPIO_IS_DGPIO | D_GPIO_* " or "D_GPIO_*"
#define GPIO_IS_DGPIO               (128)


//
// The general api for the GPIO device driver
//
extern ER       gpio_open(void);
extern ER       gpio_close(void);
extern BOOL     gpio_isOpened(void);
extern ER       gpio_waitIntFlag(UINT32 gpioIntID);

// ---- GPIO ----
extern void     gpio_setDir(UINT32 pin, GPIO_DIR dir);
extern GPIO_DIR gpio_getDir(UINT32 pin);
extern void     gpio_setPin(UINT32 pin);
extern void     gpio_clearPin(UINT32 pin);
extern UINT32   gpio_getPin(UINT32 pin);
extern UINT32   gpio_getPin_schmitt(UINT32 pin);
extern void     gpio_pullSet(UINT32 pin);
extern void     gpio_pullClear(UINT32 pin);
extern UINT32   gpio_readData(UINT32 dataidx);
extern void     gpio_writeData(UINT32 dataidx, UINT32 value);
// ---- DGPIO ----
extern UINT32   dgpio_readData(void);
extern void     dgpio_writeData(UINT32 value);

// ---- Interrupt Control ----
extern void     gpio_enableInt(UINT32 ipin);
extern void     gpio_disableInt(UINT32 ipin);
extern UINT32   gpio_getIntEnable(UINT32 ipin);
extern void     gpio_clearIntStatus(UINT32 ipin);
extern UINT32   gpio_getIntStatus(UINT32 ipin);
extern void     gpio_setIntTypePol(UINT32 ipin, GPIO_INTTYPE type, GPIO_INTPOL pol);
extern void     gpio_setIntIsr(UINT32 ipin, DRV_CB pHdl);
extern void     gpio_enableDestination(UINT32 ipin);
extern void     gpio_disableDestination(UINT32 ipin);

extern void     gpio_platform_init(void);
extern void     gpio_platform_uninit(void);
extern void     gpio_bindInt_counter(UINT32 ipin);
extern void     gpio_clrInt_counter(UINT32 ipin);
extern UINT32   gpio_readInt_counter(UINT32 ipin);
extern void     gpio_swInt_trig(UINT32 ipin);

//for external API usage
#define NVT_GPIO_STG_DATA_0  0x00
#define NVT_GPIO_STG_DIR_0   0x20
#define NVT_GPIO_STG_SET_0   0x40
#define NVT_GPIO_STG_CLR_0   0x60
#define NVT_GPIO_STS_CPU     0x80
#define NVT_GPIO_INTEN_CPU   0x94
#define NVT_GPIO_INT_TYPE    0xA8
#define NVT_GPIO_INT_POL     0xBC
#define NVT_GPIO_EDGE_TYPE   0xD0

//#define NVT_DGPIO_STS_CPU    0xC0
//#define NVT_DGPIO_INTEN_CPU  0xD0
//#define NVT_DGPIO_INT_TYPE   0xE0
//#define NVT_DGPIO_INT_POL    0xE4
//#define NVT_DGPIO_EDGE_TYPE  0xE8


/* GPIO pin number translation  */
//main part
#define C_GPIO(pin)			(pin)
#define P_GPIO(pin)		    (pin + 0x20)
#define S_GPIO(pin)		    (pin + 0x40)
#define L_GPIO(pin)			(pin + 0x60)
#define D_GPIO(pin)			(pin + 0x80)
#define H_GPIO(pin)			(pin + 0xA0)
#define A_GPIO(pin)		    (pin + 0xC0)
//not used in 538
#define DSI_GPIO(pin)		(pin + 0xE0)


#define C_GPIO_NUM			24
#define P_GPIO_NUM			26
#define S_GPIO_NUM		    16
#define L_GPIO_NUM			25
#define D_GPIO_NUM			15
#define H_GPIO_NUM			24
#define A_GPIO_NUM		    3
//not used in 538
#define DSI_GPIO_NUM		11


/* GPIO API compatible*/
#define gpio_is_opened                  gpio_isOpened
#define gpio_wait_int_flag              gpio_waitIntFlag
#define gpio_set_dir                    gpio_setDir
#define gpio_get_dir                    gpio_getDir
#define gpio_set_pin                    gpio_setPin
#define gpio_clear_pin                  gpio_clearPin
#define gpio_get_pin                    gpio_getPin
#define gpio_pull_set                   gpio_pullSet
#define gpio_pull_Clear                 gpio_pullClear
#define gpio_read_data                  gpio_readData
#define gpio_write_data                 gpio_writeData
#define gpio_enable_int                 gpio_enableInt
#define gpio_disable_int                gpio_disableInt
#define gpio_get_Int_enable             gpio_getIntEnable
#define gpio_clear_int_status           gpio_clearIntStatus
#define gpio_get_int_status             gpio_getIntStatus
#define gpio_set_int_type_pol           gpio_setIntTypePol
#define gpio_set_int_isr                gpio_setIntIsr


/*linux compatible*/
int gpio_get_value(UINT32 pin);
void gpio_set_value(UINT32 pin, int value);
void gpio_direction_input(UINT32 pin);
void gpio_direction_output(UINT32 pin, int value);
#endif /* __ASM_ARCH_NA51000_NVT_GPIO_H */
