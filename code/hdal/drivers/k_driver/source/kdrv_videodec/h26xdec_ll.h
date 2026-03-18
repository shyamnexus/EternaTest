#ifndef _HEVC_DEC_LL_H_
#define _HEVC_DEC_LL_H_

#define N_BIT_MASK(n)     (((unsigned int)1 << n) - 1)
#define N_BIT(n)			((unsigned int)1 << n)

#define LL_HEADER		((unsigned int)3)
#define LL_POLL		((unsigned int)2)
#define LL_READ		((unsigned int)1)
#define LL_WRITE		((unsigned int)0)

#define FIRST_JOB			N_BIT(0)
#define MIDDLE_JOB			N_BIT(1)
#define FINIAL_JOB			N_BIT(2)
#define NON_PLUS_ID_JOB		N_BIT(3)

#define LL_END_ADDR   0xFFFFFFF

#define LL_MAX_JOB_NUM			H26XD_CHN

typedef struct link_list_info_t
{
	unsigned int cur_cmd_offset;   // cmd_type, job_id, reg_offset
	unsigned int cur_cmd_data;     // value_data
}DecLLCmd;

typedef struct link_list_job_t
{
	unsigned int chip_id;
	unsigned int job_id;
	unsigned long base_va;
	unsigned long base_pa;
    unsigned long job_cmd_list_pa;
    unsigned long final_cmd_list_pa;
    unsigned long final_cmd_list_va;
	unsigned int *job_cmd_list;
	DecLLCmd job_header_cmd;
	unsigned int hw_chksum;
	int list_idx;
    int bd_chn_idx;
    int table_num_idx;
    unsigned int pre_codec_type;
    unsigned int pre_job_id;
    unsigned char is_busy;
    unsigned char can_fire;
    unsigned char priority;
    unsigned char change_type_flag;
}DecLLJob;

/* link list job function */
void init_ll_job(DecLLJob *ll_job_header, unsigned long start_addr_virt, unsigned long start_addr_phy, int list_idx, int chip_idx);
void reinit_ll(DecLLJob *ll_job_header);
void recover_ll_job_id(DecLLJob *ll_job_header);
void recover_ll_all(DecLLJob *ll_job_header);
int update_ll_finish_table(DecLLJob *ll_job_header, int *is_codec_header);
int get_ll_type_change_flag(DecLLJob *ll_job_header);
int get_ll_chn_idx(DecLLJob *ll_job_header);
int chk_ll_finish_job(DecLLJob *ll_job_header);
int get_ll_connect_cnt(int chip_idx, int list_idx);
void update_final_offset(DecLLJob *ll_job_header, int chn_idx, unsigned int codec_type);
void set_first_ll_header(DecLLJob **ll_header, DecLLJob *first_job_header, int chn_idx, unsigned int codec_type);
void connect_ll_job(DecLLJob *cur_job, int job_flag);
int select_ll_buf_and_set_busy(DecLLJob *ll_job_header_0, DecLLJob *ll_job_header_1, int chn_idx);
int select_ll_buf_to_fire(DecLLJob *ll_job_header_0, DecLLJob *ll_job_header_1, int *ll_buf_idx);
int test_ll_buf_idle(DecLLJob *ll_job_header);
void set_ll_buf_fire_flag(DecLLJob *ll_job_header);
void release_ll_buf_fire_flag(DecLLJob *ll_job_header);
void set_ll_buf_idle(DecLLJob *ll_job_header);
void set_ll_buf_busy(DecLLJob *ll_job_header);
void re_trigger_ll_job(DecLLJob *ll_job_header, int is_h265_ad, int isr_chn_idx);
void recover_ll_job_data(DecLLJob *ll_job_header);
/* link list cmd function */
void job_header_cmd(DecLLJob *ll_job_header, unsigned long next_job_st_addr, int job_flag);
void single_write_cmd_pre_job(DecLLJob *ll_job_header, unsigned int reg_offst, unsigned int data);
void single_write_cmd(DecLLJob *ll_job_header, unsigned int reg_offst, unsigned int data);
void single_read_cmd(DecLLJob *ll_job_header, unsigned int reg_offst);
void single_pf_pause_write_cmd(DecLLJob *ll_job_header, unsigned int reg_offst, unsigned int data);
void single_special_write_cmd(DecLLJob *ll_job_header, unsigned int reg_offst, unsigned int data);
void single_special_read_cmd(DecLLJob *ll_job_header, unsigned int reg_offst, unsigned int data);
void polling_cmd(DecLLJob *ll_job_header, unsigned int reg_offst, unsigned int poll_bit, unsigned char polling_one, unsigned int pf_pause);
void polling_cmd_val(DecLLJob *ll_job_header, unsigned int reg_offst, unsigned int poll_val, unsigned int is_MSB);
unsigned int pop_read_cmd(unsigned int *ll_base_addr, unsigned int *cmd_offset);
void chksum_polling(DecLLJob *ll_job_header, unsigned int reg_offst);
unsigned long get_ll_va_addr(DecLLJob *ll_job_header);
#endif /* _HEVC_DEC_LL_H_ */