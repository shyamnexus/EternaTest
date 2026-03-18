/*
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    @file       kdrv_vdocdc_main.h

    @brief      kdrv_vdocdc main header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2022/02/28
*/
#ifndef _KDRV_VDOCDC_MAIN_H
#define _KDRV_VDOCDC_MAIN_H
#if defined(__LINUX)
#include <linux/cdev.h>
#endif
//#define H26XE_VER_STR		"0.0.1"		/*(2023/10/25): set to first version */
//#define H26XE_VER_STR		"0.0.2"		/*(2023/12/29): [IVOT_CHIP-26028] add efuse clock check */
//#define H26XE_VER_STR		"0.0.3"		/*(2024/01/04): [NA51115-811] proc add utiliation */
//#define H26XE_VER_STR		"0.0.4"		/*(2024/01/22): [NA51115-913] modify video first encode error */
//#define H26XE_VER_STR		"0.0.5"		/*(2024/02/15): add emode for tuning tool */
//#define H26XE_VER_STR		"0.0.6"		/*(2024/03/11): Correct the return value of the codec's MD ratio */
//#define H26XE_VER_STR		"0.0.7"		/*(2024/03/15): add api of ressup */
//#define H26XE_VER_STR		"0.0.8"		/*(2024/03/22): change default value of customized qp priority */
//#define H26XE_VER_STR		"0.0.9"		/*(2024/04/01): Correct RRC coeff for HW */
//#define H26XE_VER_STR		"0.0.10"	/*(2024/04/09): 1. fix bgrdo. 2. fix dec error handling */
//#define H26XE_VER_STR		"0.0.11"	/*(2024/05/28): support smart encode (dyfr/dygop/smart roi/scene_det) */
//#define H26XE_VER_STR		"0.0.12"	/*(2022/06/06): [NA51115-1050][IVOT_N12009_CO-1076] support privacy mask video */
//#define H26XE_VER_STR		"0.0.13"	/*(2024/09/05): avoid null pointer when kdrv_videoenc_set codec_type */
//#define H26XE_VER_STR		"0.1.0"		/*(2024/09/11): modify for 539 */
//#define H26XE_VER_STR		"0.1.1"		/*(2024/09/30): change default value of customized qp priority for h264 */
//#define H26XE_VER_STR		"0.1.2"		/*(2024/10/22): [NA51115-1566] add non-block mode */
//#define H26XE_VER_STR		"0.1.3"		/*(2024/10/22): [IVOT_N12191_CO-368] set stop for non-block mode */
//#define H26XE_VER_STR		"0.1.4"		/*(2024/10/28): [IVOT_N12191_CO-361] modify for GDR I slice */
//#define H26XE_VER_STR		"0.1.5"		/*(2024/11/01): 1.[IVOT_N12191_CO-400] add error handling. 2.[IVOT_N12191_CO-408] fixed free buffer. */
//#define H26XE_VER_STR		"0.1.6"		/*(2024/11/05): [IVOT_N12191_CO-416] add error handling when hw timeout. */
//#define H26XE_VER_STR		"0.1.7"		/*(2024/11/07): [IVOT_N12191_CO-361] add error handling for slice split. */
//#define H26XE_VER_STR		"0.1.8"		/*(2024/11/11): [IVOT_CHIP-30174] return re-trigger encode status. */
//#define H26XE_VER_STR		"0.1.9"		/*(2024/11/11): [IVOT_N12191_CO-429] Adjust the method for H/W reset when a dec_err_interrupt occurs. */
//#define H26XE_VER_STR		"0.1.10"	/*(2024/12/10): [IVOT_N12191_CO-486] 1. 538 only uses the module reset. 2. Add error handling for bitstream parsing */
#define H26XE_VER_STR		"0.1.11"	/*(2024/12/20): [IVOT_N12191_CO-497] Align the start_addr of the bitstream to 8 bytes. */


#define MODULE_NAME			"kdrv_h26x"
#define MODULE_MINOR_ID		0
#define MODULE_MINOR_COUNT	1

#if defined(__LINUX)
typedef struct kdrv_vdocdc_drv_info {
	struct class *pmodule_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct cdev cdev;
	dev_t dev_id;
} KDRV_VDOCDC_DRV_INFO;
#endif

#endif	// _KDRV_VDOCDC_MAIN_H

