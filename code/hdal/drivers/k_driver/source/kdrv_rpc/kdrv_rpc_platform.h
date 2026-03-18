#ifndef _KDRV_PRC_PLATFORM_H
#define _KDRV_PRC_PLATFORM_H


extern ID                 g_rpc_flgid;

extern int kdrv_rpc_create_resource(KDRV_RPC_MODULE_INFO *pmodule_info);
extern void kdrv_rpc_release_resource(KDRV_RPC_MODULE_INFO *pmodule_info);
#endif
