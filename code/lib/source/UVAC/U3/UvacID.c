#include "UvacID.h"
#include "UVAC.h"
#include "UvacVideoTsk.h"
#include "UvacIsoInTsk.h"

ID SEMID_UVC_QUEUE;
ID SEMID_UVC_READ_CDC;
ID SEMID_UVC_WRITE_CDC;
ID SEMID_UVC_READ2_CDC;
ID SEMID_UVC_WRITE2_CDC;

ID SEMID_UVC_WRITE_HID = 0;
ID SEMID_UVC_READ_HID = 0;

ID SEMID_U3UVC_WRITE_INT = 0;

ID FLG_ID_UVAC = 0;
ID FLG_ID_UVAC_UAC = 0;
ID FLG_ID_UVAC_FRM = 0;
ID FLG_ID_UVAC_UVC3 = 0;
THREAD_HANDLE UVACVIDEOTSK_ID = 0;
THREAD_HANDLE UVACVIDEOTSK_ID2 = 0;
THREAD_HANDLE UVACVIDEOTSK_ID3 = 0;
THREAD_HANDLE UVAC_TX_VDO1_ID = 0;
THREAD_HANDLE UVAC_TX_VDO2_ID = 0;
THREAD_HANDLE UVAC_TX_VDO3_ID = 0;
THREAD_HANDLE UVAC_TX_AUD1_ID = 0;
THREAD_HANDLE UVAC_TX_AUD2_ID = 0;

#define PRI_UVACSIDC            10
#define PRI_UVACINTR            8
#define STKSIZE_UVACSIDCVND         4096
#define STKSIZE_UVACSIDCINTR        2048
//UVAC_FLAG FLG_ID_SIDC = {0};
//UVAC_FLAG FLG_ID_SIDCINTR = {0};
#if 0//gUvacMtpEnabled
UINT32 UVACSIDCTSK_ID = 0;
UINT32 UVACSIDCINTRTSK_ID = 0;
UINT32 SEMID_UVC_OBJ_SIDC = 0;
UINT32 SEMID_UVC_DIR_SIDC = 0;
#endif
extern UINT32 gUvacChannel;
extern UINT32 gHsUvacChannel;
extern BOOL gUvacCdcEnabled[CDC_COM_MAX_NUM];
extern UVAC_HID_INFO g_u3_hid_info;

ID FLG_ID_UVAC_UAC_RX = 0;
ID SEMID_UVC_UAC_QUEUE = 0;
THREAD_HANDLE UVAC_UAC_RX_ID = 0;

void U3UVAC_InstallID(void)
{
	OS_CONFIG_SEMPHORE(SEMID_UVC_QUEUE, 0, 1, 1);
	if (gUvacCdcEnabled[CDC_COM_1ST]) {
		OS_CONFIG_SEMPHORE(SEMID_UVC_READ_CDC, 0, 1, 1);
		OS_CONFIG_SEMPHORE(SEMID_UVC_WRITE_CDC, 0, 1, 1);
	}
	if (gUvacCdcEnabled[CDC_COM_2ND]) {
		OS_CONFIG_SEMPHORE(SEMID_UVC_READ2_CDC, 0, 1, 1);
		OS_CONFIG_SEMPHORE(SEMID_UVC_WRITE2_CDC, 0, 1, 1);
	}
	if(g_u3_hid_info.en) {
		OS_CONFIG_SEMPHORE(SEMID_UVC_READ_HID, 0, 1, 1);
		OS_CONFIG_SEMPHORE(SEMID_UVC_WRITE_HID, 0, 1, 1);
	}
	OS_CONFIG_SEMPHORE(SEMID_U3UVC_WRITE_INT, 0, 1, 1);
	OS_CONFIG_SEMPHORE(SEMID_UVC_UAC_QUEUE, 0, 1, 1);
	OS_CONFIG_FLAG(FLG_ID_UVAC);
	OS_CONFIG_FLAG(FLG_ID_UVAC_UAC);
	OS_CONFIG_FLAG(FLG_ID_UVAC_FRM);
	OS_CONFIG_FLAG(FLG_ID_UVAC_UAC_RX);
	OS_CONFIG_FLAG(FLG_ID_UVAC_UVC3);

}

void U3UVAC_UnInstallID(void)
{
	vos_sem_destroy(SEMID_UVC_QUEUE);
	if (gUvacCdcEnabled[CDC_COM_1ST]) {
		vos_sem_destroy(SEMID_UVC_READ_CDC);
		vos_sem_destroy(SEMID_UVC_WRITE_CDC);
	}
	if (gUvacCdcEnabled[CDC_COM_2ND]) {
		vos_sem_destroy(SEMID_UVC_READ2_CDC);
		vos_sem_destroy(SEMID_UVC_WRITE2_CDC);
	}
	if(g_u3_hid_info.en) {
		vos_sem_destroy(SEMID_UVC_READ_HID);
		vos_sem_destroy(SEMID_UVC_WRITE_HID);
	}
	vos_sem_destroy(SEMID_U3UVC_WRITE_INT);
	vos_sem_destroy(SEMID_UVC_UAC_QUEUE);
	vos_flag_destroy(FLG_ID_UVAC);
	vos_flag_destroy(FLG_ID_UVAC_UAC);
	vos_flag_destroy(FLG_ID_UVAC_FRM);
	vos_flag_destroy(FLG_ID_UVAC_UAC_RX);
	vos_flag_destroy(FLG_ID_UVAC_UVC3);
}

#if 0//gUvacMtpEnabled
void UVAC_MTPInstallIDInt(void)
{
	OS_CONFIG_SEMPHORE(SEMID_UVC_OBJ_SIDC, 0, 1, 1);
	OS_CONFIG_SEMPHORE(SEMID_UVC_DIR_SIDC, 0, 1, 1);
	OS_CONFIG_FLAG(FLG_ID_SIDC);
	OS_CONFIG_FLAG(FLG_ID_SIDCINTR);
	OS_CONFIG_TASK(UVACSIDCTSK_ID,    PRI_UVACSIDC,  STKSIZE_UVACSIDCVND,  UVAC_SIDCTsk);
	OS_CONFIG_TASK(UVACSIDCINTRTSK_ID,    PRI_UVACINTR,  STKSIZE_UVACSIDCINTR,  UVAC_SIDCINTRTsk);
}
#endif

#if 0
ER uvac_set_flg(UVAC_FLAG* flag,UINT32 setptn)
{
	pthread_mutex_lock(&flag->mutex);
	flag->flag |= setptn;
	UVAC_FLAG_DIAG("setflag 0x%x\r\n",flag);
	pthread_cond_broadcast(&flag->cond);
	pthread_mutex_unlock(&flag->mutex);

	return E_OK;
}

ER uvac_wai_flg(UINT32* p_flgptn, UVAC_FLAG* flag, UINT32 waiptn, UINT32 wfmode)
{
	UINT32 clr = wfmode & TWF_CLR;
	pthread_mutex_lock(&(flag->mutex));

	UVAC_FLAG_DIAG("waitflag 0x%x\r\n",flag->flag);
	if (wfmode & TWF_ORW) {
		if (flag->flag & waiptn){
			*p_flgptn = flag->flag & waiptn;

			if (clr) {
				flag->flag &= ~(waiptn);
			}
			pthread_mutex_unlock(&(flag->mutex));
			return E_OK;
		}
	} else {
		if ((flag->flag & waiptn) == waiptn){
			*p_flgptn = flag->flag & waiptn;

			if (clr) {
				flag->flag &= ~(waiptn);
			}
			pthread_mutex_unlock(&(flag->mutex));
			return E_OK;
		}
	}

	while (1) {
		UVAC_FLAG_DIAG("cond_wait b 0x%x\r\n",flag);
		pthread_cond_wait(&flag->cond, &flag->mutex);
		UVAC_FLAG_DIAG("cond_wait e flag 0x%x, ptn 0x%x\r\n", flag->flag, waiptn);
		if (wfmode & TWF_ORW) {
			if (flag->flag & waiptn) {
				*p_flgptn = flag->flag & waiptn;

				if (clr) {
					flag->flag &= ~(waiptn);
				}
				pthread_mutex_unlock(&flag->mutex);
				break;
			}
		} else {
			if ((flag->flag & waiptn) == waiptn){
				*p_flgptn = flag->flag & waiptn;

				if (clr) {
					flag->flag &= ~(waiptn);
				}
				pthread_mutex_unlock(&(flag->mutex));
				break;
			}
		}
	}

	return E_OK;
}

ER uvac_clr_flg(UVAC_FLAG* flag, UINT32 clrptn)
{
	pthread_mutex_lock(&flag->mutex);
	flag->flag &= ~(clrptn);
	pthread_mutex_unlock(&flag->mutex);

	return E_OK;
}

FLGPTN uvac_kchk_flg(UVAC_FLAG* flag, UINT32 chkptn)
{
	FLGPTN ptn;

	pthread_mutex_lock(&flag->mutex);
	ptn = flag->flag & chkptn;
	pthread_mutex_unlock(&flag->mutex);

	return ptn;
}

ER cfg_flg(UVAC_FLAG* flag)
{
	flag->flag = 0;
	pthread_cond_init(&flag->cond, NULL);
	pthread_mutex_init(&flag->mutex, NULL);
}

ER destroy_flg(UVAC_FLAG* flag)
{
	flag->flag = 0;
	pthread_cond_destroy(&flag->cond);
	pthread_mutex_destroy(&flag->mutex);
}
#endif
