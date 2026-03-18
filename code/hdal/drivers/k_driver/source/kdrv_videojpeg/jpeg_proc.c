#ifdef __KERNEL__
#include <linux/slab.h>
#include <linux/buffer_head.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "jpeg_api.h"
#include "jpeg_platform.h"

#define JPEG_PROC_PATH			"kdrv_jpeg"		//chip0
#define PROC_MAX_BUFFER			64

//=========================================
//  local variable
//=========================================
static struct proc_dir_entry *proc_jpg_root_entry = NULL;
static struct proc_dir_entry *proc_jpg_sim_entry = NULL;
static struct proc_dir_entry *proc_jpg_dbglevel_entry = NULL;
static struct proc_dir_entry *proc_jpg_util_entry = NULL;
static struct proc_dir_entry *proc_jpg_dbg_entry = NULL;
static struct proc_dir_entry *proc_jpg_info_entry = NULL;
static struct proc_dir_entry *proc_jpg_rc_entry = NULL;
static struct proc_dir_entry *proc_jpg_eng_entry = NULL;
#if JPEG_ADJUST_ROI_TABLE
static struct proc_dir_entry *proc_jpg_roi_tbl_entry = NULL;
#endif


//=========================================
//  local variable
//=========================================
extern unsigned int jpeg_kdrv_debug_level;


//=========================================
//  proc simulation function
//=========================================
extern int nvt_jpg_driver_msg(struct seq_file *sfile);
extern int jpeg_enc_main(int pat_idx, char *out_name, int frame_num, int sce_en, int rot_type, int osg_mask_en, int restart, int rc_enable, int block_mode_en);
extern int jpeg_dec_main(int pat_idx, char *out_name, int frame_num, int block_mode_en);
extern int jpeg_enc_main_perf(int pat_idx, char *out_name, int frame_num, int sce_en, int rot_type, int osg_mask_en, int restart, int rc_enable, int block_mode_en);
extern int jpeg_dec_main_perf(int pat_idx, char *out_name, int frame_num, int block_mode_en);
static int proc_jpg_sim_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, " ============= encode test simulation ============= \n");
	seq_printf(sfile, " encode test:       echo 0  0 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " encode(block):     echo 0  1 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " encode 1080p:      echo 0  2 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " enc 1080p(block):  echo 0  3 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " osg test:          echo 0  4 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " osg test(mask):    echo 0  5 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " mask test(solid):  echo 0  6 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " mask test(mosaic): echo 0  7 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " mask test(hollow): echo 0  8 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " gray test:         echo 0  9 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " gray test(src):    echo 0 10 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " rc test:           echo 0 11 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " rc test(block):    echo 0 12 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " rotation(90):      echo 0 13 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " rotation(270):     echo 0 14 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " rotation(180):     echo 0 15 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " ycc test:          echo 0 16 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " ycc test(block):   echo 0 17 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " ycc + rota(90):    echo 0 18 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " ycc + rota(270):   echo 0 19 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " ycc + rota(180):   echo 0 20 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " encode (2160p):    echo 0 21 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " enc slice:         echo 0 30 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " enc slice (block): echo 0 31 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " user data:         echo 0 40 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " user data:         echo 0 41 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " encode test all:   echo 0 99 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " ============= decode test simulation ============= \n");
	seq_printf(sfile, " decode test:       echo 1  0 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " decode(block):     echo 1  1 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " decode 1080p:      echo 1  2 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " dec 1080p(block):  echo 1  3 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " dec progressive:   echo 1 10 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " decode test all:   echo 1 99 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " ================== performance =================== \n");
	seq_printf(sfile, " enc perf(normal):  echo 0 50 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " enc perf(ycc):     echo 0 51 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " enc perf(tran422): echo 0 52 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " enc perf(2160p):   echo 0 53 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " enc perf(2160p tran422): echo 0 54 > /proc/kdrv_jpeg/sim \n");
	seq_printf(sfile, " dec perf(normal):  echo 1 50 > /proc/kdrv_jpeg/sim \n");
	return 0;
}

static int proc_jpg_sim_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_sim_show, NULL);
}

static int proc_jpeg_sim_main(int codec_type, int test_case)
{
	if (0 == codec_type) {
		switch (test_case) {
		/* normal */
		case 0:
			jpeg_enc_main(0, "test0_cif", 30, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(2, "test0_1080p", 6, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 1:
			jpeg_enc_main(0, "test1_cif_b", 30, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			jpeg_enc_main(2, "test1_1080p_b", 6, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			// jpeg_enc_main(2, "test3_engine2", 6, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 2/*block_mode_en*/);
			// jpeg_enc_main(2, "test3_engine3", 6, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 3/*block_mode_en*/);
			break;
		/* osg */
		case 4:
			jpeg_enc_main(0, "test4_cif_osg1", 6, 0/*sce_en*/, 0/*rot_type*/, 1/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(0, "test4_cif_osg2", 6, 0/*sce_en*/, 0/*rot_type*/, 2/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 5:
			jpeg_enc_main(0, "test5_cif_b_osg3", 6, 0/*sce_en*/, 0/*rot_type*/, 3/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			jpeg_enc_main(0, "test5_cif_b_osg4", 6, 0/*sce_en*/, 0/*rot_type*/, 4/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			break;
		/* mask */
		case 6:
			//solid
			jpeg_enc_main(0, "test6", 6, 0/*sce_en*/, 0/*rot_type*/, (0x1<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			// jpeg_enc_main(2, "test6_1080p", 6, 0/*sce_en*/, 0/*rot_type*/, (0x1<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 7:
			//mosaic
			jpeg_enc_main(0, "test7", 6, 0/*sce_en*/, 0/*rot_type*/, (0x2<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			// jpeg_enc_main(2, "test7_1080p", 6, 0/*sce_en*/, 0/*rot_type*/, (0x2<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 8:
			//hollow
			jpeg_enc_main(0, "test8", 6, 0/*sce_en*/, 0/*rot_type*/, (0x3<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			// jpeg_enc_main(2, "test8_1080p", 6, 0/*sce_en*/, 0/*rot_type*/, (0x3<<4)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* gray */
		case 9:
			jpeg_enc_main(0, "test9", 6, 0/*sce_en*/, 0/*rot_type*/, 2|(0x1<<8)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 10:
			jpeg_enc_main(0, "test10", 6, 0/*sce_en*/, 0/*rot_type*/, 2|(0x2<<8)/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* rc */
		case 11:
			jpeg_enc_main(1, "test11", 120, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 1/*rc*/, 0/*block_mode_en*/);
			break;
		case 12:
			jpeg_enc_main(1, "test12_engine1", 120, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 1/*rc*/, 1/*block_mode_en*/);
			jpeg_enc_main(1, "test12_engine2", 120, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 1/*rc*/, 2/*block_mode_en*/);
			break;
		/* rotation */
		case 13:
			jpeg_enc_main(0, "test13", 6, 0/*sce_en*/, 1/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 14:
			jpeg_enc_main(0, "test14", 6, 0/*sce_en*/, 2/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 15:
			jpeg_enc_main(0, "test15", 6, 0/*sce_en*/, 3/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* ycc */
		case 16:
			jpeg_enc_main(3, "test16", 1, 1/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 17:
			jpeg_enc_main(3, "test17", 1, 1/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			break;
		/* ycc + rotation */
		case 18:
			jpeg_enc_main(3, "test18", 1, 1/*sce_en*/, 1/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 19:
			jpeg_enc_main(3, "test19", 1, 1/*sce_en*/, 2/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 20:
			jpeg_enc_main(3, "test20", 1, 1/*sce_en*/, 3/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* 2160p */
		case 21:
			// jpeg_enc_main(4, "test21", 1, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* transform422 */
		case 22:
			jpeg_enc_main(2, "test22", 10, 0/*sce_en*/, 0x10/*transf422*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* roi */
		case 23:
			jpeg_enc_main(0, "test23_roi_cif", 6, 0/*sce_en*/, 0/*rot_type*/, 1 << 0x14/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(2, "test23_roi_1080p", 6, 0/*sce_en*/, 0/*rot_type*/, 2 << 0x14/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 24:
			jpeg_enc_main(0, "test24_roi_cif_b", 6, 0/*sce_en*/, 0/*rot_type*/, 1 << 0x14/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			jpeg_enc_main(2, "test24_roi_1080p_b", 6, 0/*sce_en*/, 0/*rot_type*/, 2 << 0x14/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			break;
		/* osg mosaic */
		case 25:
			jpeg_enc_main(0, "test25_cif_b_mosaic1", 6, 0/*sce_en*/, 0/*rot_type*/, 5/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			jpeg_enc_main(0, "test25_cif_b_mosaic2", 6, 0/*sce_en*/, 0/*rot_type*/, 6/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			jpeg_enc_main(0, "test25_cif_b_mosaic3", 6, 0/*sce_en*/, 0/*rot_type*/, 7/*osg_en*/, 0/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			break;
		/* slice */
		case 30:
			jpeg_enc_main(2, "test30", 30, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0x10000/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 31:
			jpeg_enc_main(2, "test31_engine1", 30, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0x10000/*restart*/, 0/*rc*/, 1/*block_mode_en*/);
			jpeg_enc_main(2, "test31_engine2", 30, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0x10000/*restart*/, 0/*rc*/, 2/*block_mode_en*/);
			// jpeg_enc_main(2, "test31_engine3", 30, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0x10000/*restart*/, 0/*rc*/, 3/*block_mode_en*/);
			break;
		/* user data */
		case 40:
			jpeg_enc_main(0, "test40_length1", 1, 0/*sce_en*/, 0/*rot_type*/, 1 << 0xC/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(0, "test40_length2", 1, 0/*sce_en*/, 0/*rot_type*/, 2 << 0xC/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(0, "test40_length3", 1, 0/*sce_en*/, 0/*rot_type*/, 4 << 0xC/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(0, "test40_length4", 1, 0/*sce_en*/, 0/*rot_type*/, 8 << 0xC/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 41:
			jpeg_enc_main(0, "test41_length1", 1, 0/*sce_en*/, 0/*rot_type*/, 1 << 0x10/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(0, "test41_length2", 1, 0/*sce_en*/, 0/*rot_type*/, 2 << 0x10/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(0, "test41_length3", 1, 0/*sce_en*/, 0/*rot_type*/, 4 << 0x10/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			jpeg_enc_main(0, "test41_length4", 1, 0/*sce_en*/, 0/*rot_type*/, 8 << 0x10/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		/* performance */
		case 50:
			//1088p_normal
			jpeg_enc_main_perf(2, "test50", 250, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 51:
			//1080p_ycc
			jpeg_enc_main_perf(3, "test51", 250, 1/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 52:
			//1088p_transf422
			jpeg_enc_main_perf(2, "test52", 250, 0/*sce_en*/, 0x10/*transf422*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;			
		case 53:
			// jpeg_enc_main_perf(4, "test53", 100, 0/*sce_en*/, 0/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		case 54:
			// jpeg_enc_main_perf(4, "test54", 100, 0/*sce_en*/, 0x10/*rot_type*/, 0/*osg_en*/, 0/*restart*/, 0/*rc*/, 0/*block_mode_en*/);
			break;
		default:
			break;
		}
	}
	else if (1 == codec_type) {
		switch (test_case) {
		/* normal */
		case 0:
			jpeg_dec_main(0, "test0", 10, 0);
			break;
		case 1:
			jpeg_dec_main(0, "test1_block", 10, 1);
			// jpeg_dec_main(0, "test1_engine2", 10, 2);
			break;
		/* 1080p */
		case 2:
			jpeg_dec_main(1, "test2", 6, 0);
			break;
		case 3:
			jpeg_dec_main(1, "test3_block", 6, 1);
			// jpeg_dec_main(1, "test3_engine2", 6, 2);
			break;
		/* fmt422 non-block */
		case 4:
			jpeg_dec_main(3, "test4_720p", 1, 0);
			jpeg_dec_main(6, "test4_1080p", 1, 0);
			break;
		/* 64x alignment */
		case 5:
			jpeg_dec_main(4, "test5", 1, 0);
			break;
		/* tmp */
		case 6:
			jpeg_dec_main(5, "test6", 10, 0);
			break;
		/* fmt422 block */
		case 7:
			jpeg_dec_main(3, "test7_720p", 1, 1);
			jpeg_dec_main(6, "test7_1080p", 1, 1);
			break;
		/* fmt400 non-block mode force sw decode */
		case 8:
			jpeg_dec_main(7, "test8_704x544", 1, 0);
			break;
		/* fmt400 block mode force sw decode */
		case 9:
			jpeg_dec_main(7, "test9_704x544", 1, 1);
			break;
		/* progressive */
		case 10:
			jpeg_dec_main(2, "test10", 1, 0);
			break;
		case 50:
			jpeg_dec_main_perf(1, "test50", 250, 0);
			break;
		default:
			break;
		}
	}
	else {
		// nothing
	}
	return 0;
}

static ssize_t proc_jpg_sim_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int codec_type, test_case;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		printk("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		printk("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d %d", &codec_type, &test_case);

	if (test_case == 99) {
		int i;
		for (i = 0; i < 60; i++)
			proc_jpeg_sim_main(codec_type, i);
	}
	else if (test_case == 98) {
		int i;
		for (i = 0; i < 50; i++)
			proc_jpeg_sim_main(codec_type, i);
	}
	else if (test_case == 97) {
		//mask
		int i;
		for (i = 6; i < 9; i++)
			proc_jpeg_sim_main(codec_type, i);
	}
	else {
		proc_jpeg_sim_main(codec_type, test_case);
	}

    return count;
}

static struct proc_ops proc_jpg_sim_fops = {
    .proc_open = proc_jpg_sim_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_sim_write
};

//=====================================
//  proc dbg level function
//=====================================
static int proc_jpg_dbglevel_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "JPEG dbg level = %d\n", (int)nvt_jpg_get_dbg_level());
    return 0;
}

static int proc_jpg_dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_dbglevel_show, NULL);
}

static ssize_t proc_jpg_dbglevel_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int level;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		printk("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		printk("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d", &level);

	nvt_jpg_set_dbg_level(level);
	//jpeg_kdrv_debug_level = level;

    return count;
}

static struct proc_ops proc_jpg_dbglevel_fops = {
    .proc_open = proc_jpg_dbglevel_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_dbglevel_write
};

//=====================================
//  proc utilization function
//=====================================
static int proc_jpg_util_show(struct seq_file *sfile, void *v)
{
	//seq_printf(sfile, "JPEG utildbg level = %d\n", (int)nvt_jpg_get_dbg_level());
	unsigned int util;
	int engine;

	seq_printf(sfile, "======== JEPG =============\n");
	seq_printf(sfile, "chip  eng  usage  fps\n");
	for (engine = 0; engine < JPEG_MAX_ENG; engine++) {
		util = nvt_jpeg_get_engine_util(0, engine);
		//if (util > 0)
		//seq_printf(sfile, "JPEG eng%d: HW Utilization Period=%d(sec) Utilization=%d\n", engine, nvt_jpg_get_util_record(), util);
		seq_printf(sfile, "%2d    %2d   %3d\n", 0, engine, util);
	}
    return 0;
}

static int proc_jpg_util_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_util_show, NULL);
}

static ssize_t proc_jpg_util_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	int util, engine;
	char proc_buffer[PROC_MAX_BUFFER];

	if (count > sizeof(proc_buffer) - 1) {
		printk("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		printk("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d", &util);

	nvt_jpg_set_util_record(util);

	for (engine = 0; engine < JPEG_MAX_ENG; engine++) {
		nvt_jpeg_clear_engine_util(0, engine);
	}
    return count;
}

static struct proc_ops proc_jpg_util_fops = {
    .proc_open = proc_jpg_util_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_util_write
};

//=====================================
//  proc debug parameter function
//=====================================
extern int jpeg_get_pad_disable(void);
extern int jpeg_set_pad_disable(int disable);
typedef struct jpeg_param_mapping_st
{
    char *tokenName;
    int (*get_param)(void);
    int (*set_param)(int);
    int lb;
    int ub;
    char *note;
} JPEGParamMapInfo;

static const JPEGParamMapInfo jpeg_syntax[] = {
	{"BrcUpdateMode", 		&jpeg_get_quality_update_mode,	&jpeg_set_quality_update_mode,	0,	2,		"JPEG brc update mode"},
	{"FixQualityRatio",		&jpeg_get_quality_fix_ratio,	&jpeg_set_quality_fix_ratio,	1,	99,		"JPEG fix quality ratio"},
	{"MaxQualityStep",		&jpeg_get_max_quality_step,		&jpeg_set_max_quality_step,		1,	100,	"JPEG max quality step"},
	{"HeaderChecksumEn",	&jpeg_get_header_checksum_en,	&jpeg_set_header_checksum_en,	0,	1,		"JPEG header checksum enable"},
	{"SWTimeoutPeriod",		&jpeg_get_SWTimeoutPeriod,		&jpeg_set_SWTimeoutPeriod,		0,	20000,	"JPEG SW timeout period"},
	{"ChooseEngineUse",		&jpeg_get_choose_engine_use,	&jpeg_set_choose_engine_use,	0,	2,		"JPEG choose engine use"},
	{"BRCUpperBound",		&jpeg_get_upper_bound,			&jpeg_set_upper_bound,			100,200,	"JEPG BRC upper bound"},
	{"BRCLowerBound",		&jpeg_get_lower_bound,			&jpeg_set_lower_bound,			1,	100,	"JPEG BRC lower bound"},
	{"VBRUpdatePeriod",		&jpeg_get_update_period,		&jpeg_set_update_period,		-1,	120,	"JPEG VBR update state period (-1: half fps, 0: fps)"},	
	{"CheckDmaIdle",		&jpeg_get_check_dma_idle,		&jpeg_set_check_dma_idle,		0,	1,		"JPEG check dma idle"},
	{"PadDisable",			&jpeg_get_pad_disable,			&jpeg_set_pad_disable,			0,	1,		"JPEG pad disable"},
    {NULL,					NULL,							NULL,							0,	0,		NULL}
};

static int jpeg_show_param_syntax(struct seq_file *sfile)
{
    int idx = 0;
    seq_printf(sfile, "      parameter        value                   note\r\n");
    seq_printf(sfile, "=====================  =====  ======================================\r\n");
    for (idx = 0; idx < (int)(sizeof(jpeg_syntax)/sizeof(JPEGParamMapInfo)); idx++) {
        if (NULL == jpeg_syntax[idx].tokenName)
            break;
        seq_printf(sfile, "%-22s  ", jpeg_syntax[idx].tokenName);

        seq_printf(sfile, "%3d   ", jpeg_syntax[idx].get_param());
        seq_printf(sfile, "%s (range: %d ~ %d)\r\n", jpeg_syntax[idx].note, jpeg_syntax[idx].lb, jpeg_syntax[idx].ub);
    }
    return 0;
}

static int jpeg_set_param_syntax(char *str)
{
    int value;
    char cmd_str[0x80];
    int i, idx = -1;

    sscanf(str, "%s %d\n", cmd_str, &value);

    for (i = 0; i < (int)(sizeof(jpeg_syntax)/sizeof(JPEGParamMapInfo)); i++) {
        if (NULL == jpeg_syntax[i].tokenName)
            break;
        if (strcmp(jpeg_syntax[i].tokenName, cmd_str) == 0) {
            idx = i;
            break;
        }
    }
    if (idx >= 0) {
        if (value < jpeg_syntax[idx].lb || value > jpeg_syntax[idx].ub) {
            printk("%s(%d) is out of range! (%d ~ %d)\n", jpeg_syntax[idx].tokenName, value, jpeg_syntax[idx].lb, jpeg_syntax[idx].ub);
        }
        else {
            jpeg_syntax[idx].set_param(value);
        }
    }
    else {
        printk("unknown \"%s\"\n", cmd_str);
    }
    return 0;
}

static int proc_jpg_dbg_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "usage:\n");
	jpeg_show_param_syntax(sfile);
    return 0;
}

static int proc_jpg_dbg_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_dbg_show, NULL);
}

static ssize_t proc_jpg_dbg_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	char proc_buffer[PROC_MAX_BUFFER];

    if (count > sizeof(proc_buffer) - 1) {
        printk("input is too large\n");
        return -EINVAL;
    }
    if (copy_from_user(proc_buffer, buffer, count)) {
        printk("proc dbg copy from user failed\n");
        return -EFAULT;
    }
    proc_buffer[count] = '\0';

    jpeg_set_param_syntax(proc_buffer);

    return count;
}

static struct proc_ops proc_jpg_dbg_fops = {
    .proc_open = proc_jpg_dbg_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_dbg_write
};

//=====================================
//  proc info function
//=====================================
static int proc_jpg_info_show(struct seq_file *sfile, void *v)
{
	nvt_jpg_driver_msg(sfile);
    return 0;
}

static int proc_jpg_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_info_show, NULL);
}

static ssize_t proc_jpg_info_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    return count;
}

static struct proc_ops proc_jpg_info_fops = {
    .proc_open = proc_jpg_info_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_info_write
};

//=====================================
//  proc rc function
//=====================================
static const char RC_MODE_NAME[4][5] = {"NULL", "FIX", "CBR", "VBR"};
static int proc_jpg_rc_show(struct seq_file *sfile, void *v)
{
	int chn;
	JPEG_RC_INFO rc_info = {0};

	seq_printf(sfile, "chn  mode  fraem rate   bitrate      qp     cur.qp    priority\n");
	seq_printf(sfile, "===  ====  ==========  ========  =========  ======  ============\n");
	for (chn = 0; chn < jpeg_get_enc_max_chn(); chn++) {
		jpeg_get_rc_info(0, chn, &rc_info);

		if (JPG_RC_FIX == rc_info.mode) {
			seq_printf(sfile, "%3d   %s  ----------  --------      %2d       %2d\n", chn, RC_MODE_NAME[rc_info.mode], rc_info.quality, rc_info.quality);
		}
		else if (JPG_RC_CBR == rc_info.mode) {
			seq_printf(sfile, "%3d   %s  %4d/%-4d   %8d  %2d/%2d/%2d    %d\n", chn, RC_MODE_NAME[rc_info.mode], rc_info.frame_rate_base,
				rc_info.frame_rate_incr, rc_info.bitrate, rc_info.min_quality, rc_info.init_quality, rc_info.max_quality, rc_info.quality);
		}
		else if (JPG_RC_VBR == rc_info.mode) {
			seq_printf(sfile, "%3d   %s  %4d/%-4d   %8d  %2d/%2d/%2d    %d     %d (fps=%d/%d)\n", chn, RC_MODE_NAME[rc_info.mode], rc_info.frame_rate_base,
				rc_info.frame_rate_incr, rc_info.bitrate, rc_info.min_quality, rc_info.init_quality, rc_info.max_quality, rc_info.quality,
				rc_info.priority, rc_info.cur_fps, rc_info.min_fps);
		}
	}
    return 0;
}
static int proc_jpg_rc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_rc_show, NULL);
}
static ssize_t proc_jpg_rc_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	char proc_buffer[PROC_MAX_BUFFER];
	int chn;

    if (count > sizeof(proc_buffer) - 1) {
        printk("input is too large\n");
        return -EINVAL;
    }
    if (copy_from_user(proc_buffer, buffer, count)) {
        printk("proc dbg copy from user failed\n");
        return -EFAULT;
    }
    proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d", &chn);

	jpeg_set_dump_br(chn);

    return count;
}
static struct proc_ops proc_jpg_rc_fops = {
    .proc_open = proc_jpg_rc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_rc_write
};

//=====================================
//  proc engine function
//=====================================
static int proc_jpg_eng_show(struct seq_file *sfile, void *v)
{
	int chip, engine;
	JPEG_ENG_INFO eng_info = {0};
	seq_printf(sfile, "chp  eng             io address           type  clk\n");
	seq_printf(sfile, "===  ===  ==============================  ====  ===\n");
	for (chip = 0; chip < JPEG_MAX_CHIP; chip++) {
		for (engine = 0; engine < JPEG_MAX_ENG; engine++) {
			nvt_jpeg_get_engine_info(chip, engine, &eng_info);
			if (eng_info.active) {
				seq_printf(sfile, " %d    %d   0x%lx/0x%lx   %d     %d\n", chip, engine,
					eng_info.engine_base_pa, eng_info.engine_base_va, eng_info.active_type, eng_info.clk_flag);
			}
			else {
				seq_printf(sfile, " %d    %d      non-active\n", chip, engine);
			}
		}
	}
    return 0;
}
static int proc_jpg_eng_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_eng_show, NULL);
}
static ssize_t proc_jpg_eng_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    return count;
}
static struct proc_ops proc_jpg_eng_fops = {
    .proc_open = proc_jpg_eng_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_eng_write
};


#if JPEG_ADJUST_ROI_TABLE
//=====================================
//  roi_tbl engine function
//=====================================
extern int jpeg_get_jpeg_roi_tbl(int idx, int tbl[16]);
extern int jpeg_set_jpeg_roi_tbl(int idx, int tbl[16]);

static int proc_jpg_roi_tbl_show(struct seq_file *sfile, void *v)
{
	int roi_tbl[16] = {0};
	int idx;

	for (idx = 0; idx < 3; idx++) {
		if (jpeg_get_jpeg_roi_tbl(idx, roi_tbl) < 0)
			continue;
		seq_printf(sfile, "tbl[%d]: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", idx,
			roi_tbl[0], roi_tbl[1], roi_tbl[2], roi_tbl[3], roi_tbl[4], roi_tbl[5], roi_tbl[6], roi_tbl[7],
			roi_tbl[8], roi_tbl[9], roi_tbl[10], roi_tbl[11], roi_tbl[12], roi_tbl[13], roi_tbl[14], roi_tbl[15]);
	}
    return 0;
}
static int proc_jpg_roi_tbl_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_jpg_roi_tbl_show, NULL);
}
static ssize_t proc_jpg_roi_tbl_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	char proc_buffer[PROC_MAX_BUFFER];
	int idx, tbl[16];

	if (count > sizeof(proc_buffer) - 1) {
		printk("input is too large\n");
		return -EINVAL;
	}
	if (copy_from_user(proc_buffer, buffer, count)) {
		printk("proc dbg copy from user failed\n");
		return -EFAULT;
	}
	proc_buffer[count] = '\0';

	sscanf(proc_buffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &idx,
		&tbl[0], &tbl[1], &tbl[2], &tbl[3], &tbl[4], &tbl[5], &tbl[6], &tbl[7], 
		&tbl[8], &tbl[9], &tbl[10], &tbl[11], &tbl[12], &tbl[13], &tbl[14], &tbl[15]);

	jpeg_set_jpeg_roi_tbl(idx, tbl);

    return count;
}
static struct proc_ops proc_jpg_roi_tbl_fops = {
    .proc_open = proc_jpg_roi_tbl_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = proc_jpg_roi_tbl_write
};
#endif


//=====================================
//  proc main function
//=====================================
int nvt_jpg_proc_clear(void)
{
	#if JPEG_ADJUST_ROI_TABLE
	if (proc_jpg_roi_tbl_entry)
		proc_remove(proc_jpg_roi_tbl_entry);
	#endif
	if (proc_jpg_eng_entry)
		proc_remove(proc_jpg_eng_entry);
	if (proc_jpg_rc_entry)
		proc_remove(proc_jpg_rc_entry);
	if (proc_jpg_dbg_entry)
		proc_remove(proc_jpg_dbg_entry);
	if (proc_jpg_util_entry)
		proc_remove(proc_jpg_util_entry);
	if (proc_jpg_dbglevel_entry)
		proc_remove(proc_jpg_dbglevel_entry);
	if (proc_jpg_sim_entry)
		proc_remove(proc_jpg_sim_entry);
	if (proc_jpg_root_entry)
		proc_remove(proc_jpg_root_entry);
	return 0;
}

int nvt_jpg_proc_init(void)
{
	if ((proc_jpg_root_entry = proc_mkdir(JPEG_PROC_PATH, NULL)) == NULL) {
		printk("failed to create JPEG root\n");
		goto fail_proc;
	}

	if ((proc_jpg_sim_entry = proc_create("sim", S_IRUGO | S_IXUGO, proc_jpg_root_entry, &proc_jpg_sim_fops)) == NULL) {
		printk("failed to create proc sim!\n");
		goto fail_proc;
    }

	if ((proc_jpg_dbglevel_entry = proc_create("dbglevel", S_IRUGO | S_IXUGO, proc_jpg_root_entry, &proc_jpg_dbglevel_fops)) == NULL) {
		printk("failed to create proc dbglevel!\n");
		goto fail_proc;
	}

	if ((proc_jpg_util_entry = proc_create("utilization", S_IRUGO | S_IXUGO, proc_jpg_root_entry, &proc_jpg_util_fops)) == NULL) {
		printk("failed to create proc utilization!\n");
		goto fail_proc;
	}

	if ((proc_jpg_dbg_entry = proc_create("dbg", S_IRUGO | S_IXUGO, proc_jpg_root_entry, &proc_jpg_dbg_fops)) == NULL) {
		printk("failed to create proc dbg!\n");
		goto fail_proc;
	}

	if ((proc_jpg_info_entry = proc_create("info", S_IRUGO | S_IXUGO, proc_jpg_root_entry, &proc_jpg_info_fops)) == NULL) {
		printk("failed to create proc info!\n");
		goto fail_proc;
	}

	if ((proc_jpg_rc_entry = proc_create("rc", S_IRUGO | S_IXUGO, proc_jpg_root_entry, &proc_jpg_rc_fops)) == NULL) {
		printk("failed to create proc rc!\n");
		goto fail_proc;
	}

	if ((proc_jpg_eng_entry = proc_create("engine", S_IRUGO | S_IXUGO, proc_jpg_root_entry, &proc_jpg_eng_fops)) == NULL) {
		printk("failed to create proc eng!\n");
		goto fail_proc;
	}

	#if JPEG_ADJUST_ROI_TABLE
	if ((proc_jpg_roi_tbl_entry = proc_create("roi_tbl", S_IRUGO | S_IXUGO, proc_jpg_root_entry, &proc_jpg_roi_tbl_fops)) == NULL) {
		printk("failed to create proc roi_tbl!\n");
		goto fail_proc;
	}
	#endif

	return 0;
fail_proc:
	nvt_jpg_proc_clear();
	return -1;
}

#endif

