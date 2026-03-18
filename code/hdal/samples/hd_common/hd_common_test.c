/**
 * @file hd_common_test.c
 * @brief test common memory APIs.
 * @author Lincy Lin
 * @date in the year 2018
 */
#if defined(__LINUX)
#define _GNU_SOURCE
#include <sched.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#if defined(__LINUX)
#include <signal.h>
#include <pthread.h>
#endif
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_common.h"
#include <kwrap/examsys.h>

#define DEBUG_MENU 		1
#define TEST_DDR2 		0
#define MAX_CORE_NUM 	2

#define CHKPNT    printf("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#define DBG_ERR(fmtstr, args...)  printf("\033[31mERR:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_WRN(fmtstr, args...)  printf("\033[33mWRN:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)


static UINT32 g_pid = 0;  //0:server, 1:client
static int    g_quit = 0;

static int mem_init(UINT32 cfg)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	UINT32                 i = 0;

	if (cfg == 0) {
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = 0x200;
		mem_cfg.pool_info[i].blk_cnt = 3;
		mem_cfg.pool_info[i].ddr_id = DDR_ID0;
		i+=1;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = 0x200000;
		mem_cfg.pool_info[i].blk_cnt = 3;
		mem_cfg.pool_info[i].ddr_id = DDR_ID0;
		i+=1;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = 0x300000;
		mem_cfg.pool_info[i].blk_cnt = 3;
		mem_cfg.pool_info[i].ddr_id = DDR_ID0;
		i+=1;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_OSG_POOL;
		mem_cfg.pool_info[i].blk_size = 0x100000;
		mem_cfg.pool_info[i].blk_cnt = 2;
		mem_cfg.pool_info[i].ddr_id = DDR_ID0;
		i+=1;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_OSG_POOL;
		mem_cfg.pool_info[i].blk_size = 0x200000;
		mem_cfg.pool_info[i].blk_cnt = 2;
		mem_cfg.pool_info[i].ddr_id = DDR_ID0;
		i+=1;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_GFX_POOL;
		mem_cfg.pool_info[i].blk_size = 0x400;
		mem_cfg.pool_info[i].blk_cnt = 255;
		mem_cfg.pool_info[i].ddr_id = DDR_ID0;
		i+=1;
#if TEST_DDR2
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = 0x200000;
		mem_cfg.pool_info[i].blk_cnt = 3;
		mem_cfg.pool_info[i].ddr_id = DDR_ID1;
		i+=1;
		mem_cfg.pool_info[i].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[i].blk_size = 0x300000;
		mem_cfg.pool_info[i].blk_cnt = 3;
		mem_cfg.pool_info[i].ddr_id = DDR_ID1;
		i+=1;
#endif
		ret = hd_common_mem_init(&mem_cfg);
		if (HD_OK != ret) {
			printf("hd_common_mem_init err: %d\r\n", ret);
		}
	} else {
		ret = hd_common_mem_init(NULL);
	}
	return ret;
}

static HD_RESULT mem_exit(void)
{
	return hd_common_mem_uninit();
}


static HD_RESULT test_get_block_from_common(void)
{
	HD_COMMON_MEM_VB_BLK blk;
	ULONG             pa, va;
	UINT32            blk_size;
	HD_COMMON_MEM_DDR_ID ddr_id;
	HD_COMMON_MEM_DDR_ID max_ddr;
	HD_RESULT         ret, func_ret = HD_OK;
	HD_VIDEO_FRAME    my_frame = {0};


#if TEST_DDR2
	max_ddr = DDR_ID1;
#else
	max_ddr = DDR_ID0;
#endif
	for (ddr_id=0; ddr_id <= max_ddr; ddr_id++) {
		printf("\r\ntest_get_block_from_common ddr_id %d\r\n", ddr_id);

		system("cat /proc/hdal/comm/info");

		my_frame.sign = MAKEFOURCC('V','F','R','M');
		my_frame.dim.w = 1920;
		my_frame.dim.h = 1080;
		my_frame.loff[0]= 1920;
		my_frame.loff[1]= 1920;
		my_frame.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		blk_size = hd_common_mem_calc_buf_size((void *)&my_frame);
		printf("blk_size = 0x%x\r\n", (int)blk_size);
		blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, ddr_id);
		if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("get block fail\r\n");
			return HD_ERR_NG;
		}
		printf("blk = 0x%lx\r\n", (ULONG)blk);
		pa = hd_common_mem_blk2pa(blk);
		if (pa == 0) {
			printf("blk2pa fail, blk = 0x%x\r\n", (int)blk);
			func_ret = HD_ERR_SYS;
			goto blk2pa_err;
		}
		printf("pa = 0x%lx\r\n", (ULONG)pa);
		if (pa > 0) {
			va = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size);
			if (va == 0) {
				func_ret = HD_ERR_SYS;
				goto map_err;
			}
			printf("va = 0x%lx\r\n", (ULONG)va);
			memset((void*)va, 0x11, blk_size);
			printf("va = 0x%lx\r\n", (ULONG)va);
			ret = hd_common_mem_flush_cache((void*)va, blk_size);
			printf("hd_common_mem_flush_cache ret = 0x%x\r\n", ret);
			hd_common_mem_munmap((void*)va, blk_size);
		}
		ret = vendor_common_mem_lock_block(blk);
		if (HD_OK != ret) {
			printf("lock blk fail %d\r\n", ret);
			return HD_ERR_NG;
		}
		printf("user lock block\r\n");
		system("cat /proc/hdal/comm/info");
		ret = hd_common_mem_release_block(blk);
		if (HD_OK != ret) {
			printf("release blk fail %d\r\n", ret);
			return HD_ERR_NG;
		}
	blk2pa_err:
	map_err:
		system("cat /proc/hdal/comm/info");
		ret = hd_common_mem_release_block(blk);
		if (HD_OK != ret) {
			printf("release blk fail %d\r\n", ret);
			return HD_ERR_NG;
		}
	}
	return func_ret;
}

static HD_RESULT test_get_block_from_osg(void)
{
	HD_COMMON_MEM_VB_BLK blk;
	ULONG             pa, va;
	UINT32            blk_size = 0x100000;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT         ret, func_ret = HD_OK;


	printf("\r\ntest_get_block_from_osg\r\n");
	system("cat /proc/hdal/comm/info");

	blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, blk_size, ddr_id);
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail\r\n");
		return HD_ERR_NG;
	}
	printf("blk = 0x%lx\r\n", (ULONG)blk);
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("blk2pa fail, blk = 0x%lx\r\n", (ULONG)blk);
		func_ret = HD_ERR_SYS;
		goto blk2pa_err;
	}
	printf("pa = 0x%lx\r\n", (ULONG)pa);
	if (pa > 0) {
		va = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size);
		if (va == 0) {
			func_ret = HD_ERR_SYS;
			goto map_err;
		}
		printf("va = 0x%lx\r\n", (ULONG)va);
		memset((void*)va, 0x11, blk_size);
		printf("va = 0x%lx\r\n", (ULONG)va);
		hd_common_mem_flush_cache((void*)va, blk_size);
		hd_common_mem_munmap((void*)va, blk_size);
	}
blk2pa_err:
map_err:
	system("cat /proc/hdal/comm/info");
	ret = hd_common_mem_release_block(blk);
	if (HD_OK != ret) {
		printf("release blk fail %d\r\n", ret);
		return HD_ERR_NG;
	}
	return func_ret;
}

static HD_RESULT test_alloc(HD_COMMON_MEM_DDR_ID ddr_id)
{
	void                 *va;
	ULONG                pa;
	UINT32               size = 0x200000;
	HD_RESULT            ret;

	printf("\r\ntest_alloc\r\n");
	ret = hd_common_mem_alloc("osg1", (UINTPTR *)&pa, (void **)&va, size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		return HD_ERR_NG;
	}
	printf("pa = 0x%lx, va = 0x%lx\r\n", (ULONG)pa, (ULONG)va);
	memset((void*)va, 0x33, size);
	system("cat /proc/hdal/comm/info");
	ret = hd_common_mem_free(pa, (void *)va);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (ULONG)pa, (ULONG)va);
		return HD_ERR_NG;
	}
	printf("free_mem\r\n\r\n");
	system("cat /proc/hdal/comm/info");
	return HD_OK;
}

static HD_RESULT test_alloc_max_pools(HD_COMMON_MEM_DDR_ID ddr_id)
{
	#define MAX_POOL_CNT 1000
	void                 *va[MAX_POOL_CNT] = {0};
	ULONG                pa[MAX_POOL_CNT] = {0};
	UINT32               size = 0x100;
	HD_RESULT            ret = HD_OK;
	UINT32               i;

	printf("\r\ntest_alloc_max_pools\r\n");
	for (i = 0;i< MAX_POOL_CNT;i++) {
		ret = hd_common_mem_alloc("osg1", (UINTPTR *)&pa[i], (void **)&va[i], size, ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
			goto err_alloc;
		}
	}
	system("cat /proc/hdal/comm/info");
	for (i = 0;i< MAX_POOL_CNT;i++) {
		if (pa[i] != 0) {
			hd_common_mem_free(pa[i], va[i]);
		}
	}
	system("cat /proc/hdal/comm/info");
err_alloc:
	return ret;
}

static HD_RESULT test_alloc_temp(HD_COMMON_MEM_DDR_ID ddr_id)
{
	void                 *va[3];
	ULONG                pa[3];
	UINT32               i, size = 0x200000;
	HD_RESULT            ret;

	printf("\r\ntest_alloc_temp\r\n");
	for (i = 0;i < 3;i++) {
		ret = hd_common_mem_alloc("NVTMPP_TEMP", (UINTPTR *)&pa[i], (void **)&va[i], size, ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
			return HD_ERR_NG;
		}
		printf("pa[%d] = 0x%lx, va[%d] = 0x%lx\r\n", (int)i, (ULONG)pa[i], (int)i, (ULONG)va[i]);
		memset((void*)va[i], 0x33, size);
	}
	system("cat /proc/hdal/comm/info");
	for (i = 0;i < 3;i++) {
		ret = hd_common_mem_free(pa[i], (void *)va[i]);
		if (ret != HD_OK) {
			printf("err:free pa[%d] = 0x%lx, va[%d] = 0x%lx\r\n", (int)i, (ULONG)pa[i], (int)i, (ULONG)va[i]);
			return HD_ERR_NG;
		}
	}
	printf("free_mem\r\n\r\n");
	system("cat /proc/hdal/comm/info");
	return HD_OK;
}

static HD_RESULT test_va2pa(void)
{
	void                 *va1, *va2;
	ULONG                pa1, pa2;
	UINT32               size = 0x200000;
	UINT32               va1_offset = 0x1010, va2_offset = 0x2020;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT            ret, func_ret = HD_OK;
	HD_COMMON_MEM_VIRT_INFO vir_meminfo = {0};

	printf("\r\ntest_va2pa\r\n");
	ret = hd_common_mem_alloc("test1", (UINTPTR *)&pa1, (void **)&va1, size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		return HD_ERR_SYS;
	}
	printf("pa1 = 0x%lx, va1 = 0x%lx\r\n", (ULONG)pa1, (ULONG)va1);
	ret = hd_common_mem_alloc("test2", (UINTPTR *)&pa2, (void **)&va2, size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		func_ret = HD_ERR_SYS;
		goto alloc_err;
	}
	printf("pa2 = 0x%lx, va2 = 0x%lx\r\n", (ULONG)pa2, (ULONG)va2);
	system("cat /proc/hdal/comm/info");
	vir_meminfo.va = (void *)((ULONG)va1+va1_offset);
	if (hd_common_mem_get(HD_COMMON_MEM_PARAM_VIRT_INFO, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	if (vir_meminfo.pa != pa1+va1_offset) {
		printf("err:vir_meminfo.pa = 0x%lx != 0x%lx\r\n", (ULONG)vir_meminfo.pa, (ULONG)(pa1+va1_offset));
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	vir_meminfo.va = (void *)((ULONG)va2+va2_offset);
	if (hd_common_mem_get(HD_COMMON_MEM_PARAM_VIRT_INFO, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	if (vir_meminfo.pa != pa2+va2_offset) {
		printf("err:vir_meminfo.pa = 0x%lx != 0x%lx\r\n", (ULONG)vir_meminfo.pa, (ULONG)(pa2+va2_offset));
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
va2pa_err:
	ret = hd_common_mem_free(pa2, (void *)va2);
	if (ret != HD_OK) {
		printf("err:free pa2 = 0x%lx, va2 = 0x%lx\r\n", (ULONG)pa2, (ULONG)va2);
	}
alloc_err:
	ret = hd_common_mem_free(pa1, (void *)va1);
	if (ret != HD_OK) {
		printf("err:free pa1 = 0x%lx, va1 = 0x%lx\r\n", (ULONG)pa1, (ULONG)va1);
	}
	return func_ret;
}



static HD_RESULT test_multi_va_map_same_phy(void)
{
	void                 *va1, *va2;
	ULONG                pa1, pa2;
	#if defined(__LINUX)
	void                 *va3;
	ULONG                pa3;
	#endif
	UINT32               size = 0x200000;
	UINT32               va1_offset = 0x1010, va2_offset = 0x2020;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT            ret, func_ret = HD_OK;
	HD_COMMON_MEM_VIRT_INFO vir_meminfo = {0};

	printf("\r\ntest_multi_va_map_same_phy\r\n");
	ret = hd_common_mem_alloc("test1", (UINTPTR *)&pa1, (void **)&va1, size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		return HD_ERR_SYS;
	}
	printf("pa1 = 0x%lx, va1 = 0x%lx\r\n", (ULONG)pa1, (ULONG)va1);
	pa2 = pa1;
	va2 = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa1, size);
	if (va2 == 0) {
		func_ret = HD_ERR_SYS;
		goto map_err;
	}
	printf("pa2 = 0x%lx, va2 = 0x%lx\r\n", (ULONG)pa2, (ULONG)va2);
	system("cat /proc/hdal/comm/info");
	vir_meminfo.va = (void *)((ULONG)va1+va1_offset);
	if (hd_common_mem_get(HD_COMMON_MEM_PARAM_VIRT_INFO, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	if (vir_meminfo.pa != pa1+va1_offset) {
		printf("err:vir_meminfo.pa = 0x%lx != 0x%lx\r\n", (ULONG)vir_meminfo.pa, (ULONG)(pa1+va1_offset));
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	vir_meminfo.va = (void *)((ULONG)va2+va2_offset);
	if (hd_common_mem_get(HD_COMMON_MEM_PARAM_VIRT_INFO, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	if (vir_meminfo.pa != pa2+va2_offset) {
		printf("err:vir_meminfo.pa = 0x%lx != 0x%lx\r\n", (ULONG)vir_meminfo.pa, (ULONG)(pa2+va2_offset));
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	printf("va2 = 0x%lx, is cached %d\r\n", (ULONG)va2, vir_meminfo.cached);
	if (!vir_meminfo.cached) {
		printf("va 0x%lx is not cached\r\n", (ULONG)vir_meminfo.va);
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	#if defined(__LINUX)
	pa3 = pa1;
	va3 = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_NONCACHE, pa3, size);
	if (va3 == 0) {
		func_ret = HD_ERR_SYS;
		goto map_err;
	}
	printf("pa3 = 0x%lx, va3 = 0x%lx\r\n", (ULONG)pa3, (ULONG)va3);
	system("cat /proc/hdal/comm/info");
	vir_meminfo.va = (void *)(ULONG)va3;
	if (hd_common_mem_get(HD_COMMON_MEM_PARAM_VIRT_INFO, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	printf("va3 = 0x%lx, is cached %d\r\n", (ULONG)va3, vir_meminfo.cached);
	if (vir_meminfo.cached) {
		printf("va 0x%lx is not non-cached\r\n", (ULONG)vir_meminfo.va);
		func_ret = HD_ERR_SYS;
		goto va2pa_err;
	}
	hd_common_mem_munmap((void*)va3, size);
	#endif
va2pa_err:
	hd_common_mem_munmap((void*)va2, size);
map_err:
	ret = hd_common_mem_free(pa1, (void *)va1);
	if (ret != HD_OK) {
		printf("err:free pa1 = 0x%lx, va1 = 0x%lx\r\n", (ULONG)pa1, (ULONG)va1);
	}
	return func_ret;
}

int test_timestamp(UINT32 count)
{
	UINT64     time64_old = 0, time64 = 0;
	UINT64    *p_time64_list;
	UINT64     time32_old = 0, time32 = 0;
	UINT32    *p_time32_list;
	char      *p_buf;
	int        ret = HD_OK;
	UINT32     i;

	p_buf = malloc(count * sizeof(UINT64));
	if (p_buf == NULL)
		return HD_ERR_SYS;
	// test us
	p_time64_list = (UINT64 *)p_buf;
	for (i = 0; i < count; i++) {
		time64 = hd_gettime_us();
		if (time64 < time64_old) {
			printf("error time = %lld us, prev time = %lld us\r\n", time64, time64_old);
			ret = HD_ERR_SYS;
			goto err_time;
		}
		*p_time64_list++ = time64;
		time64_old = time64;
	}
	p_time64_list = (UINT64 *) p_buf;
	for (i = 0; i < count; i++) {
		printf("%d time = %lld us\r\n", (int)i, *p_time64_list++);
	}
	// test ms
	p_time32_list = (UINT32 *)p_buf;
	for (i = 0; i < count; i++) {
		time32 = hd_gettime_ms();
		if (time32 < time32_old) {
			printf("error time = %d ms, prev time = %d ms\r\n", (int)time32, (int)time32_old);
			ret = HD_ERR_SYS;
			goto err_time;
		}
		*p_time32_list++ = time32;
		time32_old = time32;
	}
	p_time32_list = (UINT32 *) p_buf;
	for (i = 0; i < count; i++) {
		printf("%d time = %d ms\r\n", (int)i, (int)*p_time32_list++);
	}

err_time:
	free(p_buf);
	return ret;
}


int test_cacheflush_perf(void)
{
	void                 *va;
	ULONG                pa, i;
	UINT32               size = 0x2000000;
	UINT32               flush_size;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT            ret;
	UINT64               time_b, time_e;

	#if defined(__LINUX)
	{
		cpu_set_t mask;
		int curr_bind_cpu;
		static UINT32  cpu_id = 1;

		curr_bind_cpu = cpu_id;
		CPU_ZERO(&mask);
		CPU_SET(curr_bind_cpu, &mask);
		if (sched_setaffinity(0, sizeof(mask), &mask) == -1)  {
			printf("sched_setaffinity fail %d\r\n", cpu_id);
			return 0;
		}
		cpu_id ++;
		if (cpu_id >= MAX_CORE_NUM) {
			cpu_id = 0;
		}
		sleep(1);
	}
	#endif
	printf("\r\ntest_cacheflush_perf\r\n");
	ret = hd_common_mem_alloc("test_1", (UINTPTR *)&pa, (void **)&va, size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		return HD_ERR_NG;
	}
	flush_size = 0x08000;
	for (i = 0; i < 15; i++) {
		memset(va, 0x00, 0x40000);
		time_b = hd_gettime_us();
		hd_common_mem_flush_cache((void*)va, flush_size);
		time_e = hd_gettime_us();
		printf("cacheflush size 0x%x, time = %lld us\r\n", (int)flush_size, time_e-time_b);
		flush_size = flush_size << 1;
		if (flush_size > size) {
			break;
		}
	}
	flush_size = 0x08000;
	for (i = 0; i < 15; i++) {
		memset(va, 0x00, 0x40000);
		time_b = hd_gettime_us();
		vendor_common_mem_cache_sync((void*)va, flush_size, VENDOR_COMMON_MEM_DMA_TO_DEVICE);
		time_e = hd_gettime_us();
		printf("cacheclean size 0x%x, time = %lld us\r\n", (int)flush_size, time_e-time_b);
		flush_size = flush_size << 1;
		if (flush_size > size) {
			break;
		}
	}
	flush_size = 0x08000;
	for (i = 0; i < 15; i++) {
		memset(va, 0x00, 0x40000);
		time_b = hd_gettime_us();
		vendor_common_mem_cache_sync((void*)va, flush_size, VENDOR_COMMON_MEM_DMA_FROM_DEVICE);
		time_e = hd_gettime_us();
		printf("cacheinvalid size 0x%x, time = %lld us\r\n", (int)flush_size, time_e-time_b);
		flush_size = flush_size << 1;
		if (flush_size > size) {
			break;
		}
	}
	flush_size = 0x08000;
	for (i = 0; i < 15; i++) {
		memset(va, 0x00, 0x40000);
		time_b = hd_gettime_us();
		vendor_common_mem_cache_sync_all((void*)va, flush_size, VENDOR_COMMON_MEM_DMA_BIDIRECTIONAL);
		time_e = hd_gettime_us();
		printf("cache_sync_all size 0x%x, time = %lld us\r\n", (int)flush_size, time_e-time_b);
		flush_size = flush_size << 1;
		if (flush_size > size) {
			break;
		}
	}
	ret = hd_common_mem_free(pa, (void *)va);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (ULONG)pa, (ULONG)va);
	}
	return HD_OK;
}


static int mem_init_with_same_type_and_size(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[0].blk_size = 0x200000;
	mem_cfg.pool_info[0].blk_cnt = 2;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = 0x300000;
	mem_cfg.pool_info[1].blk_cnt = 2;
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;
	mem_cfg.pool_info[2].type = HD_COMMON_MEM_OSG_POOL;
	mem_cfg.pool_info[2].blk_size = 0x100000;
	mem_cfg.pool_info[2].blk_cnt = 4;
	mem_cfg.pool_info[2].ddr_id = DDR_ID0;
	mem_cfg.pool_info[3].type = HD_COMMON_MEM_OSG_POOL;
	mem_cfg.pool_info[3].blk_size = 0x100000;
	mem_cfg.pool_info[3].blk_cnt = 4;
	mem_cfg.pool_info[3].ddr_id = DDR_ID0;


	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
	}
	return ret;
}

static int test_err(void)
{
	HD_RESULT ret;
	void      *map_addr;
	HD_COMMON_MEM_VB_BLK  blk;
	void                 *va = NULL;
	ULONG                pa;
	UINT32               size;

	// test error handling of free invalid pa
	printf("\r\ntest error handling of free invalid pa\r\n");
	hd_common_mem_free(0x100, (void *)0);

	size = 0x10000;
	ret = hd_common_mem_alloc("test_1", (UINTPTR *)&pa, (void **)&va, size, DDR_ID0);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, DDR_ID0);
		return HD_ERR_NG;
	}

	// test error handling of free invalid va
	printf("\r\ntest error handling of free invalid va\r\n");
	hd_common_mem_free(pa, (void *)0x100);

	ret = hd_common_mem_free(pa, (void *)va);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (ULONG)pa, (ULONG)va);
	}


	mem_exit();
	ret = hd_common_uninit();
	if(ret != HD_OK) {
		printf("common_uninit fail=%d\n", ret);
		return HD_ERR_NG;
	}
	map_addr = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, (UINT32)0x100000 , (UINT32)0x100000);
	if (map_addr != NULL) {
		printf("munmap err handling fail\r\n");
		hd_common_mem_munmap(map_addr, 0x100000);
		return HD_ERR_NG;
	}
	ret = hd_common_mem_munmap((void *)0x100000 , 0x100000);
	if (ret >= 0) {
		printf("munmap err handling fail\r\n");
		return HD_ERR_NG;
	}
	blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, 0x100000, DDR_ID0);
	if (blk != HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get_block err handling fail\r\n");
		ret = hd_common_mem_release_block(blk);
		if (ret >= 0) {
			printf("rel_block err handling fail\r\n");
		}
		return HD_ERR_NG;
	}
	ret = hd_common_mem_release_block(blk);
	if (ret >= 0) {
		printf("rel_block err handling fail\r\n");
		return HD_ERR_NG;
	}
	ret = hd_common_init(g_pid);
	if(ret != HD_OK) {
		printf("init fail=%d\r\n", ret);
		return HD_ERR_NG;
	}
	ret = mem_init_with_same_type_and_size();
	if (ret >= 0) {
		printf("mem init handling fail\r\n");
		return HD_ERR_NG;
	}
	ret = hd_common_uninit();
	if(ret != HD_OK) {
		printf("common_uninit fail=%d\n", ret);
		return HD_ERR_NG;
	}
	ret = hd_common_init(g_pid);
	if(ret != HD_OK) {
		printf("init fail=%d\r\n", ret);
		return HD_ERR_NG;
	}
	mem_init(g_pid);
	ret = hd_common_mem_alloc("dspmmz", NULL, (void **)&va, 0x100000, DDR_ID0);
	if (HD_OK == ret) {
		printf("mem alloc ,phy_addr error handling fail\r\n");
		return HD_ERR_NG;
	}
	ret = hd_common_mem_alloc("dspmmz", (UINTPTR *)&pa, NULL, 0x100000, DDR_ID0);
	if (HD_OK == ret) {
		printf("mem alloc ,virt_addr error handling fail\r\n");
		return HD_ERR_NG;
	}
	ret = hd_common_mem_alloc("dspmmz", (UINTPTR *)&pa, (void **)&va, 0xF0000000, DDR_ID0);
	if (HD_OK == ret) {
		printf("mem alloc , size error handling fail\r\n");
		system("cat /proc/hdal/comm/info");
		ret = hd_common_mem_free(pa, va);
		if (ret != HD_OK) {
			printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (ULONG)pa, (ULONG)va);
		}
		return HD_ERR_NG;
	}
	ret = hd_common_mem_free((UINTPTR)NULL, NULL);
    if (HD_OK == ret) {
        printf("Error handle test for free an invalid address: fail %d\r\n", (int)(ret));
        return HD_ERR_NG;
    }
	// test invalid va
	ret = hd_common_mem_flush_cache((void*)0x70000000, 0x100);
	if (HD_OK == ret) {
        printf("Error handle test for flush_cache an invalid va: fail %d\r\n", (int)(ret));
        return HD_ERR_NG;
    }
	printf("Error handle test for flush_cache an invalid va: %d\r\n", (int)(ret));
	// test invalid va
	ret = hd_common_mem_cache_sync((void*)0x70000000, 0x100, HD_COMMON_MEM_DMA_TO_DEVICE);
	if (HD_OK == ret) {
        printf("Error handle test for cache_sync an invalid va: fail %d\r\n", (int)(ret));
        return HD_ERR_NG;
    }
	printf("Error handle test for cache_sync an invalid va: %d\r\n", (int)(ret));
	return HD_OK;
}

static int test_random_alloc_free(HD_COMMON_MEM_DDR_ID ddr_id)
{
	void                 *va[10];
	ULONG                pa[10];
	UINT32               size[10] = { 0x200000, 0x400000, 0x450000, 0x210000, 0x600000, 0x3000000, 0x100, 0x9000, 0x50000, 0x500000};
	int                  i;
	HD_RESULT            ret;
	char                 tmp_str[10];

	printf("\r\test_random_alloc_free\r\n");

	for (i = 0;i < 10; i++) {
		sprintf(tmp_str, "test%d", i);
		ret = hd_common_mem_alloc(tmp_str, (UINTPTR *)&pa[i], (void **)&va[i], size[i], ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", (int)size[i], ddr_id+1);
			return HD_ERR_NG;
		}
	}
	system("cat /proc/hdal/comm/info");

	ret = hd_common_mem_free(pa[5], va[5]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	ret = hd_common_mem_free(pa[6], va[6]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	ret = hd_common_mem_free(pa[7], va[7]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }

	system("cat /proc/hdal/comm/info");
	size[5] = 0x4000000;
	size[6] = 0x100000;
	size[7] = 0x8000;
	for (i = 5;i <= 7; i++) {
		sprintf(tmp_str, "test%d", i);
		ret = hd_common_mem_alloc(tmp_str, (UINTPTR *)&pa[i], (void **)&va[i], size[i], ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", (int)size[i], ddr_id+1);
			return HD_ERR_NG;
		}
	}
	system("cat /proc/hdal/comm/info");
	ret = hd_common_mem_free(pa[5], va[5]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	ret = hd_common_mem_free(pa[6], va[6]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	ret = hd_common_mem_free(pa[7], va[7]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	system("cat /proc/hdal/comm/info");
	size[5] = 0x1000000;
	size[6] = 0x70000;
	size[7] = 0x9000;
	for (i = 5;i <= 7; i++) {
		sprintf(tmp_str, "test%d", i);
		ret = hd_common_mem_alloc(tmp_str, (UINTPTR *)&pa[i], (void **)&va[i], size[i], ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", (int)size[i], ddr_id+1);
			return HD_ERR_NG;
		}
	}
	system("cat /proc/hdal/comm/info");

	ret = hd_common_mem_free(pa[5], va[5]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	ret = hd_common_mem_free(pa[6], va[6]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	ret = hd_common_mem_free(pa[7], va[7]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	system("cat /proc/hdal/comm/info");
	size[5] = 0x2000000;
	size[6] = 0x100000;
	size[7] = 0x19000;
	for (i = 5;i <= 7; i++) {
		sprintf(tmp_str, "test%d", i);
		ret = hd_common_mem_alloc(tmp_str, (UINTPTR *)&pa[i], (void **)&va[i], size[i], ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", (int)size[i], ddr_id+1);
			return HD_ERR_NG;
		}
	}
	system("cat /proc/hdal/comm/info");

	ret = hd_common_mem_free(pa[5], va[5]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	ret = hd_common_mem_free(pa[6], va[6]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	ret = hd_common_mem_free(pa[7], va[7]);
	if (HD_OK != ret) {
        printf("Error free \r\n");
        return HD_ERR_NG;
    }
	system("cat /proc/hdal/comm/info");
	size[5] = 0x3100000;
	size[6] = 0x40000;
	size[7] = 0x569000;
	for (i = 5;i <= 7; i++) {
		sprintf(tmp_str, "test%d", i);
		ret = hd_common_mem_alloc(tmp_str, (UINTPTR *)&pa[i], (void **)&va[i], size[i], ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", (int)size[i], ddr_id+1);
			return HD_ERR_NG;
		}
	}
	system("cat /proc/hdal/comm/info");
	for (i = 0;i < 10; i++) {
		ret = hd_common_mem_free(pa[i], va[i]);
		if (HD_OK != ret) {
	        printf("Error free \r\n");
	        return HD_ERR_NG;
    	}
	}
	system("cat /proc/hdal/comm/info");

	return HD_OK;
}

#if 0
static int test_get_bridge_mem(void)
{
	#define BRIDGE_FOURCC 0x47445242 ///< MAKEFOURCC('B', 'R', 'D', 'G');
	#define BRIDGE_MAX_OPT_CNT 128
	// bridge memory description !! (DO NOT MODIFY ANY MEMBER IN BRIDGE_DESC and BRIDGE_BOOT_OPTION)
	typedef struct _BRIDGE_BOOT_OPTION {
		unsigned int tag; //a fourcc tag
		unsigned long val; //the value
	} BRIDGE_BOOT_OPTION;

	typedef struct _BRIDGE_DESC {
		unsigned int bridge_fourcc;     ///< always BRIDGE_FOURCC
		unsigned int bridge_size;       ///< sizeof(BRIDGE_DESC) for check if struct match on rtos and linux
		unsigned long phy_addr;          ///< address of bridge memory described on fdt
		unsigned int phy_size;          ///< size of whole bridge memory described on fdt
		BRIDGE_BOOT_OPTION opts[BRIDGE_MAX_OPT_CNT]; ///< boot options from rtos
	} BRIDGE_DESC;

	int i;
	BRIDGE_DESC *p_bridge = NULL;
	VENDOR_COMM_BRIDGE_MEM bridge_mem = {0};
	if (HD_OK != vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_BRIDGE_MEM, &bridge_mem)) {
		return HD_ERR_NG;
	}

	p_bridge = (BRIDGE_DESC *)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, bridge_mem.phys_addr, bridge_mem.size);

	if (p_bridge == NULL) {
		return HD_ERR_NG;
	}
	if (p_bridge->bridge_fourcc != BRIDGE_FOURCC) {
		printf("invalid bridge format.\n");
		hd_common_mem_munmap((void*)p_bridge, bridge_mem.size);
		return HD_ERR_BAD_DATA;
	}
	if (p_bridge->bridge_size != sizeof(BRIDGE_DESC)) {
		printf("invalid bridge version, size not matched rtos(%d)!=linux(%d).\n", (int)p_bridge->bridge_size, (int)sizeof(BRIDGE_DESC));
		hd_common_mem_munmap((void*)p_bridge, bridge_mem.size);
		return HD_ERR_BAD_DATA;
	}

	// find a tag
	#define SENSOR_PRESET_EXPT 0x54455053 //MAKEFOURCC('S', 'P', 'E', 'T')
	for (i = 0; i < BRIDGE_MAX_OPT_CNT; i++) {
		if (p_bridge->opts[i].tag == SENSOR_PRESET_EXPT) {
			printf("find tag SENSOR_PRESET_EXPT = 0x%lX\n", p_bridge->opts[i].val);
			break;
		} else if (p_bridge->opts[i].tag == 0) {
			printf("unable to find tag: 0x%08X\n", SENSOR_PRESET_EXPT);
			break;
		}
	}
	hd_common_mem_munmap((void*)p_bridge, bridge_mem.size);
	return HD_OK;
}
#endif


static int test_alloc_maxblk(HD_COMMON_MEM_DDR_ID ddr_id)
{
	void                 *va;
	ULONG                pa;
	UINT32               size, flush_size;
	HD_RESULT            ret;
	VENDOR_COMM_MAX_FREE_BLOCK max_free = {0};
	ULONG                test_va;


	printf("\r\ntest_alloc_maxblk\r\n");
	system("cat /proc/hdal/comm/info");
	max_free.ddr = ddr_id;
	if (HD_OK != vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free)) {
		return HD_ERR_NG;
	}
	size = max_free.size;
	ret = hd_common_mem_alloc("test_1", (UINTPTR *)&pa, (void **)&va, size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		return HD_ERR_NG;
	}
	test_va = (ULONG)va;
	flush_size = 0x1000;
	while (test_va < (ULONG)va + size) {
		printf("test_va 0x%lx\r\n", test_va);
		memset((void*)test_va, 0x00, flush_size);
		hd_common_mem_flush_cache((void*)test_va, flush_size);
		test_va +=0x240000;
	}
	system("cat /proc/hdal/comm/info");


	ret = hd_common_mem_free(pa, (void *)va);
	if (ret != HD_OK) {
		printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (ULONG)pa, (ULONG)va);
	}

	// test error handling of alloc exceeds max free block
	size = max_free.size + 0x100000;
	ret = hd_common_mem_alloc("test_exceed_max", (UINTPTR *)&pa, (void **)&va, size, ddr_id);
	if (ret == HD_OK) {
		printf("err:test_exceed_max fail size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		return HD_ERR_NG;
	}

	return HD_OK;
}

static int test_get_common_pool_range(HD_COMMON_MEM_DDR_ID ddr_id)
{
	VENDOR_COMM_POOL_RANGE pool_range = {0};
	void                   *va;

	pool_range.ddr = ddr_id;
	if (HD_OK != vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_COMM_POOL_RANGE, &pool_range)) {
		return HD_ERR_NG;
	}
	printf("pool_range pa= 0x%lx, size=0x%x\r\n", (ULONG)pool_range.phys_addr, (int)pool_range.size);
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pool_range.phys_addr, pool_range.size);
	if (va == NULL) {
		return HD_ERR_NG;
	}
	printf("pool_range va= 0x%lx\r\n", (ULONG)va);
	hd_common_mem_munmap(va, pool_range.size);
	return HD_OK;
}

#if defined(__LINUX)
static void *signal_thread(void *arg)
{
	UINT32 count = 0;

	while (!g_quit) {
		usleep(50);
		setuid(0);
		count++;
		if (count % 100 == 0) {
			printf("signal count = %d\r\n", count);
		}
	}
	return 0;
}
#endif

static HD_RESULT test_create_destory_pool(void)
{
	HD_COMMON_MEM_VB_BLK blk, blk2;
	ULONG             pa, va;
	UINT32            blk_size = 0x100000, blk_cnt = 2, i;
	HD_COMMON_MEM_DDR_ID ddr_id;
	HD_COMMON_MEM_DDR_ID max_ddr;
	HD_RESULT         ret, func_ret = HD_OK;
	VENDOR_COMMON_MEM_VB_POOL pool;


#if TEST_DDR2
	max_ddr = DDR_ID1;
#else
	max_ddr = DDR_ID0;
#endif
	for (ddr_id=0; ddr_id <= max_ddr; ddr_id++) {
		printf("\r\ntest_create_destory_pool ddr_id %d\r\n", ddr_id);
		pool = vendor_common_mem_create_pool("test_c", blk_size, blk_cnt, ddr_id);
		if (pool == VENDOR_COMMON_MEM_VB_INVALID_POOL) {
			printf("create pool fail\r\n");
			return HD_ERR_NG;
		}
		system("cat /proc/hdal/comm/info");
		blk = hd_common_mem_get_block(pool, blk_size, ddr_id);
		if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("get block fail\r\n");
			return HD_ERR_NG;
		}
		printf("blk = 0x%lx\r\n", (ULONG)blk);
		pa = hd_common_mem_blk2pa(blk);
		if (pa == 0L) {
			printf("blk2pa fail, blk = 0x%lx\r\n", (ULONG)blk);
			func_ret = HD_ERR_SYS;
			goto blk2pa_err;
		}
		printf("pa = 0x%lx\r\n", pa);
		if (pa > 0L) {
			va = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size);
			if (va == 0L) {
				func_ret = HD_ERR_SYS;
				goto map_err;
			}
			printf("va = 0x%lx\r\n", (ULONG)va);
			memset((void*)va, 0x11, blk_size);
			ret = hd_common_mem_flush_cache((void*)va, blk_size);
			printf("hd_common_mem_flush_cache ret = 0x%x\r\n", ret);
			hd_common_mem_munmap((void*)va, blk_size);
		}
		for (i= 0 ; i < 3; i++) {
			system("cat /proc/hdal/comm/info");
			blk2 = hd_common_mem_get_block(pool, blk_size, ddr_id);
			if (blk2 == HD_COMMON_MEM_VB_INVALID_BLK) {
				printf("get block fail\r\n");
				return HD_ERR_NG;
			}
			printf("blk2 = 0x%lx\r\n", (ULONG)blk2);
			system("cat /proc/hdal/comm/info");
			ret = hd_common_mem_release_block(blk2);
			if (HD_OK != ret) {
				printf("release blk2 fail %d\r\n", ret);
				return HD_ERR_NG;
			}
		}
	blk2pa_err:
	map_err:
		system("cat /proc/hdal/comm/info");
		ret = hd_common_mem_release_block(blk);
		if (HD_OK != ret) {
			printf("release blk fail %d\r\n", ret);
			return HD_ERR_NG;
		}
		vendor_common_mem_destroy_pool(pool);
	}
	return func_ret;
}

static int test_fixed_pool(void)
{
	void                 *va;
	ULONG                pa;
	HD_RESULT            ret;

	ret = vendor_common_mem_alloc_fixed_pool("sdio", (UINTPTR *)&pa, (void **)&va, 0x100000, DDR_ID0);
	if (ret != HD_OK) {
		return ret;
	}
	ret = vendor_common_mem_alloc_fixed_pool("fat", (UINTPTR *)&pa, (void **)&va, 0x200000, DDR_ID0);
	if (ret != HD_OK) {
		return ret;
	}
	return HD_OK;
}

static HD_RESULT test_if_hdal_mem(void)
{
	void                 *va1 = NULL, *va2 = NULL, *va3 = NULL, *va4 = NULL, *va5 = NULL, *va6 = NULL, *va7 = NULL;
	UINTPTR              pa1, pa2, pa3, pa4, pa5, pa6, pa7;
	UINT32               size = 0x200000;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT            ret, func_ret = HD_OK;
	VENDOR_COMM_MEM_VIRT_INFO2  vir_meminfo = {0};
	char                 test_mem[10];

	printf("\r\ntest_if_hdal_mem\r\n");

	// test malloc memory from hdal ddr0
	ret = hd_common_mem_alloc("test1", &pa1, (void **)&va1, size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		return HD_ERR_SYS;
	}

	// test malloc memory from hdal ddr0
	ret = hd_common_mem_alloc("test2", &pa2, (void **)&va2, size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	vir_meminfo.va = (void *)va1;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	printf("pa1 = 0x%lx, va1 = 0x%lx, is_hdal = %d\r\n", (ULONG)pa1, (ULONG)va1, (int)vir_meminfo.is_hdal);
	if (!vir_meminfo.is_hdal) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	vir_meminfo.va = (void *)va2;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	printf("pa2 = 0x%lx, va2 = 0x%lx, is_hdal = %d\r\n", (ULONG)pa2, (ULONG)va2, (int)vir_meminfo.is_hdal);
	if (!vir_meminfo.is_hdal) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}

	// test malloc memory from linux
	va3 = malloc(0x10000);
	vir_meminfo.va = va3;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	pa3 = vir_meminfo.pa;
	printf("pa3 = 0x%lx, va3 = 0x%lx, is_hdal = %d\r\n", (ULONG)pa3, (ULONG)va3, (int)vir_meminfo.is_hdal);
	if (vir_meminfo.is_hdal) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}

	// test malloc memory from linux
	va4 = malloc(0x200000);
	vir_meminfo.va = va4;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	pa4 = vir_meminfo.pa;
	printf("pa4 = 0x%lx, va4 = 0x%lx, is_hdal = %d\r\n", (ULONG)pa4, (ULONG)va4, (int)vir_meminfo.is_hdal);
	if (vir_meminfo.is_hdal) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}

#if TEST_DDR2
	// test malloc memory from hdal ddr1
	ddr_id = DDR_ID1;
	ret = hd_common_mem_alloc("test5", &pa5, (void **)&va5, size, ddr_id);
	if (ret != HD_OK) {
		printf("wrn:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		goto chk_err;
	}
	vir_meminfo.va = (void *)va5;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	printf("pa5 = 0x%lx, va5 = 0x%lx, is_hdal = %d\r\n", (ULONG)pa5, (ULONG)va5, (int)vir_meminfo.is_hdal);

	// test malloc memory from hdal ddr2
	//ddr_id = DDR_ID2;
	ddr_id = DDR_ID1;
	ret = hd_common_mem_alloc("test6", &pa6, (void **)&va6, size, ddr_id);
	if (ret != HD_OK) {
		printf("wrn:alloc size 0x%x, ddr %d\r\n", (int)size, ddr_id+1);
		goto chk_err;
	}
	vir_meminfo.va = (void *)va6;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	printf("pa6 = 0x%lx, va6 = 0x%lx, is_hdal = %d\r\n", (ULONG)pa6, (ULONG)va6, (int)vir_meminfo.is_hdal);
#endif

	// test stack memory
	va7 = test_mem;
	vir_meminfo.va = (void *)va7;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	pa7 = vir_meminfo.pa;
	printf("pa7 = 0x%lx, va7 = 0x%lx, is_hdal = %d\r\n", (ULONG)pa7, (ULONG)va7, (int)vir_meminfo.is_hdal);
	if (vir_meminfo.is_hdal) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	#if 0
	// test hogplug memory
	pa8 = 0x60000000;
	va8 = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa8, 0x1000);
	vir_meminfo.va = (void *)va8;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	printf("pa8 = 0x%lx, va8 = 0x%lx, is_hdal = %d, infopa = 0x%lx\r\n", (ULONG)pa8, (ULONG)va8, (int)vir_meminfo.is_hdal, (ULONG)vir_meminfo.pa);

	printf("test reserved memory\r\n");
	pa9 = 0x70000000;
	va9 = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa9, 0x1000);
	vir_meminfo.va = (void *)va9;
	if (vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_VIRT_INFO2, &vir_meminfo) != HD_OK) {
		func_ret = HD_ERR_SYS;
		goto chk_err;
	}
	printf("pa9 = 0x%lx, va9 = 0x%lx, is_hdal = %d, infopa = 0x%lx\r\n", (ULONG)pa9, (ULONG)va9, (int)vir_meminfo.is_hdal, (ULONG)vir_meminfo.pa);
	#endif

chk_err:
	if (va4) {
		free(va4);
	}
	if (va3) {
		free(va3);
	}
	if (va6) {
		ret = hd_common_mem_free(pa6, (void *)va6);
		if (ret != HD_OK) {
			printf("err:free pa6 = 0x%lx, va6 = 0x%lx\r\n", (ULONG)pa6, (ULONG)va6);
		}
	}
	if (va5) {
		ret = hd_common_mem_free(pa5, (void *)va5);
		if (ret != HD_OK) {
			printf("err:free pa5 = 0x%lx, va5 = 0x%lx\r\n", (ULONG)pa5, (ULONG)va5);
		}
	}
	if (va2) {
		ret = hd_common_mem_free(pa2, (void *)va2);
		if (ret != HD_OK) {
			printf("err:free pa2 = 0x%lx, va2 = 0x%lx\r\n", (ULONG)pa2, (ULONG)va2);
		}
	}
	if (va1) {
		ret = hd_common_mem_free(pa1, (void *)va1);
		if (ret != HD_OK) {
			printf("err:free pa1 = 0x%lx, va1 = 0x%lx\r\n", (ULONG)pa1, (ULONG)va1);
		}
	}
	return func_ret;
}

static int test_calc_buf_size_p(UINT32 w, UINT32 h)
{
	UINT32    buf_size;

	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_RAW8);
	DBG_DUMP("RAW8 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_RAW12);
	DBG_DUMP("RAW12 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_RAW16);
	DBG_DUMP("RAW16 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_NRX12);
	DBG_DUMP("NRX12 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_NRX12_SHDR4);
	DBG_DUMP("NRX12_SHDR4 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);

	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_YUV420);
	DBG_DUMP("YUV420 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_YUV422);
	DBG_DUMP("YUV422 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_YUV444);
	DBG_DUMP("YUV444 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_YUV420_MB);
	DBG_DUMP("YUV420_MB w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_YUV420_MB4);
	DBG_DUMP("YUV420_MB4 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_YUV420_NVX2);
	DBG_DUMP("YUV420_NVX2 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_YUV420_NVX5);
	DBG_DUMP("YUV420_NVX5 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);

	#if 0
	DBG_DUMP("Test error case\r\n");
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_NRX10);
	DBG_DUMP("NRX10 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	buf_size = vendor_common_mem_calc_max_buf_size(w, h, HD_VIDEO_PXLFMT_ARGB8888);
	DBG_DUMP("ARGB8888 w = %d, h= %d, buf_size %d\r\n",w, h, buf_size);
	#endif

	return HD_OK;
}

static int test_calc_max_buf_size(void)
{
	HD_RESULT ret;
	VENDOR_COMM_MEM_CFG_MAX_BUF  cfg_max_buf = {0};

	cfg_max_buf.md_func = 1;
	cfg_max_buf.vprc_max_in_size.w = 3840;
	cfg_max_buf.vprc_max_in_size.h = 2160;
	cfg_max_buf.eis_func = 1;
	cfg_max_buf.gyro_max_num = 32;
	ret = vendor_common_mem_set(VENDOR_COMMON_MEM_ITEM_CFG_MAX_BUF, (VOID *)&cfg_max_buf);
	if(ret != HD_OK) {
		DBG_ERR("test_cfg_max_buf fail=%d\r\n", ret);
		return ret;
    }
	test_calc_buf_size_p(1920, 1080);
	test_calc_buf_size_p(3840, 2160);
	cfg_max_buf.vprc_max_in_size.w = 1920;
	cfg_max_buf.vprc_max_in_size.h = 1080;
	cfg_max_buf.gyro_max_num = 64;
	ret = vendor_common_mem_set(VENDOR_COMMON_MEM_ITEM_CFG_MAX_BUF, (VOID *)&cfg_max_buf);
	if(ret != HD_OK) {
		DBG_ERR("test_cfg_max_buf fail=%d\r\n", ret);
		return ret;
    }
	test_calc_buf_size_p(1920, 1080);
	test_calc_buf_size_p(3840, 2160);
	return HD_OK;
}

static HD_RESULT test_ddr_auto(void)
{
	#define BLK_CNT   6
	#define BUFF_CNT  7

	UINT32                 i, blk_size = 0x300000, alloc_size = 0x6000000;
	HD_COMMON_MEM_VB_BLK   blk[BLK_CNT], tmp_blk;
	HD_COMMON_MEM_DDR_ID   ddr_id = DDR_ID_AUTO;
	HD_RESULT              ret;
	UINTPTR                pa[BUFF_CNT] = {0}, va[BUFF_CNT] = {0}, tmp_va, tmp_pa;
	VENDOR_COMM_MAX_FREE_BLOCK max_free = {0};


	printf("\r\ntest_ddr_auto\r\n");

	#if !TEST_DDR2
	printf("test Fail ! Pelease enable TEST_DDR2 !!\r\n");
	return HD_ERR_NG;
	#endif
	for (i = 0;i < BLK_CNT; i++) {
		blk[i] = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, ddr_id);
		if (blk[i] == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("get block fail\r\n");
			return HD_ERR_NG;
		}
		system("cat /proc/hdal/comm/info");
	}

	printf("\r\n====  Error case begin ======\r\n");
	tmp_blk = hd_common_mem_get_block(HD_COMMON_MEM_COMMON_POOL, blk_size, ddr_id);
	if (tmp_blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail\r\n");
	}
	printf("\r\n====  Error case end ======\r\n");


	for (i = 0;i < DDR_ID2; i++) {
		max_free.ddr = i;
		if (HD_OK != vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free)) {
			return HD_ERR_NG;
		}
		alloc_size = max_free.size;
		ret = hd_common_mem_alloc("test_1", &pa[i], (void **)&va[i], alloc_size, ddr_id);
		if (ret != HD_OK) {
			printf("err:alloc size 0x%x, ddr %d\r\n", (int)alloc_size, ddr_id);
			return HD_ERR_NG;
		}
		system("cat /proc/hdal/comm/info");
	}

	printf("\r\n====  Error case begin ======\r\n");
	ret = hd_common_mem_alloc("test_2", &tmp_pa, (void **)&tmp_va, alloc_size, ddr_id);
	if (ret != HD_OK) {
		printf("err:alloc size 0x%x, ddr %d\r\n", (int)alloc_size, ddr_id);
	}
	printf("\r\n====  Error case end ======\r\n");


	// release buffers
	for (i = 0;i < BLK_CNT; i++) {
		ret = hd_common_mem_release_block(blk[i]);
		if (HD_OK != ret) {
			printf("release blk fail %d\r\n", ret);
			return HD_ERR_NG;
		}
	}
	for (i = 0;i < DDR_ID2; i++) {
		ret = hd_common_mem_free(pa[i], (void *)va[i]);
		if (ret != HD_OK) {
			printf("err:free pa = 0x%lx, va = 0x%lx\r\n", (ULONG)pa[i], (ULONG)va[i]);
		}
	}
	system("cat /proc/hdal/comm/info");


	printf("\r\n====  test get_ddrid ======\r\n");
	if ((ret = vendor_common_get_ddrid(HD_COMMON_MEM_USER_BLK, COMMON_PCIE_CHIP_RC, &ddr_id)) != HD_OK) {
		printf("vendor_common_get_ddrid pool_type(HD_COMMON_MEM_USER_BLK) fail\r\n");
		return HD_ERR_NG;
	}
	printf("ddrid = %d\r\n", ddr_id);

	alloc_size = 0x100000;
	hd_common_mem_alloc("test_ddrid", &pa[0], (void **)&va[0], alloc_size, ddr_id);
	system("cat /proc/hdal/comm/info");
	ret = hd_common_mem_free(pa[0], (void *)va[0]);
	system("cat /proc/hdal/comm/info");

	return HD_OK;
}

EXAMFUNC_ENTRY(hd_common_test, argc, argv)
{
    HD_RESULT ret;
    INT key;
	#if defined(__LINUX)
	INT is_test_signal = 0;
	pthread_t  signal_thread_id;
	#endif

	if (argc >= 2) {
		printf("param2=%s!\r\n", argv[2]);
		if (argv[1][0] == 's') {
			g_pid = 0;
			printf("running as <server>\r\n");
		} else if (argv[1][0] == 'c') {
			g_pid = 1;
			printf("running as <client>\r\n");
		} else {

		}
	}
	#if defined(__LINUX)
	if (argc >= 3) {
		is_test_signal = atoi(argv[2]);
		printf("is_test_signal=%d\r\n", is_test_signal);
	}
	if (is_test_signal) {
		printf("test signal\r\n");
		ret = pthread_create(&signal_thread_id, NULL, signal_thread, NULL);
		if (ret < 0) {
			printf("create signal thread failed");
			goto exit;
		}
	}
	#endif
	ret = test_fixed_pool();
    if(ret != HD_OK) {
		printf("test_fixed_pool fail=%d\r\n", ret);
		//goto exit;
    }
	//init hdal
	ret = hd_common_init(g_pid);
    if(ret != HD_OK) {
		printf("hd_common_init fail=%d\r\n", ret);
		goto exit;
    }
	ret = test_calc_max_buf_size();
	if(ret != HD_OK) {
		printf("test_calc_max_buf_size fail=%d\r\n", ret);
		goto exit;
	}
	//init memory
	ret = mem_init(g_pid);
    if(ret != HD_OK) {
		printf("mem_init fail=%d\r\n", ret);
		goto exit;
	}
	//test get block from common pool
	ret = test_get_block_from_common();
    if(ret != HD_OK) {
		printf("test_get_block_from_common fail=%d\r\n", ret);
		goto exit;
    }
	//test get block from osg pool
	ret = test_get_block_from_osg();
    if(ret != HD_OK) {
		printf("test_get_block_from_osg fail=%d\r\n", ret);
		goto exit;
    }
	//test alloc memory
	ret = test_alloc(DDR_ID0);
    if(ret != HD_OK) {
		printf("test_alloc fail=%d\r\n", ret);
		goto exit;
    }
	#if TEST_DDR2
	ret = test_alloc(DDR_ID1);
    if(ret != HD_OK) {
		printf("test_alloc fail=%d\r\n", ret);
		goto exit;
    }
	#endif
	ret = test_alloc_temp(DDR_ID0);
	if(ret != HD_OK) {
		printf("test_alloc fail=%d\r\n", ret);
		goto exit;
    }
	//test va to pa
	ret = test_va2pa();
    if(ret != HD_OK) {
		printf("test_va2pa fail=%d\r\n", ret);
		goto exit;
    }
	// test multiple va mmap to the same physical
	ret = test_multi_va_map_same_phy();
	if(ret != HD_OK) {
		printf("test_multi_va_map_same_phy fail=%d\r\n", ret);
		goto exit;
    }
	// test cache flush performance
	ret = test_cacheflush_perf();
	if(ret != HD_OK) {
		printf("test_cacheflush_perf fail=%d\r\n", ret);
		goto exit;
    }
	// test allocate maximum block
	ret = test_alloc_maxblk(DDR_ID0);
	if(ret != HD_OK) {
		printf("test_alloc_maxblk fail=%d\r\n", ret);
		goto exit;
    }
	#if TEST_DDR2
	ret = test_alloc_maxblk(DDR_ID1);
	if(ret != HD_OK) {
		printf("test_alloc_maxblk fail=%d\r\n", ret);
		goto exit;
    }
	#endif
	// test_get_common_pool_range
	ret = test_get_common_pool_range(DDR_ID0);
	if(ret != HD_OK) {
		printf("test_get_common_pool_range fail=%d\r\n", ret);
		goto exit;
    }
	#if TEST_DDR2
	ret = test_get_common_pool_range(DDR_ID1);
	if(ret != HD_OK) {
		printf("test_get_common_pool_range fail=%d\r\n", ret);
		goto exit;
    }
	#endif
	ret = test_create_destory_pool();
	if(ret != HD_OK) {
		printf("test_create_destory_pool fail=%d\r\n", ret);
		goto exit;
    }
	ret = test_if_hdal_mem();
	if(ret != HD_OK) {
		printf("test_if_hdal_mem fail=%d\r\n", ret);
		goto exit;
    }
	while (1) {
		printf("\r\nEnter q to exit\r\n");
		printf("Enter d to enter debug menu\r\n");
		printf("Enter 1 to test alloc memory on ddr1\r\n");
		#if TEST_DDR2
		printf("Enter 2 to test alloc memory on ddr2\r\n");
		#endif
		printf("Enter 3 to test timestamp\r\n");
		printf("Enter 4 to test cacheflush_perf\r\n");
		printf("Enter 5 to test err handling\r\n");
		printf("Enter 6 to test random alloc, free\r\n");
		#if 0
		printf("Enter 7 to test get bridge memory\r\n");
		#endif
		printf("Enter 8 to test get common pool range\r\n");
		#if 0
		printf("Enter 9 to test memory hotplug\r\n");
		#endif
		printf("Enter a to test hdal_mem check\r\n");
		printf("Enter c to test test_alloc_max_pools\r\n");
		printf("Enter f to test ddr-auto\r\n");
		key = NVT_EXAMSYS_GETCHAR();
		if (key == 0xa) {
			key = NVT_EXAMSYS_GETCHAR();
		}
		if (key == 'q' || key == 0x3) {
			g_quit = 1;
			break;
		}
		if (key == '1') {
			ret = test_alloc(DDR_ID0);
		    if(ret != HD_OK) {
				printf("test_alloc fail=%d\r\n", ret);
				goto exit;
		    }
			continue;
		}
		#if TEST_DDR2
		if (key == '2') {
			ret = test_alloc(DDR_ID1);
		    if(ret != HD_OK) {
				printf("test_alloc fail=%d\r\n", ret);
				goto exit;
		    }
			continue;
		}
		#endif
		if (key == '3') {
			ret = test_timestamp(1000);
			if(ret != HD_OK) {
				printf("test_timestamp fail=%d\r\n", ret);
				goto exit;
		    }
			printf("test_timestamp OK\r\n");
			continue;
		}
		if (key == '4') {
			ret = test_cacheflush_perf();
			if(ret != HD_OK) {
				printf("test_cacheflush_perf fail=%d\r\n", ret);
				goto exit;
		    }
			printf("test_cacheflush_perf OK\r\n");
			continue;
		}
		if (key == '5') {
			ret = test_err();
			if(ret != HD_OK) {
				printf("test_err fail\r\n");
				goto exit;
		    }
			printf("test_err OK\r\n");
			continue;
		}
		if (key == '6') {
			ret = test_random_alloc_free(DDR_ID0);
			if(ret != HD_OK) {
				printf("test_random_alloc_free fail=%d\r\n", ret);
				goto exit;
		    }
			printf("test_random_alloc_free OK\r\n");
			continue;
		}
		#if 0
		if (key == '7') {
			ret = test_get_bridge_mem();
			if(ret != HD_OK) {
				printf("test_get_bridge_mem fail=%d\r\n", ret);
				goto exit;
		    }
			printf("test_get_bridge_mem OK\r\n");
			continue;
		}
		#endif
		if (key == '8') {
			ret = test_get_common_pool_range(DDR_ID0);
			if(ret != HD_OK) {
				printf("test_get_common_pool_range fail=%d\r\n", ret);
				goto exit;
		    }
			printf("test_get_common_pool_range OK\r\n");
			continue;
		}
		#if 0
		if (key == '9') {
			VENDOR_LINUX_MEM_HOT_PLUG  mem_hotplug = {0};

			mem_hotplug.start_addr = 0x60000000;
			mem_hotplug.size = 0x01000000;
			ret = vendor_common_mem_set(VENDOR_COMMON_MEM_ITEM_LINUX_HOT_PLUG, (VOID *)&mem_hotplug);
			if(ret != HD_OK) {
				printf("test_mem_hotplug fail=%d\r\n", ret);
				goto exit;
		    }
			printf("test_mem_hotplug OK\r\n");
			continue;
		}
		#endif
		if (key == 'a') {
			ret = test_if_hdal_mem();
			if(ret != HD_OK) {
				printf("test_if_hdal_mem fail=%d\r\n", ret);
				goto exit;
		    }
			continue;
		}
		if (key == 'c') {
			ret = test_alloc_max_pools(DDR_ID0);
		    if(ret != HD_OK) {
				printf("test_alloc_max_pools fail=%d\r\n", ret);
				goto exit;
		    }
			continue;
		}
		if (key == 'f') {
			test_ddr_auto();
			continue;
		}
		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			// enter debug menu
			hd_debug_run_menu();
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif
	}
exit:
    //uninit memory
    ret = mem_exit();
    if(ret != HD_OK) {
        printf("mem fail=%d\n", ret);
    }
    //uninit hdal
	ret = hd_common_uninit();
    if(ret != HD_OK) {
        printf("common fail=%d\n", ret);
    }
	return 0;
}
