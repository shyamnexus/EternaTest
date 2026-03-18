/**
	@brief Source file of sorting top-N classes of accuracy.

	@file ai_cpu_post_accuracy.c

	@ingroup ai_cpu

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include <arm_neon.h>
#include <string.h>
#include "hd_type.h"
#include "ai_cpu_post_accuracy.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_ai_net/nn_verinfo.h"
#include "vendor_ai_net/nn_net.h"
#include "vendor_ai_net/nn_parm.h"

//=============================================================
#define __CLASS__ 				"[ai][lib][cpu]"

#define SWAP_N(a, b, t)       (t) = (a); (a) = (b); (b) = (t)
#define MIN_N(a, b)           ((a) < (b) ? (a) : (b))

/*-----------------------------------------------------------------------------*/
/* Local Functions                                                             */
/*-----------------------------------------------------------------------------*/
static VOID ai_net_sort_result(FLOAT *p_in, INT32 len, INT32 *p_idx, INT32 top_n, AI_NET_OUTPUT_CLASSS *p_classes)
{	
	INT32 tmp;
	INT32 i, j;

	for (i = 0; i < len; i++) {
		p_idx[i] = i;
	}

	// sort top results
	for (i = 0; i < top_n; i++) {
		for (j = len - 1; j > i; j--) {
			if (p_in[p_idx[j]] > p_in[p_idx[j - 1]]) {
				SWAP_N(p_idx[j], p_idx[j - 1], tmp);
			}
		}
	}

	// copy top results to output buffer
	for (i = 0; i < top_n; i++) {
		p_classes[i].no = p_idx[i];
		p_classes[i].score = p_in[p_idx[i]];

		//DBG_IND("no=%ld, score=%f\r\n", p_classes[i].no, p_classes[i].score);
	}
}

HD_RESULT ai_net_accuracy_process(AI_NET_ACCURACY_PARM *p_parm)
{
	FLOAT *p_in     = (FLOAT *)p_parm->in_addr;

	AI_NET_OUTPUT_CLASSS *p_classes = p_parm->classes;
	INT32 num       = p_parm->shape.num;
	INT32 channels  = p_parm->shape.channels;
	INT32 height    = p_parm->shape.height;
	INT32 width     = p_parm->shape.width;
	INT32 top_n     = p_parm->top_n;
	INT32 *p_idx    = p_parm->class_idx;
	INT32 i, j, k;

	if (width == 1 && height == 1) {
		top_n = MIN_N(channels, top_n);
		for (i = 0; i < num; i++) {
			ai_net_sort_result(p_in, channels, p_idx, top_n, p_classes);

			p_in += channels;
			p_classes += top_n;
		}
	} else {
		top_n = MIN_N(width, top_n);
		for (i = 0; i < num; i++) {
			for (j = 0; j < channels; j++) {
				for (k = 0; k < height; k++) {
					ai_net_sort_result(p_in, width, p_idx, top_n, p_classes);

					p_in += width;
					p_classes += top_n;
				}
			}
		}
	}
	p_parm->top_n = top_n;
	return HD_OK;
}


