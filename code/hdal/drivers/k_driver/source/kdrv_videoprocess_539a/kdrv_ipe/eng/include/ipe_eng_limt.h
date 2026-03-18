
#ifndef _IPE_ENG_LIMIT_H_
#define _IPE_ENG_LIMIT_H_

//=====================================================
// input / output limitation for Dram to Dram mode
#define IPE_SSDRV_SRCBUF_D2D_WMIN         32
#define IPE_SSDRV_SRCBUF_D2D_WMAX         8192
#define IPE_SSDRV_SRCBUF_D2D_WALIGN       4

#define IPE_SSDRV_SRCBUF_D2D_HMIN         16
#define IPE_SSDRV_SRCBUF_D2D_HMAX         8192
#define IPE_SSDRV_SRCBUF_D2D_HALIGN       4

#define IPE_SSDRV_SRCBUF_D2D_LOFF_ALIGN    4
#define IPE_SSDRV_SRCBUF_D2D_ADDR_ALIGN    4


//=====================================================
// input / output limitation for IPP mode
#define IPE_SSDRV_SRCBUF_IPP_WMIN         32
#define IPE_SSDRV_SRCBUF_IPP_WMAX         8192
#define IPE_SSDRV_SRCBUF_ALLDIR_WMAX      4096
#define IPE_SSDRV_SRCBUF_IPP_WALIGN       4

#define IPE_SSDRV_SRCBUF_IPP_HMIN         16
#define IPE_SSDRV_SRCBUF_IPP_HMAX         8192
#define IPE_SSDRV_SRCBUF_IPP_HALIGN       4

#define IPE_SSDRV_SRCBUF_IPP_LOFF_ALIGN    4
#define IPE_SSDRV_SRCBUF_IPP_ADDR_ALIGN    4


//=====================================================
// DEFOG input / output limitation
#define IPE_SSDRV_DEFOG_WMIN         4
#define IPE_SSDRV_DEFOG_WMAX         32

#define IPE_SSDRV_DEFOG_HMIN         4
#define IPE_SSDRV_DEFOG_HMAX         32

#define IPE_SSDRV_DEFOG_LOFF_ALIGN    4
#define IPE_SSDRV_DEFOG_ADDR_ALIGN    4




extern INT32 ipe_eng_chk_limitation(ULONG reg_base_addr, ULONG reg_flag_addr);



#endif
