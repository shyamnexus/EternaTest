#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
//#include <nvt_type.h>
#include <sys/mman.h>

#define CHKPNT    printf("\033[37mCHK: %d, %s\033[0m\r\n",__LINE__,__func__) ///< Show a color sting of line count and function name in your insert codes
#define DBG_WRN(fmtstr, args...) printf("\033[33m%s(): \033[0m" fmtstr,__func__, ##args)
#define DBG_ERR(fmtstr, args...) printf("\033[31m%s(): \033[0m" fmtstr,__func__, ##args)

#define MEM_VERSION "1.00.04"

#if 1
#define DBG_IND(fmtstr, args...)
#else
#define DBG_IND(fmtstr, args...) printf("%s(): " fmtstr, \
    __func__, ##args)
#endif

#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)


static int fd = 0;
char    str_dumpmem[64];

void* mem_mmap(int fd, size_t mapped_size, uintptr_t phy_addr)
{
	void *map_base = NULL;
	unsigned int page_size = 0;

	page_size = getpagesize();
	map_base = mmap(NULL,
			mapped_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			phy_addr & ~(uintptr_t)(page_size - 1));

	if (map_base == MAP_FAILED)
		return NULL;

	return map_base;
}

int mem_munmap(void* map_base, size_t mapped_size)
{
	if (munmap(map_base, mapped_size) == -1)
		return -1;

	return 0;
}

void debug_dumpmem(uintptr_t vir_addr, uintptr_t phy_addr, size_t length)
{
	uint32_t i, j, k, str_len, val;
	char out_ch;
	uint32_t out_ch_arr[4];

	DBG_DUMP("dump phy_addr=%lx , vir_addr=0x%lx, length=0x%lx to console:\r\n", (unsigned long)phy_addr, (unsigned long)vir_addr, length);

	for (i = 0; i < length; )
	{
		/* Address maybe 32 or 64 bits, data only support 32bits. */
		str_len=snprintf(str_dumpmem, 64, "%lx : %08X %08X %08X %08X  ", (unsigned long)(phy_addr+i), *((volatile unsigned int*)vir_addr),
				*((volatile unsigned int*)(vir_addr+4)),*((volatile unsigned int*)(vir_addr+8)), *((volatile unsigned int*)(vir_addr+12)));
		#if 1
		for (j=0;j<16;j+=4)
		{
			val = *(volatile unsigned int*)(vir_addr+j);
			out_ch_arr[3] = (val & 0xff000000) >> 24;
			out_ch_arr[2] = (val & 0xff0000) >> 16;
			out_ch_arr[1] = (val & 0xff00) >> 8;
			out_ch_arr[0] = (val & 0xff);
			k = 0;
			while (k < 4)
			{
				#if 1
				out_ch = (char)out_ch_arr[k];
				if ((out_ch<0x20) || (out_ch>=0x80))
					str_len+=snprintf(str_dumpmem+str_len, 64-str_len, ".");
				else
					str_len+=snprintf(str_dumpmem+str_len, 64-str_len, "%c",out_ch);

				#endif
				k++;
			}

		}
		#else
		for (j=0;j<16;j++)
                {
                       out_ch = *((char *)(vir_addr+j));
                       if (((UINT32)out_ch<0x20) || ((UINT32)out_ch>=0x80))
                               str_len+=snprintf(str_dumpmem+str_len, 64-str_len, ".");
                       else
                               str_len+=snprintf(str_dumpmem+str_len, 64-str_len, "%c",out_ch);
		}
		#endif
		DBG_DUMP("%s", str_dumpmem);
		DBG_DUMP("\r\n");
		i+=16;
		vir_addr+=16;
	}

	DBG_DUMP("\r\n\r\n");
}

void debug_dumpmem2file(uintptr_t addr, size_t length, char* filename)
{
	int fd;
	int flags = O_RDWR|O_CREAT|O_TRUNC;
	fd = open(filename, flags, 0777);
	if (fd < 0)
	{
		DBG_ERR("create file %s fail errno=%d\r\n",filename,errno);
		return;
	}
	if (length == 0) {
		close(fd);
		return;
	}

	if(write(fd,(void*)addr,length) < 0){
		DBG_ERR("fail to write %ld bytes\r\n", length);
	}
	close(fd);
}

void cmd_mem_w(uintptr_t addr, uint32_t data)
{
	void*  map_addr;
	uintptr_t virtual_addr, length = 0x100;
	uintptr_t page_align_addr, map_offset;
	size_t map_size;

	if (addr & 0x3) {
		DBG_DUMP("Address must be 4 bytes align:%p\n", (void *)addr);
		return;
	}

	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	map_offset = addr - page_align_addr;
	map_size = map_offset + length;

	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if (map_addr == NULL) {
		DBG_ERR("mmap error for addr %p\r\n", (void *)addr);
		return;
	}
	virtual_addr =((uintptr_t)map_addr + map_offset);

	/* The write unit is 4 bytes */
	*((unsigned int*) virtual_addr)=data;
	DBG_DUMP("addr = 0x%08lx, data = 0x%08X\r\n", (unsigned long)addr, data);
	mem_munmap(map_addr, map_size);
}

void cmd_bit_w(uintptr_t addr, uint32_t bit_num, uint32_t val)
{
	void*  map_addr;
	uintptr_t virtual_addr, length = 0x100;
	uintptr_t page_align_addr, map_offset;
	size_t map_size;

	if (addr & 0x3) {
		DBG_DUMP("Address must be 4 bytes align:%p\n", (void *)addr);
		return;
	}

	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	map_offset = addr - page_align_addr;
	map_size = map_offset + length;

	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if (map_addr == NULL) {
		DBG_ERR("mmap error for addr 0x%lx\r\n", addr);
		return;
	}
	virtual_addr = (uintptr_t)map_addr + map_offset;
	DBG_DUMP("\r\taddr = 0x%08lx, data = 0x%08X\r\n", (unsigned long)addr, *((uint32_t*) virtual_addr));
	*((unsigned int*) virtual_addr)&= ~(1<<bit_num);
	*((unsigned int*) virtual_addr)|= (val<<bit_num);
	if (val == 0)
		DBG_DUMP("\rAfter clear Bit#%u:\n", bit_num);
	else
		DBG_DUMP("\rAfter set Bit#%u:\n", bit_num);

	DBG_DUMP("\r\taddr = 0x%08lx, data = 0x%08X\r\n", (unsigned long)addr, *((uint32_t*) virtual_addr));
	mem_munmap(map_addr, map_size);
}

void cmd_mbit_w(uintptr_t addr, uint32_t bit_start, uint32_t bit_end, uint32_t val)
{
	void*  map_addr;
	uintptr_t virtual_addr, length = 0x100;
	uintptr_t page_align_addr, map_offset;
	size_t map_size;
	uint32_t bit_rst = 0, i;

	if (addr & 0x3) {
		DBG_DUMP("Address must be 4 bytes align:%p\n", (void *)addr);
		return;
	}

	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	map_offset = addr - page_align_addr;
	map_size = map_offset + length;

	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if (map_addr == NULL) {
		DBG_ERR("mmap error for addr 0x%lx\r\n", addr);
		return;
	}
	virtual_addr = (uintptr_t)map_addr + map_offset;
	DBG_DUMP("\r\taddr = 0x%08lx, data = 0x%08X\r\n", (unsigned long)addr, *((uint32_t*) virtual_addr));
	for (i = bit_start; i <= bit_end; i++) {
		bit_rst |= (1 << i);
	}
	*((unsigned int*) virtual_addr)&= ~bit_rst;
	*((unsigned int*) virtual_addr)|= (val<<bit_start);
	DBG_DUMP("\rAfter set Bit#[%u:%u]=0x%x\n", bit_end,bit_start,val);

	DBG_DUMP("\r\taddr = 0x%08lx, data = 0x%08X\r\n", (unsigned long)addr, *((uint32_t*) virtual_addr));
	mem_munmap(map_addr, map_size);
}

void cmd_mem_r(uintptr_t addr, size_t length)
{
	uintptr_t virtual_addr;
	void*  map_addr;
	uintptr_t page_align_addr, map_offset;
	size_t map_size;

	if (addr & 0x3) {
		DBG_DUMP("Address must be 4 bytes align:0x%lx\n", addr);
		return;
	}

	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);

	map_offset = addr - page_align_addr;
	map_size = map_offset + length;

	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if (map_addr == NULL) {
		DBG_ERR("mmap error for addr 0x%lx\r\n", addr);
		return;
	}

	DBG_DUMP("addr:0x%lx map_addr:%p, map_size = %ld\r\n", (unsigned long)addr, map_addr, map_size);
	virtual_addr = (uintptr_t)map_addr + map_offset;

	debug_dumpmem(virtual_addr, addr, length);
	mem_munmap(map_addr, map_size);
}
void cmd_mem_fill(uintptr_t addr, size_t length, uint32_t data)
{
	uintptr_t virtual_addr;
	void*  map_addr;
	uintptr_t page_align_addr, map_offset;
	size_t map_size;

	if (addr & 0xF) {
		DBG_DUMP("Address must be 16 bytes align:0x%lx\n", (unsigned long)addr);
		return;
	}

	if (length & 0xF) {
		DBG_DUMP("length must be divided by 16 len:0x%lx\n", length);
		return;
	}

	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	map_offset = addr - page_align_addr;
	map_size = map_offset + length;

	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if (map_addr == NULL) {
		DBG_ERR("mmap error for addr 0x%lx\r\n", addr);
		return;
	}
	virtual_addr = (uintptr_t) map_addr + map_offset;
	DBG_DUMP("addr = 0x%lx mapping_addr: 0x%lx, length = 0x%lx, data = 0x%02X\r\n", (unsigned long)addr, (unsigned long)virtual_addr, length, data);

	memset((void*)virtual_addr, data, (size_t)length);

	mem_munmap(map_addr, map_size);

}

int cmd_mem_fillfile(uintptr_t addr, size_t length, char* filename)
{
	uintptr_t virtual_addr;
	void*  map_addr;
	uintptr_t page_align_addr, map_offset;
	size_t size = 0, map_size = 0;
	FILE *file = NULL;
	struct stat st;
	int ret = 0;

	if (addr & 0x7) {
		DBG_DUMP("Address must be divided by 8 addr:0x%lx\n", (unsigned long)addr);
		return -1;
	}

	file = fopen(filename, "rb");
	if (!file) {
		DBG_ERR("create file %s fail errno=%d\r\n", filename, errno);
		return -1;
	}
	ret = stat(filename, &st);
	if (ret < 0) {
		fclose(file);
		return -1;
	} else {
		if (length == 0)
			length = st.st_size;
	}

	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	map_offset = addr - page_align_addr;
	map_size = map_offset + length;
	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if (map_addr == NULL) {
		DBG_ERR("mmap error for addr 0x%lx\r\n", (unsigned long)addr);
		fclose(file);
		return -1;
	}
	virtual_addr = (uintptr_t)map_addr + map_offset;
	size = fread((void*)virtual_addr, 1, length, file);
	if (size != length) {
		DBG_ERR("error read with size %ld\n", (unsigned long)size);
	}
	fclose(file);
	mem_munmap(map_addr, map_size);
	return 0;
}

void cmd_mem_dump(uintptr_t addr, size_t length, char* filename)
{
	uintptr_t virtual_addr;
	void*  map_addr;
	uintptr_t page_align_addr, map_offset;
	size_t map_size = 0;

	if (addr & 0x3) {
		DBG_DUMP("Address must be 4 bytes align:0x%lx\n", (unsigned long)addr);
		return;
	}

	if (length == 0) {
		return;
	}

	DBG_DUMP("dump addr = 0x%lx, length = %ld, filename = %s\r\n", (unsigned long)addr, length, filename);
	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	map_offset = addr - page_align_addr;
	map_size = map_offset + length;

	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if (map_addr == NULL) {
		DBG_ERR("mmap error for addr 0x%lx\r\n", (unsigned long)addr);
		return;
	}
	virtual_addr = (uintptr_t)map_addr + map_offset;
	debug_dumpmem2file(virtual_addr, length, filename);
	mem_munmap(map_addr, map_size);
}

void show_help(void)
{
	DBG_DUMP("Help:\r\n");
	DBG_DUMP("mem version:%s\r\n", MEM_VERSION);
	DBG_DUMP("\tw    [addr] [data]                  Write a word into memory\r\n");
	DBG_DUMP("\tr    [addr] (length)                Read a region of memory\r\n");
	DBG_DUMP("\tfill [addr] [length] (data)         Fill a region of memory with date value\r\n");
	DBG_DUMP("\tfillfile [addr] [length] (filename) Read file data and fill them into memory\r\n");
	DBG_DUMP("\tdump [addr] (length) (filename)     Dump a region of memory to a file\r\n");
	DBG_DUMP("\tbit [addr] (bit Num)(val)           To do the specific address bit operation with value 0(clear) or 1(set)\r\n");
	DBG_DUMP("\tmbit [addr] (bit_start)(bit_end)(val)  To do the specific address bit from bit_start to bit_end operation with value\r\n");
	DBG_DUMP("\te.g.\r\n");
	DBG_DUMP("\t\tmem r 0x2000 0x200\r\n");
	DBG_DUMP("\t\tmem w 0x2000 0x2c00\r\n");
	DBG_DUMP("\t\tmem fillfile 0x17000000 0 image.bin  --- Read image.bin into memory address 0x17000000\r\n");
	DBG_DUMP("\t\tmem bit 0x17000000 20 1              --- To set(1)/clear(0) bit20\r\n");
	DBG_DUMP("\t\tmem mbit 0x17000000 17 20 0x6        --- To set bit[20:17] with 0x6\r\n");
}

int init_mem_dev(void)
{
	int fd = 0;
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	return fd;
}


void uninit_mem_dev(int fd)
{
	close(fd);
	return;
}

int main(int argc, char *argv[])
{
	int ret = 0, i = 1;

	DBG_IND("argc=%d\r\n",argc);

	fd = init_mem_dev();
	if (fd < 0) {
		DBG_ERR("Open /dev/mem device failed\n");
		return -1;
	}

	if (argc <= 1) {
		show_help();
	} else {
		while (i < argc) {
			if (!strcmp(argv[i], "?")) {
				show_help();
				break;
			}
			if (!strcmp(argv[i], "r")) {
				unsigned long  addr = 0;
				size_t length = 256;
				if (argc<3)
				{
					DBG_DUMP("syntex: mem r [addr] (length)\r\n");
					break;
				}
				i++;
				if (i < argc)
					sscanf(argv[i++], "%lx", &addr);
				if (i < argc)
					sscanf(argv[i++], "%lx", &length);
				cmd_mem_r(addr,length);
				break;
			}
			if (!strcmp(argv[i], "w")) {
				unsigned long  addr = 0, data = 0;

				if (argc!=4)
				{
					DBG_DUMP("syntex: mem w [addr] [data]\r\n");
					break;
				}
				i++;
				if (i < argc)
					sscanf(argv[i++], "%lx", &addr);
				if (i < argc)
					sscanf(argv[i++], "%lx", &data);
				cmd_mem_w(addr,data);
				break;
			}
			if (!strcmp(argv[i], "fill")) {
				unsigned long  addr = 0, data = 0;
				size_t length = 0;
				if (argc < 4)
				{
					DBG_DUMP("syntex: mem fill [addr] [length] (data)\r\n");
					break;
				}
				i++;
				if (i < argc)
					sscanf(argv[i++], "%lx", &addr);
				if (i < argc)
					sscanf(argv[i++], "%lx", &length);
				if (i < argc)
					sscanf(argv[i++], "%lx", &data);
				cmd_mem_fill(addr,length,data);
				break;
			}
			if (!strcmp(argv[i], "fillfile")) {
				unsigned long  addr = 0;
				size_t length = 0;
				char    filename[256]="/mnt/sd/fill.bin";

				if (argc < 3)
				{
					DBG_DUMP("syntex: mem fillfile [addr] [length] (filename)\r\n");
					break;
				}
				i++;
				if (i < argc)
					sscanf(argv[i++], "%lx", &addr);
				if (i < argc)
					sscanf(argv[i++], "%lx", &length);
				if (i < argc)
					sscanf(argv[i++], "%s", filename);
				ret = cmd_mem_fillfile(addr,length,filename);
				if (ret < 0)
					return ret;
				break;
			}
			if (!strcmp(argv[i], "dump")) {
				unsigned long  addr = 0;
				size_t length = 256;
				char    filename[256]="/mnt/sd/dump.bin";

				if (argc < 3)
				{
					DBG_DUMP("syntex: mem dump [addr] (length) (filename)\r\n");
					break;
				}
				i++;
				if (i < argc)
					sscanf(argv[i++], "%lx", &addr);
				if (i < argc)
					sscanf(argv[i++], "%lx", &length);
				if (i < argc)
					sscanf(argv[i++], "%s", filename);
				if (length == 0)
					return -1;
				cmd_mem_dump(addr,length,filename);
				break;
			}
			if (!strcmp(argv[i], "bit")) {
				unsigned long  addr = 0, bit_num = 0, val = 0;

				if (argc!=5)
				{
					DBG_DUMP("syntex: mem bit [addr] [bit#num] [1/0] \r\n");
					break;
				}
				i++;
				if (i < argc)
					sscanf(argv[i++], "%lx", &addr);
				if (i < argc)
					sscanf(argv[i++], "%lu", &bit_num);
				if (i < argc)
					sscanf(argv[i++], "%lu", &val);
				if (val > 1) {
					DBG_DUMP("syntex: mem bit [addr] [bit#num] [1/0] \r\n");
					break;
				}
				cmd_bit_w(addr,bit_num, val);
				break;
			}
			if (!strcmp(argv[i], "mbit")) {
				uintptr_t addr = 0;
				unsigned long  bit_start = 0, bit_end = 0, val = 0;

				if (argc!=6)
				{
					DBG_DUMP("syntex: mem mbit [addr] [bit_start#num] [bit_end#num] [value#] \r\n");
					break;
				}
				i++;
				if (i < argc)
					sscanf(argv[i++], "%lx", &addr);
				if (i < argc)
					sscanf(argv[i++], "%lu", &bit_start);
				if (i < argc)
					sscanf(argv[i++], "%lu", &bit_end);

				if (i < argc)
					sscanf(argv[i++], "%lx", &val);

				if (bit_start > bit_end) {
					DBG_DUMP("syntex: mem mbit [addr] [bit_start#num] [bit_end#num] [value#] \r\n");
					break;
				}

				if (bit_end > 31) {
					DBG_DUMP("syntex: mem mbit [addr] [bit_start#num] [bit_end#num] [value#] \r\n");
					break;
				}
				cmd_mbit_w(addr, bit_start, bit_end, val);
				break;
			}
			show_help();
			break;
		}
	}

	uninit_mem_dev(fd);
	return 0;
}


