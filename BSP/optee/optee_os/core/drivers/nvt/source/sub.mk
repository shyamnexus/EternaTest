incdirs-y += include

ifeq ($(CFG_NVT_CRYPTO),y)
srcs-y += crypto/crypto_platform.c
srcs-y += crypto/crypto.c
endif

ifeq ($(CFG_NVT_HASH),y)
srcs-y += hash/hash.c
srcs-y += hash/hash_platform.c
endif

ifeq ($(CFG_NVT_ECDSA),y)
srcs-y += ecdsa/ecdsa_platform.c
srcs-y += ecdsa/ecdsa.c
endif

ifeq ($(CFG_NVT_RSA),y)
srcs-y += rsa/rsa.c
srcs-y += rsa/rsa_platform.c
endif
ifeq ($(PLATFORM_FLAVOR),na51102)
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc_na51102.c
srcs-$(CFG_NVT_OPT) += nvt_otp.c
else ifeq ($(PLATFORM_FLAVOR),ns02302)
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc_ns02302.c
srcs-$(CFG_NVT_OPT) += nvt_otp_ns02302.c
else ifeq ($(PLATFORM_FLAVOR),ns02301)
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc_ns02301.c
srcs-$(CFG_NVT_OPT) += nvt_otp_ns02301.c
else ifeq ($(PLATFORM_FLAVOR),na51055)
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc_na51089.c
srcs-$(CFG_NVT_OPT) += nvt_otp_na51055.c
else ifeq ($(PLATFORM_FLAVOR),na51089)
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc_na51089.c
srcs-$(CFG_NVT_OPT) += nvt_otp.c
else ifeq ($(PLATFORM_FLAVOR),na51090)
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc_na51090.c
srcs-$(CFG_NVT_OPT) += nvt_otp_na51090.c
else ifeq ($(PLATFORM_FLAVOR),na51103)
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc_na51103.c
srcs-$(CFG_NVT_OPT) += nvt_otp.c
else ifeq ($(PLATFORM_FLAVOR),ns02401)
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc_ns02401.c
srcs-$(CFG_NVT_OPT) += nvt_otp_ns02401.c
else
srcs-$(CFG_NVT_TZASC) += nvt_asc/nvt_tzasc.c
srcs-$(CFG_NVT_OPT) += nvt_otp.c
endif
srcs-$(CFG_NVT_TZPC) += nvt_tzpc.c

ifeq ($(PLATFORM_FLAVOR),na51090)
srcs-y += pll/nvt_clk_na51090.c
else
srcs-y += pll/nvt_clk.c
endif

srcs-y += nvt_sramctl.c

ifeq ($(CFG_WITH_SOFTWARE_PRNG),y)
else
srcs-y += nvt_trng.c
endif
