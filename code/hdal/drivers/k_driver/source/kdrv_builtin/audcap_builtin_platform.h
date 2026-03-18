#ifndef _AUDCAP_BUILTIN_PLATFORM_H
#define _AUDCAP_BUILTIN_PLATFORM_H

extern void _audcap_builtin_set_clk(int sampling_rate);
extern void _audcap_builtin_set_pad(void);
extern int audcap_builtin_get_samplerate(int nodeoffset);
extern int audcap_builtin_get_channel(int nodeoffset);
extern int audcap_builtin_get_bufcount(int nodeoffset);
extern int audcap_builtin_get_bufsamplecnt(int nodeoffset);
extern int audcap_builtin_get_rec_src(int nodeoffset);
extern int audcap_builtin_get_vol(int nodeoffset);
extern UINT32 audcap_builtin_get_aec_en(int nodeoffset);
extern UINT32 audcap_builtin_get_txchannel(int nodeoffset);
extern UINT32 audcap_builtin_get_default_setting(int nodeoffset);
extern int audcap_builtin_get_nodeoffset(int *nodeoffset);
#endif