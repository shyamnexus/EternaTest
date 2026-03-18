#ifndef _CTL_SEN_IF_INT_H_
#define _CTL_SEN_IF_INT_H_

#include "ctl_sen_int.h"
#include "ctl_sen_utility.h"

/*
	csi operation API
*/
extern INT32 ctl_sen_csi_open(void *hdl);
extern INT32 ctl_sen_csi_close(void *hdl);
extern INT32 ctl_sen_csi_start(void *hdl);
extern INT32 ctl_sen_csi_stop(void *hdl);
extern INT32 ctl_sen_csi_setmode(void *hdl);
extern INT32 ctl_sen_csi_set(void *hdl, UINT32 cfg_id, void *value);	// pls used KDRV_SSENIF_PARAM_ID as cfg_id input
extern INT32 ctl_sen_csi_get(void *hdl, UINT32 cfg_id, void *value);	// pls used KDRV_SSENIF_PARAM_ID as cfg_id input
extern CTL_SEN_INTE ctl_sen_csi_wait_interrupt(void *hdl, CTL_SEN_INTE sen_inte);
extern INT32 ctl_sen_csi_force_dis(CTL_SEN_MAP_HDL *map_hdl, BOOL force_dis);

/*
	lvds operation API
*/
extern INT32 ctl_sen_lvds_open(void *hdl);
extern INT32 ctl_sen_lvds_close(void *hdl);
extern INT32 ctl_sen_lvds_start(void *hdl);
extern INT32 ctl_sen_lvds_stop(void *hdl);
extern INT32 ctl_sen_lvds_setmode(void *hdl);
extern INT32 ctl_sen_lvds_set(void *hdl, UINT32 cfg_id, void *value);	// pls used KDRV_SSENIF_PARAM_ID as cfg_id input
extern INT32 ctl_sen_lvds_get(void *hdl, UINT32 cfg_id, void *value);	// pls used KDRV_SSENIF_PARAM_ID as cfg_id input
extern CTL_SEN_INTE ctl_sen_lvds_wait_interrupt(void *hdl, CTL_SEN_INTE sen_inte);
extern INT32 ctl_sen_lvds_force_dis(CTL_SEN_MAP_HDL *map_hdl, BOOL force_dis);

/*
	tge operation API
*/
extern INT32 ctl_sen_tge_open(void *hdl);
extern INT32 ctl_sen_tge_close(void *hdl);
extern INT32 ctl_sen_tge_start(void *hdl);
extern INT32 ctl_sen_tge_stop(void *hdl);
extern INT32 ctl_sen_tge_setmode(void *hdl);
extern INT32 ctl_sen_tge_set(void *hdl, UINT32 cfg_id, void *value);	// pls used KDRV_TGE_PARAM_ID as cfg_id input
extern INT32 ctl_sen_tge_get(void *hdl, UINT32 cfg_id, void *value);	// pls used KDRV_TGE_PARAM_ID as cfg_id input

/*
	slvsec operation API
*/
extern INT32 ctl_sen_slvsec_open(void *hdl);
extern INT32 ctl_sen_slvsec_close(void *hdl);
extern INT32 ctl_sen_slvsec_start(void *hdl);
extern INT32 ctl_sen_slvsec_stop(void *hdl);
extern INT32 ctl_sen_slvsec_setmode(void *hdl);
extern INT32 ctl_sen_slvsec_set(void *hdl, UINT32 cfg_id, void *value);
extern INT32 ctl_sen_slvsec_get(void *hdl, UINT32 cfg_id, void *value);
extern CTL_SEN_INTE ctl_sen_slvsec_wait_interrupt(void *hdl, CTL_SEN_INTE sen_inte);
extern INT32 ctl_sen_slvsec_force_dis(CTL_SEN_MAP_HDL *map_hdl, BOOL force_dis);

#endif //_CTL_SEN_IF_INT_H_
