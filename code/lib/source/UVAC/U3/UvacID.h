#include "kwrap/type.h"
#include "kwrap/error_no.h"
#include "kwrap/task.h"
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "UVAC.h"

#ifndef _UVACID_H
#define _UVACID_H

#define PRI_UVACVIDEO           5//10
#define PRI_UVACISOIN           PRI_UVACVIDEO //(PRI_UVACVIDEO + 2)
#define PRI_UVACISOOUT          4
#define STKSIZE_UVACVIDEO           8192 //8192  //4096
#define STKSIZE_UVACISOIN_VID       8192  //2048
#define STKSIZE_UVACISOIN_AUD       4096
#define STKSIZE_UVACISOOUT          4096  //2048
//#define THREAD_HANDLE pthread_t
//#define THREAD_DECLARE(name, arglist) void* name(void *arglist)

//typedef UINT32        FLGPTN;                     ///< Flag patterns
//typedef UINT32        *PFLGPTN;                   ///< Flag patterns (Pointer)
#define FLGPTN_BIT(n)       ((FLGPTN)(1 << (n)))        ///< Bit of flag pattern
#define FLGPTN_BIT_ALL      ((FLGPTN)0xFFFFFFFF)

//#define TWF_ANDW    0x00                /* AND condition            */
//#define TWF_ORW     0x02                /* OR condition             */
//#define TWF_CLR     0x01                /* Clear condition          */

/*typedef struct _UVAC_FLAG
{
	UINT32 flag;
	pthread_cond_t  cond;
    pthread_mutex_t mutex;
} UVAC_FLAG;*/

//#define sig_sem(handle) sem_post(&handle)
//#define wai_sem(handle) sem_wait(&handle)

//#define kent_tsk(x)

//#define UVAC_FLAG_DIAG

extern ID SEMID_UVC_QUEUE;
extern ID SEMID_UVC_READ_CDC;
extern ID SEMID_UVC_WRITE_CDC;
extern ID SEMID_UVC_READ2_CDC;
extern ID SEMID_UVC_WRITE2_CDC;

extern ID SEMID_UVC_WRITE_HID;
extern ID SEMID_UVC_READ_HID;

extern ID SEMID_U3UVC_WRITE_INT;

extern ID SEMID_UVC_UAC_QUEUE;

extern ID FLG_ID_UVAC_UAC_RX;

extern ID FLG_ID_UVAC;
extern ID FLG_ID_UVAC_UAC;
extern ID FLG_ID_UVAC_FRM;
extern ID FLG_ID_UVAC_UAC_RX;
extern ID FLG_ID_UVAC_UVC3;
extern THREAD_HANDLE UVACVIDEOTSK_ID;
extern THREAD_HANDLE UVACVIDEOTSK_ID2;
extern THREAD_HANDLE UVACVIDEOTSK_ID3;
extern THREAD_HANDLE UVAC_TX_VDO1_ID;
extern THREAD_HANDLE UVAC_TX_VDO2_ID;
extern THREAD_HANDLE UVAC_TX_VDO3_ID;
extern THREAD_HANDLE UVAC_TX_AUD1_ID;
extern THREAD_HANDLE UVAC_TX_AUD2_ID;
extern THREAD_HANDLE UVAC_UAC_RX_ID;

#if 0//gU2UvacMtpEnabled
extern ID UVACSIDCTSK_ID; ///< task id
extern ID UVACSIDCINTRTSK_ID; ///< task id
extern ID SEMID_UVC_OBJ_SIDC; ///< semaphore id
extern ID SEMID_UVC_DIR_SIDC; ///< semaphore id
#endif
#if 0
extern ER uvac_set_flg(UVAC_FLAG* flag,UINT32 setptn);
extern ER uvac_wai_flg(UINT32* p_flgptn, UVAC_FLAG* flag, UINT32 waiptn, UINT32 wfmode);
extern ER uvac_clr_flg(UVAC_FLAG* flag, UINT32 clrptn);
extern FLGPTN uvac_kchk_flg(UVAC_FLAG* flag, UINT32 chkptn);
extern ER cfg_flg(UVAC_FLAG* flag);
extern ER destroy_flg(UVAC_FLAG* flag);
#endif
//#define OS_CONFIG_SEMPHORE(id, attr, cnt, max_cnt) sem_init(&id, attr, max_cnt)
//#define OS_CONFIG_FLAG(id) cfg_flg(&id)

//#define set_flg(flag,setptn) uvac_set_flg(&flag,setptn)

//#define wai_flg(p_flgptn, flag, waiptn, wfmode) uvac_wai_flg(p_flgptn, &flag, waiptn, wfmode)

//#define clr_flg(flag, clrptn) uvac_clr_flg(&flag, clrptn)

//#define kchk_flg(flag, chkptn) uvac_kchk_flg(&flag, chkptn)


#endif //_UVACID_H
