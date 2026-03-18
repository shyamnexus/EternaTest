#ifndef __ctl_sie_module_api_h_
#define __ctl_sie_module_api_h_
#include "kwrap/type.h"

#define CTL_SIE_TEST_CMD DISABLE //test cmd, for develop using
#if CTL_SIE_TEST_CMD
typedef enum {
	CTL_SIE_TEST_FUNC_NONE =   0x00000000,
	CTL_SIE_TEST_FUNC_CA =     0x00000001,
	CTL_SIE_TEST_FUNC_LA =     0x00000002,
	CTL_SIE_TEST_FUNC_ENCODE = 0x00000020,
} CTL_SIE_TEST_FUNC;

#define CTL_SIE_TEST_PAT_GEN 			ENABLE	// ENABLE: sie pattern gen mode, DISABLE: Sensor in
#define CTL_SIE_TEST_DIRECT_FLOW 		DISABLE	// ENABLE: SIE to IPP direct mode flow enable, DISABLE: Dram mode
#define CTL_SIE_TEST_PAT_GEN_SZ_W		1920//3840
#define CTL_SIE_TEST_PAT_GEN_SZ_H		1080//2160
#define CTL_SIE_TEST_DVS_SENSOR_SIM 	DISABLE
#define CTL_SIE_TEST_COMBINE			DISABLE

#define CTL_SIE_TEST_PATGEN_EXT_IN_W		1920
#define CTL_SIE_TEST_PATGEN_EXT_IN_H		(1080+60+2) // CTL_SIE_TEST_PATGEN_EXT_ACT_H + CTL_SIE_TEST_PATGEN_EXT_ACT2_H + y
#define CTL_SIE_TEST_PATGEN_EXT_ACT_W		1920
#define CTL_SIE_TEST_PATGEN_EXT_ACT_H		1080
#define CTL_SIE_TEST_PATGEN_EXT_ACT2_W		1920
#define CTL_SIE_TEST_PATGEN_EXT_ACT2_H		60

#if CTL_SIE_TEST_DVS_SENSOR_SIM
#define CTL_SIE_TEST_FUNC_EN CTL_SIE_TEST_FUNC_NONE
#else
#define CTL_SIE_TEST_FUNC_EN CTL_SIE_TEST_FUNC_NONE//(CTL_SIE_TEST_FUNC_CA | CTL_SIE_TEST_FUNC_LA)
#endif

// for verify direct mode using
typedef struct {
	UINT32 id; 				//sie id, 0: SIE1, etc...
	UINT32 pat_gen_mode;	//1: color_bar, 2: random, 3: fixed, 4: h_inc, 5: hv_inc
	UINT32 pat_gen_val;		//colorbar: bar width, fixed: pixel value(12b), h_inc/hv_inc: start pixel value(12b)
	UINT32 fps;				//fps*10
	USIZE  size;
	UINT32 out_dest;		//group sie index, ref. to CTL_SIE_ID_IDX
} CTL_SIE_TEST_DIRECT_CFG_PAR;

typedef struct {
	UINT32 id; 		//sie1: 1<< CTL_SIE_ID_1, sie2: 1<< CTL_SIE_ID_2, support multi sie id input
	BOOL   en;		//0: stop, 1: start
} CTL_SIE_TEST_DIRECT_TRIG_PAR;

#endif

BOOL ctl_sie_cmd_set_dbg_type(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_set_dbg_level( unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_save_mem(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_save_raw(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_set_isp_dbg_type( unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_set_fb_dump(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_force_pat_gen(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_dump_signal(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_manual(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_info(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_dbg_log_rate(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_set_err_log_rate(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_buf(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_setclk(unsigned char argc, char **pargv);
#if CTL_SIE_TEST_CMD//Test Cmd
BOOL ctl_sie_cmd_dump_ca_rst(UINT32 id);
BOOL ctl_sie_cmd_dump_la_rst(UINT32 id);
BOOL ctl_sie_cmd_get_statis( unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_on(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_off(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_open(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_close(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_start(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_stop(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_trig(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_set_iosize(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_test_cmd(unsigned char argc, char **pargv);
BOOL ctl_sie_cmd_test(unsigned char argc, char **pargv);
ULONG ctl_sie_test_direct_open_cfg(CTL_SIE_TEST_DIRECT_CFG_PAR cfg_info);
INT32 ctl_sie_test_direct_close_cfg(UINT32 id);
BOOL ctl_sie_test_direct_trig(CTL_SIE_TEST_DIRECT_TRIG_PAR trig_par);
BOOL ctl_sie_cmd_direct(unsigned char argc, char **pargv);

#endif

#if defined(__LINUX)
int ctl_sie_cmd_execute(unsigned char argc, char **argv);
#else
#define EXPORT_SYMBOL(x)
#endif

#endif
