#ifndef _UVC_SLICE_H
#define _UVC_SLICE_H

extern HD_RESULT yuv_push_to_usb(UINT32 buf_va, HD_VIDEO_FRAME *pVideoFrame);
extern HD_RESULT mjpg_slice_push_to_usb(UINT32 buf_va, HD_VIDEOENC_BS *p_bs);

#endif
