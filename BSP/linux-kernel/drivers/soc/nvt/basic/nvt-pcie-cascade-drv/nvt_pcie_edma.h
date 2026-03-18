#ifndef __NVT_PCIE_EDMA_H__
#define __NVT_PCIE_EDMA_H__

#define EDMAC_ENGINE	2	/* two controllers */
#define EDMAC_CHAN		8

//#define ALIGN_MASK		0xF /* 128bits alignment */
#define ALIGN_MASK		0x0 /* 8bits alignment */
#define DMA_IN_PROCESS  0
#define DMA_FINISH      1
#define DMA_ABORT		2

#define ENGINE_SEMA_LOCK(cid)   	down(&edmac_info[cid].edmac_sema)
#define ENGINE_SEMA_UNLOCK(cid) 	up(&edmac_info[cid].edmac_sema)

#define CHAN_READ_SEMA_LOCK(cid)	down(&edmac_info[cid].read_chan_sema)
#define CHAN_READ_SEMA_UNLOCK(cid)	up(&edmac_info[cid].read_chan_sema)
#define CHAN_WRITE_SEMA_LOCK(cid)	down(&edmac_info[cid].write_chan_sema)
#define CHAN_WRITE_SEMA_UNLOCK(cid)	up(&edmac_info[cid].write_chan_sema)

enum edmac_dir {
	EDMAC_NONE = 0,
	EDMAC_READ = 1,
	EDMAC_WRITE = 2,
};


typedef struct edmac_info_s {
	void *edmac_vbase;
	u32	edmac_pbase;
	/* allocate channel use */
	struct semaphore edmac_sema;

	/* sema count = channel count */
	struct semaphore read_chan_sema;
	struct semaphore write_chan_sema;

	int	dma_read_full_cnt;
	int	dma_write_full_cnt;

	/* read dma channel */
	struct {
		int	busy;
		int	wait_event;
		wait_queue_head_t wait_queue;
		u32	read_count;	/* usage */
		u32	wait_jiffes;
		u32	wakeup_jiffies;
	} read_chan[EDMAC_CHAN];

	/* write dma channel */
	struct {
		int	busy;
		int	wait_event;
		wait_queue_head_t wait_queue;
		u32	write_count;	/* usage */
		u32	wait_jiffes;
		u32	wakeup_jiffies;
	} write_chan[EDMAC_CHAN];
	int	init_done;
} edmac_info_t;


#ifdef CONFIG_NVT_PCIE_CASCADE_DRV
//int nvt_pcie_edma_init(void *pcie_dbi_base, int pcie_id, pcie_num_t my_role);
int nvt_pcie_edma_read( nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len);
int nvt_pcie_edma_write(nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len);
#else
static inline int nvt_pcie_edma_read( nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len)
{
	return -1;
}
static inline int nvt_pcie_edma_write(nvtpcie_chipid_t tid, phys_addr_t src_paddr, phys_addr_t dst_paddr, u32 len)
{
	return -1;
}
#endif

#endif /* __NVT_PCIE_EDMA_H__ */
