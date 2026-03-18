#ifndef __KDRV_VIE_API_H_
#define __KDRV_VIE_API_H_

#define KDRV_VIE_TEST_CMD DISABLE //test cmd, for develop using

#if KDRV_VIE_TEST_CMD
typedef enum {
	EXAM_KDRV_VIE_FUNC_NONE =   0x00000000,
} EXAM_KDRV_VIE_FUNC;


BOOL kdrv_vie_cmd_test(unsigned char argc, char **pargv);
BOOL kdrv_vie_cmd_save_yuv(unsigned char argc, char **pargv);
#endif

BOOL kdrv_vie_cmd_set_dbg_type(unsigned char argc, char **pargv);
BOOL kdrv_vie_cmd_set_dbg_level(unsigned char argc, char **pargv);
BOOL kdrv_vie_cmd_dbg_func( unsigned char argc, char **pargv);
BOOL kdrv_vie_cmd_info(unsigned char argc, char **pargv);

#if defined(__LINUX)
int kdrv_vie_cmd_execute(unsigned char argc, char **argv);
#endif

#endif
