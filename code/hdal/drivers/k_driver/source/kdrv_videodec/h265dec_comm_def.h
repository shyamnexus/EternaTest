#ifndef __H265D_COMM_DEF_H__
#define __H265D_COMM_DEF_H__

#define MAX_HEVC_WIDTH				(8192)  /* NOTE: HW capability: 8192 pixels */
#define MAX_HEVC_HEIGHT				(8192)	/* NOTE: HW capability: 8192 pixels */
#define MAX_ST_RPS_SIZE				(64)
#define MAX_LT_RP_SIZE              (32)
#define MAX_NUM_REF_PICS            (16)
#define MAX_VPS_SUB_LAYER_SIZE      (7)
#define MAX_VPS_OP_SETS_PLUS1       (1024)
#define MAX_VPS_NUH_RESERVED_ZERO_LAYER_ID_PLUS1	(1)
#define MAX_SPS_SUB_LAYER_SIZE      (7)
#define MAX_SUB_LAYER_SIZE          (7)
#define MAX_SPS_NUM                 (16)
#define MAX_PPS_NUM                 (64)
#define MAX_VPS_NUM                 (16)
#define H264DEC_MAXSPS              (32)
#define H264DEC_MAXPPS              (256)
#define MAX_HEVC_RLIST_ENTRY        (16)
#define MAX_HEVC_QMAT_SIZE          (1000)
#define MaxTileCols                 (20)    /* max # of tile columns is 20 in profile Level 6.x */
#define MaxTileRows                 (22)    /* max # of tile rows is 22 in profile Level 6.x */
#define MAX_HEVC_BSDMA_ENTRY        (64)    /* CW adjust for sdk, ori is (1000) */

#define MAX_HEVC_ENGINE                (2) /* 0 : AD0,  1 : BIN0 */
#define MAX_PIC_RESOURCE_SIZE          (1)
#define MAX_HEVC_RPIC_SIZE             (16*4*4)
#define MAX_HEVC_RLIST_SIZE            (6*4)
#define MAX_HEVC_RPIC_MCINFO_SIZE      (17*4*4)
#define MAX_HEVC_USER_DATA_NUM         (8)

#define MAX_HEVC_CABAC_NUMBER	(1)
#define MAX_HEVC_FRAME_NUMBER	(32)

#ifndef min
#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef max
#define max(X,Y) ((X) > (Y) ? (X) : (Y))
#endif

#ifndef abs
#define abs(X)   (((int)(X) > 0) ? (X) : -(X))
#endif

#ifndef MAX_INT
#define MAX_INT (2147483647)
#endif

#define UNKNOWN_ORDER (-MAX_INT)

#define mysnprintf(str,size,args...)	({int myret = snprintf(str, size, args); myret = myret > (int)size ? (int)size : myret;})

#endif
