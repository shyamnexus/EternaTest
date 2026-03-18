#ifndef _FILEDB_ID_H
#define _FILEDB_ID_H

/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#include "kwrap/flag.h"
#include "kwrap/semaphore.h"
#include "kwrap/type.h"

/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define FILEDB_NUM                     3

/*-----------------------------------------------------------------------------*/
/* Extern Variables                                                            */
/*-----------------------------------------------------------------------------*/
extern SEM_HANDLE _SECTION(".kercfg_data") SEMID_FILEDB_COMM;
extern SEM_HANDLE _SECTION(".kercfg_data") SEMID_FILEDB[FILEDB_NUM];
extern ID _SECTION(".kercfg_data") FLG_ID_FILEDB;

#endif //_FILEDB_ID_H