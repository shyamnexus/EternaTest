#ifndef _MEMORY_520_H
#define _MEMORY_520_H

#define CC_CPU2_CPU1_CMDBUF_REG1 	0xF0160100//0xFE039000						//CC_CPU2_CPU1_CMDBUF_REG1 to store uboot starting address
#define CC_CPU2_CPU1_CMDBUF_REG2 	0xF0160104//0xFE039004 						//CC_CPU2_CPU1_CMDBUF_REG1 has used in core2_entry.s
#define CC_CORE2_WARM_RST_REG		0xF0160108
#define CC_CPU2_UBOOT_JUMP_REG 	 	0xF016010C//0xFE03900C
#define CC_CPU2_ENTRY_POLLING_REG	0xF0160110//0xFE039010						//core1~3 will jump to(except core0)
#define CC_CORE2_READY_REG          0xF0160114
#define CC_CORE3_READY_REG          0xF0160118
#define CC_CORE4_READY_REG          0xF016011C
#define CC_RC_EP_BOOT_COMM_REG      0xF0160120

#define NVT_CORE2_START 			CC_CPU2_CPU1_CMDBUF_REG2		   //core2_entry.S 's entry point

#endif