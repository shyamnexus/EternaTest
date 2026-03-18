#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "nvt_vencrc_dbg.h"
#include "h26xenc_rc.h"

#define RC_BUFFER_SIZE	(100*1024)

/* extern function */
extern int h26xEnc_RcInitBuffer(int chn, int size);
extern int h26xEnc_RcClearBuffer(int chn);

extern int h26xEnc_RcInit_usr(H26XEncRC *pRc, H26XEncRCParam *pRCParam);
extern int h26xEnc_RcPreparePicture_usr(H26XEncRC *pRc, H26XEncRCPreparePic *pPic);
extern int h26xEnc_RcUpdatePicture_usr(H26XEncRC *pRc, H26XEncRCUpdatePic *pUPic);
extern int h26xEnc_RcGetLog_usr(H26XEncRC *pRc, uintptr_t *log_addr);

int __init nvt_vencrc_module_init(void)
{
	int chn;

	rc_cb *cb = rc_cb_init();
	
	cb->h26xEnc_RcInit = h26xEnc_RcInit_usr;
	cb->h26xEnc_RcPreparePicture = h26xEnc_RcPreparePicture_usr;
	cb->h26xEnc_RcUpdatePicture = h26xEnc_RcUpdatePicture_usr;
    cb->h26xEnc_RcGetLog = h26xEnc_RcGetLog_usr;
	
	h26xEnc_setRCSyncMaqStr(0);

	for (chn = 0; chn < 8; chn++) {
		h26xEnc_RcInitBuffer(chn, RC_BUFFER_SIZE);
	}

	return 0;
}

void __exit nvt_vencrc_module_exit(void)
{
	int chn;

	rc_cb *cb = rc_cb_init();

	cb->h26xEnc_RcInit = NULL;
	cb->h26xEnc_RcPreparePicture = NULL;
	cb->h26xEnc_RcUpdatePicture = NULL;
    cb->h26xEnc_RcGetLog = NULL;
	
	h26xEnc_setRCSyncMaqStr(1);
	
	for (chn = 0; chn < 8; chn++) {
		h26xEnc_RcClearBuffer(chn);
	}
}

module_init(nvt_vencrc_module_init);
module_exit(nvt_vencrc_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("vencrc driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");
