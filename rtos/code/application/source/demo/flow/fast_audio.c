#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <kwrap/type.h>
#include <kwrap/perf.h>
#include <kwrap/cpu.h>
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include "prjcfg.h"
#include "sys_fwload.h"
#include "sys_fastboot.h"
#include "sys_linuxboot.h"
#include "flow_boot_linux.h"
#include <kwrap/debug.h>
#include "vendor_videocapture.h"
#include "isp_api.h"
#include "sen_inc.h"
#include "kwrap/util.h"
#include <dispdevctrl.h>
#include <plat/nand.h>
#include "sys_storage_partition.h"
#include <MemCheck.h>
#include "kdrv_audioio/kdrv_audioio.h"
#include "isp_builtin.h" // NOTE: Include ISP builtin

#if (CFG_LOAD_AUDIOIO)
typedef enum {
	AUDIO_CH_LEFT,              ///< Left
	AUDIO_CH_RIGHT,             ///< Right
	AUDIO_CH_STEREO,            ///< Stereo
	AUDIO_CH_MONO,              ///< Mono two channel. Obselete. Shall not use this option.
	AUDIO_CH_DUAL_MONO,         ///< Dual Mono Channels. Valid for record(RX) only.

	ENUM_DUMMY4WORD(AUDIO_CH)
} AUDIO_CH;

typedef enum {
	AUDIO_SR_8000   = 8000,     ///< 8 KHz
	AUDIO_SR_11025  = 11025,    ///< 11.025 KHz
	AUDIO_SR_12000  = 12000,    ///< 12 KHz
	AUDIO_SR_16000  = 16000,    ///< 16 KHz
	AUDIO_SR_22050  = 22050,    ///< 22.05 KHz
	AUDIO_SR_24000  = 24000,    ///< 24 KHz
	AUDIO_SR_32000  = 32000,    ///< 32 KHz
	AUDIO_SR_44100  = 44100,    ///< 44.1 KHz
	AUDIO_SR_48000  = 48000,    ///< 48 KHz

	ENUM_DUMMY4WORD(AUDIO_SR)
} AUDIO_SR;

typedef struct _AUDIO_OBJ {
	UINT32              aud_eid;
	UINT32              aud_vol;
	AUDIO_SR            aud_sr;          ///< Sample Rate
	AUDIO_CH            aud_ch;          ///< Channel
	UINT32              ch_num;          ///< Channel number
	KDRV_BUFFER_INFO    buf_info;
	KDRV_CALLBACK_FUNC  buf_cb;
	UINT32              play_num;
} AUDIO_OBJ, *PAUDIO_OBJ;

static KDRV_AUD_HANDLE g_audio_handle = KDRV_AUD_HANDLE_MAX;

#if CFG_LOAD_SOUND
/*
 * The required PCM audio sound data should be prepared,
 * And copy it to the allocated buffer.
 * PCM audio sound data should be placed on fast_audio_sound.c.
 * Use CFG_LOAD_SOUND_PATTERN to perform sound data pattern.
 */
extern void fast_get_audio_sound(UINT32 *p_sound, UINT32 *p_sound_size);
#endif
static INT32 fast_trig_audio_play_buf_cb(VOID *callback_info, VOID *user_data)
{
	printf("[fast_trig_audio_play_buf_cb] user_data = %d\r\n", (unsigned int)user_data);
	return 0;
}

static INT32 fast_trig_audio_reserve_buf(uintptr_t phy_addr, VOID *user_data)
{
	return 0;
}

static INT32 fast_trig_audio_free_buf(uintptr_t phy_addr, VOID *user_data)
{
	return 0;
}

static int fast_trig_audio(PAUDIO_OBJ aud_obj)
{
	KDRV_AUD_HANDLE handle = g_audio_handle;
	KDRV_AUDIO_TRIGGER_TYPE trigger_type = KDRV_AUDIO_TRIGGER_TYPE_TX;
	KDRV_AUDIO_TRIGGER_INFO triger_info = {0};
	KDRV_BUFFER_INFO     buf_info = {0};
	KDRV_CALLBACK_FUNC   buf_cb = {0};
	UINT32 user_data = aud_obj->play_num;
	UINT32 out_delay_queue_count = 0;
	UINT32 param[3] = {0};
	BOOL trigger = 1;

	if (handle >= KDRV_AUD_HANDLE_MAX) {
		DBG_ERR("audio handle fail = %d\r\n", handle);
		return -1;
	}

	buf_info.addr_pa = aud_obj->buf_info.addr_pa;
	buf_info.addr_va = aud_obj->buf_info.addr_va;
	buf_info.size    = aud_obj->buf_info.size;
	buf_info.ddr_id  = trigger;

	buf_cb.callback    = aud_obj->buf_cb.callback;
	buf_cb.reserve_buf = aud_obj->buf_cb.reserve_buf;
	buf_cb.free_buf    = aud_obj->buf_cb.free_buf;

	param[0] = out_delay_queue_count;
	//printf("set:hdl-0x%x/OUT_DELAY_QUEUE_COUNT:%d\r\n", handle, (int)param[0]);
	kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_DELAY_QUEUE_COUNT, (VOID *)&param[0]);

	if (trigger) {
		triger_info.ctrl = &buf_info;
		triger_info.cb_func = &buf_cb;
		triger_info.user_data = (ULONG)user_data;
		//printf("trigger:hdl-0x%x(0x%lx,0x%lx,0x%lx)\r\n", handle,
		//		(unsigned long)buf_info.addr_pa, (unsigned long)buf_info.addr_va, (unsigned long)(unsigned long)buf_info.size);
		if (kdrv_audioio_trigger(handle, trigger_type, &triger_info) != 0) {
			DBG_ERR("kdrv_audioio_trigger fail\r\n");
			return FALSE;
		}
	}

	printf("[fast_trig_audio] pa = %d va = %d size= %d\r\n", (unsigned int)aud_obj->buf_info.addr_pa, (unsigned int)aud_obj->buf_info.addr_va, (unsigned int)aud_obj->buf_info.size);

	return TRUE;
}

int fast_trig_audioout(void)
{
	int ret = 0;
	AUDIO_OBJ aud_out_obj = {0};
	UINT32 mempool_audioout = 0, memsize_audioout = 0;
	UINT32 buf_size;

	aud_out_obj.aud_eid = KDRV_DEV_ID(0, KDRV_AUDOUT_ENGINE0, 0);

	buf_size = 0x20000;
	mempool_audioout = (UINT32)malloc(buf_size);
	memsize_audioout = buf_size;

#if CFG_LOAD_SOUND
	/*
	 * The required PCM audio sound data should be prepared,
	 * And copy it to the allocated buffer.
	 */
	UINT32 sound_data_addr = 0;
	UINT32 sound_data_size = 0;
	fast_get_audio_sound((UINT32 *)&sound_data_addr, (UINT32 *)&sound_data_size);
	if (sound_data_addr == 0 || sound_data_size == 0) {
		DBG_ERR("p_sound_data get fail\r\n");
		ret = -1;
	} else {
		buf_size = (sound_data_size > memsize_audioout) ? memsize_audioout : sound_data_size;
		memcpy((void *)mempool_audioout, (void *)sound_data_addr, buf_size);
		memsize_audioout = buf_size;
	}
#endif

	if (mempool_audioout == 0) {
		DBG_ERR("mempool_audioout malloc fail\r\n");
		ret = -1;
	} else {
		aud_out_obj.buf_info.addr_pa   = mempool_audioout;
		aud_out_obj.buf_info.addr_va   = mempool_audioout;
		aud_out_obj.buf_info.size      = memsize_audioout;
		aud_out_obj.buf_info.ddr_id    = 1;

		aud_out_obj.buf_cb.callback    = fast_trig_audio_play_buf_cb;
		aud_out_obj.buf_cb.reserve_buf = fast_trig_audio_reserve_buf;
		aud_out_obj.buf_cb.free_buf    = fast_trig_audio_free_buf;

		aud_out_obj.play_num = 1;

		ret = fast_trig_audio(&aud_out_obj);
	}

	return ret;
}

int fast_open_audio(PAUDIO_OBJ aud_obj)
{
	int ret = 0;
	UINT32 aud_vol = aud_obj->aud_vol;
	UINT32 max_vol_lvl = 160;
	UINT32 aud_sr = aud_obj->aud_sr, aud_ch = aud_obj->aud_ch, aud_ch_num = aud_obj->ch_num;
	UINT32 eid = aud_obj->aud_eid;
	UINT32 param[3] = {0};
	KDRV_AUD_HANDLE handle = KDRV_AUD_HANDLE_MAX;

	//... open hardware engine
	//printf("open:chip-%d/eng-0x%x\r\n", 0, KDRV_DEV_ID_ENGINE(eid));
	handle = kdrv_audioio_open(0, KDRV_DEV_ID_ENGINE(eid));
	//printf("open:hdl-0x%x\r\n", handle);
	if (handle >= KDRV_AUD_HANDLE_MAX) {
		DBG_ERR("kdrv_audioio_open fail = %d\r\n", handle);
		return -1;
	}
	g_audio_handle = handle;

	//... setting samplerate
	param[0] = aud_sr;
	//printf("set:hdl-0x%x/GLOBAL_SAMPLE_RATE:%d\r\n", handle, (int)param[0]);
	kdrv_audioio_set(handle, KDRV_AUDIOIO_GLOBAL_SAMPLE_RATE, (VOID *)&param[0]);

	//... setting channel
	param[0] = 0;
	//printf("set:hdl-0x%x/MONO_EXPAND:%d\r\n", handle, (int)param[0]);
	kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_MONO_EXPAND, (VOID *)&param[0]);
	if (aud_ch == AUDIO_CH_STEREO) {
		/* config datatype*/
		param[0] = KDRV_AUDIO_CHANNEL_DATA_STEREO;
		//printf("set:hdl-0x%x/OUT_CHANNEL_DATATYPE:%d\r\n", handle, (int)param[0]);
		kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_CHANNEL_DATATYPE, (VOID*)&param[0]);

		param[0] = aud_ch_num;
		//printf("set:hdl-0x%x/OUT_CHANNEL_NUMBER:%d\r\n", handle, (int)param[0]);
		kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_CHANNEL_NUMBER, (VOID *)&param[0]);
	} else if (aud_ch == AUDIO_CH_LEFT) {
		/* config datatype*/
		param[0] = KDRV_AUDIO_CHANNEL_DATA_MONO;
		//printf("set:hdl-0x%x/OUT_CHANNEL_DATATYPE:%d\r\n", handle, (int)param[0]);
		kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_CHANNEL_DATATYPE, (VOID*)&param[0]);

		param[0] = KDRV_AUDIO_OUT_MONO_LEFT;
		//printf("set:hdl-0x%x/OUT_MONO_SEL:%d\r\n", handle, (int)param[0]);
		kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_MONO_SEL, (VOID *)&param[0]);
	} else {
		/* config datatype*/
		param[0] = KDRV_AUDIO_CHANNEL_DATA_MONO;
		//printf("set:hdl-0x%x/OUT_CHANNEL_DATATYPE:%d\r\n", handle, (int)param[0]);
		kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_CHANNEL_DATATYPE, (VOID*)&param[0]);

		param[0] = KDRV_AUDIO_OUT_MONO_RIGHT;
		//printf("set:hdl-0x%x/OUT_MONO_SEL:%d\r\n", handle, (int)param[0]);
		kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_MONO_SEL, (VOID *)&param[0]);
	}

	/* config bitwidth */
	param[0] = 16;
	//printf("set:hdl-0x%x/OUT_BIT_WIDTH:%d\r\n", handle, (int)param[0]);
	kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_BIT_WIDTH, (VOID*)&param[0]);

	//... setting volume
	if (((UINT32)aud_vol) > max_vol_lvl) {
		DBG_WRN("Vol too large=%d, scale down to 100\r\n", aud_vol);
		aud_vol = 100;  //fix for CID 42874
	}
	param[0] = aud_vol;
	//printf("set:hdl-0x%x/CAP_PATH:%d\r\n", handle, (int)param[0]);
	kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_VOLUME, (VOID *)&param[0]);

	//... setting outpath
	param[0] = KDRV_AUDIO_OUT_PATH_LINEOUT;
	kdrv_audioio_set(handle, KDRV_AUDIOIO_OUT_PATH, (VOID *)&param[0]);

	printf("[fast_open_audio] eid = %d, vol = %d, sr = %d, ch = %d, ch_num = %d\r\n", (unsigned int)aud_obj->aud_eid, (unsigned int)aud_obj->aud_vol, (unsigned int)aud_obj->aud_sr, (unsigned int)aud_obj->aud_ch, (unsigned int)aud_obj->ch_num);

	return ret;
}

int fast_open_audioout(void)
{
	AUDIO_OBJ aud_out_obj = {0};

	aud_out_obj.aud_eid = KDRV_DEV_ID(0, KDRV_AUDOUT_ENGINE0, 0);
	aud_out_obj.aud_sr = AUDIO_SR_16000;
	aud_out_obj.aud_ch = AUDIO_CH_STEREO;
	aud_out_obj.ch_num = 2;
	aud_out_obj.aud_vol = 100; // max_vol_lvl = 160

	return fast_open_audio(&aud_out_obj);
}
#endif
