#ifndef _VENDOR_MD_VERSION_H_
#define _VENDOR_MD_VERSION_H_

#define md_major     01
#define md_minor     01
#define md_bugfix    2411120
#define md_ext       0

#define _MD_STR(a,b,c)          #a"."#b"."#c
#define MD_STR(a,b,c)           _MD_STR(a,b,c)
#define VENDOR_MD_IMPL_VERSION    MD_STR(md_major, md_minor, md_bugfix)//implementation version major.minor.yymmdds


#endif //_VENDOR_MD_VERSION_H_