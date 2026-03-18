#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/slab.h>
#include <linux/soc/nvt/nvt-info.h>
#include <plat/cg-reg.h>
#ifdef CONFIG_COMMON_CLK_NA51055
#include "nvt-im-clk-fr.h"
#else
#include "nvt-clk-provider.h"
#endif

#if defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined(CONFIG_NVT_IVOT_PLAT_NS02301) || defined(CONFIG_NVT_IVOT_PLAT_NS02302) 
void pll_dump_info(void);
#endif
#define MAX_CMD_LENGTH 128
#define MAX_ARG_NUM 6

static int nvt_clk_prepare_cmd(struct seq_file *s, void *data)
{
	seq_printf(s,"use [ echo @device @prepare:1/unprepare:0 > clk_prepare ] to prepare clk\n");
	return 0;
}

static ssize_t nvt_proc_clk_prepare(struct file *file, const char __user *buf,
				    size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = { ' ', 0x0A, 0x0D, '\0' };
	char *argv[MAX_ARG_NUM] = { 0 };
	unsigned char ucargc = 0;

	struct clk *local_clk;
	int ret = 0;

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

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);
		if (argv[ucargc] == NULL)
			break;
	}
	if (ucargc != 2) {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	local_clk = __clk_lookup(argv[0]);
	if (IS_ERR(local_clk) || PTR_ERR(local_clk) == -ENOENT || local_clk == NULL) {
		pr_err("\nNo this device name or not support this feature\n");
		goto ERR_OUT;
	}

	if (simple_strtol(argv[1], NULL, 10) == 0) {
		clk_unprepare(local_clk);

	} else if (simple_strtol(argv[1], NULL, 10) == 1)
		ret = clk_prepare(local_clk);

	else {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	if (ret == 0)
		printk("\nPrepare/Unprepare successful\n");
	else
		printk("\nPrepare/Unprepare fail\n");

	return size;

ERR_OUT:
	return -1;
}

static int nvt_proc_clk_prepare_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_clk_prepare_cmd, NULL);
}

static const struct proc_ops clk_prepare_fops = {
	.proc_open = nvt_proc_clk_prepare_cmd,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = nvt_proc_clk_prepare,
};

static int clk_enable_cmd(struct seq_file *s, void *data)
{
	seq_printf(s,"use [ echo @device @enable:1/disable:0 > clk_emable ] to enable clk\n");
	return 0;
}

static ssize_t nvt_proc_clk_enable(struct file *file, const char __user *buf,
				   size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = { ' ', 0x0A, 0x0D, '\0' };
	char *argv[MAX_ARG_NUM] = { 0 };
	unsigned char ucargc = 0;

	struct clk *local_clk;
	int ret = 0;

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

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);
		if (argv[ucargc] == NULL)
			break;
	}
	if (ucargc != 2) {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	local_clk = __clk_lookup(argv[0]);
	if (IS_ERR(local_clk) || PTR_ERR(local_clk) == -ENOENT || local_clk == NULL) {
		pr_err("\nNo this device name or not support this feature\n");
		goto ERR_OUT;
	}

	/*should prepare/unprepare before enable/disable*/
	if (simple_strtol(argv[1], NULL, 10) == 0) {
		if (!__clk_is_enabled(local_clk)) {
			pr_err("\nAlready disable\n");
			goto ERR_OUT;
		} else
			clk_disable_unprepare(local_clk);
	} else if (simple_strtol(argv[1], NULL, 10) == 1)
		ret = clk_prepare_enable(local_clk);

	else {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	if (ret == 0)
		printk("\nEnable/Disable successful\n");
	else
		printk("\nEnable/Disable fail\n");

	return size;

ERR_OUT:
	return -1;
}

static int nvt_proc_clk_enable_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, clk_enable_cmd, NULL);
}

static const struct proc_ops clk_enable_fops = {
	.proc_open = nvt_proc_clk_enable_cmd,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = nvt_proc_clk_enable,
};

static int clk_set_parent_cmd(struct seq_file *s, void *data)
{
	seq_printf(s,"use [ echo @device @parent > clk_set_parent ] to set parent\n");
	return 0;
}

static ssize_t nvt_proc_clk_set_parent(struct file *file,
				       const char __user *buf, size_t size,
				       loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = { ' ', 0x0A, 0x0D, '\0' };
	char *argv[MAX_ARG_NUM] = { 0 };
	unsigned char ucargc = 0;

	struct clk *temp_clk, *parent_clk;
	int ret = 0;

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

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);
		if (argv[ucargc] == NULL)
			break;
	}
	if (ucargc != 2) {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	temp_clk = __clk_lookup(argv[0]);
	if (IS_ERR(temp_clk) || PTR_ERR(temp_clk) == -ENOENT || temp_clk == NULL) {
		pr_err("\nNo this device name or not support this feature\n");
		goto ERR_OUT;
	}

	parent_clk = __clk_lookup(argv[1]);
	if (IS_ERR(parent_clk) || PTR_ERR(parent_clk) == -ENOENT || parent_clk == NULL) {
		pr_err("\nNo this device name or not support this feature\n");
		goto ERR_OUT;
	}

	ret = clk_set_parent(temp_clk, parent_clk);
	if (ret == 0)
		printk("\nSet parent successful\n");
	else
		printk("\nSet parent fail\n");

	return size;

ERR_OUT:
	return -1;
}

static int nvt_proc_clk_set_parent_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, clk_set_parent_cmd, NULL);
}

static const struct proc_ops clk_set_parent_fops = {
	.proc_open = nvt_proc_clk_set_parent_cmd,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = nvt_proc_clk_set_parent,
};

static int clk_set_rate_cmd(struct seq_file *s, void *data)
{
	seq_printf(s,"use [ echo @device @rate > clk_set_rate ] to set rate\n");
	return 0;
}

static ssize_t nvt_proc_clk_set_rate(struct file *file, const char __user *buf,
				     size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = { ' ', 0x0A, 0x0D, '\0' };
	char *argv[MAX_ARG_NUM] = { 0 };
	unsigned char ucargc = 0;

	struct clk *temp_clk;
	int rate = 0;
	int ret = 0;

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

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);
		if (argv[ucargc] == NULL)
			break;
	}
	if (ucargc != 2) {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	temp_clk = __clk_lookup(argv[0]);
	if (IS_ERR(temp_clk) || PTR_ERR(temp_clk) == -ENOENT || temp_clk == NULL) {
		pr_err("\nNo this device name or not support this feature\n");
		goto ERR_OUT;
	}

	rate = simple_strtol(argv[1], NULL, 10);
	if (rate < 0) {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	ret = clk_set_rate(temp_clk, rate);
	if (ret == 0)
		printk("\nSet rate successful\n");
	else
		printk("\nSet rate fail\n");

	return size;

ERR_OUT:
	return -1;
}

static int nvt_proc_clk_set_rate_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, clk_set_rate_cmd, NULL);
}

static const struct proc_ops clk_set_rate_fops = {
	.proc_open = nvt_proc_clk_set_rate_cmd,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = nvt_proc_clk_set_rate,
};

static int clk_set_phase_cmd(struct seq_file *s, void *data)
{
	seq_printf(s,"use [ echo @device @phase > clk_set_phase ] to set phase\n");
	return 0;
}

static ssize_t nvt_proc_clk_set_phase(struct file *file, const char __user *buf,
				      size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = { ' ', 0x0A, 0x0D, '\0' };
	char *argv[MAX_ARG_NUM] = { 0 };
	unsigned char ucargc = 0;

	struct clk *temp_clk;
	int phase = 0;
	int ret = 0;

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

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);
		if (argv[ucargc] == NULL)
			break;
	}
	if (ucargc != 2) {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	temp_clk = __clk_lookup(argv[0]);
	if (IS_ERR(temp_clk) || PTR_ERR(temp_clk) == -ENOENT || temp_clk == NULL) {
		pr_err("\nNo this device name or not support this feature\n");
		goto ERR_OUT;
	}

	phase = simple_strtol(argv[1], NULL, 10);
	ret = clk_set_phase(temp_clk, phase);
	if (ret == 0)
		printk("\nSet rate successful\n");
	else
		printk("\nSet rate fail\n");

	return size;


ERR_OUT:
	return -1;
}

static int nvt_proc_clk_set_phase_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, clk_set_phase_cmd, NULL);
}

static const struct proc_ops clk_set_phase_fops = {
	.proc_open = nvt_proc_clk_set_phase_cmd,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = nvt_proc_clk_set_phase,
};

static int clk_get_info_cmd(struct seq_file *s, void *data)
{

	seq_printf(s,"use [ echo @device > clk_get_info ] to get info\n");

	return 0;
}

static int clk_framwork_get_frequency(struct clk *clk_temp)
{
	struct device_node *clk_root = NULL;
	struct device_node *clk_node = NULL;
	int max_frequency = 0;
	int ret = 0;

	clk_root = of_find_node_by_path("/nvt-clk");
	if(clk_root == NULL){
		/*cannot find nvt-clk root\n")*/;
		goto ERR_OUT;
	}

	clk_node = of_find_node_by_name(clk_root,__clk_get_name(clk_temp));
	if(clk_node == NULL){
		/*cannot find clk nood in device tree\n")*/;
		goto ERR_OUT;
	}

	ret = of_property_read_u32(clk_node,"max_rate",&max_frequency);
	if(ret == 0)
        return max_frequency;
	else{
		/*cannot find max frequency*/;
		goto ERR_OUT;
	}

ERR_OUT:
	return 0;

}

static void clk_show_parents(struct clk_hw *clk_temp,int level)
{
	printk("%*s%-*s\n",
		   level * 3 + 1, "",
		   30 - level * 3, clk_hw_get_name(clk_temp));

	clk_temp = clk_hw_get_parent(clk_temp);

	if(clk_temp)
		clk_show_parents(clk_temp,level+1);
	else
		return;

}

static ssize_t nvt_proc_clk_get_info(struct file *file, const char __user *buf,
				      size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = { ' ', 0x0A, 0x0D, '\0' };
	char *argv[MAX_ARG_NUM] = { 0 };
	unsigned char ucargc = 0;

	struct clk *temp_clk;
	int parent_count = 0;

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

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);
		if (argv[ucargc] == NULL)
			break;
	}
	if (ucargc != 1) {
		pr_err("\nCommand error\n");
		goto ERR_OUT;
	}

	temp_clk = __clk_lookup(argv[0]);
	if (IS_ERR(temp_clk) || PTR_ERR(temp_clk) == -ENOENT || temp_clk == NULL) {
		pr_err("\nNo this device name or not support this feature\n");
		goto ERR_OUT;
	}

	parent_count = clk_hw_get_num_parents(__clk_get_hw(temp_clk));

	printk("   clock                         enable_cnt        rate        max_frequency   accuracy      phase\n");
	printk("----------------------------------------------------------------------------------------------------\n");

	printk("%*s%-*s %11d %12lu %21d %10lu %10d\n",
		3, "",
		27,
		__clk_get_name(temp_clk),
		__clk_get_enable_count(temp_clk),
		clk_get_rate(temp_clk),
		clk_framwork_get_frequency(temp_clk),
		clk_get_accuracy(temp_clk),
		clk_get_phase(temp_clk));

	printk("----------------------------------------------------------------------------------------------------\n");
	printk("\n");
	printk("\n");

	if(parent_count == 0)
		printk("This clock no parent\n");
	else{
		printk("clk parent list\n");
		printk("----------------------------------------------------------------------------------------\n");
		clk_show_parents(__clk_get_hw(temp_clk),0);
	}

	return size;

ERR_OUT:
	return -1;
}

static int nvt_proc_clk_get_info_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, clk_get_info_cmd, NULL);
}

static const struct proc_ops clk_get_info_fops = {
	.proc_open = nvt_proc_clk_get_info_cmd,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = nvt_proc_clk_get_info,
};

static int clk_childless_cmd(struct seq_file *s, void *data)
{
	seq_printf(s,"use [ echo 1 > clk_childless ] to disable useless pll preset clock\n");
	return 0;
}

static ssize_t nvt_proc_clk_childless(struct file *file, const char __user *buf,
				      size_t size, loff_t *off)
{
	char pll_name[32];
	struct clk *temp_clk;
	struct clk_hw *hw;
	struct nvt_pll_clk *pll;
	int i = 0;

	for (i = 0; i < NVT_PLL_MAX_NUMBER; i++) {
		sprintf(pll_name, "pll%d", i);
		temp_clk = __clk_lookup(pll_name);
		if (IS_ERR(temp_clk) || PTR_ERR(temp_clk) == -ENOENT || temp_clk == NULL)
			continue;

		hw = __clk_get_hw(temp_clk);
		if (hw == NULL)
			continue;

		pll = container_of(hw, struct nvt_pll_clk, pll_hw);
		if (pll->pll_fixed)
			continue;

		if (!__clk_is_enabled(temp_clk))
			continue;

		if (!nvt_check_child(temp_clk)) {
			clk_disable_unprepare(temp_clk);
			pr_info("%s: disable useless pll clock [%s]\n", __func__, __clk_get_name(temp_clk));
		}
	};

	return size;
}

static int nvt_proc_clk_childless_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, clk_childless_cmd, NULL);
}

static const struct proc_ops clk_childless_fops = {
	.proc_open = nvt_proc_clk_childless_cmd,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = nvt_proc_clk_childless,
};

static int get_max_frequency(struct seq_file *s, void *data)
{
	struct device_node *clk_root = NULL;
	struct device_node *child = NULL;
	const char *output_name;
	int max_frequency = 0;
	int ret1 = 0, ret2 = 0;

	clk_root = of_find_node_by_path("/nvt-clk");
	if(clk_root == NULL){
		seq_printf(s,"cannot find nvt-clk root\n");
		goto ERR_OUT;
	}

	seq_puts(s, "             clock             max_rate\n");
	seq_puts(s, "---------------------------------------\n");
	for_each_child_of_node(clk_root,child) {
		ret1 = of_property_read_string(child,"clock-output-names",&output_name);
		ret2 = of_property_read_u32(child,"max_rate",&max_frequency);
		if(ret1 == 0 && ret2 == 0)
        	seq_printf(s, "  %16s %20d\n", output_name, max_frequency);
		else if(ret1 == 0 && ret2 != 0)
			seq_printf(s, "  %16s %20s\n", output_name, "unknow");
    }

	if(child)
		of_node_put(child);

	of_node_put(clk_root);

	return 0;

ERR_OUT:
	return 0;
}

static int nvt_proc_max_frequency(struct inode *inode, struct file *file)
{
	return single_open(file, get_max_frequency, NULL);
}

static const struct proc_ops max_frequency_fops = {
	.proc_open = nvt_proc_max_frequency,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};


static int clk_dump_info_cmd(struct seq_file *s, void *data)
{
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02201) || defined(CONFIG_NVT_IVOT_PLAT_NS02301) || defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	pll_dump_info();
#endif	
	return 0;
}

static int nvt_proc_clk_dump_info_cmd(struct inode *inode, struct file *file)
{
	return single_open(file, clk_dump_info_cmd, NULL);
}

static const struct proc_ops clk_dump_info_fops = {
	.proc_open = nvt_proc_clk_dump_info_cmd,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

typedef struct proc_pinmux {
	struct proc_dir_entry *pproc_clk_enable_entry;
	struct proc_dir_entry *pproc_clk_prepare_entry;
	struct proc_dir_entry *pproc_clk_set_rate_entry;
	struct proc_dir_entry *pproc_clk_set_parent_entry;
	struct proc_dir_entry *pproc_clk_set_phase_entry;
	struct proc_dir_entry *pproc_clk_get_info_entry;
	struct proc_dir_entry *pproc_clk_childless_entry;
	struct proc_dir_entry *pproc_max_frequency_entry;
	struct proc_dir_entry *pproc_clk_info_dump_entry;

} proc_clk_t;
proc_clk_t proc_clk_dbg;

static int __init nvt_clk_dbg_proc_init(void)
{
	int ret = 0;
	struct proc_dir_entry *pentry = NULL, *nvt_clk_dir_root =NULL;
	struct device_node *clk_root = NULL;

	nvt_clk_dir_root = proc_mkdir("nvt_clk", nvt_info_dir_root);
	if (!nvt_clk_dir_root)
		return -ENOMEM;

	pentry = proc_create("clk_enable", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &clk_enable_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc clk enable!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_clk_dbg.pproc_clk_enable_entry = pentry;

	pentry = proc_create("clk_prepare", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &clk_prepare_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc clk prepare!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_clk_dbg.pproc_clk_prepare_entry = pentry;

	pentry = proc_create("clk_set_rate", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &clk_set_rate_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc clk set rate!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_clk_dbg.pproc_clk_set_rate_entry = pentry;

	pentry = proc_create("clk_set_parent", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &clk_set_parent_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc clk set parent!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_clk_dbg.pproc_clk_set_parent_entry = pentry;

	pentry = proc_create("clk_set_phase", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &clk_set_phase_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc clk set phase!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_clk_dbg.pproc_clk_set_phase_entry = pentry;

	pentry = proc_create("clk_get_info", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &clk_get_info_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc clk get info!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_clk_dbg.pproc_clk_get_info_entry = pentry;

	pentry = proc_create("clk_dump_info", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &clk_dump_info_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc clk dump info!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_clk_dbg.pproc_clk_info_dump_entry = pentry;


	


	pentry = proc_create("clk_childless", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &clk_childless_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc clk childless!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_clk_dbg.pproc_clk_childless_entry = pentry;

	/* add on max_frequency proc */
	clk_root = of_find_node_by_path("/nvt-clk");
	if(clk_root != NULL){
		pentry = proc_create("max_frequency", S_IRUGO | S_IXUGO, nvt_clk_dir_root, &max_frequency_fops);
		if (pentry == NULL) {
			pr_err("failed to create proc max frequency!\n");
			ret = -EINVAL;
			goto remove_proc;
		}
		proc_clk_dbg.pproc_max_frequency_entry = pentry;
		of_node_put(clk_root);
	}


remove_proc:
	return ret;
}

late_initcall(nvt_clk_dbg_proc_init);
