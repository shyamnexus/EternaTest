#include <linux/dma-buf.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_clk.h>
#include <linux/of_platform.h>
#include <linux/parser.h>
#include <linux/regulator/consumer.h>
#include <linux/soc/nvt/fmem.h>
#include "nvt_simfb.h"

struct dma_buf *nvt_dma_buf = NULL;
static DEFINE_MUTEX(nvtsimfb_lock);
int in_used[FB_MAX] = {0};

static const struct fb_fix_screeninfo nvtsimfb_fix = {
	.id		= "simple",
	.type		= FB_TYPE_PACKED_PIXELS,
	.visual		= FB_VISUAL_TRUECOLOR,
	.accel		= FB_ACCEL_NONE,
};

static const struct fb_var_screeninfo nvtsimfb_var = {
	.height		= -1,
	.width		= -1,
	.activate	= FB_ACTIVATE_NOW,
	.vmode		= FB_VMODE_NONINTERLACED,
};

#define PSEUDO_PALETTE_SIZE 16


#define DATA_CONTIGUOUS 1
struct nvt_dma_buf_alloc {
	/* the real alloc */
	size_t nr_pages;
	struct page **pages;

	/* the debug usage tracking */
	int nr_attached_devices;
	int nr_device_mappings;
	int nr_cpu_mappings;

	int fb_id;

	dma_addr_t contig_dma_addr;
	void *contig_cpu_addr;
};

struct nvt_dma_buf_attachment {
	struct sg_table *sg;
	bool attachment_mapped;
};

static int nvt_dma_buf_attach(struct dma_buf *buf, struct dma_buf_attachment *attachment)
{
	struct nvt_dma_buf_alloc	*alloc;
	alloc = buf->priv;

	attachment->priv = kzalloc(sizeof(struct nvt_dma_buf_attachment), GFP_KERNEL);
	if (!attachment->priv)
		return -ENOMEM;

	/* dma_buf is externally locked during call */
	alloc->nr_attached_devices++;
	return 0;
}

static void nvt_dma_buf_detach(struct dma_buf *buf, struct dma_buf_attachment *attachment)
{
	struct nvt_dma_buf_alloc *alloc = buf->priv;
	struct nvt_dma_buf_attachment *pa = attachment->priv;

	/* dma_buf is externally locked during call */
	WARN(pa->attachment_mapped, "WARNING: dma-buf-test-exporter detected detach with open device mappings");

	alloc->nr_attached_devices--;

	kfree(pa);
}

static struct sg_table *nvt_dma_buf_map(struct dma_buf_attachment *attachment, enum dma_data_direction direction)
{
	struct sg_table *sg;
	struct nvt_dma_buf_alloc *alloc;
	struct nvt_dma_buf_attachment *pa = attachment->priv;
	int ret;

	alloc = attachment->dmabuf->priv;

	if (WARN(pa->attachment_mapped,
	    "WARNING: Attempted to map already mapped attachment."))
		return ERR_PTR(-EBUSY);

	sg = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!sg)
		return ERR_PTR(-ENOMEM);

	/* from here we access the allocation object, so lock the dmabuf pointing to it */
	mutex_lock(&attachment->dmabuf->lock);

	ret = sg_alloc_table(sg, 1, GFP_KERNEL);
	if (ret) {
		mutex_unlock(&attachment->dmabuf->lock);
		kfree(sg);
		return ERR_PTR(ret);
	}

	sg_dma_len(sg->sgl) = alloc->nr_pages * PAGE_SIZE;
	sg_set_page(sg->sgl, pfn_to_page(PFN_DOWN(alloc->contig_dma_addr)), alloc->nr_pages * PAGE_SIZE, 0);
	sg_dma_address(sg->sgl) = alloc->contig_dma_addr;

	/* 
	 * Because dma_map_sg_attrs will use phys_to_virt to cache sync
	 * but virtual memory is not directly mapped to physical memory
	 * causing cache sync to crash, so skip the cache sync step
	 * and then do cache sync through our api 
	 * */
	if (!dma_map_sg_attrs(attachment->dev, sg->sgl, sg->nents, direction, DMA_ATTR_SKIP_CPU_SYNC)) {
		mutex_unlock(&attachment->dmabuf->lock);
		sg_free_table(sg);
		kfree(sg);
		return ERR_PTR(-ENOMEM);
	}
#ifdef DATA_CONTIGUOUS 
	fmem_dcache_sync(alloc->contig_cpu_addr, sg_dma_len(sg->sgl), DMA_BIDIRECTIONAL);
#else
	for_each_sg(sg->sgl, sgl_cur, sg->nents, i) {
		if (sgl_cur->dma_address == DMA_MAPPING_ERROR || !sgl_cur->dma_address)
			continue;
		fmem_dcache_sync((void *)sgl_cur->dma_address, sg_dma_len(sgl_cur), DMA_BIDIRECTIONAL);
	}
#endif

	alloc->nr_device_mappings++;
	pa->attachment_mapped = true;
	pa->sg = sg;
	mutex_unlock(&attachment->dmabuf->lock);
	return sg;
}

static void nvt_dma_buf_unmap(struct dma_buf_attachment *attachment,
							 struct sg_table *sg, enum dma_data_direction direction)
{
	struct nvt_dma_buf_alloc *alloc;
	struct nvt_dma_buf_attachment *pa = attachment->priv;

	alloc = attachment->dmabuf->priv;

	mutex_lock(&attachment->dmabuf->lock);

	WARN(!pa->attachment_mapped, "WARNING: Unmatched unmap of attachment.");

	if (!sg || !sg->sgl) {
		pr_debug("no mapping for device %s\n", dev_name(attachment->dev));
		mutex_unlock(&attachment->dmabuf->lock);
		return;
	}

#ifdef DATA_CONTIGUOUS
	fmem_dcache_sync(alloc->contig_cpu_addr, sg_dma_len(sg->sgl), DMA_BIDIRECTIONAL);
#else
	for_each_sg(sg->sgl, sgl_cur, sg->nents, i) {
		if (sgl_cur->dma_address == DMA_MAPPING_ERROR || !sgl_cur->dma_address)
			continue;
		fmem_dcache_sync((void *)sgl_cur->dma_address, sg_dma_len(sgl_cur), DMA_BIDIRECTIONAL);
	}
#endif
	alloc->nr_device_mappings--;
	pa->attachment_mapped = false;
	pa->sg = NULL;

	mutex_unlock(&attachment->dmabuf->lock);

	dma_unmap_sg_attrs(attachment->dev, sg->sgl, sg->nents, direction, DMA_ATTR_SKIP_CPU_SYNC);
	sg_free_table(sg);
	kfree(sg);
}

static void nvt_dma_buf_release(struct dma_buf *buf)
{
	struct nvt_dma_buf_alloc *alloc = buf->priv;
	in_used[alloc->fb_id] = false;
	kvfree(alloc->pages);
	kfree(alloc);

	return;
}

static int nvt_dma_buf_sync(struct dma_buf *dmabuf)
{
	struct dma_buf_attachment *attachment;
	struct nvt_dma_buf_alloc *alloc;

	alloc = dmabuf->priv;

	mutex_lock(&dmabuf->lock);

	/* List all attachments and flush all sg list */
	list_for_each_entry(attachment, &dmabuf->attachments, node) {
		struct nvt_dma_buf_attachment *pa = attachment->priv;
		struct sg_table *sg = pa->sg;

		if (!sg || !sg->sgl) {
			pr_debug("no mapping for device %s\n", dev_name(attachment->dev));
			continue;
		}

#ifdef DATA_CONTIGUOUS
		fmem_dcache_sync(alloc->contig_cpu_addr, sg_dma_len(sg->sgl), DMA_BIDIRECTIONAL);
#else
		for_each_sg(sg->sgl, sgl_cur, sg->nents, i) {
			if (sgl_cur->dma_address == DMA_MAPPING_ERROR || !sgl_cur->dma_address)
				continue;
			fmem_dcache_sync((void *)sgl_cur->dma_address, sg_dma_len(sgl_cur), DMA_BIDIRECTIONAL);
		}
#endif
	}

	mutex_unlock(&dmabuf->lock);
	return 0;
}

static int nvt_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
					enum dma_data_direction direction)
{
	return nvt_dma_buf_sync(dmabuf);
}

static int nvt_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
				enum dma_data_direction direction)
{
	return nvt_dma_buf_sync(dmabuf);
}

static void nvt_dma_buf_mmap_open(struct vm_area_struct *vma)
{
	struct dma_buf *dma_buf;
	struct nvt_dma_buf_alloc *alloc;
	dma_buf = vma->vm_private_data;
	alloc = dma_buf->priv;

	mutex_lock(&dma_buf->lock);
	alloc->nr_cpu_mappings++;
	mutex_unlock(&dma_buf->lock);
}

static void nvt_dma_buf_mmap_close(struct vm_area_struct *vma)
{
	struct dma_buf *dma_buf;
	struct nvt_dma_buf_alloc *alloc;
	dma_buf = vma->vm_private_data;
	alloc = dma_buf->priv;

	BUG_ON(alloc->nr_cpu_mappings <= 0);
	mutex_lock(&dma_buf->lock);
	alloc->nr_cpu_mappings--;
	mutex_unlock(&dma_buf->lock);
}

static vm_fault_t nvt_dma_buf_mmap_fault(struct vm_fault *vmf)
{
	struct nvt_dma_buf_alloc *alloc;
	struct dma_buf *dmabuf;
	struct page *pageptr;

	dmabuf = vmf->vma->vm_private_data;
	alloc = dmabuf->priv;

	if (vmf->pgoff > alloc->nr_pages)
		return VM_FAULT_SIGBUS;

	pageptr = alloc->pages[vmf->pgoff];

	BUG_ON(!pageptr);

	get_page(pageptr);
	vmf->page = pageptr;

	return 0;
}

struct vm_operations_struct nvt_dma_buf_vm_ops = {
	.open = nvt_dma_buf_mmap_open,
	.close = nvt_dma_buf_mmap_close,
	.fault = nvt_dma_buf_mmap_fault
};

static int nvt_dma_buf_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	int ret;
	struct nvt_dma_buf_alloc *alloc;
	unsigned long size;

	alloc = dmabuf->priv;
	size = vma->vm_end - vma->vm_start;

	if ((size == 0) || (size > (alloc->nr_pages * PAGE_SIZE))) {
		pr_err("%s: vma mmap size is invalid", __func__);
		return -EINVAL;
	}

	vma->vm_flags |= VM_IO | VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_ops = &nvt_dma_buf_vm_ops;
	vma->vm_private_data = dmabuf;

	/* call open to do the ref-counting */
	nvt_dma_buf_vm_ops.open(vma);

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start, PFN_DOWN(alloc->contig_dma_addr) + vma->vm_pgoff, size, vma->vm_page_prot);
	return ret;
}

static struct dma_buf_ops nvt_dma_buf_ops = {
	.attach = nvt_dma_buf_attach,
	.detach = nvt_dma_buf_detach,
	.map_dma_buf = nvt_dma_buf_map,
	.unmap_dma_buf = nvt_dma_buf_unmap,
	.release = nvt_dma_buf_release,
	.begin_cpu_access = nvt_dma_buf_begin_cpu_access,
	.end_cpu_access = nvt_dma_buf_end_cpu_access,
	.mmap = nvt_dma_buf_mmap,
};

struct dma_buf_export_info export_info = {
	.exp_name = "nvt_dma_buf",
	.owner = THIS_MODULE,
	.ops = &nvt_dma_buf_ops,
	.flags = O_CLOEXEC | O_RDWR,
};


struct dma_buf *nvt_dma_buf_export(void *va_addr, unsigned long start, unsigned long size, int id)
{
	struct nvt_dma_buf_alloc *alloc;
	dma_addr_t dma_aux;
	size_t i;
	struct dma_buf *nvt_dma_buf;

	if (!size) {
		pr_err("%s: no size specified", __func__);
		goto invalid_size;
	}

	if (size % PAGE_SIZE) {
		pr_err("%s: Size must be aligned to page(%ld)\n", __func__, PAGE_SIZE);
		goto invalid_size;
	}

	alloc = kzalloc(sizeof(struct nvt_dma_buf_alloc), GFP_KERNEL);
	if (alloc == NULL) {
		pr_err("%s: couldn't alloc object", __func__);
		goto invalid_size;
	}

	alloc->fb_id = id;

	alloc->nr_pages = size / PAGE_SIZE;

	alloc->pages = kvzalloc(sizeof(struct page *) * alloc->nr_pages, GFP_KERNEL);
	if (!alloc->pages) {
		pr_err("%s: couldn't alloc %zu page structures",__func__, alloc->nr_pages);
		goto free_alloc_object;
	}

	alloc->contig_cpu_addr = va_addr;
	alloc->contig_dma_addr = (dma_addr_t)start;
	if (!alloc->contig_cpu_addr) {
		pr_err("%s: couldn't alloc contiguous buffer %zu pages",
				__func__, alloc->nr_pages);
		goto free_page_struct;
	}
	dma_aux = alloc->contig_dma_addr;
	for (i = 0; i < alloc->nr_pages; i++) {
		alloc->pages[i] = pfn_to_page(PFN_DOWN(dma_aux));
		dma_aux += PAGE_SIZE;
	}

	export_info.size = alloc->nr_pages << PAGE_SHIFT;
	export_info.priv = alloc;

	nvt_dma_buf = dma_buf_export(&export_info);

	if (IS_ERR_OR_NULL(nvt_dma_buf)) {
		pr_err("%s: couldn't export dma_buf", __func__);
		goto free_page_struct;
	}

	return nvt_dma_buf;

free_page_struct:
	kvfree(alloc->pages);
free_alloc_object:
	kfree(alloc);
invalid_size:
	return NULL;
}

void nvt_dma_buf_unexport(struct dma_buf *nvt_dma_buf)
{
	struct nvt_dma_buf_alloc *alloc = export_info.priv;
	dma_buf_put(nvt_dma_buf);
	kvfree(alloc->pages);
	kfree(alloc);
}

static int nvtsimfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			      u_int transp, struct fb_info *info)
{
	u32 *pal = info->pseudo_palette;
	u32 cr = red >> (16 - info->var.red.length);
	u32 cg = green >> (16 - info->var.green.length);
	u32 cb = blue >> (16 - info->var.blue.length);
	u32 value;

	if (regno >= PSEUDO_PALETTE_SIZE)
		return -EINVAL;

	value = (cr << info->var.red.offset) |
		(cg << info->var.green.offset) |
		(cb << info->var.blue.offset);
	if (info->var.transp.length > 0) {
		u32 mask = (1 << info->var.transp.length) - 1;
		mask <<= info->var.transp.offset;
		value |= mask;
	}
	pal[regno] = value;

	return 0;
}

struct nvtsimfb_par;
static void nvtsimfb_clocks_destroy(struct nvtsimfb_par *par);
static void nvtsimfb_regulators_destroy(struct nvtsimfb_par *par);

/*
 * fb_ops.fb_destroy is called by the last put_fb_info() call at the end
 * of unregister_framebuffer() or fb_release(). Do any cleanup here.
 */
static void nvtsimfb_destroy(struct fb_info *info)
{
	nvtsimfb_regulators_destroy(info->par);
	nvtsimfb_clocks_destroy(info->par);
	if (info->screen_base)
		iounmap(info->screen_base);

	framebuffer_release(info);
}

static struct dma_buf *nvt_fb_get_dma_buf(struct fb_info *info)
{
	struct dma_buf *nvt_dma_buf;

	mutex_lock(&nvtsimfb_lock);

	if (!in_used[info->node]) {
		nvt_dma_buf = nvt_dma_buf_export((void *)info->screen_base, info->fix.smem_start, info->fix.smem_len, info->node);
		if (IS_ERR_OR_NULL(nvt_dma_buf)) {
			pr_err("%s couldn't export dma_buf\n", __func__);
			mutex_unlock(&nvtsimfb_lock);
			return NULL;
		}
		in_used[info->node] = true;
	} else {
		pr_err("%s: dma_buf is busy\n", __func__);
		mutex_unlock(&nvtsimfb_lock);
		return NULL;
	}

	mutex_unlock(&nvtsimfb_lock);

	return nvt_dma_buf;
}

int nvt_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	/* TBD: link to HW config assign pa to HW */
	return 0;
}

static const struct fb_ops nvtsimfb_ops = {
	.owner		= THIS_MODULE,
	.fb_destroy	= nvtsimfb_destroy,
	.fb_setcolreg	= nvtsimfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_dmabuf_export = nvt_fb_get_dma_buf,
	.fb_pan_display = nvt_fb_pan_display,
};

static struct nvtsimfb_format nvtsimfb_formats[] = SIMPLE_FORMATS;

struct nvtsimfb_params {
	u32 width;
	u32 height;
	u32 stride;
	struct nvtsimfb_format *format;
};

static int nvtsimfb_parse_dt(struct platform_device *pdev,
			   struct nvtsimfb_params *params)
{
	struct device_node *np = pdev->dev.of_node;
	int ret;
	const char *format;
	int i;

	ret = of_property_read_u32(np, "width", &params->width);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse width property\n");
		return ret;
	}

	ret = of_property_read_u32(np, "height", &params->height);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse height property\n");
		return ret;
	}

	ret = of_property_read_u32(np, "stride", &params->stride);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse stride property\n");
		return ret;
	}

	ret = of_property_read_string(np, "format", &format);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse format property\n");
		return ret;
	}
	params->format = NULL;
	for (i = 0; i < ARRAY_SIZE(nvtsimfb_formats); i++) {
		if (strcmp(format, nvtsimfb_formats[i].name))
			continue;
		params->format = &nvtsimfb_formats[i];
		break;
	}
	if (!params->format) {
		dev_err(&pdev->dev, "Invalid format value\n");
		return -EINVAL;
	}

	return 0;
}

struct nvtsimfb_par {
	u32 palette[PSEUDO_PALETTE_SIZE];
#if defined CONFIG_OF && defined CONFIG_COMMON_CLK
	bool clks_enabled;
	unsigned int clk_count;
	struct clk **clks;
#endif
#if defined CONFIG_OF && defined CONFIG_REGULATOR
	bool regulators_enabled;
	u32 regulator_count;
	struct regulator **regulators;
#endif
};

#if defined CONFIG_OF && defined CONFIG_COMMON_CLK
/*
 * Clock handling code.
 *
 * Here we handle the clocks property of our "nvt_simfb" dt node.
 * This is necessary so that we can make sure that any clocks needed by
 * the display engine that the bootloader set up for us (and for which it
 * provided a nvt_simfb dt node), stay up, for the life of the simplefb
 * driver.
 *
 * When the driver unloads, we cleanly disable, and then release the clocks.
 *
 * We only complain about errors here, no action is taken as the most likely
 * error can only happen due to a mismatch between the bootloader which set
 * up simplefb, and the clock definitions in the device tree. Chances are
 * that there are no adverse effects, and if there are, a clean teardown of
 * the fb probe will not help us much either. So just complain and carry on,
 * and hope that the user actually gets a working fb at the end of things.
 */
static int nvtsimfb_clocks_get(struct nvtsimfb_par *par,
			       struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct clk *clock;
	int i;

	if (dev_get_platdata(&pdev->dev) || !np)
		return 0;

	par->clk_count = of_clk_get_parent_count(np);
	if (!par->clk_count)
		return 0;

	par->clks = kcalloc(par->clk_count, sizeof(struct clk *), GFP_KERNEL);
	if (!par->clks)
		return -ENOMEM;

	for (i = 0; i < par->clk_count; i++) {
		clock = of_clk_get(np, i);
		if (IS_ERR(clock)) {
			if (PTR_ERR(clock) == -EPROBE_DEFER) {
				while (--i >= 0) {
					if (par->clks[i])
						clk_put(par->clks[i]);
				}
				kfree(par->clks);
				return -EPROBE_DEFER;
			}
			dev_err(&pdev->dev, "%s: clock %d not found: %ld\n",
				__func__, i, PTR_ERR(clock));
			continue;
		}
		par->clks[i] = clock;
	}

	return 0;
}

static void nvtsimfb_clocks_enable(struct nvtsimfb_par *par,
				   struct platform_device *pdev)
{
	int i, ret;

	for (i = 0; i < par->clk_count; i++) {
		if (par->clks[i]) {
			ret = clk_prepare_enable(par->clks[i]);
			if (ret) {
				dev_err(&pdev->dev,
					"%s: failed to enable clock %d: %d\n",
					__func__, i, ret);
				clk_put(par->clks[i]);
				par->clks[i] = NULL;
			}
		}
	}
	par->clks_enabled = true;
}

static void nvtsimfb_clocks_destroy(struct nvtsimfb_par *par)
{
	int i;

	if (!par->clks)
		return;

	for (i = 0; i < par->clk_count; i++) {
		if (par->clks[i]) {
			if (par->clks_enabled)
				clk_disable_unprepare(par->clks[i]);
			clk_put(par->clks[i]);
		}
	}

	kfree(par->clks);
}
#else
static int nvtsimfb_clocks_get(struct nvtsimfb_par *par,
	struct platform_device *pdev) { return 0; }
static void nvtsimfb_clocks_enable(struct nvtsimfb_par *par,
	struct platform_device *pdev) { }
static void nvtsimfb_clocks_destroy(struct nvtsimfb_par *par) { }
#endif

#if defined CONFIG_OF && defined CONFIG_REGULATOR

#define SUPPLY_SUFFIX "-supply"

/*
 * Regulator handling code.
 *
 * Here we handle the num-supplies and vin*-supply properties of our
 * "nvt_simfb" dt node. This is necessary so that we can make sure
 * that any regulators needed by the display hardware that the bootloader
 * set up for us (and for which it provided a nvt_simfb dt node), stay up,
 * for the life of the nvt_simfb driver.
 *
 * When the driver unloads, we cleanly disable, and then release the
 * regulators.
 *
 * We only complain about errors here, no action is taken as the most likely
 * error can only happen due to a mismatch between the bootloader which set
 * up simplefb, and the regulator definitions in the device tree. Chances are
 * that there are no adverse effects, and if there are, a clean teardown of
 * the fb probe will not help us much either. So just complain and carry on,
 * and hope that the user actually gets a working fb at the end of things.
 */
static int nvtsimfb_regulators_get(struct nvtsimfb_par *par,
				   struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct property *prop;
	struct regulator *regulator;
	const char *p;
	int count = 0, i = 0;

	if (dev_get_platdata(&pdev->dev) || !np)
		return 0;

	/* Count the number of regulator supplies */
	for_each_property_of_node(np, prop) {
		p = strstr(prop->name, SUPPLY_SUFFIX);
		if (p && p != prop->name)
			count++;
	}

	if (!count)
		return 0;

	par->regulators = devm_kcalloc(&pdev->dev, count,
				       sizeof(struct regulator *), GFP_KERNEL);
	if (!par->regulators)
		return -ENOMEM;

	/* Get all the regulators */
	for_each_property_of_node(np, prop) {
		char name[32]; /* 32 is max size of property name */

		p = strstr(prop->name, SUPPLY_SUFFIX);
		if (!p || p == prop->name)
			continue;

		strlcpy(name, prop->name,
			strlen(prop->name) - strlen(SUPPLY_SUFFIX) + 1);
		regulator = devm_regulator_get_optional(&pdev->dev, name);
		if (IS_ERR(regulator)) {
			if (PTR_ERR(regulator) == -EPROBE_DEFER)
				return -EPROBE_DEFER;
			dev_err(&pdev->dev, "regulator %s not found: %ld\n",
				name, PTR_ERR(regulator));
			continue;
		}
		par->regulators[i++] = regulator;
	}
	par->regulator_count = i;

	return 0;
}

static void nvtsimfb_regulators_enable(struct nvtsimfb_par *par,
				       struct platform_device *pdev)
{
	int i, ret;

	/* Enable all the regulators */
	for (i = 0; i < par->regulator_count; i++) {
		ret = regulator_enable(par->regulators[i]);
		if (ret) {
			dev_err(&pdev->dev,
				"failed to enable regulator %d: %d\n",
				i, ret);
			devm_regulator_put(par->regulators[i]);
			par->regulators[i] = NULL;
		}
	}
	par->regulators_enabled = true;
}

static void nvtsimfb_regulators_destroy(struct nvtsimfb_par *par)
{
	int i;

	if (!par->regulators || !par->regulators_enabled)
		return;

	for (i = 0; i < par->regulator_count; i++)
		if (par->regulators[i])
			regulator_disable(par->regulators[i]);
}
#else
static int nvtsimfb_regulators_get(struct nvtsimfb_par *par,
	struct platform_device *pdev) { return 0; }
static void nvtsimfb_regulators_enable(struct nvtsimfb_par *par,
	struct platform_device *pdev) { }
static void nvtsimfb_regulators_destroy(struct nvtsimfb_par *par) { }
#endif

static int nvtsimfb_probe(struct platform_device *pdev)
{
	int ret;
	struct nvtsimfb_params params;
	struct fb_info *info;
	struct nvtsimfb_par *par;
	struct resource *mem;

	/*
	 * Generic drivers must not be registered if a framebuffer exists.
	 * If a native driver was probed, the display hardware was already
	 * taken and attempting to use the system framebuffer is dangerous.
	 */
	ret = nvtsimfb_parse_dt(pdev, &params);
	if (ret) {
		dev_err(&pdev->dev, "parsing device tree failed\n");
		return ret;
	}

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		dev_err(&pdev->dev, "No memory resource\n");
		return -EINVAL;
	}

	info = framebuffer_alloc(sizeof(struct nvtsimfb_par), &pdev->dev);
	if (!info) {
		dev_err(&pdev->dev, "alloc nvtsimfb_par failed\n");
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, info);

	par = info->par;

	info->fix = nvtsimfb_fix;
	info->fix.smem_start = mem->start;
	info->fix.smem_len = resource_size(mem);
	info->fix.line_length = params.stride;
	info->fix.ypanstep = 1;

	info->var = nvtsimfb_var;
	info->var.xres = params.width;
	info->var.yres = params.height;
	info->var.xres_virtual = params.width;
	info->var.yres_virtual = params.height * 2;
	info->var.bits_per_pixel = params.format->bits_per_pixel;
	info->var.red = params.format->red;
	info->var.green = params.format->green;
	info->var.blue = params.format->blue;
	info->var.transp = params.format->transp;

	info->apertures = alloc_apertures(1);
	if (!info->apertures) {
		dev_err(&pdev->dev, "alloc apertures failed\n");
		ret = -ENOMEM;
		goto error_fb_release;
	}
	info->apertures->ranges[0].base = info->fix.smem_start;
	info->apertures->ranges[0].size = info->fix.smem_len;

	info->fbops = &nvtsimfb_ops;
	info->flags = FBINFO_DEFAULT | FBINFO_MISC_FIRMWARE;
	info->screen_base = ioremap_wc(info->fix.smem_start, info->fix.smem_len);
	if (!info->screen_base) {
		dev_err(&pdev->dev, "ioreamp failed\n");
		ret = -ENOMEM;
		goto error_fb_release;
	}

	info->pseudo_palette = par->palette;

	ret = nvtsimfb_clocks_get(par, pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "get clock failed\n");
		goto error_unmap;
	}

	ret = nvtsimfb_regulators_get(par, pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "get regulators failed\n");
		goto error_clocks;
	}

	nvtsimfb_clocks_enable(par, pdev);
	nvtsimfb_regulators_enable(par, pdev);

	dev_info(&pdev->dev, "framebuffer at 0x%lx, 0x%x bytes\n",
			     info->fix.smem_start, info->fix.smem_len);
	dev_info(&pdev->dev, "format=%s, mode=%dx%dx%d, linelength=%d\n",
			     params.format->name,
			     info->var.xres, info->var.yres,
			     info->var.bits_per_pixel, info->fix.line_length);

	ret = register_framebuffer(info);
	if (ret < 0) {
		dev_err(&pdev->dev, "Unable to register nvt_simfb: %d\n", ret);
		goto error_regulators;
	}

	dev_info(&pdev->dev, "fb%d: nvt_simfb registered!\n", info->node);

	return 0;


error_regulators:
	nvtsimfb_regulators_destroy(par);
error_clocks:
	nvtsimfb_clocks_destroy(par);
error_unmap:
	iounmap(info->screen_base);
error_fb_release:
	framebuffer_release(info);
	return ret;
}

static int nvtsimfb_remove(struct platform_device *pdev)
{
	struct fb_info *info = platform_get_drvdata(pdev);

	/* nvtsimfb_destroy takes care of info cleanup */
	unregister_framebuffer(info);
	nvtsimfb_regulators_destroy(info->par);
	nvtsimfb_clocks_destroy(info->par);
	nvt_dma_buf_unexport(nvt_dma_buf);
	iounmap(info->screen_base);
	framebuffer_release(info);

	return 0;
}

static const struct of_device_id nvtsimfb_of_match[] = {
	{ .compatible = "nvt,nvt_simfb", },
	{ },
};
MODULE_DEVICE_TABLE(of, nvtsimfb_of_match);

static struct platform_driver nvtsimfb_driver = {
	.driver = {
		.name = "nvt_simfb",
		.of_match_table = nvtsimfb_of_match,
	},
	.probe = nvtsimfb_probe,
	.remove = nvtsimfb_remove,
};

static int __init nvtsimfb_init(void)
{
	int ret;

	ret = platform_driver_register(&nvtsimfb_driver);
	if (ret)
		return ret;

	return 0;
}

fs_initcall(nvtsimfb_init);

MODULE_LICENSE("GPL");
MODULE_VERSION(NVT_SIMFB_VERSION);
