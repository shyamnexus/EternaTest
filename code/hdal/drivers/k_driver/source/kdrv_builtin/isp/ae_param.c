
#include "kwrap/type.h"
#include "ae_param.h"
#include "isp_builtin_int.h"

#define AE_ISO_BASE             100
#define AE_PRV_ISO_MIN          AE_ISO_BASE
#define AE_PRV_ISO_MAX          409600
#define AE_CAP_ISO_MIN          AE_ISO_BASE
#define AE_CAP_ISO_MAX          6400
#define AE_ISP_GAIN_TH          99200

static UINT32 base_iso = AE_PRV_ISO_MIN;
static UINT32 base_gain_ratio = 1000;


static AE_EXPECT_LUM expect_lum = {
	47, //mov
	64, //photo
	{73, 74, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
	{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
};

static AE_LA_CLAMP la_clamp = {
	// For normal mode, LA clamp value
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	// For SHDR mode, ratio value, base on Target Y to calculate clamp boundary
	{ 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125},
	{   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
};

static AE_OVER_EXPOSURE over_exposure = {
	ENABLE,
	150,
	3,
	{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
	{   2,   2,   3,   3,   3,   3,   4,   5,   6,   7,   8,   9,  10,  10,  10,  10,  10,  10,  10,  10,  10},
	{   2,   2,   3,   3,   3,   3,   4,   5,   6,   7,   8,   9,  10,  10,  10,  10,  10,  10,  10,  10,  10},
	{  10,  10,  10,  10,  10,  10,  10,  10,  10,  12,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15},
	{   3,   3,   3,   3,   3,   3,   3,   3,   3,   5,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8},
};

static AE_CONVERGENCE convergence = {
	1,
	128,
	{10, 10},
	DISABLE,   // freeze en
	10,       // freeze cyc (Ex. 10 = 1s), auto reset counter when unstable ratio not over thr)
	100,       // freeze thr (Sum of AE unstable ratio during slow shoot cyc)
	ENABLE,   // slow shoot en
	30,       // slow shoot range. In this range, slow shoot function will work
	2,         // slow shoot converge thr
	0
};

static AE_CURVE_GEN_MOVIE curve_gen_movie = {
	40,
	AE_FLICKER_50HZ, // Default value, it will auto update by UI or ISP tool
	{
		{50, 100, 0, 0},
		{20000, 200, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
	},
	2, // node_num
	AE_PRV_ISO_MAX, // IsoMax
	{ 1, 16}, // setting = (hdr ratio * 64)
	DISABLE,
	{ {3000, 2400}, {2400, 2400}, {2000, 2000}, {1500, 1500}, {1200, 1200} },
	AE_ISOMODE_MOV_PARAM,
	AE_ISP_GAIN_TH,
	AE_FLICKER_NORMAL
};

static AE_METER_WINDOW meter_window = {
	{
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 2, 2, 2, 2, 2, 2, 1,
		1, 2, 3, 3, 3, 3, 2, 1,
		1, 2, 3, 4, 4, 3, 2, 1,
		1, 2, 3, 4, 4, 3, 2, 1,
		1, 2, 3, 3, 3, 3, 2, 1,
		1, 2, 2, 2, 2, 2, 2, 1,
		1, 1, 1, 1, 1, 1, 1, 1
	}
};

static AE_LUM_GAMMA lum_gamma = {
	{
		   0,   52,  101,  146,  189,  228,  266,  301,  334,  365,
		 395,  423,  449,  474,  498,  521,  543,  564,  584,  603,
		 621,  638,  655,  671,  687,  701,  716,  729,  743,  755,
		 768,  780,  791,  802,  813,  823,  833,  843,  853,  862,
		 871,  879,  888,  896,  904,  911,  919,  926,  933,  940,
		 947,  953,  960,  966,  972,  978,  983,  989,  995, 1000,
		1005, 1010, 1015, 1020, 1023
	}
};

static AE_SHDR shdr = {
	64,
	{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
	{256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256},
	{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
	ENABLE,
	0
};

static AE_SHDR_METER shdr_meter = {
	0,
	0,
	512,
	{0,902,1161,1345,1494,1620,1731,1831,1922,2006,2084,2158,2227,2293,2356,2416,2473,2528,2581,2633,2682,2730,2777,2822,2866,2909,2951,2991,3031,3070,3108,3146,3182,3218,3253,3288,3321,3355,3387,3420,3451,3482,3513,3543,3573,3602,3631,3660,3688,3716,3743,3770,3797,3823,3849,3875,3900,3926,3950,3975,4000,4024,4048,4071,4095},
	{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
	{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
	0,
	0
};

static AE_SHDR_HBS shdr_hbs = {
	0,
	{128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}
};


static AE_IRIS_CFG dciris_cfg = {
	DISABLE,
	0, // exposure min, set 0 for auto select flicker base
	0,
	110,
	0,
	{ 3, 1, 1},
	0xFFFFFFFF,  // PWMID
	200,  // drv_max
	500,  // exposure freeze_time (ms)
	1000, // exposure unfreeze_time (ms)
	0,    // ctrl_dir_inv 
	0     // cali_out_balance
};

static AE_CURVE_GEN_PHOTO curve_gen_photo = {
	{
		{1, 100, 0, 0},
		{33333, 800, 0, 0},
		{66666, 1600, 0, 0},
		{100000, 3200, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
	},
	4, // node_num
	66666, // max cap expotime
	AE_CAP_ISO_MAX, // max cap iso
	AE_ISOMODE_PHOTO_SPEED,
	AE_ISP_GAIN_TH,
};

static AE_MANUAL manual = {
	0, 0, 0, 0
};

static AE_PARAM ae_param_default = {
	&base_iso,
	&base_gain_ratio,
	&expect_lum,
	&la_clamp,
	&over_exposure,
	&convergence,
	&curve_gen_movie,
	&meter_window,
	&lum_gamma,
	&shdr,
	&shdr_meter,
	&shdr_hbs,
	&dciris_cfg,
	&curve_gen_photo,
	&manual
};

void ae_param_get_param(ULONG *param)
{
	*param = (ULONG)(&ae_param_default);
}

