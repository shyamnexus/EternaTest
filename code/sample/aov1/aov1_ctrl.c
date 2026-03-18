#include "aov1_common.h"
#include "aov1_ctrl.h"
#include <hdal.h>
#define ON_PATH "/sys/class/gpio/gpio193/value"

extern UINT32 check_t1,check_t2,check_t3,check_t4 ,check_t5;
UINT32 time_wakeup = 0;
UINT32 time_sleep = 0;

//get char without press enter
char getche(void)  
{
    struct termios oldt, newt;
    char a;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    a = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return a;
}
//keyboard hit detect
int kbhit(void) 
{
	struct termios term;
	tcgetattr(0, &term);
	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &term);

	int bytes_waiting;
	ioctl(0, FIONREAD, &bytes_waiting);

	term.c_lflag |= ICANON | ECHO;
	tcsetattr(0, TCSANOW, &term);

	return bytes_waiting > 0;
}

static void _system_power_down(void)
{
	static int fd_state = -1, len;

	if(fd_state < 0){
		fd_state = open("/sys/power/state", O_RDWR);
		if(fd_state < 0){
			printf("fail to open /sys/power/state\n");
			return;
		}
	}
	//gpio_set_value(193, 0);
	//system("echo mem > /sys/power/state");
	len = write(fd_state, "mem", strlen("mem"));
	if(len != (int)strlen("mem")){
		printf("fail to write mem to /sys/power/state\n");
		return;
	}
	//gpio_set_value(193, 1);
}

int gpio_export(unsigned pin)
{   
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "echo %d > /sys/class/gpio/export", pin);
	system(buffer);
	return 0;
}

int gpio_set_dir(unsigned pin,int dir)
{   
	char buffer[1024];
	int fd;
	gpio_export(pin);
	snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(buffer, O_WRONLY);
	if (fd == -1) {
	    printf("GPIO path open error");
	    return -1;
	}
	if (dir == 0) {
	    write(fd, "in", 2);
	} else if (dir == 1) {
	    write(fd, "out", 3);
	}
	close(fd);
	return 0;
}

int gpio_get_value(unsigned pin)
{
    char buffer[1024];
    int fd;
    char value;
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(buffer, O_RDONLY);
    if (fd == -1) {
        printf("Error : Unable to open gpio%d/value!!", pin);
        return -1;
    }
    if (read(fd, &value, 1) != 1) {
        printf("Error : Unable to read gpio%d/value!!", pin);
        close(fd);
        return -1;
    }
    close(fd);
    return value - '0';
}

int gpio_set_value(unsigned pin, int value)
{
    char buffer[1024];
    int fd;
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(buffer, O_WRONLY);
    if (fd == -1) {
        printf("Error : Unable to open gpio%d/value!!", pin);
        return -1;
    }
    if (value == 0 || value == 1) {
        char val = (value == 0) ? '0' : '1';
        if (write(fd, &val, 1) != 1) {
            printf("Error : Unable to write gpio %d to %d", pin, value);
            close(fd);
            return -1;
        }
    } else {
        printf("Error : Invalid value %d", value);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int pwm_en(unsigned pwm_id, int enable)
{
    char buffer[1024];
    int fd;
    snprintf(buffer, sizeof(buffer), "/sys/class/pwm/pwmchip0/pwm%d/enable", pwm_id);
    fd = open(buffer, O_WRONLY);
    if (fd == -1) {
        printf("Error : Unable to open pwm%d/enable!!", pwm_id);
        return -1;
    }
    if (enable == 0 || enable == 1) {
        char val = (enable == 0) ? '0' : '1';
        if (write(fd, &val, 1) != 1) {
            printf("Error : Unable to write pwm %d to %d", pwm_id, enable);
            close(fd);
            return -1;
        }
    } else {
        printf("Error : Invalid enable %d", enable);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int yuv_save(HD_VIDEO_FRAME *video_frame, char* str)
{
	uintptr_t phy_addr_main, vir_addr_main;
	UINT32 yuv_size, ret;
	char file_path_main[32] = {0};
	FILE *f_out_main;
	#define _PHY2VIRT_MAIN(pa) (vir_addr_main + ((pa) - phy_addr_main))
	#define DBGINFO_BUFSIZE()	(0x200)
	#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

	phy_addr_main = hd_common_mem_blk2pa(video_frame->blk); // Get physical addr
	if (phy_addr_main == 0) {
		printf("blk2pa fail, blk = \r\n");
    	return -1;
	}

	yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(video_frame->dim.w, video_frame->dim.h, video_frame->pxlfmt);

	// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
	vir_addr_main = (uintptr_t)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, yuv_size);
	if (vir_addr_main == 0) {
		printf("mmap error !!\r\n\r\n");
    	return -1;
	}

	snprintf(file_path_main, 32, "/mnt/sd/dump_%dx%d_frm_%s_%lu.dat",
		video_frame->dim.w, video_frame->dim.h, str, (long unsigned int) video_frame->count);
	printf("dump snapshot frame to file (%s) ....\r\n", file_path_main);


	//----- open output files -----
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		printf("open file (%s) fail....\r\n\r\n", file_path_main);
		return -1;
	}

	//save Y plane
	{
		UINT8 *ptr = (UINT8 *)_PHY2VIRT_MAIN(video_frame->phy_addr[0]);
		UINT32 len = video_frame->loff[0]*video_frame->ph[0];
		if (f_out_main) fwrite(ptr, 1, len, f_out_main);
		if (f_out_main) fflush(f_out_main);
	}
	//save UV plane
	{
		UINT8 *ptr = (UINT8 *)_PHY2VIRT_MAIN(video_frame->phy_addr[1]);
		UINT32 len = video_frame->loff[1]*video_frame->ph[1];
		if (f_out_main) fwrite(ptr, 1, len, f_out_main);
		if (f_out_main) fflush(f_out_main);
	}

	// mummap for frame buffer
	ret = hd_common_mem_munmap((void *)vir_addr_main, yuv_size);
	if (ret != HD_OK) {
		printf("mnumap error !!\r\n\r\n");
		return -1;
	}
	
	// close output file
	fclose(f_out_main);
	printf("file write yuv done!!\r\n");
	return HD_OK;

}

#if PD_USE_RTC
static BOOL RTC_INIT = FALSE;

//
/*
    Increase time by X seconds.

    Increase time by X seconds.

    @param[in, out] pTimer  Pass original time to this API and return timer + uiSecond
    @prarm[in] uiSecond     How many second to increase
    @return void
*/
void set_powerdown(int value)
{
	char path[32]="/sys/class/rtc/rtc0/wakealarm";
	char text[4];
	static int fd_rtc = -1, len;

	if(fd_rtc < 0){
		fd_rtc = open(path, O_RDWR);
		if(fd_rtc < 0){
			printf("fail to open fd_rtc\n");
			return;
		}
	}
	//system("echo 0 > /sys/class/rtc/rtc0/wakealarm");
	len = write(fd_rtc, "0", strlen("0"));
	if(len != (int)strlen("0")){
		printf("fail to write 0 to %s\n", path);
		return;
	}
	//system("echo +1 > /sys/class/rtc/rtc0/wakealarm");
	//len = write(fd_rtc, "+1", strlen("+1"));
	snprintf(text, sizeof(text), "+%d", value);
	len = write(fd_rtc, text, strlen(text));
	if(len != (int)strlen(text)){
		printf("fail to write %s to %s\n", text, path);
		return;
	}
	time_wakeup = hd_gettime_ms();
}

void system_powerdown(void)
{
	if(!RTC_INIT)
	{
		RTC_INIT = TRUE;
		printf("RTC_INIT \r\n");
		set_powerdown(2);
		msleep(100);
	}
	time_sleep = hd_gettime_ms();
	if( time_wakeup != 0 && (time_sleep - time_wakeup) > 990)
	{
		int overtime = (int)(time_sleep - time_wakeup) ;
		printf("[WRN] This round overtime (%d), reset RTC alarm time (%d)\r\n", overtime , ( overtime / 1000 ) + 2);
		set_powerdown( ( overtime / 1000 ) + 1 );
		msleep(50);
	}
	//else
	//	printf("[DBG] This round exec rime (%d)\r\n", (int)(time_sleep - time_wakeup));

	_system_power_down();
	time_wakeup = hd_gettime_ms();
	
}
#endif // if PD_USE_RTC

#if PD_USE_GPIO
static BOOL PD_GPIO_INIT = FALSE;
//static unsigned int PD_GPIO = DSI_GPIO_6;
//static unsigned int PD_GPIO_INT = GPIO_INT_27;
//
//void emu_gpioISR(UINT32 uiEvent)
//{
//	printf("gpio_disableInt\n");
//	gpio_disableInt(GPIO_INT_27);
//}

void set_powerdown(int delay_time)
{
	//// initial
	//gpio_setIntIsr(PD_GPIO_INT, emu_gpioISR);
	//gpio_setIntTypePol(PD_GPIO_INT, GPIO_INTTYPE_EDGE, GPIO_INTPOL_POSHIGH);
	//gpio_clearIntStatus(PD_GPIO_INT);
	//gpio_enableInt(PD_GPIO_INT);
	// 
	//// set output, set pad sink low
	//gpio_direction_output((PD_GPIO), 0);
	//
	//// Wait the capacitor discharge complete
	//usleep(100 * delay_time);

	//// >>>> The capacitor starts to charge <<<<
	// 
	//// set input and gpio wakeup 
	//gpio_setDir(PD_GPIO, GPIO_DIR_INPUT);
	//gpio_clearIntStatus(PD_GPIO_INT);
	//gpio_enableInt(PD_GPIO_INT);
	//TIME_PRINT(("1:%d\r\n", (int)hd_gettime_us()));
}

void system_powerdown(void)
{
	if(!PD_GPIO_INIT)
	{
		printf("======> First system_powerdown\r\n");
		PD_GPIO_INIT = TRUE;
		system("mem w 0xF0030008 A6515555");
		set_powerdown(1);
	}

		

	//printf("======> [Time] clk_powerdown S TIME=%d\r\n",*(volatile unsigned int *)(0xF0040108));
	_system_power_down();
	//printf("======> [Time] clk_powerdown E TIME=%d\r\n",*(volatile unsigned int *)(0xF0040108));
	
}
#endif	// PD_USE_GPIO
