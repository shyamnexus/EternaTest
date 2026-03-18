#ifndef _LIB_DIS_VERSION_H_
#define _LIB_DIS_VERSION_H_

#define dis_major     02
#define dis_minor     00
#define dis_bugfix    24111200
#define dis_ext       0

#define _DIS_STR(a,b,c)          #a"."#b"."#c
#define DIS_STR(a,b,c)           _DIS_STR(a,b,c)
#define LIB_DIS_IMPL_VERSION    DIS_STR(dis_major, dis_minor, dis_bugfix) //implementation version major.minor.yymmdds

#endif //_LIB_DIS_VERSION_H_