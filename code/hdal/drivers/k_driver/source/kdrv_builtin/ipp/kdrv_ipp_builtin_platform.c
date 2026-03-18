#include "kdrv_ipp_builtin.h"
#include "kdrv_ipp_builtin_int.h"
#if defined (__KERNEL__)
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
//#include <plat/nvt-sramctl.h>  //tmp need to check
#else
#include <malloc.h>
#include <string.h>
#include "interrupt.h"
#include <stdlib.h>
#include <plat/rtosfdt.h>
#include <compiler.h>
#include <libfdt.h>
#include <nvtmpp_init.h>
#include "pll_protected.h"
#include "pll.h"
#include <rcw_macro.h>
#endif

#include <libfdt.h>
#include <fdtfast.h>

extern UINT32 (*kdrv_ipp_builtin_get_reg_num)(KDRV_IPP_BUILTIN_ENG eng);


unsigned int builtin_ipp_debug_level = NVT_DBG_ERR;

void kdrv_ipp_builtin_ife_shared_sram_clksel(void)
{

#if defined (__KERNEL__)
	struct clk *parent_clk;
	struct clk *ife_sram_clk;

	parent_clk   = clk_get(NULL, "ife_sram_ife");
	if (IS_ERR(parent_clk))
		DBG_ERR("getting parent_clk error\n");

	ife_sram_clk = clk_get(NULL, "ife_shared_sram");
	if (IS_ERR(ife_sram_clk))
		DBG_ERR("getting ife_sram_clk error\n");

	clk_set_parent(ife_sram_clk, parent_clk);


#else

	pll_setClockRate(PLL_CLKSEL_IFE_SRAM, PLL_CLKSEL_IFE_SRAM_IFE);

#endif

	return;
}

#if 0 //538 removed
void kdrv_ipp_builtin_ipe_shared_sram_clksel(void)
{

#if defined (__KERNEL__)
	struct clk *parent_clk;
	struct clk *ipe_sram_clk;

	parent_clk   = clk_get(NULL, "ipe_sram_ipe");
	if (IS_ERR(parent_clk))
		DBG_ERR("getting parent_clk error\n");

	ipe_sram_clk = clk_get(NULL, "ipe_shared_sram");
	if (IS_ERR(ipe_sram_clk))
		DBG_ERR("getting ipe_sram_clk error\n");

	clk_set_parent(ipe_sram_clk, parent_clk);


#else
	//wait for porting
	//pll_setClockRate(PLL_CLKSEL_IPE_SRAM, PLL_CLKSEL_IPE_SRAM_IFE);

#endif

	return;
}

void kdrv_ipp_builtin_ime_shared_sram_clksel(void)
{

#if defined (__KERNEL__)
	struct clk *parent_clk;
	struct clk *ime_sram_clk;

	parent_clk   = clk_get(NULL, "ime_sram_ime");
	if (IS_ERR(parent_clk))
		DBG_ERR("getting parent_clk error\n");

	ime_sram_clk = clk_get(NULL, "ime_shared_sram");
	if (IS_ERR(ime_sram_clk))
		DBG_ERR("getting ime_sram_clk error\n");

	clk_set_parent(ime_sram_clk, parent_clk);


#else
    //wait for porting
	//pll_setClockRate(PLL_CLKSEL_IME_SRAM, PLL_CLKSEL_IME_SRAM_IFE);

#endif

	return;
}
#endif

#if 0 // 567 removed
void kdrv_ipp_builtin_cnn_dce_shared_sram_clksel(void)
{

#if defined (__KERNEL__)
	struct clk *parent_clk, *cnn_sram_clk;
	/* Get parent clk
	 * The name options are "cnn_sram_cnn", "cnn_sram_dce", "cnn_sram_nue"
	*/
	parent_clk = clk_get(NULL, "cnn_sram_dce");
	if (IS_ERR(parent_clk))
		pr_err("getting parent_clk error\n");


	/* Get cnn sram control node*/
	cnn_sram_clk= clk_get(NULL, "cnn_shared_sram");
	if (IS_ERR(cnn_sram_clk))
		pr_err("getting cnn_sram_clk error\n");


	/* Set clk select*/
	clk_set_parent(cnn_sram_clk, parent_clk);
#else

	pll_setClockRate(PLL_CLKSEL_CNN_SRAM, PLL_CLKSEL_CNN_SRAM_DCE);

#endif


	return;
}
#endif

void* kdrv_ipp_builtin_plat_ioremap_nocache(ULONG addr, UINT32 size)
{
#if defined (__KERNEL__)
	return ioremap(addr, size);
#else
	return (void *)addr;
#endif
}

void kdrv_ipp_builtin_plat_iounmap(void *addr)
{
#if defined (__KERNEL__)
	iounmap(addr);
#else
#endif
}


INT32 kdrv_ipp_builtin_plat_init_clk(void)
{
#if defined (__KERNEL__)
	CHAR *engine_clk_name[KDRV_IPP_BUILTIN_ENG_MAX] = {
		"2f0340000.ife",
		"2f0400000.ipe",
		"2f0410000.ime",
	};
	struct clk *source_clk = NULL;
	struct clk *eng_clk = NULL;
	struct device_node *of_node = NULL;
	UINT32 clk = 450000000;

	UINT32 i;

	of_node = of_find_node_by_path("/kdrv_ipp");
	if (of_node) {
		if (of_property_read_u32(of_node, "clock-frequency", &clk) != 0) {
			DBG_WRN("cannot find /kdrv_ipp/clock-frequency\r\n");
		}
	}

	clk /= 1000000;

	if (clk == 240) {
		source_clk = clk_get(NULL, "fix240m");
	} else if (clk == 320) {
		source_clk = clk_get(NULL, "fix320m");
	} else {
		source_clk = clk_get(NULL, "pll13");
	}

	if (IS_ERR(source_clk)) {
		DBG_ERR("get source_clk clk failed\r\n");
		return -1;
	}

	for (i = 0; i < KDRV_IPP_BUILTIN_ENG_MAX; i++) {
		eng_clk = clk_get(NULL, engine_clk_name[i]);
		if (IS_ERR(eng_clk)) {
			DBG_ERR("get clk %s failed\r\n", engine_clk_name[i]);
		} else {
			clk_set_parent(eng_clk, source_clk);
			clk_prepare(eng_clk);
			clk_enable(eng_clk);
			clk_put(eng_clk);
		}
	}
	clk_put(source_clk);
	KDRV_IPP_BUILTIN_DUMP("ipp engine clock init end\r\n");
#else
	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
	UINT32 clk = 450000000;

	// get linux space
	if (p_fdt != NULL) {
		nodeoffset = fdt_path_offset(p_fdt, "/kdrv_ipp");
		if (nodeoffset < 0) {
			DBG_WRN("failed to offset for  %s = %d \r\n", "/kdrv_ipp", nodeoffset);
		} else {
			nodep = fdt_getprop(p_fdt, nodeoffset, "clock-frequency", &len);
			if (len == 0 || nodep == NULL) {
				DBG_WRN("failed to access clock-frequency.\r\n");
			} else {
				clk = be32_to_cpu(*(unsigned int *)nodep);
			}
		}
	} else {
		DBG_WRN("null fdt_base\r\n");
	}

	clk /= 1000000;

	if (clk == 240) {
		pll_setClockRate(PLL_CLKSEL_IFE, PLL_CLKSEL_IFE_240);
		pll_setClockRate(PLL_CLKSEL_IPE, PLL_CLKSEL_IPE_240);
		pll_setClockRate(PLL_CLKSEL_IME, PLL_CLKSEL_IME_240);
	} else if (clk == 320) {
		pll_setClockRate(PLL_CLKSEL_IFE, PLL_CLKSEL_IFE_320);
		pll_setClockRate(PLL_CLKSEL_IPE, PLL_CLKSEL_IPE_320);
		pll_setClockRate(PLL_CLKSEL_IME, PLL_CLKSEL_IME_320);
	} else {
		pll_setClockRate(PLL_CLKSEL_IFE, PLL_CLKSEL_IFE_PLL13);
		pll_setClockRate(PLL_CLKSEL_IPE, PLL_CLKSEL_IPE_PLL13);
		pll_setClockRate(PLL_CLKSEL_IME, PLL_CLKSEL_IME_PLL13);
	}

	pll_enableClock(IFE_CLK);
	pll_enableClock(IPE_CLK);
	pll_enableClock(IME_CLK);


#endif
	return 0;
}

INT32 kdrv_ipp_builtin_get_dtsi_node(CHAR *path, KDRV_IPP_BUILTIN_DTSI_NODE_INFO *node)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	int nodeoffset = 0;

	if (p_fdt== NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return E_SYS;
	}

	nodeoffset = fdt_path_offset(p_fdt, path);

	if (nodeoffset < 0) {
		//DBG_DUMP("failed to offset for %s = %d \n", path, nodeoffset);
		return E_SYS;
	}
	node->p_fdt = p_fdt;
	node->root_nodeoffset = nodeoffset;
	return E_OK;
}

INT32 kdrv_ipp_builtin_get_root_node_info(CHAR *path, KDRV_IPP_BUILTIN_DTSI_NODE_INFO* root_node)
{
	root_node->p_fdt = fdtfast_get_base();

	if (root_node->p_fdt== NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return E_SYS;
	}

	root_node->root_nodeoffset = fdt_path_offset(root_node->p_fdt, path);

	return E_OK;
}

INT32 kdrv_ipp_builtin_get_child_node(KDRV_IPP_BUILTIN_DTSI_NODE_INFO* root_node, CHAR *path, KDRV_IPP_BUILTIN_DTSI_NODE_INFO* child_node)
{

	if (root_node->p_fdt== NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return E_SYS;
	}

	child_node->p_fdt = root_node->p_fdt;
	child_node->root_nodeoffset = fdt_subnode_offset(root_node->p_fdt, root_node->root_nodeoffset, path);

	return E_OK;
}

INT32 kdrv_ipp_builtin_set_child_nodeofsset(KDRV_IPP_BUILTIN_DTSI_NODE_INFO* root_node, CHAR *path)
{
	if (root_node->p_fdt== NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return E_SYS;
	}

	if (root_node->root_nodeoffset < 0) {
		DBG_ERR("%s root_nodeoffset < 0 \n", path);
		return E_SYS;
	}

	root_node->child_nodeoffset = fdt_subnode_offset(root_node->p_fdt, root_node->root_nodeoffset, path);

	return E_OK;
}

INT32 kdrv_ipp_builtin_read_dtsi_array(KDRV_IPP_BUILTIN_DTSI_NODE_INFO *node, const CHAR *tag, UINT32 *buf, UINT32 num)
{
	const void *nodep;  /* property node pointer */
	int i, len;

	nodep = fdt_getprop(node->p_fdt, node->root_nodeoffset, tag, &len);
	if (len == 0 || nodep == NULL) {
		//DBG_ERR("len = %d, nodep = 0x%x\r\n", len, nodep);
		return E_SYS;
	}
	if (len != num * 4) {
		//DBG_ERR("len = %d, num*4 = %d\r\n", len, num * 4);
		return E_SYS;
	}
	for (i = 0; i < num; i++) {
		*(buf + i) = be32_to_cpu(*((unsigned int *)nodep + i));
	}
	return E_OK;
}

INT32 kdrv_ipp_builtin_plat_read_dtsi_array(CHAR *node, CHAR *tag, UINT32 *buf, UINT32 num)
{

	KDRV_IPP_BUILTIN_DTSI_NODE_INFO dtsi_node;

	if (kdrv_ipp_builtin_get_dtsi_node(node, &dtsi_node) != E_OK) {
		DBG_ERR("Failed to find node by path: %s.\r\n", node);
		return -1;
	}

	if (kdrv_ipp_builtin_read_dtsi_array(&dtsi_node, tag, buf, num) == E_OK) {
		;
	}

	return E_OK;

}

INT32 kdrv_ipp_builtin_plat_read_dtsi_array_ex(KDRV_IPP_BUILTIN_DTSI_NODE_INFO* root_node, CHAR *tag, UINT32 *buf, UINT32 num)
{
	KDRV_IPP_BUILTIN_DTSI_NODE_INFO child_node = {0};

	if (root_node->p_fdt== NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return E_SYS;
	}

	if (root_node->root_nodeoffset < 0) {
		DBG_ERR("root_nodeoffset < 0 for %s\n", tag);
		return E_SYS;
	}

	if (root_node->child_nodeoffset < 0) {
		DBG_ERR("child_nodeoffset < 0 for %s\n", tag);
		return E_SYS;
	}

	child_node.p_fdt = root_node->p_fdt;
	child_node.root_nodeoffset = root_node->child_nodeoffset;

	if (kdrv_ipp_builtin_read_dtsi_array(&child_node, tag, buf, num) == E_OK) {
		;
	}

	return E_OK;

}

INT32 kdrv_ipp_builtin_plat_read_dtsi_string(CHAR *node, CHAR *tag, const CHAR **str)
{

	return 0;

}

static irqreturn_t ime_builtin_isr(int irq, void *param)
{
	IME_BUILTIN_HANDLE *p_hdl = (IME_BUILTIN_HANDLE *)param;

#if defined (__KERNEL__)
	UINT32 sts;
	UINT32 inte;

	if (p_hdl->fastboot_cb) {
		/* help get status for fastboot callback */
		sts = ioread32(p_hdl->resource.p_io_addr + 0x1C);
		inte = ioread32(p_hdl->resource.p_io_addr + 0x18);

		sts &= inte;
		if (sts) {
			iowrite32(sts, p_hdl->resource.p_io_addr + 0x1C);

			/* check if switch back to hdal flow */
			if (p_hdl->switch_to_hdal && (sts & IME_BUILTIN_INTS_FRM_END)) {
				sts |= IME_BUILTIN_INTS_FB_FRM_END;
			}
			p_hdl->fastboot_cb(sts);
			if (sts & IME_BUILTIN_INTS_FB_FRM_END) {
				p_hdl->fastboot_cb = NULL;
			}
		}
	} else if (p_hdl->hdal_cb) {
		p_hdl->hdal_cb(p_hdl->dev_id);
	}else{

		sts = ioread32(p_hdl->resource.p_io_addr + 0x1C); // 567 need to change
		inte = ioread32(p_hdl->resource.p_io_addr + 0x18); // 567 need to change

		sts &= inte;
		if (sts)
			iowrite32(sts, p_hdl->resource.p_io_addr + 0x1C); // 567 need to change
	}
#else
	UINT32 sts;
	UINT32 inte;

#if 1
	if (p_hdl->fastboot_cb) {
		/* help get status for fastboot callback */

		sts = INW(((ULONG)p_hdl->resource.p_io_addr + 0x1C));
		inte = INW(((ULONG)p_hdl->resource.p_io_addr + 0x18));

		sts &= inte;
		if (sts) {
			OUTW( ((ULONG)p_hdl->resource.p_io_addr + 0x1C) ,sts);

			/* check if switch back to hdal flow */
			if (p_hdl->switch_to_hdal && (sts & IME_BUILTIN_INTS_FRM_END)) {
				sts |= IME_BUILTIN_INTS_FB_FRM_END;
			}
			p_hdl->fastboot_cb(sts);

			if (sts & IME_BUILTIN_INTS_FB_FRM_END) {
				p_hdl->fastboot_cb = NULL;
			}
		}
	} else if (p_hdl->hdal_cb) {
		p_hdl->hdal_cb(p_hdl->dev_id);
	}
#endif
#endif
	return IRQ_HANDLED;
}

void kdrv_ipp_builtin_plat_request_ime_irq(INT32 irq_id, CHAR *name, void *param)
{
	int ret = 0;
	ret = request_irq(irq_id, ime_builtin_isr, IRQF_TRIGGER_HIGH, name, param);

	if (ret != 0) {
		DBG_ERR("ime_builtin request_irq fail\n");
	}
}

void kdrv_ipp_builtin_plat_release_irq(INT32 irq_id, void *param)
{
	free_irq(irq_id, param);
}

void *kdrv_ipp_builtin_plat_malloc(UINT32 size)
{
#if defined (__KERNEL__)
	return kzalloc((size), GFP_KERNEL);
#elif defined(__FREERTOS)
	return malloc(size);
#endif
}

void kdrv_ipp_builtin_plat_free(void *ptr)
{
#if defined (__KERNEL__)
	kfree(ptr);
#else
#endif
}

void kdrv_ipp_builtin_setreg(uintptr_t ofs, UINT32 value)
{
#if defined (__LINUX)
	iowrite32(value, (void*)(ofs));
#elif defined (__FREERTOS)
	OUTW((ofs), value);
#endif
}

UINT32 kdrv_ipp_builtin_getreg(uintptr_t ofs)
{
#if defined (__LINUX)
	return ioread32((void*)(ofs));
#elif defined (__FREERTOS)
	return INW(ofs);
#endif
}

#if 0
#endif

#if defined (__KERNEL__)
EXPORT_SYMBOL(kdrv_ipp_builtin_update_timestamp);
EXPORT_SYMBOL(kdrv_ipp_builtin_exit);
EXPORT_SYMBOL(kdrv_ipp_builtin_is_direct);
EXPORT_SYMBOL(kdrv_ipp_builtin_is_fastboot);
EXPORT_SYMBOL(kdrv_ipp_builtin_unlock_3dnr_ref);
EXPORT_SYMBOL(kdrv_ipp_builtin_unlock_queued_buf);
EXPORT_SYMBOL(kdrv_ipp_builtin_get_phy_out_info);
EXPORT_SYMBOL(kdrv_ipp_builtin_get_status);

EXPORT_SYMBOL(kdrv_ipp_builtin_get_reg_num);
EXPORT_SYMBOL(kdrv_ipp_builtin_reg_dtsi_cb);
EXPORT_SYMBOL(kdrv_ipp_builtin_get_hdal_reg_dtsi);
EXPORT_SYMBOL(kdrv_ipp_builtin_get_hdal_hdl_list);
EXPORT_SYMBOL(kdrv_ipp_builtin_frc_dump);

EXPORT_SYMBOL(kdrv_ipp_builtin_set);
EXPORT_SYMBOL(kdrv_ipp_builtin_get);
EXPORT_SYMBOL(ime_builtin_reg_isr_cb);
EXPORT_SYMBOL(ime_builtin_set_switch_to_hdal);
EXPORT_SYMBOL(ime_builtin_get_resource);
EXPORT_SYMBOL(kdrv_ipp_builtin_get_isp_info);
#endif
