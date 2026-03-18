#include <linux/random.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "otp_proc.h"
#include "otp_dbg.h"
#include "otp_main.h"
#include "otp_drv.h"
#include "otp_platform.h"
#include <plat/efuse_protected.h>
#include <plat/hardware.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/scatterlist.h>
#include <crypto/skcipher.h>
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
#include <plat/top.h>
#endif
//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

#define DBG_TEST_EN (0)

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PMODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
POTP_DRV_INFO pdrv_otp_info_data;

//============================================================================
// Function define
//============================================================================
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
INT32 otp_set_next_pkg_uid_test(void);
void otp_next_pkg_uid_test_init(void);
#endif

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static int nvt_otp_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nUsage\n");
	seq_printf(sfile, "\necho [command] > efuse_op\n\n");
	seq_printf(sfile, "[command]    =>\n");
	seq_printf(sfile, "             => trim (driver's trim data)\n");
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NA51103)
	seq_printf(sfile, "             => trim dump(efuse trim vs register value compare)\n");
#endif
	seq_printf(sfile, "             => uniqueid (chip's unique ID)\n");
	seq_printf(sfile, "             => keyset No (key transfer to secure engine)\n");
	seq_printf(sfile, "             => nvt_write_key No (write specific key into specific key set field)\n");
	seq_printf(sfile, "             => nvt_read_key No ( read specific key from specific key set field)\n");
	seq_printf(sfile, "             => nvt_read_key_lock No (Read lock specific key set)\n");
	seq_printf(sfile, "             => encrypt_key_set No (Encrypt/decrypt via specific key set)\n");
	seq_printf(sfile, "             => version (knlPkg.a version)\n");
	return 0;
}

static int nvt_otp_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_otp_proc_show, NULL);
}

static ssize_t nvt_otp_proc_cmd_read(struct file *fp, char __user *ubuf, size_t cnt, loff_t *ppos)
{
	int ret = 0;
	int len;
	char kbuf[32];
	UINT32 h, l;
//	BOOL    avl = FALSE;

	//pr_info("nvt_check_available_proc_cmd_read cnt = [%d]\r\n", cnt);

	if (!*ppos) {
//		pr_info("Check linrary nane [%s]\r\n", library_name);
		//avl = efuse_check_available(library_name);
		h = 0;
		l = 0;
		if (efuse_get_unique_id(&l, &h) < 0) {
			pr_err("unique ID[0x%08x][0x%08x] error\r\n", (int)h, (int)l);
			h = 0;
			l = 0;
			sprintf(kbuf, "%08x%08x\n", h, l);
		} else {
			sprintf(kbuf, "%08x%08x\n", h, l);
		}

		len = strlen(kbuf);
		if (clear_user((void *)ubuf, cnt)) {
			printk(KERN_ERR "clear error\n");
			return -EIO;
		}
		//ret = simple_read_from_buffer((char __user *)ubuf,cnt,ppos,kbuf,strlen(kbuf));
		ret = copy_to_user(ubuf, kbuf, len);
//		pr_info("efuse_unid %s ubuf %s len = %d\r\n", kbuf, ubuf, len);
		*ppos += len;
		return len;

	}

	return ret;
}

/******************************************************************
 NA51068 efuse key section 640bit => 20 word
 =============================================================
 key#   offset  word[0]  word[1]  word[2]  word[3]
 =============================================================
 key#0  [ 0]    xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
 key#1  [ 4]    xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
 key#2  [ 8]    xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
 key#3  [12]    xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
 key#4  [16]    xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
*******************************************************************/
#define AES_ECB_KEY_SIZE    16
static UINT8 key_sample1[16] = {0x24, 0xaa, 0x0d, 0x9b, 0xf1, 0xae, 0x31, 0xb4, 0x28, 0x51, 0xe4, 0xc4, 0xd1, 0x71, 0x1d, 0x1e};
static UINT8 key_sample2[16] = {0x95, 0x4d, 0x81, 0xc5, 0x5a, 0xcc, 0x2b, 0xe5, 0xdd, 0xc8, 0x74, 0xc3, 0x9f, 0xaf, 0xcf, 0x5c};
static UINT8 key_sample3[16] = {0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, 0x12, 0x11, 0x10, 0x09, 0x16, 0x15, 0x14, 0x13};
static UINT8 key_sample_crypto[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x9, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};

//Here is specific key magic number to let linux crypto framework encrypt / decrypt via key manager
#define OTP_KEY_MANAGER_IDENTIFY_OFS            4
#define OTP_KEY_MANAGER_HDR_SIZE                8
const unsigned char OTP_KEY_MANAGER_HDR[OTP_KEY_MANAGER_HDR_SIZE] = {0x65, 0x6B, 0x65, 0x79, 0x00, 0x00, 0x00, 0x00}; ///< byte[0...3] as magic tag, byte[4] as key offset (word unit)


static ssize_t nvt_otp_proc_cmd_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

//	struct skcipher_def sk;
//    struct crypto_skcipher *skcipher = NULL;
//    struct skcipher_request *req = NULL;
//    char *scratchpad = NULL;
//    char *ivdata = NULL;
//  unsigned char key[16];
//  int ret = -EFAULT;



	/*check command length*/
	if ((!len) || (len > (MAX_CMD_LENGTH - 1))) {
		pr_err("Command length is too long or 0!\n");
		goto ERR_OUT;
	}

	/*copy command string from user space*/
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	printk("CMD:%s\n", cmd_line);

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (ucargc < 1) {
		pr_err("NULL command error\n");
		goto ERR_OUT;
	}

	if (!strcmp(argv[0], "trim")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51090)
		UINT16  data;
		if (efuse_readParamOps(EFUSE_USB2_TRIM_DATA, &data) != E_OK) {
			pr_err("[0] data = NULL\n");
		} else {
			pr_info("[0] data = 0x%08x\r\n", data);
		}
		if (efuse_readParamOps(EFUSE_USBC_TRIM_DATA, &data) != E_OK) {
			pr_err("[1] data = NULL\n");
		} else {
			pr_info("[1] data = 0x%08x\r\n", data);
		}
		if (efuse_readParamOps(EFUSE_DDR_H_TRIM_DATA, &data) != E_OK) {
			pr_err("[2] data = NULL\n");
		} else {
			pr_info("[2] data = 0x%08x\r\n", data);
		}

		if (efuse_readParamOps(EFUSE_VER_PKG_UID, &data) != E_OK) {
			pr_err("[4] data = NULL\n");
		} else {
			pr_info("[4] data = 0x%08x\r\n", data);
		}

		if (efuse_readParamOps(EFUSE_DDR_V_TRIM_DATA, &data) != E_OK) {
			pr_err("[3] data = NULL\n");
		} else {
			pr_info("[3] data = 0x%08x\r\n", data);
		}

		if (efuse_readParamOps(EFUSE_HDMI_TRIM_DATA, &data) != E_OK) {
			pr_err("[5-1] data = NULL\n");
		} else {
			pr_info("[5-1] data = 0x%08x\r\n", data);
		}

		if (efuse_readParamOps(EFUSE_PCIE_REFCLK_TRIM_DATA, &data) != E_OK) {
			pr_err("[5-2] data = NULL\n");
		} else {
			pr_info("[5-2] data = 0x%08x\r\n", data);
		}

		if (efuse_readParamOps(EFUSE_THERMAL_TRIM_DATA, &data) != E_OK) {
			pr_err("[6] data = NULL\n");
		} else {
			pr_info("[6] data = 0x%08x\r\n", data);
		}

		if (efuse_readParamOps(EFUSE_COREPOWER_LEAKAGE_TRIM_DATA, &data) != E_OK) {
			pr_err("[7] data = NULL\n");
		} else {
			pr_info("[7] data = 0x%08x\r\n", data);
		}

		if (efuse_readParamOps(EFUSE_CVBS_VGA_TRIM_DATA, &data) != E_OK) {
			pr_err("[9] data = NULL\n");
		} else {
			pr_info("[9] data = 0x%08x\r\n", data);
		}
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51102)
		UINT16  value;
		UINT32  RTC_Trim_15_8, ANA_REG_3_0, ANA_REG_7_6;

		UINT32  HDMI_resistor_1_4, HDMI_LANE_VL_1_4;
		UINT32  HDMI_resistor_2_0, HDMI_LANE_VL_2_0;

		UINT32  USB_SATA_Internal_12K_resistor;
		UINT32  USB_SATA_TX_TRIM, USB_SATA_RX_TRIM;
		UINT32  USB_RX_LDO, USB_PLL_LDO;

		UINT32  RZQ_DATA, RZQ_SIGN;
		UINT32  THERMAL;
		UINT32  CORE_PWR_LKE;
		UINT32  CORE_PWR_ADJ;
		UINT32  I_Trim_2_5_G, R_Term_2_5_G;
		UINT32  clock_lane_trim[4];
		UINT32  data_lane_trim[8];

		if (efuse_readParamOps(EFUSE_RTC_TRIM_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[%d] data = NULL\n", EFUSE_RTC_TRIM_DATA);
		} else {
			pr_info("[0]RTC = 0x%04x\r\n", (int)value);
			//auto_msg(("  12K_resistor[4..0] valid from 1(0x1) ~ 20(0x14)\r\n"));
			//bit[7..0]
			RTC_Trim_15_8 = (value & 0xFF);
			//bit[9..8]
			ANA_REG_7_6   = ((value >> 8) & 0x3);
			//bit[13..10]
			ANA_REG_3_0   = ((value >> 10) & 0xF);
			pr_info("   *RTC_3_0 Trim [0x%04x]\r\n", (int)ANA_REG_3_0);
			pr_info("   *RTC_7_6 Trim [0x%04x]\r\n", (int)ANA_REG_7_6);
			pr_info("  *RTC_15_8 Trim [0x%04x]\r\n", (int)RTC_Trim_15_8);
		}

		if (efuse_readParamOps(EFUSE_HDMI_RESISTOR_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_HDMI_RESISTOR_TRIM_DATA);
		} else {
			pr_info("  HDMI resistor Trim = 0x%04x\r\n", (int)value);
			//bit[4..0]
			HDMI_resistor_1_4 = value & 0x1F;
			//bit[9..5]
			HDMI_resistor_2_0 = ((value >> 5) & 0x1F);

			if (HDMI_resistor_1_4 > 0x14) {
				pr_info("1.4 HDMI resistor Trim data error 0x%04x > 0x14\r\n", (int)HDMI_resistor_1_4);
			} else if (HDMI_resistor_1_4 < 0x1) {
				pr_info("1.4 HDMI resistor Trim data error 0x%04x < 0x1\r\n", (int)HDMI_resistor_1_4);
			} else {
				pr_info("  *1.4 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_resistor_1_4);
			}

			if (HDMI_resistor_2_0 > 0x14) {
				pr_info("2.0 HDMI resistor Trim data error 0x%04x > 0x14\r\n", (int)HDMI_resistor_2_0);
			} else if (HDMI_resistor_2_0 < 0x1) {
				pr_info("2.0 HDMI resistor Trim data error 0x%04x < 0x1\r\n", (int)HDMI_resistor_2_0);
			} else {
				pr_info("  *2.0 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_resistor_2_0);
			}
		}

		if (efuse_readParamOps(EFUSE_HDMI_2_0_VLANE_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_HDMI_2_0_VLANE_TRIM_DATA);
		} else {
			pr_info("  HDMI 2.0 VLane tunning Trim = 0x%04x\r\n", (int)value);
			//bit[11..0]
			HDMI_LANE_VL_2_0 = value & 0xFFF;
			pr_info("  *2.0 HDMI VLane Trim data range success 0x0 <= [0x%04x] <= 0xFFF\r\n", (int)HDMI_LANE_VL_2_0);
		}

		if (efuse_readParamOps(EFUSE_HDMI_1_4_VLANE_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_HDMI_1_4_VLANE_TRIM_DATA);
		} else {
			pr_info("  HDMI 1.4 VLane tunning Trim = 0x%04x\r\n", (int)value);
			//bit[11..0]
			HDMI_LANE_VL_1_4 = value & 0xFFF;
			pr_info("  *1.4 HDMI VLane Trim data range success 0x0 <= [0x%04x] <= 0xFFF\r\n", (int)HDMI_LANE_VL_1_4);
		}

		if (efuse_readParamOps(EFUSE_USB_SATA_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_USB_SATA_TRIM_DATA);
		} else {
			//12K resistor 1<= x <= 20(0x14)
			pr_info("  USB(SATA) Trim data = 0x%04x\r\n", (int)value);
			USB_SATA_Internal_12K_resistor = value & 0x1F;
			USB_SATA_TX_TRIM = ((value >> 5) & 0xF);
			USB_SATA_RX_TRIM = ((value >> 9) & 0xF);

			if (USB_SATA_Internal_12K_resistor > 0x14) {
				pr_info("12K_resistor Trim data error 0x%04x > 20(0x14)\r\n", (int)USB_SATA_Internal_12K_resistor);
			} else if (USB_SATA_Internal_12K_resistor < 0x1) {
				pr_info("12K_resistor Trim data error 0x%04x < 1(0x1)\r\n", (int)USB_SATA_Internal_12K_resistor);
			} else {
				pr_info("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB_SATA_Internal_12K_resistor);
			}

			//TX term bit[8..5] 0x2 <= x <= 0xE
			if (USB_SATA_TX_TRIM > 0xE) {
				pr_info("USB(SATA) TX Trim data error 0x%04x > 0xE\r\n", (int)USB_SATA_TX_TRIM);
			} else if (USB_SATA_TX_TRIM < 0x2) {
				pr_info("USB(SATA) TX Trim data error 0x%04x < 0x2\r\n", (int)USB_SATA_TX_TRIM);
			} else {
				pr_info("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB_SATA_TX_TRIM);
			}
			//RX term bit[12..9] 0x8 <= x <= 0xE
			if (USB_SATA_RX_TRIM > 0xE) {
				pr_info("USB(SATA) RX Trim data error 0x%04x > 0xE\r\n", (int)USB_SATA_RX_TRIM);
			} else if (USB_SATA_RX_TRIM < 0x8) {
				pr_info("USB(SATA) RX Trim data error 0x%04x < 0x8\r\n", (int)USB_SATA_RX_TRIM);
			} else {
				pr_info("  *USB(SATA) RX Trim data 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB_SATA_RX_TRIM);
			}
		}

		if (efuse_readParamOps(EFUSE_USB_LDO_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_USB_LDO_TRIM_DATA);
		} else {
			pr_info("  USB(RX/PLL LDO) Trim data = 0x%04x\r\n", (int)value);

			USB_PLL_LDO = (value & 0x1);
			USB_RX_LDO = ((value >> 1) & 0x1);

			//RX term bit[12..9] 0x8 <= x <= 0xE
			pr_info("USB PLL LDO[%d]\r\n", (int)USB_PLL_LDO);
			pr_info("USB  RX LDO[%d]\r\n", (int)USB_RX_LDO);

		}

		if (efuse_readParamOps(EFUSE_DDRP_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_DDRP_TRIM_DATA);
		} else {
			pr_info("\r\n");
			pr_info(" DDR ZQ Trim = 0x%04x\r\n", (int)value);
			//0x00~0x07 =>  0 -> 7
			//0x11~0x17 => -7 ~ -1
			RZQ_DATA = value & 0xF;
			RZQ_SIGN = (value >> 4) & 0x01;

			if (RZQ_SIGN) { //RZQ = -
				if (RZQ_DATA > 7) { //===>  < -7
					pr_info("DDR RZQ Trim data range  error RZQ_DATA -(%d) < -%d\r\n", (int)RZQ_DATA, (int)7);
					return E_SYS;
				} else if (RZQ_DATA < 1) { // ===> > -1
					pr_info("DDR RZQ Trim data range  error RZQ_DATA -(%d) > -%d\r\n", (int)RZQ_DATA, (int)1);
					return E_SYS;
				} else {
					pr_info("DDR RZQ Trim data range success RZQ_DATA -7 <= -(%d) <= -1 \r\n", (int)RZQ_DATA);
				}
			} else {        //RZQ = +
				if (RZQ_DATA > 7) {
					pr_info("DDR RZQ Trim data error RZQ_DATA %d > [%d]\r\n", (int)RZQ_DATA, (int)7);
				} else {
					pr_info("DDR RZQ Trim data RZQ_DATA 0 <= [%d] <= 7\r\n", (int)RZQ_DATA);
				}
			}
		}

		if (efuse_readParamOps(EFUSE_THERMAL_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_THERMAL_TRIM_DATA);
		} else {
			pr_info("Thermal Trim = 0x%04x => 58(0x3A) ~ 138(0x8A)\r\n", (int)value);
			THERMAL = (value & 0xFF);
			if (THERMAL > 138 || THERMAL < 58) {
				pr_info(" Thermal Trim data range  error THERMAL > 138 or THERMAL < 58 ==> [%d]\r\n", (int)THERMAL);
			} else {
				pr_info(" Thermal Trim data range success 58 <= THERMAL [%d] <= 138\r\n", (int)THERMAL);
			}
		}

		if (efuse_readParamOps(EFUSE_IDDQ_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_IDDQ_TRIM_DATA);
		} else {
			pr_info("IDDQ Trim data 0x%04x => 0~100\r\n", (int)value);
			CORE_PWR_LKE = (value & 0xFF);
			CORE_PWR_ADJ = ((value >> 8) & 0xF);
			pr_info(" CORE_PWR_LKE Trim [%d]\r\n", (int)CORE_PWR_LKE);
			pr_info(" CORE_PWR_ADJ Trim [%d]\r\n", (int)CORE_PWR_ADJ);
		}

		if (efuse_readParamOps(EFUSE_CSI_2_5G_I_TRIM_R_TERM_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_CSI_2_5G_I_TRIM_R_TERM_TRIM_DATA);
		} else {
			pr_info("CSI 2.5G I TRIM R TERM Trim = 0x%04x\r\n", (int)value);
			//bit[5..0]
			I_Trim_2_5_G = (value & 0x3F);
			//bit[9..6]
			R_Term_2_5_G = ((value >> 6) & 0xF);
			pr_info(" CSI 2.5G I trim Trim data [%d]\r\n", (int)I_Trim_2_5_G);
			pr_info(" CSI 2.5G R term Trim data [%d]\r\n", (int)R_Term_2_5_G);
		}

		if (efuse_readParamOps(EFUSE_CSI_1_5G_D1_D0_C3_C0_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_CSI_1_5G_D1_D0_C3_C0_TRIM_DATA);
		} else {
			pr_info("CSI 1.5G clock lane 0~3 & data lane 0~3 Trim data = 0x%04x\r\n", (int)value);
			//bit[11..0]
			clock_lane_trim[0] = (value & 0x3);
			clock_lane_trim[1] = ((value >> 2) & 0x3);
			clock_lane_trim[2] = ((value >> 4) & 0x3);
			clock_lane_trim[3] = ((value >> 6) & 0x3);
			data_lane_trim[0] = ((value >> 8) & 0x3);
			data_lane_trim[1] = ((value >> 10) & 0x3);

			pr_info(" CSI 1.5G clock lane 0 Trim data range success [%d]\r\n", (int)clock_lane_trim[0]);
			pr_info(" CSI 1.5G clock lane 1 Trim data range success [%d]\r\n", (int)clock_lane_trim[1]);
			pr_info(" CSI 1.5G clock lane 2 Trim data range success [%d]\r\n", (int)clock_lane_trim[2]);
			pr_info(" CSI 1.5G clock lane 3 Trim data range success [%d]\r\n", (int)clock_lane_trim[3]);
			pr_info(" CSI 1.5G  data lane 0 Trim data range success [%d]\r\n", (int)data_lane_trim[0]);
			pr_info(" CSI 1.5G  data lane 1 Trim data range success [%d]\r\n", (int)data_lane_trim[1]);
		}

		if (efuse_readParamOps(EFUSE_CSI_1_5G_D7_D2_TRIM_DATA, &value) != E_OK) {
			pr_err("[%d] data = NULL\n", EFUSE_CSI_1_5G_D7_D2_TRIM_DATA);
		} else {
			pr_info("CSI 1.5G data lane 2~7 Trim = 0x%04x\r\n", (int)value);
			//bit[11..0]
			data_lane_trim[2] = (value & 0x3);
			data_lane_trim[3] = ((value >> 2) & 0x3);
			data_lane_trim[4] = ((value >> 4) & 0x3);
			data_lane_trim[5] = ((value >> 6) & 0x3);
			data_lane_trim[6] = ((value >> 8) & 0x3);
			data_lane_trim[7] = ((value >> 10) & 0x3);

			pr_info(" CSI 1.5G data lane 2 Trim data range success [%d]\r\n", (int)data_lane_trim[2]);
			pr_info(" CSI 1.5G data lane 3 Trim data range success [%d]\r\n", (int)data_lane_trim[3]);
			pr_info(" CSI 1.5G data lane 4 Trim data range success [%d]\r\n", (int)data_lane_trim[4]);
			pr_info(" CSI 1.5G data lane 5 Trim data range success [%d]\r\n", (int)data_lane_trim[5]);
			pr_info(" CSI 1.5G data lane 6 Trim data range success [%d]\r\n", (int)data_lane_trim[6]);
			pr_info(" CSI 1.5G data lane 7 Trim data range success [%d]\r\n", (int)data_lane_trim[7]);
		}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02201)
		UINT16              value;
		INT32               trim;
		INT32               addr;
		UINT32              USB3_1_USB2_dual_phy_int_12K_resistor;          //0x2A
		UINT32              USB3_1_USB2_dual_phy_TX_term;                   //0x2A
		UINT32              USB3_1_USB2_dual_phy_RX_term;                   //0x2A

		UINT32              USB2_single_phy_RINT_resistor;                  //0x39
		UINT32              USB3_2_SATA2_Internal_12K_resistor;             //0x05
		UINT32              USB3_2_SATA2_TX_TRIM, USB3_2_SATA2_RX_TRIM;     //0x05

		UINT32              SATA1_Internal_12K_resistor;                    //0x00
		UINT32              SATA1_TX_TRIM, SATA1_RX_TRIM;                   //0x00

		UINT32              H_RZQ_DATA, H_RZQ_SIGN;
		UINT32              V_RZQ_DATA, V_RZQ_SIGN;
		UINT32              THERMAL;
		UINT32              CORE_PWR_LKE;
		UINT32              CORE_PWR_ADJ;

		UINT32              HDMI_H_resistor_1_4, HDMI_H_LANE_VL_1_4;
		UINT32              HDMI_H_resistor_2_0, HDMI_H_LANE_VL_2_0;

		UINT32              HDMI_V_resistor_1_4, HDMI_V_LANE_VL_1_4;
		UINT32              HDMI_V_resistor_2_0, HDMI_V_LANE_VL_2_0;

		UINT32              I_Term_H_PHY_Greup1, R_Term_H_PHY_Greup1;
		UINT32              I_Term_H_PHY_Greup2, R_Term_H_PHY_Greup2;

		UINT32              I_Term_V_PHY_Greup1, R_Term_V_PHY_Greup1;
		UINT32              I_Term_V_PHY_Greup2, R_Term_V_PHY_Greup2;

		UINT32              dsi_hs_output_data_swing;
		UINT32              dsi_hs_output_clk_swing;

		UINT32              slvsec_itrim;
		UINT32              slvsec_rx_term;
		UINT32              slvsec_dco_x;

		/*
		        UINT32              DDR4_H_DQ_DQS_NPD, DDR4_H_DQ_DQS_PPU;
		        UINT32              DDR4_H_CLK_CS_CKE_CA_RST_NPD, DDR4_H_CLK_CS_CKE_CA_RST_PPU;

		        UINT32              LPDDR4_H_DQ_DQS_CLK_CS_CA_NPD, LPDDR4_H_DQ_DQS_CLK_CS_CA_PPU;
		        UINT32              LPDDR4_H_CKE_RST_NPD, LPDDR4_H_CKE_RST_PPU;

		        UINT32              LPDDR4X_H_DQ_DQS_CLK_CS_CA_NPD, LPDDR4X_H_DQ_DQS_CLK_CS_CA_PPU;
		        UINT32              LPDDR4X_H_CKE_RST_NPD, LPDDR4X_H_CKE_RST_PPU;

		        UINT32              DDR4_V_DQ_DQS_NPD, DDR4_V_DQ_DQS_PPU;
		        UINT32              DDR4_V_CLK_CS_CKE_CA_RST_NPD, DDR4_V_CLK_CS_CKE_CA_RST_PPU;

		        UINT32              LPDDR4_V_DQ_DQS_CLK_CS_CA_NPD, LPDDR4_V_DQ_DQS_CLK_CS_CA_PPU;
		        UINT32              LPDDR4_V_CKE_RST_NPD, LPDDR4_V_CKE_RST_PPU;

		        UINT32              LPDDR4X_V_DQ_DQS_CLK_CS_CA_NPD, LPDDR4X_V_DQ_DQS_CLK_CS_CA_PPU;
		        UINT32              LPDDR4X_V_CKE_RST_NPD, LPDDR4X_V_CKE_RST_PPU;
		*/
		UINT32              PCIe_internal_resistor;

		UINT32              PHY0_TX_ODT_P, PHY0_TX_ODT_N, PHY0_TX_ODT_S;
		UINT32              PHY1_TX_ODT_P, PHY1_TX_ODT_N, PHY1_TX_ODT_S;

		//----------------------------------------------------------------------
		//USB3_1, USB2 Dual PHY [0x2A]
		//SATA1                 [0x0]
		//SATA2/USB3_2          [0x5]
		//----------------------------------------------------------------------
		trim = efuse_readParamOps(EFUSE_USB3_1_TRIM_DATA, &value);
		addr = 0x2A;
		if (trim == EFUSE_SUCCESS) {
			pr_info("  USB3_1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			if (trim == EFUSE_SUCCESS) {
				//12K resistor 1<= x <= 20(0x14)
				pr_info("  USB3_1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
				pr_info("  12K_resistor[4..0] valid from 1(0x1) <-> 20(0x14)\r\n");
				USB3_1_USB2_dual_phy_int_12K_resistor = value & 0x1F;
				USB3_1_USB2_dual_phy_TX_term = ((value >> 5) & 0xF);
				USB3_1_USB2_dual_phy_RX_term = ((value >> 9) & 0xF);

				if (USB3_1_USB2_dual_phy_int_12K_resistor > 0x14) {
					pr_info("12K_resistor Trim data error addr[0x%02x] = 0x%04x > 20(0x14)\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_int_12K_resistor);
				} else if (USB3_1_USB2_dual_phy_int_12K_resistor < 0x1) {
					pr_info("12K_resistor Trim data error addr[0x%02x] = 0x%04x < 1(0x1)\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_int_12K_resistor);
				} else {
					pr_info("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB3_1_USB2_dual_phy_int_12K_resistor);
				}

				//TX term bit[8..5] 0x2 <= x <= 0xE
				if (USB3_1_USB2_dual_phy_TX_term > 0xE) {
					pr_info("USB(SATA) TX Trim data error addr[0x%02x] = 0x%04x > 0xE\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_TX_term);
				} else if (USB3_1_USB2_dual_phy_TX_term < 0x2) {
					pr_info("USB(SATA) TX Trim data error addr[0x%02x] = 0x%04x < 0x2\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_TX_term);
				} else {
					pr_info("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB3_1_USB2_dual_phy_TX_term);
				}
				//RX term bit[12..9] 0x8 <= x <= 0xE
				if (USB3_1_USB2_dual_phy_RX_term > 0xE) {
					pr_info("USB(SATA) RX Trim data error addr[0x%02x] = 0x%04x > 0xE\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_RX_term);
				} else if (USB3_1_USB2_dual_phy_RX_term < 0x8) {
					pr_info("USB(SATA) RX Trim data error addr[0x%02x] = 0x%04x < 0x8\r\n", (int)addr, (int)USB3_1_USB2_dual_phy_RX_term);
				} else {
					pr_info("  *USB(SATA) RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB3_1_USB2_dual_phy_RX_term);
				}
			} else {
				pr_err("addr[%02x] error %d\r\n", (int)addr, (int)trim);
			}
		}

		addr = 0x0;
		trim = efuse_readParamOps(EFUSE_SATA_1_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("  SATA1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			if (trim == EFUSE_SUCCESS) {
				//12K resistor 1<= x <= 20(0x14)
				pr_info("  SATA1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
				pr_info("  SATA1 12K_resistor[4..0] valid from 1(0x1) <-> 20(0x14)\r\n");
				SATA1_Internal_12K_resistor = value & 0x1F;
				SATA1_TX_TRIM = ((value >> 5) & 0xF);
				SATA1_RX_TRIM = ((value >> 9) & 0xF);

				if (SATA1_Internal_12K_resistor > 0x14) {
					pr_info("SATA1 12K_resistor Trim data error [0x%02x]=0x%04x > 20(0x14)\r\n", (int)addr, (int)SATA1_Internal_12K_resistor);
				} else if (SATA1_Internal_12K_resistor < 0x1) {
					pr_info("SATA1 12K_resistor Trim data error [0x%02x]=0x%04x < 1(0x1)\r\n", (int)addr, (int)SATA1_Internal_12K_resistor);
				} else {
					pr_info("  *SATA1 12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)SATA1_Internal_12K_resistor);
				}

				//TX term bit[8..5] 0x2 <= x <= 0xE
				if (SATA1_TX_TRIM > 0xE) {
					pr_info("SATA1 TX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)SATA1_TX_TRIM);
				} else if (SATA1_TX_TRIM < 0x2) {
					pr_info("SATA1 TX Trim data error [0x%02x]=0x%04x < 0x2\r\n", (int)addr, (int)SATA1_TX_TRIM);
				} else {
					pr_info("  *SATA1 TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)SATA1_TX_TRIM);
				}
				//RX term bit[12..9] 0x8 <= x <= 0xE
				if (SATA1_RX_TRIM > 0xE) {
					pr_info("SATA1 RX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)SATA1_RX_TRIM);
				} else if (SATA1_RX_TRIM < 0x8) {
					pr_info("SATA1 RX Trim data error [0x%02x]=0x%04x < 0x8\r\n", (int)addr, (int)SATA1_RX_TRIM);
				} else {
					pr_info("  *SATA1 RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)SATA1_RX_TRIM);
				}
			} else {
				pr_err("addr[%02x] error %d\r\n", (int)addr, (int)trim);
			}
		}

		addr = 0x5;
		trim = efuse_readParamOps(EFUSE_USB3_2_SATA2_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("  SATA2/USB3_2 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			if (trim == EFUSE_SUCCESS) {
				//12K resistor 1<= x <= 20(0x14)
				pr_info("  USB3_2(SATA2) Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
				pr_info("  USB3_2(SATA2) 12K_resistor[4..0] valid from 1(0x1) <-> 20(0x14)\r\n");
				USB3_2_SATA2_Internal_12K_resistor = value & 0x1F;
				USB3_2_SATA2_TX_TRIM = ((value >> 5) & 0xF);
				USB3_2_SATA2_RX_TRIM = ((value >> 9) & 0xF);

				if (USB3_2_SATA2_Internal_12K_resistor > 0x14) {
					pr_info("12K_resistor Trim data error [0x%02x]=0x%04x > 20(0x14)\r\n", (int)addr, (int)USB3_2_SATA2_Internal_12K_resistor);
				} else if (USB3_2_SATA2_Internal_12K_resistor < 0x1) {
					pr_info("12K_resistor Trim data error [0x%02x]=0x%04x < 1(0x1)\r\n", (int)addr, (int)USB3_2_SATA2_Internal_12K_resistor);
				} else {
					pr_info("  *12K_resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)USB3_2_SATA2_Internal_12K_resistor);
				}

				//TX term bit[8..5] 0x2 <= x <= 0xE
				if (USB3_2_SATA2_TX_TRIM > 0xE) {
					pr_info("USB(SATA) TX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)USB3_2_SATA2_TX_TRIM);
				} else if (USB3_2_SATA2_TX_TRIM < 0x2) {
					pr_info("USB(SATA) TX Trim data error [0x%02x]=0x%04x < 0x2\r\n", (int)addr, (int)USB3_2_SATA2_TX_TRIM);
				} else {
					pr_info("  *USB(SATA) TX Trim data range success 0x2 <= [0x%04x] <= 0xE\r\n", (int)USB3_2_SATA2_TX_TRIM);
				}
				//RX term bit[12..9] 0x8 <= x <= 0xE
				if (USB3_2_SATA2_RX_TRIM > 0xE) {
					pr_info("USB(SATA) RX Trim data error [0x%02x]=0x%04x > 0xE\r\n", (int)addr, (int)USB3_2_SATA2_RX_TRIM);
				} else if (USB3_2_SATA2_RX_TRIM < 0x8) {
					pr_info("USB(SATA) RX Trim data error [0x%02x]=0x%04x < 0x8\r\n", (int)addr, (int)USB3_2_SATA2_RX_TRIM);
				} else {
					pr_info("  *USB(SATA) RX Trim data range success 0x8 <= [0x%04x] <= 0xE\r\n", (int)USB3_2_SATA2_RX_TRIM);
				}
			} else {
				pr_err("addr[%02x] error %d\r\n", (int)addr, (int)trim);
			}
		}
		/*----------------------------------------------------------------------
		R-term / I-term H-PHY Group1 0x3E   (1)
		R-term / I-term H-PHY Group2 0x3D   (2)
		R-term / I-term V-PHY Group1 0x3C   (3)
		R-term / I-term V-PHY Group2 0x3B   (4)
		-----------------------------------------------------------------------*/
		addr = 0x3E;
		trim = efuse_readParamOps(EFUSE_CSI_1_I_TRIM_R_TERM_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("CSI1 I TRIM R TERM H-phy group1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[5..0]
			I_Term_H_PHY_Greup1 = (value & 0x3F);
			//bit[9..6]
			R_Term_H_PHY_Greup1 = ((value >> 6) & 0xF);
			if (I_Term_H_PHY_Greup1 > 0x35) {
				pr_info("CSI1 I-term H-phy group1 Trim data error [0x%02x]=0x%04x > 0x35\r\n", (int)addr, (int)I_Term_H_PHY_Greup1);
			} else if (I_Term_H_PHY_Greup1 < 0x0D) {
				pr_info("CSI1 I-term H-phy group1 Trim data error [0x%02x]=0x%04x < 0x0D\r\n", (int)addr, (int)I_Term_H_PHY_Greup1);
			} else {
				pr_info(" CSI1 I-term H-phy group1 Trim data range success 0x0D <= [0x%04x] <= 0x35\r\n", (int)I_Term_H_PHY_Greup1);
			}

			if (R_Term_H_PHY_Greup1 > 0x9) {
				pr_info("CSI1 R-term H-phy group1 Trim data error [0x%02x]=0x%04x > 0x35\r\n", (int)addr, (int)R_Term_H_PHY_Greup1);
			} else if (R_Term_H_PHY_Greup1 < 0x2) {
				pr_info("CSI1 R-term H-phy group1 Trim data error [0x%02x]=0x%04x < 0x0D\r\n", (int)addr, (int)R_Term_H_PHY_Greup1);
			} else {
				pr_info(" CSI1 R-term H-phy group1 Trim data range success 0x2 <= [0x%04x] <= 0x9\r\n", (int)R_Term_H_PHY_Greup1);
			}
		} else {
			pr_err("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x3D;
		trim = efuse_readParamOps(EFUSE_CSI_2_I_TRIM_R_TERM_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("CSI2 I TRIM R TERM H-phy group2 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[5..0]
			I_Term_H_PHY_Greup2 = (value & 0x3F);
			//bit[9..6]
			R_Term_H_PHY_Greup2 = ((value >> 6) & 0xF);
			if (I_Term_H_PHY_Greup2 > 0x35) {
				pr_info("CSI2 I-term H-phy group2 Trim data error [0x%02x]=0x%04x > 0x35\r\n", (int)addr, (int)I_Term_H_PHY_Greup2);
			} else if (I_Term_H_PHY_Greup2 < 0x0D) {
				pr_info("CSI2 I-term H-phy group2 Trim data error [0x%02x]=0x%04x < 0x0D\r\n", (int)addr, (int)I_Term_H_PHY_Greup2);
			} else {
				pr_info(" CSI2 I-term H-phy group2 Trim data range success 0x0D <= [0x%04x] <= 0x35\r\n", (int)I_Term_H_PHY_Greup2);
			}

			if (R_Term_H_PHY_Greup2 > 0x9) {
				pr_info("CSI2 R-term H-phy group2 Trim data error [0x%02x]=0x%04x > 0x35\r\n", (int)addr, (int)R_Term_H_PHY_Greup2);
			} else if (R_Term_H_PHY_Greup2 < 0x2) {
				pr_info("CSI2 R-term H-phy group2 Trim data error [0x%02x]=0x%04x < 0x0D\r\n", (int)addr, (int)R_Term_H_PHY_Greup2);
			} else {
				pr_info(" CSI2 R-term H-phy group2 Trim data range success 0x2 <= [0x%04x] <= 0x9\r\n", (int)R_Term_H_PHY_Greup2);
			}
		} else {
			pr_err("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x3C;
		trim = efuse_readParamOps(EFUSE_CSI_3_I_TRIM_R_TERM_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("CSI3 I TRIM R TERM V-phy group1 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[5..0]
			I_Term_V_PHY_Greup1 = (value & 0x3F);
			//bit[9..6]
			R_Term_V_PHY_Greup1 = ((value >> 6) & 0xF);
			if (I_Term_V_PHY_Greup1 > 0x35) {
				pr_info("CSI3 I-term V-phy group1 Trim data error [0x%02x]=0x%04x > 0x35\r\n", (int)addr, (int)I_Term_V_PHY_Greup1);
			} else if (I_Term_V_PHY_Greup1 < 0x0D) {
				pr_info("CSI3 I-term V-phy group1 Trim data error [0x%02x]=0x%04x < 0x0D\r\n", (int)addr, (int)I_Term_V_PHY_Greup1);
			} else {
				pr_info(" CSI3 I-term V-phy group1 Trim data range success 0x0D <= [0x%04x] <= 0x35\r\n", (int)I_Term_V_PHY_Greup1);
			}

			if (R_Term_V_PHY_Greup1 > 0x9) {
				pr_info("CSI3 R-term V-phy group1 Trim data error [0x%02x]=0x%04x > 0x35\r\n", (int)addr, (int)R_Term_V_PHY_Greup1);
			} else if (R_Term_V_PHY_Greup1 < 0x2) {
				pr_info("CSI3 R-term V-phy group1 Trim data error [0x%02x]=0x%04x < 0x0D\r\n", (int)addr, (int)R_Term_V_PHY_Greup1);
			} else {
				pr_info(" CSI3 R-term V-phy group1 Trim data range success 0x2 <= [0x%04x] <= 0x9\r\n", (int)R_Term_V_PHY_Greup1);
			}
		} else {
			pr_err("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}
		addr = 0x3B;
		trim = efuse_readParamOps(EFUSE_CSI_4_I_TRIM_R_TERM_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("CSI4 I TRIM R TERM V-phy group2 Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[5..0]
			I_Term_V_PHY_Greup2 = (value & 0x3F);
			//bit[9..6]
			R_Term_V_PHY_Greup2 = ((value >> 6) & 0xF);
			if (I_Term_V_PHY_Greup2 > 0x35) {
				pr_info("CSI4 I-term V-phy group2 Trim data error [0x%02x]=0x%04x > 0x35\r\n", (int)addr, (int)I_Term_V_PHY_Greup2);
			} else if (I_Term_V_PHY_Greup2 < 0x0D) {
				pr_info("CSI4 I-term V-phy group2 Trim data error [0x%02x]=0x%04x < 0x0D\r\n", (int)addr, (int)I_Term_V_PHY_Greup2);
			} else {
				pr_info(" CSI4 I-term V-phy group2 Trim data range success 0x0D <= [0x%04x] <= 0x35\r\n", (int)I_Term_V_PHY_Greup2);
			}

			if (R_Term_V_PHY_Greup2 > 0x9) {
				pr_info("CSI4 R-term V-phy group2 Trim data error [0x%02x]=0x%04x > 0x35\r\n", (int)addr, (int)R_Term_V_PHY_Greup2);
			} else if (R_Term_V_PHY_Greup2 < 0x2) {
				pr_info("CSI4 R-term V-phy group2 Trim data error [0x%02x]=0x%04x < 0x0D\r\n", (int)addr, (int)R_Term_V_PHY_Greup2);
			} else {
				pr_info(" CSI4 R-term V-phy group2 Trim data range success 0x2 <= [0x%04x] <= 0x9\r\n", (int)R_Term_V_PHY_Greup2);
			}
		} else {
			pr_err("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		/*-----------------------------------------------------------------------
		    HDMI1 / 2
		-----------------------------------------------------------------------*/
		addr = 0x3;
		trim = efuse_readParamOps(EFUSE_HDMI_H_PHY_RESISTOR_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("  HDMI(H) resistor Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			pr_info("  (H)1.4 resistor[4..0] valid from 0x8 <-> 0x14\r\n");
			pr_info("  (H)2.0 resistor[9..5] valid from 0x8 <-> 0x14\r\n");
			//bit[4..0]
			HDMI_H_resistor_1_4 = value & 0x1F;
			//bit[9..5]
			HDMI_H_resistor_2_0 = ((value >> 5) & 0x1F);

			if (HDMI_H_resistor_1_4 > 0x14) {
				pr_info("(H)1.4 HDMI resistor Trim data error [0x%02x] = 0x%04x > 0x14\r\n", (int)addr, (int)HDMI_H_resistor_1_4);
			} else if (HDMI_H_resistor_1_4 < 0x1) {
				pr_info("(H)1.4 HDMI resistor Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)HDMI_H_resistor_1_4);
			} else {
				pr_info(" *(H)1.4 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_H_resistor_1_4);
			}

			if (HDMI_H_resistor_2_0 > 0x14) {
				pr_info("(H)2.0 HDMI resistor Trim data error [0x%02x]=0x%04x > 0x14\r\n", (int)addr, (int)HDMI_H_resistor_2_0);
			} else if (HDMI_H_resistor_2_0 < 0x1) {
				pr_info("(H)2.0 HDMI resistor Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)HDMI_H_resistor_2_0);
			} else {
				pr_info(" *(H)2.0 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_H_resistor_2_0);
			}
		} else {
			pr_err("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x1;
		trim = efuse_readParamOps(EFUSE_HDMI_H_PHY_2_0_VLANE_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("  (H)HDMI 2.0 VLane tunning Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[11..0]
			HDMI_H_LANE_VL_2_0 = value & 0xFFF;
			pr_info("  *(H)2.0 HDMI VLane Trim data success [0x%04x](no limitation)\r\n", (int)HDMI_H_LANE_VL_2_0);
		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x2F;
		trim = efuse_readParamOps(EFUSE_HDMI_H_PHY_1_4_VLANE_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			// resistor 0x8<= x <= 0x14
			pr_info("  (H)HDMI 1.4 VLane tunning Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[11..0]
			HDMI_H_LANE_VL_1_4 = value & 0xFFF;
			pr_info("  *(H)1.4 HDMI VLane Trim data success [0x%04x](no limitation)\r\n", (int)HDMI_H_LANE_VL_1_4);
		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x2E;
		trim = efuse_readParamOps(EFUSE_HDMI_V_PHY_RESISTOR_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("  HDMI(V) resistor Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			pr_info("  (V)1.4 resistor[4..0] valid from 0x8 <-> 0x14\r\n");
			pr_info("  (V)2.0 resistor[9..5] valid from 0x8 <-> 0x14\r\n");
			//bit[4..0]
			HDMI_V_resistor_1_4 = value & 0x1F;
			//bit[9..5]
			HDMI_V_resistor_2_0 = ((value >> 5) & 0x1F);

			if (HDMI_V_resistor_1_4 > 0x14) {
				pr_info("(V)1.4 HDMI resistor Trim data error [0x%02x] = 0x%04x > 0x14\r\n", (int)addr, (int)HDMI_V_resistor_1_4);
			} else if (HDMI_V_resistor_1_4 < 0x1) {
				pr_info("(V)1.4 HDMI resistor Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)HDMI_V_resistor_1_4);
			} else {
				pr_info("  *(V)1.4 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_V_resistor_1_4);
			}

			if (HDMI_V_resistor_2_0 > 0x14) {
				pr_info("(V)2.0 HDMI resistor Trim data error [0x%02x]=0x%04x > 0x14\r\n", (int)addr, (int)HDMI_V_resistor_2_0);
			} else if (HDMI_V_resistor_2_0 < 0x1) {
				pr_info("(V)2.0 HDMI resistor Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)HDMI_V_resistor_2_0);
			} else {
				pr_info("  *(V)2.0 HDMI resistor Trim data range success 0x1 <= [0x%04x] <= 0x14\r\n", (int)HDMI_V_resistor_2_0);
			}
		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x2C;
		trim = efuse_readParamOps(EFUSE_HDMI_V_PHY_2_0_VLANE_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("  (V)HDMI 2.0 VLane tunning Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[11..0]
			HDMI_V_LANE_VL_2_0 = value & 0xFFF;
			pr_info("  *(V)2.0 HDMI VLane Trim data success [0x%04x](no limitation)\r\n", (int)HDMI_V_LANE_VL_2_0);
		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x2D;
		trim = efuse_readParamOps(EFUSE_HDMI_V_PHY_1_4_VLANE_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			// resistor 0x8<= x <= 0x14
			pr_info("  (V)HDMI 1.4 VLane tunning Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[11..0]
			HDMI_V_LANE_VL_1_4 = value & 0xFFF;
			pr_info("  *(V)1.4 HDMI VLane Trim data success [0x%04x]\r\n", (int)HDMI_V_LANE_VL_1_4);
		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x39;
		trim = efuse_readParamOps(EFUSE_USB_RINT_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("  USB2 single phy Trim data = 0x%04x\r\n", (int)value);

			USB2_single_phy_RINT_resistor = (value & 0x1F);

			//USB2 single phy internal resistor 0x1 <= x <= 0x14
			if (USB2_single_phy_RINT_resistor > 0x14) {
				pr_info("USB2 single phy RINT Trim data error [0x%02x]=0x%04x > 0x14\r\n", (int)addr, (int)USB2_single_phy_RINT_resistor);
			} else if (USB2_single_phy_RINT_resistor < 0x1) {
				pr_info("USB2 single phy RINT Trim data error [0x%02x]=0x%04x < 0x1\r\n", (int)addr, (int)USB2_single_phy_RINT_resistor);
			} else {
				pr_info("USB2 single phy RINT trim 0x01 <= data[0x%04x] <= 0x14 success\r\n", (int)USB2_single_phy_RINT_resistor);
			}
		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x2;
		trim = efuse_readParamOps(EFUSE_DDRP_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("\r\n");
			pr_info(" DDR ZQ Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//0x00~0x07 =>  0 -> 7
			//0x11~0x17 => -7 ~ -1
			H_RZQ_DATA = value & 0xF;
			H_RZQ_SIGN = (value >> 4) & 0x01;

			V_RZQ_DATA = (value >> 5) & 0xF;
			V_RZQ_SIGN = (value >> 9) & 0x01;

			//H_RZQxxx
			if (H_RZQ_SIGN) { //H_RZQ = -
				if (H_RZQ_DATA > 7) { //===>  < -7
					pr_info("  H_RZQ Trim data range  error H_RZQ_DATA -(%d) < -%d\r\n", (int)H_RZQ_DATA, (int)7);
				} else if (H_RZQ_DATA < 1) { // ===> > -1
					pr_info("  H_RZQ Trim data range  error H_RZQ_DATA -(%d) > -%d\r\n", (int)H_RZQ_DATA, (int)1);
				} else {
					pr_info("  H_RZQ Trim data range success H_RZQ_DATA -7 <= -(%d) <= -1 \r\n", (int)H_RZQ_DATA);
				}
			} else {        //RZQ = +
				if (H_RZQ_DATA > 7) {
					pr_info("  H_RZQ Trim data range  error H_RZQ_DATA %d > (%d)\r\n", (int)H_RZQ_DATA, (int)7);
				} else {
					pr_info("  H_RZQ Trim data range success H_RZQ_DATA 0 <=  (%d) <= 7\r\n", (int)H_RZQ_DATA);
				}
			}

			//V_RZQxxx
			if (V_RZQ_SIGN) { //V_RZQ = -
				if (V_RZQ_DATA > 7) { //===>  < -7
					pr_info("  V_RZQ Trim data range  error V_RZQ_DATA -(%d) < -%d\r\n", (int)V_RZQ_DATA, (int)7);
				} else if (V_RZQ_DATA < 1) { // ===> > -1
					pr_info("  V_RZQ Trim data range  error V_RZQ_DATA -(%d) > -%d\r\n", (int)V_RZQ_DATA, (int)1);
				} else {
					pr_info("  V_RZQ Trim data range success V_RZQ_DATA -7 <= -(%d) <= -1 \r\n", (int)V_RZQ_DATA);
				}
			} else {        //RZQ = +
				if (V_RZQ_DATA > 7) {
					pr_info("  V_RZQ Trim data range  error V_RZQ_DATA %d >  (%d)\r\n", (int)V_RZQ_DATA, (int)7);
				} else {
					pr_info("  V_RZQ Trim data range success V_RZQ_DATA 0 <=  (%d) <= 7\r\n", (int)V_RZQ_DATA);
				}
			}
		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x6;
		trim = efuse_readParamOps(EFUSE_THERMAL_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info(" Thermal Trim data read success addr[0x%02x] = 0x%04x => 70(0x46) <-> 110(0x6E)\r\n", (int)addr, (int)value);

			//bit[7..0]
			THERMAL = (value & 0xFF);
			if (THERMAL > 110 || THERMAL < 70) {
				pr_info(" Thermal Trim data range  error THERMAL > 110(0x6E) or THERMAL < 70(0x46) ==> [%d]\r\n", (int)THERMAL);
			} else {
				pr_info(" Thermal Trim data range success 70 <= THERMAL [%d] <= 110\r\n", (int)THERMAL);
			}
		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x7;
		trim = efuse_readParamOps(EFUSE_IDDQ_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("IDDQ Trim data read success addr[0x%02x] = 0x%04x => 0~100\r\n", (int)addr, (int)value);

			CORE_PWR_LKE = (value & 0xFF);

			CORE_PWR_ADJ = ((value >> 8) & 0xF);

			pr_info(" CORE_PWR_LKE Trim data range success [%d]\r\n", (int)CORE_PWR_LKE);
			pr_info(" CORE_PWR_ADJ Trim data range success [%d]\r\n", (int)CORE_PWR_ADJ);

		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)trim);
		}

		addr = 0x3A;
		trim = efuse_readParamOps(EFUSE_DSITXPLL_RG_LEVEL_LDO_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("DSI TX output swing(data/clk) Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[3..0]
			dsi_hs_output_data_swing = (value & 0xF);
			//bit[7..4]
			dsi_hs_output_clk_swing = ((value >> 4) & 0xF);

			pr_info(" DSI output data swing Trim data success [0x%04x]\r\n", (int)dsi_hs_output_data_swing);
			pr_info(" DSI output  clk swing Trim data success [0x%04x]\r\n", (int)dsi_hs_output_clk_swing);

		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)value);
		}

		addr = 0x2B;
		trim = efuse_readParamOps(EFUSE_SLVSEC_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("SLVSEC Trim data read success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[4..0]
			slvsec_itrim = (value & 0x1F);
			//bit[8..5]
			slvsec_rx_term = ((value >> 5) & 0xF);
			//bit[10..9]
			slvsec_dco_x = ((value >> 9) & 0x3);

			pr_info(" SLVSEC   itrim data success [0x%04x]\r\n", (int)slvsec_itrim);
			pr_info(" SLVSEC rx term data success [0x%04x]\r\n", (int)slvsec_rx_term);
			pr_info(" SLVSEC   dco x data success [0x%04x]\r\n", (int)slvsec_dco_x);

		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)value);
		}

		addr = 0x30;
		trim = efuse_readParamOps(EFUSE_PCIE_RINT_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("   PCIe internal resistor trim success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[4..0]
			PCIe_internal_resistor = (value & 0x1F);           //0x7 ~ 0x19

			if (PCIe_internal_resistor < 0x7) {
				pr_info("   PCIe internal resistor 0x7 <= PCIe_internal_resistor[0x%04x] <= 0x19 error\r\n", (int)PCIe_internal_resistor);
			} else if (PCIe_internal_resistor > 0x19) {
				pr_info("   PCIe internal resistor 0x7 <= PCIe_internal_resistor[0x%04x] <= 0x19 error\r\n", (int)PCIe_internal_resistor);
			} else {
				pr_info("    PCIe internal resistor Trim data range success 0x7 <= PCIe_internal_resistor[0x%04x] <= 0x19\r\n", (int)PCIe_internal_resistor);
			}

		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)value);
		}

		addr = 0x31;
		trim = efuse_readParamOps(EFUSE_PCIE0_ODT_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("   PCIe0 ODT trim success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[ 3..0]    0x7 <= xx <= 0xC
			PHY0_TX_ODT_P = (value & 0xF);
			//bit[ 7..4]    0x7 <= xx <= 0xE
			PHY0_TX_ODT_N = ((value >> 4) & 0xF);
			//bit[11..8]    0x2 <= xx <= 0x4
			PHY0_TX_ODT_S = ((value >> 8) & 0xF);

			//0x7 <= xx <= 0xC
			if (PHY0_TX_ODT_P < 0x7 || PHY0_TX_ODT_P > 0xC) {
				pr_info("   PCIe0 TX ODT P 0x7 <= PHY0_TX_ODT_P[0x%04x] <= 0xC error\r\n", (int)PHY0_TX_ODT_P);
			} else {
				pr_info("    PCIe0 TX ODT P Trim data range success 0x7 <= PHY0_TX_ODT_P[0x%04x] <= 0xC\r\n", (int)PHY0_TX_ODT_P);
			}

			//0x7 <= xx <= 0xE
			if (PHY0_TX_ODT_N < 0x7 || PHY0_TX_ODT_N > 0xE) {
				pr_info("   PCIe0 TX ODT N 0x7 <= PHY0_TX_ODT_N[0x%04x] <= 0xE error\r\n", (int)PHY0_TX_ODT_N);
			} else {
				pr_info("    PCIe0 TX ODT N Trim data range success 0x7 <= PHY0_TX_ODT_N[0x%04x] <= 0xC\r\n", (int)PHY0_TX_ODT_N);
			}

			//0x2 <= xx <= 0x4
			if (PHY0_TX_ODT_S < 0x2 || PHY0_TX_ODT_S > 0x4) {
				pr_info("   PCIe0 TX ODT S 0x2 <= PHY0_TX_ODT_S[0x%04x] <= 0x4 error\r\n", (int)PHY0_TX_ODT_S);
			} else {
				pr_info("    PCIe0 TX ODT S Trim data range success 0x2 <= PHY0_TX_ODT_S[0x%04x] <= 0x4\r\n", (int)PHY0_TX_ODT_S);
			}

		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)value);
		}

		addr = 0x32;
		trim = efuse_readParamOps(EFUSE_PCIE1_ODT_TRIM_DATA, &value);
		if (trim == EFUSE_SUCCESS) {
			pr_info("   PCIe1 ODT trim success addr[0x%02x] = 0x%04x\r\n", (int)addr, (int)value);
			//bit[ 3..0]    0x7 <= xx <= 0xC
			PHY1_TX_ODT_P = (value & 0xF);
			//bit[ 7..4]    0x7 <= xx <= 0xE
			PHY1_TX_ODT_N = ((value >> 4) & 0xF);
			//bit[11..8]    0x2 <= xx <= 0x4
			PHY1_TX_ODT_S = ((value >> 8) & 0xF);

			//0x7 <= xx <= 0xC
			if (PHY1_TX_ODT_P < 0x7 || PHY1_TX_ODT_P > 0xC) {
				pr_info("   PCIe1 TX ODT P 0x7 <= PHY1_TX_ODT_P[0x%04x] <= 0xC error\r\n", (int)PHY1_TX_ODT_P);
			} else {
				pr_info("    PCIe1 TX ODT P Trim data range success 0x7 <= PHY1_TX_ODT_P[0x%04x] <= 0xC\r\n", (int)PHY1_TX_ODT_P);
			}

			//0x7 <= xx <= 0xE
			if (PHY1_TX_ODT_N < 0x7 || PHY1_TX_ODT_N > 0xE) {
				pr_info("   PCIe1 TX ODT N 0x7 <= PHY0_TX_ODT_N[0x%04x] <= 0xE error\r\n", (int)PHY1_TX_ODT_N);
			} else {
				pr_info("    PCIe1 TX ODT N Trim data range success 0x7 <= PHY0_TX_ODT_N[0x%04x] <= 0xC\r\n", (int)PHY1_TX_ODT_N);
			}

			//0x2 <= xx <= 0x4
			if (PHY1_TX_ODT_S < 0x2 || PHY1_TX_ODT_S > 0x4) {
				pr_info("   PCIe1 TX ODT S 0x2 <= PHY0_TX_ODT_S[0x%04x] <= 0x4 error\r\n", (int)PHY1_TX_ODT_S);
			} else {
				pr_info("    PCIe1 TX ODT S Trim data range success 0x2 <= PHY0_TX_ODT_S[0x%04x] <= 0x4\r\n", (int)PHY1_TX_ODT_S);
			}

		} else {
			pr_info("addr[%02x] error %d\r\n", (int)addr, (int)value);
		}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
		UINT16  value;
		UINT32  trim_A, trim_B;
		UINT32  THERMAL;
		UINT32  USB_Int_12K_resistor;
		UINT32  USB_TX_TRIM, USB_RX_TRIM;
		UINT32  TX_TRIM_ROUT, TX_TRIM_DAC, TX_TRIM_SEL_RX, TX_TRIM_SEL_TX;
		UINT32  MIPI_RX_Rterm_GRP1, MIPI_RX_Rterm_GRP2, MIPI_TX_iadj;

		if (efuse_readParamOps(EFUSE_ADC_TRIM_A_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[%d] data = NULL\n", EFUSE_ADC_TRIM_A_DATA);
		} else {
			pr_info("\r\n");
			trim_A = value & 0x7F;
			pr_info("[%d]trimA = 0x%04x\r\n", EFUSE_ADC_TRIM_A_DATA, (int)trim_A);
		}

		if (efuse_readParamOps(EFUSE_ADC_TRIM_B_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[%d] data = NULL\n", EFUSE_ADC_TRIM_B_DATA);
		} else {
			pr_info("\r\n");
			trim_B = value & 0x1FF;
			pr_info("[%d]trimB = 0x%04x\r\n", EFUSE_ADC_TRIM_B_DATA, (int)trim_B);
		}

		if (efuse_readParamOps(EFUSE_USBC_TRIM_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[%d] data = NULL\n", EFUSE_USBC_TRIM_DATA);
		} else {
			pr_info("\r\n");
			USB_Int_12K_resistor = value & 0x1F;
			USB_TX_TRIM = ((value >> 5) & 0xF);
			USB_RX_TRIM = ((value >> 9) & 0xF);
			pr_info("12K_resistor Trim = 0x%04x\r\n", USB_Int_12K_resistor);
			pr_info("      USB TX Trim = 0x%04x\r\n", (int)USB_TX_TRIM);
			pr_info("      USB RX Trim = 0x%04x\r\n", (int)USB_RX_TRIM);

		}

		if (efuse_readParamOps(EFUSE_ETHERNET_TRIM_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[%d] data = NULL\n", EFUSE_ETHERNET_TRIM_DATA);
		} else {
			pr_info("\r\n");
			TX_TRIM_ROUT    = value & 0x7;              // bit[2..0] all available
			TX_TRIM_DAC     = ((value >> 3) & 0x1F);    // bit[7..3]
			TX_TRIM_SEL_RX  = ((value >> 8) & 0x7);     // bit[10..8]
			TX_TRIM_SEL_TX  = ((value >> 11) & 0x7);    // bit[13..11]
			pr_info("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_ROUT);
			pr_info("  	=>  TX_TRIM_DAC[0x%04x]\r\n", (int)TX_TRIM_DAC);
			pr_info("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_SEL_RX);
			pr_info("  	=> TX_TRIM_ROUT[0x%04x]\r\n", (int)TX_TRIM_SEL_TX);
		}

		if (efuse_readParamOps(EFUSE_THERMAL_TRIM_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[%d] data = NULL\n", EFUSE_THERMAL_TRIM_DATA);
		} else {
			pr_info("\r\n");
			THERMAL = value & 0xFF;
			pr_info("Thermal Trim = 0x%04x\r\n", THERMAL);
		}

		if (efuse_readParamOps(EFUSE_MIPI_TX_RX_TRIM_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[%d] data = NULL\n", EFUSE_MIPI_TX_RX_TRIM_DATA);
		} else {
			pr_info("\r\n");
			MIPI_RX_Rterm_GRP1  = value & 0xF;              // bit[3..0] all available
			MIPI_RX_Rterm_GRP2  = ((value >> 4) & 0xF);     // bit[7..4]
			MIPI_TX_iadj        = ((value >> 8) & 0x7);     // bit[10..8]
			pr_info("  	=>	MIPI_RX_Rterm_GRP1[0x%04x]\r\n", (int)MIPI_RX_Rterm_GRP1);
			pr_info("  	=>	MIPI_RX_Rterm_GRP2[0x%04x]\r\n", (int)MIPI_RX_Rterm_GRP2);
			pr_info("  	=> 	MIPI_TX_iadj[0x%04x]\r\n", (int)MIPI_TX_iadj);
		}


#elif defined(CONFIG_NVT_IVOT_PLAT_NA51103)
		UINT16          value;
		UINT16          value_2[2];
		UINT32          RINT_SEL;
		UINT32          HDMI_resistor, HDMI_LANE_VL;
		UINT32          HDMI_LANE0;
		UINT32          HDMI_LANE1;
		UINT32          HDMI_LANE2;
		UINT32          HDMI_CLK;

		UINT32          TX_TRIM_ROUT, TX_TRIM_DAC, TX_TRIM_SEL_RX, RX_TRIM_SEL_TX;
		UINT32          THERMAL;
		UINT32          CORE_POWER_ADJ, CORE_POWER_LEAKAGE;
		UINT32          CVBS, VGA;
		UINT32          SATA_BIAS_VLDOSEL_PI, SATA_BIAS_RX_VLDO_SEL;
		UINT32          SATA_BIAS_LDOSEL_PLL, SATA_BIAS_VREF_SEL, SATA_TX_ZTX_CTL;
		//UINT32        RZQ_DATA, RZQ_SIGN;

		if (efuse_readParamOps(EFUSE_ETHERNET_TRIM_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[0] data = NULL\n");
		} else {
			pr_info("\r\n");
			pr_info("[0]Ethernet = 0x%04x\r\n", (int)value);
			TX_TRIM_ROUT    = value & 0x7;
			TX_TRIM_DAC     = ((value >> 3) & 0x1F);
			TX_TRIM_SEL_RX  = ((value >> 8) & 0x7);
			RX_TRIM_SEL_TX  = ((value >> 11) & 0x7);
			pr_info("  	=>  [TX_TRIM_ROUT]=[0x%04x]\r\n", (int)(value & 0x7));
			pr_info("  	=>   [TX_TRIM_DAC]=[0x%04x]\r\n", (int)TX_TRIM_DAC);
			pr_info("  	=>[TX_TRIM_SEL_RX]=[0x%04x]\r\n", (int)TX_TRIM_SEL_RX);
			pr_info("  	=>[RX_TRIM_SEL_TX]=[0x%04x]\r\n", (int)RX_TRIM_SEL_TX);
		}

		if (efuse_readParamOps(EFUSE_DDR_RZQ_TRIM_DATA_1ST, &value) != E_OK) {
			// Apply default value
			pr_err("[2-1] data = NULL\n");
		} else {
			pr_info("\r\n");
			pr_info("[rzq_trim]=[0x%04x]\r\n", (int)value);
		}

		if (efuse_readParamOps(EFUSE_DDR_LDO_TRIM_DATA_2ND, &value) != E_OK) {
			// Apply default value
			pr_err("[2-2] data = NULL\n");
		} else {
			pr_info("[LDO]=[0x%04x]\r\n", (int)value);
		}

		if (efuse_readParamOps(EFUSE_HDMI_TRIM_DATA, &value_2[0]) != E_OK) {
			// Apply default value
			pr_err("[3] data = NULL\n");
		} else {
			pr_info("\r\n");
			pr_info("[3][6]HDMI = 0x%04x, 0x%04x(bit11)\r\n", (int)(value_2[0]), (int)(value_2[1]));

			HDMI_resistor = value_2[0] & 0x1F;
			HDMI_LANE_VL = (value_2[0] >> 5) | (value_2[1] << 11);
			HDMI_LANE0  = (HDMI_LANE_VL & 0x7);
			HDMI_LANE1  = ((HDMI_LANE_VL >> 3) & 0x7);
			HDMI_LANE2  = ((HDMI_LANE_VL >> 6) & 0x7);
			HDMI_CLK    = ((HDMI_LANE_VL >> 9) & 0x7);

			pr_info("[3][6][HDMI_Resistor]=[0x%04x]\r\n", (int)HDMI_resistor);
			pr_info("[3][6] [HDMI_LANE_VL]=[0x%04x]\r\n", (int)HDMI_LANE_VL);
			pr_info("[3][6]   [HDMI_LANE0]=[0x%04x]\r\n", (int)HDMI_LANE0);
			pr_info("[3][6]   [HDMI_LANE1]=[0x%04x]\r\n", (int)HDMI_LANE1);
			pr_info("[3][6]   [HDMI_LANE2]=[0x%04x]\r\n", (int)HDMI_LANE2);
			pr_info("[3][6]     [HDMI_CLK]=[0x%04x]\r\n", (int)HDMI_CLK);
		}

		if (efuse_readParamOps(EFUSE_USBC_TRIM_DATA_1ST, &value) != E_OK) {
			// Apply default value
			pr_err("[5-1] data = NULL\n");
		} else {
			pr_info("\r\n");
			pr_info("[5-1] USB = 0x%04x\r\n", (int)value);
			RINT_SEL = value;
			pr_info("[12K_TRIM]=[0x%04x]\r\n", (int)RINT_SEL);
		}

		if (efuse_readParamOps(EFUSE_THERMAL_TRIM_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[6] data = NULL\n");
		} else {
			pr_info("\r\n");
			pr_info("[6]Thermal = 0x%04x\r\n", (int)value);
			THERMAL = (value & 0x3FF);
			pr_info("[6]Thermal trim data = 0x%04x\r\n", (int)THERMAL);
		}

		if (efuse_readParamOps(EFUSE_CORE_POWER_ADJ_TRIM_DATA_1ST, &value) != E_OK) {
			// Apply default value
			pr_err("[7-1] data = NULL\n");
		} else {
			pr_info("\r\n");
			pr_info("[7-1]Core power adjuest = 0x%04x\r\n", (int)value);
			CORE_POWER_ADJ = value;
			pr_info("     Core power adjuest = [0x%04x]\r\n", (int)CORE_POWER_ADJ);
		}

		if (efuse_readParamOps(EFUSE_CORE_POWER_LEAKAGE_TRIM_DATA_2ND, &value) != E_OK) {
			// Apply default value
			pr_err("[7-2] data = NULL\n");
		} else {
			pr_info("[7-2]    Core power lek = 0x%04x\r\n", (int)value);
			CORE_POWER_LEAKAGE = (value >> 8);
			pr_info("[7-2]CORE_POWER_LEAKAGE = 0x%04x\r\n", (int)CORE_POWER_LEAKAGE);
		}

		if (efuse_readParamOps(EFUSE_SATA_TRIM_DATA, &value_2[0]) != E_OK) {
			// Apply default value
			pr_err("[1][5] data = NULL\n");
		} else {
			pr_info("\n");
			pr_info("[1][5] 0x%08x, 0x%08x\r\n", (int)(value_2[0]), (int)(value_2[1]));

			SATA_BIAS_VLDOSEL_PI    = (value_2[0]) & 0xF;
			SATA_BIAS_RX_VLDO_SEL   = (value_2[0] >> 4) & 0xF;
			SATA_BIAS_LDOSEL_PLL    = (value_2[0] >> 8) & 0xF;
			SATA_BIAS_VREF_SEL      = (value_2[0] >> 12) & 0xF;
			SATA_TX_ZTX_CTL         = (value_2[1]) & 0xF;
			pr_info("  [SATA_BIAS_VLDOSEL_PI]=[0x%04x]\r\n", (int)SATA_BIAS_VLDOSEL_PI);
			pr_info(" [SATA_BIAS_RX_VLDO_SEL]=[0x%04x]\r\n", (int)SATA_BIAS_RX_VLDO_SEL);
			pr_info("  [SATA_BIAS_LDOSEL_PLL]=[0x%04x]\r\n", (int)SATA_BIAS_LDOSEL_PLL);
			pr_info("    [SATA_BIAS_VREF_SEL]=[0x%04x]\r\n", (int)SATA_BIAS_VREF_SEL);
			pr_info("       [SATA_TX_ZTX_CTL]=[0x%04x]\r\n", (int)SATA_TX_ZTX_CTL);
		}

		if (efuse_readParamOps(EFUSE_CVBS_VGA_TRIM_DATA, &value) != E_OK) {
			// Apply default value
			pr_err("[9] data = NULL\n");
		} else {
			pr_info("\n");
			pr_info("[9]CVBS&VGA = 0x%04x\n", (int)value);
			CVBS = (value & 0x7F);
			VGA  = (value >> 7) & 0x7F;
			VGA |= 0x80;
			pr_info("   [CVBS_DA_TRIM]=[0x%04x]\n", (int)CVBS);
			pr_info("    [VGA_DA_TRIM]=[0x%04x]\n", (int)VGA);
		}
#endif
		return size;
	} else if (!strcmp(argv[0], "trimdump")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined(CONFIG_NVT_IVOT_PLAT_NS02302)  || defined(CONFIG_NVT_IVOT_PLAT_NS02301)
		otp_trim_dump();
#endif
	} else if (!strcmp(argv[0], "uniqueid")) {
		UINT32 h, l;
		h = 0;
		l = 0;
		if (efuse_get_unique_id(&l, &h) >= 0) {
			pr_info("unique ID[0x%08x][0x%08x] success\r\n", (int)h, (int)l);
		} else {
			pr_err("unique ID[0x%08x][0x%08x] error\r\n", (int)h, (int)l);
		}
#if defined(CONFIG_NVT_IVOT_PLAT_NS02201)
		if (efuse_get_unique_id_2(&l, &h) >= 0) {
			pr_info("unique ID2[0x%08x][0x%08x] success\r\n", (int)h, (int)l);
		} else {
			pr_err("unique ID2[0x%08x][0x%08x] error\r\n", (int)h, (int)l);
		}
#endif
	} else if (!strcmp(argv[0], "nvt_write_key")) {
		UINT8   pKey[16];
		INT32   result;
		UINT32  key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		if (!strcmp(argv[1], "0")) {
			key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
			memcpy((void *)pKey, (void *)key_sample3, 16);
		} else if (!strcmp(argv[1], "1")) {
			key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
			memcpy((void *)pKey, (void *)key_sample1, 16);
		} else if (!strcmp(argv[1], "2")) {
			key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
			memcpy((void *)pKey, (void *)key_sample2, 16);
		} else if (!strcmp(argv[1], "3")) {
			key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
			memcpy((void *)pKey, (void *)key_sample3, 16);
		} else if (!strcmp(argv[1], "4")) {
			key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
			memcpy((void *)pKey, (void *)key_sample_crypto, 16);
		} else {
			pr_info(" => fail keyset from 0~4 currect = %s\r\n", argv[1]);
			return size;
		}
		pr_info("nvt_write_key => [%s]", argv[1]);
		result = otp_write_key(key_set, pKey);
		if (result < 0) {
			pr_info(" => fail [%d] \r\n", result);
			return size;
		} else {
			pr_info(" => success\r\n");
		}

	} else if (!strcmp(argv[0], "nvt_read_key")) {
		UINT8       pKey[16];
		INT32       result;
		UINT32  key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		if (!strcmp(argv[1], "0")) {
			key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "1")) {
			key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "2")) {
			key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "3")) {
			key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "4")) {
			key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
		}
		pr_info("nvt_read_key => [%s]", argv[1]);

		result = otp_read_key(key_set, pKey);
		if (result < 0) {
			pr_info(" => fail [%d] \r\n", result);
			return size;
		} else {
			pr_info(" => success\r\n");
			pr_info(" => value:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
					pKey[0], pKey[1], pKey[2], pKey[3],
					pKey[4], pKey[5], pKey[6], pKey[7],
					pKey[8], pKey[9], pKey[10], pKey[11],
					pKey[12], pKey[13], pKey[14], pKey[15]);
		}
	} else if (!strcmp(argv[0], "nvt_read_key_lock")) {
		INT32   result;
		UINT32  key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		if (!strcmp(argv[1], "0")) {
			key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "1")) {
			key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "2")) {
			key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "3")) {
			key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "4")) {
			key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
		}
		pr_info("nvt_read_key_lock => [%s]", argv[1]);

		result = otp_set_key_read_lock(key_set);
		if (result < 0) {
			pr_info(" => fail [%d] \r\n", result);
			return size;
		} else {
			pr_info(" => success\r\n");
		}
	} else if (!strcmp(argv[0], "version")) {
		otp_version();
	} else if (!strcmp(argv[0], "jtag_disable")) {
		if (is_JTAG_DISABLE_en() == TRUE) {
			pr_info("Disable JTAG success\r\n");
		} else {
			pr_info("Disable JTAG fail\r\n");
		}
	} else if (!strcmp(argv[0], "secure_enable")) {
		if (otp_secure_en() == TRUE) {
			pr_info("Secure enable success\r\n");
		} else {
			pr_info("Secure enable fail\r\n");
		}
	} else if (!strcmp(argv[0], "data_area_encrypt")) {
		if (otp_data_area_encrypt_en() == TRUE) {
			pr_info("Data area encrypt enable success\r\n");
		} else {
			pr_info("Data area encrypt enable fail\r\n");
		}
	} else if (!strcmp(argv[0], "signature_rsa_enable")) {
		if (otp_signature_rsa_en() == TRUE) {
			pr_info("RSA signature enable success\r\n");
		} else {
			pr_info("RSA signature enable fail\r\n");
		}
	} else if (!strcmp(argv[0], "signature_rsa_chksum_enable")) {
		if (otp_signature_rsa_chksum_en() == TRUE) {
			pr_info("RSA signature checksum enable success\r\n");
		} else {
			pr_info("RSA signature checksum enable fail\r\n");
		}
	} else if (!strcmp(argv[0], "rsapss_en")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
		if (nvt_get_chip_id() == CHIP_NS02402) {
			pr_info("=>[rsa_pss(pkcs2.1) enable]\r\n");
			efuse_signature_rsapss_en();
			pr_info("is_signature_rsapss_enable()=%d\r\n", is_signature_rsapss());
		}
#endif
	}  else if (!strcmp(argv[0], "rsapkcs1p5_en")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
		if (nvt_get_chip_id() == CHIP_NS02402) {
			pr_info("=>[rsa_pkcs1.5 enable]\r\n");
			efuse_signature_rsapss_en();
			pr_info("is_signature_rsa_chsum_enable()=%d\r\n", is_signature_rsapkcs1p5());
		}
#endif
	}  else if (!strcmp(argv[0], "header_signature_en")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
		if (nvt_get_chip_id() == CHIP_NS02402) {
			pr_info("=>[header signature enable]\r\n");
			efuse_header_signature_en();
			pr_info("is_header_signature_en()=%d\r\n", is_header_signature_en());
		}
#endif
	}  else if (!strcmp(argv[0], "venc_overclocking_enable")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51090)
		if (enable_secure_boot(SECUREBOOT_VENC_OVER_CLOCKING) == TRUE) {
			pr_info("venc overclocking enable success\r\n");
		} else {
			pr_info("venc overclocking enable fail\r\n");
		}
#endif
	} else if (!strcmp(argv[0], "vdec_overclocking_enable")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51090)
		if (enable_secure_boot(SECUREBOOT_VDEC_OVER_CLOCKING) == TRUE) {
			pr_info("vdec overclocking enable success\r\n");
		} else {
			pr_info("vdec overclocking enable fail\r\n");
		}
#endif
	} else if (!strcmp(argv[0], "trigger_key_set")) {
		UINT32  key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		void __iomem *IOADDR_CRYPTO_REG_BASE;
		if (!strcmp(argv[1], "0")) {
			key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "1")) {
			key_set = EFUSE_OTP_2ND_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "2")) {
			key_set = EFUSE_OTP_3RD_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "3")) {
			key_set = EFUSE_OTP_4TH_KEY_SET_FIELD;
		} else if (!strcmp(argv[1], "4")) {
			key_set = EFUSE_OTP_5TH_KEY_SET_FIELD;
		}
		pr_info("encrypt_key_set => @key [%s]", argv[1]);

		trigger_efuse_key(EFUSE_KEY_MANAGER_CRYPTO, key_set * 4, 4);
		if (!IOADDR_CRYPTO_REG_BASE) {
#if defined(CONFIG_NVT_IVOT_PLAT_NS02201)
			IOADDR_CRYPTO_REG_BASE = ioremap(0x2F0250000, 0x100);
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NS02301)||defined(CONFIG_NVT_IVOT_PLAT_NA51103)
			IOADDR_CRYPTO_REG_BASE = ioremap(NVT_SCE_BASE_PHYS, 0x100);
#endif
		}
		pr_info("0x10 0x%08x 0x%08x 0x%08x 0x%08x\r\n", nvt_readl(IOADDR_CRYPTO_REG_BASE + 0x10), nvt_readl(IOADDR_CRYPTO_REG_BASE + 0x14), nvt_readl(IOADDR_CRYPTO_REG_BASE + 0x18), nvt_readl(IOADDR_CRYPTO_REG_BASE + 0x1C));
		pr_info("0x20 0x%08x 0x%08x 0x%08x 0x%08x\r\n", nvt_readl(IOADDR_CRYPTO_REG_BASE + 0x20), nvt_readl(IOADDR_CRYPTO_REG_BASE + 0x24), nvt_readl(IOADDR_CRYPTO_REG_BASE + 0x28), nvt_readl(IOADDR_CRYPTO_REG_BASE + 0x2C));
	} else if (!strcmp(argv[0], "quary")) {
		pr_info("=>[quary]\r\n");
		pr_info("             is_secure_enable()=%d\r\n", is_secure_enable());
		pr_info("       is_data_area_encrypted()=%d\r\n", is_data_area_encrypted());
		pr_info("             is_signature_rsa()=%d\r\n", is_signature_rsa());
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302)
		if (nvt_get_chip_id() == CHIP_NS02402) {
			pr_info("          is_signature_rsapss()=%d\r\n", is_signature_rsapss());
			pr_info("      is_signature_rsapkcs1p5()=%d\r\n", is_signature_rsapkcs1p5());
			pr_info("       is_header_signature_en()=%d\r\n", is_header_signature_en());
		}
#endif
#if defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NS02301)
		pr_info("           is_signature_ecdsa()=%d\r\n", is_signature_ecdsa());
		pr_info("    is_signature_chsum_enable()=%d\r\n", is_signature_rsa_chsum_enable());
#else
		pr_info("is_signature_rsa_chsum_enable()=%d\r\n", is_signature_rsa_chsum_enable());
#endif
		pr_info("           is_JTAG_DISABLE_en()=%d\r\n", is_JTAG_DISABLE_en());
		pr_info("        is_1st_key_programmed()=%d\r\n", is_1st_key_programmed());
		pr_info("        is_2nd_key_programmed()=%d\r\n", is_2nd_key_programmed());
		pr_info("        is_3rd_key_programmed()=%d\r\n", is_3rd_key_programmed());
		pr_info("        is_4th_key_programmed()=%d\r\n", is_4th_key_programmed());
		pr_info("        is_5th_key_programmed()=%d\r\n", is_5th_key_programmed());
		pr_info("         is_1st_key_read_lock()=%d\r\n", is_1st_key_read_lock());
		pr_info("         is_2nd_key_read_lock()=%d\r\n", is_2nd_key_read_lock());
		pr_info("         is_3rd_key_read_lock()=%d\r\n", is_3rd_key_read_lock());
		pr_info("         is_4th_key_read_lock()=%d\r\n", is_4th_key_read_lock());
		pr_info("         is_5th_key_read_lock()=%d\r\n", is_5th_key_read_lock());
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102)
		pr_info("              is_hdmi_available()=%d\r\n", is_hdmi_available());
		pr_info("               is_dsp_available()=%d\r\n", is_dsp_available());
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02201)
		pr_info("              is_hdmi_disable()=%d\r\n", is_hdmi_disable());
		pr_info("              is_dsp1_disable()=%d\r\n", is_DSP1_disable());
		pr_info("              is_dsp2_disable()=%d\r\n", is_DSP2_disable());
		pr_info("               is_mcu_disable()=%d\r\n", is_MCU_disable());
		pr_info("               is_GPU_disable()=%d\r\n", is_GPU_disable());
#endif
#if defined(CONFIG_NVT_IVOT_PLAT_NA51090)
		pr_info("        is_VDEC_OVER_CLOCKING()=%d\r\n", quary_secure_boot(SECUREBOOT_VDEC_OVER_CLOCKING));
		pr_info("        is_VENC_OVER_CLOCKING()=%d\r\n", quary_secure_boot(SECUREBOOT_VENC_OVER_CLOCKING));
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
		pr_info("         is_CPU_OVER_CLOCKING()=%d\r\n", quary_secure_boot(SECUREBOOT_CPU_OVER_CLOCKING));
		pr_info("        is_VENC_OVER_CLOCKING()=%d\r\n", quary_secure_boot(SECUREBOOT_VENC_OVER_CLOCKING));
		pr_info("        is_CONV_OVER_CLOCKING()=%d\r\n", quary_secure_boot(SECUREBOOT_CONV_OVER_CLOCKING));
		pr_info("        is_DRAM_OVER_CLOCKING()=%d\r\n", quary_secure_boot(SECUREBOOT_DDR_OVER_CLOCKING));
#endif

	} else {
		pr_info("\nUsage\n");
		pr_info("\necho [command] > otp_trim\n\n");
		pr_info("[command]    =>\n");
		pr_info("             => trim\n");
		pr_info("             => uniqueid (chip's unique ID)\n");
		pr_info("             =>     nvt_write_key No (write specific key into specific key set field)\n");
		pr_info("             =>      nvt_read_key No ( read specific key from specific key set field)\n");
		pr_info("             =>   trigger_key_set No (                         trigger key set field)\n");
		pr_info("             => nvt_read_key_lock No (Read lock specific key set field => can not readable)\n");
		pr_info("             =>               jtag_disable (Disable JTAG interface persistent)\n");
		pr_info("             =>              secure_enable (ROM treat loader as secure enable)\n");
		pr_info("             =>           data_area_encrypt(ROM treat loader data area as cypher text)\n");
		pr_info("             =>        signature_rsa_enable(ROM treat loader by use RSA as loader's signature)o	\n");
		pr_info("             => signature_rsa_chksum_enable(Once use RSA as signature, enable RSA public checksum(use SHA256) enable)\n");
		pr_info("             => quary(Query all option)\n");
		pr_info("             => version (knlPkg.a version)\n");
	}
ERR_OUT:

	return size;
}

#if defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined(CONFIG_NVT_IVOT_PLAT_NS02302)
static void __iomem *IOADDR_TIMER_REG_BASE;
#define CALCULATE_CPU_FREQ_UNIT_US      5000
#define      timer_gettick()            readl(IOADDR_TIMER_REG_BASE + 0x108)
static void enable_cpu_counters(void)
{
	u64 val;
	/* Disable cycle counter overflow interrupt */
	asm volatile("msr pmintenset_el1, %0" : : "r"((u64)(0 << 31)));
	/* Enable cycle counter */
	asm volatile("msr pmcntenset_el0, %0" :: "r" BIT(31));
	/* Enable user-mode access to cycle counters. */
	asm volatile("msr pmuserenr_el0, %0" : : "r"(BIT(0) | BIT(2)));
	/* Clear cycle counter and start */
	asm volatile("mrs %0, pmcr_el0" : "=r"(val));
	val |= (BIT(0) | BIT(2));
	isb();
	asm volatile("msr pmcr_el0, %0" : : "r"(val));
	val = BIT(27);
	asm volatile("msr pmccfiltr_el0, %0" : : "r"(val));
}

static void disable_cpu_counters(void)
{
	/* Disable cycle counter */
	asm volatile("msr pmcntenset_el0, %0" :: "r"(0 << 31));
	/* Disable user-mode access to counters. */
	asm volatile("msr pmuserenr_el0, %0" : : "r"((u64)0));
}

//static BOOL cpu_count_open = FALSE;
void timer2_delay(u32 us)
{
	u32 start, end;
	start = timer_gettick();
	/*check timer count to target level*/
	while (1) {
		end = timer_gettick();
		if ((end - start) > us) {
			break;
		}
	}
}

static void ca53_cycle_count_start(BOOL do_reset, BOOL enable_divider)
{
	enable_cpu_counters();
	return;
}

/**
    CA53 get CPU clock cycle count

    get CPU clock cycle count

    @param[out] type
    @return success or not
        - @b UINT64:   clock cycle of CPU
*/
static u64 ca53_get_cycle_count(void)
{
	u64 cval;
	isb();
	asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(cval));
	return cval;
}

static void ca53_cycle_count_stop(void)
{
	disable_cpu_counters();
	return;
}

static int do_nvt_cpu_get_freq(void)
{
	UINT64 time_1, time_2, temp, time_interval;
	u32 freq;
	unsigned long spin_flags;
	if (!IOADDR_TIMER_REG_BASE) {
		IOADDR_TIMER_REG_BASE = ioremap(NVT_TIMER_BASE_PHYS, 0x200);
	}
	spin_flags = otp_platform_spin_lock();
	ca53_cycle_count_start(TRUE, FALSE);
	time_1 = ca53_get_cycle_count();
	timer2_delay(CALCULATE_CPU_FREQ_UNIT_US);
	time_2 = ca53_get_cycle_count();
	if (time_2 > time_1) {
		time_interval = (time_2 - time_1);
	} else {
		temp = 0xFFFFFFFF - time_1;
		time_interval = temp + time_2;
	}
	freq = (time_interval) / (CALCULATE_CPU_FREQ_UNIT_US);
	ca53_cycle_count_stop();
	otp_platform_spin_unlock(spin_flags);
	iounmap((volatile void __iomem *)IOADDR_TIMER_REG_BASE);
	IOADDR_TIMER_REG_BASE = NULL;
#if defined(CONFIG_NVT_IVOT_PLAT_NS02201)
	pr_info("CHIP[NS02201] => CPU Freq %d MHz\n", freq);
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	pr_info("CHIP[NS02302] => CPU Freq %d MHz\n", freq);
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51102)
	pr_info("CHIP[NA51102] => CPU Freq %d MHz\n", freq);
#endif
	return 0;
}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02301)
static void __iomem *IOADDR_TIMER_REG_BASE;
#define CALCULATE_CPU_FREQ_UNIT_US      5000
#define      timer_gettick()            readl(IOADDR_TIMER_REG_BASE + 0x108)

#define read_PMCR() \
	({ \
		unsigned long cfg; \
		__asm__ __volatile__(\
							 "mrc p15, 0, %0, c9, c12, 0\n\t" \
							 : "=r"(cfg) \
							);\
		cfg;\
	})

#define write_PMCR(m)\
	({\
		__asm__ __volatile__("mcr p15, 0, %0, c9, c12, 0" : : "r" (m));\
	})

static BOOL cpu_count_open = FALSE;
void timer2_delay(u32 us)
{
	u32 start, end;
	start = timer_gettick();
	/*check timer count to target level*/
	while (1) {
		end = timer_gettick();
		if ((end - start) > us) {
			break;
		}
	}
}

void ca53_cycle_count_start(BOOL do_reset, BOOL enable_divider)
{
	/* in general enable all counters (including cycle counter)*/
	int value = 1;

	if (cpu_count_open == TRUE) {
		pr_err("cpu count start already\r\n");
		return;
	}

	cpu_count_open = TRUE;

	if (do_reset) {
		value |= 2;     /* reset all counters to zero.*/
		value |= 4;     /* reset cycle counter to zero.*/
	}

	if (enable_divider) {
		value |= 8;    /* enable "by 64" divider for CCNT.*/
	}

	value |= 16;

	/* program the performance-counter control-register: */
	asm volatile("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));

	/* enable all counters: */
	asm volatile("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));

	/* clear overflows: */
	asm volatile("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
	return;
}

/**
    CA53 get CPU clock cycle count

    get CPU clock cycle count

    @param[out] type
    @return success or not
        - @b UINT64:   clock cycle of CPU
*/
u32 ca53_get_cycle_count(void)
{
	unsigned int value;
	__asm__ __volatile__("mrc p15, 0, %0, c9, c13, 0\n\t"
						 : "=r"(value)
						);
	return value;
}

void ca53_cycle_count_stop(void)
{
	u32 pmcr_reg;

	if (cpu_count_open == FALSE) {
		pr_err("cpu count not start yet\r\n");
		return;
	}

	cpu_count_open = FALSE;
	pmcr_reg = read_PMCR();
	pmcr_reg &= ~(0x1 << 0); /*E*/
	pmcr_reg &= ~(0x1 << 5); /*DP increase each clock cycle*/

	write_PMCR(pmcr_reg);
	return;
}

static int do_nvt_cpu_get_freq(void)
{
	u32 /*test, */freq;
	u32 time_1, time_2, temp, time_interval;
	unsigned long spin_flags;

	if (!IOADDR_TIMER_REG_BASE) {
		IOADDR_TIMER_REG_BASE = ioremap(NVT_TIMER_BASE_PHYS, 0x200);
	}

	spin_flags = otp_platform_spin_lock();
	ca53_cycle_count_start(TRUE, FALSE);
	//for (test = 1; test <= 1; test ++) {
	time_1 = ca53_get_cycle_count();
	timer2_delay(CALCULATE_CPU_FREQ_UNIT_US);
	time_2 = ca53_get_cycle_count();

	if (time_2 > time_1) {
		time_interval = (time_2 - time_1);
	} else {
		temp = 0xFFFFFFFF - time_1;
		time_interval = temp + time_2;
	}

	freq = (time_interval) / (CALCULATE_CPU_FREQ_UNIT_US);
	//}
	ca53_cycle_count_stop();
	otp_platform_spin_unlock(spin_flags);
	iounmap((volatile void __iomem *)IOADDR_TIMER_REG_BASE);
	IOADDR_TIMER_REG_BASE = NULL;

#if defined(CONFIG_NVT_IVOT_PLAT_NS02301)
	pr_info("CHIP[NS02301] => CPU Freq %d MHz\n", freq);
#endif

	return 0;
}

#endif

static struct proc_ops proc_otp_fops = {
	.proc_open      = nvt_otp_proc_help_open,
	.proc_release   = single_release,
	.proc_read      = nvt_otp_proc_cmd_read,
	.proc_lseek     = seq_lseek,
	.proc_write     = nvt_otp_proc_cmd_write
};


static int nvt_avl_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nUsage\n");
	seq_printf(sfile, "\necho [command] > avl\n\n");
	seq_printf(sfile, "[command]    =>\n");
	seq_printf(sfile, "             => showavl (show available list)\n");
	seq_printf(sfile, "             => version (knlPkg.a version)\n");
#if defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined(CONFIG_NVT_IVOT_PLAT_NA51102)
	seq_printf(sfile, "             => cpufreq\n");
#endif
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
	seq_printf(sfile, "             => nextpkginit\n");
	seq_printf(sfile, "             => nextpkg\n");
#endif
	return 0;
}

static int nvt_avl_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_avl_proc_show, NULL);
}

static ssize_t nvt_avl_proc_cmd_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	/*check command length*/
	if ((!len) || (len > (MAX_CMD_LENGTH - 1))) {
		pr_err("Command length is too long or 0!\n");
		goto ERR_OUT;
	}

	/*copy command string from user space*/
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	printk("CMD:%s\n", cmd_line);

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (ucargc < 1) {
		pr_err("NULL command error\n");
		goto ERR_OUT;
	}

	if (!strcmp(argv[0], "showavl")) {
		cmd_efuse_show_avl();
		return size;
	} else if (!strcmp(argv[0], "version")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51089)
		otp_version();
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
		otp_version();
#else
		pr_info("knlpkg.a 1.00.007\r\n");
#endif
	} else if (!strcmp(argv[0], "cpufreq")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NS02301)
		do_nvt_cpu_get_freq();
#endif
	} else if (!strcmp(argv[0], "nextpkg")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
		if(otp_set_next_pkg_uid_test() < 0) {
			printk("<FINISH> SIM PKGUID NULL\n");
		} else {
			cmd_efuse_show_avl();
			printk("<OK> press reboot ... \n");
		}
#endif
	} else if (!strcmp(argv[0], "nextpkginit")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
		otp_next_pkg_uid_test_init();
#endif
	}
ERR_OUT:

	return size;
}

static struct proc_ops proc_avl_fops = {
	.proc_open      = nvt_avl_proc_help_open,
	.proc_release   = single_release,
	.proc_read      = seq_read,
	.proc_lseek     = seq_lseek,
	.proc_write     = nvt_avl_proc_cmd_write
};

int nvt_otp_proc_init(POTP_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("nvt_otp_op", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}

	pdrv_info->pproc_otp_root = pmodule_root;


	pentry = proc_create("otp_trim", S_IRUGO | S_IXUGO, pmodule_root, &proc_otp_fops);

	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc otp!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_otp_entry = pentry;

	pentry = proc_create("avl", S_IRUGO | S_IXUGO, pmodule_root, &proc_avl_fops);

	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc avl!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_avl_entry = pentry;

	return ret;

remove_cmd:
	proc_remove(pdrv_info->pproc_otp_entry);
	proc_remove(pdrv_info->pproc_avl_entry);

remove_root:
	proc_remove(pdrv_info->pproc_otp_root);
	return ret;
}

int nvt_otp_proc_remove(POTP_DRV_INFO pdrv_info)
{
	return 0;
}
