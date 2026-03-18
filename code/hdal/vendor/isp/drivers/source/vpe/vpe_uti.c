#if defined(__FREERTOS)
#include <string.h>
#include <stdlib.h>
#else
#include <linux/slab.h>
#include <linux/vmalloc.h>
#endif

#include "kwrap/type.h"
#include "vpe_alg_int.h"
#include "vpe_dbg.h"

//=============================================================================
// extern functions
//=============================================================================
void *vpe_uti_kmem_alloc(UINT32 mem_size)
{
	#if defined(__FREERTOS)
	return malloc(mem_size);
	#else
	return kzalloc(mem_size, GFP_KERNEL);
	#endif
}

void vpe_uti_kmem_free(void *mem_addr)
{
	#if defined(__FREERTOS)
	free(mem_addr);
	#else
	kfree(mem_addr);
	#endif
}

void *vpe_uti_vmem_alloc(UINT32 mem_size)
{
	#if defined(__FREERTOS)
	return malloc(mem_size);
	#else
	return vmalloc(mem_size);
	#endif
}

void vpe_uti_vmem_free(void *mem_addr)
{
	#if defined(__FREERTOS)
	free(mem_addr);
	#else
	vfree(mem_addr);
	#endif
}

UINT32 vpe_uti_calc_2dlut_tbl_size(UINT32 _2dlut_idx)
{
	UINT32 _2dlut_size;

	_2dlut_size = ALIGN_CEIL(_2dlut_idx, 4) * _2dlut_idx * sizeof(UINT32);

	return _2dlut_size;
}

UINT32 vpe_uti_calc_2dlut_ioctl_size(UINT32 _2dlut_idx)
{
	UINT32 _2dlut_idx_tmp;

	if (_2dlut_idx <= VPE_ISP_2DLUT_SZ_65X65) {
		_2dlut_idx_tmp = VPE_ISP_2DLUT_SZ_65X65;
	} else {
		_2dlut_idx_tmp = _2dlut_idx;
	}

	return vpe_uti_calc_2dlut_tbl_size(_2dlut_idx_tmp) + sizeof(UINT32) * 3; // lut + size + id + precision
}

static INT32 vpe_uti_sqrt_taylor(INT32 val)
{
	INT32 ans, pre = 0;

	ans = val;
	while (abs(ans - pre) > 1) {
		pre = ans;
		ans = (ans + val / ans) / 2;
	}

	return ans;
}

static void vpe_uti_unsigned_2s_comp_to_signed_num(UINT32 tbl, INT32 *px, INT32 *py)
{
	UINT32 ux, uy;

	ux = tbl & 0xFFFF;
	uy = (tbl >> 16) & 0xFFFF;

	if ((ux & 0x8000) == 0x0) {
		*px = ux;
	} else {
		*px = -(0x8000 - (ux & 0x7FFF));
	}

	if ((uy & 0x8000) == 0x0) {
		*py = uy;
	} else {
		*py = -(0x8000 - (uy & 0x7FFF));
	}
}

void vpe_uti_calc_2dlut_output_size(USIZE *dcout_size, USIZE *lut_size, USIZE *in_size, UINT8 precision, UINT32 *lut_tbl)
{
	ISIZE step; // subsample
	INT32 i, j, dcout_x = 0, dcout_y = 0, max_dcout_x = 0, max_dcout_y = 0;
	INT32 x2d[2], y2d[2];
	INT32 padded_lut_w = ((((INT32)lut_size->w + 3) >> 2) << 2);
	INT32 frac_bit_num;

	switch (precision) {
		case 0: //abs s14.2
		default:
			frac_bit_num = 2;
			break;
		case 1: //abs s15.1
			frac_bit_num = 1;
			break;
		case 2: //abs s16.0
			frac_bit_num = 0;
			break;
		case 3: //rel s2.14
			frac_bit_num = 14;
			break;
		case 4: //rel s3.13
			frac_bit_num = 13;
			break;
		case 5: //rel s4.12
			frac_bit_num = 12;
			break;
		case 6: //rel s5.11
			frac_bit_num = 11;
			break;
	};

	if (lut_size->w < 9) {
		step.w = lut_size->w - 1; //only vertices
	} else {
		step.w = (lut_size->w - 1) >> 3;
	}
	if (lut_size->h < 9) {
		step.h = lut_size->h - 1; //only vertices
	} else {
		step.h = (lut_size->h - 1) >> 3;
	}

	//horizontal
	for (j = 0; j < (INT32)lut_size->h; j += step.h) {
		dcout_x = 0;
		for (i = 0; i < (INT32)lut_size->w - step.w; i += step.w) {
			vpe_uti_unsigned_2s_comp_to_signed_num(lut_tbl[j * padded_lut_w + i], &x2d[0], &y2d[0]);
			vpe_uti_unsigned_2s_comp_to_signed_num(lut_tbl[j * padded_lut_w + (i + step.w)], &x2d[1], &y2d[1]);

			if (precision >= 3 && precision <= 6) {
				x2d[0] *= in_size->w;
				x2d[1] *= in_size->w;
				y2d[0] *= in_size->h;
				y2d[1] *= in_size->h;
			}

			x2d[0] >>= frac_bit_num; y2d[0] >>= frac_bit_num;
			x2d[1] >>= frac_bit_num; y2d[1] >>= frac_bit_num;
			dcout_x = dcout_x + vpe_uti_sqrt_taylor((x2d[1] - x2d[0]) * (x2d[1] - x2d[0]) + (y2d[1] - y2d[0]) * (y2d[1] - y2d[0]));
		}
		if (dcout_x > max_dcout_x) {
			max_dcout_x = dcout_x;
		}
	}

	//vertical
	for (i = 0; i < (INT32)lut_size->w; i += step.w) {
		dcout_y = 0;
		for (j = 0; j < (INT32)lut_size->h - step.h; j += step.h) {
			vpe_uti_unsigned_2s_comp_to_signed_num(lut_tbl[j * padded_lut_w + i], &x2d[0], &y2d[0]);
			vpe_uti_unsigned_2s_comp_to_signed_num(lut_tbl[(j + step.h) * padded_lut_w + i], &x2d[1], &y2d[1]);

			if (precision >= 3 && precision <= 6) {
				x2d[0] *= in_size->w;
				x2d[1] *= in_size->w;
				y2d[0] *= in_size->h;
				y2d[1] *= in_size->h;
			}

			x2d[0] >>= frac_bit_num; y2d[0] >>= frac_bit_num;
			x2d[1] >>= frac_bit_num; y2d[1] >>= frac_bit_num;
			dcout_y = dcout_y + vpe_uti_sqrt_taylor((x2d[1] - x2d[0]) * (x2d[1] - x2d[0]) + (y2d[1] - y2d[0]) * (y2d[1] - y2d[0]));
		}
		if (dcout_y > max_dcout_y) {
			max_dcout_y = dcout_y;
		}
	}

	dcout_size->w = ALIGN_CEIL(max_dcout_x, 8);
	dcout_size->h = ALIGN_CEIL(max_dcout_y, 2);
}
