#include <string.h>

#include "vendor_isp.h"

//============================================================================
// global
//============================================================================
static int get_choose_int(void)
{
	CHAR buf[256];
	int val = 0, error;

	do {
		error = scanf("%d", &val);
		if (error != 1) {
			printf("Invalid option. Try again.\n");
			clearerr(stdin);
			fgets(buf, sizeof(buf), stdin);
			printf(">> ");
		}
	} while(error != 1);

	return val;
}

int main(int argc, char *argv[])
{
	INT32 option;
	UINT32 trig = 1;
	UINT32 version = 0;
	HD_RESULT result = HD_OK;
	AFT_CFG_INFO cfg_info = {0};
	AFT_DTSI_INFO dtsi_info = {0};
	AFT_OPERATION operation = {0};
	AFT_ENABLE enable = {0};
	AFT_ALG_METHOD alg_method = {0};
	AFT_SHOT_MODE shot_mode = {0};
	AFT_SEARCH_DIR search_dir = {0};
	AFT_SKIP_FRAME skip_frame = {0};
	AFT_THRES thres = {0};
	AFT_STEP_SIZE step_size = {0};
	AFT_MAX_COUNT max_count = {0};
	AFT_WIN_WEIGHT win_weight = {0};
	AFT_RETRIGGER retrigger = {0};
	AFT_ALG_CMD alg_cmd = {0};
	AFT_VA_OPT va_opt = {0};
	AFT_BACKLASH_COMP backlash_comp = {0};

	// open MCU device
	if (vendor_isp_init() == HD_ERR_NG) {
		return -1;
	}

	while (trig) {
		printf("----------------------------------------\n");
		printf("  1. Get version \n");
		printf("  2. Reload config file \n");
		printf("  3. Reload dtsi file \n");
		printf(" 10. Get operation \n");
		printf(" 11. Get alg command \n");
		printf(" 12. Get va option \n");
		printf(" 31. Get enable \n");
		printf(" 32. Get alg method \n");
		printf(" 33. Get shot mode \n");
		printf(" 34. Get search dir \n");
		printf(" 35. Get skip frame \n");
		printf(" 36. Get thres \n");
		printf(" 37. Get step size \n");
		printf(" 38. Get max count \n");
		printf(" 39. Get win weight \n");
		printf(" 41. Get backlash compensation \n");
		printf(" 60. Set operation \n");
		printf(" 61. Set alg command \n");
		printf(" 62. Set va option \n");
		printf(" 63. Set shot mode \n");
		printf(" 64. Set enable \n");
		printf(" 65. Set search dir \n");
		printf(" 66. Set thres \n");
		printf(" 67. Set step size \n");
		printf(" 68. Set max count \n");
		printf(" 69. Set win weight \n");
		printf(" 81. Set backlash compensation \n");
		printf(" 82. Set alg method \n");
		printf(" 85. Set skip frame \n");
		printf(" 91. Set retrigger \n");
		printf(" 0. Quit\n");
		printf("----------------------------------------\n");

		printf(">> ");
		option = get_choose_int();

		switch (option) {
		case 1:
			result = vendor_isp_get_af(AFT_ITEM_VERSION, &version);
			if (result == HD_OK) {
				printf("AF version = 0x%X \n", version);
			} else {
				printf("Get AFT_ITEM_VERSION fail! \n");
			}
			break;

		case 2:
			printf("Set isp id (0, 5)>> \n");
			cfg_info.id = (UINT32)get_choose_int();

			strncpy(cfg_info.path, "/mnt/app/isp/isp_imx291_0.cfg", CFG_NAME_LENGTH);

			result = vendor_isp_set_af(AFT_ITEM_RLD_CONFIG, &cfg_info);
			if (result != HD_OK) {
				printf("Set AFT_ITEM_RLD_CONFIG fail! \n");
			}
			break;

		case 3:
			printf("Set isp id (0, 5)>> \n");
			dtsi_info.id = (UINT32)get_choose_int();

			strncpy(dtsi_info.node_path, "/isp/af/imx290_af_0", DTSI_NAME_LENGTH);
			strncpy(dtsi_info.file_path, "/mnt/app/isp/isp.dtb", DTSI_NAME_LENGTH);
			dtsi_info.buf_addr = NULL;
			result = vendor_isp_set_af(AFT_ITEM_RLD_DTSI, &dtsi_info);
			if (result != HD_OK) {
				printf("Set AFT_ITEM_RLD_DTSI fail! \n");
			}
			break;

		case 10:
			printf("Set isp id (0, 5)>> \n");
			operation.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_OPERATION, &operation);
			if (result == HD_OK) {
				printf("id = %d, operation = %d \n", operation.id, operation.operation);
			} else {
				printf("Get AFT_ITEM_OPERATION fail \n");
			}
			break;

		case 11:
			printf("Set isp id (0, 5)>> \n");
			alg_cmd.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_ALG_CMD, &alg_cmd);
			if (result == HD_OK) {
				printf("id = %d, alg_cmd = %d \n", alg_cmd.id, alg_cmd.cmd);
			} else {
				printf("Get AFT_ITEM_ALG_CMD fail \n");
			}
			break;

		case 12:
			printf("Set isp id (0, 5)>> \n");
			va_opt.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_VA_OPT, &va_opt);
			if (result == HD_OK) {
				printf("id = %d, va_option = %d \n", va_opt.id, va_opt.opt);
			} else {
				printf("Get AFT_ITEM_VA_OPT fail \n");
			}
			break;

		case 31:
			printf("Set isp id (0, 5)>> \n");
			enable.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_ENABLE, &enable);
			if (result == HD_OK) {
				printf("id = %d, enable = %d \n", enable.id, enable.func_en);
			} else {
				printf("Get AFT_ITEM_ENABLE fail! \n");
			}
			break;

		case 32:
			printf("Set isp id (0, 5)>> \n");
			alg_method.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_ALG_METHOD, &alg_method);
			if (result == HD_OK) {
				printf("id = %d, alg method = %d \n", alg_method.id, alg_method.alg_method);
			} else {
				printf("Get AFT_ITEM_ALG_METHOD fail! \n");
			}
			break;

		case 33:
			printf("Set isp id (0, 5)>> \n");
			shot_mode.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_SHOT_MODE, &shot_mode);
			if (result == HD_OK) {
				printf("id = %d, shot mode = %d \n", shot_mode.id, shot_mode.shot_mode);
			} else {
				printf("Get AFT_ITEM_SHOT_MODE fail! \n");
			}
			break;

		case 34:
			printf("Set isp id (0, 5)>> \n");
			search_dir.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_SEARCH_DIR, &search_dir);
			if (result == HD_OK) {
				printf("id = %d, search_dir = %d \n", search_dir.id, search_dir.search_dir);
			} else {
				printf("Get AFT_ITEM_SEARCH_DIR fail! \n");
			}
			break;

		case 35:
			printf("Set isp id (0, 5)>> \n");
			skip_frame.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_SKIP_FRAME, &skip_frame);
			if (result == HD_OK) {
				printf("id = %d, skip frame = %d \n", skip_frame.id, skip_frame.skip_frame);
			} else {
				printf("Get AFT_ITEM_SKIP_FRAME fail! \n");
			}
			break;

		case 36:
			printf("Set isp id (0, 5)>> \n");
			thres.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_THRES, &thres);
			if (result == HD_OK) {
				printf("id = %d, thres = %d, %d, %d, %d \n", thres.id, thres.thres.thres_rough, thres.thres.thres_fine, thres.thres.thres_final, thres.thres.thres_restart);
			} else {
				printf("Get AFT_ITEM_THRES fail! \n");
			}
			break;

		case 37:
			printf("Set isp id (0, 5)>> \n");
			step_size.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_STEP_SIZE, &step_size);
			if (result == HD_OK) {
				printf("id = %d, step size = %d, %d, %d \n", step_size.id, step_size.step_size.step_rough, step_size.step_size.step_fine, step_size.step_size.step_final);
			} else {
				printf("Get AFT_ITEM_STEP_SIZE fail! \n");
			}
			break;

		case 38:
			printf("Set isp id (0, 5)>> \n");
			max_count.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_MAX_COUNT, &max_count);
			if (result == HD_OK) {
				printf("id = %d, max count = %d \n", max_count.id, max_count.max_count);
			} else {
				printf("Get AFT_ITEM_MAX_COUNT fail! \n");
			}
			break;

		case 39:
			printf("Set isp id (0, 5)>> \n");
			win_weight.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_WIN_WEIGHT, &win_weight);
			if (result == HD_OK) {
				int i,j;
				printf("id = %d, win weight(0,0) = %d \n", win_weight.id, win_weight.win_weight.wei[0][0]);
				for(i=0;i<AF_WIN_ALG_Y;i++){
					for(j=0;j<AF_WIN_ALG_X;j++){
						printf("%d,",win_weight.win_weight.wei[i][j]);
					}
					printf("\n");
				}
			} else {
				printf("Get AFT_ITEM_WIN_WEIGHT fail! \n");
			}
			break;

		case 41:
			printf("Set isp id (0, 5)>> \n");
			backlash_comp.id = (UINT32)get_choose_int();

			result = vendor_isp_get_af(AFT_ITEM_BACKLASH_COMP, &backlash_comp);
			if (result == HD_OK) {
				printf("id = %d, backlash_comp.enable = %d \n", backlash_comp.id, backlash_comp.enable);
			} else {
				printf("Get AFT_ITEM_VA_STA fail! \n");
			}
			break;

		case 60:
			printf("Set isp id (0, 5)>> \n");
			operation.id = (UINT32)get_choose_int();

			printf("Set operation (0 ~ %d)>> \n", AF_OPERATION_MAX_CNT-1);
			operation.operation = (UINT32)get_choose_int();

			result = vendor_isp_set_af(AFT_ITEM_OPERATION, &operation);
			if (result == HD_OK) {
				printf("id = %d, operation = %d \n", operation.id, operation.operation);
			} else {
				printf("Set AFT_ITEM_OPERATION fail! \n");
			}
			break;

		case 61:
			printf("Set isp id (0, 5)>> \n");
			alg_cmd.id = (UINT32)get_choose_int();

			printf("Set alg_cmd (0 ~ %d)>> \n", AFALG_CMD_QUIT);
			alg_cmd.cmd = (UINT32)get_choose_int();

			result = vendor_isp_set_af(AFT_ITEM_ALG_CMD, &alg_cmd);
			if (result == HD_OK) {
				printf("id = %d, alg command = %d \n", alg_cmd.id, alg_cmd.cmd);
			} else {
				printf("Set AFT_ITEM_ALG_CMD fail! \n");
			}
			break;

		case 62:
			printf("Set isp id (0, 5)>> \n");
			va_opt.id = (UINT32)get_choose_int();

			printf("Set VA option (0 ~ %d)>> \n", AF_VA_TYPE_MAX-1);
			va_opt.opt = (UINT32)get_choose_int();

			result = vendor_isp_set_af(AFT_ITEM_VA_OPT, &va_opt);
			if (result == HD_OK) {
				printf("id = %d, VA option = %d\n", va_opt.id, va_opt.opt);
			} else {
				printf("Set AFT_ITEM_VA_OPT fail! \n");
			}
			break;

		case 63:
			printf("Set isp id (0, 5)>> \n");
			shot_mode.id = (UINT32)get_choose_int();

			printf("Set shot mode (0, 5)>> \n");
			shot_mode.shot_mode = (UINT32)get_choose_int();

			result = vendor_isp_set_af(AFT_ITEM_SHOT_MODE, &shot_mode);
			if (result == HD_OK) {
				printf("id = %d, shot mode = %d \n", shot_mode.id, shot_mode.shot_mode);
			} else {
				printf("Set AFT_ITEM_SHOT_MODE fail! \n");
			}
			break;

		case 64:
			printf("Set isp id (0, 5)>> \n");
			enable.id = (UINT32)get_choose_int();

			printf("Set enable (0, 5)>> \n");
			enable.func_en = (UINT32)get_choose_int();
			
			result = vendor_isp_set_af(AFT_ITEM_ENABLE, &enable);
			if (result == HD_OK) {
				printf("id = %d, enable = %d \n", enable.id, enable.func_en);
			} else {
				printf("Set AFT_ITEM_ENABLE fail! \n");
			}
			break;

		case 65:
			printf("Set isp id (0, 5)>> \n");
			search_dir.id = (UINT32)get_choose_int();

			printf("Set search dir (0, 5)>> \n");
			search_dir.search_dir = (UINT32)get_choose_int();
			
			result = vendor_isp_set_af(AFT_ITEM_SEARCH_DIR, &search_dir);
			if (result == HD_OK) {
				printf("id = %d, search_dir = %d \n", search_dir.id, search_dir.search_dir);
			} else {
				printf("Set AFT_ITEM_SEARCH_DIR fail! \n");
			}
			break;
		case 66:
			{
				int opt = 0;

				printf("Set isp id (0, 5)>> \n");
				thres.id = (UINT32)get_choose_int();

				printf("Set thres:\n 0 :(20, 80, 90, 60) \n 1 :(25, 75, 92, 50)>> \n");
				opt = (UINT32)get_choose_int();

				if(opt == 0){
					thres.thres.thres_rough = 20 ;
					thres.thres.thres_fine  = 80 ;
					thres.thres.thres_final = 90 ;
					thres.thres.thres_restart = 60 ;
					}else{

					
					thres.thres.thres_rough = 25 ;
					thres.thres.thres_fine  = 75 ;
					thres.thres.thres_final = 92 ;
					thres.thres.thres_restart = 50 ;
						}
					

				
				result = vendor_isp_set_af(AFT_ITEM_THRES, &thres);
				if (result == HD_OK) {
					printf("id = %d, thres = %d, %d, %d, %d \n", thres.id, thres.thres.thres_rough, thres.thres.thres_fine, thres.thres.thres_final, thres.thres.thres_restart);
				} else {
					printf("Set AFT_ITEM_THRES fail! \n");
				}
			}
			break;
		case 67:
			{
				int opt = 0;

				printf("Set isp id (0, 5)>> \n");
				step_size.id = (UINT32)get_choose_int();

				printf("Set step:\n 0 :(8, 2, 1) \n 1 :(6, 1, 1)>> \n");
				opt = (UINT32)get_choose_int();

				if(opt == 0){
					step_size.step_size.step_rough= 8 ;
					step_size.step_size.step_fine = 2 ;
					step_size.step_size.step_final= 1 ;
				}else{
					step_size.step_size.step_rough= 6 ;
					step_size.step_size.step_fine = 1 ;
					step_size.step_size.step_final= 1 ;
				}

				result = vendor_isp_set_af(AFT_ITEM_STEP_SIZE , &step_size);
				if (result == HD_OK) {
					printf("id = %d, step size = %d, %d, %d \n", step_size.id, step_size.step_size.step_rough, step_size.step_size.step_fine, step_size.step_size.step_final);
				} else {
					printf("Set AFT_ITEM_STEP_SIZE  fail! \n");
				}
			}
			break;
		case 68:
			printf("Set isp id (0, 5)>> \n");
			max_count.id = (UINT32)get_choose_int();

			printf("Set max count >> \n");
			max_count.max_count = (UINT32)get_choose_int();
			
			result = vendor_isp_set_af(AFT_ITEM_MAX_COUNT, &max_count);
			if (result == HD_OK) {
				printf("id = %d, max count = %d \n", max_count.id, max_count.max_count);
			} else {
				printf("Set AFT_ITEM_MAX_COUNT fail! \n");
			}
			break;
		case 69:
			{
				int opt = 0;
				int i,j;

				printf("Set isp id (0, 5)>> \n");
				win_weight.id = (UINT32)get_choose_int();

				printf("Set win_weight:\n 0 :fill all 1 \n 1 :fill center 4*4 to 1>> \n");
				opt = (UINT32)get_choose_int();

				for(i=0;i<AF_WIN_ALG_Y;i++){
					for(j=0;j<AF_WIN_ALG_X;j++){
						win_weight.win_weight.wei[i][j]=0;
					}
					
				}

				if(opt == 0){
					for(i=0;i<AF_WIN_ALG_Y;i++){
						for(j=0;j<AF_WIN_ALG_X;j++){
							win_weight.win_weight.wei[i][j]=1;
						}
						
					}

				}else{
					for(i=AF_WIN_ALG_Y/2-2;i<AF_WIN_ALG_Y/2+2;i++){
						for(j=AF_WIN_ALG_X/2-2;j<AF_WIN_ALG_X/2+2;j++){
							win_weight.win_weight.wei[i][j]=1;
						}
					}
				}

				result = vendor_isp_set_af(AFT_ITEM_WIN_WEIGHT, &win_weight);
				if (result == HD_OK) {
					
					printf("id = %d, win weight(0,0) = %d \n", win_weight.id, win_weight.win_weight.wei[0][0]);
					for(i=0;i<AF_WIN_ALG_Y;i++){
						for(j=0;j<AF_WIN_ALG_X;j++){
							printf("%d,",win_weight.win_weight.wei[i][j]);
						}
						printf("\n");
					}
				} else {
					printf("Set AFT_ITEM_STEP_SIZE  fail! \n");
				}
			}
			break;
		case 81:
				printf("Set isp id (0, 5)>> \n");
				backlash_comp.id = (UINT32)get_choose_int();

				printf("Set backlash_comp enable (0, 5)>> \n");
				backlash_comp.enable = (UINT32)get_choose_int();

				result = vendor_isp_set_af(AFT_ITEM_BACKLASH_COMP, &backlash_comp);
				if (result == HD_OK) {
					printf("id = %d, backlash_comp.enable = %d \n", backlash_comp.id, backlash_comp.enable);
				} else {
					printf("Get AFT_ITEM_VA_STA fail! \n");
				}
				break;

		case 82:
			printf("Set isp id (0, 5)>> \n");
			alg_method.id = (UINT32)get_choose_int();

			printf("Set alg method (0 ~ %d)>> \n", AF_ALG_BACK_FORWARD);
			alg_method.alg_method = (UINT32)get_choose_int();

			result = vendor_isp_set_af(AFT_ITEM_ALG_METHOD, &alg_method);
			if (result == HD_OK) {
				printf("id = %d, alg method = %d \n", alg_method.id, alg_method.alg_method);
			} else {
				printf("Get AFT_ITEM_ALG_METHOD fail! \n");
			}
			break;

		case 85:
			printf("Set isp id (0, 5)>> \n");
			skip_frame.id = (UINT32)get_choose_int();

			printf("Set skip frame >> \n");
			skip_frame.skip_frame = (UINT32)get_choose_int();

			result = vendor_isp_set_af(AFT_ITEM_SKIP_FRAME, &skip_frame);
			if (result == HD_OK) {
				printf("id = %d, skip frame = %d \n", skip_frame.id, skip_frame.skip_frame);
			} else {
				printf("Get AFT_ITEM_SKIP_FRAME fail! \n");
			}
			break;

		case 91:
			printf("Set isp id (0, 5)>> \n");
			retrigger.id = (UINT32)get_choose_int();

			retrigger.retrigger = 1;
			result = vendor_isp_set_af(AFT_ITEM_RETRIGGER, &retrigger);
			if (result == HD_OK) {
				printf("id = %d retrigger \n", retrigger.id);
			} else {
				printf("Set AFT_ITEM_RETRIGGER fail! \n");
			}
			break;

		case 0:
		default:
			trig = 0;
			break;
		}
	}

	result = vendor_isp_uninit();
	if(result != HD_OK) {
		printf("vendor_isp_uninit fail=%d\n", result);
	}

	return 0;
}

