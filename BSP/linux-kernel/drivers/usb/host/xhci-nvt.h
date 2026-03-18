#ifndef _XHCI_NVT_H
#define _XHCI_NVT_H

int xhci_nvt_host_init(struct usb_hcd *hcd);
int xhci_nvt_host_resume(struct usb_hcd *hcd);
void xhci_nvt_host_uninit(struct platform_device *pdev, struct usb_hcd *hcd);

#endif /* _XHCI_NVT_H */
