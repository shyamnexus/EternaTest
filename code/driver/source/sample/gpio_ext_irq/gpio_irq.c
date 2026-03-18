/**
    @file       gpio_irq.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
**/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <plat/nvt-gpio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeirq.h>
#include <linux/suspend.h>

#define RED   "\x1B[31m"
#define RESET "\x1B[0m"

#define MY_GPIO_INT_NAME "my_button_int"
#define MY_DEV_NAME "my_device"

static int button_irq = 0;
static int button_id = P_GPIO(19);
module_param(button_id, int, 0);
MODULE_PARM_DESC(button_id, "gpio number for IRQ");

static irqreturn_t gpio_irq_test_isr(int irq, void *data)
{
	printk("START "RED"button_isr !!!"RESET", func=%s, line=%d\n", __FUNCTION__, __LINE__);
	pm_system_wakeup();
	return IRQ_HANDLED;
}

static int __init gpio_irq_test_init(void)
{
	int ret = 0;
	struct platform_device *pdev = platform_device_register_simple("gpio_ext_irq", -1, NULL, 0);
	if (!pdev) {
		printk("platform_device_register_simple fail, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = gpio_is_valid(button_id);
	if (!ret) {
		printk(" gpio is not valid, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = gpio_request(button_id,"button_id");
	if (ret < 0) {
		printk(" gpio request fail, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	button_irq = gpio_to_irq(button_id);
	if (button_irq < 0) {
		printk(" gpio_to_irq fail, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	dev_pm_set_wake_irq(&pdev->dev, button_irq);

	device_set_wakeup_capable(&pdev->dev, true);

	irq_set_irq_wake(button_irq, 1);
	ret = request_irq( button_irq, gpio_irq_test_isr, IRQF_TRIGGER_FALLING, MY_GPIO_INT_NAME, MY_DEV_NAME);
	//ret = request_irq( button_irq, gpio_irq_test_isr, IRQF_TRIGGER_LOW, MY_GPIO_INT_NAME, MY_DEV_NAME);
	if (ret) {
		printk(" request_irq fail, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	printk(" Function %s sucessfully. \n", __FUNCTION__);

	return 0;
}

static void __exit gpio_irq_test_exit(void)
{
	free_irq(button_irq, MY_DEV_NAME);
	gpio_free(button_id);
}

module_init(gpio_irq_test_init);
module_exit(gpio_irq_test_exit);
MODULE_LICENSE("GPL");
