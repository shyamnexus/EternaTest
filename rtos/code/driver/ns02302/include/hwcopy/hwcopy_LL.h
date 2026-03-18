#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/type.h"
#include "kwrap/spinlock.h"
#include "kwrap/error_no.h"
#include "kwrap/debug.h"

#include "hwcopy_reg.h"
#include "hwcopy_int.h"
#include "hwcopy_compatible.h"
#include "hwcopy_platform.h"
#include "linked_list.h"


typedef union {
	struct {
		UINT64 table_index        : 8;      // bits : 7_0
		UINT64                    : 53;
		UINT64 cmd                : 3;      // bits : 63_61
	} bit;
	UINT64 dword;
} _linked_list_null_;

typedef union {
	struct {
		UINT64 reg_val        : 32;     // bits : 31_0
		UINT64 reg_ofs        : 12;     // bits : 43_32
		UINT64 byte_en        : 4;      // bits : 47_44
		UINT64                : 11;
		UINT64 ll_offset_en   : 1;      // bits : 59
		UINT64 special_update : 1;      // bits : 60
		UINT64 cmd            : 3;      // bits : 63_61
	} bit;
	UINT64 dword;
} _linked_list_update_;

typedef union {
	struct {
		UINT64 table_index          : 8;        // bits : 7_0
		UINT64 next_job_addr        : 32;       // bits : 39_8
		UINT64                      : 21;
		UINT64 cmd                  : 3;        // bits : 63_61
	} bit;
	UINT64 dword;
} _linked_list_next_job_;

typedef union {
	struct {
		UINT64                      : 8;
		UINT64 next_update_addr     : 32;       // bits : 39_8
		UINT64                      : 21;
		UINT64 cmd                  : 3;        // bits : 63_61
	} bit;
	UINT64 dword;
} _linked_list_next_update_;


typedef union _HWCPY_DSC
{
    UINT64 cmd[12];

    struct _cmd_context
    {
        _linked_list_update_    HWCPY_OP;
        _linked_list_update_    SRC_ADDR;
        _linked_list_update_    SRC_ADDR_MSB;

        _linked_list_update_    DST_ADDR;
        _linked_list_update_    DST_ADDR_MSB;

        _linked_list_update_    SRC_LOFF;

        _linked_list_update_    ACT_HIGHT;
        _linked_list_update_    ACT_WIDTH;

        _linked_list_update_    DST_LOFF;

        _linked_list_update_    DATA_LETH;
        _linked_list_update_    CTEX;

        _linked_list_next_job_  NEXT_DSC;   //if do not have next DSC this bit can be replaced by null cmd
        
    } cmd_context;

}HWCPY_DSC;



extern ER hwcopy_LL_flush_cache(HWCOPY_CHANNEL ch, PHWCOPY_REQUEST p_request);
extern ER hwcopy_LL_request(HWCOPY_CHANNEL ch, PHWCOPY_REQUEST p_request);
extern void hwcopy_LL_initial(void);
extern void assign_CMD_address(UINT32 address, BOOL isPa);

