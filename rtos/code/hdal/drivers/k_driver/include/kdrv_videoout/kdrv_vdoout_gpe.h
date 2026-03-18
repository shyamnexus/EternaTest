/**
 * @file kdrv_vdoout_gpe.h
 * @brief type definition of KDRV API.
 * @author ESW
 * @date in the year 2021
 */

#ifndef _KDRV_VDOOUT_GPE_H_
#define _KDRV_VDOOUT_GPE_H_

#include "kdrv_type.h"


#define RE_INIT -2


typedef enum {
	GPENC_SRC_IDX_G3 = 0,   ///< GPENC group bits 3 = 8 pixels
	GPENC_SRC_IDX_G4,		///< GPENC group bits 4 = 16 pixels
	GPENC_SRC_IDX_G5,		///< GPENC group bits 5 = 32 pixels
	GPENC_SRC_IDX_G6,		///< GPENC group bits 6 = 64 pixels
	GPENC_SRC_IDX_G7,		///< GPENC group bits 7 = 128 pixels

	ENUM_DUMMY4WORD(GPENC_SRC_IDX_ID)
} GPENC_SRC_IDX_ID;

typedef enum {
	GPENC_SRC_WIDTH_2B = 1, ///< GPENC source image pixel size = 2 bytes
	GPENC_SRC_WIDTH_4B,		///< GPENC source image pixel size = 4 bytes

	ENUM_DUMMY4WORD(GPENC_SRC_WDITH_ID)
} GPENC_SRC_WDITH_ID;


/**
    VDDO gpe Parameter Definition

    This definition is used in kdrv_vddo_gpe_set.
*/
typedef struct {
	union {
		/**
		    Set gpe src index

		    This structure shuld be used like this:
		\n  gpe_ctrl.SEL.KDRV_GPE_SRCIDX.idx = GPENC_SRC_IDX_G7;
		\n  kdrv_vddo_gpe_set(handel, KDRV_GPE_SRCIDX, &gpe_ctrl);
		*/
		struct { // gpe source indx
			UINT32 idx;            ///< source index
		} KDRV_GPE_SRCIDX;

		struct { // gpe source width
			UINT32 width; 		   ///< source width
		} KDRV_GPE_SRCWIDTH;

		struct {
			UINT32 addr;     ///< soruce read address
		} KDRV_GPE_READADDR;

		struct {
			UINT32 addr;    ///< soruce write address
		} KDRV_GPE_WRITEADDR;

		struct {
			UINT32 limit;   ///< write limit
		} KDRV_GPE_WRITELIMIT;

		struct {
			UINT32 img_w;	      ///< soruce imgage widht
		} KDRV_GPE_IMGWIDTH;

		struct {
			UINT32 img_h;	      ///< soruce imgage heigh
		} KDRV_GPE_IMGHEIGH;	

		struct {
			UINT32 img_lnofs;     ///< soruce imgage lineoffset
		} KDRV_GPE_IMGLNOFS;

		struct {
			BOOL wait;           ///< no use
		} KDRV_GPE_WAITDONE;

		struct {
			UINT32 size;          ///< encode size
		} KDRV_GPE_ENCSIZE;
		
		struct {
			BOOL reset;           ///< no use
		} KDRV_GPE_SWRST;
	} SEL;
} KDRV_VDDO_GPE_PARAM;

/**
    KDRV VDDO GPE parameter select id

    This is used in kdrv_vddo_gpe_set and kdrv_vddo_gpe_get to set and get the VDDO GPE configuration parameter.
*/
typedef enum {
	/*
		gpe control group
	*/
	VDDO_GPE_CTRL_BASE = 0x00000000,

	VDDO_GPE_IDX_WIDTH,    ///< config/get GPENC index width, valid 3/4/5/6/7 for 3/4/5/6/7 pixels are group into a compress unit
	VDDO_GPE_SRC_WIDTH,    ///< config/get GPENC source width, value 1/2 for 2/4 bytes of image pixels size, ex: RGB565 set to 1, ARGB8888 set to 2
	//VDDO_GPE_WAIT_TIME,    ///< config/get waiting time
	VDDO_GPE_READ_ADDR,    ///< config/get read address
	VDDO_GPE_WRITE_ADDR,   ///< config/get write address
	VDDO_GPE_WRITE_LIMIT,  ///< config/get write limit  
	VDDO_GPE_IMG_W,        ///< config/get image width, unit bytes
	VDDO_GPE_IMG_H,        ///< config/get image height, unit lines
	VDDO_GPE_IMG_OFS,      ///< config/get image lineoffset, unit word align
	VDDO_GPE_ENC_SIZE,     ///< get encode size, unit bytes
	VDDO_GPE_WAIT_DONE,    ///< set engine's done
	VDDO_GPE_SW_RST,       ///< set sw reset

	ENUM_DUMMY4WORD(KDRV_VDDO_GPE_PARAM_ID)
} KDRV_VDDO_GPE_PARAM_ID;

/*!
 * @fn INT32 kdrv_vddo_gpe_open(UINT32 chip, UINT32 engine)
 * @brief open hardware engine
 * @param chip      the chip id of hardware
 * @param engine    the engine id of hardware
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_vddo_gpe_open(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_vddo_gpe_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_vddo_gpe_close(UINT32 chip, UINT32 engine);

/*!
 * @brief trigger hardware engine to load new config
 * @param handler 				the handler of hardware
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_vddo_gpe_trigger(UINT32 handler, KDRV_CALLBACK_FUNC *p_cb_func);

/*!
 * @fn INT32 kdrv_vddo_gpe_set(UINT32 handler, VDDO_ID id, VOID *p_param)
 * @brief set parameters to hardware engine
 * @param handler        the handler of hardware
 * @param id             the id of parameters
 * @param param          the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_vddo_gpe_set(UINT32 handler, KDRV_VDDO_GPE_PARAM_ID id, VOID *p_param);

/*!
 * @fn INT32 kdrv_vddo_gpe_get(UINT32 handler, VDDO_ID id, VOID *p_param)
 * @brief set parameters to hardware engine
 * @param handler        the handler of hardware
 * @param id             the id of parameters
 * @param param          the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_vddo_gpe_get(UINT32 handler, KDRV_VDDO_GPE_PARAM_ID id, VOID *p_param);

#endif
