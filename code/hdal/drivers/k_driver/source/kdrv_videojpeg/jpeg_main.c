//#define JPEG_VER_STR	"0.3.39"

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include "jpeg_main.h"
#include "jpeg_api.h"


//=============================================================================
// extern parameter
//=============================================================================
extern unsigned int jpeg_enc_max_chn;
extern unsigned int jpeg_dec_max_chn;


//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
//unsigned int jpeg_kdrv_debug_level = NVT_DBG_WARN;
module_param(jpeg_enc_max_chn, uint, S_IRUGO);
MODULE_PARM_DESC(jpeg_enc_max_chn, "encode max channel");
module_param(jpeg_dec_max_chn, uint, S_IRUGO);
MODULE_PARM_DESC(jpeg_dec_max_chn, "decode max channel");


//=============================================================================
// extern function
//=============================================================================
extern int nvt_jpg_proc_init(void);
extern int nvt_jpg_proc_clear(void);
extern int jpeg_kdrv_init(void);
extern void jpeg_kdrv_close(void);


int nvt_jpg_driver_msg(struct seq_file *sfile)
{
	if (NULL == sfile) {
		printk("JPEG kdrv version: %s, nvt version %s, built @ %s %s\r\n", JPEG_VER_STR, nvt_jpeg_get_version(), __DATE__, __TIME__);
	} else {
		seq_printf(sfile, "JPEG kdrv version: %s, nvt version %s, built @ %s %s\r\n", JPEG_VER_STR, nvt_jpeg_get_version(), __DATE__, __TIME__);
	}
	return 0;
}

int __init nvt_jpg_module_init(void)
{
	jpeg_kdrv_init();

	nvt_jpg_proc_init();

	nvt_jpg_driver_msg(NULL);

	return 0;
}

void __exit nvt_jpg_module_exit(void)
{
	jpeg_kdrv_close();

	nvt_jpg_proc_clear();
}

module_init(nvt_jpg_module_init);
module_exit(nvt_jpg_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("jpg driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(JPEG_VER_STR);

#endif

