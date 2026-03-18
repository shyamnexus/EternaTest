#ifdef ARM32
#include <arm32.h>
#else
#include <arm64.h>
#endif
#include <console.h>
#include <drivers/cdns_uart.h>
#include <drivers/gic.h>
#include <io.h>
#include <kernel/misc.h>
#include <kernel/panic.h>
#include <kernel/tz_ssvce_pl310.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <stdint.h>
#include <tee/entry_fast.h>
#include <tee/entry_std.h>
#include <kernel/thread.h>

#include "../include/smc_func_id/smc_id_def.h"


void tee_entry_fast(struct thread_smc_args *args)
{
	uint32_t rv;

	rv =  nvt_smc_fast(args);
	if (rv == OPTEE_SMC_RETURN_EBADCMD) {
		rv = custom_smc_fast(args);
		if (rv == OPTEE_SMC_RETURN_EBADCMD) {
			__tee_entry_fast(args);
		}
	}
}

uint32_t tee_entry_std(struct optee_msg_arg *arg, uint32_t num_params)
{
	uint32_t rv;

	rv = __tee_entry_std(arg, num_params);
	if (rv == OPTEE_SMC_RETURN_EBADCMD) {
		rv = nvt_smc_std(arg, num_params);
		if (rv == OPTEE_SMC_RETURN_EBADCMD) {
			rv = custom_smc_std(arg, num_params);
			if (rv == OPTEE_SMC_RETURN_EBADCMD) {
				EMSG("Unknown cmd 0x%x num_params:%d", arg->cmd,  num_params);
			}
		}
    }

    return rv;
}
