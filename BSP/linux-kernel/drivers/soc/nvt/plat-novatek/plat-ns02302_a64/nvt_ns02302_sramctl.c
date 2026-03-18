#include <plat/nvt-sramctl.h>

int sram_shutdown_size = 43;

struct nvt_sram_shutdown_info nvt_sram_sd_tbl[] = {
	{MAU_SD,                FALSE},
	{SDP_SD,                TRUE},
	{HWCP_SD,               TRUE},
	{TSE_SD,                TRUE},

	{GRAPH_SD,              TRUE},
	{GRAPH2_SD,             TRUE},
	{GRAPH3_SD,             TRUE},
	{JPG_SD,                TRUE},

	{IVE_SD,                TRUE},
	{MDBC_SD,               TRUE},
	{TRKE_SD,               TRUE},
	{NUE2_SD,               TRUE},

	{ETH_SD,                TRUE},
	{IDE_SD,                TRUE},
	{RSA_SD,                TRUE},
	{USB3_SD,               TRUE},

	{DSI_SD,                TRUE},
	{CSI_TX_SD,             TRUE},
	{MCU_SD,                TRUE},
	{SIE_SD,                TRUE},

	{SIE2_SD,               TRUE},
	{SIE3_SD,               TRUE},
	{SIE4_SD,               TRUE},
	{SIE5_SD,               TRUE},

	{VIE_SD,                TRUE},
	{IFE_SD,                TRUE},
	{SDIO3_SD,              TRUE},
	{SMC_SD,                TRUE},

	{DRE_SD,                TRUE},
	{ISE_SD,                TRUE},
	{IPE_SD,                TRUE},
	{IME_SD,                TRUE},

	{SDIO_SD,               TRUE},
	{SDIO2_SD,              TRUE},
	{VPE_SD,                TRUE},
	{VENC_SD,               TRUE},

	{NUE_SD,                TRUE},
	{LSU_SD,                TRUE},
	{PPU_SD,                TRUE},
	{TCM_SD,                TRUE},

	{TCM2_SD,               TRUE},
	{CONV_SD,               TRUE},
	{H264_SD,               TRUE},
};
