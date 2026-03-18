/**
    Ctl VIE Layer, Buffer ctrl

    @file       ctl_vie_buf_int.h
    @ingroup    mIIPL_Ctrl
    @note       None

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _CTL_VIE_BUF_INT_H
#define _CTL_VIE_BUF_INT_H

#include "ctl_vie_utility_int.h"

#define KFLOW_VIE_BUF_FLAG 0x5a5a5a50
#define KFLOW_VIE_DMA_WP_ALIGN 	0x100 //530 dma wp need 64 word align

/**********************************/
/*  ctl_vie_buf message queue     */
/**********************************/
#define CTL_VIE_BUF_MSG_STS_FREE    0x00000000
#define CTL_VIE_BUF_MSG_STS_LOCK    0x00000001

#define CTL_VIE_BUF_MSG_IGNORE      0x00000000
#define CTL_VIE_BUF_MSG_PUSH        0x00000001  //[0]: buf_io_fp, [1]: raw header address, [2]: reserve
#define CTL_VIE_BUF_MSG_UNLOCK      0x00000002  //[0]: buf_io_fp, [1]: raw header address, [2]: reserve
#define CTL_VIE_BUF_MSG_LOCK        0x00000004  //[0]: buf_io_fp, [1]: raw header address, [2]: reserve

#define CTL_VIE_BUF_CONV2_SIZE(w, h) (((w & 0x0000ffff) << 16) | (h & 0x0000ffff))

typedef enum {
	CTL_VIE_HEAD_ITEM_SIGN = 0,     ///< signature = MAKEFOURCC('V','F','R','M')
	CTL_VIE_HEAD_ITEM_FC,           ///< frame count
	CTL_VIE_HEAD_ITEM_TS,           ///< time stamp
	CTL_VIE_HEAD_ITEM_MAIN_OUT_IMG, ///< main image out
	CTL_VIE_HEAD_ITEM_MAIN_OUT_ADDR,///< main image out addr(ch0/ch1)
	CTL_VIE_HEAD_ITEM_DEST_CROP,    ///< dest. crop win

	// for dramout debug data compare, not header info
	CTL_VIE_HEAD_ITEM_DBG_ADDR,   ///< hw dbg data output addr
	CTL_VIE_HEAD_ITEM_DBG_DATA,   ///< hw dbg data

	CTL_VIE_HEAD_ITEM_MAX,
	ENUM_DUMMY4WORD(CTL_VIE_HEAD_ITEM)
} CTL_VIE_HEAD_ITEM;

typedef struct {
	UINT32 cmd;
	ULONG  param[5];
	UINT32 rev[2];
	CTL_VIE_LIST_HEAD list;
} CTL_VIE_BUF_MSG_EVENT;

INT32 ctl_vie_buf_msg_snd(UINT32 cmd, ULONG p1, ULONG p2, ULONG p3);
INT32 ctl_vie_buf_msg_rcv(UINT32 *cmd, ULONG *p1, ULONG *p2, ULONG *p3);
INT32 ctl_vie_buf_msg_flush(void);
INT32 ctl_vie_buf_erase_queue(UINT32 handle);
INT32 ctl_vie_buf_msg_reset_queue(void);
UINT32 ctl_vie_buf_get_free_queue_num(void);

/**********************************/
/*  ctl_vie_buf task ctrl         */
/**********************************/
INT32 ctl_vie_buf_open_tsk(void);
INT32 ctl_vie_buf_close_tsk(void);
INT32 ctl_vie_buf_set_resume(BOOL b_flush_evt);
INT32 ctl_vie_buf_set_pause(BOOL b_wait_end, BOOL b_flush_evt);
INT32 ctl_vie_buf_wait_pause_end(void);
void ctl_vie_buf_queue_flush(CTL_VIE_ID id);


/**********************************/
/*  ctl_vie_buf                   */
/**********************************/
void ctl_vie_buf_update_out_addr(CTL_VIE_HDL *ctrl_hdl, CTL_VIE_HEAD_IDX head_idx);
void ctl_vie_buf_update_out_size(CTL_VIE_HDL *ctrl_hdl);
void ctl_vie_buf_io_cfg(UINT32 id, CTL_VIE_BUF_IO_CFG io_cfg, UINT32 req_size, ULONG vie_header_addr);
/**********************************/
/*  header ctl                    */
/**********************************/
INT32 ctl_vie_header_create(CTL_VIE_HDL *ctrl_hdl, CTL_VIE_HEAD_IDX head_idx);
void ctl_vie_header_set_cur_head(CTL_VIE_HDL *ctrl_hdl);
void ctl_vie_header_set(CTL_VIE_HEAD_ITEM set_item, CTL_VIE_HDL *ctrl_hdl, CTL_VIE_HEAD_IDX head_idx);
#endif // _CTL_VIE_BUF_INT_H
