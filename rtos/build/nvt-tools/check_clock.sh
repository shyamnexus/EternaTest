#!/bin/sh
if [ -e clock_result.log ];then
	rm clock_result.log
fi
FILES="`grep -r " clk_set_rate" code/hdal/drivers | cut -d":" -f1 | grep \"\.c\"`"
touch clock_result.log
for i in $FILES
do
	if [ "code/hdal/drivers/k_driver/source/kdrv_builtin/audcap_builtin_platform.c" == "$i" ]; then
		continue
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_builtin_539a/audcap_builtin_platform.c" == "$i" ]; then
		continue
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_builtin/sie/eng/sie_eng_platform.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/kdrv_builtin/sie/fastboot/sie_init_main.c"
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_builtin_539a/sie/eng/sie_eng_platform.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/kdrv_builtin_539a/sie/fastboot/sie_init_main.c"
	fi

	if [ "code/hdal/drivers/k_driver/source/comm/gyro_spi/gyro_spi_drv.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/comm/gyro_spi/gyro_spi_main.c"
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_videoprocess/kdrv_ime/eng/ime_eng_int_platform.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/kdrv_videoprocess/kdrv_ipp/kdrv_ipp_platform.c"
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_videoprocess_539a/kdrv_ime/eng/ime_eng_int_platform.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/kdrv_videoprocess_539a/kdrv_ipp/kdrv_ipp_platform.c"
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_videoprocess/kdrv_ipe/eng/ipe_eng_platform.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/kdrv_videoprocess/kdrv_ipp/kdrv_ipp_platform.c"
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_videoprocess_539a/kdrv_ipe/eng/ipe_eng_platform.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/kdrv_videoprocess_539a/kdrv_ipp/kdrv_ipp_platform.c"
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_videoprocess/kdrv_ife/eng/ife_eng_platform.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/kdrv_videoprocess/kdrv_ipp/kdrv_ipp_platform.c"
	fi

	if [ "code/hdal/drivers/k_driver/source/kdrv_videoprocess_539a/kdrv_ife/eng/ife_eng_platform.c" == "$i" ]; then
		i="code/hdal/drivers/k_driver/source/kdrv_videoprocess_539a/kdrv_ipp/kdrv_ipp_platform.c"
	fi

	SET=`grep -r "of_property_read_u32(.*, \"current_rate\"" $i`
	if [ "$SET" == "" ];then
		if [ "`grep -r rtos $i`" != "" ]; then
			continue
		fi

		DUPLICATE="`grep -r $i clock_result.log`"
		if [ "$DUPLICATE" == "" ]; then
			echo "$i: current_rate not found!!!!, If using clk_set_rate, please check that \"of_property_read_u32(node, \"current_rate\", &current_rate)\" should be used in $i" >> clock_result.log
		fi
	fi
done
