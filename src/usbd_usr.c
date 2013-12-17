#include "usbd_usr.h"
#include "usbd_ioreq.h"
#include "usb_conf.h"
#include <stdio.h>


USBD_Usr_cb_TypeDef USR_cb = {
        USBD_USR_Init,
        USBD_USR_DeviceReset,
        USBD_USR_DeviceConfigured,
        USBD_USR_DeviceSuspended,
        USBD_USR_DeviceResumed,
        USBD_USR_DeviceConnected,
        USBD_USR_DeviceDisconnected,
};

/**
 * @brief  USBD_USR_Init
 *         Displays the message on LCD for host lib initialization
 * @param  None
 * @retval None
 */
void USBD_USR_Init (void)
{
        printf ("Init.\r\n");

//        if (SysTick_Config (SystemCoreClock / 50)) {

        if (SysTick_Config (SystemCoreClock / 100000)) {
                /* Capture error */
                while (1)
                        ;
        }
}

/**
 * @brief  USBD_USR_DeviceReset
 *         Displays the message on LCD on device Reset Event
 * @param  speed : device speed
 * @retval None
 */
void USBD_USR_DeviceReset (uint8_t speed)
{
        switch (speed) {
        case USB_OTG_SPEED_HIGH:
                printf ("USB Device Library v1.1.0 [HS]\r\n");

                break;

        case USB_OTG_SPEED_FULL:
                printf ("USB Device Library v1.1.0 [FS]\r\n");
                break;

        default:
                printf ("USB Device Library v1.1.0 [??]\r\n");
                break;

        }
}

/**
 * @brief  USBD_USR_DeviceConfigured
 *         Displays the message on LCD on device configuration Event
 * @param  None
 * @retval Staus
 */
void USBD_USR_DeviceConfigured (void)
{
        printf ("USBD_USR_DeviceConfigured : Interface started.\r\n");
}

/**
 * @brief  USBD_USR_DeviceConnected
 *         Displays the message on LCD on device connection Event
 * @param  None
 * @retval Staus
 */
void USBD_USR_DeviceConnected (void)
{
        printf ("USBD_USR_DeviceConnected : USB Device Connected.\r\n");
}

/**
 * @brief  USBD_USR_DeviceDisonnected
 *         Displays the message on LCD on device disconnection Event
 * @param  None
 * @retval Staus
 */
void USBD_USR_DeviceDisconnected (void)
{
        printf ("USBD_USR_DeviceDisconnected : USB Device Disconnected.\r\n");
}

/**
 * @brief  USBD_USR_DeviceSuspended
 *         Displays the message on LCD on device suspend Event
 * @param  None
 * @retval None
 */
void USBD_USR_DeviceSuspended (void)
{
        printf ("USBD_USR_DeviceSuspended : USB Device in Suspend Mode.\r\n");
        /* Users can do their application actions here for the USB-Reset */
}

/**
 * @brief  USBD_USR_DeviceResumed
 *         Displays the message on LCD on device resume Event
 * @param  None
 * @retval None
 */
void USBD_USR_DeviceResumed (void)
{
        printf ("USBD_USR_DeviceResumed : USB Device in Idle Mode.\r\n");
        /* Users can do their application actions here for the USB-Reset */
}


