#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/random.h>
#include <asm/signal.h>
#include <linux/clk.h>

#include "kdrv_audioio/kdrv_audioio.h"
#include "kdrv_audioio_main.h"
//#include "kdrv_audio_dai2.h"
//#include "kdrv_audioio_drv_dai2.h"
//#include "kdrv_audioio_main_dai2.h"

#include "kdrv_audioio_module_parm.h"
#include "kdrv_audioio_dbg.h"
#include <nvt_api_ver.h>

//#include "kdrv_audioio_int.h"
//#include "audio_obj_dai2.h"
#include "pll_protected.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int kdrv_audioio_debug_level = NVT_DBG_ERR ;// NVT_DBG_FUNC;NVT_DBG_ERR
module_param_named(kdrv_audioio_debug_level, kdrv_audioio_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(kdrv_audioio_debug_level, "Debug message level");
#endif

/*
static char* dai_mclk_name[PLATFORM_MAX_CHIP_CNT][2] = {
    {"dai_mclk",     "dai2_mclk" },
};*/

extern void kdrv_audio_init(void);
extern void kdrv_audio_exit(void);

extern NVT_API_CHK_DECLARE(kdrv_audio);
//=============================================================================
// Global variable
//=============================================================================

//=============================================================================
// function declaration
//=============================================================================
extern int  au_proc_init(void);
extern void  au_proc_remove(void);
int __init nvt_kdrv_audioio_module_init(void);
void __exit nvt_kdrv_audioio_module_exit(void);

//=============================================================================
// function define
//=============================================================================

static struct platform_driver nvt_kdrv_audioio_driver = {
	.driver = {
		.name   = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};
#if 1
// Parsing dtsi

void kdrv_audio_mclk_set(int chip_id, int id, unsigned long clkrate)
{
#if 0
	struct clk *dai_mclk;
	//printk("chip_id= %d, audio_i2s_mclk[%d] = %d \r\n",chip_id,id, audio_i2s_mclk[id+PLATFORM_MAX_SSP_CNT*chip_id]);

#ifdef __KERNEL__
	//struct clk *src_clk = clk_get(NULL, "pll12");

	// set mclk div
	dai_mclk = clk_get(NULL, dai_mclk_name[chip_id][id]);
	if (IS_ERR(dai_mclk)) {
		printk("failed to get dai i2s mclk\n");
	}

	//clk_set_parent(dai_mclk , src_clk);
	//clk_put(src_clk);

#endif

	// set DAI I2S Clock select
	clk_set_rate(dai_mclk, audio_i2s_mclk[id + PLATFORM_MAX_SSP_CNT * chip_id]);
	clk_put(dai_mclk);
#endif

}

void audio_dtsi_parsing(void)
{
	struct device_node *node, *mnode;
	int *value, *mclk_value;
	int i, chip_cnt, chip_cnt_max, para_len = 10, dtsi_num = 0;

	//char* audio_dtsi_vector[PLATFORM_MAX_CHIP_CNT*2]= {
	char *audio_dtsi_vector[2] = {
		"audio_i2s_part",
		"audio_mclk_part"
	};
	value = (int *)kmalloc(sizeof(int) * para_len * PLATFORM_MAX_SSP_CNT, GFP_KERNEL);

	if (value == NULL) {
		printk(" %s Fail to allocate value for dtsi parsing \n", __func__);
		return;
	}

	mclk_value = (int *)kmalloc(sizeof(int) * 2 * PLATFORM_MAX_SSP_CNT, GFP_KERNEL);

	if (mclk_value == NULL) {
		printk(" %s Fail to allocate mclk_value for dtsi parsing \n", __func__);
		kfree(value);
		return;
	}

	//chip_cnt_max = PLATFORM_MAX_CHIP_CNT;
	chip_cnt_max = 2;

	//for(chip_cnt = 0; chip_cnt < chip_cnt_max; chip_cnt++){
	for (chip_cnt = 0; chip_cnt < 1; chip_cnt++) {

		node = of_find_node_by_name(NULL, audio_dtsi_vector[chip_cnt]);
		if (node == NULL) {
			printk("[%s] doesn't exist in audio dts! \n", audio_dtsi_vector[chip_cnt]);
		} else {

			dtsi_num = of_property_count_u32_elems(node, "i2s") ;
			DBG_FUNC("dtsi_num = %d \n", dtsi_num);

			if (of_property_read_u32_array(node, "i2s", &value[0], dtsi_num) == 0) {
				para_len = dtsi_num / PLATFORM_MAX_SSP_CNT;
				for (i = 0 ; i < PLATFORM_MAX_SSP_CNT ; i++) {
					//audio_ssp_num[i]  = value[para_len*i];
					/*<i2s_num>, <enable>, <channel >, <data_stereo>, <sample_size>, <sample_rate>, <bit_clk >, <i2s_master>*/
					audio_i2s_num[i + PLATFORM_MAX_SSP_CNT * chip_cnt]    = value[para_len * i];
					audio_enable[i + PLATFORM_MAX_SSP_CNT * chip_cnt]     = value[para_len * i + 1];
					audio_tdm_chan[i + PLATFORM_MAX_SSP_CNT * chip_cnt]   = value[para_len * i + 2];
					audio_is_stereo[i + PLATFORM_MAX_SSP_CNT * chip_cnt]  = value[para_len * i + 3];
					audio_sample_size[i + PLATFORM_MAX_SSP_CNT * chip_cnt] = value[para_len * i + 4];
					audio_sample_rate[i + PLATFORM_MAX_SSP_CNT * chip_cnt] = value[para_len * i + 5];
					audio_bit_clock[i + PLATFORM_MAX_SSP_CNT * chip_cnt]  = value[para_len * i + 6];
					audio_i2s_master[i + PLATFORM_MAX_SSP_CNT * chip_cnt] = value[para_len * i + 7];

					// HDMI i2s data order always 1
#if 0
					if (i % PLATFORM_MAX_SSP_CNT == AUD_DAI_ID_HDMI) {
						audio_i2s_dataorder_type[i + PLATFORM_MAX_SSP_CNT * chip_cnt] = 1;
					}
#endif

					if (para_len > 8) {
						audio_i2s_dataorder_type[i + PLATFORM_MAX_SSP_CNT * chip_cnt] = value[para_len * i + 8];
#if 0
						if ((i % PLATFORM_MAX_SSP_CNT == AUD_DAI_ID_HDMI) && (value[para_len * i + 8] == 0)) {
							audio_i2s_dataorder_type[i + PLATFORM_MAX_SSP_CNT * chip_cnt] = 1;
							printk("force HDMI data order to type 1 \r\n");
						}
#endif
					}
					if (para_len > 9) {
						audio_source[i + PLATFORM_MAX_SSP_CNT * chip_cnt] = value[para_len * i + 9];
					}
#if 0
					printk("i2s_num 	%d = <%d>\n", i, value[para_len * i]);
					printk("enable  	%d = <%d>\n", i, value[para_len * i + 1]);
					printk("channel 	%d = <%d>\n", i, value[para_len * i + 2]);
					printk("data_stereo %d = <%d>\n", i, value[para_len * i + 3]);
					printk("sample_size %d = <%d>\n", i, value[para_len * i + 4]);
					printk("sample_rate %d = <%d>\n", i, value[para_len * i + 5]);
					printk("bit_clk 	%d = <%d>\n", i, value[para_len * i + 6]);
					printk("i2s_master 	%d = <%d>\n", i, value[para_len * i + 7]);
					printk("i2s_dataorder 	%d = <%d>\n", i, value[para_len * i + 8]);
					printk("audio_source value 	%d = <%d>\n", i, value[para_len * i + 9]);
					printk("audio_source 	%d = <%d>\n", i, audio_source[i + PLATFORM_MAX_SSP_CNT * chip_cnt]);
					printk("\r\n");
#endif
				}

				// mclk //
				mnode = of_find_node_by_name(NULL, audio_dtsi_vector[chip_cnt + 1]);

				if (mnode == NULL) {
					printk("[%s] doesn't exist in audio dts! \n", audio_dtsi_vector[chip_cnt + 1]);
				} else if (of_property_read_u32_array(mnode, "mclk", &mclk_value[0], of_property_count_u32_elems(node, "mclk")) == 0) {
					para_len = 2;

					//for(i=0;i< PLATFORM_MAX_SSP_CNT ;i++){
					for (i = 0; i < PLATFORM_MAX_SSP_CNT - 1 ; i++) {
						audio_i2s_mclk[i + PLATFORM_MAX_SSP_CNT * chip_cnt] = mclk_value[para_len * i + 1];
#if 0
						printk("i2s_num 	%d = <%d>\n", i, mclk_value[para_len * i]);
						printk("i2s_mclk 	%d = <%d>\n", i, mclk_value[para_len * i + 1]);
						printk("\r\n");
#endif
						kdrv_audio_mclk_set(chip_cnt, i, mclk_value[para_len * i + 1]);
					}
				}

			} else {
				printk("[i2s] can't be found in the [%s] node, parameter parsing fail\n", audio_dtsi_vector[chip_cnt]);
			}

			DBG_FUNC("[%s] audio dtsi parsing done! \n", audio_dtsi_vector[chip_cnt]);
		}
	}
	kfree(mclk_value);
	kfree(value);

	printk("kdrv_audio init : version %s\n", VERSION);
}
#endif

int __init nvt_kdrv_audioio_module_init(void)
{
	int ret;

	if (NVT_API_CHK_CALL(kdrv_audio) != TRUE) {
		DBG_ERR("objver mismatch %s\r\n", "kdrv_audioio");
		return -1;
	}

	nvt_dbg(IND, " nvt_kdrv_audioio_module_init  \r\n");
	nvt_dbg(WRN, "\n");
	ret = platform_driver_register(&nvt_kdrv_audioio_driver);

	audio_dtsi_parsing();

	kdrv_audio_init();

	au_proc_init();


	return 0;
}

void __exit nvt_kdrv_audioio_module_exit(void)
{
	au_proc_remove();
	nvt_dbg(WRN, "\n");
	platform_driver_unregister(&nvt_kdrv_audioio_driver);
	kdrv_audio_exit();
}

module_init(nvt_kdrv_audioio_module_init);
module_exit(nvt_kdrv_audioio_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("kdrv_audioio driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);

