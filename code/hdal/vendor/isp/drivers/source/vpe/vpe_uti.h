#ifndef _VPE_UTI_H_
#define _VPE_UTI_H_

extern void *vpe_uti_kmem_alloc(UINT32 mem_size);
extern void vpe_uti_kmem_free(void *mem_addr);
extern void *vpe_uti_vmem_alloc(UINT32 mem_size);
extern void vpe_uti_vmem_free(void *mem_addr);
extern UINT32 vpe_uti_calc_2dlut_tbl_size(UINT32 _2dlut_idx);
extern UINT32 vpe_uti_calc_2dlut_ioctl_size(UINT32 _2dlut_idx);
extern void vpe_uti_calc_2dlut_output_size(USIZE *dcout_size, USIZE *lut_size, USIZE *in_size, UINT8 precision, UINT32 *lut_tbl);

#endif

