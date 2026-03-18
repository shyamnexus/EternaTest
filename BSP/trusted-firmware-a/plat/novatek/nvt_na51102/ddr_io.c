#include "ddr_para.h"

#define SWAP(a,b)	do { \
				a = ((a) ^ (b)); \
				b = ((a) ^ (b)); \
				a = ((a) ^ (b)); \
			} while (0)



uint8_t phy_io_read8(int phy_id, uint16_t addr)
{
	uint16_t page, offset;

	page = addr >> 8;
	offset = addr & 0xff;
	//OUTREG32(DDR_PHY_REG_BASE_ADDR + 0x3F8, page); //page select
	mmio_write_32(DDR_PHY_REG_BASE_ADDR + 0x3F8, page);
	/*ddr_log_phy_io("phy_io_read8 %d 0x%03x = 0x%02x\n", phy_id, addr,
			INREG32(DDR_PHY_REG_BASE_ADDR + (offset << 2)));*/
	//return INREG32(DDR_PHY_REG_BASE_ADDR + (offset << 2));
	return mmio_read_32(DDR_PHY_REG_BASE_ADDR + (offset << 2));
}

void phy_io_write8(int phy_id, uint16_t addr, uint32_t value)
{
	uint16_t page, offset;
	page = addr >> 8;
	offset = addr & 0xff;
	//ddr_log_phy_io("phy_io_write8 %d 0x%03x 0x%02x\n", phy_id, addr, value);
	//OUTREG32(DDR_PHY_REG_BASE_ADDR + 0x3F8, page); //page select
	mmio_write_32(DDR_PHY_REG_BASE_ADDR + 0x3F8, page);
	//OUTREG32(DDR_PHY_REG_BASE_ADDR + (offset << 2), value);
	mmio_write_32(DDR_PHY_REG_BASE_ADDR + (offset << 2), value);
}

int phy_mask_poll(int phy_id, uint64_t addr, uint32_t mask,
				uint32_t value, int timeout)
{
	int i = 0;
	do {
		if ((phy_io_read8(phy_id, addr) & mask) == value)
			return 0;

		udelay(1);

		/*timeout= 0: forever loop for checking */
		if (timeout != 0)
			i++;

		//printf("\npoll [0x%08x[%d:%d] == 0x%08x result == 0x%08x]\n",addr,bit_s,bit_e,val,BITMASK_GET(addr, bit_s, bit_e));

	} while (i <= timeout);

	return -1;
}


uint8_t phy_mask_get(int phy_id, uint64_t addr, uint32_t mask)
{
	return (phy_io_read8(phy_id, addr) & (mask));
}

void phy_mask_set(int phy_id, uint64_t addr, uint32_t mask,
							uint32_t val)
{
	phy_io_write8(phy_id, addr, (uint32_t)((phy_io_read8(phy_id, addr) & ~(mask))
							| ((val) & (mask))));
}

uint8_t maskget(uint64_t addr, uint32_t mask)
{
	return (mmio_read_32(addr) & mask);
}

void phy_mask_toggle(int phy_id, uint64_t addr, uint32_t mask)
{
	uint32_t tmp;
	tmp = phy_io_read8(phy_id, addr);
	phy_io_write8(phy_id, addr, (tmp & (~mask)) | (~(tmp & mask) & mask));
}

void maskset(uint64_t addr, uint32_t mask, uint32_t val)
{
	mmio_write_32(addr, (mmio_read_32(addr) & ~mask) | (val & mask));
}

uint8_t bitmask_get(uint64_t addr, uint32_t bit_high,
					uint32_t bit_low)
{
	uint32_t mask;

	if (bit_high < bit_low)
		SWAP(bit_high, bit_low);

	mask = BIT(bit_high) - BIT(bit_low) + BIT(bit_high);

	return (maskget(addr, mask) >> bit_low);
}

void bitmask_set(uint64_t addr, uint32_t bit_high, uint32_t bit_low,
					uint32_t val)
{
	uint32_t mask;

	if (bit_high < bit_low)
		SWAP(bit_high, bit_low);

	mask = BIT(bit_high) - BIT(bit_low) + BIT(bit_high);

    //printf("mask=0x%08x val= 0x%08x\n",mask,(val << bit_low));
	maskset(addr, mask, (val << bit_low));
}

int bitmask_poll(uint64_t addr, uint32_t bit_high, uint32_t bit_low,
					uint32_t val, uint32_t timeout)
{
	int ret = -1;
    unsigned int i = 0;

	do {
		if (bitmask_get(addr, bit_high, bit_low) == val) {
			ret = 0;
			break;
		}

		udelay(1);
		/*timeout= 0: forever loop for checking */
		if (timeout != 0)
			i++;
        //printf("\npoll addr=0x%08x val= 0x%08x\n",addr,val);
		//printf("\npoll [0x%08x[%d:%d] == 0x%08x result == 0x%08x]\n",addr,bit_s,bit_e,val,BITMASK_GET(addr, bit_s, bit_e));


	} while (i <= timeout);

	return ret;
}

