#include "stm32fxxx_it.h"
#include "logf.h"
#include "sdio_high_level.h"

/******************************************************************************/
/*             Cortex-M Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler (void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler (void)
{
        logf ("HardFault_Handler\r\n");

        /* Go to infinite loop when Hard Fault exception occurs */
        while (1) {
        }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler (void)
{
        logf ("MemManage_Handler\r\n");

        /* Go to infinite loop when Memory Manage exception occurs */
        while (1) {
        }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler (void)
{
        logf ("BusFault_Handler\r\n");

        /* Go to infinite loop when Bus Fault exception occurs */
        while (1) {
        }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler (void)
{
        logf ("UsageFault_Handler\r\n");

        /* Go to infinite loop when Usage Fault exception occurs */
        while (1) {
        }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler (void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler (void)
{
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler (void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler (void)
{
}


void WWDG_IRQHandler (void)
{
        logf ("WWDG_IRQHandler\r\n");
}

/**
 * @brief  This function handles SDIO global interrupt request.
 * @param  None
 * @retval None
 */
void SDIO_IRQHandler (void)
{
        /* Process All SDIO Interrupt Sources */
        SD_ProcessIRQSrc ();
        logf ("SDIO_IRQHandler end \r\n");
}

/**
 * @brief  This function handles DMA2 Stream3 or DMA2 Stream6 global interrupts
 *         requests.
 * @param  None
 * @retval None
 */
void SD_SDIO_DMA_IRQHANDLER (void)
{
        /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
        SD_ProcessDMAIRQ ();
}


//void DMA2_Stream3_IRQHandler (void)
//{
//        /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
//        SD_ProcessDMAIRQ ();
//}
//
//void DMA2_Stream6_IRQHandler (void)
//{
//        /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
//        SD_ProcessDMAIRQ ();
//}
