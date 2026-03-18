/*
* djpeg.c
*
* Copyright (C) 1991-1997, Thomas G. Lane.
* Modified 2009 by Guido Vollbeding.
* This file is part of the Independent JPEG Group's software.
* For conditions of distribution and use, see the accompanying README file.
*
* This file contains a command-line user interface for the JPEG decompressor.
* It should work on any system with Unix- or MS-DOS-style command lines.
*
* Two different command line styles are permitted, depending on the
* compile-time switch TWO_FILE_COMMANDLINE:
*	djpeg [options]  inputfile outputfile
*	djpeg [options]  [inputfile]
* In the second style, output is always to standard output, which you'd
* normally redirect to a file or pipe to some other program.  Input is
* either from a named file or from standard input (typically redirected).
* The second style is convenient on Unix but is unhelpful on systems that
* don't support pipes.  Also, you MUST use the first style if your system
* doesn't do binary I/O to stdin/stdout.
* To simplify script writing, the "-outfile" switch is provided.  The syntax
*	djpeg [options]  -outfile outputfile  inputfile
* works regardless of which command line style is used.
1.0.1: add error handling (exit)
1.0.2: add error handling (exit)
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/blkdev.h>

#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		/* for version message */
#include "jpeg_api.h"

//#include <linux/ctype.h>		/* to declare isprint() */

//#include "jpeg_dbg.h"
#include "jpgdec.h"     //chlin

//#include <linux/kernel.h>
#if defined(__aarch64__)
//#include <linux/soc/nvt/fmem.h>
#else
//#include <mach/fmem.h>
#endif

// 1.1.0: first version
// 1.2.0: add jpgdecTranTables, generate optimized Huffman tables from one JPEG image
// 1.3.0: add output YUV format YUV_SP
// 1.3.1: Fix progressive YUV400 color error
// 1.5.0: Add RGB/YCCK/CMYK to YCbCr conversion (2022/12/09)
// 1.5.1: insufficient data stuff zeros (2023/02/17)
// 1.5.2: prevent user data include 0xff (2024/08/08)
// 1.5.3: support padding black (2024/08/21)
#define JPEG_DEC_VERSION	"1.5.3"
static char version[] = "Novatek JPEG library version number: 1.5.3 \0";
#if OUTPUT_JPEG_INFO
static char jpginfo[1024] = "\0";
#endif

/* Create the add-on message string table. */

static const char *const cdjpeg_message_table[] = {
#include "cderror.h"
	NULL
};


typedef struct JpgPrivate_s {
	struct jpeg_decompress_struct *pcinfo;
	struct jpeg_error_mgr *pjerr;
} JpgPrivate_t;

char *get_jpg_sw_dec_version(void)
{
	return version;
}

/*
* Marker processor for COM and interesting APPn markers.
* This replaces the library's built-in processor, which just skips the marker.
* We want to print out the marker as text, to the extent possible.
* Note this code relies on a non-suspending data source.
*/

#if (MEM_IO == 0)
LOCAL(unsigned int)
jpeg_getc(j_decompress_ptr cinfo)
/* Read next byte */
{
	struct jpeg_source_mgr *datasrc = cinfo->src;

	if (datasrc->bytes_in_buffer == 0) {
		if (!(*datasrc->fill_input_buffer)(cinfo))
			ERREXIT(cinfo, JERR_CANT_SUSPEND);
	}
	datasrc->bytes_in_buffer--;
	return GETJOCTET(*datasrc->next_input_byte++);
}
#endif


METHODDEF(boolean)
print_text_marker(j_decompress_ptr cinfo)
{
#if (MEM_IO != 0)
	//panic("print_text_marker");   // Tuba 20190712
#else
	boolean traceit = (cinfo->err->trace_level >= 1);
	int length;
	unsigned int ch;
	unsigned int lastch = 0;

	length = jpeg_getc(cinfo) << 8;
	length += jpeg_getc(cinfo);
	length -= 2;			/* discount the length word itself */

	if (traceit) {
		if (cinfo->unread_marker == JPEG_COM)
			fprintf(stderr, "Comment, length %ld:\n", (long)length);
		else			/* assume it is an APPn otherwise */
			fprintf(stderr, "APP%d, length %ld:\n",
				cinfo->unread_marker - JPEG_APP0, (long)length);
	}

	while (--length >= 0) {
		ch = jpeg_getc(cinfo);
		if (traceit) {
			/* Emit the character in a readable form.
			* Nonprintables are converted to \nnn form,
			* while \ is converted to \\.
			* Newlines in CR, CR/LF, or LF form will be printed as one newline.
			*/
			if (ch == '\r') {
				fprintf(stderr, "\n");
			} else if (ch == '\n') {
				if (lastch != '\r')
					fprintf(stderr, "\n");
			} else if (ch == '\\') {
				fprintf(stderr, "\\\\");
			} else if (isprint(ch)) {
				putc(ch, stderr);
			} else {
				fprintf(stderr, "\\%03o", ch);
			}
			lastch = ch;
		}
	}

	if (traceit)
		fprintf(stderr, "\n");
#endif

	return TRUE;
}


/*
* The main program.
*/

int jpgdecInit(JpgDec_t *pJpgDec)
{
	JpgPrivate_t *pJpgPrivate = NULL;
	struct jpeg_decompress_struct *pcinfo = NULL;
	struct jpeg_error_mgr *pjerr = NULL;

	//printk("jpgdecInit \r\n");
	/* output version number  */
	pJpgDec->version = version;

	pJpgPrivate = (JpgPrivate_t *)kmalloc(sizeof(JpgPrivate_t), GFP_KERNEL);
	if (NULL == pJpgPrivate)
		return -1;
	pJpgDec->pVoid = (void *)pJpgPrivate;

	pcinfo = (struct jpeg_decompress_struct *)kmalloc(sizeof(struct jpeg_decompress_struct), GFP_KERNEL);
	if (NULL == pcinfo)
		return -1;
	pJpgPrivate->pcinfo = pcinfo;

	pjerr = (struct jpeg_error_mgr *)kmalloc(sizeof(struct jpeg_error_mgr), GFP_KERNEL);
	if (NULL == pjerr)
		return -1;
	pJpgPrivate->pjerr = pjerr;

	pcinfo->err = jpeg_std_error(pjerr);
	if (jpeg_create_decompress(pcinfo) < 0)
		return -1;
	/* Add some application-specific error messages (from cderror.h) */
	pjerr->addon_message_table = cdjpeg_message_table;
	pjerr->first_addon_message = JMSG_FIRSTADDONCODE;
	pjerr->last_addon_message = JMSG_LASTADDONCODE;

	/* Insert custom marker processor for COM and APP12.
	* APP12 is used by some digital camera makers for textual info,
	* so we provide the ability to display it as text.
	* If you like, additional APPn marker types can be selected for display,
	* but don't try to override APP0 or APP14 this way (see libjpeg.doc).
	*/
	if (jpeg_set_marker_processor(pcinfo, JPEG_COM, print_text_marker) < 0)
		return -1;
	if (jpeg_set_marker_processor(pcinfo, JPEG_APP0 + 12, print_text_marker) < 0)
		return -1;

	/* Specify data source for decompression */
#if (MEM_IO == 0)
	if (jpeg_stdio_src(pcinfo, pJpgDec->pInFile) < 0)
		return -1;
#else
	if (jpeg_mem_src(pcinfo, pJpgDec->pInBuf, pJpgDec->inFileSize) < 0)   //chlin
		return -1;
#endif

	/* Read file header, set default decompression parameters */
	{
		int ret = jpeg_read_header2(pcinfo, TRUE);
		if (JPEG_SUSPENDED == ret || JPEG_HEADER_TABLES_ONLY == ret)
			return -1;
	}

#if (MEM_IO == 0)
	pJpgDec->widthBuf  = pcinfo->image_width;
	pJpgDec->heightBuf = pcinfo->image_height;
#elif (MEM_IO == 3)
	pJpgDec->widthBuf  = (pcinfo->image_width  + 63) >> 6 << 6; 	   // extend to 64x
	pJpgDec->heightBuf = (pcinfo->image_height + 15) >> 4 << 4;
#else
	pJpgDec->widthBuf  = (pcinfo->image_width  + 15) >> 4 << 4;        // extend to 16x
	pJpgDec->heightBuf = (pcinfo->image_height + 15) >> 4 << 4;
#endif

	pJpgDec->widthImg  = pcinfo->image_width;
	pJpgDec->heightImg = pcinfo->image_height;

	pJpgDec->flags = 0;
	pJpgDec->flags |= pcinfo->is_baseline       << 0;
	pJpgDec->flags |= pcinfo->progressive_mode  << 1;
	pJpgDec->flags |= pcinfo->num_components    << 2;
	pJpgDec->flags |= pcinfo->max_h_samp_factor << 4;
	pJpgDec->flags |= pcinfo->max_v_samp_factor << 6;

#if OUTPUT_JPEG_INFO
	pJpgDec->jpgInfo = jpginfo;
	sprintf(pJpgDec->jpgInfo, "%s\n\tResolution: %d x %d", pJpgDec->jpgInfo, pcinfo->image_width, pcinfo->image_height);
	sprintf(pJpgDec->jpgInfo, "%s\n\tPrecision (bits): %d", pJpgDec->jpgInfo, pcinfo->data_precision);
	sprintf(pJpgDec->jpgInfo, "%s\n\tBaseline: %d", pJpgDec->jpgInfo, pcinfo->is_baseline);
	sprintf(pJpgDec->jpgInfo, "%s\n\tProgressive: %d", pJpgDec->jpgInfo, pcinfo->progressive_mode);
	sprintf(pJpgDec->jpgInfo, "%s\n\tArithmetic Coding: %d", pJpgDec->jpgInfo, pcinfo->arith_code);
	sprintf(pJpgDec->jpgInfo, "%s\n\tJFIF Header: %d", pJpgDec->jpgInfo, pcinfo->saw_JFIF_marker);
	sprintf(pJpgDec->jpgInfo, "%s\n\tComponent: %d", pJpgDec->jpgInfo, pcinfo->num_components);
	sprintf(pJpgDec->jpgInfo, "%s\n\tH Sampling Factor: %d", pJpgDec->jpgInfo, pcinfo->max_h_samp_factor);
	sprintf(pJpgDec->jpgInfo, "%s\n\tV Sampling Factor: %d", pJpgDec->jpgInfo, pcinfo->max_v_samp_factor);
#endif

	return pjerr->num_warnings ? EXIT_WARNING : EXIT_SUCCESS;
}

int jpgdec(JpgDec_t *pJpgDec)
{
	JpgPrivate_t *pJpgPrivate = (JpgPrivate_t *)pJpgDec->pVoid;
	struct jpeg_decompress_struct *pcinfo = pJpgPrivate->pcinfo;
	struct jpeg_error_mgr *pjerr = pJpgPrivate->pjerr;
	djpeg_dest_ptr dest_mgr = NULL;
#if (MEM_IO == 0)
	FILE *input_file = pJpgDec->pInFile;
	FILE *output_file = pJpgDec->pOutFile;
#endif
	JDIMENSION num_scanlines;

	//chlin 2019/07/18: use output buffer as part of temporary buffer for progressive mode
	unsigned char *pYuvBuf[4] = { 0 };  // index: 0: U (in temp buffer) ; 1: V (in temp buffer); 2: start of Y (in output buffer)  3: 1/2 positino of Y (in temp buffer)
#if (MEM_IO > 0)
	//int offset = (int)ceil((float)pcinfo->image_width / (8 * pcinfo->max_h_samp_factor)) * (int)ceil((float)pcinfo->image_height / (8 * pcinfo->max_v_samp_factor)) * 128;
	int offset = JD_CEIL(pcinfo->image_width, 8 * pcinfo->max_h_samp_factor) * JD_CEIL(pcinfo->image_height, 8 * pcinfo->max_v_samp_factor) * 128;
	if (NULL == pJpgDec->pTmpBuf)
		pJpgDec->pTmpBuf = pJpgDec->pOutBuf;    // temp buffer not used by non-progressive JPEG, but can't be NULL pointer

	pYuvBuf[2] = pJpgDec->pOutBuf;                                                                          // Y
	pYuvBuf[3] = pJpgDec->pTmpBuf;                                                                          // Y
	pYuvBuf[0] = pJpgDec->pTmpBuf + offset * pcinfo->max_h_samp_factor * pcinfo->max_v_samp_factor * 1 / 2; // U
	pYuvBuf[1] = pYuvBuf[0] + offset;                                                                       // V
#endif


	//printk("pJpgDec->pOutBuf = 0x%x\r\n",(unsigned int)fmem_lookup_pa((unsigned int)pJpgDec->pOutBuf));

	//~chlin

	/* Initialize the output module now to let it override any crucial
	* option settings (for instance, GIF wants to force color quantization).
	*/
	dest_mgr = jinit_write_bmp(pcinfo, FALSE);
	if (NULL == dest_mgr)
		return -1;
#if (MEM_IO == 0)
	dest_mgr->output_file = output_file;
#endif

	/* Start decompressor */
	if (FALSE == jpeg_start_decompress(pcinfo, pYuvBuf)) {    //chlin 2019/07/15: mapping memory buffer to input buffer
		return -1;
	}

	if (!pcinfo->progressive_mode && pcinfo->num_components == 1) { //fill UV data for YUV400
		//memset(pJpgDec->pOutBuf, 0, pJpgDec->widthBuf * pJpgDec->heightBuf);
		memset(pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf, 128, pJpgDec->widthBuf * pJpgDec->heightBuf / 2);
	}

	/* Process data */
	while (pcinfo->output_scanline < pcinfo->output_height) {
		num_scanlines = jpeg_read_scanlines(pcinfo, dest_mgr->buffer, dest_mgr->buffer_height);
		if (0 == num_scanlines)
			return -1;
#if (MEM_IO == 0)
		(*dest_mgr->put_pixel_rows)(pcinfo, dest_mgr, num_scanlines);
#else	// (MEM_IO == 0)
#if (MEM_IO >= 2)
		if (pcinfo->progressive_mode)
#endif
		{
			int row = pcinfo->output_scanline - 1;
			// in buffer
			//unsigned char *ptrY  = *(dest_mgr->buffer);
			//unsigned char *ptrCb = ptrY  + pcinfo->output_width;
			//unsigned char *ptrCr = ptrCb + pcinfo->output_width;
			// out buffer
			unsigned char *pBufY  = pJpgDec->pOutBuf + row * pJpgDec->widthBuf;
			unsigned char *pBufCb = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf + row * pJpgDec->widthBuf / 4;
			unsigned char *pBufCr = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf + pJpgDec->widthBuf * pJpgDec->heightBuf / 4 + row * pJpgDec->widthBuf / 4;

			if (pcinfo->progressive_mode) {
				pBufCb = pYuvBuf[0] + row * pJpgDec->widthBuf / 4;
				pBufCr = pYuvBuf[1] + row * pJpgDec->widthBuf / 4;
			}

		#if NVT_CMYK_FORMULA
			// chlin 2022/12/09: add color conversion for RGB/YCCK/CMYK
			if (pcinfo->jpeg_color_space == JCS_RGB)
			{
				// in buffer
				unsigned char *ptr = *(dest_mgr->buffer);
				int i, j;

				for (i = 0, j = 0; i < pcinfo->output_width; i++, j+=3)
				{
					unsigned char r = ptr[j];
					unsigned char g = ptr[j+1];
					unsigned char b = ptr[j+2];

					pBufY[i] = (unsigned char)((257 * r + 564 * g + 98 * b + 16500)/1000);
					if ((row & 1) == 0 && (i & 1) == 0)
					{
					    pBufCb[i>>1] = (unsigned char)((-148 * r - 291 * g + 439 * b + 128500)/1000);
					    pBufCr[i>>1] = (unsigned char)(( 439 * r - 368 * g - 71 * b + 128500)/1000);
					}
				}
			}
			else if (pcinfo->jpeg_color_space == JCS_YCCK)
			{
			    // in buffer
				unsigned char *ptr = *(dest_mgr->buffer);
				int i, j;

			    for (i = 0, j = 0; i < pcinfo->output_width; i++, j+=4)
			    {
					int k = ptr[j+3];
					#if 1   //ffmpeg
					int r = (255 - ptr[j+0]) * k;
					int g = (128 - ptr[j+1]) * k;
					int b = (128 - ptr[j+2]) * k;
					pBufY[i] = r*257 >> 16;
					if ((row & 1) == 0 && (i & 1) == 0)
					{
					    pBufCb[i>>1] = (g*257 >> 16) + 128;
					    pBufCr[i>>1] = (b*257 >> 16) + 128;
					}
					#else   // KaA
					int y  = ptr[j+0];
					int cb = ptr[j+1];
					int cr = ptr[j+2];
					int r = clip(255 - (y+cr+((cr-128)*103)>>8) );
					int g = clip(255 - (y-((cb-128)*88)>>8)-((cr-128)*183)>>8 );
					int b = clip(255 - (y+(cb-128) -((cr-128)*198)>>8) );

					r = (r*k*257)>>16;
					g = (g*k*257)>>16;
					b = (b*k*257)>>16;

					pBufY[i] = clip((r * 77) >> 8) + ((g * 150) >> 8) + ((b * 29) >> 8);
					if ((row & 1) == 0 && (i & 1) == 0)
					{
					    pBufCb[i>>1] = clip(128 - ((r * 43) >> 8) - ((g * 85) >> 8) + ((b * 128) >> 8));  
					    pBufCr[i>>1] = clip(128 + ((r * 128) >> 8) - ((g * 107) >> 8) - ((b * 21)  >> 8));
					}
					#endif
					//printf("\n%d\t%d\t%d\t%d", ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3]);
				}
			}
			else if (pcinfo->jpeg_color_space == JCS_CMYK)
			{
			    // in buffer
				unsigned char *ptr = *(dest_mgr->buffer);
				int i, j;

				for (i = 0, j = 0; i < pcinfo->output_width; i++, j+=4)
				{
					int k = ptr[j+3];
					int r = ptr[j+0] * k;
					int g = ptr[j+1] * k;
					int b = ptr[j+2] * k;
					g = g*257 >> 16;
					b = b*257 >> 16;
					r = r*257 >> 16;
					#if 1   //ffmpeg
					pBufY[i] = (unsigned char)((257 * r + 564 * g + 98 * b + 16500)/1000);
					if ((row & 1) == 0 && (i & 1) == 0)
					{
					    pBufCb[i>>1] = (unsigned char)((-148 * r - 291 * g + 439 * b + 128500)/1000);
					    pBufCr[i>>1] = (unsigned char)(( 439 * r - 368 * g - 71 * b + 128500)/1000);
					}
					#else   // KaA
					pBufY[i] = clip((r * 77) >> 8) + ((g * 150) >> 8) + ((b * 29) >> 8);
					if ((row & 1) == 0 && (i & 1) == 0)
					{
					    pBufCb[i>>1] = clip(128 - ((r * 43) >> 8) - ((g * 85) >> 8) + ((b * 128) >> 8));
					    pBufCr[i>>1] = clip(128 + ((r * 128) >> 8) - ((g * 107) >> 8) - ((b * 21)  >> 8));
					}
					#endif
				}
			}
			else
			//~chlin 2022/12/09
		#endif	// NVT_CMYK_FORMULA
			{
				// in buffer
				unsigned char *ptrY  = *(dest_mgr->buffer);
				unsigned char *ptrCb = ptrY  + pcinfo->output_width;
				unsigned char *ptrCr = ptrCb + pcinfo->output_width;

				memcpy(pBufY, ptrY, pcinfo->output_width);

				if (pcinfo->num_components > 1 && (row & 1) == 0)
				{
					memcpy(pBufCb, ptrCb, pJpgDec->widthBuf / 2);
					memcpy(pBufCr, ptrCr, pJpgDec->widthBuf / 2);
				}
			}

			//clear Y right border
			if (pJpgDec->widthBuf != pcinfo->output_width)
				memset(pBufY + pcinfo->output_width, 0, pJpgDec->widthBuf - pcinfo->output_width);

			if (pcinfo->num_components > 1 && (row & 1) == 0) {
				/*if (!pcinfo->progressive_mode)
				{
				    memset(ptrCb + pcinfo->output_width / 2 + (pcinfo->output_width & 1), 128, (pJpgDec->widthBuf - pcinfo->output_width) / 2);
				    memset(ptrCr + pcinfo->output_width / 2 + (pcinfo->output_width & 1), 128, (pJpgDec->widthBuf - pcinfo->output_width) / 2);
				}*/

				//memcpy(pBufCb, ptrCb, pJpgDec->widthBuf / 2);
				//memcpy(pBufCr, ptrCr, pJpgDec->widthBuf / 2);

				//clear UV right border
				if (pJpgDec->widthBuf - pcinfo->output_width > 1) {
					memset(pBufCb + pcinfo->output_width / 2 + (pcinfo->output_width & 1), 128, (pJpgDec->widthBuf - pcinfo->output_width) / 2);
					memset(pBufCr + pcinfo->output_width / 2 + (pcinfo->output_width & 1), 128, (pJpgDec->widthBuf - pcinfo->output_width) / 2);
				}
			}
		}
#if (MEM_IO >= 2)
		else {
			int row = pcinfo->output_scanline - 1;
			// out buffer
			unsigned char *pBufY = pJpgDec->pOutBuf + row * pJpgDec->widthBuf;
			unsigned char *pBufC = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf + row * pJpgDec->widthBuf / 2;

		#if NVT_CMYK_FORMULA
			// chlin 2022/12/09: add color conversion for RGB/YCCK/CMYK
			if (pcinfo->jpeg_color_space == JCS_RGB)
			{
				// in buffer
				unsigned char *ptr = *(dest_mgr->buffer);
				int i, j;

				for (i = 0, j = 0; i < pcinfo->output_width; i++, j+=3)
				{
					unsigned char r = ptr[j];
					unsigned char g = ptr[j+1];
					unsigned char b = ptr[j+2];

					pBufY[i] = (unsigned char)((257 * r + 564 * g + 98 * b + 16500)/1000);
					if ((row & 1) == 0 && (i & 1) == 0)
					{
					    #if (MEM_IO == 2)
					    pBufC[((i>>4)<<4)+((i%16)>>2)]   = (unsigned char)((-148 * r - 291 * g + 439 * b + 128500)/1000);
					    pBufC[((i>>4)<<4)+((i%16)>>2)+8] = (unsigned char)(( 439 * r - 368 * g - 71 * b + 128500)/1000);
					    #else // MEM_IO == 3
					    pBufC[i+0] = (unsigned char)((-148 * r - 291 * g + 439 * b + 128500)/1000);
					    pBufC[i+1] = (unsigned char)(( 439 * r - 368 * g - 71 * b + 128500)/1000);
					    #endif
					}
				}
			}
			else if (pcinfo->jpeg_color_space == JCS_YCCK)
			{
			    // in buffer
				unsigned char *ptr = *(dest_mgr->buffer);
				int i, j;

				for (i = 0, j = 0; i < pcinfo->output_width; i++, j+=4)
				{
					int k = ptr[j+3];
					int r = (255 - ptr[j+0]) * k;
					int g = (128 - ptr[j+1]) * k;
					int b = (128 - ptr[j+2]) * k;
					pBufY[i] = r*257 >> 16;
					if ((row & 1) == 0 && (i & 1) == 0)
					{
					    #if (MEM_IO == 2)
					    pBufC[((i>>4)<<4)+((i%16)>>2)]   = (g*257 >> 16) + 128;
					    pBufC[((i>>4)<<4)+((i%16)>>2)+8] = (b*257 >> 16) + 128;
					    #else // MEM_IO == 3
					    pBufC[i+0] = (g*257 >> 16) + 128;
					    pBufC[i+1] = (b*257 >> 16) + 128;
					    #endif
					}
				}
			}
			else if (pcinfo->jpeg_color_space == JCS_CMYK)
			{
				// in buffer
				unsigned char *ptr = *(dest_mgr->buffer);
				int i, j;

				for (i = 0, j = 0; i < pcinfo->output_width; i++, j+=4)
				{
					int k = ptr[j+3];
					int r = ptr[j+0] * k;
					int g = ptr[j+1] * k;
					int b = ptr[j+2] * k;
					g = g*257 >> 16;
					b = b*257 >> 16;
					r = r*257 >> 16;
					pBufY[i] = (unsigned char)((257 * r + 564 * g + 98 * b + 16500)/1000);
					if ((row & 1) == 0 && (i & 1) == 0)
					{
					    #if (MEM_IO == 2)
					    pBufC[((i>>4)<<4)+((i%16)>>2)]   = (unsigned char)((-148 * r - 291 * g + 439 * b + 128500)/1000);
					    pBufC[((i>>4)<<4)+((i%16)>>2)+8] = (unsigned char)(( 439 * r - 368 * g - 71 * b + 128500)/1000);
					    #else // MEM_IO == 3
					    pBufC[i+0] = (unsigned char)((-148 * r - 291 * g + 439 * b + 128500)/1000);
					    pBufC[i+1] = (unsigned char)(( 439 * r - 368 * g - 71 * b + 128500)/1000);
					    #endif
					}
				}
			}
			else
			//~chlin 2022/12/09
		#endif
			{
				// in buffer
				unsigned char *ptrY = *(dest_mgr->buffer);
				unsigned char *ptrCb = ptrY + pcinfo->output_width;
				unsigned char *ptrCr = ptrCb + pcinfo->output_width;
				// out buffer
				unsigned char *pBufY = pJpgDec->pOutBuf + row * pJpgDec->widthBuf;
				unsigned char *pBufC = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf + row * pJpgDec->widthBuf / 2;

				memcpy(pBufY, ptrY, pcinfo->output_width);
				#if PADDING_BLACK
				//clear Y right border
				if (pJpgDec->widthBuf > pcinfo->output_width)
					memset(pBufY + pcinfo->output_width, 0, pJpgDec->widthBuf - pcinfo->output_width);
				#endif

				if (pcinfo->num_components > 1 && (row & 1) == 0) {
					int i;

					#if PADDING_BLACK
					//clear UV right border
					if (pJpgDec->widthBuf > pcinfo->output_width + 1) {
						memset(ptrCb + pcinfo->output_width / 2 + (pcinfo->output_width & 1), 128, (pJpgDec->widthBuf - pcinfo->output_width) / 2);
						memset(ptrCr + pcinfo->output_width / 2 + (pcinfo->output_width & 1), 128, (pJpgDec->widthBuf - pcinfo->output_width) / 2);
					}
					#endif

				#if (MEM_IO == 2)
					for (i = 0; i < pJpgDec->widthBuf / 2; i += 8) {
						memcpy(pBufC, ptrCb, 8);
						pBufC += 8;
						ptrCb += 8;
						memcpy(pBufC, ptrCr, 8);
						pBufC += 8;
						ptrCr += 8;
					}
				#else // MEM_IO == 3
					for (i = 0; i < pJpgDec->widthBuf / 2; i++) {
						if (pJpgDec->uv_swap) {
							*(pBufC++) = *(ptrCr++);
							*(pBufC++) = *(ptrCb++);
						} else {
							*(pBufC++) = *(ptrCb++);
							*(pBufC++) = *(ptrCr++);
						}
					}
				#endif
				}
			}
		}
#endif	// #if (MEM_IO >= 2)
#endif	// (MEM_IO == 0)
	}

	if (pcinfo->progressive_mode) { // move UV data from temporary position to correct position
		if (pcinfo->num_components == 1) { //fill UV data for YUV400
			unsigned char *pBufCb = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf;
			unsigned char *pBufCr = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf + pJpgDec->widthBuf * pJpgDec->heightBuf / 4;
			memset(pBufCb, 128, pJpgDec->widthBuf * pJpgDec->heightBuf / 4);
			memset(pBufCr, 128, pJpgDec->widthBuf * pJpgDec->heightBuf / 4);
		} else {
#if (MEM_IO == 1)
			unsigned char *pBufCb = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf;
			unsigned char *pBufCr = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf + pJpgDec->widthBuf * pJpgDec->heightBuf / 4;
			memcpy(pBufCb, pYuvBuf[0], pJpgDec->widthBuf * pJpgDec->heightBuf / 4);
			memcpy(pBufCr, pYuvBuf[1], pJpgDec->widthBuf * pJpgDec->heightBuf / 4);
#elif (MEM_IO >= 2)
			unsigned char *ptrCb = pYuvBuf[0];
			unsigned char *ptrCr = pYuvBuf[1];
			unsigned char *pBufC = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf;
			int i, j;

#if (MEM_IO == 2)
			for (i = 0; i < pJpgDec->heightBuf / 2; i ++)
				for (j = 0; j < pJpgDec->widthBuf / 2 ; j += 8) {
					memcpy(pBufC, ptrCb, 8);
					pBufC += 8;
					ptrCb += 8;
					memcpy(pBufC, ptrCr, 8);
					pBufC += 8;
					ptrCr += 8;
				}
#else // MEM_IO == 3
			for (i = 0; i < pJpgDec->heightBuf / 2; i ++)
				for (j = 0; j < pJpgDec->widthBuf / 2 ; j ++) {
					if (pJpgDec->uv_swap) {
						*(pBufC++) = *(ptrCr++);
						*(pBufC++) = *(ptrCb++);
					} else {
						*(pBufC++) = *(ptrCb++);
						*(pBufC++) = *(ptrCr++);
					}
				}
#endif
#endif	// (MEM_IO >= 2)
		}
	}

#if PADDING_BLACK
	#if (MEM_IO == 1)
	//clear bottom border
	if (pJpgDec->heightBuf != pcinfo->output_height) {
		unsigned char *pBufY = pJpgDec->pOutBuf;
		unsigned char *pBufCb = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf;
		unsigned char *pBufCr = pBufCb + pJpgDec->widthBuf * pJpgDec->heightBuf / 4;
		memset(pBufY  + pJpgDec->widthBuf * pcinfo->output_height, 0, pJpgDec->widthBuf * (pJpgDec->heightBuf - pcinfo->output_height));
		if (pJpgDec->heightBuf - pcinfo->output_height > 1) {
			memset(pBufCb + pJpgDec->widthBuf / 2 * (pcinfo->output_height / 2 + (pcinfo->output_height & 1)), 128, pJpgDec->widthBuf / 2 * ((pJpgDec->heightBuf - pcinfo->output_height) / 2));
			memset(pBufCr + pJpgDec->widthBuf / 2 * (pcinfo->output_height / 2 + (pcinfo->output_height & 1)), 128, pJpgDec->widthBuf / 2 * ((pJpgDec->heightBuf - pcinfo->output_height) / 2));
		}
	}
	#elif (MEM_IO == 3)
	if (pJpgDec->heightBuf > pcinfo->output_height) {
		unsigned char *pBufY = pJpgDec->pOutBuf;
		unsigned char *pBufC = pJpgDec->pOutBuf + pJpgDec->widthBuf * pJpgDec->heightBuf;
		memset(pBufY  + pJpgDec->widthBuf * pcinfo->output_height, 0, pJpgDec->widthBuf * (pJpgDec->heightBuf - pcinfo->output_height));
		if (pJpgDec->heightBuf > pcinfo->output_height + 1) {
			memset(pBufC + pJpgDec->widthBuf * ((pcinfo->output_height+1)/2), 128, pJpgDec->widthBuf * (pJpgDec->heightBuf/2 - (pcinfo->output_height+1)/2));
		}
	}
	#endif
#endif
	/* Finish decompression and release memory.
	* I must do it in this order because output module has allocated memory
	* of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
	*/
#if (MEM_IO == 0)
	(*dest_mgr->finish_output)(pcinfo, dest_mgr);
#endif

	return pjerr->num_warnings ? EXIT_WARNING : EXIT_SUCCESS;
}

int jpgdecFree(JpgDec_t *pJpgDec)
{
	JpgPrivate_t *pJpgPrivate = (JpgPrivate_t *)pJpgDec->pVoid;
	struct jpeg_decompress_struct *pcinfo;// = pJpgPrivate->pcinfo;
	struct jpeg_error_mgr *pjerr;// = pJpgPrivate->pjerr;
	int num_warnings = 0;// = (int)pjerr->num_warnings;

	if (pJpgPrivate) {
		pcinfo = pJpgPrivate->pcinfo;
		if (pcinfo) {
			(void)jpeg_finish_decompress2(pcinfo);
			jpeg_destroy_decompress2(pcinfo);
			kfree(pcinfo);
			pJpgPrivate->pcinfo = NULL;
		}

		pjerr = pJpgPrivate->pjerr;
		if (pjerr) {
			num_warnings = (int)pjerr->num_warnings;
			kfree(pjerr);
			pJpgPrivate->pjerr = NULL;
		}

		kfree(pJpgPrivate);
		pJpgDec->pVoid = NULL;
	}

	return num_warnings ? EXIT_WARNING : EXIT_SUCCESS;
}

int jpeg_sw_decode(JpgSWDec *dec_param)
{
	JpgDec_t jpgDec = {0};
	unsigned char *pSWTmpBuf = NULL;
	//int error_type = 0;
	int ret = 0;

	dec_param->error_type = 0;
	//DBG_FLOW("sw decode job %u\r\n", job_item->job_id);
	jpgDec.pInBuf = (unsigned char *)dec_param->jpeg_hdr_addr;
	jpgDec.inFileSize = dec_param->bs_size;
	ret = jpgdecInit(&jpgDec);
	if (ret < 0) {
		printk("SW decoder initialization failed!");
		dec_param->error_type = -1;
		goto exit;
	}
	if (1) {
		int sfacH = (jpgDec.flags & JPG_ATRIB_SFACTOR_H) >> 4;
		int sfacV = (jpgDec.flags & JPG_ATRIB_SFACTOR_V) >> 6;
		int tmpBufW = JD_CEIL(jpgDec.widthImg, 8 * sfacH) * (8 * sfacH);
		int tmpBufH = JD_CEIL(jpgDec.heightImg, 8 * sfacV) * (8 * sfacV);
		int tmpBufSize = 0, decBufSize;
		if (jpgDec.flags & JPG_ATRIB_PROGRESS) {
			tmpBufSize = JD_CEIL(jpgDec.widthImg, (8 * sfacH)) * JD_CEIL(jpgDec.heightImg, (8 * sfacV)) * 128 * (2 + sfacH * sfacV);
			tmpBufSize -= tmpBufW * tmpBufH;

			//pSWTmpBuf = (unsigned char *)jpeg_kmalloc(sizeof(unsigned char) * tmpBufSize, "sw_dec");
			pSWTmpBuf = (unsigned char *)vmalloc(sizeof(unsigned char) * tmpBufSize);
			if (NULL == pSWTmpBuf) {
				printk("allocate sw decode tmp buffer fail!!\n");
				dec_param->error_type = -1;
				goto exit;
			}
			//DBG_FLOW("sw dec allocate tmp buffer 0x%lx, size %d\n", (unsigned long)pSWTmpBuf, tmpBufSize);
		}
		decBufSize = JD_CEIL(jpgDec.widthImg, 64) * 64 * JD_CEIL(jpgDec.heightImg, 16) * 16 * 3 / 2;
		if (decBufSize > dec_param->raw_size) {
			printk("sw decode out buffer is not enough, need size %d, buffer size %d (%d x %d)\n",
			       decBufSize, dec_param->raw_size, jpgDec.widthImg, jpgDec.heightImg);
			dec_param->error_type = -1;
			goto exit;
		}
		dec_param->img_width = jpgDec.widthImg;
		dec_param->img_height = jpgDec.heightImg;
		dec_param->bg_width = JD_CEIL(jpgDec.widthImg, 64) * 64;
		dec_param->bg_height = JD_CEIL(jpgDec.heightImg, 16) * 16;
	}

	jpgDec.pOutBuf = (unsigned char *)dec_param->yuv_addr_va;
	jpgDec.pTmpBuf = pSWTmpBuf;
	jpgDec.uv_swap = dec_param->uv_swap;
	ret = jpgdec(&jpgDec);
	if (ret < 0) {
		printk("sw jpeg decode failed!");
		dec_param->error_type = -1;
		goto exit;
	}

exit:
	if (pSWTmpBuf) {
		//DBG_FLOW("sw dec free tmp buffer 0x%lx\n", (unsigned long)pSWTmpBuf);
		//jpeg_kfree((void *)pSWTmpBuf);
		vfree((void *)pSWTmpBuf);
	}
	ret = jpgdecFree(&jpgDec);
	//jpg_sw_decode_done(job_item, error_type);
	//if (0 == error_type)
	//	vos_cpu_dcache_sync(dec_param->yuv_addr_va, dec_param->raw_size, VOS_DMA_BIDIRECTIONAL);
	return ret;
}

JPEG_SW_DEC_FUNC jd_func = {
	.jpg_sw_dec = jpeg_sw_decode
};

extern int jpeg_register_func(JPEG_SW_DEC_FUNC *func);
extern void jpeg_deregister_func(void);
extern int nvt_sw_dec_proc_init(void);
extern int nvt_sw_dec_proc_clear(void);

static int __init jpgdec_init(void)
{
	nvt_sw_dec_proc_init();
	jpeg_register_func(&jd_func);
	printk("JPEG decode ver%s\r\n", JPEG_DEC_VERSION);

	return 0;
}

static void __exit jpgdec_exit(void)
{
	jpeg_deregister_func();
	nvt_sw_dec_proc_clear();
}

module_init(jpgdec_init);
module_exit(jpgdec_exit);

EXPORT_SYMBOL(jpeg_sw_decode);

MODULE_AUTHOR("Novatek Corp.");
MODULE_LICENSE("GPL");
MODULE_VERSION(JPEG_DEC_VERSION);


