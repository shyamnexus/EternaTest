/**
	@brief Source file of vendor user-space net API.

	@file vendor_ai_net.c

	@ingroup vendor_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "hd_type.h"
#if defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_)
#include "hd_common.h"
#endif
#if defined(_BSP_NS02201_) || defined(_BSP_NS02302_) || defined(_BSP_NS02401_)
#include "vendor_common.h"
#endif
#include "vendor_ai_comm_flow.h"
#include "vendor_ai_internal.h"
#include "vendor_ai.h"
#include "vendor_ai_net/vendor_ai_net_group.h"
//#include "vendor_ai_net_flow.h"
//#include "vendor_ai_net_gen.h"

#include "kwrap/platform.h"

#if defined(__LINUX)
#include <sys/ioctl.h>
#include <sys/time.h>
#endif

//=============================================================
#define __CLASS__ 				"[ai][lib][comm]"
#include "vendor_ai_debug.h"
//=============================================================

extern UINT32 g_ai_support_net_max;
extern INT32 *vendor_ais_flow_fd;
extern INT32 *vendor_ais_isp_fd; 
static UINT32 vendor_ais_init_cnt = 0;
static UINT32 vendor_ais_init_chk = 0;
extern UINT32 max_isp;
static VENDOR_AIS_MEM_INFO g_mem_info = {0};
extern UINT32 g_real_chip_id;

VOID vendor_ai_errno_location(VOID)
{
#if defined (__UITRON) || defined(__ECOS)  || defined (__FREERTOS)
	return;
#else
	fprintf(stderr, "vendor_ai_errno_location: errno = %d\r\n", errno);
	fprintf(stderr, "vendor_ai_errno_location: errno string = %s\r\n", strerror(errno));
	return;
#endif
}


VOID* vendor_ai_malloc(size_t size)
{
	VOID* ptr = malloc(size);
	if (ptr != 0) {
		g_mem_info.value[0] += size;
#if (FLOW_AI_DEV == 1)
		if (KFLOW_AI_IOCTL(vendor_ais_flow_fd[0], VENDOR_AIS_FLOW_IOC_MEM_INFO, &g_mem_info) < 0) {
			fprintf(stderr, "VENDOR_AIS_FLOW_IOC_MEM_INFO ioctl failed\r\n");
			vendor_ai_errno_location();
		}
#else
#endif
	}
	return ptr;
}

VOID vendor_ai_free(VOID* ptr, size_t size)
{
	if (ptr != 0) {
		g_mem_info.value[0] -= size;
#if (FLOW_AI_DEV == 1)
		if (KFLOW_AI_IOCTL(vendor_ais_flow_fd[0], VENDOR_AIS_FLOW_IOC_MEM_INFO, &g_mem_info) < 0) {
			fprintf(stderr, "VENDOR_AIS_FLOW_IOC_MEM_INFO ioctl failed\r\n");
			vendor_ai_errno_location();
		}
#else
#endif
	}
	free(ptr);
}

INT32 g_ai_flow_fd = -1;
INT32 g_ai_isp_fd = -1;

HD_RESULT _vendor_ai_query_support_net_max(UINT32 *p_support_net_max)
{
	//INT32 tmp_fd = 0;
	VENDOR_AIS_FLOW_ID id_info = {0};
	HD_RESULT ret = HD_OK;

#if (FLOW_AI_DEV == 1)
	// query support_net_max from kdrv_ai
	if (KFLOW_AI_IOCTL(g_ai_flow_fd, VENDOR_AIS_FLOW_IOC_NET_RESET, &id_info) < 0) {
		fprintf(stderr, "VENDOR_AIS_FLOW_IOC_NET_RESET ioctl failed\r\n");
		vendor_ai_errno_location();
		ret =  HD_ERR_ABORT;
		goto exit;
	}

	if (KFLOW_AI_IOCTL(g_ai_flow_fd, VENDOR_AIS_FLOW_IOC_NET_INIT, &id_info) < 0) {
		fprintf(stderr, "VENDOR_AIS_FLOW_IOC_NET_INIT ioctl failed\r\n");
		vendor_ai_errno_location();
		ret =  HD_ERR_ABORT;
		goto exit;
	}
#if (FLOW_AI_ISP == 1)	
	UINT32 id = 0 ;
	if(g_ai_isp_fd != -1) {
		if (KFLOW_AI_ISP_IOCTL(g_ai_isp_fd, KFLOW_AI_ISP_IOC_RESET_ISP_NET, &id) < 0) {
			fprintf(stderr, "KFLOW_AI_ISP_IOC_RESET_ISP_NET ioctl failed\r\n");
			vendor_ai_errno_location();
			ret =  HD_ERR_ABORT;
			goto exit;
		}
		if (KFLOW_AI_ISP_IOCTL(g_ai_isp_fd, KFLOW_AI_ISP_IOC_INIT_ISP_NET, &id) < 0) {
			fprintf(stderr, "KFLOW_AI_ISP_IOC_INIT_ISP_NET ioctl failed\r\n");
			vendor_ai_errno_location();
			ret =  HD_ERR_ABORT;
			goto exit;
		}
	}
#endif	
#else
	id_info.ai_support_net_max = 0;
#endif

    if (KFLOW_AI_IOCTL(g_ai_flow_fd, VENDOR_AIS_GET_MAX_ISP, &max_isp) < 0) {
        fprintf(stderr, "VENDOR_AIS_GET_MAX_ISP ioctl failed\r\n");
        vendor_ai_errno_location();
        ret =  HD_ERR_ABORT;
        goto exit;
    }

	*p_support_net_max = id_info.ai_support_net_max;
	if(*p_support_net_max < 1) {
		fprintf(stderr, "ai_support_net_max is ZERO. (ignore)\r\n");
		ret =  HD_ERR_ABORT;
		goto exit;
	}

exit:
	return ret;
}

HD_RESULT vendor_ais_cfgchk(UINT32 chk_interval)
{
	vendor_ais_init_chk = chk_interval;
	return HD_OK;
}

HD_RESULT vendor_ais_init_fd(void)
{
	unsigned int i;
	
	if (vendor_ais_init_cnt != 0)
		return HD_ERR_STATE;


	// check default value
	if (g_ai_support_net_max == 0) {
		fprintf(stderr, "g_ai_support_net_max default is ZERO\r\n");
		return HD_ERR_ABORT;
	}

	// ai net init
	g_ai_flow_fd = KFLOW_AI_OPEN(DEFAULT_DEVICE, O_RDWR);
	if (g_ai_flow_fd == -1) {
		fprintf(stderr, "device open %s failed\r\n", DEFAULT_DEVICE);
		vendor_ai_errno_location();
		return HD_ERR_NG;
	}

	g_ai_isp_fd = KFLOW_AI_ISP_OPEN(DEFAULT_ISP_DEVICE, O_RDWR);
	if (g_ai_isp_fd == -1) {
		DBG_WRN("device open %s failed\r\n", DEFAULT_ISP_DEVICE);
	}

	// query user config value
	if (_vendor_ai_query_support_net_max(&g_ai_support_net_max) != HD_OK) {
		fprintf(stderr, "device query support_net_max fail\r\n");
		return HD_ERR_NG;
	}

	//DBG_ERR("g_ai_support_net_max = %d\r\n", g_ai_support_net_max);
	vendor_ais_flow_fd = malloc(sizeof(INT32)*(g_ai_support_net_max + 1));
	if (vendor_ais_flow_fd == NULL) {
		fprintf(stderr, "device flow_fd alloc fail\r\n");
		return HD_ERR_NG;
	}
	vendor_ais_flow_fd[0] = g_ai_flow_fd;
	for (i = 1; i < g_ai_support_net_max+1; i++) {
		vendor_ais_flow_fd[i] = -1;
	}

	vendor_ais_isp_fd = malloc(sizeof(INT32)*(g_ai_support_net_max + 1));
	if (vendor_ais_isp_fd == NULL) {
		fprintf(stderr, "device isp_fd alloc fail\r\n");
		return HD_ERR_NG;
	}
	vendor_ais_isp_fd[0] = g_ai_isp_fd;
	for (i = 1; i < g_ai_support_net_max+1; i++) {
		vendor_ais_isp_fd[i] = -1;
	}

	return HD_OK;
}


HD_RESULT vendor_ais_init_all(void)
{
	HD_RESULT rv = HD_OK;
	if (vendor_ais_init_cnt != 0)
		return HD_ERR_STATE;

	if (vendor_ais_flow_fd[0] != -1) {
#if (FLOW_AI_JOB == 1)
		VENDOR_AIS_FLOW_CORE_CFG cfg;
		cfg.schd = vendor_ais_init_chk;
		
		if (KFLOW_AI_IOCTL(vendor_ais_flow_fd[0], VENDOR_AIS_FLOW_IOC_CORE_CFGCHK, &cfg) < 0) {
			DBG_ERR("VENDOR_AIS_FLOW_IOC_CORE_CFGCHK fail\r\n");
			vendor_ai_errno_location();
			return HD_ERR_NG;
		}
#else
#endif
        if (KFLOW_AI_IOCTL(vendor_ais_flow_fd[0], VENDOR_AIS_FLOW_IOC_GET_CHIP_ID, &g_real_chip_id) < 0) {
			DBG_ERR("VENDOR_AIS_FLOW_IOC_GET_CHIP_ID fail\r\n");
			vendor_ai_errno_location();
			return HD_ERR_NG;
		}
	} else {
        DBG_ERR("vendor_ais_flow_fd[0] is not open ...\n");
        return HD_ERR_NOT_AVAIL;
    }

	vendor_ais_init_net();

	vendor_ai_net_group_init();
	
	vendor_ai_dla_reset();
	
	// ai core init
	rv = vendor_ai_dla_init();
	if (HD_OK != rv) {
		DBG_ERR("dla init failed ... !! ret = %d\r\n", rv);
		return HD_ERR_INIT;
	}

	vendor_ais_init_cnt = 1;
	return HD_OK;
}

HD_RESULT vendor_ais_uninit_all(void)
{
	if (vendor_ais_init_cnt != 1)
		return HD_ERR_STATE;

	// ai core uninit
	vendor_ai_dla_uninit();

	vendor_ai_net_group_uninit();

	vendor_ais_uninit_net();
	
	return HD_OK;
}

HD_RESULT vendor_ais_uninit_fd(void)
{
	if (vendor_ais_init_cnt != 1)
		return HD_ERR_STATE;

	//vendor_ais_flow_fd = 0;
	if (vendor_ais_flow_fd != NULL) {
		//vendor_ai_free(vendor_ais_flow_fd, sizeof(INT32)*(g_ai_support_net_max + 1));
		free(vendor_ais_flow_fd);
		vendor_ais_flow_fd = 0;
	}

	// ai net uninit
	if (g_ai_flow_fd != -1) {
		//DBG_DUMP("close %s\r\n", DEFAULT_DEVICE);
		if (KFLOW_AI_CLOSE(g_ai_flow_fd) < 0) {
			fprintf(stderr, "device close %s failed\r\n", DEFAULT_DEVICE);
			vendor_ai_errno_location();
			return HD_ERR_NG;
		}
	}

	//vendor_ais_isp_fd = 0;
	if (vendor_ais_isp_fd != NULL) {
		//vendor_ai_free(vendor_ais_isp_fd, sizeof(INT32)*(g_ai_support_net_max + 1));
		free(vendor_ais_isp_fd);
		vendor_ais_isp_fd = 0;
	}

	// ai net uninit
	if (g_ai_isp_fd != -1) {
		//DBG_DUMP("close %s\r\n", DEFAULT_DEVICE);
		if (KFLOW_AI_ISP_CLOSE(g_ai_isp_fd) < 0) {
			DBG_WRN("device close %s failed\r\n", DEFAULT_ISP_DEVICE);
		}
	}

	g_ai_flow_fd = -1;
	g_ai_isp_fd = -1;
	vendor_ais_init_cnt = 0;
	return HD_OK;
}

HD_RESULT vendor_ais_cfgschd(UINT32 schd)
{
	if (vendor_ais_init_cnt != 1)
		return HD_ERR_STATE;

	//TODO: check all proc are not start!
	//if (vendor_ais_init_cnt != 1)
	//	return HD_ERR_STATE;

#if (FLOW_AI_JOB == 1)
	if (vendor_ais_flow_fd[0] != -1) {
		//DBG_DUMP("close %s\r\n", DEFAULT_DEVICE);
		VENDOR_AIS_FLOW_CORE_CFG cfg;
		cfg.schd = schd;
		if (KFLOW_AI_IOCTL(vendor_ais_flow_fd[0], VENDOR_AIS_FLOW_IOC_CORE_CFGSCHD, &cfg) < 0) {
			fprintf(stderr, "VENDOR_AIS_FLOW_IOC_CORE_CFGSCHD fail\r\n");
			vendor_ai_errno_location();
			return HD_ERR_NG;
		}
	}
#else
#endif

	return HD_OK;
}




