/*
 *   @file   file_parser.h
 *
 *   @brief  file parser header file.
 *
 *   Here defines some tool APIs to get some info from config file.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef _FILE_PARSER_H_
#define _FILE_PARSER_H_

#define GMCFG_MAX_ELEMENTS 20

struct gmcfg_pool_element_t {
	char name[8];
	unsigned int width;
	unsigned int height;
	int ddr_no;
	int ddr_no_2;
	unsigned int channels;
	unsigned int reserved[4];
};

struct gmcfg_pool_info_t {
	unsigned int element_count;
	struct gmcfg_pool_element_t element[GMCFG_MAX_ELEMENTS];
};

struct cap_proc_vg_info {
	unsigned int vcapch;
	unsigned int ch;
	unsigned int num_of_path;
	unsigned int engine_minor;
	unsigned int width;
	unsigned int height;
	unsigned int vlos_sts;
	unsigned int fps;
	unsigned int max_fps;
	unsigned int is_interlace;
};

#define LOCAL_CHIP_ID   0
#define MAX_CHIPS       4

struct platform_graph_info {
	int disp_in_ch_cnt;
	int disp_in_entity_cnt;
	int disp_in_buffer_cnt;

	int disp_out_ch_cnt;
	int disp_out_entity_cnt;
	int disp_out_buffer_cnt;

	int enc_stream_ch_cnt;
	int enc_in_entity_cnt;
	int enc_in_buffer_cnt;

	int enc_out_entity_cnt;
	int enc_out_buffer_cnt;
};

int pa2ddrid(unsigned int rc_pa);
void file_parser_init(char *cfg_path);
int gmcfg_get_min_compressed_rate(void);
int gmcfg_get_enc_in_pool_info(struct gmcfg_pool_info_t *pool);
int gmcfg_get_dec_out_pool_info(struct gmcfg_pool_info_t *pool);
int gmcfg_get_enc_stream_count(void);
int gmcfg_get_dec_stream_count(void);
int proc_get_cap_vg_info(struct cap_proc_vg_info vg_info[], int max_count);
void gmcfg_get_platform_graph_info(struct platform_graph_info *info, int *cap_count);
char *gmcfg_get_config_pathname(void);

#endif
