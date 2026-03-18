#ifndef _DPC_H_
#define _DPC_H_

typedef enum _DPC_STATUS {
    DPC_NONE = 0,
    DPC_BRIGHT_DONE,
    DPC_DARK_DONE,
    DPC_BOTH_DONE,
} DPC_STATUS;

extern DPC_STATUS dpc_get_status(void);
extern ER read_dpc_config(UINT8 *buf);
extern INT32 dpc_bright_start(void);
extern INT32 dpc_dark_start(void);

#endif
