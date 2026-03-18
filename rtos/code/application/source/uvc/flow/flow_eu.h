#ifndef _FLOW_EU_H
#define _FLOW_EU_H

extern BOOL xUvacEU_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen);

#define NUM_PACK 152	//(1920*1080*1.5)/20KB=151.875
#define SIZE_PACK 20480	//max 20KB

#endif
