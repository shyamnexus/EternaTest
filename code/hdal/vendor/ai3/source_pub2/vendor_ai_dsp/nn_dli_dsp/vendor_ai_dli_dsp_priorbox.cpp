/**
    @brief Source file of DLI dsp priorbox layer.

    @file vendor_ai_dli_dsp_priorbox.cpp

    @ingroup dli_dsp_priorbox

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_priorbox.hpp"
// #include "vendor_ai_dli_dsp_utils.hpp"
// extern "C" {
// 	#include "hd_common.h"
// }
/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Extern Function Prototype                                                   */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Functions                                                             */
/*-----------------------------------------------------------------------------*/

// UINT32 calcul_numpriors(NVTNN_PRIORBOX_PRMS *prms_addr) {
// 	NVTNN_PRIORBOX_PRMS *prms = (NVTNN_PRIORBOX_PRMS *) prms_addr;
// 	UINT32 num_priors = 0, i, j, cnt = 0;
// 	BOOL already_exist;
// 	FLOAT tmp_ratios[10], ar;
// 	for (i = 0; i < prms->min_size_num; i++) {
// //		input_data.min_sizes[i] = prms->min_sizes[i];
// 		num_priors++;
// 	}
// 	for (i = 0; i < prms->aspect_ratio_num; i++) {
// 		ar = prms->aspect_ratios[i];
// 		if (ar != 0) {
// 			already_exist = FALSE;
// 			for (j = 0; j < cnt; j++) {
// 				if (((ar - tmp_ratios[j]) < 0.000001) && ((ar - tmp_ratios[j]) > -0.000001)) {
// 					already_exist = TRUE;
// 					break;
// 				}
// 			}
// 			if (!already_exist) {
// 				tmp_ratios[cnt++] = ar;
// 				num_priors++;
// 				if (prms->flip) {
// 					tmp_ratios[cnt++] = (FLOAT)(1. / ar);
// 					num_priors++;
// 				}
// 			}
// 		}
// 	}
// 	if (prms->max_size_num > 0) {
// 		for (i = 0; i < prms->max_size_num; i++) {
// //		    input_data.max_sizes[i] = prms->max_sizes[i];
// 		    // if (input_data.max_sizes[i] > input_data.min_sizes[i]) {
// 			num_priors++;
// 		    // }
// 	    }
// 	}
// 	return num_priors;
// }

// VOID dli_nvtnn_dsp_flush_cache_priorbox(NVTNN_PRIORBOX_PRMS *p_nvtnn_prms) {
// 	UINT32 num_priors = calcul_numpriors(p_nvtnn_prms);
// 	UINT32 datasize = p_nvtnn_prms->in_width * p_nvtnn_prms->in_height * num_priors * 4 * sizeof(FLOAT) * 2;

// 	hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->out_addr, datasize);
// 	hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->tmp_addr, datasize);
// }

static VOID dli_to_nvtnn_priorbox(NN_DLI_PRIORBOX_PARM *p_dli_parm, NVTNN_PRIORBOX_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *output_info	   = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;

	p_nvtnn_prms->out_addr              = output_info->data_va;
	p_nvtnn_prms->out_fmt               = output_quant_info->fmt;
	p_nvtnn_prms->osf                   = output_quant_info->sf;
	p_nvtnn_prms->tmp_addr              = p_dli_parm->temp_buf_va;

	p_nvtnn_prms->in_width              = p_dli_parm->in_width;
	p_nvtnn_prms->in_height             = p_dli_parm->in_height;
	p_nvtnn_prms->img_width             = p_dli_parm->img_width;
	p_nvtnn_prms->img_height            = p_dli_parm->img_height;
	// p_nvtnn_prms->min_sizes             = p_dli_parm->min_sizes;
	// p_nvtnn_prms->max_sizes             = p_dli_parm->max_sizes;
	p_nvtnn_prms->min_size_num          = p_dli_parm->min_size_num;
	p_nvtnn_prms->max_size_num          = p_dli_parm->max_size_num;
	// p_nvtnn_prms->aspect_ratios         = p_dli_parm->aspect_ratios;
	p_nvtnn_prms->aspect_ratio_num      = p_dli_parm->aspect_ratio_num;
	p_nvtnn_prms->flip              	= p_dli_parm->flip;
	p_nvtnn_prms->clip              	= p_dli_parm->clip;
	// p_nvtnn_prms->variances             = p_dli_parm->variances;
	p_nvtnn_prms->variance_num          = p_dli_parm->variance_num;
	p_nvtnn_prms->offset                = p_dli_parm->offset;

	for (int i = 0; i < NN_PRIORBOX_SIZE_NUM; i++) {
		p_nvtnn_prms->min_sizes[i] = p_dli_parm->min_sizes[i];
		p_nvtnn_prms->max_sizes[i] = p_dli_parm->max_sizes[i];
	}
	for(int i = 0; i < NN_PRIORBOX_VAR_NUM; i++) {
		p_nvtnn_prms->variances[i] = p_dli_parm->variances[i];
	}
	for(int i = 0; i < NN_PRIORBOX_ASPECT_RATIO_NUM; i++) {
		p_nvtnn_prms->aspect_ratios[i] = p_dli_parm->aspect_ratios[i];
	}

	// dli_nvtnn_dsp_flush_cache_priorbox(p_nvtnn_prms);

	#if 0
	printf("output_info->data_va: %#x  p_nvtnn_prms->out_addr: %#x\n", output_info->data_va, p_nvtnn_prms->out_addr);
	printf("p_dli_parm->temp_buf_va: %#x\n", p_dli_parm->temp_buf_va);
	printf("p_nvtnn_prms->in_width: %d\n", p_nvtnn_prms->in_width);
	printf("p_nvtnn_prms->in_height: %d\n", p_nvtnn_prms->in_height);
	printf("p_nvtnn_prms->img_width: %d\n", p_nvtnn_prms->img_width);
	printf("p_nvtnn_prms->img_height: %d\n", p_nvtnn_prms->img_height);
	printf("p_nvtnn_prms->min_size_num: %d\n", p_nvtnn_prms->min_size_num);
	printf("p_nvtnn_prms->max_size_num: %d\n", p_nvtnn_prms->max_size_num);
	printf("p_nvtnn_prms->aspect_ratio_num: %d\n", p_nvtnn_prms->aspect_ratio_num);
	printf("p_nvtnn_prms->flip: %d\n", p_nvtnn_prms->flip);
	printf("p_nvtnn_prms->clip: %d\n", p_nvtnn_prms->clip);
	printf("p_nvtnn_prms->variance_num: %d\n", p_nvtnn_prms->variance_num);
	printf("p_nvtnn_prms->offset: %f\n", p_nvtnn_prms->offset);
	#endif
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_priorbox_init(NN_DLI_PRIORBOX_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_priorbox_proc(NN_DLI_PRIORBOX_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_PRIORBOX_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_priorbox(p_parm, &nvtnn_prms);
	HD_RESULT ret = nvtnndsp_op_proc(NN_DLI_PRIORBOX, &nvtnn_prms, dsp_core);

	// char out_file_dir1[360];
	// sprintf(out_file_dir1, "output.txt");
	// FILE *fp1 = fopen(out_file_dir1, "w");
	// if (fp1) {
	// 	for (int n = 0; n < 256*3*3; n++) {
	// 		fprintf(fp1, "%.9f\n", ((UINT8 *)(nvtnn_prms.out_addr))[n]/(float)(1<<6));
	// 	}
	// 	fclose(fp1);
	// }

	// Execute dsp
	return ret;
}

HD_RESULT dli_dsp_priorbox_uninit(NN_DLI_PRIORBOX_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}