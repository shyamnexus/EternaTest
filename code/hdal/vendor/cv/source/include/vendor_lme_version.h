#ifndef _VENDOR_LME_VERSION_H_
#define _VENDOR_LME_VERSION_H_

#define lme_major     01
#define lme_minor     00
#define lme_bugfix    2206090
#define lme_ext       0

#define _LME_STR(a,b,c)          #a"."#b"."#c
#define LME_STR(a,b,c)           _LME_STR(a,b,c)
#define VENDOR_LME_IMPL_VERSION    LME_STR(lme_major, lme_minor, lme_bugfix)//implementation version major.minor.yymmdds


#endif //_VENDOR_LME_VERSION_H_