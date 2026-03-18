/*
 * Usefuls routines based on the LzmaTest.c file from LZMA SDK 4.65
 *
 * Copyright (C) 2007-2008 Industrie Dial Face S.p.A.
 * Luigi 'Comio' Mantellini (luigi.mantellini@idf-hit.com)
 *
 * Copyright (C) 1999-2005 Igor Pavlov
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __LZMA_TOOL_H__
#define __LZMA_TOOL_H__

int Lzma_Compress(void *in, void *out, unsigned int insize);
int Lzma_Uncompress(void *in, void *out, unsigned int insize);

#endif
