/*#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>*/
#include "kdrv_ai_dbg.h"
#include "ai_proc.h"
#include "ai_main.h"
#include "ai_api.h"
#include "kwrap/mem.h"
#include <kwrap/file.h>
#include "../../../include/kdrv_ai_version.h"
#include "ai_emu.h"
#include "kdrv_ai.h"
#include "kwrap/error_no.h"
#include "ai_lib.h"

#if defined(__FREERTOS)
#include <stdlib.h>
#include <string.h>
#include "efuse_protected.h"
#else
#include <plat/efuse_protected.h>
#endif
#define EMUF_CEILING(a, n)              (((a) + ((n)-1)) & (~((n)-1)))

#define KDRV_AI_ECHO_TEST 0


KDRV_AI_JMISP_PATH_INFO g_path_info = {0};


ER kdrv_ai_chk_efuse_ai_api(VOID)
{
#if !defined(CONFIG_NVT_FPGA_EMULATION) && !defined(_NVT_FPGA_)
	if(efuse_check_available(NULL) != TRUE){
		DBG_ERR("objver mismatch in kdrv ai_api\r\n");
		return E_CTX;
	}
#endif

	return E_OK;
}

#if !defined(CONFIG_NVT_SMALL_HDAL)
int nvt_ai_api_read_reg(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	unsigned long reg_addr = 0;

	if (argc != 1) {
		nvt_dbg(ERR, "wrong argument:%d", argc);
		return -1;
	}

	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}
#if defined(__FREERTOS)
	if ((reg_addr = strtoul(pargv[0], NULL, 16)) == 0) {
		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
		return -1;
	}
#else
	if (kstrtoul(pargv[0], 0, &reg_addr)) {
		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
		return -1;
	}
#endif

	nvt_ai_drv_read_reg(pmodule_info, reg_addr);

	return 0;
}
#endif


#if KDRV_AI_JMISP_TEST
#include "kwrap/cpu.h"
typedef struct _JMISP_BLOCK_INFO {
	UINT32 blk_x_num;
	UINT32 blk_y_num;
	UINT32 blk_x_first_ofs;
	UINT32 blk_x_middle_ofs;
	UINT32 blk_x_last_ofs;
	UINT32 blk_y_first_height;
	UINT32 blk_y_middle_height;
	UINT32 blk_y_last_height;
} JMISP_BLOCK_INFO;

#define PL_TYPE_NULL     0
#define PL_TYPE_UPDATE   1
#define PL_TYPE_JUMP     2
#define PL_TYPE_TRIG     4
#define PL_TYPE_WAIT     5 
#define PL_TYPE_CONDJUMP 6
#define PL_TYPE_MOVE     7

static int load_data(char* path, uintptr_t addr)
{
	VOS_FILE fd;
	int len = 0;
	struct vos_stat f_stat;
	//struct file *filp  = NULL;
 
	if (1) {

		fd = vos_file_open(path, O_RDONLY, 0);
		if ((VOS_FILE)(-1) == fd) {
			//nvt_dbg(ERR, "failed in file open:%s\r\n", path);
			return 0;
		}
			
		if (vos_file_fstat(fd, &f_stat) != 0) {
			DBG_ERR("fstat error\r\n");
			vos_file_close(fd);
			return 0;
		}
		len = vos_file_read(fd, (void *)addr, f_stat.st_size);
		if (len != (int)f_stat.st_size) {
			DBG_ERR("read file error\r\n");
			vos_file_close(fd);
			return 0;
		}
		vos_file_close(fd);
	}
	return len;
}

UINT32 get_jmisp_workbuf_size(UINT32 max_lofs, UINT32 max_height)
{
	return (max_lofs*max_height*3 + 1024*9*2 + 12424);
}

VOID jmisp_set_blk_info(KDRV_AI_JMISP_FUNC_INFO* jmisp_info, JMISP_BLOCK_INFO* blk_info)
{
	UINT32 blk_x_size = 1024;
	UINT32 blk_y_size = 1024;
	
	// init blk info
	blk_info->blk_x_num = 1;
	blk_info->blk_y_num = 1;
	blk_info->blk_x_first_ofs  = jmisp_info->max_out_lofs;
	blk_info->blk_x_middle_ofs = jmisp_info->max_out_lofs;
	blk_info->blk_x_last_ofs   = jmisp_info->max_out_lofs;
	blk_info->blk_y_first_height  = jmisp_info->max_out_height;
	blk_info->blk_y_middle_height = jmisp_info->max_out_height;
	blk_info->blk_y_last_height   = jmisp_info->max_out_height;
	
	if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_BAYER) {
		blk_x_size = blk_x_size*3/2;
	}
	
	if (jmisp_info->max_out_lofs > blk_x_size) {
		if ((jmisp_info->max_out_lofs % blk_x_size) == 1) {
			blk_x_size = blk_x_size - 4;
		}

		blk_info->blk_x_first_ofs  = blk_x_size;
		blk_info->blk_x_middle_ofs = blk_x_size;
		blk_info->blk_x_last_ofs = jmisp_info->max_out_lofs % blk_x_size;
		blk_info->blk_x_num = jmisp_info->max_out_lofs / blk_x_size;
		if (blk_info->blk_x_last_ofs == 0) {
			blk_info->blk_x_last_ofs = blk_x_size;
		} else {
			blk_info->blk_x_num++;
		}
	}
	
	if (jmisp_info->max_out_height > blk_y_size) {
		if ((jmisp_info->max_out_height % blk_y_size) == 1) {
			blk_y_size = blk_y_size - 4;
		}

		blk_info->blk_y_first_height  = blk_y_size;
		blk_info->blk_y_middle_height = blk_y_size;
		blk_info->blk_y_last_height = jmisp_info->max_out_height % blk_y_size;
		blk_info->blk_y_num = jmisp_info->max_out_height / blk_y_size;
		if (blk_info->blk_y_last_height == 0) {
			blk_info->blk_y_last_height = blk_y_size;
		} else {
			blk_info->blk_y_num++;
		}
	}
}

int parse_ppu_ll_cmd3(uintptr_t ll_cmd_addr, UINT32 ll_cmd_size, uintptr_t buf_addr, KDRV_AI_JMISP_FUNC_INFO* jmisp_info, UINT32 x_ofs, UINT32 height)
{
	UINT32 job_idx = 0;
	UINT32 *p_ll_cmd = (UINT32*)ll_cmd_addr;
	UINT32 tmp_ll_idx = 0;
	UINT32 tmp_cmd = 0, tmp_ofs = 0, tmp_func = 0;;
	
	
	while(1) {
		//fread(&tmp_cmd, sizeof(unsigned int), 1, p_file);
		tmp_cmd = p_ll_cmd[tmp_ll_idx++];
		tmp_ofs = ((tmp_cmd >> 8) & 0xFFF);
		tmp_func = tmp_cmd & 0xF;
		if (tmp_func == 3) {
			job_idx++;
		}

		if (job_idx == 0) {
			if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV && tmp_ofs == 0x68) {
				p_ll_cmd[tmp_ll_idx] = vos_cpu_get_phy_addr(buf_addr);
			} else if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV && tmp_ofs == 0x70) {
				p_ll_cmd[tmp_ll_idx] = vos_cpu_get_phy_addr(buf_addr + (jmisp_info->max_out_lofs*height));
			} else if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV && tmp_ofs == 0x78) {
				p_ll_cmd[tmp_ll_idx] = vos_cpu_get_phy_addr(buf_addr + (2*jmisp_info->max_out_lofs*height));
			} else if (tmp_ofs >= 0x8C && tmp_ofs <= 0xA0) {
				p_ll_cmd[tmp_ll_idx] = jmisp_info->max_out_lofs;
			} else if (tmp_ofs == 0xA4) {
				if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_BAYER) {
					p_ll_cmd[tmp_ll_idx] = (x_ofs*2/3) & 0xFFFFFFFE;
					
				} else {
					p_ll_cmd[tmp_ll_idx] = x_ofs;
				}
			} else if (tmp_ofs == 0xA8) {
				p_ll_cmd[tmp_ll_idx] = height;
			}
		} else {
			if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV && tmp_ofs == 0x50) {
				p_ll_cmd[tmp_ll_idx] = vos_cpu_get_phy_addr(buf_addr);
			} else if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV && tmp_ofs == 0x58) {
				p_ll_cmd[tmp_ll_idx] = vos_cpu_get_phy_addr(buf_addr + (jmisp_info->max_out_lofs*height));
			} else if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV && tmp_ofs == 0x60) {
				p_ll_cmd[tmp_ll_idx] = vos_cpu_get_phy_addr(buf_addr + (2*jmisp_info->max_out_lofs*height));
			} else if (tmp_ofs >= 0x8C && tmp_ofs <= 0xA0) {
				p_ll_cmd[tmp_ll_idx] = jmisp_info->max_out_lofs;
			} else if (tmp_ofs == 0xA4) {
				if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_BAYER) {
					p_ll_cmd[tmp_ll_idx] = x_ofs*2/3;
				} else {
					p_ll_cmd[tmp_ll_idx] = x_ofs;
				}
			} else if (tmp_ofs == 0xA8) {
				p_ll_cmd[tmp_ll_idx] = height;
			}
		}
		tmp_ll_idx++;

		if (tmp_func == 0) {
			break;
		}
	}
	
	
	return 0;
}
/*
int parse_ppu_ll_cmd_texture(uintptr_t ll_cmd_addr, UINT32 ll_cmd_size, UINT32 width, UINT32 height, UINT32 in_lofs, UINT32 out_lofs)
{
	UINT32 *p_ll_cmd = (UINT32*)ll_cmd_addr;
	UINT32 tmp_ll_idx = 0;
	UINT32 tmp_cmd = 0, tmp_ofs = 0, tmp_func = 0;
	
	
	while(1) {
		//fread(&tmp_cmd, sizeof(unsigned int), 1, p_file);
		tmp_cmd = p_ll_cmd[tmp_ll_idx++];
		tmp_ofs = ((tmp_cmd >> 8) & 0xFFF);
		tmp_func = tmp_cmd & 0xF;


		if (tmp_ofs == 0x8C) {
			p_ll_cmd[tmp_ll_idx] = in_lofs;
		} else if (tmp_ofs == 0x98) {
			p_ll_cmd[tmp_ll_idx] = out_lofs;
		} else if (tmp_ofs == 0xA4) {
			p_ll_cmd[tmp_ll_idx] = width; // width
		} else if (tmp_ofs == 0xA8) {
			p_ll_cmd[tmp_ll_idx] = height;
		}
		
		tmp_ll_idx++;

		if (tmp_func == 0) {
			break;
		}
	}
	
	
	return 0;
}
*/

UINT32 pl_parse_cmd2(uintptr_t pl_addr, UINT32 pl_size, uintptr_t* current_unit_ll_addr)
{
	UINT32 i = 0;
	UINT32* p_cmd_buf;
	UINT32 cmd_type = 0;
	UINT32 pl_id = 0;
	//UINT32 reg_ofs = 0;
	//UINT32 reg_val = 0;
	UINT32 jump_addr = 0;
	UINT32 jump_ofs = 0;
	UINT64 trig_addr = 0;
	UINT32 addr_type = 0;
	//UINT32 unit_id = 0;
	UINT32 trig_cnt = 0;
	
	p_cmd_buf = (UINT32*)pl_addr;
	for (i = 0; i < pl_size/4; i++) {
		cmd_type = p_cmd_buf[i] & 0xF;
		
		if (cmd_type == PL_TYPE_NULL) {
			pl_id = p_cmd_buf[i] >> 28;
			//break;
		} else if (cmd_type == PL_TYPE_UPDATE) {
			/*reg_ofs = (p_cmd_buf[i] >> 8) & 0xFFF;
			reg_val = p_cmd_buf[i+1];
			if (reg_val == 0) {
				// for build pass
			}*/

			i++;
		} else if (cmd_type == PL_TYPE_JUMP || cmd_type == PL_TYPE_CONDJUMP) {
			jump_addr = ((p_cmd_buf[i] >> 16) & 0xFFFF) | ((p_cmd_buf[i+1] & 0xFFFFF) << 16);
			jump_ofs = jump_addr;
			jump_addr = jump_addr + vos_cpu_get_phy_addr(pl_addr);
			//if (current_use_base > 0) {
			//	jump_addr = jump_addr - current_use_base_addr;
			//	addr_type = current_use_base;
			//} else {
				addr_type = 0;
			//}
			p_cmd_buf[i] = (p_cmd_buf[i] & 0xFFFF) | ((jump_addr & 0xFFFF) << 16);
			p_cmd_buf[i+1] = (p_cmd_buf[i+1] & 0x0FF00000) | (addr_type << 28) | ((jump_addr >> 16) & 0xFFFFF);
			if (cmd_type == PL_TYPE_CONDJUMP) {
				i+=2;
			} else {
				if (jump_ofs == 0) {
					// for build pass
				}
				i++;
				//i = (jump_ofs/4) - 1;
			}
		} else if (cmd_type == PL_TYPE_TRIG) {
			trig_addr = p_cmd_buf[i+1] | ((UINT64)(p_cmd_buf[i+2] & 0xF) << 32);
			trig_addr = trig_addr + vos_cpu_get_phy_addr(pl_addr);
			// select eng ll cmd
			//unit_id = (p_cmd_buf[i] >> 4) & 0xF;
			//if (unit_job_num[unit_id] < JMISP_UNIT_MAX_JOB_NUM) {
			//	unit_job_num[unit_id]++;
			//}
			//trig_addr = vos_cpu_get_phy_addr(unit_ll_cmd_addr[unit_id][unit_job_num[unit_id]-1]);
			

			trig_addr = vos_cpu_get_phy_addr(current_unit_ll_addr[trig_cnt++]);
			printk("[dbg] trig addr = 0x%08X\n", (unsigned int)trig_addr);

			//if (current_use_base > 0) {
			//	trig_addr = trig_addr - current_use_base_addr;
			//	addr_type = current_use_base;
			//} else {
				addr_type = 0;
			//}
			p_cmd_buf[i+1] = trig_addr & 0xFFFFFFFF;
			p_cmd_buf[i+2] = (p_cmd_buf[i+2] & 0x0FFFFFF0) | (addr_type << 28) | ((trig_addr >> 32) & 0xF);
			//trig_cnt[((p_cmd_buf[i] >> 4) & 0xF)]++;
			i += 2;
		} else if (cmd_type == PL_TYPE_MOVE) {
			// bypass
		} else if (cmd_type == PL_TYPE_WAIT) {
			// bypass		
		} else {
			printk("unknown command %d!\n", cmd_type);
			break;
		}
	}
	
	// flush JL address
	vos_cpu_dcache_sync(pl_addr, pl_size, VOS_DMA_BIDIRECTIONAL);

	return pl_id;
}

int parse_pl_info(KDRV_AI_JMISP_FUNC_INFO* jmisp_info, KDRV_AI_JMISP_PATH_INFO* path_info, JMISP_BLOCK_INFO* blk_info, UINT32 path_id)
{
	UINT32 frame_slice_y_num      = 1;
	UINT32 frame_stripe_x_num     = 1;
	UINT32 slice_blk_y_num        = 1;
	UINT32 slice_blk_x_num        = 1;
	UINT32 first_stripe_blk_x_num = 1;
	UINT32 last_stripe_blk_x_num  = 1;
	UINT32 first_slice_blk_y_num  = 1;
	UINT32 last_slice_blk_y_num   = 1;
	UINT32 frame_first_stripe_ofs = 0;
	UINT32 frame_middle_stripe_ofs = 0;
	//UINT32 frame_last_stripe_ofs = 0;
	UINT32 frame_slice_y_ofs0 = 0;
	UINT32 frame_slice_y_ofs1 = 0;
	UINT32 frame_slice_blk_y_ofs0 = 0;
	UINT32 frame_slice_blk_y_ofs1 = 0;
	UINT32 frame_slice_blk_x_ofs0 = 0;
	UINT32 frame_slice_blk_x_ofs1 = 0;
	//UINT32 frame_slice_blk_x_ofs2 = 0;
	UINT32 pingpong_slice_blk_y_ofs0 = 0;
	UINT32 pingpong_slice_blk_y_ofs1 = 0;
	UINT32 pingpong_slice_blk_x_ofs0 = 0;
	UINT32 pingpong_slice_blk_x_ofs1 = 0;
	//UINT32 pingpong_slice_blk_x_ofs2 = 0;
	UINT32 frame_line_blk_num = 0;
	UINT32 frame_height_blk_num = 0;
	UINT32 unit_x_size = 0;
	UINT32 unit_y_size = 0;
	UINT32 source_lofs = 0;
	UINT32 source_height = 0;
	//UINT32 path_reg_ofs = 0x100 + 0x300*jmisp_info->path_id;
	
	frame_slice_y_num      = jmisp_info->slice_num;
	frame_stripe_x_num     = jmisp_info->stripe_num;
	slice_blk_y_num        = blk_info->blk_y_num;
	slice_blk_x_num        = blk_info->blk_x_num;
	first_stripe_blk_x_num = blk_info->blk_x_num;
	last_stripe_blk_x_num  = blk_info->blk_x_num;
	first_slice_blk_y_num  = blk_info->blk_y_num;
	last_slice_blk_y_num   = blk_info->blk_y_num;
	
	unit_x_size = jmisp_info->max_out_lofs;
	unit_y_size = jmisp_info->max_out_height;
	
	frame_first_stripe_ofs  = 0;//first_stripe_blk_x_num*unit_x_size;
	frame_middle_stripe_ofs = 0;//slice_blk_x_num*unit_x_size;
	//frame_last_stripe_ofs   = 0; //no use
	if (frame_stripe_x_num > 1) {
		frame_line_blk_num = first_stripe_blk_x_num + slice_blk_x_num*(frame_stripe_x_num-2) + last_stripe_blk_x_num;
	} else {
		frame_line_blk_num = first_stripe_blk_x_num;
	}
	if (frame_slice_y_num > 1) {
		frame_height_blk_num = first_slice_blk_y_num + slice_blk_y_num*(frame_slice_y_num-2) + last_slice_blk_y_num;
	} else {
		frame_height_blk_num = first_slice_blk_y_num;
	}
	if (frame_height_blk_num == 0 || frame_line_blk_num == 0) {
		printk("blk num shoul not be 0\n");
		return -1;
	}
	/*
	frame_slice_y_ofs0      = unit_x_size*unit_y_size;//frame_line_blk_num*unit_x_size*first_slice_blk_y_num*unit_y_size;
	frame_slice_y_ofs1      = unit_x_size*unit_y_size;//frame_line_blk_num*unit_x_size*slice_blk_y_num*unit_y_size;
	frame_slice_blk_y_ofs0  = unit_x_size*unit_y_size;//frame_line_blk_num*unit_x_size*unit_y_size;
	frame_slice_blk_y_ofs1  = unit_x_size*unit_y_size;//frame_line_blk_num*unit_x_size*unit_y_size;
	frame_slice_blk_x_ofs0  = 1*unit_x_size;
	frame_slice_blk_x_ofs1  = 1*unit_x_size;
	//frame_slice_blk_x_ofs2  = 0; //no use
	pingpong_slice_blk_y_ofs0  = unit_x_size*unit_y_size;//frame_line_blk_num*unit_x_size*unit_y_size;
	pingpong_slice_blk_y_ofs1  = unit_x_size*unit_y_size;//frame_line_blk_num*unit_x_size*unit_y_size;
	pingpong_slice_blk_x_ofs0  = 1*unit_x_size;
	pingpong_slice_blk_x_ofs1  = 1*unit_x_size;
	//pingpong_slice_blk_x_ofs2  = 0; //no use	
	source_lofs = unit_x_size;//frame_line_blk_num*unit_x_size;
	source_height = unit_y_size*frame_slice_y_num;//frame_height_blk_num*unit_y_size;
	*/
	frame_slice_y_ofs0      = unit_x_size*unit_y_size;//frame_line_blk_num*unit_x_size*first_slice_blk_y_num*unit_y_size;
	frame_slice_y_ofs1      = unit_x_size*unit_y_size;//frame_line_blk_num*unit_x_size*slice_blk_y_num*unit_y_size;
	frame_slice_blk_y_ofs0  = unit_x_size*blk_info->blk_y_first_height;//frame_line_blk_num*unit_x_size*unit_y_size;
	frame_slice_blk_y_ofs1  = unit_x_size*blk_info->blk_y_middle_height;//frame_line_blk_num*unit_x_size*unit_y_size;
	frame_slice_blk_x_ofs0  = blk_info->blk_x_first_ofs;
	frame_slice_blk_x_ofs1  = blk_info->blk_x_middle_ofs;
	//frame_slice_blk_x_ofs2  = 0; //no use
	pingpong_slice_blk_y_ofs0  = unit_x_size*blk_info->blk_y_first_height;//frame_line_blk_num*unit_x_size*unit_y_size;
	pingpong_slice_blk_y_ofs1  = unit_x_size*blk_info->blk_y_middle_height;//frame_line_blk_num*unit_x_size*unit_y_size;
	pingpong_slice_blk_x_ofs0  = blk_info->blk_x_first_ofs;
	pingpong_slice_blk_x_ofs1  = blk_info->blk_x_middle_ofs;
	//pingpong_slice_blk_x_ofs2  = 0; //no use	
	source_lofs = unit_x_size;//frame_line_blk_num*unit_x_size;
	source_height = unit_y_size*frame_slice_y_num;//frame_height_blk_num*unit_y_size;
	
	
	path_info->slice_num = frame_slice_y_num;
	path_info->stripe_num = frame_stripe_x_num;
	path_info->blk_num_info.blk_x_middle_num = slice_blk_x_num;
	path_info->blk_num_info.blk_y_middle_num = slice_blk_y_num;
	path_info->blk_num_info.blk_x_first_num = first_stripe_blk_x_num;
	path_info->blk_num_info.blk_x_last_num  = last_stripe_blk_x_num;
	path_info->blk_num_info.blk_y_first_num = first_slice_blk_y_num;
	path_info->blk_num_info.blk_y_last_num  = last_slice_blk_y_num;
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xC0, (frame_slice_y_num << 8) | (frame_stripe_x_num << 24));  
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xC8, (slice_blk_y_num) | (slice_blk_x_num << 16));
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xCC, (first_stripe_blk_x_num) | (last_stripe_blk_x_num << 16));
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x2B8, first_slice_blk_y_num | (last_slice_blk_y_num << 16));
	
	// frame buffer 0
	path_info->frame[0].first_stripe_x_ofs  = frame_first_stripe_ofs;
	path_info->frame[0].middle_stripe_x_ofs = frame_middle_stripe_ofs;
	path_info->frame[0].first_slice_y_ofs   = frame_slice_y_ofs0;
	path_info->frame[0].middle_slice_y_ofs  = frame_slice_y_ofs1;
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xD0, (frame_first_stripe_ofs | (frame_middle_stripe_ofs << 16)));  
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xD8, frame_slice_y_ofs0);  
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xDC, frame_slice_y_ofs1);  
	
	// frame buffer 1
	if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV) {
		path_info->frame[1].first_stripe_x_ofs  = frame_first_stripe_ofs/2;
		path_info->frame[1].middle_stripe_x_ofs = frame_middle_stripe_ofs/2;
		path_info->frame[1].first_slice_y_ofs   = frame_slice_y_ofs0/2;
		path_info->frame[1].middle_slice_y_ofs  = frame_slice_y_ofs1/2;
	
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xE0, ((frame_first_stripe_ofs/2) | ((frame_middle_stripe_ofs/2) << 16)));
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xE8, frame_slice_y_ofs0/2);
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xEC, frame_slice_y_ofs1/2);	
	} else {
		path_info->frame[1].first_stripe_x_ofs  = frame_first_stripe_ofs;
		path_info->frame[1].middle_stripe_x_ofs = frame_middle_stripe_ofs;
		path_info->frame[1].first_slice_y_ofs   = frame_slice_y_ofs0;
		path_info->frame[1].middle_slice_y_ofs  = frame_slice_y_ofs1;
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xE0, ((frame_first_stripe_ofs) | ((frame_middle_stripe_ofs) << 16)));
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xE8, frame_slice_y_ofs0);
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xEC, frame_slice_y_ofs1);
	}
	
	// slice buffer 0
	path_info->frame[0].blk_ofs_info.blk_y_first_ofs  = frame_slice_blk_y_ofs0;
	path_info->frame[0].blk_ofs_info.blk_y_middle_ofs = frame_slice_blk_y_ofs1;
	path_info->frame[0].blk_ofs_info.blk_x_first_ofs  = frame_slice_blk_x_ofs0;
	path_info->frame[0].blk_ofs_info.blk_x_middle_ofs = frame_slice_blk_x_ofs1;
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x130, frame_slice_blk_y_ofs0);
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x134, frame_slice_blk_y_ofs1); 
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x138, frame_slice_blk_x_ofs0 | (frame_slice_blk_x_ofs1 << 16)); 	
	
	// slice buffer 1
	if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV) {
		path_info->frame[1].blk_ofs_info.blk_y_first_ofs  = frame_slice_blk_y_ofs0/2;
		path_info->frame[1].blk_ofs_info.blk_y_middle_ofs = frame_slice_blk_y_ofs1/2;
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x140, frame_slice_blk_y_ofs0/2);
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x144, frame_slice_blk_y_ofs1/2);
	} else {
		path_info->frame[1].blk_ofs_info.blk_y_first_ofs  = frame_slice_blk_y_ofs0;
		path_info->frame[1].blk_ofs_info.blk_y_middle_ofs = frame_slice_blk_y_ofs1;
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x140, frame_slice_blk_y_ofs0);
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x144, frame_slice_blk_y_ofs1);
	}
	path_info->frame[1].blk_ofs_info.blk_x_first_ofs  = frame_slice_blk_x_ofs0;
	path_info->frame[1].blk_ofs_info.blk_x_middle_ofs = frame_slice_blk_x_ofs1;
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x148, frame_slice_blk_x_ofs0 | (frame_slice_blk_x_ofs1 << 16));
	
	// pingpong buffer 0
	path_info->pingpong[0].blk_ofs_info.blk_y_first_ofs = pingpong_slice_blk_y_ofs0;
	path_info->pingpong[0].blk_ofs_info.blk_y_middle_ofs = pingpong_slice_blk_y_ofs1;
	path_info->pingpong[0].blk_ofs_info.blk_x_first_ofs = pingpong_slice_blk_x_ofs0;
	path_info->pingpong[0].blk_ofs_info.blk_x_middle_ofs = pingpong_slice_blk_x_ofs1;
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x190, pingpong_slice_blk_y_ofs0);
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x194, pingpong_slice_blk_y_ofs1);
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x198, pingpong_slice_blk_x_ofs0 | (pingpong_slice_blk_x_ofs1 << 16));
	
	// pingpong buffer 1
	if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV) {
		path_info->pingpong[1].blk_ofs_info.blk_y_first_ofs = pingpong_slice_blk_y_ofs0/2;
		path_info->pingpong[1].blk_ofs_info.blk_y_middle_ofs = pingpong_slice_blk_y_ofs1/2;
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x1A0, pingpong_slice_blk_y_ofs0/2);
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x1A4, pingpong_slice_blk_y_ofs1/2);
	} else {
		path_info->pingpong[1].blk_ofs_info.blk_y_first_ofs = pingpong_slice_blk_y_ofs0;
		path_info->pingpong[1].blk_ofs_info.blk_y_middle_ofs = pingpong_slice_blk_y_ofs1;
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x1A0, pingpong_slice_blk_y_ofs0);
		//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x1A4, pingpong_slice_blk_y_ofs1);
	}
	path_info->pingpong[1].blk_ofs_info.blk_x_first_ofs  = pingpong_slice_blk_x_ofs0;
	path_info->pingpong[1].blk_ofs_info.blk_x_middle_ofs = pingpong_slice_blk_x_ofs1;
	//SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x1A8, pingpong_slice_blk_x_ofs0 | (pingpong_slice_blk_x_ofs1 << 16));
	
	/*
	// frame buffer 2 TODO
	if (jmisp_info->texture_enable) {
		UINT32 texture_stripe_ofs = 0;
	    if (jmisp_info->stripe_num > 1) {
	        texture_stripe_ofs = jmisp_info->texture_x_pos[1] - jmisp_info->texture_x_pos[0];
	    }
		frame_first_stripe_ofs  = texture_stripe_ofs;
		frame_middle_stripe_ofs = frame_first_stripe_ofs;
		frame_slice_y_ofs0 = jmisp_info->texture_input_width*unit_y_size/2;
		frame_slice_y_ofs1 = frame_slice_y_ofs0;
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xF0, ((frame_first_stripe_ofs) | (frame_middle_stripe_ofs << 16)));  
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xF8, frame_slice_y_ofs0);  
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0xFC, frame_slice_y_ofs1); 
	}
	
	// slice buffer 2 TODO
	if (jmisp_info->texture_enable) {
		frame_slice_blk_y_ofs0 = jmisp_info->texture_input_width*blk_info->blk_y_first_height/2;
		frame_slice_blk_y_ofs1 = jmisp_info->texture_input_width*blk_info->blk_y_middle_height/2;
		frame_slice_blk_x_ofs0 = blk_info->blk_x_first_ofs/2;
		frame_slice_blk_x_ofs1 = blk_info->blk_x_middle_ofs/2;
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x150, frame_slice_blk_y_ofs0);
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x154, frame_slice_blk_y_ofs1); 
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x158, frame_slice_blk_x_ofs0 | (frame_slice_blk_x_ofs1 << 16)); 
	}
	
	// pingpong buffer 2 TODO
	if (jmisp_info->texture_enable) {
		pingpong_slice_blk_y_ofs0 = jmisp_info->texture_slice_out_lofs*blk_info->blk_y_first_height/2;
		pingpong_slice_blk_y_ofs1 = jmisp_info->texture_slice_out_lofs*blk_info->blk_y_middle_height/2;
		pingpong_slice_blk_x_ofs0 = jmisp_info->texture_slice_out_lofs;//blk_info->blk_x_first_ofs/2;
		pingpong_slice_blk_x_ofs1 = jmisp_info->texture_slice_out_lofs;;//blk_info->blk_x_middle_ofs/2;
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x1B0, pingpong_slice_blk_y_ofs0);
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x1B4, pingpong_slice_blk_y_ofs1);
		SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x1B8, pingpong_slice_blk_x_ofs0 | (pingpong_slice_blk_x_ofs1 << 16));
	}
	
	SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x2E4, source_lofs);
	SET_32BitsValue(JMISP_REG_BASE_ADDR + path_reg_ofs + 0x2E8, source_height);
	*/
	
	// set address
	path_info->frame[0].frame_addr = jmisp_info->frame_y_addr;
	if (jmisp_info->in_ring_y_start_addr > 0) {
		path_info->frame[0].ring_en = 1;
		path_info->frame[0].ring_start_addr = jmisp_info->in_ring_y_start_addr;
		path_info->frame[0].ring_end_addr   = jmisp_info->in_ring_y_end_addr;
	} else {
		path_info->frame[0].ring_en = 0;
	}
	
	if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV) {
		path_info->frame[1].frame_addr = jmisp_info->frame_uv_addr;
		if (jmisp_info->in_ring_uv_start_addr > 0) {
			path_info->frame[1].ring_en = 1;
			path_info->frame[1].ring_start_addr = jmisp_info->in_ring_uv_start_addr;
			path_info->frame[1].ring_end_addr   = jmisp_info->in_ring_uv_end_addr;
		} else {
			path_info->frame[1].ring_en = 0;
		}
	} else {
		path_info->frame[1].frame_addr = 0;
		path_info->frame[1].ring_en = 0;
	}
	
	path_info->pingpong[0].pingpong_addr0 = jmisp_info->out_pingpong_y_addr;
	path_info->pingpong[0].pingpong_addr1 = jmisp_info->out_pingpong_y_addr + (UINT64)unit_x_size*unit_y_size;
	
	if (jmisp_info->img_fmt == KDRV_AI_NNISP_FMT_YUV) {
		path_info->pingpong[1].pingpong_addr0 = jmisp_info->out_pingpong_uv_addr;
		path_info->pingpong[1].pingpong_addr1 = jmisp_info->out_pingpong_uv_addr + (UINT64)unit_x_size*unit_y_size/2;
	} else {
		path_info->pingpong[1].pingpong_addr0 = 0;
		path_info->pingpong[1].pingpong_addr1 = 0;
	}
	
	// handshake
	//path_info->handshake = jmisp_info->handshake;
	if (jmisp_info->isp_id == 2) {
		path_info->handshake = 0;
	} else {
		path_info->handshake = 1 + (jmisp_info->isp_id*4) + jmisp_info->path_id;
	}
	
	// signal mode
	path_info->signal_mode_en = jmisp_info->signal_mode_en;
	
	// path id
	path_info->path_id = path_id;
	
	return 0;
}

INT32 set_jmisp_parm(VOID* param, uintptr_t workbuf_va, uintptr_t workbuf_pa)
{
	//int i = 0;
	int path_idx = 0;
	uintptr_t usable_addr = 0;
	char file_path[128];	
	//UINT32 file_size = 0;
	UINT32 pl_id = 0;
	uintptr_t pl_cmd_addr = 0;
	UINT32 pl_cmd_size = 0;
	uintptr_t unit_ll_addr = 0;
	UINT32 unit_ll_size = 0;
	uintptr_t texture_ll_addr = 0;
	UINT32 texture_ll_size = 0;
	UINT32 pl_en = 0;
	uintptr_t temp_buf_addr = 0;
	UINT32 pat_count = 20002;
	JMISP_BLOCK_INFO blk_info = {0};
	uintptr_t pl_pa = 0;
	KDRV_AI_JMISP_JOB_INFO* job_info;
	
	usable_addr = workbuf_va;
	
	if (param == NULL) {
		return -1;
	}
	job_info = (KDRV_AI_JMISP_JOB_INFO*)param;
	
	// transfer job info to path info
	for (path_idx = 0; path_idx < job_info->func_num; path_idx++) {
		pl_id = job_info->func_info[path_idx].path_id;
		jmisp_set_blk_info(&job_info->func_info[path_idx], &blk_info);

		pl_en = 1 << pl_id;
		
		// 1. load PL
		pl_cmd_addr = usable_addr;
		if (job_info->func_info[path_idx].frame_texture_addr) {
			snprintf(file_path, 64, "//mnt//sd//Pattern//JMISP//jmispl%d//jmispl%d_tex.bin", pat_count, pat_count);
		} else {
			snprintf(file_path, 64, "//mnt//sd//Pattern//JMISP//jmispl%d//jmispl%d_0.bin", pat_count, pat_count);
		}
		
		pl_cmd_size = load_data(file_path, pl_cmd_addr);
		if (pl_cmd_size == 0) {
			return -1;
		}
		usable_addr += pl_cmd_size;
		
		
		// 2. load PPU LL 
		if (job_info->func_info[path_idx].img_fmt == KDRV_AI_NNISP_FMT_Y8) {
			snprintf(file_path, 64, "//mnt//sd//Pattern//JMISP//jmispl%d//ll_cmd_ppu_y.bin", pat_count);
		} else if (job_info->func_info[path_idx].img_fmt == KDRV_AI_NNISP_FMT_YUV) {
			snprintf(file_path, 64, "//mnt//sd//Pattern//JMISP//jmispl%d//ll_cmd_ppu_yuv.bin", pat_count);
		} else if (job_info->func_info[path_idx].img_fmt == KDRV_AI_NNISP_FMT_BAYER) {
			snprintf(file_path, 64, "//mnt//sd//Pattern//JMISP//jmispl%d//ll_cmd_ppu_bayer12b.bin", pat_count);
		}
		unit_ll_addr = usable_addr;
		unit_ll_size = load_data(file_path, unit_ll_addr);
		if (unit_ll_size == 0) {
			return -1;
		}
		usable_addr += unit_ll_size;
			
		if (job_info->func_info[path_idx].frame_texture_addr) {
			// load texture ll cmd
			snprintf(file_path, 64, "//mnt//sd//Pattern//JMISP//jmispl%d//ll_cmd_ppu_texture.bin", pat_count);
			texture_ll_addr = usable_addr;
			texture_ll_size = load_data(file_path, texture_ll_addr);
			if (texture_ll_size == 0) {
				return -1;
			}
			usable_addr += texture_ll_size;
			//printk("[tex] tex ll cmd addr = 0x%08x\n", (UINT32)texture_ll_addr);
		}
		
		temp_buf_addr = usable_addr;
		
		// 3. parse PPU LL cmd
		if (unit_ll_size > 0) {
			UINT32 x_ofs = 0;
			UINT32 y_height = 0;
			UINT32 i = 0;
			
			unit_ll_size = unit_ll_size / 9;
			for (i = 0; i < 9; i++) {
				if (i == 0) {
					x_ofs    = blk_info.blk_x_first_ofs; 
					y_height = blk_info.blk_y_first_height;
				} else if (i == 1) {
					x_ofs    = blk_info.blk_x_middle_ofs; 
					y_height = blk_info.blk_y_first_height;
				} else if (i == 2) {
					x_ofs    = blk_info.blk_x_last_ofs; 
					y_height = blk_info.blk_y_first_height;
				} else if (i == 3) {
					x_ofs    = blk_info.blk_x_first_ofs; 
					y_height = blk_info.blk_y_middle_height;
				} else if (i == 4) {
					x_ofs    = blk_info.blk_x_middle_ofs; 
					y_height = blk_info.blk_y_middle_height;
				} else if (i == 5) {
					x_ofs    = blk_info.blk_x_last_ofs; 
					y_height = blk_info.blk_y_middle_height;
				} else if (i == 6) {
					x_ofs    = blk_info.blk_x_first_ofs; 
					y_height = blk_info.blk_y_last_height;
				} else if (i == 7) {
					x_ofs    = blk_info.blk_x_middle_ofs; 
					y_height = blk_info.blk_y_last_height;
				} else if (i == 8) {
					x_ofs    = blk_info.blk_x_last_ofs; 
					y_height = blk_info.blk_y_last_height;
				}
				//printk("case %d: x ofs = %d, y height = %d\n", i, x_ofs, y_height);
				parse_ppu_ll_cmd3(unit_ll_addr+unit_ll_size*i, unit_ll_size, temp_buf_addr, &job_info->func_info[path_idx], x_ofs, y_height);
				vos_cpu_dcache_sync(unit_ll_addr+unit_ll_size*i, unit_ll_size, VOS_DMA_BIDIRECTIONAL);
			}
		}
		if (job_info->func_info[path_idx].frame_texture_addr) {
			/*UINT32 x_ofs = 0;
			UINT32 y_height = 0;
			UINT32 i = 0;
			
			texture_ll_size = texture_ll_size / 9;
			for (i = 0; i < 9; i++) {
				if (i == 0) {
					x_ofs    = blk_info.blk_x_first_ofs/2; 
					y_height = blk_info.blk_y_first_height/2;
				} else if (i == 1) {
					x_ofs    = blk_info.blk_x_middle_ofs/2; 
					y_height = blk_info.blk_y_first_height/2;
				} else if (i == 2) {
					x_ofs    = blk_info.blk_x_last_ofs/2; 
					y_height = blk_info.blk_y_first_height/2;
				} else if (i == 3) {
					x_ofs    = blk_info.blk_x_first_ofs/2; 
					y_height = blk_info.blk_y_middle_height/2;
				} else if (i == 4) {
					x_ofs    = blk_info.blk_x_middle_ofs/2; 
					y_height = blk_info.blk_y_middle_height/2;
				} else if (i == 5) {
					x_ofs    = blk_info.blk_x_last_ofs/2; 
					y_height = blk_info.blk_y_middle_height/2;
				} else if (i == 6) {
					x_ofs    = blk_info.blk_x_first_ofs/2; 
					y_height = blk_info.blk_y_last_height/2;
				} else if (i == 7) {
					x_ofs    = blk_info.blk_x_middle_ofs/2; 
					y_height = blk_info.blk_y_last_height/2;
				} else if (i == 8) {
					x_ofs    = blk_info.blk_x_last_ofs/2; 
					y_height = blk_info.blk_y_last_height/2;
				}
				x_ofs = job_info->func_info[path_idx].texture_slice_out_lofs;
				//printk("[texture] case %d: x ofs = %d, y height = %d, total width = %d, slice out lofs = %d\n", i, x_ofs, y_height, 
				//			jmisp_info->texture_input_width, jmisp_info->texture_slice_out_lofs);
				parse_ppu_ll_cmd_texture(texture_ll_addr+texture_ll_size*i, texture_ll_size, x_ofs, y_height, 
										 job_info->func_info[path_idx].texture_input_width, job_info->func_info[path_idx].texture_slice_out_lofs);	
				vos_cpu_dcache_sync(texture_ll_addr+texture_ll_size*i, texture_ll_size, VOS_DMA_BIDIRECTIONAL);
			}*/
		}
		
		// 4. set PL setting reg
		parse_pl_info(&job_info->func_info[path_idx], &g_path_info, &blk_info, path_idx);
		
		// 5. clear temp buffer
		memset((VOID*)temp_buf_addr, 0, job_info->func_info[path_idx].max_out_lofs*job_info->func_info[path_idx].max_out_height*3);
		vos_cpu_dcache_sync(temp_buf_addr, job_info->func_info[path_idx].max_out_lofs*job_info->func_info[path_idx].max_out_height*3, VOS_DMA_BIDIRECTIONAL);
		usable_addr += (job_info->func_info[path_idx].max_out_lofs*job_info->func_info[path_idx].max_out_height*3);
		
		// 6. paring PL
		if (pl_cmd_size > 0) {
			UINT32 i = 0;
			uintptr_t tmp_ll_addr[18] = {0};
			for (i = 0; i < 9; i++) {
				tmp_ll_addr[i] = unit_ll_addr + unit_ll_size*i;
			}
			for (i = 0; i < 9; i++) {
				tmp_ll_addr[9+i] = texture_ll_addr + texture_ll_size*i;
			}
			pl_parse_cmd2(pl_cmd_addr, pl_cmd_size, tmp_ll_addr);
		}
		
		// 7. set parameter to kdrv ai
		kdrv_ai_set(KDRV_AI_PARAM_JMISP_PARAM, &g_path_info);
		
		// 8. trigger JMISP engine
		pl_pa = pl_cmd_addr - workbuf_va + workbuf_pa;
		kdrv_ai_trigger(KDRV_AI_ENG_JMISP_PLA + path_idx, pl_pa);
	}
	
	return 0;
}
#endif
#if KDRV_AI_ECHO_TEST
#include "kwrap/cpu.h"
INT32 test_rou_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_rou_cb occur\n");
	return 0;
}

INT32 test_nue2_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_nue2_cb occur\n");
	return 0;
}

INT32 test_conv_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_conv_cb occur\n");
	return 0;
}

INT32 test_jobm_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_jobm_cb occur\n");
	return 0;
}

INT32 test_lsu_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_lsu_cb occur\n");
	return 0;
}

INT32 test_util_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_util_cb occur\n");
	return 0;
}

INT32 test_cal_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_cal_cb occur\n");
	return 0;
}

INT32 test_ppu_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_ppu_cb occur\n");
	return 0;
}

INT32 test_jmisp_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_jmisp_cb occur\n");
	return 0;
}

INT32 test_jm_single(UINT32 step) 
{
	uintptr_t jl_pa = 0, jl_va = 0;
	static KDRV_AI_JOBM_DBG_PARAM jm_dbg = {0};
	
	if (step == 0) {
		// test function (JOBM)
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		uintptr_t conv_ll_pa = 0;
		uintptr_t rou_ll_pa = 0;
		
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		kdrv_ai_init();
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// prepare CONV part
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		conv_ll_pa = ll_pa;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_conv.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//CONV//convg1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		
		printk("[kdrv_ai] start update cmd\n");
		
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x120) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		// prepare ROU part
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		rou_ll_pa = ll_pa;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_rou.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ROU//roug10000//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		// prepare JOBM part
		jl_pa = usable_pa;
		jl_va = usable_va;
		cur_size = 0;
		// set cmd
		{
			UINT32* p_jl = (UINT32*)jl_va;
			UINT32 jl_idx = 0;
			// trigger CONV
			p_jl[jl_idx++] = 4 | (0<<4) | (1<<8) | (1<<16);
			p_jl[jl_idx++] = conv_ll_pa;
			p_jl[jl_idx++] = 1 << 8;
			// trigger ROU
			p_jl[jl_idx++] = 4 | (3<<4) | (2<<8) | (1<<16);
			p_jl[jl_idx++] = rou_ll_pa;
			p_jl[jl_idx++] = 2 << 8;
			// trigger CONV
			p_jl[jl_idx++] = 4 | (0<<4) | (1<<8) | (1<<16);
			p_jl[jl_idx++] = conv_ll_pa;
			p_jl[jl_idx++] = 3 << 8;
			// trigger ROU
			p_jl[jl_idx++] = 4 | (3<<4) | (2<<8) | (1<<16);
			p_jl[jl_idx++] = rou_ll_pa;
			p_jl[jl_idx++] = 4 << 8;
			// set NULL
			p_jl[jl_idx++] = 0;
			cur_size = jl_idx*4;
			vos_cpu_dcache_sync(jl_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		}
		
		// set debug
		jm_dbg.dbg_mode = KDRV_AI_JM_DBG_MODE_SINGLE;
		jm_dbg.single_jl_id = KDRV_AI_JL_E;
		kdrv_ai_set(KDRV_AI_PARAM_JOBM_DBG, &jm_dbg);
		
		// set job end callback
		{
			KDRV_AI_CBFUNC_PARAM cb_parm;
			KDRV_AI_ISRCB test_cb_func = &test_jobm_cb;
			
			cb_parm.eng = KDRV_AI_ENG_JOBM_JLE_DBG;
			cb_parm.isrcb_fp = test_cb_func;
			kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		}
				
		// run JOBM
		kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLE_DBG, jl_pa);
		printk("JOBM 1st process done\n");
	} else {
		kdrv_ai_jm_dbg_run_next(jm_dbg);
		printk("JOBM next process done\n");
	}
	
	return 0;
}

INT32 test_kdrv_ai_func(VOID)
{
	uintptr_t jl_pa = 0, jl_va = 0;
	
	kdrv_ai_init();
	kdrv_ai_uninit();
	kdrv_ai_init();
	/*{
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_jobm_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLA;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLB;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLC;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLD;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLE;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLA;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLB;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLC;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLD;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLE;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}*/
	/*
	{
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_rou_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		
		printk("test set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_ROU;
		base_addr_parm.pa = (uintptr_t)0x5A5A5A5A | ((uintptr_t)0x2 << 32);
		base_addr_parm.base_id = 0;
		kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_ROU;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_nue2_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		
		printk("test set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_NUE2;
		base_addr_parm.pa = (uintptr_t)0x5A5A5A5A | ((uintptr_t)0x2 << 32);
		base_addr_parm.base_id = 0;
		kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_NUE2;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	
	{
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_conv_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test set base addr\n");
		for (i = 0; i < 5; i++) {
			base_addr_parm.eng = KDRV_AI_ENG_CONV1;
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A+i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		for (i = 0; i < 5; i++) {
			base_addr_parm.eng = KDRV_AI_ENG_CONV2;
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A+i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		for (i = 0; i < 5; i++) {
			base_addr_parm.eng = KDRV_AI_ENG_CONV3;
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A+i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		for (i = 0; i < 5; i++) {
			base_addr_parm.eng = KDRV_AI_ENG_CONV4;
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A+i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		
		printk("test set base addr done\n");
		
		// test set cb
		for (i = 0; i < 4; i++) {
			cb_parm.eng = KDRV_AI_ENG_CONV1+i;
			cb_parm.isrcb_fp = test_cb_func;
			kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		}	
		
	}*/
	
	{
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_lsu_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test lsu set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_LSU;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_LSU;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_util_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test util set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_UTIL;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_UTIL;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_cal_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test cal set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_CAL;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_CAL;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_ppu_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test ppu set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_PPU;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_PPU;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	if (0) {
		// test function 
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_rou.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ROU//roug10000//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_ROU, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_ROU)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_ROU, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("ROU process done\n");
		
	}
	
	if (0) {
		// test function (NUE2)
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_nue2.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//NUE2//DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x08) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x18) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_NUE2, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_NUE2)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_NUE2, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("NUE2 process done\n");
		
	}
	
	if (0) {
		// test function (CONV)
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		int i = 0;
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_conv.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//CONV//convg1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		for (i = 0; i < 4; i++) {
			printk("[kdrv_ai] start update cmd\n");
			
			// step 3. update cmd
			{
				UINT64 *p_ll_cmd = (UINT64*)ll_va;
				UINT32 ll_idx = 0;
				while (1) {
					if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
						break;
					} 
					
					if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
						// update input address
						p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
					}
					
					if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x120) {
						// update output address
						p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
					}
					ll_idx++;
				
				}
				
				vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
			}
			printk("[kdrv_ai] end update cmd\n");
			
			kdrv_ai_trigger(KDRV_AI_ENG_CONV1+i, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_CONV1+i)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_CONV1+i, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			dma_out_pa += cur_size;
			printk("CONV%d process done\n", i+1);
		}
	}
	
	if (1) {
		// test function (JOBM)
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		uintptr_t conv_ll_pa = 0;
		uintptr_t rou_ll_pa = 0;
		
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// prepare CONV part
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		conv_ll_pa = ll_pa;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_conv.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//CONV//convg1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		
		printk("[kdrv_ai] start update cmd\n");
		
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x120) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		// prepare ROU part
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		rou_ll_pa = ll_pa;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_rou.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ROU//roug10000//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		// prepare JOBM part
		jl_pa = usable_pa;
		jl_va = usable_va;
		cur_size = 0;
		// set cmd
		{
			UINT32* p_jl = (UINT32*)jl_va;
			UINT32 jl_idx = 0;
			// trigger CONV
			p_jl[jl_idx++] = 4 | (0<<4) | (1<<8) | (1<<16);
			p_jl[jl_idx++] = conv_ll_pa;
			p_jl[jl_idx++] = 1 << 8;
			// trigger ROU
			p_jl[jl_idx++] = 4 | (3<<4) | (2<<8) | (1<<16);
			p_jl[jl_idx++] = rou_ll_pa;
			p_jl[jl_idx++] = 2 << 8;
			// trigger CONV
			p_jl[jl_idx++] = 4 | (0<<4) | (1<<8) | (1<<16);
			p_jl[jl_idx++] = conv_ll_pa;
			p_jl[jl_idx++] = 3 << 8;
			// trigger ROU
			p_jl[jl_idx++] = 4 | (3<<4) | (2<<8) | (1<<16);
			p_jl[jl_idx++] = rou_ll_pa;
			p_jl[jl_idx++] = 4 << 8;
			// set NULL
			p_jl[jl_idx++] = 0;
			cur_size = jl_idx*4;
			vos_cpu_dcache_sync(jl_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		}
		
		// test busy count
		{
			KDRV_AI_JOBM_BUSY_CNT_INFO cnt_info = {0};
			kdrv_ai_set(KDRV_AI_PARAM_JOBM_BUSY_CNT, &cnt_info);
		}
		
		// run JOBM
		/*kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, jl_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_JOBM_JLA)) {
			printk("reset fail\n");
		}*/
		kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, jl_pa);
		printk("JOBM process done\n");
		
		// test busy count
		{
			KDRV_AI_JOBM_BUSY_CNT_INFO cnt_info = {0};
			int i = 0;
			kdrv_ai_get(KDRV_AI_PARAM_JOBM_BUSY_CNT, &cnt_info);
			for (i = 0; i < 8; i++) printk("cnt = %d\n", cnt_info.busy_cnt[i]);
		}
	}
	
	if (0) {
		// test function (LSU)
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_lsu.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//LSU//lsug1//DI0_1.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x58) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_LSU, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_LSU)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_LSU, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("LSU process done\n");
		
	}
	
	// test function (CAL)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_cal.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//CAL//calg1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x88) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_CAL, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_CAL)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_CAL, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("CAL process done\n");
		
	}
	
	// test function (UTIL)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_util.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//UTIL//utilg1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x90) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_UTIL, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_UTIL)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_UTIL, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("UTIL process done\n");
		
	}
	
	// test function (PPU)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_ppu_kdrv.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//PPU//ppu_1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_PPU, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_PPU)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_PPU, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("PPU process done\n");
		
	}
	
	kdrv_ai_uninit();
	return jl_pa;
}



UINT32 rand(VOID) 
{
	UINT32 output = 0;
	get_random_bytes(&output, sizeof(UINT32));
	
	return output;
}

int test_jmisp_func(VOID)
{
	KDRV_AI_JMISP_JOB_INFO job_info = {0};
	uintptr_t buf_va = 0;
	uintptr_t buf_pa = 0;
	UINT32 i = 0;
	struct vos_mem_info_t buf_info = {0};
	int buf_size = 10*1024*1024;
	VOS_MEM_HDL vos_mem_id = 0;
	uintptr_t buffer_pa = 0;
	uintptr_t buffer_va = 0;
	uintptr_t usable_pa = 0;
	uintptr_t usable_va = 0;
	uintptr_t frame_y_pa = 0, frame_y_va = 0;
	uintptr_t frame_uv_pa = 0, frame_uv_va = 0;
	uintptr_t pp_y_pa = 0, pp_y_va = 0;
	uintptr_t pp_uv_pa = 0, pp_uv_va = 0;
	UINT32 frame_y_size = 0;
	UINT32 frame_uv_size = 0;
	UINT32 max_slice_lofs = 0;
	UINT32 max_slice_height = 0;
	UINT32 pat_count = 0;
	UINT32 test_num = 1;
	UINT8* p_tmp_buf = NULL;

	kdrv_ai_init();
	{
		// test set base addr
		KDRV_AI_ISRCB test_cb_func = &test_jmisp_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
	
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_JMISP_PLA;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JMISP_PLB;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JMISP_PLC;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JMISP_PLD;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	printk("[kdrv_ai] start alloc buffer\n");
	if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
		printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
		return -1;
	} else {
		vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
		if (0 == vos_mem_id) {
			nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
			return -1;
		}
	}
	printk("[kdrv_ai]   end alloc buffer\n");
	buffer_pa = (uintptr_t)buf_info.paddr;
	buffer_va = (uintptr_t)buf_info.vaddr;
	
	usable_pa = buffer_pa;
	usable_va = buffer_va;
	
	job_info.func_num = test_num;
	for (pat_count = 0; pat_count < test_num; pat_count++) {
		// init parameter/input & output buffer
		job_info.func_info[pat_count].img_fmt = KDRV_AI_NNISP_FMT_YUV;
		
		job_info.func_info[pat_count].max_out_lofs   = (4+(rand()%256)) & 0xFFFFFFFC;
		job_info.func_info[pat_count].max_out_height = (4+(rand()%48))  & 0xFFFFFFFC;
		
		//job_info.func_info[pat_count].handshake = KDRV_AI_JMISP_HANDSHAKE_CPU;
		job_info.func_info[pat_count].stripe_num = 1 + rand()%3;
		job_info.func_info[pat_count].slice_num  = 3 + rand()%5;
		job_info.func_info[pat_count].signal_mode_en = 0;
		job_info.func_info[pat_count].path_id = pat_count;
		job_info.func_info[pat_count].isp_id  = 2; // use 2 to stand for CPU handshake
		
		printk("[JMISP] cur input format = %d, #stripe = %d, #slice = %d, lofs = %d, height = %d\n", 
				job_info.func_info[pat_count].img_fmt, 
				job_info.func_info[pat_count].stripe_num, 
				job_info.func_info[pat_count].slice_num, 
				job_info.func_info[pat_count].max_out_lofs, 
				job_info.func_info[pat_count].max_out_height);
		
		max_slice_lofs   = job_info.func_info[pat_count].max_out_lofs;
		max_slice_height = job_info.func_info[pat_count].max_out_height;
		
		p_tmp_buf = (UINT8*)usable_va;
		frame_y_size = max_slice_lofs*max_slice_height;
		for (i = 0; i < frame_y_size; i++) {
			p_tmp_buf[i] = rand() & 0xFF;
		}
		frame_y_pa = usable_pa;
		frame_y_va = usable_va;
		usable_pa += frame_y_size;
		usable_va += frame_y_size;
		vos_cpu_dcache_sync(frame_y_va, frame_y_size, VOS_DMA_BIDIRECTIONAL);
		
		if (job_info.func_info[pat_count].img_fmt == KDRV_AI_NNISP_FMT_YUV) {
			p_tmp_buf = (UINT8*)usable_va;
			frame_uv_size = max_slice_lofs*max_slice_height/2;
			for (i = 0; i < frame_uv_size; i++) {
				p_tmp_buf[i] = rand() & 0xFF;
			}
			frame_uv_pa = usable_pa;
			frame_uv_va = usable_va;
			usable_pa += frame_uv_size;
			usable_va += frame_uv_size;
			vos_cpu_dcache_sync(frame_uv_va, frame_uv_size, VOS_DMA_BIDIRECTIONAL);
		} else {
			frame_uv_pa = 0;
			frame_uv_size = 0;
		}
		
		pp_y_pa = usable_pa;
		pp_y_va = usable_va;
		usable_pa += (frame_y_size*2);
		usable_va += (frame_y_size*2);
		
		if (job_info.func_info[pat_count].img_fmt == KDRV_AI_NNISP_FMT_YUV) {
			pp_uv_pa = usable_pa;
			pp_uv_va = usable_va;
			usable_pa += (frame_uv_size*2);
			usable_va += (frame_uv_size*2);
		} else {
			pp_uv_pa = 0;
			pp_uv_va = 0;
		}
		
		job_info.func_info[pat_count].frame_y_addr = frame_y_pa;
		job_info.func_info[pat_count].frame_uv_addr = frame_uv_pa;
		job_info.func_info[pat_count].frame_texture_addr = 0;
		job_info.func_info[pat_count].out_pingpong_y_addr = pp_y_pa;
		job_info.func_info[pat_count].out_pingpong_uv_addr = pp_uv_pa;
		job_info.func_info[pat_count].in_ring_y_start_addr = frame_y_pa;
		job_info.func_info[pat_count].in_ring_y_end_addr = frame_y_pa + frame_y_size;
		job_info.func_info[pat_count].in_ring_uv_start_addr = frame_uv_pa;
		job_info.func_info[pat_count].in_ring_uv_end_addr = frame_uv_pa + frame_uv_size;
		printk("frame y pa = 0x%08X, pp_y_pa = 0x%08X\n", (unsigned int)frame_y_pa, (unsigned int)pp_y_pa);
	}
	
	
	buf_va = usable_va;
	buf_pa = usable_pa;
	// set to jmisp
	set_jmisp_parm(&job_info, buf_va, buf_pa);
	
	for (pat_count = 0; pat_count < test_num; pat_count++) {
		// run by cpu handshake & verify result	
		UINT32 tmp_slice_index = 0;
		uintptr_t out_pp0_va = 0;
		uintptr_t out_pp1_va = 0;
		UINT32 pp_idx = 0;
		UINT8* p_golden;
		UINT8* p_output;
		INT32 fail_flg = 0;
		
		frame_y_size = job_info.func_info[pat_count].max_out_lofs*job_info.func_info[pat_count].max_out_height;
		frame_uv_size = frame_y_size/2;
		
		while (1) {
			printk("[JMISP] slice %d start\n", tmp_slice_index);
			
			//slice start & wait slice done
			kdrv_ai_set(KDRV_AI_PARAM_JMISP_DBG_RUN_SLICE, &job_info.func_info[pat_count].path_id);
			
			// check result
			pp_idx = (tmp_slice_index % job_info.func_info[pat_count].slice_num) & 0x1;
			
			if (pp_idx == 0) {
				out_pp0_va = job_info.func_info[pat_count].out_pingpong_y_addr - buffer_pa + buffer_va;
				if (job_info.func_info[pat_count].img_fmt == KDRV_AI_NNISP_FMT_YUV) {
					out_pp1_va = job_info.func_info[pat_count].out_pingpong_uv_addr - buffer_pa + buffer_va;
				}
			} else {
				//out_pp0_va = job_info.func_info[pat_count].out_pingpong_y_addr - buffer_pa + buffer_va;
				out_pp0_va += frame_y_size;
				if (job_info.func_info[pat_count].img_fmt == KDRV_AI_NNISP_FMT_YUV) {
					out_pp1_va += frame_uv_size;
				}
			}
			
			vos_cpu_dcache_sync(out_pp0_va, frame_y_size, VOS_DMA_BIDIRECTIONAL);
			p_golden = (UINT8*)(job_info.func_info[pat_count].frame_y_addr - buffer_pa + buffer_va);
			p_output = (UINT8*)out_pp0_va;
			for (i = 0; i < frame_y_size; i++) {
				if (p_golden[i] != p_output[i]) {
					printk("pp0 cmp fail in pos %d, golden=%d, out=%d\n", i, p_golden[i], p_output[i]);
					fail_flg = 1;
					break;
				}
			}
			memset((VOID*)out_pp0_va, 0, frame_y_size);
			vos_cpu_dcache_sync(out_pp0_va, frame_y_size, VOS_DMA_BIDIRECTIONAL);
			
			if (job_info.func_info[pat_count].img_fmt == KDRV_AI_NNISP_FMT_YUV) {
				vos_cpu_dcache_sync(out_pp1_va, frame_uv_size, VOS_DMA_BIDIRECTIONAL);
				p_golden = (UINT8*)(job_info.func_info[pat_count].frame_uv_addr - buffer_pa + buffer_va);
				p_output = (UINT8*)out_pp1_va;
				for (i = 0; i < frame_uv_size; i++) {
					if (p_golden[i] != p_output[i]) {
						printk("pp1 cmp fail in pos %d\n", i);
						fail_flg = 1;
						break;
					}
				}
				memset((VOID*)out_pp1_va, 0, frame_uv_size);
				vos_cpu_dcache_sync(out_pp1_va, frame_uv_size, VOS_DMA_BIDIRECTIONAL);
			}
			
			printk("[JMISP] slice %d cmp done\n", tmp_slice_index);
			tmp_slice_index++;
			if (tmp_slice_index == job_info.func_info[pat_count].stripe_num*job_info.func_info[pat_count].slice_num) {
				printk("[JMISP] frame done, status = %d\n", fail_flg);
				
				break;
			}
		}
	}
	kdrv_ai_uninit();
	return 0;
}

#endif
int nvt_ai_api_read_version(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{

	CHAR* kdrv_ai_version;
	kdrv_ai_version = KDRV_AI_IMPL_VERSION;
	DBG_DUMP("kdrv_ai_version:%s\n", kdrv_ai_version);
#if KDRV_AI_ECHO_TEST	
	kdrv_ai_init();
	//test_kdrv_ai_func();
	if (0) {
		static int jm_single_step = 0;
		test_jm_single(jm_single_step++); 
	}
	if (0) {
	test_kdrv_ai_func();
	test_jmisp_func();
	}
	if (0) {
		static int jm_dbg_cnt = 0;
		static uintptr_t jm_jl_pa = 0;
		
		if (jm_dbg_cnt == 0) {
			jm_jl_pa = test_kdrv_ai_func();
		} else {
			if (jm_dbg_cnt < 6) {
				// set debug
				KDRV_AI_JOBM_DBG_PARAM jm_dbg = {0};
				jm_dbg.dbg_mode = KDRV_AI_JM_DBG_MODE_SINGLE;
				jm_dbg.single_jl_id = KDRV_AI_JL_A;
				if (jm_dbg_cnt == 1) {
					kdrv_ai_set(KDRV_AI_PARAM_JOBM_DBG, &jm_dbg);
					kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, jm_jl_pa);
				} else {
					kdrv_ai_jm_dbg_run_next(jm_dbg);
				}
				
			} else {
				// set debug
				KDRV_AI_JOBM_DBG_PARAM jm_dbg = {0};
				jm_dbg.dbg_mode = KDRV_AI_JM_DBG_MODE_ARB;
				jm_dbg.arb_eng = KDRV_AI_JM_DBG_ENG_CONV0 | KDRV_AI_JM_DBG_ENG_ROU;
				if (jm_dbg_cnt == 6) {
					kdrv_ai_set(KDRV_AI_PARAM_JOBM_DBG, &jm_dbg);
					kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, jm_jl_pa);
				} else {
					kdrv_ai_jm_dbg_run_next(jm_dbg);
				}
			}
		}
		jm_dbg_cnt++;
	}
#endif	
	return 0;
}

#if !defined(CONFIG_NVT_SMALL_HDAL)
int nvt_ai_api_write_reg(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	unsigned long reg_addr = 0, reg_value = 0;

	if (argc != 2) {
		nvt_dbg(ERR, "wrong argument:%d", argc);
		return -1;
	}
	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}

#if defined(__FREERTOS)
	if ((reg_addr = strtoul(pargv[0], NULL, 16)) == 0) {
		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
		return -1;
	}
	if ((reg_value = strtoul(pargv[1], NULL, 16)) == 0) {
		nvt_dbg(ERR, "invalid reg value:%s\n", pargv[1]);
		return -1;

	}
#else
	if (kstrtoul(pargv[0], 0, &reg_addr)) {
		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
		return -1;
	}
	if (kstrtoul(pargv[1], 0, &reg_value)) {
		nvt_dbg(ERR, "invalid reg value:%s\n", pargv[1]);
		return -1;

	}
#endif

	nvt_dbg(IND, "W REG 0x%lx to 0x%lx\n", reg_value, reg_addr);

	nvt_ai_drv_write_reg(pmodule_info, reg_addr, reg_value);
	return 0;
}
#endif

int nvt_ai_api_write_pattern(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	VOS_FILE fd;
	int len = 0;
	//unsigned char *pbuffer;
	struct vos_mem_cma_info_t buf_info = {0};
	int ret = 0;
	VOS_MEM_CMA_HDL buf_info_id;

	if (argc != 1) {
		nvt_dbg(ERR, "wrong argument:%d", argc);
		return -1;
	}

	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}
	
	fd = vos_file_open(pargv[0], O_RDONLY, 0);
	if ((VOS_FILE)-1 == fd) {
		nvt_dbg(ERR, "failed in file open:%s\r\n", pargv[0]);
		return -1;
	}

	//Allocate memory
	if (0 != vos_mem_init_cma_info(&buf_info, VOS_MEM_CMA_TYPE_CACHE, 0x600000)) {
        nvt_dbg(ERR, "vos_mem_init_cma_info: init buffer fail. \r\n");
		vos_file_close(fd);
        return -1;
    } else {
        buf_info_id = vos_mem_alloc_from_cma(&buf_info);
		if (NULL == buf_info_id) {
            DBG_ERR("get buffer fail\n");
			nvt_dbg(ERR, "get buffer fail\n");
			vos_file_close(fd);
            return -1;
        }
    }

	len = vos_file_read(fd, (void *)buf_info.vaddr, 1152 * 64);
	/* Do something after get data from file */
	ret = vos_mem_release_from_cma(buf_info_id);
    if (ret != 0) {
        nvt_dbg(ERR, "failed in release buffer\n");
		vos_file_close(fd);
        return -1;
    }

	vos_file_close(fd);

	return len;
}

int nvt_kdrv_ai_api_test(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}
	return 0;
}

int nvt_kdrv_ai_func_test(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
#if (KDRV_AI_FUNC_TEST == 1)
	emu_ai(pargv);
#else
	nvt_dbg(ERR, "KDRV_AI: Error, please enable #define KDRV_AI_FUNC_TEST 1 in ai_emu.h and copy Makefile.kdrv_vfy to Makefile.\r\n");
#endif
	return 0;
}

int nvt_kdrv_ai_set_clk_rate(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	int ret = 0;
	unsigned long clk_rate = 0;
	
	if (argc != 1) {
		nvt_dbg(ERR, "wrong argument:%d", argc);
		return -1;
	}

	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}
	
	if (kdrv_ai_get_init_status()) {
		nvt_dbg(ERR, "invalid init status, user should uninit ai flow before set clk rate\n");
		return -1;
	}
	
	if (kstrtoul(pargv[0], 0, &clk_rate)) {
		nvt_dbg(ERR, "invalid input clk rate:%s\n", pargv[0]);
		return -1;
	}
	printk("[KDRV AI] CONV clk rate is set to %ld MHz\n", clk_rate);
	kdrv_ai_set_conv_clk_rate(clk_rate);

	return ret;
}

#ifdef __KERNEL__
#if KDRV_AI_JMISP_TEST
EXPORT_SYMBOL(set_jmisp_parm);
EXPORT_SYMBOL(get_jmisp_workbuf_size);
#endif
#endif