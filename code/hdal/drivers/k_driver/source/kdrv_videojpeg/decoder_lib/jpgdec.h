#ifndef _JPGDEC_H_
#define _JPGDEC_H_

//#include <stdio.h>

#define MEM_IO              3   // 0: read .jpg file and write bmp file; 1: read .jpg buffer and output YUV420 buffer; 2: read .jpg buffer and output HW YUV420 (YUV_SP8) buffer; 3: read .jpg buffer and output HW YUV420 (YUV_SP) buffer 
#define OUTPUT_JPEG_INFO    0   // 1: output JPEG related information
#define SILENCE             0   // don't print any message
#define NVT_CMYK_FORMULA	1
#define PADDING_BLACK		1

#define JPG_ATRIB_BASELINE      0x00000001
#define JPG_ATRIB_PROGRESS      0x00000002
#define JPG_ATRIB_COMPONENT     0x0000000C
#define JPG_ATRIB_SFACTOR_H     0x00000030
#define JPG_ATRIB_SFACTOR_V     0x000000C0

#define JD_CEIL(a,b)    ((((a)+(b)-1))/(b))

typedef struct JpgDec_s
{
    #if (MEM_IO == 0)
    FILE *pInFile;              // [in] input file handle (JPEG)
    FILE *pOutFile;             // [in] output file handle (BMP)
    #endif
    unsigned char *pInBuf;      // [in]  input buffer with JPEG data   
    unsigned char *pOutBuf;     // [in]  output buffer with YUV data
    unsigned char *pTmpBuf;     // [in]  temporary buffer for progressive-JPEG
    int inFileSize;             // [in]  input JPEG size (bytes)
    unsigned int widthImg;      // [out] image display width
    unsigned int heightImg;     // [out] image display height
    unsigned int widthBuf;      // [out] image buffer width
    unsigned int heightBuf;     // [out] image buffer height
    unsigned int flags;         // [out] JPEG attributes
    void *pVoid;                // [out] library private working space
    char *version;              // [out] library version
    #if OUTPUT_JPEG_INFO
    char *jpgInfo;              // [out] JPEG format information
    #endif
	unsigned int uv_swap;		// [in] 0: CbCr, 1: CrCb
} JpgDec_t;

#endif /* _JPGDEC_H_ */
