#if defined(__KERNEL__)
#include <linux/of.h>
#include <linux/clk.h>
#include "plat/pad.h"
#include "kwrap/debug.h"
#include "audcap_builtin.h"
#include "audcap_builtin_platform.h"
#include <libfdt.h>
#include <fdtfast.h>

void _audcap_builtin_set_clk(int sampling_rate)
{
	struct clk *pll7_clk;

	switch (sampling_rate) {
		case 11025:
		case 22050:
		case 44100: {
            pll7_clk = clk_get(NULL, "pll7");
            if (IS_ERR(pll7_clk)) {
				DBG_ERR("get pll7 failed\r\n");
			}
            clk_set_rate(pll7_clk, 338688000);
		} break;

		case 8000:
		case 12000:
		case 16000:
		case 24000:
		case 32000:
		case 48000:
		default: {
			pll7_clk = clk_get(NULL, "pll7");
            if (IS_ERR(pll7_clk)) {
				DBG_ERR("get pll7 failed\r\n");
			}
            clk_set_rate(pll7_clk, 344064000);
		} break;
	}

		clk_put(pll7_clk);
}

void _audcap_builtin_set_pad(void)
{
#if defined (_BSP_NA51102_) || defined (_BSP_NS02201_)
	// do not set pad power
#else
	PAD_POWER_STRUCT    pad_power;

	pad_power.pad_power_id     = PAD_POWERID_ADC;
	pad_power.pad_power		   = PAD_1P8V;
	pad_power.bias_current	   = FALSE;
	pad_power.opa_gain		   = FALSE;
	pad_power.pull_down		   = FALSE;
	pad_power.enable		   = ENABLE;
	pad_power.pad_vad		   = PAD_VAD_3P0V;
	pad_set_power(&pad_power);
#endif
}

int audcap_builtin_get_nodeoffset(int *nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}

	*nodeoffset = fdt_path_offset(p_fdt, "/fastboot/acap");
	return 0;
}


int audcap_builtin_get_samplerate(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	int aud_sr = -1;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "samplerate", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "samplerate");
		return -1;
	}
	aud_sr = be32_to_cpu(*(unsigned int *)nodep);

	return aud_sr;
}

int audcap_builtin_get_channel(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	int aud_ch = -1;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "channel", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "channel");
		return -1;
	}
	aud_ch = be32_to_cpu(*(unsigned int *)nodep);

	return aud_ch;
}

int audcap_builtin_get_bufcount(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	int aud_que_cnt = -1;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "bufnum", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "bufnum");
		return -1;
	}
	aud_que_cnt = be32_to_cpu(*(unsigned int *)nodep);

	return aud_que_cnt;
}

int audcap_builtin_get_bufsamplecnt(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	int aud_buf_sample_cnt = -1;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "bufsamplecnt", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "bufsamplecnt");
		return -1;
	}
	aud_buf_sample_cnt = be32_to_cpu(*(unsigned int *)nodep);

	return aud_buf_sample_cnt;
}

int audcap_builtin_get_rec_src(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	int rec_src = -1;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "recsrc", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "recsrc");
		return -1;
	}
	rec_src = be32_to_cpu(*(unsigned int *)nodep);

	return rec_src;
}

int audcap_builtin_get_vol(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	int aud_vol = -1;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "vol", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "vol");
		return -1;
	}
	aud_vol = be32_to_cpu(*(unsigned int *)nodep);

	return aud_vol;
}

UINT32 audcap_builtin_get_aec_en(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	int aec_en = 0;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "aec", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "aec");
		return -1;
	}
	aec_en = be32_to_cpu(*(unsigned int *)nodep);

	return aec_en;
}

UINT32 audcap_builtin_get_txchannel(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	UINT32 txchannel = 0;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "txchannel", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "txchannel");
		return -1;
	}
	txchannel = be32_to_cpu(*(unsigned int *)nodep);

	return txchannel;
}

UINT32 audcap_builtin_get_default_setting(int nodeoffset)
{
	unsigned char *p_fdt = (unsigned char *)fdtfast_get_base();
	const void *nodep;  /* property node pointer */
	int len;
	int default_setting = 0;

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return -1;
	}
	nodep = fdt_getprop(p_fdt, nodeoffset, "defaultsetting", &len);
	if (len == 0 || nodep == NULL) {
		DBG_ERR("Failed to read %s\r\n", "defaultsetting");
		return -1;
	}
	default_setting = be32_to_cpu(*(unsigned int *)nodep);

	return default_setting;
}
#endif
