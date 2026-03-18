/*
    SIF module internal header file

    SIF module internal header file

    @file       sif_int.h
    @ingroup    mIDrvIO_SIF
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef __NVT_SIF_PLAT_H__
#define __NVT_SIF_PLAT_H__

#define WAIT_SIF_TIMEOUT            100000

// SIF INTERRUPT STATUS bit definition
#define SIF_INT_DMA(ch)             (0x01000000 << (ch))
#define SIF_INT_END(ch)             (0x00000010 << (ch))
#define SIF_INT_EMPTY(ch)           (0x00000001 << (ch))

// SIF SEN duration
#define SIF_SEND_MIN                1
#define SIF_SEND_MAX                16

// SIF SEN setup time
#define SIF_SENS_MIN                0
#define SIF_SENS_MAX                15

// SIF SEN hold time
#define SIF_SENH_MIN                0
#define SIF_SENH_MAX                7

// SIF transmission length
#define SIF_TRANLEN_MIN             1
#define SIF_TRANLEN_MAX             255
#define SIF_TRANLEN_DEFAULT         1

// SIF continuous mode address bits
#define SIF_CONTADDRBITS_MIN        1
#define SIF_CONTADDRBITS_MAX        255
#define SIF_CONTADDRBITS_DEFAULT    1
 
// SIF delay between successive transmission
#define SIF_DELAY_MIN               0
#define SIF_DELAY_MAX               255
 
// SIF clock divider
#define SIF_DIVIDER_MIN             0
#define SIF_DIVIDER_MAX             255

// Maximum data reigster bits
#define SIF_MAX_DATAREG_BITS        96      // 3 data registers, 32 bits per register

// SIF source clock
#if (_FPGA_EMULATION_ == ENABLE)
// (OSC * 2) * 96 / 480 --> OSC * 2 / 5
// _FPGA_PLL_OSC_ = 24000000
#define SIF_SOURCE_CLOCK            ((24000000 << 1) / 5)
#else
#define SIF_SOURCE_CLOCK            96000000
#endif

// Data port selection
#define SIF_DPSEL_CH0               0
#define SIF_DPSEL_CH1               1
#define SIF_DPSEL_CH2               2
#define SIF_DPSEL_CH3               3
 
// SIF CONFIG bit definition
#define SIF_CONF_START_CH(ch)       (0x00000001 << (ch))
#define SIF_CONF_RESTART_CH(ch)     (0x00010000 << (ch))

// SIF Status bit definition
#define SIF_STATUS_CH(ch)           (0x00000001 << (ch))

// SIF DMA size
#define SIF_DMA_SIZE_MIN            0x4
#define SIF_DMA_SIZE_MAX            0xFFFC

// SIF maximum burst delay in ns
#define SIF_UI_BURST_DELAY_MAX      100000000

// SIF maximum burst delay
#define SIF_BURST_DELAY_MAX         0x927C00



#endif
