#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include "json.hpp"
#include "aiisp_pq.h"

#define PARAM_VERSION                       "version"
#define PARAM_MODE                          "mode"
#define PARAM_RESERVED_COMMON               "reserved_common"
#define PARAM_2DNR_STILL_STR_MANUAL         "_2dnr_still_str_manual"
#define PARAM_RESERVED_1_MANUAL             "reserved_1_manual"
#define PARAM_RESERVED_2_MANUAL             "reserved_2_manual"
#define PARAM_RESERVED_3_MANUAL             "reserved_3_manual"
#define PARAM_COEF_A_MANUAL                 "coef_a_manual"
#define PARAM_COEF_B_MANUAL                 "coef_b_manual"
#define PARAM_2DNR_STILL_STR_AUTO           "_2dnr_still_str_auto"
#define PARAM_RESERVED_1_AUTO               "reserved_1_auto"
#define PARAM_RESERVED_2_AUTO               "reserved_2_auto"
#define PARAM_RESERVED_3_AUTO               "reserved_3_auto"
#define PARAM_COEF_A_AUTO                   "coef_a_auto"
#define PARAM_COEF_B_AUTO                   "coef_b_auto"
#define PARAM_COEF_A_MIN                    "coef_a_min"
#define PARAM_COEF_A_MAX                    "coef_a_max"
#define PARAM_COEF_B_MIN                    "coef_b_min"
#define PARAM_COEF_B_MAX                    "coef_b_max"

using json = nlohmann::json;

static bool exists(const json &j, const std::string &key)
{
	return j.find(key) != j.end();
}

int aiisp_pq_parse_json(char *path, AIISP_PQ_PARAM *p_param)
{
	unsigned int i;
	std::ifstream file;
	file.open(path);

	memset(p_param, 0, sizeof(AIISP_PQ_PARAM));

	if (!file) {
		fprintf(stderr, "cannot open file: %s\n", path);
		return -1;
	}
	try {
		json cfg = json::parse(file);

		if (exists(cfg, PARAM_VERSION)) {
			p_param->version = (unsigned int)cfg[PARAM_VERSION];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_VERSION);
			return -1;
		}

		if (exists(cfg, PARAM_MODE)) {
			p_param->mode = (unsigned int)cfg[PARAM_MODE];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_MODE);
			return -1;
		}

		if (exists(cfg, PARAM_RESERVED_COMMON)) {
			p_param->reserved_common = (unsigned int)cfg[PARAM_RESERVED_COMMON];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_RESERVED_COMMON);
			return -1;
		}

		if (exists(cfg, PARAM_2DNR_STILL_STR_MANUAL)) {
			p_param->_2dnr_still_str_manual = (unsigned int)cfg[PARAM_2DNR_STILL_STR_MANUAL];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_2DNR_STILL_STR_MANUAL);
			return -1;
		}

		if (exists(cfg, PARAM_RESERVED_1_MANUAL)) {
			p_param->reserved_1_manual = (unsigned int)cfg[PARAM_RESERVED_1_MANUAL];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_RESERVED_1_MANUAL);
			return -1;
		}

		if (exists(cfg, PARAM_RESERVED_2_MANUAL)) {
			p_param->reserved_2_manual = (unsigned int)cfg[PARAM_RESERVED_2_MANUAL];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_RESERVED_2_MANUAL);
			return -1;
		}

		if (exists(cfg, PARAM_RESERVED_3_MANUAL)) {
			p_param->reserved_3_manual = (unsigned int)cfg[PARAM_RESERVED_3_MANUAL];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_RESERVED_3_MANUAL);
			return -1;
		}

		if (exists(cfg, PARAM_COEF_A_MANUAL)) {
			p_param->coef_a_manual = (unsigned int)cfg[PARAM_COEF_A_MANUAL];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_COEF_A_MANUAL);
			return -1;
		}

		if (exists(cfg, PARAM_COEF_B_MANUAL)) {
			p_param->coef_b_manual = (unsigned int)cfg[PARAM_COEF_B_MANUAL];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_COEF_B_MANUAL);
			return -1;
		}

		if (exists(cfg, PARAM_2DNR_STILL_STR_AUTO)) {
			std::vector<std::uint8_t> v = cfg[PARAM_2DNR_STILL_STR_AUTO];
			for(i = 0; i < AIISP_GAIN_MAX; i++) {
				p_param->_2dnr_still_str_auto[i] = v[i];
			}
		} else {
			fprintf(stderr, "unable to find %s\n", PARAM_2DNR_STILL_STR_AUTO);
			return -1;
		}

		if (exists(cfg, PARAM_RESERVED_1_AUTO)) {
			std::vector<std::uint8_t> v = cfg[PARAM_RESERVED_1_AUTO];
			for(i = 0; i < AIISP_GAIN_MAX; i++) {
				p_param->reserved_1_auto[i] = v[i];
			}
		} else {
			fprintf(stderr, "unable to find %s\n", PARAM_RESERVED_1_AUTO);
			return -1;
		}

		if (exists(cfg, PARAM_RESERVED_2_AUTO)) {
			std::vector<std::uint8_t> v = cfg[PARAM_RESERVED_2_AUTO];
			for(i = 0; i < AIISP_GAIN_MAX; i++) {
				p_param->reserved_2_auto[i] = v[i];
			}
		} else {
			fprintf(stderr, "unable to find %s\n", PARAM_RESERVED_2_AUTO);
			return -1;
		}

		if (exists(cfg, PARAM_RESERVED_3_AUTO)) {
			std::vector<std::uint8_t> v = cfg[PARAM_RESERVED_3_AUTO];
			for(i = 0; i < AIISP_GAIN_MAX; i++) {
				p_param->reserved_3_auto[i] = v[i];
			}
		} else {
			fprintf(stderr, "unable to find %s\n", PARAM_RESERVED_3_AUTO);
			return -1;
		}

		if (exists(cfg, PARAM_COEF_A_AUTO)) {
			std::vector<std::uint32_t> v = cfg[PARAM_COEF_A_AUTO];
			for(i = 0; i < AIISP_GAIN_MAX; i++) {
				p_param->coef_a_auto[i] = v[i];
			}
		} else {
			fprintf(stderr, "unable to find %s\n", PARAM_COEF_A_AUTO);
			return -1;
		}

		if (exists(cfg, PARAM_COEF_B_AUTO)) {
			std::vector<std::uint32_t> v = cfg[PARAM_COEF_B_AUTO];
			for(i = 0; i < AIISP_GAIN_MAX; i++) {
				p_param->coef_b_auto[i] = v[i];
			}
		} else {
			fprintf(stderr, "unable to find %s\n", PARAM_COEF_B_AUTO);
			return -1;
		}

		if (exists(cfg, PARAM_COEF_A_MIN)) {
			p_param->coef_a_min = (unsigned int)cfg[PARAM_COEF_A_MIN];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_COEF_A_MIN);
			return -1;
		}

		if (exists(cfg, PARAM_COEF_A_MAX)) {
			p_param->coef_a_max = (unsigned int)cfg[PARAM_COEF_A_MAX];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_COEF_A_MAX);
			return -1;
		}

		if (exists(cfg, PARAM_COEF_B_MIN)) {
			p_param->coef_b_min = (unsigned int)cfg[PARAM_COEF_B_MIN];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_COEF_B_MIN);
			return -1;
		}

		if (exists(cfg, PARAM_COEF_B_MAX)) {
			p_param->coef_b_max = (unsigned int)cfg[PARAM_COEF_B_MAX];
		}
		else {
			fprintf(stderr, "unable to find %s \n", PARAM_COEF_B_MAX);
			return -1;
		}
	} catch (json::parse_error &e) {
		fprintf(stderr, "%s\n", e.what());
	}

	return 0;
}

