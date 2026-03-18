#ifndef _LIB_VQA_VERSION_H_
#define _LIB_VQA_VERSION_H_

#define vqa_major     01
#define vqa_minor     00
#define vqa_bugfix    220823
#define vqa_ext       0

#define _VQA_STR(a,b,c)          #a"."#b"."#c
#define VQA_STR(a,b,c)           _VQA_STR(a,b,c)
#define LIB_VQA_IMPL_VERSION    VQA_STR(vqa_major, vqa_minor, vqa_bugfix)//implementation version major.minor.yymmdds

#endif
