/**
  ******************************************************************************
  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick)
  *             - Collection : Application 
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 




/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length);

static uint8_t  USBD_HID_DataIn (void  *pdev, uint8_t epnum);
/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Variables
  * @{
  */ 

USBD_Class_cb_TypeDef  USBD_HID_cb = 
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/  
  NULL, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  NULL, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,      
  USBD_HID_GetCfgDesc,
#ifdef USB_OTG_HS_CORE  
  USBD_HID_GetCfgDesc, /* use same config as per FS */
#endif  
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */        
__ALIGN_BEGIN static uint32_t  USBD_HID_AltSet  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */      
__ALIGN_BEGIN static uint32_t  USBD_HID_Protocol  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint32_t  USBD_HID_IdleState __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */ 
/*
 * Configuration descriptor.
 */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
        0x09,           /* bLength (1) Configuration Descriptor size in bytes, always 0x09. */
        USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType (1B) Stała 0x02 (patrz tabelka typów deskryptorów */
        USB_HID_CONFIG_DESC_SIZ,           /* wTotalLength (2B) Rozmiar w bajtach całego deskryptora wraz z jego "doklejonymi" */
        0x00,                              /* deskryptorami interfejsów i endpointów. */
        0x01,         /* bNumInterfaces (1B) Liczba interfejsów. */
        0x01,         /* bConfigurationValue (1B) numer porządkowy konfiguracji. Musi być większy równy 1. Rozumiem, że musi być */
                      /* unikalny dla każdej konfiguracji. */
        0x00,         /* iConfiguration (1B) Index of string descriptor describing the configuration, albo 0, jeśli nie ma. */
        0xE0,         /* bmAttributes (1B) Atrybuty w postaci maski bitowej. Bit 6==1 -> self powered. Bit 6==0 -> bus powered */
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
        0x01,         /* bNumEndpoints : Liczba endpointów prócz EP0. */
        0x03,         /* bInterfaceClass : Klasa. Listę można znaleźć na wikipedii. 0xff to vendor specific. Klasę urządzenia */
                      /* można także podać w deskryptorze urządzenia, ale najczęściej się podaj tu. 0x03 to HID. */
        0x01,         /* bInterfaceSubClass : 1=BOOT, 0=no boot. Ustanowione przez USB-IF dla klas. 0xff == vendor specific. */
        0x02,         /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse. Ustanowione przez USB-IF dla klas. 0xff == vendor */
                      /* specific.*/
        0,            /* iInterface: Index of string descriptor, or 0 if there is no string. */

        /*
         * HID descriptor (mouse).
         */
        0x09,         /* bLength: HID Descriptor size*/
        HID_DESCRIPTOR_TYPE, /* bDescriptorType: HID*/
        0x11,         /* bcdHID: HID Class Spec release number*/
        0x01,
        0x00,         /* bCountryCode: Hardware target country*/
        0x01,         /* bNumDescriptors: Number of HID class descriptors to follow*/
        0x22,         /* bDescriptorType*/
        HID_MOUSE_REPORT_DESC_SIZE,/* wItemLength: Total length of Report descriptor*/
        0x00,

        /*
         * Endpoint descriptor.
         */
        0x07,          /* bLength: Endpoint Descriptor size */
        USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: Stała 0x05 */
        HID_IN_EP,     /* bEndpointAddress: Endpoint Address. 4 LSB to numer endpointu. Urządzenia LS moga mieć w interfejsie */
                       /* max 3 endpointy. Pozostałe urządzenia mogą mieć 16. Bit MSB to kierunek : 0 == OUT, 1 == IN. Bity */\
                       /* 6..4 muszą być 0. */
        0x03,          /* bmAttributes : */
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
        HID_IN_PACKET, /* wMaxPacketSize (2B): Dla USB 2.0 Bity 0-10 (LSB) oznaczają liczbę bajtów. To może być wartść od 0 */
                       /* do 1204. Bity 11 i 12 oznaczają liczbę dodatkowych transakcji w mikro ramce dla transferów iso */
                       /* i interrupt  (0-2, bo 11 jest zarezerwowane). Bity 13-15 musza być 0. */
        0x00,
        0x0A,          /* bInterval: Polling Interval. Interval for polling endpoint data transfers. Value in frame counts (ms). */
                       /* Ignored for Bulk & Control Endpoints. Isochronous must equal 1 and field may range from 1 to 255 for */
                       /* interrupt endpoints. */
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ] __ALIGN_END=
{
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
};
#endif 


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
{
  0x05,   0x01,
  0x09,   0x02,
  0xA1,   0x01,
  0x09,   0x01,
  
  0xA1,   0x00,
  0x05,   0x09,
  0x19,   0x01,
  0x29,   0x03,
  
  0x15,   0x00,
  0x25,   0x01,
  0x95,   0x03,
  0x75,   0x01,
  
  0x81,   0x02,
  0x95,   0x01,
  0x75,   0x05,
  0x81,   0x01,
  
  0x05,   0x01,
  0x09,   0x30,
  0x09,   0x31,
  0x09,   0x38,
  
  0x15,   0x81,
  0x25,   0x7F,
  0x75,   0x08,
  0x95,   0x03,
  
  0x81,   0x06,
  0xC0,   0x09,
  0x3c,   0x05,
  0xff,   0x09,
  
  0x01,   0x15,
  0x00,   0x25,
  0x01,   0x75,
  0x01,   0x95,
  
  0x02,   0xb1,
  0x22,   0x75,
  0x06,   0x95,
  0x01,   0xb1,
  
  0x01,   0xc0
}; 

/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  
  /* Open EP IN */
  DCD_EP_Open(pdev,
              HID_IN_EP,
              HID_IN_PACKET,
              USB_OTG_EP_INT);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              HID_OUT_EP,
              HID_OUT_PACKET,
              USB_OTG_EP_INT);
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  DCD_EP_Close (pdev , HID_IN_EP);
  DCD_EP_Close (pdev , HID_OUT_EP);
  
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
      
      
    case HID_REQ_SET_PROTOCOL:
      USBD_HID_Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_IdleState,
                        1);        
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_MOUSE_REPORT_DESC_SIZE , req->wLength);
        pbuf = HID_MOUSE_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pbuf = USBD_HID_Desc;   
#else
        pbuf = USBD_HID_CfgDesc + 0x12;
#endif 
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_HID_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      USBD_HID_AltSet = (uint8_t)(req->wValue);
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
uint8_t USBD_HID_SendReport     (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
  if (pdev->dev.device_status == USB_OTG_CONFIGURED )
  {
    DCD_EP_Tx (pdev, HID_IN_EP, report, len);
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn (void  *pdev, 
                              uint8_t epnum)
{
  
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  DCD_EP_Flush(pdev, HID_IN_EP);
  return USBD_OK;
}

/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
