#include "UvacDbg.h"
#include "hd_type.h"
#include <stdio.h>
#include "kwrap/type.h"
#include "kwrap/cmdsys.h"
#include "kwrap/sxcmd.h"
#include "kwrap/debug.h"
#include "kwrap/stdio.h"
#include <string.h>
#include "UVAC.h"
#include "UvacVideoTsk.h"
#include "UvacIsoInTsk.h"
#include "uvac_compat.h"

#define SIMULATE_CMD_HDAD 0xFFFF0000

UINT32 m_uiUvacDbgIso = UVAC_DBG_CMD_NONE;
UINT32 m_uiUvacDbgVid = UVAC_DBG_CMD_NONE;

extern UINT32 gUvacTxfQueCurCnt[UVAC_TXF_QUE_MAX];
extern UVAC_OPS *guvac_ops;

void _U3UVAC_stream_enable(UVAC_STRM_PATH stream, BOOL enable);

#if 1
static BOOL Cmd_UVAC_iso(unsigned char argc, char **argv)
{
	UINT32 dbg = 0;

	sscanf_s(argv[0], "%x ", &dbg);

	printf("iso dbg = 0x%x\r\n", dbg);

	if ((dbg & SIMULATE_CMD_HDAD) == SIMULATE_CMD_HDAD) {
		UVAC_STRM_PATH stream;
		BOOL enable;

		stream = (dbg >> 4)&0xF;
		enable = dbg&0xF;
		_U3UVAC_stream_enable(stream, enable);

	} else {
		m_uiUvacDbgIso= dbg;
	}

	return TRUE;
}

static BOOL Cmd_UVAC_vid(unsigned char argc, char **argv)
{
	UINT32 dbg = UVAC_DBG_VID_START;

	printf("vid dbg = 0x%x\r\n", dbg);

	sscanf_s(argv[0], "%x ", &dbg);

	m_uiUvacDbgVid = dbg;

	return TRUE;
}

static BOOL Cmd_UVAC_que(unsigned char argc, char **argv)
{
	UINT32 que_id = UVAC_TXF_QUE_V1;

	sscanf_s(argv[0], "%d ", &que_id);

	printf("que[%d]=%d\r\n", que_id, gUvacTxfQueCurCnt[que_id]);

	return TRUE;
}

static BOOL Cmd_UVAC_reset_video_timestamp(unsigned char argc, char **argv)
{

#if (ISF_LATENCY_DEBUG)
	UVAC_DbgDmp_TimestampNumReset();
#endif

	return TRUE;
}

static BOOL Cmd_UVAC_reset_audio_timestamp(unsigned char argc, char **argv)
{

#if (ISF_AUDIO_LATENCY_DEBUG)
	UVAC_DbgDmp_Aud_TimestampNumReset();
#endif

	return TRUE;
}


static SXCMD_BEGIN(uvac_cmd_tbl, "UVAC_cmd_tbl")
SXCMD_ITEM("iso %", Cmd_UVAC_iso, "Check if ISO is transfering.")
SXCMD_ITEM("vid %", Cmd_UVAC_vid, "Check video task.")
SXCMD_ITEM("que %", Cmd_UVAC_que, "Check queue num.")
SXCMD_ITEM("rv %", Cmd_UVAC_reset_video_timestamp, "reset UVC timestamp dump.")
SXCMD_ITEM("ra %", Cmd_UVAC_reset_audio_timestamp, "reset UAC timestamp dump.")
SXCMD_END()

static int uvac_cmd_showhelp(int (*dump)(const char *fmt, ...))
{
	UINT32 cmd_num = SXCMD_NUM(uvac_cmd_tbl);
	UINT32 loop = 1;

	for (loop = 1 ; loop <= cmd_num ; loop++) {
		dump("%15s : %s\r\n", uvac_cmd_tbl[loop].p_name, uvac_cmd_tbl[loop].p_desc);
	}
	return 0;
}
MAINFUNC_ENTRY(uvac, argc, argv)
{
	UINT32 cmd_num = SXCMD_NUM(uvac_cmd_tbl);
	UINT32 loop;
	int    ret;

	if (argc < 2) {
		return -1;
	}
	if (strncmp(argv[1], "?", 2) == 0) {
		uvac_cmd_showhelp(vk_printk);
		return 0;
	}
	for (loop = 1 ; loop <= cmd_num ; loop++) {
		if (strncmp(argv[1], uvac_cmd_tbl[loop].p_name, strlen(argv[1])) == 0) {
			ret = uvac_cmd_tbl[loop].p_func(argc-2, &argv[2]);
			return ret;
		}
	}

	return 0;
}
#endif

