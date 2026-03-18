/*
    USB CDC Descriptors Header


    @file       ucdc_desc.h
    @ingroup    mILibUsbCDC
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _UCDCDESC_H
#define _UCDCDESC_H

#include "ucdc_int.h"
#include "usb2dev.h"

#define USB_FS_CDC_CFGA_INTF0_ALT0_EP1_MAX_PACKET_SIZE        64
#define USB_FS_CDC_CFGA_INTF0_ALT0_EP2_MAX_PACKET_SIZE        64
#define USB_FS_CDC_CFGA_INTF0_ALT0_EP3_MAX_PACKET_SIZE        64

#define USB_HS_CDC_CFGA_INTF0_ALT0_EP1_MAX_PACKET_SIZE        0x0200
#define USB_HS_CDC_CFGA_INTF0_ALT0_EP2_MAX_PACKET_SIZE        0x0200
#define USB_SS_CDC_CFGA_INTF0_ALT0_EP1_MAX_PACKET_SIZE        0x0400
#define USB_SS_CDC_CFGA_INTF0_ALT0_EP2_MAX_PACKET_SIZE        0x0400
#define USB_HS_CDC_CFGA_INTF0_ALT0_EP3_MAX_PACKET_SIZE        64

#define USB_EP_CDC_INTERRUPT_FSINTERVAL      0xFF   //255ms
#define USB_EP_CDC_INTERRUPT_HSINTERVAL      0xFF   //2**(255-1)*125us=125ms


/* number of string descriptor */
#define USB_CDC_STR_NUM_STRING             4

#define USB_CDC_IF_CFGA_IF0_NUMBER_EP      1
#define USB_CDC_IF_IF0_NUMBER              0
#define USB_CDC_CFG_A_IF0_ALT_MAX          1

#define USB_CDC_IF_CFGA_IF1_NUMBER_EP      2
#define USB_CDC_IF_IF1_NUMBER              1
#define USB_CDC_CFG_A_IF1_ALT_MAX          1


/* device descriptor */
#define USB_CDC_DEV_RELEASE_NUMBER         0x0200         /* spec. release number             */
#define USB_CDC_SS_DEV_RELEASE_NUMBER      0x0310         /* spec. release number             */
#define USB_CDC_DEV_CLASS                  2              /* class code                       */
#define USB_CDC_DEV_SUBCLASS               0              /* sub class code                   */
#define USB_CDC_DEV_PROTOCOL               0              /* protocol code                    */
#define USB_CDC_DEV_MAX_PACKET_SIZE0       0x40           /* max packet size for endpoint 0   */
#define USB_CDC_VENDOR_ID                  0x0D64         /* vendor id                        */
#define USB_CDC_PRODUCT_ID                 0x8611         /* product id                       */
#define USB_CDC_DEV_RELASE_NUMBER          0x0100         /* device release number            */
#define USB_CDC_DEV_NUM_CONFIG             1              /* number of possible configuration */

/*
    configuration descriptor
*/
#define USB_CDC_CFGA_TOTAL_LENGTH          (USB_CFG_LENGTH + (USB_IF_LENGTH * 2) + (USB_BULK_EP_LENGTH * 3)+ 14)
#define USB_CDC_SS_CFGA_TOTAL_LENGTH          (USB_CFG_LENGTH + (USB_IF_LENGTH * 2) + ((USB_BULK_EP_LENGTH+USB_SSEP_COMPANISON_LENGTH) * 3)+ 14)
/* total length                     */
#define USB_CDC_CFGA_NUMBER_IF             2                   /* number of interfaces             */
#define USB_CDC_CFGA_CFG_VALUE             1                   /* configuration value              */
#define USB_CDC_CFGA_CFG_IDX               0                   /* configuration string id          */

#define USB_CDC_CFGA_CFG_ATTRIBUES_IDX     0x07                /* Attribute index in configuration */
#define USB_CDC_CFGA_CFG_ATTRIBUES_SELF    0xC0                /* characteristics: Self Powered    */
#define USB_CDC_CFGA_CFG_ATTRIBUES_BUS     0x80                /* characteristics: Bus Powered     */

#define USB_CDC_CFGA_MAX_POWER_IDX         0x08                /* maximum power index in configuration*/
#define USB_CDC_CFGA_MAX_POWER_SELF        0x32                /* 100mA maximum power. Unit in 2mA */
#define USB_CDC_CFGA_MAX_POWER_BUS         0xFA                /* 500mA maximum power. Unit in 2mA */


/*
    Interface Descriptor
*/

/* Communacation Class Interface */
/*---- alternate setting */
#define USB_CDC_IF_ALT0                    0
/* number of endpoint */
/* bInterfaceClass */
#define USB_CDC_IF_CLASS                   2
#define USB_CDC_IF_DATA_CLASS            0x0A
/* bInterfaceSubClass */
#define USB_CDC_IF_SUBCLASS_NONE           0
#define USB_CDC_IF_SUBCLASS_ACM            2
/* bInterfaceProtocol */
#define USB_CDC_IF_PROTOCOL_NONE           0
#define USB_CDC_IF_PROTOCOL_AT_CMD         1

#define USB_CDC_IF_IDX                     0



/*
    Functional Descriptor
*/
#define USB_CS_DESCRIPTOR_TYPE                  0x24  /**< Indicates that the descriptor is a class specific interface descriptor. */
//Functional header
#define USB_CDC_FUNCTIONAL_HEADER_LENGTH        5
#define USB_CS_SUBTYPE_HEADER                   0x00  /**< CDC class-specific Header functional descriptor. */
#define USB_CDC_SPEC_VER_MAJOR                  1
#define USB_CDC_SPEC_VER_MINOR                  0x10
//Functional ACM
#define USB_CDC_FUNCTIONAL_ACM_LENGTH           4
#define USB_CS_SUBTYPE_ACM                      0x02  /**< CDC class-specific Abstract Control Model functional descriptor. */
#define USB_CDC_ACM_CAPABILITIES                0x06
//Functional Union
#define USB_CDC_FUNCTIONAL_UNION_LENGTH         5
#define USB_CS_SUBTYPE_UNION                    0x06  /**< CDC class-specific Union functional descriptor. */
#define USB_CDC_MASTER_IF_NUM                   0
#define USB_CDC_SLAVE_IF_NUM                    1

#define USB_BOS_LENGTH                 5
#define USB_SS_STRG_BOS_TOTAL_LENGTH        22
#define USB_HS_STRG_BOS_TOTAL_LENGTH        12
#define USB_SS_STRG_BOS_DEVCAP_NUMBER       2
#define USB_HS_STRG_BOS_DEVCAP_NUMBER       1






extern USB_DEVICE_DESC gUSBSSCDCDevDesc, gUSBHSCDCDevDesc, gUSBFSCDCDevDesc;
extern UINT8 gUSBSSCDCConfigDesc[], gUSBHSCDCConfigDesc[], gUSBFSCDCConfigDesc[];
extern UINT8 gUSBFSOtherCDCConfigDesc[], gUSBHSOtherCDCConfigDesc[];

extern const UINT8 USBCDCStrDesc0[];
extern const UINT16 USBCDCManuStrDesc[], USBCDCProdStrDesc[], USBCDCStrDesc3[];
extern const USB_DEVICE_DESC gUSBCDCDevQualiDesc[];
extern const UINT8 gCDCSSBosDesc[];
#endif
