# Copyright (C) 2015-2020 Novatek corp.
#
# SPDX-License-Identifier:	GPL-2.0+
#
KBUILD_CFLAGS := -fno-builtin -Werror -Iboard/$(BOARDDIR)/../common/include -fshort-wchar -Iinclude

obj-$(CONFIG_NVT_IVOT_SOC) += nvt_ivot_common.o
obj-$(CONFIG_NVT_IVOT_OPTEE_SUPPORT) += nvt_optee/nvt_optee_smc.o


obj-$(CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT) += nvt_ivot_fw_update_utils.o nvt_ivot_fw_update.o nvt_ivot_pack.o
obj-$(CONFIG_NVT_IVOT_SOC_UTILS) += nvt_ivot_soc_utils.o rtos/nvt_ivot_soc_utils_rtos.o
obj-$(CONFIG_SET_DFU_ALT_INFO) += nvt_dfu.o

obj-$(CONFIG_VIDEO_LOGO) += nvt_logo/jpg.o

obj-$(CONFIG_NVT_IVOT_DUAL_RTOS) += nvt_ivot_dual_rtos.o

obj-$(CONFIG_NVT_UTILS_CMD) += nvt_utils_cmd/nvt_debug_utils.o
obj-$(CONFIG_NVT_IVOT_OPTEE_SUPPORT) += nvt_utils_cmd/nvt_ivot_optee_utils.o
obj-$(CONFIG_NVT_IVOT_CRYPTO_CMD) += nvt_utils_cmd/nvt_ivot_crypto_cmd.o
obj-$(CONFIG_NVT_IVOT_HASH_CMD) += nvt_utils_cmd/nvt_ivot_hash_cmd.o
obj-$(CONFIG_NVT_IVOT_RSA_CMD) += nvt_utils_cmd/nvt_ivot_rsa_cmd.o
obj-$(CONFIG_NVT_IVOT_ECDSA_CMD) += nvt_utils_cmd/nvt_ivot_ecdsa_cmd.o
obj-$(CONFIG_NVT_IVOT_EFUSE_CMD) += nvt_utils_cmd/nvt_ivot_otp_cmd.o
obj-$(CONFIG_NVT_IVOT_TZPC_SUPPORT) += nvt_utils_cmd/tzpc/nvt_ivot_tzpc_utils.o	nvt_utils_cmd/tzpc/nvt_ivot_protected.o

ifeq ($(CONFIG_TARGET_NA51090_A64),y)
obj-$(CONFIG_VIDEO_LOGO) += nvt_utils_cmd/nvt_logo/cmd_bootlogo_hdmi.o
else ifeq ($(CONFIG_TARGET_NA51103)$(CONFIG_TARGET_NA51103_A64),y)
obj-$(CONFIG_VIDEO_LOGO) += nvt_utils_cmd/nvt_logo/cmd_bootlogo_hdmi.o
else
obj-$(CONFIG_VIDEO_LOGO) += nvt_utils_cmd/nvt_logo/cmd_bootlogo.o
endif

obj-$(CONFIG_NVT_PCIE_DAUGHTER_BRD) += nvt_pcie_boot_utils.o
obj-$(CONFIG_NVT_SWITCH_SECURE) += nvt_utils_cmd/nvt_switch_secure_cmd.o
