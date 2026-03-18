#include <linux/module.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/of_device.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/soc/nvt/fmem.h>

#include "nvt-pcie-lib-internal.h"

#define _DEV_NAME "nvt-pcie-lib" //nvt-pcie-lib
#define _DEV_MINOR_CNT      1
#define _DEV_BASE_MINOR     0

typedef struct {
	dev_t           devt;
	struct cdev     cdev;
	struct class    *p_class;
	struct device   *p_device;
} DEV_IOCTL_T;

static int _dev_platdrv_probe(struct platform_device *pdev);
static int _dev_platdrv_remove(struct platform_device *pdev);

static int _dev_fops_open (struct inode *inode, struct file *file);
static int _dev_fops_release (struct inode *inode, struct file *file);
static long _dev_fops_ioctl (struct file *file,  unsigned int cmd, unsigned long arg);
static int _dev_fops_mmap(struct file *filp, struct vm_area_struct *vma);

static const struct of_device_id nvt_pcie_lib_dt_match[] = {
	{.compatible = "nvt,nvt-pcie-lib"},
	{},
};
MODULE_DEVICE_TABLE(of, nvt_pcie_lib_dt_match);

static struct platform_driver _dev_platdrv_struct = {
	.probe  =   _dev_platdrv_probe,
	.remove =   _dev_platdrv_remove,
	.driver =   {
		.name   =   _DEV_NAME,
		.owner  =   THIS_MODULE,
		.of_match_table = of_match_ptr(nvt_pcie_lib_dt_match),
	},
};

// Kernel interface
static struct file_operations _dev_fops = {
	.owner          =   THIS_MODULE,
	.open           =   _dev_fops_open,
	.release        =   _dev_fops_release,
	.unlocked_ioctl =   _dev_fops_ioctl,
	.mmap           =   _dev_fops_mmap,
};

static DEV_IOCTL_T g_ioctl = {0};

/*-----------------------------------------------------------------------------*/
/* Kernel IOCTL                                                                */
/*-----------------------------------------------------------------------------*/
int nvtpcie_platdrv_init(void)
{
	if (0 != platform_driver_register(&_dev_platdrv_struct)) {
		DBG_ERR("platform_driver_register failed\n");
		return -1;
	}

	return 0;
}

void nvtpcie_platdrv_exit(void)
{
	platform_driver_unregister(&_dev_platdrv_struct);
}

static int _dev_platdrv_probe(struct platform_device *pdev)
{
	int reg_err = 0, cdev_err = 0;

	reg_err = alloc_chrdev_region(&g_ioctl.devt, _DEV_BASE_MINOR, _DEV_MINOR_CNT, _DEV_NAME);
	if (reg_err) {
		DBG_ERR(" alloc_chrdev_region failed!\n");
		goto _dev_platdrv_probe_err;
	}

	cdev_init(&g_ioctl.cdev, &_dev_fops);
	g_ioctl.cdev.owner = THIS_MODULE;

	cdev_err = cdev_add(&g_ioctl.cdev, g_ioctl.devt, _DEV_MINOR_CNT);
	if (cdev_err) {
		DBG_ERR(" cdev_add failed!\n");
		goto _dev_platdrv_probe_err;
	}

	g_ioctl.p_class = class_create(THIS_MODULE, _DEV_NAME);
	if (IS_ERR(g_ioctl.p_class)) {
		DBG_ERR(" class_create failed!\n");
		goto _dev_platdrv_probe_err;
	}

	g_ioctl.p_device = device_create(g_ioctl.p_class, NULL, g_ioctl.devt, NULL, _DEV_NAME);
	if (NULL == g_ioctl.p_device) {
		DBG_ERR(" device_create failed!\n");
		goto _dev_platdrv_probe_err;
	}

	return 0;

_dev_platdrv_probe_err:

	if (g_ioctl.p_device) {
		device_destroy(g_ioctl.p_class, g_ioctl.devt);
	}
	if (g_ioctl.p_class) {
		class_destroy(g_ioctl.p_class);
	}
	if (!cdev_err) {
		cdev_del(&g_ioctl.cdev);
	}
	if (!reg_err) {
		unregister_chrdev_region(g_ioctl.devt, _DEV_MINOR_CNT);
	}

	return -1;
}

static int _dev_platdrv_remove(struct platform_device *pdev)
{
	device_destroy(g_ioctl.p_class, g_ioctl.devt);
	class_destroy(g_ioctl.p_class);

	cdev_del(&g_ioctl.cdev);
	unregister_chrdev_region(g_ioctl.devt, _DEV_MINOR_CNT);

	return 0;
}

long IOFUNC_GET_MY_CHIPID(unsigned long user_arg)
{
	NVTPCIE_IOARG_GET_MY_CHIPID ker_arg = {0};

	ker_arg.ret_chipid = nvtpcie_get_my_chipid();

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_GET_EP_COUNT(unsigned long user_arg)
{
	NVTPCIE_IOARG_GET_EP_COUNT ker_arg = {0};

	ker_arg.ret_ep_count = nvtpcie_get_ep_count();

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_DOWNSTREAM_ACTIVE(unsigned long user_arg)
{
	NVTPCIE_IOARG_DOWNSTREAM_ACTIVE ker_arg = {0};

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	ker_arg.ret_is_active = nvtpcie_downstream_active(ker_arg.ep_chipid);

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_GET_MAPPED_PA(unsigned long user_arg)
{
	NVTPCIE_IOARG_GET_MAPPED_PA ker_arg = {0};

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	ker_arg.ret_mapped_pa = nvtpcie_get_mapped_pa(ker_arg.loc_chipid, ker_arg.tar_chipid, ker_arg.tar_pa);

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_GET_UNMAPPED_PA(unsigned long user_arg)
{
	NVTPCIE_IOARG_GET_UNMAPPED_PA ker_arg = {0};

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	ker_arg.ret_unmapped_pa = nvtpcie_get_unmapped_pa(ker_arg.loc_chipid, ker_arg.mapped_pa, &ker_arg.ret_map_chipid);

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_GET_PCIE_ADDR(unsigned long user_arg)
{
	NVTPCIE_IOARG_GET_PCIE_ADDR ker_arg = {0};

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	ker_arg.ret_pa = nvtpcie_get_pcie_addr(ker_arg.loc_chipid, ker_arg.tar_ddrid, ker_arg.tar_pa);

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_GET_PCIE_ADDR_BY_BUS(unsigned long user_arg)
{
	NVTPCIE_IOARG_GET_PCIE_ADDR_BY_BUS ker_arg = {0};

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	ker_arg.ret_pa = nvtpcie_get_pcie_addr_by_bus(ker_arg.loc_busid, ker_arg.tar_busid, ker_arg.tar_pa);

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_GET_SHMEM(unsigned long user_arg)
{
	NVTPCIE_IOARG_GET_SHMEM ker_arg = {0};

	ker_arg.ret_pa = nvtpcie_shmem_get_pa();
	ker_arg.ret_size = nvtpcie_shmem_get_size();

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_ALLOC_EPMEM(unsigned long user_arg)
{
	NVTPCIE_IOARG_ALLOC_EPMEM ker_arg = {0};
	phys_addr_t ret_ep_pa = NVTPCIE_INVALID_PA;
	phys_addr_t ret_rc_mapped_pa = NVTPCIE_INVALID_PA;

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	if (NULL == nvtpcie_alloc_epmem(ker_arg.ep_chipid, ker_arg.alloc_size, &ret_ep_pa)) {
		DBG_ERR("alloc failed, ep_chipid %d, size 0x%lX\r\n", ker_arg.ep_chipid, ker_arg.alloc_size);
		return -EINVAL;
	}


	ret_rc_mapped_pa = nvtpcie_get_downstream_pa(ker_arg.ep_chipid, ret_ep_pa);
	if (NVTPCIE_INVALID_PA == ker_arg.ret_rc_mapped_pa) {
		DBG_ERR("downstream_pa failed, ep_chipid %d, ep_pa 0x%08llX\r\n", ker_arg.ep_chipid, (u64)ret_ep_pa);
		return -EINVAL;
	}

	ker_arg.ret_ep_pa = (unsigned long)ret_ep_pa;
	ker_arg.ret_rc_mapped_pa = (unsigned long)ret_rc_mapped_pa;

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_SHMBLK_GET(unsigned long user_arg)
{
	NVTPCIE_IOARG_SHMBLK_GET ker_arg = {0};
	void *ker_va;

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	ker_va = nvtpcie_shmblk_get(ker_arg.name, ker_arg.size);
	if (NULL == ker_va) {
		DBG_ERR("Invalid arg name[%s] size %d\r\n", ker_arg.name, ker_arg.size);
		return -EINVAL;
	}

	ker_arg.ret_pa = fmem_lookup_pa((uintptr_t)ker_va);

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_SHMBLK_RELEASE(unsigned long user_arg)
{
	NVTPCIE_IOARG_SHMBLK_RELEASE ker_arg = {0};

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	nvtpcie_shmblk_release(ker_arg.name);

	return 0;
}

long IOFUNC_EDMA_COPY(unsigned long user_arg)
{
	NVTPCIE_IOARG_EDMA_COPY ker_arg = {0};

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	ker_arg.ret = nvtpcie_edma_copy(
			ker_arg.src_chipid,
			ker_arg.src_pa,
			ker_arg.dst_chipid,
			ker_arg.dst_pa,
			ker_arg.len);

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

long IOFUNC_EDMA_COPY_BY_BUS(unsigned long user_arg)
{
	NVTPCIE_IOARG_EDMA_COPY_BY_BUS ker_arg = {0};

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(ker_arg))) {
		DBG_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	ker_arg.ret = nvtpcie_edma_copy_by_bus(
			ker_arg.src_busid,
			ker_arg.src_pa,
			ker_arg.dst_busid,
			ker_arg.dst_pa,
			ker_arg.len);

	if (copy_to_user((void *)user_arg, (void *)&ker_arg, sizeof(ker_arg))) {
		return -EFAULT;
	}

	return 0;
}

static int _dev_fops_open(struct inode *inode, struct file *file)
{
	return 0;
}


static int _dev_fops_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long _dev_fops_ioctl(struct file *file,  unsigned int cmd, unsigned long user_arg)
{
	long ret = 0;

	BUILD_BUG_ON(sizeof(phys_addr_t) != sizeof(unsigned long));

	switch (cmd) {
	case NVTPCIE_IOCMD_GET_MY_CHIPID:
		ret = IOFUNC_GET_MY_CHIPID(user_arg);
		break;

	case NVTPCIE_IOCMD_GET_EP_COUNT:
		ret = IOFUNC_GET_EP_COUNT(user_arg);
		break;

	case NVTPCIE_IOCMD_DOWNSTREAM_ACTIVE:
		ret = IOFUNC_DOWNSTREAM_ACTIVE(user_arg);
		break;

	case NVTPCIE_IOCMD_GET_MAPPED_PA:
		ret = IOFUNC_GET_MAPPED_PA(user_arg);
		break;

	case NVTPCIE_IOCMD_GET_UNMAPPED_PA:
		ret = IOFUNC_GET_UNMAPPED_PA(user_arg);
		break;

	case NVTPCIE_IOCMD_GET_PCIE_ADDR:
		ret = IOFUNC_GET_PCIE_ADDR(user_arg);
		break;

	case NVTPCIE_IOCMD_GET_PCIE_ADDR_BY_BUS:
		ret = IOFUNC_GET_PCIE_ADDR_BY_BUS(user_arg);
		break;

	case NVTPCIE_IOCMD_GET_SHMEM:
		ret = IOFUNC_GET_SHMEM(user_arg);
		break;

	case NVTPCIE_IOCMD_ALLOC_EPMEM:
		ret = IOFUNC_ALLOC_EPMEM(user_arg);
		break;

	case NVTPCIE_IOCMD_SHMBLK_GET:
		ret = IOFUNC_SHMBLK_GET(user_arg);
		break;

	case NVTPCIE_IOCMD_SHMBLK_RELEASE:
		ret = IOFUNC_SHMBLK_RELEASE(user_arg);
		break;

	case NVTPCIE_IOCMD_EDMA_COPY:
		ret = IOFUNC_EDMA_COPY(user_arg);
		break;

	case NVTPCIE_IOCMD_EDMA_COPY_BY_BUS:
		ret = IOFUNC_EDMA_COPY_BY_BUS(user_arg);
		break;

	default:
		DBG_WRN("Unknown IOCMD 0x%08X\n", (u32)cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int _dev_fops_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long byte_off = vma->vm_pgoff << PAGE_SHIFT;    // byte offset from the buffer start point
	unsigned long pfn; // which page number is the start page
	phys_addr_t shmem_pa;
	unsigned long shmem_size;
	int ret;

	shmem_pa = nvtpcie_shmem_get_pa();
	if (NVTPCIE_INVALID_PA == shmem_pa) {
		DBG_ERR("Get shmem pa failed\r\n");
		return -EINVAL;
	}

	shmem_size = nvtpcie_shmem_get_size();
	if (0 == shmem_size) {
		DBG_ERR("Get shmem size failed\r\n");
		return -EINVAL;
	}

	if (size != shmem_size) {
		DBG_ERR("size 0x%lX should be 0x%lX\r\n", size, shmem_size);
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	byte_off += shmem_pa;
	pfn = byte_off >> PAGE_SHIFT;    //which page number is the start page

	ret = remap_pfn_range(vma, vma->vm_start, pfn, shmem_size, vma->vm_page_prot);

	return ret;
}
