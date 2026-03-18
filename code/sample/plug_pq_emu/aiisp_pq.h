#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// define
//=============================================================================
#define AIISP_GAIN_MAX 16

//=============================================================================
// struct & enum definition
//=============================================================================
typedef struct _AIISP_PQ_PARAM {
	unsigned int version;
	unsigned int mode;                                           // 0: auto; 1: manual
	unsigned int reserved_common;                                // value [0, 255]
	unsigned char _2dnr_still_str_manual;                        // value [0, 255]
	unsigned char reserved_1_manual;                             // value [0, 255]
	unsigned char reserved_2_manual;                             // value [0, 255]
	unsigned char reserved_3_manual;                             // value [0, 255]
	unsigned int coef_a_manual;                                  // value [coef_a_min, coef_a_max]
	unsigned int coef_b_manual;                                  // value [coef_b_min, coef_b_max]
	unsigned char _2dnr_still_str_auto[AIISP_GAIN_MAX];          // value [0, 255]
	unsigned char reserved_1_auto[AIISP_GAIN_MAX];               // value [0, 255]
	unsigned char reserved_2_auto[AIISP_GAIN_MAX];               // value [0, 255]
	unsigned char reserved_3_auto[AIISP_GAIN_MAX];               // value [0, 255]
	unsigned int coef_a_auto[AIISP_GAIN_MAX];                    // value [coef_a_min, coef_a_max]
	unsigned int coef_b_auto[AIISP_GAIN_MAX];                    // value [coef_b_min, coef_b_max]
	unsigned int coef_a_min;                                     // value [0, 65535]
	unsigned int coef_a_max;                                     // value [0, 65535]
	unsigned int coef_b_min;                                     // value [0, 65535]
	unsigned int coef_b_max;                                     // value [0, 65535]
} AIISP_PQ_PARAM;

typedef struct _AIISP_PQ_FINAL_PARAM {
	unsigned int reserved_common;
	unsigned char _2dnr_still_str;
	unsigned char reserved_1;
	unsigned char reserved_2;
	unsigned char reserved_3;
	unsigned int coef_a;
	unsigned int coef_b;
} AIISP_PQ_FINAL_PARAM;

//=============================================================================
// extern functions
//=============================================================================
extern int aiisp_pq_parse_json(char *path, AIISP_PQ_PARAM *p_param);

#ifdef __cplusplus
}
#endif

