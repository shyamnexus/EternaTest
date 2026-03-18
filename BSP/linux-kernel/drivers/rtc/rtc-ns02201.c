/*
 *  driver/rtc/rtc-ns02201.c
 *
 *  Author:     robin_hsu@novatek.com.tw
 *  Created:    Mar 15, 2023
 *  Copyright:  Novatek Inc.
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
#include <plat/rtc_int.h>
#include <plat/hardware.h>
#include <linux/clk.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/of.h>
#include <plat/efuse_protected.h>
#include <plat/thermal_int.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeirq.h>
#include <linux/timekeeping.h>
#ifdef CONFIG_PM
#include <linux/suspend.h>
extern int nvt_pm_set_wakeup_int_padding(void);
extern int nvt_pm_end(void);
#endif

static int **offset_value = NULL ;
static int dtsi_num_row;
#define DRV_VERSION     "1.01.013"

static struct completion cset_completion;
static struct semaphore rtc_sem;

static void __iomem *_REGIOBASE;
static int is_power_lost;

#define loc_cpu() down(&rtc_sem);
#define unl_cpu() up(&rtc_sem);

void rtc_trigger_cset(void);
void rtc_set_pwralarm_en(int en);
int  nvt_rtc_read_time(struct device *dev, struct rtc_time *tm);

static const unsigned short rtc_ydays[2][13] = {
	/* Normal years */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	/* Leap years */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

struct nvt_rtc_priv {
	struct rtc_device *rtc;
	struct proc_dir_entry *pproc_pwbc;
	struct proc_dir_entry *pproc_data;
	struct delayed_work temp_work; // temperature compensation work
};

static int nvt_rtc_read_clkdiv_offset(int *offset, int *div_sel);
static void rtc_setreg(uint32_t offset, REGVALUE value)
{
	nvt_writel(value, _REGIOBASE + offset);
}

static REGVALUE rtc_getreg(uint32_t offset)
{
	return nvt_readl(_REGIOBASE + offset);
}

static void nvt_rtc_add_one_sec(u32 *sec, u32 *min, u32 *hour, u32 *days)
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

/*
//add by Y_S WU 2014.5.22  CTS default time
static struct rtc_time default_tm = {
    .tm_year = (2014 - 1900), // year 2014
    .tm_mon =  1,             // month 2
    .tm_mday = 5,             // day 5
    .tm_hour = 12,
    .tm_min = 0,
    .tm_sec = 0
};
*/


static void rtc_cset_work(void)
{
	union RTC_OSCAN_REG oscan_reg;

	/*open auto read */
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.autoread_period = 0x08;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);

	complete(&cset_completion);
}

/* Top half IRQ */
static irqreturn_t rtc_update_handler(int irq, void *data)
{
	return IRQ_WAKE_THREAD;
}

/* Button half IRQ */
static irqreturn_t rtc_update_handler_thread(int irq, void *data)
{
	union RTC_STATUS_REG status_reg;
	status_reg.reg = rtc_getreg(RTC_STATUS_REG_OFS);

	if (status_reg.bit.alarm_sts || status_reg.bit.cset_sts) {
		rtc_setreg(RTC_STATUS_REG_OFS, status_reg.reg);
#ifdef CONFIG_PM
		if (status_reg.bit.alarm_sts) {
			printk("RTC Alarm !! \n");
			nvt_pm_set_wakeup_int_padding();
		}
#endif
		/*CSET done*/
		if (status_reg.bit.cset_sts) {
			rtc_cset_work();
		}
	}

	/* Disable pwr alarm */
	if (status_reg.bit.pwralarm_sts) {
		/*wait for cset done*/
		if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(500))) {
			//printk("RTC Disable pwr_alarm timeout, when another thread cset doing, need to wait next isr \n");
			return IRQ_HANDLED;
		}

		rtc_set_pwralarm_en(0);
		rtc_trigger_cset();
	}

	return IRQ_HANDLED;
}

void rtc_trigger_cset(void)
{
	ulong timeout;
	union RTC_CTRL_REG ctrl_reg;
	union RTC_OSCAN_REG oscan_reg;

	/*Wait for RTC SRST done*/
	timeout = jiffies + msecs_to_jiffies(1000);
	do {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		if (time_after_eq(jiffies, timeout)) {
			printk("RTC s/w reset timeout, plz check RTC 32K osc or VCC_VRTC\n");
			return;
		}
	} while (ctrl_reg.bit.srst == 1);

	/*Wait for RTC is ready for next CSET*/
	timeout = jiffies + msecs_to_jiffies(1000);
	do {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		if (time_after_eq(jiffies, timeout)) {
			printk("RTC s/w reset timeout, plz check RTC 32K osc or VCC_VRTC\n");
			return;
		}
	} while (ctrl_reg.bit.cset == 1);

	/*Close auto read */
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.autoread_period = 0x0;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);

	/*Trigger CSET*/
	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 1;
	ctrl_reg.bit.cset_inten = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);
}

void rtc_set_pwralarm_en(int en)
{
	union RTC_CTRL_REG ctrl_reg;

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.pwralarmday_sel = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.pwralarm_en = en;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);
}

#ifdef CONFIG_RTC_INTF_DEV

static int nvt_rtc_ioctl(struct device *dev, unsigned int cmd, \
						 unsigned long arg)
{
	/*
	    switch(cmd) {
	        case RTC_AIE_ON:
	            pr_info("RTC_AIE_ON\n");
	            break;
	        case RTC_AIE_OFF:
	            pr_info("RTC_AIE_OFF\n");
	            break;
	        case RTC_UIE_ON:
	            pr_info("RTC_UIE_ON\n");
	            break;
	        case RTC_UIE_OFF:
	            pr_info("RTC_UIE_OFF\n");
	            break;
	        case RTC_WIE_ON:
	            pr_info("RTC_WIE_ON\n");
	            break;
	        case RTC_WIE_OFF:
	            pr_info("RTC_WIE_OFF\n");
	            break;
	        case RTC_ALM_SET:
	            pr_info("RTC_ALM_SET\n");
	            break;
	        case RTC_ALM_READ:
	            pr_info("RTC_ALM_READ\n");
	            break;
	        case RTC_RD_TIME:
	            pr_info("RTC_RD_TIME\n");
	            break;
	        case RTC_SET_TIME:
	            pr_info("RTC_SET_TIME\n");
	            break;
	        case RTC_IRQP_READ:
	            pr_info("RTC_IRQP_READ\n");
	            break;
	        case RTC_IRQP_SET:
	            pr_info("RTC_IRQP_SET\n");
	            break;
	        case RTC_EPOCH_READ:
	            pr_info("RTC_EPOCH_READ\n");
	            break;
	        case RTC_EPOCH_SET:
	            pr_info("RTC_EPOCH_SET\n");
	            break;
	        case RTC_WKALM_SET:
	            pr_info("RTC_WKALM_SET\n");
	            break;
	        case RTC_WKALM_RD:
	            pr_info("RTC_WKALM_RD\n");
	            break;
	        case RTC_PLL_SET:
	            pr_info("RTC_PLL_SET\n");
	            break;
	        case RTC_PLL_GET:
	            pr_info("RTC_PLL_GET\n");
	            break;
	        default:
	            pr_info("unknown rtc ioctl :0X%X\n",cmd);
	    }
	*/
	return 0;
}

#else
#define nvt_rtc_ioctl    NULL
#endif

#ifdef CONFIG_PROC_FS

static int nvt_rtc_proc(struct device *dev, struct seq_file *seq)
{
	seq_printf(seq, "Power lost: %s\r\n", (is_power_lost ? "Yes" : "No"));
	return 0;
}

#else
#define nvt_rtc_proc     NULL
#endif

static int rtc_get_trim(void)
{
#if 0 // RTC trim not use for 690
	UINT16  value, trim = 0;
	UINT32  RTC_Trim_15_8, ANA_REG_3_0, ANA_REG_7_6;

	if (efuse_readParamOps(EFUSE_RTC_TRIM_DATA, &value) != E_OK) {
		// Apply default value
		trim = RTC_INT_OSC_ANALOG_CFG;
	} else {
		RTC_Trim_15_8 = (value & 0xFF);         //bit[7..0]
		ANA_REG_7_6   = ((value >> 8) & 0x3);   //bit[9..8]
		ANA_REG_3_0   = ((value >> 10) & 0xF);  //bit[13..10]
		//pr_info("[0]RTC = 0x%04x\r\n", (int)value);
		//pr_info("   *RTC_3_0 Trim [0x%04x]\r\n", (int)ANA_REG_3_0);
		//pr_info("   *RTC_7_6 Trim [0x%04x]\r\n", (int)ANA_REG_7_6);
		//pr_info("  *RTC_15_8 Trim [0x%04x]\r\n", (int)RTC_Trim_15_8);
		trim = (RTC_Trim_15_8 << 8) | (ANA_REG_7_6 << 6) | ANA_REG_3_0;

		/* Check RTC efuse trim value */
		if (RTC_Trim_15_8 == 0 || RTC_Trim_15_8 == 0xFF
			|| ANA_REG_7_6 == 0 || ANA_REG_7_6 == 0x3 || ANA_REG_3_0 == 0 || ANA_REG_3_0 == 0xF) {
			trim = RTC_INT_OSC_ANALOG_CFG;
		}
	}
#else  // efuse read trim not ready 
	UINT16  trim = 0;
	trim = RTC_INT_OSC_ANALOG_CFG;
#endif

	return trim;
}


static int nvt_rtc_reset(struct device *dev,  int *reset_date)
{
	ulong timeout;
	int time_ret;
	struct rtc_time cur_time;
	union RTC_CTRL_REG ctrl_reg;
	union RTC_PWRALM_REG pwralarm_reg;
	union RTC_DAYKEY_REG daykey_reg;
	union RTC_OSCAN_REG oscan_reg;
	union RTC_CLKDIV_REG div_reg;
	
	daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	div_reg.reg = rtc_getreg(RTC_CLKDIV_REG_OFS);

	// check time is valid
	complete(&cset_completion);
	time_ret = nvt_rtc_read_time(NULL , &cur_time);
	wait_for_completion(&cset_completion);

	if ((time_ret !=0 ) ||
		( (daykey_reg.bit.key == RTC_INT_KEY_POR) & (oscan_reg.bit.osc_analogcfg != RTC_INT_OSD_ANALOG_ORIGINAL) ) ||
		( (!(daykey_reg.bit.key == RTC_INT_KEY_POR)) & (oscan_reg.bit.osc_analogcfg != rtc_get_trim()) ) ||
		( div_reg.bit.clk_div_default >= 0xC8 ) ||
		( div_reg.bit.clk_div_offset  >= 0xC8 )  ) {
		
		printk("RTC Reset \n");
		// enter test mode
		loc_cpu();
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		ctrl_reg.bit.cset = 0;
		ctrl_reg.bit.pwralarmtime_sel = 1;
		ctrl_reg.bit.key_sel = 1;
		rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

		pwralarm_reg.reg = rtc_getreg(RTC_PWRALM_REG_OFS);
		pwralarm_reg.bit.hour = 0x1F;
		rtc_setreg(RTC_PWRALM_REG_OFS, pwralarm_reg.reg);

		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
		daykey_reg.bit.key = 0;
		rtc_setreg(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

		rtc_trigger_cset();
		unl_cpu();

		if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
			printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
			return -ETXTBSY;
		}

		// leave test mode
		loc_cpu();
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		ctrl_reg.bit.cset = 0;
		ctrl_reg.bit.pwralarmtime_sel = 1;
		rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

		pwralarm_reg.reg = rtc_getreg(RTC_PWRALM_REG_OFS);
		pwralarm_reg.bit.hour = 0x00;
		rtc_setreg(RTC_PWRALM_REG_OFS, pwralarm_reg.reg);

		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
		daykey_reg.bit.key = 1;
		rtc_setreg(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

		rtc_trigger_cset();
		unl_cpu();

		if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
			printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
			return -ETXTBSY;
		}

		// RTC re set all data
		*reset_date = 1 ;
	}

	// Wait for previous SRST done
	do {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	} while (ctrl_reg.bit.srst);

	// Wait for RTC is ready for next CSET
	do {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	} while (ctrl_reg.bit.cset);

	// Do software reset
	ctrl_reg.reg = 0;
	ctrl_reg.bit.srst = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	// Wait for RTC SRST finished, RTC can do other operations except SRST
	timeout = jiffies + msecs_to_jiffies(1000);
	do {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);

		if (time_after_eq(jiffies, timeout)) {
			printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
			return -ETXTBSY;
		}
	} while (ctrl_reg.bit.srst);

	return 0;
}

static int nvt_rtc_chk_power(struct device *dev, int reset_date)
{
	int ret = 0, time_ret = 0, data_ret = 0 ;
	int loop;
	int year_looper;
	uint16_t usr_data0 = 0;
	uint32_t days = 0;
	struct rtc_time cur_time;
	union RTC_TIMER_REG timer_reg;
	union RTC_DAYKEY_REG daykey_reg;
	union RTC_CTRL_REG ctrl_reg;
	union RTC_OSCAN_REG oscan_reg;
	union RTC_STATUS_REG status_reg;
	union RTC_DATA_REG data_reg;
	union RTC_PWRALM_REG pwralarm_reg;
	union RTC_CLKDIV_REG div_reg;

	const int RESET_YEAR = 1970 - 1900;
	const int RESET_MON = 0;
	UINT16  rtc_trim = 0;

	/* Wait for RTC SRST done, RTC is Ready */
	/*Quick auto read */
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.autoread_period = 0x01;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);

	for (loop = 0; loop < 100; loop++) {
		status_reg.reg = rtc_getreg(RTC_STATUS_REG_OFS);
		if (status_reg.bit.ready) {
			break;
		}
		usleep_range(250, 300);
	}
	if (loop >= 100) {
		printk("%s: RTC ready timeout, plz check 32K OSC on PCB\r\n", __func__);
		return -EIO;
	}
	usleep_range(500, 550); // wait auto read all group

	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.autoread_period = 0x08;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);

	// set divider to div_offset
	loc_cpu();
	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.clkdiv_offset_sel = 1;
	ctrl_reg.bit.pwralarmday_sel = 1;
	ctrl_reg.bit.pwralarmtime_sel = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	div_reg.reg = rtc_getreg(RTC_CLKDIV_REG_OFS);
	div_reg.bit.div_value_sel =  0; // select to div_offset
	rtc_setreg(RTC_CLKDIV_REG_OFS, div_reg.reg);

	// set power alarm default disabled
	rtc_set_pwralarm_en(0);
	pwralarm_reg.reg = 0;
	pwralarm_reg.bit.sec = 0;
	pwralarm_reg.bit.min = 0;
	pwralarm_reg.bit.hour = 0;
	pwralarm_reg.bit.day = 0;
	rtc_setreg(RTC_PWRALM_REG_OFS, pwralarm_reg.reg);

	rtc_trigger_cset();
	unl_cpu();

	for (year_looper = 0; year_looper < RESET_YEAR; year_looper++) {
		days += rtc_ydays[is_leap_year(year_looper + 1900)][12];
	}
	days += rtc_ydays[is_leap_year(year_looper + 1900)][RESET_MON];

	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		return -ETXTBSY;
	}

	daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);


	// check time is valid
	complete(&cset_completion);
	time_ret = nvt_rtc_read_time(NULL , &cur_time);
	wait_for_completion(&cset_completion);

	if ((daykey_reg.bit.key == RTC_INT_KEY_POR) ||
		(time_ret != 0) ||
		(oscan_reg.bit.osc_analogcfg != rtc_get_trim() ) ||
		(div_reg.bit.clk_div_default >= 0xC8) ||
		(div_reg.bit.clk_div_offset  >= 0xC8) ) {
		
		ret = nvt_rtc_reset(dev, &reset_date) ;
		if (ret)
			return ret;
	
	} else { 
		// check usr data initial value bit7-0 is 0xAA
		data_reg.reg = rtc_getreg(RTC_DATA_REG_OFS);
		usr_data0 = (data_reg.bit.data0) & 0x00FF;

		if( (daykey_reg.bit.key == 0xA) || (usr_data0!= 0xAA) ){
			printk("%s: RTC initial usr_data0 and key when key  \r\n", __func__ );
			reset_date = 0;
		
		} else {
			complete(&cset_completion);
			return 0;
		}
	}

	/* Wait for RTC SRST done, RTC is Ready */
	/*Quick auto read */
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.autoread_period = 0x01;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);

	for (loop = 0; loop < 100; loop++) {
		status_reg.reg = rtc_getreg(RTC_STATUS_REG_OFS);
		if (status_reg.bit.ready) {
			break;
		}
		usleep_range(250, 300);
	}
	if (loop >= 100) {
		printk("%s: RTC ready timeout, plz check 32K OSC on PCB\r\n", __func__);
		return -EIO;
	}
	usleep_range(500, 550); // wait auto read all group

	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.autoread_period = 0x08;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);

	loc_cpu();

	/* Reset date 1970 when power lost Anyway */
	if (reset_date == 1) {
		printk("%s: RTC power lost detected\r\n", __func__);
		is_power_lost = 1;

		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		ctrl_reg.bit.cset = 0;
		ctrl_reg.bit.key_sel = 1;
		ctrl_reg.bit.analog_sel = 1;
		ctrl_reg.bit.day_sel = 1;
		ctrl_reg.bit.time_sel = 1;
		ctrl_reg.bit.data0_sel = 1;
		ctrl_reg.bit.clkdiv_default_sel = 1;
		ctrl_reg.bit.clkdiv_offset_sel = 1;
		rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

		/*Set default year*/
		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
		daykey_reg.bit.day = days;
		rtc_setreg(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

		timer_reg.reg = 0;
		rtc_setreg(RTC_TIMER_REG_OFS, timer_reg.reg);

	} else if (reset_date == 0) {
		/* Reset date 1970 when power lost & time invalid & data ! = 0x5555 */

		// check time is valid
		complete(&cset_completion);
		time_ret = nvt_rtc_read_time(NULL, &cur_time);
		wait_for_completion(&cset_completion);

		// check usr data bit7-0 is 0xAA
		data_reg.reg = rtc_getreg(RTC_DATA_REG_OFS);
		usr_data0 = (data_reg.bit.data0) & 0x00FF;

		if (usr_data0 != 0xAA) {
			data_ret ++;
			//printk("rtc usr_data0 reg = 0x%x\n", data_reg.bit.data0);
		}

		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		ctrl_reg.bit.cset = 0;
		ctrl_reg.bit.key_sel = 1;
		ctrl_reg.bit.analog_sel = 1;
		ctrl_reg.bit.data0_sel = 1;
		ctrl_reg.bit.clkdiv_default_sel = 1;
		ctrl_reg.bit.clkdiv_offset_sel = 1;

		if (time_ret != 0 || data_ret != 0) { //rtc time is invalid & data is not 0xAA Reset date
			ctrl_reg.bit.day_sel = 1;
			ctrl_reg.bit.time_sel = 1;
		}
		rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

		/*Set default time/date*/
		if (time_ret != 0 || data_ret != 0) { //rtc time is invalid & data is not 0xAA Reset date
			printk("%s: RTC power lost detected\r\n", __func__);
			is_power_lost = 1;

			daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
			daykey_reg.bit.day = days;
			rtc_setreg(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

			timer_reg.reg = 0;
			rtc_setreg(RTC_TIMER_REG_OFS, timer_reg.reg);

		} else { /* Not reset date */
			printk("%s: RTC KEY is 1 \r\n", __func__);
		}
	}
	/* Set default rtc clk divider */
	div_reg.reg = rtc_getreg(RTC_CLKDIV_REG_OFS);
	div_reg.bit.clk_div_default = RTC_CLK_DIV_DEFAULT;
	div_reg.bit.clk_div_offset = RTC_CLK_DIV_DEFAULT;
	div_reg.bit.div_value_sel =  0; // select to div_offset
	rtc_setreg(RTC_CLKDIV_REG_OFS, div_reg.reg);

	
	/*Set usr data0 bit7-0 is 0xAA */
	data_reg.reg = rtc_getreg(RTC_DATA_REG_OFS);
	usr_data0 = (data_reg.bit.data0) & 0xFF00;
	usr_data0 = usr_data0 | 0X00AA;
	data_reg.bit.data0 = usr_data0;
	rtc_setreg(RTC_DATA_REG_OFS, data_reg.reg);

	/*Set init KEY */
	daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
	daykey_reg.bit.key = RTC_INT_KEY;
	rtc_setreg(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

	/*Set OSC analog parameter*/
	// get rtc trim Driving
	rtc_trim = rtc_get_trim();
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.osc_analogcfg = rtc_trim;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);

	rtc_trigger_cset();

	unl_cpu();

	return ret;
}

static int nvt_rtc_chek_time(struct rtc_time *s_tm, uint32_t s_day)
{
	union RTC_CTRL_REG ctrl_reg;
	union RTC_TIMER_REG timer_reg;
	union RTC_DAYKEY_REG daykey_reg;

	uint32_t days;
	uint32_t add_sec, add_min, add_hour, add_days;
	int count = 0;
	int re_set = 0;
	struct rtc_time tm;
	struct rtc_time *ptm = &tm;

	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		return -ETXTBSY;
	}

	//set time add 1 sec
	add_sec  = s_tm->tm_sec;
	add_min  = s_tm->tm_min;
	add_hour = s_tm->tm_hour;
	add_days = s_day;

	nvt_rtc_add_one_sec(&add_sec, &add_min, &add_hour, &add_days);

	// Check day collision
	for (count = 0; count < 3; count++) {
		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
		if (daykey_reg.bit.collision == 0) {
			break;
		}
		usleep_range(250, 300);
	}

	days = (daykey_reg.bit.day);

	if (days != s_day) {
		if (days != add_days) {
			re_set++;
		}
	}


	// Check time collision
	for (count = 0; count < 3; count++) {
		timer_reg.reg = rtc_getreg(RTC_TIMER_REG_OFS);
		if (timer_reg.bit.collision == 0) {
			break;
		}
		usleep_range(250, 300);
	}

	ptm->tm_sec  = timer_reg.bit.sec;
	ptm->tm_min  = timer_reg.bit.min;
	ptm->tm_hour = timer_reg.bit.hour;

	if (ptm->tm_sec != s_tm->tm_sec || ptm->tm_min != s_tm->tm_min ||  ptm->tm_hour != s_tm->tm_hour) {
		// time +1
		if (ptm->tm_sec != add_sec || ptm->tm_min != add_min ||  ptm->tm_hour != add_hour) {
			re_set++;
		}
	}

	if (re_set > 0) {
		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		ctrl_reg.bit.cset = 0;
		ctrl_reg.bit.day_sel = 1;
		ctrl_reg.bit.time_sel = 1;
		rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

		timer_reg.reg = 0;
		timer_reg.bit.sec = s_tm->tm_sec;
		timer_reg.bit.min = s_tm->tm_min;
		timer_reg.bit.hour = s_tm->tm_hour;
		rtc_setreg(RTC_TIMER_REG_OFS, timer_reg.reg);

		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
		daykey_reg.bit.day = s_day;
		rtc_setreg(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

		timer_reg.reg = rtc_getreg(RTC_TIMER_REG_OFS);
		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);

		rtc_trigger_cset();

		// need check again
		return 1;
	}

	complete(&cset_completion);
	return 0;

}

int nvt_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	uint32_t days, years, month_days = 0;
	union RTC_TIMER_REG timer_reg;
	union RTC_DAYKEY_REG daykey_reg;
	int count = 0, months ;

	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		dev_err(dev, "RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		return -ETXTBSY;
	}
	complete(&cset_completion);

	// Check day collision
	for (count = 0; count < 3; count++) {
		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
		if (daykey_reg.bit.collision == 0) {
			break;
		}
		usleep_range(250, 300);
	}

	days = daykey_reg.bit.day;

	for (years = 0; days >= rtc_ydays[is_leap_year(years + 1900)][12]; \
		 years++) {
		days -= rtc_ydays[is_leap_year(years + 1900)][12];
	}

	for (months = 1; months < 13; months++) {
		if (days <= rtc_ydays[is_leap_year(years + 1900)][months]) {
			days -= rtc_ydays[is_leap_year(years + 1900)][months - 1];
			months--;
			break;
		}
	}

	if( (months < 12) && (months >= 0) ){
		month_days = rtc_ydays[is_leap_year(years + 1900)][months + 1] - \
				 rtc_ydays[is_leap_year(years + 1900)][months];
	}

	if (days == month_days) {
		months++;
		days = 1;
	} else {
		days++;    /*Align linux time format*/
	}

	// Check time collision
	for (count = 0; count < 3; count++) {
		timer_reg.reg = rtc_getreg(RTC_TIMER_REG_OFS);
		if (timer_reg.bit.collision == 0) {
			break;
		}
		usleep_range(250, 300);
	}

	tm->tm_sec  = timer_reg.bit.sec;
	tm->tm_min  = timer_reg.bit.min;
	tm->tm_hour = timer_reg.bit.hour;
	tm->tm_mday = days;
	tm->tm_mon  = months;
	tm->tm_year = years;

	pr_debug("after read time: sec = %d, min = %d, hour = %d, mday = %d," \
			 "mon = %d, year = %d, wday = %d, yday = %d," \
			 "\n", tm->tm_sec, tm->tm_min, tm->tm_hour, tm->tm_mday, \
			 tm->tm_mon, tm->tm_year, tm->tm_wday, tm->tm_yday);

	return rtc_valid_tm(tm);
}

int nvt_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	int year_looper, ret, chk;
	uint32_t days = 0;
	union RTC_TIMER_REG timer_reg;
	union RTC_DAYKEY_REG daykey_reg;
	union RTC_CTRL_REG ctrl_reg;

	pr_debug("kernel set time: sec = %d, min = %d, hour = %d, mday = %d," \
			 "mon = %d, year = %d, wday = %d, yday = %d," \
			 "\n", tm->tm_sec, tm->tm_min, tm->tm_hour, tm->tm_mday, \
			 tm->tm_mon, tm->tm_year, tm->tm_wday, tm->tm_yday);

	ret = rtc_valid_tm(tm);
	if (ret < 0) {
		return ret;
	}

	for (year_looper = 0; year_looper < tm->tm_year; year_looper++) {
		days += rtc_ydays[is_leap_year(year_looper + 1900)][12];
	}

	days += rtc_ydays[is_leap_year(year_looper + 1900)][tm->tm_mon];
	tm->tm_mday--; /*subtract the day which is not ended*/
	days += tm->tm_mday;

	loc_cpu();

	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		dev_err(dev, "RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		unl_cpu();
		return -ETXTBSY;
	}

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.day_sel = 1;
	ctrl_reg.bit.time_sel = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	timer_reg.reg = 0;
	timer_reg.bit.sec = tm->tm_sec;
	timer_reg.bit.min = tm->tm_min;
	timer_reg.bit.hour = tm->tm_hour;
	rtc_setreg(RTC_TIMER_REG_OFS, timer_reg.reg);

	daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
	daykey_reg.bit.day = days;
	rtc_setreg(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

	rtc_trigger_cset();

	// check setting
	for (chk = 0 ; chk < 3 ; chk++) {
		if (nvt_rtc_chek_time(tm, days) == 0) {
			break;
		}
	}

	unl_cpu();

	return ret;
}

static int nvt_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time *tm = &alrm->time;
	uint32_t days, alarm_days, years, month_days = 0;
	union RTC_PWRALM_REG pwralarm_reg;
	union RTC_DAYKEY_REG daykey_reg;
	int count = 0, months;

	loc_cpu();
	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		dev_err(dev, "RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		unl_cpu();
		return -ETXTBSY;
	}

	complete(&cset_completion);
	unl_cpu();

	// Check day collision
	for (count = 0; count < 3; count++) {
		daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
		if (daykey_reg.bit.collision == 0) {
			break;
		}
		usleep_range(250, 300);
	}

	pwralarm_reg.reg = rtc_getreg(RTC_PWRALM_REG_OFS);

	alarm_days = pwralarm_reg.bit.day;
	days = daykey_reg.bit.day;

	if ((days & RTC_INT_MAX_PWRALARM_DAY) > alarm_days) {
		days = ((days & ~RTC_INT_MAX_PWRALARM_DAY) | alarm_days) \
			   + (1 << RTC_INT_PRWALARM_DAY_SHIFT);
	} else {
		days = ((days & ~RTC_INT_MAX_PWRALARM_DAY) | alarm_days);
	}

	for (years = 0; days >= rtc_ydays[is_leap_year(years + 1900)][12]; \
		 years++) {
		days -= rtc_ydays[is_leap_year(years + 1900)][12];
	}

	for (months = 1; months < 13; months++) {
		if (days <= rtc_ydays[is_leap_year(years + 1900)][months]) {
			days -= rtc_ydays[is_leap_year(years + 1900)][months - 1];
			months--;
			break;
		}
	}
	if( (months < 12) && (months >= 0) ){
		month_days = rtc_ydays[is_leap_year(years + 1900)][months + 1] - \
		rtc_ydays[is_leap_year(years + 1900)][months];
	}

	if (days == month_days) {
		months++;
		days = 1;
	} else {
		days++;    /*Align linux time format*/
	}

	tm->tm_sec  = pwralarm_reg.bit.sec;
	tm->tm_min  = pwralarm_reg.bit.min;
	tm->tm_hour = pwralarm_reg.bit.hour;
	tm->tm_mday = days;
	tm->tm_mon  = months;
	tm->tm_year = years;

	pr_debug("read alarm time: sec = %d, min = %d, hour = %d, mday = %d," \
			 "mon = %d, year = %d, wday = %d, yday = %d," \
			 "\n", tm->tm_sec, tm->tm_min, tm->tm_hour, tm->tm_mday, \
			 tm->tm_mon, tm->tm_year, tm->tm_wday, tm->tm_yday);

	return rtc_valid_tm(tm);
}

int nvt_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time *tm = &alrm->time;
	int year_looper, ret;
	uint32_t current_days = 0, alarm_days = 0;
	union RTC_DAYKEY_REG daykey_reg;
	union RTC_CTRL_REG ctrl_reg;
	union RTC_PWRALM_REG pwralarm_reg;
	union RTC_OSCAN_REG oscan_reg;
	union RTC_ALARM_REG alarm_reg;
	union RTC_STATUS_REG status_reg;

	daykey_reg.reg = rtc_getreg(RTC_DAYKEY_REG_OFS);
	current_days = daykey_reg.bit.day;

	pr_debug("set alarm time: sec = %d, min = %d, hour = %d, mday = %d," \
			 "mon = %d, year = %d, wday = %d, yday = %d," \
			 "\n", tm->tm_sec, tm->tm_min, tm->tm_hour, tm->tm_mday, \
			 tm->tm_mon, tm->tm_year, tm->tm_wday, tm->tm_yday);

	ret = rtc_valid_tm(tm);
	if (ret < 0) {
		return ret;
	}

	for (year_looper = 0; year_looper < tm->tm_year; year_looper++) {
		alarm_days += rtc_ydays[is_leap_year(year_looper + 1900)][12];
	}

	alarm_days += rtc_ydays[is_leap_year(year_looper + 1900)][tm->tm_mon];
	tm->tm_mday--; /*subtract the day which is not ended*/
	alarm_days += tm->tm_mday;

	/*Check date parameter for maximum register setting*/
	if ((alarm_days < current_days) || ((alarm_days - current_days) > \
										RTC_INT_MAX_PWRALARM_DAY)) {
		pr_err("Invalid parameter!\n");
		return E_PAR;
	}

	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		dev_err(dev, "RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		return -ETXTBSY;
	}
	loc_cpu();

	// clear alarm interrupt status
	status_reg.reg = rtc_getreg(RTC_STATUS_REG_OFS);
	if (status_reg.bit.alarm_sts == 1) {
		status_reg.bit.alarm_sts = 1;
		rtc_setreg(RTC_STATUS_REG_OFS, status_reg.reg);
	}

	// pwralarm enable
	rtc_set_pwralarm_en(1);

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.pwralarmtime_sel = 1;
	ctrl_reg.bit.pwralarmday_sel = 1;
	ctrl_reg.bit.alarm_inten = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	pwralarm_reg.reg = 0;
	pwralarm_reg.bit.sec = tm->tm_sec;
	pwralarm_reg.bit.min = tm->tm_min;
	pwralarm_reg.bit.hour = tm->tm_hour;
	pwralarm_reg.bit.day = alarm_days;
	rtc_setreg(RTC_PWRALM_REG_OFS, pwralarm_reg.reg);

	alarm_reg.reg = 0;
	alarm_reg.bit.sec = tm->tm_sec;
	alarm_reg.bit.min = tm->tm_min;
	alarm_reg.bit.hour = tm->tm_hour;
	rtc_setreg(RTC_ALARM_REG_OFS, alarm_reg.reg);

	/*Set OSC analog parameter*/
	oscan_reg.reg = rtc_getreg(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.osc_analogcfg = RTC_INT_OSC_ANALOG_CFG;
	rtc_setreg(RTC_OSCAN_REG_OFS, oscan_reg.reg);
	rtc_trigger_cset();
	unl_cpu();

	return ret;
}

static int nvt_rtc_alarm_irq_enable(struct device * dev, unsigned int enabled)
{
	union RTC_CTRL_REG ctrl_reg;

	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(500))) {
		printk( "RTC alarm en/dis timeout, plz check RTC 32K osc VCC_VRTC\n");
		return -EBUSY;
	}
	loc_cpu();
	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);

	if (enabled) {
		ctrl_reg.bit.alarm_inten = 1;
		rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);
		rtc_set_pwralarm_en(enabled);
		rtc_trigger_cset();
		unl_cpu();
	} else {
		ctrl_reg.bit.alarm_inten = 0;
		rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);
		rtc_set_pwralarm_en(enabled);
		rtc_trigger_cset();
		unl_cpu();
#ifdef CONFIG_PM
		nvt_pm_end();
#endif
	}

	return 0;
}

#if 1
static int nvt_rtc_read_clkdiv_offset(int *offset, int *div_sel)
{
	union RTC_CLKDIV_REG div_reg;

	loc_cpu();
	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		printk("RTC s/w cset timeout, plz check RTC 32K osc VCC_VRTC\n");
		unl_cpu();
		return -ETXTBSY;
	}

	complete(&cset_completion);
	unl_cpu();

	div_reg.reg = rtc_getreg(RTC_CLKDIV_REG_OFS);

	*offset  = div_reg.bit.clk_div_offset;

	*div_sel = div_reg.bit.div_value_sel;
	//printk(" nvt_rtc_read_offset  = 0x%x, div_sel = %d \n",*offset, *div_sel);


	return 0;
}

static int nvt_rtc_set_clkdiv_offset(int offset)
{
	union RTC_CLKDIV_REG div_reg;
	union RTC_CTRL_REG ctrl_reg;

	loc_cpu();
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		return -ETXTBSY;
	}

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.clkdiv_offset_sel = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	// select to use div_offset
	div_reg.reg = rtc_getreg(RTC_CLKDIV_REG_OFS);
	div_reg.bit.clk_div_offset =  offset;
	div_reg.bit.div_value_sel =  0;
	rtc_setreg(RTC_CLKDIV_REG_OFS, div_reg.reg);

	rtc_trigger_cset();
	unl_cpu();

	return 0;
}
static int nvt_rtc_read_clkdiv_default(struct device *dev, long *offset)
{
	union RTC_CLKDIV_REG div_reg;
	int div_sel = 0;

	loc_cpu();
	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		unl_cpu();
		return -ETXTBSY;
	}

	complete(&cset_completion);
	unl_cpu();

	div_reg.reg = rtc_getreg(RTC_CLKDIV_REG_OFS);

	*offset  = div_reg.bit.clk_div_default;
	div_sel = div_reg.bit.div_value_sel;

	printk("nvt_rtc_div_default = 0x%lx \n", *offset);
	//printk("nvt_rtc_read_div_sel = 0x%x \n",div_sel);

	return 0;
}

static int nvt_rtc_set_clkdiv_default(struct device *dev, long div_default)
{
	union RTC_CLKDIV_REG div_reg;
	union RTC_CTRL_REG ctrl_reg;

	loc_cpu();
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		return -ETXTBSY;
	}

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.clkdiv_default_sel = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	div_reg.reg = rtc_getreg(RTC_CLKDIV_REG_OFS);
	div_reg.bit.clk_div_default =  div_default;
	//div_reg.bit.div_value_sel =  0; // select to div_offset
	rtc_setreg(RTC_CLKDIV_REG_OFS, div_reg.reg);

	rtc_trigger_cset();
	unl_cpu();

	return 0;
}
static int nvt_rtc_init_temp_compensation(void)
{
	struct device_node *node, *node_ofs;
	int  dtsi_num = 0, i, k; // j
	int *value;
	int temp_com_funcen = 0;

	char *rtc_dtsi_name[2] = {
		"rtc_en_part", "rtc_part"
	};

	/* temperature compensation func Enable parsing */
	node = of_find_node_by_name(NULL, rtc_dtsi_name[0]);

	if (node == NULL) {
		//printk("[%s] doesn't exist in rtc dts! \n",rtc_dtsi_name[0]);
		return 0;
	} else {
		if (of_property_read_u32_array(node, "rtc_tempcom_func_en", &temp_com_funcen, 1) == 0) {
			if (temp_com_funcen  == 0) {
				return 0;
			}
		}
	}


	/* offset table dtsi parsing */
	node_ofs = of_find_node_by_name(NULL, rtc_dtsi_name[1]);
	if (node_ofs == NULL) {
		//printk("[%s] doesn't exist in rtc dts! \n",rtc_dtsi_name[1]);
		return 0;
	} else {
		dtsi_num = of_property_count_u32_elems(node_ofs, "offset") ;
		dtsi_num_row = dtsi_num / 3;
		//printk("nvt rtc offset dtsi_num = %d \n", dtsi_num );
		value = (int *)kmalloc(sizeof(int) * dtsi_num, GFP_KERNEL);
		if (!value) {
			return 0;
		}

		// creat kmalloc offset_value arrary
		offset_value = (int **)kmalloc(sizeof(int *) * dtsi_num_row, GFP_KERNEL);
		if (!offset_value) {
			goto free_value;
		}

		for (i = 0 ; i < dtsi_num_row ; i++) {
			offset_value[i] = (int *)kmalloc(sizeof(int) * 2, GFP_KERNEL);

			if (!offset_value[i]) {
				for (k = 0 ; k < i ; k ++) {
					kfree(offset_value[k]);
				}
				goto free_ofsset_val;
			}
		}

		if (of_property_read_u32_array(node_ofs, "offset", &value[0], dtsi_num) == 0) {
			for (i = 0 ; i < dtsi_num_row ; i++) {
				// temperature
				if (value[i * 3] == 0) { // temperature <0
					offset_value[i][0] = 0 - value[i * 3 + 1];
				} else {
					offset_value[i][0] = value[i * 3 + 1];
				}

				// offset value
				offset_value[i][1] = value[i * 3 + 2];
			}
		}
		kfree(value);
	}

#if 0
	for (i = 0 ; i < dtsi_num_row ; i++) {
		for (j = 0 ; j < 2 ; j++) {
			printk("offset_value[%d][%d]  =	%d \n", i, j, offset_value[i][j]);
		}
	}
#endif

	return 1;


free_ofsset_val :
	kfree(offset_value);

free_value :
	kfree(value);

	return 0;
}

static int nvt_rtc_get_temp(void)
{
	int temp = 0, cal_temp_table = 0, set_offset = 0, read_offset = 0, div_sel = 0;
	int i ;

	nvt_thermal_get_temp(&temp);
	//printk(" rtc_get_thermal temperature = %d \r\n",temp);

	cal_temp_table = 0;

	// find temperature offset table
	for (i = 0; i < dtsi_num_row; i++) {
		if (temp < offset_value[i][0]) {

			if (i == 0) {
				cal_temp_table = i; // temp is mini value
			} else {
				if ((offset_value[i][0] - temp) > (temp - offset_value[i - 1][0])) {
					cal_temp_table = i - 1;
				} else {
					cal_temp_table = i;
				}
			}
			break;
		}
		// temp is maxi value
		if (i == (dtsi_num_row - 1)) {
			cal_temp_table = dtsi_num_row - 1 ;
		}
	}

	set_offset = offset_value[cal_temp_table][1];

	//setting
	nvt_rtc_read_clkdiv_offset(&read_offset, &div_sel);

	// if power lost div_sel will reset to 1 (div_default)
	if ((read_offset != set_offset)  || (div_sel != 0)) {
		nvt_rtc_set_clkdiv_offset(set_offset);
		//nvt_rtc_read_clkdiv_offset(&read_offset, &div_sel);
		//printk("nvt_rtc clk_div_offset = %d ,div_sel = %d \r\n",read_offset, div_sel);
	}

	return 0;
}

static void rtc_temp_work(struct work_struct *work)
{
	union RTC_CLKDIV_REG div_reg;
	union RTC_CTRL_REG ctrl_reg;
	const char *symbol_name ;
	void *addr;
	int i = 0;

	struct nvt_rtc_priv *priv;
	struct delayed_work *delay_work;

	delay_work = to_delayed_work(work);
	priv = container_of( delay_work, struct nvt_rtc_priv, temp_work);
	
	symbol_name = "nvt_thermal_get_temp";
	addr = __symbol_get(symbol_name);

	if (addr == NULL) {
		msleep(10000);
		symbol_name = "nvt_thermal_get_temp";
		addr = __symbol_get(symbol_name);
	}

	if (addr == NULL) {
		printk(" please check thermal module opne when Eanable RTC temperature compensation  ! \n");

		// kfree offset_value
		for (i = 0 ; i < dtsi_num_row ; i++) {
			kfree(offset_value[i]);
		}
		kfree(offset_value);

	} else {
		// set rtc clk divider to offset firstly
		loc_cpu();
		if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
			printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
			return;
		}

		ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
		ctrl_reg.bit.cset = 0;
		ctrl_reg.bit.clkdiv_offset_sel = 1;
		rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

		// select to use div_offset
		div_reg.reg = rtc_getreg(RTC_CLKDIV_REG_OFS);
		div_reg.bit.div_value_sel =  0;
		rtc_setreg(RTC_CLKDIV_REG_OFS, div_reg.reg);

		rtc_trigger_cset();
		unl_cpu();

		nvt_rtc_get_temp();
		schedule_delayed_work(&priv->temp_work, msecs_to_jiffies(10000));
	}
}

#endif
static const struct rtc_class_ops nvt_rtc_ops = {
	.ioctl      = nvt_rtc_ioctl,
	.proc       = nvt_rtc_proc,
	.read_time  = nvt_rtc_read_time,
	.set_time   = nvt_rtc_set_time,
	.read_alarm = nvt_rtc_read_alarm,
	.set_alarm  = nvt_rtc_set_alarm,
	.read_offset = nvt_rtc_read_clkdiv_default,
	.set_offset = nvt_rtc_set_clkdiv_default,
	.alarm_irq_enable = nvt_rtc_alarm_irq_enable,
};

/*
    User data related
*/
static int data_proc_show(struct seq_file *seq, void *v)
{
	union RTC_DATA_REG data_reg;

	loc_cpu();
	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		unl_cpu();
		return -ETXTBSY;
	}
	complete(&cset_completion);
	unl_cpu();

	data_reg.reg = rtc_getreg(RTC_DATA_REG_OFS);
	seq_printf(seq, "0x%x\n", (data_reg.bit.data0 & 0xFF00) >> 8);

	return 0;
}

static int data_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, data_proc_show, NULL);
}

#define MAX_CMD_LENGTH 30
static ssize_t data_proc_cmd_write(struct file *file, const char __user *buf,
								   size_t size, loff_t *off)
{
	int val, usr_data0;
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	union RTC_CTRL_REG ctrl_reg;
	union RTC_DATA_REG data_reg;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk("%s: Command length %d is too long\n", __func__, len);
		return -EINVAL;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		return -EINVAL;
	}

	if (len == 0) {
		cmd_line[0] = '\0';
	} else {
		cmd_line[len - 1] = '\0';
	}


	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);

	loc_cpu();

	/*wait for cset done*/
	if (!wait_for_completion_timeout(&cset_completion, msecs_to_jiffies(1000))) {
		printk("RTC s/w reset timeout, plz check RTC 32K osc VCC_VRTC\n");
		unl_cpu();
		return -ETXTBSY;
	}

	ctrl_reg.reg = rtc_getreg(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.data0_sel = 1;
	rtc_setreg(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	data_reg.reg = rtc_getreg(RTC_DATA_REG_OFS);
	usr_data0 = (data_reg.bit.data0) & 0x00FF;
	usr_data0 = ((val & 0X000000FF) << 8) | usr_data0;

	data_reg.bit.data0 = usr_data0;
	rtc_setreg(RTC_DATA_REG_OFS, data_reg.reg);

	rtc_trigger_cset();

	unl_cpu();

	return size;
}

static struct proc_ops proc_data_fops = {
	.proc_open      = data_proc_open,
	.proc_read      = seq_read,
	.proc_lseek     = seq_lseek,
	.proc_release   = single_release,
	.proc_write     = data_proc_cmd_write,
};

static int nvt_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	struct nvt_rtc_priv *priv;
	struct resource *memres = NULL;
	struct proc_dir_entry *pentry = NULL;
	int ret = 0, irq = 0, temp_com_funcen = 0, reset_date = 0 ;

	memres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(!memres)) {
		dev_err(&pdev->dev, "failed to get resource\n");
		return -ENXIO;
	}
	
	_REGIOBASE = devm_ioremap_resource(&pdev->dev, memres);;
	if (unlikely(_REGIOBASE == 0)) {
		dev_err(&pdev->dev, "failed to get io memory\n");
		return -ENOMEM;
	}

	priv = kzalloc(sizeof(struct nvt_rtc_priv), GFP_KERNEL);
	if (!priv) {
		return -ENOMEM;
	}

	is_power_lost = 0;

	platform_set_drvdata(pdev, priv);

	device_init_wakeup(&pdev->dev, 1);

	/*Set default HW configuration*/
	/*Define what data type will be used, RCW_DEF or manual define*/

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "No IRQ resource!\n");
		goto out;
	}

	// init before request irq to prevent power alarm interrupt when RTC first power on
	init_completion(&cset_completion);
	sema_init(&rtc_sem, 1);

	dev_pm_set_wake_irq(&pdev->dev, irq);

	ret = devm_request_threaded_irq(&pdev->dev, irq,
									rtc_update_handler, rtc_update_handler_thread, IRQF_ONESHOT, pdev->name, priv);
	/* To config rtc irq can wakeup system after system suspended */
	irq_set_irq_wake(irq, 1);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to request IRQ: #%d: %d\n", irq, ret);
		goto out;
	}

	if (of_property_read_u32(pdev->dev.of_node, "rtc_reset_date", &reset_date) != 0) {
		//printk("can't not find  <rtc_reset_date> in dtsi ");
		reset_date = 0; // reset date when power lost Anyway
	}

	nvt_rtc_chk_power(&pdev->dev, reset_date);

	pentry = proc_create("rtc_data", S_IRUGO | S_IXUGO, NULL, &proc_data_fops);
	if (pentry == NULL) {
		dev_err(&pdev->dev, "failed to create data proc!\n");
		ret = -EINVAL;
		goto out_proc_data;
	}
	priv->pproc_data = pentry;

	rtc =  devm_rtc_device_register(&pdev->dev, "nvt_rtc",
									&nvt_rtc_ops, THIS_MODULE);

	rtc->uie_unsupported = 1;

	if (IS_ERR(rtc)) {
		ret = PTR_ERR(rtc);
		goto out_proc_data;
	}
	priv->rtc = rtc;

	/* tempeture compensation */
	temp_com_funcen = nvt_rtc_init_temp_compensation();

	// Enable temperature compensation function work
	if (temp_com_funcen) {
		INIT_DELAYED_WORK(&priv->temp_work, rtc_temp_work);
		schedule_delayed_work(&priv->temp_work, msecs_to_jiffies(10000));
	}

	return 0;

out_proc_data:
	if (priv->pproc_data) {
		proc_remove(priv->pproc_data);
	}
out:
	if (priv->rtc)
		//rtc_device_unregister(rtc);

	{
		kfree(priv);
	}
	return ret;
}

static int nvt_rtc_remove(struct platform_device *pdev)
{
	struct nvt_rtc_priv *priv = platform_get_drvdata(pdev);
	int temp_com_funcen = 0;
	struct device_node *node;

	/* temperature compensation func Enable parsing */
	node = of_find_node_by_name(NULL, "rtc_en_part");
	if (node == NULL) {
		temp_com_funcen = 0;
	} else {
		if (of_property_read_u32_array(node, "rtc_tempcom_func_en", &temp_com_funcen, 1) == 0) {

		} else {
			temp_com_funcen = 0;
		}
	}

	//cancel_delayed_work
	if (temp_com_funcen) {
		cancel_delayed_work_sync(&priv->temp_work);
	}

	if (priv->pproc_data) {
		proc_remove(priv->pproc_data);
	}
	//rtc_device_unregister(rtc);
	kfree(priv);
	return 0;
}

// Suspend Resume
// reference rtc/class.c rtc_suspend() / rtc_resume()
#ifdef CONFIG_PM
static struct timespec64 old_rtc, old_system, old_delta;
int nvt_rtc_suspend(struct device *dev)
{
	struct rtc_device   *rtc = to_rtc_device(dev);
	struct rtc_time     tm;
	struct timespec64   delta, delta_delta;
	int err;

	/* snapshot the current RTC and system time at suspend*/
	err = nvt_rtc_read_time(dev, &tm);
	if (err < 0) {
		pr_debug("%s:  fail to read rtc time\n", dev_name(&rtc->dev));
		return 0;
	}

	ktime_get_real_ts64(&old_system);
	old_rtc.tv_sec = rtc_tm_to_time64(&tm);
	
	/*
	* To avoid drift caused by repeated suspend/resumes,
	* which each can add ~1 second drift error,
	* try to compensate so the difference in system time
	* and rtc time stays close to constant.
	*/
    delta = timespec64_sub(old_system, old_rtc);
    delta_delta = timespec64_sub(delta, old_delta);

	/*
	* if delta_delta is too large, assume time correction
	* has occurred and set old_delta to the current delta.
	*/
    if (delta_delta.tv_sec < -2 || delta_delta.tv_sec >= 2) {
		old_delta = delta;
		
    } else {
        /* Otherwise try to adjust old_system to compensate */
        old_system = timespec64_sub(old_system, delta_delta);
	}

	return 0;
}

int nvt_rtc_resume(struct device *dev)
{
	struct rtc_device   *rtc = to_rtc_device(dev);
	struct rtc_time     tm;
	struct timespec64   new_system, new_rtc;
	struct timespec64   sleep_time;
	int err;

	/* snapshot the current rtc and system time at resume */
	ktime_get_real_ts64(&new_system);
	err = nvt_rtc_read_time(dev, &tm);
	if (err < 0) {
		pr_debug("%s:  fail to read rtc time\n", dev_name(&rtc->dev));
		return 0;
	}

	new_rtc.tv_sec = rtc_tm_to_time64(&tm);
	new_rtc.tv_nsec = 0;

	if (new_rtc.tv_sec < old_rtc.tv_sec) {
		pr_debug("%s:  time travel!\n", dev_name(&rtc->dev));
		return 0;
	}

	/* calculate the RTC time delta (sleep time)*/
	sleep_time = timespec64_sub(new_rtc, old_rtc);

    /*
	* Since these RTC suspend/resume handlers are not called
	* at the very end of suspend or the start of resume,
	* some run-time may pass on either sides of the sleep time
	* so subtract kernel run-time between rtc_suspend to rtc_resume
	* to keep things accurate.
	*/
	sleep_time = timespec64_sub(sleep_time,
	timespec64_sub(new_system, old_system)); //already compensate in timekeeping_resume 

	if (sleep_time.tv_sec >= 0)
		timekeeping_inject_sleeptime64(&sleep_time);

	return 0;
}

static const struct dev_pm_ops nvt_rtc_pm = {
	.suspend        = nvt_rtc_suspend,
	.resume         = nvt_rtc_resume,
};

#define nvt_rtc_pm_ops (&nvt_rtc_pm)
#else
#define nvt_rtc_pm_ops NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id nvt_rtc_match[] = {
	{ .compatible = "nvt,nvt_rtc" },
	{},
};
MODULE_DEVICE_TABLE(of, nvt_rtc_match);
#endif

static struct platform_driver nvt_rtc_platform_driver = {
	.driver     = {
		.name   = "nvt_rtc",
		.owner  = THIS_MODULE,
		.pm     = nvt_rtc_pm_ops,
#ifdef CONFIG_OF
		.of_match_table = nvt_rtc_match,
#endif
	},
	.probe      = nvt_rtc_probe,
	.remove     = nvt_rtc_remove,
};

static int __init nvt_rtc_init(void)
{
	int ret;

	ret = platform_driver_register(&nvt_rtc_platform_driver);
	return ret;
}

static void __exit nvt_rtc_exit(void)
{
	platform_driver_unregister(&nvt_rtc_platform_driver);
}

MODULE_AUTHOR("Novatek");
MODULE_DESCRIPTION("nvt RTC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_ALIAS("platform:rtc-nvt");

EXPORT_SYMBOL(nvt_rtc_read_time);

module_init(nvt_rtc_init);
module_exit(nvt_rtc_exit);
