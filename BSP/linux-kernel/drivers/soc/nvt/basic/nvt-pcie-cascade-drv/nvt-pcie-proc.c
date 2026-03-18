#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/bitfield.h>
#include <linux/delay.h>

#include <plat/hardware.h>

#include "nvt_pcie_int.h"
#include "nvt_pcie_tba.h"

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

#define EC_G0_NUM     7
#define EC_G1_NUM     11
#define EC_G2_NUM     8
#define EC_G3_NUM     6

//============================================================================
// Declaration
//============================================================================
#if 0
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PMODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;
#endif

//============================================================================
// Global variable
//============================================================================
static struct nvt_pcie_chip *pdrv_info_data;
//static PXXX_DRV_INFO pdrv_info_data;

//============================================================================
// Function define
//============================================================================


//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
//UINT32  g_user_ec_sel;
PCIE_TBA_SEL  g_user_sel;
UINT32 uncorr_error_cnt=0;
UINT32 rx_error_cnt=0;
UINT32 run=0;
struct nvt_pcie_chip *glb_nvt_pcie_chip_ptr = NULL;
//for debug messages
static char g0_str[EC_G0_NUM][MAX_CMD_LENGTH]= {"EBUF Overflow","EBUF Under-run","Decode Error","Running Disparity Error","SKP OS Parity Error","SYNC Header Error","Rx Valid de-assertion"};
static char g1_str[EC_G1_NUM][MAX_CMD_LENGTH]= {"Reserved","Reserved","Reserved","Reserved","Reserved","Detect EI Infer","Receiver Error","Rx Recovery Request","N_FTS Timeout","Framing Error","Deskew Error"};
static char g2_str[EC_G2_NUM][MAX_CMD_LENGTH]= {"BAD TLP","LCRC Error","BAD DLLP","Replay Number Rollover","Replay Timeout","Rx Nak DLLP","Tx Nak DLLP","Retry TLP"};
static char g3_str[EC_G3_NUM][MAX_CMD_LENGTH]= {"FC Timeout","Poisoned TLP","ECRC Error","Unsupported Request","Completer Abort","Completion Timeout"};

static char* str_ltssm[] = {
	"DETECT_QUIET",		"DETECT_ACT",		"POLL_ACTIVE",
	"POLL_COMPLIANCE",	"POLL_CONFIG",		"PRE_DETECT_QUIET",
	"DETECT_WAIT",		"CFG_LINKWD_START",	"CFG_LINKWD_ACCEPT",
	"CFG_LANENUM_WAIT",	"CFG_LANENUM_ACCEPT",	"CFG_COMPLETE",
	"CFG_IDLE",		"RCVRY_LOCK",		"RCVRY_SPEED",
	"RCVRY_RCVRCFG",	"RCVRY_IDLE",		"L0",
	"L0S",			"L123_SEND_EIDLE",	"L1_IDLE",
	"L2_IDLE",		"L2_WAKE",		"DISABLED_ENTRY",
	"DISABLED_IDLE",	"DISABLED",		"LPBK_ENTRY",
	"LPBK_ACTIVE",		"LPBK_EXIT",		"LPBK_EXIT_TIMEOUT",
	"HOT_RESET_ENTRY",	"HOT_RESET",		"RCVRY_EQ0",
	"RCVRY_EQ1",		"RCVRY_EQ2",		"RCVRY_EQ3",
};

// mask in bootstrap regsiter
#define TOP_BOOTSTRAP_OFS	(0x00)
#define TOP_PCIE_BOOT_MSK	GENMASK(8, 7)
#define TOP_PCIE_BOOT_RC	(0)
#define TOP_PCIE_BOOT_EP	(1)
#define TOP_PCIE_BOOT_EP_CPU_ON	(2)
#define TOP_REFCLK_MSK		GENMASK(10, 10)
#define TOP_REFCLK_MPLL5	(0)
#define TOP_REFCLK_PAD		(1)

// mask in pcie pin relative register
#define TOP_PCIE_OFS		(0x28)
#define TOP_REFCLK_OUT_MSK	GENMASK(9, 9)
#define TOP_REFCLK_NOT_OUT	(0)
#define TOP_REFCLK_OUT		(1)

#define DBI_DEVICE_CONTROL_DEVICE_STATUS_REG	(0x78)
#define PCIE_CAP_MAX_PAYLOAD_SIZE_CS_MSK	GENMASK(7, 5)

#define DBI_LINK_CONTROL_LINK_STATUS_REG	(0x80)
#define PCIE_CAP_LINK_SPEED_MSK	GENMASK(19, 16)

#define DBI_UNCORR_ERR_STATUS_OFF		(0x104)
#define PSND_TLP_EGRSS_BLOCKED_ERR_STATUS_MSK	GENMASK(26, 26)
#define INTERNAL_ERR_STATUS_MSK			GENMASK(22, 22)
#define UNSUPPORTED_REQ_ERR_STATUS_MSK		GENMASK(20, 20)
#define ECRC_ERR_STATUS_MSK			GENMASK(19, 19)
#define MALF_TLP_ERR_STATUS_MSK			GENMASK(18, 18)
#define REC_OVERFLOW_ERR_STATUS_MSK		GENMASK(17, 17)
#define UNEXP_CMPLT_ERR_STATUS_MSK		GENMASK(16, 16)
#define CMPLT_ABORT_ERR_STATUS_MSK		GENMASK(15, 15)
#define CMPLT_TIMEOUT_ERR_STATUS_MSK		GENMASK(14, 14)
#define FC_PROTOCOL_ERR_STATUS_MSK		GENMASK(13, 13)
#define POIS_TLP_ERR_STATUS_MSK			GENMASK(12, 12)
#define SURPRISE_DOWN_ERR_STATUS_MSK		GENMASK(5, 5)
#define DL_PROTOCOL_ERR_STATUS_MSK		GENMASK(4, 4)

#define DBI_CORR_ERR_STATUS_OFF			(0x110)
#define HEADER_LOG_OVERFLOW_STATUS_MSK		GENMASK(15, 15)
#define CORRECTED_INT_ERR_STATUS_MSK		GENMASK(14, 14)
#define ADVISORY_NON_FATAL_ERR_STATUS_MSK	GENMASK(13, 13)
#define RPL_TIMER_TIMEOUT_STATUS_MSK		GENMASK(12, 12)
#define REPLAY_NO_ROLEOVER_STATUS_MSK		GENMASK(8, 8)
#define BAD_DLLP_STATUS_MSK			GENMASK(7, 7)
#define BAD_TLP_STATUS_MSK			GENMASK(6, 6)
#define RX_ERR_STATUS_MSK			GENMASK(0, 0)

#define PCIE_TOP_BASE		(0x2F04F0000)
// PCIETOP 0x08
#define PCIETOP_STS_OFS		(0x08)
#define PCIETOP_PHY_LINK_MSK	GENMASK(11, 11)
#define PCIETOP_CTRL_LINK_MSK	GENMASK(0, 0)
#define PCIETOP_LTSSM_MSK	GENMASK(9, 4)

#define PCIE_PHY_BASE		(0x2F08B0000)
// phy reg 0xA00
#define PHY_AGC_MSK		GENMASK(3, 0)
#define PHY_CSPK_MSK		GENMASK(14, 12)
#define PHY_RSPK_MSK		GENMASK(19, 8)

// phy reg 0xA08
#define PHY_CDR_TC_MSK		GENMASK(25, 16)
#define PHY_CDR_CT_MSK		GENMASK(6, 0)

// phy reg 0xA0C
#define PHY_CDR_FT_MSK		GENMASK(6, 0)

// phy reg 0xA18
#define PHY_FREQ_VAL_TXCLK_MSK	GENMASK(29, 16)
#define PHY_FREQ_VAL_RXCLK_MSK	GENMASK(13, 0)

// phy reg 0x3018
#define PHY_DEC_ERR_MSK		GENMASK(27, 24)	// 128/130b decode err
#define PHY_EBUF_OVF_MSK	GENMASK(28, 28)	// EBUF overflow
#define PHY_EBUF_UNF_MSK	GENMASK(29, 29)	// EBUF underflow

static void print_ras_error_count(struct seq_file *sfile, void *v)
{
	UINT32 Sel;
	UINT32 data;
	UINT32 i,j;

	seq_printf(sfile, "\r\n Test RAS D.E.S --- event count analysis function\r\n");
	//printk("\r\n TBA select = 0x%x\r\n",g_user_ec_sel);

	Sel = 0x0000;

    spin_lock(&glb_nvt_pcie_chip_ptr->lock_ras);

	i=0;
	seq_printf(sfile, "=== Physical Layer 0 === (Grp %d)\n\n\t", i);
	for(j=0; j <EC_G0_NUM; j++){
		pcie_ras_ec_set(Sel|j); //Group0
		data = pcie_ras_ec_data();
		seq_printf(sfile, "(%d)[%-25s]: 0x%08x, ", j, g0_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(sfile, "\n\t");
		else
			seq_printf(sfile, " ;\t ");
	}
	seq_printf(sfile, "\r\n");
	Sel+=0x100;

	i=1;
	seq_printf(sfile, "\r\n=== Physical Layer 1 === (Grp %d)\n\n\t", i);
	for(j=0; j <EC_G1_NUM; j++){
		pcie_ras_ec_set(Sel|j); //Group1
		data = pcie_ras_ec_data();
		seq_printf(sfile, "(%d)[%-25s]: 0x%08x, ", j, g1_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(sfile, "\n\t");
		else
			seq_printf(sfile, " ;\t ");
	}
	seq_printf(sfile, "\r\n");
	Sel+=0x100;

	i=2;
	seq_printf(sfile, "\r\n=== Data Link Layer === (Grp %d)\n\n\t", i);
	for(j=0; j <EC_G2_NUM; j++){
		pcie_ras_ec_set(Sel|j); //Group2
		data = pcie_ras_ec_data();
		seq_printf(sfile, "(%d)[%-25s]: 0x%08x, ", j, g2_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(sfile, "\n\t");
		else
			seq_printf(sfile, " ;\t ");
	}
	seq_printf(sfile, "\r\n");
	Sel+=0x100;

	i=3;
	seq_printf(sfile, "\r\n=== Transaction Layer === (Grp %d)\n\n\t", i);
	for(j=0; j <EC_G3_NUM; j++){
		pcie_ras_ec_set(Sel|j); //Group3
		data = pcie_ras_ec_data();
		seq_printf(sfile, "(%d)[%-25s]: 0x%08x, ", j, g3_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(sfile, "\n\t");
		else
			seq_printf(sfile, " ;\t ");
	}
	seq_printf(sfile, "\r\n");
    spin_unlock(&glb_nvt_pcie_chip_ptr->lock_ras);

}

static char* ltssm2str(int ltssm)
{
	if (ltssm < ARRAY_SIZE(str_ltssm))
		return str_ltssm[ltssm];
	else
		return "INVALID";
}

static int proc_sts_show(struct seq_file *sfile, void *v)
{
	struct nvt_pcie_chip *chip = (struct nvt_pcie_chip *)sfile->private;
	void __iomem		*ptr;
	void __iomem		*ptr_top;
	void __iomem		*ptr_pcietop;
	void __iomem		*ptr_phy;
	UINT32   val;

	ptr = ioremap(0x2FD000000, 4096);
	ptr_top = ioremap(NVT_TOP_BASE_PHYS, 4096);
	ptr_pcietop = ioremap(PCIE_TOP_BASE, 4096);
	ptr_phy = ioremap(PCIE_PHY_BASE, 16384);

	val = readl(ptr+DBI_UNCORR_ERR_STATUS_OFF);
	if (val) {
		seq_printf(sfile, "\r\n UNCORR_ERR_STATUS_OFF :\r\n");
		seq_printf(sfile, "0x2FD000104: 0x%8x\r\n", readl(ptr+DBI_UNCORR_ERR_STATUS_OFF));
		writel(val, (ptr+DBI_UNCORR_ERR_STATUS_OFF));
		uncorr_error_cnt++;
	}
	val = readl(ptr+DBI_CORR_ERR_STATUS_OFF);
	if (val) {
		seq_printf(sfile, "\r\n CORR_ERR_STATUS_OFF :\r\n");
		seq_printf(sfile, "0x2FD000110: 0x%8x\r\n", readl(ptr+DBI_CORR_ERR_STATUS_OFF));
		writel(val, (ptr+DBI_CORR_ERR_STATUS_OFF));
		rx_error_cnt++;
	}
	// Device Status Register
	val = readl(ptr+DBI_DEVICE_CONTROL_DEVICE_STATUS_REG);
	if (val & (0x3 << 16)) {
		seq_printf(sfile, "\r\n DEVICE_STATUS :\r\n");
		seq_printf(sfile, "0x2FD000078: 0x%8x\r\n", readl(ptr+DBI_DEVICE_CONTROL_DEVICE_STATUS_REG));
		writel(val, (ptr+DBI_DEVICE_CONTROL_DEVICE_STATUS_REG));
	      //rx_error_cnt ++;
	}
	if (val & (0x3 << 18)) {
		seq_printf(sfile, "\r\n DEVICE_STATUS :\r\n");
		seq_printf(sfile, "0x2FD000078: 0x%8x\r\n", readl(ptr+DBI_DEVICE_CONTROL_DEVICE_STATUS_REG));
		writel(val, (ptr+DBI_DEVICE_CONTROL_DEVICE_STATUS_REG));
	       //uncorr_error_cnt ++;
	}

	seq_printf(sfile, "\n");
	seq_printf(sfile, "\r\n ===================== \r\n");
	seq_printf(sfile, "This IC is %s (set by DTS)\n", (chip->is_rc)?"RC":"EP");
	val = readl(ptr_top+TOP_BOOTSTRAP_OFS);
	seq_printf(sfile, "Bootstrap reg (at 0x%lx) = 0x%08x: boot mode ",
		NVT_TOP_BASE_PHYS+TOP_BOOTSTRAP_OFS, val);
	switch (FIELD_GET(TOP_PCIE_BOOT_MSK, val)) {
	case TOP_PCIE_BOOT_RC:
		seq_printf(sfile, "RC");
		break;
	case TOP_PCIE_BOOT_EP:
		seq_printf(sfile, "EP");
		break;
	default:
		seq_printf(sfile, "INVALID");
		break;
	}
	seq_printf(sfile, ", REFCLK src from %s\n",
		(FIELD_GET(TOP_REFCLK_MSK, val)==TOP_REFCLK_MPLL5)?"MPLL5":"PAD");
	val = readl(ptr_top+TOP_PCIE_OFS);
	seq_printf(sfile, "TOP ctrl reg 10 (at 0x%lx) = 0x%08x: REFCLK pad output is %s\n",
		NVT_TOP_BASE_PHYS+TOP_PCIE_OFS, val,
		(FIELD_GET(TOP_REFCLK_OUT_MSK, val)==TOP_REFCLK_NOT_OUT)?"Disabled":"Enabled");
	val = readl(ptr+DBI_LINK_CONTROL_LINK_STATUS_REG);
	seq_printf(sfile, "Link speeed gen%ld, ", FIELD_GET(PCIE_CAP_LINK_SPEED_MSK, val));
	val = readl(ptr_pcietop+PCIETOP_STS_OFS);
	seq_printf(sfile, "PHY link %s, CTRL link %s, LTSSM %s, ",
		(FIELD_GET(PCIETOP_PHY_LINK_MSK, val)==1)?"ON":"OFF",
		(FIELD_GET(PCIETOP_CTRL_LINK_MSK, val)==1)?"ON":"OFF",
		ltssm2str(FIELD_GET(PCIETOP_LTSSM_MSK, val)));
	val = readl(ptr+DBI_DEVICE_CONTROL_DEVICE_STATUS_REG);
	seq_printf(sfile, "Max payload size %d bytes\n", 128*(1<<FIELD_GET(PCIE_CAP_MAX_PAYLOAD_SIZE_CS_MSK, val)));
	seq_printf(sfile, "\r\n ===================== \r\n");
	seq_printf(sfile, "\r\n HW intrested info: plz consult with HW member \r\n\r\n");
	val = readl(ptr_phy+0xA00);
	seq_printf(sfile, "Phy reg 0xA00 = 0x%08x: AGC 0x%lx, CSPK 0x%lx, RSPK 0x%lx\n",
		val,
		FIELD_GET(PHY_AGC_MSK, val),
		FIELD_GET(PHY_CSPK_MSK, val),
		FIELD_GET(PHY_RSPK_MSK, val));
	val = readl(ptr_phy+0xA08);
	seq_printf(sfile, "Phy reg 0xA08 = 0x%08x: CDR_TC 0x%lx, CDR_CT 0x%lx\n",
		val,
		FIELD_GET(PHY_CDR_TC_MSK, val),
		FIELD_GET(PHY_CDR_CT_MSK, val));
	val = readl(ptr_phy+0xA0C);
	seq_printf(sfile, "Phy reg 0xA0C = 0x%08x: CDR_FT 0x%lx\n",
		val,
		FIELD_GET(PHY_CDR_FT_MSK, val));
	val = readl(ptr_phy+0x918);
	writel(val|(1<<17), ptr_phy+0x918);
	writel(val&(~(1<<17)), ptr_phy+0x918);
	udelay(10);
	val = readl(ptr_phy+0xA18);
	seq_printf(sfile, "Phy reg 0xA18 = 0x%08x: FREQ_VAL_TXCLK 0x%lx, FREQ_VAL_RXCLK 0x%lx\n",
		val,
		FIELD_GET(PHY_FREQ_VAL_TXCLK_MSK, val),
		FIELD_GET(PHY_FREQ_VAL_RXCLK_MSK, val));
	seq_printf(sfile, "\r\n ===================== \r\n");
	seq_printf(sfile, "AER check...");
	if (readl(ptr+DBI_UNCORR_ERR_STATUS_OFF) || readl(ptr+DBI_CORR_ERR_STATUS_OFF)) {
		val = readl(ptr+DBI_UNCORR_ERR_STATUS_OFF);
		if (val) {
			seq_printf(sfile, "\r\n UNCORR_ERR_STATUS_OFF :\r\n");
			seq_printf(sfile, "0x2FD000104: 0x%8x\r\n", readl(ptr+DBI_UNCORR_ERR_STATUS_OFF));
			writel(val, (ptr+DBI_UNCORR_ERR_STATUS_OFF));
			uncorr_error_cnt++;

			if (FIELD_GET(PSND_TLP_EGRSS_BLOCKED_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tPoisoned TLP Egress Blocked Status\r\n");
			}
			if (FIELD_GET(INTERNAL_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tUncorrectable Internal Error Status\r\n");
			}
			if (FIELD_GET(UNSUPPORTED_REQ_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tUnsupported Request Error Status\r\n");
			}
			if (FIELD_GET(ECRC_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tECRC Error Status\r\n");
			}
			if (FIELD_GET(MALF_TLP_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tMalformed TLP Status\r\n");
			}
			if (FIELD_GET(REC_OVERFLOW_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tReceiver Overflow Status\r\n");
			}
			if (FIELD_GET(UNEXP_CMPLT_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tUnexpected Completion Status\r\n");
			}
			if (FIELD_GET(CMPLT_ABORT_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tCompleter Abort Status\r\n");
			}
			if (FIELD_GET(CMPLT_TIMEOUT_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tCompletion Timeout Status\r\n");
			}
			if (FIELD_GET(FC_PROTOCOL_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tFlow Control Protocol Error Status\r\n");
			}
			if (FIELD_GET(POIS_TLP_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tPoisoned TLP Received Status\r\n");
			}
			if (FIELD_GET(SURPRISE_DOWN_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tSurprise Down Error Status\r\n");
			}
			if (FIELD_GET(DL_PROTOCOL_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tData Link Protocol Error Status\r\n");
			}
		}
		val = readl(ptr+DBI_CORR_ERR_STATUS_OFF);
		if (val) {
			seq_printf(sfile, "\r\n CORR_ERR_STATUS_OFF :\r\n");
			seq_printf(sfile, "0x2FD000110: 0x%8x\r\n", readl(ptr+DBI_CORR_ERR_STATUS_OFF));
			writel(val, (ptr+DBI_CORR_ERR_STATUS_OFF));
			rx_error_cnt++;

			if (FIELD_GET(HEADER_LOG_OVERFLOW_STATUS_MSK, val)) {
				seq_printf(sfile, "\tHeader Log Overflow Status\r\n");
			}
			if (FIELD_GET(CORRECTED_INT_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tCorrected Internal Error Status\r\n");
			}
			if (FIELD_GET(ADVISORY_NON_FATAL_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tAdvisory Non-Fatal Error Status\r\n");
			}
			if (FIELD_GET(RPL_TIMER_TIMEOUT_STATUS_MSK, val)) {
				seq_printf(sfile, "\tReplay Timer Timeout Status\r\n");
			}
			if (FIELD_GET(REPLAY_NO_ROLEOVER_STATUS_MSK, val)) {
				seq_printf(sfile, "\tREPLAY_NUM Rollover Status\r\n");
			}
			if (FIELD_GET(BAD_DLLP_STATUS_MSK, val)) {
				seq_printf(sfile, "\tBad DLLP Status\r\n");
			}
			if (FIELD_GET(BAD_TLP_STATUS_MSK, val)) {
				seq_printf(sfile, "\tBad TLP Status\r\n");
			}
			if (FIELD_GET(RX_ERR_STATUS_MSK, val)) {
				seq_printf(sfile, "\tReceiver Error Status\r\n");
			}

		}
	} else {
		seq_printf(sfile, "OK\r\n");
	}
	seq_printf(sfile, "\r\n Error count: \r\n");
	seq_printf(sfile, "uncorr_error_cnt: %d\r\n", uncorr_error_cnt);
	seq_printf(sfile, "rx_error_cnt: %d\r\n", rx_error_cnt);
	seq_printf(sfile, "\r\n ===================== \r\n");

	if(run == 0)
	{
		run++;

		val = readl(ptr_phy+0x3014);
		val = val|0x2;
		writel(val, (ptr_phy+0x3014));

		val = readl(ptr_phy+0x3014);
		val &= ~0x00000002;
		writel(val, (ptr_phy+0x3014));
	}

	seq_printf(sfile, "0x2F08B301C: 0x%8x\r\n", readl(ptr_phy+0x301C));
	seq_printf(sfile, "0x2F08B3018: 0x%8x\r\n", readl(ptr_phy+0x3018));
	val = readl(ptr_phy+0x3018);
	if (FIELD_GET(PHY_DEC_ERR_MSK, val) || FIELD_GET(PHY_EBUF_OVF_MSK, val) || FIELD_GET(PHY_EBUF_UNF_MSK, val)) {
		if (FIELD_GET(PHY_DEC_ERR_MSK, val)) {
			seq_printf(sfile, "PHY DEC error 0x%lx, ", FIELD_GET(PHY_DEC_ERR_MSK, val));
		}
		if (FIELD_GET(PHY_EBUF_OVF_MSK, val)) {
			seq_printf(sfile, "PHY EBUF overflow, ");
		}
		if (FIELD_GET(PHY_EBUF_UNF_MSK, val)) {
			seq_printf(sfile, "PHY EBUF underflow, ");
		}
	} else {
		seq_printf(sfile, "PHY status check OK\n");
	}
	seq_printf(sfile, "\r\n");

	iounmap(ptr);
	iounmap(ptr_top);
	iounmap(ptr_pcietop);
	iounmap(ptr_phy);

	seq_printf(sfile, "\r\n ===================== \r\n");
	print_ras_error_count(sfile, v);

	return 0;
}

static int proc_sts_open(struct inode *inode, struct file *file)
{
	//printk("\n");
	//printk("%s: ptr 0x%p\r\n", __func__, pdrv_info_data);
	return single_open(file, proc_sts_show, PDE_DATA(inode));
}

static ssize_t proc_sts_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	//int val;
	char cmd_line[MAX_CMD_LENGTH];
	//char *cmdstr = cmd_line;
	//const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	//char *p_arg;
	//struct nvt_pcie_chip *nvt_pcie_chip_ptr = ((struct seq_file *)(file->private_data))->private;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk(KERN_ERR "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	//printk("CMD:%s\n", cmd_line);

	//p_arg = strsep(&cmdstr, delimiters);
	//sscanf(p_arg, "%d", &g_user_ec_sel);

	//printk("CMD:%d\n", g_user_ec_sel);

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_sts_fops = {
	.proc_open    = proc_sts_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = proc_sts_write
};

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static int proc_ras_show(struct seq_file *sfile, void *v)
{
	//UINT32 data;

	printk("\n");
	printk("\r\n Test RAS D.E.S --- time based analysis function\r\n");
	printk("\r\n TBA select = 0x%x\r\n",g_user_sel);

	pcie_ras_tba_set(g_user_sel , PCIE_TBA_TIME_MANUAL);
	pcie_ras_tba_start();


	return 0;
}

static int proc_ras_open(struct inode *inode, struct file *file)
{
	printk("\n");
	//printk("%s: ptr 0x%p\r\n", __func__, pdrv_info_data);
	return single_open(file, proc_ras_show, pdrv_info_data);
}

static ssize_t proc_ras_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int val;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	//struct nvt_pcie_chip *nvt_pcie_chip_ptr = ((struct seq_file *)(file->private_data))->private;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk(KERN_ERR "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	//printk("CMD:%s\n", cmd_line);

	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%d", &val);
	g_user_sel = val;

	printk("========================\n");
	printk("PCIE_TBA_SEL_ONECYCLE: 0\n");
	printk("PCIE_TBA_SEL_TXL0S: 1\n");
	printk("PCIE_TBA_SEL_RXL0S: 2\n");
	printk("PCIE_TBA_SEL_L0: 3\n");
	printk("PCIE_TBA_SEL_L1: 4\n");
	printk("PCIE_TBA_SEL_L1_1: 5\n");
	printk("PCIE_TBA_SEL_L1_2: 6\n");
	printk("PCIE_TBA_SEL_CFG_RCVRY: 7\n");
	printk("PCIE_TBA_SEL_TXRXL0S: 8\n");
	printk("PCIE_TBA_SEL_L1_AUX: 9\n");

	printk("PCIE_TBA_SEL_1CYCLE: 16\n");
	printk("PCIE_TBA_SEL_TXL0S_: 17\n");
	printk("PCIE_TBA_SEL_RXL0S_: 18\n");
	printk("PCIE_TBA_SEL_L0_: 19\n");
	printk("PCIE_TBA_SEL_L1_: 20\n");

	printk("PCIE_TBA_SEL_CFG_RCVRY_: 23\n");
	printk("PCIE_TBA_SEL_TXRXL0S_: 24\n");

	printk("PCIE_TBA_SEL_TX_TLP: 32\n");
	printk("PCIE_TBA_SEL_RX_TLP: 33\n");
	printk("PCIE_TBA_SEL_TX_CCIXTLP: 34\n");
	printk("PCIE_TBA_SEL_RX_CCIXTLP: 35\n");
	printk("========================\n");

	printk("CMD:%d\n", g_user_sel);

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_ras_fops = {
	.proc_open    = proc_ras_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = proc_ras_write
};


//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================

static int proc_ras_stop_show(struct seq_file *sfile, void *v)
{
	UINT32 data;

	pcie_ras_tba_stop();

	data =pcie_ras_tba_data();

	switch(g_user_sel){

		case PCIE_TBA_SEL_ONECYCLE:
			printk("RAS TBA Sel = PCIE_TBA_SEL_ONECYCLE\r\n");
		break;
		case PCIE_TBA_SEL_TXL0S:
			printk("RAS TBA Sel = PCIE_TBA_SEL_TXL0S\r\n");
		break;
		case PCIE_TBA_SEL_RXL0S:
			printk("RAS TBA Sel = PCIE_TBA_SEL_RXL0S\r\n");
		break;
		case PCIE_TBA_SEL_L0:
			printk("RAS TBA Sel = PCIE_TBA_SEL_L0\r\n");
		break;
		case PCIE_TBA_SEL_L1:
			printk("RAS TBA Sel = PCIE_TBA_SEL_L1\r\n");
		break;
		case PCIE_TBA_SEL_L1_1:
			printk("RAS TBA Sel = PCIE_TBA_SEL_L1_1\r\n");
		break;
		case PCIE_TBA_SEL_L1_2:
			printk("RAS TBA Sel = PCIE_TBA_SEL_L1_2\r\n");
		break;
		case PCIE_TBA_SEL_CFG_RCVRY:
			printk("RAS TBA Sel = PCIE_TBA_SEL_CFG_RCVRY\r\n");
		break;
		case PCIE_TBA_SEL_TXRXL0S:
			printk("RAS TBA Sel = PCIE_TBA_SEL_TXRXL0S\r\n");
		break;
		case PCIE_TBA_SEL_L1_AUX:
			printk("RAS TBA Sel = PCIE_TBA_SEL_L1_AUX\r\n");
		break;
		case PCIE_TBA_SEL_1CYCLE:
			printk("RAS TBA Sel = PCIE_TBA_SEL_1CYCLE\r\n");
		break;
		case PCIE_TBA_SEL_TXL0S_:
			printk("RAS TBA Sel = PCIE_TBA_SEL_TXL0S_\r\n");
		break;
		case PCIE_TBA_SEL_RXL0S_:
			printk("RAS TBA Sel = PCIE_TBA_SEL_RXL0S_\r\n");
		break;
		case PCIE_TBA_SEL_L0_:
			printk("RAS TBA Sel = PCIE_TBA_SEL_L0_\r\n");
		break;
		case PCIE_TBA_SEL_L1_:
			printk("RAS TBA Sel = PCIE_TBA_SEL_L1_\r\n");
		break;
		case PCIE_TBA_SEL_CFG_RCVRY_:
			printk("RAS TBA Sel = PCIE_TBA_SEL_CFG_RCVRY_\r\n");
		break;
		case PCIE_TBA_SEL_TXRXL0S_:
			printk("RAS TBA Sel = PCIE_TBA_SEL_TXRXL0S_\r\n");
		break;
		case PCIE_TBA_SEL_TX_TLP:
			printk("RAS TBA Sel = PCIE_TBA_SEL_TX_TLP\r\n");
		break;
		case PCIE_TBA_SEL_RX_TLP:
			printk("RAS TBA Sel = PCIE_TBA_SEL_RX_TLP\r\n");
		break;
		case PCIE_TBA_SEL_TX_CCIXTLP:
			printk("RAS TBA Sel = PCIE_TBA_SEL_TX_CCIXTLP\r\n");
		break;
		case PCIE_TBA_SEL_RX_CCIXTLP:
			printk("RAS TBA Sel = PCIE_TBA_SEL_RX_CCIXTLP\r\n");
		break;
		default:
			printk("RAS TBA Sel undefined = 0x%x\r\n", g_user_sel);
		break;
	}

	printk("RAS TBA data  = %d\r\n", data);
	printk("RAS TBA data (HEX) = 0x%x\r\n", data);

	return 0;
}

static int proc_ras_stop_open(struct inode *inode, struct file *file)
{
	printk("\n");
	//printk("%s: ptr 0x%p\r\n", __func__, pdrv_info_data);
	return single_open(file, proc_ras_stop_show, pdrv_info_data);
}

static ssize_t proc_ras_stop_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int val;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	//struct nvt_pcie_chip *nvt_pcie_chip_ptr = ((struct seq_file *)(file->private_data))->private;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk(KERN_ERR "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	//printk("CMD:%s\n", cmd_line);

	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);


	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_ras_stop_fops = {
	.proc_open    = proc_ras_stop_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = proc_ras_stop_write
};

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================

static int proc_cmd_show(struct seq_file *sfile, void *v)
{
	printk("\n");
	// call MSIX pending display function
	// (sfile->private points struct nvt_pcie_chip *)
	msix_proc_list_pending(sfile, sfile->private);
//	dump_pcie_dbi(sfile->private);	// sample

{
	void __iomem		*ptr;

	ptr = ioremap(0x610020000, 4096);

	printk("0x610020000: 0x%8x\t0x%8x\t0x%8x\t0x%8x\r\n",
		readl(ptr+0), readl(ptr+4), readl(ptr+8), readl(ptr+12));

	iounmap(ptr);
}
{
	void __iomem		*ptr;

	ptr = ioremap(0x2FD000000, 4096);

	printk("0x2FD000000: 0x%8x\t0x%8x\t0x%8x\t0x%8x\r\n",
		readl(ptr+0), readl(ptr+4), readl(ptr+8), readl(ptr+12));

	iounmap(ptr);
}
	dump_pcie_msix_tbl(sfile->private);	// sample
	return 0;
}

static int proc_cmd_open(struct inode *inode, struct file *file)
{
	printk("\n");
	printk("%s: ptr 0x%p\r\n", __func__, pdrv_info_data);
	return single_open(file, proc_cmd_show, pdrv_info_data);
}

static ssize_t proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int val;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = ((struct seq_file *)(file->private_data))->private;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk(KERN_ERR "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	printk("CMD:%s\n", cmd_line);

	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);

	// call clear MSIX pending
	if (val != 0) {
		msix_proc_clear_pending(nvt_pcie_chip_ptr);
	}

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	.proc_open    = proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = proc_cmd_write
};

static int proc_eom_show(struct seq_file *sfile, void *v)
{
	struct nvt_pcie_chip *chip = (struct nvt_pcie_chip *)sfile->private;

	spin_lock(&chip->lock_eom);
	if (chip->state_eom == EOM_SM_SCAN_ONGOING) {
		unsigned long elps = jiffies - chip->eom_start_jiffies;
		printk("%s: eom scan is still on-going (elps %d s), please cat after scanning is done\r\n",
			__func__,
			jiffies_to_msecs(elps)/1000);
	} else if (chip->state_eom == EOM_SM_IDLE) {
		printk("%s: eom scan is still not triggered, plz write scan_rx_eye to trigger\r\n", __func__);
	} else {
		int i, j;
		unsigned long elps = chip->eom_end_jiffies - chip->eom_start_jiffies;

		seq_printf(sfile, "\r\nEOM scan result (elps %d s) :\r\n", jiffies_to_msecs(elps)/1000);
		for (i=63; i>=0; i--) {
			seq_printf(sfile, "  ");
			for (j=0; j<64; j++) {
				seq_printf(sfile, "%3d ", chip->eom_result[j][i]);
			}
			seq_printf(sfile, "\r\n");
		}
		for (i=0; i<64; i++) {
			seq_printf(sfile, "  ");
			for (j=0; j<64; j++) {
				seq_printf(sfile, "%3d ", chip->eom_result[j][i+64]);
			}
			seq_printf(sfile, "\r\n");
		}
	}
	spin_unlock(&chip->lock_eom);

	return 0;
}

static int proc_eom_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_eom_show, PDE_DATA(inode));
}

static ssize_t proc_eom_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int val;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = ((struct seq_file *)(file->private_data))->private;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk(KERN_ERR "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);

	spin_lock(&nvt_pcie_chip_ptr->lock_eom);
	if (nvt_pcie_chip_ptr->state_eom == EOM_SM_SCAN_ONGOING) {
		printk("%s: previous scan is still on-going\r\n", __func__);
	} else {
		printk("%s: start scanning...\r\n", __func__);
		nvt_pcie_chip_ptr->state_eom = EOM_SM_SCAN_ONGOING;
		nvt_pcie_chip_ptr->eom_start_jiffies = jiffies;
		schedule_delayed_work(&nvt_pcie_chip_ptr->eom_work, 1);
	}
	spin_unlock(&nvt_pcie_chip_ptr->lock_eom);

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_eom_fops = {
	.proc_open    = proc_eom_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = proc_eom_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, " pcie_sts:\tDisplay PCIE link/phy status\n");
	seq_printf(sfile, " scan_rx_eye:\tscan rx eye diagram (trigger by write file)\n");
	seq_printf(sfile, "=====================================================================\n");
	return 0;
}

static int proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	.proc_open   = proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

static UINT32 eom_scan_y(void __iomem *ptr_phy, int idx_y)
{
	int i;
	UINT32 val;

	// change y phase
	val = readl(ptr_phy+0x1048);
	val &= ~GENMASK(13, 8);
	val |= FIELD_PREP(GENMASK(13, 8), idx_y);
	writel(val, ptr_phy+0x1048);

	val = readl(ptr_phy+0x1214);
	val &= ~GENMASK(18, 18);
	writel(val, ptr_phy+0x1214);
//	usleep_range(10, 20);
	val |= FIELD_PREP(GENMASK(18, 18), 1);
	writel(val, ptr_phy+0x1214);

	for (i=0; i<1000; i++) {
		val = readl(ptr_phy+0x115C);
		if (FIELD_GET(GENMASK(1, 1), val)) break;

		usleep_range(10, 20);
	}
	if (i >= 1000) printk("%s: loop timeout\r\n", __func__);

	val = readl(ptr_phy+0x1160);
	return val;
}

static void eom_scan_rx_work(struct work_struct *work)
{
	struct delayed_work *delayed_work = to_delayed_work(work);
	struct nvt_pcie_chip *chip =
		container_of(delayed_work, struct nvt_pcie_chip, eom_work);
	void __iomem		*ptr_phy;
	UINT32   val;
	int idx_x;

	ptr_phy = ioremap(PCIE_PHY_BASE, 16384);

	// EOM manual
	val = readl(ptr_phy+0x1034);
	val |= FIELD_PREP(GENMASK(30, 29), 3);
	writel(val, ptr_phy+0x1034);

	// set EOM period
	val = readl(ptr_phy+0x1214);
	val |= FIELD_PREP(GENMASK(30, 19), 0xFFF);
	writel(val, ptr_phy+0x1214);

	for (idx_x=0; idx_x<64; idx_x++) {
		int idx_y;

		// change x phase
		val = readl(ptr_phy+0x1048);
		val &= ~GENMASK(7, 2);
		val |= FIELD_PREP(GENMASK(7, 2), idx_x);
		writel(val, ptr_phy+0x1048);

		// scan upper eye
		val = readl(ptr_phy+0x1048);
		val |= FIELD_PREP(GENMASK(14, 14), 1);
		writel(val, ptr_phy+0x1048);
		val = readl(ptr_phy+0x1214);
		val |= FIELD_PREP(GENMASK(6, 6), 1);
		writel(val, ptr_phy+0x1214);
		val = readl(ptr_phy+0x1048);
		val &= ~GENMASK(0, 0);
		writel(val, ptr_phy+0x1048);

		for (idx_y=63; idx_y>=0; idx_y--) {
			UINT64 a;

			a = eom_scan_y(ptr_phy, idx_y);
			a = (a*100 + 0x7FF80/2) / 0x7FF80;
			if (a > 100) a = 100;

			chip->eom_result[idx_x][idx_y] = a;
		}

		// scan lower eye
		val = readl(ptr_phy+0x1048);
		val &= ~GENMASK(14, 14);
		writel(val, ptr_phy+0x1048);
		val = readl(ptr_phy+0x1214);
		val &= ~GENMASK(6, 6);
		writel(val, ptr_phy+0x1214);
		val = readl(ptr_phy+0x1048);
		val |= FIELD_PREP(GENMASK(0, 0), 1);
		writel(val, ptr_phy+0x1048);

		for (idx_y=0; idx_y<64; idx_y++) {
			UINT64 a;

			a = eom_scan_y(ptr_phy, idx_y);
			a = (a*100 + 0x7FF80/2) / 0x7FF80;
			if (a > 100) a = 100;

			chip->eom_result[idx_x][64+idx_y] = a;
		}
	}

	iounmap(ptr_phy);

	chip->eom_end_jiffies = jiffies;

	spin_lock(&chip->lock_eom);
	chip->state_eom = EOM_SM_SCAN_DONE;
	spin_unlock(&chip->lock_eom);
}

int nvt_pcie_proc_init(struct nvt_pcie_chip *nvt_pcie_chip_ptr)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	printk("%s: ptr 0x%p\r\n", __func__, nvt_pcie_chip_ptr);
    glb_nvt_pcie_chip_ptr = nvt_pcie_chip_ptr;
	memset(nvt_pcie_chip_ptr->eom_result, 0, sizeof(nvt_pcie_chip_ptr->eom_result));
	spin_lock_init(&nvt_pcie_chip_ptr->lock_eom);
	nvt_pcie_chip_ptr->state_eom = EOM_SM_IDLE;
	INIT_DELAYED_WORK(&nvt_pcie_chip_ptr->eom_work, eom_scan_rx_work);

	pmodule_root = proc_mkdir("nvt_pcie", NULL);
	if (pmodule_root == NULL) {
		printk(KERN_ERR "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	nvt_pcie_chip_ptr->pproc_module_root = pmodule_root;

	pentry = proc_create_data("pcie_sts", S_IRUGO | S_IXUGO, pmodule_root, &proc_sts_fops, (void*)nvt_pcie_chip_ptr);
	if (pentry == NULL) {
		printk("error to pcie_sts proc\n");
		goto remove_cmd;
	}

	pentry = proc_create_data("scan_rx_eye", S_IRUGO | S_IXUGO, pmodule_root, &proc_eom_fops, (void*)nvt_pcie_chip_ptr);
	if (pentry == NULL) {
		printk(KERN_ERR "failed to create proc scan_rx_eye!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}

	pentry = proc_create("ras", S_IRUGO | S_IXUGO, pmodule_root, &proc_ras_fops);
	if (pentry == NULL) {
		printk("error to ras proc\n");
		goto remove_cmd;
	}

	pentry = proc_create("ras_stop", S_IRUGO | S_IXUGO, pmodule_root, &proc_ras_stop_fops);
	if (pentry == NULL) {
		printk("error to ras_stop proc\n");
		goto remove_cmd;
	}

	pentry = proc_create("cmd_msix", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		printk(KERN_ERR "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	nvt_pcie_chip_ptr->pproc_cmd_msix = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		printk(KERN_ERR "failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	nvt_pcie_chip_ptr->pproc_help_entry = pentry;

	pdrv_info_data = nvt_pcie_chip_ptr;

	return ret;

remove_cmd:
	proc_remove(nvt_pcie_chip_ptr->pproc_cmd_msix);

remove_root:
	proc_remove(nvt_pcie_chip_ptr->pproc_module_root);
	return ret;
}

int nvt_pcie_proc_remove(struct nvt_pcie_chip *nvt_pcie_chip_ptr)
{
	proc_remove(nvt_pcie_chip_ptr->pproc_help_entry);
	proc_remove(nvt_pcie_chip_ptr->pproc_cmd_msix);
	proc_remove(nvt_pcie_chip_ptr->pproc_module_root);

	cancel_delayed_work_sync(&nvt_pcie_chip_ptr->eom_work);
	return 0;
}

