/**
	@brief Demo code of 1 videocap path + 2 videoenc path + 1 videoout path.\n

	@file demo1_osg.c

	@author iVOT/PSW

	@ingroup mhdal

	@note This file is modified from these sample code:
	      video_record_with_osg.c
	      video_liveview_with_osg.c

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vsp_osg.h"


#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#define DBG_ERR(fmtstr, args...)  printf("\033[31mERR:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_WRN(fmtstr, args...)  printf("\033[33mWRN:%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
#define DBG_IND(fmtstr, args...)



///////////////////////////////////////////////////////////////////////////////

int osg_query_buf_size(UINT32 w, UINT32 h, HD_VIDEO_PXLFMT pxlfmt)
{
	HD_VIDEO_FRAME frame = {0};
	int stamp_size;

	frame.sign   = MAKEFOURCC('O','S','G','P');
	frame.dim.w  = w;
	frame.dim.h  = h;
	frame.pxlfmt = pxlfmt;

	//get required buffer size for a single image
	stamp_size = hd_common_mem_calc_buf_size(&frame);
	if(!stamp_size){
		DBG_ERR("fail to query buffer size\n");
		return -1;
	}

	//ping pong buffer needs double size
	stamp_size *= 2;

	return stamp_size;
}

///////////////////////////////////////////////////////////////////////////////

static int osg_alloc_stamp_data(UINT32 id, VSP_OSG* osg)
{
	UINTPTR                  pa, va;
	HD_COMMON_MEM_VB_BLK    blk;

	if(!osg->stamp_size){
		DBG_ERR("stamp_size is unknown\n");
		return -1;
	}

	//get osd ping pong buffer block
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL+id, osg->stamp_size, OSG_DDR_ID);
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		DBG_ERR("stamp %d get block fail, blk = 0x%x\r\n", id, blk);
		return -1;
	}
	osg->stamp_blk = blk;
	//translate stamp block to physical address
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		DBG_ERR("blk2pa fail, blk = 0x%x\r\n", blk);
		goto release_1;
	}
	osg->stamp_pa = pa;
	//get osd stamp data block
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL+id, osg->stamp_size, OSG_DDR_ID);
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		DBG_ERR("stamp %d get block fail, blk = 0x%x\r\n", id, blk);
		goto release_1;
	}
	osg->stamp_data_blk = blk;
	//translate stamp block to physical address
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		DBG_ERR("blk2pa fail, blk = 0x%x\r\n", blk);
		goto release_2;
	}
	osg->stamp_data_pa = pa;

	va = (ULONG)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, osg->stamp_size);
	if (va == 0) {
		goto release_2;
	}
	osg->stamp_data_va = va;

	return 0;
release_1:
	hd_common_mem_release_block(osg->stamp_blk);
	osg->stamp_blk = HD_COMMON_MEM_VB_INVALID_BLK;
	return -1;
release_2:
	hd_common_mem_release_block(osg->stamp_blk);
	hd_common_mem_release_block(osg->stamp_data_blk);
	osg->stamp_blk = HD_COMMON_MEM_VB_INVALID_BLK;
	osg->stamp_data_blk = HD_COMMON_MEM_VB_INVALID_BLK;
	return -1;
}

static void osg_free_stamp_data(UINT32 id, VSP_OSG* osg)
{
	HD_RESULT         ret;

	ret = hd_common_mem_release_block(osg->stamp_blk);
	if (HD_OK != ret) {
		DBG_ERR("release blk fail %d\r\n", ret);
	}
	ret = hd_common_mem_release_block(osg->stamp_data_blk);
	if (HD_OK != ret) {
		DBG_ERR("release blk fail %d\r\n", ret);
	}
}

static int set_vprc_stamp_img(UINT32 id, HD_PATH_ID stamp_path, UINT32 w, UINT32 h, HD_VIDEO_PXLFMT pxlfmt, UINTPTR stamp_pa, UINT32 stamp_size, UINTPTR stamp_data)
{
	HD_OSG_STAMP_BUF  buf;
	HD_OSG_STAMP_IMG  img;

	if(!stamp_pa){
		DBG_ERR("stamp buffer %d is not allocated\n", id);
		return -1;
	}

	memset(&buf, 0, sizeof(HD_OSG_STAMP_BUF));

	buf.type      = HD_OSG_BUF_TYPE_PING_PONG;
	buf.p_addr    = stamp_pa;
	buf.size      = stamp_size;

	if(hd_videoproc_set(stamp_path, HD_VIDEOPROC_PARAM_IN_STAMP_BUF, &buf) != HD_OK){
		DBG_ERR("fail to set vprc stamp buffer %d\n", id);
		return -1;
	}

	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));

	img.fmt        = pxlfmt;
	img.dim.w      = w;
	img.dim.h      = h;
	img.p_addr     = (uintptr_t)stamp_data;

	if(hd_videoproc_set(stamp_path, HD_VIDEOPROC_PARAM_IN_STAMP_IMG, &img) != HD_OK){
		DBG_ERR("fail to set vprc stamp image %d\n", id);
		return -1;
	}

	return 0;
}

static int set_vprc_stamp_attr(UINT32 id, HD_PATH_ID stamp_path, UINT32 x, UINT32 y)
{
	HD_OSG_STAMP_ATTR attr;

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));

	attr.position.x = x;
	attr.position.y = y;
	attr.alpha      = 255;
	attr.layer      = 0;
	attr.region     = 0;

	return hd_videoproc_set(stamp_path, HD_VIDEOPROC_PARAM_IN_STAMP_ATTR, &attr);
}

static int set_venc_stamp_img(UINT32 id, HD_PATH_ID stamp_path, UINT32 w, UINT32 h, HD_VIDEO_PXLFMT pxlfmt, UINTPTR stamp_pa, UINT32 stamp_size, UINTPTR stamp_data)
{
	HD_OSG_STAMP_BUF  buf;
	HD_OSG_STAMP_IMG  img;

	if(!stamp_pa){
		DBG_ERR("stamp buffer %d is not allocated\n", id);
		return -1;
	}

	memset(&buf, 0, sizeof(HD_OSG_STAMP_BUF));

	buf.type      = HD_OSG_BUF_TYPE_PING_PONG;
	buf.p_addr    = stamp_pa;
	buf.size      = stamp_size;

	if(hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_BUF, &buf) != HD_OK){
		DBG_ERR("fail to set venc stamp buffer%d\n", id);
		return -1;
	}

	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));

	img.fmt        = pxlfmt;
	img.dim.w      = w;
	img.dim.h      = h;
	img.p_addr     = (uintptr_t)stamp_data;

	if(hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_IMG, &img) != HD_OK){
		DBG_ERR("fail to set venc stamp image %d\n", id);
		return -1;
	}

	return 0;
}

static int set_venc_stamp_attr(UINT32 id, HD_PATH_ID stamp_path, UINT32 x, UINT32 y)
{
	HD_OSG_STAMP_ATTR attr;

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));

	attr.position.x = x;
	attr.position.y = y;
	attr.alpha      = 255;
	attr.layer      = 0;
	attr.region     = 0;

	return hd_videoenc_set(stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &attr);
}

static int set_vout_stamp_img(UINT32 id, HD_PATH_ID stamp_path, UINT32 w, UINT32 h, HD_VIDEO_PXLFMT pxlfmt, UINTPTR stamp_pa, UINT32 stamp_size, UINTPTR stamp_data)
{
	HD_OSG_STAMP_BUF  buf;
	HD_OSG_STAMP_IMG  img;

	if(!stamp_pa){
		DBG_ERR("stamp buffer %d is not allocated\n", id);
		return -1;
	}

	memset(&buf, 0, sizeof(HD_OSG_STAMP_BUF));

	buf.type      = HD_OSG_BUF_TYPE_PING_PONG;
	buf.p_addr    = stamp_pa;
	buf.size      = stamp_size;

	if(hd_videoout_set(stamp_path, HD_VIDEOOUT_PARAM_OUT_STAMP_BUF, &buf) != HD_OK){
		DBG_ERR("fail to set vout stamp buffer %d\n", id);
		return -1;
	}

	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));

	img.fmt        = pxlfmt;
	img.dim.w      = w;
	img.dim.h      = h;
	img.p_addr     = (uintptr_t)stamp_data;

	if(hd_videoout_set(stamp_path, HD_VIDEOOUT_PARAM_OUT_STAMP_IMG, &img) != HD_OK){
		DBG_ERR("fail to set vout stamp image %d\n", id);
		return -1;
	}

	return 0;
}

static int set_vout_stamp_attr(UINT32 id, HD_PATH_ID stamp_path, UINT32 x, UINT32 y)
{
	HD_OSG_STAMP_ATTR attr;

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));

	attr.position.x = x;
	attr.position.y = y;
	attr.alpha      = 0xF0;
	attr.layer      = 0;
	attr.region     = 0;

	return hd_videoout_set(stamp_path, HD_VIDEOOUT_PARAM_OUT_STAMP_ATTR, &attr);
}

////////////////////////////
HD_RESULT osg_read_pattern(UINT32 id, VSP_OSG* osg)
{
	FILE     *fp = NULL;
    char     osg_file_str[3][50] = { "/mnt/sd/osg/7824x3470_argb4444.bin",
		                             "/mnt/sd/osg/3840x1704_argb4444.bin",
		                             "/mnt/sd/osg/1920x852_argb4444.bin"
		                            };
	UINT32 file_size = 0, read_size = 0;

	if (id >= 3) {
		DBG_ERR("id %d exceeds limit\r\n", id);
		return HD_ERR_LIMIT;
	}
    if ((fp = fopen(osg_file_str[id], "rb")) == NULL) {
		DBG_ERR(" open %s fail\r\n !!", osg_file_str[id]);
		return HD_ERR_PARAM;
    }
	fseek(fp, 0, SEEK_END);
	file_size = ALIGN_CEIL_4(ftell(fp));
	fseek(fp, 0, SEEK_SET);
	if (file_size > osg->stamp_size) {
		DBG_WRN("file_size = %d > stamp_size = %d\r\n", (int)file_size, (int)osg->stamp_size);
	}
	read_size = fread((void *)osg->stamp_data_va, 1, file_size, fp);
	if (read_size != file_size) {
		DBG_ERR("size mismatch, read = %d, file_size = %d\r\n", (int)read_size, (int)file_size);
	}
	fclose(fp);
	hd_common_mem_flush_cache((void*)osg->stamp_data_va, read_size);
	return HD_OK;
}


HD_RESULT osg_init(UINT32 id, VSP_OSG* osg, UINT32 w, UINT32 h, HD_VIDEO_PXLFMT pxlfmt, unsigned short c)
{
	HD_RESULT ret;

	osg->stamp_dim.w = w;
	osg->stamp_dim.h = h;
	osg->stamp_fmt = pxlfmt;

	// get blk of stamp data
	osg->stamp_blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	osg->stamp_pa   = 0;
	osg->stamp_size = osg_query_buf_size(osg->stamp_dim.w, osg->stamp_dim.h, osg->stamp_fmt);
	if(osg->stamp_size <= 0){
		DBG_ERR("osg_query_buf_size() fail\n");
		return HD_ERR_LIMIT;
	}
	// allocate buffer
	ret = osg_alloc_stamp_data(id, osg);
	if(ret){
		DBG_ERR("fail to allocate stamp buffer %d\n", id);
		return HD_ERR_NOMEM;
	}
	osg_read_pattern(id, osg);
	return HD_OK;
}

HD_RESULT osg_open(UINT32 id, VSP_OSG* osg, UINT32 type, UINT32 path)
{
	HD_RESULT ret;

	if(type == OSG_TYPE_VIDEOPROC) {

		if((ret = hd_videoproc_open(path, HD_STAMP_0, &osg->vprc_stamp_path)) != HD_OK) {
			DBG_ERR("fail to open vprc stamp\n");
			return ret;
		}

	} else if(type == OSG_TYPE_VIDEOENC) {

		if((ret = hd_videoenc_open(path, HD_STAMP_0, &osg->venc_stamp_path)) != HD_OK) {
			DBG_ERR("fail to open venc stamp\n");
			return ret;
		}

	} else if(type == OSG_TYPE_VIDEOOUT) {

		if((ret = hd_videoout_open(path, HD_STAMP_0, &osg->vout_stamp_path)) != HD_OK) {
			DBG_ERR("fail to open vout stamp\n");
			return ret;
		}
	}

	return HD_OK;
}

HD_RESULT osg_close(UINT32 id, VSP_OSG* osg, UINT32 type)
{
	HD_RESULT ret;
	if(type == OSG_TYPE_VIDEOPROC) {

		if((ret = hd_videoproc_close(osg->vprc_stamp_path)) != HD_OK)
			return ret;

	} else if(type == OSG_TYPE_VIDEOENC) {

		if((ret = hd_videoenc_close(osg->venc_stamp_path)) != HD_OK)
			return ret;

	} else if(type == OSG_TYPE_VIDEOOUT) {

		if((ret = hd_videoout_close(osg->vout_stamp_path)) != HD_OK)
			return ret;

	}

	return HD_OK;
}

HD_RESULT osg_start(UINT32 id, VSP_OSG* osg, UINT32 type, UINT32 x, UINT32 y)
{
	HD_RESULT ret;

	if(type == OSG_TYPE_VIDEOPROC) {

		// setup vprc stamp image
		if(set_vprc_stamp_img(id, osg->vprc_stamp_path, osg->stamp_dim.w,  osg->stamp_dim.h, osg->stamp_fmt, osg->stamp_pa, osg->stamp_size, osg->stamp_data_va)){
			DBG_ERR("fail to set vprc stamp img\r\n");
			return HD_ERR_ABORT;
		}

		// setup vprc stamp x,y
		if(set_vprc_stamp_attr(id, osg->vprc_stamp_path, x, y)){
			DBG_ERR("fail to set vprc stamp attr\r\n");
			return HD_ERR_ABORT;
		}

		// render vprc stamp
		ret = hd_videoproc_start(osg->vprc_stamp_path);
		if (ret != HD_OK) {
			DBG_ERR("start vprc stamp fail=%d\r\n", ret);
			return HD_ERR_ABORT;
		}

	} else if(type == OSG_TYPE_VIDEOENC) {

		// setup venc stamp image
		if(set_venc_stamp_img(id, osg->venc_stamp_path, osg->stamp_dim.w,  osg->stamp_dim.h, osg->stamp_fmt, osg->stamp_pa, osg->stamp_size, osg->stamp_data_va)){
			DBG_ERR("fail to set venc stamp img\r\n");
			return HD_ERR_ABORT;
		}

		// setup venc stamp x,y
		if(set_venc_stamp_attr(id, osg->venc_stamp_path, x, y)){
			DBG_ERR("fail to set venc stamp attr\r\n");
			return HD_ERR_ABORT;
		}

		// render venc stamp
		ret = hd_videoenc_start(osg->venc_stamp_path);
		if (ret != HD_OK) {
			DBG_ERR("start venc stamp fail=%d\r\n", ret);
			return HD_ERR_ABORT;
		}

	} else if(type == OSG_TYPE_VIDEOOUT) {

		// setup vout stamp image
		if(set_vout_stamp_img(id, osg->vout_stamp_path, osg->stamp_dim.w,  osg->stamp_dim.h, osg->stamp_fmt, osg->stamp_pa, osg->stamp_size, osg->stamp_data_va)){
			DBG_ERR("fail to set vout stamp img\r\n");
			return HD_ERR_ABORT;
		}

		// setup vout stamp x,y
		if(set_vout_stamp_attr(id, osg->vout_stamp_path, x, y)){
			DBG_ERR("fail to set vout stamp attr\r\n");
			return HD_ERR_ABORT;
		}

		// render vout stamp
		ret = hd_videoout_start(osg->vout_stamp_path);
		if (ret != HD_OK) {
			DBG_ERR("start vout stamp fail=%d\r\n", ret);
			return HD_ERR_ABORT;
		}
	}
	return HD_OK;
}

int osg_save_file(char *filename, UINT32 buf_size, void *va)
{
	int fd, len;

	if(filename == NULL){
		DBG_ERR("save_file():filename is NULL\n");
		return -1;
	}

	fd = open(filename, O_WRONLY | O_CREAT, 0644);
	if(fd == -1){
		DBG_ERR("save_file():fail to open %s\n", filename);
		return -1;
	}

	len = write(fd, va, buf_size);
	close(fd);
	if(len != (int)buf_size){
		DBG_ERR("fail to write %s\n", filename);
		return -1;
	}
	printf("result is %s\n", filename);

	return HD_OK;
}


HD_RESULT osg_update(UINT32 id, VSP_OSG* osg, char *pattern)
{
	#if 0
	HD_OSG_STAMP_IMG  img;
	char              char_w = osg->stamp_dim.w/MAX_CHAR;
	int               len;


	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));
	len = strlen(pattern);
	if (len > MAX_CHAR) {
		DBG_ERR("string too long %s\r\n", pattern);
		return -1;
	}
	img.fmt        = osg->stamp_fmt;
	img.dim.w      = char_w*len;
	img.dim.h      = osg->stamp_dim.h;
	img.p_addr     = (uintptr_t)osg->stamp_data_va;


	//gfx_create_timestamp(pattern, char_w, img.dim.h, osg->stamp_size, osg->stamp_data_pa);
	hd_common_mem_flush_cache((void*)osg->stamp_data_va, osg->stamp_size);

	#if 0
	{
		static BOOL isSave = 0;
		HD_RESULT    ret;

		if (!isSave) {
			isSave = 1;
			ret = osg_save_file("/mnt/sd/vsp/timestamp_1.argb1555", osg->stamp_size, (void *)osg->stamp_data_va);
			if(ret != HD_OK){
				DBG_ERR("save_file(/mnt/sd/vsp/timestamp_1.argb1555) fail=%d\r\n", ret);
			} else {
				DBG_DUMP("save_file(/mnt/sd/vsp/timestamp_1.argb1555) OK\r\n");
			}
		}
	}
	#endif
	if(hd_videoout_set(osg->vout_stamp_path, HD_VIDEOOUT_PARAM_OUT_STAMP_IMG, &img) != HD_OK){
		DBG_ERR("fail to set vout stamp image %d\n", id);
		return -1;
	}
	#endif
	return HD_OK;
}
HD_RESULT osg_uninit(UINT32 id, VSP_OSG* osg)
{
	// release blk of stamp data
	osg_free_stamp_data(id, osg);
	return HD_OK;
}

