/**
	@brief Header file of utility functions of vendor dsp module.

	@file vendor_dsp_util.h

	@ingroup vendor_dsp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _VENDOR_DSP_UTIL_H_
#define _VENDOR_DSP_UTIL_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_common.h"
#include "vendor_dsp.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define DSP1_APP_IPC_TOKEN              "DspApp"
#define DSP2_APP_IPC_TOKEN              "DspApp2"

#define DSP_FW_MAX_SIZE                 0x800000

/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
	TYPE DEFINITION
********************************************************************/

/**
	DSP bin information.
*/
typedef struct _DSP_BIN_INFO {
	UINT32	Offset; 	///< original bin position in file
	UINT32	Size;		///< size of original bin
	UINT32	TargetAddr; ///< target address in DRAM( physical address)
	UINT32	CheckSum;	///< checksum of original
} DSP_BIN_INFO;

typedef struct _DSP_BIN_HEADER {
	UINT32	BinSize;
	UINT32	Reserve1;
	UINT32	Reserve2;
	UINT32	StartAddr;
} DSP_BIN_HEADER, *PDSP_BIN_HEADER;


/**
	DSP header information.
*/
typedef struct _DSP_FW_HEADER {
	UINT32		  uiFourCC; ///< 'D','S','P','1' for DSP1 and 'D','S','P','2' for DSP2
	UINT32		  uiSize;	///< sizeof(DSP_FW_HEADER)
	DSP_BIN_INFO  BLoader;	///< boot loader bin info
	DSP_BIN_INFO  ProgInt;	///< ProgInt bin info
	DSP_BIN_INFO  DataInt;	///< DataInt bin info
	DSP_BIN_INFO  ProgExt;	///< ProgExt bin info
	DSP_BIN_INFO  DataExt;	///< DataExt bin info
	UINT32	TotalSize;		///< total all in one bin size including header size
	UINT32	EmbMemBlkSize;	///< EMBMEM_BLK_SIZE
	UINT32		  uiProgExtDramSize;///<
	UINT32		  uiDataExtDramSize;///<
	CHAR	BootVer[16];	///< DSP boot code version
	CHAR	MainVer[16];	///< DSP main code version
} DSP_FW_HEADER;

typedef struct _HEADER_BFC {
	UINT32 uiFourCC;	///< FourCC = BCL1
	UINT16 uiChkSum16;	///< 16bit check sum value
	UINT16 uiAlgorithm; ///< algorithm always is 9
	UINT32 uiSizeUnComp;///< big endian uncompressed size
	UINT32 uiSizeComp;	///< big endian compressed size
} HEADER_BFC, *PHEADER_BFC;


typedef struct _DSP_MEM_RANGE {
	ULONG				 va;		///< Memory buffer starting address
	ULONG				 addr;		///< Memory buffer starting address
	UINT32				 size;		///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} DSP_MEM_RANGE, *PDSP_MEM_RANGE;

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern INT32 dsp_get_dram_partition(DSP_CORE_ID core_id, DSP_MEM_RANGE *p_mem_range);
extern ULONG dsp_pa2va(ULONG pa, DSP_MEM_RANGE *p_dsp_partition);
extern int get_dsp_header(char *filepath, DSP_FW_HEADER *pH);
extern BOOL dsp_chk_fw_header(DSP_CORE_ID DspCore, DSP_MEM_RANGE *p_partition, DSP_FW_HEADER *pHeader);
extern BOOL dsp_fw_checksum(DSP_FW_HEADER *pH, DSP_MEM_RANGE *p_dsp_range);
extern BOOL dsp_load_fw(DSP_CORE_ID dsp_core_id, PDSP_MEM_RANGE pWorkBuf);
extern HD_RESULT vendor_dsp_load(DSP_CORE_ID dsp_core_id, ULONG va);

extern HD_RESULT vendor_dsp_readbin(UINTPTR buf, UINT32 size, const CHAR *filename);
extern HD_RESULT vendor_dsp_writebin(UINTPTR buf, UINT32 size, const CHAR *filename);


#endif  /* _VENDOR_DSP_UTIL_H_ */
