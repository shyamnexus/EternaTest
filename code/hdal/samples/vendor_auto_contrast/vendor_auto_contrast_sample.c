
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include "auto_contrast_lib.h"

//============================================================================
// PLATFORM define
//============================================================================


//============================================================================
// Macro
//============================================================================

//============================================================================
// global
//============================================================================
pthread_t auto_contrast_thread_id;
static UINT32 auto_contrast_conti_run = 1;

BOOL auto_contrast_enable = FALSE;
//BOOL auto_contrast_dbg = FALSE;

static INT32 get_choose_int(void)
{
	CHAR buf[256];
	INT val, error;

	error = scanf("%d", &val);

	if (error != 1) {
		printf("Invalid option. Try again.\n");
		clearerr(stdin);
		fgets(buf, sizeof(buf), stdin);
		val = -1;
	}

	return val;
}

static void *auto_contrast_thread(void *arg)
{
	ISPT_HISTO_DATA histo_data = {0};
	IQT_YCURVE_PARAM ycurve = {0};
	IQT_GAMMA_PARAM gamma = {0};
	AET_STATUS_INFO status = {0};
	UINT32 id = *(UINT32 *)arg;
	INT rt = 0;

	histo_data.id = id;
	gamma.id = id;
	ycurve.id = id;
	status.id = id;

	while (auto_contrast_conti_run) {
		if (auto_contrast_enable == TRUE) {
			vendor_isp_get_common(ISPT_ITEM_HISTO_DATA, &histo_data);
			vendor_isp_get_iq(IQT_ITEM_GAMMA_PARAM, &gamma);
			vendor_isp_get_ae(AET_ITEM_STATUS, &status);

			rt = auto_contrast_cal(&histo_data, &ycurve, &gamma, &status);
			if (rt != HD_OK) {
				printf("auto_contrast_cal fail=%d\n", rt);
			}
			ycurve.ycurve.enable = TRUE;
			vendor_isp_set_iq(IQT_ITEM_YCURVE_PARAM, &ycurve);
		} else {
			vendor_isp_get_iq(IQT_ITEM_YCURVE_PARAM, &ycurve);
			ycurve.ycurve.enable = FALSE;
			vendor_isp_set_iq(IQT_ITEM_YCURVE_PARAM, &ycurve);
			//printf("disable Ycurve id = %d \n", id);
		}
		sleep(1);
	}

	return 0;
}

//============================================================================
// DEMO board platform control
//============================================================================

int main(int argc, char *argv[])
{
	UINT32 id;                    // [id]
	UINT32 strength;
	UINT32 option, infinite_loop = 1;
	BOOL dbg_enable = 0;

	// open MCU device
	if (vendor_isp_init() == HD_ERR_NG) {
		return -1;
	}
	auto_contrast_set_dbg_out(dbg_enable);
	do {
		printf("Set isp id (0, 1)>> \n");
		id = (UINT32)get_choose_int();
		printf("Set auto contrast enable (0, 1)>> \n");
		auto_contrast_enable = (BOOL)get_choose_int();
		printf("Set auto contrast strength (0, 16)>> \n");
		strength = (UINT32)get_choose_int();
		auto_contrast_set_str(strength);
	} while (0);

	auto_contrast_conti_run = 1;
	if (pthread_create(&auto_contrast_thread_id, NULL, auto_contrast_thread, &id) < 0) {
		printf("create auto_contrast thread failed");
	}

	while (infinite_loop) {
		printf("----------------------------------------\n");
		printf(" 1. Enter auto contrast strength\n");
		printf(" 2. Enter ycurve Enable\n");
		printf(" 3. Enter debug Enable\n");
		printf(" 4. Quit\n");
		printf("----------------------------------------\n");

		do {
			printf(">> ");
			option = get_choose_int();
		} while (0);

		switch (option) {
		case 1:
			printf(" Please Enter strength (0~16)\n");
			printf(">> ");
			strength = (UINT32)get_choose_int();
			auto_contrast_set_str(strength);
			break;

		case 2:
			printf(" Please Enter auto contrast enable(0/1)\n");
			printf(">> ");
			auto_contrast_enable = (BOOL)get_choose_int();
			break;

		case 3:
			printf(" Please Enter debug message enable(0/1)\n");
			printf(">> ");
			dbg_enable = (BOOL)get_choose_int();
			auto_contrast_set_dbg_out(dbg_enable);
			break;

		case 4:
		default:
			auto_contrast_conti_run = 0;
			infinite_loop = 0;
			break;
		}
	}

	// destroy encode thread
	pthread_join(auto_contrast_thread_id, NULL);

	vendor_isp_uninit();

	return 0;
}

