/*
 *  driver/power/reset/na51102-poweroff.c
 *  Novatek     Power Button Control driver
 *
 *  Author:     robin_hsu@novatek.com.tw
 *  Created:	April 13, 2022
 *  Copyright:	Novatek Inc.
 *
 */

#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/soc/nvt/nvt-io.h>
#include <plat/rtc_reg.h>
#include <plat/pwbc_reg.h>
#include <plat/pwbc_int.h>
#include <plat/rtc_int.h>
#include <plat/hardware.h>
#include <linux/clk.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/of.h>
#include <linux/reboot.h>
#include <linux/pm.h>
#include <linux/errno.h>

#define DRV_VERSION		"1.01.001"

#ifdef CONFIG_RTC_NVT_PWBC_SDT_USER
#define RTC_PWBC_SDT_ISR_DISABLE	1
#else
#define RTC_PWBC_SDT_ISR_DISABLE	0
#endif

#define PWBC_INT_PWR_SW  0x01000000
#define PWBC_INT_PWR_SW2 0x02000000
#define PWBC_INT_PWR_SW3 0x04000000
#define PWBC_INT_PWR_SW4 0x08000000
#define PWBC_INT_CSET_INTEN  0X2

static struct completion pwbc_cset_completion;
static struct semaphore pwbc_sem;
static void __iomem *_REGIOBASE_PWBC;

#define loc_cpu() down(&pwbc_sem);
#define unl_cpu() up(&pwbc_sem);

void nvt_pwbc_trigger_cset(void);
void nvt_pwbc_reset_shutdown_timer(void);
void nvt_pwbc_power_off(void);

void __iomem* hyload_base ;

struct nvt_pwbc_priv {
	struct device *pwbc;
	struct proc_dir_entry *pproc_pwbc;
	struct proc_dir_entry *pproc_en2;
	struct proc_dir_entry *pproc_en3;
    struct delayed_work cset_work;
};

static void pwbc_setreg(uint32_t offset, REGVALUE value)
{
	nvt_writel(value, _REGIOBASE_PWBC + offset);
}

static REGVALUE pwbc_getreg(uint32_t offset)
{
	return nvt_readl(_REGIOBASE_PWBC + offset);
}

void nvt_pwbc_trigger_cset(void)
{
    ulong timeout;
    union PWBC_CTRL2_REG pwbcctrl2_reg;

	/*Wait for PWBC is ready for next CSET*/
    timeout = jiffies + msecs_to_jiffies(1000);
	do {
		pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
        if (time_after_eq(jiffies, timeout)) {
			printk("PWBC CSET timeout, plz check RTC 32K OSC, PWR_SW or VCC_VBAT\n");
			return;
		}
	} while (pwbcctrl2_reg.bit.pwbc_cset == 1);

	/*Trigger CSET*/
	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 1;
	pwbcctrl2_reg.bit.pwbc_cset_inten = 1;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);
}

void nvt_pwbc_reset_shutdown_timer(void)
{
    union PWBC_CTRL_REG pwbcctrl_reg;
    union PWBC_CTRL2_REG pwbcctrl2_reg;

	/*Wait for previous CSET done */
	if (!wait_for_completion_timeout(&pwbc_cset_completion, msecs_to_jiffies(1000))) {
        printk("PWBC CSET timeout, plz check RTC 32K OSC, PWR_SW or VCC_VBAT\n");
		return;
    }

	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 0;
	pwbcctrl2_reg.bit.seq_time1_sel = 1;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);

	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
	pwbcctrl_reg.bit.reset_sdt_timer = 1;
	pwbc_setreg(PWBC_CTRL_REG_OFS, pwbcctrl_reg.reg);

    nvt_pwbc_trigger_cset();
}

void nvt_pwbc_power_off(void)
{
    union PWBC_CTRL_REG pwbcctrl_reg;
    union PWBC_CTRL2_REG pwbcctrl2_reg;
	
	/*Wait for previous CSET done */
	if (!wait_for_completion_timeout(&pwbc_cset_completion, msecs_to_jiffies(1000))) {
        printk("PWBC CSET timeout, plz check RTC 32K OSC, PWR_SW or VCC_VBAT\n");
		return;
    }
    
	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 0;
	pwbcctrl2_reg.bit.seq_time1_sel = 1;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);

	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
	pwbcctrl_reg.bit.pwr_off = 1;
	pwbc_setreg(PWBC_CTRL_REG_OFS, pwbcctrl_reg.reg);

    nvt_pwbc_trigger_cset();
}

/* Top half IRQ */
static irqreturn_t pwbc_update_handler(int irq, void *data)
{
	return IRQ_WAKE_THREAD;
}

static irqreturn_t pwbc_update_handler_thread(int irq, void *data)
{
    union PWBC_CTRL_REG pwbcctrl_reg;
	union PWBC_CTRL2_REG pwbcctrl2_reg;
    union PWBC_STS_REG pwbcsts_reg;
    
	pwbcsts_reg.reg = pwbc_getreg(PWBC_STS_REG_OFS);
	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);

	// Only handle interrupt enabled status
    pwbcsts_reg.reg &= (pwbcctrl_reg.reg & (PWBC_INT_PWR_SW | PWBC_INT_PWR_SW2 | PWBC_INT_PWR_SW3 | PWBC_INT_PWR_SW4) );
	pwbcsts_reg.reg |= (pwbcctrl2_reg.reg & PWBC_INT_CSET_INTEN ) << 21 ;

	if (pwbcsts_reg.reg) {
		// clear interrupt status 
		pwbc_setreg(PWBC_STS_REG_OFS, pwbcsts_reg.reg);
	
		if (pwbcsts_reg.bit.pwr_sw1_int_sts) {
			// reset shutdown timer to prevent SW1 shutdown system
			if (pwbcctrl_reg.bit.reset_sdt_timer == 0) {
            	nvt_pwbc_reset_shutdown_timer();
			}
		}
		if(pwbcsts_reg.bit.pwbc_cset_done) {
			complete(&pwbc_cset_completion);
		}
	}
	return IRQ_HANDLED;
}

static int nvt_pwbc_init_sw(void)
{
#if (RTC_PWBC_SDT_ISR_DISABLE == 0)
	union PWBC_CTRL_REG pwbc_ctrl_reg;

	pwbc_ctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
	pwbc_ctrl_reg.bit.pwr_sw1_inten = 1;
	pwbc_setreg(PWBC_CTRL_REG_OFS, pwbc_ctrl_reg.reg);
#endif
	return 0;
}


static void nvt_pwbc_add_one_sec(u32* sec, u32* min, u32* hour, u32* days)
{
	if (*sec == 59) {
		if (*min == 59) {
			*hour += 1;
			*min = 0;
			*sec = 0;
		} else {
			*min += 1;
			*sec = 0;
		}
	} else {
		*sec += 1;
	}

	if (*hour > 23) {
		*hour = 0;
		*days += 1;
	}
}

void nvt_pwbc_power_control(int reboot_sec)
{
	struct rtc_time cur_time;
    struct rtc_wkalrm alrm;
    u32 sec, min, hour, days;
    int i;

    if (reboot_sec) { // set rtc power alarm

        nvt_rtc_read_time(NULL, &cur_time);
        alrm.time = cur_time;

        sec  = cur_time.tm_sec;
        min  = cur_time.tm_min;
    	hour = cur_time.tm_hour;
    	days = cur_time.tm_mday;

        for (i = 0;i < reboot_sec; i++)
    		nvt_pwbc_add_one_sec(&sec, &min, &hour, &days);

        alrm.time.tm_sec  = sec;
        alrm.time.tm_min  = min;
        alrm.time.tm_hour = hour;
        alrm.time.tm_mday = days;
        alrm.enabled      = 1;

         // set pwr_alarm time
         nvt_rtc_set_alarm(NULL, &alrm);
	}

    // set reset shutdown timer and power off
    nvt_pwbc_reset_shutdown_timer();
    nvt_pwbc_power_off();

    printk("%s: If not power off, plz check 32K crystal, pad PWR_EN or VCC_VBAT\r\n", __func__);
}

static int nvt_pwbc_proc_show(struct seq_file *seq, void *v)
{
	union PWBC_STS_REG pwbcsts_reg;

	pwbcsts_reg.reg = pwbc_getreg(PWBC_STS_REG_OFS);
	seq_printf(seq, "power switch 1 value %d\n", pwbcsts_reg.bit.pwr_sw1_pin);
	seq_printf(seq, "power switch 2 value %d\n", pwbcsts_reg.bit.pwr_sw2_pin);
	seq_printf(seq, "power switch 3 value %d\n", pwbcsts_reg.bit.pwr_sw3_pin);
	seq_printf(seq, "power switch 4 value %d\n", pwbcsts_reg.bit.pwr_sw4_pin);
	seq_printf(seq, "power on source ");
	if (pwbcsts_reg.bit.pwronsrc_sw1) {
		seq_printf(seq, "SW1\r\n");
	} else if (pwbcsts_reg.bit.pwronsrc_sw2) {
		seq_printf(seq, "SW2\r\n");
	} else if (pwbcsts_reg.bit.pwronsrc_sw3) {
		seq_printf(seq, "SW3\r\n");
	} else if (pwbcsts_reg.bit.pwronsrc_sw4) {
		seq_printf(seq, "SW4\r\n");
	} else {
		seq_printf(seq, "None\r\n");
	}
	return 0;
}

static int nvt_pwbc_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, nvt_pwbc_proc_show, NULL);
}

#define PWBC_MAX_CMD_LENGTH 30
static ssize_t nvt_pwbc_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int val;
	int len = size;
	char cmd_line[PWBC_MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
    union PWBC_CTRL_REG pwbcctrl_reg;

	// check command length
	if (len > (PWBC_MAX_CMD_LENGTH - 1)) {
		printk("%s: Command length %d is too long\n", __func__, len);
		return -EINVAL;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		return -EINVAL;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);
	
	loc_cpu();

	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
    //reset shutdown timer to prevent SW1 shutdown system
	if (pwbcctrl_reg.bit.reset_sdt_timer == 0) {
		nvt_pwbc_reset_shutdown_timer();
	}
	
	unl_cpu();
	return size;
}

static struct proc_ops proc_pwbc_fops = {
	.proc_open      = nvt_pwbc_proc_open,
	.proc_read      = seq_read,
	.proc_lseek     = seq_lseek,
	.proc_release   = single_release,
	.proc_write     = nvt_pwbc_proc_cmd_write,
};

/*
	PWR_EN2 related
*/
static int en2_proc_show(struct seq_file *seq, void *v)
{
	union PWBC_CTRL_REG pwbcctrl_reg;

	loc_cpu();
	/*Wait for previous CSET done */
	if (!wait_for_completion_timeout(&pwbc_cset_completion, msecs_to_jiffies(1000))) {
        printk("PWBC completion timeout, plz check RTC 32K OSC, PWR_SW or VCC_VBAT\n");
		unl_cpu();
		return -ETXTBSY;
    }

	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);

	complete(&pwbc_cset_completion);
	unl_cpu();

	seq_printf(seq, "PWR_EN2 setting = %d\n", pwbcctrl_reg.bit.pwr_en2_ctrl );

	return 0;
}

static int en2_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, en2_proc_show, NULL);
}

static ssize_t en2_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int val;
	int len = size;
	char cmd_line[PWBC_MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	union PWBC_CTRL_REG pwbcctrl_reg;
	union PWBC_CTRL2_REG pwbcctrl2_reg;

	// check command length
	if (len > (PWBC_MAX_CMD_LENGTH - 1)) {
		printk("%s: Command length %d is too long\n", __func__, len);
		return -EINVAL;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		return -EINVAL;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);

	loc_cpu();

   /*Wait for previous CSET done */
	if (!wait_for_completion_timeout(&pwbc_cset_completion, msecs_to_jiffies(1000))) {
        printk("PWBC completion timeout, plz check RTC 32K OSC, PWR_SW or VCC_VBAT\n");
		unl_cpu();
		return -ETXTBSY;
    }

	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 0;
	pwbcctrl2_reg.bit.pwr_en2_ctrl_sel = 1;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);

	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
	pwbcctrl_reg.bit.pwr_en2_ctrl = (val==0)?0:1;
	pwbc_setreg(PWBC_CTRL_REG_OFS, pwbcctrl_reg.reg);

	nvt_pwbc_trigger_cset();
	unl_cpu();

	if (val == 0) {
		printk("PWR_EN2 LOW will take effect after system poweroff\r\n");
	}

	return size;
}

static struct proc_ops proc_en2_fops = {
	.proc_open      = en2_proc_open,
	.proc_read      = seq_read,
	.proc_lseek     = seq_lseek,
	.proc_release   = single_release,
	.proc_write     = en2_proc_cmd_write,
};

/*
	PWR_EN3 related
*/
static int en3_proc_show(struct seq_file *seq, void *v)
{
	union PWBC_CTRL_REG pwbcctrl_reg;

	loc_cpu();
	/*Wait for previous CSET done */
	if (!wait_for_completion_timeout(&pwbc_cset_completion, msecs_to_jiffies(1000))) {
        printk("PWBC completion timeout, plz check RTC 32K OSC, PWR_SW or VCC_VBAT\n");
		unl_cpu();
		return -ETXTBSY;
    }

	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);

	complete(&pwbc_cset_completion);
	unl_cpu();

	seq_printf(seq, "PWR_EN3 setting = %d\n", pwbcctrl_reg.bit.pwr_en3_ctrl );

	return 0;
}

static int en3_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, en3_proc_show, NULL);
}

static ssize_t en3_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int val;
	int len = size;
	char cmd_line[PWBC_MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	union PWBC_CTRL_REG pwbcctrl_reg;
	union PWBC_CTRL2_REG pwbcctrl2_reg;

	// check command length
	if (len > (PWBC_MAX_CMD_LENGTH - 1)) {
		printk("%s: Command length %d is too long\n", __func__, len);
		return -EINVAL;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		return -EINVAL;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';


	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);

	loc_cpu();

    /*Wait for previous CSET done */
	if (!wait_for_completion_timeout(&pwbc_cset_completion, msecs_to_jiffies(1000))) {
		printk("PWBC completion timeout, plz check RTC 32K OSC, PWR_SW or VCC_VBAT\n");
		unl_cpu();
		return -ETXTBSY;
    }

	pwbcctrl2_reg.reg = pwbc_getreg(PWBC_CTRL2_REG_OFS);
	pwbcctrl2_reg.bit.pwbc_cset = 0;
	pwbcctrl2_reg.bit.pwr_en3_ctrl_sel = 1;
	pwbc_setreg(PWBC_CTRL2_REG_OFS, pwbcctrl2_reg.reg);

	pwbcctrl_reg.reg = pwbc_getreg(PWBC_CTRL_REG_OFS);
	pwbcctrl_reg.bit.pwr_en3_ctrl = (val==0)?0:1;
	pwbc_setreg(PWBC_CTRL_REG_OFS, pwbcctrl_reg.reg);

	nvt_pwbc_trigger_cset();
	unl_cpu();

	return size;
}

static struct proc_ops proc_en3_fops = {
	.proc_open      = en3_proc_open,
	.proc_read      = seq_read,
	.proc_lseek     = seq_lseek,
	.proc_release   = single_release,
	.proc_write     = en3_proc_cmd_write,
};

// poweroff 
static int nvt_pwbc_restart(struct notifier_block *this, unsigned long action,
		      void *cmd)
{
    nvt_pwbc_power_control(action);
	return NOTIFY_DONE;
}

static struct notifier_block pwbc_restart_nb = {
	.notifier_call = nvt_pwbc_restart,
	.priority = 128,
};

/*static void nvt_do_pwbc_poweroff(void)
{
	nvt_pwbc_restart(&pwbc_restart_nb, 10, NULL);
}*/


static int nvt_pwbc_probe(struct platform_device *pdev)
{
    struct nvt_pwbc_priv *priv;
	struct resource *memres = NULL;
	struct proc_dir_entry *pentry = NULL;
    int ret = 0, irq = 0;

    memres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(!memres)) {
		dev_err(&pdev->dev, "failed to get resource\n");
		return -ENXIO;
	}

    _REGIOBASE_PWBC = devm_ioremap_resource(&pdev->dev, memres);;
	if (unlikely(_REGIOBASE_PWBC == 0)) {
		dev_err(&pdev->dev, "failed to get io memory\n");
		goto out;
	}

	priv = kzalloc(sizeof(struct nvt_pwbc_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);

	device_init_wakeup(&pdev->dev, 1);

	/*Set default HW configuration*/
	/*Define what data type will be used, RCW_DEF or manual define*/

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "No IRQ resource!\n");
		goto out;
	}

	init_completion(&pwbc_cset_completion);

	sema_init(&pwbc_sem, 1);

	ret = devm_request_threaded_irq(&pdev->dev, irq, 
		pwbc_update_handler, pwbc_update_handler_thread, IRQF_ONESHOT, pdev->name, priv);

	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to request IRQ: #%d: %d\n", irq, ret);
		goto out;
	}

    nvt_pwbc_init_sw();

	pentry = proc_create("pwbc", S_IRUGO | S_IXUGO, NULL, &proc_pwbc_fops);
	if (pentry == NULL) {
			dev_err(&pdev->dev, "failed to create pwbc proc!\n");
			ret = -EINVAL;
			goto out_proc_pwbc;
	}
	priv->pproc_pwbc = pentry;

	pentry = proc_create("pwbc_en2", S_IRUGO | S_IXUGO, NULL, &proc_en2_fops);
        if (pentry == NULL) {
                dev_err(&pdev->dev, "failed to create pwbc_en2 proc!\n");
                ret = -EINVAL;
                goto out_proc_en2;
        }
	priv->pproc_en2 = pentry;

	pentry = proc_create("pwbc_en3", S_IRUGO | S_IXUGO, NULL, &proc_en3_fops);
        if (pentry == NULL) {
                dev_err(&pdev->dev, "failed to create pwbc_en3 proc!\n");
                ret = -EINVAL;
                goto out_proc_en3;
        }
	priv->pproc_en3 = pentry;

	complete(&pwbc_cset_completion);
	register_restart_handler(&pwbc_restart_nb);

	return 0;

out_proc_en3:
	if (priv->pproc_en3) {
		proc_remove(priv->pproc_en3);
	}
out_proc_en2:
	if (priv->pproc_en2) {
		proc_remove(priv->pproc_en2);
	}

out_proc_pwbc:
	if (priv->pproc_pwbc) {
		proc_remove(priv->pproc_pwbc);
	}
out:
    kfree(priv);
	return ret;
}

static int nvt_pwbc_remove(struct platform_device *pdev)
{
	struct nvt_pwbc_priv *priv = platform_get_drvdata(pdev);

	if (priv->pproc_pwbc) {
		proc_remove(priv->pproc_pwbc);
	}
    
	kfree(priv);
	return 0; 
}
#ifdef CONFIG_OF
static const struct of_device_id nvt_pwbc_of_dt_ids[] = {
    { .compatible = "nvt,nvt_pwbc", },
    {},
};
MODULE_DEVICE_TABLE(of, nvt_pwbc_of_dt_ids);
#endif

static struct platform_driver nvt_pwbc_platform_driver = {
	.driver		= {
		.name	= "nvt_pwbc",
		.owner	= THIS_MODULE,
		.of_match_table = nvt_pwbc_of_dt_ids,
	},
	.probe		= nvt_pwbc_probe,
	.remove		= nvt_pwbc_remove,
};

static int __init nvt_pwbc_init(void)
{
	int ret;
    
	ret = platform_driver_register(&nvt_pwbc_platform_driver);
	return ret;
}

static void __exit nvt_pwbc_exit(void)
{
	platform_driver_unregister(&nvt_pwbc_platform_driver);
}

MODULE_AUTHOR("Novatek");
MODULE_DESCRIPTION("nvt PWBC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_ALIAS("platform:pwbc-nvt");

module_init(nvt_pwbc_init);
module_exit(nvt_pwbc_exit);

