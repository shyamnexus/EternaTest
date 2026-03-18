#ifndef __HEVC_HW_H__
#define __HEVC_HW_H__

#include "hevc_resmgr.h"
#include "../../h26xdec_platform.h"

#define HEVC_REG_CMD_SWRST          (1<<0)
#define HEVC_REG_CMD_START          (1<<1)
#define HEVC_REG_CMD_CMD_ADDR_VALID (1<<2)
#define HEVC_REG_CMD_BIN_ADDR_VALID (1<<3)
#define HEVC_REG_STATUS_PIC_FINISH  (1<<16)
#define HEVC_REG_STATUS_BSDMA_END   (1<<17)
#define HEVC_REG_STATUS_BIN_END     (1<<20)
#define HEVC_REG_STATUS_HW_TIMEOUT	(1<<22)
#define HEVC_REG_STATUS_BIN_FULL    (1<<24)

#define HEVC_REG_DMA_ILF_BURST_MODE               (1<<0)
#define HEVC_REG_DMA_MISC_BURST_MODE              (1<<1)
#define HEVC_REG_DMA_MISC_RD_AFTER_WR             (1<<2)
#define HEVC_REG_DMA_PIPE_STALL_OPT               (1<<3)
#define HEVC_REG_DMA_PIPE_FLUSH_CNT(x)            ((x)<<4)
#define HEVC_REG_DMA_ILF_URI_LOCAL_SRAM_EN        (1<<8)
#define HEVC_REG_DMA_ILF_TILE_LOCAL_SRAM_EN       (1<<9)
#define HEVC_REG_DMA_ILF_URI_SI_INFO_SRAM_EN      (1<<10)
#define HEVC_REG_DMA_ILF_TILE_SI_INFO_SRAM_EN     (1<<11)
#define HEVC_REG_DMA_MISC_URI_LOCAL_SRAM_EN       (1<<12)
#define HEVC_REG_DMA_MISC_TILE_LOCAL_SRAM_EN      (1<<13)
#define HEVC_REG_DMA_MISC_RDMA_PRELOAD_OPTION     (1<<16)
#define HEVC_REG_DMA_MISC_WDMA_POSTWRITE_OPTION   (1<<17)
#define HEVC_REG_DMA_MISC_DIFF_LAYER_COL_INFO_VALID (1<<18)
#define HEVC_REG_DMA_LAST_CTB_IN_SLC_IN           (1<<19)
#define HEVC_REG_DMA_BIN_FIFO_CNT(x)              ((x)<<20)
#define HEVC_REG_DMA_IQT_FIFO_CNT(x)              ((x)<<24)
#define HEVC_REG_DMA_MC_INTP_FIFO_CNT(x)          ((x)<<28)

#define HEVC_REG_FINISH_INT_EN                    (1<<0)
#define HEVC_REG_BSDMA_END_INT_EN                 (1<<1)
#define HEVC_REG_BINDMA_END_INT_EN                (1<<4)
#define HEVC_REG_BINDMA_FULL_INT_EN               (1<<5)
#define HEVC_REG_HW_TIMEOUT_INT_EN                (1<<6)

#define HEVC_OUTPUT0_FORMAT_16x2_TRUE8            (0)
#define HEVC_OUTPUT0_COLOR_SPACE_Y                (0)
#define HEVC_OUTPUT0_COLOR_SPACE_UV               (8)
#define HEVC_GET_OUTPUT0_BIT_DEPTH(out,space)     (((out >> (space)) & 0x3) + 8)
#define HEVC_SET_OUTPUT0_FORMAT(fmt,space)        ((fmt)<<(2+(space)))
#define HEVC_SET_OUTPUT0_BIT_DEPTH(bits,space)    (((bits) - 8)<<((space)))
#define HEVC_EXTRA_WRITE_EN                       (1 << 31)
#define HEVC_SCALING_1_4x                         (0)
#define HEVC_SCALING_1_2x                         (1)
#define HEVC_SCALING_10_16x                       (2)
#define HEVC_SCALING_12_16x                       (3)

#define HEVC_HAVE_EXTRA_WRITE(fmt)                ((fmt) & HEVC_EXTRA_WRITE_EN)
#define HEVC_SET_OUTPUT1_SCALING_RATIO(ratio)     ((ratio << 29) | HEVC_EXTRA_WRITE_EN)
#define HEVC_GET_OUTPUT1_SCALING_RATIO(out)       (((out) >> 29) & 3)

#define isInterruptFromBin(chip_idx)  (IMVQ_VDEC_GetInterruptStatus(1, chip_idx) & INTERRUPT_STATUS_DEC)
#define isInterruptFromAd0(chip_idx)  (IMVQ_VDEC_GetInterruptStatus(1, chip_idx) & INTERRUPT_STATUS_AD0)

/* for sw timeout, add by CW */
#define isTimeoutFromBin(chip_idx)  (IMVQ_VDEC_GetHWBusyFlag(1, chip_idx) & INTERRUPT_STATUS_DEC)
#define isTimeoutFromAd0(chip_idx)  (IMVQ_VDEC_GetHWBusyFlag(1, chip_idx) & INTERRUPT_STATUS_AD0)

extern uintptr_t regWrapper[H26XD_CHIP];

static inline unsigned int getRegStatus(volatile ST_HEVC_REG *regs)
{
    /* make this function atomic , otherwise timer interrupt and h/w inteerupt will fight */
    unsigned int status;

    status = H26XD_READ_REG(&regs->reg000.dec_ctrl);

    status &= (HEVC_REG_STATUS_PIC_FINISH
                | HEVC_REG_STATUS_BSDMA_END
                | HEVC_REG_STATUS_BIN_END
                | HEVC_REG_STATUS_HW_TIMEOUT
                | HEVC_REG_STATUS_BIN_FULL);

    H26XD_WRITE_REG(status, (void *)&regs->reg000.dec_ctrl);

    return status;
}

static inline void setRegStatus(ST_HEVC_REG *regs, int value)
{
    H26XD_WRITE_REG(value, (void *)&regs->reg000.dec_ctrl);
}

static inline int getRegPoc (ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg08c.value);
}

static inline int getRegSliceNr(ST_HEVC_REG *regs)
{
    return (H26XD_READ_REG(&regs->reg090.value) & 0x3FFFF);
}

static inline unsigned int getRegBsCmdAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg094.value);
}

static inline unsigned int getRegRpicAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg098.value);
}

static inline unsigned int getRegRlistAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg09c.value);
}

static inline unsigned int getRegMbInfoAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0a4.value);
}

static inline unsigned int getRegBinUriAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0a8.value);
}

static inline unsigned int getRegIlfUpperRowAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0b0.value);
}

static inline unsigned int getRegIlfRecYAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0b8.value);
}

static inline unsigned int getRegIlfRecUVAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0bc.value);
}

static inline int getRegIlfRecYStride(ST_HEVC_REG *regs)
{
    return (H26XD_READ_REG(&regs->reg0c0.value) & 0xFFFF);
}

static inline int getRegIlfRecUVStride(ST_HEVC_REG *regs)
{
    return ((H26XD_READ_REG(&regs->reg0c0.value) & 0xFFFF0000) >> 16);
}

static inline unsigned int getRegIqtQsAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0c4.value);
}

static inline unsigned int getRegQpAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0c8.value);
}

static inline unsigned int getRegDmaConf(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0cc.value);
}

static inline void setRegDmaConf(ST_HEVC_REG *regs, unsigned int value)
{
    H26XD_WRITE_REG(value, (void *)&regs->reg0cc.value);
}

static inline int getRegRpsInfo(ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg0d0.value);
}

static inline int getRegLenListEntry(ST_HEVC_REG *regs)
{
    return (H26XD_READ_REG(&regs->reg0d4.value) & 0xF);
}

static inline unsigned int getRegBinOutAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg0d8.value);
}

static inline int getRegIntEn(ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg0dc.value);
}

static inline void setRegIntEn(ST_HEVC_REG *regs, int value)
{
    H26XD_WRITE_REG(value, (void *)&regs->reg0dc.value);
}

static inline int getRegBinBufSize(ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg0e0.value);
}

static inline int getRegBinOutBitLen(ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg0e4.value);
}

static inline int getRegBsConsumedBitLen(ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg0e8.value);
}

static inline int getRegEcedMaxDecLen(ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg0ec.value);
}

static inline int getRegIlf_chk_sum(ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg0f0.value);
}

static inline unsigned int getRegCycleCount(volatile ST_HEVC_REG *regs)
{
    H26XD_WRITE_REG(0x0, (void *)&regs->reg100.value);
    return H26XD_READ_REG(&regs->reg100.value);
}

static inline unsigned int getRegBinDmaCmdAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg104.value);
}

static inline unsigned int getRegBestRefYAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg104.value);
}

static inline unsigned int getRegBestRefUVAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg108.value);
}

static inline int getRegNumOfConcealMb(ST_HEVC_REG *regs)
{
    return (int)H26XD_READ_REG(&regs->reg11c.value);
}

static inline unsigned int getRegCtbNumInSlcAddr(ST_HEVC_REG *regs)
{
    return H26XD_READ_REG(&regs->reg124.value);
}

static inline int getVDecLocalSram0En(int chip_idx)
{
    unsigned int *local_sram0_addr = (unsigned int *)regWrapper + 4;
    return (H26XD_READ_REG(local_sram0_addr) & 0x8);
}

static inline int getVDecLocalSram1En(int chip_idx)
{
    unsigned int *local_sram1_addr = (unsigned int *)regWrapper + 5;
    return (H26XD_READ_REG(local_sram1_addr) & 0x8);
}

static inline int getVDecLocalSram2En(int chip_idx)
{
    unsigned int *local_sram2_addr = (unsigned int *)regWrapper + 6;
    return (H26XD_READ_REG(local_sram2_addr) & 0x8);
}

ST_HEVC_REG *getHWRegister(EN_HEVC_Engine type, int chip_idx);
void setHWRegister(uintptr_t addr, int chip_idx);
int triggerHW (ST_HEVC_CONTEXT *hevc, ST_PIC_RESOURCE *pic);
int stopHW(ST_HEVC_CONTEXT *hevc, EN_HEVC_Engine type);
int updateRegisterMapForParser(ST_PIC_RESOURCE *pic, ST_SPS *sps, ST_PPS *pps, ST_SLICE *slice);
int updateRegisterMapForAd(ST_PIC_RESOURCE *pic, ST_HEVC_CONTEXT *hevc);
int updateRegisterMapForBin_post(ST_PIC_RESOURCE *pic, ST_HEVC_CONTEXT *hevc);
void setBINTriggerLLCMD(ST_HEVC_CONTEXT *hevc);

#endif
