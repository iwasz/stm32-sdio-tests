#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_conf.h"
#include "usb_regs.h"

#define USBD_VID                     0x20a0
#define USBD_PID                     0x41ff
#define USBD_PRODUCT_VER             0x0001

#define USBD_LANGID_STRING            0x409
#define USBD_MANUFACTURER_STRING      "iwasz.pl"
#define USBD_PRODUCT_FS_STRING        "forrest prototype hub"
#define USBD_SERIALNUMBER_FS_STRING   "0001"
#define USBD_CONFIGURATION_FS_STRING  "Forrest HUB Config"
#define USBD_INTERFACE_FS_STRING      "Forrest HUB Interface"

USBD_DEVICE USR_desc =
{
        USBD_USR_DeviceDescriptor,
        USBD_USR_LangIDStrDescriptor,
        USBD_USR_ManufacturerStrDescriptor,
        USBD_USR_ProductStrDescriptor,
        USBD_USR_SerialStrDescriptor,
        USBD_USR_ConfigStrDescriptor,
        USBD_USR_InterfaceStrDescriptor,
};

/**
 * USB Device Descriptor
 */
__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC] __ALIGN_END =
{                                   /* Nazwa (rozmiar w bajtach) opis. */
        0x12,                       /* bLength (1B) Rozmiar deskryptora, ktory jest stały i wynosi 0x12. */
        USB_DEVICE_DESCRIPTOR_TYPE, /* bDescriptorType (1B) Typ deskryptora (stała, ktora, patrz tabelka wyżej, wynosi 0x01). */
        0x00,                       /* bcdUSB (2B) USB specification release number (BCD). Dla USB 1.1 : 0x0110, */
        0x02,                       /* dla USB 2.0 : 0x0200. */
        0x00,                       /* bDeviceClass (1B) Kod klasy. Ale to pole nie jest obowiązkowe, bo klasę urządzenia można */
                                    /* zdefiniować w ''interface descriptor'' (tak się zazwyczaj robi). W takim przypadku */
                                    /* ustawiamy tu 0x00 gdy urządzenie NIE posiada ''association descriptor'', a 0xef gdy */
                                    /* posiada. */
        0x00,                       /* bDeviceSubClass (1B) Podklasa. Gdy bDeviceClass jest 0x00, to bDeviceSubClass */
                                    /* MUSI być 0x00. W innym przypadku, trzeba sprawdzić w tabelkach. */
        0x00,                       /* bDeviceProtocol (1B) Kolejna specyficzna wartość ustalana dla klas przez USB-IF. */
        USB_OTG_MAX_EP0_SIZE,       /* bMaxPacketSize0 (1B) Max rozmiar pakietu dla EP0. W USB 2.0 : 8 dla LS, */
                                    /* 8, 16, 32 lub 64 dla FS, oraz 64 dla HS. */
        LOBYTE (USBD_VID),          /* idVendor (2B) */
        HIBYTE (USBD_VID),
        LOBYTE (USBD_PID),          /* idProduct (2B) */
        HIBYTE (USBD_PID),
        LOBYTE (USBD_PRODUCT_VER),  /* bcdDevice (2B) Numer wersji urządzenia. Host może wybrać odpowiedni driver na */
        HIBYTE (USBD_PRODUCT_VER),  /* podstawie tego pola. */
        USBD_IDX_MFC_STR,           /* iManufacturer (1B) Index of manufacturer string descriptor, albo 0 jeśli nie ma. */
        USBD_IDX_PRODUCT_STR,       /* iProduct (1B) Index of product string descriptor, or 0 if not present. */
        USBD_IDX_SERIAL_STR,        /* iSerialNumber (1B) Index of serial number string descriptor, or 0. */
        USBD_CFG_MAX_NUM            /* bNumConfigurations (1B) Liczba konfiguracji obsługiwanych przez urządzenie w aktualnej */
                                    /* prędkości (LS, FS, HS). */
};

/**
 * USB device_qualifier descriptor.
 */
__ALIGN_BEGIN uint8_t USBD_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
        USB_LEN_DEV_QUALIFIER_DESC,     /* bLength (1B) Zawsze 0x0a */
        USB_DESC_TYPE_DEVICE_QUALIFIER, /* bDescriptorType (1B) Zawsze 0x06 */
        0x00,                           /* bcdUSB (2B) Jak wcześniej i wszystkie następne jak wcześniej. */
        0x02,
        0x00,                           /* bDeviceClass (1B) */
        0x00,                           /* bDeviceSubclass (1B) */
        0x00,                           /* bDeviceProtocol (1B) */
        USB_OTG_MAX_EP0_SIZE,           /* bMaxPacketSize0 (1B) */
        USBD_CFG_MAX_NUM,               /* bNumConfigurations (1B) */
        0x00                            /* Reserved */
};

/*
 * USB String descriptor with index 0. Language definiction (English only).
 */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID] __ALIGN_END =
{
        USB_SIZ_STRING_LANGID,      /* bLength : różna długość, trzeba policzyć. */
        USB_DESC_TYPE_STRING,       /* bDescriptorType : stała 0x03. */
        LOBYTE(USBD_LANGID_STRING), /* wLANGID : 0x0409 to angielski. 2 bajty */
        HIBYTE(USBD_LANGID_STRING), /* */
};

/**
 * @brief  USBD_USR_DeviceDescriptor
 *         return the device descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t * USBD_USR_DeviceDescriptor (uint8_t speed, uint16_t *length)
{
        *length = sizeof(USBD_DeviceDesc);
        return USBD_DeviceDesc;
}

/**
 * @brief  USBD_USR_LangIDStrDescriptor
 *         return the LangID string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t * USBD_USR_LangIDStrDescriptor (uint8_t speed, uint16_t *length)
{
        *length = sizeof(USBD_LangIDDesc);
        return USBD_LangIDDesc;
}

/**
 * @brief  USBD_USR_ProductStrDescriptor
 *         return the product string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t * USBD_USR_ProductStrDescriptor (uint8_t speed, uint16_t *length)
{

        USBD_GetString (USBD_PRODUCT_FS_STRING, USBD_StrDesc, length);
        return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_ManufacturerStrDescriptor
 *         return the manufacturer string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t * USBD_USR_ManufacturerStrDescriptor (uint8_t speed, uint16_t *length)
{
        USBD_GetString (USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
        return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_SerialStrDescriptor
 *         return the serial number string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t * USBD_USR_SerialStrDescriptor (uint8_t speed, uint16_t *length)
{
        USBD_GetString (USBD_SERIALNUMBER_FS_STRING, USBD_StrDesc, length);
        return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_ConfigStrDescriptor
 *         return the configuration string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t * USBD_USR_ConfigStrDescriptor (uint8_t speed, uint16_t *length)
{
        USBD_GetString (USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, length);
        return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_InterfaceStrDescriptor
 *         return the interface string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t * USBD_USR_InterfaceStrDescriptor (uint8_t speed, uint16_t *length)
{
        USBD_GetString (USBD_INTERFACE_FS_STRING, USBD_StrDesc, length);
        return USBD_StrDesc;
}


