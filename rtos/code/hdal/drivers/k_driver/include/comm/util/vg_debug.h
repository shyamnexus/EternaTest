/*
 *   @file   vg_debug.h
 *
 *   @brief  Videograph debug utility.
 *
 *   This is to establish graph links for different features.
 *   Depending on the different project, the corresponding graph_type_X.c is used.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#define __VG_DEBUG_H__

#define ENABLE_VG_TRACE_COUNT 0

#if ENABLE_VG_TRACE_COUNT
typedef struct {
	char func_name[32];
	char usr_string[32];
	int count;
} trace_point_info_t;

typedef struct {
	int total_line;
	int *is_used_ary;
	trace_point_info_t *trace_point_info_ary;
} trace_point_handle_t;
void vgdbg_print_trace_count(trace_point_handle_t *trace_name);

#endif /* ENABLE_VG_TRACE_COUNT */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#if ENABLE_VG_TRACE_COUNT

/* TRACE COUNT
 *   用來計算code被run到次數
 * Usage: 以file為單位
 *    VGDBG_TRACE_COUNT_INIT(sch_trace, 1000);    //初始化，並定義個數(第2參數是該檔案的最大行數)
 *    func() {
 *        ...
 *        COUNT_HERE(sch_trace, "your string");   //計算run到這裡的次數
 *        ...
 *    }
 *
 *    EXTERN_TRACE_COUNT(sch_trace);              //若是在別的檔案地方印，需要extern該變數
 *    print_func() {
 *      VGDBG_PRINT_TRACE_COUNT(sch_trace);       //印出count
 *    }
 */
#define COUNT_HERE(trace_name, tag_string)  \
	do { \
		if (!trace_name.is_used_ary[__LINE__]) { \
			strncpy(trace_name.trace_point_info_ary[__LINE__].func_name, __FUNCTION__, 31); \
			if (tag_string) \
				strncpy(trace_name.trace_point_info_ary[__LINE__].usr_string, tag_string, 31); \
			else \
				strncpy(trace_name.trace_point_info_ary[__LINE__].usr_string, "", 31); \
			trace_name.is_used_ary[__LINE__] = 1; \
			trace_name.trace_point_info_ary[__LINE__].count = 1;  \
		} \
		trace_name.trace_point_info_ary[__LINE__].count ++; \
	} while(0)


#define VGDBG_TRACE_COUNT_INIT(trace_name, total_file_lines) \
	static int trace_point_is_used[total_file_lines] = {0}; \
	static trace_point_info_t trace_info[total_file_lines]; \
	trace_point_handle_t trace_name = {total_file_lines, trace_point_is_used, trace_info};

#define VGDBG_PRINT_TRACE_COUNT(trace_name)     vgdbg_print_trace_count(&trace_name)

#define EXTERN_TRACE_COUNT(trace_name)     extern trace_point_handle_t trace_name
#else
#define COUNT_HERE(trace_name, tag_string)
#define VGDBG_TRACE_COUNT_INIT(trace_name, total_file_lines)
#define VGDBG_PRINT_TRACE_COUNT(trace_name)
#define EXTERN_TRACE_COUNT(trace_name)
#endif /* ENABLE_VG_TRACE_COUNT */

