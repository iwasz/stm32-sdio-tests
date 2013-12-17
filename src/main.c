#include <stm32f4xx.h>
#include <stdio.h>

#include <usb_core.h> // OTG driver
#include <usbd_core.h> // Device library
#include <usbd_usr.h> // USBD_Usr_cb_TypeDef structure implementaion
#include "usbd_vendor_class.h"// Class library
#include "usbd_desc.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

/**
 * For printf.
 */
void initUsart (void)
{
        RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

        GPIO_InitTypeDef gpioInitStruct;

        gpioInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
        gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
        gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        gpioInitStruct.GPIO_OType = GPIO_OType_PP;
        gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOB, &gpioInitStruct);
        GPIO_PinAFConfig (GPIOB, GPIO_PinSource6, GPIO_AF_USART1); // TX
        GPIO_PinAFConfig (GPIOB, GPIO_PinSource7, GPIO_AF_USART1); // RX

        USART_InitTypeDef usartInitStruct;
        usartInitStruct.USART_BaudRate = 9600;
        usartInitStruct.USART_WordLength = USART_WordLength_8b;
        usartInitStruct.USART_StopBits = USART_StopBits_1;
        usartInitStruct.USART_Parity = USART_Parity_No;
        usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_Init (USART1, &usartInitStruct);
        USART_Cmd (USART1, ENABLE);
}

void initExti (void)
{
        // Konfiguracja portu jak zwykle.
        RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOC, ENABLE);
        RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOD, ENABLE);
        RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOE, ENABLE);
        RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOF, ENABLE);
        GPIO_InitTypeDef gpioInitStruct;
        gpioInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
                                  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
        gpioInitStruct.GPIO_Speed = GPIO_Speed_100MHz;
        gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_Init (GPIOC, &gpioInitStruct);
        GPIO_Init (GPIOD, &gpioInitStruct);
        GPIO_Init (GPIOE, &gpioInitStruct);
        GPIO_Init (GPIOF, &gpioInitStruct);

#if 0
        /*
         * Podłaczenie pinu portu i linii EXTI.  EXTI_PinSource1 oznacza, pin1 (portu E) podłączony
         * będzie do linii 1.
         */
        SYSCFG_EXTILineConfig (EXTI_PortSourceGPIOD, EXTI_PinSource0);
        SYSCFG_EXTILineConfig (EXTI_PortSourceGPIOD, EXTI_PinSource1);

        // Konfiguracja linii 1 EXTI.
        EXTI_InitTypeDef extiInitStructure;
        extiInitStructure.EXTI_Line = EXTI_Line0;
        extiInitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        extiInitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
        extiInitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&extiInitStructure);

        extiInitStructure.EXTI_Line = EXTI_Line1;
        extiInitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        extiInitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
        extiInitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&extiInitStructure);

        // Konfiguracja NVIC. Grupa jest ustawiona w pliku usb_bsp.c na 1
        // NVIC_PriorityGroupConfig (NVIC_PriorityGroup_0);
#endif
        NVIC_InitTypeDef nvicInitStructure;
//        nvicInitStructure.NVIC_IRQChannel = EXTI0_IRQn;
//        nvicInitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//        nvicInitStructure.NVIC_IRQChannelSubPriority = 0;
//        nvicInitStructure.NVIC_IRQChannelCmd = ENABLE;
//        NVIC_Init (&nvicInitStructure);
//
//        nvicInitStructure.NVIC_IRQChannel = EXTI1_IRQn;
//        nvicInitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//        nvicInitStructure.NVIC_IRQChannelSubPriority = 0;
//        nvicInitStructure.NVIC_IRQChannelCmd = ENABLE;
//        NVIC_Init (&nvicInitStructure);

        // SysTick ma mniejszy priorytet niż GPIO.
        nvicInitStructure.NVIC_IRQChannel = SysTick_IRQn;
        nvicInitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        nvicInitStructure.NVIC_IRQChannelSubPriority = 0;
        nvicInitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init (&nvicInitStructure);
}

/**
 * Configure the USB OTG engine.
 */
void initUsb (void)
{
        USBD_Init(&USB_OTG_dev,
                  USB_OTG_FS_CORE_ID,
                  &USR_desc,
                  &USBDVendorClass,
                  &USR_cb);
}

int main (void)
{
        initUsart ();
        initUsb ();
        initExti ();

        while (1) {
        }
}
