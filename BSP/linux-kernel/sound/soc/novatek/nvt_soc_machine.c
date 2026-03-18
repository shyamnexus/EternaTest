/**
    NVT ALSA SoC Machine Driver
    @file nvt_soc_machine.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2023. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
//! Standard
#include <linux/init.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include "nvt_alsa.h"


/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define NVT_CARD_MAXNUM       1
#define NVT_PLAT_DEV_NUM      1


#define NVT_MACHINE_NAME     "nvt,alsa_mach"
#define NVT_PLATFORM_NAME    "nvt,alsa_plat"
#define NVT_CODEC_NAME       "nvt,ext_codec"
#define NVT_CODECDAI_NAME    "nvt,codec_dai"
#define NVT_CPUDAI_NAME0     "nvt,cpu_dai"

#define NVT_EXT1_PLATFORM_NAME    "nvt,ext1_alsa_plat"
#define NVT_EXT1_CODEC_NAME       "nvt,ext1_codec"
#define NVT_EXT1_CODECDAI_NAME    "nvt,ext1_codec_dai"
#define NVT_EXT1_CPUDAI_NAME0     "nvt,ext1_cpu_dai"

#define NVT_EXT2_PLATFORM_NAME    "nvt,ext2_alsa_plat"
#define NVT_EXT2_CODEC_NAME       "nvt,ext2_codec"
#define NVT_EXT2_CODECDAI_NAME    "nvt,ext2_codec_dai"
#define NVT_EXT2_CPUDAI_NAME0     "nvt,ext2_cpu_dai"

#define NVT_EXT3_PLATFORM_NAME    "nvt,ext3_alsa_plat"
#define NVT_EXT3_CODEC_NAME       "nvt,ext3_codec"
#define NVT_EXT3_CODECDAI_NAME    "nvt,ext3_codec_dai"
#define NVT_EXT3_CPUDAI_NAME0     "nvt,ext3_cpu_dai"


/*-----------------------------------------------------------------------------*/
/* ALSA Debug                                                                  */
/*-----------------------------------------------------------------------------*/
#define NVTALSA_PREFIX "[alsa]"

#define NVTALSA_TRACE( fmt,...)   //printk( NVTALSA_PREFIX "[%s]" fmt "\n",__FUNCTION__, ##__VA_ARGS__);
#define NVTALSA_ERR( fmt,...)     printk( NVTALSA_PREFIX "[%s:%d]" fmt , __FUNCTION__, __LINE__, ##__VA_ARGS__);


/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/

struct nvtalsa_platform_device
{
	const char		       *name;
	struct platform_device *pdev;
};

typedef struct _ST_FAKE_CARD
{
	struct snd_soc_card *soc_card;

} ST_FAKE_CARD;

/*-----------------------------------------------------------------------------*/
/* Local Function Prototype                                                    */
/*-----------------------------------------------------------------------------*/
static int nvt_machine_probe(struct platform_device *pdev);
static int nvt_machine_remove(struct platform_device *pdev);

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
static struct platform_driver nvt_machine_driver = {
    .driver = {
		.name	= NVT_MACHINE_NAME,
	    .owner  = THIS_MODULE,
    },

	.probe		= nvt_machine_probe,
	.remove		= nvt_machine_remove,

};

SND_SOC_DAILINK_DEFS(capture,
	DAILINK_COMP_ARRAY(COMP_CPU(NVT_CPUDAI_NAME0)),
	DAILINK_COMP_ARRAY(COMP_CODEC(NVT_CODEC_NAME, NVT_CODECDAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM(NVT_PLATFORM_NAME)));

SND_SOC_DAILINK_DEFS(capture_ext1,
	DAILINK_COMP_ARRAY(COMP_CPU(NVT_EXT1_CPUDAI_NAME0)),
	DAILINK_COMP_ARRAY(COMP_CODEC(NVT_EXT1_CODEC_NAME, NVT_EXT1_CODECDAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM(NVT_EXT1_PLATFORM_NAME)));

SND_SOC_DAILINK_DEFS(capture_ext2,
	DAILINK_COMP_ARRAY(COMP_CPU(NVT_EXT2_CPUDAI_NAME0)),
	DAILINK_COMP_ARRAY(COMP_CODEC(NVT_EXT2_CODEC_NAME, NVT_EXT2_CODECDAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM(NVT_EXT2_PLATFORM_NAME)));

SND_SOC_DAILINK_DEFS(capture_ext3,
	DAILINK_COMP_ARRAY(COMP_CPU(NVT_EXT3_CPUDAI_NAME0)),
	DAILINK_COMP_ARRAY(COMP_CODEC(NVT_EXT3_CODEC_NAME, NVT_EXT3_CODECDAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM(NVT_EXT3_PLATFORM_NAME)));

static struct snd_soc_dai_link fakecard_dailink[] = {
	{
		.name 			= "NT69xEXT",
		.stream_name    = "EXT",

		SND_SOC_DAILINK_REG(capture),

		#if NVT_DAI3_I2S
		.dai_fmt		= SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF
						| SND_SOC_DAIFMT_CBS_CFS,
		#endif
	},

	{
		.name 			= "NT69xEXT1",
		.stream_name    = "EXT1",

		SND_SOC_DAILINK_REG(capture_ext1),

		#if 1
		.dai_fmt		= SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF
						| SND_SOC_DAIFMT_CBS_CFS,
		#endif
	},

	{
		.name 			= "NT69xEXT2",
		.stream_name    = "EXT2",

		SND_SOC_DAILINK_REG(capture_ext2),

		#if 1
		.dai_fmt		= SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF
						| SND_SOC_DAIFMT_CBS_CFS,
		#endif
	},

	{
		.name 			= "NT69xEXT3",
		.stream_name    = "EXT3",

		SND_SOC_DAILINK_REG(capture_ext3),

		#if 1
		.dai_fmt		= SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF
						| SND_SOC_DAIFMT_CBS_CFS,
		#endif
	}

};

/* SoC card */
static struct snd_soc_card snd_soc_fakecard[NVT_CARD_MAXNUM] = {
	{
		.name 		= "nvt_card",
		.owner 		= THIS_MODULE,
		.dai_link 	= &fakecard_dailink[0],
		.num_links 	= 4,
	},
};

static struct nvtalsa_platform_device g_stNvtAudPlatformDevice[] =
{
	{.name = NVT_MACHINE_NAME,   .pdev = NULL,},
};


ST_FAKE_CARD g_stFakeCards[NVT_CARD_MAXNUM] =
{
	{.soc_card= NULL},
};

/*-----------------------------------------------------------------------------*/
/* Local Functions                                                             */
/*-----------------------------------------------------------------------------*/

//! machine
static int nvt_machine_probe(struct platform_device *pdev)
{
	NVTALSA_TRACE( "Called");
	return 0;
}

static int nvt_machine_remove(struct platform_device *pdev)
{
	NVTALSA_TRACE( "Called");
	return 0;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
#define __INTERFACE_FUNCTION__

int nvt_nvtalsa_card_init(int iCardIdx)
{
	int iRet = 0, iIdx = 0, iTargetCard = 0;

	NVTALSA_TRACE( "Called [Card%d]",iCardIdx);

	for (iTargetCard = 0; iTargetCard < NVT_CARD_MAXNUM; iTargetCard++)
	{
		if((g_stFakeCards[iTargetCard].soc_card != NULL))
		{
			iTargetCard = NVT_CARD_MAXNUM;
			break;
		}
		else if(g_stFakeCards[iTargetCard].soc_card == NULL)
		{
			g_stFakeCards[iTargetCard].soc_card = &snd_soc_fakecard[iTargetCard];
			//break;
		}

	}


	{
		iRet = platform_driver_register(&nvt_machine_driver);
		if (iRet)
		{
			NVTALSA_ERR("platform_driver_register failed(%d)!\n", iRet);
		}


		//! Allocate device
		for (iIdx = 0; iIdx < NVT_PLAT_DEV_NUM; iIdx++)
		{
			g_stNvtAudPlatformDevice[iIdx].pdev = platform_device_alloc(g_stNvtAudPlatformDevice[iIdx].name,-1);
			if (!g_stNvtAudPlatformDevice[iIdx].pdev)
			{
				NVTALSA_ERR( "platform_device_alloc failed!!\n");
				return -ENOMEM;
			}

			iRet = platform_device_add(g_stNvtAudPlatformDevice[iIdx].pdev);
			if (iRet)
			{
				platform_device_put(g_stNvtAudPlatformDevice[iIdx].pdev);
				NVTALSA_ERR( "platform_device_add failed(%d)! \n", iRet);
			}
		}
	}


	for (iTargetCard = 0; iTargetCard < NVT_CARD_MAXNUM; iTargetCard++)
	{
		//! use same mc fake device for fake card
		snd_soc_fakecard[iTargetCard].dev = &g_stNvtAudPlatformDevice[0].pdev->dev;
		iRet = snd_soc_register_card(&snd_soc_fakecard[iTargetCard]);
		if (iRet)
		{
			NVTALSA_ERR("snd_soc_register_card idx:%d failed(%d)!\n", iTargetCard, iRet);
		}

		NVTALSA_TRACE( "Init done [Card%d]",iCardIdx);
	}
	return iRet;

}

//! Remove all fake card
void nvt_nvtalsa_card_exit(void)
{
	int iRet = 0, iIdx = 0, iTargetCard = 0;

	NVTALSA_TRACE( "Called");

	for (iTargetCard = 0; iTargetCard < NVT_CARD_MAXNUM; iTargetCard++)
	{
		if(g_stFakeCards[iTargetCard].soc_card != NULL)
		{
			iRet = snd_soc_unregister_card(&snd_soc_fakecard[iTargetCard]);
			if (iRet)
			{
				NVTALSA_ERR("snd_soc_unregister_card failed(%d)!\n", iRet);
			}
			g_stFakeCards[iTargetCard].soc_card = NULL;
		}
	}

	//! release platform driver
	platform_driver_unregister(&nvt_machine_driver);

	for (iIdx = 0; iIdx < NVT_PLAT_DEV_NUM; iIdx++)
	{
		platform_device_unregister(g_stNvtAudPlatformDevice[iIdx].pdev);
	}
}

int __init _nvt_audio_alsa_init(void)
{
    nvt_nvtalsa_card_init(0);

    return 0;
}

void __exit _nvt_audio_alsa_exit(void)
{
	nvt_nvtalsa_card_exit();
}

module_init(_nvt_audio_alsa_init);
module_exit(_nvt_audio_alsa_exit);

MODULE_AUTHOR("NOVATEK");
MODULE_DESCRIPTION("NVT ASoC driver");
MODULE_LICENSE("GPL");
