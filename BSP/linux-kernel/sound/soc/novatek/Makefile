ifneq ($(CONFIG_SND_SOC_NVT),)
ifneq ($(CONFIG_NVT_IVOT_PLAT_NS02201),)
snd-soc-nvt-ext-objs	:= ns02201_soc_ext.o eac.o
snd-soc-nvt-ext1-objs	:= ns02201_soc_ext1.o
snd-soc-nvt-ext2-objs	:= ns02201_soc_ext2.o
snd-soc-nvt-ext3-objs	:= ns02201_soc_ext3.o
snd-soc-nvt-plat-objs	:= ns02201_soc_plat.o dai.o eac.o
snd-soc-nvt-plat-ext1-objs	:= ns02201_soc_plat_ext1.o dai.o
snd-soc-nvt-plat-ext2-objs	:= ns02201_soc_plat_ext2.o dai_multi.o
snd-soc-nvt-plat-ext3-objs	:= ns02201_soc_plat_ext3.o dai_multi.o
endif
endif
snd-soc-nvt-objs    := nvt_soc_machine.o

obj-$(CONFIG_SND_SOC_NVT) += snd-soc-nvt.o snd-soc-nvt-plat.o snd-soc-nvt-plat-ext1.o snd-soc-nvt-plat-ext2.o snd-soc-nvt-plat-ext3.o ns02201_soc_ext.o ns02201_soc_ext1.o ns02201_soc_ext2.o ns02201_soc_ext3.o