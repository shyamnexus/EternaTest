#ifndef MD5_H
#define MD5_H

#include "kwrap/type.h"

#define MD5_FILE_BUFFER_LEN 1024

struct MD5Context {
    UINT32 buf[4];
    UINT32 bits[2];
    unsigned char in[64];
};

BOOL md5_calc(unsigned char *digest, unsigned char *in_buf, unsigned int in_len);

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(UINT32 buf[4], UINT32 const in[16]);

#if 0
int getBytesMD5(const unsigned char* src, unsigned int length, char* md5);
int getStringMD5(const char* src, char* md5);
int getFileMD5(const char* path, char* md5);
#endif

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct MD5Context MD5_CTX;

#endif /* !MD5_H */
