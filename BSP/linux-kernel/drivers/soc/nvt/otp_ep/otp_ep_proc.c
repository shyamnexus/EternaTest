#include <linux/random.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "otp_ep_proc.h"
#include "otp_ep_dbg.h"
#include "otp_ep_main.h"
#include <plat/efuse_protected.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/scatterlist.h>
#include <crypto/skcipher.h>
#if defined (__KERNEL__)
#include <linux/version.h>
#endif

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

#define DBG_TEST_EN	(0)

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
POTP_DRV_INFO pdrv_otp_ep_info_data;

//============================================================================
// Function define
//============================================================================
static int nvt_otp_ep_proc_help_open(struct inode *inode, struct file *file);

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static int nvt_otp_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nUsage\n");
	seq_printf(sfile, "\necho [command] > efuse_op\n\n");
	seq_printf(sfile, "[command]    =>\n");
	seq_printf(sfile, "             => trim (driver's trim data)\n");
	seq_printf(sfile, "             => version (knlPkg.a version)\n");
	return 0;
}

static int nvt_otp_ep_proc_help_open(struct inode *inode, struct file *file)
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
#define OTP_KEY_MANAGER_IDENTIFY_OFS			4
#define OTP_KEY_MANAGER_HDR_SIZE				8
const unsigned char OTP_EP_KEY_MANAGER_HDR[OTP_KEY_MANAGER_HDR_SIZE] = {0x65, 0x6B, 0x65, 0x79, 0x00, 0x00, 0x00, 0x00}; ///< byte[0...3] as magic tag, byte[4] as key offset (word unit)


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
		if (efuse_read_param_ops_ep(0, EFUSE_USB2_TRIM_DATA, &data) != E_OK) {
			pr_err("[0] data = NULL\n");
		} else {
			pr_info("[0] data = 0x%08x\r\n", data);
		}
		if (efuse_read_param_ops_ep(0, EFUSE_USBC_TRIM_DATA, &data) != E_OK) {
			pr_err("[1] data = NULL\n");
		} else {
			pr_info("[1] data = 0x%08x\r\n", data);
		}
		if (efuse_read_param_ops_ep(0, EFUSE_DDR_H_TRIM_DATA, &data) != E_OK) {
			pr_err("[2] data = NULL\n");
		} else {
			pr_info("[2] data = 0x%08x\r\n", data);
		}
		if (efuse_read_param_ops_ep(0, EFUSE_DDR_V_TRIM_DATA, &data) != E_OK) {
			pr_err("[3] data = NULL\n");
		} else {
			pr_info("[3] data = 0x%08x\r\n", data);
		}
		if (efuse_read_param_ops_ep(0, EFUSE_VER_PKG_UID, &data) != E_OK) {
			pr_err("[4] data = NULL\n");
		} else {
			pr_info("[4] data = 0x%08x\r\n", data);
		}

		if (efuse_read_param_ops_ep(0, EFUSE_HDMI_TRIM_DATA, &data) != E_OK) {
			pr_err("[5-1] data = NULL\n");
		} else {
			pr_info("[5-1] data = 0x%08x\r\n", data);
		}

		if (efuse_read_param_ops_ep(0, EFUSE_PCIE_REFCLK_TRIM_DATA, &data) != E_OK) {
			pr_err("[5-2] data = NULL\n");
		} else {
			pr_info("[5-2] data = 0x%08x\r\n", data);
		}

		if (efuse_read_param_ops_ep(0, EFUSE_THERMAL_TRIM_DATA, &data) != E_OK) {
			pr_err("[6] data = NULL\n");
		} else {
			pr_info("[6] data = 0x%08x\r\n", data);
		}

		if (efuse_read_param_ops_ep(0, EFUSE_COREPOWER_LEAKAGE_TRIM_DATA, &data) != E_OK) {
			pr_err("[7] data = NULL\n");
		} else {
			pr_info("[7] data = 0x%08x\r\n", data);
		}

		if (efuse_read_param_ops_ep(0, EFUSE_CVBS_VGA_TRIM_DATA, &data) != E_OK) {
			pr_err("[9] data = NULL\n");
		} else {
			pr_info("[9] data = 0x%08x\r\n", data);
		}

#endif
		return size;
	} else if (!strcmp(argv[0], "uniqueid")) {
		UINT32 h, l;
		h = 0;
		l = 0;
		if (efuse_get_unique_id_ep(0, &l, &h) >= 0)
			pr_info("unique ID ep0[0x%08x][0x%08x] success\r\n", (int)h, (int)l);
		else
			pr_err("unique ID ep0[0x%08x][0x%08x] error\r\n", (int)h, (int)l);
	} else if (!strcmp(argv[0], "nvt_write_key")) {
		UINT8  	pKey[16];
		INT32	result;
		UINT32	key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
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
		}
		pr_info("nvt_write_key => [%s]", argv[1]);
		result = otp_write_key_ep(0, key_set, pKey);
		if (result < 0) {
			pr_info(" => fail [%d] \r\n", result);
			return size;
		} else {
			pr_info(" => success\r\n");
		}

	} else if (!strcmp(argv[0], "nvt_read_key")) {
		UINT8  		pKey[16];
		INT32		result;
		UINT32	key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
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

		result = otp_read_key_ep(0, key_set, pKey);
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
		INT32	result;
		UINT32	key_set = EFUSE_OTP_1ST_KEY_SET_FIELD;
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
		pr_info("nvt_read_key_lock_ep => [%s]", argv[1]);

		result = otp_set_key_read_lock_ep(0, key_set);
		if (result < 0) {
			pr_info(" => fail [%d] \r\n", result);
			return size;
		} else {
			pr_info(" => success\r\n");
		}
	} else if (!strcmp(argv[0], "quary")) {
		pr_info("=>[quary]\r\n");
		pr_info("             is_secure_enable_ep()=%d\r\n", is_secure_enable_ep(0));
		pr_info("       is_data_area_encrypted_ep()=%d\r\n", is_data_area_encrypted_ep(0));
		pr_info("             is_signature_rsa_ep()=%d\r\n", is_signature_rsa_ep(0));
		pr_info("is_signature_rsa_chsum_enable_ep()=%d\r\n", is_signature_rsa_chsum_enable_ep(0));
		pr_info("           is_JTAG_DISABLE_en_ep()=%d\r\n", is_JTAG_DISABLE_en_ep(0));
		pr_info("        is_1st_key_programmed_ep()=%d\r\n", is_1st_key_programmed_ep(0));
		pr_info("        is_2nd_key_programmed_ep()=%d\r\n", is_2nd_key_programmed_ep(0));
		pr_info("        is_3rd_key_programmed_ep()=%d\r\n", is_3rd_key_programmed_ep(0));
		pr_info("        is_4th_key_programmed_ep()=%d\r\n", is_4th_key_programmed_ep(0));
		pr_info("        is_5th_key_programmed_ep()=%d\r\n", is_5th_key_programmed_ep(0));
		pr_info("         is_1st_key_read_lock_ep()=%d\r\n", is_1st_key_read_lock_ep(0));
		pr_info("         is_2nd_key_read_lock_ep()=%d\r\n", is_2nd_key_read_lock_ep(0));
		pr_info("         is_3rd_key_read_lock_ep()=%d\r\n", is_3rd_key_read_lock_ep(0));
		pr_info("         is_4th_key_read_lock_ep()=%d\r\n", is_4th_key_read_lock_ep(0));
		pr_info("         is_5th_key_read_lock_ep()=%d\r\n", is_5th_key_read_lock_ep(0));
	} else if (!strcmp(argv[0], "version")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51089)
		otp_version();
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
		otp_version_ep();
#else
		pr_info("knlpkg.a 1.00.006\r\n");
#endif
	} else {
		pr_info("\nUsage\n");
		pr_info("\necho [command] > otp_trim\n\n");
		pr_info("[command]    =>\n");
		pr_info("             => trim\n");
		pr_info("             => nvt_write_key No (write specific key into specific key set field)\n");
		pr_info("             => nvt_read_key No ( read specific key from specific key set field)\n");
		pr_info("             => nvt_read_key_lock No (Read lock specific key set field => can not readable)\n");
		pr_info("             => uniqueid (chip's unique ID)\n");
		pr_info("             => quary(Query all option)\n");
		pr_info("             => version (knlPkg.a version)\n");
	}
ERR_OUT:

	return size;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_otp_fops = {
	.proc_open   	= nvt_otp_ep_proc_help_open,
	.proc_release 	= single_release,
	.proc_read   	= nvt_otp_proc_cmd_read,
	.proc_lseek 	= seq_lseek,
	.proc_write   	= nvt_otp_proc_cmd_write
};
#else
static struct file_operations proc_otp_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_otp_ep_proc_help_open,
	.release = single_release,
	.read   = nvt_otp_proc_cmd_read,
	.llseek = seq_lseek,
	.write   = nvt_otp_proc_cmd_write
};
#endif

static int nvt_avl_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nUsage\n");
	seq_printf(sfile, "\necho [command] > avl\n\n");
	seq_printf(sfile, "[command]    =>\n");
	seq_printf(sfile, "             => showavl (show available list)\n");
	seq_printf(sfile, "             => version (knlPkg.a version)\n");
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
		cmd_efuse_show_avl_ep(0);
		return size;
	} else if (!strcmp(argv[0], "version")) {
#if defined(CONFIG_NVT_IVOT_PLAT_NA51089)
		otp_version();
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
		otp_version_ep();
#else
		pr_info("knlpkg.a 1.00.007\r\n");
#endif
	}
ERR_OUT:

	return size;
}
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_avl_fops = {
	.proc_open   	= nvt_avl_proc_help_open,
	.proc_release 	= single_release,
	.proc_read   	= seq_read,
	.proc_lseek 	= seq_lseek,
	.proc_write   	= nvt_avl_proc_cmd_write
};
#else
static struct file_operations proc_avl_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_avl_proc_help_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
	.write   = nvt_avl_proc_cmd_write
};
#endif

int nvt_otp_ep_proc_init(POTP_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("nvt_otp_ep_op", NULL);
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
//#if 1
	pentry = proc_create("avl", S_IRUGO | S_IXUGO, pmodule_root, &proc_avl_fops);

	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc avl!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_avl_entry = pentry;
//#endif
	return ret;

remove_cmd:
	proc_remove(pdrv_info->pproc_otp_entry);
	proc_remove(pdrv_info->pproc_avl_entry);

remove_root:
	proc_remove(pdrv_info->pproc_otp_root);
	return ret;
}

int nvt_otp_ep_proc_remove(POTP_DRV_INFO pdrv_info)
{
	return 0;
}
