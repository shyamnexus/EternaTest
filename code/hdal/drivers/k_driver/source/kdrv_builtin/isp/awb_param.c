
#include "kwrap/type.h"
#include "awb_param.h"
#include "isp_builtin_int.h"
#include "awb_builtin_alg.h"

// NOTE: From os02k10

/******************************/
/**
	AWB Method1 table
*/
static AWB_TH awb_th = {
	   5, //y_l
	 240, //y_u
	  58, //rpb_l
	 240, //rpb_u
	 -53, //rsb_l
	 118, //rsb_u
	  38, //r2g_l
	 150, //r2g_u
	  19, //b2g_l
	  90, //b2g_u
	  20, //rmb_l
	  41, //rmb_u
};

/******************************/
/**
	LV check
*/
static AWB_LV awb_lv = {
	 1,    4, //LV1~LV4
	 6,   10, //LV6~LV10
	14,   26, //LV14~LV26
};

/******************************/
/**
	CT Weight
*/
static AWB_CT_WEIGHT awb_ct_weight = {
	{ 2300,  2800,  3700,  4700,  6500, 11000},
	{  806,   852,   923,   988,  1077,  1144},
	{    1,     1,     1,     1,     1,     1},
	{    1,     1,     1,     1,     1,     1},
	{    1,     1,     1,     1,     1,     1}
};

/******************************/
/**
	AWB target
*/
static AWB_TARGET awb_target = {
	//2300 2800  3700  4700  6500 11000
	{ 803,  840,  907,  975, 1070, 1141}, //cx
	{1024, 1024, 1024, 1024, 1024, 1024}, //rgratio
	{1024, 1024, 1024, 1024, 1024, 1024}, //bgratio
};

/******************************/
/**
	AWB CT table
	The color temperature must be steeled sort
*/
static AWB_CT_INFO awb_ct_info = {
	{              2300,               2800,               3700,               4700,               6500,              11000},
	{AWB_CT_2300K_RGAIN, AWB_CT_2800K_RGAIN, AWB_CT_3700K_RGAIN, AWB_CT_4700K_RGAIN, AWB_CT_6500K_RGAIN, AWB_CT_11000K_RGAIN},
	{AWB_CT_2300K_GGAIN, AWB_CT_2800K_GGAIN, AWB_CT_3700K_GGAIN, AWB_CT_4700K_GGAIN, AWB_CT_6500K_GGAIN, AWB_CT_11000K_GGAIN},
	{AWB_CT_2300K_BGAIN, AWB_CT_2800K_BGAIN, AWB_CT_3700K_BGAIN, AWB_CT_4700K_BGAIN, AWB_CT_6500K_BGAIN, AWB_CT_11000K_BGAIN},
};

/******************************/
/**
	AWB Manual table
*/
static AWB_MWB_GAIN awb_mwb_gain = {
	//UNIT,  AWB_MODE_DAYLIGHT,          AWB_MODE_CLOUY,  AWB_MODE_TUNGSTEN,    AWB_MODE_SUNSET, AWB_SCENE_CUSTOMER1, AWB_SCENE_CUSTOMER2, AWB_SCENE_CUSTOMER3, AWB_SCENE_CUSTOMER4, AWB_SCENE_CUSTOMER5, AWB_SCENE_NIGHT_MODE, AWB_SCENE_MGAIN,
	{ 256,  AWB_CT_6500K_RGAIN,      AWB_CT_6500K_RGAIN, AWB_CT_2300K_RGAIN, AWB_CT_2800K_RGAIN,                 256,                 256,                 256,                 256,                 256,                  256,             256},
	{ 256,                 256,                     256,                256,                256,                 256,                 256,                 256,                 256,                 256,                  256,             256},
	{ 256,  AWB_CT_6500K_BGAIN, AWB_CT_6500K_BGAIN*0.85, AWB_CT_2300K_BGAIN, AWB_CT_2800K_BGAIN,                 256,                 256,                 256,                 256,                 256,                  256,             256},
};

static AWB_CONVERGE awb_converge = {
	4, //skip frame
	50, //speed
	259, //tolerance 259/256=1%
};

static AWB_EXPAND_BLOCK awb_expand_block = {
	{     AWBALG_EXPAND_TH_OFF,     AWBALG_EXPAND_TH_OFF,     AWBALG_EXPAND_TH_OFF,     AWBALG_EXPAND_TH_OFF}, //mode
	{                       6,                        6,                        6,                        6}, //lv_l
	{                      10,                       10,                       10,                       10}, //lv_h
	{                       5,                        5,                        5,                        5}, //y_l
	{                     245,                      245,                      245,                      245}, //y_u
	{                     115,                      105,                       91,                      114}, //rpb_l
	{                     141,                      122,                      105,                      128}, //rpb_u
	{                      14,                       56,                       18,                       32}, //rsb_l
	{                      40,                       74,                       32,                      -53}, //rsb_u
};

static AWB_LUMA_WEIGHT awb_luma_weight = {
	FALSE,
	{   4,    8,   20,   32,  192,  208,  224,  240},
	{   1,    1,    1,    1,    1,    1,    1,    1}
};

static AWB_PARAM awb_param_default = {
	&awb_th,
	&awb_lv,
	&awb_ct_weight,
	&awb_target,
	&awb_ct_info,
	&awb_mwb_gain,
	&awb_converge,
	&awb_expand_block,
	&awb_luma_weight,
};

void awb_param_get_param(ULONG *param)
{
	*param = (ULONG)(&awb_param_default);
}

