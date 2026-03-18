#ifndef _THIRD_PARTY_MODULE_LIB_H_
#define _THIRD_PARTY_MODULE_LIB_H_

#include <stdio.h>
#include <stdlib.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_ai.h"

#include "kflow_ai_net/kflow_ai_net.h"

#if defined(_BSP_NS02301_) || defined(_BSP_NA51089_) || defined(_BSP_NA51055_) || defined(_BSP_NA51102_) || defined(_BSP_NA51103_)
#define IN_CNN25_ID   (1)
#else
#define IN_CNN25_ID   (0)
#endif

#define LINE_WIDTH_M     (512)
#define MAX_MODEL_NUM_M  (16)
#define PATH_LENGTH_M    (384)

#ifndef MIN
#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(x,min,max)    (((x) > (max)) ? (max) : (((x) > (min)) ? (x) : (min)))
#endif

#define BOXAREA(x1, x2, y1, y2)  ((x2 - x1) * (y2 - y1))

// ----- network layer's pixel format ---
#define NN_PXLFMT_CLASS_MASK_M                  0xa
#define NN_PXLFMT_TYPE_MASK_M                   0x0f000000
#define NN_PXLFMT_BITS_MASK_M                   0x00ff0000
#define NN_PXLFMT_INT_MASK_M                    0x0000ff00
#define NN_PXLFMT_FRAC_MASK_M                   0x000000ff

#define NN_PXLFMT_SIGN_M(pxlfmt)                ((((pxlfmt) & NN_PXLFMT_TYPE_MASK_M) >> 24) & 0x01)
#define NN_PXLFMT_BITS_M(pxlfmt)                (((pxlfmt) & NN_PXLFMT_BITS_MASK_M) >> 16)
#define NN_PXLFMT_INT_M(pxlfmt)                 (((pxlfmt) & NN_PXLFMT_INT_MASK_M) >> 8)
#define NN_PXLFMT_FRAC_M(pxlfmt)                ((pxlfmt) & NN_PXLFMT_FRAC_MASK_M)
#define NN_PXLFMT_FMT_M(pxlfmt)                 ((pxlfmt) & (((NN_PXLFMT_CLASS_MASK_M << 28) | NN_PXLFMT_TYPE_MASK_M) | NN_PXLFMT_BITS_MASK_M))
// -------------------------------------

#define _PACKED_BEGIN
#define _PACKED_END __attribute__ ((packed))


typedef VENDOR_AI3_CFG_BUF NN_CFG_BUF_M;


typedef _PACKED_BEGIN struct tagBITMAPFILEHEADER
{
    unsigned short  bfType;
    unsigned long   bfSize;
    unsigned short  bfReserved1;
    unsigned short  bfReserved2;
    unsigned long   bfOffBits;
} _PACKED_END BITMAPFILEHEADER;

typedef _PACKED_BEGIN struct tagBITMAPINFOHEADER
{
    unsigned long   biSize;
    unsigned long   biWidth;
    unsigned long   biHeight;
    unsigned short  biPlanes;
    unsigned short  biBitCount;
    unsigned long   biCompression;
    unsigned long   biSizeImage;
    unsigned long   biXPelsPerMeter;
    unsigned long   biYPelsPerMeter;
    unsigned long   biClrUsed;
    unsigned long   biClrImportant;
} _PACKED_END BITMAPINFOHEADER;

typedef _PACKED_BEGIN struct tagRGBQUAD /* for bit count 8 and below, need a rgb quad to reduce image size */
{
    unsigned char   rgbBlue;
    unsigned char   rgbGreen;
    unsigned char   rgbRed;
    unsigned char   rgbReserved;
} _PACKED_END RGBQUAD;

typedef enum _NN_PXLFMT_AI_M {
    NN_PXLFMT_AI_NONE = 0x0,
    NN_PXLFMT_AI_SINT8 = 0xa1080000,
    NN_PXLFMT_AI_UINT8 = 0xa2080000,
    NN_PXLFMT_AI_SINT16 = 0xa1100000,
    NN_PXLFMT_AI_UINT16 = 0xa2100000,
    NN_PXLFMT_AI_SINT32 = 0xa1200000,
    NN_PXLFMT_AI_UINT32 = 0xa2200000,
    NN_PXLFMT_AI_FLOAT16 = 0xa3100000,
    NN_PXLFMT_AI_FLOAT32 = 0xa3200000,
} NN_PXLFMT_M;

/**
     ROI coordinates.

*/
typedef struct _NN_F_RECT {
	float  x1;                           ///< x coordinate of the top-left point of the rectangle
	float  y1;                           ///< y coordinate of the top-left point of the rectangle
	float  x2;                           ///< rectangle width
	float  y2;                           ///< rectangle height
} NN_F_RECT;

typedef struct _NN_I_RECT {
	signed int x1;                           ///< x coordinate of the top-left point of the rectangle
	signed int y1;                           ///< y coordinate of the top-left point of the rectangle
	signed int x2;                           ///< rectangle width
	signed int y2;                           ///< rectangle height
} NN_I_RECT;

typedef struct _NN_U_RECT {
	unsigned int x1;                           ///< x coordinate of the top-left point of the rectangle
	unsigned int y1;                           ///< y coordinate of the top-left point of the rectangle
	unsigned int x2;                           ///< rectangle width
	unsigned int y2;                           ///< rectangle height
} NN_U_RECT;

typedef struct _NN_I_POINT {
	signed int x;                           ///< x coordinate of the point
	signed int y;                           ///< y coordinate of the point
} NN_I_POINT;


/**
     CNN result integrate buffer.

*/
typedef struct _AI_RSLT_PARM {
	NN_CFG_BUF_M rslt_mem;
	unsigned long obj_num;
} AI_RSLT_PARM;


/**
     CNN result.

*/
typedef struct _MLIB_NN_RESULT
{
	signed long   category;
	float 	score;
	float 	x1;
	float 	y1;
	float 	x2;
	float 	y2;
} NN_RESULT_M;


/**
     CNN model_bin && para_txt path.

*/
typedef struct _NN_FILE_PATH{
	char model_file[PATH_LENGTH_M];
	char para_file[PATH_LENGTH_M];
} NN_FILE_PATH;


// ============================================================
// =                    Interface function                    =
// ============================================================

/**
 * @brief Split a line of file by specified delimiter.
 * 
 * @param p_string_1					: Output delimited first information.
 * @param p_string_2					: Output delimited second information.
 * @param input							: Input line information.
 * @param delimit						: Specified delimiter.
 * @return void
 */
void line_split_m_(char *p_string_1, char *p_string_2, char *input, char delimit);

/**
 * @brief Sparse a line information of file to {name: value}.
 * 
 * @param line_infos					: Output delimited information.
 * @param p_name						: Output name (char *).
 * @param p_value						: Output value (char *).
 * @return void
 */
void line_parser_m_(char *line_infos, char *p_name, char *p_value);

/**
 * @brief Load binary file to specified virtual address.
 * 
 * @param p_filename					: Absolute Path of binary file.
 * @param va							: Specified virtual address.
 * @return INT32           				: Output size of file.
 */
INT32 load_file_m_(const char *p_filename, uintptr_t va);

/**
 * @brief Load bin model to specified virtual address.
 * 
 * @param p_filename					: Absolute Path of model bin file.
 * @param va							: Specified virtual address.
 * @return HD_RESULT 					: HD_OK = Successful, Others = Failed.
 */
HD_RESULT load_model_m_(const char *filename, uintptr_t va);

/**
 * @brief Check if specified file exists.
 * 
 * @param p_filename					: Absolute Path of specified file.
 * @return HD_RESULT 					: HD_OK = Successful, Others = Failed.
 */
int chk_file_exist_m_(const char *filename);

/**
 * @brief Get size of given model.
 * 
 * @param filename						: Abosulte path of model bin file.
 * @return INT32 						: Output size of model.
 */
INT32 network_get_modelsize_m_(char *filename);

/**
 * @brief Transform fixed point data to float point data.
 * 
 * @param in_data						: Input fixed point data.
 * @param in_fmt						: Data type of input data, only support int8/uint8/int16/uint16.
 * @param out_data						: Output float point data.
 * @param out_scale_ratio				: Scale ratio of the fixed2float.
 * @param data_size						: Length of input data.
 * @return HD_RESULT 					: HD_OK = Successful, Others = Failed.
 */
HD_RESULT network_util_fixed2float_m_(VOID *in_data, HD_VIDEO_PXLFMT in_fmt, FLOAT *out_data, FLOAT out_scale_ratio, INT32 data_size, INT32 zero_point);

VOID readbmp_header_m_(UINT32 *p_height, UINT32 *p_width, UINT32 *p_bitcount, CHAR *p_file);
VOID readbmp_body_m_(UINT8 *p_data, UINT32 height, UINT32 width, UINT32 bitcount, UINT8 *p_colortable, UINT8 flipupdown, CHAR *p_file);
VOID writebmp_m_(UINT8 *p_data, UINT32 lineoffset, UINT32 height, UINT32 width, UINT32 bitcount, UINT8 *p_colortable, UINT8 flipupdown, CHAR *p_file);

/**
 * @brief Transform GFX_IMG ot VENDOR image.
 * 
 * @param out_img						: Output vendor image.
 * @param in_img 						: Iuput gfx image.
 * @param va 							: Virtual address of vendor image.
 * @return VOID.
 */
VOID network_cvt_gfx2vendor_m_(void *p_vendor_img, HD_GFX_IMG_BUF *in_img, uintptr_t va);

/**
 * @brief Crop image.
 * 
 * @param dst_img						: Output cropped image.
 * @param src_img 						: Iuput source image.
 * @param method 						: Crop method.
 * @param roi 							: Region of interest in source image to crop.
 * @return HD_RESULT 					: HD_OK = Successful, Others = Failed.
 */
HD_RESULT network_crop_img_m_(HD_GFX_IMG_BUF *dst_img, void *p_vendor_img, HD_GFX_SCALE_QUALITY method, NN_F_RECT *roi);

/**
 * @brief Get memory from Pre-allocated buffer space.
 * 
 * @param buf							: Pre-allocated source buffer space.
 * @param req_size 						: Required memory size to allocate.
 * @param align_size 					: Align size of required memory size.
 * @param align_addr 					: Align size of required memory address.
 * @return NN_CFG_BUF_M 	: Output allocated memory.
 */
NN_CFG_BUF_M sample_getmem_m_(NN_CFG_BUF_M *buf, UINT32 req_size, UINT32 align_size, BOOL align_addr);

/**
 * @brief Get memory to specified address from Pre-allocated buffer space.
 * 
 * @param buf							: Pre-allocated source buffer space.
 * @param req_mem 						: Output specified target memory.
 * @param req_size 						: Required memory size to allocate.
 * @param align_size 					: Align size of required memory size.
 * @param align_addr 					: Align size of required memory address.
 * @return HD_RESULT 					: HD_OK = Successful, Others = Failed.
 */
HD_RESULT sample_get_mem_m_(NN_CFG_BUF_M *buf, NN_CFG_BUF_M *req_mem, UINT32 req_size, UINT32 align_size, BOOL align_addr);

/**
 * @brief Transform memory's physical address to virtual address.
 * 
 * @param base_buf						: Memory's base address.
 * @param pa 							: Input physical address.
 * @return uintptr_t 					: Output virtual address.
 */
uintptr_t pa2va_m_(NN_CFG_BUF_M base_buf, uintptr_t pa);

/**
 * @brief Transform memory's virtual address to physical address.
 * 
 * @param base_buf						: Memory's base address.
 * @param pa 							: Input virtual address.
 * @return uintptr_t 					: Output physical address.
 */
uintptr_t va2pa_m_(NN_CFG_BUF_M base_buf, uintptr_t va);

/**
 * @brief Fast exp function.
 * 
 * @param x								: Input float value.
 * @return float 						: Output float value.
 */
float fastexp_m_(float x);

/**
 * @brief Quick sort function.
 * 
 * @param objs							: Input detection bounding box information to process.
 * @param left							: Left index.
 * @param right							: Right index.
 * @return VOID
 */
VOID network_quick_sort_m_(NN_RESULT_M *objs, INT32 left, INT32 right);

/**
 * @brief Check if overlap between bounding boxes.
 * 
 * @param p_box1						: Input detection bounding box one.
 * @param p_box2						: Input detection bounding box two.
 * @param ratio							: Overlap ratio threshold.
 * @param method						: Check method [1|2|3].
 * @return BOOL							: TRUE = overlaped, FALSE: not overlap.
 */
BOOL network_check_overlap_m_(NN_RESULT_M *p_box1, NN_RESULT_M *p_box2, FLOAT ratio, INT32 method);

/**
 * @brief Perform non-maximum suppression.
 * 
 * @param p_boxes						: Input detection bounding box information.
 * @param num							: Input detection bounding box number.
 * @param ratio							: nms threshold.
 * @param method						: check method [1|2|3].
 * @param after_num						: Maximum output bounding box number.
 * @return UINT32						: Output bounding box number.
 */
UINT32 network_nonmax_suppress_m_(NN_RESULT_M *p_boxes, INT32 num, FLOAT ratio, INT32 method, INT32 after_num);

/**
 * @brief Cutoff bounding box's coordinates that exceed boundaries.
 * 
 * @param objs_info						: Input detection bounding box information.
 * @param objs_num						: Input detection bounding box number.
 * @param p_coord						: Boundaries.
 * @return VOID
 */
VOID detection_cutoff_boundary_m_(NN_RESULT_M *objs_info, INT32 objs_num, NN_F_RECT* p_coord);


#endif

