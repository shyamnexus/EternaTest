#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h> 



#include "hdmitx_proc.h"
#include "hdmitx_dbg.h"
#include "hdmitx_main.h"
#include "hdmitx_api.h"
#include "hdmitx.h"
#include "hdmitx_int.h"


//#include "../include/hdmitx.h"

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PHDMITX_MODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
PHDMITX_DRV_INFO phdmitx_drv_info_data;

//============================================================================
// Function define
//============================================================================


//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static PROC_CMD cmd_read_list[] = {
	// keyword          function name
	{ "reg",            nvt_hdmitx_api_read_reg           },
};

#define NUM_OF_READ_CMD (sizeof(cmd_read_list) / sizeof(PROC_CMD))

static PROC_CMD cmd_write_list[] = {
	// keyword          function name
	{ "reg",            nvt_hdmitx_api_write_reg          },
//	{ "pattern",        nvt_hdmitx_api_write_pattern      }
};

#define NUM_OF_WRITE_CMD (sizeof(cmd_write_list) / sizeof(PROC_CMD))

static int nvt_hdmitx_proc_cmd_show(struct seq_file *sfile, void *v)
{
	nvt_dbg(IND, "\n");

	return 0;
}

static int nvt_hdmitx_proc_cmd_open(struct inode *inode, struct file *file)
{
	nvt_dbg(IND, "\n");

	return single_open(file, nvt_hdmitx_proc_cmd_show, &phdmitx_drv_info_data->module_info);
}

static ssize_t nvt_hdmitx_proc_cmd_write(struct file *file, const char __user *buf,
									   size_t size, loff_t *off)
{
	int len = size;
	int ret = -EINVAL;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned char loop;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		nvt_dbg(ERR, "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	nvt_dbg(IND, "CMD:%s\n", cmd_line);

	// parse command string
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	// dispatch command handler
	if (strncmp(argv[0], "r", 2) == 0) {
		for (loop = 0 ; loop < NUM_OF_READ_CMD; loop++) {
			if (strncmp(argv[1], cmd_read_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
				ret = cmd_read_list[loop].execute(&phdmitx_drv_info_data->module_info, ucargc - 2, &argv[2]);
				break;
			}
		}
		if (loop >= NUM_OF_READ_CMD) {
			goto ERR_INVALID_CMD;
		}

	} else if (strncmp(argv[0], "w", 2) == 0)  {

		for (loop = 0 ; loop < NUM_OF_WRITE_CMD ; loop++) {
			if (strncmp(argv[1], cmd_write_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
				ret = cmd_write_list[loop].execute(&phdmitx_drv_info_data->module_info, ucargc - 2, &argv[2]);
				break;
			}
		}

		if (loop >= NUM_OF_WRITE_CMD) {
			goto ERR_INVALID_CMD;
		}

	} else {
		goto ERR_INVALID_CMD;
	}

	return size;

ERR_INVALID_CMD:
	nvt_dbg(ERR, "Invalid CMD \"%s\"\n", cmd_line);

ERR_OUT:
	return -1;
}

static struct file_operations proc_cmd_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_hdmitx_proc_cmd_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
	.write   = nvt_hdmitx_proc_cmd_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_hdmitx_proc_help_show(struct seq_file *sfile, void *v)
{
	int reg;
	UINT32 raw_len,i;
	UINT8 *data; 


	hdmitx_api("%s\n", __func__);

	if (hdmitx_check_hotplug()) {
		HDMI_VDOABI vdoAbi[32] = {0};
		UINT32 i,len;

		len = 32;
		hdmitx_get_video_ability(&len, vdoAbi);
		for (i = 0; i < len; i++) {
			seq_printf(sfile, "VideoAbility[%d] ID=%d 3D_Flag=0x%08X\n",i+1,vdoAbi[i].video_id,vdoAbi[i].vdo_3d_ability);
		}
		seq_printf(sfile, "HDMI_EDID_INFO = 0x%08X\n",hdmitx_get_edid_info());
	}

	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, "HDMITX[0] REG (0x%p)::\n", phdmitx_drv_info_data->module_info.io_addr[0]);
	for (reg = 0x00000; reg < 0x00180; reg += 16) {
		seq_printf(sfile, "0x%03X: 0x%08X 0x%08X 0x%08X 0x%08X\n", reg&0x1FF, nvt_hdmitx_drv_read_reg(&(phdmitx_drv_info_data->module_info), reg)
			, nvt_hdmitx_drv_read_reg(&(phdmitx_drv_info_data->module_info), reg+4), nvt_hdmitx_drv_read_reg(&(phdmitx_drv_info_data->module_info), reg+8),
			nvt_hdmitx_drv_read_reg(&(phdmitx_drv_info_data->module_info), reg+12));
	}
	seq_printf(sfile, "=====================================================================\n");


	raw_len = 256;
	data = (UINT8 *)kmalloc(sizeof(UINT8) * raw_len, GFP_KERNEL);

	if(data == NULL){
		return -ENOMEM;
	}

	/*parsing hdmi driver info*/
	
	
	printk("=== HDMI driver info: ==============================================\r\n");
	printk("    version code: [%s]\r\n",HDMI_VERSION);
	printk("     output mode: [%d] (0: RGB888, 1: YCbCr422, 2: YCbCr444)\r\n",g_dispdev_hdmi_mode);
	printk("        video id: [%d]\r\n",g_dispdev_hdmi_vid);
	printk("    audio format: [%d] (0: 32Khz,  1:  44.1Khz, 2: 48Khz)\r\n",g_dispdev_aud_fmt);
	printk("      ide source: [%d]\r\n",g_dispdev_src_fmt);
	printk("=====================================================================\r\n");
	
	hdmitx_get_edid_raw_data(&raw_len,data);
	
	for(i=0; i<raw_len; i+=4){
		printk("EDID_data[%03d] = [0x%02x] [0x%02x] [0x%02x] [0x%02x]\r\n",i,data[i],data[i+1],data[i+2],data[i+3]);
	}	
	
	kfree(data);
	return 0;
}

static int nvt_hdmitx_proc_help_open(struct inode *inode, struct file *file)
{
	hdmitx_api("%s\n", __func__);

	return single_open(file, nvt_hdmitx_proc_help_show, NULL);
}

static struct file_operations proc_help_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_hdmitx_proc_help_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};

int nvt_hdmitx_proc_init(PHDMITX_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	hdmitx_api("%s\n", __func__);

	pmodule_root = proc_mkdir("hdmitx", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_module_root = pmodule_root;


	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_help_entry = pentry;


	phdmitx_drv_info_data = pdrv_info;

	return ret;

remove_cmd:
	proc_remove(pdrv_info->pproc_cmd_entry);

remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

int nvt_hdmitx_proc_remove(PHDMITX_DRV_INFO pdrv_info)
{
	hdmitx_api("%s\n", __func__);

	proc_remove(pdrv_info->pproc_help_entry);
	proc_remove(pdrv_info->pproc_cmd_entry);
	proc_remove(pdrv_info->pproc_module_root);
	return 0;
}
