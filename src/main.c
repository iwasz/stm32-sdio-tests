#include <stm32f4xx.h>
#include <stdio.h>
#if 0
#include "sdio_high_level.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {
        FAILED = 0, PASSED = !FAILED
} TestStatus;

/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512 /* Block Size in Bytes */

#define NUMBER_OF_BLOCKS      100  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)

#define SD_OPERATION_ERASE          0
#define SD_OPERATION_BLOCK          1
#define SD_OPERATION_MULTI_BLOCK    2
#define SD_OPERATION_END            3

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t aBuffer_Block_Tx[BLOCK_SIZE];
uint8_t aBuffer_Block_Rx[BLOCK_SIZE];
uint8_t aBuffer_MultiBlock_Tx[MULTI_BUFFER_SIZE];
uint8_t aBuffer_MultiBlock_Rx[MULTI_BUFFER_SIZE];
__IO TestStatus EraseStatus = FAILED;
__IO TestStatus TransferStatus1 = FAILED;
__IO TestStatus TransferStatus2 = FAILED;

SD_Error Status = SD_OK;
__IO uint32_t uwSDCardOperation = SD_OPERATION_ERASE;

/* Private function prototypes -----------------------------------------------*/
static void NVIC_Configuration (void);
static void SD_EraseTest (void);
static void SD_SingleBlockTest (void);
static void SD_MultiBlockTest (void);
static void Fill_Buffer (uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);

static TestStatus Buffercmp (uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);
static TestStatus eBuffercmp (uint8_t* pBuffer, uint32_t BufferLength);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Configures SDIO IRQ channel.
 * @param  None
 * @retval None
 */
static void NVIC_Configuration (void)
{
        NVIC_InitTypeDef NVIC_InitStructure;

        /* Configure the NVIC Preemption Priority Bits */
        NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);

        NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init (&NVIC_InitStructure);
        NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_Init (&NVIC_InitStructure);
}

/**
 * @brief  Tests the SD card erase operation.
 * @param  None
 * @retval None
 */
static void SD_EraseTest (void)
{
        /*------------------- Block Erase ------------------------------------------*/
        if (Status == SD_OK) {
                /* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
                Status = SD_Erase (0x00, (BLOCK_SIZE * NUMBER_OF_BLOCKS));
        }

        if (Status == SD_OK) {
                Status = SD_ReadMultiBlocks (aBuffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);

                /* Check if the Transfer is finished */
                Status = SD_WaitReadOperation ();

                /* Wait until end of DMA transfer */
                while (SD_GetStatus () != SD_TRANSFER_OK)
                        ;
        }

        /* Check the correctness of erased blocks */
        if (Status == SD_OK) {
                EraseStatus = eBuffercmp (aBuffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
        }

        if (EraseStatus == PASSED) {
                printf ("SD erase test passed\r\n");
        }
        else {
                printf ("SD erase test failed\r\n");
        }
}

/**
 * @brief  Tests the SD card Single Blocks operations.
 * @param  None
 * @retval None
 */
static void SD_SingleBlockTest (void)
{
        /*------------------- Block Read/Write --------------------------*/
        /* Fill the buffer to send */
        Fill_Buffer (aBuffer_Block_Tx, BLOCK_SIZE, 0x320F);

        if (Status == SD_OK) {
                /* Write block of 512 bytes on address 0 */
                Status = SD_WriteBlock (aBuffer_Block_Tx, 0x00, BLOCK_SIZE);
                /* Check if the Transfer is finished */
                Status = SD_WaitWriteOperation ();
                while (SD_GetStatus () != SD_TRANSFER_OK)
                        ;
        }

        if (Status == SD_OK) {
                /* Read block of 512 bytes from address 0 */
                Status = SD_ReadBlock (aBuffer_Block_Rx, 0x00, BLOCK_SIZE);
                /* Check if the Transfer is finished */
                Status = SD_WaitReadOperation ();
                while (SD_GetStatus () != SD_TRANSFER_OK)
                        ;
        }

        /* Check the correctness of written data */
        if (Status == SD_OK) {
                TransferStatus1 = Buffercmp (aBuffer_Block_Tx, aBuffer_Block_Rx, BLOCK_SIZE);
        }

        if (TransferStatus1 == PASSED) {
                printf ("Single block test passed\r\n");
        }
        else {
                printf ("Single block test failed\r\n");
        }
}

/**
 * @brief  Tests the SD card Multiple Blocks operations.
 * @param  None
 * @retval None
 */
static void SD_MultiBlockTest (void)
{
        /* Fill the buffer to send */
        Fill_Buffer (aBuffer_MultiBlock_Tx, MULTI_BUFFER_SIZE, 0x0);

        if (Status == SD_OK) {
                /* Write multiple block of many bytes on address 0 */
                Status = SD_WriteMultiBlocks (aBuffer_MultiBlock_Tx, 0, BLOCK_SIZE, NUMBER_OF_BLOCKS);

                /* Check if the Transfer is finished */
                Status = SD_WaitWriteOperation ();
                while (SD_GetStatus () != SD_TRANSFER_OK)
                        ;
        }

        if (Status == SD_OK) {
                /* Read block of many bytes from address 0 */
                Status = SD_ReadMultiBlocks (aBuffer_MultiBlock_Rx, 0, BLOCK_SIZE, NUMBER_OF_BLOCKS);

                /* Check if the Transfer is finished */
                Status = SD_WaitReadOperation ();
                while (SD_GetStatus () != SD_TRANSFER_OK)
                        ;
        }

        /* Check the correctness of written data */
        if (Status == SD_OK) {
                TransferStatus2 = Buffercmp (aBuffer_MultiBlock_Tx, aBuffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
        }

        if (TransferStatus2 == PASSED) {
                printf ("Multiple block test passed\r\n");
        }
        else {
                printf ("Multiple block test failed\r\n");
        }
}

/**
 * @brief  Compares two buffers.
 * @param  pBuffer1, pBuffer2: buffers to be compared.
 * @param  BufferLength: buffer's length
 * @retval PASSED: pBuffer1 identical to pBuffer2
 *         FAILED: pBuffer1 differs from pBuffer2
 */
static TestStatus Buffercmp (uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
{
        while (BufferLength--) {
                if (*pBuffer1 != *pBuffer2) {
                        return FAILED;
                }

                pBuffer1++;
                pBuffer2++;
        }

        return PASSED;
}

/**
 * @brief  Fills buffer with user predefined data.
 * @param  pBuffer: pointer on the Buffer to fill
 * @param  BufferLength: size of the buffer to fill
 * @param  Offset: first value to fill on the Buffer
 * @retval None
 */
static void Fill_Buffer (uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
        uint16_t index = 0;

        /* Put in global buffer same values */
        for (index = 0; index < BufferLength; index++) {
                pBuffer[index] = index + Offset;
        }
}

/**
 * @brief  Checks if a buffer has all its values are equal to zero.
 * @param  pBuffer: buffer to be compared.
 * @param  BufferLength: buffer's length
 * @retval PASSED: pBuffer values are zero
 *         FAILED: At least one value from pBuffer buffer is different from zero.
 */
static TestStatus eBuffercmp (uint8_t* pBuffer, uint32_t BufferLength)
{
        while (BufferLength--) {
                /* In some SD Cards the erased state is 0xFF, in others it's 0x00 */
                if ((*pBuffer != 0xFF) && (*pBuffer != 0x00)) {
                        return FAILED;
                }

                pBuffer++;
        }

        return PASSED;
}
#endif

/**
 * For printf.
 */
void initUsart (void)
{
#define USE_USART 1
#define USE_PIN_SET 1

#if USE_USART == 1
        #define USART USART1
        #define RCC_USART RCC_APB2Periph_USART1
        #define RCC_APBxPeriphClockCmd RCC_APB2PeriphClockCmd
        #define GPIO_AF_USART GPIO_AF_USART1

         #if USE_PIN_SET == 1
                #define RCC_GPIO RCC_AHB1Periph_GPIOB
                #define PORT GPIOB
                #define TX_PIN_SOURCE GPIO_PinSource6
                #define RX_PIN_SOURCE GPIO_PinSource7
                #define TX_PIN GPIO_Pin_6
                #define RX_PIN GPIO_Pin_7
         #endif

        #if USE_PIN_SET == 2
               #define RCC_GPIO RCC_AHB1Periph_GPIOA
               #define PORT GPIOA
               #define TX_PIN_SOURCE GPIO_PinSource9
               #define RX_PIN_SOURCE GPIO_PinSource10
               #define TX_PIN GPIO_Pin_9
               #define RX_PIN GPIO_Pin_10
        #endif
#endif

#if USE_USART == 2
        #define USART USART2
        #define RCC_USART RCC_APB1Periph_USART2
        #define RCC_APBxPeriphClockCmd RCC_APB1PeriphClockCmd
        #define GPIO_AF_USART GPIO_AF_USART2

         #if USE_PIN_SET == 1
                #define RCC_GPIO RCC_AHB1Periph_GPIOA
                #define PORT GPIOA
                #define TX_PIN_SOURCE GPIO_PinSource2
                #define RX_PIN_SOURCE GPIO_PinSource3
                #define TX_PIN GPIO_Pin_2
                #define RX_PIN GPIO_Pin_3
         #endif

        #if USE_PIN_SET == 2
               #define RCC_GPIO RCC_AHB1Periph_GPIOA
               #define PORT GPIOA
               #define TX_PIN_SOURCE GPIO_PinSource9
               #define RX_PIN_SOURCE GPIO_PinSource10
               #define TX_PIN GPIO_Pin_9
               #define RX_PIN GPIO_Pin_10
        #endif
#endif

        RCC_APBxPeriphClockCmd (RCC_USART, ENABLE);
        GPIO_InitTypeDef gpioInitStruct;

        RCC_AHB1PeriphClockCmd (RCC_GPIO, ENABLE);
        gpioInitStruct.GPIO_Pin = TX_PIN | RX_PIN;
        gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
        gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
        gpioInitStruct.GPIO_OType = GPIO_OType_PP;
        gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_Init (PORT, &gpioInitStruct);
        GPIO_PinAFConfig (PORT, TX_PIN_SOURCE, GPIO_AF_USART); // TX
        GPIO_PinAFConfig (PORT, RX_PIN_SOURCE, GPIO_AF_USART); // RX

        USART_InitTypeDef usartInitStruct;
        usartInitStruct.USART_BaudRate = 9600;
        usartInitStruct.USART_WordLength = USART_WordLength_8b;
        usartInitStruct.USART_StopBits = USART_StopBits_1;
        usartInitStruct.USART_Parity = USART_Parity_No;
        usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_Init (USART, &usartInitStruct);
        USART_Cmd (USART, ENABLE);
}

int main (void)
{
        initUsart ();
        printf ("Init\r\n");

//        /*!< At this stage the microcontroller clock setting is already configured,
//         this is done through SystemInit() function which is called from startup
//         files (startup_stm32f40_41xxx.s/startup_stm32f427_437xx.s)
//         before to branch to application main. To reconfigure the default setting
//         of SystemInit() function, refer to system_stm32f4xx.c file
//         */
//
//        /* NVIC Configuration */
//        NVIC_Configuration ();
//
//        /*------------------------------ SD Init ---------------------------------- */
//        if ((Status = SD_Init ()) != SD_OK) {
//                printf ("SD_Init went OK\r\n");
//        }
//
//        while ((Status == SD_OK) && (uwSDCardOperation != SD_OPERATION_END) && (SD_Detect () == SD_PRESENT)) {
//                switch (uwSDCardOperation) {
//                /*-------------------------- SD Erase Test ---------------------------- */
//                case (SD_OPERATION_ERASE):
//                {
//                        SD_EraseTest ();
//                        uwSDCardOperation = SD_OPERATION_BLOCK;
//                        break;
//                }
//                        /*-------------------------- SD Single Block Test --------------------- */
//                case (SD_OPERATION_BLOCK):
//                {
//                        SD_SingleBlockTest ();
//                        uwSDCardOperation = SD_OPERATION_MULTI_BLOCK;
//                        break;
//                }
//                        /*-------------------------- SD Multi Blocks Test --------------------- */
//                case (SD_OPERATION_MULTI_BLOCK):
//                {
//                        SD_MultiBlockTest ();
//                        uwSDCardOperation = SD_OPERATION_END;
//                        break;
//                }
//                }
//        }

        /* Infinite loop */
        while (1) {
        }
}
