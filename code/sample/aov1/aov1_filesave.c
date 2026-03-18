#include "aov1_filesave.h"
#include "aov1_common.h"

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>

#define container_of(ptr, type, member)                             \
    __extension__({                                                 \
        const __typeof__(((type *)0)->member) *(__pmember) = (ptr); \
        (type *)((char *)__pmember - offsetof(type, member));       \
    })

struct _FILESAVE_CTX {
    BOOL in_use;
    BOOL is_saving;
    CHAR filesave_id[256];
    pthread_mutex_t lock;
    UINTPTR buffer;
    ULONG buffer_unit_size;
    ULONG size;
    ULONG written;
    UINT32 last_index;
    UINT32 curr_index;
    FILE *f_out;
    UINT8 f_num;
};

static pthread_t filesave_tid;
static BOOL filesave_is_enable = FALSE;
static sem_t filesave_ready;
static FILESAVE_CTX filesave_ctx[FILESAVE_MAX_USER];

static FILE *fopen_new(CHAR *id, UINT8 file_num)
{
    FILE *file;
    CHAR filename[256];

    snprintf(filename, sizeof(filename), FILENAME, id, file_num);
    file = fopen(filename, "wb");
    if (!file) {
        printf("Open file [%s] failed: %s\n", filename, strerror(errno));
        return NULL;
    }
    printf("Create new file [%s] \r\n", filename);

    return file;
}

/**
 * @brief Filewrite function but only provide write range data, and do not support idx2 < idx1
 *
 * @param ctx filesave context
 * @param idx1 first index
 * @param idx2 second index
 * @return ULONG return written bytes
 */
static ULONG _filesave_write(FILESAVE_CTX *ctx, ULONG idx1, ULONG idx2)
{
    ULONG written;
    ULONG tb_write;
    ULONG file_remain;
    UINT8 *ctx_buff = (UINT8 *)ctx->buffer;

    if (idx2 < idx1) {
        (void)fprintf(stderr, "[%s] Parameter invalid\n", __func__);
        return 0;
    }

    file_remain = FILESIZE_MAX - ctx->written;
    if (file_remain < (idx2 - idx1)) {
        tb_write = file_remain;
        written = fwrite(&ctx_buff[idx1], (sizeof(UINT8)), tb_write, ctx->f_out);
        if (written != tb_write) {
            (void)fprintf(stderr, "File[%s] failed: %s\n", ctx->filesave_id, strerror(errno));
        }
        ctx->written = 0; // reset to 0, because new file will open
        fflush(ctx->f_out);
        fsync(ctx->f_out->_fileno);
        fclose(ctx->f_out);

        ctx->f_out = fopen_new(ctx->filesave_id, ++ctx->f_num);
        if (!ctx->f_out) {
            (void)fprintf(stderr, "File[%s][%d] open failed", ctx->filesave_id, ctx->f_num);
            goto exit;
        }
        return written + _filesave_write(ctx, idx1 + written, idx2);
    } else {
        tb_write = idx2 - idx1;
        written = fwrite(&ctx_buff[ctx->last_index], (sizeof(UINT8)), tb_write, ctx->f_out);
        if (written != tb_write) {
            (void)fprintf(stderr, "File[%s] failed: %s\n", ctx->filesave_id, strerror(errno));
        }
        fflush(ctx->f_out);
        fsync(ctx->f_out->_fileno);
        ctx->written += written;
    }

exit:
    return written;
}

static void *thrd_filesave(void *ptr)
{
    UINT32 i;
    ULONG written_p1;
    ULONG written_p2;
    ULONG tb_write;
    ULONG file_remain;

    (void)ptr; // Not used

    printf("Start file saving ...\n");

    while (filesave_is_enable) {
        sem_wait(&filesave_ready);

        if (!filesave_is_enable) {
            break; // early leave
        }

        for (i = 0; i < ARRAY_SIZE(filesave_ctx); i++) {
            if (!filesave_ctx[i].in_use) {
                continue;
            }

            written_p1 = written_p2 = 0;
            pthread_mutex_lock(&filesave_ctx[i].lock);

            if (!filesave_ctx[i].f_out) {
                printf("\r\n\n\n\n\033[1;37;43m  File descriptor is not inited \033[0m\r\n\n\n");
                filesave_ctx[i].f_out = fopen_new(filesave_ctx[i].filesave_id, filesave_ctx[i].f_num);
                if (!filesave_ctx[i].f_out) {
                    filesave_ctx[i].f_out = NULL;
                    goto next_loop;
                }
            }

            if (filesave_ctx[i].curr_index == filesave_ctx[i].last_index) { // No change
                goto next_loop;
            }

            if (filesave_ctx[i].curr_index < filesave_ctx[i].last_index) {
                written_p1 = _filesave_write(&filesave_ctx[i], filesave_ctx[i].last_index, filesave_ctx[i].size);
                if (written_p1 == 0) {
                    goto next_loop;
                }
                filesave_ctx[i].last_index = 0;
            }

            if (filesave_ctx[i].last_index < filesave_ctx[i].curr_index) {
                written_p2 = _filesave_write(&filesave_ctx[i], filesave_ctx[i].last_index, filesave_ctx[i].curr_index);
                if (written_p2 == 0) {
                    goto next_loop;
                }
                filesave_ctx[i].last_index += written_p2;
            }

            filesave_ctx[i].is_saving = FALSE;

        next_loop:
            pthread_mutex_unlock(&filesave_ctx[i].lock);
        }
    }

    pthread_exit(NULL);
}

static HD_RESULT filesave_target_trigger(FILESAVE_CTX *ctx)
{
    ULONG written_p1;
    ULONG written_p2;
    ULONG tb_write;
    ULONG file_remain;

    if (!ctx->in_use) {
        return HD_OK;
    }

    written_p1 = written_p2 = 0;

    if (!ctx->f_out) {
        printf("\r\n\n\n\n\033[1;37;43m  File descriptor is not inited \033[0m\r\n\n\n");
        ctx->f_out = fopen_new(ctx->filesave_id, ctx->f_num);
        if (!ctx->f_out) {
            ctx->f_out = NULL;
            goto exit;
        }
    }

    if (ctx->curr_index == ctx->last_index) { // No change
        goto exit;
    }

    if (ctx->curr_index < ctx->last_index) {
        written_p1 = _filesave_write(ctx, ctx->last_index, ctx->size);
        if (written_p1 == 0) {
            goto exit;
        }
        ctx->last_index = 0;
    }

    if (ctx->last_index < ctx->curr_index) {
        written_p2 = _filesave_write(ctx, ctx->last_index, ctx->curr_index);
        if (written_p2 == 0) {
            goto exit;
        }
        ctx->last_index += written_p2;
    }


exit:
    ctx->is_saving = FALSE;

    return HD_OK;
}

HD_RESULT filesave_trigger(void)
{
    sem_post(&filesave_ready);

    return HD_OK;
}

HD_RESULT filesave_record(FILESAVE_CTX *ctx, UINTPTR buffer, ULONG length)
{
    HD_RESULT ret = HD_OK;
    UINT32 curr_index = 0;
    UINT8 *user_buff;
    UINT8 *ctx_buff;

    pthread_mutex_lock(&ctx->lock);

    if (length == 0 || buffer == 0) {
        return HD_OK;
    }
    user_buff = (UINT8 *)buffer;
    ctx_buff = (UINT8 *)ctx->buffer;

    if (length > (ctx->size - ctx->curr_index)) {
        (void)memcpy(&ctx_buff[ctx->curr_index], &user_buff[curr_index], (ctx->size - ctx->curr_index));
        length -= (ctx->size - ctx->curr_index);
        curr_index += (ctx->size - ctx->curr_index);
        ctx->curr_index = 0;
    }

    if (length < (ctx->size - ctx->curr_index)) {
        (void)memcpy(&ctx_buff[ctx->curr_index], &user_buff[curr_index], length);
        ctx->curr_index += length;
        curr_index += length;
    }

    if (ctx->curr_index > ctx->last_index) {
        if ((ctx->curr_index - ctx->last_index) > ctx->buffer_unit_size && ctx->is_saving == FALSE) {
            ctx->is_saving = TRUE;
            printf("\r\n\n\n\n\033[1;37;42m  File [" FILENAME "] saved %lu , FileSize:%d  \033[0m\r\n\n\n", 
					ctx->filesave_id, ctx->f_num, (ctx->curr_index - ctx->last_index), ctx->written);
            filesave_target_trigger(ctx);
        }
    } else {
        if ((ctx->size - ctx->last_index + ctx->curr_index) > ctx->buffer_unit_size && ctx->is_saving == FALSE) {
            ctx->is_saving = TRUE;
            printf("\r\n\n\n\n\033[1;37;42m  File [" FILENAME "] saved %lu , FileSize:%d  \033[0m\r\n\n\n", 
					ctx->filesave_id, ctx->f_num, (ctx->size - ctx->last_index + ctx->curr_index), ctx->written);
            filesave_target_trigger(ctx);
        }
    }

    pthread_mutex_unlock(&ctx->lock);

    return HD_OK;
}

HD_RESULT filesave_init(void)
{
    UINT32 i;

    if (filesave_is_enable) {
        return HD_OK;
    }

    filesave_is_enable = TRUE;

    sem_init(&filesave_ready, 0, 0);

    (void)memset(filesave_ctx, 0, sizeof(filesave_ctx));

    for (i = 0; i < ARRAY_SIZE(filesave_ctx); i++) {
        pthread_mutex_init(&filesave_ctx[i].lock, NULL);
    }

    if (pthread_create(&filesave_tid, NULL, thrd_filesave, NULL) < 0) {
        printf("Thread [filesave] create failed\n");
        goto finish;
    }

    return HD_OK;

finish:
    return HD_ERR_NG;
}

FILESAVE_CTX *filesave_get_serv(ULONG filesize, CHAR *id)
{
    FILESAVE_CTX *avail_ctx = NULL;
    UINT32 i;
    UINT32 total_buffer_size;

    if (!filesave_is_enable) {
        (void)fprintf(stderr, "filesave don't inited\n");
        return NULL;
    }

    for (i = 0; i < ARRAY_SIZE(filesave_ctx); i++) {
        pthread_mutex_lock(&filesave_ctx[i].lock);
        if (!filesave_ctx[i].in_use) {
            avail_ctx = &filesave_ctx[i];
            pthread_mutex_unlock(&filesave_ctx[i].lock);
            break; // find next
        }
        pthread_mutex_unlock(&filesave_ctx[i].lock);
    }

    if (!avail_ctx) {
        (void)fprintf(stderr, "filesave context resource full\n");
        return NULL;
    }

    avail_ctx->in_use = TRUE;
    avail_ctx->is_saving = FALSE;

#define RETRY_TIMES (5)
    total_buffer_size = sizeof(UINT8) * filesize * 5;
    for (i = 0; i < RETRY_TIMES; i++) {
        avail_ctx->buffer = (UINTPTR)malloc(total_buffer_size);
        if (avail_ctx->buffer) {
            break;
        }
        printf("malloc filesave_ctx buffer[%d] fail: %s\n", i, strerror(errno));
        continue;
    }
    avail_ctx->buffer_unit_size = filesize;
    avail_ctx->size = total_buffer_size;
    avail_ctx->written = 0;
    avail_ctx->last_index = 0;
    avail_ctx->curr_index = 0;
    avail_ctx->f_num = 0;
    (void)strncpy(avail_ctx->filesave_id, id, strlen(id));
    avail_ctx->f_out = fopen_new(avail_ctx->filesave_id, avail_ctx->f_num);

    return avail_ctx;
}

HD_RESULT filesave_uninit(void)
{
    UINT32 i;

    if (!filesave_is_enable) {
        return HD_OK;
    }
    
    filesave_trigger();
    filesave_is_enable = FALSE;
    // pthread_cancel(filesave_tid);
    pthread_join(filesave_tid, NULL);
    sem_destroy(&filesave_ready);

    for (i = 0; i < ARRAY_SIZE(filesave_ctx); i++) {
        if (filesave_ctx[i].buffer != 0 && filesave_ctx[i].in_use) {
            free((void *)filesave_ctx[i].buffer);
            fclose(filesave_ctx[i].f_out);
        }
        filesave_ctx[i].in_use = FALSE;
    }

    (void)memset(filesave_ctx, 0, sizeof(filesave_ctx));

    return HD_OK;
}
