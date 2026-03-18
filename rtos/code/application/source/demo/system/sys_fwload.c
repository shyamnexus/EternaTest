#include <stdlib.h>
#include <libfdt.h>
#include <compiler.h>
#include <rtosfdt.h>
#include <kwrap/flag.h>
#include <kwrap/util.h>
#include <kwrap/debug.h>
#include <kwrap/task.h>
#include <kwrap/perf.h>
#include <comm/shm_info.h>
#include <strg_def.h>
#include <FwSrvApi.h>
#include <bin_info.h>
#include <nvtpack.h>
#include <zlib.h>
#include "prjcfg.h"
#include "sys_fwload.h"
#include "sys_storage_partition.h"

//#NT#PARTIAL_COMPRESS, we use rtos-main size + 1 block size
#if (POWERON_MODE == POWERON_MODE_BOOT_LINUX)
#define FW_PARTIAL_COMPRESS_WORK_BUF_SIZE 0x100000
#else
#define FW_PARTIAL_COMPRESS_WORK_BUF_SIZE 0x500000
#endif

#define CFG_MAX_FW_SIZE 0x800000 //for rtos boot linux, the original fw size must be smaller

#if defined(_FW_TYPE_PARTIAL_) || defined(_FW_TYPE_PARTIAL_COMPRESS_)

static ID fwload_flg_id = 0;

//This array is sort by section id
static UINT32 UserSection_Load[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//This array is sort by loading order
static UINT32 UserSection_Order_full[] = {
	CODE_SECTION_02, CODE_SECTION_03, CODE_SECTION_04, CODE_SECTION_05, CODE_SECTION_06,
	CODE_SECTION_07, CODE_SECTION_08, CODE_SECTION_09, CODE_SECTION_10, FWSRV_PL_BURST_END_TAG
};

int fwload_init(void)
{
	vos_flag_create(&fwload_flg_id, NULL, "fwload_flg_id");
	vos_flag_clr(fwload_flg_id, (FLGPTN)-1);
	// CODE_SECTION_01 has loaded by loader or u-boot
	vos_flag_set(fwload_flg_id, (FLGPTN)(1 << CODE_SECTION_01));
	return 0;
}

int fwload_set_done(CODE_SECTION section)
{
	vos_perf_list_mark("pl", __LINE__, (int)section);
	vos_flag_set(fwload_flg_id, (FLGPTN)(1 << section));
	return 0;
}

int fwload_wait_done(CODE_SECTION section)
{
	FLGPTN flgptn;
	vos_flag_wait(&flgptn, fwload_flg_id, (FLGPTN)(1 << section), TWF_ANDW);
	return 0;
}

static void UserSection_LoadCb(const UINT32 Idx)
{
	DBG_DUMP("Section-%.2ld: (LOAD)\r\n", Idx + 1);
	UserSection_Load[Idx] = 1; //mark loaded
	fwload_set_done(Idx);
}


#if defined(_FW_TYPE_PARTIAL_COMPRESS_)
static void *gz_alloc(void *x, unsigned items, unsigned size)
{
	void *p = malloc(size * items);
	if (p) {
		memset(p, 0, size * items);
	}
	return p;
}
static void gz_free(void *x, void *addr, unsigned nb)
{
	free(addr);
}
static int unzip(unsigned char *in, unsigned char *out,
	unsigned int insize, unsigned int outsize)
{
	int err;
	z_stream stream = {0};
	stream.next_in = (z_const Bytef *)in;
	stream.avail_in = insize;
	stream.next_out = (z_const Bytef *)out;
	stream.avail_out = outsize;
	stream.zalloc = (alloc_func)gz_alloc;
	stream.zfree = (free_func)gz_free;
	stream.opaque = (voidpf)0;
	err = inflateInit(&stream);
	if (err != Z_OK) {
		DBG_ERR("Failed to inflateInit, err = %d\r\n", err);
		inflateEnd(&stream);
		return err;
	}

	err = inflate(&stream, Z_NO_FLUSH);

	inflateEnd(&stream);

	if (err == Z_STREAM_END) {
		return 0;
	}

	return err;
}
#endif

int fwload_partload(void)
{
	extern BININFO bin_info;
	SHMINFO *p_shm;
	FWSRV_INIT Init = {0};
	FWSRV_CMD Cmd = {0};
	FWSRV_PL_LOAD_BURST_IN pl_in = {0};
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	// read SHMEM_PATH
	nodeoffset = fdt_path_offset(p_fdt, SHMEM_PATH);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", SHMEM_PATH, nodeoffset);
	} else {
		DBG_DUMP("offset for  %s = %d \n", SHMEM_PATH, nodeoffset);
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("failed to access reg.\n");
		return 0;
	} else {
		unsigned int *p_data = (unsigned int *)nodep;
		p_shm = (SHMINFO *)(uintptr_t)be32_to_cpu(p_data[0]);
		DBG_DUMP("p_shm = 0x%lX\n", (unsigned long)p_shm);
	}

	// init fwsrv
	Init.uiApiVer = FWSRV_API_VERSION;
	Init.StrgMap.pStrgFdt = EMB_GETSTRGOBJ(STRG_OBJ_FW_FDT);
	Init.StrgMap.pStrgApp = EMB_GETSTRGOBJ(STRG_OBJ_FW_APP);
	Init.StrgMap.pStrgUboot = EMB_GETSTRGOBJ(STRG_OBJ_FW_UBOOT);
	Init.StrgMap.pStrgRtos = EMB_GETSTRGOBJ(STRG_OBJ_FW_RTOS);
	Init.PlInit.uiApiVer = PARTLOAD_API_VERSION;
	Init.PlInit.pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_RTOS);
#if !defined(_ENCRYPT_RTOS_ON_)
#if defined(_FW_TYPE_PARTIAL_COMPRESS_)
	Init.PlInit.DataType = PARTLOAD_DATA_TYPE_COMPRESS_GZ;
	Init.PlInit.uiWorkingAddr = (ULONG)malloc(FW_PARTIAL_COMPRESS_WORK_BUF_SIZE);
	Init.PlInit.uiWorkingSize = FW_PARTIAL_COMPRESS_WORK_BUF_SIZE ;
#else // !defined(_FW_TYPE_PARTIAL_COMPRESS_)
	Init.PlInit.DataType = PARTLOAD_DATA_TYPE_UNCOMPRESS;
	Init.PlInit.uiWorkingAddr = (ULONG)malloc(_EMBMEM_BLK_SIZE_);
	Init.PlInit.uiWorkingSize = _EMBMEM_BLK_SIZE_ ;
#endif // defined(_FW_TYPE_PARTIAL_COMPRESS_)
#else // defined(_ENCRYPT_RTOS_ON_)
#if defined(_FW_TYPE_PARTIAL_COMPRESS_)
	DBG_ERR("partial compress is unsupported to secure boot.\n");
	return -1;
#else
	Init.PlInit.DataType = PARTLOAD_DATA_TYPE_SIGN_UNCOMPRESS;
	Init.PlInit.uiWorkingAddr = (ULONG)malloc(_EMBMEM_BLK_SIZE_);
	Init.PlInit.uiWorkingSize = _EMBMEM_BLK_SIZE_ ;
#endif
#endif
	Init.PlInit.uiAddrBegin = _BOARD_RTOS_ADDR_ + p_shm->boot.LdLoadSize;
	FwSrv_Init(&Init);
	FwSrv_Open();

	// start partial load
	void (*LoadCallback)(const UINT32 Idx) = UserSection_LoadCb;
	UINT32 *SecOrderTable = UserSection_Order_full;

	LoadCallback(CODE_SECTION_01); // 1st part has loaded by loader

	// partial load not support on T.bin
	if ((p_shm->boot.LdCtrl2 & LDCF_BOOT_CARD) == 0 &&
		p_shm->boot.LdLoadSize != bin_info.head.BinLength) {
		ER er;
		pl_in.puiIdxSequence = SecOrderTable;
		pl_in.fpLoadedCb = LoadCallback;
		Cmd.Idx = FWSRV_CMD_IDX_PL_LOAD_BURST; //continue load
		Cmd.In.pData = &pl_in;
		Cmd.In.uiNumByte = sizeof(pl_in);
		Cmd.Prop.bExitCmdFinish = TRUE;

		er = FwSrv_Cmd(&Cmd);
		if (er != FWSRV_ER_OK) {
			FwSrv_Close();
			free((void *)Init.PlInit.uiWorkingAddr);
			DBG_ERR("Process failed!\r\n");
			return -1;
		} else {
			FwSrv_Close();
			free((void *)Init.PlInit.uiWorkingAddr);
		}
	} else {
#if defined(_FW_TYPE_PARTIAL_COMPRESS_)
		ULONG part1_size = *(ULONG *)(bin_info.head.CodeEntry + CODE_SECTION_OFFSET + sizeof(long));
		NVTPACK_BFC_HDR *p_bfc = (NVTPACK_BFC_HDR *)(bin_info.head.CodeEntry + part1_size);
		// if boot rtos by u-boot, it has decompressed.
		// only boot rtos from loader needing to decode by rtos itself
		if (p_bfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
			unsigned int gzip_image_len = be32_to_cpu((unsigned int)p_bfc->uiSizeComp);
			if (gzip_image_len > CFG_MAX_FW_SIZE) {
				DBG_ERR("compressed fw size too large: %d bytes\r\n", gzip_image_len);
				return -1;
			}

			unsigned char *gzip_image = (unsigned char *)malloc(gzip_image_len);
			if (gzip_image == NULL) {
				DBG_ERR("failed to malloc gzip_image, need: %d bytes\r\n", gzip_image_len);
				return -1;
			}

			memcpy(gzip_image, &p_bfc[1], gzip_image_len);
			unsigned int unzip_image_len = be32_to_cpu((unsigned int)p_bfc->uiSizeUnComp);
			if (unzip_image_len > CFG_MAX_FW_SIZE) {
				DBG_ERR("original fw size too large: %d bytes\r\n", unzip_image_len);
				free(gzip_image);
				return -1;
			}
			if (unzip(gzip_image, (unsigned char *) p_bfc, gzip_image_len, unzip_image_len) != 0) {
				DBG_ERR("failed to unzip partial compressed rtos image.\r\n");
				free(gzip_image);
				return -1;
			}
			free(gzip_image);
		}
#endif
		UINT32 i;
		for (i = CODE_SECTION_02; i <= CODE_SECTION_10; i++) {
			if (i == CODE_SECTION_10) {
				FwSrv_Close();
				free((void *)Init.PlInit.uiWorkingAddr);
			}
			LoadCallback(i);
		}

	}

	return 0;
}

#else

int fwload_init(void)
{
	return 0;
}

int fwload_set_done(CODE_SECTION section)
{
	return 0;
}

int fwload_wait_done(CODE_SECTION section)
{
	return 0;
}

int fwload_partload(void)
{
	//only POWERON_MODE_BOOT_LINUX needs FwSrv
#if (POWERON_MODE == POWERON_MODE_BOOT_LINUX) && !defined(_EMBMEM_NONE_)
	FWSRV_INIT Init = {0};
	// init fwsrv
	Init.uiApiVer = FWSRV_API_VERSION;
	Init.StrgMap.pStrgFdt = EMB_GETSTRGOBJ(STRG_OBJ_FW_FDT);
	Init.StrgMap.pStrgApp = EMB_GETSTRGOBJ(STRG_OBJ_FW_APP);
	Init.StrgMap.pStrgUboot = EMB_GETSTRGOBJ(STRG_OBJ_FW_UBOOT);
	Init.StrgMap.pStrgRtos = EMB_GETSTRGOBJ(STRG_OBJ_FW_RTOS);
	Init.PlInit.uiApiVer = PARTLOAD_API_VERSION;
	Init.PlInit.pStrg = EMB_GETSTRGOBJ(STRG_OBJ_FW_RTOS);
	Init.PlInit.DataType = PARTLOAD_DATA_TYPE_UNCOMPRESS;
	FwSrv_Init(&Init);
#endif
	return 0;
}

#endif

