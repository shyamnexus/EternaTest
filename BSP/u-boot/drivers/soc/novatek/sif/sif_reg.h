/*
    Register offset and bit definition for SIF module

    Register offset and bit definition for SIF module.

    @file       sif_reg.h
    @ingroup    mIDrvIO_SIF
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef __NVT_SIF_REG_H__
#define __NVT_SIF_REG_H__

#define SIF_SETREG(ofs,value)	OUTW(IOADDR_SIF_REG_BASE+(ofs),(value))
#define SIF_GETREG(ofs)		    INW(IOADDR_SIF_REG_BASE+(ofs))

// SIF Mode Register
#define SIF_MODE_REG_OFS            0x00
union SIF_MODE_REG {
	uint32_t reg;
	struct {
	unsigned int Mode_Ch0:4;
	unsigned int Mode_Ch1:4;
	unsigned int Mode_Ch2:4;
	unsigned int Mode_Ch3:4;
	unsigned int Reserved0:16;
	} bit;
};

// SIF Config Register 0
#define SIF_CONF0_REG_OFS            0x04
union SIF_CONF0_REG {
	uint32_t reg;
	struct {
	unsigned int Start_Ch0:1;
	unsigned int Start_Ch1:1;
	unsigned int Start_Ch2:1;
	unsigned int Start_Ch3:1;
	unsigned int Reserved0:12;
	unsigned int Restart_Ch0:1;
	unsigned int Restart_Ch1:1;
	unsigned int Restart_Ch2:1;
	unsigned int Restart_Ch3:1;
	unsigned int Reserved1:12;
	} bit;
};
 

// SIF Config Register 1
#define SIF_CONF1_REG_OFS            0x08
union SIF_CONF1_REG {
	uint32_t reg;
	struct {
	unsigned int Dir_Ch0:1;
	unsigned int Dir_Ch1:1;
	unsigned int Dir_Ch2:1;
	unsigned int Dir_Ch3:1;
	unsigned int Cont_Ch0:1;
	unsigned int Cont_Ch1:1;
	unsigned int Reserved0:26;
	} bit;
};
 
// SIF Config Register 2
#define SIF_CONF2_REG_OFS            0x0C
union SIF_CONF2_REG {
	uint32_t reg;
	struct {
	unsigned int Ch0_Addr_Bits:8;
	unsigned int Ch1_Addr_Bits:8;
	unsigned int Reserved0:16;
	} bit;
};

// SIF Config Register 2
#define SIF_CLKCTRL0_REG_OFS            0x10
union SIF_CLKCTRL0_REG {
	uint32_t reg;
	struct {
	unsigned int ClkDiv_Ch0:8;
	unsigned int ClkDiv_Ch1:8;
	unsigned int ClkDiv_Ch2:8;
	unsigned int ClkDiv_Ch3:8;
	} bit;
};

// Reserved (0x14)

// Reserved (0x18)

// SIF Transmit Size Register 0
#define SIF_TXSIZE0_REG_OFS            0x1C
union SIF_TXSIZE0_REG {
	uint32_t reg;
	struct {
	unsigned int TxSize_Ch0:8;
	unsigned int TxSize_Ch1:8;
	unsigned int TxSize_Ch2:8;
	unsigned int TxSize_Ch3:8;
	} bit;
};
 

// Reserved (0x20~0x28)

// SIF Status Register
#define SIF_STS_REG_OFS            0x2C
union SIF_STS_REG {
	uint32_t reg;
	struct {
	unsigned int Status_Ch0:1;
	unsigned int Status_Ch1:1;
	unsigned int Status_Ch2:1;
	unsigned int Status_Ch3:1;
	unsigned int Reserved0:28;
	} bit;
};
 
// SIF Data Register 0
#define SIF_DATA0_REG_OFS            0x30
union SIF_DATA0_REG {
	uint32_t reg;
	struct {
	unsigned int DR0:32;
	} bit;
};
 
// SIF Data Register 1
#define SIF_DATA1_REG_OFS            0x34
union SIF_DATA1_REG {
	uint32_t reg;
	struct {
	unsigned int DR1:32;
	} bit;
};

// SIF Data Register 2
#define SIF_DATA2_REG_OFS            0x38
union SIF_DATA2_REG {
	uint32_t reg;
	struct {
	unsigned int DR2:32;
	} bit;
};
 
// SIF Data Port Select Register
#define SIF_DATASEL_REG_OFS            0x3C
union SIF_DATASEL_REG {
	uint32_t reg;
	struct {
	unsigned int DP_Sel:2;
	unsigned int Reserved0:30;
	} bit;
};

// SIF Delay Register 0
#define SIF_DELAY0_REG_OFS            0x40
union SIF_DELAY0_REG {
	uint32_t reg;
	struct {
	unsigned int Delay_Ch0:8;
	unsigned int Delay_Ch1:8;
	unsigned int Delay_Ch2:8;
	unsigned int Delay_Ch3:8;
	} bit;
};

// Reserved (0x44 ~ 0x4C)

// SIF Timing Config Register 0
#define SIF_TIMING0_REG_OFS            0x50
union SIF_TIMING0_REG {
	uint32_t reg;
	struct {
	unsigned int SEND_Ch0:4;
	unsigned int SENH_Ch0:3;
	unsigned int Reserved0:1;
	unsigned int SENS_Ch0:4;
	unsigned int Reserved1:4;
	unsigned int SEND_Ch1:4;
	unsigned int SENH_Ch1:3;
	unsigned int Reserved2:1;
	unsigned int SENS_Ch1:4;
	unsigned int Reserved3:4;
	} bit;
};
 
// SIF Timing Config Register 1
#define SIF_TIMING1_REG_OFS            0x54
union SIF_TIMING1_REG {
	uint32_t reg;
	struct {
	unsigned int SEND_Ch2:4;
	unsigned int SENH_Ch2:3;
	unsigned int Reserved0:1;
	unsigned int SENS_Ch2:4;
	unsigned int Reserved1:4;
	unsigned int SEND_Ch3:4;
	unsigned int SENH_Ch3:3;
	unsigned int Reserved2:1;
	unsigned int SENS_Ch3:4;
	unsigned int Reserved3:4;
	} bit;
};
 
// Reserved (0x58~0x5C)

// SIF DMA0 Control Register 0
#define SIF_DMA0_CTRL0_REG_OFS            0x60
union SIF_DMA0_CTRL0_REG {
	uint32_t reg;
	struct {
	unsigned int DMA0_En:1;
	unsigned int Reserved0:3;
	unsigned int DMA0_Sync_En:1;
	unsigned int Reserved1:3;
	unsigned int DMA0_Sync_Src:3;
	unsigned int Reserved2:1;
	unsigned int DMA0_Burst_N:2;
	unsigned int Reserved3:6;
	unsigned int DMA0_Burst_IntVal1:3;
	unsigned int Reserved4:1;
	unsigned int DMA0_Burst_IntVal2:3;
	unsigned int Reserved5:1;
	unsigned int DMA0_Delay:1;
	unsigned int Reserved6:3;
	} bit;
};
 
// SIF DMA0 Control Register 1
#define SIF_DMA0_CTRL1_REG_OFS            0x64
union SIF_DMA0_CTRL1_REG {
	uint32_t reg;
	struct {
	unsigned int DMA0_BufSize1:16;
	unsigned int DMA0_BufSize2:16;
	} bit;
};
 
// SIF DMA0 Control Register 2
#define SIF_DMA0_CTRL2_REG_OFS            0x68
union SIF_DMA0_CTRL2_REG {
	uint32_t reg;
	struct {
	unsigned int DMA0_BufSize3:16;
	unsigned int Reserved0:16;
	} bit;
};
 
// SIF DMA0 Control Register 3
#define SIF_DMA0_CTRL3_REG_OFS            0x6C
union SIF_DMA0_CTRL3_REG {
	uint32_t reg;
	struct {
	unsigned int DMA_Burst_Delay:24;
	unsigned int Reserved0:8;
	} bit;
};
 
// SIF DMA0 Start Address Register
#define SIF_DMA0_STARTADDR_REG_OFS            0x70
union SIF_DMA0_STARTADDR_REG {
	uint32_t reg;
	struct {
	unsigned int DMA_StartAddr:32;
	} bit;
};

// SIF DMA1 delay tag Register
#define SIF_DMA0_DLY_REG_OFS            0x74
union SIF_DMA0_DLY_REG {
	uint32_t reg;
	struct {
	unsigned int SIF_DLY_TAG:16;
	unsigned int Reserved0:16;
	} bit;
};

// Reserved (0x78~0x7C)

// SIF DMA1 Control Register 0
#define SIF_DMA1_CTRL0_REG_OFS            0x80
union SIF_DMA1_CTRL0_REG {
	uint32_t reg;
	struct {
	unsigned int DMA1_En:1;
	unsigned int Reserved0:3;
	unsigned int DMA1_Sync_En:1;
	unsigned int Reserved1:3;
	unsigned int DMA1_Sync_Src:3;
	unsigned int Reserved2:1;
	unsigned int DMA1_Burst_N:3;
	unsigned int Reserved3:6;
	unsigned int DMA1_Burst_IntVal1:3;
	unsigned int Reserved4:1;
	unsigned int DMA1_Burst_IntVal2:3;
	unsigned int Reserved5:1;
	unsigned int DMA1_Delay:1;
	unsigned int Reserved6:3;
	} bit;
};
 
// SIF DMA1 Control Register 1
#define SIF_DMA1_CTRL1_REG_OFS            0x84
union SIF_DMA1_CTRL1_REG {
	uint32_t reg;
	struct {
	unsigned int DMA1_BufSize1:16;
	unsigned int DMA1_BufSize2:16;
	} bit;
};

// SIF DMA1 Control Register 2
#define SIF_DMA1_CTRL2_REG_OFS            0x88
union SIF_DMA1_CTRL2_REG {
	uint32_t reg;
	struct {
	unsigned int DMA1_BufSize3:16;
	unsigned int Reserved0:16;
	} bit;
};

// SIF DMA1 Control Register 3
#define SIF_DMA1_CTRL3_REG_OFS            0x8C
union SIF_DMA1_CTRL3_REG {
	uint32_t reg;
	struct {
	unsigned int DMA_Burst_Delay:24;
	unsigned int Reserved0:8;
	} bit;
};

// SIF DMA1 Start Address Register
#define SIF_DMA1_STARTADDR_REG_OFS            0x90
union SIF_DMA1_STARTADDR_REG {
	uint32_t reg;
	struct {
	unsigned int DMA_StartAddr:32;
	} bit;
};

// SIF DMA1 delay tag Register
#define SIF_DMA1_DLY_REG_OFS            0x94
union SIF_DMA1_DLY_REG {
	uint32_t reg;
	struct {
	unsigned int SIF_DLY_TAG:16;
	unsigned int Reserved0:16;
	} bit;
};

// SIF Interrupt Desitenation Register
#define SIF_INT_DES_REG_OFS            0x98
union SIF_INT_DES_REG {
	uint32_t reg;
	struct {
	unsigned int INT_TO_CPU1:4;
	unsigned int Reserved0:12;
	unsigned int INT_TO_CPU2:4;
	unsigned int Reserved1:12;
	} bit;
};
 
// SIF Interrupt Enable Register 0
#define SIF_INTEN0_REG_OFS            0x9C
union SIF_INTEN0_REG {
	uint32_t reg;
	struct {
	unsigned int Data_Empty_Ch0_IntEn:1;
	unsigned int Data_Empty_Ch1_IntEn:1;
	unsigned int Data_Empty_Ch2_IntEn:1;
	unsigned int Data_Empty_Ch3_IntEn:1;
	unsigned int TxEnd_Ch0_IntEn:1;
	unsigned int TxEnd_Ch1_IntEn:1;
	unsigned int TxEnd_Ch2_IntEn:1;
	unsigned int TxEnd_Ch3_IntEn:1;
	unsigned int Reserved0:16;
	unsigned int DMAEnd_DMA0_IntEn:1;
	unsigned int DMAEnd_DMA1_IntEn:1;
	unsigned int Reserved1:6;
	} bit;
};

// SIF Interrupt Enable Register 1 to core2
#define SIF_INTEN1_REG_OFS            0xA0
union SIF_INTEN1_REG {
	uint32_t reg;
	struct {
	unsigned int Data_Empty_Ch0_IntEn_core2:1;
	unsigned int Data_Empty_Ch1_IntEn_core2:1;
	unsigned int Data_Empty_Ch2_IntEn_core2:1;
	unsigned int Data_Empty_Ch3_IntEn_core2:1;
	unsigned int TxEnd_Ch0_IntEn_core2:1;
	unsigned int TxEnd_Ch1_IntEn_core2:1;
	unsigned int TxEnd_Ch2_IntEn_core2:1;
	unsigned int TxEnd_Ch3_IntEn_core2:1;
	unsigned int Reserved0:16;
	unsigned int DMAEnd_DMA0_IntEn_core2:1;
	unsigned int DMAEnd_DMA1_IntEn_core2:1;
	unsigned int Reserved1:6;
	} bit;
};


// SIF Interrupt Status Register 0
#define SIF_INTSTS0_REG_OFS            0xA4
union SIF_INTSTS0_REG {
	uint32_t reg;
	struct {
	unsigned int Data_Empty_Ch0:1;
	unsigned int Data_Empty_Ch1:1;
	unsigned int Data_Empty_Ch2:1;
	unsigned int Data_Empty_Ch3:1;
	unsigned int TxEnd_Ch0:1;
	unsigned int TxEnd_Ch1:1;
	unsigned int TxEnd_Ch2:1;
	unsigned int TxEnd_Ch3:1;
	unsigned int Reserved0:16;
	unsigned int DMAEnd_DMA0:1;
	unsigned int DMAEnd_DMA1:1;
	unsigned int Reserved1:6;
	} bit;
};
 

// SIF Interrupt Status Register 1
#define SIF_INTSTS1_REG_OFS            0xA8
union SIF_INTSTS1_REG {
	uint32_t reg;
	struct {
	unsigned int Data_Empty_Ch0_core2:1;
	unsigned int Data_Empty_Ch1_core2:1;
	unsigned int Data_Empty_Ch2_core2:1;
	unsigned int Data_Empty_Ch3_core2:1;
	unsigned int TxEnd_Ch0_core2:1;
	unsigned int TxEnd_Ch1_core2:1;
	unsigned int TxEnd_Ch2_core2:1;
	unsigned int TxEnd_Ch3_core2:1;
	unsigned int Reserved0:16;
	unsigned int DMAEnd_DMA0_core2:1;
	unsigned int DMAEnd_DMA1_core2:1;
	unsigned int Reserved1:6;
	} bit;
};
#endif
