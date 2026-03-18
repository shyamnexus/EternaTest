/**
    Header file for Interrupt module

    This file is the header file that define the API for Interrupt module.

    @file       Interrupt.h
    @ingroup    mIDrvSys_Interrupt
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _MSIx_H
#define _MSIx_H

#include <linux/types.h>
#include <linux/soc/nvt/nvt_type.h>

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

#define MSIx_Group_CNT	32
typedef enum {

	MSIx_SEL_TYPE_PULSE_GEN = 0x0,
	MSIx_SEL_TYPE_AXICMD,

	MSIx_SEL_TYPE_CNT,
	ENUM_DUMMY4WORD(MSIx_SEL_TYPE)
} MSIx_SEL_TYPE;

// Capacity of Storage device
typedef struct {
	UINT32  ui_axi_address_lsb;
	UINT32  ui_axi_address_msb;
	UINT32	ui_msix_sel;
} AXI_WRITE_ADDR, *PAXI_WRITE_ADDR;
/**
    Interrupt module ID

    Interrupt module ID for int_getIRQId() and int_getDummyId().
*/
#define INT_GIC_SPI_START_ID    32

typedef enum {

	MSIx_CONFIG_ID_SET_AXI_WRITE_ADDRESS = 0x0,
	MSIx_CONFIG_ID_SET_2ND_AXI_WRITE_ADDRESS,

	MSIx_CONFIG_ID_CNT,
	ENUM_DUMMY4WORD(MSIx_CONFIG_ID)
} MSIx_CONFIG_ID;


//extern ER msix_open(struct nvt_pcie_chip *nvt_pcie_chip_ptr, MSIx_SEL_TYPE msix_type, UINT32 lsb, UINT32 msb);
//extern ER msix_set_config(MSIx_CONFIG_ID cfg_id, UINT32 ui_config);
//extern void msix_enable_irq(int number);
//extern void msix_clear_msix_tx_irq_status(int number);
//extern void msix_disable_irq(int number);
//extern void msix_recv_cmd_set_pending_irq(UINT32 group, UINT32 number);
//extern void msix_recv_cmd_clr_pending_irq(UINT32 group, UINT32 number);
//extern void msix_recv_cmd_set_en_irq(UINT32 group, UINT32 number);
//extern void msix_recv_cmd_clr_en_irq(UINT32 group, UINT32 number);

//extern UINT32 msix_recv_cmd_get_pending_irq(UINT32 group);

//@}

#endif
