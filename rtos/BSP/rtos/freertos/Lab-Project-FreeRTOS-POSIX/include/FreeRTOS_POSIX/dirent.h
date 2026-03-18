#ifndef _NVT_FREERTOS_DIRENT_H
#define _NVT_FREERTOS_DIRENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DT_FIFO     1
#define DT_CHR      2
#define DT_DIR      4
#define DT_BLK      6
#define DT_REG      8
#define DT_LNK      10
#define DT_SOCK     12
#define DT_WHT      14

typedef UINT32 DIR;

struct dirent {
    ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* not an offset; see NOTES */
    unsigned short d_reclen;    /* length of this record */
    unsigned char  d_type;      /* type of file; not supported
                                   by all filesystem types */
    char           d_name[256]; /* filename */
};

DIR *opendir(const char *name);

struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

void rewinddir(DIR *dirp);

int closedir(DIR *dirp);

#ifdef __cplusplus
}
#endif

#endif //_NVT_FREERTOS_DIRENT_H

