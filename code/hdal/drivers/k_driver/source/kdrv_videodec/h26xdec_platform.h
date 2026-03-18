/*
*					0x00100000	k_driver first sdk version
*	version:		0x00000000	k_driver initial version
*/

#ifndef _VERSION_H26XD_H_
	#define _VERSION_H26XD_H_

	#define H26XD_VER           0x00100900
	//							HIIFFFBB
	//							H huge change
	//							 II interface change
	//							   FFF functional modified or bug fixed
	//							      BB branch for customer request

	#define H26XD_VER_MAJOR_N(N)     (((N)>>28)&0x0f)  // huge change
	#define H26XD_VER_MINOR_N(N)     (((N)>>20)&0xff)  // inerface change
	#define H26XD_VER_MINOR2_N(N)    (((N)>>8)&0xfff)  // functional modified or bug fixed
	#define H26XD_VER_BRANCH_N(N)    ((N)&0xff)        // branch for customer request


	#define H26XD_VER_MAJOR     H26XD_VER_MAJOR_N(H26XD_VER)   // huge change
	#define H26XD_VER_MINOR     H26XD_VER_MINOR_N(H26XD_VER)   // inerface change
	#define H26XD_VER_MINOR2    H26XD_VER_MINOR2_N(H26XD_VER)  // functional modified or bug fixed
	#define H26XD_VER_BRANCH    H26XD_VER_BRANCH_N(H26XD_VER)  // branch for customer request

	/* for converting H26XD_VER to string */
	#define STR(x)          #x
	#define STR2(x)         STR(x)
	#define H26XD_VER_STR   STR2(H26XD_VER)
#endif /* _VERSION_H26XD_H_ */

#ifndef _H26XDEC_PLATFORM_H_

#define _H26XDEC_PLATFORM_H_

#define H26XD_PLT_STR_9833X "98530"

#define H26XD_CHIP_VER_VAL			0x98530  /* define H26XD_CHIP_VER_VAL, H26XD_CHIP_VER_VAL to 0 to skip chip version checking */
#define H26XD_CHIP_VER_MASK			0xFFFF0
#define H26XD_PLT_STR				H26XD_PLT_STR_9833X

#ifdef CONFIG_NVT_IVOT_PLAT_NA51102
#define H26XD_CHIP					1
#else
#error "please define H26XD_CHIP"
#endif

#define H26XD_NUM					1
#define H26XD_CHN                   32

struct h26xdec_dev_info_t
{
	struct clk *h264d_aclk;
	struct clk *h265d_aclk;
	struct clk *h26xd_pclk; //gather between h264/h265
	struct clk *h264d_mclk;
	struct clk *h265d_mclk;
	int h26xd_clk_cnt;
};

int pf_h26xd_clk_on(int chip_idx);
int pf_h26xd_clk_off(int chip_idx);
int pf_h26xd_xclk_on(int chip_idx);
int pf_h26xd_xclk_off(int chip_idx);
int pf_h26xd_auto_clk_gate_on(int chip_idx);
int pf_h26xd_auto_clk_gate_off(int chip_idx);
/* get plock in hz */
unsigned int pf_h26xd_get_plck(int chip_idx);

extern const char irq_name[H26XD_CHIP][H26XD_NUM][20];

#endif /* _H26XDEC_PLATFORM_H_ */

