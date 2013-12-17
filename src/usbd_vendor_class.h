#ifndef __USB_HID_CORE_H_
#define __USB_HID_CORE_H_

#include  "usbd_ioreq.h"

//#define USB_HID_CONFIG_DESC_SIZ       34
#define USB_HID_DESC_SIZ              9
//#define HID_MOUSE_REPORT_DESC_SIZE    74
//
//#define HID_DESCRIPTOR_TYPE           0x21
//#define HID_REPORT_DESC               0x22


#define HID_REQ_SET_PROTOCOL          0x0B
#define HID_REQ_GET_PROTOCOL          0x03

#define HID_REQ_SET_IDLE              0x0A
#define HID_REQ_GET_IDLE              0x02

#define HID_REQ_SET_REPORT            0x09
#define HID_REQ_GET_REPORT            0x01

/**
 *
 */
extern USBD_Class_cb_TypeDef USBDVendorClass;

/**
 *
 */
uint8_t vendorSendReport (USB_OTG_CORE_HANDLE *pdev, uint8_t *report, uint16_t len);

#endif  // __USB_HID_CORE_H_
