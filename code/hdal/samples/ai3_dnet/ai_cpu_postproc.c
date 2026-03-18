/**
	@brief Source file of postproc for detection output network.

	@file ai_cpu_postproc.c

	@ingroup ai_net_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "ai_cpu_postproc.h"


// set model parameter here
UINT32 model_input_dim[4] = {
	1, 3, 300, 300	// batch, channel, height, width
};

static const CAFFE_PARAM_DETOUT g_detout_param = {
	0, NN_PRIORBOX_CODE_CENTER, 0.25, 100, {0.45, 100}, 2, TRUE
};

static const CAFFE_PARAM_PRIORBOX g_priorbox_param[NUM_PRIORBOX] = {
	{{1,  512, 19, 19}, {2.0, 0.0}, TRUE, FALSE, 105,  60, 0.5, {0.1, 0.1, 0.2, 0.2}},
	{{1, 1024, 10, 10}, {2.0, 3.0}, TRUE, FALSE, 150, 105, 0.5, {0.1, 0.1, 0.2, 0.2}},
	{{1,  512,  5,  5}, {2.0, 3.0}, TRUE, FALSE, 195, 150, 0.5, {0.1, 0.1, 0.2, 0.2}},
	{{1,  256,  3,  3}, {2.0, 3.0}, TRUE, FALSE, 240, 195, 0.5, {0.1, 0.1, 0.2, 0.2}},
	{{1,  256,  2,  2}, {2.0, 3.0}, TRUE, FALSE, 285, 240, 0.5, {0.1, 0.1, 0.2, 0.2}},
	{{1,  128,  1,  1}, {2.0, 3.0}, TRUE, FALSE, 300, 285, 0.5, {0.1, 0.1, 0.2, 0.2}},
} ;

CHAR g_outlayer_name[OUT_NUM_LAYER][256] = {
	"Concat_mbox_loc_Y",
	"Flatten_mbox_conf_flatten_Y",
};

/*-----------------------------------------------------------------------------*/
/* Local Functions                                                             */
/*-----------------------------------------------------------------------------*/
#if SETTING_CHK
static UINT32 ai_dnet_priorbox_checkparm(NN_PRIORBOX_PARM *p_parm)
{

	UINT32 max_size_num = p_parm->max_size_num;
	UINT32 min_size_num = p_parm->min_size_num;

	UINT32 aspect_ratios_num = p_parm->aspect_ratio_num;
	UINT32 variance_num = p_parm->variance_num;
	UINT32 img_width = p_parm->img_width;
	UINT32 img_height = p_parm->img_height;
	UINT32 in_width = p_parm->in_width;
	UINT32 in_height = p_parm->in_height;

	if ((in_height <= 0) || (in_width <= 0) || (img_height <= 0) || (img_width <= 0)
		|| (min_size_num <= 0) || (aspect_ratios_num <= 0) || (variance_num <= 0)) {
		printf("shape should be positive, but input is %dx%dx%dx%d(img_width x img_height x in_width x in_height),%dx%dx%dx%d(max_size_num x min_size_num x variance_num x aspect_ratios_num)\r\n",
				(INT32)img_width, (INT32)img_height, (INT32)in_width, (INT32)in_height, (INT32)max_size_num, (INT32)min_size_num, (INT32)variance_num, (INT32)aspect_ratios_num);
		return 2;
	}
	return 0;
}
#endif

HD_RESULT ai_dnet_priorbox_c_func(INPUTDATA_PRIORBOX *input, OUTPUTDATA_PRIORBOX *output)
{
	INT32 layer_height = (INT32)input->in_height;
	INT32 layer_width  = (INT32)input->in_width;
	INT32 img_width    = (INT32)input->img_width;
	INT32 img_height   = (INT32)input->img_height;

	INT32 min_size_num = (INT32)input->min_size_num;
	INT32 max_size_num = (INT32)input->max_size_num;

	INT32 aspect_ratio_num = (INT32)input->aspect_ratio_num;
	FLOAT step_h = (FLOAT)(img_height) / layer_height;
	FLOAT step_w = (FLOAT)(img_width) / layer_width;
	FLOAT s_img_width = 1.0 / img_width;
	FLOAT s_img_height = 1.0 / img_height;
	FLOAT offset = input->off_set;
	INT32 h, w, s, r, d, i, j, idx = 0, count = 0;
	FLOAT center_x, center_y, box_width, box_height;
	FLOAT min_size_, max_size_, ar, tmp;
	FLOAT *top_data = output->out_point;
	FLOAT *out_var  = output->out_var;
	FLOAT var;
	INT32 dim = layer_height * layer_width * input->num_prior * 4;

#if PRIORBOX_PRINT_PARM
	printf("step_w = %.9f, step_h = %.9f\r\n", step_w, step_h);
#endif
	for (h = 0; h < layer_height; h++) {
		for (w = 0; w < layer_width; w++) {
			center_x = (w + offset) * step_w;
			center_y = (h + offset) * step_h;
			//printf("center_x = %lf, center_y = %lf\r\n", center_x, center_y);

			for (s = 0; s < min_size_num; s++) {
				min_size_ = input->min_sizes[s];
				box_width = box_height = min_size_;
				tmp = center_x - box_width * 0.5;
				top_data[idx++] = tmp * s_img_width;
				tmp = center_y - box_height * 0.5;
				top_data[idx++] = tmp * s_img_height;
				tmp = center_x + box_width * 0.5;
				top_data[idx++] = tmp * s_img_width;
				tmp = center_y + box_height * 0.5;
				top_data[idx++] = tmp * s_img_height;

				if(max_size_num > 0){

					max_size_ = input->max_sizes[s];
					var = sqrt(min_size_ * max_size_);
					box_width = box_height = var;
					tmp = center_x - box_width * 0.5;
					top_data[idx++] = tmp * s_img_width;
					tmp = center_y - box_height * 0.5;
					top_data[idx++] = tmp * s_img_height;
					tmp = center_x + box_width * 0.5;
					top_data[idx++] = tmp * s_img_width;
					tmp = center_y + box_height * 0.5;
					top_data[idx++] = tmp * s_img_height;
				}
				//the rest of priors
				for (r = 0; r < aspect_ratio_num; r++) {
					ar = input->aspect_ratios[r];
					if (((ar - 1.0) > -0.000001) && ((ar - 1.0) < 0.000001)) {
						continue;
					}
					var = sqrt(ar);
					box_width = min_size_ * var;
					box_height = min_size_ / var;
					tmp = center_x - box_width * 0.5;
					top_data[idx++] = tmp * s_img_width;
					tmp = center_y - box_height * 0.5;
					top_data[idx++] = tmp * s_img_height;
					tmp = center_x + box_width * 0.5;
					top_data[idx++] = tmp * s_img_width;
					tmp = center_y + box_height * 0.5;
					top_data[idx++] = tmp * s_img_height;
				}
			}
		}
	}
	// clip the prior's coordidate such that it is within [0, 1]
	if (input->clip) {
		for (d = 0; d < dim; d++) {
			top_data[d] = MIN(MAX(top_data[d], 0.), 1.);
		}
	}
	//set the variance
	for (h = 0; h < layer_height; h++) {
		for (w = 0; w < layer_width; w++) {
			for (i = 0; i < input->num_prior; i++) {
				for (j = 0; j < 4; j++) {
					out_var[count] = input->variances[j];
					count++;
				}
			}
		}
	}
	return HD_OK;
}

HD_RESULT ai_dnet_priorbox_neon_func(INPUTDATA_PRIORBOX *input, OUTPUTDATA_PRIORBOX *output)
{
#define VAR_NUM  	16
#define MAX_SIZE 	16
#define NUM_LANE 	4
#define NUM_VEC_B  	1
#define BATCH    	(NUM_LANE * NUM_VEC_B)
#define STEP     	(BATCH * 4)
#define NUM_LANE_1V 4
#define NUM_VEC_1V  4
#define BATCH_1V    (NUM_LANE_1V * NUM_VEC_1V)

	INT32 h, w, s, r, d, i, j, size, num, idx1 = 0, count = 0;
	INT32 layer_height = (INT32)input->in_height;
	INT32 layer_width  = (INT32)input->in_width;
	INT32 img_width    = (INT32)input->img_width;
	INT32 img_height   = (INT32)input->img_height;
	INT32 min_size_num = (INT32)input->min_size_num;
	INT32 max_size_num = (INT32)input->max_size_num;
	INT32 aspect_ratio_num = (INT32)input->aspect_ratio_num;
	INT32 dim = layer_height * layer_width * input->num_prior * 4;
	FLOAT step_h = (FLOAT)(img_height) / layer_height;
	FLOAT step_w = (FLOAT)(img_width) / layer_width;
	FLOAT tmp;
	float32_t offset = input->off_set;
	float32_t center_x, center_y, min_size_, max_size_, s_img_width, s_img_height;
	float32_t ar, var, value = 0.5;
	float32_t *p_src_width, *p_src_height, *p_dst, *out_var;
	float32_t box_width[MAX_SIZE] = {0}, box_height[MAX_SIZE] = {0}, variance[VAR_NUM] = {0};
	float32x4_t vdataset_w, vdataset_h, cmp_min, cmp_max;
	float32x4_t v_center_x, v_center_y, v_xmin, v_xmax, v_ymin, v_ymax, v_max;
	float32x4x4_t vdataset, vdata_final;
	p_dst = output->out_point;
	out_var  = output->out_var;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			variance[count++] = input->variances[j];
		}
	}

	//box_width and box_height
	for (s = 0; s < min_size_num; s++) {
		// first prior: aspect_ratio = 1, size = min_size
		min_size_ = input->min_sizes[s];
		box_width[idx1] = min_size_;
		box_height[idx1] = min_size_;
		idx1++;
		// second prior: aspect_ratio = 1, size = sqrt(min_size * max_size)

		if (max_size_num > 0){

		  max_size_ = input->max_sizes[s];
		  var = sqrt(min_size_ * max_size_);
		  box_width[idx1] = var;
		  box_height[idx1] = var;
		  idx1++;
		}
		//the rest of priors
		for (r = 0; r < aspect_ratio_num; r++) {
			ar = input->aspect_ratios[r];
			if (((ar - 1.0) > -0.000001) && ((ar - 1.0) < 0.000001)) {
				continue;
			}
			var = sqrt(ar);
			box_width[idx1] = min_size_ * var;
			box_height[idx1] = min_size_ / var;
			idx1++;
		}
	}

#if PRIORBOX_PRINT_PARM
	printf("step_w = %.9f, step_h = %.9f\r\n", step_w, step_h);
	printf("idx1 = %d\r\n", idx1);
	for (i = 0; i < idx1; i++) {
		printf("box_width[%d] = %.9f\r\n", i, box_width[i]);
		printf("box_height[%d] = %.9f\r\n", i, box_height[i]);
	}
#endif
	
	s_img_width = 1.0 / img_width;
	s_img_height = 1.0 / img_height;
	
#if PRIORBOX_PRINT_PARM
	printf("s_img_width = %.9f, s_img_height = %.9f\r\n", s_img_width, s_img_height);
#endif
	for (h = 0; h < layer_height; h++) {
		center_y = (h + offset) * step_h;
		v_center_y = vmovq_n_f32(center_y);
		for (w = 0; w < layer_width; w++) {
			center_x = (w + offset) * step_w;
			v_center_x = vmovq_n_f32(center_x);
			//printf("center_x = %lf, center_y = %lf\r\n", center_x, center_y);
	
			p_src_width = box_width;
			p_src_height = box_height;
			num = ALIGN_FLOOR(idx1, BATCH);
			for (i = 0; i < num; i += BATCH) {
				vdataset_w = vld1q_f32(p_src_width);
				v_xmin = vmlsq_n_f32(v_center_x, vdataset_w, value);
				v_xmax = vmlaq_n_f32(v_center_x, vdataset_w, value);
				vdata_final.val[0] = vmulq_n_f32(v_xmin, s_img_width);//xmin
				vdata_final.val[2] = vmulq_n_f32(v_xmax, s_img_width);//xmax
	
				vdataset_h = vld1q_f32(p_src_height);
				v_ymin = vmlsq_n_f32(v_center_y, vdataset_h, value);
				v_ymax = vmlaq_n_f32(v_center_y, vdataset_h, value);
				vdata_final.val[1] = vmulq_n_f32(v_ymin, s_img_height);//ymin
				vdata_final.val[3] = vmulq_n_f32(v_ymax, s_img_height);//ymax
	
				vst4q_f32(p_dst, vdata_final);
				p_dst += STEP;
				p_src_width += BATCH;
				p_src_height += BATCH;
			}
	
			size = idx1 - num;
			for (i = 0; i < size; i++) {
				tmp = center_x - p_src_width[i] * 0.5;
				p_dst[0] = tmp * s_img_width;
				tmp = center_y - p_src_height[i] * 0.5;
				p_dst[1] = tmp * s_img_height;
				tmp = center_x + p_src_width[i] * 0.5;
				p_dst[2] = tmp * s_img_width;
				tmp = center_y + p_src_height[i] * 0.5;
				p_dst[3] = tmp * s_img_height;
				p_dst += 4;
			}
		}
	}
	// clip the prior's coordidate such that it is within [0, 1]
	if (input->clip) {
		cmp_min = vmovq_n_f32(1.0);
		cmp_max = vmovq_n_f32(0.0);
		p_dst = output->out_point;
		num = ALIGN_FLOOR(dim, BATCH_1V);
		for (d = 0; d < num; d += BATCH_1V) {
			//top_data[d] = MIN(MAX(top_data[d], 0.), 1.);
			vdataset = vld4q_f32(p_dst);
			v_max = vmaxq_f32(vdataset.val[0], cmp_max);
			vdata_final.val[0] = vminq_f32(v_max, cmp_min);
			v_max = vmaxq_f32(vdataset.val[1], cmp_max);
			vdata_final.val[1] = vminq_f32(v_max, cmp_min);
			v_max = vmaxq_f32(vdataset.val[2], cmp_max);
			vdata_final.val[2] = vminq_f32(v_max, cmp_min);
			v_max = vmaxq_f32(vdataset.val[3], cmp_max);
			vdata_final.val[3] = vminq_f32(v_max, cmp_min);
			vst4q_f32(p_dst, vdata_final);
			p_dst += BATCH_1V;
		}
		
		size = dim - num;
		for (d = 0; d < size; d++) {
			p_dst[d] = MIN(MAX(p_dst[d], 0.), 1.);
		}
	}
	//set the variance
	num = ALIGN_FLOOR(dim, BATCH_1V);
	for (d = 0; d < num; d += BATCH_1V) {
		vdataset = vld4q_f32(variance);
		vst4q_f32(out_var, vdataset);
		out_var += BATCH_1V;
	}
	size = dim - num;
	for (d = 0; d < size; d++) {
		out_var[d] = variance[d];
	}
		
	return HD_OK;
}

static HD_RESULT ai_dnet_priorbox_process(NN_PRIORBOX_PARM *p_parm)
{

	HD_RESULT ret = HD_OK;

	INPUTDATA_PRIORBOX input_data = {0};
	OUTPUTDATA_PRIORBOX output_data = {0};
	UINT32 i;
	INT16 j, num_priors = 0;
	BOOL already_exist;
	FLOAT ar;
#if SETTING_CHK
	if (ai_dnet_priorbox_checkparm(p_parm)) {
		return HD_ERR_PARAM;
	}
#endif
	input_data.flip             = p_parm->flip;
	input_data.clip             = p_parm->clip;
	input_data.img_height       = p_parm->img_height;
	input_data.img_width        = p_parm->img_width;
	input_data.in_height        = p_parm->in_height;
	input_data.in_width         = p_parm->in_width;
	input_data.off_set          = p_parm->offset;
	input_data.variance_num     = p_parm->variance_num;

	input_data.max_size_num     = p_parm->max_size_num;
	input_data.min_size_num     = p_parm->min_size_num;

	for (i = 0; i < input_data.min_size_num; i++){
		input_data.min_sizes[i] = p_parm->min_sizes[i];
	}

	for (i = 0; i < NN_INPUTDATA_ASPECT_RATIO_NUM; i++) {
		input_data.aspect_ratios[i] = 0.0;
	}
	input_data.aspect_ratios[0] = 1.0;
	num_priors++;

	for (i = 0; i < p_parm->aspect_ratio_num; i++) {
		ar = p_parm->aspect_ratios[i];
		if (ar != 0) {
			already_exist = FALSE;
			for (j = 0; j < num_priors; j++) {
				if (((ar - input_data.aspect_ratios[j]) < 0.000001) && ((ar - input_data.aspect_ratios[j]) > -0.000001)) {
					already_exist = TRUE;
					break;
				}
			}
			if (!already_exist) {
				input_data.aspect_ratios[num_priors] = ar;
				num_priors++;
				if (p_parm->flip) {
					input_data.aspect_ratios[num_priors] = 1. / ar;
					num_priors++;
				}
			}
		}
	}
	input_data.aspect_ratio_num = num_priors;

	if (p_parm->max_size_num > 0) {
		for (i = 0; i < p_parm->max_size_num; i++) {
			input_data.max_sizes[i] = p_parm->max_sizes[i];
			if (input_data.max_sizes[i] > input_data.min_sizes[i]) {
				num_priors++;
			}
		}
	}

	input_data.num_prior = num_priors;
	p_parm->out_channel_ofs = 4 * num_priors * p_parm->in_width * p_parm->in_height * sizeof(FLOAT);
	p_parm->out_addr = (UINTPTR)malloc(2 * p_parm->out_channel_ofs);

	if (p_parm->variance_num > 1) {
		if (p_parm->variance_num >= 4) {
			for (i = 0; i < p_parm->variance_num; i++) {
				if (p_parm->variances[i] > 0) {
					input_data.variances[i] = p_parm->variances[i];
				}
			}
		}
	} else if (p_parm->variance_num == 1) {
		if (p_parm->variances[0] > 0) {
			input_data.variances[0] = p_parm->variances[0];
		}
	} else {
		input_data.variances[0] = 0.1;
	}

	output_data.out_point = (FLOAT *)p_parm->out_addr;
	output_data.out_var   = (FLOAT *)(p_parm->out_addr + p_parm->out_channel_ofs);

#if PRIORBOX_NEON
	ret = ai_dnet_priorbox_neon_func(&input_data, &output_data);
#if PRIORBOX_PRINT_PARM
	printf("nvtnn_priorbox_neon_func:sta = %d!\r\n", ret);
#endif
#else
	ret = ai_dnet_priorbox_c_func(&input_data, &output_data);
#if PRIORBOX_PRINT_PARM
	printf("nvtnn_priorbox_c_func:sta = %d!\r\n", ret);
#endif
#endif

	return ret;
}


static inline FLOAT fast_exp(FLOAT x)
{
	union {
		UINT32 i;
		FLOAT f;
	} v;
	v.i = (1 << 23) * (1.4426950409f * x + 126.93490512f);

	return v.f;
}

static VOID decode_bboxes(NORM_BOX *prior_bboxes, FLOAT *prior_variances, INT32 code_type, INT32 variance_encoded_in_target,
				  INT32 clip_bbox, NORM_BOX *bboxes, NORM_BOX *decode_bboxes, INT32 num_priors_, int num_loc_classes)
{
	for (INT32 c = 0; c < num_loc_classes; ++c) {
		for (INT32 i = 0; i < num_priors_; ++i) {
			NORM_BOX *decode_bbox = &decode_bboxes[c * num_priors_ + i];
			NORM_BOX *prior_bbox = &prior_bboxes[i];
			NORM_BOX *bbox = &bboxes[c * num_priors_ + i];
			FLOAT *prior_variance = &prior_variances[i * 4];

			if (code_type == NN_PRIORBOX_CODE_CENTER) {
				FLOAT prior_width = prior_bbox->xmax - prior_bbox->xmin;
				FLOAT prior_height = prior_bbox->ymax - prior_bbox->ymin;

				FLOAT prior_center_x = (prior_bbox->xmin + prior_bbox->xmax) / 2.;
				FLOAT prior_center_y = (prior_bbox->ymin + prior_bbox->ymax) / 2.;

				FLOAT decode_bbox_center_x, decode_bbox_center_y;
				FLOAT decode_bbox_width, decode_bbox_height;
				if (variance_encoded_in_target) {
					decode_bbox_center_x = bbox->xmin * prior_width + prior_center_x;
					decode_bbox_center_y = bbox->ymin * prior_height + prior_center_y;
					decode_bbox_width = fast_exp(bbox->xmax) * prior_width;
					decode_bbox_height = fast_exp(bbox->ymax) * prior_height;
				} else {
					decode_bbox_center_x = prior_variance[0] * bbox->xmin * prior_width + prior_center_x;
					decode_bbox_center_y = prior_variance[1] * bbox->ymin * prior_height + prior_center_y;

					FLOAT delta_w_f = fast_exp(prior_variance[2] * bbox->xmax);
					FLOAT delta_h_f = fast_exp(prior_variance[3] * bbox->ymax);
					decode_bbox_width   = delta_w_f * prior_width;
					decode_bbox_height  = delta_h_f * prior_height;
				}

				decode_bbox->xmin = (decode_bbox_center_x - decode_bbox_width / 2.);
				decode_bbox->ymin = (decode_bbox_center_y - decode_bbox_height / 2.);
				decode_bbox->xmax = (decode_bbox_center_x + decode_bbox_width / 2.);
				decode_bbox->ymax = (decode_bbox_center_y + decode_bbox_height / 2.);
			} else if (code_type == NN_PRIORBOX_CODE_CORNER) {
				if (variance_encoded_in_target) {
					decode_bbox->xmin = prior_bbox->xmin + bbox->xmin;
					decode_bbox->ymin = prior_bbox->ymin + bbox->ymin;
					decode_bbox->xmax = prior_bbox->xmax + bbox->xmax;
					decode_bbox->ymax = prior_bbox->ymax + bbox->ymax;
				} else {
					decode_bbox->xmin = prior_bbox->xmin + prior_variance[0] * bbox->xmin;
					decode_bbox->ymin = prior_bbox->ymin + prior_variance[1] * bbox->ymin;
					decode_bbox->xmax = prior_bbox->xmax + prior_variance[2] * bbox->xmax;
					decode_bbox->ymax = prior_bbox->ymax + prior_variance[3] * bbox->ymax;
				}
			} else if (code_type == NN_PRIORBOX_CODE_CORNER_SIZE) {
				FLOAT prior_width = prior_bbox->xmax - prior_bbox->xmin;
				FLOAT prior_height = prior_bbox->ymax - prior_bbox->ymin;
				if (variance_encoded_in_target) {
					decode_bbox->xmin = prior_bbox->xmin + bbox->xmin * prior_width;
					decode_bbox->ymin = prior_bbox->ymin + bbox->ymin * prior_height;
					decode_bbox->xmax = prior_bbox->xmax + bbox->xmax * prior_width;
					decode_bbox->ymax = prior_bbox->ymax + bbox->ymax * prior_height;
				} else {
					decode_bbox->xmin = prior_bbox->xmin + prior_variance[0] * bbox->xmin * prior_width;
					decode_bbox->ymin = prior_bbox->ymin + prior_variance[1] * bbox->ymin * prior_height;
					decode_bbox->xmax = prior_bbox->xmax + prior_variance[2] * bbox->xmax * prior_width;
					decode_bbox->ymax = prior_bbox->ymax + prior_variance[3] * bbox->ymax * prior_height;
				}
			} else {
				printf("Unknown code type.");
			}

			if (clip_bbox == 1) {
				decode_bbox->xmin = MAX(MIN(decode_bbox->xmin, 1), 0);
				decode_bbox->ymin = MAX(MIN(decode_bbox->ymin, 1), 0);
				decode_bbox->xmax = MAX(MIN(decode_bbox->xmax, 1), 0);
				decode_bbox->ymax = MAX(MIN(decode_bbox->ymax, 1), 0);
			}
		}
	}
}

static VOID quick_sort(BOX_IDX *p_boxes, INT32 left, INT32 right)
{
	INT32 l = left;
	INT32 r = right;
	BOX_IDX key = p_boxes[left];
	FLOAT key_score = key.score;

	while (l < r) {
		while (l < r && key_score >= p_boxes[r].score) {
			r--;
		}

		if (l < r) {
			p_boxes[l] = p_boxes[r];
		}

		while (l < r && key_score <= p_boxes[l].score) {
			l++;
		}

		if (l < r) {
			p_boxes[r] = p_boxes[l];
			r--;
		}
	}

	p_boxes[l] = key;
	if (left < l - 1) {
		quick_sort(p_boxes, left, l - 1);
	}
	if (l + 1 < right) {
		quick_sort(p_boxes, l + 1, right);
	}
}

static VOID calc_area(BOX *p_boxes, BOX_INFO *p_boxes_info, INT32 box_num)
{
#define NUM_LANE            4
#define NUM_VEC_BOX         4
#define NUM_VEC_BOX_INFO    2

	float32_t *p_box;
	float32_t *p_box_info;
	float32x4_t   vwidth, vheight;
	float32x4x4_t vbox;
	float32x4x2_t vbox_info;
	INT32 i;

	p_box      = (float32_t *)p_boxes;
	p_box_info = (float32_t *)p_boxes_info;
	for (i = 0; i < ALIGN_CEIL(box_num, NUM_LANE); i += NUM_LANE) {
		vbox      = vld4q_f32(p_box);
		vbox_info = vld2q_f32(p_box_info);
		/* C code
		area[i] = (p_box->xmax - p_box->xmin) * (p_box->ymax - p_box->ymin);
		*/
		vwidth  = vsubq_f32(vbox.val[BOX_XMAX], vbox.val[BOX_XMIN]);
		vheight = vsubq_f32(vbox.val[BOX_YMAX], vbox.val[BOX_YMIN]);
		vbox_info.val[BOX_AREA] = vmulq_f32(vwidth, vheight);
		vst2q_f32(p_box_info, vbox_info);

		p_box      += NUM_LANE * NUM_VEC_BOX;
		p_box_info += NUM_LANE * NUM_VEC_BOX_INFO;
	}
}

static INT32 check_overlap(BOX *p_boxes, BOX_INFO *p_boxes_info, INT32 box_num, FLOAT overlap_thresh, INT32 max_out_box_num, INT32 *last_box_idx)
{
#define NUM_LANE            4
#define NUM_VEC_BOX         4
#define NUM_VEC_BOX_INFO    2
#define NUM_VEC             4

	float32_t *p_box;
	float32_t *p_box_info;
	float32x4_t   vleft, vright, vtop, vbottom;
	float32x4_t   vwidth, vheight;
	float32x4_t   vinter, vunion, vthresh, voverlapf, vpositivef;
	float32x4x4_t vbox, vout_box;
	float32x4x2_t vbox_info, vout_box_info;
	uint32x4_t    voverlap, vpositive;
	INT32 cnt = 0;
	INT32 last_idx = 0;
	INT32 i, j;

	vout_box.val     [0] = vmovq_n_f32(0);
	vout_box_info.val[0] = vmovq_n_f32(0);
	for (i = 0; i < box_num; i++) {
		p_box_info = (float32_t *)&p_boxes_info[i];
		vout_box_info = vld2q_lane_f32(p_box_info, vout_box_info, 0);

		if (vgetq_lane_f32(vout_box_info.val[BOX_SCORE], 0) < 0) {
			continue;
		} else {
			cnt++;
			last_idx = i;
			if (cnt >= max_out_box_num) {
				break;
			}
		}

		p_box = (float32_t *)&p_boxes[i];
		vout_box = vld4q_lane_f32(p_box, vout_box, 0);
		for (j = 0; j < NUM_VEC; j++) {
			vout_box.val[j] = vdupq_n_f32(vgetq_lane_f32(vout_box.val[j], 0));
		}

		p_box      = (float32_t *)&p_boxes     [i + 1];
		p_box_info = (float32_t *)&p_boxes_info[i + 1];
		for (j = i + 1; j < box_num;) {
			if (((BOX_INFO *)p_box_info)->score < 0) {
				j++;
				p_box      += NUM_VEC_BOX;
				p_box_info += NUM_VEC_BOX_INFO;
				continue;
			}

			vbox      = vld4q_f32(p_box);
			vbox_info = vld2q_f32(p_box_info);

			vleft     = vmaxq_f32(vbox.val[BOX_XMIN], vout_box.val[BOX_XMIN]);
			vright    = vminq_f32(vbox.val[BOX_XMAX], vout_box.val[BOX_XMAX]);
			vtop      = vmaxq_f32(vbox.val[BOX_YMIN], vout_box.val[BOX_YMIN]);
			vbottom   = vminq_f32(vbox.val[BOX_YMAX], vout_box.val[BOX_YMAX]);

			vwidth    = vsubq_f32(vright, vleft);
			vpositive = vcgeq_f32(vwidth, vmovq_n_f32(0));
			vpositive = vandq_u32(vpositive, vmovq_n_u32(1));
			vpositivef= vcvtq_f32_u32(vpositive);
			vwidth    = vmulq_f32(vwidth, vpositivef);

			vheight   = vsubq_f32(vbottom, vtop);
			vpositive = vcgeq_f32(vheight, vmovq_n_f32(0));
			vpositive = vandq_u32(vpositive, vmovq_n_u32(1));
			vpositivef= vcvtq_f32_u32(vpositive);
			vheight    = vmulq_f32(vheight, vpositivef);

			vinter    = vmulq_f32(vwidth, vheight);
			//vunion    = vaddq_f32(vbox_info.val[BOX_AREA], vout_box_info.val[BOX_AREA]);
			vunion    = vaddq_f32(vbox_info.val[BOX_AREA], vdupq_n_f32(vout_box_info.val[BOX_AREA][0]));
			vunion    = vsubq_f32(vunion, vinter);

			vthresh   = vmulq_n_f32(vunion, overlap_thresh);
			voverlap  = vcgeq_f32(vinter, vthresh);
			voverlapf = vcvtq_f32_s32(vreinterpretq_s32_u32(voverlap));
			vbox_info.val[BOX_SCORE] = vaddq_f32(vbox_info.val[BOX_SCORE], voverlapf);

			vst2q_f32(p_box_info, vbox_info);

			j          += NUM_LANE;
			p_box      += NUM_LANE * NUM_VEC_BOX;
			p_box_info += NUM_LANE * NUM_VEC_BOX_INFO;
		}
	}

	*last_box_idx = last_idx;
	return cnt;
}

static UINT32 nonmax_suppress(BOX_IDX *p_idx, BOX *p_out_boxes, BOX_INFO *p_out_boxes_info, INT32 num, FLOAT overlap_thresh, INT32 max_out_num)
{
	INT32 i = 0;
	INT32 j = 0;
	INT32 out_num = 0;
	INT32 cnt = 0;
	INT32 last_idx = 0;

	if (num == 0) {
		return 0;
	}
	if (num > 1) {
		quick_sort(p_idx, 0, num - 1);
	}

	for (i = 0; i < num; i++) {
		p_out_boxes[i] = *(BOX *)p_idx[i].box;
		p_out_boxes_info[i].score = p_idx[i].score;
	}
	p_out_boxes_info[num    ].score = -1;
	p_out_boxes_info[num + 1].score = -1;
	p_out_boxes_info[num + 2].score = -1;

	calc_area(p_out_boxes, p_out_boxes_info, num);
	cnt = check_overlap(p_out_boxes, p_out_boxes_info, num, overlap_thresh, max_out_num, &last_idx);

	i = 0;
	j = 1;
	out_num = MIN(max_out_num, cnt);
	while (1) {
		while (p_out_boxes_info[i].score > 0 && i < out_num) {
			p_idx[i].score = p_out_boxes_info[i].score;
			i++;
		}

		if (i >= out_num) {
			break;
		}

		if (i >= j) {
			j = i + 1;
		}

		while (p_out_boxes_info[j].score < 0 && j <= last_idx) {
			j++;
		}

		if (j > last_idx) {
			break;
		} else {
			p_idx[i].score = p_out_boxes_info[j].score;
			p_idx[i].box = p_idx[j].box;
			p_out_boxes_info[j].score = -1;
			i++;
			j++;
		}
	}

	return out_num;
}

static HD_RESULT ai_dnet_detout_process(NN_DETOUT_PARM *p_parm)
{

	HD_RESULT ret = HD_OK;

	INT32 num_loc_classes = (p_parm->share_loc == 1) ? 1 : p_parm->num_classes;
	INT32 background_label_id_ = p_parm->bg_lbl_id;
	FLOAT nms_threshold_ = p_parm->nms_thresh;
	FLOAT conf_threshold = p_parm->conf_thresh;
	INT32 code_type_ = p_parm->code_type;	//0 1 2depends
	INT32 variance_encoded_in_target_ = 0;
	INT32 num_priors_ = p_parm->num_priors;
	INT32 num_classes_ = p_parm->num_classes;

	FLOAT* mbox_loc = (FLOAT*)p_parm->in_addr[NN_DETOUT_IN_LOC];
	FLOAT* mbox_conf = (FLOAT*)p_parm->in_addr[NN_DETOUT_IN_CONF];
	FLOAT* mbox_priorbox = (FLOAT*)p_parm->in_addr[NN_DETOUT_IN_PRIOR];
	FLOAT* nms_rslt = (FLOAT*)p_parm->out_addr;


	NN_DETOUT_CALC_PRMS g_detout_prms = {0};
	g_detout_prms.all_loc_preds     = (NORM_BOX *)malloc(num_priors_ * num_loc_classes * sizeof(NORM_BOX));
	g_detout_prms.prior_bboxes      = (NORM_BOX *)malloc(num_priors_ * sizeof(NORM_BOX));
	g_detout_prms.all_decode_bboxes = (NORM_BOX *)malloc(num_priors_ * num_loc_classes * sizeof(NORM_BOX));
	g_detout_prms.decode_bboxes_    = (NORM_BOX *)malloc(num_priors_ * sizeof(NORM_BOX));
	g_detout_prms.all_conf_scores   = (FLOAT *)malloc(num_priors_ * num_classes_ * sizeof(FLOAT));
	g_detout_prms.prior_variances   = (FLOAT *)malloc(num_priors_ * 4 * sizeof(FLOAT));
	g_detout_prms.decode_bboxes_idx = (BOX_IDX *)malloc(num_priors_ * sizeof(BOX_IDX));
	g_detout_prms.out_bboxes        = (BOX *)malloc(ALIGN_4(num_priors_) * sizeof(BOX));
	g_detout_prms.out_bboxes_info   = (BOX_INFO *)malloc(ALIGN_4(num_priors_) * sizeof(BOX_INFO));

	INT32 index = 0;
	for (INT32 p = 0; p < num_priors_; ++p) {
		INT32 start_idx = p * num_loc_classes * 4;
		for (INT32 c = 0; c < num_loc_classes; ++c) {
			g_detout_prms.all_loc_preds[c * num_priors_ + p].xmin = mbox_loc[start_idx + c * 4];
			g_detout_prms.all_loc_preds[c * num_priors_ + p].ymin = mbox_loc[start_idx + c * 4 + 1];
			g_detout_prms.all_loc_preds[c * num_priors_ + p].xmax = mbox_loc[start_idx + c * 4 + 2];
			g_detout_prms.all_loc_preds[c * num_priors_ + p].ymax = mbox_loc[start_idx + c * 4 + 3];
		}
		NORM_BOX bbox = {0};
		bbox.xmin = mbox_priorbox[start_idx];
		bbox.ymin = mbox_priorbox[start_idx + 1];
		bbox.xmax = mbox_priorbox[start_idx + 2];
		bbox.ymax = mbox_priorbox[start_idx + 3];
		//float bbox_size = BBoxSize(&bbox);
		//bbox.size = bbox_size;
		g_detout_prms.prior_bboxes[p] = bbox;
		INT32 start_idx0 = num_priors_ * 4 + p * 4;
		for (INT32 j = 0; j < 4; ++j) {
			g_detout_prms.prior_variances[index] = mbox_priorbox[start_idx0 + j];
			index++;
		}
	}

	INT32 clip_bbox = 0;
	decode_bboxes(g_detout_prms.prior_bboxes, g_detout_prms.prior_variances, code_type_, variance_encoded_in_target_, clip_bbox, g_detout_prms.all_loc_preds, g_detout_prms.all_decode_bboxes, num_priors_, num_loc_classes);

	INT32 num_kept = p_parm->keep_top_k;
	INT32 after_num = 20;

	memset((VOID *)p_parm->out_addr, 0, sizeof(NORM_BOX) * num_classes_ * num_kept);
	NORM_BOX *p_box = g_detout_prms.decode_bboxes_;
	BOX_IDX *p_idx = g_detout_prms.decode_bboxes_idx;

	for (INT32 c = 0; c < num_classes_; ++c) {
		//printf("classs = %d\r\n", c);
		INT32 idx_nms = 0;
		INT32 out_num = 0;
		INT32 out_num_final = 0;
		INT32 nms_num_per_class = 0;
		if (c == background_label_id_) {
			continue;
		}
		for (INT32 i = 0; i < num_priors_; i++) {
			if (mbox_conf[i * num_classes_ + c] >= conf_threshold) {

				p_box[idx_nms].score = mbox_conf[i * num_classes_ + c];
				p_box[idx_nms].xmin  = g_detout_prms.all_decode_bboxes[i].xmin;
				p_box[idx_nms].xmax  = g_detout_prms.all_decode_bboxes[i].xmax;
				p_box[idx_nms].ymin  = g_detout_prms.all_decode_bboxes[i].ymin;
				p_box[idx_nms].ymax  = g_detout_prms.all_decode_bboxes[i].ymax;

				p_idx[idx_nms].score = p_box[idx_nms].score;
				p_idx[idx_nms].box	 = &p_box[idx_nms];

				idx_nms ++;
			}
		}

		if (idx_nms > 0) {
			nms_num_per_class = nonmax_suppress(p_idx, g_detout_prms.out_bboxes, g_detout_prms.out_bboxes_info, idx_nms, nms_threshold_, after_num);
		}

		if (nms_num_per_class != 0) {
			if (nms_num_per_class <= num_kept) {
				out_num = nms_num_per_class;
			} else {
				out_num = num_kept;
			}
			//printf("classs_id = %d, class_num = %d\r\n", c, nms_num_per_class);
			for (INT32 j = 0; j < out_num; j++) {
				p_box = p_idx[j].box;
				if(p_box->xmin >=1 || p_box->ymin >=1)
				  continue;
				if(p_box->xmax <=0 || p_box->ymax <=0)
				  continue;

				*(nms_rslt ++) = p_box->xmin;
				*(nms_rslt ++) = p_box->ymin;
				*(nms_rslt ++) = p_box->xmax;
				*(nms_rslt ++) = p_box->ymax;
				*(nms_rslt ++) = p_idx[j].score;
				out_num_final ++;
				//printf("score = %f, xmin = %f, ymin = %f, xmax = %f, ymax = %f\r\n", p_idx[j].score, p_box->xmin, p_box->ymin, p_box->xmax, p_box->ymax);
			}
		}
		for (INT32 j = out_num_final; j < TOP_N; j++) {
			for (INT32 k = 0; k < 5; k++) {
				*(nms_rslt ++) = -1;
			}
		}
	}

	free((VOID*)g_detout_prms.all_loc_preds);
	free((VOID*)g_detout_prms.prior_bboxes);
	free((VOID*)g_detout_prms.all_decode_bboxes);
	free((VOID*)g_detout_prms.decode_bboxes_);
	free((VOID*)g_detout_prms.all_conf_scores);
	free((VOID*)g_detout_prms.prior_variances);
	free((VOID*)g_detout_prms.decode_bboxes_idx);
	free((VOID*)g_detout_prms.out_bboxes);
	free((VOID*)g_detout_prms.out_bboxes_info);
	
	return ret;

}


static VOID concat_in6_axis2(FLOAT** in, FLOAT* out, INT32* in_step)
{

	INT32 i, j;
	INT32 step = in_step[0] + in_step[1] + in_step[2] + in_step[3] + in_step[4] + in_step[5];

	for (i = 0; i < 6; i++) {
		if (i == 0) {
			for (j = 0; j < in_step[i]; j++) {
				out[j] = in[0][j];
				out[j + step] = in[0][j + in_step[0]];
			}
		} else if (i == 1){
			for (j = 0; j < in_step[i]; j++){
				out[j + in_step[0]] = in[1][j];
				out[j + step + in_step[0]] = in[1][j + in_step[1]];
			}
		} else if (i == 2){
			for (j = 0; j < in_step[i]; j++){
				out[j + in_step[0] + in_step[1]] = in[2][j];
				out[j + step + in_step[0] + in_step[1]] = in[2][j + in_step[2]];
			}
		} else if (i == 3){
			for (j = 0; j < in_step[i]; j++){
				out[j + in_step[0] + in_step[1] + in_step[2]] = in[3][j];
				out[j + step + in_step[0] + in_step[1] + in_step[2]] = in[3][j + in_step[3]];
			}
		} else if (i == 4){
			for (j = 0; j < in_step[i]; j++){
				out[j + in_step[0] + in_step[1] + in_step[2] + in_step[3]] = in[4][j];
				out[j + step + in_step[0] + in_step[1] + in_step[2] + in_step[3]] = in[4][j + in_step[4]];
			}
		} else if (i == 5){
			for (j = 0; j < in_step[i]; j++){
				out[j + in_step[0] + in_step[1] + in_step[2] + in_step[3] + in_step[4]] = in[5][j];
				out[j + step + in_step[0] + in_step[1] + in_step[2] + in_step[3] + in_step[4]] = in[5][j + in_step[5]];
			}
		}
	}
}



HD_RESULT set_priorbox(NN_PRIORBOX_PARM* priorbox, UINT32 num_priorbox)
{
	
	HD_RESULT ret = HD_OK;

	for (UINT32 i = 0; i < num_priorbox; i++){

		priorbox[i].flip   = g_priorbox_param[i].flip;
		priorbox[i].clip   = g_priorbox_param[i].clip;
		priorbox[i].offset = g_priorbox_param[i].offset;
		
		priorbox[i].in_height  = g_priorbox_param[i].tensor_dim[2];
		priorbox[i].in_width   = g_priorbox_param[i].tensor_dim[3];
		priorbox[i].img_height = model_input_dim[2];
		priorbox[i].img_width  = model_input_dim[3];

		priorbox[i].variance_num = 4;
		priorbox[i].variances[0] = g_priorbox_param[i].variance[0];
		priorbox[i].variances[1] = g_priorbox_param[i].variance[1];
		priorbox[i].variances[2] = g_priorbox_param[i].variance[2];
		priorbox[i].variances[3] = g_priorbox_param[i].variance[3];

		priorbox[i].aspect_ratio_num = 2;
		priorbox[i].aspect_ratios[0] = g_priorbox_param[i].aspect_ratio[0];
		priorbox[i].aspect_ratios[1] = g_priorbox_param[i].aspect_ratio[1];

		priorbox[i].min_size_num = g_priorbox_param[i].min_size == 0 ? 0 : 1;
		priorbox[i].min_sizes[0] = g_priorbox_param[i].min_size;

		priorbox[i].max_size_num = g_priorbox_param[i].max_size == 0 ? 0 : 1;
		priorbox[i].max_sizes[0] = g_priorbox_param[i].max_size;

		ret = ai_dnet_priorbox_process(&priorbox[i]);
		if (HD_OK != ret){
			printf("nvtnn_priorbox_process failed\n");
			return ret;
		}

	}
	return ret;
}

HD_RESULT set_detout(FLOAT** conf_loc, NN_DETOUT_PARM* detout, NN_PRIORBOX_PARM* priorbox, UINT32 num_priorbox)
{

	HD_RESULT ret = HD_OK;

	detout->num_priors = 0;
	detout->batch_num = model_input_dim[0];
	detout->share_loc = g_detout_param.share_location;
	detout->bg_lbl_id = g_detout_param.background_label_id;
	detout->code_type = g_detout_param.code_type;
	detout->keep_top_k = g_detout_param.keep_top_k;
	detout->top_k      = g_detout_param.nms_param.top_k;
	detout->nms_thresh = g_detout_param.nms_param.nms_threshold;
	detout->conf_thresh = g_detout_param.confidence_threshold;
	detout->num_classes = g_detout_param.num_classes;

	FLOAT* priorbox_out[NUM_PRIORBOX];
	INT32 priorbox_concat_step[NUM_PRIORBOX];
	INT32 priorbox_all_byte = 0;
	for (UINT32 i = 0; i < NUM_PRIORBOX; i++) {
		priorbox_out[i] = (FLOAT*)priorbox[i].out_addr;
		priorbox_concat_step[i] = priorbox[i].out_channel_ofs / 4;
		priorbox_all_byte += priorbox[i].out_channel_ofs * 2;
		detout->num_priors += priorbox[i].out_channel_ofs / 16;
	}

	FLOAT* concat_priorbox = (FLOAT*)malloc(priorbox_all_byte);
	concat_in6_axis2(priorbox_out, concat_priorbox, priorbox_concat_step);

	detout->in_addr[NN_DETOUT_IN_LOC] = (UINTPTR)conf_loc[0];
	detout->in_addr[NN_DETOUT_IN_CONF] = (UINTPTR)conf_loc[1];
	detout->in_addr[NN_DETOUT_IN_PRIOR] = (UINTPTR)concat_priorbox;
	detout->out_addr = (UINTPTR)malloc(sizeof(NORM_BOX) * detout->num_classes * detout->keep_top_k);

	return ret;

}

HD_RESULT run_detout(NN_DETOUT_PARM* detout)
{

	HD_RESULT ret = HD_OK;

	ret = ai_dnet_detout_process(detout);
	if (HD_OK != ret){
		printf("nvtnn_detout_process failed\n");
		return ret;
	}

	return ret;
}

HD_RESULT release_buffer(VENDOR_AI3_BUF* buf, NN_DETOUT_PARM* detout, NN_PRIORBOX_PARM* priorbox, UINT32 num_priorbox, FLOAT** conf_loc)
{

	for (UINT32 i = 0; i < num_priorbox; i++) {
		free((VOID*)priorbox[i].out_addr);
	}

	free((VOID*)detout->in_addr[NN_DETOUT_IN_LOC]);		// conf_loc[0]
	free((VOID*)detout->in_addr[NN_DETOUT_IN_CONF]);	// conf_loc[1]
	free((VOID*)detout->in_addr[NN_DETOUT_IN_PRIOR]);
	free((VOID*)detout->out_addr);
	free((VOID*)priorbox);
	free((VOID*)detout);
	free((VOID*)conf_loc);
	free((VOID*)buf);

	return HD_OK;
}


