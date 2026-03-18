/**
    NVT ALSA SoC Codec Driver
    @file na51102_soc_embd_r.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2023. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/


#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>

#include <sound/soc.h>
#include <sound/tlv.h>

#include "nvt_alsa.h"


#define DBG_WRN(fmt, args...) 	//printk(fmt, ##args)
#define ENABLE					1
#define DISABLE					0


/*				DECLARE_TLV_DB_SCALE(name,  min,step, mute) */

static struct snd_kcontrol_new nvt_ext_controls[] = {
//SOC_DOUBLE_TLV("Lineout Playback Volume",EAC_SPK_OUT_VOL, 0, 8, 255, 0, spk_tlv),
};

static int nvt_ext_probe(struct snd_soc_component *codec)
{
	int status;
	status = snd_soc_add_component_controls(codec, nvt_ext_controls,ARRAY_SIZE(nvt_ext_controls));

	DBG_WRN("nvt_ext_probe\n");
	if (status < 0) {
		DBG_WRN("%s: failed to add nvt_ext_controls (%d).\n",__func__, status);
		return status;
	}
	return 0;
}

static void nvt_ext_remove(struct snd_soc_component *codec)
{
	//struct adau1701 *adau1701 = snd_soc_codec_get_drvdata(codec);
	DBG_WRN("nvt_ext_remove\n");
	return;
}

static int nvt_ext_set_bias_level(struct snd_soc_component *codec, enum snd_soc_bias_level level)
{
	//struct adau1701 *adau1701 = snd_soc_codec_get_drvdata(codec);

	DBG_WRN("nvt_ext_set_bias_level %d\n",(int)level);
	switch (level) {
	case SND_SOC_BIAS_ON:
		break;
	case SND_SOC_BIAS_PREPARE:
		break;
	case SND_SOC_BIAS_STANDBY:
		break;
	case SND_SOC_BIAS_OFF:
		break;
	}
	return 0;
}


static int nvt_ext_read_reg(void *context, unsigned int reg,
				 unsigned int *value)
{

	//DBG_WRN("nvt_ext_read_reg\n");
	return 0;
}

/* Write to a register in the audio-bank of AB8500 */
static int nvt_ext_write_reg(void *context, unsigned int reg,
				  unsigned int value)
{
	return 0;
}

static const struct regmap_config nvt_ext_regmap = {
	.reg_bits = 64,
	.val_bits = 32,
	.reg_read = nvt_ext_read_reg,
	.reg_write = nvt_ext_write_reg,
};


//! codec
static struct snd_soc_component_driver nvt_ext_codec = {
	.probe			= nvt_ext_probe,
	.remove			= nvt_ext_remove,
	//.resume		= adau1701_resume,
	//.suspend		= adau1701_suspend,
	.set_bias_level		= nvt_ext_set_bias_level,
	.idle_bias_on		= false,


	/*
	.component_driver = {
		.controls		= adau1701_controls,
		.num_controls		= ARRAY_SIZE(adau1701_controls),
		.dapm_widgets		= adau1701_dapm_widgets,
		.num_dapm_widgets	= ARRAY_SIZE(adau1701_dapm_widgets),
		.dapm_routes		= adau1701_dapm_routes,
		.num_dapm_routes	= ARRAY_SIZE(adau1701_dapm_routes),
	},
	*/

	//.set_sysclk		= adau1701_set_sysclk,
	//.set_pll
};




static int nvt_ext_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	//struct adau1373 *adau1373 = snd_soc_codec_get_drvdata(codec);
	DBG_WRN("nvt_ext_hw_params\n");
	return 0;
}


static int nvt_ext_prepare(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	//struct snd_pcm_runtime *runtime = substream->runtime;
	//struct audio_substream_data *rtd = runtime->private_data;

	DBG_WRN("nvt_embd_prepare\r\n");

	//DBG_WRN("rt2 format=%d rate=%d ch=%d frmbits=%d\r\n", (int)runtime->format, (int)runtime->rate, (int)runtime->channels, (int)runtime->frame_bits);
	//DBG_WRN("rt2 addr=0x%08X bufsz=%d-frames prdsz=%d-frames\r\n", (int)runtime->dma_addr, (int)runtime->buffer_size, (int)runtime->period_size);

	/*if (runtime->channels == 2) {
		iowrite32(ioread32((volatile void __iomem *)(_EAC_REG_BASE_ADDR[0]+0xA4))|0x4,  (volatile void __iomem *)(_EAC_REG_BASE_ADDR[0]+0xA4));
	} else if (runtime->channels == 1) {
		iowrite32(ioread32((volatile void __iomem *)(_EAC_REG_BASE_ADDR[0]+0xA4)) & ~0x4,  (volatile void __iomem *)(_EAC_REG_BASE_ADDR[0]+0xA4));
	}*/

	return 0;
}


static const struct snd_soc_dai_ops nvt_ext_dai_ops = {
	.hw_params		= nvt_ext_hw_params,
	.prepare        = nvt_ext_prepare,
	//.set_sysclk	= adau1373_set_dai_sysclk,
	//.set_fmt		= adau1373_set_dai_fmt,
	//.set_bclk_ratio
};


static struct snd_soc_dai_driver nvt_ext_codec_dais[] = {

	{
		.name = "nvt,ext2_codec_dai",
		.id = 0,

		.playback = {
			.stream_name 	= "lineout",
			.channels_min 	= 1,
			.channels_max 	= 2,
			.rates 			= SNDRV_PCM_RATE_8000_48000,
			.rate_min 		= 8000,
			.rate_max 		= 48000,
			.formats 		= SNDRV_PCM_FMTBIT_S32_LE | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_U8,
		},

		.capture = {
			.stream_name 	= "mic",
			.channels_min 	= 1,
			.channels_max 	= 4,
			.rates 			= SNDRV_PCM_RATE_8000_48000,
			.rate_min 		= 8000,
			.rate_max 		= 48000,
			.formats 		= SNDRV_PCM_FMTBIT_S32_LE | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_U8,
		},
		.ops = &nvt_ext_dai_ops,
	},
};


static int nvt_ext_codec_device_probe(struct platform_device *pdev)
{

	return snd_soc_register_component(&pdev->dev, &nvt_ext_codec, nvt_ext_codec_dais, ARRAY_SIZE(nvt_ext_codec_dais));
}

static int nvt_ext_codec_device_remove (struct platform_device *pdev)
{
	DBG_WRN( "nvt_ext2_codec_device_remove\n");
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}


static struct platform_driver nvt_ext_codec_driver = {
	.driver		= {
		.name		= "nvt,ext2_codec",
		.owner		= THIS_MODULE,
	},
	.probe		= nvt_ext_codec_device_probe,
	.remove		= nvt_ext_codec_device_remove,
};


static struct platform_device *nvt_pdev;


int __init nvt_alsa_ext_module_init(void)
{
	int iRet;
	struct platform_device *pdev;

	DBG_WRN("nvt_alsa_ext2_module_init\r\n");
	iRet = platform_driver_register(&nvt_ext_codec_driver);
	if (iRet) {
		printk("nvt_alsa_ext2_module_init platform_driver_register error\n");
	}

	pdev = platform_device_alloc("nvt,ext2_codec",-1);
	if (!pdev)
	{
		printk("platform_device_alloc failed!!\n");
	}

	iRet = platform_device_add(pdev);
	if (iRet)
	{
		platform_device_put(pdev);
		printk( "platform_device_add failed(%d)! \n", iRet);
	}
	nvt_pdev = pdev;
	return 0;
}

void __exit nvt_alsa_ext_module_exit(void)
{
	DBG_WRN("nvt_alsa_ext2_module_exit\r\n");
	platform_device_unregister(nvt_pdev);
	platform_driver_unregister(&nvt_ext_codec_driver);
}

module_init(nvt_alsa_ext_module_init);
module_exit(nvt_alsa_ext_module_exit);


//module_platform_driver(nvt_dma_driver);

MODULE_AUTHOR("klins_chen@novatek.com.tw");
MODULE_DESCRIPTION("NOVATEK ALSA ext2 Codec Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:snd-soc-nvt-ext2");
