#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NVTSL_DEFAULT_PORT 3034

/// @brief Init 
/// @param  
/// @return 0: suceeded, others: failed
int nvtsl_init(int port);

/// @brief Close
/// @param  
/// @return 0: suceeded, others: failed
int nvtsl_close(void);

typedef enum 
{
    StitchingCommand_Snapshot = 0,
    StitchingCommand_Switch,
    StitchingCommand_Distance
} StitchingCommand;

/// @brief Callback for stitching lib
typedef int (*StitchingCallbackFunc)(StitchingCommand nCmd, char *strParam);

/// @brief Set callback for stitching lib
/// @param callback 
/// @return 0: suceeded, others: failed
int nvtsl_setCallBack(StitchingCallbackFunc callback);


/// @brief Send snapshot to PC
/// @param strName snapshot name
/// @param buffer 
/// @param len 
/// @return 0: suceeded, others: failed
int nvtsl_sendSnapshot(char *strName, char *buffer, int len);


/// @brief copy file to parent dir
int copy_files_to_parent_dir(const char* dir_path);