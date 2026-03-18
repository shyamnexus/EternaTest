/**
    Image operation module.

    This module include the image operation like fill image,copy image, scale image, rotate image ...

    @file       gximage.h
    @ingroup    imgtrans

    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _GX_IMAGE_INTERNAL_H_
#define _GX_IMAGE_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int  gximage_init(void);

extern void gximage_exit(void);

extern void gximg_memset(void *buf, unsigned char val, int len);

extern void gximg_memcpy(void *buf, void *src, int len);

extern int gximg_set_force_no_flush(int no_flush);

extern int gximg_get_force_no_flush(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //_GX_IMAGE_INTERNAL_H_
