global-incdirs-y += .
global-incdirs-y += ./vos/drivers/include ./vos/drivers/source/include

srcs-y += main.c
srcs-y += nvt_headinfo.c
srcs-y += plat_init.S
srcs-y += smc/source/nvt_smc.c
srcs-y += smc/source/smc_op.c
srcs-y += smc/source/custom_smc.c
srcs-y += smc/source/nvt_smc_util.c
ifeq ($(CFG_NVT_OPT),y)
srcs-y += smc/source/nvt_efuse_smc.c
endif
srcs-y += vos/drivers/source/rtos_os_cpu.c
srcs-y += vos/drivers/source/rtos_os_flag.c
srcs-y += vos/drivers/source/rtos_os_sem.c

ifeq ($(CFG_NVT_CRYPTO),y)
srcs-y += smc/source/crypto/nvt_aes_smc.c
srcs-y += lib/source/crypto/nvt_ecb.c
endif

ifeq ($(CFG_NVT_HASH),y)
srcs-y += smc/source/crypto/nvt_sha_smc.c
srcs-y += lib/source/hash/nvt_hash.c
srcs-y += lib/source/hash/nvt_hmac.c
endif

ifeq ($(CFG_NVT_RSA),y)
srcs-y += smc/source/crypto/nvt_rsa_smc.c
srcs-y += lib/source/rsa/nvt_rsa.c
endif

ifeq ($(CFG_NVT_ECDSA),y)
srcs-y += smc/source/crypto/nvt_ecdsa_smc.c
endif
