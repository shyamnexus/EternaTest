#ifndef _FILE_SAVE_H_
#define _FILE_SAVE_H_

#include "vendor_common.h"

#define FILESAVE_MAX_USER (5U)
#define FILESIZE_MAX (2 * 1024 * 1024 * 1024U) // Single file maxium size is 2GB
#define FILENAME "/mnt/sd/dump_bs-%s-%03d.dat"

typedef struct _FILESAVE_CTX FILESAVE_CTX;

HD_RESULT filesave_init(void);
FILESAVE_CTX *filesave_get_serv(ULONG filesize, CHAR *id);
HD_RESULT filesave_uninit(void);
HD_RESULT filesave_record(FILESAVE_CTX *ctx, UINTPTR buffer, ULONG length);
HD_RESULT filesave_trigger(void);

#endif // _FILE_SAVE_H_