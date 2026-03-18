
#include <string.h>
#include "kdrv_builtin/nvtmpp_init.h"
#include "vendor_isp.h"
#include "isp_builtin.h"
#include "fast_awb.h"

//=============================================================================
// struct & enum definition
//=============================================================================
#define AWB_OB_VALUE        64  // 8-bit
#define AWB_IMG_W           512
#define AWB_IMG_H           512

#define AWB_GRAY_ENABLE     	0
#define RGB_PRINT_MSG       	0
#define RAW_PRINT_MSG       	0
#define CA_PRINT_MSG        	0
#define WHITE_POINT_PRINT_MSG	0
#define LINUX_ENV          		0
#define USING_NVTMPP        	1

//=============================================================================
// global
//=============================================================================
#if (!AWB_GRAY_ENABLE)
static AWB_TH awb_th = {5, 180, 101, 257, -40, 106, 56, 164, 45, 94, 40, 65};  // os04e10
#endif

//=============================================================================
// internal functions
//=============================================================================

//=============================================================================
// external functions
//=============================================================================
void fast_awb_flow(HD_VIDEO_FRAME *video_frame, UINT32 *r_gain, UINT32 *g_gain, UINT32 *b_gain)
{
	UINT32 i, j, n, m;
	UINT16 r = 0, g = 0, b = 0;
	UINT16 g2r = 0, g2b = 0;
	#if (!AWB_GRAY_ENABLE)
	UINT16 y = 0, r2g = 0, b2g = 0, rpb2g = 0, rsb2g = 0, rmb2gg = 0;
	#endif
	UINT32 gray_g2r_sum = 0, gray_g2b_sum = 0;
	UINT16 gray_w_cnt = 0;
	UINT8 *src_raw = NULL;
	#if (USING_NVTMPP)
	UINT32 *rgb_data = NULL, *rgb_data_r = NULL, *rgb_data_g = NULL, *rgb_data_b = NULL;
	UINT32 *ca_data = NULL, *ca_data_r = NULL, *ca_data_g = NULL, *ca_data_b = NULL;
	#else
	static UINT32 rgb_data[AWB_IMG_W * AWB_IMG_H][3];
	static UINT32 ca_data[AWB_CA_W_WINNUM][AWB_CA_H_WINNUM][3];
	#endif

	#if (USING_NVTMPP)
	rgb_data = (UINT32 *)nvtmpp_get_fastboot_blk(sizeof(UINT32) * AWB_IMG_W * AWB_IMG_H * 3);
	if (rgb_data == NULL) {
		printf("get rgb_data fail \n");
		return;
	}
	rgb_data_r = rgb_data;
	rgb_data_g = rgb_data + AWB_IMG_W * AWB_IMG_H;
	rgb_data_b = rgb_data + AWB_IMG_W * AWB_IMG_H * 2;
	ca_data = (UINT32 *)nvtmpp_get_fastboot_blk(sizeof(UINT32) * AWB_CA_W_WINNUM * AWB_CA_H_WINNUM * 3);
	if (ca_data == NULL) {
		printf("get ca_data fail \n");
		return;
	}
	ca_data_r = ca_data;
	ca_data_g = ca_data + AWB_CA_W_WINNUM * AWB_CA_H_WINNUM;
	ca_data_b = ca_data + AWB_CA_W_WINNUM * AWB_CA_H_WINNUM * 2;
	#endif

	#if (USING_NVTMPP)
	memset(rgb_data, 0, sizeof(UINT32) * AWB_IMG_W * AWB_IMG_H * 3);
	memset(ca_data, 0, sizeof(UINT32) * AWB_CA_W_WINNUM * AWB_CA_H_WINNUM * 3);
	#else
	memset(&rgb_data, 0, sizeof(rgb_data));
	memset(&ca_data, 0, sizeof(ca_data));
	#endif

	// NOTE: RAW -> CA
	#if (LINUX_ENV)  // Linux
	src_raw = (UINT8 *)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, video_frame->phy_addr[0], video_frame->loff[0] * video_frame->ph[0]);
	#else  // RTOS
	src_raw = (UINT8 *)video_frame->phy_addr[0];
	#endif

	#if (RAW_PRINT_MSG)
	printf("(%d)\r\n",*(src_raw));
	#endif

	for (j = 0; j < video_frame->ph[0] / 2; j ++) {
		for (i = 0; i < video_frame->pw[0] / 2; i ++) {
			#if (USING_NVTMPP)
			*(rgb_data_r + (j * video_frame->pw[0] / 2 + i)) = *(src_raw +((j * 2) * video_frame->loff[0]) + i * 2) - AWB_OB_VALUE;
			*(rgb_data_g + (j * video_frame->pw[0] / 2 + i)) = ((*(src_raw +((j * 2) * video_frame->loff[0]) + i * 2 + 1) + *(src_raw +((j * 2 + 1) * video_frame->loff[0]) + i * 2)) >> 1) - AWB_OB_VALUE;
			*(rgb_data_b + (j * video_frame->pw[0] / 2 + i)) = *(src_raw +((j * 2 + 1) * video_frame->loff[0]) + i * 2 + 1) - AWB_OB_VALUE;
			#else
			rgb_data[j * video_frame->pw[0] / 2 + i][0] = *(src_raw +((j * 2) * video_frame->loff[0]) + i * 2) - AWB_OB_VALUE;
			rgb_data[j * video_frame->pw[0] / 2 + i][1] = ((*(src_raw +((j * 2) * video_frame->loff[0]) + i * 2 + 1) + *(src_raw +((j * 2 + 1) * video_frame->loff[0]) + i * 2)) >> 1) - AWB_OB_VALUE;
			rgb_data[j * video_frame->pw[0] / 2 + i][2] = *(src_raw +((j * 2 + 1) * video_frame->loff[0]) + i * 2 + 1) - AWB_OB_VALUE;
			#endif
		}
	}

	#if (RGB_PRINT_MSG)
	#if (USING_NVTMPP)
	printf("rgb (%d, %d, %d),(%d, %d, %d),(%d, %d, %d)\r\n",*rgb_data_r, *(rgb_data_g), *(rgb_data_b), *(rgb_data_r + 32769), *(rgb_data_g + 32769), *(rgb_data_b + 32769), *(rgb_data_r + 65535), *(rgb_data_g + 65535), *(rgb_data_b + 65535));
	#else
	printf("rgb (%d, %d, %d),(%d, %d, %d),(%d, %d, %d)\r\n",rgb_data[0][0], rgb_data[0][1], rgb_data[0][2], rgb_data[32769][0], rgb_data[32769][1], rgb_data[32769][2], rgb_data[65535][0], rgb_data[65535][1], rgb_data[65535][2]);
	#endif
	#endif
	{
		for (j = 0; j < AWB_CA_H_WINNUM; j++) {
			for (i = 0; i < AWB_CA_W_WINNUM; i++) {
				for (n = 0; n < 8; n++) {
					for (m = 0; m < 8; m++) {
						#if (USING_NVTMPP)
						*(ca_data_r + (j * 8 + i)) = *(ca_data_r + (j * 8 + i)) + *(rgb_data_r + video_frame->ph[0] / 2 * (j * 8 + n) + (i * 8 + m));
						*(ca_data_g + (j * 8 + i)) = *(ca_data_g + (j * 8 + i)) + *(rgb_data_g + video_frame->ph[0] / 2 * (j * 8 + n) + (i * 8 + m));
						*(ca_data_b + (j * 8 + i)) = *(ca_data_b + (j * 8 + i)) + *(rgb_data_b + video_frame->ph[0] / 2 * (j * 8 + n) + (i * 8 + m));
						#else
						ca_data[j][i][0] = ca_data[j][i][0] + rgb_data[video_frame->ph[0] / 2 * (j * 8 + n) + (i * 8 + m)][0];
						ca_data[j][i][1] = ca_data[j][i][1] + rgb_data[video_frame->ph[0] / 2 * (j * 8 + n) + (i * 8 + m)][1];
						ca_data[j][i][2] = ca_data[j][i][2] + rgb_data[video_frame->ph[0] / 2 * (j * 8 + n) + (i * 8 + m)][2];
						#endif
					}
				}

				#if (USING_NVTMPP)
				*(ca_data_r + (j * 8 + i)) = *(ca_data_r + (j * 8 + i)) / 64;
				*(ca_data_g + (j * 8 + i)) = *(ca_data_g + (j * 8 + i)) / 64;
				*(ca_data_b + (j * 8 + i)) = *(ca_data_b + (j * 8 + i)) / 64;
				if ((*(ca_data_r + (j * 8 + i)) > 255) || (*(ca_data_g + (j * 8 + i)) > 255) || (*(ca_data_g + (j * 8 + i)) > 255))
				{
					#if (CA_PRINT_MSG)
					printf("ca (%d,%d) (%d, %d, %d)\r\n", (int)j, (int)i, *(ca_data_r + (j * 8 + i)), *(ca_data_g + (j * 8 + i)), *(ca_data_b + (j * 8 + i)));
					#endif
				}
				#else
				ca_data[j][i][0] = ca_data[j][i][0] / 64;
				ca_data[j][i][1] = ca_data[j][i][1] / 64;
				ca_data[j][i][2] = ca_data[j][i][2] / 64;
				if ((ca_data[j][i][0] > 255) || (ca_data[j][i][1] > 255) || (ca_data[j][i][2] > 255))
				{
					#if (CA_PRINT_MSG)
					printf("ca (%d,%d) (%d, %d, %d)\r\n", (int)j, (int)i, ca_data[j][i][0], ca_data[j][i][1], ca_data[j][i][2]);
					#endif
				}
				#endif

			}
		}
	}

	for (j = 0; j < AWB_CA_H_WINNUM; j++) {
		for (i = 0; i < AWB_CA_W_WINNUM; i++) {
			#if (USING_NVTMPP)
			r = *(ca_data + (j * 8 + i));
			g = *(ca_data + AWB_CA_H_WINNUM * AWB_CA_H_WINNUM + (j * 8 + i));
			b = *(ca_data + AWB_CA_H_WINNUM * AWB_CA_H_WINNUM * 2 + (j * 8 + i));
			#else
			r = ca_data[j][i][0];
			g = ca_data[j][i][1];
			b = ca_data[j][i][2];
			#endif
			if (r == 0 || g == 0 || b == 0) {
				continue;
			}

			#if (AWB_GRAY_ENABLE)
			g2r = (g << 8) / r;
			g2b = (g << 8) / b;

			// Gray world process
			gray_g2r_sum += g2r;
			gray_g2b_sum += g2b;
			gray_w_cnt++;
			#else
			y = (r + b + g + g) >> 2;
			r2g = (r << 7) / g;
			b2g = (b << 7) / g;
			rpb2g = r2g + b2g;
			rsb2g = r2g - b2g;
			rmb2gg = (r2g * b2g) >> 7;
			if ((y > (awb_th.y_l)) && (y < (awb_th.y_u)) && (rpb2g > awb_th.rpb_l) && (rpb2g < awb_th.rpb_u)
				&& (rsb2g > awb_th.rsb_l) && (rsb2g < awb_th.rsb_u) && (r2g > awb_th.r2g_l) && (r2g < awb_th.r2g_u)
				&& (b2g > awb_th.b2g_l) && (b2g < awb_th.b2g_u) && (rmb2gg > awb_th.rmb_l) && (rmb2gg < awb_th.rmb_u)) {
				g2r = (g << 8) / r;
				g2b = (g << 8) / b;

				gray_g2r_sum += g2r;
				gray_g2b_sum += g2b;
				gray_w_cnt++;
			}
			#endif
		}
	}

	#if (WHITE_POINT_PRINT_MSG)
	printf("white point = %d\r\n",gray_w_cnt);
	#endif

	#if (USING_NVTMPP)
	if (rgb_data != NULL) {
		nvtmpp_unlock_fastboot_blk((uintptr_t)rgb_data);
		rgb_data = NULL;
	}
	if (ca_data != NULL) {
		nvtmpp_unlock_fastboot_blk((uintptr_t)ca_data);
		ca_data = NULL;
	}
	#endif

	if(gray_w_cnt == 0) {
		printf("AWB FREEZE \n");
		//*r_gain = 400;
		//*g_gain = 256;
		//*b_gain = 512;
		return;
	}

	*r_gain = gray_g2r_sum / gray_w_cnt;
	*g_gain = 256;
	*b_gain = gray_g2b_sum / gray_w_cnt;

	return;
}

