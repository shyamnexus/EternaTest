
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
		UINT64                : 13;
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

//-----------------------------------------------------------------------------

UINT64 linked_list_null(UINT32 tab_idx)
{
	_linked_list_null_ ll_cmd;

	ll_cmd.dword = 0x0;

	ll_cmd.bit.table_index = tab_idx;
	ll_cmd.bit.cmd = 0x0;

	return ll_cmd.dword;
}
//-----------------------------------------------------------------------------

UINT64 linked_list_update(UINT32 byte_en, UINT32 reg_ofs, UINT32 reg_val)
{
	_linked_list_update_ ll_cmd;

	ll_cmd.dword = 0x0;

	ll_cmd.bit.byte_en = byte_en;
	ll_cmd.bit.reg_ofs = reg_ofs;
	ll_cmd.bit.reg_val = reg_val;
	ll_cmd.bit.cmd = 0x4;

	return ll_cmd.dword;
}
//-----------------------------------------------------------------------------

UINT64 linked_list_next_job(UINT32 next_job_addr, UINT32 tab_idx)
{
	_linked_list_next_job_ ll_cmd;

	ll_cmd.dword = 0x0;

	ll_cmd.bit.next_job_addr = next_job_addr;
	ll_cmd.bit.table_index = tab_idx;
	ll_cmd.bit.cmd = 0x1;

	return ll_cmd.dword;
}
//-----------------------------------------------------------------------------

UINT64 linked_list_next_update(UINT32 next_update_addr)
{
	_linked_list_next_update_ ll_cmd;

	ll_cmd.dword = 0x0;

	ll_cmd.bit.next_update_addr = next_update_addr;
	ll_cmd.bit.cmd = 0x2;

	return ll_cmd.dword;
}
//-----------------------------------------------------------------------------
