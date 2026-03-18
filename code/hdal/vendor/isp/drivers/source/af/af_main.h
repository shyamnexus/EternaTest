#ifndef _AF_MAIN_H_
#define _AF_MAIN_H_

// From af_flow.h
extern AF_PARAM *af_param[AF_ID_MAX_NUM];
extern void af_flow_show_info(struct seq_file *sfile);
extern ER af_init_module(UINT32 id_list);
extern ER af_uninit_module(void);
extern void af_dump_va(struct seq_file *sfile);

// From aft_api_int.h
extern UINT32 aft_api_get_item_size(AFT_ITEM item);
extern ER aft_api_get_cmd(AFT_ITEM item, ULONG addr);
extern ER aft_api_set_cmd(AFT_ITEM item, ULONG addr);

#endif

