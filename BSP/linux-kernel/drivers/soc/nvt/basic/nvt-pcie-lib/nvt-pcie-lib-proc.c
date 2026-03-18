#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/soc/nvt/nvt-pci.h>
#include "nvt-pcie-lib-internal.h"

#define PROC_CMD_LENGTH 64
#define PROC_ARG_NUM     6

static struct proc_dir_entry *pg_proc_root = NULL;

// definition -----------
typedef struct proc_cmd {
	char cmd[PROC_CMD_LENGTH];
	int (*pfunc)(int argc, char** argv);
} PROC_CMD;

// proc ----------
int proc_func_perf(int argc, char** argv)
{
	nvtpcie_test_perf();
	return 0;
}

int proc_func_test(int argc, char** argv)
{
	nvtpcie_test_api();
	return 0;
}

int proc_func_test_shmblk(int argc, char** argv)
{
	nvtpcie_test_shmblk();
	return 0;
}

int proc_func_dump(int argc, char** argv)
{
	_nvtpcie_dump_rc_downmap();
	_nvtpcie_dump_ep_upmap();
	_nvtpcie_dump_ddr_map();
	_nvtpcie_dump_ep_allocmem();
	_nvtpcie_shmem_dump_shmblk();
	nvtpcie_shmem_dump(0, 0x100);

	DBG_DUMP("my chipid = %d\r\n", nvtpcie_get_my_chipid());
	DBG_DUMP("ep count = %d\r\n", nvtpcie_get_ep_count());
	DBG_DUMP("is cascade = %d\r\n", nvtpcie_is_cascade());
	return 0;
}

int proc_func_dumpsys(int argc, char** argv)
{
	_nvtpcie_shmem_dump_sysinfo();

	return 0;
}

static void dumpmem(nvtpcie_chipid_t ep_chipid, unsigned long ep_pa, unsigned long length)
{
	unsigned long offs;
	u32 str_len;
	u32 cnt;
	u32 u32array[4];
	char str_dumpmem[128];
	void* va = NULL;
	u32 *p_u32;
	char *p_char;
	unsigned long mapped_pa;

	if (0 == length) {
		DBG_DUMP("length %ld\r\n", length);
		return;
	}

	mapped_pa = nvtpcie_get_downstream_pa(ep_chipid, ep_pa);
	if (NVTPCIE_INVALID_PA == mapped_pa) {
		DBG_ERR("downstream ep_chipid %d ep_pa 0x%lx failed\r\n", ep_chipid, ep_pa);
		return;
	}

	mapped_pa = ALIGN_DOWN(mapped_pa, 4); //align to 4 bytes (u32)

	va = ioremap(mapped_pa, length);
	if (NULL == va) {
		DBG_DUMP("ioremap() failed, pa 0x%lx, length %ld\r\n", mapped_pa, length);
		return;
	}
	p_u32 = (u32 *)va;


	DBG_DUMP("dump va=%08lx, mapped_pa=%08lx ep_pa=0x%08lx len=0x%08lx\r\n",
		(unsigned long)p_u32, mapped_pa, ep_pa, length);

	for (offs = 0; offs < length; offs += sizeof(u32array)) {
		u32array[0] = readl(p_u32++);
		u32array[1] = readl(p_u32++);
		u32array[2] = readl(p_u32++);
		u32array[3] = readl(p_u32++);

		str_len = snprintf(str_dumpmem, sizeof(str_dumpmem), "%08lX->%08lX : %08X %08X %08X %08X  ",
			(mapped_pa + offs), (ep_pa + offs), u32array[0], u32array[1], u32array[2], u32array[3]);

		p_char = (char *)&u32array[0];
		for (cnt = 0; cnt < sizeof(u32array); cnt++, p_char++) {
			if (*p_char < 0x20 || *p_char >= 0x80)
				str_len += snprintf(str_dumpmem+str_len, sizeof(str_dumpmem)-str_len, ".");
			else
				str_len += snprintf(str_dumpmem+str_len, sizeof(str_dumpmem)-str_len, "%c", *p_char);
		}

		DBG_DUMP("%s\r\n", str_dumpmem);
	}
	DBG_DUMP("\r\n\r\n");
	if (NULL != va) {
		iounmap(va);
	}
}

int proc_func_epmem(int argc, char** argv)
{
	unsigned long chipid;
	unsigned long local_pa;
	unsigned long size = 512;
	char *p_rw, *p_chipid, *p_addr, *p_size;

	if (argc < 3) {
		goto exit_usage;
	}

	p_rw = argv[0];
	p_chipid = argv[1];
	p_addr = argv[2];
	p_size = (argc > 3) ? argv[3] : NULL;

	if (0 == strncmp(p_rw, "r", 1)) {
		DBG_DUMP("read\r\n");
	} else if (0 == strncmp(p_rw, "w", 1)) {
		DBG_DUMP("write\r\n");
	} else {
		goto exit_usage;
	}

	if (0 != kstrtoul(p_chipid, 0, &chipid)) {
		DBG_ERR("parsing chipid [%s] failed\r\n", p_chipid);
		goto exit_usage;
	}

	if (0 != kstrtoul(p_addr, 0, &local_pa)) {
		DBG_ERR("parsing address [%s] failed\r\n", p_addr);
		goto exit_usage;
	}

	if (p_size) {
		if (0 != kstrtoul(p_size, 0, &size)) {
			DBG_ERR("parsing size [%s] failed\r\n", p_size);
			goto exit_usage;
		}
	}

	if (!ISVALID_EP_CHIPID(chipid)) {
		DBG_ERR("%ld not a valid ep chipid\r\n", chipid);
		goto exit_usage;
	}

	DBG_DUMP("epmem %s %ld 0x%lx 0x%lx\r\n", p_rw, chipid, local_pa, size);
	dumpmem(chipid, local_pa, size);

	return 0;

exit_usage:
	DBG_DUMP("Usage:\r\n");
	DBG_DUMP("echo epmem r [ep chipid] [0xADDR] [0xSIZE(optional)] > /proc/nvt-pcie-lib/cmd\r\n");
	return 0;
}

int proc_func_epfile(int argc, char** argv)
{
	char *p_chipid, *p_remotepath, *p_localpath;
	unsigned long chipid;

	if (argc < 2) {
		goto exit_usage;
	}

	p_chipid = argv[0];
	p_remotepath = argv[1];
	p_localpath = (argc > 2) ? argv[2] : NULL;

	if (0 != kstrtoul(p_chipid, 0, &chipid)) {
		DBG_ERR("parsing chipid [%s] failed\r\n", p_chipid);
		goto exit_usage;
	}

	DBG_DUMP("epfile %ld [%s] [%s]\r\n", chipid, p_remotepath, p_localpath);
	return 0;

exit_usage:
	DBG_DUMP("Usage:\r\n");
	DBG_DUMP("echo epfile [ep chipid] [remote path] [local path(optional)] > /proc/nvt-pcie-lib/cmd\r\n");
	return 0;
}

int proc_func_version(int argc, char** argv)
{
	DBG_DUMP("nvt-pcie-lib: %s\n", NVT_PCIE_LIB_VERSION);
	return 0;
}

int proc_func_rescan(int argc, char** argv)
{
#if IS_ENABLED(CONFIG_NVT_NT98336_PCI_EP_EVB)
	_nvtpcie_atu_board_init();
#elif IS_ENABLED(CONFIG_PCI)
	_nvtpcie_atu_common_init();
#else
	_nvtpcie_atu_cascade_init();
#endif
	return 0;
}

static PROC_CMD g_proc_cmd_list[] = {
	{"dump", proc_func_dump},
	{"dumpsys", proc_func_dumpsys},
	{"epmem", proc_func_epmem},
	{"epfile", proc_func_epfile},
	{"perf", proc_func_perf},
	{"rescan", proc_func_rescan},
	{"test", proc_func_test},
	{"test_shmblk", proc_func_test_shmblk},
	{"version", proc_func_version},
};

static int proc_show_cmd(struct seq_file *sfile, void *v)
{
	unsigned int cmd_idx;

	DBG_DUMP("Usage: echo xxx > cmd\n");
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		DBG_DUMP("%s\n", g_proc_cmd_list[cmd_idx].cmd);
	}
	return 0;
}

static int proc_open_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_cmd, NULL);
}

static ssize_t proc_write_cmd(struct file *file, const char __user *buf,
                                  size_t size, loff_t *off)
{
	char cmd_line[PROC_CMD_LENGTH];
	const char delimiters[] = {' ', '\r', '\n', '\0'};
	char *argv[PROC_ARG_NUM] = {0};
	unsigned char argc = 0;

	char *cmdstr = cmd_line;
	unsigned int cmd_idx;
	int ret = -EINVAL;

	// Note: In fact, size will never be smaller than 1 in proc cmd
	//       to prevent coverity warning, we still check the size < 1
	// e.g. cmd [abcde], size = 6
	// e.g. cmd [], size = 1
	if (size > PROC_CMD_LENGTH || size < 1) {
		DBG_ERR("Invalid cmd size %ld\n", (unsigned long)size);
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, size))
		goto ERR_OUT;

	cmd_line[size - 1] = '\0';

	DBG_DUMP("PROC_CMD: [%s], size %ld\n", cmd_line, (unsigned long)size);

	// parse command string
	for (argc = 0; argc < PROC_ARG_NUM; argc++) {
		argv[argc] = strsep(&cmdstr, delimiters);

		if (argv[argc] == NULL)
			break;
	}

	// dispatch command handler
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		if (strncmp(argv[0], g_proc_cmd_list[cmd_idx].cmd, PROC_CMD_LENGTH) == 0) {
			ret = g_proc_cmd_list[cmd_idx].pfunc(argc - 1, &argv[1]);
			break;
		}
	}
	if (cmd_idx >= ARRAY_SIZE(g_proc_cmd_list))
		goto ERR_INVALID_CMD;

	return size;

ERR_INVALID_CMD:
	DBG_ERR("Invalid CMD [%s]\n", cmd_line);

ERR_OUT:
	return -EINVAL;
}

static struct proc_ops proc_fops_cmd = {
	.proc_open    = proc_open_cmd,
	.proc_read    = seq_read,
	.proc_write   = proc_write_cmd,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
};

static int proc_show_help(struct seq_file *sfile, void *v)
{
	int cmd_idx;

	seq_printf(sfile, "cmd_list:\r\n");
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		seq_printf(sfile, "%s\r\n", g_proc_cmd_list[cmd_idx].cmd);
	}

	return 0;
}

static int proc_open_help(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show_help, NULL);
}

static struct proc_ops proc_fops_help = {
	.proc_open    = proc_open_help,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
};

ssize_t ep_count_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos)
{
	char kbuf[32] = {0};
	snprintf(kbuf, sizeof(kbuf), "%d\n", nvtpcie_get_ep_count());
	return simple_read_from_buffer(ubuf, count, ppos, kbuf, strlen(kbuf));
}

static struct proc_ops ep_count_fops = {
	.proc_open = simple_open,
	.proc_read = ep_count_read,
	.proc_lseek = generic_file_llseek,
};

ssize_t is_cascade_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos)
{
	char kbuf[32] = {0};
	snprintf(kbuf, sizeof(kbuf), "%d\n", nvtpcie_is_cascade());
	return simple_read_from_buffer(ubuf, count, ppos, kbuf, strlen(kbuf));
}

static struct proc_ops is_cascade_fops = {
	.proc_open = simple_open,
	.proc_read = is_cascade_read,
	.proc_lseek = generic_file_llseek,
};

int nvtpcie_proc_init(void)
{
	pg_proc_root = proc_mkdir("nvt-pcie-lib", NULL);
	if (NULL == pg_proc_root) {
		DBG_ERR("NVTPCIE: proc_mkdir\n");
		goto pciesys_proc_init_err;
	}

	if (NULL == proc_create("cmd", S_IRUGO | S_IXUGO, pg_proc_root, &proc_fops_cmd)) {
		DBG_ERR("NVTPCIE: proc_create cmd\n");
		goto pciesys_proc_init_err;
	}

	if (NULL == proc_create("help", S_IRUGO | S_IXUGO, pg_proc_root, &proc_fops_help)) {
		DBG_ERR("NVTPCIE: proc_create help\n");
		goto pciesys_proc_init_err;
	}

	if (NULL == proc_create("ep_count", S_IRUGO | S_IXUGO, pg_proc_root, &ep_count_fops)) {
		DBG_ERR("NVTPCIE: proc_create ep_count\n");
		goto pciesys_proc_init_err;
	}

	if (NULL == proc_create("is_cascade", S_IRUGO | S_IXUGO, pg_proc_root, &is_cascade_fops)) {
		DBG_ERR("NVTPCIE: proc_create is_cascade\n");
		goto pciesys_proc_init_err;
	}

	return 0;

pciesys_proc_init_err:
	if (pg_proc_root) {
		proc_remove(pg_proc_root);
		pg_proc_root = NULL;
	}

	return -1;
}

void nvtpcie_proc_exit(void)
{
	if (pg_proc_root) {
		proc_remove(pg_proc_root);
		pg_proc_root = NULL;
	}
}
