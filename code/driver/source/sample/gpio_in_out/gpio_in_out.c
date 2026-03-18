/**
    @file       gpio_in_out.c
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

#define RED   "\x1B[31m"
#define RESET "\x1B[0m"

#define BUTTON_1 P_GPIO(20)
#define BUTTON_2 S_GPIO(10)
#define BUTTON_3 D_GPIO(9)
#define BUTTON_4 A_GPIO(1)


static int __init gpio_in_out_test_init(void)
{
	int ret = 0, i = 0;
	int g_value_1 = 0, g_value_2 = 0, g_value_3 = 0, g_value_4 = 0;

	ret = gpio_is_valid(BUTTON_1);
	if (!ret) {
		printk(" gpio is not valid, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = gpio_request(BUTTON_1, "BUTTON_1");
	if (ret < 0) {
		printk(" gpio request fail, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = gpio_is_valid(BUTTON_2);
	if (!ret) {
		printk(" gpio is not valid, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = gpio_request(BUTTON_2, "BUTTON_2");
	if (ret < 0) {
		printk(" gpio request fail, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = gpio_is_valid(BUTTON_3);
	if (!ret) {
		printk(" gpio is not valid, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = gpio_request(BUTTON_3, "BUTTON_3");
	if (ret < 0) {
		printk(" gpio request fail, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ret = gpio_is_valid(BUTTON_4);
	if (!ret) {
		printk(" gpio is not valid, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = gpio_request(BUTTON_4, "BUTTON_4");
	if (ret < 0) {
		printk(" gpio request fail, func=%s, line=%d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	// Test gpio output high/low
	gpio_direction_output(BUTTON_1, 1);
	for (i = 0; i < 5; i++) {
		g_value_1 = gpio_get_value(BUTTON_1);
		printk(" org_gpio %s value = %d , %s - %d  \n", "P_GPIO(11)", g_value_1, __FUNCTION__, __LINE__);
		gpio_set_value(BUTTON_1, 1);
		g_value_1 = gpio_get_value(BUTTON_1);
		printk(" set_HIGH_gpio %s value = %d , %s - %d  \n", "P_GPIO(11)", g_value_1, __FUNCTION__, __LINE__);
		msleep(3000);
		gpio_set_value(BUTTON_1, 0);
		g_value_1 = gpio_get_value(BUTTON_1);
		printk(" set_LOW_gpio %s value = %d , %s - %d  \n", "P_GPIO(11)", g_value_1, __FUNCTION__, __LINE__);
		msleep(3000);
	}

	gpio_direction_output(BUTTON_2, 1);
	for (i = 0; i < 5; i++) {
		g_value_2 = gpio_get_value(BUTTON_2);
		printk(" org_gpio %s value = %d , %s - %d  \n", "D_GPIO(1)", g_value_2, __FUNCTION__, __LINE__);
		gpio_set_value(BUTTON_2, 1);
		g_value_2 = gpio_get_value(BUTTON_2);
		printk(" set_HIGH_gpio %s value = %d , %s - %d  \n", "D_GPIO(1)", g_value_2, __FUNCTION__, __LINE__);
		msleep(3000);
		gpio_set_value(BUTTON_2, 0);
		g_value_2 = gpio_get_value(BUTTON_2);
		printk(" set_LOW_gpio %s value = %d , %s - %d  \n", "D_GPIO(1)", g_value_2, __FUNCTION__, __LINE__);
		msleep(3000);
	}

	gpio_direction_output(BUTTON_3, 1);
	for (i = 0; i < 5; i++) {
		g_value_3 = gpio_get_value(BUTTON_3);
		printk(" org_gpio %s value = %d , %s - %d  \n", "S_GPIO(1)", g_value_3, __FUNCTION__, __LINE__);
		gpio_set_value(BUTTON_3, 1);
		g_value_3 = gpio_get_value(BUTTON_3);
		printk(" set_HIGH_gpio %s value = %d , %s - %d  \n", "S_GPIO(1)", g_value_3, __FUNCTION__, __LINE__);
		msleep(3000);
		gpio_set_value(BUTTON_3, 0);
		g_value_3 = gpio_get_value(BUTTON_3);
		printk(" set_LOW_gpio %s value = %d , %s - %d  \n", "S_GPIO(1)", g_value_3, __FUNCTION__, __LINE__);
		msleep(3000);
	}

	gpio_direction_output(BUTTON_4, 1);
	for (i = 0; i < 5; i++) {
		g_value_4 = gpio_get_value(BUTTON_4);
		printk(" org_gpio %s value = %d , %s - %d  \n", "S_GPIO(15)", g_value_4, __FUNCTION__, __LINE__);
		gpio_set_value(BUTTON_4, 1);
		g_value_4 = gpio_get_value(BUTTON_4);
		printk(" set_HIGH_gpio %s value = %d , %s - %d  \n", "S_GPIO(15)", g_value_4, __FUNCTION__, __LINE__);
		msleep(3000);
		gpio_set_value(BUTTON_4, 0);
		g_value_4 = gpio_get_value(BUTTON_4);
		printk(" set_LOW_gpio %s value = %d , %s - %d  \n", "S_GPIO(15)", g_value_4, __FUNCTION__, __LINE__);
		msleep(3000);
	}

	/*
	// Test gpio input
	gpio_direction_input(BUTTON_1);
	gpio_direction_input(BUTTON_2);
	gpio_direction_input(BUTTON_3);
	gpio_direction_input(BUTTON_4);

	for (i = 0; i < 300; i++) {
		g_value_1 = gpio_get_value(BUTTON_1);
		g_value_2 = gpio_get_value(BUTTON_2);
		g_value_3 = gpio_get_value(BUTTON_3);
		g_value_4 = gpio_get_value(BUTTON_4);
		printk(" current gpio value: P_GPIO(11)=%d, D_GPIO(1)=%d, S_GPIO(1)=%d, S_GPIO(15)=%d \n", g_value_1, g_value_2, g_value_3, g_value_4);
		msleep(1000);
	}
	*/

	printk(" Function %s sucessfully. \n", __FUNCTION__);
	return 0;
}

static void __exit gpio_in_out_test_exit(void)
{
	gpio_free(BUTTON_1);
	gpio_free(BUTTON_2);
	gpio_free(BUTTON_3);
	gpio_free(BUTTON_4);
}

module_init(gpio_in_out_test_init);
module_exit(gpio_in_out_test_exit);
MODULE_LICENSE("GPL");
