#ifndef _FLOW_UPDFW_H
#define _FLOW_UPDFW_H

//sync from uboot's xxxxx_utils.c
typedef struct _VERSION_CTRL {
	unsigned int fourcc;        // MAKEFOURCC 'V','E','R','C'
	unsigned int bank_id;       // 0 or 1 indicate uitron[0] or uitron[1]
	unsigned int fw_type;       // 0: debug, 1: release
	unsigned int fw_version;    // major: 1byte, minor 2byte
	unsigned int fw_date;       // date version
	unsigned int update_cnt;    // this device total upgrade counts
	unsigned char hash[1024];   // must 4 bytes alignment
	unsigned char reserved[64]; // must 4 bytes alignment
} VERSION_CTRL;

int flow_updfw_init_bank_info(void);
int flow_updfw(unsigned char *p_nvtpack, unsigned int size, BOOL *p_abort, VERSION_CTRL *p_verctrl);

#endif
