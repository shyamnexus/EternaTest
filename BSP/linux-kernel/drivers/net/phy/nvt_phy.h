/*
 * drivers/net/phy/nvt_phy.h
 *
 * Novatek PHYs register
 *
 * Copyright (c) 2023 Novatek Microelectronics Corp. All rights reserved.
 *
 */

//#define HW_RESET_ADDRESS     0xFD020088
//#define HW_RESET_MASK        0xC0000000
#define ETH_TO_PHY_OFFSET 0x3000
#define PHY_BASE_SIZE 400

#define SW_RESET_ADDRESS     (NVT_PHY_BASE + 0x800)
#define SW_RESET_MASK        0x00000002
#define PLLREGWR_MASK        0x00000008

#define EQ_RESET_ADDRESS     (NVT_PHY_BASE + 0x818)
#define EQ_RESET_MASK        0x00000002

#if defined(CONFIG_NVT_IVOT_PLAT_NS02302) || defined(CONFIG_NVT_IVOT_PLAT_NS02301)
#define NVT_ETH_TRIM_APPLY
#define NVT_ETH_TX_TRIM_ROUT 0
#define NVT_ETH_TX_TRIM_DAC 0
#define NVT_ETH_TX_TRIM_SEL_RX 3
#define NVT_ETH_TX_TRIM_SEL_TX 3
#endif

#define NVT_TRIM_RANGE_CHECK(min, max, trim) ((trim) >= (min) && (trim) <= (max) ? 1 : 0)

#define PHY_REGISTER_READ(data,addr) do { \
        data = ioread32((void *)addr); \
}while(0)
/*
#define phy_hw_reset_enable() do { \
        unsigned long v; \
        v = ioread32((void *)HW_RESET_ADDRESS); \
        v &= ~(HW_RESET_MASK); \
        iowrite32(v, (void *)HW_RESET_ADDRESS);\
}while(0)

#define phy_hw_reset_disable() do { \
        unsigned long v; \
        v = ioread32((void *)HW_RESET_ADDRESS); \
        v |= HW_RESET_MASK; \
        iowrite32(v, (void *)HW_RESET_ADDRESS);\
}while(0)
*/
#define phy_sw_reset_enable() do { \
        unsigned long v; \
        v = ioread32((void *)SW_RESET_ADDRESS); \
        v |= SW_RESET_MASK; \
        iowrite32(v, (void *)SW_RESET_ADDRESS);\
}while(0)

#define phy_sw_reset_disable() do { \
        unsigned long v; \
        v = ioread32((void *)SW_RESET_ADDRESS); \
        v |= PLLREGWR_MASK; \
        v &= ~(SW_RESET_MASK); \
        iowrite32(v, (void *)SW_RESET_ADDRESS);\
}while(0)

#define eq_reset_enable() do { \
        unsigned long v; \
        v = ioread32((void *)EQ_RESET_ADDRESS); \
        v |= EQ_RESET_MASK; \
        iowrite32(v, (void *)EQ_RESET_ADDRESS);\
}while(0)

#define eq_reset_disable() do { \
        unsigned long v; \
        v = ioread32((void *)EQ_RESET_ADDRESS); \
        v &= ~(EQ_RESET_MASK); \
        iowrite32(v, (void *)EQ_RESET_ADDRESS);\
}while(0)

#define set_break_link_timer() do { \
        iowrite32(0x53, (void *)(NVT_PHY_BASE + 0xA88));\
        iowrite32(0x07, (void *)(NVT_PHY_BASE + 0xA8C));\
        iowrite32(0xC9, (void *)(NVT_PHY_BASE + 0xA84));\
}while(0)

#define set_one_led_link_act() do { \
        iowrite32(0x40, (void *)(NVT_PHY_BASE + 0x900));\
        iowrite32(0x300, (void *)(NVT_PHY_BASE + 0x00C));\
}while(0)

#define set_two_leds_link_act() do { \
        iowrite32(0x40, (void *)(NVT_PHY_BASE + 0x900));\
        iowrite32(0x312, (void *)(NVT_PHY_BASE + 0x00C));\
}while(0)

#define set_led_inv() do { \
        unsigned long v; \
        v = ioread32((void *)(NVT_PHY_BASE + 0x00C)); \
	v &= ~(0x3<<8); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0x00C));\
} while (0)

#define set_best_setting_t22() do { \
        unsigned long v; \
        v = ioread32((void *)NVT_PHY_BASE + 0xB74); \
        v &= ~(0x07); \
        v |= (0x05); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xB74));\
}while(0)


#define set_best_setting_t28() do { \
        unsigned long v; \
        v = ioread32((void *)(NVT_PHY_BASE + 0xAF8)); \
        v &= ~(0x07); \
        v |= (0x03); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xAF8));\
        v = ioread32((void *)(NVT_PHY_BASE + 0xB54)); \
        v |= (1<<7); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xB54));\
        v = ioread32((void *)(NVT_PHY_BASE + 0xB58)); \
        v |= (0x3<<4); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xB58));\
}while(0)


#define set_am_option() do { \
        unsigned long v; \
        v = ioread32((void *)NVT_PHY_BASE + 0xA98); \
        v &= ~(0x03); \
        v |= (0x03); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA98));\
	v = ioread32((void *)NVT_PHY_BASE + 0x9FC); \
        v &= ~(0x03); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0x9FC));\
	v = ioread32((void *)NVT_PHY_BASE + 0xAB4); \
        v &= ~(0x70); \
	v |= (0x20); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xAB4));\
	v = ioread32((void *)NVT_PHY_BASE + 0xAB8); \
        v |= (0x73); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xAB8));\
	v = ioread32((void *)NVT_PHY_BASE + 0xA14); \
        v |= (0x03); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA14));\
	v = ioread32((void *)NVT_PHY_BASE + 0xA18); \
        v &= ~(0x03); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA18));\
	v = ioread32((void *)NVT_PHY_BASE + 0xA1C); \
        v &= ~(0xF0); \
	v |= (0x10); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA1C));\
	v = ioread32((void *)NVT_PHY_BASE + 0xA20); \
        v &= ~(0xFF); \
        v |= (0x32); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA20));\
	v = ioread32((void *)NVT_PHY_BASE + 0xA24); \
        v &= ~(0xFF); \
        v |= (0x43); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA24));\
	v = ioread32((void *)NVT_PHY_BASE + 0xA28); \
        v &= ~(0xFF); \
        v |= (0x55); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA28));\
	v = ioread32((void *)NVT_PHY_BASE + 0xA2C); \
        v &= ~(0xFF); \
        v |= (0x55); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA2C));\
	v = ioread32((void *)NVT_PHY_BASE + 0xA30); \
        v &= ~(0xFF); \
        v |= (0x55); \
        iowrite32(v, (void *)(NVT_PHY_BASE + 0xA30));\
}while(0)

