#include <kernel/thread.h>
#include <optee_msg.h>
#include "../include/smc_func_id/smc_id_def.h"

uint32_t custom_smc_fast(struct thread_smc_args *args)
{
	uint32_t ret = 0;

	switch (args->a0) {



	default:
		ret = OPTEE_SMC_RETURN_EBADCMD;
	}

	return ret;
}


uint32_t custom_smc_std(struct optee_msg_arg *arg, uint32_t num_params __maybe_unused)
{
	uint32_t rv = OPTEE_SMC_RETURN_OK;

	switch (arg->cmd) {


	default:
		//EMSG("Unknown cmd 0x%x", arg->cmd);
		rv = OPTEE_SMC_RETURN_EBADCMD;
	}

	return rv;
}

