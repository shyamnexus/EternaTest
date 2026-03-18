#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <kwrap/verinfo.h>
#include <kwrap/type.h>

extern int nvt_ddr_arb_module_init(void);
extern void nvt_ddr_arb_module_exit(void);
extern int nvt_timer_module_init(void);
extern void nvt_timer_module_exit(void);
extern int nvt_pwm_module_init(void);
extern void nvt_pwm_module_exit(void);
extern int nvt_hwcopy_module_init(void);
extern void nvt_hwcopy_module_exit(void);
extern int nvt_sif_init(void);
extern void nvt_sif_exit(void);
extern int log_init(void);
extern void log_clearnup(void);
extern int dma_util_init(void);
extern void dma_util_exit(void);
extern int nvt_drvdump_init(void);
extern void nvt_drvdump_exit(void);


int __init kdrv_comm_init(void)
{
	int ret;

	if ((ret=nvt_pwm_module_init()) !=0 ) {
		return ret;
	}

	if ((ret=nvt_ddr_arb_module_init()) !=0 ) {
		return ret;
	}

	if ((ret=nvt_timer_module_init()) !=0 ) {
		return ret;
	}
	if ((ret=log_init()) !=0 ) {
		return ret;
	}

#if !defined(CONFIG_NVT_FPGA_EMULATION) && !defined(_NVT_FPGA_)
	if ((ret=dma_util_init()) !=0 ) {
		return ret;
	}
#endif

	if ((ret=nvt_sif_init()) !=0 ) {
		return ret;
	}

	if ((ret=nvt_drvdump_init()) !=0 ) {
		return ret;
	}

	return 0;
}

void __exit kdrv_comm_exit(void)
{
	nvt_sif_exit();
	dma_util_exit();
	log_clearnup();
	nvt_timer_module_exit();
	nvt_ddr_arb_module_exit();
	nvt_pwm_module_exit();
	nvt_drvdump_exit();
}


module_init(kdrv_comm_init);
module_exit(kdrv_comm_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("kdrv_comm driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.00.00");

#else
#endif
