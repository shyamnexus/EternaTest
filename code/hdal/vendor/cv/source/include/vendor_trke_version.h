#ifndef _VENDOR_TRKE_VERSION_H_
#define _VENDOR_TRKE_VERSION_H_

#define trke_major     01
#define trke_minor     00
#define trke_bugfix    2203030
#define trke_ext       0

#define _TRKE_STR(a,b,c)          #a"."#b"."#c
#define TRKE_STR(a,b,c)           _TRKE_STR(a,b,c)
#define VENDOR_TRKE_IMPL_VERSION    TRKE_STR(trke_major, trke_minor, trke_bugfix)//implementation version major.minor.yymmdds


#endif //_VENDOR_IVE_VERSION_H_