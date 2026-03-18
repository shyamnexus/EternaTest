/*
 * Copyright (c) 2021, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NOVATEK_DDR_PARA_H
#define NOVATEK_DDR_PARA_H

#include <stddef.h>
#include <errno.h>
#include <libfdt.h>
#include <platform_def.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <novatek_private.h>
#include <novatek_def.h>

typedef struct
{
	uint32_t					  ini_ver; //bit[27:24] 0x0:336 DDR3 FB, 0x1:336 DDR4 FB, 0x2: 333 DDR4 FB
    uint32_t                      pllRatio;

    uint32_t                      mau_reg_000; //MSTR
    uint32_t                      mau_reg_010; //MRCTRL0
    uint32_t                      mau_reg_050; //RFSHCTL0
    uint32_t                      mau_reg_064; //RFSHTMG
    uint32_t                      mau_reg_068; //RFSHTMG
    uint32_t                      mau_reg_0D0; //INIT0
    uint32_t                      mau_reg_0D4; //INIT1
    uint32_t                      mau_reg_0D8; //INIT2
    uint32_t                      mau_reg_0DC; //INIT3
    uint32_t                      mau_reg_0E0; //INIT4
    uint32_t                      mau_reg_0E4; //INIT5
    uint32_t                      mau_reg_0E8; //INIT6
    uint32_t                      mau_reg_0EC; //INIT7

    uint32_t                      mau_reg_100; //DRAMTMG0
    uint32_t                      mau_reg_104; //DRAMTMG1
    uint32_t                      mau_reg_108; //DRAMTMG2
    uint32_t                      mau_reg_10C; //DRAMTMG3
    uint32_t                      mau_reg_110; //DRAMTMG4
    uint32_t                      mau_reg_114; //DRAMTMG5
    uint32_t                      mau_reg_118; //DRAMTMG6
    uint32_t                      mau_reg_11C; //DRAMTMG7
    uint32_t                      mau_reg_120; //DRAMTMG8
    uint32_t                      mau_reg_124; //DRAMTMG9
    uint32_t                      mau_reg_130; //DRAMTMG12
    uint32_t                      mau_reg_134; //DRAMTMG13
    uint32_t                      mau_reg_138; //DRAMTMG14

    uint32_t                      mau_reg_180; //ZQCTL0
    uint32_t                      mau_reg_184; //ZQCTL1

    uint32_t                      mau_reg_190; //DFITMG0
    uint32_t                      mau_reg_194; //DFITMG1

    uint32_t                      mau_reg_240; //ODTCFG
    uint32_t                      dram_size; //ODTCFG
    uint32_t                      calibration; //[0]:write leveling, [1]:write 1D, [2]:write 2D, [3]:read 1D 1st, [4]:read 2D 1st
                                             //[5]:read 1D 2nd, [6]:read 2D 2nd, [7]:cbt, [8]:pmi, [9]:read deskew, [10]:write deskew, [11]:write deskew(mau)

	////////////////
	uint8_t                       dll_phase_0; //Set RDC_CK0 [8:0]
	uint8_t                       dll_phase_1; //Set RDC_CMD[1:0] [8:0]
	uint8_t                       dll_phase_2; //Set RDC_CMD[9:4] [8:0]
	uint8_t                       dll_phase_3; //Set RDC_CMD[3:2] [8:0]
	uint8_t                       dll_phase_4; //Set RDC_CK1 [8:0]
	uint8_t                       dll_phase_5; //Set RDC_CMD[11:10] [8:0]
	uint8_t                       dll_phase_6; //Set RDC_CMD[19:14] [8:0]
	uint8_t                       dll_phase_7; //Set RDC_CMD[13:12] [8:0]
	uint8_t                       dll_phase_8; //Set RDC_CMD[23:20] [8:0]
	uint8_t                       dll_phase_9; //Set RDC_CMD[27:24] [8:0]
	uint8_t                       dll_phase_A; //Set RDC_RDQS0_master [8:0]
	uint8_t                       dll_phase_B; //Set RDC_RDQS0_slave [8:0]
	uint8_t                       dll_phase_C; //Set RDC_WDQS_DI0[3:0] [8:0]
	uint8_t                       dll_phase_D; //Set RDC_WDQ_byte0 [8:0]
	uint8_t                       dll_phase_E; //Set RDC_RDQS1_master [8:0]
	uint8_t                       dll_phase_F; //Set RDC_RDQS1_slave [8:0]
	uint8_t                       dll_phase_10; //Set RDC_WDQS_DI1[3:0] [8:0]
	uint8_t                       dll_phase_11; //Set RDC_WDQ_byte1 [8:0]
	uint8_t                       dll_phase_12; //Set RDC_RDQS2_master [8:0]
	uint8_t                       dll_phase_13; //Set RDC_RDQS2_slave [8:0]
	uint8_t                       dll_phase_14; //Set RDC_WDQS_DI2[3:0] [8:0]
	uint8_t                       dll_phase_15; //Set RDC_WDQ_byte2 [8:0]
	uint8_t                       dll_phase_16; //Set RDC_RDQS3_master [8:0]
	uint8_t                       dll_phase_17; //Set RDC_RDQS3_slave [8:0]
	uint8_t                       dll_phase_18; //Set RDC_WDQS_DI3[3:0] [8:0]
	uint8_t                       dll_phase_19; //Set RDC_WDQ_byte3 [8:0]
	uint8_t                       dll_phase_1A; //Set RDC_ZQK_TOP

    uint8_t                       reg_000;     //[7]1:ddr4, 0:lpddr4; [3]1: 2t preambe (ddr4 mode only), 0 :1t preambe
    uint8_t                       reg_001;     //set tccd_num [5:0]
    uint8_t                       reg_002;     //trcd_num[3:0]
    uint8_t                       reg_004;     //trp_num[3:0]
    uint8_t                       reg_005;     //trtp_num[3:0]
    uint8_t                       reg_006;     //tmod_num[4:0]
    uint8_t                       reg_007;     //tmrd_num[3:0]
    uint8_t                       reg_009;     //write_latency[5:0]
    uint8_t                       reg_00A;     //read_latency[5:0]
    uint8_t                       reg_00B;     //write_recovery[4:0]
    uint8_t                       reg_00D;     //rst_num_ddr43[7:0]
    uint8_t                       reg_010;     //cke_num_ddr43[7:0]
    uint8_t                       reg_011;     //cke_num_ddr43[15:8]
    uint8_t                       reg_012;     //cke_num_ddr43[18:16], mrs_num_ddr43[3:0]
    uint8_t                       reg_013;     //mrs_num_ddr43[8:4]
    uint8_t                       reg_015;     //ddr4 MR0[7:0], lpddr4 MR1[7:0]
    uint8_t                       reg_016;     //ddr4 MR0[15:8], lpddr4 MR1[15:8]
    uint8_t                       reg_017;     //ddr4 MR1[7:0], lpddr4 MR2[7:0]
    uint8_t                       reg_018;     //ddr4 MR1[15:8], lpddr4 MR2[15:8]
    uint8_t                       reg_019;     //ddr4 MR2[7:0], lpddr4 MR3[7:0]
    uint8_t                       reg_01A;     //ddr4 MR2[15:8], lpddr4 MR3[15:8]
    uint8_t                       reg_01B;     //ddr4 MR3[7:0], lpddr4 MR11[7:0]
    uint8_t                       reg_01C;     //ddr4 MR3[15:8], lpddr4 MR11[15:8]
    uint8_t                       reg_01D;     //lpddr4 MRW MR14 VREF(DQ) store in PHY REG 0x01D
    uint8_t                       reg_01E;     //lpddr4 MRW MR12 VREF(CA) store in PHY REG 0x01E
    uint8_t                       reg_020;     //trfi_num[7:0]
    uint8_t                       reg_021;     //trfi_num[11:8]
    uint8_t                       reg_022;     //trfc_num[7:0]
    uint8_t                       reg_023;     //tr2w_num[5:0]
    uint8_t                       reg_027;     //set tzqcal_num [7:0]
    uint8_t                       reg_028;     //set tzqcal_num [10:8]
    uint8_t                       reg_02D;     //trrd_num[2:0]
    uint8_t                       reg_02E;     //set tzqcallat_num [7:0]
    uint8_t                       reg_060;     //ddr4 MR4[7:0], lpddr4 MR22[7:0]
    uint8_t                       reg_061;     //ddr4 MR4[15:8], lpddr4 MR22[15:8]
    uint8_t                       reg_062;     //ddr4 MR5[7:0]
    uint8_t                       reg_063;     //ddr4 MR5[15:8]
    uint8_t                       reg_064;     //ddr4 MR6[7:0]
    uint8_t                       reg_065;     //ddr4 MR6[15:8]
    uint8_t                       reg_066;     //DBI Enable [1]:rd_dbi_en, [0]:wr_dbi_en
    uint8_t                       reg_083;     //[0]1:ddr4 qs_gate, 0:lpddr4 qs_gate

    uint8_t                       reg_100;     //dll_phase_RDQS0_master [6:0]
    uint8_t                       reg_101;     //dll_phase_RDQS1_master [6:0]
    uint8_t                       reg_102;     //dll_phase_RDQS2_master [6:0]
    uint8_t                       reg_103;     //dll_phase_RDQS3_master [6:0]
    uint8_t                       reg_104;     //dll_phase_RDQS0_slave [6:0]
    uint8_t                       reg_105;     //dll_phase_RDQS1_slave [6:0]
    uint8_t                       reg_106;     //dll_phase_RDQS2_slave [6:0]
    uint8_t                       reg_107;     //dll_phase_RDQS3_slave [6:0]

    uint8_t                       reg_108;     //dll_phase_WDQS_DI0[3:0] [6:0]
    uint8_t                       reg_109;     //dll_phase_WDQS_DI1[3:0] [6:0]
    uint8_t                       reg_10A;     //dll_phase_WDQS_DI2[3:0] [6:0]
    uint8_t                       reg_10B;     //dll_phase_WDQS_DI3[3:0] [6:0]

    uint8_t                       reg_111;     //dll_phase_WDQ_byte1 [6:0]
	
    uint8_t                       reg_16C;     //WDQ[0] BDC[6:0]
    uint8_t                       reg_16D;     //WDQ[2] BDC[6:0]
    uint8_t                       reg_16E;     //WDQ[3] BDC[6:0]
    uint8_t                       reg_16F;     //WDQ[4] BDC[6:0]
    uint8_t                       reg_170;     //WDQ[5] BDC[6:0]
    uint8_t                       reg_171;     //WDQ[6] BDC[6:0]
    uint8_t                       reg_172;     //WDQ[7] BDC[6:0]
    uint8_t                       reg_173;     //WDQ[8] BDC[6:0]
    uint8_t                       reg_174;     //WDQ[1] BDC[6:0]	

    uint8_t                       reg_175;     //RDQ[0] BDC[6:0]
    uint8_t                       reg_176;     //RDQ[2] BDC[6:0]
    uint8_t                       reg_177;     //RDQ[3] BDC[6:0]
    uint8_t                       reg_178;     //RDQ[4] BDC[6:0]
    uint8_t                       reg_179;     //RDQ[5] BDC[6:0]
    uint8_t                       reg_17A;     //RDQ[6] BDC[6:0]
    uint8_t                       reg_17B;     //RDQ[7] BDC[6:0]
    uint8_t                       reg_17C;     //RDQ[8] BDC[6:0]
    uint8_t                       reg_17D;     //RDQ[1] BDC[6:0]
	
    uint8_t                       reg_17E;     //WDQ[9] BDC[6:0]
    uint8_t                       reg_17F;     //WDQ[10] BDC[6:0]
    uint8_t                       reg_180;     //WDQ[11] BDC[6:0]
    uint8_t                       reg_181;     //WDQ[13] BDC[6:0]
    uint8_t                       reg_182;     //WDQ[14] BDC[6:0]
    uint8_t                       reg_183;     //WDQ[15] BDC[6:0]
    uint8_t                       reg_184;     //WDQ[16] BDC[6:0]
    uint8_t                       reg_185;     //WDQ[17] BDC[6:0]
    uint8_t                       reg_186;     //WDQ[12] BDC[6:0]	

    uint8_t                       reg_187;     //RDQ[9] BDC[6:0]
    uint8_t                       reg_188;     //RDQ[10] BDC[6:0]
    uint8_t                       reg_189;     //RDQ[11] BDC[6:0]
    uint8_t                       reg_18A;     //RDQ[13] BDC[6:0]
    uint8_t                       reg_18B;     //RDQ[14] BDC[6:0]
    uint8_t                       reg_18C;     //RDQ[15] BDC[6:0]
    uint8_t                       reg_18D;     //RDQ[16] BDC[6:0]
    uint8_t                       reg_18E;     //RDQ[17] BDC[6:0]
    uint8_t                       reg_18F;     //RDQ[12] BDC[6:0]

    uint8_t                       reg_190;     //WDQ[18] BDC[6:0]
    uint8_t                       reg_191;     //WDQ[20] BDC[6:0]
    uint8_t                       reg_192;     //WDQ[21] BDC[6:0]
    uint8_t                       reg_193;     //WDQ[22] BDC[6:0]
    uint8_t                       reg_194;     //WDQ[23] BDC[6:0]
    uint8_t                       reg_195;     //WDQ[24] BDC[6:0]
    uint8_t                       reg_196;     //WDQ[25] BDC[6:0]
    uint8_t                       reg_197;     //WDQ[26] BDC[6:0]
    uint8_t                       reg_198;     //WDQ[19] BDC[6:0]
	
    uint8_t                       reg_199;     //RDQ[18] BDC[6:0]
    uint8_t                       reg_19A;     //RDQ[20] BDC[6:0]
    uint8_t                       reg_19B;     //RDQ[21] BDC[6:0]
    uint8_t                       reg_19C;     //RDQ[22] BDC[6:0]
    uint8_t                       reg_19D;     //RDQ[23] BDC[6:0]
    uint8_t                       reg_19E;     //RDQ[24] BDC[6:0]
    uint8_t                       reg_19F;     //RDQ[25] BDC[6:0]
    uint8_t                       reg_1A0;     //RDQ[26] BDC[6:0]
    uint8_t                       reg_1A1;     //RDQ[19] BDC[6:0]

    uint8_t                       reg_1A2;     //WDQ[27] BDC[6:0]
    uint8_t                       reg_1A3;     //WDQ[29] BDC[6:0]
    uint8_t                       reg_1A4;     //WDQ[30] BDC[6:0]
    uint8_t                       reg_1A5;     //WDQ[31] BDC[6:0]
    uint8_t                       reg_1A6;     //WDQ[32] BDC[6:0]
    uint8_t                       reg_1A7;     //WDQ[33] BDC[6:0]
    uint8_t                       reg_1A8;     //WDQ[34] BDC[6:0]
    uint8_t                       reg_1A9;     //WDQ[35] BDC[6:0]
    uint8_t                       reg_1AA;     //WDQ[28] BDC[6:0]
	
    uint8_t                       reg_1AB;     //RDQ[27] BDC[6:0]
    uint8_t                       reg_1AC;     //RDQ[29] BDC[6:0]
    uint8_t                       reg_1AD;     //RDQ[30] BDC[6:0]
    uint8_t                       reg_1AE;     //RDQ[31] BDC[6:0]
    uint8_t                       reg_1AF;     //RDQ[32] BDC[6:0]
    uint8_t                       reg_1B0;     //RDQ[33] BDC[6:0]
    uint8_t                       reg_1B1;     //RDQ[34] BDC[6:0]
    uint8_t                       reg_1B2;     //RDQ[35] BDC[6:0]
    uint8_t                       reg_1B3;     //RDQ[28] BDC[6:0]

    uint8_t                       reg_500;     //[7]1:lpddr4, 0:ddr4
    uint8_t                       reg_501;     //set r_t_mrd[5:0]
    uint8_t                       reg_509;     //set init_1 timer [7:0]
    uint8_t                       reg_50A;     //set init_1 timer [10:8]
    uint8_t                       reg_50B;     //set init_3 timer [7:0]
    uint8_t                       reg_50C;     //set init_3 timer [14:8]
    uint8_t                       reg_50D;     //set init_5 timer [7:0]
    uint8_t                       reg_50E;     //set init_5 timer [11:8]

    uint8_t                       reg_511;     //CBT timing
    uint8_t                       reg_512;     //CBT timing
    uint8_t                       reg_515;     //CBT timing
    uint8_t                       reg_52E;     //MPC Write to Read timing

    uint8_t                       reg_556;     //MAU [7]:r_dfi_rxen_extend_enable, [4:0]:r_dfi_rxen_extend_num[4:0]
    uint8_t                       reg_557;     //MAU [7]:r_dfi_odten_extend_enable, [4:0]:r_dfi_odten_extend_num[4:0]




    uint8_t                       reg_603;    //CA Write enable for CA[23:16]
    uint8_t                       reg_604;    //CA Write enable for CLK[1:0], CA[28:24]
    uint8_t                       reg_60A;    //CA0 RX mode selection for CLK
    uint8_t                       reg_60B;    //CA0 PMOS pull-up driving strength control for CLK
    uint8_t                       reg_60C;    //CA0 NMOS pull-down driving strength control for CLK
    uint8_t                       reg_611;    //CA0 PMOS pull-up trim code for CLK
    uint8_t                       reg_612;    //CA0 NMOS pull-down trim code for CLK
    uint8_t                       reg_614;    //CA0 RX mode selection for CS
    uint8_t                       reg_615;    //CA0 PMOS pull-up driving strength control for CS
    uint8_t                       reg_616;    //CA0 NMOS pull-down driving strength control for CS
    uint8_t                       reg_61B;    //CA0 PMOS pull-up trim code for CS
    uint8_t                       reg_61C;    //CA0 NMOS pull-down trim code for CS
    uint8_t                       reg_61E;    //CA0 RX mode selection for CKE
    uint8_t                       reg_61F;    //CA0 PMOS pull-up driving strength control for CKE
    uint8_t                       reg_620;    //CA0 NMOS pull-down driving strength control for CKE
    uint8_t                       reg_625;    //CA0 NMOS pull-down trim code for CKE
    uint8_t                       reg_626;    //CA0 PMOS pull-up trim code for CLK
    uint8_t                       reg_628;    //CA0 RX mode selection for CA
    uint8_t                       reg_629;    //CA0 PMOS pull-up driving strength control for CA
    uint8_t                       reg_62A;    //CA0 NMOS pull-down driving strength control for CA
    uint8_t                       reg_62F;    //CA0 PMOS pull-up trim code for CA
    uint8_t                       reg_630;    //CA0 NMOS pull-down trim code for CA
    uint8_t                       reg_649;    //CA2 RX mode selection for CA
    uint8_t                       reg_64A;    //CA2 PMOS pull-up driving strength control for CA
    uint8_t                       reg_64B;    //CA2 NMOS pull-down driving strength control for CA
    uint8_t                       reg_654;    //CA1 RX mode selection for CLK
    uint8_t                       reg_655;    //CA1 PMOS pull-up driving strength control for CLK
    uint8_t                       reg_656;    //CA1 NMOS pull-down driving strength control for CLK
    uint8_t                       reg_65B;    //CA1 PMOS pull-up trim code for CLK
    uint8_t                       reg_65C;    //CA1 NMOS pull-down trim code for CLK
    uint8_t                       reg_65E;    //CA1 RX mode selection for CS
    uint8_t                       reg_65F;    //CA1 PMOS pull-up driving strength control for CS
    uint8_t                       reg_660;    //CA1 NMOS pull-down driving strength control for CS
    uint8_t                       reg_665;    //CA1 PMOS pull-up trim code for CS
    uint8_t                       reg_666;    //CA1 NMOS pull-down trim code for CS
    uint8_t                       reg_668;    //CA1 RX mode selection for CKE
    uint8_t                       reg_669;    //CA1 PMOS pull-up driving strength control for CKE
    uint8_t                       reg_66A;    //CA1 NMOS pull-down driving strength control for CKE
    uint8_t                       reg_66F;    //CA1 PMOS pull-up trim code for CKE
    uint8_t                       reg_670;    //CA1 NMOS pull-down trim code for CKE
    uint8_t                       reg_672;    //CA1 RX mode selection for CA
    uint8_t                       reg_673;    //CA1 PMOS pull-up driving strength control for CA
    uint8_t                       reg_674;    //CA1 NMOS pull-down driving strength control for CA
    uint8_t                       reg_679;    //CA1 PMOS pull-up trim code for CA
    uint8_t                       reg_67A;    //CA1 NMOS pull-down trim code for CA
    uint8_t                       reg_693;    //CA3 RX mode selection for CA
    uint8_t                       reg_694;    //CA3 PMOS pull-up driving strength control for CA
    uint8_t                       reg_695;    //CA3 NMOS pull-down driving strength control for CA

    uint8_t                       reg_6A1;    //CMD[1:0] UI_Delay[4:0]
    uint8_t                       reg_6A2;    //CMD[3:2] UI_Delay[4:0]
    uint8_t                       reg_6A3;    //CMD[9:4] UI_Delay[4:0]
    uint8_t                       reg_6A4;    //CMD[11:10] UI_Delay[4:0]
    uint8_t                       reg_6A5;    //CMD[13:12] UI_Delay[4:0]
    uint8_t                       reg_6A6;    //CMD[19:14] UI_Delay[4:0]
    uint8_t                       reg_6A7;    //CMD[23:20] UI_Delay[4:0]
    uint8_t                       reg_6A8;    //CMD[27:24] UI_Delay[4:0]
	uint8_t                       reg_6A9;    //CMD[14] DDR4 ODT UI_Delay[4:0]
    uint8_t                       reg_6AF;    //CA WODT
    uint8_t                       reg_6B0;    //CA WODT
    uint8_t                       reg_6B1;    //CA WODT
	uint8_t                       reg_6B7;    //CMD[14] DDR4 ODT UI_Delay[4:0]

    uint8_t                       reg_70A;     //RX mode selection for Byte0 DQS
    uint8_t                       reg_70B;     //PMOS pull-up driving strength control for Byte0 DQS
    uint8_t                       reg_70C;     //NMOS pull-down driving strength control for Byte0 DQS
    uint8_t                       reg_70E;     //PMOS pull-up ODT resistance select for Byte0 DQS
    uint8_t                       reg_70F;     //NMOS pull-down ODT resistance select for Byte0 DQS
    uint8_t                       reg_711;     //PMOS pull-up trim code for Byte0 DQS and DQ
    uint8_t                       reg_712;     //NMOS pull-down trim code for Byte0 DQS and DQ
    uint8_t                       reg_713;     //VREF power-down control for Byte0 DQS
    uint8_t                       reg_714;     //RX mode selection for Byte0 DQ
    uint8_t                       reg_715;     //PMOS pull-up driving strength control for Byte0 DQ
    uint8_t                       reg_716;     //NMOS pull-down driving strength control for Byte0 DQ
    uint8_t                       reg_718;     //PMOS pull-up ODT resistance select for Byte0 DQ
    uint8_t                       reg_719;     //NMOS pull-down ODT resistance select for Byte0 DQ
    uint8_t                       reg_71B;     //PMOS pull-up trim code for Byte0 DQS and DQ(no use)
    uint8_t                       reg_71C;     //NMOS pull-down trim code for Byte0 DQS and DQ(no use)
    uint8_t                       reg_71D;     //VREF power-down control for Byte0 DQ
    uint8_t                       reg_720;     //RX mode selection for Byte1 DQS
    uint8_t                       reg_721;     //PMOS pull-up driving strength control for Byte1 DQS
    uint8_t                       reg_722;     //NMOS pull-down driving strength control for Byte1 DQS
    uint8_t                       reg_724;     //PMOS pull-up ODT resistance select for Byte1 DQS
    uint8_t                       reg_725;     //NMOS pull-down ODT resistance select for Byte1 DQS
    uint8_t                       reg_729;     //VREF power-down control for Byte1 DQS
    uint8_t                       reg_72A;     //RX mode selection for Byte1 DQ
    uint8_t                       reg_72B;     //PMOS pull-up driving strength control for Byte1 DQ
    uint8_t                       reg_72C;     //NMOS pull-down driving strength control for Byte1 DQ
    uint8_t                       reg_72E;     //PMOS pull-up ODT resistance select for Byte1 DQ
    uint8_t                       reg_72F;     //NMOS pull-down ODT resistance select for Byte1 DQ
    uint8_t                       reg_733;     //VREF power-down control for Byte1 DQ
    uint8_t                       reg_736;     //RX mode selection for Byte2 DQS
    uint8_t                       reg_737;     //PMOS pull-up driving strength control for Byte2 DQS
    uint8_t                       reg_738;     //NMOS pull-down driving strength control for Byte2 DQS
    uint8_t                       reg_73A;     //PMOS pull-up ODT resistance select for Byte2 DQS
    uint8_t                       reg_73B;     //NMOS pull-down ODT resistance select for Byte2 DQS
    uint8_t                       reg_73D;     //PMOS pull-up trim code for Byte2 DQS and DQ
    uint8_t                       reg_73E;     //NMOS pull-down trim code for Byte2 DQS and DQ
    uint8_t                       reg_73F;     //VREF power-down control for Byte2 DQS
    uint8_t                       reg_740;     //RX mode selection for Byte2 DQ
    uint8_t                       reg_741;     //PMOS pull-up driving strength control for Byte2 DQ
    uint8_t                       reg_742;     //NMOS pull-down driving strength control for Byte2 DQ
    uint8_t                       reg_744;     //PMOS pull-up ODT resistance select for Byte2 DQ
    uint8_t                       reg_745;     //NMOS pull-down ODT resistance select for Byte2 DQ
    uint8_t                       reg_747;     //PMOS pull-up trim code for Byte2 DQS and DQ(no use)
    uint8_t                       reg_748;     //NMOS pull-down trim code for Byte2 DQS and DQ(no use)
    uint8_t                       reg_749;     //VREF power-down control for Byte2 DQ
    uint8_t                       reg_74C;     //RX mode selection for Byte3 DQS
    uint8_t                       reg_74D;     //PMOS pull-up driving strength control for Byte3 DQS
    uint8_t                       reg_74E;     //NMOS pull-down driving strength control for Byte3 DQS
    uint8_t                       reg_750;     //PMOS pull-up ODT resistance select for Byte3 DQS
    uint8_t                       reg_751;     //NMOS pull-down ODT resistance select for Byte3 DQS
    uint8_t                       reg_755;     //VREF power-down control for Byte3 DQS
    uint8_t                       reg_756;     //RX mode selection for Byte3 DQ
    uint8_t                       reg_757;     //PMOS pull-up driving strength control for Byte3 DQ
    uint8_t                       reg_758;     //NMOS pull-down driving strength control for Byte3 DQ
    uint8_t                       reg_75A;     //PMOS pull-up ODT resistance select for Byte3 DQ
    uint8_t                       reg_75B;     //NMOS pull-down ODT resistance select for Byte3 DQ
    uint8_t                       reg_75F;     //VREF power-down control for Byte3 DQ
	uint8_t                       reg_76C;     //WDQS_WEN0[3:0] UI_Delay[4:0]
	uint8_t                       reg_76D;     //WDQS_WEN1[3:0] UI_Delay[4:0]
	uint8_t                       reg_76E;     //WDQS_WEN2[3:0] UI_Delay[4:0]
	uint8_t                       reg_76F;     //WDQS_WEN3[3:0] UI_Delay[4:0]
	uint8_t                       reg_770;     //WDQS_DI0[3:0] UI_Delay[4:0]
	uint8_t                       reg_771;     //WDQS_DI1[3:0] UI_Delay[4:0]
	uint8_t                       reg_772;     //WDQS_DI2[3:0] UI_Delay[4:0]
	uint8_t                       reg_773;     //WDQS_DI3[3:0] UI_Delay[4:0]

	uint8_t                       reg_774;	//WDQ_WEN0[3:0] UI_Delay[4:0]
	uint8_t                       reg_775;	//WDQ_WEN1[3:0] UI_Delay[4:0]
	uint8_t                       reg_776;	//WDQ_WEN2[3:0] UI_Delay[4:0]
	uint8_t                       reg_777;	//WDQ_WEN3[3:0] UI_Delay[4:0]
	uint8_t                       reg_778;	//WDQ_OE0[3:0] UI_Delay[4:0]
	uint8_t                       reg_779;	//WDQ_OE1[3:0] UI_Delay[4:0]
	uint8_t                       reg_77A;	//WDQ_OE2[3:0] UI_Delay[4:0]
	uint8_t                       reg_77B;	//WDQ_OE3[3:0] UI_Delay[4:0]

	uint8_t                       reg_780;	//WDQ_byte0 UI_Delay[4:0]
	uint8_t                       reg_781;
	uint8_t                       reg_782;
	uint8_t                       reg_783;
	uint8_t                       reg_784;
	uint8_t                       reg_785;
	uint8_t                       reg_786;
	uint8_t                       reg_787;
	uint8_t                       reg_788;
	uint8_t                       reg_789;	//WDQ_byte1 UI_Delay[4:0]
	uint8_t                       reg_78A;
	uint8_t                       reg_78B;
	uint8_t                       reg_78C;
	uint8_t                       reg_78D;
	uint8_t                       reg_78E;
	uint8_t                       reg_78F;
	uint8_t                       reg_790;
	uint8_t                       reg_791;
	uint8_t                       reg_792;	//WDQ_byte2 UI_Delay[4:0]
	uint8_t                       reg_793;
	uint8_t                       reg_794;
	uint8_t                       reg_795;
	uint8_t                       reg_796;
	uint8_t                       reg_797;
	uint8_t                       reg_798;
	uint8_t                       reg_799;
	uint8_t                       reg_79A;
	uint8_t                       reg_79B;	//WDQ_byte3 UI_Delay[4:0]
	uint8_t                       reg_79C;
	uint8_t                       reg_79D;
	uint8_t                       reg_79E;
	uint8_t                       reg_79F;
	uint8_t                       reg_7A0;
	uint8_t                       reg_7A1;
	uint8_t                       reg_7A2;
	uint8_t                       reg_7A3;

    uint8_t                       reg_7A4;     //qsen_fifo_in_sel[1:0]
    uint8_t                       reg_7B0;     //DQS RX enable

    uint8_t                       reg_7B1;     //byte0_vref_sel_dqs[6:0]
    uint8_t                       reg_7B2;     //byte0_vref_sel_012[6:0]
    uint8_t                       reg_7B3;     //byte0_vref_sel_345[6:0]
    uint8_t                       reg_7B4;     //byte0_vref_sel_678[6:0]
    uint8_t                       reg_7B5;     //byte1_vref_sel_dqs[6:0]
    uint8_t                       reg_7B6;     //byte1_vref_sel_012[6:0]
    uint8_t                       reg_7B7;     //byte1_vref_sel_345[6:0]
    uint8_t                       reg_7B8;     //byte1_vref_sel_678[6:0]
    uint8_t                       reg_7B9;     //byte2_vref_sel_dqs[6:0]
    uint8_t                       reg_7BA;     //byte2_vref_sel_012[6:0]
    uint8_t                       reg_7BB;     //byte2_vref_sel_345[6:0]
    uint8_t                       reg_7BC;     //byte2_vref_sel_678[6:0]
    uint8_t                       reg_7BD;     //byte3_vref_sel_dqs[6:0]
    uint8_t                       reg_7BE;     //byte3_vref_sel_012[6:0]
    uint8_t                       reg_7BF;     //byte3_vref_sel_345[6:0]
    uint8_t                       reg_7C0;     //byte3_vref_sel_678[6:0]


    uint8_t                       reg_7D6;    //odten_extend_cycle_ddr4
    uint8_t                       reg_7D7;    //odten_extend_cycle_ddr4
    uint8_t                       reg_7D8;    //odten_extend_cycle_ddr4


    uint8_t                       reg_7D9;    //odten_extend_cycle_lp4
    uint8_t                       reg_7DA;    //odten_extend_cycle_lp4
    uint8_t                       reg_7DB;    //odten_extend_cycle_lp4

    uint8_t                       reg_7DC;    //rxen_extend_cycle_ddr4
    uint8_t                       reg_7DD;    //rxen_extend_cycle_ddr4
    uint8_t                       reg_7DE;    //rxen_extend_cycle_ddr4


    uint8_t                       reg_7DF;    //rxen_extend_cycle_lp4
    uint8_t                       reg_7E0;    //rxen_extend_cycle_lp4
    uint8_t                       reg_7E1;    //rxen_extend_cycle_lp4


    uint8_t                       reg_812;    //ZQK VREF range selsection
    uint8_t                       reg_813;    //ZQK left PPU ODT parallel number selection
    uint8_t                       reg_814;    //ZQK right PPU ODT parallel number selection
    uint8_t                       reg_815;    //ZQK VREF selection


    uint8_t                       reg_821;    //PMI timing
    uint8_t                       reg_822;    //PMI timing
    uint8_t                       reg_823;    //PMI timing
    uint8_t                       reg_825;    //PMI timing

    uint8_t                       reg_852;     //MPC Write to Read timing

    uint8_t                       reg_8E0;     //[7]0:ddc_update_disable when use RDC, 1:ddc_update_enable when use dll_phase

    uint8_t                       reg_8E1;     //dll_phase_WDQ_byte0 [7:0]
    uint8_t                       reg_8E2;     //dll_phase_WDQ_byte1 [7:0]
    uint8_t                       reg_8E3;     //dll_phase_WDQ_byte2 [7:0]
    uint8_t                       reg_8E4;     //dll_phase_WDQ_byte3 [7:0]


} DMA_CONFIG_SETTING;


void plat_save_system_data(void *data_base);
void plat_memcpy_word(void *dst, void *src, unsigned int len);



#define rphy phy_io_read8
#define wphy phy_io_write8
#define REG_PHY_PAGE_SEL	0xFE //0xFE , shift left 2 = 0x3FE
#define DDR_NUM 1
#define clrbits8(val,mask)	((unsigned int)((val) & ~(mask)))
#define setbits8(val,mask)	((unsigned int)((val) | (mask)))

//macro
#define REG_ADDR(name)		name##_ADDR
#define REG_SHIFT(name)		name##_SHIFT
#define REG_SIZE(name)		name##_SIZE
#define REG_MASK(name)		name##_MASK
#define REG_HIGH_BIT(name)	name##_HIGH_BIT
#define REG_LOW_BIT(name)	name##_LOW_BIT
#define DECLARE_REG(addr, high_bit, low_bit, name) \
	enum { \
		REG_ADDR(name) = (addr), \
		REG_HIGH_BIT(name) = (high_bit), \
		REG_LOW_BIT(name) = (low_bit), \
		REG_SHIFT(name) = (low_bit), \
		REG_SIZE(name) = (high_bit) - (low_bit) + 1, \
		REG_MASK(name) = ((unsigned int)((1ULL << REG_SIZE(name)) - 1) \
					<< REG_SHIFT(name)), \
	}

// register
#define DDR_PHY_REG_BASE_ADDR 0x2F00D8000
#define DDR_CTRL_REG_BASE_ADDR 0x2f00d0000

#define PHY_READY_REG                           PHY_READY_REG
DECLARE_REG(0x000, 0, 0, PHY_READY_REG);

#define DDR4                                    DDR4
DECLARE_REG(0x000, 7, 7, DDR4);

#define ENABLE_RDC_CAL_CLK                      ENABLE_RDC_CAL_CLK
DECLARE_REG(0x55c, 7, 7, ENABLE_RDC_CAL_CLK);

#define LPDDR4                                  LPDDR4
DECLARE_REG(0x500, 7, 7, LPDDR4);

#define LP4_TRAIN_R_DLY_WAIT_TIME_3_0_          LP4_TRAIN_R_DLY_WAIT_TIME_3_0_
DECLARE_REG(0x524, 7, 4, LP4_TRAIN_R_DLY_WAIT_TIME_3_0_);

#define MCK_DI_REG_1_0_                         MCK_DI_REG_1_0_
DECLARE_REG(0x03f, 7, 6, MCK_DI_REG_1_0_);

#define ASYNC_FIFO_SHIFT_IN                     ASYNC_FIFO_SHIFT_IN
DECLARE_REG(0x530, 7, 7, ASYNC_FIFO_SHIFT_IN);

#define LP4_DLY_CAL_START_TGL_7_0_              LP4_DLY_CAL_START_TGL_7_0_
DECLARE_REG(0x531, 7, 0, LP4_DLY_CAL_START_TGL_7_0_);

#define LP4_DLY_CAL_START_TGL_15_8_             LP4_DLY_CAL_START_TGL_15_8_
DECLARE_REG(0x532, 7, 0, LP4_DLY_CAL_START_TGL_15_8_);

#define LP4_DLY_CAL_START_TGL_23_16_            LP4_DLY_CAL_START_TGL_23_16_
DECLARE_REG(0x533, 7, 0, LP4_DLY_CAL_START_TGL_23_16_);

#define LP4_DLY_CAL_START_TGL_26_24_            LP4_DLY_CAL_START_TGL_26_24_
DECLARE_REG(0x534, 7, 5, LP4_DLY_CAL_START_TGL_26_24_);

#define LP4_CAL_DONE_7_0_                       LP4_CAL_DONE_7_0_
DECLARE_REG(0x535, 7, 0, LP4_CAL_DONE_7_0_);

#define LP4_CAL_DONE_15_8_                      LP4_CAL_DONE_15_8_
DECLARE_REG(0x536, 7, 0, LP4_CAL_DONE_15_8_);

#define LP4_CAL_DONE_23_16_                     LP4_CAL_DONE_23_16_
DECLARE_REG(0x537, 7, 0, LP4_CAL_DONE_23_16_);

#define LP4_CAL_DONE_25_24_                     LP4_CAL_DONE_25_24_
DECLARE_REG(0x538, 7, 5, LP4_CAL_DONE_25_24_);

#define LP4_CAL_FAIL_7_0_                       LP4_CAL_FAIL_7_0_
DECLARE_REG(0x539, 7, 0, LP4_CAL_FAIL_7_0_);

#define LP4_CAL_FAIL_15_8_                      LP4_CAL_FAIL_15_8_
DECLARE_REG(0x53a, 7, 0, LP4_CAL_FAIL_15_8_);

#define LP4_CAL_FAIL_23_16_                     LP4_CAL_FAIL_23_16_
DECLARE_REG(0x53b, 7, 0, LP4_CAL_FAIL_23_16_);

#define LP4_CAL_FAIL_26_24_                     LP4_CAL_FAIL_26_24_
DECLARE_REG(0x53c, 7, 5, LP4_CAL_FAIL_26_24_);

#define TRAINING_BYPASS_REG                     TRAINING_BYPASS_REG
DECLARE_REG(0x001, 6, 6, TRAINING_BYPASS_REG);

#define LP4_R_DLY_CAL_RSTB                      LP4_R_DLY_CAL_RSTB
DECLARE_REG(0x52f, 6, 6, LP4_R_DLY_CAL_RSTB);

#define CA0_RDC_CK_CNTL_7_0_                    CA0_RDC_CK_CNTL_7_0_
DECLARE_REG(0x560, 7, 0, CA0_RDC_CK_CNTL_7_0_);

#define CA0_RDC_CK_CNTL_8_                      CA0_RDC_CK_CNTL_8_
DECLARE_REG(0x561, 7, 7, CA0_RDC_CK_CNTL_8_);

#define CA0_RDC_CKE_CS_CNTL_7_0_                CA0_RDC_CKE_CS_CNTL_7_0_
DECLARE_REG(0x562, 7, 0, CA0_RDC_CKE_CS_CNTL_7_0_);

#define CA0_RDC_CKE_CS_CNTL_8_                  CA0_RDC_CKE_CS_CNTL_8_
DECLARE_REG(0x563, 7, 7, CA0_RDC_CKE_CS_CNTL_8_);

#define CA0_RDC_CA_CNTL_7_0_                    CA0_RDC_CA_CNTL_7_0_
DECLARE_REG(0x564, 7, 0, CA0_RDC_CA_CNTL_7_0_);

#define CA0_RDC_CA_CNTL_8_                      CA0_RDC_CA_CNTL_8_
DECLARE_REG(0x565, 7, 7, CA0_RDC_CA_CNTL_8_);

#define CA0_RDC_CKE_CS_DUAL_CNTL_7_0_           CA0_RDC_CKE_CS_DUAL_CNTL_7_0_
DECLARE_REG(0x566, 7, 0, CA0_RDC_CKE_CS_DUAL_CNTL_7_0_);

#define CA0_RDC_CKE_CS_DUAL_CNTL_8_             CA0_RDC_CKE_CS_DUAL_CNTL_8_
DECLARE_REG(0x567, 7, 7, CA0_RDC_CKE_CS_DUAL_CNTL_8_);

#define CA1_RDC_CK_CNTL_7_0_                    CA1_RDC_CK_CNTL_7_0_
DECLARE_REG(0x568, 7, 0, CA1_RDC_CK_CNTL_7_0_);

#define CA1_RDC_CK_CNTL_8_                      CA1_RDC_CK_CNTL_8_
DECLARE_REG(0x569, 7, 7, CA1_RDC_CK_CNTL_8_);

#define CA1_RDC_CKE_CS_CNTL_7_0_                CA1_RDC_CKE_CS_CNTL_7_0_
DECLARE_REG(0x56a, 7, 0, CA1_RDC_CKE_CS_CNTL_7_0_);

#define CA1_RDC_CKE_CS_CNTL_8_                  CA1_RDC_CKE_CS_CNTL_8_
DECLARE_REG(0x56b, 7, 7, CA1_RDC_CKE_CS_CNTL_8_);

#define CA1_RDC_CA_CNTL_7_0_                    CA1_RDC_CA_CNTL_7_0_
DECLARE_REG(0x56c, 7, 0, CA1_RDC_CA_CNTL_7_0_);

#define CA1_RDC_CA_CNTL_8_                      CA1_RDC_CA_CNTL_8_
DECLARE_REG(0x56d, 7, 7, CA1_RDC_CA_CNTL_8_);

#define CA1_RDC_CKE_CS_DUAL_CNTL_7_0_           CA1_RDC_CKE_CS_DUAL_CNTL_7_0_
DECLARE_REG(0x56e, 7, 0, CA1_RDC_CKE_CS_DUAL_CNTL_7_0_);

#define CA1_RDC_CKE_CS_DUAL_CNTL_8_             CA1_RDC_CKE_CS_DUAL_CNTL_8_
DECLARE_REG(0x56f, 7, 7, CA1_RDC_CKE_CS_DUAL_CNTL_8_);

#define CA2_RDC_CA_CNTL_7_0_                    CA2_RDC_CA_CNTL_7_0_
DECLARE_REG(0x570, 7, 0, CA2_RDC_CA_CNTL_7_0_);

#define CA2_RDC_CA_CNTL_8_                      CA2_RDC_CA_CNTL_8_
DECLARE_REG(0x571, 7, 7, CA2_RDC_CA_CNTL_8_);

#define CA3_RDC_CA_CNTL_7_0_                    CA3_RDC_CA_CNTL_7_0_
DECLARE_REG(0x572, 7, 0, CA3_RDC_CA_CNTL_7_0_);

#define CA3_RDC_CA_CNTL_8_                      CA3_RDC_CA_CNTL_8_
DECLARE_REG(0x573, 7, 7, CA3_RDC_CA_CNTL_8_);

#define BYTE0_RDC_RDQS_M_CNTL_7_0_              BYTE0_RDC_RDQS_M_CNTL_7_0_
DECLARE_REG(0x574, 7, 0, BYTE0_RDC_RDQS_M_CNTL_7_0_);

#define BYTE0_RDC_RDQS_M_CNTL_8_                BYTE0_RDC_RDQS_M_CNTL_8_
DECLARE_REG(0x575, 7, 7, BYTE0_RDC_RDQS_M_CNTL_8_);

#define BYTE0_RDC_RDQS_S_CNTL_7_0_              BYTE0_RDC_RDQS_S_CNTL_7_0_
DECLARE_REG(0x576, 7, 0, BYTE0_RDC_RDQS_S_CNTL_7_0_);

#define BYTE0_RDC_RDQS_S_CNTL_8_                BYTE0_RDC_RDQS_S_CNTL_8_
DECLARE_REG(0x577, 7, 7, BYTE0_RDC_RDQS_S_CNTL_8_);

#define BYTE0_RDC_DQS_CNTL_7_0_                 BYTE0_RDC_DQS_CNTL_7_0_
DECLARE_REG(0x578, 7, 0, BYTE0_RDC_DQS_CNTL_7_0_);

#define BYTE0_RDC_DQS_CNTL_8_                   BYTE0_RDC_DQS_CNTL_8_
DECLARE_REG(0x579, 7, 7, BYTE0_RDC_DQS_CNTL_8_);

#define BYTE0_RDC_DQ_CNTL_7_0_                  BYTE0_RDC_DQ_CNTL_7_0_
DECLARE_REG(0x57a, 7, 0, BYTE0_RDC_DQ_CNTL_7_0_);

#define BYTE0_RDC_DQ_CNTL_8_                    BYTE0_RDC_DQ_CNTL_8_
DECLARE_REG(0x57b, 7, 7, BYTE0_RDC_DQ_CNTL_8_);

#define BYTE1_RDC_RDQS_M_CNTL_7_0_              BYTE1_RDC_RDQS_M_CNTL_7_0_
DECLARE_REG(0x57c, 7, 0, BYTE1_RDC_RDQS_M_CNTL_7_0_);

#define BYTE1_RDC_RDQS_M_CNTL_8_                BYTE1_RDC_RDQS_M_CNTL_8_
DECLARE_REG(0x57d, 7, 7, BYTE1_RDC_RDQS_M_CNTL_8_);

#define BYTE1_RDC_RDQS_S_CNTL_7_0_              BYTE1_RDC_RDQS_S_CNTL_7_0_
DECLARE_REG(0x57e, 7, 0, BYTE1_RDC_RDQS_S_CNTL_7_0_);

#define BYTE1_RDC_RDQS_S_CNTL_8_                BYTE1_RDC_RDQS_S_CNTL_8_
DECLARE_REG(0x57f, 7, 7, BYTE1_RDC_RDQS_S_CNTL_8_);

#define BYTE1_RDC_DQS_CNTL_7_0_                 BYTE1_RDC_DQS_CNTL_7_0_
DECLARE_REG(0x580, 7, 0, BYTE1_RDC_DQS_CNTL_7_0_);

#define BYTE1_RDC_DQS_CNTL_8_                   BYTE1_RDC_DQS_CNTL_8_
DECLARE_REG(0x581, 7, 7, BYTE1_RDC_DQS_CNTL_8_);

#define BYTE1_RDC_DQ_CNTL_7_0_                  BYTE1_RDC_DQ_CNTL_7_0_
DECLARE_REG(0x582, 7, 0, BYTE1_RDC_DQ_CNTL_7_0_);

#define BYTE1_RDC_DQ_CNTL_8_                    BYTE1_RDC_DQ_CNTL_8_
DECLARE_REG(0x583, 7, 7, BYTE1_RDC_DQ_CNTL_8_);

#define BYTE2_RDC_RDQS_M_CNTL_7_0_              BYTE2_RDC_RDQS_M_CNTL_7_0_
DECLARE_REG(0x584, 7, 0, BYTE2_RDC_RDQS_M_CNTL_7_0_);

#define BYTE2_RDC_RDQS_M_CNTL_8_                BYTE2_RDC_RDQS_M_CNTL_8_
DECLARE_REG(0x585, 7, 7, BYTE2_RDC_RDQS_M_CNTL_8_);

#define BYTE2_RDC_RDQS_S_CNTL_7_0_              BYTE2_RDC_RDQS_S_CNTL_7_0_
DECLARE_REG(0x586, 7, 0, BYTE2_RDC_RDQS_S_CNTL_7_0_);

#define BYTE2_RDC_RDQS_S_CNTL_8_                BYTE2_RDC_RDQS_S_CNTL_8_
DECLARE_REG(0x587, 7, 7, BYTE2_RDC_RDQS_S_CNTL_8_);

#define BYTE2_RDC_DQS_CNTL_7_0_                 BYTE2_RDC_DQS_CNTL_7_0_
DECLARE_REG(0x588, 7, 0, BYTE2_RDC_DQS_CNTL_7_0_);

#define BYTE2_RDC_DQS_CNTL_8_                   BYTE2_RDC_DQS_CNTL_8_
DECLARE_REG(0x589, 7, 7, BYTE2_RDC_DQS_CNTL_8_);

#define BYTE2_RDC_DQ_CNTL_7_0_                  BYTE2_RDC_DQ_CNTL_7_0_
DECLARE_REG(0x58a, 7, 0, BYTE2_RDC_DQ_CNTL_7_0_);

#define BYTE2_RDC_DQ_CNTL_8_                    BYTE2_RDC_DQ_CNTL_8_
DECLARE_REG(0x58b, 7, 7, BYTE2_RDC_DQ_CNTL_8_);

#define BYTE3_RDC_RDQS_M_CNTL_7_0_              BYTE3_RDC_RDQS_M_CNTL_7_0_
DECLARE_REG(0x58c, 7, 0, BYTE3_RDC_RDQS_M_CNTL_7_0_);

#define BYTE3_RDC_RDQS_M_CNTL_8_                BYTE3_RDC_RDQS_M_CNTL_8_
DECLARE_REG(0x58d, 7, 7, BYTE3_RDC_RDQS_M_CNTL_8_);

#define BYTE3_RDC_RDQS_S_CNTL_7_0_              BYTE3_RDC_RDQS_S_CNTL_7_0_
DECLARE_REG(0x58e, 7, 0, BYTE3_RDC_RDQS_S_CNTL_7_0_);

#define BYTE3_RDC_RDQS_S_CNTL_8_                BYTE3_RDC_RDQS_S_CNTL_8_
DECLARE_REG(0x58f, 7, 7, BYTE3_RDC_RDQS_S_CNTL_8_);

#define BYTE3_RDC_DQS_CNTL_7_0_                 BYTE3_RDC_DQS_CNTL_7_0_
DECLARE_REG(0x590, 7, 0, BYTE3_RDC_DQS_CNTL_7_0_);

#define BYTE3_RDC_DQS_CNTL_8_                   BYTE3_RDC_DQS_CNTL_8_
DECLARE_REG(0x591, 7, 7, BYTE3_RDC_DQS_CNTL_8_);

#define BYTE3_RDC_DQ_CNTL_7_0_                  BYTE3_RDC_DQ_CNTL_7_0_
DECLARE_REG(0x592, 7, 0, BYTE3_RDC_DQ_CNTL_7_0_);

#define BYTE3_RDC_DQ_CNTL_8_                    BYTE3_RDC_DQ_CNTL_8_
DECLARE_REG(0x593, 7, 7, BYTE3_RDC_DQ_CNTL_8_);

#define ZQK_RDC_CNTL_7_0_                       ZQK_RDC_CNTL_7_0_
DECLARE_REG(0x594, 7, 0, ZQK_RDC_CNTL_7_0_);

#define ZQK_RDC_CNTL_8_                         ZQK_RDC_CNTL_8_
DECLARE_REG(0x595, 7, 7, ZQK_RDC_CNTL_8_);

#define CA0_RDC_CK_CNTL_REG_7_0_                CA0_RDC_CK_CNTL_REG_7_0_
DECLARE_REG(0x598, 7, 0, CA0_RDC_CK_CNTL_REG_7_0_);

#define CA0_RDC_CK_CNTL_REG_8_                  CA0_RDC_CK_CNTL_REG_8_
DECLARE_REG(0x599, 7, 7, CA0_RDC_CK_CNTL_REG_8_);

#define CA0_RDC_CKE_CS_CNTL_REG_7_0_            CA0_RDC_CKE_CS_CNTL_REG_7_0_
DECLARE_REG(0x59a, 7, 0, CA0_RDC_CKE_CS_CNTL_REG_7_0_);

#define CA0_RDC_CKE_CS_CNTL_REG_8_              CA0_RDC_CKE_CS_CNTL_REG_8_
DECLARE_REG(0x59b, 7, 7, CA0_RDC_CKE_CS_CNTL_REG_8_);

#define CA0_RDC_CA_CNTL_REG_7_0_                CA0_RDC_CA_CNTL_REG_7_0_
DECLARE_REG(0x59c, 7, 0, CA0_RDC_CA_CNTL_REG_7_0_);

#define CA0_RDC_CA_CNTL_REG_8_                  CA0_RDC_CA_CNTL_REG_8_
DECLARE_REG(0x59d, 7, 7, CA0_RDC_CA_CNTL_REG_8_);

#define CA0_RDC_CKE_CS_DUAL_CNTL_REG_7_0_       CA0_RDC_CKE_CS_DUAL_CNTL_REG_7_0_
DECLARE_REG(0x59e, 7, 0, CA0_RDC_CKE_CS_DUAL_CNTL_REG_7_0_);

#define CA0_RDC_CKE_CS_DUAL_CNTL_REG_8_         CA0_RDC_CKE_CS_DUAL_CNTL_REG_8_
DECLARE_REG(0x59f, 7, 7, CA0_RDC_CKE_CS_DUAL_CNTL_REG_8_);

#define CA1_RDC_CK_CNTL_REG_7_0_                CA1_RDC_CK_CNTL_REG_7_0_
DECLARE_REG(0x5a0, 7, 0, CA1_RDC_CK_CNTL_REG_7_0_);

#define CA1_RDC_CK_CNTL_REG_8_                  CA1_RDC_CK_CNTL_REG_8_
DECLARE_REG(0x5a1, 7, 7, CA1_RDC_CK_CNTL_REG_8_);

#define CA1_RDC_CKE_CS_CNTL_REG_7_0_            CA1_RDC_CKE_CS_CNTL_REG_7_0_
DECLARE_REG(0x5a2, 7, 0, CA1_RDC_CKE_CS_CNTL_REG_7_0_);

#define CA1_RDC_CKE_CS_CNTL_REG_8_              CA1_RDC_CKE_CS_CNTL_REG_8_
DECLARE_REG(0x5a3, 7, 7, CA1_RDC_CKE_CS_CNTL_REG_8_);

#define CA1_RDC_CA_CNTL_REG_7_0_                CA1_RDC_CA_CNTL_REG_7_0_
DECLARE_REG(0x5a4, 7, 0, CA1_RDC_CA_CNTL_REG_7_0_);

#define CA1_RDC_CA_CNTL_REG_8_                  CA1_RDC_CA_CNTL_REG_8_
DECLARE_REG(0x5a5, 7, 7, CA1_RDC_CA_CNTL_REG_8_);

#define CA1_RDC_CKE_CS_DUAL_CNTL_REG_7_0_       CA1_RDC_CKE_CS_DUAL_CNTL_REG_7_0_
DECLARE_REG(0x5a6, 7, 0, CA1_RDC_CKE_CS_DUAL_CNTL_REG_7_0_);

#define CA1_RDC_CKE_CS_DUAL_CNTL_REG_8_         CA1_RDC_CKE_CS_DUAL_CNTL_REG_8_
DECLARE_REG(0x5a7, 7, 7, CA1_RDC_CKE_CS_DUAL_CNTL_REG_8_);

#define CA2_RDC_CA_CNTL_REG_7_0_                CA2_RDC_CA_CNTL_REG_7_0_
DECLARE_REG(0x5a8, 7, 0, CA2_RDC_CA_CNTL_REG_7_0_);

#define CA2_RDC_CA_CNTL_REG_8_                  CA2_RDC_CA_CNTL_REG_8_
DECLARE_REG(0x5a9, 7, 7, CA2_RDC_CA_CNTL_REG_8_);

#define CA3_RDC_CA_CNTL_REG_7_0_                CA3_RDC_CA_CNTL_REG_7_0_
DECLARE_REG(0x5aa, 7, 0, CA3_RDC_CA_CNTL_REG_7_0_);

#define CA3_RDC_CA_CNTL_REG_8_                  CA3_RDC_CA_CNTL_REG_8_
DECLARE_REG(0x5ab, 7, 7, CA3_RDC_CA_CNTL_REG_8_);

#define BYTE0_RDC_RDQS_M_CNTL_REG_7_0_          BYTE0_RDC_RDQS_M_CNTL_REG_7_0_
DECLARE_REG(0x5ac, 7, 0, BYTE0_RDC_RDQS_M_CNTL_REG_7_0_);

#define BYTE0_RDC_RDQS_M_CNTL_REG_8_            BYTE0_RDC_RDQS_M_CNTL_REG_8_
DECLARE_REG(0x5ad, 7, 7, BYTE0_RDC_RDQS_M_CNTL_REG_8_);

#define BYTE0_RDC_RDQS_S_CNTL_REG_7_0_          BYTE0_RDC_RDQS_S_CNTL_REG_7_0_
DECLARE_REG(0x5ae, 7, 0, BYTE0_RDC_RDQS_S_CNTL_REG_7_0_);

#define BYTE0_RDC_RDQS_S_CNTL_REG_8_            BYTE0_RDC_RDQS_S_CNTL_REG_8_
DECLARE_REG(0x5af, 7, 7, BYTE0_RDC_RDQS_S_CNTL_REG_8_);

#define BYTE0_RDC_DQS_CNTL_REG_7_0_             BYTE0_RDC_DQS_CNTL_REG_7_0_
DECLARE_REG(0x5b0, 7, 0, BYTE0_RDC_DQS_CNTL_REG_7_0_);

#define BYTE0_RDC_DQS_CNTL_REG_8_               BYTE0_RDC_DQS_CNTL_REG_8_
DECLARE_REG(0x5b1, 7, 7, BYTE0_RDC_DQS_CNTL_REG_8_);

#define BYTE0_RDC_DQ_CNTL_REG_7_0_              BYTE0_RDC_DQ_CNTL_REG_7_0_
DECLARE_REG(0x5b2, 7, 0, BYTE0_RDC_DQ_CNTL_REG_7_0_);

#define BYTE0_RDC_DQ_CNTL_REG_8_                BYTE0_RDC_DQ_CNTL_REG_8_
DECLARE_REG(0x5b3, 7, 7, BYTE0_RDC_DQ_CNTL_REG_8_);

#define BYTE1_RDC_RDQS_M_CNTL_REG_7_0_          BYTE1_RDC_RDQS_M_CNTL_REG_7_0_
DECLARE_REG(0x5b4, 7, 0, BYTE1_RDC_RDQS_M_CNTL_REG_7_0_);

#define BYTE1_RDC_RDQS_M_CNTL_REG_8_            BYTE1_RDC_RDQS_M_CNTL_REG_8_
DECLARE_REG(0x5b5, 7, 7, BYTE1_RDC_RDQS_M_CNTL_REG_8_);

#define BYTE1_RDC_RDQS_S_CNTL_REG_7_0_          BYTE1_RDC_RDQS_S_CNTL_REG_7_0_
DECLARE_REG(0x5b6, 7, 0, BYTE1_RDC_RDQS_S_CNTL_REG_7_0_);

#define BYTE1_RDC_RDQS_S_CNTL_REG_8_            BYTE1_RDC_RDQS_S_CNTL_REG_8_
DECLARE_REG(0x5b7, 7, 7, BYTE1_RDC_RDQS_S_CNTL_REG_8_);

#define BYTE1_RDC_DQS_CNTL_REG_7_0_             BYTE1_RDC_DQS_CNTL_REG_7_0_
DECLARE_REG(0x5b8, 7, 0, BYTE1_RDC_DQS_CNTL_REG_7_0_);

#define BYTE1_RDC_DQS_CNTL_REG_8_               BYTE1_RDC_DQS_CNTL_REG_8_
DECLARE_REG(0x5b9, 7, 7, BYTE1_RDC_DQS_CNTL_REG_8_);

#define BYTE1_RDC_DQ_CNTL_REG_7_0_              BYTE1_RDC_DQ_CNTL_REG_7_0_
DECLARE_REG(0x5ba, 7, 0, BYTE1_RDC_DQ_CNTL_REG_7_0_);

#define BYTE1_RDC_DQ_CNTL_REG_8_                BYTE1_RDC_DQ_CNTL_REG_8_
DECLARE_REG(0x5bb, 7, 7, BYTE1_RDC_DQ_CNTL_REG_8_);

#define BYTE2_RDC_RDQS_M_CNTL_REG_7_0_          BYTE2_RDC_RDQS_M_CNTL_REG_7_0_
DECLARE_REG(0x5bc, 7, 0, BYTE2_RDC_RDQS_M_CNTL_REG_7_0_);

#define BYTE2_RDC_RDQS_M_CNTL_REG_8_            BYTE2_RDC_RDQS_M_CNTL_REG_8_
DECLARE_REG(0x5bd, 7, 7, BYTE2_RDC_RDQS_M_CNTL_REG_8_);

#define BYTE2_RDC_RDQS_S_CNTL_REG_7_0_          BYTE2_RDC_RDQS_S_CNTL_REG_7_0_
DECLARE_REG(0x5be, 7, 0, BYTE2_RDC_RDQS_S_CNTL_REG_7_0_);

#define BYTE2_RDC_RDQS_S_CNTL_REG_8_            BYTE2_RDC_RDQS_S_CNTL_REG_8_
DECLARE_REG(0x5bf, 7, 7, BYTE2_RDC_RDQS_S_CNTL_REG_8_);

#define BYTE2_RDC_DQS_CNTL_REG_7_0_             BYTE2_RDC_DQS_CNTL_REG_7_0_
DECLARE_REG(0x5c0, 7, 0, BYTE2_RDC_DQS_CNTL_REG_7_0_);

#define BYTE2_RDC_DQS_CNTL_REG_8_               BYTE2_RDC_DQS_CNTL_REG_8_
DECLARE_REG(0x5c1, 7, 7, BYTE2_RDC_DQS_CNTL_REG_8_);

#define BYTE2_RDC_DQ_CNTL_REG_7_0_              BYTE2_RDC_DQ_CNTL_REG_7_0_
DECLARE_REG(0x5c2, 7, 0, BYTE2_RDC_DQ_CNTL_REG_7_0_);

#define BYTE2_RDC_DQ_CNTL_REG_8_                BYTE2_RDC_DQ_CNTL_REG_8_
DECLARE_REG(0x5c3, 7, 7, BYTE2_RDC_DQ_CNTL_REG_8_);

#define BYTE3_RDC_RDQS_M_CNTL_REG_7_0_          BYTE3_RDC_RDQS_M_CNTL_REG_7_0_
DECLARE_REG(0x5c4, 7, 0, BYTE3_RDC_RDQS_M_CNTL_REG_7_0_);

#define BYTE3_RDC_RDQS_M_CNTL_REG_8_            BYTE3_RDC_RDQS_M_CNTL_REG_8_
DECLARE_REG(0x5c5, 7, 7, BYTE3_RDC_RDQS_M_CNTL_REG_8_);

#define BYTE3_RDC_RDQS_S_CNTL_REG_7_0_          BYTE3_RDC_RDQS_S_CNTL_REG_7_0_
DECLARE_REG(0x5c6, 7, 0, BYTE3_RDC_RDQS_S_CNTL_REG_7_0_);

#define BYTE3_RDC_RDQS_S_CNTL_REG_8_            BYTE3_RDC_RDQS_S_CNTL_REG_8_
DECLARE_REG(0x5c7, 7, 7, BYTE3_RDC_RDQS_S_CNTL_REG_8_);

#define BYTE3_RDC_DQS_CNTL_REG_7_0_             BYTE3_RDC_DQS_CNTL_REG_7_0_
DECLARE_REG(0x5c8, 7, 0, BYTE3_RDC_DQS_CNTL_REG_7_0_);

#define BYTE3_RDC_DQS_CNTL_REG_8_               BYTE3_RDC_DQS_CNTL_REG_8_
DECLARE_REG(0x5c9, 7, 7, BYTE3_RDC_DQS_CNTL_REG_8_);

#define BYTE3_RDC_DQ_CNTL_REG_7_0_              BYTE3_RDC_DQ_CNTL_REG_7_0_
DECLARE_REG(0x5ca, 7, 0, BYTE3_RDC_DQ_CNTL_REG_7_0_);

#define BYTE3_RDC_DQ_CNTL_REG_8_                BYTE3_RDC_DQ_CNTL_REG_8_
DECLARE_REG(0x5cb, 7, 7, BYTE3_RDC_DQ_CNTL_REG_8_);

#define ZQK_RDC_CNTL_REG_7_0_                   ZQK_RDC_CNTL_REG_7_0_
DECLARE_REG(0x5cc, 7, 0, ZQK_RDC_CNTL_REG_7_0_);

#define ZQK_RDC_CNTL_REG_8_                     ZQK_RDC_CNTL_REG_8_
DECLARE_REG(0x5cd, 7, 7, ZQK_RDC_CNTL_REG_8_);

#define CA0_RDC_CK_DLY_OUT_EN                   CA0_RDC_CK_DLY_OUT_EN
DECLARE_REG(0x868, 7, 7, CA0_RDC_CK_DLY_OUT_EN);

#define CA0_RDC_CKE_CS_DLY_OUT_EN               CA0_RDC_CKE_CS_DLY_OUT_EN
DECLARE_REG(0x868, 6, 6, CA0_RDC_CKE_CS_DLY_OUT_EN);

#define CA0_RDC_CA_DLY_OUT_EN                   CA0_RDC_CA_DLY_OUT_EN
DECLARE_REG(0x868, 5, 5, CA0_RDC_CA_DLY_OUT_EN);

#define CA0_RDC_CKE_CS_DUAL_DLY_OUT_EN          CA0_RDC_CKE_CS_DUAL_DLY_OUT_EN
DECLARE_REG(0x868, 4, 4, CA0_RDC_CKE_CS_DUAL_DLY_OUT_EN);

#define CA1_RDC_CK_DLY_OUT_EN                   CA1_RDC_CK_DLY_OUT_EN
DECLARE_REG(0x868, 3, 3, CA1_RDC_CK_DLY_OUT_EN);

#define CA1_RDC_CKE_CS_DLY_OUT_EN               CA1_RDC_CKE_CS_DLY_OUT_EN
DECLARE_REG(0x868, 2, 2, CA1_RDC_CKE_CS_DLY_OUT_EN);

#define CA1_RDC_CA_DLY_OUT_EN                   CA1_RDC_CA_DLY_OUT_EN
DECLARE_REG(0x868, 1, 1, CA1_RDC_CA_DLY_OUT_EN);

#define CA1_RDC_CKE_CS_DUAL_DLY_OUT_EN          CA1_RDC_CKE_CS_DUAL_DLY_OUT_EN
DECLARE_REG(0x868, 0, 0, CA1_RDC_CKE_CS_DUAL_DLY_OUT_EN);

#define CA2_RDC_CA_DLY_OUT_EN                   CA2_RDC_CA_DLY_OUT_EN
DECLARE_REG(0x869, 7, 7, CA2_RDC_CA_DLY_OUT_EN);

#define CA3_RDC_CA_DLY_OUT_EN                   CA3_RDC_CA_DLY_OUT_EN
DECLARE_REG(0x869, 6, 6, CA3_RDC_CA_DLY_OUT_EN);

#define ZQK_RDC_DLY_OUT_EN                      ZQK_RDC_DLY_OUT_EN
DECLARE_REG(0x869, 0, 0, ZQK_RDC_DLY_OUT_EN);

#define BYTE0_RDC_RDQS_M_DLY_OUT_EN             BYTE0_RDC_RDQS_M_DLY_OUT_EN
DECLARE_REG(0x86a, 7, 7, BYTE0_RDC_RDQS_M_DLY_OUT_EN);

#define BYTE0_RDC_RDQS_S_DLY_OUT_EN             BYTE0_RDC_RDQS_S_DLY_OUT_EN
DECLARE_REG(0x86a, 6, 6, BYTE0_RDC_RDQS_S_DLY_OUT_EN);

#define BYTE0_RDC_DQS_DLY_OUT_EN                BYTE0_RDC_DQS_DLY_OUT_EN
DECLARE_REG(0x86a, 5, 5, BYTE0_RDC_DQS_DLY_OUT_EN);

#define BYTE0_RDC_DQ_DLY_OUT_EN                 BYTE0_RDC_DQ_DLY_OUT_EN
DECLARE_REG(0x86a, 4, 4, BYTE0_RDC_DQ_DLY_OUT_EN);

#define BYTE1_RDC_RDQS_M_DLY_OUT_EN             BYTE1_RDC_RDQS_M_DLY_OUT_EN
DECLARE_REG(0x86a, 3, 3, BYTE1_RDC_RDQS_M_DLY_OUT_EN);

#define BYTE1_RDC_RDQS_S_DLY_OUT_EN             BYTE1_RDC_RDQS_S_DLY_OUT_EN
DECLARE_REG(0x86a, 2, 2, BYTE1_RDC_RDQS_S_DLY_OUT_EN);

#define BYTE1_RDC_DQS_DLY_OUT_EN                BYTE1_RDC_DQS_DLY_OUT_EN
DECLARE_REG(0x86a, 1, 1, BYTE1_RDC_DQS_DLY_OUT_EN);

#define BYTE1_RDC_DQ_DLY_OUT_EN                 BYTE1_RDC_DQ_DLY_OUT_EN
DECLARE_REG(0x86a, 0, 0, BYTE1_RDC_DQ_DLY_OUT_EN);

#define BYTE2_RDC_RDQS_M_DLY_OUT_EN             BYTE2_RDC_RDQS_M_DLY_OUT_EN
DECLARE_REG(0x86b, 7, 7, BYTE2_RDC_RDQS_M_DLY_OUT_EN);

#define BYTE2_RDC_RDQS_S_DLY_OUT_EN             BYTE2_RDC_RDQS_S_DLY_OUT_EN
DECLARE_REG(0x86b, 6, 6, BYTE2_RDC_RDQS_S_DLY_OUT_EN);

#define BYTE2_RDC_DQS_DLY_OUT_EN                BYTE2_RDC_DQS_DLY_OUT_EN
DECLARE_REG(0x86b, 5, 5, BYTE2_RDC_DQS_DLY_OUT_EN);

#define BYTE2_RDC_DQ_DLY_OUT_EN                 BYTE2_RDC_DQ_DLY_OUT_EN
DECLARE_REG(0x86b, 4, 4, BYTE2_RDC_DQ_DLY_OUT_EN);

#define BYTE3_RDC_RDQS_M_DLY_OUT_EN             BYTE3_RDC_RDQS_M_DLY_OUT_EN
DECLARE_REG(0x86b, 3, 3, BYTE3_RDC_RDQS_M_DLY_OUT_EN);

#define BYTE3_RDC_RDQS_S_DLY_OUT_EN             BYTE3_RDC_RDQS_S_DLY_OUT_EN
DECLARE_REG(0x86b, 2, 2, BYTE3_RDC_RDQS_S_DLY_OUT_EN);

#define BYTE3_RDC_DQS_DLY_OUT_EN                BYTE3_RDC_DQS_DLY_OUT_EN
DECLARE_REG(0x86b, 1, 1, BYTE3_RDC_DQS_DLY_OUT_EN);

#define BYTE3_RDC_DQ_DLY_OUT_EN                 BYTE3_RDC_DQ_DLY_OUT_EN
DECLARE_REG(0x86b, 0, 0, BYTE3_RDC_DQ_DLY_OUT_EN);

#define ZQK_RDC_CNTL_DLL_PHASE_6_0_             ZQK_RDC_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x111, 6, 0, ZQK_RDC_CNTL_DLL_PHASE_6_0_);

#define R_BYTE3_BYPASS_DLL_PHASE_RDQS_M         R_BYTE3_BYPASS_DLL_PHASE_RDQS_M
DECLARE_REG(0x112, 7, 7, R_BYTE3_BYPASS_DLL_PHASE_RDQS_M);

#define R_BYTE2_BYPASS_DLL_PHASE_RDQS_M         R_BYTE2_BYPASS_DLL_PHASE_RDQS_M
DECLARE_REG(0x112, 6, 6, R_BYTE2_BYPASS_DLL_PHASE_RDQS_M);

#define R_BYTE1_BYPASS_DLL_PHASE_RDQS_M         R_BYTE1_BYPASS_DLL_PHASE_RDQS_M
DECLARE_REG(0x112, 5, 5, R_BYTE1_BYPASS_DLL_PHASE_RDQS_M);

#define R_BYTE0_BYPASS_DLL_PHASE_RDQS_M         R_BYTE0_BYPASS_DLL_PHASE_RDQS_M
DECLARE_REG(0x112, 4, 4, R_BYTE0_BYPASS_DLL_PHASE_RDQS_M);

#define R_BYTE3_BYPASS_DLL_PHASE_RDQS_S         R_BYTE3_BYPASS_DLL_PHASE_RDQS_S
DECLARE_REG(0x112, 3, 3, R_BYTE3_BYPASS_DLL_PHASE_RDQS_S);

#define R_BYTE2_BYPASS_DLL_PHASE_RDQS_S         R_BYTE2_BYPASS_DLL_PHASE_RDQS_S
DECLARE_REG(0x112, 2, 2, R_BYTE2_BYPASS_DLL_PHASE_RDQS_S);

#define R_BYTE1_BYPASS_DLL_PHASE_RDQS_S         R_BYTE1_BYPASS_DLL_PHASE_RDQS_S
DECLARE_REG(0x112, 1, 1, R_BYTE1_BYPASS_DLL_PHASE_RDQS_S);

#define R_BYTE0_BYPASS_DLL_PHASE_RDQS_S         R_BYTE0_BYPASS_DLL_PHASE_RDQS_S
DECLARE_REG(0x112, 0, 0, R_BYTE0_BYPASS_DLL_PHASE_RDQS_S);

#define R_BYTE3_BYPASS_DLL_PHASE_DQS            R_BYTE3_BYPASS_DLL_PHASE_DQS
DECLARE_REG(0x113, 7, 7, R_BYTE3_BYPASS_DLL_PHASE_DQS);

#define R_BYTE2_BYPASS_DLL_PHASE_DQS            R_BYTE2_BYPASS_DLL_PHASE_DQS
DECLARE_REG(0x113, 6, 6, R_BYTE2_BYPASS_DLL_PHASE_DQS);

#define R_BYTE1_BYPASS_DLL_PHASE_DQS            R_BYTE1_BYPASS_DLL_PHASE_DQS
DECLARE_REG(0x113, 5, 5, R_BYTE1_BYPASS_DLL_PHASE_DQS);

#define R_BYTE0_BYPASS_DLL_PHASE_DQS            R_BYTE0_BYPASS_DLL_PHASE_DQS
DECLARE_REG(0x113, 4, 4, R_BYTE0_BYPASS_DLL_PHASE_DQS);

#define R_BYTE3_BYPASS_DLL_PHASE_DQ             R_BYTE3_BYPASS_DLL_PHASE_DQ
DECLARE_REG(0x113, 3, 3, R_BYTE3_BYPASS_DLL_PHASE_DQ);

#define R_BYTE2_BYPASS_DLL_PHASE_DQ             R_BYTE2_BYPASS_DLL_PHASE_DQ
DECLARE_REG(0x113, 2, 2, R_BYTE2_BYPASS_DLL_PHASE_DQ);

#define R_BYTE1_BYPASS_DLL_PHASE_DQ             R_BYTE1_BYPASS_DLL_PHASE_DQ
DECLARE_REG(0x113, 1, 1, R_BYTE1_BYPASS_DLL_PHASE_DQ);

#define R_BYTE0_BYPASS_DLL_PHASE_DQ             R_BYTE0_BYPASS_DLL_PHASE_DQ
DECLARE_REG(0x113, 0, 0, R_BYTE0_BYPASS_DLL_PHASE_DQ);

#define R_BYPASS_DLL_PHASE_ZQK                  R_BYPASS_DLL_PHASE_ZQK
DECLARE_REG(0x114, 7, 7, R_BYPASS_DLL_PHASE_ZQK);

#define R_BYTE3_RDQS_M_TRAINING_BYPASS          R_BYTE3_RDQS_M_TRAINING_BYPASS
DECLARE_REG(0x115, 7, 7, R_BYTE3_RDQS_M_TRAINING_BYPASS);

#define R_BYTE2_RDQS_M_TRAINING_BYPASS          R_BYTE2_RDQS_M_TRAINING_BYPASS
DECLARE_REG(0x115, 6, 6, R_BYTE2_RDQS_M_TRAINING_BYPASS);

#define R_BYTE1_RDQS_M_TRAINING_BYPASS          R_BYTE1_RDQS_M_TRAINING_BYPASS
DECLARE_REG(0x115, 5, 5, R_BYTE1_RDQS_M_TRAINING_BYPASS);

#define R_BYTE0_RDQS_M_TRAINING_BYPASS          R_BYTE0_RDQS_M_TRAINING_BYPASS
DECLARE_REG(0x115, 4, 4, R_BYTE0_RDQS_M_TRAINING_BYPASS);

#define R_BYTE3_RDQS_S_TRAINING_BYPASS          R_BYTE3_RDQS_S_TRAINING_BYPASS
DECLARE_REG(0x115, 3, 3, R_BYTE3_RDQS_S_TRAINING_BYPASS);

#define R_BYTE2_RDQS_S_TRAINING_BYPASS          R_BYTE2_RDQS_S_TRAINING_BYPASS
DECLARE_REG(0x115, 2, 2, R_BYTE2_RDQS_S_TRAINING_BYPASS);

#define R_BYTE1_RDQS_S_TRAINING_BYPASS          R_BYTE1_RDQS_S_TRAINING_BYPASS
DECLARE_REG(0x115, 1, 1, R_BYTE1_RDQS_S_TRAINING_BYPASS);

#define R_BYTE0_RDQS_S_TRAINING_BYPASS          R_BYTE0_RDQS_S_TRAINING_BYPASS
DECLARE_REG(0x115, 0, 0, R_BYTE0_RDQS_S_TRAINING_BYPASS);

#define R_BYTE3_DQS_TRAINING_BYPASS             R_BYTE3_DQS_TRAINING_BYPASS
DECLARE_REG(0x116, 7, 7, R_BYTE3_DQS_TRAINING_BYPASS);

#define R_BYTE2_DQS_TRAINING_BYPASS             R_BYTE2_DQS_TRAINING_BYPASS
DECLARE_REG(0x116, 6, 6, R_BYTE2_DQS_TRAINING_BYPASS);

#define R_BYTE1_DQS_TRAINING_BYPASS             R_BYTE1_DQS_TRAINING_BYPASS
DECLARE_REG(0x116, 5, 5, R_BYTE1_DQS_TRAINING_BYPASS);

#define R_BYTE0_DQS_TRAINING_BYPASS             R_BYTE0_DQS_TRAINING_BYPASS
DECLARE_REG(0x116, 4, 4, R_BYTE0_DQS_TRAINING_BYPASS);

#define R_BYTE3_DQ_TRAINING_BYPASS              R_BYTE3_DQ_TRAINING_BYPASS
DECLARE_REG(0x116, 3, 3, R_BYTE3_DQ_TRAINING_BYPASS);

#define R_BYTE2_DQ_TRAINING_BYPASS              R_BYTE2_DQ_TRAINING_BYPASS
DECLARE_REG(0x116, 2, 2, R_BYTE2_DQ_TRAINING_BYPASS);

#define R_BYTE1_DQ_TRAINING_BYPASS              R_BYTE1_DQ_TRAINING_BYPASS
DECLARE_REG(0x116, 1, 1, R_BYTE1_DQ_TRAINING_BYPASS);

#define R_BYTE0_DQ_TRAINING_BYPASS              R_BYTE0_DQ_TRAINING_BYPASS
DECLARE_REG(0x116, 0, 0, R_BYTE0_DQ_TRAINING_BYPASS);

#define R_BYTE3_DLY_OUT_EN_RDQS_M_TRAINING_BYPASSR_BYTE3_DLY_OUT_EN_RDQS_M_TRAINING_BYPASS
DECLARE_REG(0x117, 7, 7, R_BYTE3_DLY_OUT_EN_RDQS_M_TRAINING_BYPASS);

#define R_BYTE2_DLY_OUT_EN_RDQS_M_TRAINING_BYPASSR_BYTE2_DLY_OUT_EN_RDQS_M_TRAINING_BYPASS
DECLARE_REG(0x117, 6, 6, R_BYTE2_DLY_OUT_EN_RDQS_M_TRAINING_BYPASS);

#define R_BYTE1_DLY_OUT_EN_RDQS_M_TRAINING_BYPASSR_BYTE1_DLY_OUT_EN_RDQS_M_TRAINING_BYPASS
DECLARE_REG(0x117, 5, 5, R_BYTE1_DLY_OUT_EN_RDQS_M_TRAINING_BYPASS);

#define R_BYTE0_DLY_OUT_EN_RDQS_M_TRAINING_BYPASSR_BYTE0_DLY_OUT_EN_RDQS_M_TRAINING_BYPASS
DECLARE_REG(0x117, 4, 4, R_BYTE0_DLY_OUT_EN_RDQS_M_TRAINING_BYPASS);

#define R_BYTE3_DLY_OUT_EN_RDQS_S_TRAINING_BYPASSR_BYTE3_DLY_OUT_EN_RDQS_S_TRAINING_BYPASS
DECLARE_REG(0x117, 3, 3, R_BYTE3_DLY_OUT_EN_RDQS_S_TRAINING_BYPASS);

#define R_BYTE2_DLY_OUT_EN_RDQS_S_TRAINING_BYPASSR_BYTE2_DLY_OUT_EN_RDQS_S_TRAINING_BYPASS
DECLARE_REG(0x117, 2, 2, R_BYTE2_DLY_OUT_EN_RDQS_S_TRAINING_BYPASS);

#define R_BYTE1_DLY_OUT_EN_RDQS_S_TRAINING_BYPASSR_BYTE1_DLY_OUT_EN_RDQS_S_TRAINING_BYPASS
DECLARE_REG(0x117, 1, 1, R_BYTE1_DLY_OUT_EN_RDQS_S_TRAINING_BYPASS);

#define R_BYTE0_DLY_OUT_EN_RDQS_S_TRAINING_BYPASSR_BYTE0_DLY_OUT_EN_RDQS_S_TRAINING_BYPASS
DECLARE_REG(0x117, 0, 0, R_BYTE0_DLY_OUT_EN_RDQS_S_TRAINING_BYPASS);

#define R_BYTE3_DLY_OUT_EN_DQS_TRAINING_BYPASS  R_BYTE3_DLY_OUT_EN_DQS_TRAINING_BYPASS
DECLARE_REG(0x118, 7, 7, R_BYTE3_DLY_OUT_EN_DQS_TRAINING_BYPASS);

#define R_BYTE2_DLY_OUT_EN_DQS_TRAINING_BYPASS  R_BYTE2_DLY_OUT_EN_DQS_TRAINING_BYPASS
DECLARE_REG(0x118, 6, 6, R_BYTE2_DLY_OUT_EN_DQS_TRAINING_BYPASS);

#define R_BYTE1_DLY_OUT_EN_DQS_TRAINING_BYPASS  R_BYTE1_DLY_OUT_EN_DQS_TRAINING_BYPASS
DECLARE_REG(0x118, 5, 5, R_BYTE1_DLY_OUT_EN_DQS_TRAINING_BYPASS);

#define R_BYTE0_DLY_OUT_EN_DQS_TRAINING_BYPASS  R_BYTE0_DLY_OUT_EN_DQS_TRAINING_BYPASS
DECLARE_REG(0x118, 4, 4, R_BYTE0_DLY_OUT_EN_DQS_TRAINING_BYPASS);

#define R_BYTE3_DLY_OUT_EN_DQ_TRAINING_BYPASS   R_BYTE3_DLY_OUT_EN_DQ_TRAINING_BYPASS
DECLARE_REG(0x118, 3, 3, R_BYTE3_DLY_OUT_EN_DQ_TRAINING_BYPASS);

#define R_BYTE2_DLY_OUT_EN_DQ_TRAINING_BYPASS   R_BYTE2_DLY_OUT_EN_DQ_TRAINING_BYPASS
DECLARE_REG(0x118, 2, 2, R_BYTE2_DLY_OUT_EN_DQ_TRAINING_BYPASS);

#define R_BYTE1_DLY_OUT_EN_DQ_TRAINING_BYPASS   R_BYTE1_DLY_OUT_EN_DQ_TRAINING_BYPASS
DECLARE_REG(0x118, 1, 1, R_BYTE1_DLY_OUT_EN_DQ_TRAINING_BYPASS);

#define R_BYTE0_DLY_OUT_EN_DQ_TRAINING_BYPASS   R_BYTE0_DLY_OUT_EN_DQ_TRAINING_BYPASS
DECLARE_REG(0x118, 0, 0, R_BYTE0_DLY_OUT_EN_DQ_TRAINING_BYPASS);

#define BYTE0_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_    BYTE0_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x100, 6, 0, BYTE0_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_);

#define BYTE1_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_    BYTE1_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x101, 6, 0, BYTE1_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_);

#define BYTE2_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_    BYTE2_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x102, 6, 0, BYTE2_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_);

#define BYTE3_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_    BYTE3_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x103, 6, 0, BYTE3_RDC_RDQS_M_CNTL_DLL_PHASE_6_0_);

#define BYTE0_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_    BYTE0_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x104, 6, 0, BYTE0_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_);

#define BYTE1_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_    BYTE1_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x105, 6, 0, BYTE1_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_);

#define BYTE2_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_    BYTE2_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x106, 6, 0, BYTE2_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_);

#define BYTE3_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_    BYTE3_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x107, 6, 0, BYTE3_RDC_RDQS_S_CNTL_DLL_PHASE_6_0_);

#define BYTE0_RDC_DQS_CNTL_DLL_PHASE_6_0_       BYTE0_RDC_DQS_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x108, 6, 0, BYTE0_RDC_DQS_CNTL_DLL_PHASE_6_0_);

#define BYTE1_RDC_DQS_CNTL_DLL_PHASE_6_0_       BYTE1_RDC_DQS_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x109, 6, 0, BYTE1_RDC_DQS_CNTL_DLL_PHASE_6_0_);

#define BYTE2_RDC_DQS_CNTL_DLL_PHASE_6_0_       BYTE2_RDC_DQS_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x10a, 6, 0, BYTE2_RDC_DQS_CNTL_DLL_PHASE_6_0_);

#define BYTE3_RDC_DQS_CNTL_DLL_PHASE_6_0_       BYTE3_RDC_DQS_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x10b, 6, 0, BYTE3_RDC_DQS_CNTL_DLL_PHASE_6_0_);

#define BYTE0_RDC_DQ_CNTL_DLL_PHASE_6_0_        BYTE0_RDC_DQ_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x10c, 6, 0, BYTE0_RDC_DQ_CNTL_DLL_PHASE_6_0_);

#define BYTE1_RDC_DQ_CNTL_DLL_PHASE_6_0_        BYTE1_RDC_DQ_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x10d, 6, 0, BYTE1_RDC_DQ_CNTL_DLL_PHASE_6_0_);

#define BYTE2_RDC_DQ_CNTL_DLL_PHASE_6_0_        BYTE2_RDC_DQ_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x10e, 6, 0, BYTE2_RDC_DQ_CNTL_DLL_PHASE_6_0_);

#define BYTE3_RDC_DQ_CNTL_DLL_PHASE_6_0_        BYTE3_RDC_DQ_CNTL_DLL_PHASE_6_0_
DECLARE_REG(0x10f, 6, 0, BYTE3_RDC_DQ_CNTL_DLL_PHASE_6_0_);

#define WEYE_DELAY_BYTE_00_7_0_                 WEYE_DELAY_BYTE_00_7_0_
DECLARE_REG(0x8e1, 7, 0, WEYE_DELAY_BYTE_00_7_0_);

#define WEYE_DELAY_BYTE_01_7_0_                 WEYE_DELAY_BYTE_01_7_0_
DECLARE_REG(0x8e2, 7, 0, WEYE_DELAY_BYTE_01_7_0_);

#define WEYE_DELAY_BYTE_02_7_0_                 WEYE_DELAY_BYTE_02_7_0_
DECLARE_REG(0x8e3, 7, 0, WEYE_DELAY_BYTE_02_7_0_);

#define WEYE_DELAY_BYTE_03_7_0_                 WEYE_DELAY_BYTE_03_7_0_
DECLARE_REG(0x8e4, 7, 0, WEYE_DELAY_BYTE_03_7_0_);

#define DLY_OUT_EN_TRAINING_BYPASS_REG          DLY_OUT_EN_TRAINING_BYPASS_REG
DECLARE_REG(0x003, 7, 7, DLY_OUT_EN_TRAINING_BYPASS_REG);

#define CK1_TRAINING_BYPASS_REG                 CK1_TRAINING_BYPASS_REG
DECLARE_REG(0x003, 3, 3, CK1_TRAINING_BYPASS_REG);

#define CK0_TRAINING_BYPASS_REG                 CK0_TRAINING_BYPASS_REG
DECLARE_REG(0x003, 2, 2, CK0_TRAINING_BYPASS_REG);

#define CK1_DLY_OUT_EN_TRAINING_BYPASS_REG      CK1_DLY_OUT_EN_TRAINING_BYPASS_REG
DECLARE_REG(0x003, 1, 1, CK1_DLY_OUT_EN_TRAINING_BYPASS_REG);

#define CK0_DLY_OUT_EN_TRAINING_BYPASS_REG      CK0_DLY_OUT_EN_TRAINING_BYPASS_REG
DECLARE_REG(0x003, 0, 0, CK0_DLY_OUT_EN_TRAINING_BYPASS_REG);

#define CA3_TRAINING_BYPASS_REG                 CA3_TRAINING_BYPASS_REG
DECLARE_REG(0x024, 7, 7, CA3_TRAINING_BYPASS_REG);

#define CA2_TRAINING_BYPASS_REG                 CA2_TRAINING_BYPASS_REG
DECLARE_REG(0x024, 6, 6, CA2_TRAINING_BYPASS_REG);

#define CA1_TRAINING_BYPASS_REG                 CA1_TRAINING_BYPASS_REG
DECLARE_REG(0x024, 5, 5, CA1_TRAINING_BYPASS_REG);

#define CA0_TRAINING_BYPASS_REG                 CA0_TRAINING_BYPASS_REG
DECLARE_REG(0x024, 4, 4, CA0_TRAINING_BYPASS_REG);

#define CA3_DLY_OUT_EN_TRAINING_BYPASS_REG      CA3_DLY_OUT_EN_TRAINING_BYPASS_REG
DECLARE_REG(0x024, 3, 3, CA3_DLY_OUT_EN_TRAINING_BYPASS_REG);

#define CA2_DLY_OUT_EN_TRAINING_BYPASS_REG      CA2_DLY_OUT_EN_TRAINING_BYPASS_REG
DECLARE_REG(0x024, 2, 2, CA2_DLY_OUT_EN_TRAINING_BYPASS_REG);

#define CA1_DLY_OUT_EN_TRAINING_BYPASS_REG      CA1_DLY_OUT_EN_TRAINING_BYPASS_REG
DECLARE_REG(0x024, 1, 1, CA1_DLY_OUT_EN_TRAINING_BYPASS_REG);

#define CA0_DLY_OUT_EN_TRAINING_BYPASS_REG      CA0_DLY_OUT_EN_TRAINING_BYPASS_REG
DECLARE_REG(0x024, 0, 0, CA0_DLY_OUT_EN_TRAINING_BYPASS_REG);

//training
int delay_cell_calibration(int phy_id, int ddr_gen, int byte_mask);

//Config
#define CONFIG_ENABLE_DFI_SWITCH_WORKAROUND


//RDC
#define RDC_MAX		(384)
#define is_cmd_phase_rdc(rdc_idx)	((rdc_idx) <= IDX_CA3_RDC_CA_CNTL)
typedef enum {
	IDX_CA0_RDC_CK_CNTL = 0,
	IDX_CA0_RDC_CKE_CS_CNTL,
	IDX_CA0_RDC_CA_CNTL,
	IDX_CA0_RDC_CKE_CS_DUAL_CNTL,
	IDX_CA1_RDC_CK_CNTL,
	IDX_CA1_RDC_CKE_CS_CNTL,
	IDX_CA1_RDC_CA_CNTL,
	IDX_CA1_RDC_CKE_CS_DUAL_CNTL,
	IDX_CA2_RDC_CA_CNTL,
	IDX_CA3_RDC_CA_CNTL,
	IDX_LAST_CMD_PHASE = IDX_CA3_RDC_CA_CNTL,
	IDX_BYTE0_RDC_RDQS_M_CNTL,
	IDX_BYTE0_RDC_RDQS_S_CNTL,
	IDX_BYTE0_RDC_DQS_CNTL,
	IDX_BYTE0_RDC_DQ_CNTL,
	IDX_BYTE1_RDC_RDQS_M_CNTL,
	IDX_BYTE1_RDC_RDQS_S_CNTL,
	IDX_BYTE1_RDC_DQS_CNTL,
	IDX_BYTE1_RDC_DQ_CNTL,
	IDX_BYTE2_RDC_RDQS_M_CNTL,
	IDX_BYTE2_RDC_RDQS_S_CNTL,
	IDX_BYTE2_RDC_DQS_CNTL,
	IDX_BYTE2_RDC_DQ_CNTL,
	IDX_BYTE3_RDC_RDQS_M_CNTL,
	IDX_BYTE3_RDC_RDQS_S_CNTL,
	IDX_BYTE3_RDC_DQS_CNTL,
	IDX_BYTE3_RDC_DQ_CNTL,
	IDX_ZQK_RDC_CNTL,
	RDC_NUM
} rdc_t;

struct rdc_list {
	union {
		unsigned int raw[RDC_NUM];
		struct {
			unsigned int ca0_rdc_ck_cntl;
			unsigned int ca0_rdc_cke_cs_cntl;
			unsigned int ca0_rdc_ca_cntl;
			unsigned int ca0_rdc_cke_cs_dual_cntl;
			unsigned int ca1_rdc_ck_cntl;
			unsigned int ca1_rdc_cke_cs_cntl;
			unsigned int ca1_rdc_ca_cntl;
			unsigned int ca1_rdc_cke_cs_dual_cntl;
			unsigned int ca2_rdc_ca_cntl;
			unsigned int ca3_rdc_ca_cntl;
			unsigned int byte0_rdc_rdqs_m_cntl;
			unsigned int byte0_rdc_rdqs_s_cntl;
			unsigned int byte0_rdc_dqs_cntl;
			unsigned int byte0_rdc_dq_cntl;
			unsigned int byte1_rdc_rdqs_m_cntl;
			unsigned int byte1_rdc_rdqs_s_cntl;
			unsigned int byte1_rdc_dqs_cntl;
			unsigned int byte1_rdc_dq_cntl;
			unsigned int byte2_rdc_rdqs_m_cntl;
			unsigned int byte2_rdc_rdqs_s_cntl;
			unsigned int byte2_rdc_dqs_cntl;
			unsigned int byte2_rdc_dq_cntl;
			unsigned int byte3_rdc_rdqs_m_cntl;
			unsigned int byte3_rdc_rdqs_s_cntl;
			unsigned int byte3_rdc_dqs_cntl;
			unsigned int byte3_rdc_dq_cntl;
			unsigned int zqk_rdc_cntl;
		};
	};
};
void rdc_store(int phy_id);
//extern struct rdc_list *_trained_rdc[];
int phy_dll_phase_set(int phy_id, rdc_t rdc_id, unsigned int dll_phase_64x);
int phy_dll_phase_get(int phy_id, rdc_t rdc_id);
#define convert_dllphase2rdc_round(phase_64, ui2rdc) \
	((phase_64 * ui2rdc + 32) / 64)
#define convert_rdc2dllphase_round(rdc, ui2rdc) \
	(((rdc) * 64 + (ui2rdc) / 2) / (ui2rdc))

/* IO */


/* PHY IO */
#define phy_mask_set_1(id,addr,mask)	phy_mask_set(id,addr,mask,mask)
#define phy_mask_set_0(id,addr,mask)	phy_mask_set(id,addr,mask,0)

#define phy_reg_set_0(phy_id, name) \
	phy_mask_set_0(phy_id, REG_ADDR(name), REG_MASK(name))
	
#define phy_reg_set_1(phy_id, name) \
	phy_mask_set_1(phy_id, REG_ADDR(name), REG_MASK(name))

#define phy_reg_set(phy_id, name, value) \
	phy_mask_set(phy_id, REG_ADDR(name), REG_MASK(name), \
				((value) << REG_SHIFT(name)))
				
#define phy_reg_get(phy_id, name) \
	(phy_mask_get(phy_id, REG_ADDR(name), REG_MASK(name)) >> REG_SHIFT(name))
	
#define phy_reg_poll(phy_id, name, value, timeout) \
	phy_mask_poll(phy_id, REG_ADDR(name), REG_MASK(name), \
			(value) << REG_SHIFT(name), (timeout))
#define phy_reg_poll_1(phy_id, name, timeout) \
	phy_reg_poll(phy_id, name, (REG_MASK(name) >> REG_SHIFT(name)), timeout)
#define phy_reg_poll_0(phy_id, name, timeout) \
	phy_reg_poll(phy_id, name, 0, timeout)
	
uint8_t phy_io_read8(int phy_id, uint16_t addr);
void phy_io_write8(int phy_id, uint16_t addr, uint32_t value);

int phy_mask_poll(int phy_id, uint64_t addr, uint32_t mask,
				uint32_t value, int timeout);


uint8_t phy_mask_get(int phy_id, uint64_t addr, uint32_t mask);
void phy_mask_set(int phy_id, uint64_t addr, uint32_t mask,
							uint32_t val);

int phy_check_rank2(int phy_id);

// bit op
//#define BIT(nr)			(1UL << (nr))

uint8_t maskget(uint64_t addr, uint32_t mask);
void maskset(uint64_t addr, uint32_t mask, uint32_t val);
uint8_t bitmask_get(uint64_t addr, uint32_t bit_high,
					uint32_t bit_low);
void bitmask_set(uint64_t addr, uint32_t bit_high, uint32_t bit_low,
					uint32_t val);
int bitmask_poll(uint64_t addr, uint32_t bit_high, uint32_t bit_low,
					uint32_t val, uint32_t timeout);
void phy_mask_toggle(int phy_id, uint64_t addr, uint32_t mask);

#endif /* NOVATEK_MMAP_H */
