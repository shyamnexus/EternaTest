#include "platform_port.h"
#include "h26xdec_platform.h"
#include "portab.h"

const char irq_name[H26XD_CHIP][H26XD_NUM][20] = { { "h26xdec-0_0" } };

struct h26xdec_dev_info_t vdec_clk_info[H26XD_CHIP] = {0};
static void *cg_vbase[2] = {NULL, NULL};

/* enable mclk auto gating */
int pf_h26xd_auto_clk_gate_on(int chip_idx)
{
	struct h26xdec_dev_info_t *p_vdec_clk_info = &vdec_clk_info[chip_idx];

	if ((unsigned int)chip_idx >= H26XD_CHIP) {
		printk("[DE]%s, out of range chip_idx:%d \n", __func__, chip_idx);
		return -1;
	}

	if (in_interrupt() || in_atomic() || irqs_disabled()) {
		volatile unsigned int val;

        val = ioread32((unsigned char *)cg_vbase[chip_idx] + 0xC0);
		val |= (0x3 << 15);
		iowrite32(val, (unsigned char *)cg_vbase[chip_idx] + 0xC0);

        val = ioread32((unsigned char *)cg_vbase[chip_idx] + 0xDC);
		val |= (0x1 << 11);
		iowrite32(val, (unsigned char *)cg_vbase[chip_idx] + 0xDC);
	} else {
		clk_set_phase(p_vdec_clk_info->h264d_mclk, 1);  // enable h264d_mclk
		clk_set_phase(p_vdec_clk_info->h265d_mclk, 1);  // enable h265d_mclk
        clk_set_phase(p_vdec_clk_info->h26xd_pclk, 1);  // enable h26xd_pclk
	}

	return 0;
}

/* disable mclk auto gating */
int pf_h26xd_auto_clk_gate_off(int chip_idx)
{
	struct h26xdec_dev_info_t *p_vdec_clk_info = &vdec_clk_info[chip_idx];

	if ((unsigned int)chip_idx >= H26XD_CHIP) {
		printk("[DE]%s, out of range chip_idx:%d \n", __func__, chip_idx);
		return -1;
	}

	if (in_interrupt() || in_atomic() || irqs_disabled()) {
		volatile unsigned int val;

        val = ioread32((unsigned char *)cg_vbase[chip_idx] + 0xC0);
		val &= ~(0x3 << 15);
		iowrite32(val, (unsigned char *)cg_vbase[chip_idx] + 0xC0);

        val = ioread32((unsigned char *)cg_vbase[chip_idx] + 0xDC);
		val &= ~(0x1 << 11);
		iowrite32(val, (unsigned char *)cg_vbase[chip_idx] + 0xDC);
	} else {
		clk_set_phase(p_vdec_clk_info->h264d_mclk, 0);  // disable h264d_mclk
		clk_set_phase(p_vdec_clk_info->h265d_mclk, 0);  // disable h265d_mclk
		clk_set_phase(p_vdec_clk_info->h26xd_pclk, 0);  // disable h26xd_pclk
	}

	return 0;
}

/* must work in atomic context to avoid racing condition */
int pf_h26xd_xclk_on(int chip_idx)
{
	struct h26xdec_dev_info_t *p_vdec_clk_info = &vdec_clk_info[chip_idx];

	if ((unsigned int)chip_idx >= H26XD_CHIP) {
		printk("[DE]%s, out of range chip_idx:%d \n", __func__, chip_idx);
		return -1;
	}

    /* enable all clk when first decode */
	if (p_vdec_clk_info->h26xd_clk_cnt == 0) {
		clk_enable(p_vdec_clk_info->h264d_mclk);
		clk_enable(p_vdec_clk_info->h264d_aclk);

		clk_enable(p_vdec_clk_info->h265d_mclk);
		clk_enable(p_vdec_clk_info->h265d_aclk);

        clk_enable(p_vdec_clk_info->h26xd_pclk);

		p_vdec_clk_info->h26xd_clk_cnt = 2;
	}
	return 0;
}

/* must work in atomic context to avoid racing condition */
int pf_h26xd_xclk_off(int chip_idx)
{
	struct h26xdec_dev_info_t *p_vdec_clk_info = &vdec_clk_info[chip_idx];

	if ((unsigned int)chip_idx >= H26XD_CHIP) {
		printk("[DE]%s, out of range chip_idx:%d \n", __func__, chip_idx);
		return -1;
	}

    /* disable all clk when insert driver */
	if (p_vdec_clk_info->h26xd_clk_cnt == 1) {
		p_vdec_clk_info->h26xd_clk_cnt = 0;
		clk_disable(p_vdec_clk_info->h264d_mclk);
		clk_disable(p_vdec_clk_info->h264d_aclk);

		clk_disable(p_vdec_clk_info->h265d_mclk);
		clk_disable(p_vdec_clk_info->h265d_aclk);

        clk_disable(p_vdec_clk_info->h26xd_pclk);
	}

	return 0;
}

/*
* select clock source, stop reset and enable clock
*/
int pf_h26xd_clk_on(int chip_idx)
{
	struct h26xdec_dev_info_t *p_vdec_clk_info;
	struct clk *parent_mclk;

	if ((unsigned int)chip_idx >= H26XD_CHIP) {
		printk("[DE] %s, chip_idx is out of range! \n", __func__);
		return -1;
	}

	p_vdec_clk_info = &vdec_clk_info[chip_idx];

	if (chip_idx == 0) {
		p_vdec_clk_info->h264d_mclk = clk_get(NULL, "h264d_mclk");
	} else {
		p_vdec_clk_info->h264d_mclk = clk_get(NULL, "h264d_mclk_ep0");
	}
	if (IS_ERR(p_vdec_clk_info->h264d_mclk)) {
		printk("[DE] get chip:%d h264d_mclk failed!\n", chip_idx);
		p_vdec_clk_info->h264d_mclk = 0;
		goto exit_clk;
	}

	if (chip_idx == 0) {
		p_vdec_clk_info->h265d_mclk = clk_get(NULL, "h265d_mclk");
	} else {
		p_vdec_clk_info->h265d_mclk = clk_get(NULL, "h265d_mclk_ep0");
	}
	if (IS_ERR(p_vdec_clk_info->h265d_mclk)) {
		printk("[DE] get chip:%d h265d_mclk failed!\n", chip_idx);
		p_vdec_clk_info->h265d_mclk = 0;
		goto exit_clk;
	}
	clk_prepare_enable(p_vdec_clk_info->h265d_mclk);

	p_vdec_clk_info->h26xd_pclk = clk_get(NULL, "h264d_pclk");
	if (IS_ERR(p_vdec_clk_info->h26xd_pclk)) {
		printk("[DE] get h264d_pclk failed!\n");
		p_vdec_clk_info->h26xd_pclk = 0;
		goto exit_clk;
	}
	clk_prepare_enable(p_vdec_clk_info->h26xd_pclk);

	if (chip_idx == 0) {
		p_vdec_clk_info->h264d_aclk = clk_get(NULL, "h264d_aclk");
	} else {
		p_vdec_clk_info->h264d_aclk = clk_get(NULL, "h264d_aclk_ep0");
	}
	if (IS_ERR(p_vdec_clk_info->h264d_aclk)) {
		printk("[DE] get chip:%d h264d_aclk failed!\n", chip_idx);
		p_vdec_clk_info->h264d_aclk = 0;
		goto exit_clk;
	}

	if (chip_idx == 0) {
		p_vdec_clk_info->h265d_aclk = clk_get(NULL, "h265d_aclk");
	} else {
		p_vdec_clk_info->h265d_aclk = clk_get(NULL, "h265d_aclk_ep0");
	}
	if (IS_ERR(p_vdec_clk_info->h265d_aclk)) {
		printk("[DE] chip:%d get h265d_aclk failed!\n", chip_idx);
		p_vdec_clk_info->h265d_aclk = 0;
		goto exit_clk;
	}
	clk_prepare_enable(p_vdec_clk_info->h265d_aclk);

	parent_mclk = clk_get(NULL, "pll24");
	if (IS_ERR(parent_mclk)) {
		printk("[DE]FAIL! Switch h26xd mclk to PLL24 for chip:%d!!! ", chip_idx);
	} else {
		clk_set_parent(p_vdec_clk_info->h264d_mclk, parent_mclk);
		clk_set_parent(p_vdec_clk_info->h264d_aclk, parent_mclk);
		printk("[DE]switch h26xd mclk to PLL24 for chip:%d. \n", chip_idx);
		clk_put(parent_mclk);
	}
	/* after set_parent, we can enable clock now. */
	clk_prepare_enable(p_vdec_clk_info->h264d_mclk);
	clk_prepare_enable(p_vdec_clk_info->h264d_aclk);

	/* workaround, IP hangs up!!! Must do module reset again to recover IP. It is done in atomic context. */
	if (cg_vbase[chip_idx] == NULL) {
#ifdef __aarch64__
		uintptr_t cg_paddr = 0x2f0020000;
#else
		uintptr_t cg_paddr = 0xf0020000;
#endif
		cg_vbase[chip_idx] = ioremap(cg_paddr, PAGE_SIZE);
	}

	pf_h26xd_auto_clk_gate_on(chip_idx);

	p_vdec_clk_info->h26xd_clk_cnt = 1;

	return 0;

exit_clk:
	pf_h26xd_clk_off(chip_idx);
	return -1;

}

/*
* disable clock and assert module reset. Only called when driver cleanup
*/
int pf_h26xd_clk_off(int chip_idx)
{
	struct h26xdec_dev_info_t *p_vdec_clk_info = &vdec_clk_info[chip_idx];

	if (p_vdec_clk_info->h264d_mclk) {
		if (p_vdec_clk_info->h26xd_clk_cnt == 0) {
			clk_unprepare(p_vdec_clk_info->h264d_mclk);
		} else {
			clk_disable_unprepare(p_vdec_clk_info->h264d_mclk);
		}
		clk_put(p_vdec_clk_info->h264d_mclk);
		p_vdec_clk_info->h264d_mclk = 0;
	}

	if (p_vdec_clk_info->h265d_mclk) {
		if (p_vdec_clk_info->h26xd_clk_cnt == 0) {
			clk_unprepare(p_vdec_clk_info->h265d_mclk);
		} else {
			clk_disable_unprepare(p_vdec_clk_info->h265d_mclk);
		}
		clk_put(p_vdec_clk_info->h265d_mclk);
		p_vdec_clk_info->h265d_mclk = 0;
	}


	if (p_vdec_clk_info->h26xd_pclk) {
		if (p_vdec_clk_info->h26xd_clk_cnt == 0) {
			clk_unprepare(p_vdec_clk_info->h26xd_pclk);
		} else {
			clk_disable_unprepare(p_vdec_clk_info->h26xd_pclk);
		}
		clk_put(p_vdec_clk_info->h26xd_pclk);
		p_vdec_clk_info->h26xd_pclk = 0;
	}

	if (p_vdec_clk_info->h264d_aclk) {
		if (p_vdec_clk_info->h26xd_clk_cnt == 0) {
			clk_unprepare(p_vdec_clk_info->h264d_aclk);
		} else {
			clk_disable_unprepare(p_vdec_clk_info->h264d_aclk);
		}
		clk_put(p_vdec_clk_info->h264d_aclk);
		p_vdec_clk_info->h264d_aclk = 0;
	}

	if (p_vdec_clk_info->h265d_aclk) {
		if (p_vdec_clk_info->h26xd_clk_cnt == 0) {
			clk_unprepare(p_vdec_clk_info->h265d_aclk);
		} else {
			clk_disable_unprepare(p_vdec_clk_info->h265d_aclk);
		}
		clk_put(p_vdec_clk_info->h265d_aclk);
		p_vdec_clk_info->h265d_aclk = 0;
	}

	p_vdec_clk_info->h26xd_clk_cnt = 0;

	pf_h26xd_auto_clk_gate_off(chip_idx);

	return 0;
}

/* return hz */
unsigned int pf_h26xd_get_plck(int chip_idx)
{
	unsigned int clk_rate;

	struct h26xdec_dev_info_t *p_vdec_clk_info = &vdec_clk_info[chip_idx];

	if (p_vdec_clk_info->h26xd_pclk) {
		clk_rate = (unsigned int)clk_get_rate(p_vdec_clk_info->h26xd_pclk);

		if (clk_rate == 0)
			clk_rate = 150000000;

		return clk_rate;
	}

	return 0;
}
