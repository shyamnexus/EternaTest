/**
	@brief Source file of vendor media common.

	@file vendor_common.c

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#if defined(__LINUX)
#define _GNU_SOURCE
#include <sched.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if defined(__LINUX)
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif
#include "hdal.h"
#include "hd_logger_p.h"
#include "kflow_common/nvtmpp.h"
#include "kflow_common/nvtmpp_ioctl.h"
#include "vendor_common.h"
#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
#define NVTMPP_OPEN(...) 0
#define NVTMPP_IOCTL nvtmpp_ioctl
#define NVTMPP_CLOSE(...)
#endif
#if defined(__LINUX)
#define NVTMPP_OPEN  open
#define NVTMPP_IOCTL ioctl
#define NVTMPP_CLOSE close
#endif

/*-----------------------------------------------------------------------------*/
/* Debug Variables & Functions                                                 */
/*-----------------------------------------------------------------------------*/

#define HD_COMM_DBG_FATAL     0
#define HD_COMM_DBG_ERR       1
#define HD_COMM_DBG_WRN       2
#define HD_COMM_DBG_MSG       3
#define HD_COMM_DBG_IND       4
#define HD_COMM_DBG_FUNC      5


#define HD_COMM_FLOW_ERR(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_COMM] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_COMM]) >= HD_COMM_DBG_ERR) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_COMM_FLOW_WRN(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_COMM] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_COMM]) >= HD_COMM_DBG_WRN) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_COMM_FLOW_MSG(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_COMM] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_COMM]) >= HD_COMM_DBG_MSG) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_COMM_FLOW_IND(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_COMM] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_COMM]) >= HD_COMM_DBG_IND) { hdal_flow_log_p(fmt, ##args);  }}
#define HD_COMM_FLOW_FUNC(fmt, args...) { if (hd_flow_dbg_lvl[HD_FLOW_DBG_COMM] && (*hd_flow_dbg_lvl[HD_FLOW_DBG_COMM]) >= HD_COMM_DBG_FUNC) { hdal_flow_log_p(fmt, ##args);  }}

#define CHKPNT                    printf("\033[37mCHK: %d, %s\033[0m\r\n",__LINE__,__func__) ///< Show a color sting of line count and function name in your insert codes
#define DBG_ERR(fmtstr, args...)  printf("\033[31mvendor_comm ERR:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_WRN(fmtstr, args...)  printf("\033[33mvendor_comm WRN:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
#define DBG_IND(fmtstr, args...)


/*-----------------------------------------------------------------------------*/
/* Calcuate buffer size                                                        */
/*-----------------------------------------------------------------------------*/

//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NRX: RAW compress: Only support 12bit mode
#define RAW_COMPRESS_RATIO g_cfg_max_buf.raw_compress_ratio
#define VDO_NRX_BUFSIZE(w, h)           (ALIGN_CEIL_4(ALIGN_CEIL_64(w) / 64 * ((24*RAW_COMPRESS_RATIO+99)/100) * 4 * (h)))
//CA for AWB
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
//LA for AE
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)
//VA for AF
#define VDO_VA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 2)
//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NVX: YUV compress
#define YUV_COMPRESS_RATIO 75
#define VDO_NVX_BUFSIZE(w, h, pxlfmt)	(VDO_YUV_BUFSIZE(w, h, pxlfmt) * YUV_COMPRESS_RATIO / 100)

// MD
#define MD_HEAD_BUFSIZE()	            (0x40)
// Note , the md info w, h is vprc input w, h not out w, h
#define MD_INFO_BUFSIZE(w, h)           (ALIGN_CEIL_64((((w + 511) >> 9) << 2) * ((h + 15) >> 4)))

#define RAW_COMPRESS_RATIO_MIN          41
#define RAW_COMPRESS_RATIO_MAX          59
#define VCAP_DBGINFO_BUFSIZE()	        (0x200)
#define CA_WIN_NUM_W		32
#define CA_WIN_NUM_H		32
#define LA_WIN_NUM_W		32
#define LA_WIN_NUM_H		32
#define VA_WIN_NUM_W		16
#define VA_WIN_NUM_H		16

// EIS
#define GYRO_MAX_NUM_LIMIT            256
#define EIS_DBG_CTX_SIZE              100
#define EIS_2DLUT_SIZE                65*68*4 // 65x65 2dlut
#define GYRO_DBG_SIZE                 12
#define GYRO_DATA_SIZE(gyro_num)      (4*gyro_num*6 + sizeof(UINT32)*gyro_num)


static VENDOR_COMM_MEM_CFG_MAX_BUF g_cfg_max_buf = {
	.md_func = 1,
	.vprc_max_in_size = {3840, 2160},
	.eis_func = 0,
	.gyro_max_num = 32,
	.raw_compress_ratio = RAW_COMPRESS_RATIO_MIN,
};



#define MAX_CORE_NUM 	2
/*-----------------------------------------------------------------------------*/


extern int nvtmpp_hdl;
extern BOOL hd_common_chk_if_hdal_mem(UINTPTR pa);
extern void hd_common_rescan_linux_mem(void);

#if defined(__LINUX)
static void sysfs_write_p(char *path, char *s)
{
	char buf[80];
	int len;
	int fd = open(path, O_WRONLY);

	if (fd < 0) {
		if (strerror_r(errno, buf, sizeof(buf)) == 0) {
			printf("err: %s\r\n" ,buf);
		}
		return;
	}

	len = write(fd, s, strlen(s) + 1);
	if (len < 0) {
		if (strerror_r(errno, buf, sizeof(buf)) == 0) {
			printf("err: %s\r\n" ,buf);
		}
	}
	close(fd);
}

static ssize_t sysfs_read_p(char *path, char *s, int num_bytes)
{
	char buf[80];
	ssize_t count = 0;
	int fd = open(path, O_RDONLY);

	if (fd < 0) {
		if (strerror_r(errno, buf, sizeof(buf)) == 0) {
			printf("err: %s\r\n" ,buf);
		}
		return -1;
	}

	if ((count = read(fd, s, (num_bytes -1))) < 0) {
		if (strerror_r(errno, buf, sizeof(buf)) == 0) {
			printf("err: %s\r\n" ,buf);
		}
	} else {
		if ((count >= 1) && (s[count-1] == '\n')) {
			s[count-1] = '\0';
		} else {
			s[count] = '\0';
		}
	}
	close(fd);
	return count;
}

static int hotplug_align_size_p(void)
{
	char buf[15] = {0};
	int ret = 0;

	ret = sysfs_read_p("/sys/devices/system/memory/block_size_bytes", buf, 15);
	if (ret < 0) {
		DBG_ERR("%s: can't get alignment size\r\n", __func__);
		return HD_ERR_DRV;
	}

	ret = (int)strtol(buf, NULL, 16);
	if (ret <= 0)
		return HD_ERR_DRV;
	else
		return ret;
}

static int hotplug_set_p(unsigned long mem_addr, unsigned long mem_size)
{
	char buf[60] = {0};
	int block_size_bytes = 0;
	unsigned long size = 0;

	block_size_bytes = hotplug_align_size_p();
	if (block_size_bytes < 0)
		return HD_ERR_DRV;

	if ((mem_addr % block_size_bytes) != 0 ||
		(mem_size % block_size_bytes) != 0) {
		DBG_ERR("%s: Your parameters are not aligned.  \
			Addr: 0x%x size: 0x%x aligned size: 0x%x\n", __func__, (int)mem_addr, (int)mem_size, (int)block_size_bytes);
		return HD_ERR_INV;
	}

	// check if mem has plugged before
	struct stat sb;
	sprintf(buf, "/sys/devices/system/memory/memory%d", (int)(mem_addr/block_size_bytes));
	if (stat(buf, &sb) == 0 && S_ISDIR(sb.st_mode)) {
		//printf("mem has plugged.\n");
		return HD_OK;
	}

	// Add memblock probe
	size = mem_size;
	while (size > 0) {
		size -= block_size_bytes;
		sprintf(buf, "0x%08lx", mem_addr + size);
		sysfs_write_p("/sys/devices/system/memory/probe", buf);
		sprintf(buf, "/sys/devices/system/memory/memory%u/state", (unsigned int)((mem_addr + size) / block_size_bytes));
		sysfs_write_p(buf, "online_movable");
	}
	// re-scan linux memory
	hd_common_rescan_linux_mem();
	return HD_OK;
}
#endif




HD_RESULT vendor_common_mem_set(VENDOR_COMMON_MEM_PARAM_ID id, VOID *p_param)
{
	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}
	switch (id) {
	#if defined(__LINUX)
	case VENDOR_COMMON_MEM_ITEM_LINUX_HOT_PLUG: {
			VENDOR_LINUX_MEM_HOT_PLUG *p_mem_hotplug;

			p_mem_hotplug = (VENDOR_LINUX_MEM_HOT_PLUG *)p_param;
			return hotplug_set_p(p_mem_hotplug->start_addr, p_mem_hotplug->size);
		}
	#endif
	case VENDOR_COMMON_MEM_ITEM_CFG_MAX_BUF: {
		VENDOR_COMM_MEM_CFG_MAX_BUF *p_msg;

		p_msg = (VENDOR_COMM_MEM_CFG_MAX_BUF *)p_param;
		g_cfg_max_buf = *p_msg;
		HD_COMM_FLOW_MSG("vendor_common_mem_set CFG_MAX_BUF\r\n");
		HD_COMM_FLOW_MSG("    md_func(%d) vprc_max_in_w(%d) vprc_max_in_h(%d)\r\n",
							g_cfg_max_buf.md_func, g_cfg_max_buf.vprc_max_in_size.w, g_cfg_max_buf.vprc_max_in_size.h);
		HD_COMM_FLOW_MSG("    eis_func(%d) gyro_max_num(%d)\r\n",
							g_cfg_max_buf.eis_func, g_cfg_max_buf.gyro_max_num);
		HD_COMM_FLOW_MSG("    raw_compress_ratio(%d)\r\n",
							g_cfg_max_buf.raw_compress_ratio);
		if (g_cfg_max_buf.gyro_max_num > GYRO_MAX_NUM_LIMIT) {
			DBG_WRN("gyro_max_num %d exceeds limit, reduce to %d\r\n", g_cfg_max_buf.gyro_max_num, GYRO_MAX_NUM_LIMIT);
			g_cfg_max_buf.gyro_max_num = GYRO_MAX_NUM_LIMIT;
		}
		if (g_cfg_max_buf.raw_compress_ratio == 0) {
			DBG_DUMP("raw_compress_ratio %d, set to default %d\r\n", g_cfg_max_buf.raw_compress_ratio, RAW_COMPRESS_RATIO_MIN);
			g_cfg_max_buf.raw_compress_ratio = RAW_COMPRESS_RATIO_MIN;
		} else if (g_cfg_max_buf.raw_compress_ratio < RAW_COMPRESS_RATIO_MIN) {
			DBG_WRN("raw_compress_ratio %d smaller than min, set to %d\r\n", g_cfg_max_buf.raw_compress_ratio, RAW_COMPRESS_RATIO_MIN);
			g_cfg_max_buf.raw_compress_ratio = RAW_COMPRESS_RATIO_MIN;
		} else if (g_cfg_max_buf.raw_compress_ratio > RAW_COMPRESS_RATIO_MAX) {
			DBG_WRN("raw_compress_ratio %d larger than max, set to %d\r\n", g_cfg_max_buf.raw_compress_ratio, RAW_COMPRESS_RATIO_MAX);
			g_cfg_max_buf.raw_compress_ratio = RAW_COMPRESS_RATIO_MAX;
		}
		break;
	}
	default:
		DBG_ERR("Not Support parm id 0x%x\r\n", id);
		return HD_ERR_NOT_SUPPORT;
	}
	return HD_OK;

}

HD_RESULT vendor_common_mem_get(VENDOR_COMMON_MEM_PARAM_ID id, VOID *p_param)
{
	if (p_param == NULL) {
		return HD_ERR_NULL_PTR;
	}
	switch (id) {
	case VENDOR_COMMON_MEM_ITEM_BRIDGE_MEM: {
			int ret;
			VENDOR_COMM_BRIDGE_MEM *p_bridge_mem = {0};
			NVTMPP_GET_SYSMEM_REGION_S sysmem_region = {0};
			p_bridge_mem = (VENDOR_COMM_BRIDGE_MEM *)p_param;
			ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_GET_BRIDGE_MEM, &sysmem_region);
		    if (ret < 0 || sysmem_region.rtn != 0) {
				DBG_ERR("get bridge mem failed.\r\n");
				return HD_ERR_FAIL;
		    }
			p_bridge_mem->phys_addr = sysmem_region.phys_addr;
			p_bridge_mem->size = sysmem_region.size;
		}
		break;
	case VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE: {
			int ret;
			VENDOR_COMM_MAX_FREE_BLOCK    *p_maxfreeblk = {0};
			NVTMPP_IOC_VB_GET_MAX_FREE_S  free_s = {0};

			p_maxfreeblk = (VENDOR_COMM_MAX_FREE_BLOCK *)p_param;
			free_s.ddr = p_maxfreeblk->ddr;
			ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_GET_MAX_FREE_BLK_SZ, &free_s);
		    if (ret < 0) {
				DBG_ERR("get maxfreeblk failed.\r\n");
				return HD_ERR_FAIL;
		    }
			p_maxfreeblk->size = free_s.size;
		}
		break;
	case VENDOR_COMMON_MEM_ITEM_COMM_POOL_RANGE: {
			int                         ret;
			VENDOR_COMM_POOL_RANGE      *p_range = {0};
			NVTMPP_GET_COMM_POOL_RANGE_S msg = {0};

			p_range = (VENDOR_COMM_POOL_RANGE *)p_param;
			msg.ddr = p_range->ddr;
			ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_GET_COMM_POOL_RANGE, &msg);
		    if (ret < 0 || msg.rtn != 0) {
				DBG_ERR("get comm pool range failed.\r\n");
				return HD_ERR_FAIL;
		    }
			p_range->phys_addr = msg.phys_addr;
			p_range->size = msg.size;
		}
		break;
	case VENDOR_COMMON_MEM_ITEM_FREE_SIZE: {
			int ret;
			VENDOR_COMM_FREE_SIZE        *p_free = {0};
			NVTMPP_IOC_VB_GET_FREE_S      free_s = {0};

			p_free = (VENDOR_COMM_FREE_SIZE *)p_param;
			free_s.ddr = p_free->ddr;
			ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_GET_FREE_SZ, &free_s);
		    if (ret < 0) {
				DBG_ERR("get maxfreeblk failed.\r\n");
				return HD_ERR_FAIL;
		    }
			p_free->size = free_s.size;
		}
		break;

	case VENDOR_COMMON_MEM_ITEM_VIRT_INFO2: {
			int ret;
			VENDOR_COMM_MEM_VIRT_INFO2          *p_vir_info = {0};
			NVTMPP_IOC_VB_GET_USER_VA_INFO_S           msg = {0};

			p_vir_info = (VENDOR_COMM_MEM_VIRT_INFO2 *)p_param;
			msg.va = p_vir_info->va;
			p_vir_info->pa = 0;
			p_vir_info->cached = 0;
			p_vir_info->is_hdal = 0;
			ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_GET_USER_VA_INFO, &msg);
		    if (ret < 0) {
				return HD_ERR_SYS;
		    }
			if (msg.rtn < 0) {
		        return HD_ERR_INV;
		    }
			p_vir_info->pa = msg.pa;
			p_vir_info->cached = msg.cached;
			#if defined(__LINUX)
			p_vir_info->is_hdal = hd_common_chk_if_hdal_mem(msg.pa);
			#else
			p_vir_info->is_hdal = 1;
			#endif
		}
		break;
	default:
		DBG_ERR("Not Support parm id 0x%x\r\n", id);
		return HD_ERR_NOT_SUPPORT;
	}
	return HD_OK;

}

static void* vendor_common_mem_mmap(int fd, HD_COMMON_MEM_MEM_TYPE mem_type, UINTPTR phy_addr, UINT32 size)
{
	void        *map_addr;

	if (0 == size) {
		DBG_ERR("size is 0\r\n");
        return NULL;
	}
	HD_COMM_FLOW_IND("vendor_common_mem_mmap:\r\n    mem_type(%d), phy_addr(0x%08x), size(0x%08x)\r\n", mem_type, (int)phy_addr, (int)size);
	#if defined(__LINUX)
	if ((phy_addr & (sysconf(_SC_PAGE_SIZE) - 1)) != 0) {
		DBG_ERR("phy_addr 0x%lx not page align\r\n", (ULONG)phy_addr);
        return NULL;
	}
	if (HD_COMMON_MEM_MEM_TYPE_NONCACHE == mem_type) {
		#define NONCACHE_FLAG_32        0x80000000L
		#define NONCACHE_FLAG_64        0x8000000000000000L

		if (sizeof(UINTPTR) == 4) {
			phy_addr |= NONCACHE_FLAG_32;
		} else {
			phy_addr |= NONCACHE_FLAG_64;
		}
	}
	map_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr);
	if (map_addr == MAP_FAILED) {
		DBG_ERR("mmap fail phy_addr 0x%lx, size 0x%x\r\n", (ULONG)phy_addr, (int)size);
		goto map_err;
	}
	HD_COMM_FLOW_IND("    map_addr(0x%08lx)\r\n", (ULONG)map_addr);
	return map_addr;
map_err:
    return NULL;
	#else
	/*
	map_addr = (void *)nvtmpp_sys_pa2va(phy_addr);
	if (0L == (ULONG)map_addr) {
		DBG_ERR("mmap fail phy_addr 0x%lx, size 0x%x\r\n", (ULONG)phy_addr, (int)size);
		goto map_err;
	}
	*/
	map_addr = (void*)phy_addr;
	return map_addr;
	#endif
}

HD_RESULT vendor_common_mem_alloc_fixed_pool(CHAR* name, UINTPTR *phy_addr, void **virt_addr, UINT32 size, HD_COMMON_MEM_DDR_ID ddr)
{
	NVTMPP_IOC_VB_CREATE_FIXPOOL_S   cre_fixpool_s = {0} ;
	UINTPTR                          pa;
	void                            *va;
    int                              ret;
	int                              fd;

	if (name == NULL) {
		DBG_ERR("name is NULL\r\n");
		return HD_ERR_NULL_PTR;
	}
	if (phy_addr == NULL) {
		DBG_ERR("phy_addr is NULL\r\n");
		return HD_ERR_NULL_PTR;
	}
	if (virt_addr == NULL) {
		DBG_ERR("virt_addr is NULL\r\n");
		return HD_ERR_NULL_PTR;
	}
	fd = NVTMPP_OPEN("/dev/nvtmpp", O_RDWR|O_SYNC);
	if (fd < 0) {
		DBG_ERR("open /dev/nvtmpp error\r\n");
		return HD_ERR_SYS;
	}
	strncpy(cre_fixpool_s.pool_name, name, sizeof(cre_fixpool_s.pool_name)-1);
	cre_fixpool_s.blk_cnt = 1;
	cre_fixpool_s.blk_size = size;
	cre_fixpool_s.ddr = ddr;
	ret = NVTMPP_IOCTL(fd, NVTMPP_IOC_VB_CREATE_FIXED_POOL, &cre_fixpool_s);
	pa = cre_fixpool_s.rtn;
    if (ret < 0 || pa == 0) {
		DBG_ERR("cre_fixedpool_fail\r\n");
		goto cre_fixpool_fail;
    }
	va = vendor_common_mem_mmap(fd, HD_COMMON_MEM_MEM_TYPE_CACHE, pa, size);
	if (va == 0) {
		DBG_ERR("mmap fail, pa = 0x%lx, size = 0x%x\r\n", (ULONG)pa, (int)size);
		goto cre_fixpool_fail;
	}
	*phy_addr = pa;
	*virt_addr = va;
	NVTMPP_CLOSE(fd);
	return HD_OK;
cre_fixpool_fail:
	*phy_addr = 0;
	*virt_addr = 0;
	NVTMPP_CLOSE(fd);
	return HD_ERR_NOMEM;
}

HD_RESULT vendor_common_mem_relayout(HD_COMMON_MEM_INIT_CONFIG *p_mem_config)
{
	NVTMPP_IOC_VB_CONF_S  vb_conf;
	NVTMPP_IOC_VB_INIT_S  init_s;
	UINT32                i, pool_cnt = 0;
	int                   ret;

	//hdal_flow_log_p("hd_common_mem_relayout:\r\n");
	if (nvtmpp_hdl < 0) {
		DBG_ERR("Please init hd_common_mem firstly\r\n");
		return HD_ERR_INIT;
	}
	if (p_mem_config == NULL) {
		ret = HD_ERR_NULL_PTR;
		goto mem_init_fail;
	}
	memset(&vb_conf, 0x00, sizeof(vb_conf));
	vb_conf.max_pool_cnt = NVTMPP_VB_MAX_COMM_POOLS;
	pool_cnt = 0;
	for (i = 0; i < HD_COMMON_MEM_MAX_POOL_NUM; i++) {
		if (pool_cnt >= NVTMPP_VB_MAX_COMM_POOLS) {
			DBG_ERR("exceeds vb pool limit %d\r\n", NVTMPP_VB_MAX_COMM_POOLS);
			ret = HD_ERR_LIMIT;
			goto mem_init_fail;
		}
		if (p_mem_config->pool_info[i].blk_size == 0) {
			continue;
		}
		vb_conf.common_pool[pool_cnt].type = p_mem_config->pool_info[i].type;
		vb_conf.common_pool[pool_cnt].ddr = p_mem_config->pool_info[i].ddr_id;
		vb_conf.common_pool[pool_cnt].blk_cnt = p_mem_config->pool_info[i].blk_cnt;
		vb_conf.common_pool[pool_cnt].blk_size = p_mem_config->pool_info[i].blk_size;
		//hdal_flow_log_p("    mem type(0x%08x) ddr(%d) blk_cnt(%d) blk_size(0x%08x)\r\n", vb_conf.common_pool[pool_cnt].type, vb_conf.common_pool[pool_cnt].ddr,
		//	             vb_conf.common_pool[pool_cnt].blk_cnt, vb_conf.common_pool[pool_cnt].blk_size);
		pool_cnt++;
	}
	ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_CONF_SET, &vb_conf);
    if (ret < 0) {
        ret = HD_ERR_SYS;
		goto mem_init_fail;
    }
	if (vb_conf.rtn < 0) {
		ret = HD_ERR_PARAM;
		goto mem_init_fail;
	}
	ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_RELAYOUT, &init_s);
    if (ret < 0) {
        ret = HD_ERR_SYS;
		goto mem_init_fail;
    }
	if (init_s.rtn < 0) {
		ret = HD_ERR_PARAM;
		goto mem_init_fail;
	}
	return HD_OK;
mem_init_fail:
	NVTMPP_CLOSE(nvtmpp_hdl);
	nvtmpp_hdl = -1;
	return ret;
}


HD_RESULT vendor_common_mem_cache_sync(void* virt_addr, unsigned int size, VENDOR_COMMON_MEM_DMA_DIR dir)
{
	return hd_common_mem_cache_sync(virt_addr, size, dir);
}

HD_RESULT vendor_common_mem_cache_sync_all(void* virt_addr, unsigned int size, VENDOR_COMMON_MEM_DMA_DIR dir)
{
	#if 0
	return hd_common_mem_cache_sync(virt_addr, size, dir);
	#else
	#if defined(__LINUX)
	NVTMPP_IOC_VB_CACHE_SYNC_BY_CPU_S    msg;
	int                                  ret, i;
	cpu_set_t                            mask;

	if (nvtmpp_hdl < 0) {
		DBG_ERR("Please init hd_common_mem firstly\r\n");
		return HD_ERR_INIT;
	}
	if (0 == size) {
		DBG_ERR("size is 0\r\n");
		return HD_ERR_INV;
	}
	if (sched_getaffinity(0, sizeof(mask), &mask) == -1)  {
		DBG_ERR("sched_getaffinity fail %d\r\n");
		return HD_ERR_SYS;
	}
	msg.virt_addr = virt_addr;
	msg.size = size;
	msg.dma_dir = dir;
	msg.cpu_count = CPU_COUNT(&mask);
	for (i = 0; i< MAX_CORE_NUM; i++) {
		if (CPU_ISSET(i, &mask)) {
			msg.cpu_id = i;
		}
	}
	HD_COMM_FLOW_IND("vendor_common_mem_cache_sync_all:\r\n    virt_addr(0x%08lx), size(0x%08x), dir(%d), cpu_count(%d), cpu_id(%d)\r\n"
						,virt_addr, size, dir, msg.cpu_count, msg.cpu_id);
	//printf("msg.cpu_count = %d, cpu_id = %d\r\n", msg.cpu_count, msg.cpu_id);
	ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_CACHE_SYNC_BY_CPU, &msg);
    if (ret < 0) {
        return HD_ERR_SYS;
    }
    return HD_OK;
	#else
	return HD_ERR_SYS;
	#endif
	#endif
}

VENDOR_COMMON_MEM_VB_POOL vendor_common_mem_create_pool(CHAR *pool_name, UINT32 blk_size, UINT32 blk_cnt, HD_COMMON_MEM_DDR_ID ddr)
{
	NVTMPP_IOC_VB_CREATE_POOL_S   cre_pool_s = {0};
	NVTMPP_VB_POOL                pool;
	int                           ret;

	if (pool_name == NULL) {
		HD_COMM_FLOW_MSG("vendor_common_mem_create_pool:\r\n    ddr(%d), blk_size(0x%08x), blk_cnt(%d), name(NULL)\r\n",
						ddr, blk_size, blk_cnt);
	} else {
		HD_COMM_FLOW_MSG("vendor_common_mem_create_pool:\r\n    ddr(%d), blk_size(0x%08x), blk_cnt(%d), name(%s)\r\n",
						ddr, blk_size, blk_cnt, pool_name);
	}
    if (nvtmpp_hdl < 0) {
		return VENDOR_COMMON_MEM_VB_INVALID_POOL;
	}
	if (blk_size == 0) {
		DBG_ERR("blk_size is 0\r\n");
		return VENDOR_COMMON_MEM_VB_INVALID_POOL;
	}
	if (pool_name == NULL) {
		strncpy(cre_pool_s.pool_name, "user", sizeof(cre_pool_s.pool_name)-1);
	} else {
		strncpy(cre_pool_s.pool_name, pool_name, sizeof(cre_pool_s.pool_name)-1);
	}
	cre_pool_s.blk_cnt = blk_cnt;
	cre_pool_s.blk_size = blk_size;
	cre_pool_s.ddr = ddr;
	ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_CREATE_POOL, &cre_pool_s);
	if (ret < 0) {
		HD_COMM_FLOW_ERR("    ioctl create pool fail\r\n");
		DBG_ERR("cre_pool_fail\r\n");
		goto cre_pool_fail;
	}
	if (cre_pool_s.rtn == VENDOR_COMMON_MEM_VB_INVALID_POOL) {
		HD_COMM_FLOW_ERR("    create pool fail\r\n");
		DBG_ERR("cre_pool_fail\r\n");
		goto cre_pool_fail;
	}
	pool = cre_pool_s.rtn;
	return pool;
cre_pool_fail:
	return VENDOR_COMMON_MEM_VB_INVALID_POOL;
}


HD_RESULT vendor_common_mem_destroy_pool(VENDOR_COMMON_MEM_VB_POOL pool)
{
	NVTMPP_IOC_VB_DESTROY_POOL_S  des_pool_s;
    int                           ret;

	HD_COMM_FLOW_MSG("vendor_common_mem_destroy_pool:\r\n    pool(0x%08x)\r\n", (int)pool);
    if (nvtmpp_hdl < 0) {
		return HD_ERR_UNINIT;
	}
	des_pool_s.pool = pool;
	ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_DESTROY_POOL, &des_pool_s);
	if (ret < 0) {
		DBG_ERR("destroy pool fail, pool = 0x%x\r\n", (int)pool);
		return HD_ERR_SYS;
	}
	if (des_pool_s.rtn < 0) {
		return HD_ERR_PARAM;
	}
	return HD_OK;
}


HD_RESULT vendor_common_mem_lock_block(HD_COMMON_MEM_VB_BLK blk)
{
	NVTMPP_IOC_VB_LOCK_BLK_S   msg;
    int                        ret;

    if (nvtmpp_hdl < 0) {
		return HD_ERR_UNINIT;
	}
	msg.blk = blk;
	ret = NVTMPP_IOCTL(nvtmpp_hdl, NVTMPP_IOC_VB_LOCK_BLK, &msg);
    if (ret < 0) {
        return HD_ERR_SYS;
    }
	if (msg.rtn < 0) {
		return HD_ERR_INV;
	}
    return HD_OK;
}

UINT32 _vendor_common_mem_calc_vcap_extra_size_for_eis(void)
{
	return (GYRO_DBG_SIZE + GYRO_DATA_SIZE(g_cfg_max_buf.gyro_max_num));
}

UINT32 _vendor_common_mem_calc_vprc_extra_size_for_eis(void)
{
	return (EIS_DBG_CTX_SIZE + EIS_2DLUT_SIZE + GYRO_DBG_SIZE + GYRO_DATA_SIZE(g_cfg_max_buf.gyro_max_num));
}


static UINT32 _vendor_common_mem_calc_vcap_extra_size(void)
{
	UINT32 buf_size = 0, tmp_size = 0;

	// debug info size
	buf_size += VCAP_DBGINFO_BUFSIZE();
	// ca size
	tmp_size = VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H);
	buf_size += tmp_size;
	DBG_IND("ca_size = %d\r\n", tmp_size);
	// la size
	tmp_size = VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	buf_size += tmp_size;
	DBG_IND("la_size = %d\r\n", tmp_size);
	// va size
	tmp_size = VDO_VA_BUF_SIZE(VA_WIN_NUM_W, VA_WIN_NUM_H);
	buf_size += tmp_size;
	DBG_IND("va_size = %d\r\n", tmp_size);
	// EIS info
	if (g_cfg_max_buf.eis_func) {
		tmp_size = _vendor_common_mem_calc_vcap_extra_size_for_eis();
		buf_size += tmp_size;
		DBG_IND("gyro_size = %d\r\n", tmp_size);
	}
	DBG_IND("vcap_extra_size = %d\r\n", buf_size);
	return buf_size;
}

static UINT32 _vendor_common_mem_calc_vprc_extra_size(void)
{
	UINT32 buf_size = 0, tmp_size = 0;

	// MD info,  Note: the md info w, h is vprc input w, h not out w, h
	if (g_cfg_max_buf.md_func) {
		tmp_size = (MD_HEAD_BUFSIZE()+ MD_INFO_BUFSIZE(g_cfg_max_buf.vprc_max_in_size.w, g_cfg_max_buf.vprc_max_in_size.h));
		buf_size += tmp_size;
		DBG_IND("md_size = %d\r\n", tmp_size);
	}
	// EIS info
	if (g_cfg_max_buf.eis_func) {
		tmp_size = _vendor_common_mem_calc_vprc_extra_size_for_eis();
		buf_size += tmp_size;
		DBG_IND("eis_size = %d\r\n", tmp_size);
	}
	DBG_IND("vprc_extra_size = %d\r\n", buf_size);
	return buf_size;
}


UINT32 vendor_common_mem_calc_max_buf_size(UINT32 w, UINT32 h, HD_VIDEO_PXLFMT pxlfmt)
{
	UINT32 buf_size;

	switch (pxlfmt) {
	/* video yuv format	*/
	case HD_VIDEO_PXLFMT_YUV400:
	case HD_VIDEO_PXLFMT_YUV420_PLANAR:
	case HD_VIDEO_PXLFMT_YUV420:
	case HD_VIDEO_PXLFMT_YUV420_LEGACY:
	case HD_VIDEO_PXLFMT_YUV422_PLANAR:
	case HD_VIDEO_PXLFMT_YUV422:
	case HD_VIDEO_PXLFMT_YUV422_UYVY:
	case HD_VIDEO_PXLFMT_YUV422_VYUY:
	case HD_VIDEO_PXLFMT_YUV422_YUYV:
	case HD_VIDEO_PXLFMT_YUV422_YVYU:
	case HD_VIDEO_PXLFMT_YUV444_PLANAR:
	case HD_VIDEO_PXLFMT_YUV444:
	case HD_VIDEO_PXLFMT_YUV444_ONE:
	case HD_VIDEO_PXLFMT_YUV420_W8:
	case HD_VIDEO_PXLFMT_YUV420_MB:
	case HD_VIDEO_PXLFMT_YUV420_MB2:
	case HD_VIDEO_PXLFMT_YUV420_MB3:
	case HD_VIDEO_PXLFMT_YUV420_MB4:
	case HD_VIDEO_PXLFMT_YUV420_MB5:
		buf_size = VDO_YUV_BUFSIZE(w, h, pxlfmt) + _vendor_common_mem_calc_vprc_extra_size();
		break;
	/* video yuv compress format */
	case HD_VIDEO_PXLFMT_YUV420_NVX1_H264:
	case HD_VIDEO_PXLFMT_YUV420_NVX1_H265:
	case HD_VIDEO_PXLFMT_YUV420_NVX2:
	case HD_VIDEO_PXLFMT_YUV420_NVX3:
	case HD_VIDEO_PXLFMT_YUV420_NVX4:
	case HD_VIDEO_PXLFMT_YUV422_NVX3:
	case HD_VIDEO_PXLFMT_YUV420_NVX5:
		buf_size = VDO_NVX_BUFSIZE(w, h, pxlfmt) + _vendor_common_mem_calc_vprc_extra_size();
		break;

	/* video raw format */
	case HD_VIDEO_PXLFMT_RAW8:
	case HD_VIDEO_PXLFMT_RAW10:
	case HD_VIDEO_PXLFMT_RAW12:
	case HD_VIDEO_PXLFMT_RAW14:
	case HD_VIDEO_PXLFMT_RAW16:
	case HD_VIDEO_PXLFMT_RAW8_SHDR2:
	case HD_VIDEO_PXLFMT_RAW10_SHDR2:
	case HD_VIDEO_PXLFMT_RAW12_SHDR2:
	case HD_VIDEO_PXLFMT_RAW14_SHDR2:
	case HD_VIDEO_PXLFMT_RAW16_SHDR2:
	case HD_VIDEO_PXLFMT_RAW8_SHDR3:
	case HD_VIDEO_PXLFMT_RAW10_SHDR3:
	case HD_VIDEO_PXLFMT_RAW12_SHDR3:
	case HD_VIDEO_PXLFMT_RAW14_SHDR3:
	case HD_VIDEO_PXLFMT_RAW16_SHDR3:
	case HD_VIDEO_PXLFMT_RAW8_SHDR4:
	case HD_VIDEO_PXLFMT_RAW10_SHDR4:
	case HD_VIDEO_PXLFMT_RAW12_SHDR4:
	case HD_VIDEO_PXLFMT_RAW14_SHDR4:
	case HD_VIDEO_PXLFMT_RAW16_SHDR4:
		buf_size = VDO_RAW_BUFSIZE(w, h, pxlfmt) + _vendor_common_mem_calc_vcap_extra_size();
		break;
	/* video raw compress format, only support 12 bits */
	case HD_VIDEO_PXLFMT_NRX12:
	case HD_VIDEO_PXLFMT_NRX12_SHDR2:
	case HD_VIDEO_PXLFMT_NRX12_SHDR3:
	case HD_VIDEO_PXLFMT_NRX12_SHDR4:
		buf_size = VDO_NRX_BUFSIZE(w, h) + _vendor_common_mem_calc_vcap_extra_size();
		break;

	default:
		DBG_ERR("Not Support pxlfmt 0x%x\r\n", pxlfmt);
		buf_size = 0;
	}
	DBG_IND("buf_size = %d\r\n", buf_size);
	return ALIGN_CEIL_64(buf_size);
}

HD_RESULT vendor_common_get_ddrid(HD_COMMON_MEM_POOL_TYPE pool_type, INT chip_id, HD_COMMON_MEM_DDR_ID *ddr_id)
{
	*ddr_id = DDR_ID0;
	return HD_OK;
}


