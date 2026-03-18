#ifndef _SEN_PROC_H_
#define _SEN_PROC_H_

typedef int (*DUMP_CB)(unsigned int sen_mode, 
						unsigned int fps, 
						unsigned int expt, 
						unsigned int gain, 
						unsigned int mirror, 
						unsigned int flip, 
						char *filepath);

int sen_proc_init(char *module_name, DUMP_CB dump_cb);
void sen_proc_exit(void);

#endif