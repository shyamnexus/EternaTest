#ifndef _BRIDGE_FOURCC_H
#define _BRIDGE_FOURCC_H

//element's fourcc
#define SENSOR_PRESET_NAME   0x454E5053 //MAKEFOURCC('S', 'P', 'N', 'E');
#define SENSOR_CHGMODE_FPS   0x53464353 //MAKEFOURCC('S', 'C', 'F', 'S');
#define SENSOR_PRESET_EXPT   0x54455053 //MAKEFOURCC('S', 'P', 'E', 'T');
#define SENSOR_PRESET_GAIN   0x4E475053 //MAKEFOURCC('S', 'P', 'G', 'N');
#define SENSOR_EXPT_MAX      0x584D4553 //MAKEFOURCC('S', 'E', 'M', 'X');
#define SENSOR_I2C_ID        0x44494953 //MAKEFOURCC('S', 'I', 'I', 'D');
#define SENSOR_I2C_ADDR      0x52414953 //MAKEFOURCC('S', 'I', 'A', 'R');
#define SENSOR_CUR_MODE      0x4D524353 //MAKEFOURCC('S', 'C', 'R', 'M');
#define SENSOR_ROW_TIME      0x54575253 //MAKEFOURCC('S', 'R', 'W', 'T');
#define SENSOR_VD            0x31445653 //MAKEFOURCC('S', 'V', 'D', '1');
#define SENSOR_DFT_FPS       0x46544453 //MAKEFOURCC('S', 'D', 'T', 'F');
#define SENSOR_FRAME_NUM     0x4E454653 //MAKEFOURCC('S', 'F', 'E', 'N');
#define SENSOR_MAX_GAIN      0x47584D53 //MAKEFOURCC('S', 'M', 'X', 'G');
#define ISP_D_GAIN           0x4E474449 //MAKEFOURCC('I', 'D', 'G', 'N');
#define ISP_R_GAIN           0x4E475249 //MAKEFOURCC('I', 'R', 'G', 'N');
#define ISP_G_GAIN           0x4E474749 //MAKEFOURCC('I', 'G', 'G', 'N');
#define ISP_B_GAIN           0x4E474249 //MAKEFOURCC('I', 'B', 'G', 'N');
#define ISP_SHDR_ENABLE      0x45455349 //MAKEFOURCC('I', 'S', 'E', 'E');
#define ISP_SHDR_PATH        0x48505348 //MAKEFOURCC('I', 'S', 'P', 'H');
#define ISP_SHDR_MASK        0x4B4D5349 //MAKEFOURCC('I', 'S', 'M', 'K');
#define ISP_NR_LV            0x4C524E49 //MAKEFOURCC('I', 'N', 'R', 'L');
#define ISP_3DNR_LV          0x4C523349 //MAKEFOURCC('I', '3', 'R', 'L');
#define ISP_SHARPNESS_LV     0x4C535349 //MAKEFOURCC('I', 'S', 'S', 'L');
#define ISP_SATURATION_LV    0x4C4E5349 //MAKEFOURCC('I', 'S', 'N', 'L');
#define ISP_CONTRAST_LV      0x4C544349 //MAKEFOURCC('I', 'C', 'T', 'L');
#define ISP_BRIGHTNESS_LV    0x4C534248 //MAKEFOURCC('I', 'B', 'S', 'L');
#define ISP_NIGHT_MODE       0x4D544E49 //MAKEFOURCC('I', 'N', 'T', 'M');

#define SENSOR_PRESET_NAME_2 0x324E5053 //MAKEFOURCC('S', 'P', 'N', '2');
#define SENSOR_CHGMODE_FPS_2 0x32464353 //MAKEFOURCC('S', 'C', 'F', '2');
#define SENSOR_PRESET_EXPT_2 0x32455053 //MAKEFOURCC('S', 'P', 'E', '2');
#define SENSOR_PRESET_GAIN_2 0x32475053 //MAKEFOURCC('S', 'P', 'G', '2');
#define SENSOR_EXPT_MAX_2    0x324D4553 //MAKEFOURCC('S', 'E', 'M', '2');
#define SENSOR_I2C_ID_2      0x32494953 //MAKEFOURCC('S', 'I', 'I', '2');
#define SENSOR_I2C_ADDR_2    0x32414953 //MAKEFOURCC('S', 'I', 'A', '2');
#define SENSOR_CUR_MODE_2    0x32524353 //MAKEFOURCC('S', 'C', 'R', '2');
#define SENSOR_ROW_TIME_2    0x32575253 //MAKEFOURCC('S', 'R', 'W', '2');
#define SENSOR_VD_2          0x32445653 //MAKEFOURCC('S', 'V', 'D', '2');
#define SENSOR_DFT_FPS_2     0x32544453 //MAKEFOURCC('S', 'D', 'T', '2')
#define SENSOR_FRAME_NUM_2   0x32454653 //MAKEFOURCC('S', 'F', 'E', '2');
#define SENSOR_MAX_GAIN_2    0x32584D53 //MAKEFOURCC('S', 'M', 'X', '2');
#define ISP_PATH_2           0x32485049 //MAKEFOURCC('I', 'P', 'H', '2');
#define ISP_D_GAIN_2         0x32474449 //MAKEFOURCC('I', 'D', 'G', '2');
#define ISP_R_GAIN_2         0x32475249 //MAKEFOURCC('I', 'R', 'G', '2');
#define ISP_G_GAIN_2         0x32474749 //MAKEFOURCC('I', 'G', 'G', '2');
#define ISP_B_GAIN_2         0x32474249 //MAKEFOURCC('I', 'B', 'G', '2');
#define ISP_SHDR_ENABLE_2    0x32455349 //MAKEFOURCC('I', 'S', 'E', '2');
#define ISP_SHDR_PATH_2      0x32505348 //MAKEFOURCC('I', 'S', 'P', '2');
#define ISP_SHDR_MASK_2      0x324D5349 //MAKEFOURCC('I', 'S', 'M', '2');
#define ISP_NR_LV_2          0x32524E49 //MAKEFOURCC('I', 'N', 'R', '2');
#define ISP_3DNR_LV_2        0x32523349 //MAKEFOURCC('I', '3', 'R', '2');
#define ISP_SHARPNESS_LV_2   0x32535349 //MAKEFOURCC('I', 'S', 'S', '2');
#define ISP_SATURATION_LV_2  0x324E5349 //MAKEFOURCC('I', 'S', 'N', '2');
#define ISP_CONTRAST_LV_2    0x32544349 //MAKEFOURCC('I', 'C', 'T', '2');
#define ISP_BRIGHTNESS_LV_2  0x32534248 //MAKEFOURCC('I', 'B', 'S', '2');
#define ISP_NIGHT_MODE_2     0x32544E49 //MAKEFOURCC('I', 'N', 'T', '2');

// sie
#define SIE_SKIP_ID_BIT      0x42495353 //MAKEFOURCC('S', 'S', 'I', 'B');
#define SIE_FC_ID_1          0x31494653 //MAKEFOURCC('S', 'F', 'I', '1');
#define SIE_FC_ID_2          0x32494653 //MAKEFOURCC('S', 'F', 'I', '2');
#define SIE_FC_ID_3          0x33494653 //MAKEFOURCC('S', 'F', 'I', '3');
#define SIE_FC_ID_4          0x34494653 //MAKEFOURCC('S', 'F', 'I', '4');
#define SIE_FC_ID_5          0x35494653 //MAKEFOURCC('S', 'F', 'I', '5');

#define SIE_TS_ID_1          0x31495453 //MAKEFOURCC('S', 'T', 'I', '1');
#define SIE_TS_ID_2          0x32495453 //MAKEFOURCC('S', 'T', 'I', '2');
#define SIE_TS_ID_3          0x33495453 //MAKEFOURCC('S', 'T', 'I', '3');
#define SIE_TS_ID_4          0x34495453 //MAKEFOURCC('S', 'T', 'I', '4');
#define SIE_TS_ID_5          0x35495453 //MAKEFOURCC('S', 'T', 'I', '5');

#define SIE_ID_1_ADDR_0      0x30413153 //MAKEFOURCC('S', '1', 'A', '0'); SIE1 out buf 0 LSB
#define SIE_ID_1_ADDR_1      0x31413153 //MAKEFOURCC('S', '1', 'A', '1'); SIE1 out buf 1 LSB
#define SIE_ID_1_ADDR_2      0x32413153 //MAKEFOURCC('S', '1', 'A', '2'); SIE1 out buf 0 MSB
#define SIE_ID_1_ADDR_3      0x33413153 //MAKEFOURCC('S', '1', 'A', '3'); SIE1 out buf 1 MSB

#define SIE_ID_2_ADDR_0      0x30413253 //MAKEFOURCC('S', '2', 'A', '0'); SIE2 out buf 0 LSB
#define SIE_ID_2_ADDR_1      0x31413253 //MAKEFOURCC('S', '2', 'A', '1'); SIE2 out buf 1 LSB
#define SIE_ID_2_ADDR_2      0x32413253 //MAKEFOURCC('S', '2', 'A', '2'); SIE2 out buf 0 MSB
#define SIE_ID_2_ADDR_3      0x33413253 //MAKEFOURCC('S', '2', 'A', '3'); SIE2 out buf 1 MSB

#define SIE_ID_3_ADDR_0      0x30413353 //MAKEFOURCC('S', '3', 'A', '0'); SIE3 out buf 0 LSB
#define SIE_ID_3_ADDR_1      0x31413353 //MAKEFOURCC('S', '3', 'A', '1'); SIE3 out buf 1 LSB
#define SIE_ID_3_ADDR_2      0x32413353 //MAKEFOURCC('S', '3', 'A', '2'); SIE3 out buf 0 MSB
#define SIE_ID_3_ADDR_3      0x33413353 //MAKEFOURCC('S', '3', 'A', '3'); SIE3 out buf 1 MSB

#define SIE_ID_4_ADDR_0      0x30413453 //MAKEFOURCC('S', '4', 'A', '0'); SIE4 out buf 0 LSB
#define SIE_ID_4_ADDR_1      0x31413453 //MAKEFOURCC('S', '4', 'A', '1'); SIE4 out buf 1 LSB
#define SIE_ID_4_ADDR_2      0x32413453 //MAKEFOURCC('S', '4', 'A', '2'); SIE4 out buf 0 MSB
#define SIE_ID_4_ADDR_3      0x33413453 //MAKEFOURCC('S', '4', 'A', '3'); SIE4 out buf 1 MSB

#define SIE_ID_5_ADDR_0      0x30413553 //MAKEFOURCC('S', '5', 'A', '0'); SIE5 out buf 0 LSB
#define SIE_ID_5_ADDR_1      0x31413553 //MAKEFOURCC('S', '5', 'A', '1'); SIE5 out buf 1 LSB
#define SIE_ID_5_ADDR_2      0x32413553 //MAKEFOURCC('S', '5', 'A', '2'); SIE5 out buf 0 MSB
#define SIE_ID_5_ADDR_3      0x33413553 //MAKEFOURCC('S', '5', 'A', '3'); SIE5 out buf 1 MSB


#define IPP_RTOS_BIT         0x42525049 //MAKEFOURCC('I', 'P', 'R', 'B');
#define IPP_RTOS_CNT         0x43525049 //MAKEFOURCC('I', 'P', 'R', 'C');
#define IPP_RTOS_ERR         0x45525049 //MAKEFOURCC('I', 'P', 'R', 'E');
#define IPP_RTOS_FRM         0x46525049 //MAKEFOURCC('I', 'P', 'R', 'F');

#define JPG_FRAME_OFS        0x534F504A //MAKEFOURCC('J', 'P', 'O', 'S');
#define JPG_FRAME_SIZE       0x5A53504A //MAKEFOURCC('J', 'P', 'S', 'Z');

#define VENC_PRE_INIT        0x49504556 //MAKEFOURCC('V', 'E', 'P', 'I');
#define VENC_PRE_OBJ         0x4A424F50 //MAKEFOURCC('P', 'O', 'B', 'J');
#define VENC_PRE_VAR         0x52415650 //MAKEFOURCC('P', 'V', 'A', 'R');
#define VENC_EN_INT_FIRST    0x544E4945 //MAKEFOURCC('E', 'I', 'N', 'T');
#define VENC_PRE_NUM_0       0x304E5056 //MAKEFOURCC('V', 'P', N', '0');
#define VENC_BS_ADDR_0       0x30444142 //MAKEFOURCC('B', 'A', D', '0');
#define VENC_BS_SIZE_0       0x305A5342 //MAKEFOURCC('B', 'S', Z', '0');
#define VENC_BS_TIMESTAMP_0  0x30535442 //MAKEFOURCC('B', 'T', S', '0');
#define VENC_BS_NXT_TYPE_0   0x30544E42 //MAKEFOURCC('B', 'N', T', '0');
#define VENC_BS_QP_0         0x30505142 //MAKEFOURCC('B', 'Q', P', '0');
#define VENC_BS1_SIZE_0      0x30315342 //MAKEFOURCC('B', 'S', 1', '0');
#define VENC_BS_FRM_TYPE_0   0x30544642 //MAKEFOURCC('B', 'F', T', '0');
#define VENC_BS_IS_KEY_0     0x304B4942 //MAKEFOURCC('B', 'I', K', '0');

#define IPP_LL_START_TIME    0x54534C4C //MAKEFOURCC('L', 'L', 'S', 'T');
#define AMP_TAKEOVER_FROM_RTOS  0x52465441 //MAKEFOURCC('A', 'T', 'F', 'R');

#endif