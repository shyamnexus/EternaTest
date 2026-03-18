/**
    NVT ACP function
    @file       nvt-acp.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_NVT_ACP_H
#define __SOC_NVT_NVT_ACP_H


#ifdef CONFIG_NVT_ACP
phys_addr_t nvtacp_get_address(phys_addr_t addr);
#else
static phys_addr_t inline nvtacp_get_address(phys_addr_t addr)
{
	return addr;
}
#endif

#endif /* __SOC_NVT_NVT_ACP_H */
