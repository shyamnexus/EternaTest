#ifndef _IQ_MAIN_H_
#define _IQ_MAIN_H_

// From iq_flow.h
extern BOOL test_mode_en[IQ_ID_MAX_NUM];
extern IQ_OB_MODE test_mode_ob[IQ_ID_MAX_NUM];
extern IQ_DG_MODE test_mode_dg[IQ_ID_MAX_NUM];
extern IQ_CG_MODE test_mode_cg[IQ_ID_MAX_NUM];
extern BOOL companding_en[IQ_ID_MAX_NUM];
extern BOOL fcurve_en[IQ_ID_MAX_NUM];
extern BOOL cfa_en[IQ_ID_MAX_NUM];
extern BOOL test_shdr_en[IQ_ID_MAX_NUM];
extern UINT32 test_ev_ratio[IQ_ID_MAX_NUM][ISP_SEN_MFRAME_MAX_NUM];
extern UINT32 test_tm_ratio[IQ_ID_MAX_NUM];
extern BOOL test_shdr_hbs_en[IQ_ID_MAX_NUM];
extern ISP_AE_HBS_PARAM test_hbs[IQ_ID_MAX_NUM];
extern UINT32 nr_r_ratio[IQ_ID_MAX_NUM];
extern UINT32 nr_b_ratio[IQ_ID_MAX_NUM];
extern UINT32 nr_ir_ratio[IQ_ID_MAX_NUM];
extern UINT32 residue_reset_num[IQ_ID_MAX_NUM];
extern BOOL tmnr_ae_still_en[IQ_ID_MAX_NUM];
extern void iq_flow_ext_set_irsub(IQ_ID id, UINT32 lb, UINT32 th, UINT32 rng);
extern void iq_flow_ext_set_pink_red(IQ_ID id, BOOL en, UINT32 mode, UINT32 th1, UINT32 th2);
extern void iq_flow_ext_set_sie_ob_manual(IQ_ID id, UINT32 enable, UINT32 value);
extern void iq_flow_ext_set_pre_f_ob_manual(IQ_ID id, UINT32 enable, UINT32 *value);
extern void iq_flow_ext_set_pre_ob_manual(IQ_ID id, UINT32 enable, UINT32 *value);
extern void iq_flow_ext_set_ife_f_ob_manual(IQ_ID id, UINT32 enable, UINT32 *value);
extern void iq_flow_ext_set_ife_ob_manual(IQ_ID id, UINT32 enable, UINT32 *value);
extern void iq_flow_ext_set_ob_mode_manual(IQ_ID id, UINT32 enable, UINT32 mode);
extern void iq_flow_ext_set_sie_dg_manual(IQ_ID id, UINT32 enable, UINT32 value);
extern void iq_flow_ext_set_pre_dg_manual(IQ_ID id, UINT32 enable, UINT32 value);
extern void iq_flow_ext_set_ife_dg_manual(IQ_ID id, UINT32 enable, UINT32 value);
extern void iq_flow_ext_set_dg_mode_manual(IQ_ID id, UINT32 enable, UINT32 mode);
extern void iq_flow_ext_set_sie_dg_max(IQ_ID id, UINT32 sie_dg_max);
extern void iq_flow_ext_set_cg_mode_manual(IQ_ID id, UINT32 enable, UINT32 mode);
extern void iq_flow_ext_set_dg_ai(IQ_ID id, UINT32 enable);
extern void iq_flow_ext_set_nr_size(IQ_ID id, UINT32 size);
extern void iq_flow_ext_set_wdr_subimg_lpf(IQ_ID id, UINT32 coef_0, UINT32 coef_1, UINT32 coef_2);
extern void iq_flow_ext_set_wdr_blend_w(IQ_ID id, UINT32 blend_w);
extern void iq_flow_ext_set_rgblpf(IQ_ID id, UINT32 en, UINT32 th0, UINT32 th1);
extern void iq_flow_ext_set_ipe_subimg_size(IQ_ID id, UINT32 width, UINT32 height);
extern void iq_flow_ext_set_edge_gamma_sel(IQ_ID id, UINT32 sel);
extern void iq_flow_ext_set_edge_region_str(IQ_ID id, UINT32 enable, UINT32 enh_thin, UINT32 enh_robust);
extern void iq_flow_ext_set_edge_overshoot_w(IQ_ID id, UINT32 overshoot_w, UINT32 undershoot_w);
extern void iq_flow_ext_set_edge_dir_th_shift(IQ_ID id, UINT32 th_shift);
extern void iq_flow_ext_set_dbcs_mode(IQ_ID id, UINT32 mode);
extern void iq_flow_ext_set_dbcs_y_wt(IQ_ID id, UINT32 wt0, UINT32 wt1, UINT32 wt2, UINT32 wt3, UINT32 wt4, UINT32 wt5  \
												, UINT32 wt6, UINT32 wt7, UINT32 wt8, UINT32 wt9, UINT32 wt10          \
												, UINT32 wt11, UINT32 wt12, UINT32 wt13, UINT32 wt14, UINT32 wt15);
extern void iq_flow_ext_set_dbcs_c_wt(IQ_ID id, UINT32 wt0, UINT32 wt1, UINT32 wt2, UINT32 wt3, UINT32 wt4, UINT32 wt5  \
												, UINT32 wt6, UINT32 wt7, UINT32 wt8, UINT32 wt9, UINT32 wt10          \
												, UINT32 wt11, UINT32 wt12, UINT32 wt13, UINT32 wt14, UINT32 wt15);
extern void iq_flow_ext_set_defog_min_diff(IQ_ID id, UINT32 min_diff);
extern void iq_flow_ext_set_lca_location(IQ_ID id, UINT32 ofs);
extern void iq_flow_ext_set_lca_edge_ker_size(IQ_ID id, UINT32 ker_size);
extern void iq_flow_ext_set_lca_y_out(IQ_ID id, UINT32 y_out);
extern void iq_flow_ext_set_lca_c_out(IQ_ID id, UINT32 c_out);
extern void iq_flow_ext_set_post_sharpen_con_eng(IQ_ID id, UINT32 con_eng);
extern BOOL iq_flow_get_id_valid(UINT32 id);
extern UINT32 iq_flow_get_info_map(UINT32 id);
extern ISP_MODULE *iq_get_module(void);
extern ER iq_init_module(UINT32 id_list, UINT32 dpc_en, UINT32 ecs_en, UINT32 _3dcc_en);
extern ER iq_uninit_module(void);

// From iqt_api_int.h
extern UINT32 iqt_api_get_item_size(IQT_ITEM item);
extern ER iqt_api_get_cmd(IQT_ITEM item, ULONG addr);
extern ER iqt_api_set_cmd(IQT_ITEM item, ULONG addr);
extern ER iqt_api_nnsc_get_cmd(IQT_ITEM item, ULONG addr);
extern ER iqt_api_nnsc_set_cmd(IQT_ITEM item, ULONG addr);

// From iq_msg.h
#if defined(__FREERTOS)
extern void iq_msg_show_info(void);
extern void iq_msg_show_buffer_size(void);
extern void iq_msg_show_param(IQ_ID iq_proc_id, UINT32 iq_proc_iso);
extern void iq_msg_show_ui(IQ_ID iq_proc_id);
extern void iq_msg_show_nnsc(IQ_ID iq_proc_id);
extern void iq_msg_show_cfg_path(void);
extern void iq_msg_show_low_power_info(void);
extern void iq_msg_show_low_power_param(void);
extern void iq_msg_show_manual_info(void);
#else
extern void iq_msg_show_info(struct seq_file *sfile);
extern void iq_msg_show_buffer_size(struct seq_file *sfile);
extern void iq_msg_show_param(struct seq_file *sfile, IQ_ID iq_proc_id, UINT32 iq_proc_iso);
extern void iq_msg_show_ui(struct seq_file *sfile, IQ_ID iq_proc_id);
extern void iq_msg_show_nnsc(struct seq_file *sfile, IQ_ID iq_proc_id);
extern void iq_msg_show_cfg_path(struct seq_file *sfile);
extern void iq_msg_show_low_power_info(struct seq_file *sfile);
extern void iq_msg_show_low_power_param(struct seq_file *sfile);
extern void iq_msg_show_manual_info(struct seq_file *sfile);

#endif

// From iq_ui_int.h
extern ER iq_ui_set_info(IQ_ID id, IQ_UI_ITEM index, UINT32 value);
extern UINT32 iq_ui_get_info(IQ_ID id, IQ_UI_ITEM index);

// From iq_nnsc_int.h
extern ER iq_nnsc_set_info(IQ_ID id, IQ_NNSC_ITEM index, UINT32 value);
extern UINT32 iq_nnsc_get_info(IQ_ID id, IQ_NNSC_ITEM index);

#endif

