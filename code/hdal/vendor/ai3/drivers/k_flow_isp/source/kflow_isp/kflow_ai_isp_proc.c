/**
	@brief Source file of vendor net flow sample.

	@file kflow_ai_isp_proc.c

	@ingroup kflow ai net proc file

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#if defined(__FREERTOS)
#else
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <kwrap/dev.h>
#include <linux/vmalloc.h>
#endif

#include "kwrap/type.h"
#include "kwrap/file.h"
#include "kwrap/cpu.h"

#if !defined(__FREERTOS)
#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_))
#include <stdarg.h>
#elif defined(_BSP_NS02401_)
#include <linux/stdarg.h>
#endif
#endif
//=============================================================
#define __CLASS__ 				"[ai][kflow_isp][cmd]"
#include "kflow_ai_isp_debug.h"
//=============================================================
#include "kdrv_ai.h"
#include "kflow_isp/kflow_ai_isp_net.h"
#include "kflow_isp/kflow_isp.h"
#include "kflow_isp/kflow_isp_change_res.h"
#include "kflow_ai_isp_proc.h"
#include "kflow_ai_isp_version.h"
#include "kflow_ai_net/kflow_ai_net_platform.h"

static struct proc_dir_entry *kflow_ai_isp_proc_version_proc;
static struct proc_dir_entry *kflow_ai_isp_proc;
static struct proc_dir_entry *kflow_ai_isp_proc_kcmd_proc;

/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
// #define KFLOW_AI_DEBUG_PROG		1
// #define KFLOW_AI_DEBUG_RUN		2


#define KFLOW_AI_ISP_MAX_ARG_NUM    20
// #define KFLOW_AI_WAIT_VALUE     0xffffffff
#define KFLOW_AI_ISP_MAX_CMD_LENGTH 256
// static UINT32 kflow_ai_cmd_out_debug_state = KFLOW_AI_DEBUG_RUN;


extern VENDOR_AIS_ISP_PARM * g_ai_isp_parm ; 
extern AI_JMISP_INFO* g_ai_jmisp_info ; 
extern UINT32 *g_proc_trace ; 
extern INT32 kflow_isp_used_pool[4];
extern UINT32 kflow_isp_pool[4][5];
extern INT32 *kflow_isp_proc_pool_map;
extern INT32 *kflow_isp_mode_map;
extern UINT32 *kflow_isp_proc_core_mask;
extern UINT32 *kflow_isp_proc_ub_mask;
extern UINT32 g_isp_trig_mode ;
extern UINT32 MAX_ISP_NUM ;
extern UINT32 MAX_ISP_UB ;
/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/
typedef struct _PROC_CMD {
	char cmd[KFLOW_AI_ISP_MAX_CMD_LENGTH];
	int (*execute)(void* p_ctx, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

/*-----------------------------------------------------------------------------*/
/* Local Macros Declarations                                                   */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Extern Global Variables                                                     */
/*-----------------------------------------------------------------------------*/
// extern UINT32 g_ai_support_net_max;

char output_path[KFLOW_AI_ISP_MAX_CMD_LENGTH] = DBG_OUT_PATH;

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/
UINT32 kflow_isp_kcmd_proc_id = 0;
UINT32 kflow_isp_kcmd_trace = 0;

static int kflow_isp_write_file(char *filename, void *data, size_t count, unsigned long long *offset)
{
	int ret = 0;
	VOS_FILE fd;

	fd = vos_file_open(filename, O_RDWR|O_CREAT, 0666);

	if ((VOS_FILE)(-1) == fd) {
		DBG_DUMP("%s fails to open file %s\n", __func__, filename);
		ret = -1;
		goto exit;
	}

	ret = vos_file_write(fd, (void *)data, count);

	if (ret <= 0) {
		DBG_DUMP("%s: Fail to write file %s(errno = %d)!\n", __func__, filename, ret);
		ret = -1;
		goto exit;
	}

exit:
	if ((VOS_FILE)(-1) != fd)
		vos_file_close(fd);

	return ret;
}

/* ============================================================================= */
/* proc "Kernel Command" file operation functions								 */
/* ============================================================================= */



int kflow_ai_isp_kcmd_outpath(void* p_ctx, unsigned char argc, char **argv)
{

	if (argc < 1) {
		return -1;
	}
	snprintf(output_path, KFLOW_AI_ISP_MAX_CMD_LENGTH -1, argv[0]);
	DBG_DUMP("Set output path: %s\r\n", output_path);
	return 0;
}
INT _cvt_buff_name(e_AI_JMISP_IOBUF_CONST buffType, CHAR *p_ret_string, INT max_str_len)
{
	switch (buffType) {
		case e_AI_JMISP_IOBUF_RING_IN0:   snprintf(p_ret_string, max_str_len, "%s/RING_IN0.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_RING_IN1:   snprintf(p_ret_string, max_str_len, "%s/RING_IN1.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_PINGPONG_OUT0:  snprintf(p_ret_string, max_str_len, "%s/PINGPONG_OUT0.bin", output_path);  break;
		case e_AI_JMISP_IOBUF_PINGPONG_OUT1:   snprintf(p_ret_string, max_str_len, "%s/PINGPONG_OUT1.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_PINGPONG_OUT2:  snprintf(p_ret_string, max_str_len, "%s/PINGPONG_OUT2.bin", output_path);  break;
		case e_AI_JMISP_IOBUF_REF_IN0:   snprintf(p_ret_string, max_str_len, "%s/REF_IN0.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_REF_IN1:   snprintf(p_ret_string, max_str_len, "%s/REF_IN1.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_REF_OUT0:   snprintf(p_ret_string, max_str_len, "%s/REF_OUT0.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_REF_OUT1:   snprintf(p_ret_string, max_str_len, "%s/REF_OUT1.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG0:   snprintf(p_ret_string, max_str_len, "%s/INPUTFEAT_PINGPONG_SEG0.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_INPUTFEAT_PINGPONG_SEG1:   snprintf(p_ret_string, max_str_len, "%s/INPUTFEAT_PINGPONG_SEG1.bin", output_path);   break;
        case e_AI_JMISP_IOBUF_PPU_PIPEBUF0:   snprintf(p_ret_string, max_str_len, "%s/PPU_PIPEBUF0.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_PPU_PIPEBUF1:   snprintf(p_ret_string, max_str_len, "%s/PPU_PIPEBUF1.bin", output_path);   break;
        case e_AI_JMISP_IOBUF_FUSION_WEIGHT_IN:   snprintf(p_ret_string, max_str_len, "%s/FUSION_WEIGHT_IN.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_SIGMA_IN:   snprintf(p_ret_string, max_str_len, "%s/SIGMA_IN.bin", output_path);   break;
        case e_AI_JMISP_IOBUF_GAMMA_IN:   snprintf(p_ret_string, max_str_len, "%s/GAMMA_IN.bin", output_path);   break;
        case e_AI_JMISP_IOBUF_GAMMA_OUT:   snprintf(p_ret_string, max_str_len, "%s/GAMMA_OUT.bin", output_path);   break;
		case e_AI_JMISP_IOBUF_UNKNOWN:   snprintf(p_ret_string, max_str_len, "%s/???.bin", output_path);   break;
		default:
			snprintf(p_ret_string, max_str_len, "%s/xxx.bin", output_path);   break;
			return (-1);
	}
	return 0;
}
int kflow_ai_isp_dump_jmisp_buf(UINT32 proc_id)
{
	AI_JMISP_INFO *jmisp_info = 0 ;
	CHAR  buff_name[KFLOW_AI_ISP_MAX_CMD_LENGTH];
	UINT32 i = 0 ; 

	if (proc_id >= kflow_ai_get_net_supported_num()){
		DBG_ERR(" proc_id[%u] is out of range ! \r\n",proc_id) ; 
		return -1;
	}

	jmisp_info = &g_ai_jmisp_info[proc_id] ; 
	
	DBG_DUMP( "dump proc_id[%u] nnisp buf..... \n",proc_id );
	for(i = 0 ; i < (UINT32)e_AI_IOBUF_NUM ; i++){
		AI_JMISP_IOBUF_INFO* iobuf = &jmisp_info->iobuf[i] ; 
	 
		if ( iobuf->buffer_sz > 0 ){
			_cvt_buff_name((e_AI_JMISP_IOBUF_CONST)i, buff_name, KFLOW_AI_ISP_MAX_CMD_LENGTH);
			DBG_DUMP(" Dumping %s ....\n", buff_name ) ; 
			iobuf->va = (uintptr_t)nvt_ai_pa2va_remap(iobuf->pa, iobuf->buffer_sz);
			DBG_DUMP(" iobuf->va %lx .iobuf->pa.%lx iobuf->size %u ..\n", (ULONG)iobuf->va , (ULONG)iobuf->pa, iobuf->buffer_sz ) ; 
			vos_cpu_dcache_sync(iobuf->va, (UINT32)(iobuf->buffer_sz), VOS_DMA_TO_DEVICE);
			kflow_isp_write_file(buff_name, (void*)iobuf->va, iobuf->buffer_sz, 0) ;
			nvt_ai_pa2va_unmap(iobuf->va, iobuf->pa) ; 
		}else{
			continue ;
		}
	}
	return 0;
}

int kflow_ai_isp_kcmd_dump(void* p_ctx, unsigned char argc, char **argv)
{
	if (argv[0] == NULL) {
		DBG_ERR("Invalid cmd_args\r\n");
		DBG_DUMP("=====================================================================\n");
		DBG_DUMP( "dump");
		DBG_DUMP("=====================================================================\n");
		DBG_DUMP( "dump [proc_id] :       dump nnisp buf");
		return -1;
	}

	sscanf(argv[0], "%u", &kflow_isp_kcmd_proc_id);
	if (g_ai_isp_parm == NULL ){
		DBG_ERR(" Not init yet \n");
		return -1;
	}
	g_proc_trace[kflow_isp_kcmd_proc_id] |= AIISP_DUMP ;
	
	return 0;
}

int kflow_ai_isp_kcmd_clear_debug(void* p_ctx, unsigned char argc, char **argv)
{
	if (argc < 2) {
		return -1;
	}
	sscanf(argv[0], "%u", &kflow_isp_kcmd_proc_id);
	if (strncmp(argv[1], "all", 3) == 0) {
		g_proc_trace[kflow_isp_kcmd_proc_id] = 0;
	
	} else if (strcmp(argv[1], "trace") == 0) {
		kflow_isp_kcmd_trace = 0;
	} else {
		DBG_ERR("Invalid cmd_args\r\n");
	}
	DBG_DUMP("\n=> clear debug: proc[%d]\r\n", (int)kflow_isp_kcmd_proc_id);

	return 0;
}

int kflow_ai_isp_kcmd_trace(void* p_ctx, unsigned char argc, char **argv)
{
	if (argc < 2) {
		DBG_ERR("Invalid cmd_args\r\n");
		DBG_DUMP("=====================================================================\n");
		DBG_DUMP( "trace");
		DBG_DUMP("=====================================================================\n");
		DBG_DUMP( "trace [mask] :       set trace");

		return -1;
	}
	sscanf(argv[0], "%u", &kflow_isp_kcmd_proc_id);
	sscanf(argv[1], "%x", &kflow_isp_kcmd_trace);
	if (g_proc_trace != NULL)
		g_proc_trace[kflow_isp_kcmd_proc_id] = kflow_isp_kcmd_trace;
	DBG_DUMP("\n=> trace = %#x: proc[%d]\r\n", kflow_isp_kcmd_trace, (int)kflow_isp_kcmd_proc_id);
	
	return 0;
}
int kflow_ai_isp_kcmd_pool(void* p_ctx, unsigned char argc, char **argv)
{
	UINT32 i ;
	INT32 pool_id ; 
	UINT32 isp_mode;
	CHAR  isp_mode_name[32];

	DBG_DUMP("=> pool status dump begin:\r\n");
	DBG_DUMP("------ POOL LAYOUT -----\r\n");
	DBG_DUMP("[ai-isp-cfg] : <isp_trig_mode : %u> (0: off , 1: frame mode) \n", g_isp_trig_mode );
	DBG_DUMP("[ai_maxisp] : <%u> \n", MAX_ISP_NUM );
	DBG_DUMP("[ai_maxubuf] : <%u> \n\n", MAX_ISP_UB );
	DBG_DUMP("[pool_id] : [UB status][UB st_id][UB end_id][Core st_ch][Core end_ch] \n");
	for (i=0; i < 4; i++){
		if (kflow_isp_used_pool[i] >= 0 ){
			DBG_DUMP("pool_%u: <%u %u %u %u %u> \n", i, kflow_isp_pool[i][0], kflow_isp_pool[i][1], kflow_isp_pool[i][2], kflow_isp_pool[i][3], kflow_isp_pool[i][4]) ;
		}
	}
	if (kflow_isp_proc_pool_map != NULL) {
		DBG_DUMP("------ POOL STATUS -----\r\n");
		for (i=0 ; i < kflow_ai_get_net_supported_num(); i++){
			if (kflow_isp_proc_pool_map[i] >= 0 ){
				pool_id = kflow_isp_proc_pool_map[i] ; 
				isp_mode = kflow_isp_mode_map[i];
				if (isp_mode == 0){
					snprintf(isp_mode_name, 32, "AI3_ISP_SIMGLE_MODE");
				}else if (isp_mode == 1){
					snprintf(isp_mode_name, 32, "AI3_ISP_MULTI_ISO_MODE");
				}else if (isp_mode == 2){
					snprintf(isp_mode_name, 32, "AI3_ISP_MULTI_DEV_MODE");
				}else{
					snprintf(isp_mode_name, 32, "MODE is not setting");
				}
				DBG_DUMP("proc_id(%u) => pool_%d: <%u %u %u %u %u> %s \n", i, pool_id, kflow_isp_pool[pool_id][0], kflow_isp_pool[pool_id][1], kflow_isp_pool[pool_id][2], kflow_isp_pool[pool_id][3], kflow_isp_pool[pool_id][4], isp_mode_name) ; 
			}
			if (kflow_isp_proc_core_mask[i] > 0 ){
				DBG_DUMP("proc_id(%u) => core_mask(0x%02x) ub_mask(0x%02x)\n", i, kflow_isp_proc_core_mask[i], kflow_isp_proc_ub_mask[i]); 
			}

		}	
	}
	DBG_DUMP("------------------------\r\n");
	DBG_DUMP("=> pool status dump end:\r\n");
	return 0;
}

static PROC_CMD kcmd_list[] = {
	// keyword      function name
	{ "dump",		    kflow_ai_isp_kcmd_dump       },
	{ "clear_debug",  	kflow_ai_isp_kcmd_clear_debug},
	{ "trace",			kflow_ai_isp_kcmd_trace  	 },
	{ "outpath",        kflow_ai_isp_kcmd_outpath    },
	{ "pool",           kflow_ai_isp_kcmd_pool       },

};

static int kflow_ai_isp_proc_kcmd_show(struct seq_file *s, void *v)
{
	DBG_DUMP("proc[%d]:\r\n", (int)kflow_isp_kcmd_proc_id);
	DBG_DUMP("  trace = %#x\r\n", kflow_isp_kcmd_trace);

	return 0;
}

#define NUM_OF_CMD (sizeof(kcmd_list) / sizeof(PROC_CMD))

static ssize_t kflow_ai_isp_proc_kcmd_write(struct file *file, const char __user *buf, long unsigned int size, long long *off)
{
	int len = size;
	int ret = 0;
	char cmd_line[KFLOW_AI_ISP_MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[KFLOW_AI_ISP_MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned char loop;

	// check command length
	if (len > (KFLOW_AI_ISP_MAX_CMD_LENGTH - 1)) {
		DBG_ERR("Command length is too long!\n");
		goto ERR_OUT;
	}

	if (len == 0) {
		DBG_ERR("Command length = 0!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	DBG_IND("CMD:%s\n", cmd_line);

	// parse command string
	for (ucargc = 0; ucargc < KFLOW_AI_ISP_MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	// dispatch command handler
	for (loop = 0 ; loop < NUM_OF_CMD; loop++) {
		if (strncmp(argv[0], kcmd_list[loop].cmd, KFLOW_AI_ISP_MAX_CMD_LENGTH) == 0) {
			ret = kcmd_list[loop].execute(0, ucargc - 1, &argv[1]);
			break;
		}
	}
	if (loop >= NUM_OF_CMD) {
		goto ERR_INVALID_CMD;
	}
	if (ret != 0) {
		goto ERR_INVALID_CMD;
	}

	return size;

ERR_INVALID_CMD:
	DBG_ERR("Invalid CMD !!\r\n  Usage : type  \"cat /proc/kflow_ai/help\" for help.\r\n");
	return -EINVAL;

ERR_OUT:
	return -EPERM;
}

static int kflow_ai_isp_proc_kcmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, kflow_ai_isp_proc_kcmd_show, NULL);
}

#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
static struct proc_ops kflow_ai_isp_proc_kcmd_ops = {
	.proc_open    = kflow_ai_isp_proc_kcmd_open,
	.proc_read    = seq_read,
	.proc_write   = kflow_ai_isp_proc_kcmd_write,
	.proc_lseek  = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations kflow_ai_isp_proc_kcmd_ops = {
	.owner   = THIS_MODULE,
	.open    = kflow_ai_isp_proc_kcmd_open,
	.read    = seq_read,
	.write   = kflow_ai_isp_proc_kcmd_write,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

static int kflow_ai_isp_proc_version_show(struct seq_file *s, void *v)
{
	seq_printf(s, "KFLOW_AI_ISP_IMPL_VERSION: %s\n", KFLOW_AI_ISP_IMPL_VERSION);

	return 0;
}

static int kflow_ai_isp_proc_version_open(struct inode *inode, struct file *file)
{
	return single_open(file, kflow_ai_isp_proc_version_show, NULL);
}

#if (defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_))
static struct proc_ops kflow_ai_isp_proc_version_ops = {
	.proc_open    = kflow_ai_isp_proc_version_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = seq_release
};
#else
static struct file_operations kflow_ai_isp_proc_version_ops = {
	.owner   = THIS_MODULE,
	.open    = kflow_ai_isp_proc_version_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif


int kflow_ai_isp_proc_create(void)
{
	kflow_ai_isp_proc = proc_mkdir("kflow_ai_isp", NULL);
	if (kflow_ai_isp_proc == NULL) {
		return -EFAULT;
	}

	kflow_ai_isp_proc_kcmd_proc = proc_create("kcmd", 0, kflow_ai_isp_proc, &kflow_ai_isp_proc_kcmd_ops);
	if (kflow_ai_isp_proc_kcmd_proc == NULL) {
		return -EFAULT;
	}

	kflow_ai_isp_proc_version_proc = proc_create("version", 0, kflow_ai_isp_proc, &kflow_ai_isp_proc_version_ops);
	if (kflow_ai_isp_proc_version_proc == NULL) {
		return -EFAULT;
	}

	return 0;
}

void kflow_ai_isp_proc_remove(void)
{
	if (kflow_ai_isp_proc_version_proc != 0) {
		proc_remove(kflow_ai_isp_proc_version_proc);
	}
	if (kflow_ai_isp_proc_kcmd_proc != 0) {
		proc_remove(kflow_ai_isp_proc_kcmd_proc);
	}
	if (kflow_ai_isp_proc != 0) {
		proc_remove(kflow_ai_isp_proc);
	}
}

int kflow_ai_isp_dbg_cal_slice(NN_ISP_FRAME* isp_input_frame)
{
	NN_ISP_SET_RES_PARAM in_info = {0} ; 
	NN_ISP_NN_INFO nn_info = {0} ; 
	ER er = E_OK ;
	
	in_info.proc_id = isp_input_frame->proc_id ;
	in_info.height =  isp_input_frame->height ;
	in_info.width = isp_input_frame->width;
	in_info.stripe_num = 1 ;
	in_info.stripe_overlap = 0 ;
	in_info.stripe_width[0] = isp_input_frame->width; ;
	er = kflow_isp_cb (NN_ISP_OP_CAL_SLICE, (void*) &in_info, (void*) &nn_info) ;
	
	return er ;
}

int kflow_ai_isp_dbg_push_frame(VENDOR_AIS_ISP_INPUT_INFO* isp_input_info)
{
	ER er = E_OK ;
	NN_ISP_FRAME input = {0} ;
	NN_ISP_FRAME output = {0};
	// imem[0] => input frame 
	// imem[1] => output frame 
	input.proc_id = isp_input_info->net_id ;
	input.pa = isp_input_info->imem[0].pa ; 
	output.proc_id = isp_input_info->net_id ;
	output.pa = isp_input_info->imem[1].pa ; 

	er = kflow_isp_cb (NN_ISP_OP_PUSH_FRAME, (void*) &input, (void*) &output) ;
	return er ;

}

int kflow_ai_isp_dbg_set_ref_frame(VENDOR_AIS_ISP_INPUT_INFO* isp_input_info)
{
	ER er = E_OK ;
	NN_ISP_FRAME input = {0} ;
	NN_ISP_FRAME output = {0};
	// imem[0] => input frame
	// imem[1] => output frame
	input.proc_id = isp_input_info->net_id ;
	input.pa = isp_input_info->imem[0].pa ;
	output.proc_id = isp_input_info->net_id ;
	output.pa = isp_input_info->imem[1].pa ;

	switch (isp_input_info->ref_frame_id) {
		case 0: // fusion_weight
			er = kflow_isp_cb (NN_ISP_OP_SET_FUSION_WEIGHT, (void*) &input, (void*) &output) ;
			break;
		case 1: // sigma
			er = kflow_isp_cb (NN_ISP_OP_SET_SIGMA, (void*) &input, (void*) &output) ;
			break;
		case 2: // gamma
			er = kflow_isp_cb (NN_ISP_OP_SET_GAMMA, (void*) &input, (void*) &output) ;
			break;
		default:
			DBG_ERR("ref_frame_id = (%d) is NOT supported...\r\n", (int)isp_input_info->ref_frame_id);
			return E_SYS;
	}

	return er ;

}

int kflow_ai_isp_dbg_set_isp_param(NN_ISP_ISP_PARAM* isp_param)
{
	ER er = E_OK ;
	er = kflow_isp_cb (NN_ISP_OP_SET_ISP_PARAM, (void*)isp_param, NULL) ;
	return er ;

}


