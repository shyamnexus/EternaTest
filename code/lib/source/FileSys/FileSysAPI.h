///////////////////////////////////////////////////////////////////
/**
    File system module.

    The file system supports a lot of file operations of one stoarge. Including file open/read/write/close/copy/delete ...\n
    It also supports the disk format or disk information access.\n
    File system task is a independent task, through the file system, it will call low level storage driver to acceess storage.\n
    If there are errors occured and function call will never return(This situation may happen while card error).\n
    User can close file system task and open again to re-initialize the file system.

    @file       FileSysAPI.h
    @ingroup    mILibFileSys
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.

*/

#ifndef _FILESYS_API_H
#define _FILESYS_API_H
#include "FileSysTsk.h"

/**
    @addtogroup mILibFileSys
*/
//@{

#if defined(__FREERTOS)
/**
     Install FileSys task,flag and semaphore id.

*/
extern void FsUitron_InstallID(void) _SECTION(".kercfg_text");

// --------------------------------------------------------------------
// storage operation
// --------------------------------------------------------------------

extern INT32    FsUitron_Open(CHAR Drive, FS_HANDLE pTarStrgDXH, FILE_TSK_INIT_PARAM *pInitParam);
extern INT32    FsUitron_Close(CHAR Drive, UINT32 TimeOut);

extern INT32    FsUitron_GetStrgObj(CHAR Drive, FS_HANDLE *pStrgDXH);
extern INT32    FsUitron_FormatAndLabel(CHAR Drive, FS_HANDLE pStrgDXH, BOOL ChgDisk, CHAR *pLabelName);
extern INT32    FsUitron_GetLabel(CHAR Drive, CHAR *pLabel);
extern INT32    FsUitron_ChangeDisk(CHAR Drive, FS_HANDLE pStrgDXH);

extern ER       FsUitron_SetParam(CHAR Drive, FST_PARM_ID param_id, UINT32 value);
extern UINT32   FsUitron_GetParam(CHAR Drive, FST_PARM_ID param_id, UINT32 parm1);

extern UINT64   FsUitron_GetDiskInfo(CHAR Drive, FST_INFO_ID info_id);

extern INT32    FsUitron_WaitFinish(CHAR Drive);
extern INT32    FsUitron_MakeDir(char *pPath);

extern INT32    FsUitron_ScanDir(char *pPath, FileSys_ScanDirCB DirCB, BOOL bGetlong, ULONG CBArg);
extern INT32    FsUitron_DelDirFiles(char *pPath, FileSys_DelDirCB CB);
extern INT32    FsUitron_LockDirFiles(char *pPath, BOOL isLock, FileSys_LockDirCB CB);

extern INT32    FsUitron_GetLongName(char *pPath, UINT16 *wFileName);

extern INT32    FsUitron_GetAttrib(char *pPath, UINT8 *pAttrib);
extern INT32    FsUitron_SetAttrib(char *pPath, UINT8 Attrib, BOOL bSet);

extern INT32    FsUitron_GetDateTime(char *pPath, UINT32  creDateTime[FST_FILE_DATETIME_MAX_ID], UINT32  modDateTime[FST_FILE_DATETIME_MAX_ID]);
extern INT32    FsUitron_SetDateTime(char *pPath, UINT32  creDateTime[FST_FILE_DATETIME_MAX_ID], UINT32  modDateTime[FST_FILE_DATETIME_MAX_ID]);


extern INT32    FsUitron_DeleteFile(char *pPath);
extern INT32    FsUitron_DeleteDir(char *pPath);

extern INT32    FsUitron_RenameFile(char *pNewname, char *pPath, BOOL bIsOverwrite);
extern INT32    FsUitron_RenameDir(char *pNewname, char *pPath, BOOL bIsOverwrite);


extern INT32    FsUitron_MoveFile(char *pSrcPath, char *pDstPath);
extern UINT64   FsUitron_GetFileLen(char *pPath);
extern void     FsUitron_RegisterCB(CHAR Drive, FileSys_CB CB);


extern FST_FILE  FsUitron_OpenFile(char *pPath, UINT32 Flag);
extern INT32    FsUitron_CloseFile(FST_FILE pFile);
extern INT32    FsUitron_ReadFile(FST_FILE pFile, UINT8 *pBuf, UINT32 *pBufSize, UINT32 Flag, FileSys_CB CB);
extern INT32    FsUitron_WriteFile(FST_FILE pFile, UINT8 *pBuf, UINT32 *pBufSize, UINT32 Flag, FileSys_CB CB);
extern INT32    FsUitron_SeekFile(FST_FILE pFile, UINT64 offset, FST_SEEK_CMD fromwhere);
extern UINT64   FsUitron_TellFile(FST_FILE pFile);
extern INT32    FsUitron_FlushFile(FST_FILE pFile);
extern INT32    FsUitron_StatFile(FST_FILE pFile, FST_FILE_STATUS *pFileStat);
extern INT32    FsUitron_TruncFile(FST_FILE pFile, UINT64 NewSize);
extern INT32    FsUitron_AllocFile(FST_FILE pFile, UINT64 NewSize, UINT32 Reserved1, UINT32 Reserved2);

extern INT32    FsUitron_CopyToByName(COPYTO_BYNAME_INFO *pCopyInfo);
extern void     FsUitron_InstallCmd(void);
extern INT32    FsUitron_Benchmark(CHAR Drive, FS_HANDLE StrgDXH, UINT8 *pBuf, UINT32 BufSize);

extern FS_SEARCH_HDL FsUitron_SearchFileOpen(CHAR *pPath, FIND_DATA *pFindData, int Direction, UINT16 *pLongFilename);
extern int FsUitron_SearchFile(FS_SEARCH_HDL pSearch, FIND_DATA *pFindData, int Direction, UINT16 *pLongFilename);
extern int FsUitron_SearchFileClose(FS_SEARCH_HDL pSearch);
extern int FsUitron_SearchFileRewind(FS_SEARCH_HDL pSearch);

extern CHAR *FsUitron_GetCwd(void);
extern VOID FsUitron_SetCwd(const CHAR *pPath);
extern int FsUitron_GetParentDir(const CHAR *pPath, CHAR *parentDir);
extern int FsUitron_ChDir(CHAR *pPath);

extern INT32 FsUitron_DeleteMultiFiles(FST_MULTI_FILES *pMultiFiles);
#endif

#if defined(__LINUX_USER__)
/**
    Install FsLinux flag and semaphore id.

    @return void.
*/
extern void FsLinux_InstallID(void) _SECTION(".kercfg_text");

/**
    Open FsLinux task.

    This is FsLinux initialization function.

    @return
        - @b FST_STA_OK if success, others fail
*/
extern INT32 FsLinux_Open(CHAR Drive, FS_HANDLE StrgDXH, FILE_TSK_INIT_PARAM *pInitParam);

/**
    Close FsLinux task.

    @return
        - @b FST_STA_OK if success, others fail
*/
extern INT32 FsLinux_Close(CHAR Drive, UINT32 TimeOut);


extern FST_FILE FsLinux_OpenFile(CHAR *pPath, UINT32 flag);
extern INT32 FsLinux_CloseFile(FST_FILE pFile);
extern INT32 FsLinux_ReadFile(FST_FILE pFile, UINT8 *pBuf, UINT32 *pBufSize, UINT32 Flag, FileSys_CB CB);
extern INT32 FsLinux_WriteFile(FST_FILE pFile, UINT8 *pBuf, UINT32 *pBufSize, UINT32 Flag, FileSys_CB CB);
extern INT32 FsLinux_WaitFinish(CHAR Drive);

extern INT32 FsLinux_SeekFile(FST_FILE pFile, UINT64 offset, FST_SEEK_CMD fromwhere);
extern UINT64 FsLinux_TellFile(FST_FILE pFile);
extern INT32 FsLinux_DeleteFile(CHAR *pPath);
extern INT32 FsLinux_DeleteMultiFiles(FST_MULTI_FILES *pMultiFiles);
extern INT32 FsLinux_StatFile(FST_FILE pFile, FST_FILE_STATUS *pFileStat);
extern INT32 FsLinux_FlushFile(FST_FILE pFile);
extern INT32 FsLinux_TruncFile(FST_FILE pFile, UINT64 Size);
extern INT32 FsLinux_AllocFile(FST_FILE pFile, UINT64 Size, UINT32 Reserved1, UINT32 Reserved2);
extern UINT64 FsLinux_GetFileLen(char *pPath);

extern INT32 FsLinux_GetDateTime(char *pPath, UINT32 creDateTime[FST_FILE_DATETIME_MAX_ID], UINT32 modDateTime[FST_FILE_DATETIME_MAX_ID]);
extern INT32 FsLinux_SetDateTime(char *pPath, UINT32 creDateTime[FST_FILE_DATETIME_MAX_ID], UINT32 modDateTime[FST_FILE_DATETIME_MAX_ID]);

extern INT32 FsLinux_MakeDir(CHAR *pPath);
extern INT32 FsLinux_DeleteDir(CHAR *pPath);

extern INT32 FsLinux_MoveFile(CHAR *pSrcPath, CHAR *pDstPath);
extern INT32 FsLinux_RenameFile(CHAR *pNewname, CHAR *pPath, BOOL bIsOverwrite);
extern INT32 FsLinux_RenameDir(CHAR *pNewname, CHAR *pPath, BOOL bIsOverwrite);

extern FS_SEARCH_HDL FsLinux_SearchFileOpen(CHAR *pPath, FIND_DATA *pFindData, int Direction, UINT16 *pLongFilename);
extern int FsLinux_SearchFile(FS_SEARCH_HDL pSearch, FIND_DATA *pFindData, int Direction, UINT16 *pLongFilename);
extern int FsLinux_SearchFileClose(FS_SEARCH_HDL pSearch);
extern int FsLinux_SearchFileRewind(FS_SEARCH_HDL pSearch);

extern INT32 FsLinux_GetAttrib(CHAR *pPath, UINT8 *pAttrib);
extern INT32 FsLinux_SetAttrib(CHAR *pPath, UINT8 Attrib, BOOL bSet);

extern INT32 FsLinux_CopyToByName(COPYTO_BYNAME_INFO *pCopyInfo);

extern CHAR *FsLinux_GetCwd(void);
extern VOID FsLinux_SetCwd(const CHAR *pPath);
extern int FsLinux_GetParentDir(const CHAR *pPath, CHAR *parentDir);
extern int FsLinux_ChDir(CHAR *pPath);

extern UINT32 FsLinux_GetParam(CHAR Drive, FST_PARM_ID param_id, UINT32 parm1);

extern INT32 FsLinux_ScanDir(char *pPath, FileSys_ScanDirCB DirCB, BOOL bGetlong, ULONG CBArg);
extern INT32 FsLinux_DelDirFiles(char *pPath, FileSys_DelDirCB DelCB);
extern INT32 FsLinux_LockDirFiles(char *pPath, BOOL isLock, FileSys_LockDirCB ApplyCB);

extern UINT64 FsLinux_GetDiskInfo(CHAR Drive, FST_INFO_ID InfoId);
extern INT32 FsLinux_Benchmark(CHAR Drive, FS_HANDLE StrgDXH, UINT8 *pBuf, UINT32 BufSize);
extern INT32 FsLinux_GetStrgObj(CHAR Drive, FS_HANDLE *pStrgDXH);
extern INT32 FsLinux_ChangeDisk(CHAR Drive, FS_HANDLE StrgDXH);
extern INT32 FsLinux_FormatAndLabel(CHAR Drive, FS_HANDLE pStrgDXH, BOOL ChgDisk, CHAR *pLabelName);
extern INT32 FsLinux_GetLabel(CHAR Drive, CHAR *pLabel);

//obselete functions
extern ER    FsLinux_SetParam(CHAR Drive, FST_PARM_ID param_id, UINT32 value);
extern INT32 FsLinux_GetLongName(CHAR *pPath, UINT16 *wFileName);
extern void  FsLinux_RegisterCB(CHAR Drive, FileSys_CB CB);
#endif

//@}
#endif /* _FILESYS_API_H  */