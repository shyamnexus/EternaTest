/*
    TGE module platfrom header.

    @file       tge_platform.h
    @ingroup    mIIPPTGE

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _TGE_PLATFORM_H_
#define _TGE_PLATFORM_H_

#include "kwrap/type.h"
#include "kwrap/flag.h"

#include "tge_lib.h"

/*************************************************************************************
 *  TGE Platform I/O R/W Definition
 *************************************************************************************/
#if defined(__LINUX)
#include <linux/io.h>
#define TGE_SETREG(_base, _ofs, _v)     (iowrite32((unsigned int)(_v), (void __iomem *)((_base)+(_ofs))))
#define TGE_GETREG(_base, _ofs)         (ioread32((void __iomem *)((_base)+(_ofs))))
#else
#define TGE_SETREG(_base, _ofs, _v)     (((volatile unsigned int *)(_base))[(_ofs)>>2]=(unsigned int)(_v))
#define TGE_GETREG(_base, _ofs)         (((volatile unsigned int *)(_base))[(_ofs)>>2])
#endif

/*************************************************************************************
 *  TGE Platform Event Flag Definition
 *************************************************************************************/
#define FLGPTN_TGE_VD                   FLGPTN_BIT(0)
#define FLGPTN_TGE_VD2                  FLGPTN_BIT(1)
#define FLGPTN_TGE_VD3                  FLGPTN_BIT(2)
#define FLGPTN_TGE_VD4                  FLGPTN_BIT(3)
#define FLGPTN_TGE_VD_BP                FLGPTN_BIT(8)
#define FLGPTN_TGE_VD2_BP               FLGPTN_BIT(9)
#define FLGPTN_TGE_VD3_BP               FLGPTN_BIT(10)
#define FLGPTN_TGE_VD4_BP               FLGPTN_BIT(11)
#define FLGPTN_TGE_FLSHA_TRG            FLGPTN_BIT(16)
#define FLGPTN_TGE_MSHA_CLOSE_TRG       FLGPTN_BIT(17)
#define FLGPTN_TGE_MSHA_OPEN_TRG        FLGPTN_BIT(18)
#define FLGPTN_TGE_FLSHA_END            FLGPTN_BIT(21)
#define FLGPTN_TGE_MSHA_CLOSE_END       FLGPTN_BIT(22)
#define FLGPTN_TGE_MSHA_OPEN_END        FLGPTN_BIT(23)

/*************************************************************************************
 *  TGE Platform Function Prototype
 *************************************************************************************/
extern int  tge_platform_is_inited(void);
extern ER   tge_platform_create_resource(TGE_RESOURCE *pRes);
extern void tge_platform_release_resource(void);

extern void tge_platform_prepare_clk(void);
extern void tge_platform_unprepare_clk(void);
extern void tge_platform_enable_clk(void);
extern void tge_platform_disable_clk(void);

extern ER   tge_platform_flg_clear(FLGPTN flg);
extern ER   tge_platform_flg_set(FLGPTN flg);
extern ER   tge_platform_flg_wait(PFLGPTN p_flgptn, FLGPTN flg);
extern ER   tge_platform_flg_wait_timeout(PFLGPTN p_flgptn, FLGPTN flg, int timeout_ms);

extern ER   tge_platform_sem_wait(void);
extern ER   tge_platform_sem_signal(void);

extern ER   tge_platform_set_flash_a_clk_src(TGE_FLASH_CLKSRC_SEL ClkSrc);
extern ER   tge_platform_set_msh_a_clk_src(TGE_MSH_CLKSRC_SEL ClkSrc);

#endif  /* _TGE_PLATFORM_H_ */
