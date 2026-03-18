#!/bin/bash
# global settings
KEEP_SENSOR_LIST=(sen_os02k10 sen_os04c10)
KEEP_LCD_LIST=(disp_if8b_lcd1_psd300_ili8961)
SAMPLE_INSTALL_LIST=(hd_video_record_with_fastboot rc_profile)

# install appfs
pushd ${NVT_HDAL_DIR}/samples/vendor_cfg
make install
popd

# install hd_video_record_with_fastboot sample
for n in "${SAMPLE_INSTALL_LIST[@]}"
do
	pushd ${NVT_HDAL_DIR}/samples/${n}
	make install
	mv ${ROOTFS_DIR}/rootfs/usr/bin/${n} ${ROOTFS_DIR}/rootfs/bin
	popd
done

# remove sensor ko and install sensor cfg to /usr/local/etc/
KEEP_SENSOR_DIR=${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/keep_sensores
SENSOR_CFG_DIR=${ROOTFS_DIR}/rootfs/usr/local/etc
mkdir -p ${SENSOR_CFG_DIR}
mkdir ${KEEP_SENSOR_DIR}
for n in "${KEEP_SENSOR_LIST[@]}"
do
	cp ${NVT_HDAL_DIR}/ext_devices/sensor/configs/cfg/${n}*.cfg ${SENSOR_CFG_DIR}
	mv ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/${n} ${KEEP_SENSOR_DIR}
done
rm -rf  ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/sen_*
mv ${KEEP_SENSOR_DIR}/* ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal
rmdir ${KEEP_SENSOR_DIR}

# remove unused lcd ko
KEEP_LCD_DIR=${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/display_panel/keep_lcds
mkdir ${KEEP_LCD_DIR}
for n in "${KEEP_LCD_LIST[@]}"
do
	mv ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/display_panel/${n} ${KEEP_LCD_DIR}
done
rm -rf  ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/display_panel/disp_*
mv ${KEEP_LCD_DIR}/* ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/display_panel
rmdir ${KEEP_LCD_DIR}

# remove unused misc
rm -rf ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/dummy
rm -rf ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/dummy2
rm -rf ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/hdal/comm/dummy
rm -rf ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/extra/msdcnvt
rm -rf ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/extra/sample
rm -rf ${ROOTFS_DIR}/rootfs/lib/modules/${NVT_LINUX_VER}/vos/dummy

# make plugin
if [ -d ${CONFIG_DIR}/cfg_gen/plugin ]; then
	SUBDIRS=$(dirname $(find ${CONFIG_DIR}/cfg_gen/plugin -name Makefile))
	for n in ${SUBDIRS}
	do
		pushd ${n};
		make;
		make install;
		make clean;
		popd;
	done
fi

# static device node
rm -rf ${ROOTFS_DIR}/rootfs/dev/*
if grep -wq "NVT_STATIC_DEVNODE_ON" ${NVT_PRJCFG_MODEL_CFG}; then
	echo -e "'\033[1;33m'NVT_STATIC_DEVNODE_ON'\033[0m'"
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/null c 1 3
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/mtdblock7 b 31 7
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/mtdblock8 b 31 8
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/nvt_vos c 255 1
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/log_vg c 255 2
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/nvtmpp c 255 3
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/isf_flow0 c 255 4
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/isf_flow1 c 255 5
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/isf_flow2 c 255 6
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/isf_flow3 c 255 7
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/isf_flow4 c 255 8
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/isf_flow5 c 255 9
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/isf_flow6 c 255 10
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/isf_flow7 c 255 11
	sudo mknod ${ROOTFS_DIR}/rootfs/dev/nvt_isp c 255 12
	sudo chown ${UID}:${UID} ${ROOTFS_DIR}/rootfs/dev/*
	sudo chmod 664 ${ROOTFS_DIR}/rootfs/dev/*
	sed -i 's/^tmpfs.*\/dev/#tmpfs\t\t\/dev/g' ${ROOTFS_DIR}/rootfs/etc/fstab
else
	echo -e "'\033[1;33m'NVT_STATIC_DEVNODE_OFF'\033[0m'"
	sed -i 's/^#tmpfs.*\/dev/tmpfs\t\t\/dev/g' ${ROOTFS_DIR}/rootfs/etc/fstab
fi

#dynamic dump dtsi. use rsync instead of cp, bcz of removing symbolic
if [ -d ${CONFIG_DIR}/cfg_gen/fastboot/fastboot ]; then
	rsync -avL --delete ${CONFIG_DIR}/cfg_gen/fastboot/fastboot ${ROOTFS_DIR}/rootfs/etc
fi