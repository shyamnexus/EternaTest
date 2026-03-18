#include "h26x.h"
#include "h26xenc_api.h"
#include "h26xenc_wrap.h"
#include "h26x_bitstream.h"
#include "h264enc_api.h"
#include "h265enc_api.h"
#include "h26xenc_rc.h"
#include "nvt_vdocdc_platform.h"

#if defined(__LINUX)
EXPORT_SYMBOL(h26x_getHwRegSize);
EXPORT_SYMBOL(h26x_setNextBsDmaBuf);
EXPORT_SYMBOL(h26x_resetHW);
EXPORT_SYMBOL(h26x_open);
EXPORT_SYMBOL(h26x_close);
EXPORT_SYMBOL(h26x_setBsLen);
EXPORT_SYMBOL(h26x_setBSDMA);
EXPORT_SYMBOL(h26x_setBSDMA2);
EXPORT_SYMBOL(h26x_flushCache);
EXPORT_SYMBOL(h26x_cache_clean);
EXPORT_SYMBOL(h26x_cache_invalidate);
EXPORT_SYMBOL(h26x_getPhyAddr);
EXPORT_SYMBOL(h26x_waitINT);
EXPORT_SYMBOL(h26x_start);
EXPORT_SYMBOL(h26x_start_disdma);
EXPORT_SYMBOL(h26x_exit_isr);
EXPORT_SYMBOL(h26x_reset);
EXPORT_SYMBOL(h26x_setIntEn);
EXPORT_SYMBOL(h26x_getIntEn);
EXPORT_SYMBOL(h26x_setEncDirectRegSet);
EXPORT_SYMBOL(h26x_getBusyStatus);
EXPORT_SYMBOL(h26x_setEncLLRegSet);
//EXPORT_SYMBOL(h26x_wp_cb_init);
EXPORT_SYMBOL(h26x_prtReg);
EXPORT_SYMBOL(h26x_prtMem);
EXPORT_SYMBOL(h26x_getDebug);
EXPORT_SYMBOL(h26x_module_reset);
EXPORT_SYMBOL(h26x_setDmaChDis);
EXPORT_SYMBOL(h26x_getClk);
EXPORT_SYMBOL(h26x_get_utilization);
EXPORT_SYMBOL(h26x_create_resource);
EXPORT_SYMBOL(h26x_release_resource);
EXPORT_SYMBOL(h26x_setUnitChecksum);
EXPORT_SYMBOL(h26x_setUnLock);
EXPORT_SYMBOL(h26x_getDbg1);

EXPORT_SYMBOL(h26xEnc_getGdrCfg);
EXPORT_SYMBOL(h26xEnc_wrapJndCfg);
EXPORT_SYMBOL(h26XEnc_setAqCfg);
EXPORT_SYMBOL(h26xEnc_getAqCfg);
EXPORT_SYMBOL(h26xEnc_getIspRatioCfg);
EXPORT_SYMBOL(h26xEnc_getRowRcCfg);
EXPORT_SYMBOL(h26XEnc_setRcInit);
EXPORT_SYMBOL(h26XEnc_setRoiCfg);
EXPORT_SYMBOL(h26XEnc_setOsgPalCfg);
//EXPORT_SYMBOL(h26XEnc_setMotAddrCfg);
EXPORT_SYMBOL(h26XEnc_setMdInfoCfg);
#if H26X_UQPMAP_FROM_MD
EXPORT_SYMBOL(h26XEnc_setBGRdoCfg);
EXPORT_SYMBOL(h26XEnc_setUsrQPMap);
#endif
EXPORT_SYMBOL(h26XEnc_setUsrQpCfg);
EXPORT_SYMBOL(h26xEnc_getVaAPBAddr);
EXPORT_SYMBOL(h26XEnc_setRowRcCfg);
EXPORT_SYMBOL(h26XEnc_setOsgWinCfg);
EXPORT_SYMBOL(h26XEnc_setIspCbCfg);
EXPORT_SYMBOL(h26XEnc_setSliceSplitCfg);
EXPORT_SYMBOL(h26XEnc_setOsgRgbCfg);
EXPORT_SYMBOL(h26XEnc_setOsgGlobalCfg);
EXPORT_SYMBOL(h26XEnc_setGdrCfg);
EXPORT_SYMBOL(h26xEnc_getBsLen);
EXPORT_SYMBOL(h26XEnc_setJndCfg);
EXPORT_SYMBOL(h26xEnc_setResSupCfg);
EXPORT_SYMBOL(h26xEnc_getNaluLenResult);
#if H26X_SET_PROC_PARAM
EXPORT_SYMBOL(h26xEnc_getRCDumpLog);
EXPORT_SYMBOL(h26xEnc_setRCDumpLog);
#endif

#if LPM_PROC_ENABLE
EXPORT_SYMBOL(h26XEnc_getLpmCfg);
EXPORT_SYMBOL(h26xEnc_getInterOnlyCfg);
#endif

EXPORT_SYMBOL(h26xEnc_getNDQPStep);
EXPORT_SYMBOL(h26xEnc_setNDQPStep);
EXPORT_SYMBOL(h26xEnc_getNDQPRange);
EXPORT_SYMBOL(h26xEnc_setNDQPRange);
EXPORT_SYMBOL(h26xEnc_getRRCSyncQPCond);
EXPORT_SYMBOL(h26xEnc_setRRCSyncQPCond);
#if H26X_DIS_OSG_CACHE_OPERATION
EXPORT_SYMBOL(h26xEnc_getOsgCacheFlush);
EXPORT_SYMBOL(h26xEnc_setOsgCacheFlush);
#endif
EXPORT_SYMBOL(h26xEnc_getESMVTh);
EXPORT_SYMBOL(h26xEnc_setESMVTh);
EXPORT_SYMBOL(h264Enc_setFroSync);
EXPORT_SYMBOL(h264Enc_getFroSync);
#if H264_SET_COEF_COST_THD
EXPORT_SYMBOL(h264Enc_setYCoefCostTh);
EXPORT_SYMBOL(h264Enc_getYCoefCostTh);
EXPORT_SYMBOL(h264Enc_setCCoefCostTh);
EXPORT_SYMBOL(h264Enc_getCCoefCostTh);
#endif
#if H26X_SET_MB_LIMIT_QP_IND
EXPORT_SYMBOL(h26xEnc_getIMinDeltaMBQP);
EXPORT_SYMBOL(h26xEnc_setIMinDeltaMBQP);
EXPORT_SYMBOL(h26xEnc_getIMaxDeltaMBQP);
EXPORT_SYMBOL(h26xEnc_setIMaxDeltaMBQP);
EXPORT_SYMBOL(h26xEnc_getPMinDeltaMBQP);
EXPORT_SYMBOL(h26xEnc_setPMinDeltaMBQP);
EXPORT_SYMBOL(h26xEnc_getPMaxDeltaMBQP);
EXPORT_SYMBOL(h26xEnc_setPMaxDeltaMBQP);
#endif

EXPORT_SYMBOL(h26xEnc_getVersion);
EXPORT_SYMBOL(h26xEnc_getVaLLCAddr);

EXPORT_SYMBOL(h26xEnc_wrapAqCfg);
EXPORT_SYMBOL(h26xEnc_wrapVarCfg);
EXPORT_SYMBOL(h26xEnc_wrapRndCfg);
EXPORT_SYMBOL(h26xEnc_wrapMotAqCfg);
EXPORT_SYMBOL(h26xEnc_wrapLpmCfg);
EXPORT_SYMBOL(h26xEnc_wrapQpRelatedCfg);
EXPORT_SYMBOL(h26XEnc_setSpnCfg);
EXPORT_SYMBOL(h26xEnc_setMotDetectCfg);
EXPORT_SYMBOL(h26xEnc_setMotFilterCfg);

//EXPORT_SYMBOL(h26xEnc_wrapBgrCfg);
//EXPORT_SYMBOL(h26xEnc_wrapRmdCfg);
//EXPORT_SYMBOL(h26xEnc_wrapTnrCfg);
//EXPORT_SYMBOL(h26xEnc_wrapLambdaCfg);
EXPORT_SYMBOL(h26xEnc_queryRecFrmNum);
EXPORT_SYMBOL(get_bits);
EXPORT_SYMBOL(init_parse_bitstream);
EXPORT_SYMBOL(read_rbsp_trailing_bits);
EXPORT_SYMBOL(read_uvlc_codeword);
EXPORT_SYMBOL(read_signed_uvlc_codeword);

EXPORT_SYMBOL(SetMemoryAddr);
EXPORT_SYMBOL(ebspTorbsp);
EXPORT_SYMBOL(save_to_reg);
EXPORT_SYMBOL(h26x_setLLCmd);

EXPORT_SYMBOL(h264Enc_InitRdoCfg);
EXPORT_SYMBOL(h264Enc_setRdoCfg);
EXPORT_SYMBOL(h264Enc_setBGRdoCfg);
EXPORT_SYMBOL(h264Enc_setFroCfg);
EXPORT_SYMBOL(h264Enc_queryMemSize);

EXPORT_SYMBOL(h264Enc_getNxtPicType);
EXPORT_SYMBOL(h264Enc_setNxtPicType);
EXPORT_SYMBOL(h264Enc_getGopNum);
EXPORT_SYMBOL(h264Enc_setGopNum);
EXPORT_SYMBOL(h264Enc_getPocNum);
EXPORT_SYMBOL(h264Enc_getFrmSkipFlag);
EXPORT_SYMBOL(h264Enc_setFrmSkipFlag);

EXPORT_SYMBOL(h264Enc_prepareOnePicture);
EXPORT_SYMBOL(h264Enc_getResult);
EXPORT_SYMBOL(h264Enc_initEncoder);
EXPORT_SYMBOL(h264Enc_getSeqHdr);
EXPORT_SYMBOL(h264Enc_getRdoCfg);
EXPORT_SYMBOL(h264Enc_queryRecFrmSize);

#if H26X_SET_PROC_PARAM
EXPORT_SYMBOL(h264Enc_getRowRCStopFactor);
EXPORT_SYMBOL(h264Enc_setRowRCStopFactor);
EXPORT_SYMBOL(h264Enc_getPReduce16Planar);
EXPORT_SYMBOL(h264Enc_setPReduce16Planar);
#endif
EXPORT_SYMBOL(h265Enc_setFroSync);
EXPORT_SYMBOL(h265Enc_getFroSync);
EXPORT_SYMBOL(h264Enc_setFrameNumGapAllow);
EXPORT_SYMBOL(h264Enc_setFixSPSLog2Poc);
EXPORT_SYMBOL(h264Enc_getEncodeRatio);

EXPORT_SYMBOL(h265Enc_getSeqHdr);
EXPORT_SYMBOL(h265Enc_queryMemSize);

EXPORT_SYMBOL(h265Enc_getNxtPicType);
EXPORT_SYMBOL(h265Enc_setNxtPicType);
EXPORT_SYMBOL(h265Enc_setGopNum);
EXPORT_SYMBOL(h265Enc_getGopNum);
EXPORT_SYMBOL(h265Enc_getPocNum);
EXPORT_SYMBOL(h265Enc_getFrmSkipFlag);
EXPORT_SYMBOL(h265Enc_setFrmSkipFlag);

EXPORT_SYMBOL(h265Enc_prepareOnePicture);
EXPORT_SYMBOL(h265Enc_initEncoder);
EXPORT_SYMBOL(h265Enc_setFroCfg);
EXPORT_SYMBOL(h265Enc_getResult);
EXPORT_SYMBOL(h265Enc_InitRdoCfg);
EXPORT_SYMBOL(h265Enc_setRdoCfg);
EXPORT_SYMBOL(h265Enc_setBGRdoCfg);
EXPORT_SYMBOL(h265Enc_getRdoCfg);
EXPORT_SYMBOL(h265Enc_queryRecFrmSize);
EXPORT_SYMBOL(h265Enc_getEncodeRatio);

#if H26X_SET_PROC_PARAM
EXPORT_SYMBOL(h265Enc_getRowRCStopFactor);
EXPORT_SYMBOL(h265Enc_setRowRCStopFactor);
#endif
EXPORT_SYMBOL(h26x_getDummyWTSize);
EXPORT_SYMBOL(rc_cb_init);
#if H26X_MEM_USAGE
EXPORT_SYMBOL(h26xEnc_getMemUsage);
#endif
EXPORT_SYMBOL(h265Enc_setLongStartCode);
EXPORT_SYMBOL(h26xEnc_setMaskWinCfg);
EXPORT_SYMBOL(h26xEnc_setMaskInitCfg);

EXPORT_SYMBOL(bit_reverse);
EXPORT_SYMBOL(h26XEnc_setMotAddrCfg);
EXPORT_SYMBOL(h26XEnc_setSdecCfg);
EXPORT_SYMBOL(h26x_getVirAddr);
EXPORT_SYMBOL(h26x_getNalLen);
EXPORT_SYMBOL(h26x_getStableSliceNum);
EXPORT_SYMBOL(h26x_getStableLen);
EXPORT_SYMBOL(h264Enc_setSEIBsChksumen);
EXPORT_SYMBOL(h265Enc_setSEIBsChksumen);
#if H26X_USE_DIFF_MAQ
EXPORT_SYMBOL(h26XEnc_setMaqDiffBitmap);
EXPORT_SYMBOL(h26XEnc_setMAQDiffInfoCfg);
EXPORT_SYMBOL(h26XEnc_getMAQDiffInfoCfg);
#endif
EXPORT_SYMBOL(h26xEnc_getMDMode);
EXPORT_SYMBOL(h26xEnc_setMDMode);

#if SUPPORT_BUILTIN_RC
	#if H26X_RC_LOG_VERSION
	EXPORT_SYMBOL(h26xEnc_getRcVersion);
	#endif
	#if H26X_LOG_RC_INIT_INFO
	EXPORT_SYMBOL(h26xEnc_RcGetInitInfo);
	#endif
	#if H26X_RC_BR_MEASURE
	EXPORT_SYMBOL(h26xEnc_RcSetDumpBR);
	#endif
	#if H26X_RC_DBG_LEVEL
	EXPORT_SYMBOL(h26xEnc_RcSetDbgLv);
	EXPORT_SYMBOL(h26xEnc_RcGetDbgLv);
	#endif
	#if H26X_RC_DBG_CMD
	EXPORT_SYMBOL(h26xEnc_RcGetCmd);
	EXPORT_SYMBOL(h26xEnc_RcSetCmd);
	#endif
	#if SUPPORT_CVBR
	EXPORT_SYMBOL(h26xEnc_RcGetCVBRInfo);
	#endif
#endif
EXPORT_SYMBOL(h26xEnc_setRCSyncMaqStr);

EXPORT_SYMBOL(h26x_reset2);
EXPORT_SYMBOL(h26x_setChkSumEn);
EXPORT_SYMBOL(h26XEnc_setLpmCfg);
EXPORT_SYMBOL(h26XEnc_setUserDataCfg);
EXPORT_SYMBOL(h26x_getCurJobNum);
EXPORT_SYMBOL(h26xEnc_queryMinBSBufSize);
EXPORT_SYMBOL(h26xEnc_getMinBSRatio);
EXPORT_SYMBOL(h26xEnc_getEncInfo);
EXPORT_SYMBOL(h26x_checkINT);
EXPORT_SYMBOL(h26x_polling_reset);
//EXPORT_SYMBOL(h26xEnc_setQpRelatedCfg);
EXPORT_SYMBOL(h26XEnc_setVarCfg);
EXPORT_SYMBOL(h265Enc_setQpRelatedCfg);
EXPORT_SYMBOL(h26XEnc_setScdCfg);
EXPORT_SYMBOL(h26XEnc_setMotAqCfg);
EXPORT_SYMBOL(h26XEnc_setPSNRCfg);
EXPORT_SYMBOL(h26XEnc_setRndCfg);

EXPORT_SYMBOL(h26x_setNextBsBuf);
EXPORT_SYMBOL(h26xEnc_setWaterMarkCfg);
EXPORT_SYMBOL(h26x_setSliceSetEn);
EXPORT_SYMBOL(h26x_setSliceSetCtrlEn);
EXPORT_SYMBOL(h26x_getSliceBsLenwoDummy);
EXPORT_SYMBOL(h26x_getSliceBsLenwiDummy);
EXPORT_SYMBOL(h26xEnc_getLastSliceForSliceLowLatency);
EXPORT_SYMBOL(h26xEnc_getJndCfg);
EXPORT_SYMBOL(h26xEnc_getMotDetectCfg);
EXPORT_SYMBOL(h26xEnc_getMotFilterCfg);
EXPORT_SYMBOL(h26xEnc_getResSupCfg);
EXPORT_SYMBOL(h26XEnc_setIspParamCfg);
#if H26X_SUPPORT_SCENE_DETECT
EXPORT_SYMBOL(h26XEnc_SceneTrigger);
EXPORT_SYMBOL(h26XEnc_setSceneDetCfg);
EXPORT_SYMBOL(h26XEnc_getSceneDetCfg);
#endif
#if H26X_SUPPORT_SMART_ROI
EXPORT_SYMBOL(h26xEnc_setSmartRoiCfg);
EXPORT_SYMBOL(h26xEnc_setSmartBBoxCfg);
EXPORT_SYMBOL(h26XEnc_setSmartRoi2UserQP);
#endif
#if H26X_SUPPORT_DYNAMIC_FR
EXPORT_SYMBOL(h26xEnc_setDynFRCfg);
EXPORT_SYMBOL(h26xEnc_setDynFRProcCfg);
EXPORT_SYMBOL(h26xEnc_setDynFRParam);
EXPORT_SYMBOL(h26xEnc_SkipPframe);
#endif
#if H26X_SUPPORT_DYNAMIC_GOP
EXPORT_SYMBOL(h26xEnc_InsertIframe);
EXPORT_SYMBOL(h26xEnc_setDynGopCfg);
EXPORT_SYMBOL(h26xEnc_setDynGopProcCfg);
EXPORT_SYMBOL(h26xEnc_setDynGopParam);
#endif
#if H264_AEFIFO_RRC_BIAS
EXPORT_SYMBOL(h26xEnc_set264TBias);
EXPORT_SYMBOL(h26xEnc_get264TBias);
#endif

EXPORT_SYMBOL(h26x_setPatchCount);
EXPORT_SYMBOL(h26x_getPatchCount);

EXPORT_SYMBOL(h26xEnc_setSEISignedVdo);
EXPORT_SYMBOL(h26xEnc_getSEISignedVdo);
EXPORT_SYMBOL(h26xEnc_setInterOnlyCfg);
EXPORT_SYMBOL(h26XEnc_switchLpmSetByMdMode);

// platform //
EXPORT_SYMBOL(nvt_vdocdc_vmalloc);
EXPORT_SYMBOL(nvt_vdocdc_vfree);
#endif	//  defined(__LINUX)
