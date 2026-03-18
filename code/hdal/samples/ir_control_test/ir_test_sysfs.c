/*
 * IR LED PWM Control Test - Sysfs Interface
 * 
 * This sample demonstrates controlling the IR LED via the Linux sysfs PWM interface.
 * Tested on NT98538 platform.
 *
 * Hardware:
 *   - IR LED: PWM channel 11 (P_GPIO11)
 *   - IR Cut: L_GPIO18 (IRC_B), L_GPIO19 (IRC_A)
 *
 * Usage:
 *   ir_test_sysfs led on [brightness]   - Turn on IR LED (brightness 0-100, default 75)
 *   ir_test_sysfs led off               - Turn off IR LED
 *   ir_test_sysfs cut day               - IR Cut to day mode (block IR)
 *   ir_test_sysfs cut night             - IR Cut to night mode (pass IR)
 *   ir_test_sysfs test                  - Run full test sequence
 *
 * Compile:
 *   aarch64-linux-gnu-gcc -o ir_test_sysfs ir_test_sysfs.c -Wall -O2
 *
 * Sysfs vs Kernel Driver Trade-offs:
 * -----------------------------------
 * SYSFS Advantages:
 *   + No kernel module required
 *   + Works on any Linux system with PWM subsystem
 *   + Easy to debug (cat/echo from shell)
 *   + No special permissions beyond file access
 *   + Portable across kernel versions
 *
 * SYSFS Disadvantages:
 *   - Slightly higher latency (file I/O overhead)
 *   - Less precise timing control
 *   - Multiple syscalls per operation
 *
 * KERNEL DRIVER Advantages:
 *   + Lower latency (direct register access)
 *   + More precise timing control
 *   + Can be atomic operations
 *   + Better for high-frequency updates
 *
 * KERNEL DRIVER Disadvantages:
 *   - Requires kernel module development
 *   - Platform-specific code
 *   - Harder to debug
 *   - May need to handle kernel version changes
 *
 * RECOMMENDATION:
 *   For IR LED control (< 100Hz updates), sysfs is sufficient and more portable.
 *   Kernel driver is only needed for high-speed PWM applications (motor control, etc.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* PWM Configuration */
#define PWM_CHIP_PATH       "/sys/class/pwm/pwmchip0"
#define PWM_IR_LED_CHANNEL  11
#define PWM_PERIOD_NS       1000    /* 1000ns = 1MHz frequency */

/* GPIO Configuration for IR Cut */
#define GPIO_EXPORT_PATH    "/sys/class/gpio/export"
#define GPIO_UNEXPORT_PATH  "/sys/class/gpio/unexport"
#define L_GPIO_BASE         96
#define GPIO_IRC_B          (L_GPIO_BASE + 18)  /* L_GPIO18 = 114 */
#define GPIO_IRC_A          (L_GPIO_BASE + 19)  /* L_GPIO19 = 115 */

/*===========================================================================*/
/* Helper Functions                                                          */
/*===========================================================================*/

static int write_file(const char *path, const char *value)
{
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        /* Silently fail for export (may already be exported) */
        return -1;
    }
    
    int ret = write(fd, value, strlen(value));
    close(fd);
    
    return (ret > 0) ? 0 : -1;
}

static int write_file_int(const char *path, int value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    return write_file(path, buf);
}

static int read_file_int(const char *path, int *value)
{
    char buf[32];
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    
    int ret = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    
    if (ret > 0) {
        buf[ret] = '\0';
        *value = atoi(buf);
        return 0;
    }
    return -1;
}

/*===========================================================================*/
/* PWM Control Functions                                                     */
/*===========================================================================*/

static int pwm_export(int channel)
{
    char path[128];
    
    /* Check if already exported */
    snprintf(path, sizeof(path), "%s/pwm%d/enable", PWM_CHIP_PATH, channel);
    if (access(path, F_OK) == 0) {
        printf("PWM%d already exported\n", channel);
        return 0;
    }
    
    /* Export the channel */
    snprintf(path, sizeof(path), "%s/export", PWM_CHIP_PATH);
    if (write_file_int(path, channel) < 0) {
        perror("Failed to export PWM");
        return -1;
    }
    
    usleep(100000);  /* Wait for sysfs to create files */
    printf("PWM%d exported\n", channel);
    return 0;
}

static int pwm_unexport(int channel)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/unexport", PWM_CHIP_PATH);
    return write_file_int(path, channel);
}

static int pwm_set_period(int channel, int period_ns)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/period", PWM_CHIP_PATH, channel);
    return write_file_int(path, period_ns);
}

static int pwm_set_duty_cycle(int channel, int duty_ns)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/duty_cycle", PWM_CHIP_PATH, channel);
    return write_file_int(path, duty_ns);
}

static int pwm_set_polarity(int channel, const char *polarity)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/polarity", PWM_CHIP_PATH, channel);
    return write_file(path, polarity);
}

static int pwm_enable(int channel, int enable)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/enable", PWM_CHIP_PATH, channel);
    return write_file_int(path, enable ? 1 : 0);
}

/*===========================================================================*/
/* GPIO Control Functions                                                    */
/*===========================================================================*/

static int gpio_export(int gpio)
{
    char path[128];
    
    /* Check if already exported */
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio);
    if (access(path, F_OK) == 0) {
        return 0;  /* Already exported */
    }
    
    write_file_int(GPIO_EXPORT_PATH, gpio);
    usleep(10000);
    return 0;
}

static int gpio_set_direction(int gpio, const char *dir)
{
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio);
    return write_file(path, dir);
}

static int gpio_set_value(int gpio, int value)
{
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);
    return write_file_int(path, value);
}

/*===========================================================================*/
/* IR LED Control                                                            */
/*===========================================================================*/

int ir_led_on(int brightness)
{
    if (brightness < 0) brightness = 0;
    if (brightness > 100) brightness = 100;
    
    int duty_ns = (PWM_PERIOD_NS * brightness) / 100;
    
    printf("Turning IR LED ON (brightness: %d%%, duty: %d ns)\n", brightness, duty_ns);
    
    if (pwm_export(PWM_IR_LED_CHANNEL) < 0) return -1;
    if (pwm_set_period(PWM_IR_LED_CHANNEL, PWM_PERIOD_NS) < 0) {
        perror("Failed to set period");
        return -1;
    }
    if (pwm_set_duty_cycle(PWM_IR_LED_CHANNEL, duty_ns) < 0) {
        perror("Failed to set duty cycle");
        return -1;
    }
    pwm_set_polarity(PWM_IR_LED_CHANNEL, "normal");
    if (pwm_enable(PWM_IR_LED_CHANNEL, 1) < 0) {
        perror("Failed to enable PWM");
        return -1;
    }
    
    printf("IR LED ON - PWM%d enabled\n", PWM_IR_LED_CHANNEL);
    return 0;
}

int ir_led_off(void)
{
    printf("Turning IR LED OFF\n");
    
    if (pwm_enable(PWM_IR_LED_CHANNEL, 0) < 0) {
        perror("Failed to disable PWM");
        return -1;
    }
    
    printf("IR LED OFF - PWM%d disabled\n", PWM_IR_LED_CHANNEL);
    return 0;
}

/*===========================================================================*/
/* IR Cut Filter Control                                                     */
/*===========================================================================*/

int ir_cut_init(void)
{
    gpio_export(GPIO_IRC_A);
    gpio_export(GPIO_IRC_B);
    gpio_set_direction(GPIO_IRC_A, "out");
    gpio_set_direction(GPIO_IRC_B, "out");
    gpio_set_value(GPIO_IRC_A, 0);
    gpio_set_value(GPIO_IRC_B, 0);
    return 0;
}

int ir_cut_day(void)
{
    printf("IR Cut: Switching to DAY mode (block IR)\n");
    
    ir_cut_init();
    
    /* Pulse IRC_B to switch to day mode */
    gpio_set_value(GPIO_IRC_A, 0);
    gpio_set_value(GPIO_IRC_B, 1);
    usleep(100000);  /* 100ms pulse */
    gpio_set_value(GPIO_IRC_B, 0);
    
    printf("IR Cut: DAY mode set (IRC_B pulsed, GPIO%d)\n", GPIO_IRC_B);
    return 0;
}

int ir_cut_night(void)
{
    printf("IR Cut: Switching to NIGHT mode (pass IR)\n");
    
    ir_cut_init();
    
    /* Pulse IRC_A to switch to night mode */
    gpio_set_value(GPIO_IRC_B, 0);
    gpio_set_value(GPIO_IRC_A, 1);
    usleep(100000);  /* 100ms pulse */
    gpio_set_value(GPIO_IRC_A, 0);
    
    printf("IR Cut: NIGHT mode set (IRC_A pulsed, GPIO%d)\n", GPIO_IRC_A);
    return 0;
}

/*===========================================================================*/
/* Test Sequence                                                             */
/*===========================================================================*/

void run_test_sequence(void)
{
    printf("\n=== IR Control Test Sequence ===\n\n");
    
    /* Test IR LED */
    printf("[1/6] IR LED at 25%%\n");
    ir_led_on(25);
    sleep(2);
    
    printf("\n[2/6] IR LED at 50%%\n");
    ir_led_on(50);
    sleep(2);
    
    printf("\n[3/6] IR LED at 100%%\n");
    ir_led_on(100);
    sleep(2);
    
    printf("\n[4/6] IR LED OFF\n");
    ir_led_off();
    sleep(1);
    
    /* Test IR Cut (if hardware is mounted) */
    printf("\n[5/6] IR Cut to NIGHT mode\n");
    ir_cut_night();
    sleep(2);
    
    printf("\n[6/6] IR Cut to DAY mode\n");
    ir_cut_day();
    sleep(1);
    
    printf("\n=== Test Complete ===\n");
}

/*===========================================================================*/
/* Main                                                                      */
/*===========================================================================*/

void print_usage(const char *prog)
{
    printf("Usage: %s <command> [args]\n", prog);
    printf("\nCommands:\n");
    printf("  led on [brightness]  - Turn on IR LED (brightness 0-100, default 75)\n");
    printf("  led off              - Turn off IR LED\n");
    printf("  cut day              - IR Cut to day mode (block IR)\n");
    printf("  cut night            - IR Cut to night mode (pass IR)\n");
    printf("  test                 - Run full test sequence\n");
    printf("\nExamples:\n");
    printf("  %s led on 50         - IR LED at 50%% brightness\n", prog);
    printf("  %s led off           - Turn off IR LED\n", prog);
    printf("  %s test              - Run all tests\n", prog);
    printf("\nHardware:\n");
    printf("  IR LED:  PWM%d (P_GPIO11)\n", PWM_IR_LED_CHANNEL);
    printf("  IR Cut:  GPIO%d (IRC_B, L_GPIO18), GPIO%d (IRC_A, L_GPIO19)\n", 
           GPIO_IRC_B, GPIO_IRC_A);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "led") == 0) {
        if (argc < 3) {
            printf("Error: 'led' requires 'on' or 'off'\n");
            return 1;
        }
        
        if (strcmp(argv[2], "on") == 0) {
            int brightness = (argc > 3) ? atoi(argv[3]) : 75;
            return ir_led_on(brightness);
        } else if (strcmp(argv[2], "off") == 0) {
            return ir_led_off();
        } else {
            printf("Unknown led command: %s\n", argv[2]);
            return 1;
        }
    } else if (strcmp(argv[1], "cut") == 0) {
        if (argc < 3) {
            printf("Error: 'cut' requires 'day' or 'night'\n");
            return 1;
        }
        
        if (strcmp(argv[2], "day") == 0) {
            return ir_cut_day();
        } else if (strcmp(argv[2], "night") == 0) {
            return ir_cut_night();
        } else {
            printf("Unknown cut command: %s\n", argv[2]);
            return 1;
        }
    } else if (strcmp(argv[1], "test") == 0) {
        run_test_sequence();
        return 0;
    } else {
        printf("Unknown command: %s\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}
