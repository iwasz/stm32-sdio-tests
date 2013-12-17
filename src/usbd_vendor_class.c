#include "usbd_vendor_class.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include <stdio.h>

static uint8_t vendorInit (void *pdev, uint8_t cfgidx);
static uint8_t vendorDeInit (void *pdev, uint8_t cfgidx);
static uint8_t vendorSetup (void *pdev, USB_SETUP_REQ *req);
static uint8_t *vendorGetCfgDesc (uint8_t speed, uint16_t *length);
static uint8_t vendorDataIn (void *pdev, uint8_t epnum);
static uint8_t vendorDataOut (void *pdev, uint8_t epnum);
static uint8_t vendorSOF (void *pdev);
static uint8_t vendorIN_Incplt (void *pdev);
static uint8_t vendorOUT_Incplt (void *pdev);

extern int16_t angle[32];

/**
 *
 */
USBD_Class_cb_TypeDef USBDVendorClass =
{
        vendorInit,
        vendorDeInit,
        vendorSetup,
        NULL, /*EP0_TxSent*/
        NULL, /*EP0_RxReady*/
        vendorDataIn, /*DataIn*/
        vendorDataOut, /*DataOut*/
        vendorSOF, /*SOF */
        vendorIN_Incplt,
        vendorOUT_Incplt,
        vendorGetCfgDesc,
};

__ALIGN_BEGIN static uint32_t  vendorAltSet  __ALIGN_END = 0;

/*
 * Configuration descriptor.
 */
__ALIGN_BEGIN static uint8_t vendorCfgDesc[] __ALIGN_END =
{
        0x09,         /* bLength (1) Configuration Descriptor size in bytes, always 0x09. */
        USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType (1B) Stała 0x02 (patrz tabelka typów deskryptorów */
        9+9+9+7,           /* wTotalLength (2B) Rozmiar w bajtach całego deskryptora wraz z jego "doklejonymi" */
        0x00,                              /* deskryptorami interfejsów i endpointów. */
        0x01,         /* bNumInterfaces (1B) Liczba interfejsów. */
        0x01,         /* bConfigurationValue (1B) numer porządkowy konfiguracji. Musi być większy równy 1. Rozumiem, że musi być */
                      /* unikalny dla każdej konfiguracji. */
        0x00,         /* iConfiguration (1B) Index of string descriptor describing the configuration, albo 0, jeśli nie ma. */
        0xa0,         /* bmAttributes (1B) Atrybuty w postaci maski bitowej. Bit 6==1 -> self powered. Bit 6==0 -> bus powered */
                      /* Bit 5==1 -> urządzenie obsługuje "remote wakeup feature". Bit5==0 -> nie obsługuje. Bit 4..0 muszą być 0 */
                      /* Bit7 musi być równy 1. */
        0x32,         /* bMaxPower (1B) Max prąd w jednostkach 2mA dla USB 2.0 i w jednostkach 8mA dla USB 3.0. W tym przypadku */
                      /* 0x32 = 50, czyli 100mA. */

        /*
         * Device descriptor of mouse interface.
         */
        0x09,         /* bLength : Interface Descriptor size == 0x09. */
        USB_INTERFACE_DESCRIPTOR_TYPE,/* bDescriptorType : Interface descriptor type (constant 0x04). */
        0x00,         /* bInterfaceNumber: Unikalny numer. Urządzenia typu composite będą miały następne interfejsy, a każdy */
                      /* będzie miał kolejne numery. Domyślny interfejs ma numer 0. */
        0x00,         /* bAlternateSetting: Unikalny numer "alternate settings". Każdy interfejs może mieć odmiany, które */
                      /* właśnie nazywamy "alternate setting". Każde takie ustawienie musi mieć unikalny numer w tym polu. */
                      /* Domyślny numer to 0 i kazdy interfejs musi mieć takie "alternatywne ustawienie" o nr. 0. */
        0x00,         /* bNumEndpoints : Liczba endpointów prócz EP0. */
        0xff,         /* bInterfaceClass : Klasa. Listę można znaleźć na wikipedii. 0xff to vendor specific. Klasę urządzenia */
                      /* można także podać w deskryptorze urządzenia, ale najczęściej się podaj tu. 0x03 to HID. */
        0xff,         /* bInterfaceSubClass : 1=BOOT, 0=no boot. Ustanowione przez USB-IF dla klas. 0xff == vendor specific. */
        0xff,         /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse. Ustanowione przez USB-IF dla klas. 0xff == vendor */
                      /* specific.*/
        0,            /* iInterface: Index of string descriptor, or 0 if there is no string. */

        /**
         * alt
         */
        0x09,         /* bLength : Interface Descriptor size == 0x09. */
        USB_INTERFACE_DESCRIPTOR_TYPE,/* bDescriptorType : Interface descriptor type (constant 0x04). */
        0x00,         /* bInterfaceNumber: Unikalny numer. Urządzenia typu composite będą miały następne interfejsy, a każdy */
                      /* będzie miał kolejne numery. Domyślny interfejs ma numer 0. */
        0x01,         /* bAlternateSetting: Unikalny numer "alternate settings". Każdy interfejs może mieć odmiany, które */
                      /* właśnie nazywamy "alternate setting". Każde takie ustawienie musi mieć unikalny numer w tym polu. */
                      /* Domyślny numer to 0 i kazdy interfejs musi mieć takie "alternatywne ustawienie" o nr. 0. */
        0x01,         /* bNumEndpoints : Liczba endpointów prócz EP0. */
        0xff,         /* bInterfaceClass : Klasa. Listę można znaleźć na wikipedii. 0xff to vendor specific. Klasę urządzenia */
                      /* można także podać w deskryptorze urządzenia, ale najczęściej się podaj tu. 0x03 to HID. */
        0xff,         /* bInterfaceSubClass : 1=BOOT, 0=no boot. Ustanowione przez USB-IF dla klas. 0xff == vendor specific. */
        0xff,         /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse. Ustanowione przez USB-IF dla klas. 0xff == vendor */
                      /* specific.*/
        0,            /* iInterface: Index of string descriptor, or 0 if there is no string. */

        /*
         * Endpoint descriptor.
         */
        0x07,          /* bLength: Endpoint Descriptor size */
        USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: Stała 0x05 */
        IN_EP,         /* bEndpointAddress: Endpoint Address. 4 LSB to numer endpointu. Urządzenia LS moga mieć w interfejsie */
                       /* max 3 endpointy. Pozostałe urządzenia mogą mieć 16. Bit MSB to kierunek : 0 == OUT, 1 == IN. Bity */\
                       /* 6..4 muszą być 0. */
        0x03,/*0x0d,*/          /* bmAttributes : */
                       /* Bits 0..1 Transfer Type */
                       /*  00 = Control */
                       /*  01 = Isochronous */
                       /*  10 = Bulk */
                       /*  11 = Interrupt */
                       /* Bits 2..7 are reserved. If Isochronous endpoint, */
                       /* Bits 3..2 = Synchronisation Type (Iso Mode) */
                       /*  00 = No Synchonisation */
                       /*  01 = Asynchronous */
                       /*  10 = Adaptive */
                       /*  11 = Synchronous */
                       /* Bits 5..4 = Usage Type (Iso Mode) */
                       /*  00 = Data Endpoint */
                       /*  01 = Feedback Endpoint */
                       /*  10 = Explicit Feedback Data Endpoint */
                       /*  11 = Reserved */
        IN_PACKET_SIZE, /* wMaxPacketSize (2B): Dla USB 2.0 Bity 0-10 (LSB) oznaczają liczbę bajtów. To może być wartść od 0 */
                       /* do 1024. Bity 11 i 12 oznaczają liczbę dodatkowych transakcji w mikro ramce dla transferów iso */
                       /* i interrupt  (0-2, bo 11 jest zarezerwowane). Bity 13-15 musza być 0. */
        0x00,
        0xa/*0x01*/,          /* bInterval: Polling Interval. Interval for polling endpoint data transfers. Value in frame counts (ms). */
                       /* Ignored for Bulk & Control Endpoints. Isochronous must equal 1 and field may range from 1 to 255 for */
                       /* interrupt endpoints. */
};

/**
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t vendorInit (void *pdev, uint8_t cfgidx)
{
        /* Open EP IN */
        printf ("vendorInit\r\n");
        DCD_EP_Open (pdev, IN_EP, IN_PACKET_SIZE, USB_OTG_EP_INT);
        return USBD_OK;
}

/**
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t vendorDeInit (void *pdev, uint8_t cfgidx)
{
        /* Close HID EPs */
        printf ("vendorDeInit\r\n");
        DCD_EP_Close (pdev, IN_EP);
        return USBD_OK;
}

/**
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t vendorSetup (void *pdev, USB_SETUP_REQ *req)
{
//        uint16_t len = 0;
//        uint8_t *pbuf = NULL;

        printf ("vendorSetup\r\n");

        switch (req->bmRequest & USB_REQ_TYPE_MASK) {
//        case USB_REQ_TYPE_CLASS:
//                switch (req->bRequest) {
//
//                case HID_REQ_SET_PROTOCOL:
//                        USBD_HID_Protocol = (uint8_t) (req->wValue);
//                        break;
//
//                case HID_REQ_GET_PROTOCOL:
//                        USBD_CtlSendData (pdev, (uint8_t *) &USBD_HID_Protocol, 1);
//                        break;
//
//                case HID_REQ_SET_IDLE:
//                        USBD_HID_IdleState = (uint8_t) (req->wValue >> 8);
//                        break;
//
//                case HID_REQ_GET_IDLE:
//                        USBD_CtlSendData (pdev, (uint8_t *) &USBD_HID_IdleState, 1);
//                        break;
//
//                default:
//                        USBD_CtlError (pdev, req);
//                        return USBD_FAIL;
//                }
//                break;

        case USB_REQ_TYPE_STANDARD:
                switch (req->bRequest) {
//                case USB_REQ_GET_DESCRIPTOR:
//                        if (req->wValue >> 8 == HID_REPORT_DESC) {
//                                len = MIN(HID_MOUSE_REPORT_DESC_SIZE , req->wLength);
//                                pbuf = HID_MOUSE_ReportDesc;
//                        }
//                        else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE) {
//
//#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
//                                pbuf = USBD_HID_Desc;
//#else
//                                pbuf = USBD_HID_CfgDesc + 0x12;
//#endif
//                                len = MIN(USB_HID_DESC_SIZ , req->wLength);
//                        }
//
//                        USBD_CtlSendData (pdev, pbuf, len);
//
//                        break;

                case USB_REQ_GET_INTERFACE:
                        USBD_CtlSendData (pdev, (uint8_t *) &vendorAltSet, 1);
                        break;

                case USB_REQ_SET_INTERFACE:
                        vendorAltSet = (uint8_t) (req->wValue);
                        break;
                }
        }
        return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport 
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t vendorSendReport (USB_OTG_CORE_HANDLE *pdev, uint8_t *report, uint16_t len)
{
        if (pdev->dev.device_status == USB_OTG_CONFIGURED) {
                DCD_EP_Tx (pdev, IN_EP, report, len);
//                printf (".\r\n");
        }

        return USBD_OK;
}

/**
 *
 */
static uint8_t *vendorGetCfgDesc (uint8_t speed, uint16_t *length)
{
        printf ("vendorGetCfgDesc\r\n");
        *length = sizeof(vendorCfgDesc);
        return vendorCfgDesc;
}

/**
 *
 */
static uint8_t vendorDataIn (void *pdev, uint8_t epnum)
{
        /*
         * Ensure that the FIFO is empty before a new transfer, this condition could
         * be caused by  a new transfer before the end of the previous transfer
         */
//        printf ("vendorDataIn\r\n");
        DCD_EP_Flush (pdev, IN_EP);
        return USBD_OK;
}

static uint8_t vendorDataOut (void *pdev, uint8_t epnum)
{
        return USBD_OK;
}

static uint8_t vendorSOF (void *pdev)
{
        vendorSendReport (pdev, (uint8_t *)angle, IN_PACKET_SIZE);
        return USBD_OK;
}

static uint8_t vendorOUT_Incplt (void *pdev)
{
        return USBD_OK;
}

static uint8_t vendorIN_Incplt (void *pdev)
{
        return USBD_OK;
}
