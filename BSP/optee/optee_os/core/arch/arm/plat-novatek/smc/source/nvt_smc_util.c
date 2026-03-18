#include "../include/nvt_smc_util.h"
#include <kernel/tz_ssvce_pl310.h>
#include <kernel/cache_helpers.h>
#include <kernel/thread.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <sm/optee_smc.h>


uintptr_t nvt_smc_pa_to_va(uintptr_t pa, uintptr_t size)
{
        if(core_pbuf_is(CORE_MEM_NSEC_SHM,pa,size))
        {
                return (uintptr_t)phys_to_virt(pa, MEM_AREA_NSEC_SHM, size);
        }
        else{
                EMSG("pa not in CORE_MEM_NSEC_SHM  pa%lx size:%lx\n",pa,size);
                return 0;
        }
}

