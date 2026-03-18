#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "h26xenc_rate_control.h"
#include "../nvt_vencrc_dbg.h"

#define RC_MAX_CHN	8
/*
	return 0: success
	return -1: fail
*/ 
struct UsrRCParam 
{
	void *param_buf_addr;	// buffer start address
	int param_buf_size;		// buffer size
	int param_data_size;	// user data size
	int chn;
	int bitrate;
	int frame_rate_base;
	int frame_rate_incr;
	int min_i_qp;
	int max_i_qp;
	int min_p_qp;
	int max_p_qp;
};

struct UsrRCParam rc_param[RC_MAX_CHN] = {0};

int h26xEnc_RcInitBuffer(int chn, int size)
{
	if (chn < RC_MAX_CHN) {
		if (NULL == rc_param[chn].param_buf_addr) {
			rc_param[chn].param_buf_addr = kmalloc(size, GFP_KERNEL);
			if (rc_param[chn].param_buf_addr == NULL) {
				printk("allocate rc buffer fail\n");
				return -1;
			}
			rc_param[chn].param_buf_size = size;
		}
	}
	return 0;
}

int h26xEnc_RcClearBuffer(int chn)
{
	if (chn < RC_MAX_CHN) {
		if (rc_param[chn].param_buf_addr) {
			kfree(rc_param[chn].param_buf_addr);
		}
	}
	return 0;
}

int h26xEnc_RcInit_usr(H26XEncRC *pRc, H26XEncRCParam *pRCParam)
{
	int chn = pRCParam->uiEncId;
	unsigned int *ptr;
	int i;
	
	if (chn > RC_MAX_CHN)
		return -1;
	pRc->m_chn = pRCParam->uiEncId;
	pRc->m_currQP = 26;
	
	rc_param[chn].chn = pRCParam->uiEncId;
	rc_param[chn].bitrate = pRCParam->uiBitRate;
	rc_param[chn].frame_rate_base = pRCParam->uiFrameRateBase;
	rc_param[chn].frame_rate_incr = pRCParam->uiFrameRateIncr;
	rc_param[chn].min_i_qp = pRCParam->uiMinIQp;
	rc_param[chn].max_i_qp = pRCParam->uiMaxIQp;
	rc_param[chn].min_p_qp = pRCParam->uiMinPQp;
	rc_param[chn].max_p_qp = pRCParam->uiMaxPQp;
	
	if (rc_param[chn].param_buf_addr == NULL) {
		printk("rc buffer fail\n");
		return -1;
	}
	if (rc_param[chn].param_buf_size < pRCParam->uiUserDataSize) {
		printk("allocate buffer size (%d) is less than user data size (%d)\n", rc_param[chn].param_buf_size, pRCParam->uiUserDataSize);
		return -1;
	}
	if (copy_from_user(rc_param[chn].param_buf_addr, (void *)pRCParam->uiUserDataAddr, pRCParam->uiUserDataSize)) {
        printk("copy from user failed\n");
        return -1;
    }
	rc_param[chn].param_data_size = pRCParam->uiUserDataSize;
	printk("{chn%d} bitrate %d, fps %d/%d\n", rc_param[chn].chn, rc_param[chn].bitrate, rc_param[chn].frame_rate_base, rc_param[chn].frame_rate_incr);
	
	ptr = (unsigned int *)rc_param[chn].param_buf_addr;
	for (i = 0; i < rc_param[chn].param_data_size/4; i+=4) {
		printk("%08x %08x %08x %08x\n", ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
	
	return 0;
}

/*
	return frame qp
*/
int h26xEnc_RcPreparePicture_usr(H26XEncRC *pRc, H26XEncRCPreparePic *pPic)
{
	printk("{chn%d} qp = %d\n", pRc->m_chn, pRc->m_currQP);
	return pRc->m_currQP;
}

int h26xEnc_RcUpdatePicture_usr(H26XEncRC *pRc, H26XEncRCUpdatePic *pUPic)
{
	pRc->m_currQP++;
	if (pRc->m_currQP > 45)
		pRc->m_currQP = 26;
    return 0;
}

int h26xEnc_RcGetLog_usr(H26XEncRC *pRc, uintptr_t *log_addr)
{
    *log_addr = (uintptr_t)(&pRc->rc_log_info);
    return sizeof(pRc->rc_log_info);
}
