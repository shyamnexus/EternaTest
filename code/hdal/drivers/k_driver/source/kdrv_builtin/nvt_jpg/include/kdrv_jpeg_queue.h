/*
    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.

    @file       kdrv_jpeg_queue.h

    @brief      kdrv_jpeg internal header file

    @version    V1.00.000
    @author     Novatek FW Team
    @date       2017/02/13
*/
#ifndef _KDRV_JPEG_QUEUE_H
#define _KDRV_JPEG_QUEUE_H

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "kdrv_videojpeg/kdrv_videoenc_jpeg.h"
#include "kdrv_videojpeg/kdrv_videodec_jpeg.h"


#define KDRV_JPEG_QUEUE_ELEMENT_NUM   8


typedef struct _KDRV_JPEG_QUEUE_ELEMENT {
	KDRV_CALLBACK_FUNC		*p_cb_func;
	KDRV_VDOJPGE_PARAM		*jpeg_enc_param;
	KDRV_VDOJPGD_PARAM		*jpeg_dec_param;
	BOOL                    jpeg_mode;
	//VOID                    *p_user_data;
	//UINT32                   flg_ptn;
} KDRV_JPEG_QUEUE_ELEMENT;


typedef struct _KDRV_JPEG_QUEUE_INFO {
	UINT32                   in_idx;
	UINT32                   out_idx;
	KDRV_JPEG_QUEUE_ELEMENT   element[KDRV_JPEG_QUEUE_ELEMENT_NUM];
} KDRV_JPEG_QUEUE_INFO;

extern INT32 kdrv_jpeg_queue_init_p(void);
extern INT32 kdrv_jpeg_queue_exit_p(void);
extern KDRV_JPEG_QUEUE_INFO *kdrv_jpeg_get_queue_by_coreid(void);
extern INT32 kdrv_jpeg_queue_is_empty_p(KDRV_JPEG_QUEUE_INFO *p_queue);
extern INT32 kdrv_jpeg_queue_add_p(KDRV_JPEG_QUEUE_INFO *p_queue, KDRV_JPEG_QUEUE_ELEMENT *element);
extern INT32 kdrv_jpeg_queue_del_p(KDRV_JPEG_QUEUE_INFO *p_queue, KDRV_JPEG_QUEUE_ELEMENT *element);
#endif

