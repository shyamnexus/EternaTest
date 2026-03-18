#ifndef _NVT_API_VER_H
#define _NVT_API_VER_H

#define NVT_API_VERSION 0x22080808
#define NVT_API_CHK_DECLARE(name) BOOL name##_api_chk(int ver)
#define NVT_API_CHK_CALL(name) name##_api_chk(NVT_API_VERSION)
#define NVT_API_CHK_DECLARE_EXTEND(name) BOOL name##_api_chk_extend(int ver, unsigned int param1, unsigned int param2)
#define NVT_API_CHK_CALL_EXTEND(name, param1, param2) name##_api_chk_extend(NVT_API_VERSION, param1, param2)

#endif