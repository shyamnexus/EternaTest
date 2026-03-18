#include <linux/uaccess.h>
#include <linux/fb.h>
#include "kdrv_videoout/kdrv_vdoout.h"
#include "nvt_fb_na51102.h"
#include "nvt_dbg.h"
#include "display.h"
#include "dispcomn.h"

#define IOCTL_VIDEOOUT_ITEM_FB_CURSOR 0x55

unsigned char driver_name_osd[] = "nvt_fb_osd";
struct fb_ops nvt_fb_osd_ops;

static int nvt_fb_osd_fill_para(struct fb_info *fbinfo);

static unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	DBG_IND("\n");
	return ((chan & 0xffff) >> (16 - bf->length)) << bf->offset;
}

int nvt_fb_osd_probe(struct fb_info *fbinfo, unsigned int ide_id, unsigned int fb_layer)
{
	int ret = 0;
	uintptr_t addr_phy;
	struct nvt_fb_par *fb_par = (struct nvt_fb_par *) fbinfo->par;

	DBG_IND("+ 0x%lx\n", (unsigned long)fbinfo);

	ret = nvt_fb_osd_fill_para(fbinfo);

	if (ret) {
		return ret;
	}
	
	if(fb_par->pandisplay_type == NVT_PANDISPLAY_TYPE_LINUX){
		addr_phy = fbinfo->fix.smem_start;
	}else if(fb_par->pandisplay_type == NVT_PANDISPLAY_TYPE_RLE){
		addr_phy = (fb_par->free_pingpong_idx ? fb_par->pandisplay_buffer[0] : fb_par->pandisplay_buffer[1]);
	}else if(fb_par->pandisplay_type == NVT_PANDISPLAY_TYPE_COPY){
		if(fb_par->pandisplay_buffer_cnt == 0){
			addr_phy = fbinfo->fix.smem_start;
		}else if(fb_par->pandisplay_buffer_cnt == 1){
			addr_phy = fb_par->pandisplay_buffer[0];
		}else if(fb_par->pandisplay_buffer_cnt == 2){
			addr_phy = (fb_par->free_pingpong_idx ? fb_par->pandisplay_buffer[0] : fb_par->pandisplay_buffer[1]);
		}else{
			printk(KERN_ERR "invalid copy value(%d) in framebuffer device tree\n", fb_par->pandisplay_buffer_cnt);
			return -1;
		}
	}else{
		printk(KERN_ERR "invalid framebuffer pandisplay type(%d)\n", fb_par->pandisplay_type);
		return -1;
	}

	ret = display_obj_set_osd_addr(ide_id, addr_phy, 1);

	return ret;
}

static int nvt_fb_osd_fill_para(struct fb_info *fbinfo)
{
	int ret = 0;

	fbinfo->fix.type      = FB_TYPE_PACKED_PIXELS;
	fbinfo->fix.type_aux  = 0;
	fbinfo->fix.xpanstep  = 1;
	fbinfo->fix.ypanstep  = 1;
	fbinfo->fix.ywrapstep = 0;
	fbinfo->fix.accel     = FB_ACCEL_NONE;
	//fbinfo->fix.visual    = FB_VISUAL_PSEUDOCOLOR;
	fbinfo->fix.visual    = FB_VISUAL_TRUECOLOR;
	strcpy(fbinfo->fix.id, driver_name_osd);

	display_obj_get_osd_param(DISP_1, &fbinfo->var, &fbinfo->fix);

	fbinfo->fbops = &nvt_fb_osd_ops;
	fbinfo->flags = FBINFO_FLAG_DEFAULT;

	fbinfo->var.nonstd      = 0;
	fbinfo->var.activate    = FB_ACTIVATE_NOW;
	fbinfo->var.accel_flags = 0;
	fbinfo->var.vmode       = FB_VMODE_NONINTERLACED;

	((struct nvt_fb_par *)fbinfo->par)->palette_sz = 255;

	return ret;
}

int nvt_fb_osd_check_var(struct fb_var_screeninfo *var, struct fb_info *fbinfo)
{
	struct fb_fix_screeninfo *fix = &fbinfo->fix;

 	if ((var->yres_virtual * fix->line_length) > fix->smem_len) {
		nvt_dbg(ERR, "invalid yres_virtual:%d\n", var->yres_virtual);
		return -EINVAL;
	}

 	if ((var->xres > var->xres_virtual) || (var->yres > var->yres_virtual)) {
		nvt_dbg(ERR, "invalid parameter xres:%d yres:%d xres_virt:%d yres_virt:%d\n", var->xres, var->yres,  var->xres_virtual, var->yres_virtual);
		return -EINVAL;
	}

	return 0;
}

static void nvt_fb_osd_activate_var(struct fb_info *info)
{
/*	struct nvt_fb_par *fb_par = (struct nvt_fb_par *) info->par;
	struct fb_var_screeninfo *var = &info->var; */

    /*display_obj_set_osd_param(NVT_FB_GET_IDE_ID(fb_par->fb_id), var, NULL);*/
	/* set video address to IDE driver */
	//	display_obj_set_osd_addr(NVT_FB_GET_IDE_ID(fb_par->fb_id), info->fix.smem_start + var->yoffset, 1);
	nvt_fb_pan_display(&info->var, info);

}

int nvt_fb_osd_set_par(struct fb_info *info)
{
	nvt_fb_osd_activate_var(info);
	return 0;
}

int nvt_fb_osd_blank(int blank_mode, struct fb_info *info)
{
	struct nvt_fb_par *fb_par = (struct nvt_fb_par *) info->par;
	int blank;
	int ret = 0;

	DBG_IND("block_mode:%d\n", blank_mode);

	switch (blank_mode) {
	case FB_BLANK_UNBLANK: /* Unblanking */
		blank = 0;
		info->flags &= (~(int)(NVT_FB_BLANK));
		break;

	case FB_BLANK_NORMAL: /* Normal blanking */
	case FB_BLANK_VSYNC_SUSPEND: /* VESA blank (vsync off) */
	case FB_BLANK_HSYNC_SUSPEND: /* VESA blank (hsync off) */
	case FB_BLANK_POWERDOWN: /* Poweroff */
		blank = 1;
		info->flags |= (int)NVT_FB_BLANK;
		break;

	default:
		ret = -EINVAL;
	}

	if (ret == 0) {
		display_obj_set_osd_blank(NVT_FB_GET_IDE_ID(fb_par->fb_id), blank);
	}

	return ret;
}

static int nvt_fb_osd_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val;

	DBG_IND("setcol: regno=%d, rgb=%d,%d,%d tans=%d\n",
		   regno, red, green, blue, transp);

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseudo-palette */

		if (regno < 256) {
			u32 *pal = info->pseudo_palette;

			val  = chan_to_field(red,   &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue,  &info->var.blue);
			val |= chan_to_field(transp,  &info->var.transp);

			pal[regno] = val;
		}
		break;

	case FB_VISUAL_PSEUDOCOLOR:
		if (regno < 256) {
			val  = (red & 0xFF00) >> 8;
			val |= (green & 0xFF00) ;
			val |= (blue & 0xFF00) << 8;
			val |= (transp & 0xFF00) << 16;

			/*Call to IDE driver to fill palette register */
			if (display_obj_set_osd_palette(DISP_1, regno, val)) {
				return -EINVAL;
			}

		} else {
			DBG_ERR("Pal range err:%d", regno);
			return -EINVAL;	/* unknown type */
		}

		break;

	default:
		return -EINVAL;	/* unknown type */
	}

	return 0;
}

static u32 Convert_RGB2YCbCr(u8 r, u8 g, u8 b)
{
    /* TBD */
    int tcolor;
    u32 YCbCr;
    ///Convert RGB to Y
    tcolor = (257 * r + 504 * g + 98 * b) / 1000 + 16;
    YCbCr = (tcolor & 0xff) << 16;
    ///Convert RGB to Cb
    tcolor = (int)(439 * b - 291 * g - 148 * r) / 1000 + 128;
    YCbCr |= (tcolor & 0xff) << 8;
    ///Convert RGB to Cr
    tcolor = (int)(439 * r - 368 * g - 71 * b) / 1000 + 128;
    YCbCr |= (tcolor & 0xff);
    return YCbCr;
}

static int dev_cursor(unsigned int ide_id, struct fb_cursor *cursor)
{
    int ret = 0;
	UINT32 handler;
	KDRV_VDDO_DISPLAYER_PARAM kdrv_disp_layer = { 0 };
    int i, set;
    unsigned long retVal;
	KDRV_DEV_ENGINE engine;

	if(!cursor){
		printk(KERN_ERR "fb_cursor is NULL\n");
		return -1;
	}
	set = cursor->set;

	if(ide_id == 0){
		engine = KDRV_VDOOUT_ENGINE0;
	} else if(ide_id == 1){
		engine = KDRV_VDOOUT_ENGINE1;
	}else{
		printk(KERN_ERR "unknown ide_id(%x)\n", ide_id);
		return -1;
	}

	handler = KDRV_DEV_ID(0, engine, 0);
	ret=kdrv_vddo_open(0, engine);
	if(ret) {
		DBG_ERR("fail to open vddo\n");
		return -1;
	}

    if (set & FB_CUR_SETPOS) {
		kdrv_disp_layer.SEL.KDRV_VDDO_HWC_CONFIG.hwc_en=1;
		kdrv_disp_layer.SEL.KDRV_VDDO_HWC_CONFIG.hwc_x=cursor->image.dx;
		kdrv_disp_layer.SEL.KDRV_VDDO_HWC_CONFIG.hwc_y=cursor->image.dy;
		ret=kdrv_vddo_set(handler, VDDO_DISPLAYER_HWC_CONFIG, &kdrv_disp_layer);
		if(ret) {
			DBG_ERR("fail to set fb cursor position\n");
			return -1;
		}
    }

    if (set & FB_CUR_SETCMAP) {
        struct fb_cmap cmap = cursor->image.cmap;
        u32 tmp_color;
        int cmap_len;
        __u16 cmapr[16+1]={0};
        __u16 cmapg[16+1]={0};
        __u16 cmapb[16+1]={0};
        __u16 cmapt[16+1]={0};

        cmap_len = (cursor->image.cmap.len > 16) ? 16 : cursor->image.cmap.len;
        retVal = copy_from_user(cmapr, cursor->image.cmap.red, cmap_len * sizeof(__u16));
        cmap.red = cmapr;
        retVal = copy_from_user(cmapg, cursor->image.cmap.green, cmap_len * sizeof(__u16));
        cmap.green = cmapg;
        retVal = copy_from_user(cmapb, cursor->image.cmap.blue, cmap_len * sizeof(__u16));
        cmap.blue = cmapb;
        retVal = copy_from_user(cmapt, cursor->image.cmap.transp, cmap_len * sizeof(__u16));
        cmap.transp = cmapt;

        for (i = 1; i < cmap_len; i++) {
            tmp_color = Convert_RGB2YCbCr(cmap.red[i], cmap.green[i], cmap.blue[i]);
			kdrv_disp_layer.SEL.KDRV_VDDO_HWCPAL.pal_idx = (DISPHWC_NUM1+i-1); // 1~15
			kdrv_disp_layer.SEL.KDRV_VDDO_HWCPAL.color_y = ((tmp_color & 0x0ff0000)>>16);
			kdrv_disp_layer.SEL.KDRV_VDDO_HWCPAL.color_cb = ((tmp_color & 0x0ff00)>>8);
			kdrv_disp_layer.SEL.KDRV_VDDO_HWCPAL.color_cr = (tmp_color & 0x0ff);
			ret=kdrv_vddo_set(handler, VDDO_DISPLAYER_HWC_PAL, &kdrv_disp_layer);
			if(ret) {
				DBG_ERR("fail to set fb cursor palette[%d]\n", i);
				return -1;
			}
        }
    }

    if (set & (FB_CUR_SETSHAPE | FB_CUR_SETIMAGE)) {
        /* how many bytes per line(width) for the input image */
        u32 s_pitch = (cursor->image.width * cursor->image.depth + 7) >> 3, value;
        int size = (cursor->image.width * cursor->image.height * cursor->image.depth) >> 3;
        int cp_size = s_pitch * cursor->image.height;
        u8 *mask, *rawdata;

        mask = kmalloc(cp_size, GFP_KERNEL);
        rawdata = kmalloc(cp_size, GFP_KERNEL);
        if (!mask || !rawdata) {
            DBG_ERR("no memory for cursor mask, size: %d \n", cp_size);
            if (mask)
                kfree(mask);
            if (rawdata)
                kfree(rawdata);
            return -1;
        }

        retVal = copy_from_user(mask, cursor->mask, cp_size);
        retVal = copy_from_user(rawdata, cursor->image.data, cp_size);
        size = size >> 2;   /* bytes to word */

        for (i = 0; i < size; i ++) {
#define SWAP8(x)	((((x) >> 4) & 0xF) | (((x) & 0xF) << 4))
            value = SWAP8(rawdata[4*i] & mask[4*i]) | SWAP8(rawdata[4*i+1] & mask[4*i+1]) << 8 |
                    SWAP8(rawdata[4*i+2] & mask[4*i+2]) << 16 | SWAP8(rawdata[4*i+3] & mask[4*i+3]) << 24;

			kdrv_disp_layer.SEL.KDRV_VDDO_HWCFONT.font_entry = i; //index 0, 0~511
			kdrv_disp_layer.SEL.KDRV_VDDO_HWCFONT.font_data = value;
			ret=kdrv_vddo_set(handler, VDDO_DISPLAYER_HWC_FONT, &kdrv_disp_layer);
			if(ret) {
				DBG_ERR("fail to set fb cursor pixel[%d]\n", i);
				kfree(mask);
				kfree(rawdata);
				return -1;
			}
        }

        kfree(mask);
        kfree(rawdata);
    } /* set & (FB_CUR_SETSHAPE | FB_CUR_SETIMAGE */
	
	ret=kdrv_vddo_trigger(handler, NULL); 
	if(ret) {
		DBG_ERR("fail to trigger fb cursor update\n");
		return -1;
	}

    return 0;
}

int nvt_fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	struct nvt_fb_par *fb_par = NULL;
	DISP_ID ide_id;
	struct fb_cursor fbc = { 0 };

	if(cmd != IOCTL_VIDEOOUT_ITEM_FB_CURSOR){
		printk(KERN_ERR "fb cmd(%x) is not supported\n", cmd);
		return -1;
	}

	fb_par = (struct nvt_fb_par *) info->par;
	if(!fb_par){
		printk(KERN_ERR "fb_par is NULL\n");
		return -1;
	}
	ide_id = NVT_FB_GET_IDE_ID(fb_par->fb_id);

	//coverity[tainted_argument]
	if (copy_from_user(&fbc, (unsigned int *)arg, sizeof(struct fb_cursor))) {
		printk(KERN_ERR "copy_from_user  err cmd:0x%x\n", cmd);
		return -1;
	}

	//coverity[tainted_data]
	return dev_cursor(ide_id, &fbc);
}

struct fb_ops nvt_fb_osd_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= nvt_fb_osd_check_var,
	.fb_set_par	= nvt_fb_osd_set_par,
	.fb_setcolreg	= nvt_fb_osd_setcolreg,
	.fb_blank	= nvt_fb_osd_blank,
#ifdef CONFIG_FB_CFB_FILLRECT
	.fb_fillrect	= cfb_fillrect, /* Use native function */
#endif
#ifdef CONFIG_FB_CFB_COPYAREA
	.fb_copyarea	= cfb_copyarea, /* Use native function */
#endif
#ifdef CONFIG_FB_CFB_IMAGEBLIT
	.fb_imageblit	= cfb_imageblit, /* Use native function */
#endif
	.fb_pan_display	= nvt_fb_pan_display,
	.fb_ioctl       = nvt_fb_ioctl
};
