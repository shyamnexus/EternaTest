/* NVT board configuration */
/* FLASH        FileSystem */
#if defined(CONFIG_NVT_SPI_NAND) || defined(CONFIG_NVT_SPI_NONE)
#define CONFIG_SYS_MAX_NAND_DEVICE              1
#define CONFIG_SYS_NAND_MAX_CHIPS               1
#endif

#ifdef CONFIG_NVT_FPGA_EMULATION
#define CONFIG_SYS_HZ_CLOCK			24000000
#else /* !CONFIG_NVT_FPGA_EMULATION */
#define CONFIG_SYS_HZ_CLOCK			12000000
#endif /* CONFIG_NVT_FPGA_EMULATION */


/*
 * Serial driver init.
 *
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE			-1
#define CONFIG_SYS_NS16550_COM1                         0x2f0110000
#define CONFIG_SYS_NS16550_COM2                         0x2f0111000
#define CONFIG_SYS_NS16550_COM3                         0x2f0112000
#define CONFIG_SYS_NS16550_COM4                         0x2f0113000
#define CONFIG_SYS_NS16550_COM5                         0x2f0114000
#ifdef CONFIG_NVT_FPGA_EMULATION
#define CONFIG_SYS_NS16550_CLK                          24000000        /* COM1: 24M, other COM port 48M */
#else
#define CONFIG_SYS_NS16550_CLK                          24000000        /* COM1: 24M, other COM port 48M */
#endif

/* Secure boot relative option */
#ifdef _ENCRYPT_ATF_ON_
#define CONFIG_ENCRYPT_ATF
#endif

#ifdef _ENCRYPT_DTS_ON_
#define CONFIG_ENCRYPT_DTS
#endif

#ifdef _ENCRYPT_UBOOT_ON_
#define CONFIG_ENCRYPT_UBOOT
#endif

#ifdef _ENCRYPT_KERNEL_ON_
#define CONFIG_ENCRYPT_KERNEL
#endif

#ifdef _ENCRYPT_RTOS_ON_
#define CONFIG_ENCRYPT_RTOS
#endif

#ifdef _ENCRYPT_OPTEE_ON_
#define CONFIG_ENCRYPT_OPTEE
#endif

#ifdef _ENCRYPT_RAMDISK_ON_
#define CONFIG_ENCRYPT_RAMDISK
#endif

#ifdef _ENCRYPT_ROOTFS_ON_
        #ifdef  NVT_LINUX_RAMDISK_SUPPORT
                #define CONFIG_ENCRYPT_ROOTFS
        #else
                #error need NVT_ROOTFS_TYPE_RAMDISK to enable CONFIG_ENCRYPT_ROOTFS
        #endif
#endif

#if defined(_SIGNATUREMETHOD_ECDSA_)
#define CONFIG_SIGNATUREMETHOD_ECDSA
#elif defined(SIGNATUREMETHOD_RSA)
#define CONFIG_SIGNATUREMETHOD_RSA
#endif

#ifdef _CHECK_SIGNATURE_BEFORE_BOOT_ON_
#define CONFIG_CHECK_SIGNATURE_BEFORE_BOOT
#endif

